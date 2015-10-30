/* rotateRecord.h
 */
#ifndef _ROTATERECORD_H
#define _ROTATERECORD_H

#include <boost/noncopyable.hpp>
#include <string>
#include <vector>

namespace gpsshogi
{
  class RotateRecord : boost::noncopyable
  {
    std::string src_filename, work_filename;
    int window, cur;
  public:
    RotateRecord(const std::string& src, int window, const std::string& work, int start=0);
    ~RotateRecord();

    void rotate(const std::vector<std::tuple<int,double> >& all, double average);
  };
}

#endif /* _ROTATERECORD_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
