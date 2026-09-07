

$ErrorActionPreference = "Stop"

$root       = $PSScriptRoot
$srcDir     = Join-Path $root "src"
$includeDir = Join-Path $root "include"
$testsDir   = Join-Path $root "tests"
$binDir     = Join-Path $root "bin"

if (-not (Test-Path $binDir)) {
    New-Item -ItemType Directory -Path $binDir | Out-Null
}

$perftTest = Join-Path $testsDir "perft_tests.cpp"
if (-not (Test-Path $perftTest)) {
    Write-Error "tests\perft_tests.cpp not found at $perftTest"
    exit 1
}


$excluded = @("Search.cpp", "Evaluation.cpp")
$engineSources = Get-ChildItem -Path $srcDir -Filter "*.cpp" |
    Where-Object { $excluded -notcontains $_.Name } |
    ForEach-Object { $_.FullName }

if ($engineSources.Count -eq 0) {
    Write-Error "No .cpp files found in $srcDir"
    exit 1
}

$output = Join-Path $binDir "perft_tests.exe"
$allSources = @($perftTest) + $engineSources

Write-Host "Compiling perft_tests.exe ..."
& g++ -std=c++20 -O2 -Wall -Wextra "-I$includeDir" "-I$srcDir" $allSources -o $output

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed: perft_tests.exe"
    exit $LASTEXITCODE
}

Write-Host "  -> $output"
Write-Host "Build succeeded."