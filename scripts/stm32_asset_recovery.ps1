param(
    [Parameter(Mandatory=$true)]
    [ValidateSet("PRESERVE_PERSISTENCE","CLEAN_STATE")]
    [string]$Mode,
    [Parameter(Mandatory=$true)]
    [string]$BundleDir,
    [string]$ProgrammerCli = "D:\Projects\STM32\Tools\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
    [ValidateRange(125,950)]
    [int]$SwdKHz = 950
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = "Stop"

$FlashBytes = 65536
$ApplicationBytes = 55296
$PersistenceOffset = 0xF800
$PersistenceBytes = 2048

if (-not (Test-Path -LiteralPath $ProgrammerCli -PathType Leaf)) {
    throw "STM32CubeProgrammer CLI not found: $ProgrammerCli"
}
if (-not (Test-Path -LiteralPath $BundleDir -PathType Container)) {
    throw "Recovery bundle directory not found: $BundleDir"
}

$ManifestPath = Join-Path $BundleDir "manifest.json"
$HashesPath = Join-Path $BundleDir "hashes.sha256"
$ApplicationPath = Join-Path $BundleDir "application_region_54k.bin"

foreach ($required in @($ManifestPath,$HashesPath,$ApplicationPath)) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Recovery bundle file missing: $required"
    }
}

$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
if ([string]$manifest.boundary -ne "ASSET_CONFIGURATION_TRANSFER_FOUNDATION") {
    throw "Manifest boundary mismatch"
}
if ([string]$manifest.contract -ne "ASSET_CONFIGURATION_STLINK_RECOVERY_V1") {
    throw "Manifest recovery contract mismatch"
}
foreach ($identity in @(
    [pscustomobject]@{ Name = "base_head"; Value = [string]$manifest.base_head },
    [pscustomobject]@{ Name = "candidate_tree"; Value = [string]$manifest.candidate_tree },
    [pscustomobject]@{ Name = "firmware_source_tree"; Value = [string]$manifest.firmware_source_tree }
)) {
    if ($identity.Value -notmatch '^[0-9a-f]{40}$') {
        throw ("Manifest {0} identity is invalid" -f $identity.Name)
    }
}
if ([string]::IsNullOrWhiteSpace([string]$manifest.toolchain_version)) {
    throw "Manifest toolchain version is missing"
}
if ([string]::IsNullOrWhiteSpace([string]$manifest.cubeprogrammer_version)) {
    throw "Manifest CubeProgrammer version is missing"
}
if ([int64]$manifest.application_region_length -ne $ApplicationBytes) {
    throw "Manifest application-region length mismatch"
}
if ([int64]$manifest.expected_flash_bytes -ne $FlashBytes) {
    throw "Manifest expected Flash-size mismatch"
}
if ([string]$manifest.expected_device_id -ne "0x410") {
    throw "Manifest expected Device ID mismatch"
}
if ([string]$manifest.application_origin -ne "0x08000000") {
    throw "Manifest application origin mismatch"
}
if ([string]$manifest.relocation_headroom -ne "0x0800D800..0x0800F7FF") {
    throw "Manifest relocation-headroom mismatch"
}
if ([string]$manifest.persistence_slot_a -ne "0x0800F800" -or
    [string]$manifest.persistence_slot_b -ne "0x0800FC00") {
    throw "Manifest persistence addresses mismatch"
}

$expectedHashes = @{}
foreach ($line in Get-Content -LiteralPath $HashesPath) {
    if ($line -match '^([0-9A-Fa-f]{64})\s+\*?(.+)$') {
        $expectedHashes[$Matches[2].Trim()] = $Matches[1].ToUpperInvariant()
    }
}

foreach ($name in @(
    "manifest.json",
    "os.bin",
    "application_region_54k.bin",
    "recovery-common.ps1",
    "restore-preserve-persistence.ps1",
    "restore-clean-state.ps1",
    "README.txt"
)) {
    if (-not $expectedHashes.ContainsKey($name)) {
        throw "hashes.sha256 is missing $name"
    }

    $path = Join-Path $BundleDir $name
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Recovery bundle file missing: $path"
    }

    $actual = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToUpperInvariant()
    if ($actual -ne $expectedHashes[$name]) {
        throw "Recovery bundle SHA-256 mismatch for $name"
    }
}

$OsBinPath = Join-Path $BundleDir "os.bin"
$RecoveryCommonPath = Join-Path $BundleDir "recovery-common.ps1"
$PreservePath = Join-Path $BundleDir "restore-preserve-persistence.ps1"
$CleanPath = Join-Path $BundleDir "restore-clean-state.ps1"

if ((Get-Item -LiteralPath $OsBinPath).Length -ne [int64]$manifest.os_bin_size) {
    throw "Manifest os.bin size mismatch"
}
if ((Get-FileHash -LiteralPath $OsBinPath -Algorithm SHA256).Hash.ToUpperInvariant() -ne
    [string]$manifest.os_bin_sha256) {
    throw "Manifest os.bin SHA-256 mismatch"
}
if ((Get-FileHash -LiteralPath $ApplicationPath -Algorithm SHA256).Hash.ToUpperInvariant() -ne
    [string]$manifest.application_region_sha256) {
    throw "Manifest application-region SHA-256 mismatch"
}
if ((Get-FileHash -LiteralPath $RecoveryCommonPath -Algorithm SHA256).Hash.ToUpperInvariant() -ne
    [string]$manifest.recovery_common_sha256) {
    throw "Manifest recovery-common SHA-256 mismatch"
}
if ((Get-FileHash -LiteralPath $PreservePath -Algorithm SHA256).Hash.ToUpperInvariant() -ne
    [string]$manifest.restore_preserve_sha256) {
    throw "Manifest PRESERVE wrapper SHA-256 mismatch"
}
if ((Get-FileHash -LiteralPath $CleanPath -Algorithm SHA256).Hash.ToUpperInvariant() -ne
    [string]$manifest.restore_clean_sha256) {
    throw "Manifest CLEAN wrapper SHA-256 mismatch"
}

if ((Get-Item -LiteralPath $ApplicationPath).Length -ne $ApplicationBytes) {
    throw "application_region_54k.bin must be exactly $ApplicationBytes bytes"
}

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$EvidenceDir = Join-Path $BundleDir ("recovery_evidence_" + $timestamp)
New-Item -ItemType Directory -Path $EvidenceDir -Force | Out-Null
$LogPath = Join-Path $EvidenceDir "recovery.log"
$PreFlashPath = Join-Path $EvidenceDir "pre_recovery_flash.bin"
$PostFlashPath = Join-Path $EvidenceDir "post_recovery_flash.bin"
$PersistenceBackupPath = Join-Path $EvidenceDir "persistence_backup.bin"

$script:Log = [System.Collections.Generic.List[string]]::new()

function Add-Log {
    param([string]$Text)
    $script:Log.Add($Text)
    Write-Host $Text
}

function Save-Log {
    $script:Log | Set-Content -LiteralPath $LogPath -Encoding utf8
}

function Assert-NoForbiddenRecoveryArgument {
    param([string[]]$Arguments)

    $tokens = @($Arguments | ForEach-Object { $_.ToLowerInvariant() })
    if ($tokens -contains "all") {
        throw "Mass erase token 'all' is forbidden"
    }

    foreach ($token in $tokens) {
        if ($token -in @("-rdu","--readunprotect","--optionbytes")) {
            throw "Forbidden recovery mutation argument: $token"
        }
    }

    for ($i = 0; $i -lt $tokens.Count; ++$i) {
        if ($tokens[$i] -eq "-ob" -and (($i + 1) -ge $tokens.Count -or $tokens[$i + 1] -ne "displ")) {
            throw "Only '-ob displ' is permitted"
        }
    }
}

function Invoke-Cube {
    param(
        [Parameter(Mandatory=$true)][string]$Label,
        [Parameter(Mandatory=$true)][string[]]$Arguments
    )

    Assert-NoForbiddenRecoveryArgument -Arguments $Arguments
    Add-Log ""
    Add-Log ("SECTION=" + $Label)
    Add-Log ("COMMAND=STM32_Programmer_CLI.exe " + ($Arguments -join " "))

    $output = @(& $ProgrammerCli @Arguments 2>&1 | ForEach-Object { $_.ToString() })
    $exitCode = $LASTEXITCODE

    foreach ($line in $output) {
        Add-Log $line
    }

    Add-Log ("EXIT_CODE=" + $exitCode)

    if ($exitCode -ne 0) {
        Save-Log
        throw "$Label failed with exit $exitCode"
    }

    return ($output -join [Environment]::NewLine)
}

function Parse-RegisterValue {
    param([string]$Text,[string]$Address)

    $pattern = "(?im)" + [regex]::Escape($Address) + "\s*[:=]\s*(?:0x)?([0-9a-f]{8})"
    if ($Text -notmatch $pattern) {
        throw "Unable to parse register $Address from CubeProgrammer output"
    }

    return [Convert]::ToUInt32($Matches[1],16)
}

function Assert-EqualBytes {
    param([byte[]]$Actual,[int]$ActualOffset,[byte[]]$Expected,[int]$ExpectedOffset,[int]$Length,[string]$Label)

    for ($i = 0; $i -lt $Length; ++$i) {
        if ($Actual[$ActualOffset + $i] -ne $Expected[$ExpectedOffset + $i]) {
            throw "$Label mismatch at relative offset 0x$($i.ToString('X'))"
        }
    }
}

function Assert-ErasedBytes {
    param([byte[]]$Bytes,[int]$Offset,[int]$Length,[string]$Label)

    for ($i = 0; $i -lt $Length; ++$i) {
        if ($Bytes[$Offset + $i] -ne 0xFF) {
            throw "$Label is not erased at relative offset 0x$($i.ToString('X'))"
        }
    }
}

function Get-ExplicitPages {
    param([int]$LastPageInclusive)

    $pages = [System.Collections.Generic.List[string]]::new()
    for ($page = 0; $page -le $LastPageInclusive; ++$page) {
        $pages.Add($page.ToString([System.Globalization.CultureInfo]::InvariantCulture))
    }
    return @($pages)
}

Add-Log "STM32 OS ASSET/CONFIGURATION ST-LINK RECOVERY V1"
Add-Log ("TIMESTAMP=" + (Get-Date -Format o))
Add-Log ("MODE=" + $Mode)
Add-Log ("PROGRAMMER_CLI=" + $ProgrammerCli)
Add-Log ("SWD_KHZ=" + $SwdKHz)
Add-Log "MASS_ERASE=FORBIDDEN READ_UNPROTECT=FORBIDDEN OPTION_BYTE_MUTATION=FORBIDDEN"

$connection = Invoke-Cube -Label "PREFLIGHT_CONNECT" -Arguments @("-c","port=SWD","freq=$SwdKHz")
if ($connection -notmatch "(?i)Device\s+ID\s*:\s*0x410") {
    Save-Log
    throw "Unexpected target Device ID; expected 0x410"
}
if ($connection -notmatch "(?i)Voltage\s*:\s*([0-9]+(?:\.[0-9]+)?)\s*V") {
    Save-Log
    throw "Unable to parse target voltage from CubeProgrammer connection output"
}
$voltage = [double]::Parse($Matches[1],[System.Globalization.CultureInfo]::InvariantCulture)
if ($voltage -lt 2.0 -or $voltage -gt 3.6) {
    Save-Log
    throw "Target voltage $voltage V is outside 2.0..3.6 V Flash-programming range"
}
Add-Log ("TARGET_VOLTAGE=" + $voltage.ToString("F3",[System.Globalization.CultureInfo]::InvariantCulture))

$flashSizeOutput = Invoke-Cube -Label "PREFLIGHT_FLASH_SIZE" -Arguments @("-c","port=SWD","freq=$SwdKHz","-r32","0x1FFFF7E0","0x4")
$flashSizeWord = Parse-RegisterValue -Text $flashSizeOutput -Address "0x1FFFF7E0"
if (($flashSizeWord -band 0xFFFFu) -ne 64u) {
    Save-Log
    throw ("Unexpected factory Flash size register: 0x{0:X8}" -f $flashSizeWord)
}

$obrOutput = Invoke-Cube -Label "PREFLIGHT_OBR" -Arguments @("-c","port=SWD","freq=$SwdKHz","-r32","0x4002201C","0x4")
$obr = Parse-RegisterValue -Text $obrOutput -Address "0x4002201C"
if ($obr -ne 0x000003FCu) {
    Save-Log
    throw ("Unexpected FLASH_OBR: 0x{0:X8}" -f $obr)
}

$wrprOutput = Invoke-Cube -Label "PREFLIGHT_WRPR" -Arguments @("-c","port=SWD","freq=$SwdKHz","-r32","0x40022020","0x4")
$wrpr = Parse-RegisterValue -Text $wrprOutput -Address "0x40022020"
if ($wrpr -ne 0xFFFFFFFFu) {
    Save-Log
    throw ("Unexpected FLASH_WRPR: 0x{0:X8}" -f $wrpr)
}

$optionBytes = Invoke-Cube -Label "PREFLIGHT_OPTION_BYTES" -Arguments @("-c","port=SWD","freq=$SwdKHz","-ob","displ")
if ($optionBytes -notmatch "(?i)RDP") {
    Save-Log
    throw "Option-byte display did not contain RDP field"
}

Invoke-Cube -Label "PRE_RECOVERY_READBACK" -Arguments @("-c","port=SWD","freq=$SwdKHz","--upload","0x08000000","0x10000",$PreFlashPath) | Out-Null

$preFlash = [System.IO.File]::ReadAllBytes($PreFlashPath)
if ($preFlash.Length -ne $FlashBytes) {
    Save-Log
    throw "Pre-recovery readback length mismatch"
}
Add-Log ("PRE_RECOVERY_FLASH_SHA256=" + (Get-FileHash -LiteralPath $PreFlashPath -Algorithm SHA256).Hash.ToUpperInvariant())

if ($Mode -eq "PRESERVE_PERSISTENCE") {
    $persistence = New-Object byte[] $PersistenceBytes
    [Array]::Copy($preFlash,$PersistenceOffset,$persistence,0,$PersistenceBytes)
    [System.IO.File]::WriteAllBytes($PersistenceBackupPath,$persistence)
    Add-Log ("PERSISTENCE_BACKUP_SHA256=" + (Get-FileHash -LiteralPath $PersistenceBackupPath -Algorithm SHA256).Hash.ToUpperInvariant())
    $erasePages = Get-ExplicitPages -LastPageInclusive 61
} else {
    $erasePages = Get-ExplicitPages -LastPageInclusive 63
}

$eraseArgs = @("-c","port=SWD","freq=$SwdKHz","-e") + $erasePages
Invoke-Cube -Label "EXPLICIT_PAGE_ERASE" -Arguments $eraseArgs | Out-Null

Invoke-Cube -Label "APPLICATION_PROGRAM_VERIFY" -Arguments @("-c","port=SWD","freq=$SwdKHz","--skiperase","-w",$ApplicationPath,"0x08000000","-v") | Out-Null

Invoke-Cube -Label "POST_RECOVERY_READBACK" -Arguments @("-c","port=SWD","freq=$SwdKHz","--upload","0x08000000","0x10000",$PostFlashPath) | Out-Null

$postFlash = [System.IO.File]::ReadAllBytes($PostFlashPath)
$application = [System.IO.File]::ReadAllBytes($ApplicationPath)
if ($postFlash.Length -ne $FlashBytes) {
    Save-Log
    throw "Post-recovery readback length mismatch"
}

Assert-EqualBytes -Actual $postFlash -ActualOffset 0 -Expected $application -ExpectedOffset 0 -Length $ApplicationBytes -Label "application region"
Assert-ErasedBytes -Bytes $postFlash -Offset 0xD800 -Length 0x2000 -Label "relocation headroom"

if ($Mode -eq "PRESERVE_PERSISTENCE") {
    $persistence = [System.IO.File]::ReadAllBytes($PersistenceBackupPath)
    Assert-EqualBytes -Actual $postFlash -ActualOffset $PersistenceOffset -Expected $persistence -ExpectedOffset 0 -Length $PersistenceBytes -Label "preserved persistence"
} else {
    Assert-ErasedBytes -Bytes $postFlash -Offset $PersistenceOffset -Length $PersistenceBytes -Label "clean persistence"
}

Add-Log ("POST_RECOVERY_FLASH_SHA256=" + (Get-FileHash -LiteralPath $PostFlashPath -Algorithm SHA256).Hash.ToUpperInvariant())
Add-Log "BYTE_EXACT_READBACK=PASS"

Invoke-Cube -Label "FINAL_SOFTWARE_RESET" -Arguments @("-c","port=SWD","freq=$SwdKHz","-rst") | Out-Null

Add-Log "RECOVERY_FLASH_PHASE=PASS"
Add-Log "NOTE=Runtime HELLO/sysinfo/ping/OLED proof remains a separate acceptance-harness step."
Save-Log

$logSha = (Get-FileHash -LiteralPath $LogPath -Algorithm SHA256).Hash.ToUpperInvariant()
Write-Host ("RECOVERY_LOG=" + $LogPath)
Write-Host ("RECOVERY_LOG_SHA256=" + $logSha)
Write-Host "RECOVERY_OUTCOME=PASS"
