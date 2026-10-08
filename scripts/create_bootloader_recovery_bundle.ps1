param(
    [Parameter(Mandatory=$true)][string]$ProjectRoot,
    [Parameter(Mandatory=$true)][string]$BootloaderBin,
    [Parameter(Mandatory=$true)][string]$ApplicationBin,
    [Parameter(Mandatory=$true)][string]$BaselinePackage,
    [Parameter(Mandatory=$true)][string]$UpdatePackage,
    [Parameter(Mandatory=$true)][string]$UpdateKeyFile,
    [Parameter(Mandatory=$true)][string]$Gate3EvidenceSha256,
    [Parameter(Mandatory=$true)][string]$Gate3FullCandidateTree,
    [Parameter(Mandatory=$true)][string]$Gate3FirmwareCandidateTree,
    [Parameter(Mandatory=$true)][string]$Gate3HostCandidateTree,
    [Parameter(Mandatory=$true)][string]$RecoveryToolingTree,
    [Parameter(Mandatory=$true)][string]$OutputDir,
    [Parameter(Mandatory=$true)][string]$ToolchainVersion,
    [Parameter(Mandatory=$true)][string]$CubeProgrammerVersion
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'

$BootloaderRegionBytes = 8192
$ApplicationRegionBytes = 53248
$MetadataPageBytes = 1024
$RecoveryRegionBytes = 63488
$ApplicationOffset = 0x2000
$MetadataAOffset = 0xF000
$MetadataBOffset = 0xF400
$MetadataMarkerOffset = 0x50
$ExpectedProductId = [uint32]0x534F4544
$ExpectedTargetId = [uint16]0x0410
$WrongTargetId = [uint16]0x0411
$ApplicationOrigin = [uint32]0x08002000

foreach ($identity in @(
    [pscustomobject]@{ Name='Gate3FullCandidateTree'; Value=$Gate3FullCandidateTree },
    [pscustomobject]@{ Name='Gate3FirmwareCandidateTree'; Value=$Gate3FirmwareCandidateTree },
    [pscustomobject]@{ Name='Gate3HostCandidateTree'; Value=$Gate3HostCandidateTree },
    [pscustomobject]@{ Name='RecoveryToolingTree'; Value=$RecoveryToolingTree }
)) {
    if ($identity.Value -notmatch '^[0-9a-f]{40}$') {
        throw ('{0} must be 40 lowercase hex characters' -f $identity.Name)
    }
}
if ($Gate3EvidenceSha256 -notmatch '^[0-9A-F]{64}$') {
    throw 'Gate3EvidenceSha256 must be 64 uppercase hex characters'
}
if ([string]::IsNullOrWhiteSpace($ToolchainVersion) -or
    [string]::IsNullOrWhiteSpace($CubeProgrammerVersion)) {
    throw 'Tool versions must not be empty'
}

$RecoverySource = Join-Path $ProjectRoot 'scripts\stm32_bootloader_recovery.ps1'
foreach ($required in @(
    $BootloaderBin,$ApplicationBin,$BaselinePackage,$UpdatePackage,
    $UpdateKeyFile,$RecoverySource
)) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw ('Required recovery-bundle input missing: ' + $required)
    }
}

function Set-U16LE {
    param(
        [Parameter(Mandatory=$true)][byte[]]$Bytes,
        [Parameter(Mandatory=$true)][int]$Offset,
        [Parameter(Mandatory=$true)][uint16]$Value
    )
    $Bytes[$Offset] = [byte]($Value -band 0xFF)
    $Bytes[$Offset + 1] = [byte](($Value -shr 8) -band 0xFF)
}

function Read-U16LE {
    param([Parameter(Mandatory=$true)][byte[]]$Bytes,[Parameter(Mandatory=$true)][int]$Offset)
    return [uint16](([uint16]$Bytes[$Offset]) -bor (([uint16]$Bytes[$Offset + 1]) -shl 8))
}

function Read-U32LE {
    param([Parameter(Mandatory=$true)][byte[]]$Bytes,[Parameter(Mandatory=$true)][int]$Offset)
    return [uint32](
        ([uint32]$Bytes[$Offset]) -bor
        (([uint32]$Bytes[$Offset + 1]) -shl 8) -bor
        (([uint32]$Bytes[$Offset + 2]) -shl 16) -bor
        (([uint32]$Bytes[$Offset + 3]) -shl 24))
}

function Get-Sha256Hex {
    param([Parameter(Mandatory=$true)][byte[]]$Bytes)
    $sha = [System.Security.Cryptography.SHA256]::Create()
    try {
        return -join @($sha.ComputeHash($Bytes) | ForEach-Object { $_.ToString('X2') })
    }
    finally {
        $sha.Dispose()
    }
}

function Get-HmacTag {
    param(
        [Parameter(Mandatory=$true)][byte[]]$Key,
        [Parameter(Mandatory=$true)][byte[]]$Header
    )
    if ($Header.Length -ne 48) {
        throw 'HMAC header must be exactly 48 bytes'
    }
    $domain = [System.Text.Encoding]::ASCII.GetBytes('DEUSHDR1')
    $authenticated = [byte[]]::new(8 + 4 + 48)
    [Array]::Copy($domain,0,$authenticated,0,8)
    $authenticated[8] = 0x00
    $authenticated[9] = 0x20
    $authenticated[10] = 0x00
    $authenticated[11] = 0x08
    [Array]::Copy($Header,0,$authenticated,12,48)

    $hmac = [System.Security.Cryptography.HMACSHA256]::new($Key)
    try {
        return $hmac.ComputeHash($authenticated)
    }
    finally {
        $hmac.Dispose()
        [Array]::Clear($authenticated,0,$authenticated.Length)
    }
}

function Assert-EqualBytes {
    param(
        [Parameter(Mandatory=$true)][byte[]]$Actual,
        [Parameter(Mandatory=$true)][int]$ActualOffset,
        [Parameter(Mandatory=$true)][byte[]]$Expected,
        [Parameter(Mandatory=$true)][int]$ExpectedOffset,
        [Parameter(Mandatory=$true)][int]$Length,
        [Parameter(Mandatory=$true)][string]$Label
    )
    for ($i = 0; $i -lt $Length; ++$i) {
        if ($Actual[$ActualOffset + $i] -ne $Expected[$ExpectedOffset + $i]) {
            throw ('{0} mismatch at relative offset 0x{1:X}' -f $Label,$i)
        }
    }
}

function Assert-ErasedBytes {
    param(
        [Parameter(Mandatory=$true)][byte[]]$Bytes,
        [Parameter(Mandatory=$true)][int]$Offset,
        [Parameter(Mandatory=$true)][int]$Length,
        [Parameter(Mandatory=$true)][string]$Label
    )
    for ($i = 0; $i -lt $Length; ++$i) {
        if ($Bytes[$Offset + $i] -ne 0xFF) {
            throw ('{0} is not erased at relative offset 0x{1:X}' -f $Label,$i)
        }
    }
}

function Get-SubsequenceCount {
    param(
        [Parameter(Mandatory=$true)][byte[]]$Haystack,
        [Parameter(Mandatory=$true)][byte[]]$Needle
    )
    if ($Needle.Length -eq 0 -or $Needle.Length -gt $Haystack.Length) {
        return 0
    }
    $count = 0
    for ($offset = 0; $offset -le ($Haystack.Length - $Needle.Length); ++$offset) {
        $match = $true
        for ($index = 0; $index -lt $Needle.Length; ++$index) {
            if ($Haystack[$offset + $index] -ne $Needle[$index]) {
                $match = $false
                break
            }
        }
        if ($match) { ++$count }
    }
    return $count
}

function Validate-SignedPackage {
    param(
        [Parameter(Mandatory=$true)][byte[]]$PackageBytes,
        [Parameter(Mandatory=$true)][byte[]]$ApplicationBytes,
        [Parameter(Mandatory=$true)][byte[]]$Key,
        [Parameter(Mandatory=$true)][uint32]$ExpectedVersion,
        [Parameter(Mandatory=$true)][uint16]$ExpectedTarget,
        [Parameter(Mandatory=$true)][string]$Label
    )

    if ($PackageBytes.Length -ne (80 + $ApplicationBytes.Length)) {
        throw ($Label + ' package length mismatch')
    }
    if ((Read-U32LE -Bytes $PackageBytes -Offset 0) -ne $ExpectedProductId) {
        throw ($Label + ' product mismatch')
    }
    if ($PackageBytes[4] -ne 1 -or $PackageBytes[5] -ne 0) {
        throw ($Label + ' format/reserved mismatch')
    }
    if ((Read-U16LE -Bytes $PackageBytes -Offset 6) -ne $ExpectedTarget) {
        throw ($Label + ' target mismatch')
    }
    if ((Read-U32LE -Bytes $PackageBytes -Offset 8) -ne [uint32]$ApplicationBytes.Length) {
        throw ($Label + ' image length mismatch')
    }
    if ((Read-U32LE -Bytes $PackageBytes -Offset 12) -ne $ExpectedVersion) {
        throw ($Label + ' version mismatch')
    }

    Assert-EqualBytes -Actual $PackageBytes -ActualOffset 80 -Expected $ApplicationBytes -ExpectedOffset 0 -Length $ApplicationBytes.Length -Label ($Label + ' payload')

    $payloadSha = Get-Sha256Hex -Bytes $ApplicationBytes
    $headerDigest = -join @($PackageBytes[16..47] | ForEach-Object { $_.ToString('X2') })
    if ($payloadSha -cne $headerDigest) {
        throw ($Label + ' payload digest mismatch')
    }

    $header = [byte[]]::new(48)
    [Array]::Copy($PackageBytes,0,$header,0,48)
    $expectedTag = Get-HmacTag -Key $Key -Header $header
    try {
        Assert-EqualBytes -Actual $PackageBytes -ActualOffset 48 -Expected $expectedTag -ExpectedOffset 0 -Length 32 -Label ($Label + ' HMAC')
    }
    finally {
        [Array]::Clear($header,0,$header.Length)
        [Array]::Clear($expectedTag,0,$expectedTag.Length)
    }
}

$key = [System.IO.File]::ReadAllBytes($UpdateKeyFile)
if ($key.Length -ne 32) {
    [Array]::Clear($key,0,$key.Length)
    throw 'UpdateKeyFile must contain exactly 32 raw bytes'
}

try {
    $bootloader = [System.IO.File]::ReadAllBytes($BootloaderBin)
    $application = [System.IO.File]::ReadAllBytes($ApplicationBin)
    $baseline = [System.IO.File]::ReadAllBytes($BaselinePackage)
    $update = [System.IO.File]::ReadAllBytes($UpdatePackage)

    if ($bootloader.Length -lt 8 -or $bootloader.Length -gt $BootloaderRegionBytes) {
        throw ('Bootloader length {0} violates 8-KiB recovery region' -f $bootloader.Length)
    }
    if ($application.Length -lt 8 -or
        $application.Length -gt $ApplicationRegionBytes -or
        ($application.Length % 4) -ne 0) {
        throw ('Application length {0} violates recovery/application bounds' -f $application.Length)
    }

    $keyOccurrences = Get-SubsequenceCount -Haystack $bootloader -Needle $key
    if ($keyOccurrences -ne 1) {
        throw ('Bootloader embedded acceptance-key occurrence count expected=1 actual={0}' -f $keyOccurrences)
    }

    Validate-SignedPackage -PackageBytes $baseline -ApplicationBytes $application -Key $key -ExpectedVersion 1 -ExpectedTarget $ExpectedTargetId -Label 'baseline_v1'
    Validate-SignedPackage -PackageBytes $update -ApplicationBytes $application -Key $key -ExpectedVersion 2 -ExpectedTarget $ExpectedTargetId -Label 'update_v2'

    $wrongTarget = [byte[]]$update.Clone()
    Set-U16LE -Bytes $wrongTarget -Offset 6 -Value $WrongTargetId
    $wrongHeader = [byte[]]::new(48)
    [Array]::Copy($wrongTarget,0,$wrongHeader,0,48)
    $wrongTag = Get-HmacTag -Key $key -Header $wrongHeader
    try {
        [Array]::Copy($wrongTag,0,$wrongTarget,48,32)
    }
    finally {
        [Array]::Clear($wrongHeader,0,$wrongHeader.Length)
        [Array]::Clear($wrongTag,0,$wrongTag.Length)
    }
    Validate-SignedPackage -PackageBytes $wrongTarget -ApplicationBytes $application -Key $key -ExpectedVersion 2 -ExpectedTarget $WrongTargetId -Label 'wrong_target_v2'

    $recovery = [byte[]]::new($RecoveryRegionBytes)
    [Array]::Fill[byte]($recovery,0xFF)
    [Array]::Copy($bootloader,0,$recovery,0,$bootloader.Length)
    [Array]::Copy($application,0,$recovery,$ApplicationOffset,$application.Length)
    [Array]::Copy($baseline,0,$recovery,$MetadataAOffset,80)
    Set-U16LE -Bytes $recovery -Offset ($MetadataAOffset + $MetadataMarkerOffset) -Value 0xA55A

    Assert-ErasedBytes -Bytes $recovery -Offset $bootloader.Length -Length ($BootloaderRegionBytes - $bootloader.Length) -Label 'bootloader padding'
    Assert-ErasedBytes -Bytes $recovery -Offset ($ApplicationOffset + $application.Length) -Length ($ApplicationRegionBytes - $application.Length) -Label 'application padding'
    Assert-ErasedBytes -Bytes $recovery -Offset ($MetadataAOffset + $MetadataMarkerOffset + 2) -Length ($MetadataPageBytes - $MetadataMarkerOffset - 2) -Label 'metadata A tail'
    Assert-ErasedBytes -Bytes $recovery -Offset $MetadataBOffset -Length $MetadataPageBytes -Label 'metadata B'

    . (Join-Path $PSScriptRoot 'output_directory_safety.ps1')
    $OutputDir = Reset-DeusGeneratedOutputDirectory -ProjectRoot $ProjectRoot -OutputDir $OutputDir -OwnerId 'DEUS_OS_BOOTLOADER_RECOVERY_V1'

    $RecoveryRegionPath = Join-Path $OutputDir 'recovery_region_62pages.bin'
    $BaselineOutPath = Join-Path $OutputDir 'baseline_v1.pkg'
    $UpdateOutPath = Join-Path $OutputDir 'update_v2.pkg'
    $WrongTargetOutPath = Join-Path $OutputDir 'wrong_target_v2.pkg'
    $RecoveryCommonPath = Join-Path $OutputDir 'recovery-common.ps1'

    [System.IO.File]::WriteAllBytes($RecoveryRegionPath,$recovery)
    [System.IO.File]::WriteAllBytes($BaselineOutPath,$baseline)
    [System.IO.File]::WriteAllBytes($UpdateOutPath,$update)
    [System.IO.File]::WriteAllBytes($WrongTargetOutPath,$wrongTarget)
    Copy-Item -LiteralPath $RecoverySource -Destination $RecoveryCommonPath -Force

    $ValidateWrapper = @'
param()
Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'recovery-common.ps1') -Mode VALIDATE_ONLY -BundleDir $PSScriptRoot
'@
    $PreserveWrapper = @'
param(
    [Parameter(Mandatory=$true)][string]$EvidenceDir,
    [string]$ProgrammerCli = 'D:\Projects\STM32\Tools\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe',
    [int]$SwdKHz = 950
)
Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'recovery-common.ps1') -Mode PRESERVE_PERSISTENCE -BundleDir $PSScriptRoot -EvidenceDir $EvidenceDir -ProgrammerCli $ProgrammerCli -SwdKHz $SwdKHz
'@
    $CleanWrapper = @'
param(
    [Parameter(Mandatory=$true)][string]$EvidenceDir,
    [string]$ProgrammerCli = 'D:\Projects\STM32\Tools\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe',
    [int]$SwdKHz = 950
)
Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'recovery-common.ps1') -Mode CLEAN_STATE -BundleDir $PSScriptRoot -EvidenceDir $EvidenceDir -ProgrammerCli $ProgrammerCli -SwdKHz $SwdKHz
'@

    [System.IO.File]::WriteAllText(
        (Join-Path $OutputDir 'validate-only.ps1'),
        $ValidateWrapper,
        [System.Text.UTF8Encoding]::new($false))
    [System.IO.File]::WriteAllText(
        (Join-Path $OutputDir 'restore-preserve-persistence.ps1'),
        $PreserveWrapper,
        [System.Text.UTF8Encoding]::new($false))
    [System.IO.File]::WriteAllText(
        (Join-Path $OutputDir 'restore-clean-state.ps1'),
        $CleanWrapper,
        [System.Text.UTF8Encoding]::new($false))

    $Readme = @"
Deus OS Firmware Update / Bootloader immutable Gate-4 recovery bundle.

THIS BUNDLE IS A SENSITIVE OPERATIONAL ARTIFACT.
The raw acceptance key is NOT stored here, but recovery_region_62pages.bin contains the
compiled bootloader that embeds the verification key. Keep this ZIP private. Do not add it
to the repository, evidence ZIP or publication artifacts.

VALIDATE ONLY (no target I/O):
  pwsh -NoProfile -File .\validate-only.ps1

PRESERVE PERSISTENCE:
  pwsh -NoProfile -File .\restore-preserve-persistence.ps1 -EvidenceDir <outside-bundle-dir>

CLEAN STATE:
  pwsh -NoProfile -File .\restore-clean-state.ps1 -EvidenceDir <outside-bundle-dir>

Flash contract:
  pages 0..7   bootloader
  pages 8..59  relocated application
  page 60      authenticated baseline-v1 metadata A
  page 61      erased metadata B
  pages 62..63 persistence, excluded from recovery_region_62pages.bin

PRESERVE erases/programs pages 0..61 only and proves persistence byte-identical.
CLEAN explicitly erases pages 0..63, programs pages 0..61, and proves persistence erased.
Mass erase, read-unprotect and option-byte mutation are forbidden.
Full 64-KiB post-readback is authoritative.

Signed acceptance vectors:
  baseline_v1.pkg
  update_v2.pkg
  wrong_target_v2.pkg  (valid HMAC for target 0x0411; device must reject target)
"@
    [System.IO.File]::WriteAllText(
        (Join-Path $OutputDir 'README.txt'),
        $Readme,
        [System.Text.UTF8Encoding]::new($false))

    $manifest = [ordered]@{
        boundary = 'FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION'
        contract = 'DEUS_OS_BOOTLOADER_STLINK_RECOVERY_V1'
        gate3_evidence_sha256 = $Gate3EvidenceSha256
        gate3_full_candidate_tree = $Gate3FullCandidateTree
        gate3_firmware_candidate_tree = $Gate3FirmwareCandidateTree
        gate3_host_candidate_tree = $Gate3HostCandidateTree
        recovery_tooling_tree = $RecoveryToolingTree
        toolchain_version = $ToolchainVersion
        cubeprogrammer_version = $CubeProgrammerVersion
        expected_device_id = '0x410'
        expected_flash_bytes = 65536
        bootloader_range = '0x08000000..0x08001FFF'
        application_range = '0x08002000..0x0800EFFF'
        metadata_a = '0x0800F000'
        metadata_b = '0x0800F400'
        persistence_a = '0x0800F800'
        persistence_b = '0x0800FC00'
        recovery_region_length = $RecoveryRegionBytes
        bootloader_bin_size = $bootloader.Length
        bootloader_bin_sha256 = (Get-FileHash -LiteralPath $BootloaderBin -Algorithm SHA256).Hash.ToUpperInvariant()
        application_bin_size = $application.Length
        application_bin_sha256 = (Get-FileHash -LiteralPath $ApplicationBin -Algorithm SHA256).Hash.ToUpperInvariant()
        baseline_version = 1
        update_version = 2
        wrong_target_device_id = '0x0411'
        baseline_package_sha256 = (Get-FileHash -LiteralPath $BaselineOutPath -Algorithm SHA256).Hash.ToUpperInvariant()
        update_package_sha256 = (Get-FileHash -LiteralPath $UpdateOutPath -Algorithm SHA256).Hash.ToUpperInvariant()
        wrong_target_package_sha256 = (Get-FileHash -LiteralPath $WrongTargetOutPath -Algorithm SHA256).Hash.ToUpperInvariant()
        recovery_region_sha256 = (Get-FileHash -LiteralPath $RecoveryRegionPath -Algorithm SHA256).Hash.ToUpperInvariant()
        recovery_common_sha256 = (Get-FileHash -LiteralPath $RecoveryCommonPath -Algorithm SHA256).Hash.ToUpperInvariant()
        key_storage = 'RAW_KEY_NOT_STORED'
        bootloader_key_binding = 'SAME_TRANSIENT_KEY_EXACT_OCCURRENCE_COUNT_1'
    }

    $ManifestPath = Join-Path $OutputDir 'manifest.json'
    [System.IO.File]::WriteAllText(
        $ManifestPath,
        ($manifest | ConvertTo-Json -Depth 6),
        [System.Text.UTF8Encoding]::new($false))

    $HashNames = @(
        'manifest.json',
        'recovery_region_62pages.bin',
        'baseline_v1.pkg',
        'update_v2.pkg',
        'wrong_target_v2.pkg',
        'recovery-common.ps1',
        'restore-preserve-persistence.ps1',
        'restore-clean-state.ps1',
        'validate-only.ps1',
        'README.txt'
    )
    $HashLines = foreach ($name in $HashNames) {
        $hash = (Get-FileHash -LiteralPath (Join-Path $OutputDir $name) -Algorithm SHA256).Hash.ToUpperInvariant()
        ($hash + '  ' + $name)
    }
    [System.IO.File]::WriteAllLines(
        (Join-Path $OutputDir 'hashes.sha256'),
        $HashLines,
        [System.Text.Encoding]::ASCII)

    Write-Host ('RECOVERY_BUNDLE_DIR=' + $OutputDir)
    Write-Host ('RECOVERY_GATE3_EVIDENCE_SHA256=' + $Gate3EvidenceSha256)
    Write-Host ('RECOVERY_GATE3_FULL_TREE=' + $Gate3FullCandidateTree)
    Write-Host ('RECOVERY_GATE3_FIRMWARE_TREE=' + $Gate3FirmwareCandidateTree)
    Write-Host ('RECOVERY_GATE3_HOST_TREE=' + $Gate3HostCandidateTree)
    Write-Host ('RECOVERY_TOOLING_TREE=' + $RecoveryToolingTree)
    Write-Host ('RECOVERY_BOOTLOADER_KEY_MATCH_COUNT=' + $keyOccurrences)
    Write-Host ('RECOVERY_REGION_SHA256=' + $manifest.recovery_region_sha256)
    Write-Host ('RECOVERY_BASELINE_PACKAGE_SHA256=' + $manifest.baseline_package_sha256)
    Write-Host ('RECOVERY_UPDATE_PACKAGE_SHA256=' + $manifest.update_package_sha256)
    Write-Host ('RECOVERY_WRONG_TARGET_PACKAGE_SHA256=' + $manifest.wrong_target_package_sha256)
    Write-Host 'RECOVERY_RAW_KEY_STORED=NO'
    Write-Host 'RECOVERY_BUNDLE_GENERATION=PASS'
}
finally {
    if ($null -ne $key) {
        [Array]::Clear($key,0,$key.Length)
    }
}
