/* usiSlave.h
 */
#ifndef GPSSHOGI_USISLAVE_H
#define GPSSHOGI_USISLAVE_H
#include "usiStatus.h"
#include "interimReport.h"
#include "osl/misc/milliSeconds.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <memory>
#include <functional>
#include <set>

namespace gpsshogi
{
  class SlaveManager;
  struct WorkSpace
  {
    InterimReport interim_report;
    std::function<void(InterimReport)> job_progress, job_finished;
    /** invoked when error */
    std::function<void(void)> fail_handler;
    /** message listener */
    std::function<void(std::string)> listener;
    std::string position;
    volatile int position_id;

    WorkSpace();
    ~WorkSpace();
    void reset(int owner, const std::string& position, int id,
	       std::function<void(InterimReport)> progress, 
	       std::function<void(InterimReport)> finished, 
	       std::function<void(std::string)> listener);
    void reset(int owner);

    std::function<void(void)> bindProgress();
    std::function<void(void)> bindFinished();
    void failed();
  };
  struct SlaveStatus
  {
    osl::time_point last_access;
    volatile uint64_t node_count;
    volatile double elapsed;
    volatile UsiStatus usi;
    volatile bool info_update_needed, timer_waiting;

    SlaveStatus();
    double nps() const;
    void update(const InterimReport&);
  };
  class UsiSlave : public std::enable_shared_from_this<UsiSlave>
  {
  private:
    friend class SlaveManager;
    boost::asio::ip::tcp::socket socket;
    boost::asio::streambuf buffer_r, buffer_w;
    boost::asio::io_service &usi_queue, &local_queue;
    boost::asio::strand strand;
    SlaveManager &manager;
    boost::system::error_code last_error;
    std::set<std::string> options;
    WorkSpace work_space;
    SlaveStatus status;
    const int slave_id;
    std::string message_waiting;
    volatile bool sending_message;
    std::string client_name, multi_pv_key;
    std::unique_ptr<std::ostream> local_log;
    int slave_tlp;
    boost::asio::deadline_timer timer;
  public:
    UsiSlave(SlaveManager &,
	     boost::asio::io_service &io, boost::asio::io_service &u,
	     int slave_id, const std::string& client_log_dir);
    virtual ~UsiSlave();
    // initialization sequence
    void setSlaveTLP(int n) { slave_tlp = n; }
    void start();
    void sentUsi(const boost::system::error_code& ec);
    void handleOptions(const boost::system::error_code& ec);
    void sentIsReady(const boost::system::error_code& ec);
    void handleReadyOK(const boost::system::error_code& ec);
    void sentInitialGo(const boost::system::error_code& ec);
    void handleInitialBestMove(const boost::system::error_code& ec);
    void updateInfoByTimer();
    
    // steady state
    void handleMessage(const boost::system::error_code& ec);
    // stop
    /** @return stop sent */
    bool stop(int go_id);

    // quit
    /** usi thread */
    void prepareQuit();
    void sentQuit(const boost::system::error_code& ec);

    UsiStatus usiStatus() const { return status.usi; }
    /** usi thread */
    void goMate(int id, const std::string& position, int millisec,
		std::function<void(std::string)> finish_handler);
    void handleMateMessage(const std::string& msg);
    static void mateResult(std::function<void(std::string)> output,
			   InterimReport report);

    /** usi thread */
    void go(int go_id, const std::string& position, 
	    const std::string& ignore_moves, int millisec,
	    std::function<void(InterimReport)> progress_handler,
	    std::function<void(InterimReport)> finish_handler,
	    int draw_value, int multi_pv=1);
    void handleSearchMessage(const std::string& msg);

    /** usi thread */
    void echoIfIdle(osl::time_point now);
    void send(const std::string& msg);
    void sendLocal(const std::string& msg);

    int id() const { return slave_id; }
    const std::string& name() const { return client_name; }
    void messageSent(std::string msg,
		     const boost::system::error_code& ec);
    double nps() const { return status.nps(); }
    const std::string idMark() const;
    const boost::system::error_code& error() const 
    {
      return last_error;
    }
    bool isMateSolver() const 
    {
      return name().find(mate_solver_key) != std::string::npos;
    }
    static void setMateSolverKey(const std::string& new_key) {
      mate_solver_key = new_key;
    }
  private:
    void finishedGoOrGoMate(const std::string& line);
    static std::string mate_solver_key;
  };
  typedef std::shared_ptr<UsiSlave> UsiSlavePtr;
}


#endif /* GPSSHOGI_USISLAVE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
