[CmdletBinding()]
param(
  [string]$BuildDir = "out/build",
  [string]$Config = "Release",
  [string]$Preset = "blog-banner",
  [string]$OutDir = "artifacts/style_lab",
  [string[]]$Studies = @()
)

$ErrorActionPreference = "Stop"

$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
& (Join-Path $PSScriptRoot "Build.ps1") -BuildDir $BuildDir -Config $Config -SkipTests

$BuildPath = Join-Path $Root $BuildDir
$Candidates = @(
  (Join-Path (Join-Path $BuildPath $Config) "adt_style_lab.exe"),
  (Join-Path $BuildPath "adt_style_lab.exe")
)

$StyleLab = $Candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $StyleLab) {
  throw "Could not find adt_style_lab.exe under $BuildPath"
}

$OutputDirectory = Join-Path $Root $OutDir
if ($Studies.Count -eq 0) {
  & $StyleLab --preset $Preset --all --out-dir $OutputDirectory
}
else {
  New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
  foreach ($Study in $Studies) {
    & $StyleLab --preset $Preset --study $Study --out (Join-Path $OutputDirectory "$Study.png")
    if ($LASTEXITCODE -ne 0) {
      throw "adt_style_lab failed for study '$Study' with exit code $LASTEXITCODE"
    }
  }
}

if ($LASTEXITCODE -ne 0) {
  throw "adt_style_lab failed with exit code $LASTEXITCODE"
}
