/* instanceData.h
 */
#ifndef GPSSHOGI_INSTANCEDATA_H
#define GPSSHOGI_INSTANCEDATA_H
#include <vector>

namespace gpsshogi
{
  struct InstanceData
  {
    std::vector<unsigned int> index;
    std::vector<double> value;
    double y;

    size_t size() const { return index.size(); }
    void clear()
    {
      index.clear();
      value.clear();
    }
  };  
}

#endif /* GPSSHOGI_INSTANCEDATA_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
