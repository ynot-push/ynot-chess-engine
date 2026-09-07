# build.ps1
$ErrorActionPreference = "Stop"
mkdir bin -Force | Out-Null

$flags = @("-std=c++20", "-O3", "-march=native", "-flto", "-DNDEBUG", "-Iinclude")

$targets = @{
    "tests\PerftTests.cpp"           = "bin\perft_tests.exe"
    "tests\SearchTest.cpp"           = "bin\search_test.exe"
    "main.cpp"                       = "bin\chess_main.exe"
   "tests\UCI.cpp" = "bin\chess_bot.exe"
}

foreach ($src in $targets.Keys) {
    $out = $targets[$src]
    Write-Host "Building $out ..."
    g++ @flags -o $out $src src\*.cpp
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAILED: $src" -ForegroundColor Red
        exit 1
    }
}

Write-Host "All builds succeeded." -ForegroundColor Green