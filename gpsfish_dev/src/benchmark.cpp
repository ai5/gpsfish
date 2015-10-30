/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2010 Marco Costalba, Joona Kiiski, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fstream>
#include <iostream>
#include <vector>

#include "position.h"
#include "search.h"
#include "ucioption.h"

using namespace std;

static const string Defaults[] = {
#ifdef GPSFISH
"kng4nl/lsg2r3/pppppsbpp/5pp2/7P1/2P1P1PS1/PP1PSP2P/1BK1G2R1/LN1G3NL w - 1",
"l6nl/1r3sgk1/2np1gspp/2pbppp2/pp1P4P/2P1P1PP1/PPSG1P2L/1KGB1S2R/LN5N1 w - 1",
"kng1g2nl/ls7/pppps1bpp/4p1p2/7P1/2PS1rP1P/PPBP5/L1G2G1R1/KNS4NL w 2Pp 1",
"ln2gk2l/3s2gs1/2ppppn1p/pr7/9/P1P3R2/3PPPP1P/1SG1KSG2/LN5NL b B3Pb2p 1",

"lns2g1n1/1r3ks2/p1pp1p1pp/3gp1p2/9/2P1L3P/P1+BP1PPP1/1S2RK3/LN1G1GSNL b Pb2p 1",
"ln3gsnl/1r1s3k1/p2pp1bpp/5pp2/4R4/PB1P5/1P2PPPPP/5KS2/LN1G1G1NL w G3Ps 1",
"ln4p2/2gkgs1+R1/ppsppp3/8p/9/1P7/P1PPb1PPP/1SG1R4/LN5KL w B2NLPgs3p 1",
"l4k1nl/3b2g2/p5spp/2pp2p2/4P4/2rP2P2/PP2RP1PP/2GS5/LNBK3NL b 2S2P2gn2p 1",

"ln1g1B2l/1sk1g2s1/ppp1ppnpp/3p2r2/3N5/2RKBP3/PP1PPGgPP/1S7/L6sL w NP2p 1",
"ln4R1l/2k2g3/pppp2n1p/9/P3Spp2/2P6/1P1PP1P1P/1KG1+s1+p2/LN5NL w 2B2Pr2g2sp 1",
"4r4/1kg2+L2l/1s1g5/ppp4pp/4ppp2/LPPS4P/P3PP3/g1K6/1N4R1L w B2S3Nbg4p 1",
"7nk/6ssl/7pp/p1p2p3/1p6P/P1P3pL1/1P3PNP1/6P1L/3+RP1N1K w RB2GNLb2g2s3p 1",

"1n1g4l/1ks1g4/ppppps1p1/5p3/4P3P/2PB1G2L/1P1P1P1P1/6+r2/1N2K4 b BGSN2L3Prsnp 1",
"4k4/1sG1g4/lpPpp1+L2/2p1s3N/3P4r/1R5pp/3bPGPP1/6K2/6SNL b B2NL4Pgs3p 1",
"ln6+B/1Sk5l/2ps5/pg2g2pp/1NNp3N1/PK2PP+b1P/4S4/7R1/L7L w RGS7Pg2p 1",
"3g3+B1/1Rs2+P3/3kp2p1/Nppp2p1p/pn2L2N1/2P5P/LP1PP4/PG1+r5/KG7 b SLPbg2snl2p 1",
""
#else
  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
  "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
  "4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
  "rq3rk1/ppp2ppp/1bnpb3/3N2B1/3NP3/7P/PPPQ1PP1/2KR3R w - - 7 14",
  "r1bq1r1k/1pp1n1pp/1p1p4/4p2Q/4Pp2/1BNP4/PPP2PPP/3R1RK1 w - - 2 14",
  "r3r1k1/2p2ppp/p1p1bn2/8/1q2P3/2NPQN2/PPP3PP/R4RK1 b - - 2 15",
  "r1bbk1nr/pp3p1p/2n5/1N4p1/2Np1B2/8/PPP2PPP/2KR1B1R w kq - 0 13",
  "r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16",
  "4r1k1/r1q2ppp/ppp2n2/4P3/5Rb1/1N1BQ3/PPP3PP/R5K1 w - - 1 17",
  "2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11",
  "r1bq1r1k/b1p1npp1/p2p3p/1p6/3PP3/1B2NN2/PP3PPP/R2Q1RK1 w - - 1 16",
  "3r1rk1/p5pp/bpp1pp2/8/q1PP1P2/b3P3/P2NQRPP/1R2B1K1 b - - 6 22",
  "r1q2rk1/2p1bppp/2Pp4/p6b/Q1PNp3/4B3/PP1R1PPP/2K4R w - - 2 18",
  "4k2r/1pb2ppp/1p2p3/1R1p4/3P4/2r1PN2/P4PPP/1R4K1 b - - 3 22",
  "3q2k1/pb3p1p/4pbp1/2r5/PpN2N2/1P2P2P/5PP1/Q2R2K1 b - - 4 26",
  ""
#endif
};


/// benchmark() runs a simple benchmark by letting Stockfish analyze a set
/// of positions for a given limit each.  There are five parameters; the
/// transposition table size, the number of search threads that should
/// be used, the limit value spent for each position (optional, default
/// is ply 12), an optional file name where to look for positions in fen
/// format (default are the BenchmarkPositions defined above) and the type
/// of the limit value: depth (default), time in secs or number of nodes.
/// The analysis is written to a file named bench.txt.

void benchmark(int argc, char* argv[]) {
#ifdef GPSFISH
  bool ok = osl::eval::ml::OpenMidEndingEval::setUp();
  ok &= osl::progress::ml::NewProgress::setUp();
  if (! ok) {
    std::cerr << "set up failed\n";
    return;
  }
#endif

  vector<string> fenList;
  SearchLimits limits;
  int64_t totalNodes;
  int time;

  // Load default positions
  for (int i = 0; !Defaults[i].empty(); i++)
      fenList.push_back(Defaults[i]);

  // Assign default values to missing arguments
  string ttSize  = argc > 2 ? argv[2] : "128";
  string threads = argc > 3 ? argv[3] : "1";
  string valStr  = argc > 4 ? argv[4] : "12";
  string fenFile = argc > 5 ? argv[5] : "default";
  string valType = argc > 6 ? argv[6] : "depth";

  Options["Hash"].set_value(ttSize);
  Options["Threads"].set_value(threads);
  Options["OwnBook"].set_value("false");

  // Search should be limited by nodes, time or depth ?
  if (valType == "nodes")
      limits.maxNodes = atoi(valStr.c_str());
  else if (valType == "time")
      limits.maxTime = 1000 * atoi(valStr.c_str()); // maxTime is in ms
  else
      limits.maxDepth = atoi(valStr.c_str());

  // Do we need to load positions from a given FEN file ?
  if (fenFile != "default")
  {
      string fen;
      ifstream f(fenFile.c_str());

      if (f.is_open())
      {
          fenList.clear();

          while (getline(f, fen))
              if (!fen.empty())
                  fenList.push_back(fen);

          f.close();
      }
      else
      {
          cerr << "Unable to open FEN file " << fenFile << endl;
          exit(EXIT_FAILURE);
      }
  }

  // Ok, let's start the benchmark !
  totalNodes = 0;
  time = get_system_time();

  for (size_t i = 0; i < fenList.size(); i++)
  {
      Move moves[] = { MOVE_NONE };
#ifdef GPSFISH
      Position pos(fenList[i], 0);
#else
      Position pos(fenList[i], false, 0);
#endif

      cerr << "\nBench position: " << i + 1 << '/' << fenList.size() << endl;

      if (valType == "perft")
      {
          int64_t cnt = perft(pos, limits.maxDepth * ONE_PLY);
          totalNodes += cnt;

          cerr << "\nPerft " << limits.maxDepth << " nodes counted: " << cnt << endl;
      }
      else
      {
          if (!think(pos, limits, moves))
              break;

          totalNodes += pos.nodes_searched();
      }
  }

  time = get_system_time() - time;

  cerr << "\n==============================="
       << "\nTotal time (ms) : " << time
       << "\nNodes searched  : " << totalNodes
       << "\nNodes/second    : " << (int)(totalNodes / (time / 1000.0)) << endl << endl;

  // MS Visual C++ debug window always unconditionally closes when program
  // exits, this is bad because we want to read results before.
  #if (defined(WINDOWS) || defined(WIN32) || defined(WIN64))
  cerr << "Press any key to exit" << endl;
  cin >> time;
  #endif
}
