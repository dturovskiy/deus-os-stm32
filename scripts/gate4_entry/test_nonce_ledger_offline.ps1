#requires -Version 7.0
# RA-03 SOURCE-ONLY isolated Windows local nonce durability tests; NO device.
Set-StrictMode -Version 3
$ErrorActionPreference='Stop'
$count=0
function Need([bool]$Valid,[string]$Code){if(-not $Valid){throw $Code}}
function Passed([string]$Name){$script:count++;Write-Output ('RA03_NATIVE_WINDOWS_CASE=PASS '+$Name)}
function RejectCase([string]$Name,[scriptblock]$Action){
    $rejected=$false
    try { $null=& $Action } catch { $rejected=$true }
    Need $rejected ('RA03_EXPECTED_STOP_'+$Name)
    Passed $Name
}
function ProtectLedger([string]$Folder) {
    $acl=Get-Acl -LiteralPath $Folder -ErrorAction Stop
    $acl.SetAccessRuleProtection($true,$false)
    foreach($oldRule in @($acl.Access)){$null=$acl.RemoveAccessRuleSpecific($oldRule)}
    $sid=[Security.Principal.WindowsIdentity]::GetCurrent().User
    $inherit=[Security.AccessControl.InheritanceFlags]::ContainerInherit -bor [Security.AccessControl.InheritanceFlags]::ObjectInherit
    $rule=[Security.AccessControl.FileSystemAccessRule]::new($sid,[Security.AccessControl.FileSystemRights]::FullControl,$inherit,[Security.AccessControl.PropagationFlags]::None,[Security.AccessControl.AccessControlType]::Allow)
    $acl.AddAccessRule($rule)
    Set-Acl -LiteralPath $Folder -AclObject $acl -ErrorAction Stop
    $verify=Get-Acl -LiteralPath $Folder -ErrorAction Stop
    Need $verify.AreAccessRulesProtected 'RA03_NATIVE_ACL_PROTECTION_FAILED'
}
function NewLedger([string]$Parent,[string]$Name){
    $p=Join-Path $Parent $Name
    $null=[IO.Directory]::CreateDirectory($p)
    ProtectLedger $p
    return $p
}
$nonce1='0123456789abcdef0123456789abcdef'
$sha1='A'*64
$sha2='B'*64
$root=Join-Path ([IO.Path]::GetTempPath()) ('deus_ra03_offline_'+[guid]::NewGuid().ToString('N'))
$null=[IO.Directory]::CreateDirectory($root)
try {
    $module=Join-Path $PSScriptRoot 'nonce_ledger_offline.psm1'
    $child=Join-Path $PSScriptRoot 'nonce_ledger_child_offline.ps1'
    Need ([IO.File]::Exists($module) -and [IO.File]::Exists($child)) 'RA03_SOURCE_MISSING'
    Import-Module $module -Force -ErrorAction Stop
    Passed 'OFFLINE_MODULE_IMPORT'
    $dir=NewLedger $root 'secure'
    $null=Assert-G4ProtectedLedgerDirectory $dir
    Passed 'WINDOWS_OWNER_ACL_PROTECTED'
    $first=Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce $nonce1 -StatementSha256 $sha1 -OperatorZipSha256 $sha2
    Need ($first.classification -ceq 'RA03_DURABLE_SOURCE_ONLY_NONCE_CONSUMED' -and $first.authorization -ceq 'DENIED' -and -not $first.launch_authorized) 'RA03_POSITIVE_DID_NOT_DENY_RELEASE'
    $record=Join-Path $dir ('used-'+$nonce1+'.json')
    Need ([IO.File]::Exists($record)) 'RA03_RECORD_MISSING'
    $actual=[Convert]::ToHexString([Security.Cryptography.SHA256]::HashData([IO.File]::ReadAllBytes($record)))
    Need ($first.record_sha256 -ceq $actual) 'RA03_POSITIVE_SHA_MISMATCH'
    $obj=[IO.File]::ReadAllText($record)|ConvertFrom-Json -ErrorAction Stop
    Need ($obj.state -ceq 'CONSUMED_NO_AUTOMATIC_RETRY' -and $obj.authorization -ceq 'DENIED_SOURCE_ONLY') 'RA03_RECORD_CONTRACT'
    Passed 'ATOMIC_CREATE_FLUSH_TRUE_ORIGINAL_RECORD'
    RejectCase 'REPLAY_IDENTICAL' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce $nonce1 -StatementSha256 $sha1 -OperatorZipSha256 $sha2}
    RejectCase 'REPLAY_CHANGED_OPERATOR_SHA' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce $nonce1 -StatementSha256 $sha2 -OperatorZipSha256 $sha1}
    $before='11111111111111111111111111111111'
    RejectCase 'FAULT_BEFORE_CREATE' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce $before -StatementSha256 $sha1 -OperatorZipSha256 $sha2 -Fault BEFORE_CREATE}
    Need (-not [IO.File]::Exists((Join-Path $dir ('used-'+$before+'.json')))) 'RA03_BEFORE_CREATE_WRITTEN'
    Passed 'BEFORE_CREATE_ZERO_CONSUMPTION'
    foreach($f in @('AFTER_CREATE','AFTER_WRITE','AFTER_FLUSH')){
        $n=if($f -ceq 'AFTER_CREATE'){'22222222222222222222222222222222'}elseif($f -ceq 'AFTER_WRITE'){'33333333333333333333333333333333'}else{'44444444444444444444444444444444'}
        RejectCase ('INJECT_'+$f) {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce $n -StatementSha256 $sha1 -OperatorZipSha256 $sha2 -Fault $f}
        Need ([IO.File]::Exists((Join-Path $dir ('used-'+$n+'.json')))) ('RA03_RECORD_LOST_'+$f)
        RejectCase ('NO_RETRY_'+$f) {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce $n -StatementSha256 $sha1 -OperatorZipSha256 $sha2}
    }
    RejectCase 'NONCE_UPPERCASE' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce ('A'*32) -StatementSha256 $sha1 -OperatorZipSha256 $sha2}
    RejectCase 'NONCE_ZERO' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce ('0'*32) -StatementSha256 $sha1 -OperatorZipSha256 $sha2}
    RejectCase 'NONCE_TRAVERSAL' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce ('../'+$nonce1) -StatementSha256 $sha1 -OperatorZipSha256 $sha2}
    RejectCase 'SHA_LOWERCASE' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce ('5'*32) -StatementSha256 ('a'*64) -OperatorZipSha256 $sha2}
    RejectCase 'SHA_WRONG_LENGTH' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $dir -Nonce ('5'*32) -StatementSha256 'ABCDE' -OperatorZipSha256 $sha2}
    RejectCase 'UNC_ROOT' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory '\\fakehost\untrusted\ledger' -Nonce ('5'*32) -StatementSha256 $sha1 -OperatorZipSha256 $sha2}
    RejectCase 'MISSING_ROOT' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory (Join-Path $root 'not_created') -Nonce ('5'*32) -StatementSha256 $sha1 -OperatorZipSha256 $sha2}
    $unprotected=Join-Path $root 'not_acl_protected'
    $null=[IO.Directory]::CreateDirectory($unprotected)
    RejectCase 'INHERITED_ACL_REJECTED' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $unprotected -Nonce ('5'*32) -StatementSha256 $sha1 -OperatorZipSha256 $sha2}
    $badAccess=NewLedger $root 'unexpected_ace'
    # This fixture changes only the DACL. Set-Acl with a modified Get-Acl
    # descriptor tried to set SACL on owner Windows and required
    # SeSecurityPrivilege; icacls /grant does not require that privilege.
    $icacls=(Get-Command 'icacls.exe' -CommandType Application -ErrorAction Stop).Source
    $grantOutput=@(& $icacls $badAccess '/grant' '*S-1-1-0:(R)' 2>&1)
    Need ($LASTEXITCODE -eq 0) 'RA03_EVERYONE_FIXTURE_DACL_GRANT_FAILED'
    $acl=Get-Acl -LiteralPath $badAccess -ErrorAction Stop
    Need $acl.AreAccessRulesProtected 'RA03_EVERYONE_FIXTURE_NOT_PROTECTED'
    $actualAces=@($acl.GetAccessRules($true,$true,[Security.Principal.SecurityIdentifier]))
    Need (@($actualAces|Where-Object {
        $_.IdentityReference.Value -ceq 'S-1-1-0' -and
        $_.AccessControlType -eq [Security.AccessControl.AccessControlType]::Allow
    }).Count -gt 0) 'RA03_EVERYONE_FIXTURE_NOT_INSTALLED'
    $rejectedEveryone=$false
    try {
        $null=Write-G4ConsumedNonceSourceOnly -LedgerDirectory $badAccess -Nonce ('5'*32) -StatementSha256 $sha1 -OperatorZipSha256 $sha2
    } catch {
        if($_.Exception.Message -cne 'RA03_LEDGER_ACL_UNTRUSTED'){throw}
        $rejectedEveryone=$true
    }
    Need $rejectedEveryone 'RA03_EVERYONE_ACE_NOT_REJECTED'
    Need (-not [IO.File]::Exists((Join-Path $badAccess ('used-'+('5'*32)+'.json')))) 'RA03_UNTRUSTED_ACE_NONCE_CONSUMED'
    Passed 'UNEXPECTED_EVERYONE_ACE'
    $other=NewLedger $root 'parallel'
    $nParallel='abcdefabcdefabcdefabcdefabcdefab'
    $pwsh=Join-Path $PSHOME 'pwsh.exe'
    Need ([IO.File]::Exists($pwsh)) 'RA03_PWSH_EXE_MISSING'
    $arg=@('-NoLogo','-NoProfile','-NonInteractive','-File',('"' + $child + '"'),('"' + $other + '"'),$nParallel,$sha1,$sha2)
    $one=Start-Process -FilePath $pwsh -ArgumentList $arg -PassThru -WindowStyle Hidden -ErrorAction Stop
    $two=Start-Process -FilePath $pwsh -ArgumentList $arg -PassThru -WindowStyle Hidden -ErrorAction Stop
    try {
        Need ($one.WaitForExit(30000) -and $two.WaitForExit(30000)) 'RA03_PARALLEL_TIMEOUT'
        $codes=@($one.ExitCode,$two.ExitCode)|Sort-Object
        Need ($codes[0] -eq 0 -and $codes[1] -eq 21) 'RA03_PARALLEL_RACE_NOT_EXCLUSIVE'
    } finally {
        if(-not $one.HasExited){$one.Kill();$one.WaitForExit()}
        if(-not $two.HasExited){$two.Kill();$two.WaitForExit()}
        $one.Dispose();$two.Dispose()
    }
    Need ([IO.File]::Exists((Join-Path $other ('used-'+$nParallel+'.json')))) 'RA03_PARALLEL_ORIGINAL_MISSING'
    Passed 'TWO_PROCESS_CREATE_NEW_ONE_WINNER_ONE_STOP'
    $link=Join-Path $root 'junction'
    $target=NewLedger $root 'junction_target'
    $args='/d /c mklink /J "'+$link+'" "'+$target+'"'
    $p=Start-Process -FilePath (Join-Path $env:SystemRoot 'System32\cmd.exe') -ArgumentList $args -PassThru -Wait -WindowStyle Hidden -ErrorAction Stop
    Need ($p.ExitCode -eq 0 -and [IO.Directory]::Exists($link)) 'RA03_JUNCTION_NEGATIVE_NOT_PREPARED'
    RejectCase 'REPARSE_JUNCTION_REJECTED' {Write-G4ConsumedNonceSourceOnly -LedgerDirectory $link -Nonce ('5'*32) -StatementSha256 $sha1 -OperatorZipSha256 $sha2}
    # Never recurse through a reparse point during test cleanup.
    [IO.Directory]::Delete($link,$false)
    Write-Output ('RA03_OFFLINE_WINDOWS=PASS COUNT='+$count+' USB=NO SSH=NO RESET=NO FLASH=NO INSTALL=NO AUTHORIZATION=DENIED')
}
finally {
    if([IO.Directory]::Exists($root)){
        try{[IO.Directory]::Delete($root,$true)}catch{Write-Output ('RA03_CLEANUP_STOP='+$_.Exception.Message);throw}
    }
}
