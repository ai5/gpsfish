#ifndef _ANALYSIS_VIEWER_H
#define _ANALYSIS_VIEWER_H
#include "boardAndListTabChild.h"
#include "moveTree.h"
#include "pvModel.h"

#include "osl/numEffectState.h"
#include "osl/search/simpleHashTable.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/searchPlayer.h"
#include "osl/container/moveStack.h"


namespace osl
{
  namespace search
  {
    class SimpleHashRecord;
    namespace analyzer
    {
      class RecordSet;
    } // namespace analyzer
  }
}

class QMenu;
class QTableView;
class SearchMoveTree;
class SearchMoveTreeItem;
class SearchDialogImpl;
class QContextMenuEvent;
class QWidget;

class AnalysisViewer : public BoardAndListTabChild
{
Q_OBJECT
public:
  AnalysisViewer(QWidget *parent = 0);
  virtual QSize sizeHint() const;
  bool analyze(const osl::SimpleState& s,
	       const std::vector<osl::Move>& moves);
  bool analyzeWithSavedValue(const osl::SimpleState& s,
			     const std::vector<osl::Move>& moves);
  const osl::search::SimpleHashRecord *selectedRecord();
  const osl::search::SimpleHashRecord *getRecord() {
    return record;
  }
  int getLimit();
public slots:
  QWidget *moveGenerateDialog();
private slots:
  void openMoves(const QModelIndex &);
  void expandMove(QTreeWidgetItem *_item);
private:
  void bestMoves(const osl::NumEffectState&, 
		 const osl::search::SimpleHashTable&);
  int getRecordValue(const osl::search::SimpleHashRecord *r,
		     osl::Player turn);
  void saveSearchValue(const SearchDialogImpl& dialog);

  QTableView *pvView;
  std::unique_ptr<QAbstractItemModel> pvModel;
  std::unique_ptr<QSortFilterProxyModel> sortedPvModel;
  std::unique_ptr<osl::game_playing::SearchPlayer> player;
  const osl::search::SimpleHashRecord *record;

  QString algorithm;
  int algorithmIndex;
  int depthLimit;
  int initialDepthLimit;
  int deepningStep;
  int nodeLimit;
  int tableSizeLimit;
  int tableRecordLimit;
  int multiPVWidth;
  int searchTime;

  static const QString PROGRESS_ALPHA_BETA;
  static const QString TEST_ALPHA_BETA;
  static const QString USI_PROXY;
};

class SearchMoveTree : public MoveTree
{
  Q_OBJECT
public:
  SearchMoveTree(QWidget *parent = 0);
  void addRecord(const osl::SimpleState& state,
		 const osl::search::SimpleHashTable *table,
		 const osl::search::SimpleHashRecord *record,
		 const osl::game_playing::SearchPlayer *player,
		 const osl::MoveStack& history);
  const osl::SimpleState initialState() const { return initial_state; }
  const osl::search::SimpleHashTable& getTable() const { return *table; }
protected:
  void buildContextMenu(QMenu *);
private slots:
  void showQuiescenceRecord();
  void showSubtree();
  void showRecord();
  void showMoves();
  void showMovesInKanji();
private:
  void addRootItem(const osl::search::SimpleHashRecord *record,
		   const osl::MoveStack& history);

  osl::SimpleState initial_state;
  const osl::search::SimpleHashTable *table;
  const osl::game_playing::SearchPlayer *player;
};
#endif // _ANALYSIS_VIEWER_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
