#requires -Version 7.0
# Internal child of RA-03 OFFLINE concurrency test; NEVER a physical operator.
param([Parameter(Mandatory)][string]$Ledger,[Parameter(Mandatory)][string]$Nonce,
      [Parameter(Mandatory)][string]$StatementSha,[Parameter(Mandatory)][string]$ZipSha)
$ErrorActionPreference='Stop'
try {
    Import-Module (Join-Path $PSScriptRoot 'nonce_ledger_offline.psm1') -Force -ErrorAction Stop
    $value=Write-G4ConsumedNonceSourceOnly -LedgerDirectory $Ledger -Nonce $Nonce -StatementSha256 $StatementSha -OperatorZipSha256 $ZipSha
    if($value.authorization -cne 'DENIED' -or $value.device_io -cne 'NONE'){exit 41}
    exit 0
}
catch {exit 21}
