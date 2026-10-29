<#
.SYNOPSIS
    Installs the physx engine build output on Windows: pulls the Windows subset from the
    WSL build output, copies it to %LOCALAPPDATA%\godot-physx, installs the export
    templates and (with -SetEnv) persists the user-level env vars.
.PARAMETER SetEnv
    Persist GODOT_PHYSICS_EXTENDED and GODOT_PHYSICS_EXTENDED_NUPKGS at user level.
.PARAMETER Source
    WSL build output as a Windows/UNC path (e.g. \\wsl.localhost\Ubuntu-22.04\home\you\godot-physx-out).
    Default: resolved by asking WSL for the Windows path of $GODOT_PHYSX_HOME or ~/godot-physx-out.
.PARAMETER Distro
    WSL distro to ask when resolving -Source. Default: WSL's default distro.
.PARAMETER PhysxHome
    (Also accepted as -Home.)
    Destination folder on the Windows filesystem. Default: %LOCALAPPDATA%\godot-physx.
#>
[CmdletBinding()]
param(
    [switch]$SetEnv,
    [string]$Source,
    [string]$Distro,
    [Alias('Home')]
    [string]$PhysxHome = $(Join-Path $env:LOCALAPPDATA 'godot-physx')
)

$ErrorActionPreference = 'Stop'

# Resolve the WSL build output as a Windows/UNC path.
if (-not $Source) {
    $wslArgs = @()
    if ($Distro) { $wslArgs = @('-d', $Distro) }
    $Source = (& wsl.exe @wslArgs -e bash -c 'wslpath -w "${GODOT_PHYSX_HOME:-$HOME/godot-physx-out}"') -join '' -replace "`0", ''
    $Source = $Source.Trim()
}
if (-not $Source) { throw "Could not resolve the WSL build output path. Run physx-build/build.sh in WSL first, or pass -Source <\\wsl.localhost\... path>." }
$manifestPath = Join-Path $Source 'manifest.txt'
if (-not (Test-Path $manifestPath)) { throw "manifest.txt not found in '$Source'. Run physx-build/build.sh in WSL first." }
$manifest = @{}
Get-Content $manifestPath | ForEach-Object { if ($_ -match '^([^=]+)=(.*)$') { $manifest[$Matches[1]] = $Matches[2] } }

$folder = "$($manifest['version']).$($manifest['status']).mono"
Write-Host "Engine version folder: $folder (sha $($manifest['sha']), dirty=$($manifest['dirty']), profile=$($manifest['profile']))"

# Copy the Windows subset from WSL to the Windows filesystem.
Write-Host "Copying Windows build output from $Source to $PhysxHome"
New-Item -ItemType Directory -Force -Path $PhysxHome | Out-Null
foreach ($sub in 'windows', 'nupkgs') {
    $srcSub = Join-Path $Source $sub
    if (-not (Test-Path $srcSub)) { continue }
    $dstSub = Join-Path $PhysxHome $sub
    if (Test-Path $dstSub) { Remove-Item $dstSub -Recurse -Force }
    Copy-Item -Path $srcSub -Destination $dstSub -Recurse -Force
}
Copy-Item $manifestPath (Join-Path $PhysxHome 'manifest.txt') -Force

$winDir = Join-Path $PhysxHome 'windows'
$nupkgs = Join-Path $PhysxHome 'nupkgs'

# Export templates
$templateDir = Join-Path $env:APPDATA "Godot\export_templates\$folder"
New-Item -ItemType Directory -Force -Path $templateDir | Out-Null
foreach ($kind in 'debug', 'release') {
    $src = Join-Path $winDir "godot.windows.template_$kind.x86_64.mono.exe"
    if (Test-Path $src) {
        Copy-Item $src (Join-Path $templateDir "windows_${kind}_x86_64.exe") -Force
        Write-Host "Installed windows_${kind}_x86_64.exe"
    } else {
        Write-Warning "Missing $src (skipped)"
    }
}

# NuGet packages
$pkgCount = @(Get-ChildItem $nupkgs -Filter *.nupkg -ErrorAction SilentlyContinue).Count
if ($pkgCount -eq 0) { throw "No .nupkg files in '$nupkgs'." }
Write-Host "Found $pkgCount nupkgs in $nupkgs"

$editor = Join-Path $winDir 'godot.windows.editor.x86_64.mono.exe'
$hasEditor = Test-Path $editor
if (-not $hasEditor) { Write-Warning "Editor exe not found at $editor (build windows-editor to get it)" }

if ($SetEnv) {
    if ($hasEditor) {
        [Environment]::SetEnvironmentVariable('GODOT_PHYSICS_EXTENDED', $editor, 'User')
    } else {
        Write-Warning "GODOT_PHYSICS_EXTENDED left unchanged: there is no editor exe to point it at."
    }
    [Environment]::SetEnvironmentVariable('GODOT_PHYSICS_EXTENDED_NUPKGS', $nupkgs, 'User')
    Write-Host "Set user env vars. Restart terminals/IDEs to pick them up."
} else {
    Write-Host "`nSet these (or re-run with -SetEnv):"
    Write-Host "  GODOT_PHYSICS_EXTENDED   = $editor"
    Write-Host "  GODOT_PHYSICS_EXTENDED_NUPKGS  = $nupkgs"
}

Write-Host "`nVerify: & '$editor' --version"
