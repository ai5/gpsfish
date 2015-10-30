/* jobBuffer.h
 */
#ifndef _JOBBUFFER_H
#define _JOBBUFFER_H

#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/scoped_array.hpp>
#include <iostream>
/**
 * thread safe な学習用データの受け渡しbuffer
 *
 * 入れた順で消費することは仕様
 * 余計なnsleep は不安定な FreeBSD の libthr のために必要 
 */
template <typename Data, size_t MaxBufSize>
struct JobBuffer
{
public:
  typedef Data data_t;
  static const size_t max_buf_size = MaxBufSize;
private:
  data_t circular_buf[max_buf_size];
  size_t begin, end, buffered;
  boost::mutex monitor;
  boost::condition buffer_not_full, buffer_not_empty;
  size_t in, out;
public:
  JobBuffer() : begin(0), end(0), buffered(0), in(0), out(0)
  {
  }
  ~JobBuffer()
  {
    std::cerr << "buffer in " << in << " out " << out << "\n";
  }
  static void nsleep(int nsec)
  {
    boost::xtime xt;
    boost::xtime_get(&xt, boost::TIME_UTC);
    xt.nsec += nsec;
    boost::thread::sleep(xt);
  }
  static void redundantNsleep(int nsec)
  {
#ifdef FRAGILE_LOCK
    nsleep(nsec);
#endif
  }
  void send(data_t e)
  {
    {
      redundantNsleep(281);
      boost::mutex::scoped_lock lk(monitor);
      while (buffered == max_buf_size)
	buffer_not_full.wait(lk);
      circular_buf[end] = e;
      end = (end+1) % max_buf_size;
      ++buffered;
      // std::cerr << "+ " << e << "\n";
    }
    redundantNsleep(410);
    buffer_not_empty.notify_one();
    redundantNsleep(255);
    ++in;
  }
  data_t receive(bool block=true)
  {
    if ((buffered == 0)
	&& (! block))
      return 0;
    data_t result;
    {
      redundantNsleep(503);
      boost::mutex::scoped_lock lk(monitor);
      while (buffered == 0)
	buffer_not_empty.wait(lk);
      result = circular_buf[begin];
      begin = (begin+1) % max_buf_size;
      --buffered;
      // std::cerr << "- " << result << "\n";
    }
    redundantNsleep(287);
    buffer_not_full.notify_one();
    redundantNsleep(127);
    ++out;
    return result;
  }
};



#endif /* _JOBBUFFER_H */

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
