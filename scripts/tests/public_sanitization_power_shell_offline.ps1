# Safe Windows PowerShell 7 offline acceptance. NEVER executes firmware tools or probes hardware.
[CmdletBinding()]
param()
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$scripts = Join-Path $root 'scripts'
$seven = @('build_bootloader.ps1','build_firmware.ps1','create_asset_recovery_bundle.ps1','create_bootloader_recovery_bundle.ps1','stm32_asset_recovery.ps1','stm32_bootloader_recovery.ps1','stm32_readonly_flash_preflight.ps1')
function Must([bool]$condition, [string]$label) { if (-not $condition) { throw "OFFLINE_TEST_FAIL=$label" } }
function Parse-Source([string]$source, [string]$label) {
    $tokens = $null
    $errors = $null
    [void][System.Management.Automation.Language.Parser]::ParseInput($source,[ref]$tokens,[ref]$errors)
    Must ($errors.Count -eq 0) "PARSE_$label"
}
function Extract-Resolver([string]$file,[string]$variable,[string]$stop) {
    $source = [IO.File]::ReadAllText((Join-Path $scripts $file))
    $needle = 'if ([string]::IsNullOrWhiteSpace($' + $variable + ')) {'
    $start = $source.IndexOf($needle,[StringComparison]::Ordinal)
    Must ($start -ge 0) "RESOLVER_START_$file"
    $end = $source.IndexOf($stop,$start + $needle.Length,[StringComparison]::Ordinal)
    Must ($end -gt $start) "RESOLVER_STOP_$file"
    $part = $source.Substring($start,$end-$start)
    Parse-Source $part "RESOLVER_$file"
    return $part
}
function Resolve-Test([string]$snippet,[string]$variable,[string]$value) {
    $sb = [scriptblock]::Create($snippet + [Environment]::NewLine + '$' + $variable)
    return & { param($name,$initial,$code)
        Set-Variable -Name $name -Value $initial
        & $code
    } $variable $value $sb
}
function Expect-Ambiguous([string]$part,[string]$variable,[string]$label) {
    $caught = $false
    try { [void](Resolve-Test $part $variable '') }
    catch { if ($_.Exception.Message -notlike '*not uniquely configured*') { throw }; $caught = $true }
    Must $caught $label
}
foreach ($name in $seven) {
    $body=[IO.File]::ReadAllText((Join-Path $scripts $name))
    Parse-Source $body $name
    Must (-not $body.Contains('D:\Projects\STM32\Tools\')) "OLD_DEFAULT_$name"
}
$wrappers=0
foreach ($name in @('create_asset_recovery_bundle.ps1','create_bootloader_recovery_bundle.ps1')) {
    $body=[IO.File]::ReadAllText((Join-Path $scripts $name))
    foreach ($kind in @('PreserveWrapper','CleanWrapper')) {
        $pattern='(?s)\$' + $kind + '\s*=\s*@''\r?\n(?<src>.*?)\r?\n''@'
        $match=[regex]::Match($body,$pattern)
        Must $match.Success "WRAPPER_$name-$kind"
        $part=$match.Groups['src'].Value
        Parse-Source $part "WRAPPER_$name-$kind"
        Must $part.Contains('$env:DEUS_STM32_PROGRAMMER_CLI') "WRAPPER_ENV_$name-$kind"
        Must $part.Contains('-ProgrammerCli $ProgrammerCli') "WRAPPER_FORWARD_$name-$kind"
        $wrappers++
    }
}
Must ($wrappers -eq 4) 'WRAPPER_COUNT'
$boot=[IO.File]::ReadAllText((Join-Path $scripts 'stm32_bootloader_recovery.ps1'))
Must $boot.Contains('VALIDATE_ONLY') 'VALIDATE_ONLY'
Must $boot.Contains('0xF800') 'PERSISTENCE_OFFSET'
Must $boot.Contains('0xF400') 'METADATA_OFFSET'
foreach ($name in @('stm32_asset_recovery.ps1','stm32_bootloader_recovery.ps1','stm32_readonly_flash_preflight.ps1')) {
    $body=[IO.File]::ReadAllText((Join-Path $scripts $name))
    Must $body.Contains('GetFileName($ProgrammerCli)') "PATH_LOGGING_$name"
}
$gcc=Extract-Resolver 'build_bootloader.ps1' 'ToolchainRoot' '$Gcc = Join-Path'
$gcc2=Extract-Resolver 'build_firmware.ps1' 'ToolchainRoot' '$Gcc = Join-Path'
Must ($gcc -ceq $gcc2) 'GCC_RESOLVER_DRIFT'
$cli=Extract-Resolver 'stm32_asset_recovery.ps1' 'ProgrammerCli' 'if (-not (Test-Path -LiteralPath $ProgrammerCli'
foreach ($n in @('stm32_bootloader_recovery.ps1','stm32_readonly_flash_preflight.ps1')) {
    $other=Extract-Resolver $n 'ProgrammerCli' 'if (-not (Test-Path -LiteralPath $ProgrammerCli'
    Must ($cli -ceq $other) "CLI_RESOLVER_DRIFT_$n"
}
$tmp=Join-Path ([IO.Path]::GetTempPath()) ('deus_ps_mock_' + [guid]::NewGuid().ToString('N'))
$one=Join-Path $tmp 'space directory one\bin'
$two=Join-Path $tmp 'space directory two\bin'
$empty=Join-Path $tmp 'empty'
$original=$env:PATH
try {
    foreach ($dir in @($one,$two,$empty)) { [void](New-Item -ItemType Directory -Path $dir -Force) }
    foreach ($dir in @($one,$two)) {
        foreach ($binary in @('STM32_Programmer_CLI.exe','arm-none-eabi-gcc.exe')) {
            [void](New-Item -ItemType File -Path (Join-Path $dir $binary) -Force)
        }
    }
    $provided=Join-Path $tmp 'explicit selection with spaces\tool.exe'
    Must ((Resolve-Test $gcc 'ToolchainRoot' $provided) -ceq $provided) 'EXPLICIT_GCC'
    Must ((Resolve-Test $cli 'ProgrammerCli' $provided) -ceq $provided) 'EXPLICIT_CLI'
    $env:PATH=$one
    Must ((Resolve-Test $cli 'ProgrammerCli' '') -ceq (Join-Path $one 'STM32_Programmer_CLI.exe')) 'ONE_CLI'
    Must ((Resolve-Test $gcc 'ToolchainRoot' '') -ceq (Split-Path -Parent $one)) 'ONE_GCC'
    $env:PATH=$one + [IO.Path]::PathSeparator + $two
    Expect-Ambiguous $cli 'ProgrammerCli' 'AMBIGUOUS_CLI'
    Expect-Ambiguous $gcc 'ToolchainRoot' 'AMBIGUOUS_GCC'
    $env:PATH=$empty
    Expect-Ambiguous $cli 'ProgrammerCli' 'ABSENT_CLI'
    Expect-Ambiguous $gcc 'ToolchainRoot' 'ABSENT_GCC'
} finally {
    $env:PATH=$original
    if (Test-Path -LiteralPath $tmp) { Remove-Item -LiteralPath $tmp -Recurse -Force }
}
Write-Host 'PS_PARSE=PASS scripts=7 generated_wrappers=4'
Write-Host 'PS_RESOLVER=PASS explicit=2 unique=2 ambiguous=2 missing=2'
Write-Host 'HARDWARE_ACCESS=NONE'
