#requires -Version 7.0
# Owner-Windows source-only RA03 adapter: one consumed nonce, NEVER a release.
# Controller pins whole source; this is not a stand-alone device operator.
param(
    [Parameter(Mandatory)][string]$LedgerDirectory,
    [Parameter(Mandatory)][string]$LedgerModule,
    [Parameter(Mandatory)][string]$Nonce,
    [Parameter(Mandatory)][string]$StatementSha256,
    [Parameter(Mandatory)][string]$OperatorZipSha256
)
Set-StrictMode -Version 3
$ErrorActionPreference='Stop'
try {
    if(-not [OperatingSystem]::IsWindows()){throw 'RA03_WINDOWS_ONLY'}
    if($Nonce -cnotmatch '\A[a-f0-9]{32}\z' -or $Nonce -ceq ('0'*32)){
        throw 'RA03_NONCE_FORMAT'
    }
    foreach($h in @($StatementSha256,$OperatorZipSha256)){
        if($h -cnotmatch '\A[A-F0-9]{64}\z'){throw 'RA03_DIGEST_FORMAT'}
    }
    # Only the exact accepted RA03 module, never the operator bundle.
    $raw=[IO.File]::ReadAllBytes($LedgerModule)
    $sha=[Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($raw))
    if($sha -cne 'D67EB9820C016902FCDBC7FD3A13A51AE3F1317C33927E60B49D6B5565C351BC'){
        throw 'RA03_MODULE_IDENTITY_CHANGED'
    }
    Import-Module -Name $LedgerModule -Force -ErrorAction Stop
    $value=Write-G4ConsumedNonceSourceOnly -LedgerDirectory $LedgerDirectory -Nonce $Nonce -StatementSha256 $StatementSha256 -OperatorZipSha256 $OperatorZipSha256
    if($value.classification -cne 'RA03_DURABLE_SOURCE_ONLY_NONCE_CONSUMED' -or
       $value.authorization -cne 'DENIED' -or
       $value.launch_authorized -ne $false -or
       $value.device_io -cne 'NONE' -or
       $value.nonce -cne $Nonce -or
       $value.record_sha256 -cnotmatch '\A[A-F0-9]{64}\z'){
        throw 'RA03_UNTRUSTED_WRITE_RESULT'
    }
    $record=[ordered]@{
        classification=[string]$value.classification
        nonce=[string]$value.nonce
        record_sha256=[string]$value.record_sha256
        authorization='DENIED'
        launch_authorized=$false
        device_io='NONE'
    }
    [Console]::Out.WriteLine(($record|ConvertTo-Json -Compress -Depth 3))
    exit 0
} catch {
    [Console]::Error.WriteLine('RA03_ONE_ATTEMPT_STOP')
    exit 1
}
