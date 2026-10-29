#requires -Version 7.0
# ONLY synthetic filesystem / synthetic error paths. No device/SSH/flash calls.
[CmdletBinding()]
param()
Set-StrictMode -Version 3
$ErrorActionPreference='Stop'

function Require([bool]$Condition,[string]$Code) {
    if(-not $Condition){ throw $Code }
}
Import-Module (Join-Path $PSScriptRoot 'windows_result_owner_offline.psm1') -Force -ErrorAction Stop

$workspace=Join-Path ([IO.Path]::GetTempPath()) ('g3_win_offline_'+[guid]::NewGuid().ToString('N'))
$repo=Join-Path $workspace 'OS'
$evidence=Join-Path $workspace 'Evidence'
$caseRoot=Join-Path $workspace 'case_results'
$null=[IO.Directory]::CreateDirectory((Join-Path $repo '.git'))
$null=[IO.Directory]::CreateDirectory($evidence)
$null=[IO.Directory]::CreateDirectory($caseRoot)
$count=0
try {
    $scenarios=@(
        @('PREINIT_FAILURE','OFFLINE_PREINIT_FAILURE','PREINIT','YES'),
        @('REMOTE_START_FAILED','STOP_BEFORE_RESET','REMOTE_CHILD_NOT_STARTED','YES'),
        @('SSH_TIMEOUT','RESET_POSSIBLY_ATTEMPTED','REMOTE_CHILD_STARTED','YES'),
        @('REMOTE_JSON_MISSING','RESET_POSSIBLY_ATTEMPTED','REMOTE_CHILD_STARTED','YES'),
        @('REMOTE_JSON_INVALID','RESET_POSSIBLY_ATTEMPTED','REMOTE_CHILD_STARTED','YES'),
        @('POST_ENTER_FAILURE','STOP_AFTER_ONE_ENTER_NO_RETRY','POST_ENTER','YES'),
        @('EVIDENCE_FAILURE','RESET_POSSIBLY_ATTEMPTED','EVIDENCE_SEAL','NO')
    )
    foreach($case in $scenarios) {
        $name=[string]$case[0]
        $resultPath=Join-Path $caseRoot ($name+'.json')
        $r=Invoke-G3OfflineResultOwner -RepositoryRoot $repo -ResultPath $resultPath -Scenario $name
        Require ($r.result -ceq 'FAIL' -and $r.classification -ceq [string]$case[1]) ('CLASS_DRIFT_'+$name)
        Require ($r.failure_phase -ceq [string]$case[2]) ('PHASE_DRIFT_'+$name)
        Require ([IO.File]::Exists($resultPath)) ('RESULT_ABSENT_'+$name)
        $disk=Get-Content -LiteralPath $resultPath -Raw | ConvertFrom-Json -AsHashtable
        Require ($disk.gate -ceq 'stm32_os_gate3_result_owner_offline_qualification_v1') ('RESULT_GATE_'+$name)
        Require ($disk.result -ceq 'FAIL' -and $disk.error -ne 'NONE') ('FAIL_RESULT_'+$name)
        Require ($disk.classification -ceq [string]$case[1]) ('RESULT_CLASS_'+$name)
        Require ($disk.metrics.Contains('RETRY=NO;FLASH=NO;FULL_GATE3=NOT_ACCEPTED')) ('UNSAFE_METRICS_'+$name)
        if([string]$case[3] -ceq 'YES') {
            Require ([IO.File]::Exists([string]$disk.evidence_zip)) ('MISSING_EVIDENCE_'+$name)
            $hash=(Get-FileHash -LiteralPath $disk.evidence_zip -Algorithm SHA256).Hash
            Require ($hash -ceq [string]$disk.evidence_sha256) ('EVIDENCE_HASH_'+$name)
            $zip=[IO.Compression.ZipFile]::OpenRead([string]$disk.evidence_zip)
            try {
                Require ($zip.Entries.Count -eq 3) ('ZIP_MEMBER_COUNT_'+$name)
                foreach($entry in $zip.Entries) {
                    Require ($entry.FullName -cin @('outcome.txt','run.log','hashes.sha256')) ('ZIP_MEMBER_NAME_'+$name)
                }
            } finally { $zip.Dispose() }
        } else {
            Require ($disk.evidence_zip -ceq 'NONE' -and $disk.evidence_sha256 -ceq 'NONE') ('EVIDENCE_FAILURE_NOT_FAIL_CLOSED_'+$name)
        }
        $count++
        Write-Output ('G3_WINDOWS_OWNER_CASE=PASS SCENARIO='+$name)
    }

    # Unknown/missing repository => classified FAIL, never redirects evidence.
    $invalid=Join-Path $caseRoot 'INVALID_REPO.json'
    $bad=Invoke-G3OfflineResultOwner -RepositoryRoot (Join-Path $workspace 'missing\OS') -ResultPath $invalid -Scenario PREINIT_FAILURE
    Require ($bad.classification -ceq 'EVIDENCE_STORAGE_FAILURE') 'WRONG_REPO_ACCEPTED'
    Require ($bad.evidence_zip -ceq 'NONE') 'WRONG_REPO_EVIDENCE_FALLBACK'
    $count++
    Write-Output 'G3_WINDOWS_OWNER_CASE=PASS SCENARIO=INVALID_REPO'

    # Evidence directory is not auto-created and does not fall back to TEMP.
    [IO.Directory]::Move($evidence,($evidence+'_renamed'))
    $missingEvidence=Join-Path $caseRoot 'EVIDENCE_ROOT_MISSING.json'
    $bad=Invoke-G3OfflineResultOwner -RepositoryRoot $repo -ResultPath $missingEvidence -Scenario PREINIT_FAILURE
    Require ($bad.classification -ceq 'EVIDENCE_STORAGE_FAILURE') 'MISSING_EVIDENCE_ROOT_ACCEPTED'
    Require ($bad.evidence_zip -ceq 'NONE') 'MISSING_EVIDENCE_ROOT_FALLBACK'
    $count++
    Write-Output 'G3_WINDOWS_OWNER_CASE=PASS SCENARIO=EVIDENCE_ROOT_MISSING'

    # A duplicate result path must fail closed without overwriting original.
    $duplicate=Join-Path $caseRoot 'PREINIT_FAILURE.json'
    $originalBytes=[IO.File]::ReadAllBytes($duplicate)
    $rejected=$false
    try {
        $null=Invoke-G3OfflineResultOwner -RepositoryRoot $repo -ResultPath $duplicate -Scenario PREINIT_FAILURE
    } catch { $rejected=$true }
    Require $rejected 'DUPLICATE_RESULT_ACCEPTED'
    Require ([Convert]::ToHexString([IO.File]::ReadAllBytes($duplicate)) -ceq
             [Convert]::ToHexString($originalBytes)) 'DUPLICATE_RESULT_OVERWRITTEN'
    $count++
    Write-Output 'G3_WINDOWS_OWNER_CASE=PASS SCENARIO=DUPLICATE_RESULT_REJECTED'

    Write-Output ('GATE3_WINDOWS_RESULT_OWNER_OFFLINE=PASS COUNT='+$count+' USB=NO SSH=NO RESET=NO FLASH=NO')
} finally {
    if([IO.Directory]::Exists($workspace)) {
        [IO.Directory]::Delete($workspace,$true)
    }
}
