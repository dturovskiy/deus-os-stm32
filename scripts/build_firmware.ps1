param(
    [Parameter(Mandatory=$true)][string]$ProjectRoot,
    [Parameter(Mandatory=$true)][string]$OutputDir,
    [Parameter(Mandatory=$true)][string]$FirmwareSourceTree,
    [string]$ToolchainRoot = "D:\Projects\STM32\Tools\arm-gnu-toolchain-15.3.rel1-mingw-w64-x86_64-arm-none-eabi"
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = "Stop"

$FlashAcceptanceCeiling = 54272
$SramAcceptanceCeiling = 12288

if ($FirmwareSourceTree -notmatch '^[0-9a-f]{40}$') {
    throw "FirmwareSourceTree must be 40 lowercase hex characters"
}

$Gcc = Join-Path $ToolchainRoot "bin\arm-none-eabi-gcc.exe"
$Objcopy = Join-Path $ToolchainRoot "bin\arm-none-eabi-objcopy.exe"
$Size = Join-Path $ToolchainRoot "bin\arm-none-eabi-size.exe"
$Nm = Join-Path $ToolchainRoot "bin\arm-none-eabi-nm.exe"

foreach ($Tool in @($Gcc,$Objcopy,$Size,$Nm)) {
    if (-not (Test-Path -LiteralPath $Tool -PathType Leaf)) {
        throw ("required tool missing: {0}" -f $Tool)
    }
}

$SourcePaths = @(
    "src/drivers/flash_persistence.c"
    "src/drivers/iwdg.c"
    "src/drivers/ssd1306.c"
    "src/drivers/usb_device.c"
    "src/gfx/font3x5.c"
    "src/gfx/font5x6.c"
    "src/gfx/font5x7.c"
    "src/gfx/mono_fb.c"
    "src/gfx/text_renderer.c"
    "src/kernel.c"
    "src/kernel/application_commands.c"
    "src/kernel/application_runtime_bridge.c"
    "src/kernel/application_runtime.c"
    "src/kernel/asset_persistence.c"
    "src/kernel/asset_transfer.c"
    "src/kernel/binary_frame.c"
    "src/kernel/binary_rpc.c"
    "src/kernel/command_service.c"
    "src/kernel/oled_console.c"
    "src/kernel/oled_status_bar.c"
    "src/kernel/oled_ui_layout.c"
    "src/kernel/oled_ui_layout_config_v1.c"
    "src/kernel/scheduler_diagnostics.c"
    "src/kernel/scheduler.c"
    "src/kernel/system_identity.c"
    "src/kernel/usb_management.c"
)

$ObjectNames = @(
    "src_drivers_flash_persistence.o"
    "src_drivers_iwdg.o"
    "src_drivers_ssd1306.o"
    "src_drivers_usb_device.o"
    "src_gfx_font3x5.o"
    "src_gfx_font5x6.o"
    "src_gfx_font5x7.o"
    "src_gfx_mono_fb.o"
    "src_gfx_text_renderer.o"
    "src_kernel.o"
    "src_kernel_application_commands.o"
    "src_kernel_application_runtime_bridge.o"
    "src_kernel_application_runtime.o"
    "src_kernel_asset_persistence.o"
    "src_kernel_asset_transfer.o"
    "src_kernel_binary_frame.o"
    "src_kernel_binary_rpc.o"
    "src_kernel_command_service.o"
    "src_kernel_oled_console.o"
    "src_kernel_oled_status_bar.o"
    "src_kernel_oled_ui_layout.o"
    "src_kernel_oled_ui_layout_config_v1.o"
    "src_kernel_scheduler_diagnostics.o"
    "src_kernel_scheduler.o"
    "src_kernel_system_identity.o"
    "src_kernel_usb_management.o"
)

if ($SourcePaths.Count -ne $ObjectNames.Count) {
    throw "source/object list length mismatch"
}

$DiscoveredSources = @(
    Get-ChildItem -LiteralPath (Join-Path $ProjectRoot "src") -File -Recurse -Filter "*.c" |
    ForEach-Object {
        [System.IO.Path]::GetRelativePath($ProjectRoot,$_.FullName).Replace("\","/")
    } |
    Sort-Object
)
$ExpectedSourcesSorted = @($SourcePaths | Sort-Object)
$SourceDiff = @(Compare-Object -ReferenceObject $ExpectedSourcesSorted -DifferenceObject $DiscoveredSources)
if ($SourceDiff.Count -ne 0 -or $DiscoveredSources.Count -ne $SourcePaths.Count) {
    $SourceDiff | ForEach-Object { Write-Host $_ }
    throw "current firmware C source set differs from versioned build source list"
}

if (Test-Path -LiteralPath $OutputDir) {
    Remove-Item -LiteralPath $OutputDir -Recurse -Force
}
New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null

$Header = Join-Path $OutputDir "deus_build_identity.h"
$HeaderText = "#ifndef DEUS_BUILD_IDENTITY_H`r`n#define DEUS_BUILD_IDENTITY_H`r`n#define DEUS_FIRMWARE_SOURCE_TREE_HEX `"$FirmwareSourceTree`"`r`n#endif`r`n"
[System.IO.File]::WriteAllText(
    $Header,
    $HeaderText,
    [System.Text.UTF8Encoding]::new($false)
)
$HeaderSha = (Get-FileHash -LiteralPath $Header -Algorithm SHA256).Hash.ToUpperInvariant()

function Invoke-Tool {
    param(
        [Parameter(Mandatory=$true)][string]$Label,
        [Parameter(Mandatory=$true)][string]$Exe,
        [Parameter(Mandatory=$true)][AllowEmptyCollection()][string[]]$Arguments
    )
    Write-Host ("CMD {0}: {1} {2}" -f $Label,$Exe,($Arguments -join " "))
    & $Exe @Arguments
    $Rc = $LASTEXITCODE
    Write-Host ("EXIT {0}: {1}" -f $Label,$Rc)
    if ($Rc -ne 0) {
        throw ("{0} failed with exit {1}" -f $Label,$Rc)
    }
}

$CommonC = @(
    "-mcpu=cortex-m3",
    "-mthumb",
    "-std=c11",
    "-g3",
    "-ffreestanding",
    "-fno-builtin",
    "-fno-common",
    "-fdata-sections",
    "-ffunction-sections",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-fstack-usage",
    "-include",$Header,
    "-I",(Join-Path $ProjectRoot "include")
)

$Objects = [System.Collections.Generic.List[string]]::new()

$SizeOptimizedSources = @(
    "src/drivers/flash_persistence.c"
    "src/kernel/asset_persistence.c"
    "src/kernel/asset_transfer.c"
    "src/kernel/oled_ui_layout_config_v1.c"
)

for ($i = 0; $i -lt $SourcePaths.Count; ++$i) {
    $Source = Join-Path $ProjectRoot $SourcePaths[$i].Replace("/","\")
    $Object = Join-Path $OutputDir $ObjectNames[$i]
    $Optimization = if ($SizeOptimizedSources -contains $SourcePaths[$i]) {
        "-Os"
    } else {
        "-O2"
    }

    Invoke-Tool -Label ("compile-" + $ObjectNames[$i]) -Exe $Gcc -Arguments (
        $CommonC + @($Optimization,"-c",$Source,"-o",$Object)
    )

    $Objects.Add($Object)
}

$StartupObject = Join-Path $OutputDir "startup.o"
Invoke-Tool -Label "compile-startup" -Exe $Gcc -Arguments @(
    "-mcpu=cortex-m3",
    "-mthumb",
    "-x","assembler-with-cpp",
    "-g3",
    "-c",(Join-Path $ProjectRoot "src\startup.s"),
    "-o",$StartupObject
)

$Elf = Join-Path $OutputDir "os.elf"
$Map = Join-Path $OutputDir "os.map"
$Bin = Join-Path $OutputDir "os.bin"

$LinkArgs = [System.Collections.Generic.List[string]]::new()
foreach ($Arg in @(
    "-mcpu=cortex-m3",
    "-mthumb",
    "-nostdlib",
    "-Wl,--gc-sections",
    "-Wl,--build-id=none",
    ("-Wl,-Map={0}" -f $Map),
    "-T",(Join-Path $ProjectRoot "linker\stm32f103c8.ld"),
    "-o",$Elf,
    $StartupObject
)) {
    $LinkArgs.Add($Arg)
}

foreach ($Object in $Objects) {
    $LinkArgs.Add($Object)
}

$LinkArgs.Add("-lgcc")
Invoke-Tool -Label "link" -Exe $Gcc -Arguments @($LinkArgs)

Invoke-Tool -Label "objcopy-bin" -Exe $Objcopy -Arguments @("-O","binary",$Elf,$Bin)

$SizeLines = @(& $Size $Elf)
$SizeRc = $LASTEXITCODE
$SizeLines | ForEach-Object { Write-Host $_ }
if ($SizeRc -ne 0) {
    throw ("size failed with exit {0}" -f $SizeRc)
}

$SizeDataLine = $SizeLines | Select-Object -Last 1
if ($SizeDataLine -notmatch '^\s*(\d+)\s+(\d+)\s+(\d+)\s+') {
    throw ("cannot parse arm-none-eabi-size output: {0}" -f $SizeDataLine)
}

$TextBytes = [int64]$Matches[1]
$DataBytes = [int64]$Matches[2]
$BssBytes = [int64]$Matches[3]
$FlashUsage = $TextBytes + $DataBytes
$SramUsage = $DataBytes + $BssBytes

Invoke-Tool -Label "nm-undefined" -Exe $Nm -Arguments @("-u",$Elf)

$Undefined = @(& $Nm -u $Elf)
if ($LASTEXITCODE -ne 0) {
    throw "nm -u failed"
}
if ($Undefined.Count -ne 0) {
    throw ("undefined symbols remain: {0}" -f ($Undefined -join "; "))
}

$BinInfo = Get-Item -LiteralPath $Bin
$ElfInfo = Get-Item -LiteralPath $Elf
$MapInfo = Get-Item -LiteralPath $Map

if ($BinInfo.Length -ne $FlashUsage) {
    throw ("BIN size {0} does not match text+data Flash usage {1}" -f $BinInfo.Length,$FlashUsage)
}
if ($FlashUsage -gt $FlashAcceptanceCeiling) {
    throw ("Flash usage {0} exceeds Asset Gate-2 ceiling {1}" -f $FlashUsage,$FlashAcceptanceCeiling)
}
if ($SramUsage -gt $SramAcceptanceCeiling) {
    throw ("SRAM usage {0} exceeds Asset Gate-2 ceiling {1}" -f $SramUsage,$SramAcceptanceCeiling)
}

$BinSha = (Get-FileHash -LiteralPath $Bin -Algorithm SHA256).Hash.ToUpperInvariant()
$ElfSha = (Get-FileHash -LiteralPath $Elf -Algorithm SHA256).Hash.ToUpperInvariant()
$MapSha = (Get-FileHash -LiteralPath $Map -Algorithm SHA256).Hash.ToUpperInvariant()

Write-Host ("BUILD_SOURCE_TREE={0}" -f $FirmwareSourceTree)
Write-Host ("BUILD_HEADER_SHA256={0}" -f $HeaderSha)
Write-Host ("BUILD_TEXT={0}" -f $TextBytes)
Write-Host ("BUILD_DATA={0}" -f $DataBytes)
Write-Host ("BUILD_BSS={0}" -f $BssBytes)
Write-Host ("BUILD_FLASH={0}" -f $FlashUsage)
Write-Host ("BUILD_FLASH_CEILING={0}" -f $FlashAcceptanceCeiling)
Write-Host ("BUILD_SRAM={0}" -f $SramUsage)
Write-Host ("BUILD_SRAM_CEILING={0}" -f $SramAcceptanceCeiling)
Write-Host ("BUILD_BIN_SIZE={0}" -f $BinInfo.Length)
Write-Host ("BUILD_BIN_SHA256={0}" -f $BinSha)
Write-Host ("BUILD_ELF_SIZE={0}" -f $ElfInfo.Length)
Write-Host ("BUILD_ELF_SHA256={0}" -f $ElfSha)
Write-Host ("BUILD_MAP_SIZE={0}" -f $MapInfo.Length)
Write-Host ("BUILD_MAP_SHA256={0}" -f $MapSha)
Write-Host "BUILD_OUTCOME=PASS"
