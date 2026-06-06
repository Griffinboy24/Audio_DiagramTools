[CmdletBinding()]
param(
  [string]$FramesDir = "artifacts/frames/am_sine",
  [string]$Output = "artifacts/gifs/am_sine.gif",
  [double]$Fps = 30.0,
  [int]$Width = 960,
  [int]$Quality = 88
)

$ErrorActionPreference = "Stop"

$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$FramesPath = Join-Path $Root $FramesDir
$OutputPath = Join-Path $Root $Output
$OutputParent = Split-Path -Parent $OutputPath

if (-not (Test-Path $FramesPath)) {
  throw "Frames directory does not exist: $FramesPath"
}

if ($OutputParent) {
  New-Item -ItemType Directory -Force -Path $OutputParent | Out-Null
}

$Gifski = Get-Command gifski -ErrorAction SilentlyContinue

if (-not $Gifski) {
  throw "gifski is required for GIF export. Install it and try again. See docs/pipeline.md for links."
}

$FrameFiles = Get-ChildItem -Path $FramesPath -Filter "frame_*.png" |
  Sort-Object Name |
  ForEach-Object { $_.FullName }

if ($FrameFiles.Count -eq 0) {
  throw "No frame_*.png files found in $FramesPath"
}

& gifski --fps $Fps --width $Width --quality $Quality -o $OutputPath @FrameFiles

Write-Host "Wrote $OutputPath"
