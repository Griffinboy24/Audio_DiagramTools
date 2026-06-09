[CmdletBinding()]
param(
  [string]$BuildDir = "out/build",
  [string]$Config = "Release",
  [string]$Preset = "blog-banner",
  [string]$Out = "artifacts/science/eight-sample-waveform.png"
)

$ErrorActionPreference = "Stop"

$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
& (Join-Path $PSScriptRoot "Build.ps1") -BuildDir $BuildDir -Config $Config -SkipTests

$BuildPath = Join-Path $Root $BuildDir
$Candidates = @(
  (Join-Path (Join-Path $BuildPath $Config) "adt_eight_sample_waveform.exe"),
  (Join-Path $BuildPath "adt_eight_sample_waveform.exe")
)

$Renderer = $Candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $Renderer) {
  throw "Could not find adt_eight_sample_waveform.exe under $BuildPath"
}

& $Renderer --preset $Preset --out (Join-Path $Root $Out)
if ($LASTEXITCODE -ne 0) {
  throw "adt_eight_sample_waveform failed with exit code $LASTEXITCODE"
}
