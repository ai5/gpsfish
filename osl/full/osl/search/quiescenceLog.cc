/* quiescenceLog.cc
 */
#include "osl/search/quiescenceLog.h"
#include "osl/search/quiescenceRecord.h"
#include <fstream>
#include <iostream>

namespace 
{
  std::unique_ptr<std::ofstream> os;
} // anonymous namespace

std::ostream* osl::search::QuiescenceLog::
os()
{
  return ::os.get();
}

void osl::search::QuiescenceLog::
init(const char *filename)
{
  ::os.reset(new std::ofstream(filename));
}

void osl::search::QuiescenceLog::
close()
{
  ::os.reset();
}

void osl::search::QuiescenceLog::
enter(const SimpleState& state)
{
  if (os())
  {
    *os() << '*' << "new node\n";
    *os() << state;
  }
}

void osl::search::QuiescenceLog::
pushMove(int depth, Move move, const QuiescenceRecord *record)
{
  if (os())
  {
    *os() << std::string(2+std::max(0,QSearchTraits::MaxDepth-depth), '*') 
	  << move << "\n" << std::flush;
    if (record)
      record->dump(*os());
  }
}

void osl::search::QuiescenceLog::
staticValue(int depth, int value)
{
  if (os())
    *os() << std::string(2+std::max(0,QSearchTraits::MaxDepth-depth), '*') 
	<<" static " << value << "\n" << std::flush;
}

void osl::search::QuiescenceLog::
node(int depth, int alpha, int beta, int result)
{
  if (os())
    *os() << std::string(1+std::max(0,QSearchTraits::MaxDepth-depth), '*')
	  << alpha << " " << beta << " => " << result << "\n" << std::flush;
};


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
