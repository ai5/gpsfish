/* random_play.cc
 */
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include <string>
#include <iostream>
#include <random>
#include <sys/time.h>
using namespace osl;

/**
 * @file ランダムプレイヤ
 */

void showState(const NumEffectState& state)
{
  std::cout << state << std::endl;
#if 0
  KanjiPrint printer(std::cout);
  printer.print(state);
#endif
}

/**
 * ランダムに選ぶ
 */
Move selectMove(const NumEffectState& state, const MoveVector& moves)
{
  static std::mt19937 generator(random());
  return moves[generator() % moves.size()];
#if 0
  boost::uniform_int<boost::mt11213b> random(generator, 0, moves.size());
  return moves[random()];
#endif
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
  srandom(time(0));

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
