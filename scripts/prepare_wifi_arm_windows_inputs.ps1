# Transfer exact SHA-locked STM32 WiFi build INPUTS into a new Windows
# candidate OS directory. Does not touch the current OS or any device.
# Must be called by owner in native Windows PowerShell 7 with actual paths.
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$SourceRoot,
    [Parameter(Mandatory=$true)][string]$DestinationRoot
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT -or
    $PSVersionTable.PSVersion.Major -lt 7) {
    throw 'WIFI_SOURCE_COPY_REQUIRES_NATIVE_WINDOWS_POWERSHELL7'
}
$SourceRoot = [IO.Path]::GetFullPath($SourceRoot)
$DestinationRoot = [IO.Path]::GetFullPath($DestinationRoot)
if (-not (Test-Path -LiteralPath $SourceRoot -PathType Container)) {
    throw 'WIFI_SOURCE_ROOT_MISSING'
}
if (Test-Path -LiteralPath $DestinationRoot) {
    throw 'WIFI_DESTINATION_MUST_NOT_EXIST_NO_OVERWRITE'
}
$cmp = [StringComparison]::OrdinalIgnoreCase
$sep = [IO.Path]::DirectorySeparatorChar
if ($SourceRoot.Equals($DestinationRoot,$cmp) -or
    $DestinationRoot.StartsWith($SourceRoot.TrimEnd($sep)+$sep,$cmp) -or
    $SourceRoot.StartsWith($DestinationRoot.TrimEnd($sep)+$sep,$cmp)) {
    throw 'WIFI_SOURCE_DESTINATION_NESTED_OR_IDENTICAL'
}
$parent = Split-Path -Parent $DestinationRoot
if (-not (Test-Path -LiteralPath $parent -PathType Container)) {
    throw 'WIFI_DESTINATION_PARENT_NOT_FOUND'
}
# The offline compiler's default path is sibling Tools in this parent.
$gcc = Join-Path $parent 'Tools/arm-gnu-toolchain-15.3.rel1-mingw-w64-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc.exe'
if (-not (Test-Path -LiteralPath $gcc -PathType Leaf)) {
    throw ('WIFI_DESTINATION_REQUIRES_SIBLING_TOOLS: ' + $gcc)
}
# Never write through a Windows junction/symlink parent.
$walk = [IO.DirectoryInfo]::new($parent)
while ($null -ne $walk) {
    if ($walk.Exists -and
        ($walk.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
        throw ('WIFI_DESTINATION_REPARSE_PARENT: ' + $walk.FullName)
    }
    $walk = $walk.Parent
}
$manifestRel = 'scripts/wifi_arm_candidate_build_inputs_20261110.sha256'
$manifest = Join-Path $SourceRoot ($manifestRel.Replace('/','\'))
if (-not (Test-Path -LiteralPath $manifest -PathType Leaf)) {
    throw 'WIFI_SOURCE_MANIFEST_MISSING'
}
$manifestSha = (Get-FileHash -LiteralPath $manifest -Algorithm SHA256).Hash.ToLowerInvariant()
if ($manifestSha -cne 'ae8c802794121c8e35bf644c659a295081d35d1fd3f2cfb5ca8e162123b12e43') {
    throw ('WIFI_SOURCE_MANIFEST_WRONG_SHA ' + $manifestSha)
}
$inputs = [Collections.Generic.List[object]]::new()
$unique = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($line in [IO.File]::ReadAllLines($manifest)) {
    if ($line -cnotmatch '^([0-9a-f]{64})  ((src|include|nodes|linker|scripts)/[A-Za-z0-9_./-]+)$') {
        throw ('WIFI_SOURCE_MANIFEST_INVALID ' + $line)
    }
    $sha = $Matches[1]; $relative = $Matches[2]
    if ($relative.Contains('..') -or $relative.Contains('//') -or
        -not $unique.Add($relative)) {
        throw ('WIFI_SOURCE_PATH_UNSAFE ' + $relative)
    }
    $path = Join-Path $SourceRoot ($relative.Replace('/','\'))
    if (-not (Test-Path -LiteralPath $path -PathType Leaf) -or
        ((Get-Item -LiteralPath $path -Force).Attributes -band
          [IO.FileAttributes]::ReparsePoint) -ne 0) {
        throw ('WIFI_SOURCE_FILE_MISSING_OR_LINK ' + $relative)
    }
    if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -cne $sha) {
        throw ('WIFI_SOURCE_FILE_SHA_DRIFT ' + $relative)
    }
    $inputs.Add([pscustomobject]@{ Relative=$relative; Sha=$sha; Path=$path })
}
if ($inputs.Count -ne 84) { throw 'WIFI_SOURCE_FILE_COUNT_NOT_84' }
# Mutation boundary: create exactly one new isolated folder.
New-Item -ItemType Directory -Path $DestinationRoot -ErrorAction Stop | Out-Null
foreach ($x in $inputs) {
    $dest = Join-Path $DestinationRoot ($x.Relative.Replace('/','\'))
    $dir = Split-Path -Parent $dest
    if (-not (Test-Path -LiteralPath $dir)) {
        New-Item -ItemType Directory -Path $dir -Force -ErrorAction Stop | Out-Null
    }
    Copy-Item -LiteralPath $x.Path -Destination $dest -ErrorAction Stop
    if ((Get-FileHash -LiteralPath $dest -Algorithm SHA256).Hash.ToLowerInvariant() -cne $x.Sha) {
        throw ('WIFI_COPY_VERIFY_FAILED ' + $x.Relative)
    }
}
$destManifest = Join-Path $DestinationRoot ($manifestRel.Replace('/','\'))
Copy-Item -LiteralPath $manifest -Destination $destManifest -ErrorAction Stop
if ((Get-FileHash -LiteralPath $destManifest -Algorithm SHA256).Hash.ToLowerInvariant() -cne $manifestSha) {
    throw 'WIFI_COPIED_MANIFEST_SHA_FAILED'
}
Write-Host 'WIFI_WINDOWS_ISOLATED_SOURCE_COPY=PASS EXACT_SHA_84_84'
Write-Host ('WIFI_WINDOWS_SOURCE_MANIFEST_SHA256=' + $manifestSha)
Write-Host ('WIFI_WINDOWS_CANDIDATE_ROOT=' + $DestinationRoot)
Write-Host 'WIFI_NO_OVERWRITE_EXISTING_OS_NO_HARDWARE=YES'
Write-Host 'NEXT=pwsh -NoProfile -File .\scripts\build_wifi_arm_offline_candidate.ps1 -PreflightOnly'
