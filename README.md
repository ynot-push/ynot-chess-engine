# YNOT Chess Engine

A C++ chess engine project built from scratch using a bitboard-based implementation for move generation, search, evaluation, and performance testing. The repository also contains a separate `ynot-web` demo app plus training tools for the NNUE evaluation pipeline.

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
- Optional: Node.js + npm for the web demo
- Optional: Python 3.10+ for the backend and training tools

If you are on a Unix-like system, the scripts are Windows-specific and may need to be adapted, but the core compilation path is still `g++` with `-std=c++20`.

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

## Web app / demo

The browser demo lives in the separate `ynot-web` project and is split into a Python backend and a React frontend.

Important: a normal browser cannot directly run the native C++ engine in the same way a desktop program can. The usual approach is:

- the React UI runs in the browser
- the backend exposes an API
- the backend calls or wraps the engine
- the browser asks the backend for moves / evaluations

This is the easiest way to get a demo that behaves like a playable chess app without shipping a native desktop executable.

### Backend

Run the backend from the repository root:

```powershell
.\ynot-web\backend\.venv\Scripts\python.exe -m uvicorn app:app --app-dir .\ynot-web\backend --host 127.0.0.1 --port 8000
```

This starts the FastAPI app on:

```text
http://127.0.0.1:8000
```

If `ynot-web` is not present in your checkout, make sure the submodule or nested repo is initialized:

```powershell
git submodule update --init --recursive
```

### Frontend (React)

From the React app directory:

```powershell
npm install
npm run dev
```

This is the usual Vite/React development flow and should start a local frontend server, typically on a URL like:

```text
http://localhost:5173
```

Open the URL printed by the frontend and connect it to the backend.

### Making a simple playable demo

A good demo does not need the full chess engine to run in-browser. The easiest version is:

1. Start the backend
2. Start the React app
3. Send the current board/FEN to the backend
4. Let the backend ask the engine for the best move
5. Return the move to the frontend and render it on the board

A basic demo flow could look like this:

```text
Frontend -> POST /api/next-move { fen, side, time_limit }
Backend -> engine wrapper -> evaluate position -> best move
Backend -> return bestmove
Frontend -> render move on board
```

For a quick demo, you can also simplify the UX:

- limit to one move at a time
- use a fixed time limit such as 1 or 2 seconds
- allow human vs engine or engine vs engine
- show a simple board UI instead of a full tournament interface

If you want the app to be truly playable in the browser, the backend should expose a clean API and the UI should call it instead of trying to run the C++ engine directly in JavaScript.

## Quick verification checklist

Use this sequence to validate the project after cloning:

```powershell
git checkout bitboard
.\build.ps1
.\bin\perft_tests.exe
.\bin\search_test.exe
```

The most important correctness check is the perft test, because it validates move generation against known values.

## Suggested repo polish / naming

The project already has a strong engine core, but it would benefit from clearer separation between the engine, the web demo, and the supporting tooling. My suggestion is to keep the product brand as `YNOT Chess` and split responsibilities cleanly:

- `ynot-chess-engine` - native engine core (C++)
- `ynot-chess-web` - React + backend demo app
- `ynot-chess-trainer` - NNUE training pipeline
- `docs/` - architecture, roadmap, and demo notes

If you want a simpler public-facing product name, I would recommend:

- `YNOT Chess` for the product branding
- `ynot-chess-engine` for the actual engine repo
- `ynot-web` or `ynot-chess-web` for the UI wrapper

This keeps the name more memorable while separating the actual engine from the app layer.

### Good structure for the repo

```text
ynot-chess-engine/
  README.md
  build.ps1
  main.cpp
  include/
  src/
  tests/
  tools/
  nnue-eval-trainer/
  ynot-web/
```

This structure makes it obvious which parts are:

- core engine logic
- tests and validation
- training tooling
- browser demo

## Notes

- The `UCI.cpp` wrapper includes a conservative time-allocation system to respect a move budget while still performing iterative deepening.
- `NNUE.cpp` is loaded from `tools/nnue.bin` by default. If the file is missing or the path is incorrect, the engine falls back to material-only evaluation.
- Search and evaluation are designed for performance, with `-O3`, `-march=native`, and `-flto` enabled in the build script.
- The web demo is best treated as a front-end wrapper around the engine rather than a way to directly run the native binary inside a browser.

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
5. Run the web/demo flow separately through the backend and frontend

That sequence checks correctness first, then performance, then protocol compatibility, and finally the browser experience.

## Disclaimer

This README describes the expected build and validation flow from the repository contents and source files. If you encounter compiler or runtime issues on your machine, the first things to verify are:

- `g++` is installed and on `PATH`
- `build.ps1` is run from the repository root
- the repository branch is `bitboard`
- `tools/nnue.bin` exists if the UCI/search tools are expected to use NNUE evaluation
- `ynot-web` is initialized if you want to run the browser demo
