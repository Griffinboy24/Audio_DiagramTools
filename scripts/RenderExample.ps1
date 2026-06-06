[CmdletBinding()]
param(
  [string]$BuildDir = "out/build",
  [string]$Config = "Release",
  [string]$Preset = "blog-wide",
  [int]$Width = 0,
  [int]$Height = 0,
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

$Args = @("--fps", "$Fps")
if (-not [string]::IsNullOrWhiteSpace($Preset)) {
  $Args += @("--preset", $Preset)
}
if ($Width -gt 0) {
  $Args += @("--width", "$Width")
}
if ($Height -gt 0) {
  $Args += @("--height", "$Height")
}

if ($Frames -gt 1) {
  $Args += @("--frames", "$Frames", "--out-dir", (Join-Path $Root $OutDir))
}
else {
  $Args += @("--out", (Join-Path $Root $Out))
}

& $Renderer @Args
if ($LASTEXITCODE -ne 0) {
  throw "adt_render failed with exit code $LASTEXITCODE"
}
