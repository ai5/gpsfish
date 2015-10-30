/* benchmark.h
 */
#ifndef GPSSHOGI_BENCHMARK_H
#define GPSSHOGI_BENCHMARK_H

#include "osl/game_playing/searchPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/search/alphaBeta2.h" /* for EVAL_CLASS */
#include "osl/csa.h"
#include "osl/misc/perfmon.h"
#include <iostream>

void benchmark(osl::game_playing::SearchPlayer& player, const std::string& filename, int seconds)
{
  using namespace osl;
  using namespace osl::game_playing;
  player.setVerbose(2);
  player.setNextIterationCoefficient(1.0);

  NumEffectState nstate(CsaString( // gpw 2007, v.s. kakinoki shogi
			  "P1-KY-KE *  *  *  *  * -KE-OU\n"
			  "P2 *  *  * +UM-KY-GI * -GI-KY\n"
			  "P3 *  *  *  *  * -FU *  *  * \n"
			  "P4 *  * -FU-FU-FU *  *  * -FU\n"
			  "P5-FU+KE * +FU *  *  *  *  * \n"
			  "P6 *  *  *  *  * +FU+FU * +FU\n"
			  "P7+FU+FU *  * +KI * +KE+FU * \n"
			  "P8 *  *  *  * +FU * +GI+OU * \n"
			  "P9-RY-HI *  *  * +KI *  * +KY\n"
			  "P+00KA00KI00KI00FU00FU\n"
			  "P-00GI00FU00FU\n"
			  "+\n").initialState());
  std::vector<Move> moves;
  if (! filename.empty()) {
    CsaFileMinimal file(filename);
    nstate = NumEffectState(file.initialState());
    moves = file.moves();
  }
  std::cerr << "benchmark start " << filename << "\n" << nstate << "\n";
  GameState state(nstate);
  misc::PerfMon clock;
  const Move best_move
    = player.selectBestMove(state, 0, 0, seconds).move;
  const unsigned long long cycles = clock.stop();
  std::cout << csa::show(best_move) << "\n";

  const osl::search::CountRecorder& recorder = player.recorder();
  const unsigned int nodes = recorder.nodeCount();
  const unsigned int qnodes = recorder.quiescenceCount();
  misc::PerfMon::message(cycles, "search ", nodes + qnodes);
  misc::PerfMon::message(cycles, "search+checkmate ", 
			 nodes + qnodes + recorder.checkmateCount());
  if (! moves.empty()) {
    std::cerr << "  recorded ";
    for (size_t i=0; i<moves.size(); ++i) 
      std::cerr << " " << csa::show(moves[i]);
    std::cerr << "\n";
  }
}

void benchmark_more(osl::game_playing::SearchPlayer& player, int num_problems, int seconds) 
{
  using namespace osl;
  const char *selected[] = 
    {
      "../data/problems/floodgate_problems/bad_move/002.csa", // 0
      "../data/problems/floodgate_problems/bad_move/004.csa", // 1
      "../data/problems/floodgate_problems/bad_move/006.csa", // 2
      "../data/problems/floodgate_problems/bad_move/001.csa", // 3
      "../data/problems/floodgate_problems/bad_move/009.csa", // 4
      "../data/problems/floodgate_problems/bad_move/016.csa", // 5
      "../data/problems/floodgate_problems/bad_move/039.csa", // 6
      "../data/problems/floodgate_problems/bad_move/026.csa", // 7
      "../data/problems/floodgate_problems/bad_move/011.csa", // 8
      "../data/problems/floodgate_problems/bad_move/044.csa", // 9
      "../data/problems/floodgate_problems/bad_move/003.csa", // 10
      "../data/checkmate-problems/disproof-20k-200k/5.csa", // 11
      "../data/checkmate-problems/disproof-20k-200k/9.csa", // 12
      "../data/checkmate-problems/disproof-20k-200k/23.csa", // 13
      "../data/checkmate-problems/disproof-20k-200k/1.csa", // 14
      "../data/checkmate-problems/disproof-20k-200k/7.csa", // 15
    };
  const int n_selected = sizeof(selected)/sizeof(const char*);
  const char *bad_moves[] = 
    {
      "../data/problems/floodgate_problems/bad_move/001.csa", // 3
      "../data/problems/floodgate_problems/bad_move/002.csa", // 0
      "../data/problems/floodgate_problems/bad_move/003.csa", // 10
      "../data/problems/floodgate_problems/bad_move/004.csa", // 1
      "../data/problems/floodgate_problems/bad_move/005.csa", // 3
      "../data/problems/floodgate_problems/bad_move/006.csa", // 2
      "../data/problems/floodgate_problems/bad_move/007.csa", // 0
      "../data/problems/floodgate_problems/bad_move/008.csa", // 0
      "../data/problems/floodgate_problems/bad_move/009.csa", // 4
      "../data/problems/floodgate_problems/bad_move/010.csa", // 2
      "../data/problems/floodgate_problems/bad_move/011.csa", // 8
      "../data/problems/floodgate_problems/bad_move/012.csa", // 0
      "../data/problems/floodgate_problems/bad_move/013.csa", // 1
      "../data/problems/floodgate_problems/bad_move/014.csa", // 1
      "../data/problems/floodgate_problems/bad_move/015.csa", // 0
      "../data/problems/floodgate_problems/bad_move/016.csa", // 5
      "../data/problems/floodgate_problems/bad_move/017.csa", // 2
      "../data/problems/floodgate_problems/bad_move/018.csa", // 0
      "../data/problems/floodgate_problems/bad_move/019.csa", // 0
      "../data/problems/floodgate_problems/bad_move/020.csa", // 0
      "../data/problems/floodgate_problems/bad_move/021.csa", // 2
      "../data/problems/floodgate_problems/bad_move/022.csa", // 3
      "../data/problems/floodgate_problems/bad_move/023.csa", // 1
      "../data/problems/floodgate_problems/bad_move/024.csa", // 1
      "../data/problems/floodgate_problems/bad_move/025.csa", // 2
      "../data/problems/floodgate_problems/bad_move/026.csa", // 7
      "../data/problems/floodgate_problems/bad_move/027.csa", // 8
      "../data/problems/floodgate_problems/bad_move/028.csa", // 3
      "../data/problems/floodgate_problems/bad_move/029.csa", // 5
      "../data/problems/floodgate_problems/bad_move/030.csa", // 1
      "../data/problems/floodgate_problems/bad_move/031.csa", // 0
      "../data/problems/floodgate_problems/bad_move/032.csa", // 0
      "../data/problems/floodgate_problems/bad_move/033.csa", // 0
      "../data/problems/floodgate_problems/bad_move/034.csa", // 0
      "../data/problems/floodgate_problems/bad_move/035.csa", // 0
      "../data/problems/floodgate_problems/bad_move/036.csa", // 0
      "../data/problems/floodgate_problems/bad_move/037.csa", // 1
      "../data/problems/floodgate_problems/bad_move/038.csa", // 0
      "../data/problems/floodgate_problems/bad_move/039.csa", // 6
      "../data/problems/floodgate_problems/bad_move/040.csa", // 0
      "../data/problems/floodgate_problems/bad_move/041.csa", // 2
      "../data/problems/floodgate_problems/bad_move/042.csa", // 8
      "../data/problems/floodgate_problems/bad_move/043.csa", // 5
      "../data/problems/floodgate_problems/bad_move/044.csa", // 9
      "../data/problems/floodgate_problems/bad_move/045.csa", // 3
      "../data/problems/floodgate_problems/bad_move/046.csa", // 1
      "../data/problems/floodgate_problems/bad_move/047.csa", // 0
      "../data/problems/floodgate_problems/bad_move/048.csa", // 0
      "../data/problems/floodgate_problems/bad_move/049.csa", // 0
    };
  const int n_bad_moves = sizeof(bad_moves)/sizeof(const char*);
  const char *good_moves[] = 
    {
      "../data/problems/floodgate_problems/good_move/001.csa", // 0
      "../data/problems/floodgate_problems/good_move/002.csa", // 0
      "../data/problems/floodgate_problems/good_move/003.csa", // 4
      "../data/problems/floodgate_problems/good_move/004.csa", // 1
      "../data/problems/floodgate_problems/good_move/005.csa", // 2
      "../data/problems/floodgate_problems/good_move/006.csa", // 2
      "../data/problems/floodgate_problems/good_move/007.csa", // 6
      "../data/problems/floodgate_problems/good_move/008.csa", // 3
      "../data/problems/floodgate_problems/good_move/009.csa", // 6
      "../data/problems/floodgate_problems/good_move/010.csa", // 2
      "../data/problems/floodgate_problems/good_move/011.csa", // 4
      "../data/problems/floodgate_problems/good_move/012.csa", // 0
      "../data/problems/floodgate_problems/good_move/013.csa", // 10
      "../data/problems/floodgate_problems/good_move/014.csa", // 0
      "../data/problems/floodgate_problems/good_move/015.csa", // 0
      "../data/problems/floodgate_problems/good_move/016.csa", // 3
      "../data/problems/floodgate_problems/good_move/017.csa", // 0
      "../data/problems/floodgate_problems/good_move/018.csa", // 0
      "../data/problems/floodgate_problems/good_move/019.csa", // 4
      "../data/problems/floodgate_problems/good_move/020.csa", // 4
      "../data/problems/floodgate_problems/good_move/021.csa", // 0
      "../data/problems/floodgate_problems/good_move/022.csa", // 10
      "../data/problems/floodgate_problems/good_move/023.csa", // 0
      "../data/problems/floodgate_problems/good_move/024.csa", // 0
      "../data/problems/floodgate_problems/good_move/025.csa", // 3
      "../data/problems/floodgate_problems/good_move/026.csa", // 0
      "../data/problems/floodgate_problems/good_move/027.csa", // 0
      "../data/problems/floodgate_problems/good_move/028.csa", // 4
      "../data/problems/floodgate_problems/good_move/029.csa", // 4
    };
  const int n_good_moves = sizeof(good_moves)/sizeof(const char*);
  int count = 0;
  for (int i=0; i<n_selected && count < num_problems; ++i,++count)
    benchmark(player, selected[i], seconds);
  for (int i=0; i<n_bad_moves && count < num_problems; ++i,++count)
    benchmark(player, bad_moves[i], seconds);
  for (int i=0; i<n_good_moves && count < num_problems; ++i,++count)
    benchmark(player, good_moves[i], seconds);
}


#endif /* GPSSHOGI_BENCHMARK_H */
