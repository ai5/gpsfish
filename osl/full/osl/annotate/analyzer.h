/* analyzer.h
 */
#ifndef OSL_ANNOTATE_ANALYZER_H
#define OSL_ANNOTATE_ANALYZER_H

#include "osl/annotate/analysesResult.h"
#include "osl/numEffectState.h"
#include <vector>

namespace osl
{
  namespace annotate
  {
    class Analyzer
    {
    public:
      virtual ~Analyzer();
      virtual void match(AnalysesResult&,
			 const NumEffectState& src, const std::vector<Move>& moves,
			 int last_move)=0;

      static Trivalent isCheckmate(NumEffectState& state, Move& best_move, bool attack=true,
				   size_t *node_count=0);
    };
    /** 千日手模様(同一局面)の検知 */
    class RepetitionAnalyzer : public Analyzer
    {
    public:
      void match(AnalysesResult&,
		 const NumEffectState& src, const std::vector<Move>& moves,
		 int last_move);
    };
    /** 指した王手が正解で詰み */
    class CheckmateAnalyzer : public Analyzer
    {
    public:
      void match(AnalysesResult&,
		 const NumEffectState& src, const std::vector<Move>& moves,
		 int last_move);
    };
    /** 手番側が正しく指せば詰み */
    class CheckmateWin : public Analyzer
    {
    public:
      void match(AnalysesResult&,
		 const NumEffectState& src, const std::vector<Move>& moves,
		 int last_move);
    };
    class EscapeFromCheck : public Analyzer
    {
    public:
      void match(AnalysesResult&,
		 const NumEffectState& src, const std::vector<Move>& moves,
		 int last_move);
      static bool matchMain(const NumEffectState& src, const std::vector<Move>& moves,
			    int last_move);
    };
    class ThreatmateAnalyzer : public Analyzer
    {
    public:
      void match(AnalysesResult&,
		 const NumEffectState& src, const std::vector<Move>& moves,
		 int last_move);
    };
    class CheckmateForCapture : public Analyzer
    {
    public:
      void match(AnalysesResult&,
		 const NumEffectState& src, const std::vector<Move>& moves,
		 int last_move);
    };
    class CheckmateForEscape : public Analyzer
    {
    public:
      void match(AnalysesResult&,
		 const NumEffectState& src, const std::vector<Move>& moves,
		 int last_move);
    };
    class ThreatmateIfMorePieces : public Analyzer
    {
    public:
      void match(AnalysesResult&,
		 const NumEffectState& src, const std::vector<Move>& moves,
		 int last_move);
      static bool suitable(const NumEffectState& state, Piece p);
    };
    class Vision3 : public Analyzer
    {
    public:
      void match(AnalysesResult&,
		 const NumEffectState& src, const std::vector<Move>& moves,
		 int last_move);
    };
  }
}

#endif /* OSL_ANNOTATE_ANALYZER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
