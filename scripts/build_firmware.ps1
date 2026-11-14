param(
    [Parameter(Mandatory=$true)][string]$ProjectRoot,
    [Parameter(Mandatory=$true)][string]$OutputDir,
    [Parameter(Mandatory=$true)][string]$FirmwareSourceTree,
    [string]$ToolchainRoot = $env:DEUS_ARM_TOOLCHAIN_ROOT,
    [switch]$EspStm32BoardQualified,
    [switch]$EspSplitAdapterBoot
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = "Stop"

$FlashAcceptanceCeiling = 53248
$SramAcceptanceCeiling = 11264

if ($FirmwareSourceTree -notmatch '^[0-9a-f]{40}$') {
    throw "FirmwareSourceTree must be 40 lowercase hex characters"
}

if ([string]::IsNullOrWhiteSpace($ToolchainRoot)) {
    $ToolchainCandidates = @(Get-Command -Name 'arm-none-eabi-gcc.exe' -CommandType Application -All -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source -Unique)
    if ($ToolchainCandidates.Count -ne 1) {
        throw ('ARM GNU toolchain not uniquely configured. Set -ToolchainRoot or DEUS_ARM_TOOLCHAIN_ROOT, or put exactly one arm-none-eabi-gcc.exe on PATH. Candidates=' + $ToolchainCandidates.Count)
    }
    $ToolchainRoot = Split-Path -Parent (Split-Path -Parent $ToolchainCandidates[0])
}

$Gcc = Join-Path $ToolchainRoot "bin\arm-none-eabi-gcc.exe"
$Objcopy = Join-Path $ToolchainRoot "bin\arm-none-eabi-objcopy.exe"
$Size = Join-Path $ToolchainRoot "bin\arm-none-eabi-size.exe"
$Nm = Join-Path $ToolchainRoot "bin\arm-none-eabi-nm.exe"

foreach ($Tool in @($Gcc,$Objcopy,$Size,$Nm)) {
    if (-not (Test-Path -LiteralPath $Tool -PathType Leaf)) {
        throw ("required tool missing: {0}" -f [System.IO.Path]::GetFileName($Tool))
    }
}

$SourcePaths = @(
    "src/drivers/flash_persistence.c"
    "src/drivers/i2c1.c"
    "src/drivers/iwdg.c"
    "src/drivers/ssd1306.c"
    "src/drivers/status_led.c"
    "src/drivers/stm32f103_clock.c"
    "src/drivers/usart1.c"
    "src/drivers/usart2_node.c"
    "src/drivers/esp_gpio_board_v0.c"
    "src/drivers/deus_node_link.c"
    "src/drivers/wifi_node_extension_v0.c"
    "src/drivers/node_link_memory.c"
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
    "src/kernel/esp_bridge_core_v0.c"
    "src/kernel/esp_boot_control_v0.c"
    "src/kernel/esp_stm32_owner_v1.c"
    "src/kernel/oled_console.c"
    "src/kernel/oled_status_bar.c"
    "src/kernel/oled_ui_layout.c"
    "src/kernel/oled_ui_layout_config_v1.c"
    "src/kernel/scheduler_diagnostics.c"
    "src/kernel/scheduler.c"
    "src/kernel/node_transport.c"
    "src/kernel/node_wifi_client_v0.c"
    "src/kernel/system_identity.c"
    "src/kernel/system_service_state.c"
    "src/kernel/usb_management.c"
)

$ObjectNames = @(
    "src_drivers_flash_persistence.o"
    "src_drivers_i2c1.o"
    "src_drivers_iwdg.o"
    "src_drivers_ssd1306.o"
    "src_drivers_status_led.o"
    "src_drivers_stm32f103_clock.o"
    "src_drivers_usart1.o"
    "src_drivers_usart2_node.o"
    "src_drivers_esp_gpio_board_v0.o"
    "src_drivers_deus_node_link.o"
    "src_drivers_wifi_node_extension_v0.o"
    "src_drivers_node_link_memory.o"
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
    "src_kernel_esp_bridge_core_v0.o"
    "src_kernel_esp_boot_control_v0.o"
    "src_kernel_esp_stm32_owner_v1.o"
    "src_kernel_oled_console.o"
    "src_kernel_oled_status_bar.o"
    "src_kernel_oled_ui_layout.o"
    "src_kernel_oled_ui_layout_config_v1.o"
    "src_kernel_scheduler_diagnostics.o"
    "src_kernel_scheduler.o"
    "src_kernel_node_transport.o"
    "src_kernel_node_wifi_client_v0.o"
    "src_kernel_system_identity.o"
    "src_kernel_system_service_state.o"
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

. (Join-Path $PSScriptRoot 'output_directory_safety.ps1')
$OutputDir = Reset-DeusGeneratedOutputDirectory -ProjectRoot $ProjectRoot -OutputDir $OutputDir -OwnerId 'DEUS_OS_FIRMWARE_BUILD_V1'

$Header = Join-Path $OutputDir "deus_build_identity.h"
if ($EspStm32BoardQualified -and $EspSplitAdapterBoot) {
    throw "ESP GPIO BOOT/EN ownership conflict: select exactly one board profile"
}
$BoardOwnerValue = if ($EspStm32BoardQualified) { "1" } else { "0" }
$SplitAdapterValue = if ($EspSplitAdapterBoot) { "1" } else { "0" }
$HeaderText = "#ifndef DEUS_BUILD_IDENTITY_H`r`n#define DEUS_BUILD_IDENTITY_H`r`n#define DEUS_FIRMWARE_SOURCE_TREE_HEX `"$FirmwareSourceTree`"`r`n#define DEUS_ESP_STM32_BOARD_QUALIFIED $BoardOwnerValue`r`n#define DEUS_ESP_SPLIT_BOOT_ADAPTER $SplitAdapterValue`r`n#endif`r`n"
Write-Host ("ESP_STM32_BOARD_OWNER_PROFILE={0}" -f $(if ($EspStm32BoardQualified) { "QUALIFIED_WIRING_REQUIRES_PHYSICAL_PROOF" } else { "CURRENT_HARD_DENY_DEFAULT" }))
Write-Host ("ESP_SPLIT_BOOT_ADAPTER_PROFILE={0}" -f $(if ($EspSplitAdapterBoot) { "EXISTING_CH340_CONTROL_ONLY_USART2_DATA" } else { "OFF" }))
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
    "-I",(Join-Path $ProjectRoot "include"),
    "-I",(Join-Path $ProjectRoot "nodes/esp8266/firmware/wifi_lab")
)

$Objects = [System.Collections.Generic.List[string]]::new()

$SizeOptimizedSources = @(
    "src/drivers/flash_persistence.c"
    "src/drivers/i2c1.c"
    "src/drivers/ssd1306.c"
    "src/drivers/status_led.c"
    "src/drivers/stm32f103_clock.c"
    "src/drivers/usart1.c"
    "src/drivers/usart2_node.c"
    "src/drivers/esp_gpio_board_v0.c"
    "src/drivers/deus_node_link.c"
    "src/drivers/wifi_node_extension_v0.c"
    "src/drivers/node_link_memory.c"
    "src/kernel/node_transport.c"
    "src/kernel/node_wifi_client_v0.c"
    "src/kernel.c"
    "src/kernel/application_commands.c"
    "src/kernel/application_runtime_bridge.c"
    "src/kernel/application_runtime.c"
    "src/kernel/asset_persistence.c"
    "src/kernel/asset_transfer.c"
    "src/kernel/binary_frame.c"
    "src/kernel/binary_rpc.c"
    "src/kernel/command_service.c"
    "src/kernel/oled_ui_layout_config_v1.c"
    "src/kernel/scheduler.c"
    "src/kernel/scheduler_diagnostics.c"
    "src/gfx/font3x5.c"
    "src/gfx/mono_fb.c"
    "src/gfx/text_renderer.c"
    "src/kernel/oled_console.c"
    "src/kernel/oled_status_bar.c"
    "src/kernel/esp_bridge_core_v0.c"
    "src/kernel/esp_boot_control_v0.c"
    "src/kernel/esp_stm32_owner_v1.c"
    "src/kernel/system_service_state.c"
    "src/kernel/usb_management.c"
)

# Only ESP ROM bridge/owner modules use cross-module LTO in an electrically
# qualified board build. Keep USB IRQ, scheduler, watchdog and all other
# stable firmware units on their previously accepted native code-generation.
$EspBridgePartialLtoSources = @(
    "src/drivers/esp_gpio_board_v0.c"
    "src/kernel/esp_boot_control_v0.c"
    "src/kernel/esp_stm32_owner_v1.c"
    "src/kernel/usb_management.c"
)

# Preserve the optimized normal9 firmware baseline. For the CURRENT CH340
# BOOT/EN + STM32 USART2 ESP topology, LTO touches ONLY these application,
# Wi-Fi and diagnostic units. Never LTO the USB IRQ, scheduler context switch,
# watchdog or flash writer in this profile. Native ARM A/B: 52,164 -> 50,328B.
$EspSplitWifiPartialLtoSources = @(
    "src/drivers/deus_node_link.c"
    "src/drivers/wifi_node_extension_v0.c"
    "src/kernel.c"
    "src/kernel/application_commands.c"
    "src/kernel/application_runtime.c"
    "src/kernel/asset_transfer.c"
    "src/kernel/binary_rpc.c"
    "src/kernel/command_service.c"
    "src/kernel/esp_bridge_core_v0.c"
    "src/kernel/node_transport.c"
    "src/kernel/node_wifi_client_v0.c"
    "src/kernel/oled_console.c"
    "src/kernel/oled_status_bar.c"
    "src/kernel/scheduler_diagnostics.c"
    "src/kernel/usb_management.c"
)
if (@($EspSplitWifiPartialLtoSources | Where-Object { $SourcePaths -notcontains $_ }).Count -ne 0) {
    throw "ESP WiFi LTO source list drift"
}
for ($i = 0; $i -lt $SourcePaths.Count; ++$i) {
    $Source = Join-Path $ProjectRoot $SourcePaths[$i].Replace("/","\")
    $Object = Join-Path $OutputDir $ObjectNames[$i]
    $Optimization = if ($SizeOptimizedSources -contains $SourcePaths[$i]) {
        "-Os"
    } else {
        "-O2"
    }

    $CompileFlags = @($Optimization)
    if (($EspStm32BoardQualified -and
         ($EspBridgePartialLtoSources -contains $SourcePaths[$i])) -or
        ($EspSplitAdapterBoot -and
         ($EspSplitWifiPartialLtoSources -contains $SourcePaths[$i]))) {
        $CompileFlags += "-flto"
    }
    Invoke-Tool -Label ("compile-" + $ObjectNames[$i]) -Exe $Gcc -Arguments (
        $CommonC + $CompileFlags + @("-c",$Source,"-o",$Object)
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
if ($EspStm32BoardQualified -or $EspSplitAdapterBoot) {
    $LinkArgs.Add("-flto")
    Write-Host ("ESP_ARM_PARTIAL_LTO={0}" -f $(if ($EspStm32BoardQualified) { "QUALIFIED_BOARD_FOUR_MODULES" } else { "CURRENT_SPLIT_WIFI_FIFTEEN_NON_IRQ_MODULES" }))
}
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
$FlashSectionBytes = $TextBytes + $DataBytes
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

# Count the actual binary, not just sections. The original IRQ38 image had
# a 4-byte gap between the 220-byte vector and .text, but a size-optimized
# object order can legitimately place .text immediately after the vector.
# Verify the exact 0/4-byte gap from the native linker MAP, without accepting
# arbitrary extra bytes or modifying the bootloader/metadata Flash boundary.
$FlashUsage = [int64]$BinInfo.Length
$FlashLoadPadding = $FlashUsage - $FlashSectionBytes
$MapText = [System.IO.File]::ReadAllText($Map)
$MapVector = [regex]::Matches($MapText, '(?m)^\.isr_vector[ \t]+0x([0-9A-Fa-f]+)[ \t]+0x([0-9A-Fa-f]+)')
$MapCode = [regex]::Matches($MapText, '(?m)^\.text[ \t]+0x([0-9A-Fa-f]+)[ \t]+0x([0-9A-Fa-f]+)')
$MapData = [regex]::Matches($MapText, '(?m)^\.data[ \t]+0x([0-9A-Fa-f]+)[ \t]+0x([0-9A-Fa-f]+)[ \t]+load address 0x([0-9A-Fa-f]+)')
if (($MapVector.Count -ne 1) -or ($MapCode.Count -ne 1) -or ($MapData.Count -ne 1)) {
    throw "native MAP missing unique Flash vector/text/data output section"
}
$VectorOrigin = [Convert]::ToInt64($MapVector[0].Groups[1].Value,16)
$VectorBytes = [Convert]::ToInt64($MapVector[0].Groups[2].Value,16)
$TextOrigin = [Convert]::ToInt64($MapCode[0].Groups[1].Value,16)
$TextBytesInMap = [Convert]::ToInt64($MapCode[0].Groups[2].Value,16)
$DataBytesInMap = [Convert]::ToInt64($MapData[0].Groups[2].Value,16)
$DataLoadOrigin = [Convert]::ToInt64($MapData[0].Groups[3].Value,16)
$MapFlashGap = $TextOrigin - ($VectorOrigin + $VectorBytes)
$MapBinBytes = ($DataLoadOrigin + $DataBytesInMap) - $VectorOrigin
if (($VectorOrigin -ne 0x08002000) -or ($VectorBytes -ne 220) -or
    (($TextOrigin % 4) -ne 0) -or
    (($MapFlashGap -ne 0) -and ($MapFlashGap -ne 4)) -or
    ($MapFlashGap -ne $FlashLoadPadding) -or
    (($VectorBytes + $TextBytesInMap) -ne $TextBytes) -or
    ($DataBytesInMap -ne $DataBytes) -or
    ($DataLoadOrigin -ne ($TextOrigin + $TextBytesInMap)) -or
    ($MapBinBytes -ne $FlashUsage)) {
    throw ("native Flash load geometry mismatch: padding={0}, MAP_gap={1}, MAP_bin={2}, binary={3}" -f
        $FlashLoadPadding,$MapFlashGap,$MapBinBytes,$FlashUsage)
}
if ($FlashUsage -gt $FlashAcceptanceCeiling) {
    throw ("Flash usage {0} exceeds Bootloader application ceiling {1}" -f $FlashUsage,$FlashAcceptanceCeiling)
}
if ($SramUsage -gt $SramAcceptanceCeiling) {
    throw ("SRAM usage {0} exceeds Bootloader application ceiling {1}" -f $SramUsage,$SramAcceptanceCeiling)
}

$BinSha = (Get-FileHash -LiteralPath $Bin -Algorithm SHA256).Hash.ToUpperInvariant()
$ElfSha = (Get-FileHash -LiteralPath $Elf -Algorithm SHA256).Hash.ToUpperInvariant()
$MapSha = (Get-FileHash -LiteralPath $Map -Algorithm SHA256).Hash.ToUpperInvariant()

Write-Host ("BUILD_SOURCE_TREE={0}" -f $FirmwareSourceTree)
Write-Host ("BUILD_HEADER_SHA256={0}" -f $HeaderSha)
Write-Host ("BUILD_TEXT={0}" -f $TextBytes)
Write-Host ("BUILD_DATA={0}" -f $DataBytes)
Write-Host ("BUILD_BSS={0}" -f $BssBytes)
Write-Host ("BUILD_FLASH_SECTION_SUM={0}" -f $FlashSectionBytes)
Write-Host ("BUILD_FLASH_LOAD_PADDING={0}" -f $FlashLoadPadding)
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
