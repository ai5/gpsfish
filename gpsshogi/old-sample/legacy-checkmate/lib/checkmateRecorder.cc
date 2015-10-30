/* checkmateRecorder.cc
 */
#include "checkmateRecorder.h"
#include "checkHashRecord.h"
#include "checkMoveList.h"
#include "checkStack.h"
#include "osl/state/simpleState.h"
#include "osl/record/csa.h"
#include <string>
#include <iostream>
#include <fstream>
#include <signal.h>

#ifndef MINIMAL

/** この値は必要に応じて編集する */
int osl::checkmate::CheckmateRecorder::DepthTracer::maxVerboseLogDepth=1;
int osl::checkmate::CheckmateRecorder::DepthTracer::maxLogDepth=1;
/** depth の初期値は 0 に固定 */
int osl::checkmate::CheckmateRecorder::DepthTracer::depth=0;
osl::checkmate::CheckMove *osl::checkmate::CheckmateRecorder::nextMove;
const char * osl::checkmate::CheckmateRecorder::leaveReason=0;
const osl::SimpleState* osl::checkmate::CheckmateRecorder::currentState=0;

std::ostream& osl::checkmate::CheckmateRecorder::DepthTracer::stream()
{
  static std::ofstream nullStream("/dev/null");
  if (DepthTracer::maxVerboseLogDepth <= depth)
    return nullStream;
  return CheckmateRecorder::stream();
}

namespace osl
{
  namespace checkmate
  {
    static CheckStack check_stack;
#ifdef CHECKMATE_DEBUG    
    void print_check_stack(int)
    {
      printf("oops\n");
      printf("stack size %ld\n", (long int)check_stack.size());
      CheckmateRecorder::dumpStack();
    }
    class CheckSignalInitializer
    {
    public:
      CheckSignalInitializer()
      {
	signal(SIGINT, print_check_stack);
      }
    } initializer;
#endif
  } // namespace checkmate
} // namespace osl

namespace
{
  std::ostream& makeOutlineStream()
  {
    static std::ofstream os("checkmate.log");
    os << ";;; -*-mode:outline-*-\n"
       << "* root\n";
    return os;
  }
  std::string makeHeader(int depth)
  {
    assert(depth >= 0);
    std::string header(depth+1, '*');
    header += ' ';
    return header;
  }
}

std::ostream& osl::checkmate::CheckmateRecorder::stream()
{
  static std::ostream& os = makeOutlineStream();
  return os;
}


void osl::checkmate::CheckmateRecorder::dumpStack()
{
  if (currentState)
    std::cerr << *currentState << "\n";
  std::cerr << check_stack;
  if (! check_stack.empty())
    check_stack[check_stack.size()-1].record->dump(std::cerr, 3);
}

void osl::checkmate::CheckmateRecorder::
dumpRecord(const SimpleState& state,
	   const CheckHashRecord *record, 
	   unsigned int proofLimit,
	   unsigned int disProofLimit,
	   unsigned int currentProofNumber,
	   unsigned int currentDisProofNumber)
{
  stream() << "CheckmateRecorder::dumpRecord\n";
  stream() << "proofLimit " << proofLimit << "\n"
	   << "disProofLimit " << disProofLimit << "\n"
	   << "cur proofNumber " << currentProofNumber << "\n"
	   << "cur disProofNumber " << currentDisProofNumber << "\n";
  stream() << state;
  stream() << record << "\n";
  if (record)
  {
    record->dump(stream(), 1);
  }
  stream() << std::flush;
}

void osl::checkmate::CheckmateRecorder::writeRootLog(
#ifdef CHECKMATE_DEBUG
  const char *msg, unsigned int tableSize, unsigned int continuousNoExpandLoop
#else
  const char *, unsigned int, unsigned int
#endif
  )
{
#ifdef CHECKMATE_DEBUG
  if (DepthTracer::maxVerboseLogDepth <= 1)
    return;
  stream() << msg << " table " << tableSize
	   << " continuousNoExpandLoop "<< continuousNoExpandLoop << "\n"
	   << std::flush;
#endif
}

void osl::checkmate::CheckmateRecorder::stat(const char *msg, Player P,
			     unsigned int tableSize,
			     unsigned int totalNodeCount, 
			     unsigned int totalNodeLimit)
{
  std::cerr << msg << " (" << P << ") "
	    << tableSize << "/"
	    << totalNodeCount << "/" << totalNodeLimit << std::endl;
}

void osl::checkmate::
CheckmateRecorder::enter(int depth, const char *name, 
			 CheckHashRecord *record, 
			 const HashKey& key, const PathEncoding& path,
			 unsigned int proofLimit, unsigned int disProofLimit)
{
#ifndef OSL_SMP
  check_assert(depth >= 0);
  check_stack.push_back(CheckStackEntry(nextMove, name, record, key, path,
				       proofLimit, disProofLimit));

  if (DepthTracer::maxVerboseLogDepth <= depth)
    return;

  stream() << makeHeader(depth) << "==> " << name << " ";
  if (nextMove)
  {
    csaShow(stream(), nextMove->move);
    if (nextMove->record != record)
    {
      std::cerr << "nextMove->record " << nextMove->record
		<< " != record " << record << " " << nextMove->move << "\n";
      stream() << "nextMove->record " << nextMove->record
	       << " != record " << record << " " << nextMove->move << "\n";
    }
  }
  else
    stream() << "root";
  stream() << " limit(" << proofLimit << "," << disProofLimit << ")";
  if (record)
  {
    stream() << " depth " << record->distance
	     << " " << record->proofDisproof()
	     << " #moves " << record->moves.size();
#if 0
    // TODO: 内訳の表示
    if (! record->interposeMoves.empty())
      stream() << " i " << record->interposeMoves.size();
    if (! record->noPromoteMoves.empty())
      stream() << " np " << record->noPromoteMoves.size();
    if (! record->upwardMoves.empty())
      stream() << " up " << record->upwardMoves.size();
#endif
    if (record->isConfluence)
      stream() << " (c)";
    if (record->useMaxInsteadOfSum)
      stream() << " (max)";
  }
  stream() << ' ' << record << ' ' << path << "\n" << std::flush;
#ifdef CHECKMATE_DEBUG
#  if 0
  if (record)
    record->dump(stream(), 1);
#  endif
#endif

#endif /* OSL_SMP */
}
  
void osl::checkmate::CheckmateRecorder::leave(int depth, const char *name, 
			      const CheckHashRecord *record)
{
#ifndef OSL_SMP
  check_assert(depth >= 0);
  check_stack.pop_back();

  if (DepthTracer::maxVerboseLogDepth <= depth)
    return;
  
  stream() << makeHeader(depth) << "<== " << name << " "
	   << "(" << record->distance << ") "
	   << record->proofDisproof();
  if (leaveReason)
  {
    stream() << " " << leaveReason;
    leaveReason = 0;
  }
  stream() << " " << record << "\n" << std::flush;
#endif
}

void osl::checkmate::
checkAbort(const char *func, const char *file, int line, const char *exp)
{
  std::cerr << "check_assert failed: (" << exp << "), function " << func
	    << ", file " << file << ", line " << line << ".\n";
  CheckmateRecorder::dumpStack();
}

#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
