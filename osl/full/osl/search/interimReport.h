/* interimReport.h
 */
#ifndef OSL_INTERIMREPORT_H
#define OSL_INTERIMREPORT_H

#include "osl/basic_type.h"
#include <functional>
#include <atomic>
#include <string>
#include <vector>
#include <map>
namespace osl
{
  namespace search
  {
    constexpr int usi_pawn_value = 100;
    constexpr int usi_win_value = usi_pawn_value*300;
    struct PVInfo
    {
      volatile int depth, score;
      volatile double elapsed;
      std::vector<std::string> moves;
      PVInfo() : depth(0), score(0), elapsed(0.0) {}

      void clear() { moves.clear(); }
      void push_back(const std::string& m) { moves.push_back(m); }
      bool empty() const { return moves.empty(); }
      size_t size() const { return moves.size(); }
      const std::string& operator[](size_t i) const {
	assert(i<size());
	return moves[i]; 
      }
    };
    typedef std::map<std::string,PVInfo> pv_table;
    struct InterimReport
    {
      int owner;
      PVInfo pv;
      pv_table alternatives;
      std::string last_string, result_line;
      /** sign of best_value: relative in UsiSlave, fixed in SearchNode */
      volatile int depth_head;
      volatile int64_t node_count;
      volatile double elapsed;
      volatile bool stopped, aborted, last_message_ignored;

      explicit InterimReport(int owner=-1);
      ~InterimReport();
      /** @return importance on search tree */
      bool updateByInfo(const std::string& line, int id);
      void finished(const std::string& line);

      std::string composeInfo(bool negate_score=false) const;
      std::string joinPV() const;
      std::string makeSearchResult() const;

      void set(osl::Player turn, const InterimReport&);
      bool finishedNormally() const {
	return ! stopped && ! aborted;
      }
      int bestValue() const { return pv.score; }

      static std::function<void(std::string)> info, warn, error;
    };
  }
}

#endif /* OSL_INTERIMREPORT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
