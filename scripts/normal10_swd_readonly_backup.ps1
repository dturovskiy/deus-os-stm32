# BENCH preparation. NEVER launch without a separate scoped owner SWD GO.
# This script writes a PRIVATE 64-KiB host backup; it never writes target Flash.
param(
    [Parameter(Mandatory=$true)][string]$ProtectedBackupRoot,
    [Parameter(Mandatory=$true)][string]$WorkspaceRoot,
    [ValidateSet(950,480,240,125)][int]$SwdKHz = 950
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'
$FlashBytes = 65536
$ProgrammerCli = $null
$PinnedCubeCliSha256 = '269DB404ED3F2CDD7CFCCCC257AB0A907F2364C3D44E39EDAB6B007CB5F668ED'
$script:phase = 'HOST_PRECHECK'
$script:logPath = $null

function Add-Record {
    param([Parameter(Mandatory=$true)][string]$Text)
    Write-Host $Text
    if ($null -ne $script:logPath) {
        [System.IO.File]::AppendAllText(
            $script:logPath, ($Text + [Environment]::NewLine),
            [System.Text.UTF8Encoding]::new($false))
    }
}

function Assert-PrivateAcl {
    param([Parameter(Mandatory=$true)][string]$Path)
    $acl = Get-Acl -LiteralPath $Path -ErrorAction Stop
    foreach ($entry in $acl.Access) {
        $sid = $entry.IdentityReference.Translate(
            [System.Security.Principal.SecurityIdentifier]).Value
        if ($entry.AccessControlType -eq [System.Security.AccessControl.AccessControlType]::Allow -and
            $sid -in @('S-1-1-0','S-1-5-11','S-1-5-32-545')) {
            throw 'Protected Flash backup ACL grants access to a broad principal'
        }
    }
}

function Assert-AllowedCubeArguments {
    param([Parameter(Mandatory=$true)][string[]]$Arguments)
    $prefix = @('-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz))
    if ($Arguments.Count -lt 4) { throw 'Cube argument prefix absent' }
    for ($i = 0; $i -lt 4; ++$i) {
        if ($Arguments[$i] -cne $prefix[$i]) { throw 'Cube argument prefix mismatch' }
    }
    if ($Arguments.Count -eq 4) { return }
    if ($Arguments.Count -eq 7 -and
        $Arguments[4] -ceq '-r32' -and
        $Arguments[6] -ceq '0x4' -and
        $Arguments[5] -cin @('0x1FFFF7E0','0x4002201C','0x40022020')) { return }
    if ($Arguments.Count -eq 6 -and
        $Arguments[4] -ceq '-ob' -and $Arguments[5] -ceq 'displ') { return }
    if ($Arguments.Count -eq 8 -and
        $Arguments[4] -ceq '--upload' -and
        $Arguments[5] -ceq '0x08000000' -and
        $Arguments[6] -ceq '0x10000' -and
        $Arguments[7] -ceq $script:backupPath) { return }
    throw 'Forbidden or unexpected CubeProgrammer operation'
}

function Invoke-ReadOnlyCube {
    param(
        [Parameter(Mandatory=$true)][string]$Label,
        [Parameter(Mandatory=$true)][string[]]$Arguments,
        [int]$TimeoutMs = 30000
    )
    Assert-AllowedCubeArguments -Arguments $Arguments
    $script:phase = $Label
    Add-Record ('PHASE=' + $Label)
    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $ProgrammerCli
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.RedirectStandardInput = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    foreach ($argument in $Arguments) { [void]$psi.ArgumentList.Add($argument) }
    $process = [System.Diagnostics.Process]::new()
    try {
        $process.StartInfo = $psi
        if (-not $process.Start()) { throw ('Cube did not start: ' + $Label) }
        $process.StandardInput.Close()
        $stdoutTask = $process.StandardOutput.ReadToEndAsync()
        $stderrTask = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit($TimeoutMs)) {
            try { $process.Kill($true) } catch {}
            throw ('Cube timed out: ' + $Label)
        }
        $stdout = $stdoutTask.GetAwaiter().GetResult()
        $stderr = $stderrTask.GetAwaiter().GetResult()
        $exitCode = $process.ExitCode
        Add-Record ('EXIT_CODE=' + $exitCode)
        if ($exitCode -ne 0) {
            foreach ($line in @($stdout -split '\r?\n' | Select-Object -Last 8)) {
                if ($line.Length -gt 0) { Add-Record ('STDOUT_TAIL=' + $line) }
            }
            foreach ($line in @($stderr -split '\r?\n' | Select-Object -Last 8)) {
                if ($line.Length -gt 0) { Add-Record ('STDERR_TAIL=' + $line) }
            }
            throw ('Cube failed: ' + $Label)
        }
        $combined = $stdout + [Environment]::NewLine + $stderr
        if ($combined -notmatch '(?im)^\s*Connect mode\s*:\s*Hot Plug\s*$') {
            throw ('HOTPLUG not confirmed: ' + $Label)
        }
        return $combined
    }
    finally { $process.Dispose() }
}

function Read-Register {
    param([string]$Address)
    $valueText = Invoke-ReadOnlyCube -Label ('READ_' + $Address) -Arguments @(
        '-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz),'-r32',$Address,'0x4')
    $pattern = '(?im)' + [regex]::Escape($Address) + '\s*[:=]\s*(?:0x)?([0-9a-f]{8})'
    if ($valueText -notmatch $pattern) { throw ('Register parse failed: ' + $Address) }
    return [Convert]::ToUInt32($Matches[1],16)
}

try {
    $root = [System.IO.Path]::GetFullPath($ProtectedBackupRoot)
    if (-not (Test-Path -LiteralPath $root -PathType Container)) {
        throw 'Protected backup directory must already exist'
    }
    $workspaceFull = [System.IO.Path]::GetFullPath($WorkspaceRoot)
    if (-not (Test-Path -LiteralPath (Join-Path $workspaceFull 'OS/.git') -PathType Container)) {
        throw 'Expected Windows workspace OS Git root is missing'
    }
    $expectedProtectedRoot = [System.IO.Path]::GetFullPath(
        (Join-Path $workspaceFull 'ProtectedRecovery'))
    $comparison = [System.StringComparison]::OrdinalIgnoreCase
    if (-not ($root.Equals($expectedProtectedRoot,$comparison) -or
        $root.StartsWith(($expectedProtectedRoot.TrimEnd('\') + '\'),$comparison))) {
        throw 'Output must be under the workspace sibling ProtectedRecovery directory'
    }
    $item = Get-Item -LiteralPath $root -Force
    while ($null -ne $item) {
        if (($item.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw 'Reparse-point ancestor forbidden for protected backup'
        }
        $item = $item.Parent
    }
    Assert-PrivateAcl -Path $root
    # Proven Windows Gate3 CubeProgrammer identity: workspace sibling Tools, not PATH.
    # No environment override, Get-Command fallback, installer scan or network lookup.
    $cubeDir = [System.IO.Path]::GetFullPath(
        (Join-Path $workspaceFull 'Tools/STM32Cube/STM32CubeProgrammer/bin'))
    $ProgrammerCli = [System.IO.Path]::GetFullPath(
        (Join-Path $cubeDir 'STM32_Programmer_CLI.exe'))
    foreach ($candidate in @(
        (Join-Path $workspaceFull 'Tools'),
        (Join-Path $workspaceFull 'Tools/STM32Cube'),
        (Join-Path $workspaceFull 'Tools/STM32Cube/STM32CubeProgrammer'),
        $cubeDir
    )) {
        if (-not [System.IO.Directory]::Exists($candidate)) {
            throw 'CUBE_CLI_PINNED_DIRECTORY_MISSING'
        }
        if (([System.IO.File]::GetAttributes($candidate) -band
            [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw 'CUBE_CLI_DIRECTORY_REPARSE_FORBIDDEN'
        }
    }
    if (-not [System.IO.File]::Exists($ProgrammerCli)) {
        throw 'CUBE_CLI_PINNED_WORKSPACE_TOOLS_MISSING'
    }
    if (([System.IO.File]::GetAttributes($ProgrammerCli) -band
        [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
        throw 'CUBE_CLI_BINARY_REPARSE_FORBIDDEN'
    }
    $actualCubeSha = (Get-FileHash -LiteralPath $ProgrammerCli -Algorithm SHA256).Hash.ToUpperInvariant()
    if ($actualCubeSha -cne $PinnedCubeCliSha256) {
        throw 'CUBE_CLI_PINNED_SHA256_MISMATCH'
    }
    $id = 'normal10_swd_preflight_' + (Get-Date).ToUniversalTime().ToString('yyyyMMddTHHmmssZ') +
        '_' + [Guid]::NewGuid().ToString('N').Substring(0,8)
    $runDir = Join-Path $root $id
    if (Test-Path -LiteralPath $runDir) { throw 'Preflight output already exists' }
    [void](New-Item -ItemType Directory -Path $runDir -ErrorAction Stop)
    Assert-PrivateAcl -Path $runDir
    $script:logPath = Join-Path $runDir 'receipt.txt'
    $script:backupPath = Join-Path $runDir 'preflash64k.bin'
    Add-Record 'SCHEMA=DEUS_NORMAL10_SWD_BENCH_READONLY_V1'
    Add-Record ('UTC=' + (Get-Date).ToUniversalTime().ToString('o'))
    Add-Record ('RUN_ID=' + $id)
    Add-Record ('SWD_KHZ=' + $SwdKHz)
    Add-Record 'HOST_BACKUP_DESTINATION=ProtectedRecovery'
    Add-Record 'CUBE_CLI_RESOLUTION=PINNED_WORKSPACE_TOOLS_SHA256'
    Add-Record ('CUBE_CLI_SHA256=' + $actualCubeSha)
    Add-Record 'NONVOLATILE_MUTATION_REQUESTED=NO'
    Add-Record 'NO_RESET_NO_ERASE_NO_PROGRAM_NO_OPTION_BYTE_WRITE=YES'
    Add-Record 'CPU_NONINTERFERENCE_NOT_ATTESTED=YES'

    $connection = Invoke-ReadOnlyCube -Label 'SWD_HOTPLUG_CONNECT' -Arguments @(
        '-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz))
    if ($connection -notmatch '(?im)Device\s+ID\s*:\s*0x0*410\b') {
        throw 'Target mismatch: DEV_ID != 0x410'
    }
    if ($connection -notmatch '(?im)Voltage\s*:\s*([0-9]+(?:\.[0-9]+)?)\s*V') {
        throw 'Target voltage unavailable'
    }
    $voltage = [double]::Parse($Matches[1],[Globalization.CultureInfo]::InvariantCulture)
    if ($voltage -lt 2.0 -or $voltage -gt 3.6) {
        throw 'Target voltage outside verified Flash programming range'
    }
    Add-Record 'TARGET_DEVICE_ID=0x410'
    Add-Record ('TARGET_VOLTAGE_V=' + $voltage.ToString('F2',[Globalization.CultureInfo]::InvariantCulture))
    $size = Read-Register -Address '0x1FFFF7E0'
    if (($size -band 0xFFFF) -ne 64) { throw 'Factory Flash size != 64 KiB' }
    $obr = Read-Register -Address '0x4002201C'
    if ($obr -ne 0x000003FC) { throw 'FLASH_OBR != 0x000003FC' }
    $wrpr = Read-Register -Address '0x40022020'
    if ($wrpr -ne [uint32]::MaxValue) { throw 'FLASH_WRPR != 0xFFFFFFFF' }
    $optionBytes = Invoke-ReadOnlyCube -Label 'OPTION_BYTES_DISPLAY' -Arguments @(
        '-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz),'-ob','displ')
    if ($optionBytes -notmatch '(?i)RDP') { throw 'Option bytes display missing RDP' }
    Add-Record 'DEV_ID_FLASH64K_OBR_WRPR_RDP_DISPLAY=PASS'

    [void](Invoke-ReadOnlyCube -Label 'FULL_FLASH_READBACK' -TimeoutMs 120000 -Arguments @(
        '-c','port=SWD','mode=HOTPLUG',('freq=' + $SwdKHz),
        '--upload','0x08000000','0x10000',$script:backupPath))
    $file = Get-Item -LiteralPath $script:backupPath -ErrorAction Stop
    if ($file.Length -ne $FlashBytes) { throw '64-KiB Flash backup size mismatch' }
    $flash = [System.IO.File]::ReadAllBytes($script:backupPath)
    Add-Record ('FLASH_BACKUP_BYTES=' + $flash.Length)
    Add-Record ('FLASH_BACKUP_SHA256=' + (Get-FileHash -LiteralPath $script:backupPath -Algorithm SHA256).Hash.ToUpperInvariant())
    Add-Record ('META_A_UNAUTHENTICATED_VERSION=' + [BitConverter]::ToUInt32($flash,0xF00C))
    Add-Record ('META_B_UNAUTHENTICATED_VERSION=' + [BitConverter]::ToUInt32($flash,0xF40C))
    Add-Record ('META_A_MARKER=0x{0:X4}' -f [BitConverter]::ToUInt16($flash,0xF050))
    Add-Record ('META_B_MARKER=0x{0:X4}' -f [BitConverter]::ToUInt16($flash,0xF450))
    Add-Record 'AUTHENTICATED_VERSION_FLOOR=NOT_ATTESTED'
    Add-Record 'POST_SWD_USB_RUNTIME=NOT_ATTESTED'
    Add-Record 'SIGNED_NORMAL10_RESCUE11_NORMAL12=NOT_ATTESTED'
    Add-Record 'PREFLIGHT=PASS_READONLY_SWD_BACKUP_ONLY'
}
catch {
    Add-Record ('PREFLIGHT=FAIL PHASE=' + $script:phase)
    Add-Record ('ERROR=' + $_.Exception.GetType().FullName + ': ' + $_.Exception.Message)
    throw
}
