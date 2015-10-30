#include "searchdialogimpl.h"

#include <qglobal.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qspinbox.h>

SearchDialogImpl::SearchDialogImpl(const QStringList *list,
				   int algorithmIndex,
				   int depthLimit, int initialDepthLimit,
				   int deepningStep, int nodeLimit,
				   int tableSizeLimit, int tableRecordLimit,
				   int multiPVWidth,
				   int searchTime,
				   QWidget *parent)
  : QDialog(parent),
    algorithmIndex(algorithmIndex),
    depthLimit(depthLimit), initialDepthLimit(initialDepthLimit),
    deepningStep(deepningStep), nodeLimit(nodeLimit),
    tableSizeLimit(tableSizeLimit), tableRecordLimit(tableRecordLimit),
    multiPVWidth(multiPVWidth),
    searchTime(searchTime)
{
  setupUi(this);
  searchPlayerComboBox->insertItems(0, *list);
  connect(defaultButton, SIGNAL(clicked()), this, SLOT(setDefaultValue()));
  connect(resetButton, SIGNAL(clicked()), this, SLOT(resetValue()));
  resetValue();
}

void SearchDialogImpl::setDefaultValue()
{
  searchPlayerComboBox->setCurrentIndex(0);
  depthLimitBox->setValue(DEFAULT_NODE_DEPTH_LIMIT);
  initialDepthLimitBox->setValue(DEFAULT_NODE_INITIAL_DEPTH_LIMIT);
  deepningStepBox->setValue(DEFAULT_NODE_DEEPNING_STEP);
  nodeLimitBox->setValue(DEFAULT_NODE_LIMIT);
  tableSizeLimitBox->setValue(DEFAULT_TABLE_SIZE_LIMIT);
  tableRecordLimitBox->setValue(DEFAULT_TABLE_RECORD_LIMIT);
  multiPVWidthBox->setValue(DEFAULT_MULTI_PV_WIDTH);
  searchTimeBox->setValue(DEFAULT_TIME);
}

void SearchDialogImpl::resetValue()
{
  searchPlayerComboBox->setCurrentIndex(algorithmIndex);
  depthLimitBox->setValue(depthLimit);
  initialDepthLimitBox->setValue(initialDepthLimit);
  deepningStepBox->setValue(deepningStep);
  nodeLimitBox->setValue(nodeLimit);
  tableSizeLimitBox->setValue(tableSizeLimit);
  tableRecordLimitBox->setValue(tableRecordLimit);
  multiPVWidthBox->setValue(multiPVWidth);
  searchTimeBox->setValue(searchTime);
}
