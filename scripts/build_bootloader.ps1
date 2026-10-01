param(
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot),
    [string]$OutputDir = (Join-Path (Split-Path -Parent $PSScriptRoot) 'build\bootloader'),
    [Parameter(Mandatory=$true)][string]$UpdateKeyFile,
    [string]$ToolchainRoot = 'D:\Projects\STM32\Tools\arm-gnu-toolchain-15.3.rel1-mingw-w64-x86_64-arm-none-eabi'
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'

$FlashCeiling = 8192
$DataBssCeiling = 1024
$MspBytes = 1024
$ConventionalSramCeiling = 2048

$Gcc = Join-Path $ToolchainRoot 'bin\arm-none-eabi-gcc.exe'
$Objcopy = Join-Path $ToolchainRoot 'bin\arm-none-eabi-objcopy.exe'
$Size = Join-Path $ToolchainRoot 'bin\arm-none-eabi-size.exe'
$Nm = Join-Path $ToolchainRoot 'bin\arm-none-eabi-nm.exe'

foreach ($tool in @($Gcc,$Objcopy,$Size,$Nm)) {
    if (-not (Test-Path -LiteralPath $tool -PathType Leaf)) {
        throw ('Required tool missing: ' + $tool)
    }
}

if (-not (Test-Path -LiteralPath $UpdateKeyFile -PathType Leaf)) {
    throw 'UpdateKeyFile does not exist'
}
$keyBytes = [System.IO.File]::ReadAllBytes($UpdateKeyFile)
if ($keyBytes.Length -ne 32) {
    throw ('UpdateKeyFile must contain exactly 32 bytes; actual=' + $keyBytes.Length)
}

New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null

$BootSource = Join-Path $ProjectRoot 'bootloader\bootloader.c'
$Startup = Join-Path $ProjectRoot 'bootloader\startup.s'
$Linker = Join-Path $ProjectRoot 'linker\stm32f103c8_bootloader.ld'
foreach ($path in @($BootSource,$Startup,$Linker)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw ('Bootloader source missing: ' + $path)
    }
}

$BootObject = Join-Path $OutputDir 'bootloader.o'
$StartupObject = Join-Path $OutputDir 'startup.o'
$Elf = Join-Path $OutputDir 'bootloader.elf'
$Bin = Join-Path $OutputDir 'bootloader.bin'
$Map = Join-Path $OutputDir 'bootloader.map'
$SecretTemp = Join-Path $env:TEMP ('deus_boot_key_' + [guid]::NewGuid().ToString('N'))
$KeySource = Join-Path $SecretTemp 'update_key.c'
$KeyObject = Join-Path $SecretTemp 'update_key.o'

function Invoke-NativeChecked {
    param(
        [string]$Executable,
        [string[]]$Arguments
    )

    $processInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $processInfo.FileName = $Executable
    $processInfo.WorkingDirectory = $ProjectRoot
    $processInfo.UseShellExecute = $false
    $processInfo.RedirectStandardOutput = $true
    $processInfo.RedirectStandardError = $true
    $processInfo.CreateNoWindow = $true
    foreach ($argumentValue in $Arguments) {
        [void]$processInfo.ArgumentList.Add($argumentValue)
    }

    $nativeProcess = [System.Diagnostics.Process]::new()
    $nativeProcess.StartInfo = $processInfo
    if (-not $nativeProcess.Start()) {
        throw ('Failed to start ' + $Executable)
    }

    $stdoutText = $nativeProcess.StandardOutput.ReadToEnd()
    $stderrText = $nativeProcess.StandardError.ReadToEnd()
    $nativeProcess.WaitForExit()

    if ($nativeProcess.ExitCode -ne 0) {
        if (-not [string]::IsNullOrWhiteSpace($stdoutText)) {
            Write-Host $stdoutText
        }
        if (-not [string]::IsNullOrWhiteSpace($stderrText)) {
            Write-Error $stderrText
        }
        throw ('Native command failed: ' + [System.IO.Path]::GetFileName($Executable) + ' exit=' + $nativeProcess.ExitCode)
    }

    return [pscustomobject]@{
        StdOut = $stdoutText
        StdErr = $stderrText
    }
}

try {
    New-Item -ItemType Directory -Path $SecretTemp -Force | Out-Null

    $keyLiteral = [System.Collections.Generic.List[string]]::new()
    foreach ($keyByte in $keyBytes) {
        [void]$keyLiteral.Add(('0x{0:X2}u' -f $keyByte))
    }
    [string[]]$keySourceLines = @(
        '#include <stdint.h>',
        ('const uint8_t deus_update_key[32] = { ' + ($keyLiteral -join ', ') + ' };')
    )
    [System.IO.File]::WriteAllLines(
        $KeySource,
        $keySourceLines,
        [System.Text.UTF8Encoding]::new($false))

    [string[]]$common = @(
        '-mcpu=cortex-m3',
        '-mthumb',
        '-std=c11',
        '-ffreestanding',
        '-fno-builtin',
        '-fno-common',
        '-fdata-sections',
        '-ffunction-sections',
        '-Wall',
        '-Wextra',
        '-Werror',
        '-fstack-usage',
        '-Os'
    )

    $bootArgs = [System.Collections.Generic.List[string]]::new()
    foreach ($argumentValue in $common) { [void]$bootArgs.Add($argumentValue) }
    foreach ($argumentValue in @('-c',$BootSource,'-o',$BootObject)) { [void]$bootArgs.Add($argumentValue) }
    [void](Invoke-NativeChecked $Gcc $bootArgs.ToArray())

    [void](Invoke-NativeChecked $Gcc ([string[]]@(
        '-mcpu=cortex-m3','-mthumb','-x','assembler-with-cpp',
        '-c',$Startup,'-o',$StartupObject
    )))

    $keyArgs = [System.Collections.Generic.List[string]]::new()
    foreach ($argumentValue in $common) { [void]$keyArgs.Add($argumentValue) }
    foreach ($argumentValue in @('-c',$KeySource,'-o',$KeyObject)) { [void]$keyArgs.Add($argumentValue) }
    [void](Invoke-NativeChecked $Gcc $keyArgs.ToArray())

    [void](Invoke-NativeChecked $Gcc ([string[]]@(
        '-mcpu=cortex-m3',
        '-mthumb',
        '-nostdlib',
        '-Wl,--gc-sections',
        '-Wl,--build-id=none',
        ('-Wl,-Map=' + $Map),
        '-T',$Linker,
        '-o',$Elf,
        $StartupObject,
        $BootObject,
        $KeyObject,
        '-lgcc'
    )))

    [void](Invoke-NativeChecked $Objcopy ([string[]]@('-O','binary',$Elf,$Bin)))

    $sizeResult = Invoke-NativeChecked $Size ([string[]]@($Elf))
    $sizeLines = @($sizeResult.StdOut -split '\r?\n')
    $sizeRecord = $null
    foreach ($sizeLine in $sizeLines) {
        if ($sizeLine -match '^\s*(\d+)\s+(\d+)\s+(\d+)\s+\d+\s+[0-9A-Fa-fx]+\s+') {
            $sizeRecord = [pscustomobject]@{
                Text = [int64]$Matches[1]
                Data = [int64]$Matches[2]
                Bss = [int64]$Matches[3]
            }
        }
    }
    if ($null -eq $sizeRecord) {
        throw 'Unable to parse bootloader size output'
    }

    $FlashUsage = $sizeRecord.Text + $sizeRecord.Data
    $BinSize = (Get-Item -LiteralPath $Bin).Length

    # GNU size reports every allocated NOLOAD section in its aggregate BSS
    # column, including the dedicated .boot_stack reservation.  Therefore
    # size.data + size.bss is not the bootloader .data+.bss contract and
    # adding $MspBytes to it would count the stack twice.  SRAM contract
    # authority comes from the linker-owned section-boundary symbols.
    $symbolResult = Invoke-NativeChecked $Nm ([string[]]@(
        '-n',
        '--defined-only',
        $Elf
    ))
    $symbolValues = @{}
    foreach ($symbolLine in @($symbolResult.StdOut -split '\r?\n')) {
        if ($symbolLine -match '^\s*([0-9A-Fa-f]+)\s+\S\s+(\S+)\s*$') {
            $symbolValues[[string]$Matches[2]] =
                [Convert]::ToInt64([string]$Matches[1],16)
        }
    }

    foreach ($requiredSymbol in @(
        '_sdata',
        '_edata',
        '_sbss',
        '_ebss',
        '_sboot_stack',
        '_eboot_stack',
        '_estack'
    )) {
        if (-not $symbolValues.ContainsKey($requiredSymbol)) {
            throw ('Bootloader linker symbol missing: ' + $requiredSymbol)
        }
    }

    $DataBytes = [int64]$symbolValues['_edata'] -
        [int64]$symbolValues['_sdata']
    $BssBytes = [int64]$symbolValues['_ebss'] -
        [int64]$symbolValues['_sbss']
    $LinkedMspBytes = [int64]$symbolValues['_eboot_stack'] -
        [int64]$symbolValues['_sboot_stack']

    if ($DataBytes -lt 0 -or $BssBytes -lt 0 -or $LinkedMspBytes -lt 0) {
        throw 'Bootloader linker section bounds are inverted'
    }
    if ($LinkedMspBytes -ne $MspBytes) {
        throw ('Bootloader MSP reservation mismatch: linked={0} expected={1}' -f $LinkedMspBytes,$MspBytes)
    }
    if ([int64]$symbolValues['_ebss'] -gt [int64]$symbolValues['_sboot_stack']) {
        throw 'Bootloader RAM overlaps MSP reservation'
    }
    if ([int64]$symbolValues['_eboot_stack'] -ne [int64]$symbolValues['_estack']) {
        throw 'Bootloader MSP top does not match _estack'
    }

    $DataBss = $DataBytes + $BssBytes
    $ConventionalSram = $DataBss + $LinkedMspBytes

    if ($FlashUsage -gt $FlashCeiling -or $BinSize -gt $FlashCeiling) {
        throw ('Bootloader Flash ceiling exceeded: flash={0} bin={1} ceiling={2}' -f $FlashUsage,$BinSize,$FlashCeiling)
    }
    if ($DataBss -gt $DataBssCeiling) {
        throw ('Bootloader data+bss ceiling exceeded: {0}/{1}' -f $DataBss,$DataBssCeiling)
    }
    if ($ConventionalSram -gt $ConventionalSramCeiling) {
        throw ('Bootloader conventional SRAM ceiling exceeded: {0}/{1}' -f $ConventionalSram,$ConventionalSramCeiling)
    }

    $undefined = Invoke-NativeChecked $Nm ([string[]]@('-u',$Elf))
    if (-not [string]::IsNullOrWhiteSpace($undefined.StdOut)) {
        throw ('Bootloader undefined symbols: ' + $undefined.StdOut.Trim())
    }

    Write-Host ('BOOT_BUILD_FLASH={0}' -f $FlashUsage)
    Write-Host ('BOOT_BUILD_FLASH_CEILING={0}' -f $FlashCeiling)
    Write-Host ('BOOT_BUILD_DATA_BYTES={0}' -f $DataBytes)
    Write-Host ('BOOT_BUILD_BSS_BYTES={0}' -f $BssBytes)
    Write-Host ('BOOT_BUILD_DATA_BSS={0}' -f $DataBss)
    Write-Host ('BOOT_BUILD_DATA_BSS_CEILING={0}' -f $DataBssCeiling)
    Write-Host ('BOOT_BUILD_MSP={0}' -f $LinkedMspBytes)
    Write-Host ('BOOT_BUILD_CONVENTIONAL_SRAM={0}' -f $ConventionalSram)
    Write-Host ('BOOT_BUILD_CONVENTIONAL_SRAM_CEILING={0}' -f $ConventionalSramCeiling)
    Write-Host ('BOOT_BUILD_BIN_SIZE={0}' -f $BinSize)
    Write-Host ('BOOT_BUILD_BIN_SHA256={0}' -f (Get-FileHash -LiteralPath $Bin -Algorithm SHA256).Hash.ToUpperInvariant())
    Write-Host ('BOOT_BUILD_ELF_SHA256={0}' -f (Get-FileHash -LiteralPath $Elf -Algorithm SHA256).Hash.ToUpperInvariant())
    Write-Host ('BOOT_BUILD_MAP_SHA256={0}' -f (Get-FileHash -LiteralPath $Map -Algorithm SHA256).Hash.ToUpperInvariant())
    Write-Host 'BOOT_BUILD_KEY_BYTES=EXTERNAL_NOT_LOGGED'
    Write-Host 'BOOT_BUILD_OUTCOME=PASS'
}
finally {
    if (Test-Path -LiteralPath $SecretTemp) {
        Remove-Item -LiteralPath $SecretTemp -Recurse -Force -ErrorAction SilentlyContinue
    }
}
