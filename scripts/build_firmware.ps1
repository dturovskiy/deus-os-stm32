param(
    [Parameter(Mandatory=$true)][string]$ProjectRoot,
    [Parameter(Mandatory=$true)][string]$OutputDir,
    [string]$ToolchainRoot = "D:\Projects\STM32\Tools\arm-gnu-toolchain-15.3.rel1-mingw-w64-x86_64-arm-none-eabi",
    [string]$FirmwareSourceTree = "b895955f7738aceb6fca0272d510cc433378c6ab"
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = "Stop"

$ExpectedTree = "b895955f7738aceb6fca0272d510cc433378c6ab"
$ExpectedHeaderSha = "F6EAA98172FEE67338BFD978F208BD95731063A1160B0C9C1282306A5D6658A5"

if ($FirmwareSourceTree -ne $ExpectedTree) {
    throw ("FirmwareSourceTree must equal accepted candidate tree {0}" -f $ExpectedTree)
}
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
    "src/kernel/binary_frame.c"
    "src/kernel/binary_rpc.c"
    "src/kernel/command_service.c"
    "src/kernel/oled_console.c"
    "src/kernel/oled_status_bar.c"
    "src/kernel/oled_ui_layout.c"
    "src/kernel/scheduler_diagnostics.c"
    "src/kernel/scheduler.c"
    "src/kernel/system_identity.c"
    "src/kernel/usb_management.c"
)
$ObjectNames = @(
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
    "src_kernel_binary_frame.o"
    "src_kernel_binary_rpc.o"
    "src_kernel_command_service.o"
    "src_kernel_oled_console.o"
    "src_kernel_oled_status_bar.o"
    "src_kernel_oled_ui_layout.o"
    "src_kernel_scheduler_diagnostics.o"
    "src_kernel_scheduler.o"
    "src_kernel_system_identity.o"
    "src_kernel_usb_management.o"
)

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
    throw "current firmware C source set differs from recovered historical source list"
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
if ($HeaderSha -ne $ExpectedHeaderSha) {
    throw ("generated build identity header mismatch: {0}" -f $HeaderSha)
}

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
    "-O2",
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
for ($i = 0; $i -lt $SourcePaths.Count; ++$i) {
    $Source = Join-Path $ProjectRoot $SourcePaths[$i].Replace("/","\")
    $Object = Join-Path $OutputDir $ObjectNames[$i]
    Invoke-Tool -Label ("compile-" + $ObjectNames[$i]) -Exe $Gcc -Arguments (
        $CommonC + @("-c",$Source,"-o",$Object)
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
foreach ($Object in $Objects) { $LinkArgs.Add($Object) }
$LinkArgs.Add("-lgcc")
Invoke-Tool -Label "link" -Exe $Gcc -Arguments @($LinkArgs)

Invoke-Tool -Label "objcopy-bin" -Exe $Objcopy -Arguments @("-O","binary",$Elf,$Bin)
Invoke-Tool -Label "size" -Exe $Size -Arguments @($Elf)
Invoke-Tool -Label "nm-undefined" -Exe $Nm -Arguments @("-u",$Elf)

$Undefined = @(& $Nm -u $Elf)
if ($LASTEXITCODE -ne 0) { throw "nm -u failed" }
if ($Undefined.Count -ne 0) {
    throw ("undefined symbols remain: {0}" -f ($Undefined -join "; "))
}

$BinInfo = Get-Item -LiteralPath $Bin
$ElfInfo = Get-Item -LiteralPath $Elf
$MapInfo = Get-Item -LiteralPath $Map
$BinSha = (Get-FileHash -LiteralPath $Bin -Algorithm SHA256).Hash.ToUpperInvariant()
$ElfSha = (Get-FileHash -LiteralPath $Elf -Algorithm SHA256).Hash.ToUpperInvariant()
$MapSha = (Get-FileHash -LiteralPath $Map -Algorithm SHA256).Hash.ToUpperInvariant()

Write-Host ("BUILD_HEADER_SHA256={0}" -f $HeaderSha)
Write-Host ("BUILD_BIN_SIZE={0}" -f $BinInfo.Length)
Write-Host ("BUILD_BIN_SHA256={0}" -f $BinSha)
Write-Host ("BUILD_ELF_SIZE={0}" -f $ElfInfo.Length)
Write-Host ("BUILD_ELF_SHA256={0}" -f $ElfSha)
Write-Host ("BUILD_MAP_SIZE={0}" -f $MapInfo.Length)
Write-Host ("BUILD_MAP_SHA256={0}" -f $MapSha)
Write-Host "BUILD_OUTCOME=PASS"
