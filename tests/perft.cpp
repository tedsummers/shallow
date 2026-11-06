#include "EvalCoefficients.h"
#include "Helpers.h"
#include "MovesGenerator.h"
#include "magicbb.h"
#include "xoptimize.h"
#include "xbitmath.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
using namespace NEngine;
using TestBoard = SBoard<Board, UndoInfo, Board::GameLength>;

uint64_t perft(TestBoard& board, int depth)
{
  if(depth == 0)
    return 1;

  uint64_t nodes = 0;
  FastGenerator<Board, SMove> generator(
    static_cast<const Board&>(board), SMove{true}, SMove{true});
  while(auto* move = generator.next())
  {
    board.makeMove(*move);
    nodes += perft(board, depth - 1);
    board.unmakeMove(*move);
  }
  return nodes;
}

struct PerftCase
{
  const char* name;
  const char* fen;
  std::array<uint64_t, 4> expected;
};

constexpr PerftCase cases[] = {
  {"start position", "", {20, 400, 8902, 197281}},
  {"Kiwipete castling and checks",
   "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
   {48, 2039, 97862, 4085603}},
  {"en passant and rook endgame",
   "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", {14, 191, 2812, 43238}},
  {"promotions and castling",
   "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -",
   {6, 264, 9467, 422333}},
  {"promotion and checks",
   "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -",
   {44, 1486, 62379, 2103487}},
  {"tactical middlegame",
   "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -",
   {46, 2079, 89890, 3894594}},
};
}

int main()
{
  using namespace NEngine;
  initGlobals();
  init_popcount_ptr();
  magic_ns::initialize();
  EvalCoefficients::initialize();

  bool passed = true;
  for(const auto& test : cases)
  {
    for(int depth = 1; depth <= 4; ++depth)
    {
      TestBoard board;
      if(!fromFEN(test.fen, board))
        throw std::runtime_error(std::string("could not parse test FEN: ") + test.name);

      if(depth == 1 && !xverifyMoves(board))
      {
        std::cerr << "FAIL " << test.name << ": optimized move generator mismatch\n";
        passed = false;
        continue;
      }

      const uint64_t actual = perft(board, depth);
      const uint64_t expected = test.expected[depth - 1];
      const bool ok = actual == expected;
      passed = passed && ok;
      std::cout << (ok ? "PASS " : "FAIL ") << test.name
                << " depth " << depth << ": " << actual
                << " (expected " << expected << ")\n";
    }
  }

  TestBoard checkedKing;
  if(!fromFEN("4r1k1/8/8/8/8/8/8/4K2R w K - 0 1", checkedKing))
    throw std::runtime_error("could not parse castling-under-check regression FEN");
  const Move illegalCastle{4, 6, Figure::TypeNone};
  const bool castleRejected = !checkedKing.validateMove(illegalCastle);
  passed = passed && castleRejected;
  std::cout << (castleRejected ? "PASS " : "FAIL ")
            << "castling while in check is rejected\n";

  return passed ? 0 : 1;
}
