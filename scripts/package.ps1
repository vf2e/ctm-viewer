#Requires -Version 5.1
<#
.SYNOPSIS
  Build and package ctm-viewer for Windows x64.

.EXAMPLE
  .\scripts\package.ps1

.EXAMPLE
  .\scripts\package.ps1 -QtPath "C:\qt5.15.2\5.15.2\msvc2019_64" -SkipBuild
#>
[CmdletBinding()]
param(
    [string]$QtPath = "",
    [string]$MagaimQtRoot = "D:/2025WorkSpace/magaim-qt",
    [string]$BuildDir = "build",
    [ValidateSet("Release", "Debug")]
    [string]$Config = "Release",
    [switch]$SkipBuild,
    [switch]$NoZip
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Step([string]$Message) {
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Resolve-QtPath {
    param([string]$Preferred)

    $candidates = @()
    if ($Preferred) { $candidates += $Preferred }
    if ($env:CMAKE_PREFIX_PATH) { $candidates += $env:CMAKE_PREFIX_PATH }
    $candidates += @(
        "C:\qt5.15.2\5.15.2\msvc2019_64",
        "C:\Qt\5.15.2\msvc2019_64"
    )

    foreach ($path in $candidates) {
        if (-not $path) { continue }
        $windeployqt = Join-Path $path "bin\windeployqt.exe"
        if (Test-Path $windeployqt) {
            return (Resolve-Path $path).Path
        }
    }

    throw "Qt not found. Set -QtPath or CMAKE_PREFIX_PATH (must contain bin\windeployqt.exe)."
}

function Ensure-Command([string]$Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Command not found: $Name"
    }
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path (Join-Path $ScriptDir "..")
$Version = "1.0.0"
$PackageName = "ctm-viewer-$Version-win64"
$DistRoot = Join-Path $ProjectRoot "dist"
$PackageDir = Join-Path $DistRoot $PackageName
$BuildPath = Join-Path $ProjectRoot $BuildDir
$OutputDir = Join-Path $BuildPath $Config
$ExePath = Join-Path $OutputDir "ctm-viewer.exe"

Write-Host "CTM Viewer package" -ForegroundColor Green
Write-Host "Project: $ProjectRoot"

Ensure-Command cmake

$QtRoot = Resolve-QtPath -Preferred $QtPath
$Windeployqt = Join-Path $QtRoot "bin\windeployqt.exe"
Write-Host "Qt: $QtRoot"
Write-Host "magaim-qt: $MagaimQtRoot"

if (-not (Test-Path $MagaimQtRoot)) {
    throw "magaim-qt path does not exist: $MagaimQtRoot"
}

if (-not $SkipBuild) {
    Write-Step "CMake configure"
    $cmakeArgs = @(
        "-S", $ProjectRoot,
        "-B", $BuildPath,
        "-DCMAKE_PREFIX_PATH=$QtRoot",
        "-DMAGIMAIM_QT_ROOT=$MagaimQtRoot"
    )
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

    Write-Step "Build $Config"
    & cmake --build $BuildPath --config $Config --parallel
    if ($LASTEXITCODE -ne 0) { throw "Build failed" }
}

if (-not (Test-Path $ExePath)) {
    throw "Executable not found: $ExePath"
}

$AssetsDir = Join-Path $OutputDir "assets"
if (-not (Test-Path $AssetsDir)) {
    throw "Assets directory not found: $AssetsDir"
}

Write-Step "Prepare package directory"
if (Test-Path $PackageDir) {
    Remove-Item $PackageDir -Recurse -Force
}
New-Item -ItemType Directory -Path $PackageDir -Force | Out-Null

Write-Step "Copy build output"
Copy-Item -Path (Join-Path $OutputDir "*") -Destination $PackageDir -Recurse -Force

Write-Step "Run windeployqt"
$deployArgs = @(
    "--$($Config.ToLower())",
    "--no-translations",
    "--no-opengl-sw",
    "--no-system-d3d-compiler",
    "--dir", $PackageDir,
    (Join-Path $PackageDir "ctm-viewer.exe")
)
& $Windeployqt @deployArgs
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed" }

Write-Step "Write VERSION.txt"
$versionFile = Join-Path $PackageDir "VERSION.txt"
@(
    "CTM Viewer $Version"
    "BuildConfig: $Config"
    "PackagedAt: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    "Qt: $QtRoot"
) | Set-Content -Path $versionFile -Encoding UTF8

if (-not $NoZip) {
    Write-Step "Create zip archive"
    if (-not (Test-Path $DistRoot)) {
        New-Item -ItemType Directory -Path $DistRoot -Force | Out-Null
    }

    $zipPath = Join-Path $DistRoot "$PackageName.zip"
    if (Test-Path $zipPath) {
        Remove-Item $zipPath -Force
    }

    Compress-Archive -Path $PackageDir -DestinationPath $zipPath -Force
    Write-Host "Zip: $zipPath" -ForegroundColor Green
}

Write-Host ""
Write-Host "Done" -ForegroundColor Green
Write-Host "Folder: $PackageDir"
Write-Host "Run: $(Join-Path $PackageDir 'ctm-viewer.exe')"
