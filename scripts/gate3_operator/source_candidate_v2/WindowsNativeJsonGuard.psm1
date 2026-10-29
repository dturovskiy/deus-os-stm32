#requires -Version 7.0
# Source-only native Windows raw-JSON adjudication. NO USB/SSH/device calls.
Set-StrictMode -Version 3
function Need-G3([bool]$Condition,[string]$Code) {
    if(-not $Condition){ throw $Code }
}
function Check-G3Keys([object]$Obj,[string[]]$Keys,[bool]$Exact=$false) {
    Need-G3 ($Obj -is [Collections.IDictionary]) 'JSON_NOT_OBJECT'
    foreach($k in $Keys) { Need-G3 (@($Obj.Keys) -ccontains $k) ('JSON_MISSING_'+$k) }
    if($Exact) { Need-G3 ($Obj.Count -eq $Keys.Count) 'JSON_EXTRA_FIELDS' }
}
function Check-G3Int([object]$Value,[int]$Min,[int]$Max) {
    Need-G3 ($Value -is [int] -or $Value -is [long]) 'JSON_INTEGER_TYPE'
    Need-G3 ($Value -ge $Min -and $Value -le $Max) 'JSON_INTEGER_RANGE'
}
function Check-G3Unique([Text.Json.JsonElement]$Element) {
    if($Element.ValueKind -eq [Text.Json.JsonValueKind]::Object) {
        $names=[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach($member in $Element.EnumerateObject()) {
            Need-G3 ($names.Add($member.Name)) 'JSON_DUPLICATE_PROPERTY'
            Check-G3Unique $member.Value
        }
    } elseif($Element.ValueKind -eq [Text.Json.JsonValueKind]::Array) {
        foreach($v in $Element.EnumerateArray()){Check-G3Unique $v}
    }
}
function Check-G3Tokens([object]$Process,[string[]]$Keys) {
    Check-G3Keys $Process @('exit','timeout','stderr_empty','tokens')
    Check-G3Int $Process.exit 0 0
    Need-G3 ($Process.timeout -is [bool] -and -not $Process.timeout -and
             $Process.stderr_empty -is [bool] -and $Process.stderr_empty) 'REMOTE_HELPER_NOT_CLEAN'
    Need-G3 ($Process.tokens -is [array] -and $Process.tokens.Count -eq $Keys.Count) 'TOKEN_COUNT'
    $tokens=[Collections.Generic.Dictionary[string,string]]::new([StringComparer]::Ordinal)
    foreach($line in $Process.tokens) {
        Need-G3 ($line -is [string] -and $line.Length -le 160 -and
                 $line -cmatch '^([A-Z_]+)=([^\r\n]{1,100})$') 'TOKEN_FORMAT'
        $k=$Matches[1];$v=$Matches[2]
        Need-G3 (-not $tokens.ContainsKey($k)) 'TOKEN_DUPLICATE'
        $tokens.Add($k,$v)
    }
    foreach($k in $Keys){Need-G3 ($tokens.ContainsKey($k)) ('TOKEN_MISSING_'+$k)}
    return $tokens
}
function Check-G3ExactReceipt([object]$R) {
    Check-G3Keys $R @('schema','result','classification','host','physical_port',
        'firmware_flash','firmware_erase','protocol_run_as_root',
        'physical_identity_authenticated','gate3_full','gate4','reset_attempts',
        'info_attempts','before','after','observed_detach_ordinal',
        'observed_attach_ordinal','usb_events','enter','info','permission')
    foreach($p in @(
        @('schema','DEUS_G3_000D_ONE_RESET_INFO_PHYSICAL_V1'),
        @('result','PASS'),
        @('classification','SCOPED_PHYSICAL_ONE_ENTER_ONE_INFO_STOP_NO_FLASH'),
        @('host','UBUNTU_MACMINI'),@('physical_port','usb:001:8'),
        @('firmware_flash','NO'),@('firmware_erase','NO'),
        @('protocol_run_as_root','NO'),@('physical_identity_authenticated','NO'),
        @('gate3_full','NOT_ACCEPTED'),@('gate4','NOT_AUTHORIZED')
    )) {
        Need-G3 ($R[$p[0]] -is [string] -and $R[$p[0]] -ceq $p[1]) ('SCOPE_'+$p[0])
    }
    Check-G3Int $R.reset_attempts 1 1
    Check-G3Int $R.info_attempts 1 1
    Check-G3Keys $R.before @('pid','port','devnum')
    Check-G3Keys $R.after @('pid','port','devnum')
    Need-G3 ($R.before.pid -ceq '000c' -and $R.after.pid -ceq '000d' -and
             $R.before.port -ceq 'usb:001:8' -and $R.after.port -ceq 'usb:001:8') 'USB_TOPOLOGY_DRIFT'
    Check-G3Int $R.before.devnum 1 127
    Check-G3Int $R.after.devnum 1 127
    Need-G3 ($R.before.devnum -ne $R.after.devnum) 'USB_DEVNUM_REUSED'
    Check-G3Int $R.observed_detach_ordinal 1 1
    Check-G3Int $R.observed_attach_ordinal 2 2
    Need-G3 ($R.usb_events -is [array] -and $R.usb_events.Count -eq 2) 'USB_EVENT_COUNT'
    $previous=-1.0
    for($i=0;$i -lt 2;$i++){
        $e=$R.usb_events[$i]
        Check-G3Keys $e @('event','ordinal','monotonic')
        $expected=$(if($i -eq 0){'none'}else{'000d:'+ $R.after.devnum})
        Need-G3 ($e.event -is [string] -and $e.event -ceq $expected) 'USB_EVENT_VALUE'
        Check-G3Int $e.ordinal ($i+1) ($i+1)
        Need-G3 ($e.monotonic -is [double] -or $e.monotonic -is [decimal] -or
                 $e.monotonic -is [int] -or $e.monotonic -is [long]) 'USB_EVENT_TIME_TYPE'
        $clock=[double]$e.monotonic
        Need-G3 ([double]::IsFinite($clock) -and $clock -ge 0 -and
                 $clock -gt $previous) 'USB_EVENT_TIME_NOT_ORDERED'
        $previous=$clock
    }
    Check-G3Keys $R.permission @('status','node')
    Need-G3 ($R.permission.status -cin @('ALREADY_OWNER_0600','EPHEMERAL_NODE_OWNER_0600_PASS') -and
             $R.permission.node -ceq ('001/'+([int]$R.after.devnum).ToString('D3'))) 'DEVICE_NODE_OWNERSHIP'
    $enter=Check-G3Tokens $R.enter @('RUNTIME_NEGOTIATED','RUNTIME_TREE',
        'RUNTIME_CAPABILITIES','ENTER_BOOTLOADER_REQUEST_BOUNDARY','ENTER_RESPONSE')
    Need-G3 ($enter['RUNTIME_NEGOTIATED'] -ceq 'PASS' -and
             $enter['RUNTIME_TREE'] -ceq 'ccbd50200ec7f0ec53da6b1048d7377271dfb7bd' -and
             $enter['ENTER_BOOTLOADER_REQUEST_BOUNDARY'] -ceq 'ARMED_SINGLE_SEND' -and
             $enter['ENTER_RESPONSE'] -ceq 'OK_RESETTING') 'ENTER_NOT_PINNED'
    $cap=$enter['RUNTIME_CAPABILITIES']
    Need-G3 ($cap -cmatch '^0x[0-9A-F]{8}$') 'CAPABILITY_FORMAT'
    Need-G3 (([Convert]::ToUInt32($cap.Substring(2),16) -band [uint32]64) -ne 0) 'FW_CAPABILITY_ABSENT'
    $info=Check-G3Tokens $R.info @('INFO_STOP','INFO_OBSERVED',
        'INFO_VERSION_FLOOR','INFO_COMMITTED','PHYSICAL_IDENTITY_AUTHENTICATED',
        'DESTRUCTIVE_OPERATION_AUTHORIZED')
    Need-G3 ($info['INFO_STOP'] -ceq 'InfoObservedGateClosed' -and
             $info['INFO_OBSERVED'] -ceq 'True' -and
             $info['PHYSICAL_IDENTITY_AUTHENTICATED'] -ceq 'False' -and
             $info['DESTRUCTIVE_OPERATION_AUTHORIZED'] -ceq 'False') 'INFO_SCOPE_INVALID'
    Need-G3 ($info['INFO_VERSION_FLOOR'] -cmatch '^[0-9]+$' -and
             $info['INFO_COMMITTED'] -cmatch '^[0-9]+$') 'INFO_VERSION_FORMAT'
    $floor=[uint32]::Parse($info['INFO_VERSION_FLOOR'])
    $committed=[uint32]::Parse($info['INFO_COMMITTED'])
    Need-G3 ($committed -in @(5,6) -and $committed -le $floor -and $floor -lt 7) 'INFO_FLOOR_POLICY'
}
function Get-G3NativeRemoteDecision {
    [CmdletBinding()]
    param([Parameter(Mandatory)][bool]$Started,
          [Parameter(Mandatory)][string]$ExitText,
          [Parameter(Mandatory)][string]$RawPath)
    if(-not $Started){return 'STOP_BEFORE_RESET'}
    if($ExitText -cnotmatch '^-?[0-9]{1,5}$'){return 'RESET_POSSIBLY_ATTEMPTED'}
    try {
        $rc=[int]::Parse($ExitText)
        Need-G3 ($rc -ge -65535 -and $rc -le 65535) 'REMOTE_EXIT_RANGE'
        Need-G3 ([IO.File]::Exists($RawPath)) 'REMOTE_JSON_MISSING'
        $attrs=[IO.File]::GetAttributes($RawPath)
        Need-G3 (($attrs -band [IO.FileAttributes]::ReparsePoint) -eq 0) 'REMOTE_JSON_SYMLINK'
        $fi=[IO.FileInfo]::new($RawPath)
        Need-G3 ($fi.Length -gt 0 -and $fi.Length -le 65536) 'REMOTE_JSON_SIZE'
        $raw=[Text.UTF8Encoding]::new($false,$true).GetString([IO.File]::ReadAllBytes($RawPath))
        $doc=[Text.Json.JsonDocument]::Parse($raw)
        try{Check-G3Unique $doc.RootElement}finally{$doc.Dispose()}
        $obj=ConvertFrom-Json -InputObject $raw -AsHashtable -Depth 32 -ErrorAction Stop
        Check-G3Keys $obj @('reset_attempts','info_attempts')
        Check-G3Int $obj.reset_attempts 0 1
        Check-G3Int $obj.info_attempts 0 1
        Need-G3 ($obj.info_attempts -le $obj.reset_attempts) 'INFO_COUNT_WITHOUT_ENTER'
        if($obj.reset_attempts -eq 0){return 'RESET_POSSIBLY_ATTEMPTED'}
        if($rc -ne 0){return 'STOP_AFTER_ONE_ENTER_NO_RETRY'}
        Check-G3ExactReceipt $obj
        return 'SCOPED_PHYSICAL_ONE_ENTER_ONE_INFO_STOP_NO_FLASH'
    } catch {return 'RESET_POSSIBLY_ATTEMPTED'}
}
Export-ModuleMember -Function Get-G3NativeRemoteDecision
