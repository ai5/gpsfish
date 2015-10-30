#include "osl/annotate/facade.h"
#include "osl/annotate/analyzer.h"
#include "osl/progress.h"
#include "osl/eval/openMidEndingEval.h"
#include <boost/ptr_container/ptr_vector.hpp>

void osl::annotate::
analyze(const NumEffectState& src, const std::vector<Move>& moves,
	int last_move,
	AnalysesResult& result)
{
  static boost::ptr_vector<Analyzer> analyzers;
  static bool initialized = false;
  if (! initialized) 
  {
    analyzers.push_back(new RepetitionAnalyzer);
    analyzers.push_back(new CheckmateAnalyzer);
    analyzers.push_back(new CheckmateWin);
    analyzers.push_back(new EscapeFromCheck);
    analyzers.push_back(new CheckmateForCapture);
    analyzers.push_back(new ThreatmateAnalyzer);
    analyzers.push_back(new CheckmateForEscape);
    analyzers.push_back(new ThreatmateIfMorePieces);
    analyzers.push_back(new Vision3);
    progress::ml::NewProgress::setUp();
    eval::ml::OpenMidEndingEval::setUp();
    initialized = true;
  }
  result = AnalysesResult();
  for (Analyzer& a: analyzers)
  {
    a.match(result, src, moves, last_move);
    if (result.checkmate == True)
      break;
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
