/* coordinator.h
 */
#ifndef GPSSHOGI_COORDINATOR_H
#define GPSSHOGI_COORDINATOR_H
#include "upStream.h"
#include "slaveManager.h"
#include "timeCondition.h"
#include <boost/asio/deadline_timer.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <thread>
#include <functional>
#include <fstream>
namespace gpsshogi
{
  class SearchTree;
  class Coordinator
  {
    boost::asio::io_service slave_io, search_io;
    boost::asio::deadline_timer timer;
    std::unique_ptr<UpStream> upstream;
    SlaveManager manager;
    std::unique_ptr<boost::asio::io_service::work> work_u, work_c;
    boost::ptr_vector<std::thread> threads;
    std::map<int,std::function<void(void)> > stop_handler;
    int expected_slaves, parallel_io;
    std::unique_ptr<std::ostream> log_file, log_file_in_game;
    std::unique_ptr<SearchTree> tree;
    int last_position;
    volatile bool quitting;
  public:
    Coordinator(UpStream *, int expected_slaves, int parallel_io=2,
		const std::string& udp_dst="",
		bool slave_udp=false,
		const std::string& slave_log_dir="",
		int slave_tlp=0);
    ~Coordinator();
    void start();
    // go mate
    void handleGoMate(int position_id, std::string position, int millisec);
    void mateResult(int position_id, std::string msg);
    // go
    void handleGo(int position_id, std::string position, int millisec);
    void searchResult(int position_id, std::string msg);

    /** go in game */
    void handleGoInGame(int position_id, std::string position,
			const TimeCondition& seconds);
    /** set new root move from the previous positon */
    void moveRoot(std::string move);
    void newGameStart();
    void gameFinished();

    // misc
    void showStatus(std::ostream&);
    void handleStop(int position_id);
    void waitReady();
    void prepareQuit();
    void handleTimer(const boost::system::error_code& ec);
  private:
    std::function<void(void)> 
    assignGo(int id, const std::string& position, 
	     std::function<void(std::string)> progress,
	     std::function<void(std::string)> finish,
	     const TimeCondition& seconds);
  };
}


#endif /* GPSSHOGI_COORDINATOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
