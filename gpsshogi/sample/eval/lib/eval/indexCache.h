/* indexCache.h
 */
#ifndef GPSSHOGI_INDEXCACHE_H
#define GPSSHOGI_INDEXCACHE_H

#include "osl/container.h"
#include <vector>
#include <algorithm>

#define L1BALL_NO_SORT

namespace gpsshogi
{
  template <size_t Capacity>
  struct IndexCache
  {
    osl::FixedCapacityVector<std::pair<int,double>,Capacity> indices;
    void add(int index, double coef) 
    {
      indices.push_back(std::make_pair(index, coef));
    }
    void output(std::vector<std::pair<int, double> >& out,
		size_t offset);
  };    

  template <size_t Capacity>
  struct IndexCacheI
  {
    osl::FixedCapacityVector<std::pair<int,int>,Capacity> indices;
    void add(int index, int coef) 
    {
      indices.push_back(std::make_pair(index, coef));
    }
    template <size_t Capacity2>
    void output(osl::FixedCapacityVector<std::pair<int, int>, Capacity2>& out,
		size_t offset=0);
    const std::pair<int, int>& operator[](size_t i) const 
    {
      return indices[i];
    }
    size_t size() const { return indices.size(); }
  };    
}

template <size_t Capacity>
void gpsshogi::
IndexCache<Capacity>::output(std::vector<std::pair<int, double> >& out,
			     size_t offset)
{
  for (size_t i=0; i < indices.size(); ++i)
    out.push_back(std::make_pair(indices[i].first+offset, indices[i].second));
  return;
}

template <size_t Capacity>
template <size_t Capacity2>
void gpsshogi::
IndexCacheI<Capacity>::output(osl::FixedCapacityVector<std::pair<int, int>, Capacity2>& out,
			      size_t offset)
{
  for (size_t i=0; i < indices.size(); ++i)
    out.push_back(std::make_pair(indices[i].first+offset, indices[i].second));
  return;
}

#endif /* GPSSHOGI_INDEXCACHE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
