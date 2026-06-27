$ErrorActionPreference = "Stop"
$out = Join-Path (Split-Path $PSScriptRoot -Parent) "assets\icon.ico"
New-Item -ItemType Directory -Force -Path (Split-Path $out) | Out-Null

Add-Type -AssemblyName System.Drawing

$bmp = New-Object System.Drawing.Bitmap 256, 256
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.Clear([System.Drawing.Color]::FromArgb(255, 248, 250, 254))

$brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
    [System.Drawing.Point]::new(40, 30),
    [System.Drawing.Point]::new(220, 230),
    [System.Drawing.Color]::FromArgb(255, 59, 130, 246),
    [System.Drawing.Color]::FromArgb(255, 14, 165, 233)
)
$g.FillEllipse($brush, 48, 48, 160, 160)

$inner = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(220, 255, 255, 255))
$g.FillEllipse($inner, 78, 78, 100, 100)

$pen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(255, 37, 99, 235)), 8
$g.DrawArc($pen, 70, 95, 116, 80, 200, 140)

$icon = [System.Drawing.Icon]::FromHandle($bmp.GetHicon())
$fs = [System.IO.File]::Create($out)
$icon.Save($fs)
$fs.Close()

$g.Dispose()
$bmp.Dispose()

Write-Host "Created $out"
