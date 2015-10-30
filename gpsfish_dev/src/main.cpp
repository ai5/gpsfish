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

// To profile with callgrind uncomment following line
//#define USE_CALLGRIND

#include <cstdio>
#include <iostream>
#include <string>

#ifndef GPSFISH
#include "bitboard.h"
#include "evaluate.h"
#include "osl/oslConfig.h"
#endif
#include "position.h"
#include "thread.h"
#include "search.h"
#include "ucioption.h"
#ifdef GPSFISH
#include <cstdlib>
#ifndef _WIN32
#include <netdb.h>
#endif
#include <cstring>
#endif

#ifdef USE_CALLGRIND
#include <valgrind/callgrind.h>
#endif

using namespace std;

extern bool execute_uci_command(const string& cmd);
extern void benchmark(int argc, char* argv[]);
extern void init_kpk_bitbase();

int main(int argc, char* argv[]) {

  // Disable IO buffering for C and C++ standard libraries
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  cout.rdbuf()->pubsetbuf(NULL, 0);
  cin.rdbuf()->pubsetbuf(NULL, 0);
#if defined(GPSFISH) && !defined(_WIN32)
  if(argc==2 && !strncmp(argv[1],"tcp:",4)){
    std::string s(&argv[1][4]);
    std::string::iterator it=find(s.begin(),s.end(),':');
    if(it==s.end()){
      cerr << "tcp:hostname:port" << endl;
      exit(1);
    }
    std::string hostname(s.begin(),it),portstr(it+1,s.end());
    int portnum=atoi(portstr.c_str());
    int sock_fd=socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd<0){
      cerr << "failed make socket" << endl;
      exit(1);
    }
    struct sockaddr_in sa;
    sa.sin_family=AF_INET;
    sa.sin_port=htons(portnum);
    struct hostent *host;
    host = gethostbyname(hostname.c_str());
    if (host == 0 ) {
      cerr << "Failed gethostbyname" << endl;
    }
    memcpy(&(sa.sin_addr.s_addr),host->h_addr,host->h_length);
    if(connect(sock_fd,(const sockaddr*)&sa,sizeof(sa))<0){
      cerr << "failed connect" << endl;
      exit(1);
    }
    dup2(sock_fd,0);
    dup2(sock_fd,1);
    argc--;
    using_tcp_connection = true;
  }    
#endif

  // Startup initializations
#ifdef GPSFISH
  if (const char *env = getenv("GPSFISH_HOME"))
    osl::OslConfig::home(env);
#  ifdef GPSFISH_HOME
  osl::OslConfig::home(GPSFISH_HOME);
#  endif
#endif
#ifndef GPSFISH
  init_bitboards();
#endif
  Position::init_zobrist();
#ifndef GPSFISH
  Position::init_piece_square_tables();
  init_kpk_bitbase();
#endif
  init_search();
  Threads.init();

#ifdef USE_CALLGRIND
  CALLGRIND_START_INSTRUMENTATION;
#endif

  if (argc < 2)
  {
      // Print copyright notice
      cout << engine_name() << " by " << engine_authors() << endl;

      if (CpuHasPOPCNT)
          cout << "Good! CPU has hardware POPCNT." << endl;

      // Wait for a command from the user, and passes this command to
      // execute_uci_command() and also intercepts EOF from stdin to
      // ensure that we exit gracefully if the GUI dies unexpectedly.
      string cmd;
      while (getline(cin, cmd) && execute_uci_command(cmd)) {}
  }
  else if (string(argv[1]) == "bench" && argc < 8)
      benchmark(argc, argv);
  else
      cout << "Usage: stockfish bench [hash size = 128] [threads = 1] "
           << "[limit = 12] [fen positions file = default] "
           << "[limited by depth, time, nodes or perft = depth]" << endl;

  Threads.exit();
  return 0;
}
