# YNOT Chess Engine

A C++ chess engine project from scratch implemented with two different board representations:

- `bitboard` — bitboard-based chess engine
- `array-engine` — array-based chess engine

The project is focused on efficient move generation, search, evaluation, and engine performance.

## Requirements

- Windows
- C++20 compiler
- MinGW-w64 / GCC
- PowerShell

## Build

From the project root:

```powershell
.\build.ps1
```

## Test

For perft tests:

```powershell
.\bin\perft_tests.exe
```

For search performance test:

```powershell
.\bin\search_test.exe
```

## Branches

### bitboard

Bitboard-based implementation focused on efficient board representation and move generation.

### array-engine

Array-based implementation maintained as a separate engine implementation for comparison and experimentation.
