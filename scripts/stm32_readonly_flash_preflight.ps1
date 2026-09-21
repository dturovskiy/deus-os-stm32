param(
    [string]$ProgrammerCli = "D:\Projects\STM32\Tools\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$downloads = Join-Path $env:USERPROFILE "Downloads"
if (-not (Test-Path -LiteralPath $downloads -PathType Container)) {
    throw "Downloads directory not found: $downloads"
}

if (-not (Test-Path -LiteralPath $ProgrammerCli -PathType Leaf)) {
    throw "STM32CubeProgrammer CLI not found: $ProgrammerCli"
}

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$logPath = Join-Path $downloads "stm32_os_readonly_flash_preflight_$timestamp.log"
$hashPath = "$logPath.sha256"

$script:log = [System.Collections.Generic.List[string]]::new()

function Add-Log {
    param([string]$Text)
    $script:log.Add($Text)
    Write-Host $Text
}

function Assert-ReadOnlyArguments {
    param([string[]]$Arguments)

    $joined = " " + ($Arguments -join " ").ToLowerInvariant() + " "

    $forbidden = @(
        " -e ", " --erase ",
        " -d ", " --download ",
        " -w8 ", " -w16 ", " -w32 ",
        " --write ",
        " -rdu ", " --readunprotect ",
        " -rst ", " --reset ",
        " -go ", " --start "
    )

    foreach ($token in $forbidden) {
        if ($joined.Contains($token)) {
            throw "Internal safety check rejected non-read-only STM32CubeProgrammer argument: $token"
        }
    }

    for ($i = 0; $i -lt $Arguments.Count; ++$i) {
        if ($Arguments[$i].ToLowerInvariant() -eq "-ob") {
            if (($i + 1) -ge $Arguments.Count -or
                $Arguments[$i + 1].ToLowerInvariant() -ne "displ") {
                throw "Internal safety check permits only '-ob displ'."
            }
        }
    }
}

function Invoke-ReadOnlyCubeCommand {
    param(
        [string]$Label,
        [string[]]$Arguments
    )

    Assert-ReadOnlyArguments -Arguments $Arguments

    Add-Log ""
    Add-Log "========================================================================"
    Add-Log "SECTION=$Label"
    Add-Log "COMMAND=STM32_Programmer_CLI.exe $($Arguments -join ' ')"
    Add-Log "========================================================================"

    $output = @(& $ProgrammerCli @Arguments 2>&1 | ForEach-Object { $_.ToString() })
    $exitCode = $LASTEXITCODE

    foreach ($line in $output) {
        Add-Log $line
    }

    Add-Log "EXIT_CODE=$exitCode"

    return [pscustomobject]@{
        ExitCode = $exitCode
        Output   = ($output -join [Environment]::NewLine)
    }
}

Add-Log "STM32 OS READ-ONLY MCU / FLASH PREFLIGHT"
Add-Log "TIMESTAMP=$(Get-Date -Format o)"
Add-Log "PROGRAMMER_CLI=$ProgrammerCli"
Add-Log "SAFETY=NO_ERASE_NO_DOWNLOAD_NO_WRITE_NO_OPTION_BYTE_PROGRAM_NO_READ_UNPROTECT"
Add-Log "NOTE=SWD attachment may affect live CPU execution state, but this script does not request nonvolatile mutation."

$frequencies = @(950, 480, 240, 125)
$selectedFrequency = $null
$discovery = $null

foreach ($frequency in $frequencies) {
    $candidate = Invoke-ReadOnlyCubeCommand -Label "SWD_DISCOVERY_$($frequency)KHZ" -Arguments @("-c", "port=SWD", "freq=$frequency")

    if ($candidate.ExitCode -eq 0 -and
        $candidate.Output -match "(?i)Device\s+ID") {
        $selectedFrequency = $frequency
        $discovery = $candidate
        break
    }
}

if ($null -eq $selectedFrequency) {
    $script:log | Set-Content -LiteralPath $logPath -Encoding utf8
    throw "Unable to establish a read-only SWD session at 950/480/240/125 kHz. Close STM32CubeProgrammer GUI and inspect $logPath"
}

Add-Log ""
Add-Log "SELECTED_SWD_KHZ=$selectedFrequency"

if ($discovery.Output -notmatch "(?i)0x410") {
    $script:log | Set-Content -LiteralPath $logPath -Encoding utf8
    throw "Unexpected target Device ID. Expected 0x410. Inspect $logPath"
}

$idcode = Invoke-ReadOnlyCubeCommand -Label "DBGMCU_IDCODE" -Arguments @("-c", "port=SWD", "freq=$selectedFrequency", "-r32", "0xE0042000", "0x4")
if ($idcode.ExitCode -ne 0) {
    $script:log | Set-Content -LiteralPath $logPath -Encoding utf8
    throw "DBGMCU_IDCODE read failed. Inspect $logPath"
}

$flashSize = Invoke-ReadOnlyCubeCommand -Label "FACTORY_FLASH_SIZE_REGISTER" -Arguments @("-c", "port=SWD", "freq=$selectedFrequency", "-r32", "0x1FFFF7E0", "0x4")
if ($flashSize.ExitCode -ne 0) {
    $script:log | Set-Content -LiteralPath $logPath -Encoding utf8
    throw "Factory Flash-size register read failed. Inspect $logPath"
}

$flashObr = Invoke-ReadOnlyCubeCommand -Label "FLASH_OBR" -Arguments @("-c", "port=SWD", "freq=$selectedFrequency", "-r32", "0x4002201C", "0x4")
if ($flashObr.ExitCode -ne 0) {
    $script:log | Set-Content -LiteralPath $logPath -Encoding utf8
    throw "FLASH_OBR read failed. Inspect $logPath"
}

$flashWrpr = Invoke-ReadOnlyCubeCommand -Label "FLASH_WRPR" -Arguments @("-c", "port=SWD", "freq=$selectedFrequency", "-r32", "0x40022020", "0x4")
if ($flashWrpr.ExitCode -ne 0) {
    $script:log | Set-Content -LiteralPath $logPath -Encoding utf8
    throw "FLASH_WRPR read failed. Inspect $logPath"
}

$optionBytes = Invoke-ReadOnlyCubeCommand -Label "OPTION_BYTES_DISPLAY" -Arguments @("-c", "port=SWD", "freq=$selectedFrequency", "-ob", "displ")
if ($optionBytes.ExitCode -ne 0) {
    $script:log | Set-Content -LiteralPath $logPath -Encoding utf8
    throw "Option-byte display failed. Inspect $logPath"
}

if ($optionBytes.Output -notmatch "(?i)RDP") {
    $script:log | Set-Content -LiteralPath $logPath -Encoding utf8
    throw "Option-byte output did not contain an RDP field. Inspect $logPath"
}

Add-Log ""
Add-Log "READ_ONLY_PREFLIGHT=PASS"
Add-Log "TARGET_DEVICE_ID_EXPECTED=0x410"
Add-Log "DBGMCU_IDCODE_ADDRESS=0xE0042000"
Add-Log "FLASH_SIZE_REGISTER_ADDRESS=0x1FFFF7E0"
Add-Log "FLASH_OBR_ADDRESS=0x4002201C"
Add-Log "FLASH_WRPR_ADDRESS=0x40022020"
Add-Log "OPTION_BYTES_MODE=DISPLAY_ONLY"
Add-Log "NONVOLATILE_MUTATION_REQUESTED=NO"

$script:log | Set-Content -LiteralPath $logPath -Encoding utf8
$sha256 = (Get-FileHash -LiteralPath $logPath -Algorithm SHA256).Hash.ToUpperInvariant()
"$sha256  $([System.IO.Path]::GetFileName($logPath))" | Set-Content -LiteralPath $hashPath -Encoding ascii

Write-Host ""
Write-Host "========================================================================"
Write-Host "READ_ONLY_PREFLIGHT=PASS"
Write-Host "LOG=$logPath"
Write-Host "LOG_SHA256=$sha256"
Write-Host "SHA256_FILE=$hashPath"
Write-Host "========================================================================"
