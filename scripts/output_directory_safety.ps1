# Deus OS generated-output reset policy (RDC-04).
# Dot-source this file. This function performs validation AND the reset.
function Reset-DeusGeneratedOutputDirectory {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true)][string]$ProjectRoot,
        [Parameter(Mandatory=$true)][AllowEmptyString()][string]$OutputDir,
        [Parameter(Mandatory=$true)][ValidatePattern('^[A-Za-z][A-Za-z0-9_]{2,79}$')][string]$OwnerId
    )

    Set-StrictMode -Version Latest

    if ([string]::IsNullOrWhiteSpace($ProjectRoot) -or
        [string]::IsNullOrWhiteSpace($OutputDir)) {
        throw 'OUTPUT_SAFETY_EMPTY_PATH'
    }
    # Windows device namespaces, NTFS ADS, 8.3 aliases and navigation components
    # are intentionally not supported for caller-controlled destructive operations.
    foreach ($p in @($ProjectRoot,$OutputDir)) {
        if ($p.StartsWith('\\') -or
            $p -match '(^|[\\/])\.\.([\\/]|$)' -or
            $p -match '(?i)(^|[\\/])[^\\/]*~[0-9]+([\\/]|$)' -or
            $p -match '(^|[\\/])[^\\/]+[. ]([\\/]|$)' -or
            $p -match '^[A-Za-z]:[^\\/]') {
            throw 'OUTPUT_SAFETY_AMBIGUOUS_PATH'
        }
        if ($p.Contains(':') -and
            ($p -notmatch '^[A-Za-z]:[\\/]' -or $p.Substring(2).Contains(':'))) {
            throw 'OUTPUT_SAFETY_AMBIGUOUS_PATH'
        }
    }

    $cmp = [System.StringComparison]::OrdinalIgnoreCase
    $sep = [System.IO.Path]::DirectorySeparatorChar
    $repo = [System.IO.Path]::TrimEndingDirectorySeparator([System.IO.Path]::GetFullPath($ProjectRoot))
    if (-not [System.IO.Path]::IsPathFullyQualified($ProjectRoot) -or
        -not [System.IO.Directory]::Exists($repo)) {
        throw 'OUTPUT_SAFETY_PROJECT_ROOT_INVALID'
    }
    $resolved = [System.IO.Path]::TrimEndingDirectorySeparator(
        [System.IO.Path]::GetFullPath($OutputDir,$repo))
    $buildRoot = [System.IO.Path]::TrimEndingDirectorySeparator(
        [System.IO.Path]::GetFullPath('build',$repo))
    $tempRoot = [System.IO.Path]::TrimEndingDirectorySeparator(
        [System.IO.Path]::GetFullPath([System.IO.Path]::GetTempPath()))
    $homeRoot = [System.IO.Path]::TrimEndingDirectorySeparator(
        [System.IO.Path]::GetFullPath([Environment]::GetFolderPath('UserProfile')))
    $pathRoot = [System.IO.Path]::TrimEndingDirectorySeparator([System.IO.Path]::GetPathRoot($resolved))

    # Exact / descendant comparisons use normalized paths PLUS separator boundaries.
    $isBuildChild = $resolved.StartsWith(($buildRoot + $sep),$cmp)
    $isRepoChild = $resolved.StartsWith(($repo + $sep),$cmp)
    $isProjectAncestor = $repo.Equals($resolved,$cmp) -or $repo.StartsWith(($resolved + $sep),$cmp)
    $isHome = $resolved.Equals($homeRoot,$cmp)
    $isTemp = $resolved.Equals($tempRoot,$cmp)
    if ($resolved.Equals($pathRoot,$cmp) -or $isProjectAncestor -or $isHome -or $isTemp -or
        ($isRepoChild -and -not $isBuildChild)) {
        throw 'OUTPUT_SAFETY_PROTECTED_PATH'
    }

    # Never inspect through a reparse ancestor. Enumerate actual existing path
    # components, including parents of the project root, temp root and output.
    function Assert-NoReparseAncestors([string]$Target) {
        $walk = [System.IO.DirectoryInfo]::new($Target)
        while ($null -ne $walk) {
            if ($walk.Exists) {
                if (($walk.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
                    throw 'OUTPUT_SAFETY_REPARSE_ANCESTOR'
                }
            }
            elseif ([System.IO.File]::Exists($walk.FullName)) {
                throw 'OUTPUT_SAFETY_PATH_IS_FILE'
            }
            $walk = $walk.Parent
        }
    }
    Assert-NoReparseAncestors $repo
    Assert-NoReparseAncestors $resolved
    Assert-NoReparseAncestors $tempRoot

    $markerName = '.deus-generated-output.json'
    $markerFile = [System.IO.Path]::Combine($resolved,$markerName)
    $exists = [System.IO.Directory]::Exists($resolved)
    if ([System.IO.File]::Exists($resolved)) {
        throw 'OUTPUT_SAFETY_PATH_IS_FILE'
    }

    # Existing markers, wherever located, MUST be exact. For external outputs
    # the marker is mandatory; for build outputs an unmarked legacy dir is allowed.
    if ($exists -and [System.IO.Directory]::Exists($markerFile)) {
        throw 'OUTPUT_SAFETY_MARKER_MALFORMED'
    }
    $hasMarker = $exists -and [System.IO.File]::Exists($markerFile)
    if ($exists -and -not $isBuildChild -and -not $hasMarker) {
        throw 'OUTPUT_SAFETY_EXTERNAL_UNOWNED'
    }
    if (-not $exists -and -not $isBuildChild) {
        # A new temp output must be a unique IMMEDIATE child, using a UUID in
        # its name. No creation of arbitrary paths outside <repo>/build.
        $parent = [System.IO.Path]::GetDirectoryName($resolved)
        $leaf = [System.IO.Path]::GetFileName($resolved)
        $uuid = '(?i)([0-9a-f]{32}|[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})'
        if (-not ($parent.Equals($tempRoot,$cmp) -and $leaf -match $uuid)) {
            throw 'OUTPUT_SAFETY_EXTERNAL_UNOWNED'
        }
    }
    if ($hasMarker) {
        $markerAttributes = [System.IO.File]::GetAttributes($markerFile)
        if (($markerAttributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw 'OUTPUT_SAFETY_REPARSE_MARKER'
        }
        $marker = $null
        try {
            $marker = [System.IO.File]::ReadAllText($markerFile) | ConvertFrom-Json -ErrorAction Stop
        }
        catch { throw 'OUTPUT_SAFETY_MARKER_MALFORMED' }
        if ($null -eq $marker -or
            $marker.schema -cne 'DEUS_GENERATED_OUTPUT_V1' -or
            $marker.version -ne 1 -or
            $marker.generated -cne $true -or
            $marker.owner_id -cne $OwnerId -or
            -not [string]::Equals([string]$marker.project_root,$repo,$cmp) -or
            -not [string]::Equals([string]$marker.output_path,$resolved,$cmp)) {
            throw 'OUTPUT_SAFETY_MARKER_MISMATCH'
        }
    }

    # Before deleting, examine every existing descendant without recursively
    # traversing a link. A link anywhere in the tree blocks the entire reset.
    if ($exists) {
        $pending = [System.Collections.Generic.Stack[string]]::new()
        $pending.Push($resolved)
        while ($pending.Count -gt 0) {
            $dir = $pending.Pop()
            foreach ($item in @(Get-ChildItem -LiteralPath $dir -Force -ErrorAction Stop)) {
                if (($item.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0) {
                    throw 'OUTPUT_SAFETY_REPARSE_DESCENDANT'
                }
                if ($item.PSIsContainer) { $pending.Push($item.FullName) }
            }
        }
    }

    # Recheck at the mutation boundary. There is intentionally only ONE
    # repository-owned recursive output deletion site.
    Assert-NoReparseAncestors $resolved
    if ($exists) {
        Remove-Item -LiteralPath $resolved -Recurse -Force -ErrorAction Stop
    }
    [System.IO.Directory]::CreateDirectory($resolved) | Out-Null
    $newMarker = [ordered]@{
        schema = 'DEUS_GENERATED_OUTPUT_V1'
        version = 1
        generated = $true
        owner_id = $OwnerId
        project_root = $repo
        output_path = $resolved
    }
    $json = $newMarker | ConvertTo-Json -Depth 3
    [System.IO.File]::WriteAllText($markerFile,$json,[System.Text.UTF8Encoding]::new($false))
    return $resolved
}
