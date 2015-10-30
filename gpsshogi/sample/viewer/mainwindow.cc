#include "mainwindow.h"

#include <QToolBar>
#include <qmenubar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qtooltip.h>

#include "httpwindow.h"

#include "viewer.h"
#include "copyLabel.h"
#include "clickableLabel.h"
#include "gpsshogi/revision.h"
#include "osl/search/usiProxy.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), usiProxyAvailable(true)
{
  viewer = new Viewer(this);
  setCentralWidget(viewer);
  resize(QSize(1000, 600));
  std::string usi_proxy = "";

  for (int i = 0; i < qApp->arguments().size(); i++)
  {
    if (qApp->arguments()[i] == "-csa")
    {
      if (i + 1 < qApp->arguments().size() && qApp->arguments()[i + 1][0] != '-')
      {
	const QString fileName(qApp->arguments()[i + 1]);
	viewer->open(fileName);
      }
      if (i + 2 < qApp->arguments().size() && qApp->arguments()[i + 2][0] != '-')
      {
	bool ok;
	int index = QString(qApp->arguments()[i + 2]).toInt(&ok);
	if (ok)
	  viewer->moveTo(index - 1);
      }
    }
    else if (qApp->arguments()[i] == "-kisen")
    {
      if (i + 2 < qApp->arguments().size() && qApp->arguments()[i + 2][0] != '-')
      {
	const QString fileName(qApp->arguments()[i + 1]);
	bool ok;
	int index = QString(qApp->arguments()[i + 2]).toInt(&ok);
	if (ok)
	  viewer->open(fileName, index);
      }
    }
    else if (qApp->arguments()[i] == "-kif")
    {
      if (i + 1 < qApp->arguments().size() && qApp->arguments()[i + 1][0] != '-')
      {
	const QString fileName(qApp->arguments()[i + 1]);
	viewer->openKakinoki(fileName);
      }
    }
    else if (qApp->arguments()[i] == "-usi")
    {
      if (i + 1 < qApp->arguments().size() && qApp->arguments()[i + 1][0] != '-')
      {
	const QString fileName(qApp->arguments()[i + 1]);
	viewer->openUsi(fileName);
      }
    }
    else if (qApp->arguments()[i] == "-url")
    {
      if (i + 1 < qApp->arguments().size() && qApp->arguments()[i + 1][0] != '-')
      {
	HttpWindow::setLastUrl(qApp->arguments()[i + 1]);
	viewer->reloadUrl();
      }
    }
    else if (qApp->arguments()[i] == "-usi-proxy")
    {
      if (i + 1 < qApp->arguments().size() && qApp->arguments()[i + 1][0] != '-')
      {
	usi_proxy = qApp->arguments()[i + 1].toStdString();
      }
    }    
  }

  try
  {
    if (osl::search::UsiProxy::setUp(usi_proxy, {"setoption name OwnBook value false"}))
    {
      setUsiProxyAvailable(true);
      std::cout << "Enable UsiProxy: " << osl::search::UsiProxy::getProgram() << std::endl; 
    }
    else
    {
      setUsiProxyAvailable(false);
    }
  }
  catch (...)
  {
    setUsiProxyAvailable(false);
    std::cerr << "Failed to set up UsiProxy\n";
  }

  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  QAction *openAction = new QAction(tr("&Open"), this);
  openAction->setShortcut(tr("Ctrl+o"));
  connect(openAction, SIGNAL(triggered()), viewer, SLOT(open()));
  fileMenu->addAction(openAction);

  QAction *nextAction = new QAction(tr("Open &Next"), this);
  connect(nextAction, SIGNAL(triggered()), viewer, SLOT(nextFile()));
  fileMenu->addAction(nextAction);

  QAction *prevAction = new QAction(tr("Open &Previous"), this);
  connect(prevAction, SIGNAL(triggered()), viewer, SLOT(prevFile()));
  fileMenu->addAction(prevAction);

  QAction *urlAction = new QAction(tr("Open &Url"), this);
  urlAction->setShortcut(tr("Ctrl+u"));
  connect(urlAction, SIGNAL(triggered()), viewer, SLOT(openUrl()));
  fileMenu->addAction(urlAction);

  QAction *reloadUrlAction = new QAction(tr("&Reload Url"), this);
  reloadUrlAction->setShortcut(tr("Ctrl+r"));
  connect(reloadUrlAction, SIGNAL(triggered()), viewer, SLOT(reloadUrl()));
  fileMenu->addAction(reloadUrlAction);

  QAction *hirateAction = new QAction(tr("&Hirate"), this);
  connect(hirateAction, SIGNAL(triggered()), viewer, SLOT(hirate()));
  fileMenu->addAction(hirateAction);

  QAction *networkAction = new QAction(tr("Open Network &Connection"), this);
  connect(networkAction, SIGNAL(triggered()), viewer, SLOT(network()));
  fileMenu->addAction(networkAction);

  QAction *viewAction = new QAction(tr("&View"), this);
  connect(viewAction, SIGNAL(triggered()), viewer, SLOT(view()));
  fileMenu->addAction(viewAction);

  QAction *viewInNewTabAction = new QAction(tr("View In New &Tab"), this);
  connect(viewInNewTabAction, SIGNAL(triggered()), viewer, SLOT(viewInNewTab()));
  fileMenu->addAction(viewInNewTabAction);

  QAction *watchFileAction = new QAction(tr("&Watch File"), this);
  watchFileAction->setCheckable(true);
  connect(watchFileAction, SIGNAL(toggled(bool)), viewer, SLOT(watchFile(bool)));
  fileMenu->addAction(watchFileAction);

  QAction *saveToAction = new QAction(tr("Save &Moves To Now"), this);
  connect(saveToAction, SIGNAL(triggered()), viewer, SLOT(saveMovesToCurrent()));
  fileMenu->addAction(saveToAction);

  QAction *autoSaveMovesAction = new QAction(tr("&Auto Save Moves..."), this);
  connect(autoSaveMovesAction, SIGNAL(triggered()), viewer, SLOT(autoSaveMoves()));
  fileMenu->addAction(autoSaveMovesAction);
  autoSaveMovesAction->setWhatsThis(tr("Select a file where moves played in the Kifu viewer are saved automatically."));

  QAction *exportAction = new QAction(tr("&Export"), this);
  connect(exportAction, SIGNAL(triggered()), viewer, SLOT(exportCurrent()));
  fileMenu->addAction(exportAction);

  QAction *exitAction = new QAction(tr("&Quit"), this);
  exitAction->setShortcut(tr("Ctrl+q"));
  connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
  fileMenu->addAction(exitAction);

  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
  QAction *copyAction = new QAction(tr("&Copy"), this);
  copyAction->setShortcut(tr("Ctrl+c"));
  connect(copyAction, SIGNAL(triggered()), viewer, SLOT(copy()));
  editMenu->addAction(copyAction);

  QAction *copyBoardAndMovesAction =
    new QAction(tr("Copy &Board and Moves"), this);
  connect(copyBoardAndMovesAction, SIGNAL(triggered()),
	  viewer, SLOT(copyBoardAndMoves()));
  editMenu->addAction(copyBoardAndMovesAction);

  QAction *copyUsiAction = new QAction(tr("Copy &USI"), this);
  connect(copyUsiAction, SIGNAL(triggered()), viewer, SLOT(copyUsi()));
  editMenu->addAction(copyUsiAction);

  QAction *copyBoardAndMovesUsiAction =
    new QAction(tr("Copy Board and Moves USI"), this);
  connect(copyBoardAndMovesUsiAction, SIGNAL(triggered()),
	  viewer, SLOT(copyBoardAndMovesUsi()));
  editMenu->addAction(copyBoardAndMovesUsiAction);

  QAction *pasteAction = new QAction(tr("&Paste"), this);
  pasteAction->setShortcut(tr("Ctrl+v"));
  connect(pasteAction, SIGNAL(triggered()), viewer, SLOT(paste()));
  editMenu->addAction(pasteAction);

  editMenu->addSeparator();

  QAction *editStateAction = new QAction(tr("Edit B&oard"), this);
  connect(editStateAction, SIGNAL(triggered()),
	  viewer, SLOT(editState()));
  editMenu->addAction(editStateAction);

  QMenu *goMenu = menuBar()->addMenu(tr("&Go"));

  QAction *initialAction = new QAction(tr("&Initial State"), this);
  connect(initialAction, SIGNAL(triggered()), viewer, SLOT(toInitialState()));
  initialAction->setIcon(loadPixmap("beginning.png"));
  goMenu->addAction(initialAction);

  QAction *forwardAction = new QAction(tr("&Forward"), this);
  forwardAction->setShortcut(tr("Ctrl+f"));
  forwardAction->setIcon(loadPixmap("forward.png"));
  connect(forwardAction, SIGNAL(triggered()), viewer, SLOT(forward()));
  goMenu->addAction(forwardAction);

  QAction *backAction = new QAction(tr("&Backward"), this);
  backAction->setShortcut(tr("Ctrl+b"));
  backAction->setIcon(loadPixmap("backward.png"));
  connect(backAction, SIGNAL(triggered()), viewer, SLOT(backward()));
  goMenu->addAction(backAction);

  QAction *lastAction = new QAction(tr("&Last"), this);
  lastAction->setIcon(loadPixmap("end.png"));
  connect(lastAction, SIGNAL(triggered()), viewer, SLOT(toLastState()));
  goMenu->addAction(lastAction);

  QMenu *analysisMenu = menuBar()->addMenu(tr("&Analyze"));

  QAction *analyzeAction = new QAction(tr("&Search"), this);
  connect(analyzeAction, SIGNAL(triggered()), viewer, SLOT(analyze()));
  analysisMenu->addAction(analyzeAction);

  QAction *analyzeInNewTabAction = new QAction(tr("Search In New &Tab"), this);
  connect(analyzeInNewTabAction, SIGNAL(triggered()), viewer,
	  SLOT(analyzeInNewTab()));
  analysisMenu->addAction(analyzeInNewTabAction);
  QAction *checkmateAction = new QAction(tr("&Checkmate Search"), this);
  connect(checkmateAction, SIGNAL(triggered()), viewer, SLOT(checkmateSearch()));
  analysisMenu->addAction(checkmateAction);

  QAction *altCheckmateAction = new QAction(tr("C&heckmate Search (alt)"), this);
  connect(altCheckmateAction, SIGNAL(triggered()), viewer, SLOT(altCheckmateSearch()));
  analysisMenu->addAction(altCheckmateAction);

#ifndef OSL_PUBLIC_RELEASE
  QAction *quiescenceHalfAction = new QAction(tr("Q&uiescence Search Half Depth"), this);
  connect(quiescenceHalfAction, SIGNAL(triggered()), viewer, SLOT(quiescenceSearchHalf()));
  analysisMenu->addAction(quiescenceHalfAction);

  analysisMenu->addSeparator();

  QAction *moveGenerateAction = new QAction(tr("Show &Moves"), this);
  connect(moveGenerateAction, SIGNAL(triggered()), viewer, SLOT(moveGenerateDialog()));
  analysisMenu->addAction(moveGenerateAction);

  analysisMenu->addSeparator();

  QAction *kifuAnalyzeAction = new QAction(tr("Analyze &Kifu"), this);
  connect(kifuAnalyzeAction, SIGNAL(triggered()), viewer, SLOT(kifuAnalyze()));
  analysisMenu->addAction(kifuAnalyzeAction);

  QMenu *evalDebugMenu = menuBar()->addMenu(tr("&Debug"));

  QAction *evalDebugAction = new QAction(tr("Analyze Evaluation Function (OpenMidEndingEvavl)"),
					 this);
  connect(evalDebugAction, SIGNAL(triggered()), viewer,
	  SLOT(testEvalDebug()));
  evalDebugMenu->addAction(evalDebugAction);

  QAction *progressDebugAction = new QAction(tr("Analyze Progress"), this);
  evalDebugMenu->addAction(progressDebugAction);
  connect(progressDebugAction, SIGNAL(triggered()), viewer,
          SLOT(showProgressDebug()));
  evalDebugMenu->addSeparator();

  QAction *evalDebugSetStatus1Action = new QAction(tr("Set first status to be analyzed"), this);
  QAction *evalDebugSetStatus2Action = new QAction(tr("Set second status to be analyzed"), this);
  QAction *evalDebugDiffAction = new QAction(tr("Show Eval Diff"), this);
  evalDebugMenu->addAction(evalDebugSetStatus1Action);
  connect(evalDebugSetStatus1Action, SIGNAL(triggered()), viewer,
          SLOT(setState1()));
  evalDebugMenu->addAction(evalDebugSetStatus2Action);
  connect(evalDebugSetStatus2Action, SIGNAL(triggered()), viewer,
          SLOT(setState2()));
  evalDebugMenu->addAction(evalDebugDiffAction);
  connect(evalDebugDiffAction, SIGNAL(triggered()), viewer,
          SLOT(showEvalDiff()));
#endif
  analyzeOnlineAction = new QAction(tr("Analyze &Online"), this);
  connect(analyzeOnlineAction, SIGNAL(toggled(bool)), viewer,
	  SLOT(setAnalyzeOnline(bool)));
  analyzeOnlineAction->setCheckable(true);
  analysisMenu->addAction(analyzeOnlineAction);

  QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

  flipAction = new QAction(tr("&Flip View"), this);
  connect(flipAction, SIGNAL(toggled(bool)), viewer,
	  SLOT(setOrientation(bool)));
  flipAction->setCheckable(true);
  viewMenu->addAction(flipAction);
  effectAction = new QAction(tr("Show &Effect"), this);
  connect(effectAction, SIGNAL(toggled(bool)), viewer,
	  SLOT(enableEffect(bool)));
  effectAction->setCheckable(true);
  viewMenu->addAction(effectAction);
  QAction *evalGraphAction = new QAction(tr("Evaluation &Graph"), this);
  viewMenu->addAction(evalGraphAction);
  connect(evalGraphAction, SIGNAL(triggered()), viewer, SLOT(openEvalGraph()));
  lastMoveAction = new QAction(tr("Highlight &Last Move"), this);
  connect(lastMoveAction, SIGNAL(toggled(bool)), viewer,
	  SLOT(highlightLastMove(bool)));
  lastMoveAction->setCheckable(true);
  viewMenu->addAction(lastMoveAction);
  lastMoveAction->toggle();

  bookMoveAction = new QAction(tr("Highlight &Book Move"), this);
  connect(bookMoveAction, SIGNAL(toggled(bool)), viewer,
	  SLOT(highlightBookMove(bool)));
  bookMoveAction->setCheckable(true);
  viewMenu->addAction(bookMoveAction);
  bookMoveAction->toggle();

  arrowMoveAction = new QAction(tr("Show &Arrows for Moves"), this);
  connect(arrowMoveAction, SIGNAL(toggled(bool)), viewer,
	  SLOT(showArrowMove(bool)));
  arrowMoveAction->setCheckable(true);
  viewMenu->addAction(arrowMoveAction);

  QToolBar *toolbar = addToolBar(tr("Go"));
  toolbar->addAction(loadPixmap("beginning.png"),
		     "&Initial State",
		     viewer, SLOT(toInitialState()));
  toolbar->addAction(loadPixmap("backward.png"),
		     tr("&Backward"),
		     viewer, SLOT(backward()));
  toolbar->addAction(loadPixmap("forward.png"),
		     tr("&Forward"),
		     viewer, SLOT(forward()));
  toolbar->addAction(loadPixmap("end.png"),
		     tr("&Last"),
		     viewer, SLOT(toLastState()));

  moveLabel = new QLabel(QString::fromUtf8(" 1234手 "), statusBar());
  moveLabel->setAlignment(Qt::AlignHCenter);
  moveLabel->setMinimumSize(moveLabel->sizeHint());
  statusBar()->addWidget(moveLabel);

  turnLabel = new QLabel(QString::fromUtf8(" 先手 "), statusBar());
  turnLabel->setAlignment(Qt::AlignHCenter);
  turnLabel->setMinimumSize(turnLabel->sizeHint());
  statusBar()->addWidget(turnLabel);

  evalLabel = new ClickableLabel(QString::fromUtf8("評価値: -1234 (-1234 -1234 -1234 -1234) -1234"),
				 statusBar());
  evalLabel->setToolTip(QString::fromUtf8("評価値: 合計 (序盤 中盤1 中盤2 終盤"
					  ") 緊張度"));
  evalLabel->setAlignment(Qt::AlignHCenter);
  evalLabel->setMinimumSize(evalLabel->sizeHint());
  statusBar()->addWidget(evalLabel);
  connect(evalLabel, SIGNAL(clicked()), viewer, SLOT(testEvalDebug()));

  progressLabel = new CopyRateLabel(QString::fromUtf8("進行度: 12 (12 12)"),
				    statusBar());
  progressLabel->setToolTip(QString::fromUtf8("進行度: 総合 (先手 後手)"));
  progressLabel->setAlignment(Qt::AlignHCenter);
  progressLabel->setMinimumSize(progressLabel->sizeHint());
  statusBar()->addWidget(progressLabel);

  threatmateLabel = new CopyRateLabel(QString::fromUtf8("詰めろ? 12.3 34.5"),
				      statusBar());
  threatmateLabel->setAlignment(Qt::AlignHCenter);
  threatmateLabel->setMinimumSize(threatmateLabel->sizeHint());
  statusBar()->addWidget(threatmateLabel);

  filenameLabel = new CopyLabel(QString::fromUtf8("12345678901234567890"),
				statusBar());
  filenameLabel->setAlignment(Qt::AlignHCenter);
  filenameLabel->setMinimumSize(filenameLabel->sizeHint());
  statusBar()->addWidget(filenameLabel);

  std::string gpsname = "gpsshogi ";
#ifdef OSL_SMP
  gpsname += "(smp) ";
#endif
  gpsname += gpsshogi::gpsshogi_revision;
  QLabel *revisionLabel = new CopyLabel(QString::fromStdString(gpsname), statusBar());
  revisionLabel->setAlignment(Qt::AlignHCenter);
  revisionLabel->setMinimumSize(revisionLabel->sizeHint());
  statusBar()->addWidget(revisionLabel);

  connect(viewer, SIGNAL(statusChanged()),
	  this, SLOT(updateStatusBar()));
  connect(viewer, SIGNAL(orientationChanged(bool)),
	  this, SLOT(updateFlipButton(bool)));
  connect(viewer, SIGNAL(effectChanged(bool)),
	  this, SLOT(updateEffectButton(bool)));
  connect(viewer, SIGNAL(analyzeOnlineDisabled()),
	  this, SLOT(turnOffAnalyzeOnlineButton()));
  updateStatusBar();
}

void MainWindow::updateStatusBar()
{
  moveLabel->setText(QString::fromUtf8(" %1手 ").arg(viewer->moveCount()));
  if (viewer->turn() == osl::BLACK)
    turnLabel->setText(QString::fromUtf8("先手"));
  else
    turnLabel->setText(QString::fromUtf8("後手"));

  const viewer::EvalInfo eval = viewer->evalState();
  evalLabel->setText(QString::fromUtf8("評価値: %1 (%2 %3 %4 %5) %6")
		     .arg(eval.total).arg(eval.opening).arg(eval.mid1)
		     .arg(eval.mid2).arg(eval.ending).arg(eval.tension));
  evalLabel->setRate(eval.total / 3000.0);
  std::vector<int> progress;
  viewer->progressOfState(progress);
  progressLabel->setText(QString::fromUtf8("進行度: %1 (%2 %3)")
			 .arg(progress[0]).arg(progress[1]).arg(progress[2]));
  progressLabel->setRate(progress[0]/16.0);
  const std::pair<double,double> threatmate_probability = viewer->checkmateProbability();
  threatmateLabel->setText(QString::fromUtf8("詰めろ? %1 %2")
			   .arg(threatmate_probability.first*100,0,'g',3)
			   .arg(threatmate_probability.second*100,0,'g',3));
  threatmateLabel->setRate(threatmate_probability.first);
  filenameLabel->setText(viewer->getFilename());
}

void MainWindow::updateFlipButton(bool sente)
{
  flipAction->setChecked(!sente);
}

void MainWindow::updateEffectButton(bool on)
{
  effectAction->setChecked(on);
}

void MainWindow::turnOffAnalyzeOnlineButton()
{
  analyzeOnlineAction->setChecked(false);
}

QPixmap MainWindow::loadPixmap(const QString &name)
{
  return QPixmap(":/images/" + name);
}

void MainWindow::setUsiProxyAvailable(bool b)
{
  usiProxyAvailable = b;
}

bool MainWindow::isUsiProxyAvailable() const
{
  return usiProxyAvailable;
}

