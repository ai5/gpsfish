/* threatmateState.h
 */
#ifndef SEARCH_THREATMATESTATE_H
#define SEARCH_THREATMATESTATE_H

#include "osl/basic_type.h"
#ifdef OSL_SMP
#  include "osl/misc/lightMutex.h"
#endif
#include "osl/container.h"
#include <iosfwd>
namespace osl
{
  namespace search
  {
    struct ThreatmateStateData
    {
      char current_status;
    };
    class DualThreatmateState;
    /**
     * 詰めろを考慮した詰将棋探索のための状態管理.
     * http://www31.ocn.ne.jp/~kfend/inside_kfend/ptc.html#c3
     *
     * - (MAYBE_)THREATMATE --(check)--> CHECK_AFTER_THREATMATE 
     * - (MAYBE_)THREATMATE --(no-check)--> MAY_HAVE_CHECKMATE
     * - CHECK_AFTER_THREATMATE --(escape)-->  MAYBE_THREATMATE
     */
    class ThreatmateState : protected ThreatmateStateData
    {
      friend class DualThreatmateState;
    public:
      enum Status {
	UNKNOWN = 0,
	/** threatmate found by checkmate search */
	THREATMATE,
	/** threatmate, not sure */
	MAYBE_THREATMATE,
	/** status after threatmate responded by check */
	CHECK_AFTER_THREATMATE,
	/** status after threatmate responded by non-check move */
	MAY_HAVE_CHECKMATE,
      };
    private:
      static const CArray<Status,5*2> transition;
    public:
      ThreatmateState(Status s=UNKNOWN)
      {
	current_status = s;
      }
      void setThreatmate(Status s) { 
	current_status = s; 
      }
      bool isUnknown() const { 
	return current_status == UNKNOWN; 
      }
      bool isThreatmate() const { 
	return current_status == THREATMATE; 
      }
      bool maybeThreatmate() const { 
	return (current_status == THREATMATE) 
	  || (current_status == MAYBE_THREATMATE);
      }
      bool mayHaveCheckmate() const {
	return current_status == MAY_HAVE_CHECKMATE; 
      }
      Status status() const { 
	return static_cast<Status>(current_status); 
      }
      const ThreatmateState newStatus(bool is_check) const
      {
	return transition[current_status*2+is_check];
      }

      void update(const ThreatmateState *parent, bool in_check)
      {
	if (maybeThreatmate() || ! parent)
	  return;
	const ThreatmateState new_status = parent->newStatus(in_check);
	  *this = new_status;
      }
    };
    std::ostream& operator<<(std::ostream&, ThreatmateState);
  } // namespace search
} // osl

#endif /* SEARCH_THREATMATESTATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
