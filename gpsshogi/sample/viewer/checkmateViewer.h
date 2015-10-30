#ifndef _CHECKMATE_VIEWER_H
#define _CHECKMATE_VIEWER_H
#include "boardAndListTabChild.h"
#include "moveTree.h"

#include "osl/numEffectState.h"
#include "osl/checkmate/dualDfpn.h"
#include "osl/checkmate/dfpn.h"

class CheckMoveItem;
class MoveCountTip;

class CheckMoveViewer : public MoveTree
{
  Q_OBJECT
public:
  CheckMoveViewer(QWidget *parent = 0);
  ~CheckMoveViewer();
  bool analyze(const osl::NumEffectState &state, int node_limit,
	       bool change_turn);
  const osl::NumEffectState initialState() const { return initial_state; }
  const osl::checkmate::DfpnTable& getTable() const { return *table; }
private slots:
  void showRecord();
  void showMoves();
  void showMovesInKanji();
protected:
  void showRecord(const osl::checkmate::DfpnRecord& record);
  void contextMenuEvent(QContextMenuEvent *);
private:
  enum check_state
  {
    CHECKMATE = 0,
    NO_CHECKMATE,
    NO_CHECKMATE_LOOP,
    UNKNOWN
  };

  check_state analyze(const osl::NumEffectState &state, int table_size,
		      int node_limit, bool change_turn);

  typedef osl::checkmate::DualDfpn searcher_t;
  std::unique_ptr<searcher_t> searcher;
  osl::NumEffectState initial_state;
  const osl::checkmate::DfpnTable *table; // acquaintance
};

class CheckmateViewer : public BoardAndListTabChild
{
Q_OBJECT
public:
  CheckmateViewer(QWidget *parent = 0);
  bool analyze(const osl::NumEffectState &state, int node_limit,
	       bool change_turn=false);
private slots:
  void expandMove(QTreeWidgetItem *_item);
};
#endif // _CHECKMATE_VIEWER_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
