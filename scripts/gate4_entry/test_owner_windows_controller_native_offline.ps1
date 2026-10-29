#requires -Version 7.0
# Gate4 A05 OFFLINE real Windows adapter/controller boundary qualifier.
# Temp-only ledger. No owner production ledger, no signing, no device I/O.
param([Parameter(Mandatory)][string]$Adapter,
      [Parameter(Mandatory)][string]$LedgerModule,
      [Parameter(Mandatory)][string]$ControllerPython)
Set-StrictMode -Version 3
$ErrorActionPreference='Stop'
$PSNativeCommandUseErrorActionPreference=$false
$Count=0
function Need([bool]$Ok,[string]$Code){if(-not $Ok){throw $Code}}
function Passed([string]$Name){$script:Count++;Write-Output ('G4_A05_NATIVE_CASE=PASS '+$Name)}
# PowerShell native invocation can split launcher '-3' into '-' and '3',
# causing the interpreter to read stdin forever. Use .NET ArgumentList and
# a closed stdin, with a hard timeout + entire-child-tree termination.
function Invoke-BoundedPythonProbe([string]$Executable,[string[]]$ArgumentVector,[int]$TimeoutMilliseconds){
    $info=[Diagnostics.ProcessStartInfo]::new()
    $info.FileName=$Executable
    $info.UseShellExecute=$false
    $info.CreateNoWindow=$true
    $info.RedirectStandardInput=$true
    $info.RedirectStandardOutput=$true
    $info.RedirectStandardError=$true
    foreach($part in $ArgumentVector){[void]$info.ArgumentList.Add([string]$part)}
    $proc=[Diagnostics.Process]::new()
    $proc.StartInfo=$info
    $stdoutTask=$null;$stderrTask=$null
    $didStart=$false;$timedOut=$false;$complete=$false
    try{
        $didStart=$proc.Start()
        Need $didStart 'PYTHON_PROBE_PROCESS_NOT_STARTED'
        $proc.StandardInput.Close()
        $stdoutTask=$proc.StandardOutput.ReadToEndAsync()
        $stderrTask=$proc.StandardError.ReadToEndAsync()
        $complete=$proc.WaitForExit($TimeoutMilliseconds)
        if(-not $complete){
            $timedOut=$true
            try{$proc.Kill($true)}catch{}
            [void]$proc.WaitForExit(2000)
        }
        $tasksReady=$stdoutTask.Wait(2000) -and $stderrTask.Wait(2000)
        if(-not $tasksReady){throw 'PYTHON_PROBE_STREAM_TIMEOUT'}
        $out=[string]$stdoutTask.Result
        $err=[string]$stderrTask.Result
        if($out.Length -gt 2000){$out=$out.Substring($out.Length-2000)}
        if($err.Length -gt 2000){$err=$err.Substring($err.Length-2000)}
        $code=if($timedOut){-2}elseif($proc.HasExited){[int]$proc.ExitCode}else{-3}
        return [pscustomobject]@{ ExitCode=$code; Timeout=$timedOut; Stdout=$out; Stderr=$err }
    }finally{
        if($didStart -and -not $proc.HasExited){
            try{$proc.Kill($true)}catch{}
            [void]$proc.WaitForExit(1000)
        }
        $proc.Dispose()
    }
}
function HashFile([string]$Path){
    $raw=[IO.File]::ReadAllBytes($Path)
    return [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($raw))
}
function SecureDir([string]$Folder){
    $acl=Get-Acl -LiteralPath $Folder -ErrorAction Stop
    $acl.SetAccessRuleProtection($true,$false)
    foreach($rule in @($acl.Access)){$null=$acl.RemoveAccessRuleSpecific($rule)}
    $owner=[Security.Principal.WindowsIdentity]::GetCurrent().User
    $inherit=[Security.AccessControl.InheritanceFlags]::ContainerInherit -bor [Security.AccessControl.InheritanceFlags]::ObjectInherit
    $rule=[Security.AccessControl.FileSystemAccessRule]::new($owner,[Security.AccessControl.FileSystemRights]::FullControl,$inherit,[Security.AccessControl.PropagationFlags]::None,[Security.AccessControl.AccessControlType]::Allow)
    $acl.AddAccessRule($rule)
    Set-Acl -LiteralPath $Folder -AclObject $acl -ErrorAction Stop
    Need ((Get-Acl -LiteralPath $Folder).AreAccessRulesProtected) 'OWNER_ACL_UNPROTECTED'
}
$root=Join-Path ([IO.Path]::GetTempPath()) ('deus_a05_controller_native_'+[guid]::NewGuid().ToString('N'))
$null=[IO.Directory]::CreateDirectory($root)
$engine=Join-Path $PSHOME 'pwsh.exe'
$nonce='abcdef0123456789abcdef0123456789'
$shaA='A'*64
$shaB='B'*64
try{
    Need ([OperatingSystem]::IsWindows()) 'WINDOWS_ONLY'
    Need ([IO.File]::Exists($engine)) 'PWSH_MISSING'
    Passed 'WINDOWS_POWERSHELL7'
    Need ((HashFile $Adapter) -ceq 'E4766F4FB5595475B3D886AF36EB9E8BE52A34F503DA0725DB28B7204D17E699') 'ADAPTER_SHA_CHANGED'
    Passed 'EXACT_CONTROLLER_ADAPTER_PIN'
    Need ((HashFile $LedgerModule) -ceq 'D67EB9820C016902FCDBC7FD3A13A51AE3F1317C33927E60B49D6B5565C351BC') 'RA03_MODULE_SHA_CHANGED'
    Passed 'EXACT_RA03_SOURCE_PIN'
    Need ([IO.File]::Exists($ControllerPython)) 'CONTROLLER_SOURCE_MISSING'
    Need ((HashFile $ControllerPython) -ceq '05B20216DD7354482C4AB009487DF4CF04E16FDB31482FC2DD67449F183CB2C6') 'CONTROLLER_SOURCE_PIN'
    Passed 'CONTROLLER_PYTHON_SOURCE_PIN'
    foreach($source in @($Adapter,$LedgerModule,$PSCommandPath)){
        $tok=$null;$err=$null
        [void][Management.Automation.Language.Parser]::ParseFile($source,[ref]$tok,[ref]$err)
        Need ($null -eq $err -or $err.Count -eq 0) 'SOURCE_AST_FAILED'
    }
    Passed 'NATIVE_POWERSHELL_AST_3'
    # Host dependency admission runs BEFORE any native nonce/ACL primitives.
    # Python's optional Windows Store App Execution Alias may resolve as
    # python.exe while a usable py.exe / Python3 is installed. Test both
    # candidate launchers; record a bounded original diagnostic on failure.
    # This probes imports only: no install, no pip, no nonce/USB operations.
    $probe='import sys;assert sys.version_info >= (3,10);sys.path.insert(0,sys.argv[1]);import cryptography,release_approval_contract_v1,verify_release_signature_v1,release_admission_source_composition_v1,owner_windows_release_controller_v1;print("G4_A05_CONTROLLER_IMPORT=PASS")'
    $pythonReady=$false
    $probeAttempted=0
    foreach($launcherName in @('py.exe','python.exe')){
        $launcher=Get-Command -Name $launcherName -CommandType Application -ErrorAction SilentlyContinue
        if($null -eq $launcher){continue}
        $probeAttempted++
        $argv=[Collections.Generic.List[string]]::new()
        if($launcherName -ceq 'py.exe'){[void]$argv.Add('-3')}
        [void]$argv.Add('-c')
        [void]$argv.Add($probe)
        [void]$argv.Add($PSScriptRoot)
        try{
            $result=Invoke-BoundedPythonProbe -Executable $launcher.Source -ArgumentVector $argv.ToArray() -TimeoutMilliseconds 12000
        }catch{
            $errorText=([string]$_.Exception.Message -replace '[\x00-\x1F\x7F]','?')
            if($errorText.Length -gt 900){$errorText=$errorText.Substring(0,900)}
            Write-Output ('G4_A05_PYTHON_PROBE_FAIL='+$launcherName+' EXIT=LAUNCH_OR_STREAM_STOP DETAIL='+$errorText)
            continue
        }
        if($result.ExitCode -eq 0 -and -not $result.Timeout -and
           $result.Stdout.Trim() -ceq 'G4_A05_CONTROLLER_IMPORT=PASS' -and
           [string]::IsNullOrWhiteSpace($result.Stderr)){
            $pythonReady=$true
            break
        }
        # Emit the terminal exception as its OWN indexed diagnostic record.
        # Never rely on the parent preserving a long flattened traceback line.
        $stderrLines=@([string]$result.Stderr -split '\r?\n' | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
        $lastError=if($stderrLines.Count -gt 0){[string]$stderrLines[-1]}else{'NO_STDERR_EXCEPTION_LINE'}
        $lastError=$lastError -replace '[\x00-\x1F\x7F]','?'
        if($lastError.Length -gt 450){$lastError=$lastError.Substring($lastError.Length-450)}
        Write-Output ('G4_A05_PYTHON_EXCEPTION='+$launcherName+' EXIT='+$result.ExitCode+' MESSAGE='+$lastError)
        $diag=(($result.Stdout+' '+$result.Stderr) -replace '[\x00-\x08\x0B-\x1F\x7F]','?')
        if($diag.Length -gt 1400){$diag=$diag.Substring($diag.Length-1400)}
        $diag=$diag -replace '\r?\n',' | '
        Write-Output ('G4_A05_PYTHON_PROBE_FAIL='+$launcherName+' EXIT='+$result.ExitCode+' TIMEOUT='+$result.Timeout+' DETAIL='+$diag)
    }
    if($probeAttempted -eq 0){
        Write-Output 'G4_A05_PYTHON_PROBE_FAIL=NO_PYTHON3_LAUNCHER_FOUND'
    }
    Need $pythonReady 'WINDOWS_PYTHON_SOURCE_OR_CRYPTOGRAPHY_IMPORT_FAILED'
    Passed 'WINDOWS_PYTHON_CRYPTOGRAPHY_CONTROLLER_IMPORT'
    $ledger=Join-Path $root 'ledger'
    $null=[IO.Directory]::CreateDirectory($ledger)
    SecureDir $ledger
    Import-Module $LedgerModule -Force -ErrorAction Stop
    $null=Assert-G4ProtectedLedgerDirectory $ledger
    Passed 'TEMP_OWNER_PROTECTED_ACL'
    $arguments=@('-NoLogo','-NoProfile','-NonInteractive','-File',$Adapter,
                  '-LedgerDirectory',$ledger,'-LedgerModule',$LedgerModule,
                  '-Nonce',$nonce,'-StatementSha256',$shaA,'-OperatorZipSha256',$shaB)
    $output=@(& $engine @arguments 2>&1)
    $code=[int]$LASTEXITCODE
    Need ($code -eq 0 -and $output.Count -eq 1) 'ADAPTER_FIRST_CALL_NOT_ONE_JSON'
    $obj=([string]$output[0])|ConvertFrom-Json -ErrorAction Stop
    Need ($obj.classification -ceq 'RA03_DURABLE_SOURCE_ONLY_NONCE_CONSUMED' -and
          $obj.nonce -ceq $nonce -and $obj.authorization -ceq 'DENIED' -and
          $obj.launch_authorized -eq $false -and $obj.device_io -ceq 'NONE') 'ADAPTER_RESULT_INVALID'
    Passed 'ONCE_ADAPTER_RETURNS_DENIED'
    $record=Join-Path $ledger ('used-'+$nonce+'.json')
    Need ([IO.File]::Exists($record)) 'CONSUMED_RECORD_ABSENT'
    $before=HashFile $record
    Need ($before -ceq $obj.record_sha256) 'CONSUMED_RECORD_SHA'
    Passed 'CREATE_NEW_DURABLE_RECORD_SHA'
    $repeat=@(& $engine @arguments 2>&1)
    Need ($LASTEXITCODE -ne 0) 'NONCE_REPLAY_NOT_STOPPED'
    Need ((HashFile $record) -ceq $before) 'NONCE_REPLAY_MODIFIED_RECORD'
    Passed 'REPLAY_STOP_NO_OVERWRITE'
    $changed=@($arguments)
    $changed[([Array]::IndexOf($changed,'-OperatorZipSha256')+1)]='C'*64
    $repeat=@(& $engine @changed 2>&1)
    Need ($LASTEXITCODE -ne 0 -and (HashFile $record) -ceq $before) 'REPLAY_CHANGED_SHA'
    Passed 'REPLAY_CHANGED_SHA_STOP'
    $invalid=@($arguments)
    $invalid[([Array]::IndexOf($invalid,'-Nonce')+1)]='0'*32
    $repeat=@(& $engine @invalid 2>&1)
    Need ($LASTEXITCODE -ne 0) 'ZERO_NONCE_ACCEPTED'
    Passed 'INVALID_NONCE_NO_LEDGER_WRITE'
    $invalid=@($arguments)
    $invalid[([Array]::IndexOf($invalid,'-StatementSha256')+1)]='a'*64
    $repeat=@(& $engine @invalid 2>&1)
    Need ($LASTEXITCODE -ne 0) 'LOWERCASE_SHA_ACCEPTED'
    Passed 'INVALID_SOURCE_SHA_NO_WRITE'
    $bad=Join-Path $root 'modified.psm1'
    [IO.File]::WriteAllBytes($bad,([IO.File]::ReadAllBytes($LedgerModule)+[byte[]](10,35,120)))
    $tampered=@($arguments)
    $tampered[([Array]::IndexOf($tampered,'-LedgerModule')+1)]=$bad
    $repeat=@(& $engine @tampered 2>&1)
    Need ($LASTEXITCODE -ne 0) 'TAMPERED_MODULE_ACCEPTED'
    Passed 'TAMPERED_LEDGER_MODULE_STOP'
    $inherited=Join-Path $root 'unprotected'
    $null=[IO.Directory]::CreateDirectory($inherited)
    $testArgs=@($arguments)
    $testArgs[([Array]::IndexOf($testArgs,'-LedgerDirectory')+1)]=$inherited
    $testArgs[([Array]::IndexOf($testArgs,'-Nonce')+1)]='1234567890abcdef1234567890abcdef'
    $repeat=@(& $engine @testArgs 2>&1)
    Need ($LASTEXITCODE -ne 0) 'UNPROTECTED_LEDGER_ACCEPTED'
    Need (-not [IO.File]::Exists((Join-Path $inherited 'used-1234567890abcdef1234567890abcdef.json'))) 'UNPROTECTED_LEDGER_CREATED_NONCE'
    Passed 'UNPROTECTED_DIRECTORY_STOP'
    Need (@(Get-ChildItem -LiteralPath $ledger -File -Filter 'used-*.json').Count -eq 1) 'LEDGER_COUNT_DRIFT'
    Passed 'ONE_TOTAL_RECORD_NO_RETRY'
    Write-Output ('G4_A05_NATIVE_OFFLINE=PASS COUNT='+$Count+' USB=NO SSH=NO RESET=NO FLASH=NO INSTALL=NO RELEASE=DENIED')
}
finally{
    if([IO.Directory]::Exists($root)){[IO.Directory]::Delete($root,$true)}
}
