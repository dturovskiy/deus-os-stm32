#requires -Version 7.0
# Gate4 RA-03 isolated Windows nonce reservation primitive.
# SOURCE ONLY: No real owner approval, release controller, device I/O or Go.
Set-StrictMode -Version 3

function Assert-G4Ledger([bool]$Condition,[string]$Code) {
    if (-not $Condition) { throw $Code }
}
function Assert-G4SafeLedgerPath([string]$Path) {
    Assert-G4Ledger (-not [string]::IsNullOrWhiteSpace($Path)) 'RA03_EMPTY_PATH'
    Assert-G4Ledger ([IO.Path]::IsPathFullyQualified($Path)) 'RA03_PATH_NOT_FULLY_QUALIFIED'
    Assert-G4Ledger (-not $Path.StartsWith('\\') -and
        $Path -notmatch '(^|[\\/])\.\.([\\/]|$)' -and
        $Path -notmatch '(?i)(^|[\\/])[^\\/]*~[0-9]+([\\/]|$)' -and
        $Path -notmatch '[. ]([\\/]|$)' -and
        $Path.Substring(2).IndexOf(':') -lt 0) 'RA03_AMBIGUOUS_PATH'
    $full=[IO.Path]::GetFullPath($Path)
    Assert-G4Ledger (-not $full.StartsWith('\\')) 'RA03_UNC_REJECTED'
    $walk=[IO.DirectoryInfo]::new($full)
    while($null -ne $walk) {
        if($walk.Exists) {
            Assert-G4Ledger (($walk.Attributes -band [IO.FileAttributes]::ReparsePoint) -eq 0) 'RA03_REPARSE_ANCESTOR'
        } elseif([IO.File]::Exists($walk.FullName)) {
            throw 'RA03_ANCESTOR_IS_FILE'
        }
        $walk=$walk.Parent
    }
    return $full
}
function Assert-G4ProtectedLedgerDirectory([string]$Path) {
    Assert-G4Ledger ([Environment]::OSVersion.Platform -eq [PlatformID]::Win32NT) 'RA03_WINDOWS_REQUIRED'
    $folder=Assert-G4SafeLedgerPath $Path
    Assert-G4Ledger ([IO.Directory]::Exists($folder)) 'RA03_LEDGER_MUST_PREEXIST'
    Assert-G4Ledger (-not [IO.File]::Exists((Join-Path $folder '.untrusted'))) 'RA03_UNTRUSTED_MARKER'
    $acl=Get-Acl -LiteralPath $folder -ErrorAction Stop
    Assert-G4Ledger $acl.AreAccessRulesProtected 'RA03_ACL_INHERITANCE_NOT_DISABLED'
    $owner=[Security.Principal.WindowsIdentity]::GetCurrent().User.Value
    $allowed=@($owner,'S-1-5-18','S-1-5-32-544')
    $access=$acl.GetAccessRules($true,$true,[Security.Principal.SecurityIdentifier])
    $ownerAllowed=$false
    foreach($ace in $access) {
        $sid=[string]$ace.IdentityReference.Value
        Assert-G4Ledger (-not $ace.IsInherited -and
            $sid -in $allowed -and
            $ace.AccessControlType -eq [Security.AccessControl.AccessControlType]::Allow) 'RA03_LEDGER_ACL_UNTRUSTED'
        if($sid -eq $owner -and
            (($ace.FileSystemRights -band [Security.AccessControl.FileSystemRights]::WriteData) -ne 0)) {
            $ownerAllowed=$true
        }
    }
    Assert-G4Ledger $ownerAllowed 'RA03_OWNER_WRITE_MISSING'
    return $folder
}
function Write-G4ConsumedNonceSourceOnly {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)][string]$LedgerDirectory,
        [Parameter(Mandatory)][string]$Nonce,
        [Parameter(Mandatory)][string]$StatementSha256,
        [Parameter(Mandatory)][string]$OperatorZipSha256,
        [Parameter()][ValidateSet('NONE','BEFORE_CREATE','AFTER_CREATE','AFTER_WRITE','AFTER_FLUSH')]
        [string]$Fault='NONE'
    )
    # This is intentionally NOT a Gate4 release permit. A future independent
    # protected controller must call an adapted and separately native-accepted
    # version before it can even consider launching a physical child.
    Assert-G4Ledger ($Nonce -cmatch '\A[a-f0-9]{32}\z' -and
        $Nonce -cne ('0'*32)) 'RA03_NONCE_INVALID'
    foreach($digest in @($StatementSha256,$OperatorZipSha256)) {
        Assert-G4Ledger ($digest -cmatch '\A[A-F0-9]{64}\z') 'RA03_SHA_INVALID'
    }
    $dir=Assert-G4ProtectedLedgerDirectory $LedgerDirectory
    $leaf='used-'+$Nonce+'.json'
    $target=Join-Path $dir $leaf
    Assert-G4Ledger (-not [IO.File]::Exists($target) -and -not
        [IO.Directory]::Exists($target)) 'RA03_NONCE_ALREADY_USED'
    if($Fault -ceq 'BEFORE_CREATE'){throw 'RA03_INJECT_BEFORE_CREATE'}
    # CREATE_NEW is atomic across independent Windows processes. Never use
    # overwrite/replace/delete, even when record creation has not completed.
    $stream=$null
    try {
        $stream=[IO.FileStream]::new($target,[IO.FileMode]::CreateNew,
            [IO.FileAccess]::Write,[IO.FileShare]::None,4096,
            [IO.FileOptions]::WriteThrough)
        if($Fault -ceq 'AFTER_CREATE'){throw 'RA03_INJECT_AFTER_CREATE'}
        $record=[ordered]@{
            schema='DEUS_OS_GATE4_RA03_NONCE_RESERVATION_V1'
            nonce=$Nonce
            statement_sha256=$StatementSha256
            operator_zip_sha256=$OperatorZipSha256
            state='CONSUMED_NO_AUTOMATIC_RETRY'
            authorization='DENIED_SOURCE_ONLY'
            created_utc=[DateTime]::UtcNow.ToString('o')
        }
        $utf8=[Text.UTF8Encoding]::new($false,$true)
        $bytes=$utf8.GetBytes(($record|ConvertTo-Json -Compress -Depth 4)+[char]10)
        $stream.Write($bytes,0,$bytes.Length)
        if($Fault -ceq 'AFTER_WRITE'){throw 'RA03_INJECT_AFTER_WRITE'}
        $stream.Flush($true)
        if($Fault -ceq 'AFTER_FLUSH'){throw 'RA03_INJECT_AFTER_FLUSH'}
    }
    finally {
        if($null -ne $stream){$stream.Dispose()}
    }
    Assert-G4Ledger ([IO.File]::Exists($target)) 'RA03_CONSUMED_RECORD_MISSING'
    $raw=[IO.File]::ReadAllBytes($target)
    Assert-G4Ledger ($raw.Length -gt 0 -and $raw.Length -lt 1500) 'RA03_RECORD_SIZE'
    $digest=[Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($raw))
    return [pscustomobject]@{
        classification='RA03_DURABLE_SOURCE_ONLY_NONCE_CONSUMED'
        record_sha256=$digest
        nonce=$Nonce
        authorization='DENIED'
        launch_authorized=$false
        device_io='NONE'
    }
}
Export-ModuleMember -Function Write-G4ConsumedNonceSourceOnly,Assert-G4ProtectedLedgerDirectory
