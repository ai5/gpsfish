#include "mainwindow.h"

#if QT_VERSION >= 0x040000
#include <Q3PopupMenu>
#include <Q3Action>
#include <QToolBar>
#include <QMenuBar>
#else
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qaction.h>
#define Q3PopupMenu QPopupMenu
#define Q3Action QAction
#endif
#include <qapplication.h>
#include <qlabel.h>
#include <qstatusbar.h>

#include "openingEditor.h"

MainWindow::MainWindow(QWidget *parent, const char *name)
  : QMainWindow(parent, name)
{
  std::string fileName("opening");
  char *csaFile = NULL;
  for (int i = 0; i < qApp->argc(); i++)
  {
    if (strcmp("-file", qApp->argv()[i]) == 0)
    {
      if (i + 1 < qApp->argc() && qApp->argv()[i + 1][0] != '-')
	fileName = std::string(qApp->argv()[i + 1]);
    }
    if (strcmp("-csa", qApp->argv()[i]) == 0)
    {
      if (i + 1 < qApp->argc() && qApp->argv()[i + 1][0] != '-')
	csaFile = qApp->argv()[i + 1];
    }
  }
  editor = new OpeningEditor(fileName, csaFile, this);
  setCentralWidget(editor);
  resize(QSize(800, 600));

  Q3PopupMenu *fileMenu = new Q3PopupMenu(this);
  Q3Action *exitAction = new Q3Action("&Quit", QKeySequence("Ctrl+q"), this);
  connect(exitAction, SIGNAL(activated()), qApp, SLOT(quit()));
  exitAction->addTo(fileMenu);

  Q3PopupMenu *editMenu = new Q3PopupMenu(this);

  Q3Action *addAction = new Q3Action("&Add", 0, this);
  connect(addAction, SIGNAL(activated()), editor, SLOT(addMove()));
  addAction->addTo(editMenu);

  Q3Action *deleteAction = new Q3Action("&Delete", 0, this);
  connect(deleteAction, SIGNAL(activated()), editor, SLOT(deleteMove()));
  deleteAction->addTo(editMenu);

  editMenu->insertSeparator();

  Q3Action *clearAction = new Q3Action("&Clear Weight", 0, this);
  connect(clearAction, SIGNAL(activated()), editor, SLOT(clearWeight()));
  clearAction->addTo(editMenu);

  Q3Action *normalizeAction = new Q3Action("&Normalize Weight", 0, this);
  connect(normalizeAction, SIGNAL(activated()), editor, SLOT(normalizeWeight()));
  normalizeAction->addTo(editMenu);

  Q3PopupMenu *goMenu = new Q3PopupMenu(this);

  Q3Action *initialAction = new Q3Action("&Initial State", 0, this);
  connect(initialAction, SIGNAL(activated()), editor, SLOT(initialState()));
  initialAction->addTo(goMenu);

  Q3Action *forwardAction = new Q3Action("&Forward", QKeySequence("Ctrl+f"), this);
  connect(forwardAction, SIGNAL(activated()), editor, SLOT(forward()));
  forwardAction->addTo(goMenu);

  Q3Action *backAction = new Q3Action("&Back", QKeySequence("Ctrl+b"), this);
  connect(backAction, SIGNAL(activated()), editor, SLOT(back()));
  backAction->addTo(goMenu);

  Q3PopupMenu *viewMenu = new Q3PopupMenu(this);

  Q3Action *showStatesAction = new Q3Action("&Show states", 0, this);
  connect(showStatesAction, SIGNAL(activated()), editor, SLOT(showStates()));
  showStatesAction->addTo(viewMenu);

  menuBar()->insertItem("&File", fileMenu);
  menuBar()->insertItem("&Edit", editMenu);
  menuBar()->insertItem("&Go", goMenu);
  menuBar()->insertItem("&View", viewMenu);

  moveLabel = new QLabel(QString::fromUtf8(" 1234手 "), this);
  moveLabel->setAlignment(Qt::AlignHCenter);
  moveLabel->setMinimumSize(moveLabel->sizeHint());
  statusBar()->addWidget(moveLabel);

  turnLabel = new QLabel(QString::fromUtf8(" 先手 "), this);
  turnLabel->setAlignment(Qt::AlignHCenter);
  turnLabel->setMinimumSize(turnLabel->sizeHint());
  statusBar()->addWidget(turnLabel);

  winCountLabel = new QLabel(QString::fromUtf8(" 先手: 1234567890 後手 1234567890 "),
			 this);
  winCountLabel->setAlignment(Qt::AlignHCenter);
  winCountLabel->setMinimumSize(winCountLabel->sizeHint());
  statusBar()->addWidget(winCountLabel);

  connect(editor, SIGNAL(statusChanged()),
	  this, SLOT(updateStatusBar()));

  updateStatusBar();
}

void MainWindow::updateStatusBar()
{
  moveLabel->setText(QString::fromUtf8(" %1手 ").arg(editor->moveCount()));
  if (editor->turn() == osl::BLACK)
    turnLabel->setText(QString::fromUtf8("先手"));
  else
    turnLabel->setText(QString::fromUtf8("後手"));

  winCountLabel->setText(QString::fromUtf8("先手: %1 後手: %2").
			 arg(editor->blackWinCount()).
			 arg(editor->whiteWinCount()));
}

