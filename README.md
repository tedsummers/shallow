# Shallow

Shallow is an open-source UCI chess engine written in C++, originally developed by [dimock/shallow](https://github.com/dimock/shallow).

This fork incorporates critical bug fixes for chess rule correctness, search and repetition handling, endgame evaluation tables, and test coverage.

---

## Key Improvements & Fixes in this Fork

- **Full Underpromotion Generation**:
  - Pawn move and capture generators now generate all legal promotion pieces (Queen, Rook, Bishop, Knight) rather than restricting to Queen and checking Knights.
  - Move optimization verification (`xverifyMoves`) now validates all promotions.

- **Castling Legality Validation**:
  - Corrected castling check logic in `Board::validateMove` to cleanly reject castling attempts when the king is in check, resolving assertion failures.

- **Search & Repetition Handling**:
  - Repetition tracking (`Board::countReps`) now respects null-move pruning boundaries, ensuring synthetic search-tree passes do not create false 3-fold repetition matches with actual game history.
  - Null move execution resets `repsCounter_` to prevent invalid repetition triggers in search lines.

- **Root Move Ordering**:
  - Improved `Engine::sortMoves0` so best-move prioritization (`bring_to_front`) executes even when only two legal moves are present.

- **Endgame Evaluation Tables**:
  - Corrected swapped endgame table definitions in `SpecialCasesDetector`: Rook + Bishop vs Rook now correctly invokes `bishopRooks()`, and Bishop pair vs single Bishop correctly evaluates as almost draw.

- **Hash Table & Memory Safety**:
  - Prevented 32-bit integer overflow in hash size memory calculations.
  - Increased maximum allowable hash table allocation to 2048 MB.

- **Test Suite & Regression Testing**:
  - Added a perft test harness (`shallow_perft`) with CTest integration testing depths 1–4 across standard benchmark positions, move verification, and castling regression checks.

---

## Building

### Prerequisites
- C++14 capable compiler (GCC 7+ or Clang 5+)
- CMake 3.10+ (optional, for CMake builds & CTest)
- Support for `-mpopcnt` and pthreads

### Quick Build with GCC (Linux)

From the repository root:

```sh
g++ -std=c++14 -O2 -mpopcnt -Iinclude source/*.cpp -pthread -o shallow-linux
```

### Build on Windows

#### Option A: Visual Studio / MSVC (CMake)
Open the project directory in Visual Studio or build from a **Developer Command Prompt for VS**:

```cmd
cmake -S . -B build
cmake --build build --config Release
```
The resulting executable will be in `build\Release\shallow_*.exe`.

#### Option B: MinGW-w64 / MSYS2 (GCC)
From an MSYS2 or MinGW-w64 terminal:

```sh
g++ -std=c++14 -O2 -mpopcnt -Iinclude source/*.cpp -pthread -static -o shallow-windows-x86_64.exe
```

#### Option C: Cross-compiling for Windows on Linux
You can easily cross-compile a Windows 64-bit binary from Linux using `zig c++` or `x86_64-w64-mingw32-g++`:

```sh
# Using Zig
zig c++ -target x86_64-windows-gnu -std=c++14 -O2 -mpopcnt -Iinclude source/*.cpp -pthread -o shallow-windows-x86_64.exe

# Using MinGW-w64
x86_64-w64-mingw32-g++ -std=c++14 -O2 -mpopcnt -Iinclude source/*.cpp -pthread -static -o shallow-windows-x86_64.exe
```

### Build with CMake (Linux / macOS)

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The compiled engine executable will be placed in the `build/` directory.

---

## Testing

Run the move generation (perft) suite and regression tests:

```sh
cmake -S . -B build
cmake --build build --target shallow_perft
ctest --test-dir build --output-on-failure
```

---

## UCI Usage

Shallow conforms to the Universal Chess Interface (UCI) protocol. You can run it directly in a terminal or connect it to any chess GUI (e.g., Arena, Cute Chess, Banksia GUI, Nibbler).

Example terminal session:

```sh
$ ./build/shallow_84a3
uci
id name shallow_84a3
id author ...
option name Hash type spin default 64 min 1 max 2048
option name Threads type spin default 1 min 1 max 128
uciok
isready
readyok
position startpos moves e2e4 e7e5
go depth 8
```

### Supported UCI Options

- `Hash` (spin, min 1, max 2048, default 64): Transposition table size in megabytes.
- `Clear Hash` (button): Flushes the transposition table.
- `Threads` (spin, min 1, max 128, default 1): Number of search threads.
