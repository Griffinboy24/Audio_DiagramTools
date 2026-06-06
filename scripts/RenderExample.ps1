[CmdletBinding()]
param(
  [string]$BuildDir = "out/build",
  [string]$Config = "Release",
  [int]$Width = 1280,
  [int]$Height = 720,
  [int]$Frames = 1,
  [double]$Fps = 30.0,
  [string]$Out = "artifacts/stills/am_sine.png",
  [string]$OutDir = "artifacts/frames/am_sine"
)

$ErrorActionPreference = "Stop"

$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
& (Join-Path $PSScriptRoot "Build.ps1") -BuildDir $BuildDir -Config $Config -SkipTests

$BuildPath = Join-Path $Root $BuildDir
$Candidates = @(
  (Join-Path (Join-Path $BuildPath $Config) "adt_render.exe"),
  (Join-Path $BuildPath "adt_render.exe")
)

$Renderer = $Candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $Renderer) {
  throw "Could not find adt_render.exe under $BuildPath"
}

if ($Frames -gt 1) {
  & $Renderer --width $Width --height $Height --fps $Fps --frames $Frames --out-dir (Join-Path $Root $OutDir)
}
else {
  & $Renderer --width $Width --height $Height --fps $Fps --out (Join-Path $Root $Out)
}
