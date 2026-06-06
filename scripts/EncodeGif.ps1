[CmdletBinding()]
param(
  [string]$BuildDir = "out/build",
  [string]$Config = "Release",
  [string]$Preset = "gif-preview",
  [int]$Width = 0,
  [int]$Height = 0,
  [int]$Frames = 120,
  [double]$Fps = 30.0,
  [string]$Out = "artifacts/gifs/am_sine.gif",
  [switch]$NoDither
)

$ErrorActionPreference = "Stop"

$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
& (Join-Path $PSScriptRoot "Build.ps1") -BuildDir $BuildDir -Config $Config -SkipTests

$BuildPath = Join-Path $Root $BuildDir
$Candidates = @(
  (Join-Path (Join-Path $BuildPath $Config) "adt_encode_gif.exe"),
  (Join-Path $BuildPath "adt_encode_gif.exe")
)

$Encoder = $Candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $Encoder) {
  throw "Could not find adt_encode_gif.exe under $BuildPath"
}

$Args = @(
  "--fps", "$Fps",
  "--frames", "$Frames",
  "--out", (Join-Path $Root $Out)
)

if (-not [string]::IsNullOrWhiteSpace($Preset)) {
  $Args = @("--preset", $Preset) + $Args
}
if ($Width -gt 0) {
  $Args += @("--width", "$Width")
}
if ($Height -gt 0) {
  $Args += @("--height", "$Height")
}
if ($NoDither) {
  $Args += "--no-dither"
}

& $Encoder @Args
if ($LASTEXITCODE -ne 0) {
  throw "adt_encode_gif failed with exit code $LASTEXITCODE"
}
