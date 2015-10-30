#ifndef _PROGRESS_DEBUG_H
#define _PROGRESS_DEBUG_H

#include "osl/progress.h"
#include "osl/numEffectState.h"
#include <QDialog>

namespace Ui
{
  class ProgressDebugDialog;
}
class NewProgressDebugModel;

class NewProgressDebugDialog : public QDialog
{
public:
  NewProgressDebugDialog(const osl::NumEffectState &state,
                         QWidget *parent = 0);
  virtual ~NewProgressDebugDialog();
private:
  Ui::ProgressDebugDialog *ui;
  NewProgressDebugModel *model;
};

#endif // _PROGRESS_DEBUG_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
