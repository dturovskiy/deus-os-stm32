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
function Write-G3IndexedFailEvidence(
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
function Invoke-G3OfflineResultOwner {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)][string]$RepositoryRoot,
        [Parameter(Mandatory)][string]$ResultPath,
        [Parameter(Mandatory)][ValidateSet(
            'PREINIT_FAILURE','REMOTE_START_FAILED','SSH_TIMEOUT',
            'REMOTE_JSON_MISSING','REMOTE_JSON_INVALID','POST_ENTER_FAILURE',
            'EVIDENCE_FAILURE')][string]$Scenario
    )
    # This function deliberately cannot authorize a hardware PASS.
    $owner = 'stm32_os_gate3_result_owner_offline_qualification_v1'
    $classification = 'OFFLINE_PREINIT_FAILURE'
    $phase = 'PREINIT'
    $reason = 'NOT_COMPLETED'
    $possibleReset = $false
    $receipt = $null
    $evidence = $null
    $evidenceFailure = $false
    try {
        $parent=[IO.Path]::GetDirectoryName([IO.Path]::GetFullPath($ResultPath))
        Assert-G3 ([IO.Directory]::Exists($parent)) 'RESULT_PARENT_MISSING'
        Assert-G3 (-not [IO.File]::Exists($ResultPath)) 'RESULT_ALREADY_EXISTS'
        $evidence = Resolve-G3EvidenceDirectory $RepositoryRoot
        switch ($Scenario) {
            'PREINIT_FAILURE' { throw 'INJECTED_PREINIT_FAILURE' }
            'REMOTE_START_FAILED' {
                $classification='STOP_BEFORE_RESET'
                $phase='REMOTE_CHILD_NOT_STARTED'
                $reason='INJECTED_REMOTE_START_FAILED'
            }
            'SSH_TIMEOUT' {
                $classification='RESET_POSSIBLY_ATTEMPTED'
                $phase='REMOTE_CHILD_STARTED'
                $reason='INJECTED_SSH_TIMEOUT'
                $possibleReset=$true
            }
            'REMOTE_JSON_MISSING' {
                $classification='RESET_POSSIBLY_ATTEMPTED'
                $phase='REMOTE_CHILD_STARTED'
                $reason='INJECTED_RESULT_MISSING'
                $possibleReset=$true
            }
            'REMOTE_JSON_INVALID' {
                $classification='RESET_POSSIBLY_ATTEMPTED'
                $phase='REMOTE_CHILD_STARTED'
                $reason='INJECTED_RESULT_INVALID'
                $possibleReset=$true
            }
            'POST_ENTER_FAILURE' {
                $classification='STOP_AFTER_ONE_ENTER_NO_RETRY'
                $phase='POST_ENTER'
                $reason='INJECTED_POST_ENTER_FAILURE'
                $possibleReset=$true
            }
            'EVIDENCE_FAILURE' {
                $classification='RESET_POSSIBLY_ATTEMPTED'
                $phase='REMOTE_CHILD_STARTED'
                $reason='INJECTED_EVIDENCE_FAILURE'
                $possibleReset=$true
                $evidenceFailure=$true
            }
        }
    }
    catch {
        $classification='OFFLINE_PREINIT_FAILURE'
        $phase='PREINIT'
        $reason='SANITIZED_PREINIT_FAILURE'
    }
    finally {
        $report=[ordered]@{
            schema='DEUS_G3_OFFLINE_RESULT_OWNER_QUALIFICATION_V1'
            scenario=$Scenario
            result='FAIL'
            classification=$classification
            failure_phase=$phase
            error=$reason
            remote_start_boundary=$(if($possibleReset){'POSSIBLY_ENTERED'}else{'NOT_STARTED'})
            reset_attempts=$(if($possibleReset){'UNKNOWN_POSSIBLY_ONE'}else{'ZERO_ONLY_IF_NOT_STARTED'})
            info_attempts='NOT_ATTESTED'
            retry_authorized='NO'
            flash_authorized='NO'
            full_gate3='NOT_ACCEPTED'
            gate4='NOT_AUTHORIZED'
            timestamp=(Get-Date).ToString('o')
        }
        if ($null -ne $evidence) {
            try {
                if ($evidenceFailure) { throw 'INJECTED_EVIDENCE_WRITE_FAILURE' }
                $receipt=Write-G3IndexedFailEvidence $evidence $owner $report
            }
            catch {
                $classification=$(if($possibleReset){'RESET_POSSIBLY_ATTEMPTED'}else{'EVIDENCE_STORAGE_FAILURE'})
                $phase='EVIDENCE_SEAL'
                $reason='EVIDENCE_SEAL_FAILED'
            }
        }
        else {
            $classification='EVIDENCE_STORAGE_FAILURE'
            $phase='EVIDENCE_ROOT'
            $reason='EVIDENCE_ROUTE_UNAVAILABLE'
        }
        $result=[ordered]@{
            schema='DEUS_OS_RUNNER_RESULT_V1'
            gate=$owner
            result='FAIL'
            classification=$classification
            failure_phase=$phase
            error=$reason
            candidate_identity='OFFLINE_RESULT_OWNER_ONLY_NOT_PHYSICAL'
            evidence_zip=$(if($null -ne $receipt){$receipt.Path}else{'NONE'})
            evidence_sha256=$(if($null -ne $receipt){$receipt.Sha256}else{'NONE'})
            metrics=('SCENARIO='+$Scenario+';RESET_POSSIBLY_ATTEMPTED='+$possibleReset+
                ';RETRY=NO;FLASH=NO;FULL_GATE3=NOT_ACCEPTED')
        }
        # One local writer. An invalid result path is fatal; never emit PASS.
        $parent=[IO.Path]::GetDirectoryName([IO.Path]::GetFullPath($ResultPath))
        Assert-G3 ([IO.Directory]::Exists($parent)) 'RESULT_PARENT_MISSING'
        Assert-G3 (-not [IO.File]::Exists($ResultPath)) 'RESULT_ALREADY_EXISTS'
        $json=($result | ConvertTo-Json -Depth 8)
        $bytes=[Text.UTF8Encoding]::new($false).GetBytes($json+[char]10)
        $file=[IO.File]::Open($ResultPath,[IO.FileMode]::CreateNew,
            [IO.FileAccess]::Write,[IO.FileShare]::None)
        try { $file.Write($bytes,0,$bytes.Length); $file.Flush($true) }
        finally { $file.Dispose() }
    }
    return [pscustomobject]$result
}
Export-ModuleMember -Function Invoke-G3OfflineResultOwner
