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

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include "evaluate.h"
#include "misc.h"
#include "move.h"
#include "position.h"
#include "search.h"
#include "ucioption.h"
#ifdef GPSFISH
#include "movegen.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include "osl/enterKing.h"
#include <vector>
#endif

using namespace std;

namespace {

  // FEN string for the initial position
#ifdef GPSFISH
  const string StartPositionFEN = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
#else
  const string StartPositionFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
#endif

  // UCIParser is a class for parsing UCI input. The class
  // is actually a string stream built on a given input string.
  typedef istringstream UCIParser;

  void set_option(UCIParser& up);
  void set_position(Position& pos, UCIParser& up);
  bool go(Position& pos, UCIParser& up);
  void perft(Position& pos, UCIParser& up);
}
#ifdef GPSFISH
std::vector<Move> ignore_moves;
#endif


/// execute_uci_command() takes a string as input, uses a UCIParser
/// object to parse this text string as a UCI command, and calls
/// the appropriate functions. In addition to the UCI commands,
/// the function also supports a few debug commands.

bool execute_uci_command(const string& cmd) {

#ifdef GPSFISH
  static Position pos(StartPositionFEN, 0); // The root position
#else
  static Position pos(StartPositionFEN, false, 0); // The root position
#endif

  UCIParser up(cmd);
  string token;

  up >> token; // operator>>() skips any whitespace

  if (token == "quit")
    return false;

  if (token == "go")
    return go(pos, up);

#ifdef GPSFISH
  if (token == "usinewgame")
    pos.from_fen(StartPositionFEN);
#else
  if (token == "ucinewgame")
    pos.from_fen(StartPositionFEN, false);
#endif

  else if (token == "isready") {
#ifdef GPSFISH
    bool ok = osl::eval::ml::OpenMidEndingEval::setUp();
    ok &= osl::progress::ml::NewProgress::setUp();
    if (! ok) {
      std::cerr << "set up failed\n";
      return false;
    }
#endif
    cout << "readyok" << endl;
  }

    else if (token == "position")
      set_position(pos, up);

    else if (token == "setoption")
      set_option(up);

    else if (token == "perft")
      perft(pos, up);

    else if (token == "d")
      pos.print();

#ifndef GPSFISH
    else if (token == "flip")
      pos.flip();
#endif

#ifndef GPSFISH
    else if (token == "eval")
    {
      read_evaluation_uci_options(pos.side_to_move());
      cout << trace_evaluate(pos) << endl;
    }
#endif
    else if (token == "key")
#ifdef GPSFISH
      cout << "key: " << hex     << pos.get_key() << endl;
#else
    cout << "key: " << hex     << pos.get_key()
	 << "\nmaterial key: " << pos.get_material_key()
	 << "\npawn key: "     << pos.get_pawn_key() << endl;
#endif

#ifdef GPSFISH
    else if ( token == "ignore_moves"){
      ignore_moves.clear();
      while(up >> token) ignore_moves.push_back(move_from_uci(pos, token));
    }
#endif    
#ifdef GPSFISH
    else if (token == "usi")
#else
    else if (token == "uci")
#endif
      cout << "id name "     << engine_name()
           << "\nid author " << engine_authors()
#ifdef GPSFISH
  << Options.print_all()
           << "\nusiok"      << endl;
#else
  << "\n"           << Options.print_all();
#endif
#ifdef GPSFISH
    else if (token == "stop"){
    }
    else if (token == "echo"){
      up >> token;
      cout << token << endl;
    }
#endif
    else if (token == "show_tree"){
      show_tree(pos);
    }
    else
      cout << "Unknown command: " << cmd << endl;

    return true;
  }


  namespace {

    // set_position() is called when engine receives the "position" UCI
    // command. The function sets up the position described in the given
    // fen string ("fen") or the starting position ("startpos") and then
    // makes the moves given in the following move list ("moves").

    void set_position(Position& pos, UCIParser& up) {

      string token, fen;

#ifdef GPSFISH
      ignore_moves.clear();
#endif
      up >> token; // operator>>() skips any whitespace

      if (token == "startpos")
      {
#ifdef GPSFISH
        pos.from_fen(StartPositionFEN);
#else
        pos.from_fen(StartPositionFEN, false);
#endif
        up >> token; // Consume "moves" token if any
      }
#ifdef GPSFISH
      else if (token == "sfen")
#else
      else if (token == "fen")
#endif
      {
        while (up >> token && token != "moves")
	  fen += token + " ";

#ifdef GPSFISH
        pos.from_fen(fen);
#else
        pos.from_fen(fen, Options["UCI_Chess960"].value<bool>());
#endif
      }
      else return;

      // Parse move list (if any)
      while (up >> token)
        pos.do_setup_move(move_from_uci(pos, token));
#ifdef GPSFISH
      assert(pos.eval_is_ok());
#endif
    }


    // set_option() is called when engine receives the "setoption" UCI
    // command. The function updates the corresponding UCI option ("name")
    // to the given value ("value").

    void set_option(UCIParser& up) {

      string token, name;
      string value = "true"; // UCI buttons don't have a "value" field

      up >> token; // Consume "name" token
      up >> name;  // Read option name

      // Handle names with included spaces
      while (up >> token && token != "value")
        name += " " + token;

      up >> value; // Read option value

      // Handle values with included spaces
      while (up >> token)
        value += " " + token;

      if (Options.find(name) != Options.end())
        Options[name].set_value(value);
      else
        cout << "No such option: " << name << endl;
    }


    // go() is called when engine receives the "go" UCI command. The
    // function sets the thinking time and other parameters from the input
    // string, and then calls think(). Returns false if a quit command
    // is received while thinking, true otherwise.

    bool go(Position& pos, UCIParser& up) {

      string token;
      SearchLimits limits;
      Move searchMoves[MAX_MOVES], *cur = searchMoves;
#ifdef GPSFISH
      osl::CArray<int,2> time={{0,0}},inc={{0,0}};
#else
      int time[] = { 0, 0 }, inc[] = { 0, 0 };
#endif

      while (up >> token)
      {
        if (token == "infinite")
	  limits.infinite = true;
        else if (token == "ponder")
	  limits.ponder = true;
        else if (token == "wtime")
	  up >> time[WHITE];
        else if (token == "btime")
	  up >> time[BLACK];
        else if (token == "winc")
	  up >> inc[WHITE];
        else if (token == "binc")
	  up >> inc[BLACK];
        else if (token == "movestogo")
	  up >> limits.movesToGo;
        else if (token == "depth")
	  up >> limits.maxDepth;
        else if (token == "nodes")
	  up >> limits.maxNodes;
#ifdef GPSFISH
        else if (token == "movetime" || token=="byoyomi")
	  up >> limits.maxTime;
        else if (token == "mate"){
	  int mateTime;
	  up >> mateTime;
	  do_checkmate(pos, mateTime);
	  return true;
	}
#else
        else if (token == "movetime")
	  up >> limits.maxTime;
#endif
        else if (token == "searchmoves")
	  while (up >> token)
	    *cur++ = move_from_uci(pos, token);
      }

      *cur = MOVE_NONE;
      limits.time = time[pos.side_to_move()];
      limits.increment = inc[pos.side_to_move()];

      assert(pos.is_ok());

#ifdef GPSFISH
      if(searchMoves == cur && !ignore_moves.empty()){
	MoveStack mlist[MAX_MOVES];
	MoveStack* last = generate<MV_PSEUDO_LEGAL>(pos, mlist);
	for(MoveStack* mp=mlist;mp<last;mp++){
	  if(find(ignore_moves.begin(),ignore_moves.end(),mp->move)==ignore_moves.end()){
	    *cur++= mp->move;
	  }
	}
	*cur = MOVE_NONE;
      }
      ignore_moves.clear();
      if(!using_tcp_connection
	 && osl::EnterKing::canDeclareWin(pos.osl_state)){
	cout << "bestmove win" << endl;
	return true;
      }
#endif
      return think(pos, limits, searchMoves);
    }


    // perft() is called when engine receives the "perft" command.
    // The function calls perft() passing the required search depth
    // then prints counted leaf nodes and elapsed time.

    void perft(Position& pos, UCIParser& up) {

      int depth, time;
      int64_t n;

      if (!(up >> depth))
        return;

      time = get_system_time();

      n = perft(pos, depth * ONE_PLY);

      time = get_system_time() - time;

      std::cout << "\nNodes " << n
		<< "\nTime (ms) " << time
		<< "\nNodes/second " << int(n / (time / 1000.0)) << std::endl;
    }
  }
