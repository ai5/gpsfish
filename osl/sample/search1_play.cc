/* random_play.cc
 */
#include "osl/numEffectState.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/csa.h"
#include "osl/book/bookInMemory.h"
#include "osl/random.h"
#include <string>
#include <iostream>
using namespace osl;

/**
 * @file 一手読み
 */

void showState(const NumEffectState& state)
{
  std::cout << state << std::endl;
#if 0
  KanjiPrint printer(std::cout);
  printer.print(state);
#endif
}

/** 先手有利なほど正に大きな数を, 後手有利なほど負に大きな数を返す */
int evaluate(const NumEffectState& state) 
{
  OpenMidEndingEval e(state);
  return e.value();
}

const bool use_book = true;
Move selectMove(const NumEffectState& original, const MoveVector& moves)
{
  if (use_book) {
    const BookInMemory& book = BookInMemory::instance();
    osl::HashKey key(original);
    osl::MoveVector moves;
    book.find(key, moves);
    if(!moves.empty())
      return moves[osl::time_seeded_random()%moves.size()];
  }
  /** 先手+1, 後手-1 */
  const int sgn = sign(original.turn());
  int best_value = 0;
  Move best_move;
  for (auto move: moves) {
    NumEffectState state = original;
    state.makeMove(move);
    int value = evaluate(state);
    if (best_move == Move() || sgn*best_value < sgn*value) {
      best_move = move;
      best_value = value;
    }
  }
  return best_move;
}

/**
 * 指した後，王が取られたら負け
 */
bool isMated(const NumEffectState& state)
{ 
  return state.inCheck(alt(state.turn()));
}

int main()
{
  OslConfig::setUp();
  NumEffectState state;
  std::string line;
  while (true)
  {
    // 自分の手を指す
    MoveVector moves;
    state.generateLegal(moves);
    if (moves.empty())
    {
      std::cerr << "make masita\n";
      break;
    }
    const Move my_move = selectMove(state, moves);
    assert(state.isValidMove(my_move));
    state.makeMove(my_move);

    showState(state);
    std::cout << csa::show(my_move) << "\n";

    if (isMated(state))
    {
      std::cerr << "checkmate!";
      break;
    }

    // 相手の指手を読みこむ
    if (! std::getline(std::cin, line))
      break;

    const Move op_move=csa::strToMove(line, state);
    if (! state.isValidMove(op_move))
      break;
    
    state.makeMove(op_move);

    showState(state);
    std::cout << csa::show(op_move) << "\n";
  } 
}



/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
