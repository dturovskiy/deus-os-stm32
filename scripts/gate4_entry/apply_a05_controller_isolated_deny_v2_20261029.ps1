#requires -Version 7.0
# DEUS OS Gate4 A05: OFFLINE native controller DENY/target import proof only. READ ONLY; no nonce, USB, SSH, reset, Flash or global Python.
Set-StrictMode -Version 3
$ErrorActionPreference='Stop'
$PackageId='stm32_os_gate4_a05_controller_isolated_native_deny_v2_20261029'
$utf8=[Text.UTF8Encoding]::new($false,$true)
$nl=[Environment]::NewLine
$phase='INIT'
$outcome='FAIL'
$class='A05_CONTROLLER_NATIVE_DENIAL_NOT_PROVEN'
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
$isolatedTarget='a05_cp314_isolated_import_v1_ed2d2b32da064f4a9d8ba141847a9f45'
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
function BoundedPy([string]$exe,[string]$script,[int]$ms,[string[]]$extra,[string]$target){
  $psi=[Diagnostics.ProcessStartInfo]::new()
  $psi.FileName=$exe
  $psi.CreateNoWindow=$true
  $psi.UseShellExecute=$false
  $psi.RedirectStandardInput=$true
  $psi.RedirectStandardOutput=$true
  $psi.RedirectStandardError=$true
  $psi.Environment['PYTHONPATH']=$target
  $psi.Environment['PYTHONDONTWRITEBYTECODE']='1'
  $psi.Environment['PYTHONNOUSERSITE']='1'
  $psi.Environment['PIP_NO_INDEX']='1'
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
      schema='DEUS_A05_CONTROLLER_ISOLATED_NATIVE_DENY_EVIDENCE_V1'
      gate=$PackageId;result=$outcome;classification=$class;failure_phase=$phase;error=$reason
      isolated_status=$dependencyStatus;child_exit_code=$childStatus;terminal_exception=$terminal
      isolated_target_basename=$isolatedTarget;target_tree_sha256=$isolatedTreeSha
      receipt_sha256=$receiptSha
      worker_sha256='CAB0E157E204CF119B1FDE0CD5E642BD2D25ACEB9715D2D60B57FB5F97CDCD7B';controller_sha256='05B20216DD7354482C4AB009487DF4CF04E16FDB31482FC2DD67449F183CB2C6';expected_receipt_sha256='75152279350DAFD4816F2FAE12D49A0C218F612BEBA6981B70A9EE9EB6AE4A3A'
      original_host_tree_sha256='31E08CB9B4AEE5917D4E2F9AF49B4D513370D6E4981331397D70590CF4A21BC4'
      original_deploy_evidence_sha256='1287E148FDAF78B7B79EF1E27F56E0576FAD94721B08588F2C6D0D4EE303A532'
      git_pre_sha256=$pre;git_post_sha256=$post;network='NO'
      installation='NONE'
      controller_release='DENIED';nonce_consumed='NO';global_python_changed='NO';device_io='NONE'
      usb='NO';ssh='NO';reset='NO';flash='NO';nonce_ledger='NOT_USED'
      gate4='NOT_AUTHORIZED';time=(Get-Date).ToString('o')
    }
    WriteNew (Join-Path $stage 'outcome.txt') (($typed|ConvertTo-Json -Depth 7)+$nl)
    WriteNew (Join-Path $stage 'native_checks.txt') (($checks.ToArray() -join $nl)+$nl)
    WriteNew (Join-Path $stage 'run.log') ('PACKAGE_ID='+$PackageId+$nl+'RESULT='+$outcome+$nl+
      'CLASSIFICATION='+$class+$nl+'FAILURE_PHASE='+$phase+$nl+'ERROR='+$reason+$nl+
      'CHILD_EXIT='+$childStatus+$nl+'CONTROLLER_STATUS='+$dependencyStatus+$nl+
      'ISOLATED_TARGET='+$isolatedTarget+$nl+'TREE_SHA256='+$isolatedTreeSha+$nl+
      'USB=NO SSH=NO RESET=NO FLASH=NO GLOBAL_INSTALL=NO NONCE=NO AUTHORIZATION=DENIED'+$nl)
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

try{
  $phase='PRESTATE'
  Need ([OperatingSystem]::IsWindows()) 'WINDOWS_ONLY'
  $repo=LocateRepo
  $parent=([IO.Directory]::GetParent($repo)).FullName
  $evidenceDir=SafePath (Join-Path $parent 'Evidence')
  Need ([IO.Directory]::Exists($evidenceDir)) 'ORIGINAL_EVIDENCE_DIR_MISSING'
  $pre=GitFingerprint
  $phase='SOURCE_AND_ACCEPTED_TARGET_PREFLIGHT'
  $workerName='native_a05_isolated_controller_probe_v1.py'
  $workerZip=SafePath (Join-Path $PSScriptRoot $workerName)
  $workerRepo=SafePath (Join-Path (Join-Path $repo 'scripts/gate4_entry') $workerName)
  AddCheck 'WORKER_SOURCE_ZIP_SHA' ((ShaFile $workerZip) -ceq 'CAB0E157E204CF119B1FDE0CD5E642BD2D25ACEB9715D2D60B57FB5F97CDCD7B')
  AddCheck 'WORKER_SOURCE_REPO_SHA' ((ShaFile $workerRepo) -ceq 'CAB0E157E204CF119B1FDE0CD5E642BD2D25ACEB9715D2D60B57FB5F97CDCD7B')
  $sourceDir=SafePath (Join-Path $repo 'scripts/gate4_entry')
  $receipt=SafePath (Join-Path (Join-Path (Join-Path $parent 'HostDependencies') $isolatedTarget) 'ADMISSION.json')
  AddCheck 'ACCEPTED_DEPENDENCY_RECEIPT_SHA' ((ShaFile $receipt) -ceq '75152279350DAFD4816F2FAE12D49A0C218F612BEBA6981B70A9EE9EB6AE4A3A')
  $oldEvidence=SafePath (Join-Path (Join-Path $parent 'Evidence') 'stm32_os_gate4_a05_cp314_isolated_deploy_v2_20261029_20261029_003947_c4a078e0bdab489eb40171cc07fa4399.evidence.zip')
  AddCheck 'ACCEPTED_DEPLOY_EVIDENCE_SHA' ((ShaFile $oldEvidence) -ceq '1287E148FDAF78B7B79EF1E27F56E0576FAD94721B08588F2C6D0D4EE303A532')
  $target=SafePath (Join-Path (Join-Path (Join-Path $parent 'HostDependencies') $isolatedTarget) 'python_target')
  AddCheck 'TARGET_DIRECTORY_EXISTS' ([IO.Directory]::Exists($target))
  $phase='PYTHON_LAUNCHER'
  $cmd=Get-Command -Name 'py.exe' -CommandType Application -ErrorAction SilentlyContinue
  Need ($null -ne $cmd) 'PYTHON_LAUNCHER_MISSING'
  AddCheck 'PYTHON3_LAUNCHER_FOUND' $true
  $phase='NATIVE_ISOLATED_CONTROLLER_DENY_PROBE'
  $ret=BoundedPy $cmd.Source $workerZip 30000 @($repo,$sourceDir) $target
  $childStatus=[string]$ret.ExitCode
  [void]$lines.Add('G4_A05_CHILD_EXIT='+$childStatus)
  foreach($line in ($ret.Stdout -split '\r?\n')){
    if(-not [string]::IsNullOrWhiteSpace($line)){[void]$lines.Add('CHILD_STDOUT='+(Clean $line 1100))}
  }
  foreach($line in ($ret.Stderr -split '\r?\n')){
    if(-not [string]::IsNullOrWhiteSpace($line)){[void]$lines.Add('CHILD_STDERR='+(Clean $line 1100))}
  }
  if($ret.TimedOut){
    $terminal='TimeoutError: controller native child exceeded 30 seconds'
    [void]$lines.Add('TERMINAL_EXCEPTION='+$terminal)
    throw 'CHILD_TIMEOUT'
  }
  $rows=@($ret.Stdout -split '\r?\n' | Where-Object {$_ -like 'DEUS_A05_CONTROLLER_NATIVE_JSON=*'})
  Need ($rows.Count -eq 1) 'ONE_TYPED_CHILD_RECORD_REQUIRED'
  $detail=$rows[0].Substring('DEUS_A05_CONTROLLER_NATIVE_JSON='.Length)|ConvertFrom-Json
  if($ret.ExitCode -ne 0 -or $detail.result -cne 'PASS'){
    $type=if($detail.exception_type){[string]$detail.exception_type}else{'RuntimeError'}
    $msg=if($detail.terminal_message){[string]$detail.terminal_message}else{'CHILD_FAILURE_UNSPECIFIED'}
    $terminal=Clean ($type+': '+$msg) 700
    [void]$lines.Add('TERMINAL_EXCEPTION='+$terminal)
    throw 'CONTROLLER_NATIVE_CHILD_NONZERO'
  }
  AddCheck 'CHILD_EXIT_ZERO' ($ret.ExitCode -eq 0)
  AddCheck 'STRICT_DENY_ONLY' (
       $detail.schema -ceq 'DEUS_A05_ISOLATED_CONTROLLER_NATIVE_PROBE_V1' -and
       $detail.classification -ceq 'A05_ISOLATED_CONTROLLER_NATIVE_DENY_ONLY_SCOPED_PASS' -and
       $detail.case_count -eq 4 -and $detail.authorization -ceq 'DENIED' -and
       $detail.release_admitted -ceq 'NO' -and
       $detail.nonce -ceq 'NOT_USED' -and $detail.executor_started -ceq 'NO' -and
       $detail.device_io -ceq 'NONE' -and $detail.gate4 -ceq 'NOT_AUTHORIZED')
  AddCheck 'ACCEPTED_TARGET_TREE_SHA' ($detail.original_tree_sha256 -ceq '31E08CB9B4AEE5917D4E2F9AF49B4D513370D6E4981331397D70590CF4A21BC4' -and $detail.original_tree_members -eq 174)
  AddCheck 'SOURCE_NATIVE_IMPORT_PASS' ($detail.controller_import -ceq 'PASS')
  $isolatedTreeSha=[string]$detail.original_tree_sha256
  $receiptSha='75152279350DAFD4816F2FAE12D49A0C218F612BEBA6981B70A9EE9EB6AE4A3A'
  $phase='POSTSTATE'
  $post=GitFingerprint
  AddCheck 'GIT_PRE_POST_IDENTICAL' ($post -ceq $pre)
  $dependencyStatus='NATIVE_IMPORT_AND_MISSING_CONFIG_DENIAL_PASS'
  $outcome='PASS'
  $class='A05_CONTROLLER_ISOLATED_NATIVE_DENY_ONLY_SCOPED_PASS'
  $phase='NONE';$reason='NONE';$terminal=''
}catch{
  $outcome='FAIL';$class='A05_CONTROLLER_ISOLATED_NATIVE_STOP'
  $msg=[string]$_.Exception.Message
  $reason=($msg -replace '[^A-Za-z0-9_.,:-]','_')
  if($reason.Length -gt 130){$reason=$reason.Substring(0,130)}
  if($terminal -ceq 'NOT_ATTESTED'){$terminal='RuntimeError: '+$reason}
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
    candidate_identity='A05_CONTROLLER_ISOLATED_NATIVE_DENY_ONLY'
    evidence_zip=$evidencePath;evidence_sha256=$evidenceSha
    metrics=('SOURCE_CHECKS='+$checks.Count+
        ';CONTROLLER_PROBE='+$dependencyStatus+
        ';HOST_TARGET='+$isolatedTarget+
        ';GLOBAL_PYTHON_INSTALL=NO;NONCE=NO;AUTHORIZATION=DENIED;USB=NO;SSH=NO;RESET=NO;FLASH=NO')
  }
  WriteNew ([IO.Path]::GetFullPath($env:DEUS_RUN_RESULT_PATH)) (($record|ConvertTo-Json -Depth 6)+$nl)
}catch{exit 1}
if($outcome -ceq 'PASS'){exit 0}
exit 1
