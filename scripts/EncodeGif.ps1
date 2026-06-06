[CmdletBinding()]
param(
  [string]$BuildDir = "out/build",
  [string]$Config = "Release",
  [int]$Width = 1280,
  [int]$Height = 720,
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
  "--width", "$Width",
  "--height", "$Height",
  "--fps", "$Fps",
  "--frames", "$Frames",
  "--out", (Join-Path $Root $Out)
)

if ($NoDither) {
  $Args += "--no-dither"
}

& $Encoder @Args
if ($LASTEXITCODE -ne 0) {
  throw "adt_encode_gif failed with exit code $LASTEXITCODE"
}
