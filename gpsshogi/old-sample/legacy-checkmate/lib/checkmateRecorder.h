/* checkmateRecorder.h
 */
#ifndef _CHECKMATERECORDER_H
#define _CHECKMATERECORDER_H

#include "osl/move.h"
#include <iosfwd>

namespace osl
{
  namespace state
  {
    class SimpleState;
  }
  namespace hash
  {
    class HashKey;
  }
  class PathEncoding;
  namespace checkmate
  {
    class CheckHashRecord;
    class CheckMove;
    using state::SimpleState;
    using hash::HashKey;
    /**
     * 詰将棋の記録を取るクラス.
     * 現在は全てクラスメソッド．並列化の際は， CheckmateSearcher* から
     * CheckmateRecorder を選ぶしかけが必要?
     */
    class CheckmateRecorder
    {
    public:
      static CheckMove *nextMove;
      static const char *leaveReason;
      static const SimpleState *currentState;

      static std::ostream& stream();
      static void dumpStack();
      static void dumpRecord(const SimpleState& state,
			     const CheckHashRecord *record, 
			     unsigned int proofLimit,
			     unsigned int disproofLimit,
			     unsigned int currentProofNumber,
			     unsigned int currentDisproofNumber);

      static void stat(const char *msg, Player P, unsigned int tableSize,
		       unsigned int totalNodeCount, unsigned int totalNodeLimit);
      /** root での情報を記録 */
      static void rootLog(
#ifdef CHECKMATE_DEBUG
	const char *msg, unsigned int tableSize,
	unsigned int continuousNoExpandLoop
#else
	const char *, unsigned int, unsigned int 
#endif
	)
      {
#ifdef CHECKMATE_DEBUG
	writeRootLog(msg, tableSize, continuousNoExpandLoop);
#endif
      }
      static void writeRootLog(const char *msg, unsigned int tableSize,
			       unsigned int continuousNoExpandLoop);
      /** attack/defense に入る時記録 */
      static void enter(int depth, const char *name, 
			CheckHashRecord *record, const HashKey& key,
			const PathEncoding& path,
			unsigned int proofLimit, unsigned int disproofLimit);
      /** attack/defense から帰る時記録 */
      static void leave(int depth, const char *name, 
			const CheckHashRecord *record);
      static void setNextMove(CheckMove *
#ifdef CHECKMATE_DEBUG
			      m
#endif
	)
      {
#ifdef CHECKMATE_DEBUG
	nextMove = m;
#endif
      }
      static void setLeaveReason(const char *
#ifdef CHECKMATE_DEBUG
				 reason
#endif
	)
      {
#ifdef CHECKMATE_DEBUG
	leaveReason = reason;
#endif
      }
      static void setState(const SimpleState *
#ifdef CHECKMATE_DEBUG
			   state
#endif
	)
      {
#ifdef CHECKMATE_DEBUG
	currentState = state;
#endif
      }
      struct DepthTracer
      {
	static int depth;
	static int maxLogDepth;
	static int maxVerboseLogDepth;
	static std::ostream& stream();

	CheckHashRecord *record;
	const char *name;
	DepthTracer(const char *n, 
		    CheckHashRecord *r,
		    const HashKey& key,
		    const PathEncoding& path,
		    unsigned int proofLimit,
		    unsigned int disproofLimit)
	  : record(r), name(n)
	{
	  ++depth;
	  if (depth < maxLogDepth)
	    enter(depth, name, record, key, path, proofLimit, disproofLimit);
	}
	~DepthTracer()
	{
	  if (depth < maxLogDepth)
	  {
	    leave(depth, name, record);
	  }
	  depth--;
	}
      };
      struct NullTracer
      {
	NullTracer(const char *, const CheckHashRecord *,
		   const HashKey&, const PathEncoding&,
		   unsigned int, unsigned int)
	{
	}
	~NullTracer()
	{
	}
      };
#ifdef CHECKMATE_DEBUG
      typedef DepthTracer Tracer;
#else
      typedef NullTracer Tracer;
#endif
    };  
  } // namespace checkmate
} // namespace osl

#endif /* _CHECKMATERECORDER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
