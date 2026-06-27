#Requires -Version 5.1
<#
.SYNOPSIS
  Configure, build, and run ctm-viewer unit tests.

.EXAMPLE
  .\scripts\run_tests.ps1

.EXAMPLE
  .\scripts\run_tests.ps1 -Config Debug -VerboseOutput
#>
[CmdletBinding()]
param(
    [string]$QtPath = "",
    [string]$MagaimQtRoot = "D:/2025WorkSpace/magaim-qt",
    [string]$BuildDir = "build",
    [ValidateSet("Release", "Debug")]
    [string]$Config = "Release",
    [switch]$VerboseOutput
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

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
        if (Test-Path (Join-Path $path "bin\qmake.exe")) {
            return (Resolve-Path $path).Path
        }
    }
    throw "Qt not found. Set -QtPath or CMAKE_PREFIX_PATH."
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path (Join-Path $ScriptDir "..")
$BuildPath = Join-Path $ProjectRoot $BuildDir
$QtRoot = Resolve-QtPath -Preferred $QtPath

Write-Host "CTM Viewer tests" -ForegroundColor Green
Write-Host "Qt: $QtRoot"
Write-Host "Build: $BuildPath ($Config)"

cmake -S $ProjectRoot -B $BuildPath `
    -DCMAKE_PREFIX_PATH="$QtRoot" `
    -DMAGIMAIM_QT_ROOT="$MagaimQtRoot" `
    -DBUILD_TESTING=ON

cmake --build $BuildPath --config $Config --target test_fieldcolormap test_fieldsimulator test_modelloader test_apptheme

$qtBin = Join-Path $QtRoot "bin"
$vtkBin = Join-Path $MagaimQtRoot "3dLib\VTK64\bin"
$openctmDir = Join-Path $MagaimQtRoot "3dLib\openctm"
$env:PATH = "$qtBin;$vtkBin;$openctmDir;$env:PATH"

$ctestArgs = @("--test-dir", $BuildPath, "-C", $Config, "--output-on-failure")
if ($VerboseOutput) {
    $ctestArgs += "-V"
}

ctest @ctestArgs
if ($LASTEXITCODE -ne 0) {
    throw "One or more tests failed."
}

Write-Host "All tests passed." -ForegroundColor Green
