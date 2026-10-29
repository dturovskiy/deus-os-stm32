#requires -Version 7.0
# Only synthetic in-TEMP source qualification. No ssh/device/reset/flash.
[CmdletBinding()]
param()
Set-StrictMode -Version 3
$ErrorActionPreference='Stop'
function Require([bool]$ok,[string]$message){if(-not $ok){throw $message}}
Import-Module (Join-Path $PSScriptRoot 'WindowsResultOwner.psm1') -Force -ErrorAction Stop
$root=Join-Path ([IO.Path]::GetTempPath()) ('deus_g3_v2_bridge_'+[guid]::NewGuid().ToString('N'))
$repo=Join-Path $root 'OS'
$evidence=Join-Path $root 'Evidence'
$reports=Join-Path $root 'results'
[void][IO.Directory]::CreateDirectory((Join-Path $repo '.git'))
[void][IO.Directory]::CreateDirectory($evidence)
[void][IO.Directory]::CreateDirectory($reports)
$count=0
try {
    # Synthetic remote result, composed in native PowerShell; NO Python.
    $validFile=Join-Path $reports 'valid_remote.json'
    $valid=[ordered]@{
        schema='DEUS_G3_000D_ONE_RESET_INFO_PHYSICAL_V1'
        result='PASS'; classification='SCOPED_PHYSICAL_ONE_ENTER_ONE_INFO_STOP_NO_FLASH'
        host='UBUNTU_MACMINI'; physical_port='usb:001:8'
        firmware_flash='NO'; firmware_erase='NO'; protocol_run_as_root='NO'
        physical_identity_authenticated='NO'; gate3_full='NOT_ACCEPTED'; gate4='NOT_AUTHORIZED'
        reset_attempts=1; info_attempts=1
        before=[ordered]@{pid='000c';port='usb:001:8';devnum=18}
        after=[ordered]@{pid='000d';port='usb:001:8';devnum=19}
        observed_detach_ordinal=1; observed_attach_ordinal=2
        usb_events=@(
            [ordered]@{event='none';ordinal=1;monotonic=2.0},
            [ordered]@{event='000d:19';ordinal=2;monotonic=2.3}
        )
        permission=[ordered]@{status='ALREADY_OWNER_0600';node='001/019'}
        enter=[ordered]@{
            exit=0;timeout=$false;stderr_empty=$true
            tokens=@(
                'RUNTIME_NEGOTIATED=PASS',
                'RUNTIME_TREE=ccbd50200ec7f0ec53da6b1048d7377271dfb7bd',
                'RUNTIME_CAPABILITIES=0x00000040',
                'ENTER_BOOTLOADER_REQUEST_BOUNDARY=ARMED_SINGLE_SEND',
                'ENTER_RESPONSE=OK_RESETTING')
        }
        info=[ordered]@{
            exit=0;timeout=$false;stderr_empty=$true
            tokens=@(
                'INFO_STOP=InfoObservedGateClosed',
                'INFO_OBSERVED=True',
                'INFO_VERSION_FLOOR=5',
                'INFO_COMMITTED=5',
                'PHYSICAL_IDENTITY_AUTHENTICATED=False',
                'DESTRUCTIVE_OPERATION_AUTHORIZED=False')
        }
    }
    [IO.File]::WriteAllText($validFile,($valid|ConvertTo-Json -Depth 16),
                            [Text.UTF8Encoding]::new($false))
    $badTree=Join-Path $reports 'wrong_tree.json'
    $invalidTree=(Get-Content -LiteralPath $validFile -Raw).Replace(
        'ccbd50200ec7f0ec53da6b1048d7377271dfb7bd',
        '081aac70c31909bf681254393e2288917cb9b8b2')
    [IO.File]::WriteAllText($badTree,$invalidTree,[Text.UTF8Encoding]::new($false))
    $raw=Join-Path $reports 'duplicate_remote.json'
    # Raw JSON duplicate keys intentionally retained, NEVER converted first.
    [IO.File]::WriteAllText($raw,
        '{"reset_attempts":1,"reset_attempts":0}',
        [Text.UTF8Encoding]::new($false))
    foreach($item in @(
        @('NOT_STARTED',$false,'UNAVAILABLE','MISSING','STOP_BEFORE_RESET'),
        @('SSH_TIMEOUT',$true,'UNAVAILABLE','MISSING','RESET_POSSIBLY_ATTEMPTED'),
        @('RAW_DUPLICATE',$true,'0',$raw,'RESET_POSSIBLY_ATTEMPTED'),
        @('REMOTE_EXIT_FAILED',$true,'9','MISSING','RESET_POSSIBLY_ATTEMPTED'),
        @('VALID_SCOPED_RECEIPT',$true,'0',$validFile,'SCOPED_RECEIPT_VALID_SOURCE_ONLY_NOT_RELEASED'),
        @('WRONG_RUNTIME_TREE',$true,'0',$badTree,'RESET_POSSIBLY_ATTEMPTED')
    )) {
        $name=[string]$item[0]
        $resultPath=Join-Path $reports ($name+'.json')
        $result=Complete-G3CapturedResult -RepositoryRoot $repo -ResultPath $resultPath -RemoteStarted ([bool]$item[1]) -ProcessExit ([string]$item[2]) -OriginalRemoteJsonPath ([string]$item[3])
        Require ($result.result -ceq 'FAIL' -and $result.classification -ceq [string]$item[4]) ('BAD_CLASS_'+$name)
        Require ([IO.File]::Exists($resultPath)) ('MISSING_RESULT_'+$name)
        $disk=Get-Content -LiteralPath $resultPath -Raw | ConvertFrom-Json -AsHashtable
        Require ($disk.gate -ceq 'stm32_os_gate3_capture_source_candidate_v2_unreleased') ('GATE_ID_DRIFT_'+$name)
        Require ($disk.classification -ceq [string]$item[4]) ('DISK_CLASS_DRIFT_'+$name)
        Require ($disk.metrics.Contains('RETRY=NO;FLASH=NO;FULL_GATE3=NOT_ACCEPTED')) ('UNSAFE_CLAIM_'+$name)
        Require ([IO.File]::Exists([string]$disk.evidence_zip)) ('EVIDENCE_MISSING_'+$name)
        $sha=(Get-FileHash -LiteralPath $disk.evidence_zip -Algorithm SHA256).Hash
        Require ($sha -ceq [string]$disk.evidence_sha256) ('EVIDENCE_SHA_DRIFT_'+$name)
        $zip=[IO.Compression.ZipFile]::OpenRead([string]$disk.evidence_zip)
        try {Require ($zip.Entries.Count -eq 3) ('EVIDENCE_MEMBER_COUNT_'+$name)}
        finally {$zip.Dispose()}
        $count++
        Write-Output ('G3_SOURCE_V2_WINDOWS_BRIDGE_CASE=PASS SCENARIO='+$name)
    }
    Write-Output ('G3_SOURCE_V2_WINDOWS_BRIDGE=PASS COUNT='+$count+' LIVE_USB=NO SSH=NO RESET=NO FLASH=NO')
} finally {
    if([IO.Directory]::Exists($root)){[IO.Directory]::Delete($root,$true)}
}
