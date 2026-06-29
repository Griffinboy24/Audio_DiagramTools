param(
  [Parameter(Mandatory = $true)]
  [string] $Slug,

  [Parameter(Mandatory = $true)]
  [string] $Title,

  [string] $Author = "griffinboy",
  [string] $ArticlesRoot = "articles",
  [switch] $Force
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")
$tool = Join-Path $repoRoot "out\build\adt_create_article_project.exe"

if (-not (Test-Path -LiteralPath $tool)) {
  throw "Missing $tool. Build first with scripts\Build.ps1."
}

$args = @(
  "--articles-root", $ArticlesRoot,
  "--slug", $Slug,
  "--title", $Title,
  "--author", $Author
)

if ($Force) {
  $args += "--force"
}

& $tool @args
