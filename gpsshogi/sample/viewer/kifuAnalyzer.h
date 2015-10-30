#ifndef _KIFU_ANALIZER_H
#define _KIFU_ANALIZER_H
#include "osl/basic_type.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/simpleState.h"
#include <QDialog>
#include <vector>

class AnalyzeThread;
class EvaluationGraph;
class QLabel;
class QTreeWidget;

struct Result
{
  Result(osl::Move move, osl::Move computed_move,
         int value, int depth, const osl::MoveVector moves)
    : move(move), computed_move(computed_move),
      value(value), depth(depth), pvs(moves) {
  }
  osl::Move move;
  osl::Move computed_move;
  int value;
  int depth;
  osl::MoveVector pvs;
};

class KifuAnalyzer : public QDialog
{
public:
  KifuAnalyzer(const osl::SimpleState &state,
	       const std::vector<osl::Move> &moves,
	       QWidget *parent = 0);
  virtual ~KifuAnalyzer();
  QLabel *getCurrentStatusLabel() const
  { return currentStatus; }
private:
  friend class AnalyzeThread;
  bool shouldSearch() const {
    return search;
  }
  void addResult(const osl::Move move,
		 const osl::game_playing::SearchPlayer& player,
		 const osl::game_playing::GameState& state);
  std::unique_ptr<AnalyzeThread> thread;
  EvaluationGraph *graph;
  QTreeWidget *list;
  QLabel *currentStatus;
  std::vector<Result> result;
  bool search;
  int pawnValue;
};

#endif // _KIFU_ANALIZER_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
