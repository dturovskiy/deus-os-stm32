param(
    [Parameter(Mandatory=$true)][string]$ApplicationBin,
    [Parameter(Mandatory=$true)][uint32]$FirmwareVersion,
    [Parameter(Mandatory=$true)][string]$UpdateKeyFile,
    [Parameter(Mandatory=$true)][string]$OutputPath
)

Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'

$ProductId = [uint32]0x534F4544
$FormatVersion = [byte]1
$TargetDeviceId = [uint16]0x0410
$ApplicationOrigin = [uint32]0x08002000
$MaxImageBytes = 53248
$HeaderBytes = 48
$TagBytes = 32

if ($FirmwareVersion -eq 0) {
    throw 'FirmwareVersion must be nonzero'
}

if (-not (Test-Path -LiteralPath $ApplicationBin -PathType Leaf)) {
    throw 'ApplicationBin not found'
}
if (-not (Test-Path -LiteralPath $UpdateKeyFile -PathType Leaf)) {
    throw 'UpdateKeyFile not found'
}

$image = [System.IO.File]::ReadAllBytes($ApplicationBin)
if (($image.Length -lt 8) -or
    ($image.Length -gt $MaxImageBytes) -or
    (($image.Length % 4) -ne 0)) {
    throw ('Application image length {0} violates firmware-update v1 bounds' -f $image.Length)
}

$key = [System.IO.File]::ReadAllBytes($UpdateKeyFile)
if ($key.Length -ne 32) {
    [Array]::Clear($key,0,$key.Length)
    throw 'UpdateKeyFile must contain exactly 32 raw bytes'
}

function Set-UInt16LittleEndian {
    param(
        [Parameter(Mandatory=$true)][byte[]]$Buffer,
        [Parameter(Mandatory=$true)][int]$Offset,
        [Parameter(Mandatory=$true)][uint16]$Value
    )

    $Buffer[$Offset] = [byte]($Value -band 0xFF)
    $Buffer[$Offset + 1] = [byte](($Value -shr 8) -band 0xFF)
}

function Set-UInt32LittleEndian {
    param(
        [Parameter(Mandatory=$true)][byte[]]$Buffer,
        [Parameter(Mandatory=$true)][int]$Offset,
        [Parameter(Mandatory=$true)][uint32]$Value
    )

    $Buffer[$Offset] = [byte]($Value -band 0xFF)
    $Buffer[$Offset + 1] = [byte](($Value -shr 8) -band 0xFF)
    $Buffer[$Offset + 2] = [byte](($Value -shr 16) -band 0xFF)
    $Buffer[$Offset + 3] = [byte](($Value -shr 24) -band 0xFF)
}

try {
    $payloadDigest = [System.Security.Cryptography.SHA256]::HashData($image)
    $header = [byte[]]::new($HeaderBytes)

    Set-UInt32LittleEndian -Buffer $header -Offset 0 -Value $ProductId
    $header[4] = $FormatVersion
    $header[5] = 0
    Set-UInt16LittleEndian -Buffer $header -Offset 6 -Value $TargetDeviceId
    Set-UInt32LittleEndian -Buffer $header -Offset 8 -Value ([uint32]$image.Length)
    Set-UInt32LittleEndian -Buffer $header -Offset 12 -Value $FirmwareVersion
    $payloadDigest.CopyTo($header,16)

    $domain = [System.Text.Encoding]::ASCII.GetBytes('DEUSHDR1')
    $origin = [byte[]]::new(4)
    Set-UInt32LittleEndian -Buffer $origin -Offset 0 -Value $ApplicationOrigin

    $authenticatedBytes = [byte[]]::new(
        $domain.Length + $origin.Length + $header.Length)
    $domain.CopyTo($authenticatedBytes,0)
    $origin.CopyTo($authenticatedBytes,$domain.Length)
    $header.CopyTo(
        $authenticatedBytes,
        $domain.Length + $origin.Length)

    $hmac = [System.Security.Cryptography.HMACSHA256]::new($key)
    try {
        $tag = $hmac.ComputeHash($authenticatedBytes)
    }
    finally {
        $hmac.Dispose()
    }

    if ($tag.Length -ne $TagBytes) {
        throw 'Unexpected HMAC-SHA-256 tag length'
    }

    $package = [byte[]]::new(
        $header.Length + $tag.Length + $image.Length)
    $header.CopyTo($package,0)
    $tag.CopyTo($package,$header.Length)
    $image.CopyTo($package,$header.Length + $tag.Length)

    $outputFullPath = [System.IO.Path]::GetFullPath($OutputPath)
    $outputDirectory = [System.IO.Path]::GetDirectoryName($outputFullPath)
    if (-not [string]::IsNullOrEmpty($outputDirectory)) {
        [System.IO.Directory]::CreateDirectory($outputDirectory) | Out-Null
    }
    [System.IO.File]::WriteAllBytes($outputFullPath,$package)

    $imageSha = [Convert]::ToHexString($payloadDigest)
    $packageSha = (Get-FileHash -LiteralPath $outputFullPath -Algorithm SHA256).Hash.ToUpperInvariant()

    Write-Host ('PACKAGE_OUTCOME=PASS')
    Write-Host ('PACKAGE_FORMAT_VERSION={0}' -f $FormatVersion)
    Write-Host ('PACKAGE_PRODUCT_ID=0x{0:X8}' -f $ProductId)
    Write-Host ('PACKAGE_TARGET_DEVICE_ID=0x{0:X4}' -f $TargetDeviceId)
    Write-Host ('PACKAGE_APPLICATION_ORIGIN=0x{0:X8}' -f $ApplicationOrigin)
    Write-Host ('PACKAGE_FIRMWARE_VERSION={0}' -f $FirmwareVersion)
    Write-Host ('PACKAGE_IMAGE_LENGTH={0}' -f $image.Length)
    Write-Host ('PACKAGE_IMAGE_SHA256={0}' -f $imageSha)
    Write-Host ('PACKAGE_LENGTH={0}' -f $package.Length)
    Write-Host ('PACKAGE_SHA256={0}' -f $packageSha)
}
finally {
    [Array]::Clear($key,0,$key.Length)
}
