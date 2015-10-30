/* moveData.h
 */
#ifndef GPSSHOGI_MOVEDATA_H
#define GPSSHOGI_MOVEDATA_H
#include <vector>
#include <iosfwd>

namespace gpsshogi
{
  struct MoveData
  {
    int value, progress;
    std::vector<std::pair<int,double> > diffs;
    explicit MoveData() : value(0), progress(0)
    {
    }
    void clear()
    {
      value = progress = 0;
      diffs.clear();
    }
    void reserve(size_t capacity)
    {
      diffs.reserve(capacity);
    }
  };
  std::ostream& operator<<(std::ostream& os, const MoveData& md);
}


#endif /* GPSSHOGI_MOVEDATA_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
