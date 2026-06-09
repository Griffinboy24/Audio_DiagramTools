[CmdletBinding()]
param(
  [string]$BuildDir = "out/build",
  [string]$Config = "Release",
  [string]$VisageDir = "build/_vendor/visage",
  [string]$Generator = "MinGW Makefiles",
  [string]$Architecture = "",
  [int]$Jobs = 4,
  [switch]$SkipTests
)

$ErrorActionPreference = "Stop"

$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$BuildPath = Join-Path $Root $BuildDir

function Invoke-NativeChecked {
  param(
    [string]$Command,
    [string[]]$Arguments
  )

  & $Command @Arguments
  if ($LASTEXITCODE -ne 0) {
    throw "$Command failed with exit code $LASTEXITCODE"
  }
}

$ConfigureArgs = @(
  "-S", $Root,
  "-B", $BuildPath,
  "-G", $Generator,
  "-DAUDIO_DIAGRAM_VISAGE_DIR=$VisageDir",
  "-DADT_BUILD_TESTS=ON"
)

if ($Architecture -and $Generator -like "Visual Studio*") {
  $ConfigureArgs += @("-A", $Architecture)
}

Invoke-NativeChecked cmake $ConfigureArgs
Invoke-NativeChecked cmake @("--build", $BuildPath, "--config", $Config, "--target", "adt_render", "--parallel", "$Jobs")
Invoke-NativeChecked cmake @("--build", $BuildPath, "--config", $Config, "--target", "adt_encode_gif", "--parallel", "$Jobs")
Invoke-NativeChecked cmake @("--build", $BuildPath, "--config", $Config, "--target", "adt_style_lab", "--parallel", "$Jobs")

if (-not $SkipTests) {
  Invoke-NativeChecked cmake @("--build", $BuildPath, "--config", $Config, "--target", "adt_smoke_tests", "--parallel", "$Jobs")
  Invoke-NativeChecked cmake @("--build", $BuildPath, "--config", $Config, "--target", "adt_gif_smoke_tests", "--parallel", "$Jobs")
  Invoke-NativeChecked ctest @("--test-dir", $BuildPath, "-C", $Config, "--output-on-failure")
}
