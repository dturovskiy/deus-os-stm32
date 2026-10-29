#requires -Version 7.0
# DEUS OS Gate4 A05: OFFLINE isolated host dependencies ONLY. No global Python mutation, nonce, USB, SSH, reset or Flash.
Set-StrictMode -Version 3
$ErrorActionPreference='Stop'
$PackageId='stm32_os_gate4_a05_cp314_isolated_deploy_v2_20261029'
$utf8=[Text.UTF8Encoding]::new($false,$true)
$nl=[Environment]::NewLine
$phase='INIT'
$outcome='FAIL'
$class='A05_DEPENDENCY_ISOLATION_NOT_PROVEN'
$reason='NOT_FINISHED'
$evidenceDir=''
$evidencePath='NONE'
$evidenceSha='NONE'
$repo=''
$pre=''
$post=''
$childStatus='NOT_STARTED'
$terminal='NOT_ATTESTED'
$dependencyStatus='NOT_ATTESTED'
$isolatedTarget='NONE'
$isolatedTreeSha='NONE'
$receiptSha='NONE'
$lines=[Collections.Generic.List[string]]::new()
$checks=[Collections.Generic.List[string]]::new()
function Need([bool]$ok,[string]$errorCode){if(-not $ok){throw $errorCode}}
function ShaFile([string]$p){
  $stream=[IO.File]::OpenRead($p)
  try{return [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($stream))}
  finally{$stream.Dispose()}
}
function WriteNew([string]$p,[string]$text){
  $bytes=$utf8.GetBytes($text)
  $f=[IO.File]::Open($p,[IO.FileMode]::CreateNew,[IO.FileAccess]::Write,[IO.FileShare]::None)
  try{$f.Write($bytes,0,$bytes.Length);$f.Flush($true)}finally{$f.Dispose()}
}
function SafePath([string]$p){
  $full=[IO.Path]::GetFullPath($p)
  Need (-not $full.StartsWith('\\')) 'UNC_FORBIDDEN'
  $current=$full
  while(-not [string]::IsNullOrEmpty($current)){
    if([IO.File]::Exists($current) -or [IO.Directory]::Exists($current)){
      Need (([IO.File]::GetAttributes($current) -band [IO.FileAttributes]::ReparsePoint) -eq 0) 'REPARSE_DENIED'
    }
    $parent=[IO.Directory]::GetParent($current)
    if($null -eq $parent){break}
    $current=$parent.FullName
  }
  return $full
}
function LocateRepo(){
  $candidates=@((Get-Location).Path)
  if($env:DEUS_OS_ROOT){$candidates=@($env:DEUS_OS_ROOT)+$candidates}
  foreach($base in $candidates){
    $p=[IO.Path]::GetFullPath($base)
    for($n=0;$n -lt 12;$n++){
      if((Split-Path $p -Leaf) -ceq 'OS' -and [IO.Directory]::Exists((Join-Path $p '.git'))){
        return (SafePath $p)
      }
      $parent=[IO.Directory]::GetParent($p)
      if($null -eq $parent){break}
      $p=$parent.FullName
    }
  }
  throw 'OS_ROOT_UNAVAILABLE'
}
function GitFingerprint(){
  $result=@(& git -C $repo status --porcelain=v1 --untracked-files=all 2>&1)
  Need ($LASTEXITCODE -eq 0) 'GIT_STATUS_FAILED'
  return [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($utf8.GetBytes(($result -join [char]10)+[char]10)))
}
function AddCheck([string]$name,[bool]$ok){
  Need $ok ('CHECK_FAILED_'+$name)
  [void]$checks.Add('PASS '+$name)
}
function Clean([string]$s,[int]$limit=900){
  $v=$s -replace '[\x00-\x08\x0B-\x1F\x7F]','?'
  $v=$v -replace '(?i)[A-Z]:\\[^\s\x22]+','[LOCAL_PATH]'
  if($v.Length -gt $limit){
    $head=$v.Substring(0,100)
    $tail=$v.Substring($v.Length-($limit-160))
    $v=$head+' [MIDDLE_TRUNCATED_TAIL_PRESERVED] '+$tail
  }
  return $v
}
function BoundedPy([string]$exe,[string]$script,[int]$ms,[string[]]$extra){
  $psi=[Diagnostics.ProcessStartInfo]::new()
  $psi.FileName=$exe
  $psi.CreateNoWindow=$true
  $psi.UseShellExecute=$false
  $psi.RedirectStandardInput=$true
  $psi.RedirectStandardOutput=$true
  $psi.RedirectStandardError=$true
  # Exact '-3' via ArgumentList; never PowerShell-native '- 3' split.
  foreach($p in (@('-3',$script)+$extra)){[void]$psi.ArgumentList.Add([string]$p)}
  $proc=[Diagnostics.Process]::new()
  $proc.StartInfo=$psi
  $started=$false;$timeout=$false
  try{
    $started=$proc.Start()
    Need $started 'PYTHON_START_FAILED'
    $proc.StandardInput.Close()
    $stdout=$proc.StandardOutput.ReadToEndAsync()
    $stderr=$proc.StandardError.ReadToEndAsync()
    if(-not $proc.WaitForExit($ms)){
      $timeout=$true
      try{$proc.Kill($true)}catch{}
      [void]$proc.WaitForExit(2000)
    }
    Need ($stdout.Wait(2000) -and $stderr.Wait(2000)) 'PYTHON_OUTPUT_DRAIN_TIMEOUT'
    $out=[string]$stdout.Result
    $err=[string]$stderr.Result
    Need ($out.Length -lt 40000 -and $err.Length -lt 24000) 'PYTHON_OUTPUT_OVERSIZED'
    $exitCode=if($timeout){-2}else{[int]$proc.ExitCode}
    return [pscustomobject]@{
      ExitCode=$exitCode
      TimedOut=$timeout
      Stdout=$out
      Stderr=$err
    }
  }finally{
    if($started -and -not $proc.HasExited){
      try{$proc.Kill($true)}catch{}
      [void]$proc.WaitForExit(1000)
    }
    $proc.Dispose()
  }
}
function Seal([string]$directory){
  Need ([IO.Directory]::Exists($directory)) 'EVIDENCE_DIRECTORY_MISSING'
  if($outcome -ceq 'FAIL' -and $childStatus -cne 'NOT_STARTED'){
    Need ($lines.ToArray() -match '^TERMINAL_EXCEPTION=') 'DIAGNOSTIC_INCOMPLETE_TERMINAL_EXCEPTION'
  }
  $base=$PackageId+'_'+(Get-Date).ToString('yyyyMMdd_HHmmss')+'_'+[guid]::NewGuid().ToString('N')
  $stage=Join-Path $directory ('.'+$base)
  $part=Join-Path $directory ($base+'.evidence.zip.tmp')
  $final=Join-Path $directory ($base+'.evidence.zip')
  Need (-not [IO.File]::Exists($final) -and -not [IO.Directory]::Exists($stage)) 'EVIDENCE_COLLISION'
  [void][IO.Directory]::CreateDirectory($stage)
  try{
    $typed=[ordered]@{
      schema='DEUS_A05_CP314_HOST_ISOLATED_DEPLOY_EVIDENCE_V1'
      gate=$PackageId;result=$outcome;classification=$class;failure_phase=$phase;error=$reason
      isolated_status=$dependencyStatus;child_exit_code=$childStatus;terminal_exception=$terminal
      isolated_target_basename=$isolatedTarget;target_tree_sha256=$isolatedTreeSha
      receipt_sha256=$receiptSha
      worker_sha256='492E95B7F1FCD221EEAFF653BF4AEDAF61ED0FB5488C78A3AD518DA26E0F8445';wheel_verifier_sha256='880923C0DA9A81BB81105584679F32AE9B5AC3949A8300357F66F6756B2ECB2C';selection_sha256='28B6FFE7AD8B39DC81D9EA0E8B5AA7FDDAF8E13FA5018C8EA26AB918A1B0CBFC'
      original_wheelhouse_sha256='CD45CB76A47C893DE0032BF53E3EAB1304F0822D56D29254CB10A0888702198C'
      original_download_evidence_sha256='9D444C61709C9C98AAEB43B2B6D75E2AB0D4F02116D1B3F41B8990ED968617DD'
      git_pre_sha256=$pre;git_post_sha256=$post;network='NO'
      installation='ISOLATED_HOST_DEPENDENCY_TARGET_ONLY'
      wheel_bytes_verified='ORIGINAL_ACCEPTED';global_python_changed='NO';device_io='NONE'
      usb='NO';ssh='NO';reset='NO';flash='NO';nonce_ledger='NOT_USED'
      gate4='NOT_AUTHORIZED';time=(Get-Date).ToString('o')
    }
    WriteNew (Join-Path $stage 'outcome.txt') (($typed|ConvertTo-Json -Depth 7)+$nl)
    WriteNew (Join-Path $stage 'native_checks.txt') (($checks.ToArray() -join $nl)+$nl)
    WriteNew (Join-Path $stage 'run.log') ('PACKAGE_ID='+$PackageId+$nl+'RESULT='+$outcome+$nl+
      'CLASSIFICATION='+$class+$nl+'FAILURE_PHASE='+$phase+$nl+'ERROR='+$reason+$nl+
      'CHILD_EXIT='+$childStatus+$nl+'ISOLATED_STATUS='+$dependencyStatus+$nl+
      'ISOLATED_TARGET='+$isolatedTarget+$nl+'TREE_SHA256='+$isolatedTreeSha+$nl+
      'USB=NO SSH=NO RESET=NO FLASH=NO GLOBAL_INSTALL=NO'+$nl)
    WriteNew (Join-Path $stage 'native_test_output.txt') (($lines.ToArray() -join $nl)+$nl)
    $names=@('outcome.txt','native_checks.txt','run.log','native_test_output.txt')
    $sizeIndex=[Collections.Generic.List[string]]::new()
    $shaIndex=[Collections.Generic.List[string]]::new()
    foreach($n in $names){
      $p=Join-Path $stage $n
      [void]$sizeIndex.Add(([IO.FileInfo]::new($p)).Length.ToString()+'  '+$n)
      [void]$shaIndex.Add((ShaFile $p)+'  '+$n)
    }
    WriteNew (Join-Path $stage 'sizes.txt') (($sizeIndex.ToArray() -join $nl)+$nl)
    [void]$shaIndex.Add((ShaFile (Join-Path $stage 'sizes.txt'))+'  sizes.txt')
    WriteNew (Join-Path $stage 'hashes.sha256') (($shaIndex.ToArray() -join $nl)+$nl)
    [IO.Compression.ZipFile]::CreateFromDirectory($stage,$part,[IO.Compression.CompressionLevel]::Optimal,$false)
    $zip=[IO.Compression.ZipFile]::OpenRead($part)
    try{
      Need ($zip.Entries.Count -eq 6) 'EVIDENCE_MEMBER_COUNT'
      foreach($e in $zip.Entries){
        Need ($e.FullName -cin @('outcome.txt','native_checks.txt','run.log','native_test_output.txt','sizes.txt','hashes.sha256')) 'EVIDENCE_MEMBER_NAME'
        $source=Join-Path $stage $e.FullName
        Need ($e.Length -eq ([IO.FileInfo]::new($source)).Length) 'EVIDENCE_MEMBER_SIZE'
        $stream=$e.Open()
        try{$digest=[Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($stream))}
        finally{$stream.Dispose()}
        Need ($digest -ceq (ShaFile $source)) 'EVIDENCE_MEMBER_HASH'
      }
    }finally{$zip.Dispose()}
    [IO.File]::Move($part,$final,$false)
    return [pscustomobject]@{Path=$final;Sha=(ShaFile $final)}
  }finally{
    if([IO.Directory]::Exists($stage)){try{[IO.Directory]::Delete($stage,$true)}catch{}}
    if([IO.File]::Exists($part)){try{[IO.File]::Delete($part)}catch{}}
  }
}

function AssertHostDependencyOwnerACL([string]$directory){
  $id=[Security.Principal.WindowsIdentity]::GetCurrent()
  $sid=$id.User
  if(-not [IO.Directory]::Exists($directory)){
    [void][IO.Directory]::CreateDirectory($directory)
    $acl=Get-Acl -LiteralPath $directory -ErrorAction Stop
    $acl.SetAccessRuleProtection($true,$false)
    foreach($entry in @($acl.Access)){$null=$acl.RemoveAccessRuleSpecific($entry)}
    $inherit=[Security.AccessControl.InheritanceFlags]::ContainerInherit -bor [Security.AccessControl.InheritanceFlags]::ObjectInherit
    $rule=[Security.AccessControl.FileSystemAccessRule]::new(
      $sid,[Security.AccessControl.FileSystemRights]::FullControl,$inherit,
      [Security.AccessControl.PropagationFlags]::None,[Security.AccessControl.AccessControlType]::Allow)
    $acl.AddAccessRule($rule)
    Set-Acl -LiteralPath $directory -AclObject $acl -ErrorAction Stop
  }
  $now=Get-Acl -LiteralPath $directory -ErrorAction Stop
  Need $now.AreAccessRulesProtected 'HOST_DEPENDENCY_ACL_NOT_PROTECTED'
  $ownerAllowed=$false
  $trusted=@($sid.Value,'S-1-5-18','S-1-5-32-544')
  foreach($rule in $now.Access){
    $actualSid=''
    try{$actualSid=$rule.IdentityReference.Translate([Security.Principal.SecurityIdentifier]).Value}
    catch{$actualSid='UNRESOLVED_ACL_PRINCIPAL'}
    if($rule.AccessControlType -eq [Security.AccessControl.AccessControlType]::Allow){
      Need ($actualSid -cin $trusted) 'HOST_DEPENDENCY_UNTRUSTED_ACL_ALLOW'
      if($actualSid -ceq $sid.Value){$ownerAllowed=$true}
    }
  }
  Need $ownerAllowed 'HOST_DEPENDENCY_OWNER_NOT_ALLOWED'
}

try{
  $phase='PRESTATE'
  Need ([OperatingSystem]::IsWindows()) 'WINDOWS_ONLY'
  $repo=LocateRepo
  $parent=([IO.Directory]::GetParent($repo)).FullName
  $evidenceDir=SafePath (Join-Path $parent 'Evidence')
  Need ([IO.Directory]::Exists($evidenceDir)) 'CANONICAL_EVIDENCE_DIR_MISSING'
  $pre=GitFingerprint
  $phase='PIN_SOURCES'
  $worker='deploy_a05_isolated_cp314_wheels_v1.py'
  $guard='audit_a05_windows_wheelhouse_v1.py'
  $selection='a05_windows_python314_regular_wheel_selection_v1.json'
  foreach($name in @($worker,$guard,$selection)){
    $want=switch($name){
      'deploy_a05_isolated_cp314_wheels_v1.py' {'492E95B7F1FCD221EEAFF653BF4AEDAF61ED0FB5488C78A3AD518DA26E0F8445'}
      'audit_a05_windows_wheelhouse_v1.py' {'880923C0DA9A81BB81105584679F32AE9B5AC3949A8300357F66F6756B2ECB2C'}
      default {'28B6FFE7AD8B39DC81D9EA0E8B5AA7FDDAF8E13FA5018C8EA26AB918A1B0CBFC'}
    }
    AddCheck ('ZIP_SOURCE_'+$name.Replace('.','_')) ((ShaFile (SafePath (Join-Path $PSScriptRoot $name))) -ceq $want)
    AddCheck ('REPO_SOURCE_'+$name.Replace('.','_')) ((ShaFile (SafePath (Join-Path (Join-Path $repo 'scripts/gate4_entry') $name))) -ceq $want)
  }
  $originalWheelhouseFile=SafePath (Join-Path (Join-Path $parent 'Wheelhouse') 'a05_python314_regular_wheels_20261028_04ba1366de464d8aad14622aa90ae956.zip')
  AddCheck 'ORIGINAL_WHEELHOUSE_SHA' ((ShaFile $originalWheelhouseFile) -ceq 'CD45CB76A47C893DE0032BF53E3EAB1304F0822D56D29254CB10A0888702198C')
  $originalDownloadEvidence=SafePath (Join-Path (Join-Path $parent 'Evidence') 'stm32_os_gate4_a05_python314_wheel_download_v1_20261028_20261028_234401_166a876f51fd4febaa490cec9ae41239.evidence.zip')
  AddCheck 'ORIGINAL_DOWNLOAD_EVIDENCE_SHA' ((ShaFile $originalDownloadEvidence) -ceq '9D444C61709C9C98AAEB43B2B6D75E2AB0D4F02116D1B3F41B8990ED968617DD')
  $phase='HOST_TARGET_ACL'
  $hostTarget=SafePath (Join-Path $parent 'HostDependencies')
  AssertHostDependencyOwnerACL $hostTarget
  AddCheck 'HOST_DEPENDENCY_OWNER_PROTECTED_ACL' $true
  $phase='PYTHON_LAUNCHER'
  $cmd=Get-Command -Name 'py.exe' -CommandType Application -ErrorAction SilentlyContinue
  Need ($null -ne $cmd) 'PYTHON_LAUNCHER_MISSING'
  AddCheck 'PYTHON3_LAUNCHER_PRESENT' $true
  $phase='OFFLINE_ISOLATED_DEPENDENCY_DEPLOY'
  $ret=BoundedPy $cmd.Source (Join-Path $PSScriptRoot $worker) 215000 @((Join-Path $parent 'Wheelhouse'),$hostTarget,$repo,$PSScriptRoot,$cmd.Source)
  $childStatus=[string]$ret.ExitCode
  [void]$lines.Add('G4_A05_CHILD_EXIT='+$childStatus)
  foreach($line in ($ret.Stdout -split '\r?\n')){
    if(-not [string]::IsNullOrWhiteSpace($line)){[void]$lines.Add('CHILD_STDOUT='+(Clean $line 1100))}
  }
  foreach($line in ($ret.Stderr -split '\r?\n')){
    if(-not [string]::IsNullOrWhiteSpace($line)){[void]$lines.Add('CHILD_STDERR='+(Clean $line 1100))}
  }
  if($ret.TimedOut){
    $terminal='TimeoutError: Python isolation worker exceeded 215s; process tree termination requested'
    [void]$lines.Add('TERMINAL_EXCEPTION='+$terminal)
    throw 'ISOLATED_WORKER_TIMEOUT'
  }
  $statusRows=@($ret.Stdout -split '\r?\n' | Where-Object {$_ -like 'DEUS_ISOLATED_DEPLOY_JSON=*'})
  Need ($statusRows.Count -eq 1) 'ISOLATED_DEPLOY_CHILD_JSON_MISSING'
  $details=$statusRows[0].Substring('DEUS_ISOLATED_DEPLOY_JSON='.Length)|ConvertFrom-Json
  if($ret.ExitCode -ne 0 -or $details.result -cne 'PASS'){
    $message=if($details.terminal_message){[string]$details.terminal_message}else{'UNKNOWN_ISOLATED_DEPLOY_FAILURE'}
    $type=if($details.exception_type){[string]$details.exception_type}else{'RuntimeError'}
    $terminal=Clean ($type+': '+$message) 700
    [void]$lines.Add('TERMINAL_EXCEPTION='+$terminal)
    throw 'ISOLATED_DEPLOY_CHILD_NONZERO'
  }
  AddCheck 'CHILD_EXIT_ZERO' ($ret.ExitCode -eq 0)
  AddCheck 'SCOPED_DEPLOY_SEMANTICS' ($details.schema -ceq 'DEUS_A05_CP314_ISOLATED_DEPLOY_V1' -and
      $details.classification -ceq 'A05_ISOLATED_HOST_DEPENDENCIES_AND_CONTROLLER_IMPORT_SCOPED_PASS' -and
      $details.python_env_global_modified -ceq 'NO' -and $details.no_network -ceq 'YES')
  AddCheck 'CONTROLLER_IMPORT_FINAL' ($details.python_import -ceq 'PASS_FINAL')
  $isolatedTarget=[string]$details.deployment_basename
  $isolatedTreeSha=[string]$details.target_tree_sha256
  $receiptSha=[string]$details.receipt_sha256
  Need ($isolatedTarget -cmatch '\Aa05_cp314_isolated_import_v1_[a-f0-9]{32}\z') 'ISOLATED_TARGET_BASENAME'
  Need ($isolatedTreeSha -cmatch '\A[A-F0-9]{64}\z' -and $receiptSha -cmatch '\A[A-F0-9]{64}\z') 'ISOLATED_RECEIPT_SHA_FORMAT'
  AddCheck 'ISOLATED_RECEIPT_SHA' ((ShaFile (SafePath (Join-Path (Join-Path $hostTarget $isolatedTarget) 'ADMISSION.json'))) -ceq $receiptSha)
  AddCheck 'ISOLATED_TARGET_PRESENT' ([IO.Directory]::Exists((Join-Path (Join-Path $hostTarget $isolatedTarget) 'python_target')))
  $phase='POSTSTATE'
  $post=GitFingerprint
  AddCheck 'GIT_PRE_POST_IDENTICAL' ($post -ceq $pre)
  $dependencyStatus='ISOLATED_CP314_CONTROLLER_IMPORT_PASS'
  $outcome='PASS'
  $class='A05_ISOLATED_CP314_DEPENDENCIES_NATIVE_IMPORT_SCOPED_PASS'
  $phase='NONE';$reason='NONE';$terminal=''
}catch{
  $outcome='FAIL';$class='A05_ISOLATED_CP314_DEPENDENCIES_STOP'
  $msg=[string]$_.Exception.Message
  $reason=($msg -replace '[^A-Za-z0-9_.,:-]','_')
  if($reason.Length -gt 135){$reason=$reason.Substring(0,135)}
  if($terminal -ceq 'NOT_ATTESTED'){
     $terminal='RuntimeError: '+$reason
  }
  if(-not (@($lines|Where-Object {([string]$_) -match '^TERMINAL_EXCEPTION='}).Count -gt 0)){
    [void]$lines.Add('TERMINAL_EXCEPTION='+$terminal)
  }
  if($repo -and $pre -ne 'NOT_ATTESTED'){
     try{$post=GitFingerprint}catch{$post='POSTSTATE_NOT_ATTESTED'}
  }
}
try{
  $lines.Insert(0,'G4_A05_CHILD_EXIT='+$childStatus)
  if($evidenceDir -and [IO.Directory]::Exists($evidenceDir)){
    $sealed=Seal $evidenceDir
    $evidencePath=$sealed.Path;$evidenceSha=$sealed.Sha
  }else{
    $outcome='FAIL';$class='EVIDENCE_ROOT_UNAVAILABLE'
    $phase='EVIDENCE_ROOT';$reason='EVIDENCE_ROOT_UNAVAILABLE'
  }
}catch{
  $outcome='FAIL';$class='EVIDENCE_SEAL_FAILED'
  $phase='EVIDENCE_SEAL';$reason='EVIDENCE_SEAL_FAILED'
}
try{
  Need (-not [string]::IsNullOrEmpty($env:DEUS_RUN_RESULT_PATH)) 'RUNNER_RESULT_PATH_MISSING'
  $record=[ordered]@{
    schema='DEUS_OS_RUNNER_RESULT_V1';gate=$PackageId;result=$outcome
    classification=$class;failure_phase=$phase;error=$reason
    candidate_identity='A05_OFFLINE_ISOLATED_CP314_DEPENDENCIES_IMPORT_ONLY'
    evidence_zip=$evidencePath;evidence_sha256=$evidenceSha
    metrics=('SOURCE_CHECKS='+$checks.Count+
        ';ISOLATED_DEPLOY='+$dependencyStatus+
        ';HOST_TARGET='+$isolatedTarget+
        ';GLOBAL_PYTHON_INSTALL=NO;USB=NO;SSH=NO;RESET=NO;FLASH=NO')
  }
  WriteNew ([IO.Path]::GetFullPath($env:DEUS_RUN_RESULT_PATH)) (($record|ConvertTo-Json -Depth 6)+$nl)
}catch{exit 1}
if($outcome -ceq 'PASS'){exit 0}
exit 1
