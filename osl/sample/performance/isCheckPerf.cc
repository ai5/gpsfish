/**
 * 王手判定の速さを見る
 */
#include "osl/numEffectState.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/move_action.h"
#include "osl/move_classifier/check_.h"
#include "osl/csa.h"
#include "osl/misc/perfmon.h"
#include <iostream>
#include <cstdio>
using namespace osl;

int main(int argc,char **argv)
{
  // extern char *optarg;

  char c;
  while ((c = getopt(argc, argv, "vh")) != EOF)
  {
    switch(c)
    {
    default:
      std::cerr << "unknown option\n";
      return 1;
    }
  }

  NumEffectState state((CsaString(
		    "P1-KY *  *  * -KY * -FU-KE * \n"
		    "P2 *  *  *  * -OU *  *  *  * \n"
		    "P3 *  *  * -FU-FU+RY *  * -KY\n"
		    "P4-FU *  * -GI *  *  *  *  * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6+FU *  * +RY *  * +FU *  * \n"
		    "P7 * +FU * +FU+FU+FU *  *  * \n"
		    "P8 *  * +OU * -TO *  *  *  * \n"
		    "P9+KY *  *  *  *  *  * +KE * \n"
		    "P+00KI00GI00GI00GI00KE00KE00FU00FU00FU00KI\n"
		    "P-00KA00KA00KI00FU00FU00FU00FU00KI\n"
		    "-\n").initialState()));

  MoveVector moves;
  GenerateAllMoves::generate(state.turn(),state,moves);

  int count = 0;
  misc::PerfMon clock;
  for (size_t i=0; i<moves.size(); ++i)
  {
    if (move_classifier::Check<WHITE>::isMember
	(state, moves[i].ptype(), moves[i].from(), moves[i].to()))
      ++count;
  }
  clock.stop("total", moves.size());
  std::cerr << "checks " << count << " / " << moves.size() << "\n";
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
