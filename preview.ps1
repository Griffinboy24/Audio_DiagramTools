param(
  [int] $Port = 8066,
  [string] $Project = "hise-dsp-buffer",
  [switch] $Tools,
  [switch] $Open
)

$ErrorActionPreference = "Stop"
$repoRoot = $PSScriptRoot
$server = Join-Path $repoRoot "scripts\serve_preview_lab.py"

if (-not (Test-Path -LiteralPath $server)) {
  throw "Missing preview server: $server"
}

$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) {
  throw "Python was not found on PATH."
}

$serverArgs = @($server, $Port, "--project", $Project)

if ($Tools) {
  $serverArgs += "--tools"
}

if ($Open) {
  $serverArgs += "--open"
}

& $python.Source @serverArgs
