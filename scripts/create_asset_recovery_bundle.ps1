param(
    [Parameter(Mandatory=$true)][string]$ProjectRoot,
    [Parameter(Mandatory=$true)][string]$BuildOutputDir,
    [Parameter(Mandatory=$true)][string]$BaseHead,
    [Parameter(Mandatory=$true)][string]$CandidateTree,
    [Parameter(Mandatory=$true)][string]$FirmwareSourceTree,
    [Parameter(Mandatory=$true)][string]$OutputDir,
    [Parameter(Mandatory=$true)][string]$ToolchainVersion,
    [Parameter(Mandatory=$true)][string]$CubeProgrammerVersion
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = "Stop"

$GitIdentityPattern = '^[0-9a-f]{40}$'

foreach ($identity in @(
    [pscustomobject]@{ Name = "BaseHead"; Value = $BaseHead },
    [pscustomobject]@{ Name = "CandidateTree"; Value = $CandidateTree },
    [pscustomobject]@{ Name = "FirmwareSourceTree"; Value = $FirmwareSourceTree }
)) {
    if ($identity.Value -notmatch $GitIdentityPattern) {
        throw ("{0} must be 40 lowercase hex characters" -f $identity.Name)
    }
}

if ([string]::IsNullOrWhiteSpace($ToolchainVersion)) {
    throw "ToolchainVersion must not be empty"
}
if ([string]::IsNullOrWhiteSpace($CubeProgrammerVersion)) {
    throw "CubeProgrammerVersion must not be empty"
}

$BinPath = Join-Path $BuildOutputDir "os.bin"
$BuildScript = Join-Path $ProjectRoot "scripts\build_firmware.ps1"
$RecoveryScript = Join-Path $ProjectRoot "scripts\stm32_asset_recovery.ps1"

foreach ($required in @($BinPath,$BuildScript,$RecoveryScript)) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Required input missing: $required"
    }
}

$bin = [System.IO.File]::ReadAllBytes($BinPath)
if ($bin.Length -gt 54272) {
    throw "os.bin exceeds Asset Gate-2 Flash ceiling: $($bin.Length)"
}

if (Test-Path -LiteralPath $OutputDir) {
    Remove-Item -LiteralPath $OutputDir -Recurse -Force
}
New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null

Copy-Item -LiteralPath $BinPath -Destination (Join-Path $OutputDir "os.bin")
Copy-Item -LiteralPath $RecoveryScript -Destination (Join-Path $OutputDir "recovery-common.ps1")

$application = New-Object byte[] 55296
[Array]::Fill[byte]($application,0xFF)
[Array]::Copy($bin,0,$application,0,$bin.Length)
$ApplicationPath = Join-Path $OutputDir "application_region_54k.bin"
[System.IO.File]::WriteAllBytes($ApplicationPath,$application)

if ((Get-Item -LiteralPath $ApplicationPath).Length -ne 55296) {
    throw "application_region_54k.bin length mismatch"
}
for ($index = $bin.Length; $index -lt $application.Length; ++$index) {
    if ($application[$index] -ne 0xFF) {
        throw ("application_region_54k.bin padding mismatch at offset {0}" -f $index)
    }
}

$PreserveWrapper = @'
param(
    [string]$ProgrammerCli =
        "D:\Projects\STM32\Tools\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
    [int]$SwdKHz = 950
)
Set-StrictMode -Version 3.0
$ErrorActionPreference = "Stop"
& (Join-Path $PSScriptRoot "recovery-common.ps1") -Mode PRESERVE_PERSISTENCE -BundleDir $PSScriptRoot -ProgrammerCli $ProgrammerCli -SwdKHz $SwdKHz
'@

$CleanWrapper = @'
param(
    [string]$ProgrammerCli =
        "D:\Projects\STM32\Tools\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
    [int]$SwdKHz = 950
)
Set-StrictMode -Version 3.0
$ErrorActionPreference = "Stop"
& (Join-Path $PSScriptRoot "recovery-common.ps1") -Mode CLEAN_STATE -BundleDir $PSScriptRoot -ProgrammerCli $ProgrammerCli -SwdKHz $SwdKHz
'@

[System.IO.File]::WriteAllText(
    (Join-Path $OutputDir "restore-preserve-persistence.ps1"),
    $PreserveWrapper,
    [System.Text.UTF8Encoding]::new($false))
[System.IO.File]::WriteAllText(
    (Join-Path $OutputDir "restore-clean-state.ps1"),
    $CleanWrapper,
    [System.Text.UTF8Encoding]::new($false))

$Readme = @"
Deus OS Asset/Configuration recovery bundle.

PRESERVE:
  pwsh -NoProfile -ExecutionPolicy Bypass -File .\restore-preserve-persistence.ps1

CLEAN:
  pwsh -NoProfile -ExecutionPolicy Bypass -File .\restore-clean-state.ps1

Both modes require ST-LINK/SWD and STM32CubeProgrammer CLI.
Close the STM32CubeProgrammer GUI before running recovery.
PRESERVE never erases persistence pages 62/63.
CLEAN explicitly erases pages 0..63 by page code; it never uses mass erase.
Programming uses --skiperase after the explicit erase set.
Full 64-KiB readback is authoritative.
"@
[System.IO.File]::WriteAllText(
    (Join-Path $OutputDir "README.txt"),
    $Readme,
    [System.Text.UTF8Encoding]::new($false))

$manifest = [ordered]@{
    boundary = "ASSET_CONFIGURATION_TRANSFER_FOUNDATION"
    contract = "ASSET_CONFIGURATION_STLINK_RECOVERY_V1"
    base_head = $BaseHead
    candidate_tree = $CandidateTree
    firmware_source_tree = $FirmwareSourceTree
    build_script_sha256 = (Get-FileHash -LiteralPath $BuildScript -Algorithm SHA256).Hash.ToUpperInvariant()
    toolchain_version = $ToolchainVersion
    cubeprogrammer_version = $CubeProgrammerVersion
    os_bin_size = $bin.Length
    os_bin_sha256 = (Get-FileHash -LiteralPath (Join-Path $OutputDir "os.bin") -Algorithm SHA256).Hash.ToUpperInvariant()
    application_origin = "0x08000000"
    application_region_length = 55296
    application_region_sha256 = (Get-FileHash -LiteralPath $ApplicationPath -Algorithm SHA256).Hash.ToUpperInvariant()
    relocation_headroom = "0x0800D800..0x0800F7FF"
    persistence_slot_a = "0x0800F800"
    persistence_slot_b = "0x0800FC00"
    expected_device_id = "0x410"
    expected_flash_bytes = 65536
    recovery_common_sha256 = (Get-FileHash -LiteralPath (Join-Path $OutputDir "recovery-common.ps1") -Algorithm SHA256).Hash.ToUpperInvariant()
    restore_preserve_sha256 = (Get-FileHash -LiteralPath (Join-Path $OutputDir "restore-preserve-persistence.ps1") -Algorithm SHA256).Hash.ToUpperInvariant()
    restore_clean_sha256 = (Get-FileHash -LiteralPath (Join-Path $OutputDir "restore-clean-state.ps1") -Algorithm SHA256).Hash.ToUpperInvariant()
}

$ManifestPath = Join-Path $OutputDir "manifest.json"
$manifest | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $ManifestPath -Encoding utf8

$HashNames = @(
    "manifest.json",
    "os.bin",
    "application_region_54k.bin",
    "recovery-common.ps1",
    "restore-preserve-persistence.ps1",
    "restore-clean-state.ps1",
    "README.txt"
)

$HashLines = foreach ($name in $HashNames) {
    $sha = (Get-FileHash -LiteralPath (Join-Path $OutputDir $name) -Algorithm SHA256).Hash.ToUpperInvariant()
    "$sha  $name"
}
$HashLines | Set-Content -LiteralPath (Join-Path $OutputDir "hashes.sha256") -Encoding ascii

Write-Host ("RECOVERY_BUNDLE_DIR=" + $OutputDir)
Write-Host ("RECOVERY_BASE_HEAD=" + $manifest.base_head)
Write-Host ("RECOVERY_CANDIDATE_TREE=" + $manifest.candidate_tree)
Write-Host ("RECOVERY_FIRMWARE_SOURCE_TREE=" + $manifest.firmware_source_tree)
Write-Host ("RECOVERY_APPLICATION_REGION_SHA256=" + $manifest.application_region_sha256)
Write-Host "RECOVERY_BUNDLE_GENERATION=PASS"
