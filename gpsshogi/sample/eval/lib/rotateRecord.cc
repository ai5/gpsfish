/* rotateRecord.cc
 */
#include "rotateRecord.h"
#include "osl/record/kisen.h"
#include "osl/stat/average.h"
#include <cassert>
#include <iostream>

gpsshogi::RotateRecord::
RotateRecord(const std::string& src, int w, const std::string& work, int start)
  : src_filename(src), work_filename(work), window(w), cur(start)
{
  osl::record::KisenFile kisen(src_filename);
  if ((int)kisen.size() < window) {
    std::cerr << "warning kisen.size() is smaller than window " 
	      << kisen.size() << " " << window << "\n";
    window = kisen.size();
  }
  if (start >= (int)kisen.size()) {
    std::cerr << "warning kisen.size() is smaller than start " 
	      << kisen.size() << " " << start << "\n";
    start = 0;
  }

  std::ofstream os(work_filename.c_str());
  osl::KisenWriter out(os);
  for (int i=0; i<window; ++i) {
    auto moves = kisen.moves((i+start)%kisen.size());
    out.save({kisen.initialState(), moves});
  }
  cur = start+window;
}
gpsshogi::RotateRecord::
~RotateRecord()
{
}

void gpsshogi::RotateRecord::
rotate(const std::vector<std::tuple<int,double> >& all, double average)
{
  assert((int)all.size() == window);
  std::vector<int> used;
  used.reserve(window);
  for (size_t i=0; i<all.size(); ++i)
    if (std::get<1>(all[i]) > average)
      used.push_back(std::get<0>(all[i]));
  std::sort(used.begin(), used.end());
  osl::record::KisenFile kisen(src_filename);
  int sorted = used.size();
  for ( ; (int)used.size() < window; ++cur) {
    if (cur >= (int)kisen.size())
      cur %= kisen.size();
    if (std::binary_search(used.begin(), used.begin()+sorted, cur))
      continue;
    used.push_back(cur);
  }
  std::random_shuffle(used.begin(), used.end());
  std::ofstream os(work_filename.c_str());
  osl::KisenWriter out(os);
  osl::Average average_moves;
  for (int i=0; i<window; ++i) {
    auto moves = kisen.moves(used[i]);
    out.save({kisen.initialState(), moves});
    average_moves.add(moves.size());
  }
  std::cerr << "  preserved " << sorted << " records, add " << window - sorted << " records"
	    << ", cur " << cur << ", moves " << average_moves.average() << "\n";  
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
