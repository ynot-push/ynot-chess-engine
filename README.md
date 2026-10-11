# YNOT Chess Engine

A C++ chess engine project built from scratch using a bitboard-based implementation for move generation, search, evaluation, and performance testing. The repository also contains a separate `array-engine` branch for comparison and experimentation.

## Project status

This repository is currently set up for the `bitboard` implementation. The build script compiles a few runnable binaries into a local `bin/` directory:

- `bin/chess_main.exe` - basic executable that prints the initial board
- `bin/chess_bot.exe` - UCI-compatible engine
- `bin/perft_tests.exe` - perft validation test binary
- `bin/search_test.exe` - search performance benchmark

## Requirements

- Windows
- C++20 compatible compiler (`g++`/MinGW-w64)
- PowerShell
- The project root checked out locally

If you are on a Unix-like system, the scripts are Windows-specific and may need to be adapted, but the core compilation path is still `g++` with `-std=c++20`.

## Repository structure

```text
.
├── README.md
├── build.ps1
├── main.cpp
├── include/
│   ├── Board.h
│   ├── NNUE.h
│   └── ...
├── src/
│   ├── Board.cpp
│   ├── NNUE.cpp
│   ├── Pawn.cpp
│   ├── Rook.cpp
│   ├── Bishop.cpp
│   ├── Knight.cpp
│   ├── Queen.cpp
│   ├── King.cpp
│   ├── square.cpp
│   └── ...
├── tests/
│   ├── PerftTests.cpp
│   ├── SearchTest.cpp
│   └── UCI.cpp
├── tools/
│   └── nnue.bin
└── bin/
```

## Build

From the repository root in PowerShell:

```powershell
.\build.ps1
```

The script creates `bin/` if needed and builds the following targets:

```powershell
"tests\PerftTests.cpp"           = "bin\perft_tests.exe"
"tests\SearchTest.cpp"           = "bin\search_test.exe"
"main.cpp"                       = "bin\chess_main.exe"
"tests\UCI.cpp"                  = "bin\chess_bot.exe"
```

The build command used is:

```powershell
g++ -std=c++20 -O3 -march=native -flto -DNDEBUG -Iinclude -o <output> <source> src\*.cpp
```

## Run the engine

### 1) Basic board test

```powershell
.\bin\chess_main.exe
```

This runs `main.cpp`, which creates a `Board`, loads the starting position, and prints the board.

### 2) Perft validation

```powershell
.\bin\perft_tests.exe
```

The test program is hard-coded to check the standard starting position at depths 1 through 5 and compares the results to known values:

```text
perft(1) = 20, expected 20
perft(2) = 400, expected 400
perft(3) = 8902, expected 8902
perft(4) = 197281, expected 197281
perft(5) = 4865609, expected 4865609
```

The source expects the following known-correct values:

```cpp
const long long expected[] = { 1, 20, 400, 8902, 197281, 4865609 };
```

This is a standard and widely accepted perft validation set for the initial chess position.

### 3) Search benchmark

```powershell
.\bin\search_test.exe
```

This repeatedly searches from the starting position at depths 1..6 and prints timing results such as:

```text
depth 1: e2e4  nodes=...  time=...s  nps=...
depth 2: ...
...
```

It also prints breakdown timing for:

- move generation
- legality filtering
- evaluation
- make/unmake operations

### 4) UCI engine mode

```powershell
.\bin\chess_bot.exe
```

The program implements a minimal UCI protocol and accepts commands like:

```text
uci
isready
position startpos
go depth 5
bestmove e2e4
```

This makes it compatible with standard UCI clients such as Arena, Cutechess, and similar GUI/tournament tools.

## Quick verification checklist

Use this sequence to validate the project after cloning:

```powershell
git checkout bitboard
.\build.ps1
.\bin\perft_tests.exe
.\bin\search_test.exe
```

The most important correctness check is the perft test, because it validates move generation against known values.

## Notes

- The `UCI.cpp` wrapper includes a conservative time-allocation system to respect a move budget while still performing iterative deepening.
- `NNUE.cpp` is loaded from `tools/nnue.bin` by default. If the file is missing or the path is incorrect, the engine falls back to material-only evaluation.
- Search and evaluation are designed for performance, with `-O3`, `-march=native`, and `-flto` enabled in the build script.

## Branches

### `bitboard`

Bitboard-based implementation focused on efficient board representation and move generation.

### `array-engine`

Array-based implementation maintained as a separate engine for comparison and experimentation.

## Intended validation protocol

Because this project is a chess engine, the most rigorous validation path is:

1. Compile with `build.ps1`
2. Run perft validation at standard depths
3. Run the search benchmark for timing and node throughput
4. Use `chess_bot.exe` in UCI mode against a GUI or tournament harness

That sequence checks correctness first, then performance, then protocol compatibility.

## Disclaimer

This README describes the expected build and validation flow from the repository contents and source files. If you encounter compiler or runtime issues on your machine, the first things to verify are:

- `g++` is installed and on `PATH`
- `build.ps1` is run from the repository root
- the repository branch is `bitboard`
- `tools/nnue.bin` exists if the UCI/search tools are expected to use NNUE evaluation
