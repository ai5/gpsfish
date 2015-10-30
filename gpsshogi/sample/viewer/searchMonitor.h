#ifndef _SEARCH_MONITOR_H
#define _SEARCH_MONITOR_H

#include "osl/basic_type.h"
#include "osl/hashKey.h"
#include "osl/search/searchMonitor.h"
#include "osl/numEffectState.h"
#include <vector>
#include <QObject>
#include <QMetaType>

class QLabel;
class QTextCodec;

Q_DECLARE_METATYPE(std::vector<int>);

/*
 * This class translates SearchMonitor notifications to Qt events because Qt
 * allows only the event loop thread to update Qt widgets.
 */
class ViewerSearchMonitor : public QObject, public osl::search::SearchMonitor
{
  Q_OBJECT

  QTextCodec *codec;
  int minimumDepth;
  void updateStatus();
protected:
  osl::NumEffectState root;
  osl::HashKey key;
  osl::Move root_move;
  int cur_depth;
  size_t node_count;
  double elapsed;
  double memory;
  int pv_value;
  std::vector<osl::Move> pv;
  std::vector<char> pv_threatmate;
signals:
  void updated(const QString& status);
  void newDepthSignal(unsigned long key, int depth);
  void showPVSignal(unsigned long key,
                    int depth, unsigned long node_count, double elapsed, int value, const std::vector<int>&);
  void showFailLowSignal(unsigned long, int depth, size_t node_count, double elapsed, int value, int cur);
  void rootMoveSignal(unsigned long, int cur);
  void rootFirstMoveSignal(unsigned long, int cur);
  void timeInfoSignal(unsigned long, unsigned long node_count, double elapsed);
  void hashInfoSignal(unsigned long, double ratio);
  void rootForcedMoveSignal(unsigned long, int the_move);
  void rootLossByCheckmateSignal(unsigned long);
  void depthFinishedNormallySignal(unsigned long, int depth);
  void searchFinishedSignal(unsigned long);
public:
  ViewerSearchMonitor(const osl::NumEffectState& state, int minimumDepth_ = 4);

  /* Override methods */
  void newDepth(int depth);
  void showPV(int depth, size_t node_count, double elapsed, int value, osl::Move cur, const osl::Move *first, const osl::Move *last,
              const bool *threatmate_first, const bool *threatmate_last);
  void showFailLow(int depth, size_t node_count, double elapsed, int value, osl::Move cur);
  void rootMove(osl::Move cur);
  void rootFirstMove(osl::Move cur);
  void timeInfo(size_t node_count, double elapsed);
  void hashInfo(double ratio);
  void rootForcedMove(osl::Move the_move);
  void rootLossByCheckmate();
  void depthFinishedNormally(int depth);
  void searchFinished();
};

#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

