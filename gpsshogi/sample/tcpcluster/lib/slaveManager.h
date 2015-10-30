/* SlaveManager.h
 */
#ifndef GPSSHOGI_SLAVEMANAGER_H
#define GPSSHOGI_SLAVEMANAGER_H

#include "usiSlave.h"
#include "timeCondition.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <mutex>
#include <condition_variable>
#include <functional>
namespace gpsshogi
{
  class SlaveManager
  {
    boost::asio::io_service &io, &usi_queue;  
    boost::asio::ip::tcp::acceptor acceptor;
    std::vector<UsiSlavePtr> connections, active, active_mate;
    osl::time_point last_ping;
    std::string client_log_dir;
    volatile int num_slave;
    volatile bool quitting;
    /** if > 0, force #threads for each slave. */
    int slave_tlp;
    std::set<UsiSlave*> dead_slave;
    double nps_sum;
  public:
    SlaveManager(boost::asio::io_service &i,
		 boost::asio::io_service &u,
		 const std::string& client_log_dir,
		 int slave_tlp=0);
    ~SlaveManager();

    void handleAccept(const boost::system::error_code& ec,
		      UsiSlavePtr slave);
    void addActiveSlave(UsiSlavePtr slave);
    void addActiveMateSolver(UsiSlavePtr slave);
    void removeSlave(UsiSlave *slave);
    /** block until specified number of clients become ready */
    void waitSlaves(int number);

    std::vector<UsiSlavePtr> idleSlavesInQueue() const 
    {
      return selectIdleInQueue(active, dead_slave); 
    }
    std::vector<UsiSlavePtr> idleMateSolversInQueue() const
    {
      return selectIdleInQueue(active_mate, dead_slave); 
    }

    void showStatus(std::ostream&);
    void prepareQuit();
    void periodic();
    int slavesEstimated() const { return num_slave; }

    static void acceptMultipleSlaves(const std::string& filename);
    bool allowMultipleLogin(const boost::asio::ip::address& address);
    double npsSum() const { return nps_sum; }

    mutable std::mutex mutex;
    std::condition_variable condition;
  private:
    void newAccept();
    void removeSlaveInQueue(UsiSlave *slave);
    void addActiveSlaveInQueue(UsiSlavePtr slave);
    void addActiveMateSolverInQueue(UsiSlavePtr slave);
    bool sourceAlreadyUsed(const boost::asio::ip::address& address) const;
    static std::vector<UsiSlavePtr> selectIdleInQueue
    (const std::vector<UsiSlavePtr>&, const std::set<UsiSlave*>&); 
  };
}

#endif /* GPSSHOGI_SLAVEMANAGER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
