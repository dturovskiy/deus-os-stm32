param(
    [Parameter(Mandatory=$true)]
    [ValidateSet('VALIDATE_ONLY','PRESERVE_PERSISTENCE','CLEAN_STATE')]
    [string]$Mode,
    [Parameter(Mandatory=$true)]
    [string]$BundleDir,
    [string]$EvidenceDir = '',
    [string]$ProgrammerCli =
        'D:\Projects\STM32\Tools\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe',
    [ValidateRange(125,950)]
    [int]$SwdKHz = 950
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'

$FlashBytes = 65536
$RecoveryBytes = 63488
$BootloaderBytes = 8192
$ApplicationBytes = 53248
$MetadataPageBytes = 1024
$ApplicationOffset = 0x2000
$MetadataAOffset = 0xF000
$MetadataBOffset = 0xF400
$PersistenceOffset = 0xF800
$PersistenceBytes = 2048
$MarkerOffset = 0x50

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

function Get-SliceSha256 {
    param(
        [Parameter(Mandatory=$true)][byte[]]$Bytes,
        [Parameter(Mandatory=$true)][int]$Offset,
        [Parameter(Mandatory=$true)][int]$Length
    )
    $slice = [byte[]]::new($Length)
    [Array]::Copy($Bytes,$Offset,$slice,0,$Length)
    return Get-Sha256Hex -Bytes $slice
}

function Validate-Package {
    param(
        [Parameter(Mandatory=$true)][string]$Path,
        [Parameter(Mandatory=$true)][uint32]$ExpectedVersion,
        [Parameter(Mandatory=$true)][uint16]$ExpectedTarget,
        [Parameter(Mandatory=$true)][byte[]]$RecoveryRegion,
        [Parameter(Mandatory=$true)][int]$ApplicationSize,
        [Parameter(Mandatory=$true)][string]$Label
    )

    $packageBytes = [System.IO.File]::ReadAllBytes($Path)
    if ($packageBytes.Length -ne (80 + $ApplicationSize)) {
        throw ('{0} length mismatch' -f $Label)
    }
    if ((Read-U32LE -Bytes $packageBytes -Offset 0) -ne 0x534F4544) {
        throw ('{0} product ID mismatch' -f $Label)
    }
    if ($packageBytes[4] -ne 1 -or $packageBytes[5] -ne 0) {
        throw ('{0} format/reserved mismatch' -f $Label)
    }
    if ((Read-U16LE -Bytes $packageBytes -Offset 6) -ne $ExpectedTarget) {
        throw ('{0} target mismatch' -f $Label)
    }
    if ((Read-U32LE -Bytes $packageBytes -Offset 8) -ne [uint32]$ApplicationSize) {
        throw ('{0} image length mismatch' -f $Label)
    }
    if ((Read-U32LE -Bytes $packageBytes -Offset 12) -ne $ExpectedVersion) {
        throw ('{0} version mismatch' -f $Label)
    }

    $payload = [byte[]]::new($ApplicationSize)
    [Array]::Copy($packageBytes,80,$payload,0,$ApplicationSize)
    Assert-EqualBytes -Actual $payload -ActualOffset 0 -Expected $RecoveryRegion -ExpectedOffset $ApplicationOffset -Length $ApplicationSize -Label ($Label + ' payload')

    $payloadSha = Get-Sha256Hex -Bytes $payload
    $headerDigest = -join @($packageBytes[16..47] | ForEach-Object { $_.ToString('X2') })
    if ($payloadSha -cne $headerDigest) {
        throw ('{0} payload digest mismatch' -f $Label)
    }

    return $packageBytes
}

if (-not (Test-Path -LiteralPath $BundleDir -PathType Container)) {
    throw ('Recovery bundle directory not found: ' + $BundleDir)
}

$ManifestPath = Join-Path $BundleDir 'manifest.json'
$HashesPath = Join-Path $BundleDir 'hashes.sha256'
$RecoveryRegionPath = Join-Path $BundleDir 'recovery_region_62pages.bin'
$BaselinePackagePath = Join-Path $BundleDir 'baseline_v1.pkg'
$UpdatePackagePath = Join-Path $BundleDir 'update_v2.pkg'
$WrongTargetPackagePath = Join-Path $BundleDir 'wrong_target_v2.pkg'
$RecoveryCommonPath = Join-Path $BundleDir 'recovery-common.ps1'
$PreservePath = Join-Path $BundleDir 'restore-preserve-persistence.ps1'
$CleanPath = Join-Path $BundleDir 'restore-clean-state.ps1'
$ValidatePath = Join-Path $BundleDir 'validate-only.ps1'
$ReadmePath = Join-Path $BundleDir 'README.txt'

foreach ($required in @(
    $ManifestPath,$HashesPath,$RecoveryRegionPath,$BaselinePackagePath,
    $UpdatePackagePath,$WrongTargetPackagePath,$RecoveryCommonPath,
    $PreservePath,$CleanPath,$ValidatePath,$ReadmePath
)) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw ('Recovery bundle file missing: ' + $required)
    }
}

$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
if ([string]$manifest.boundary -ne 'FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION') {
    throw 'Manifest boundary mismatch'
}
if ([string]$manifest.contract -ne 'DEUS_OS_BOOTLOADER_STLINK_RECOVERY_V1') {
    throw 'Manifest recovery contract mismatch'
}
foreach ($identityName in @(
    'gate3_full_candidate_tree',
    'gate3_firmware_candidate_tree',
    'gate3_host_candidate_tree',
    'recovery_tooling_tree'
)) {
    $identityValue = [string]$manifest.$identityName
    if ($identityValue -notmatch '^[0-9a-f]{40}$') {
        throw ('Manifest identity invalid: ' + $identityName)
    }
}
if ([string]$manifest.gate3_evidence_sha256 -notmatch '^[0-9A-F]{64}$') {
    throw 'Manifest Gate-3 evidence SHA is invalid'
}
if ([int64]$manifest.recovery_region_length -ne $RecoveryBytes) {
    throw 'Manifest recovery-region length mismatch'
}
if ([int64]$manifest.expected_flash_bytes -ne $FlashBytes) {
    throw 'Manifest expected Flash length mismatch'
}
if ([string]$manifest.expected_device_id -ne '0x410') {
    throw 'Manifest expected Device ID mismatch'
}
if ([string]$manifest.bootloader_range -ne '0x08000000..0x08001FFF' -or
    [string]$manifest.application_range -ne '0x08002000..0x0800EFFF' -or
    [string]$manifest.metadata_a -ne '0x0800F000' -or
    [string]$manifest.metadata_b -ne '0x0800F400' -or
    [string]$manifest.persistence_a -ne '0x0800F800' -or
    [string]$manifest.persistence_b -ne '0x0800FC00') {
    throw 'Manifest Flash geometry mismatch'
}
if ([int]$manifest.baseline_version -ne 1 -or
    [int]$manifest.update_version -ne 2 -or
    [string]$manifest.wrong_target_device_id -ne '0x0411') {
    throw 'Manifest package-vector contract mismatch'
}

$expectedHashes = @{}
foreach ($line in Get-Content -LiteralPath $HashesPath) {
    if ($line -match '^([0-9A-Fa-f]{64})\s+\*?(.+)$') {
        $expectedHashes[$Matches[2].Trim()] = $Matches[1].ToUpperInvariant()
    }
}
$hashOwnedNames = @(
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
foreach ($name in $hashOwnedNames) {
    if (-not $expectedHashes.ContainsKey($name)) {
        throw ('hashes.sha256 is missing ' + $name)
    }
    $path = Join-Path $BundleDir $name
    $actualSha = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToUpperInvariant()
    if ($actualSha -cne [string]$expectedHashes[$name]) {
        throw ('Recovery bundle SHA-256 mismatch for ' + $name)
    }
}

$region = [System.IO.File]::ReadAllBytes($RecoveryRegionPath)
if ($region.Length -ne $RecoveryBytes) {
    throw 'recovery_region_62pages.bin length mismatch'
}
if ((Get-FileHash -LiteralPath $RecoveryRegionPath -Algorithm SHA256).Hash.ToUpperInvariant() -cne
    [string]$manifest.recovery_region_sha256) {
    throw 'Manifest recovery-region SHA mismatch'
}

$bootloaderSize = [int]$manifest.bootloader_bin_size
$applicationSize = [int]$manifest.application_bin_size
if ($bootloaderSize -lt 8 -or $bootloaderSize -gt $BootloaderBytes) {
    throw 'Manifest bootloader size invalid'
}
if ($applicationSize -lt 8 -or $applicationSize -gt $ApplicationBytes -or ($applicationSize % 4) -ne 0) {
    throw 'Manifest application size invalid'
}

if ((Get-SliceSha256 -Bytes $region -Offset 0 -Length $bootloaderSize) -cne
    [string]$manifest.bootloader_bin_sha256) {
    throw 'Bootloader bytes SHA mismatch inside recovery region'
}
Assert-ErasedBytes -Bytes $region -Offset $bootloaderSize -Length ($BootloaderBytes - $bootloaderSize) -Label 'bootloader padding'

if ((Get-SliceSha256 -Bytes $region -Offset $ApplicationOffset -Length $applicationSize) -cne
    [string]$manifest.application_bin_sha256) {
    throw 'Application bytes SHA mismatch inside recovery region'
}
Assert-ErasedBytes -Bytes $region -Offset ($ApplicationOffset + $applicationSize) -Length ($ApplicationBytes - $applicationSize) -Label 'application padding'

$baseline = Validate-Package -Path $BaselinePackagePath -ExpectedVersion 1 -ExpectedTarget 0x0410 -RecoveryRegion $region -ApplicationSize $applicationSize -Label 'baseline_v1.pkg'
$update = Validate-Package -Path $UpdatePackagePath -ExpectedVersion 2 -ExpectedTarget 0x0410 -RecoveryRegion $region -ApplicationSize $applicationSize -Label 'update_v2.pkg'
$wrongTarget = Validate-Package -Path $WrongTargetPackagePath -ExpectedVersion 2 -ExpectedTarget 0x0411 -RecoveryRegion $region -ApplicationSize $applicationSize -Label 'wrong_target_v2.pkg'

Assert-EqualBytes -Actual $region -ActualOffset $MetadataAOffset -Expected $baseline -ExpectedOffset 0 -Length 80 -Label 'metadata A header/tag'
if ((Read-U16LE -Bytes $region -Offset ($MetadataAOffset + $MarkerOffset)) -ne 0xA55A) {
    throw 'Metadata A commit marker mismatch'
}
Assert-ErasedBytes -Bytes $region -Offset ($MetadataAOffset + $MarkerOffset + 2) -Length ($MetadataPageBytes - $MarkerOffset - 2) -Label 'metadata A tail'
Assert-ErasedBytes -Bytes $region -Offset $MetadataBOffset -Length $MetadataPageBytes -Label 'metadata B'

Write-Host ('RECOVERY_BUNDLE_GATE3_EVIDENCE_SHA256=' + [string]$manifest.gate3_evidence_sha256)
Write-Host ('RECOVERY_BUNDLE_GATE3_FULL_TREE=' + [string]$manifest.gate3_full_candidate_tree)
Write-Host ('RECOVERY_BUNDLE_RECOVERY_TOOLING_TREE=' + [string]$manifest.recovery_tooling_tree)
Write-Host ('RECOVERY_REGION_SHA256=' + [string]$manifest.recovery_region_sha256)
Write-Host ('RECOVERY_BASELINE_PACKAGE_SHA256=' + [string]$manifest.baseline_package_sha256)
Write-Host ('RECOVERY_UPDATE_PACKAGE_SHA256=' + [string]$manifest.update_package_sha256)
Write-Host ('RECOVERY_WRONG_TARGET_PACKAGE_SHA256=' + [string]$manifest.wrong_target_package_sha256)
Write-Host 'RECOVERY_BUNDLE_VALIDATION=PASS'

if ($Mode -eq 'VALIDATE_ONLY') {
    Write-Host 'RECOVERY_OUTCOME=PASS'
    return
}

if (-not (Test-Path -LiteralPath $ProgrammerCli -PathType Leaf)) {
    throw ('STM32CubeProgrammer CLI not found: ' + $ProgrammerCli)
}

if ([string]::IsNullOrWhiteSpace($EvidenceDir)) {
    $EvidenceDir = Join-Path $env:TEMP ('deus_bootloader_recovery_' + (Get-Date -Format 'yyyyMMdd_HHmmss'))
}
if (Test-Path -LiteralPath $EvidenceDir) {
    throw ('Recovery EvidenceDir must not already exist: ' + $EvidenceDir)
}
New-Item -ItemType Directory -Path $EvidenceDir -Force | Out-Null

$LogPath = Join-Path $EvidenceDir 'recovery.log'
$PreFlashPath = Join-Path $EvidenceDir 'pre_recovery_flash.bin'
$PostFlashPath = Join-Path $EvidenceDir 'post_recovery_flash.bin'
$PersistenceBackupPath = Join-Path $EvidenceDir 'persistence_backup.bin'
$script:Log = [System.Collections.Generic.List[string]]::new()

function Add-Log {
    param([Parameter(Mandatory=$true)][AllowEmptyString()][string]$Text)
    [void]$script:Log.Add($Text)
    Write-Host $Text
}

function Save-Log {
    [System.IO.File]::WriteAllLines($LogPath,$script:Log,[System.Text.UTF8Encoding]::new($false))
}

function Assert-NoForbiddenRecoveryArgument {
    param([Parameter(Mandatory=$true)][string[]]$Arguments)
    $tokens = @($Arguments | ForEach-Object { $_.ToLowerInvariant() })
    if ($tokens -contains 'all') {
        throw "Mass erase token 'all' is forbidden"
    }
    foreach ($token in $tokens) {
        if ($token -in @('-rdu','--readunprotect','--optionbytes')) {
            throw ('Forbidden recovery mutation argument: ' + $token)
        }
    }
    for ($i = 0; $i -lt $tokens.Count; ++$i) {
        if ($tokens[$i] -eq '-ob' -and (($i + 1) -ge $tokens.Count -or $tokens[$i + 1] -ne 'displ')) {
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
    Add-Log -Text ''
    Add-Log -Text ('SECTION=' + $Label)
    Add-Log -Text ('COMMAND=STM32_Programmer_CLI.exe ' + ($Arguments -join ' '))

    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $ProgrammerCli
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.RedirectStandardInput = $true
    $psi.CreateNoWindow = $true
    foreach ($argument in $Arguments) {
        [void]$psi.ArgumentList.Add($argument)
    }

    $p = [System.Diagnostics.Process]::new()
    $p.StartInfo = $psi
    if (-not $p.Start()) {
        throw ('Unable to start CubeProgrammer for ' + $Label)
    }
    $p.StandardInput.Close()
    $stdoutTask = $p.StandardOutput.ReadToEndAsync()
    $stderrTask = $p.StandardError.ReadToEndAsync()
    if (-not $p.WaitForExit(120000)) {
        try { $p.Kill($true) } catch {}
        Save-Log
        throw ($Label + ' timed out')
    }
    $stdout = $stdoutTask.GetAwaiter().GetResult()
    $stderr = $stderrTask.GetAwaiter().GetResult()
    $exitCode = $p.ExitCode
    $p.Dispose()

    foreach ($line in @($stdout -split '\r?\n')) {
        if (-not [string]::IsNullOrEmpty($line)) { Add-Log -Text $line }
    }
    foreach ($line in @($stderr -split '\r?\n')) {
        if (-not [string]::IsNullOrEmpty($line)) { Add-Log -Text ('STDERR=' + $line) }
    }
    Add-Log -Text ('EXIT_CODE=' + $exitCode)
    if ($exitCode -ne 0) {
        Save-Log
        throw ('{0} failed with exit {1}' -f $Label,$exitCode)
    }
    return ($stdout + [Environment]::NewLine + $stderr)
}

function Parse-RegisterValue {
    param([Parameter(Mandatory=$true)][string]$Text,[Parameter(Mandatory=$true)][string]$Address)
    $pattern = '(?im)' + [regex]::Escape($Address) + '\s*[:=]\s*(?:0x)?([0-9a-f]{8})'
    if ($Text -notmatch $pattern) {
        throw ('Unable to parse register ' + $Address)
    }
    return [Convert]::ToUInt32($Matches[1],16)
}

function Assert-CubeConnectMode {
    param(
        [Parameter(Mandatory=$true)][string]$Text,
        [Parameter(Mandatory=$true)][ValidateSet('Hot Plug','Normal')][string]$Expected
    )
    if ($Text -notmatch ('(?im)^\s*Connect mode\s*:\s*' + [regex]::Escape($Expected) + '\s*$')) {
        throw ('CubeProgrammer connect-mode mismatch; expected {0}' -f $Expected)
    }
}

function Get-ExplicitPages {
    param([Parameter(Mandatory=$true)][int]$LastPageInclusive)
    $pages = [System.Collections.Generic.List[string]]::new()
    for ($page = 0; $page -le $LastPageInclusive; ++$page) {
        [void]$pages.Add($page.ToString([System.Globalization.CultureInfo]::InvariantCulture))
    }
    return @($pages)
}

Add-Log -Text 'DEUS OS BOOTLOADER ST-LINK RECOVERY V1'
Add-Log -Text ('TIMESTAMP=' + (Get-Date -Format o))
Add-Log -Text ('MODE=' + $Mode)
Add-Log -Text ('PROGRAMMER_CLI=' + $ProgrammerCli)
Add-Log -Text ('SWD_KHZ=' + $SwdKHz)
Add-Log -Text 'MASS_ERASE=FORBIDDEN READ_UNPROTECT=FORBIDDEN OPTION_BYTE_MUTATION=FORBIDDEN'

$connection = Invoke-Cube -Label 'PREFLIGHT_CONNECT' -Arguments @('-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz))
Assert-CubeConnectMode -Text $connection -Expected 'Hot Plug'
if ($connection -notmatch '(?i)Device\s+ID\s*:\s*0x410') {
    Save-Log
    throw 'Unexpected target Device ID; expected 0x410'
}
if ($connection -notmatch '(?i)Voltage\s*:\s*([0-9]+(?:\.[0-9]+)?)\s*V') {
    Save-Log
    throw 'Unable to parse target voltage'
}
$voltage = [double]::Parse($Matches[1],[System.Globalization.CultureInfo]::InvariantCulture)
if ($voltage -lt 2.0 -or $voltage -gt 3.6) {
    Save-Log
    throw ('Target voltage {0} V outside Flash-programming range' -f $voltage)
}
Add-Log -Text ('TARGET_VOLTAGE=' + $voltage.ToString('F3',[System.Globalization.CultureInfo]::InvariantCulture))

$flashSizeOutput = Invoke-Cube -Label 'PREFLIGHT_FLASH_SIZE' -Arguments @('-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz),'-r32','0x1FFFF7E0','0x4')
Assert-CubeConnectMode -Text $flashSizeOutput -Expected 'Hot Plug'
$flashSizeWord = Parse-RegisterValue -Text $flashSizeOutput -Address '0x1FFFF7E0'
if (($flashSizeWord -band 0xFFFF) -ne 64) {
    Save-Log
    throw ('Unexpected factory Flash size register: 0x{0:X8}' -f $flashSizeWord)
}

$obrOutput = Invoke-Cube -Label 'PREFLIGHT_OBR' -Arguments @('-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz),'-r32','0x4002201C','0x4')
Assert-CubeConnectMode -Text $obrOutput -Expected 'Hot Plug'
$obr = Parse-RegisterValue -Text $obrOutput -Address '0x4002201C'
if ($obr -ne 0x000003FC) {
    Save-Log
    throw ('Unexpected FLASH_OBR: 0x{0:X8}' -f $obr)
}

$wrprOutput = Invoke-Cube -Label 'PREFLIGHT_WRPR' -Arguments @('-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz),'-r32','0x40022020','0x4')
Assert-CubeConnectMode -Text $wrprOutput -Expected 'Hot Plug'
$wrpr = Parse-RegisterValue -Text $wrprOutput -Address '0x40022020'
if ($wrpr -ne [uint32]::MaxValue) {
    Save-Log
    throw ('Unexpected FLASH_WRPR: 0x{0:X8}' -f $wrpr)
}

$optionBytes = Invoke-Cube -Label 'PREFLIGHT_OPTION_BYTES' -Arguments @('-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz),'-ob','displ')
Assert-CubeConnectMode -Text $optionBytes -Expected 'Hot Plug'
if ($optionBytes -notmatch '(?i)RDP') {
    Save-Log
    throw 'Option-byte display did not contain RDP'
}

$preReadbackOutput = Invoke-Cube -Label 'PRE_RECOVERY_READBACK' -Arguments @('-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz),'--upload','0x08000000','0x10000',$PreFlashPath)
Assert-CubeConnectMode -Text $preReadbackOutput -Expected 'Hot Plug'
$preFlash = [System.IO.File]::ReadAllBytes($PreFlashPath)
if ($preFlash.Length -ne $FlashBytes) {
    Save-Log
    throw 'Pre-recovery readback length mismatch'
}
Add-Log -Text ('PRE_RECOVERY_FLASH_SHA256=' + (Get-FileHash -LiteralPath $PreFlashPath -Algorithm SHA256).Hash.ToUpperInvariant())

if ($Mode -eq 'PRESERVE_PERSISTENCE') {
    $persistence = [byte[]]::new($PersistenceBytes)
    [Array]::Copy($preFlash,$PersistenceOffset,$persistence,0,$PersistenceBytes)
    [System.IO.File]::WriteAllBytes($PersistenceBackupPath,$persistence)
    Add-Log -Text ('PERSISTENCE_BACKUP_SHA256=' + (Get-FileHash -LiteralPath $PersistenceBackupPath -Algorithm SHA256).Hash.ToUpperInvariant())
    $erasePages = Get-ExplicitPages -LastPageInclusive 61
}
else {
    $erasePages = Get-ExplicitPages -LastPageInclusive 63
}

$eraseArgs = @('-c','port=SWD','mode=NORMAL',('freq=' + $SwdKHz),'-e') + $erasePages
$eraseOutput = Invoke-Cube -Label 'EXPLICIT_PAGE_ERASE' -Arguments $eraseArgs
Assert-CubeConnectMode -Text $eraseOutput -Expected 'Normal'
$programOutput = Invoke-Cube -Label 'RECOVERY_REGION_PROGRAM_VERIFY' -Arguments @('-c','port=SWD','mode=NORMAL',('freq=' + $SwdKHz),'--skiperase','-w',$RecoveryRegionPath,'0x08000000','-v')
Assert-CubeConnectMode -Text $programOutput -Expected 'Normal'
$postReadbackOutput = Invoke-Cube -Label 'POST_RECOVERY_READBACK' -Arguments @('-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz),'--upload','0x08000000','0x10000',$PostFlashPath)
Assert-CubeConnectMode -Text $postReadbackOutput -Expected 'Hot Plug'

$postFlash = [System.IO.File]::ReadAllBytes($PostFlashPath)
if ($postFlash.Length -ne $FlashBytes) {
    Save-Log
    throw 'Post-recovery readback length mismatch'
}
Assert-EqualBytes -Actual $postFlash -ActualOffset 0 -Expected $region -ExpectedOffset 0 -Length $RecoveryBytes -Label 'recovery region pages 0..61'

if ($Mode -eq 'PRESERVE_PERSISTENCE') {
    $persistence = [System.IO.File]::ReadAllBytes($PersistenceBackupPath)
    Assert-EqualBytes -Actual $postFlash -ActualOffset $PersistenceOffset -Expected $persistence -ExpectedOffset 0 -Length $PersistenceBytes -Label 'preserved persistence'
}
else {
    Assert-ErasedBytes -Bytes $postFlash -Offset $PersistenceOffset -Length $PersistenceBytes -Label 'clean persistence'
}

Add-Log -Text ('POST_RECOVERY_FLASH_SHA256=' + (Get-FileHash -LiteralPath $PostFlashPath -Algorithm SHA256).Hash.ToUpperInvariant())
Add-Log -Text 'BYTE_EXACT_READBACK=PASS'

$finalResetOutput = Invoke-Cube -Label 'FINAL_SOFTWARE_RESET' -Arguments @('-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz),'-rst')
Assert-CubeConnectMode -Text $finalResetOutput -Expected 'Hot Plug'
Add-Log -Text 'RECOVERY_FLASH_PHASE=PASS'
Add-Log -Text 'NOTE=Runtime USB/UART/OLED proof is owned by the enclosing Gate-5 acceptance harness.'
Save-Log

Write-Host ('RECOVERY_EVIDENCE_DIR=' + $EvidenceDir)
Write-Host ('RECOVERY_LOG_SHA256=' + (Get-FileHash -LiteralPath $LogPath -Algorithm SHA256).Hash.ToUpperInvariant())
Write-Host 'RECOVERY_OUTCOME=PASS'
