#include "searchMonitor.h"
#include "osl/record/ki2.h"

#include <QLabel>
#include <QTextCodec>
#include <QLocale>

ViewerSearchMonitor::ViewerSearchMonitor(const osl::NumEffectState& state, int minimumDepth_)
  : codec(QTextCodec::codecForName("EUC-JP")), minimumDepth(minimumDepth_),
    root(state), key(state), root_move(), cur_depth(0),
    node_count(0), elapsed(0), memory(0), pv_value(0)
{
}

void ViewerSearchMonitor::
newDepth(int depth)
{
  cur_depth = depth;
  
  if (cur_depth > minimumDepth)
  {
    emit newDepthSignal(key.signature(), depth);
  }
}

void ViewerSearchMonitor::
showPV(int depth, size_t node_count, double elapsed, int value, osl::Move cur, const osl::Move *first, const osl::Move *last,
            const bool *threatmate_first, const bool *threatmate_last)
{
  cur_depth = std::max(cur_depth, depth);

  pv.clear();
  pv_threatmate.clear();
  if (first == last || cur != *first) {
    pv.push_back(cur);
    pv_threatmate.push_back(false);
  }
  pv.insert(pv.end(), first, last);
  pv_threatmate.insert(pv_threatmate.end(), threatmate_first, threatmate_last);
  pv_value = value * osl::sign(root.turn());
  this->node_count = node_count;
  this->elapsed = elapsed;
  updateStatus();

  if (cur_depth > minimumDepth)
  {
    std::vector<int> pv_int;
    pv_int.reserve(pv.size());
    for (auto move:pv)
    {
      pv_int.push_back(move.intValue());
    }

    emit showPVSignal(key.signature(), depth, node_count, elapsed, value, pv_int);
  }
}

void ViewerSearchMonitor::
showFailLow(int depth, size_t node_count, double elapsed, int value, osl::Move cur)
{
  if (cur_depth > minimumDepth)
  {
    emit showFailLowSignal(key.signature(), depth, node_count, elapsed, value, cur.intValue());
  }
}

void ViewerSearchMonitor::
rootMove(osl::Move cur)
{
  root_move = cur;
  updateStatus();

  if (cur_depth > minimumDepth)
  {
    emit rootMoveSignal(key.signature(), cur.intValue());
  }
}

void ViewerSearchMonitor::
rootFirstMove(osl::Move cur)
{
  root_move = cur;
  updateStatus();

  if (cur_depth > minimumDepth)
  {
    emit rootFirstMoveSignal(key.signature(), cur.intValue());
  }
}

void ViewerSearchMonitor::
timeInfo(size_t node_count, double elapsed)
{
  this->node_count = node_count;
  this->elapsed = elapsed;
  updateStatus();

  if (cur_depth > minimumDepth)
  {
    emit timeInfoSignal(key.signature(), node_count, elapsed);
  }
}

void ViewerSearchMonitor::
hashInfo(double ratio)
{
  memory = ratio * 100;
  updateStatus();

  if (cur_depth > minimumDepth)
  {
    emit hashInfoSignal(key.signature(), ratio);
  }
}

void ViewerSearchMonitor::
rootForcedMove(osl::Move the_move)
{
  emit rootForcedMoveSignal(key.signature(), the_move.intValue());
}

void ViewerSearchMonitor::
rootLossByCheckmate()
{
  emit rootLossByCheckmateSignal(key.signature());
}

void ViewerSearchMonitor::
depthFinishedNormally(int depth)
{
  if (cur_depth > minimumDepth)
  {
    emit depthFinishedNormallySignal(key.signature(), depth);
  }
}

void ViewerSearchMonitor::
searchFinished()
{
  if (cur_depth > minimumDepth)
  {
    emit searchFinishedSignal(key.signature());
  }
}

void ViewerSearchMonitor::updateStatus()
{
  if (! root_move.isNormal())
  {
    return;
  }

  const std::string euc_pv = osl::ki2::show
    (&*pv.begin(), &*pv.end(), &*pv_threatmate.begin(), &*pv_threatmate.end(), root);
  const std::string euc_move = osl::ki2::show(root_move, root);
  const QString status = QString::fromUtf8("探索中 %1:  %2節点  %3秒 (NPS %4) Memory %5%\n[%6] %7")
                 .arg(codec->toUnicode(euc_move.c_str(), euc_move.length()))
                 .arg(QLocale(QLocale::English).toString(static_cast<qulonglong>(node_count)))
                 .arg(static_cast<int>(elapsed))
                 .arg(QLocale(QLocale::English).toString(static_cast<int>(node_count/elapsed)))
                 .arg(static_cast<int>(memory))
                 .arg(pv_value, 5)
                 .arg(codec->toUnicode(euc_pv.c_str(), euc_pv.length()));
  emit updated(status);
}
