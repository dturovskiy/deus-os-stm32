# Deus OS Wi-Fi candidate: native Windows OFFLINE ARM build only.
# No USB, serial, RF, reset, signing, staging, package creation or Flash.
[CmdletBinding()]
param(
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot),
    [string]$ToolchainRoot = '',
    [switch]$PreflightOnly
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT) {
    throw 'WIFI_ARM_WINDOWS_ONLY: run native Windows pwsh, NOT WSL/Mac.'
}
if ($PSVersionTable.PSVersion.Major -lt 7) { throw 'WIFI_ARM_POWERSHELL_7_REQUIRED' }
$ProjectRoot = [IO.Path]::GetFullPath($ProjectRoot)
$scriptDir = [IO.Path]::GetFullPath((Join-Path $ProjectRoot 'scripts'))
if (-not $scriptDir.Equals([IO.Path]::GetFullPath($PSScriptRoot),
    [StringComparison]::OrdinalIgnoreCase)) {
    throw 'WIFI_ARM_SCRIPT_NOT_IN_OS_SCRIPTS'
}
function Assert-PlainFile([string]$p) {
    if (-not (Test-Path -LiteralPath $p -PathType Leaf)) {
        throw ('WIFI_ARM_FILE_MISSING ' + $p)
    }
    if (((Get-Item -LiteralPath $p -Force).Attributes -band
        [IO.FileAttributes]::ReparsePoint) -ne 0) {
        throw ('WIFI_ARM_REPARSE_REFUSED ' + $p)
    }
}
$manifest = Join-Path $PSScriptRoot 'wifi_arm_candidate_build_inputs_20261110.sha256'
Assert-PlainFile $manifest
$locked = [Collections.Generic.Dictionary[string,string]]::new(
    [StringComparer]::Ordinal)
foreach ($line in [IO.File]::ReadAllLines($manifest)) {
    if ($line -cnotmatch '^([0-9a-f]{64})  ((src|include|nodes|linker|scripts)/[A-Za-z0-9_./-]+)$') {
        throw ('WIFI_ARM_BAD_MANIFEST_LINE ' + $line)
    }
    $sha = $Matches[1]; $relative = $Matches[2]
    if ($relative.Contains('..') -or $relative.Contains('//') -or
        -not $locked.TryAdd($relative,$sha)) {
        throw ('WIFI_ARM_UNSAFE_INPUT ' + $relative)
    }
    $file = Join-Path $ProjectRoot ($relative.Replace('/','\'))
    Assert-PlainFile $file
    $got = (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($got -cne $sha) {
        throw ('WIFI_ARM_SOURCE_DRIFT ' + $relative + ' GOT_SHA=' + $got)
    }
}
# Check the complete build-input set; added C/headers cannot bypass manifest.
$actual = [Collections.Generic.List[string]]::new()
foreach ($spec in @(
    @{ Dir='src'; Filter='*.c' },
    @{ Dir='src'; Filter='*.s' },
    @{ Dir='include'; Filter='*.h' }
)) {
    $found = @(Get-ChildItem -LiteralPath (Join-Path $ProjectRoot $spec.Dir) -File -Recurse -Filter $spec.Filter)
    foreach ($f in $found) {
        $actual.Add([IO.Path]::GetRelativePath($ProjectRoot,$f.FullName).Replace('\','/'))
    }
}
foreach ($name in @(
    'nodes/esp8266/firmware/wifi_lab/wifi_node_extension_v0.c',
    'nodes/esp8266/firmware/wifi_lab/wifi_node_extension_v0.h',
    'nodes/esp8266/firmware/wifi_lab/wifi_scan_v0.h',
    'linker/stm32f103c8.ld',
    'scripts/build_firmware.ps1',
    'scripts/output_directory_safety.ps1',
    'scripts/build_wifi_arm_offline_candidate.ps1'
)) { $actual.Add($name) }
if ($actual.Count -ne $locked.Count) {
    throw ('WIFI_ARM_INPUT_SET_DRIFT ACTUAL=' + $actual.Count +
           ' PINNED=' + $locked.Count)
}
foreach ($name in $actual) {
    if (-not $locked.ContainsKey($name)) {
        throw ('WIFI_ARM_UNPINNED_FILE ' + $name)
    }
}
$manifestSha = (Get-FileHash -LiteralPath $manifest -Algorithm SHA256).Hash.ToLowerInvariant()
# Deliberate candidate-only identifier: SHA256 manifest first 160 bits.
# NOT a Git tree OID and NEVER an authorization to sign/install firmware.
$sourceId = $manifestSha.Substring(0,40)
Write-Host ('WIFI_ARM_SOURCE_LOCK=PASS FILES=' + $locked.Count)
Write-Host ('WIFI_ARM_MANIFEST_SHA256=' + $manifestSha)
Write-Host ('WIFI_ARM_CANDIDATE_ID_SHA256_PREFIX160=' + $sourceId)
if ([string]::IsNullOrWhiteSpace($ToolchainRoot)) {
    $parent = Split-Path -Parent $ProjectRoot
    $ToolchainRoot = Join-Path $parent 'Tools/arm-gnu-toolchain-15.3.rel1-mingw-w64-x86_64-arm-none-eabi'
}
$ToolchainRoot = [IO.Path]::GetFullPath($ToolchainRoot)
$binDir = Join-Path $ToolchainRoot 'bin'
foreach ($name in @('arm-none-eabi-gcc.exe','arm-none-eabi-size.exe',
                    'arm-none-eabi-objcopy.exe','arm-none-eabi-nm.exe')) {
    Assert-PlainFile (Join-Path $binDir $name)
}
$gcc = Join-Path $binDir 'arm-none-eabi-gcc.exe'
$version = [string](& $gcc -dumpfullversion)
if ($LASTEXITCODE -ne 0 -or $version.Trim() -cne '15.3.1') {
    throw ('WIFI_ARM_UNEXPECTED_GCC_VERSION ' + $version)
}
Write-Host ('WIFI_ARM_GCC_VERSION=' + $version)
Write-Host ('WIFI_ARM_GCC_SHA256=' +
    (Get-FileHash -LiteralPath $gcc -Algorithm SHA256).Hash)
Write-Host 'WIFI_ARM_PHYSICAL_DEVICE_ACTIONS=NONE'
if ($PreflightOnly) { Write-Host 'WIFI_ARM_PREFLIGHT=PASS'; return }

# Distinct build/ subdirectory. Do not overwrite accepted v9 artifacts.
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss') + '_' + [Guid]::NewGuid().ToString('N')
$runRoot = Join-Path $ProjectRoot ('build/wifi_arm_offline_' + $runId)
if (Test-Path -LiteralPath $runRoot) { throw 'WIFI_ARM_OUTPUT_EXISTS' }
New-Item -ItemType Directory -Path $runRoot | Out-Null
$fwOut = Join-Path $runRoot 'firmware'
$log = Join-Path $runRoot 'arm_build.log'
$build = Join-Path $PSScriptRoot 'build_firmware.ps1'
try {
    & $build -ProjectRoot $ProjectRoot -OutputDir $fwOut -ToolchainRoot $ToolchainRoot -FirmwareSourceTree $sourceId *>&1 |
        Out-File -LiteralPath $log -Encoding utf8
    if (-not $?) { throw 'WIFI_ARM_BUILD_RETURNED_FALSE' }
}
catch {
    Write-Host ('WIFI_ARM_BUILD=FAIL ORIGINAL_LOG=' + $log)
    Write-Host 'WIFI_ARM_NO_SIGNING_NO_FLASH=YES'
    throw
}
$logText = [IO.File]::ReadAllText($log)
function Metric([string]$key) {
    $match = [regex]::Match($logText,('(?m)^' + $key + '=(\d+)\s*$'))
    if (-not $match.Success) { throw ('WIFI_ARM_BUILD_METRIC_MISSING ' + $key) }
    return [long]::Parse($match.Groups[1].Value)
}
if (-not $logText.Contains('BUILD_SOURCE_TREE=' + $sourceId)) {
    throw 'WIFI_ARM_BUILD_SOURCE_ID_MISMATCH'
}
$flash = Metric 'BUILD_FLASH'
$sram = Metric 'BUILD_SRAM'
$flashLimit = Metric 'BUILD_FLASH_CEILING'
$sramLimit = Metric 'BUILD_SRAM_CEILING'
if ($flashLimit -ne 53248 -or $sramLimit -ne 11264 -or
    $flash -gt $flashLimit -or $sram -gt $sramLimit) {
    throw ('WIFI_ARM_LIMIT_FAIL FLASH=' + $flash + ' SRAM=' + $sram)
}
$elf = Join-Path $fwOut 'os.elf'
$map = Join-Path $fwOut 'os.map'
$bin = Join-Path $fwOut 'os.bin'
foreach ($f in @($elf,$map,$bin)) { Assert-PlainFile $f }
$su = @(Get-ChildItem -LiteralPath $fwOut -File -Filter '*.su')
if ($su.Count -ne 38) {
    throw ('WIFI_ARM_STACK_USAGE_FILES_WRONG GOT=' + $su.Count)
}
foreach ($name in @('src_kernel.su','src_kernel_node_transport.su',
                    'src_kernel_node_wifi_client_v0.su',
                    'src_drivers_wifi_node_extension_v0.su')) {
    Assert-PlainFile (Join-Path $fwOut $name)
}
$stack = [Collections.Generic.List[object]]::new()
foreach ($file in $su) {
    foreach ($line in [IO.File]::ReadAllLines($file.FullName)) {
        if ($line -match '^(.*)\t(\d+)\t([A-Za-z,]+)$') {
            $stack.Add([pscustomobject]@{
                entry=$Matches[1]; stack_bytes=[int]$Matches[2];
                class=$Matches[3]; su_file=$file.Name
            })
        }
    }
}
if ($stack.Count -eq 0) { throw 'WIFI_ARM_NO_STATIC_STACK_ENTRIES' }
# Static GCC .su != task0 worst-case call-chain/PSP watermark.
$topStack = @($stack | Sort-Object stack_bytes -Descending | Select-Object -First 15)
$report = [ordered]@{
    schema='DEUS_WIFI_ARM_OFFLINE_V0'
    status='ARM_LINKED_WITHIN_SECTION_CEILINGS_NOT_AUTHORIZED_TO_FLASH'
    manifest_sha256=$manifestSha
    source_id_sha256_prefix160_not_git_oid=$sourceId
    source_file_count=$locked.Count
    gcc_version=$version
    gcc_sha256=(Get-FileHash -LiteralPath $gcc -Algorithm SHA256).Hash
    flash_bytes=$flash
    flash_limit_bytes=$flashLimit
    sram_bytes=$sram
    sram_limit_bytes=$sramLimit
    bin_bytes=(Get-Item -LiteralPath $bin).Length
    bin_sha256=(Get-FileHash -LiteralPath $bin -Algorithm SHA256).Hash
    elf_sha256=(Get-FileHash -LiteralPath $elf -Algorithm SHA256).Hash
    map_sha256=(Get-FileHash -LiteralPath $map -Algorithm SHA256).Hash
    su_file_count=$su.Count
    top_per_function_static_stack=$topStack
    runtime_psp_watermark='NOT_ATTESTED'
    signed_package='NOT_CREATED'
    device_io='NONE'
    installation='NOT_AUTHORIZED'
}
$reportPath = Join-Path $runRoot 'wifi_arm_offline_result.json'
[IO.File]::WriteAllText($reportPath,($report | ConvertTo-Json -Depth 6),
    [Text.UTF8Encoding]::new($false))
Write-Host ('WIFI_ARM_OFFLINE_BUILD=PASS FLASH=' + $flash + '/53248 SRAM=' + $sram + '/11264')
Write-Host ('WIFI_ARM_BIN_SHA256=' + $report.bin_sha256)
Write-Host ('WIFI_ARM_RESULT_JSON=' + $reportPath)
Write-Host ('WIFI_ARM_BUILD_LOG=' + $log)
Write-Host 'WIFI_ARM_PSP_RUNTIME_PENDING=YES NO_SIGNING_NO_FLASH=YES'
