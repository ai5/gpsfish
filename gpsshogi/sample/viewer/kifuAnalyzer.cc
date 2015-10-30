#include "kifuAnalyzer.h"
#include "searchMonitor.h"
#include "gpsshogi/gui/util.h"
#include "osl/sennichite.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/search/fixedEval.h"
#include "osl/search/simpleHashTable.h"
#include "osl/numEffectState.h"
#include "osl/eval/openMidEndingEval.h"

#include <QThread>
#include <QLabel>
#include <QTreeWidget>

#include <QVBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QPainter>

class EvaluationGraph : public QWidget
{
public:
  EvaluationGraph(QWidget *parent = 0);
  void setResult(const std::vector<Result>& r) {
    result = r;
    update();
  }
  virtual QSize sizeHint() const {
    return QSize(700, 400);
  }
protected:
  void paintEvent(QPaintEvent *);
private:
  std::vector<Result> result;
};

EvaluationGraph::EvaluationGraph(QWidget *parent)
  : QWidget(parent)
{
}

void EvaluationGraph::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  QBrush brush(QColor("white"));
  painter.fillRect(0, 0, width(), height(), brush);
  painter.setPen(QColor("grey"));
  painter.drawLine(60, 100, 640, 100);
  painter.drawLine(60, 200, 640, 200);
  painter.drawLine(60, 300, 640, 300);
  painter.setPen(QColor("blue"));
  for (int i = 0; i < (int)result.size() - 1; i++)
  {
    int left = std::max(std::min(200 - result[i].value / 10, 400), 0);
    int right = std::max(std::min(200 - result[i + 1].value / 10, 400), 0);
    painter.drawLine(60 + 4 * i,  left,
		     60 + 4 * (i + 1), right);
  }
  painter.setPen(QColor("black"));
  painter.drawText(0, 200 - 30, 50, 60, Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(0));
  painter.drawText(0, 100 - 30, 50, 60, Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(1000));
  painter.drawText(0, 300 - 30, 50, 60, Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(-1000));

  painter.drawText(60 + 4 * (50 - 1), 200 - 30, 60, 60,
                   Qt::AlignLeft | Qt::AlignVCenter, QString("%1").arg(50));
  painter.drawText(60 + 4 * (100 - 1), 200 - 30, 60, 60,
                   Qt::AlignLeft | Qt::AlignVCenter, QString("%1").arg(100));
}

class AnalyzeItem : public QTreeWidgetItem
{
public:
  AnalyzeItem(QTreeWidget *parent, int i, const Result& result);
  bool operator<(const QTreeWidgetItem& other) const;
  int getNumber() const { return number; }
private:
  int number;
};

AnalyzeItem::AnalyzeItem(QTreeWidget *parent, int n, const Result& result)
  : QTreeWidgetItem(parent), number(n)
{
  int i = 0;
  setText(i++, QString("%1").arg(number));
  setText(i++, gpsshogi::gui::Util::moveToString(result.move));
  setText(i++, gpsshogi::gui::Util::moveToString(result.computed_move));
  setText(i++, QString("%1").arg(result.value));
  setText(i++, QString("%1").arg(result.depth));
  QString pv;
  for (size_t i = 0; i < result.pvs.size(); ++i)
  {
    if (!pv.isEmpty())
    {
      pv.append(", ");
    }
    pv.append(gpsshogi::gui::Util::moveToString(result.pvs[i]));
  }
  setText(i++, pv);

  setTextAlignment(0, Qt::AlignRight);
  setTextAlignment(3, Qt::AlignRight);
  setTextAlignment(4, Qt::AlignRight);
}

bool AnalyzeItem::operator<(const QTreeWidgetItem& other) const
{
  const AnalyzeItem& item = (const AnalyzeItem&)other;
  return number < item.getNumber();
}

class AnalyzeThread : public QThread
{
public:
  AnalyzeThread(KifuAnalyzer *w, const osl::SimpleState &s,
                const std::vector<osl::Move> &ms) : widget(w), moves(ms)
  {
    player.reset(new osl::game_playing::AlphaBeta2OpenMidEndingEvalPlayer());
    player->setDepthLimit(1200, 400, 200);
    if (osl::OslConfig::isMemoryLimitEffective()) 
    {
      player->setTableLimit(std::numeric_limits<size_t>::max(), 0);
      player->setNodeLimit(std::numeric_limits<size_t>::max());
    }
    game_state.reset(new osl::game_playing::GameState(s));
    monitor.reset(new ViewerSearchMonitor(game_state->state()));

    player->addMonitor(monitor);
    QObject::connect(monitor.get(), SIGNAL(updated(const QString&)),
                     w->getCurrentStatusLabel(), SLOT(setText(const QString&)));
  }
  void run()
  {
    for (size_t i = 0; i < moves.size() && widget->shouldSearch(); i++)
    {
      player->selectBestMove(*game_state, 0, 0, 5);
      widget->addResult(moves[i], *player, *game_state);
      game_state->pushMove(moves[i]);
    }
  }
private:
  KifuAnalyzer *widget;
  const std::vector<osl::Move> moves;
  std::unique_ptr<osl::game_playing::AlphaBeta2OpenMidEndingEvalPlayer> player;
  std::unique_ptr<osl::game_playing::GameState> game_state;
  std::shared_ptr<ViewerSearchMonitor> monitor;
};

KifuAnalyzer::KifuAnalyzer(const osl::SimpleState &state,
			   const std::vector<osl::Move> &moves,
			   QWidget *parent)
  : QDialog(parent), search(true)
{
  setAttribute(Qt::WA_DeleteOnClose);
  QTabWidget *tab = new QTabWidget(this);

  list = new QTreeWidget(tab);
  list->setColumnCount(6);
  list->setHeaderLabels(QStringList() << "" <<
                                         "Move" <<
                                         "Computed Move" <<
                                         "Value" <<
                                         "Depth" <<
                                         "PV");
  osl::NumEffectState estate(state);
  osl::eval::ml::OpenMidEndingEval eval(estate);
  graph = new EvaluationGraph(tab);
  pawnValue = eval.captureValue(osl::newPtypeO(osl::WHITE, osl::PAWN)) / 2;

  QVBoxLayout *layout = new QVBoxLayout(this);
  tab->addTab(graph, "Graph");
  tab->addTab(list, "List");
  layout->addWidget(tab);

  currentStatus = new QLabel(this);
  layout->addWidget(currentStatus);

  QPushButton *button = new QPushButton(this);
  button->setText("&OK");
  layout->addWidget(button);
  connect(button, SIGNAL(clicked()), this, SLOT(accept()));

  thread.reset(new AnalyzeThread(this, state, moves));;
  thread->start();
}

KifuAnalyzer::~KifuAnalyzer()
{
  search = false;
  thread->wait();
}

void KifuAnalyzer::addResult(const osl::Move move,
			     const osl::game_playing::SearchPlayer& player,
			     const osl::game_playing::GameState& state)
{
  const osl::search::SimpleHashTable& table = *player.table();
  const osl::HashKey key(state.state());
  const osl::search::SimpleHashRecord *record = table.find(key);

  osl::MoveVector moves;
  if (record)
  {
    int value = record->hasLowerBound(0) ? record->lowerBound()  * 100L / pawnValue :
      (result.empty() ? 0 : result.back().value);
    int limit = (record->lowerLimit() > record->upperLimit()) ?
      record->lowerLimit() : record->upperLimit();
    const osl::Move best_move = record->bestMove().move();
    if (best_move.isValid())
    {
      const osl::HashKey new_key = key.newHashWithMove(best_move);
      if (table.find(new_key))
      {
        moves.push_back(best_move);
        table.getPV(new_key, moves);
      }
    }
    result.push_back(Result(move, record->bestMove().move(),
                            value, limit, moves));
  }
  else
  {
    int value = result.empty() ? 0 : result.back().value;
    result.push_back(Result(move, osl::Move::INVALID(), value, -1, moves));
  }
  graph->setResult(result);
  new AnalyzeItem(list, result.size(), result.back());
}
