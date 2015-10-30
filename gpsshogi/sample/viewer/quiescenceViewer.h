#ifndef _QUIESCENSE_VIEWER_H
#define _QUIESCENSE_VIEWER_H
#include "boardAndListTabChild.h"
#include "osl/numEffectState.h"
#include "osl/search/simpleHashTable.h"
#include "osl/game_playing/gameState.h"

namespace osl
{
  namespace search
  {
    class SimpleHashRecord;
    class SimpleHashTable;
  }
}

class QuiescenceItem;

class QuiescenceViewer : public BoardAndListTabChild
{
public:
  QuiescenceViewer(QWidget *parent = 0);
  bool analyze(const osl::SimpleState& s,
	       const std::vector<osl::Move>& moves);
  bool analyzeHalfDepth(const osl::SimpleState& s,
			const std::vector<osl::Move>& moves);
private:
  bool analyze(const osl::SimpleState& s,
	       const std::vector<osl::Move>& moves,
	       int depth);
  template <class Eval>
  void analyze(const osl::SimpleState& state,
	       const osl::Move last_move, int depth);
  std::unique_ptr<osl::search::SimpleHashTable> table;
};
#endif // _QUIESCENCE_VIEWER_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
