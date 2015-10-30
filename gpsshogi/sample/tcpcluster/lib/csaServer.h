/* csaServer.h
 */
#ifndef GPSHSOGI_CSASERVER_H
#define GPSHSOGI_CSASERVER_H
#include "upStream.h"
#include "interimReport.h"
#include "osl/state/historyState.h"
#include "osl/misc/milliSeconds.h"
#include <memory>
#include <tuple>
#include <vector>
#include <iostream>
namespace gpsshogi
{
  struct Account
  {
    std::string username, password;
    int limit_seconds, limit_byoyomi;
    Account() : limit_seconds(-1), limit_byoyomi(-1) {
    }
  };
  /**
   * - single login (tcp/ip)
   * - stdio
   * - multi login (tcp/ip)  --- experimental
   */
  struct CsaConfig
  {
    std::string servername;
    int portnumber;
    std::string username, password;
    std::string stdio_config;
    std::vector<Account> multi_login;
    int games;
    int book_width_root, book_width, book_depth;
    int limit_seconds, limit_byoyomi;
    bool ponder, send_info;
    int wcsc;
    CsaConfig() : portnumber(4081), games(1),
		  book_width_root(16), book_width(10), book_depth(35),
		  limit_seconds(-1), limit_byoyomi(-1),
		  ponder(false), send_info(false), wcsc(0)
    {
    }
  };
  struct CsaGameCondition
  {
    osl::NumEffectState initial_state;
    std::vector<osl::Move> initial_moves;
    osl::Player my_turn;    
    std::string csa_filename;
    int seconds, byoyomi;
    std::string csalines;
    std::unique_ptr<std::ostream> csafile;
    osl::HistoryState state;
    osl::CArray<int,2> time_used;

    CsaGameCondition()
      : my_turn(osl::BLACK), seconds(0), byoyomi(0)
    {
      time_used.fill(0);
    }
    void addToCsaFile(const std::string& msg);
    void csaFileFlush();
  };
  class CsaServer : public UpStream
  {
  public:
    class Connection
    {
    public:
      virtual ~Connection();
      virtual bool loginWithRetry(const CsaConfig& config)=0;
      virtual bool readLine(std::string&)=0;
      virtual void writeLine(const std::string&)=0;
    };
    class TcpIp;
    class MultiTcpIp;
    class StdIO;
    class FileStream;
  private:
    std::unique_ptr<Connection> connection;
    std::vector<std::string> go_positions;
    std::vector<osl::Move> bestmove_history;
    bool verbose;
    CsaConfig config;
    CsaGameCondition game;
    InterimReport info;
    std::string last_pv;
    osl::Move my_last_move;
    int my_last_score;
    std::string deffered_input;
    std::vector<std::tuple<int,osl::Move,int> > game_history;
    osl::time_point last_move_time;
  public:
    explicit CsaServer(const CsaConfig&);
    ~CsaServer();
    void start();
    void playGames(int num_games);

    void outputSearchProgress(int position_id, const std::string& msg);
    void outputSearchResult(int position_id, const std::string& msg);
  private:
    bool waitOpponent(int game);
    void logDisconnected();
    bool makeMoveFromServer();
    osl::Move searchBestMove();
  };

}

#endif /* GPSHSOGI_CSASERVER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
