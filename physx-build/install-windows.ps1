<#
.SYNOPSIS
    Installs the physx engine build output on Windows (export templates, nupkgs, env vars).
.PARAMETER SetEnv
    Persist GODOT_PHYSICS_EXTENDED and GODOT_PHYSICS_EXTENDED_NUPKGS at user level.
.PARAMETER PhysxHome
    (Also accepted as -Home.)
    Build output folder. Default: $env:GODOT_PHYSX_HOME or %LOCALAPPDATA%\godot-physx.
#>
[CmdletBinding()]
param(
    [switch]$SetEnv,
    [Alias('Home')]
    [string]$PhysxHome = $(if ($env:GODOT_PHYSX_HOME) { $env:GODOT_PHYSX_HOME } else { Join-Path $env:LOCALAPPDATA 'godot-physx' })
)

$ErrorActionPreference = 'Stop'

$manifestPath = Join-Path $PhysxHome 'manifest.txt'
if (-not (Test-Path $manifestPath)) { throw "manifest.txt not found in '$PhysxHome'. Run physx-build/build.sh in WSL first." }
$manifest = @{}
Get-Content $manifestPath | ForEach-Object { if ($_ -match '^([^=]+)=(.*)$') { $manifest[$Matches[1]] = $Matches[2] } }

$folder = "$($manifest['version']).$($manifest['status']).mono"
$winDir = Join-Path $PhysxHome 'windows'
$nupkgs = Join-Path $PhysxHome 'nupkgs'
Write-Host "Engine version folder: $folder (sha $($manifest['sha']), dirty=$($manifest['dirty']), profile=$($manifest['profile']))"

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
if (-not (Test-Path $editor)) { Write-Warning "Editor exe not found at $editor" }

if ($SetEnv) {
    [Environment]::SetEnvironmentVariable('GODOT_PHYSICS_EXTENDED', $editor, 'User')
    [Environment]::SetEnvironmentVariable('GODOT_PHYSICS_EXTENDED_NUPKGS', $nupkgs, 'User')
    Write-Host "Set user env vars. Restart terminals/IDEs to pick them up."
} else {
    Write-Host "`nSet these (or re-run with -SetEnv):"
    Write-Host "  GODOT_PHYSICS_EXTENDED   = $editor"
    Write-Host "  GODOT_PHYSICS_EXTENDED_NUPKGS  = $nupkgs"
}

Write-Host "`nVerify: & '$editor' --version"
