#ifndef _EVAL_DEBUG_H
#define _EVAL_DEBUG_H

#include "itemDelegate.h"
#include <QWidget>
#include <QDialog>
#include <QModelIndex>
#include <QStringList>
#include "osl/eval/openMidEndingEval.h"
#include "osl/numEffectState.h"
#include "osl/container/pieceValues.h"
#include <vector>

class OpenMidEndingEvalDebugModel;
class OpenMidEndingEvalDiffModel;

class OpenMidEndingEvalUtil
{
public:
  static QStringList featureName();
  static int progressAdjustedValue(const osl::eval::ml::OpenMidEndingEvalDebugInfo &debug_info,
                                   int progress,
                                   int progress_stage,
                                   int index);
  static int progressAdjustedTotal(const osl::eval::ml::OpenMidEndingEvalDebugInfo &debug_info,
                                   int progress,
                                   int index);
};

class OpenMidEndingEvalDebug : public QWidget
{
  Q_OBJECT
public:
  OpenMidEndingEvalDebug(
    const osl::NumEffectState &s,
    QWidget *parent = 0);
  void setStatus(const osl::SimpleState &state);
private:
  OpenMidEndingEvalDebugModel *model;
  std::unique_ptr<DoubleItemDelegate> delegate;
};

class OpenMidEndingEvalDiff : public QWidget
{
public:
  OpenMidEndingEvalDiff(const osl::NumEffectState &s1,
                        const osl::NumEffectState &s2,
                        QWidget *parent = 0);
private:
  OpenMidEndingEvalDiffModel *model;
  std::unique_ptr<DoubleItemDelegate> delegate;
};

class OpenMidEndingEvalDebugDialog : public QDialog
{
  Q_OBJECT
public:
  OpenMidEndingEvalDebugDialog(const osl::SimpleState &s,
		      QWidget *parent = 0);
public slots:
  void setStatus(const osl::SimpleState &state,
		 const std::vector<osl::Move> &,
		 int, osl::Move);
private:
  OpenMidEndingEvalDebug *debug;
};

class OpenMidEndingEvalDiffDialog : public QDialog
{
  Q_OBJECT
public:
  OpenMidEndingEvalDiffDialog(
    const osl::NumEffectState &s1,
    const osl::NumEffectState &s2,
    QWidget *parent = 0);
  QSize sizeHint() const;
private slots:
  void selectTab(int);
private:
  OpenMidEndingEvalDiff *debug;
  OpenMidEndingEvalDebug *debug1;
  OpenMidEndingEvalDebug *debug2;
};

#endif // _EVAL_DEBUG_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
