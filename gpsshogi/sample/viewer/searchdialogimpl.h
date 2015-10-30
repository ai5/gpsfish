#ifndef _SEARCH_DIALOG_IMPL_H
#define _SEARCH_DIALOG_IMPL_H
#include "ui_searchdialog4.h"

class SearchDialogImpl : public QDialog, public Ui::SearchDialog
{
  Q_OBJECT
public:
  SearchDialogImpl(const QStringList *list,
		   int algorithmIndex,
		   int depthLimit, int initialDepthLimit,
		   int deepningStep, int nodeLimit,
		   int tableSizeLimit, int tableRecordLimit,
		   int checkmateNodeLimit,
		   int searchTime,
		   QWidget *parent = 0);
private slots:
  void setDefaultValue();
  void resetValue();
private:
  int algorithmIndex;
  int depthLimit;
  int initialDepthLimit;
  int deepningStep;
  int nodeLimit;
  int tableSizeLimit;
  int tableRecordLimit;
  int multiPVWidth;
  int searchTime;

public:
  static const int DEFAULT_NODE_DEPTH_LIMIT = 1200;
  static const int DEFAULT_NODE_INITIAL_DEPTH_LIMIT = 400;
  static const int DEFAULT_NODE_DEEPNING_STEP = 200;
  static const int DEFAULT_NODE_LIMIT = 1600000;

  static const int DEFAULT_TABLE_SIZE_LIMIT = 5000000;
  static const int DEFAULT_TABLE_RECORD_LIMIT = 200;
  static const int DEFAULT_MULTI_PV_WIDTH = 0;
  static const int DEFAULT_TIME = 120;
};

#endif // _SEARCH_DIALOG_IMPL_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
