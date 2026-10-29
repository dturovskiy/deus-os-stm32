#requires -Version 7.0
# SOURCE-ONLY Windows failure/result owner. NO USB, SSH, sudo, reset, update, signing.
Set-StrictMode -Version 3

function Assert-G3([bool]$Condition, [string]$Code) {
    if (-not $Condition) { throw $Code }
}
function Assert-G3LocalPath([string]$Path) {
    $full = [IO.Path]::GetFullPath($Path)
    Assert-G3 (-not $full.StartsWith('\\')) 'UNC_PATH_NOT_ALLOWED'
    Assert-G3 ([IO.Path]::IsPathFullyQualified($full)) 'PATH_NOT_ABSOLUTE'
    $part = $full
    while (-not [string]::IsNullOrWhiteSpace($part)) {
        if ([IO.File]::Exists($part) -or [IO.Directory]::Exists($part)) {
            $attr = [IO.File]::GetAttributes($part)
            Assert-G3 (($attr -band [IO.FileAttributes]::ReparsePoint) -eq 0) 'REPARSE_COMPONENT'
        }
        $parent = [IO.Directory]::GetParent($part)
        if ($null -eq $parent -or $parent.FullName -ceq $part) { break }
        $part = $parent.FullName
    }
    return $full
}
function Resolve-G3EvidenceDirectory([string]$RepositoryRoot) {
    $repo = Assert-G3LocalPath $RepositoryRoot
    Assert-G3 ([IO.Directory]::Exists($repo)) 'OS_REPO_DIRECTORY_NOT_FOUND'
    Assert-G3 (([IO.Directory]::Exists((Join-Path $repo '.git'))) -or
               ([IO.File]::Exists((Join-Path $repo '.git')))) 'OS_GIT_CHECKOUT_NOT_VERIFIED'
    Assert-G3 ((Split-Path $repo -Leaf) -ceq 'OS') 'OS_REPOSITORY_NAME_NOT_EXACT'
    $parent = [IO.Directory]::GetParent($repo)
    Assert-G3 ($null -ne $parent) 'WORKSPACE_PARENT_NOT_FOUND'
    $evidence = Assert-G3LocalPath (Join-Path $parent.FullName 'Evidence')
    Assert-G3 ([IO.Directory]::Exists($evidence)) 'EVIDENCE_DIRECTORY_NOT_FOUND'
    Assert-G3 (-not $evidence.StartsWith($repo + [IO.Path]::DirectorySeparatorChar,
        [StringComparison]::OrdinalIgnoreCase)) 'EVIDENCE_INSIDE_REPOSITORY'
    return $evidence
}
function Get-G3Sha256([string]$Path) {
    return [Convert]::ToHexString(
        [Security.Cryptography.SHA256]::HashData([IO.File]::ReadAllBytes($Path)))
}
function Write-G3IndexedEvidence(
    [string]$EvidenceDirectory, [string]$RunId,
    [System.Collections.IDictionary]$Report
) {
    $id = [guid]::NewGuid().ToString('N')
    $stage = Join-Path $EvidenceDirectory ('.gate3_offline_' + $id)
    $target = Join-Path $EvidenceDirectory ($RunId + '_' + $id + '.evidence.zip')
    $temporary = $target + '.tmp'
    $utf8 = [Text.UTF8Encoding]::new($false)
    [void][IO.Directory]::CreateDirectory($stage)
    try {
        [IO.File]::WriteAllText((Join-Path $stage 'outcome.txt'),
            ($Report | ConvertTo-Json -Depth 10), $utf8)
        [IO.File]::WriteAllText((Join-Path $stage 'run.log'),
            ('GATE3_OFFLINE_OWNER=FAIL' + [Environment]::NewLine +
             'SCENARIO=' + $Report.scenario + [Environment]::NewLine +
             'CLASSIFICATION=' + $Report.classification + [Environment]::NewLine +
             'LIVE_USB=NO RESET=NO FLASH=NO' + [Environment]::NewLine), $utf8)
        $lines = [Collections.Generic.List[string]]::new()
        foreach ($name in @('outcome.txt', 'run.log')) {
            $f = Join-Path $stage $name
            [void]$lines.Add((Get-G3Sha256 $f) + '  ' + $name)
        }
        [IO.File]::WriteAllText((Join-Path $stage 'hashes.sha256'),
            (($lines.ToArray() -join [char]10) + [char]10), $utf8)
        Assert-G3 (-not [IO.File]::Exists($target)) 'EVIDENCE_COLLISION'
        Assert-G3 (-not [IO.File]::Exists($temporary)) 'EVIDENCE_TEMP_COLLISION'
        [IO.Compression.ZipFile]::CreateFromDirectory(
            $stage, $temporary, [IO.Compression.CompressionLevel]::Optimal, $false)
        $zip = [IO.Compression.ZipFile]::OpenRead($temporary)
        try {
            Assert-G3 ($zip.Entries.Count -eq 3) 'EVIDENCE_MEMBER_COUNT'
            foreach ($item in $zip.Entries) {
                Assert-G3 ($item.FullName -cin @('outcome.txt','run.log','hashes.sha256')) 'EVIDENCE_MEMBER_NAME'
                $stream = $item.Open()
                try {
                    $hash = [Security.Cryptography.SHA256]::Create()
                    try { $observed = [Convert]::ToHexString($hash.ComputeHash($stream)) }
                    finally { $hash.Dispose() }
                }
                finally { $stream.Dispose() }
                Assert-G3 ($observed -ceq (Get-G3Sha256 (Join-Path $stage $item.FullName))) 'EVIDENCE_MEMBER_SHA'
            }
        }
        finally { $zip.Dispose() }
        [IO.File]::Move($temporary, $target, $false)
        return [pscustomobject]@{ Path=$target; Sha256=(Get-G3Sha256 $target) }
    }
    finally {
        if ([IO.Directory]::Exists($stage)) {
            try { [IO.Directory]::Delete($stage, $true) } catch {}
        }
        if ([IO.File]::Exists($temporary)) {
            try { [IO.File]::Delete($temporary) } catch {}
        }
    }
}
# JSON policy implementation is a native PowerShell 7/.NET module.
Import-Module (Join-Path $PSScriptRoot 'WindowsNativeJsonGuard.psm1') -Force -ErrorAction Stop

function Complete-G3CapturedResult {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)][string]$RepositoryRoot,
        [Parameter(Mandatory)][string]$ResultPath,
        [Parameter(Mandatory)][bool]$RemoteStarted,
        [Parameter(Mandatory)][string]$ProcessExit,
        [Parameter(Mandatory)][string]$OriginalRemoteJsonPath
    )
    # This routine cannot execute SSH or a device request; a separate
    # released owner must capture the process boundary BEFORE Start().
    # No physical PASS can be produced by this unadmitted source candidate.
    $owner='stm32_os_gate3_capture_source_candidate_v2_unreleased'
    $phase='PREINIT'
    $reason='NOT_COMPLETED'
    $classification=$(if($RemoteStarted){'RESET_POSSIBLY_ATTEMPTED'}else{'STOP_BEFORE_RESET'})
    $decision='NOT_EVALUATED'
    $receipt=$null
    $evidence=$null
    try {
        $parent=[IO.Path]::GetDirectoryName([IO.Path]::GetFullPath($ResultPath))
        Assert-G3 ([IO.Directory]::Exists($parent)) 'RESULT_PARENT_MISSING'
        Assert-G3 (-not [IO.File]::Exists($ResultPath)) 'RESULT_ALREADY_EXISTS'
        $evidence=Resolve-G3EvidenceDirectory $RepositoryRoot
        $phase='ORIGINAL_REMOTE_RECEIPT_ADJUDICATION'
        $decision=Get-G3NativeRemoteDecision $RemoteStarted $ProcessExit $OriginalRemoteJsonPath
        $reason='SOURCE_ONLY_NATIVE_JSON_ADJUDICATION'
        $classification=$(if($decision -ceq 'SCOPED_PHYSICAL_ONE_ENTER_ONE_INFO_STOP_NO_FLASH'){
            'SCOPED_RECEIPT_VALID_SOURCE_ONLY_NOT_RELEASED'
        }else{$decision})
    }
    catch {
        $reason='SANITIZED_BRIDGE_OR_INIT_FAILURE'
        $classification=$(if($RemoteStarted){'RESET_POSSIBLY_ATTEMPTED'}else{'STOP_BEFORE_RESET'})
    }
    finally {
        $report=[ordered]@{
            schema='DEUS_G3_SOURCE_CANDIDATE_V2_CAPTURE_RESULT'
            scenario='CAPTURED_RESULT'
            result='FAIL'
            classification=$classification
            phase=$phase
            error=$reason
            remote_started=$RemoteStarted
            remote_exit=$ProcessExit
            adjudicated_scope=$decision
            reset_may_have_been_attempted=$RemoteStarted
            retry_authorized='NO'
            flash_authorized='NO'
            gate3_full='NOT_ACCEPTED'
            gate4='NOT_AUTHORIZED'
            timestamp=(Get-Date).ToString('o')
        }
        if($null -ne $evidence) {
            try {
                $receipt=Write-G3IndexedEvidence $evidence $owner $report
            } catch {
                $classification=$(if($RemoteStarted){'RESET_POSSIBLY_ATTEMPTED'}else{'EVIDENCE_STORAGE_FAILURE'})
                $reason='EVIDENCE_SEAL_FAILED'
                $phase='EVIDENCE_SEAL'
            }
        } else {
            if(-not $RemoteStarted){$classification='EVIDENCE_STORAGE_FAILURE'}
            $reason='EVIDENCE_ROOT_UNAVAILABLE'
            $phase='EVIDENCE_ROOT'
        }
        $result=[ordered]@{
            schema='DEUS_OS_RUNNER_RESULT_V1'
            gate=$owner
            result='FAIL'
            classification=$classification
            failure_phase=$phase
            error=$reason
            candidate_identity='V2_SOURCE_ONLY_NOT_OWNER_SIGNED_OR_RELEASED'
            evidence_zip=$(if($null -ne $receipt){$receipt.Path}else{'NONE'})
            evidence_sha256=$(if($null -ne $receipt){$receipt.Sha256}else{'NONE'})
            metrics=('REMOTE_STARTED='+$RemoteStarted+';ORIGINAL_SCOPE='+$decision+
                     ';RETRY=NO;FLASH=NO;FULL_GATE3=NOT_ACCEPTED')
        }
        $parent=[IO.Path]::GetDirectoryName([IO.Path]::GetFullPath($ResultPath))
        Assert-G3 ([IO.Directory]::Exists($parent)) 'RESULT_PARENT_MISSING'
        Assert-G3 (-not [IO.File]::Exists($ResultPath)) 'RESULT_ALREADY_EXISTS'
        $json=($result|ConvertTo-Json -Depth 8)
        $bytes=[Text.UTF8Encoding]::new($false).GetBytes($json+[char]10)
        $file=[IO.File]::Open($ResultPath,[IO.FileMode]::CreateNew,
                             [IO.FileAccess]::Write,[IO.FileShare]::None)
        try {$file.Write($bytes,0,$bytes.Length);$file.Flush($true)}
        finally {$file.Dispose()}
    }
    return [pscustomobject]$result
}
Export-ModuleMember -Function Complete-G3CapturedResult
