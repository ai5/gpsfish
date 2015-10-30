#include "viewer.h"
#include "tabchild.h"
#include "analysisViewer.h"
#include "checkmateViewer.h"
#include "kifuViewer.h"
#include "quiescenceViewer.h"
#include "networkViewer.h"
#include "moveGeneratorDialog.h"
#include "ui_kisendialog4.h"
#include "ui_networkdialog4.h"
#include "kifuFile.h"
#include "evalDebug.h"
#include "kisenModel.h"
#include "progressDebug.h"

#include "httpwindow.h"
#include "boardEditor2.h"

#include "osl/search/simpleHashTable.h"
#include "osl/eval/progressEval.h"
#include "osl/numEffectState.h"
#include "osl/progress/effect5x3.h"
#include "osl/record/kisen.h"
#include "osl/record/kakinoki.h"
#include "osl/csa.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/threatmate/mlPredictor.h"
#include "osl/threatmate/treePredictor.h"
#include <QFileDialog>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qtextcodec.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <QClipboard>

#include <stdexcept>

Viewer::Viewer(QWidget *parent)
  : QTabWidget(parent), chatIcon(false)
{
  osl::eval::ProgressEval::setUp();
  osl::eval::ml::OpenMidEndingEval::setUp();
  osl::eval::ml::NewProgress::setUp();
  kifuViewer = new KifuViewer(this);
  addTab(kifuViewer, "Kifu");
  analysisViewer = 0;
  checkmateViewer = new CheckmateViewer(this);
  addTab(checkmateViewer, "Check Search");
  quiescenceViewer = new QuiescenceViewer(this);
  addTab(quiescenceViewer, "Quiescence Search");
  networkViewer = new NetworkViewer(this);
  addTab(networkViewer, "Network");
  editor = new BoardEditor2(this);
  addTab(editor, "Edit Board");

  QPixmap close = QPixmap(":/images/close.png");

  QPushButton *closeButton = new QPushButton(close, "", this);
  setCornerWidget(closeButton);

  connect(this, SIGNAL(currentChanged(int)),
	  this, SIGNAL(statusChanged()));
  connect(kifuViewer, SIGNAL(statusChanged()),
	  this, SIGNAL(statusChanged()));
  connect(kifuViewer, SIGNAL(analyzeOnlineDisabled()),
	  this, SIGNAL(analyzeOnlineDisabled()));
  connect(checkmateViewer, SIGNAL(statusChanged()),
	  this, SIGNAL(statusChanged()));
  connect(quiescenceViewer, SIGNAL(statusChanged()),
	  this, SIGNAL(statusChanged()));
  connect(networkViewer, SIGNAL(statusChanged()),
	  this, SIGNAL(statusChanged()));
  connect(closeButton, SIGNAL(clicked()),
	  this, SLOT(closeTab()));
  connect(this, SIGNAL(currentChanged(int)),
	  this, SLOT(notifyOrientation(int)));
  connect(this, SIGNAL(currentChanged(int)),
	  this, SLOT(notifyEffect(int)));

  connect(networkViewer, SIGNAL(chatReceived()),
	  this, SLOT(chatReceived()));
  connect(networkViewer, SIGNAL(painted()),
	  this, SLOT(chatDisplayed()));

  resetIcon();
}

const viewer::EvalInfo Viewer::evalState()
{
  viewer::EvalInfo result;
  TabChild *child = (TabChild *)currentWidget();
  osl::NumEffectState nState(child->getState());
  if (nState.kingPiece(osl::BLACK).isOnBoard() &&
      nState.kingPiece(osl::WHITE).isOnBoard())
  {
    osl::eval::ml::OpenMidEndingEval eval(nState);
#ifdef EVAL_QUAD
    const int piece_scale = osl::progress::ml::NewProgress::maxProgress()/3;
#else
    const int piece_scale = osl::progress::ml::NewProgress::maxProgress();
#endif
    result.total = eval.value() / piece_scale;
    result.opening = eval.openingValue();
    result.mid1 = eval.midgameValue();
#ifdef EVAL_QUAD
    result.mid2 = eval.midgame2Value();
#endif
    result.ending  = eval.endgameValue();
    osl::eval::PieceEval piece(nState);
    result.tension = result.total - piece.value();
  }
  return result;
}

void Viewer::progressOfState(std::vector<int>& out)
{
  TabChild *child = (TabChild *)currentWidget();
  out.clear();
  // TODO: better abstraction needed
  if (currentWidget() == editor)
  {
    for (int i = 0; i < 3; i++)
      out.push_back(0);
    return;
  }
  osl::NumEffectState nState(child->getState());
  if (nState.kingPiece(osl::BLACK).isOnBoard() &&
      nState.kingPiece(osl::WHITE).isOnBoard())
  {
    osl::progress::ml::NewProgress progress(nState);
    out.push_back(progress.progress16().value());
    out.push_back(progress.progress16(osl::BLACK).value());
    out.push_back(progress.progress16(osl::WHITE).value());
  }
  else
  {
    for (int i = 0; i < 3; i++)
      out.push_back(0);
  }
}

std::pair<double,double> Viewer::checkmateProbability()
{
  TabChild *child = (TabChild *)currentWidget();
  std::vector<osl::Move> moves;
  osl::SimpleState state = child->getStateAndMovesToCurrent(moves);

  if ((moves.size() == 0) || (! moves.back().isNormal()))
    return std::make_pair(0.0, 0.0);

  if (!state.kingPiece(osl::BLACK).isOnBoard() ||
      !state.kingPiece(osl::WHITE).isOnBoard())
    return std::make_pair(0.0, 0.0);

  osl::NumEffectState nState(state);
  for (size_t i=0; i<moves.size(); ++i)
    nState.makeMove(moves[i]);
  osl::threatmate::MlPredictor predictor;
  osl::threatmate::TreePredictor tree;
  return std::make_pair(predictor.probability(nState, moves.back()),
			tree.probability(nState, moves.back()));
}


void Viewer::forward()
{
  setChatIcon();
  TabChild *child = (TabChild *)currentWidget();
  child->forward();
}

void Viewer::backward()
{
  resetIcon();
  TabChild *child = (TabChild *)currentWidget();
  child->backward();
}

void Viewer::toInitialState()
{
  TabChild *child = (TabChild *)currentWidget();
  child->toInitialState();
}

void Viewer::toLastState()
{
  TabChild *child = (TabChild *)currentWidget();
  child->toLastState();
}

void Viewer::open(const QString& fileName)
{
  kifuViewer->open(new CsaFile(fileName.toLocal8Bit().constData()));
  setCurrentIndex(indexOf(kifuViewer));
}

void Viewer::openKakinoki(const QString& fileName)
{
  kifuViewer->open(new KakinokiFile(fileName.toLocal8Bit().constData()));
  setCurrentIndex(indexOf(kifuViewer));
}

void Viewer::openKi2(const QString& fileName)
{
  kifuViewer->open(new Ki2File(fileName.toLocal8Bit().constData()));
  setCurrentIndex(indexOf(kifuViewer));
}

void Viewer::openUsi(const QString& fileName)
{
  kifuViewer->open(new UsiFile(fileName.toLocal8Bit().constData()));
  setCurrentIndex(indexOf(kifuViewer));
}

void Viewer::open(const QString& fileName, int index)
{
  kifuViewer->open(new KisenFile(fileName.toLocal8Bit().constData(), index));
  setCurrentIndex(indexOf(kifuViewer));
}

void Viewer::openEvalGraph()
{
  kifuViewer->openEvalGraph(true);
  setCurrentIndex(indexOf(kifuViewer));
}

void Viewer::open()
{
  QFileDialog *dialog = new QFileDialog(this);
  QString filename = kifuViewer->getFilename();
  if (!filename.isNull())
  {
    QFileInfo fileInfo(filename);
    dialog->setDirectory(fileInfo.dir());
  }
    
  dialog->setNameFilter("Kisen (*.kpf *.kif)");
  dialog->setNameFilter("CSA (*.csa)");
  dialog->setNameFilter("Kakinoki (*.kif)");
  dialog->setNameFilter("Usi (*.usi)");
  dialog->setNameFilter("Shogi files (*.csa *.kpf *.kif *.ki2 *.usi)");
  if (dialog->exec() == QDialog::Accepted)
  {
    const QStringList files = dialog->selectedFiles();
    QString selectedFile;
    if (!files.isEmpty())
      selectedFile = files[0];
    if (selectedFile.endsWith(".csa", Qt::CaseInsensitive))
    {
      open(selectedFile);
    }
    else if (selectedFile.endsWith(".usi", Qt::CaseInsensitive))
    {
      openUsi(selectedFile);
    }
    else if (selectedFile.endsWith(".ki2", Qt::CaseInsensitive))
    {
      openKi2(selectedFile);
    }
    else if (selectedFile.endsWith(".kif", Qt::CaseInsensitive)
	     && osl::KakinokiFile::isKakinokiFile(selectedFile.toLocal8Bit().constData()))
    {
      openKakinoki(selectedFile);
    }
    else if (selectedFile.endsWith(".kif", Qt::CaseInsensitive))
    {
      QString ipxFile(selectedFile);
      ipxFile.replace(ipxFile.length() - 3, 3, "ipx");
      osl::record::KisenIpxFile ipx(ipxFile.toStdString());
      if (ipx.size() > 1000)
      {
	bool ok;
	int index = QInputDialog::getInt(this, "Kisen Index",
					 QString("Choose Kisen Index %1").arg(ipx.size()),
					 0, 0, ipx.size(), 1, &ok);
	if (ok)
	{
	  open(selectedFile, index);
	}
      }
      else
      {
	std::unique_ptr<QDialog> qdialog(new QDialog(this));
	std::unique_ptr<Ui_kisenDialog> dialog(new Ui_kisenDialog());
        dialog->setupUi(qdialog.get());
        std::unique_ptr<KisenModel> kisen_model(new KisenModel(&ipx, this));
        dialog->tableView->setModel(kisen_model.get());
        dialog->tableView->verticalHeader()->hide();
        dialog->tableView->resizeColumnsToContents();
        dialog->tableView->setShowGrid(false);
	int ret = qdialog->exec();
	if (ret == QDialog::Accepted)
	{
          QModelIndexList selected =
            dialog->tableView->selectionModel()->selection().indexes();
	  if (!selected.empty())
	  {
	    open(selectedFile, selected.first().row());
	  }
	}
      }
    }
    else
    {
      QMessageBox::warning(this, "Open File",
			   "Unknown file type " + selectedFile);
      return;
    }
    emit statusChanged();
  }
}

void Viewer::nextFile()
{
  const KifuFile *file = kifuViewer->getKifuFile();
  if (file == NULL)
    return;
  KifuFile *next_file = file->nextFile();
  if (next_file)
  {
    kifuViewer->open(next_file);
    setCurrentIndex(indexOf(kifuViewer));
    emit statusChanged();
  }
}

void Viewer::prevFile()
{
  const KifuFile *file = kifuViewer->getKifuFile();
  if (file == NULL)
    return;
  KifuFile *prev_file = file->prevFile();
  if (prev_file)
  {
    kifuViewer->open(prev_file);
    setCurrentIndex(indexOf(kifuViewer));
    emit statusChanged();
  }
}

void Viewer::watchFile(bool enable)
{
  // FIXME allow each kifuviewer to watch its own file
  kifuViewer->watchFile(enable);
}

void Viewer::openUrl(bool reload)
{
  HttpWindow *http = new HttpWindow(this, reload);
  http->show();
  if (http->exec() == QDialog::Accepted)
  {
    if (http->getFilename().endsWith(".kif", Qt::CaseInsensitive))
      openKakinoki(http->getFilename());
    else if (http->getFilename().endsWith(".ki2", Qt::CaseInsensitive))
      openKi2(http->getFilename());
    else if (http->getFilename().endsWith(".usi", Qt::CaseInsensitive))
      openUsi(http->getFilename());
    else
      open(http->getFilename());
    kifuViewer->toLastState();
  }
}

void Viewer::openUrl()
{
  openUrl(false);
}

void Viewer::reloadUrl()
{
  openUrl(true);
}

void Viewer::hirate()
{
  TabChild *child = (TabChild *)currentWidget();
  if (!child->inherits("KifuViewer"))
    return;
  KifuViewer *kifu = (KifuViewer *)child;

  std::vector<osl::Move> moves;
  osl::SimpleState state(osl::HIRATE);
  kifu->open(state, moves);
  setCurrentIndex(indexOf(kifu));
}

void Viewer::network()
{
  std::unique_ptr<QDialog> qdialog(new QDialog(this));
  std::unique_ptr<Ui::NetworkDialog> dialog(new Ui::NetworkDialog());
  dialog->setupUi(qdialog.get());

  qdialog->exec();

  networkViewer->setHostname(dialog->servername->text());
  networkViewer->setUsername(dialog->username->text());
  networkViewer->setPassword(dialog->password->text());
  networkViewer->connect();

  setCurrentIndex(indexOf(networkViewer));
}

void Viewer::view()
{
  TabChild *child = (TabChild *)currentWidget();
  if (child == kifuViewer)
    return;

  std::vector<osl::Move> moves;
  osl::SimpleState state = child->getStateAndMovesToCurrent(moves);
  kifuViewer->open(state, moves);
  kifuViewer->toLastState();
  setCurrentIndex(indexOf(kifuViewer));
}

void Viewer::viewInNewTab()
{
  TabChild *child = (TabChild *)currentWidget();

  KifuViewer *viewer = new KifuViewer(this);
  connect(viewer, SIGNAL(statusChanged()), this, SIGNAL(statusChanged()));
  connect(viewer, SIGNAL(analyzeOnlineDisabled()),
	  this, SIGNAL(analyzeOnlineDisabled()));
  std::vector<osl::Move> moves;
  osl::SimpleState state = child->getStateAndMovesToCurrent(moves);
  viewer->open(state, moves);
  viewer->toLastState();
  addTab(viewer, "Kifu");
  setCurrentIndex(indexOf(viewer));
}

int Viewer::moveCount() const
{
  TabChild *child = (TabChild *)currentWidget();
  return child->moveCount();
}

osl::Player Viewer::turn() const
{
  TabChild *child = (TabChild *)currentWidget();
  return child->turn();
}

const osl::SimpleState& Viewer::getState()
{
  TabChild *child = (TabChild *)currentWidget();
  return child->getState();
}

void Viewer::analyze()
{
  analyze(false);
}

void Viewer::analyzeInNewTab()
{
  analyze(true);
}

void Viewer::analyze(bool newTab)
{
  bool tabCreated = false;
  TabChild *child = (TabChild *)currentWidget();
  std::vector<osl::Move> moves;
  osl::SimpleState state = child->getStateAndMovesToCurrent(moves);
  AnalysisViewer *av;

  if (!analysisViewer || newTab)
  {
    tabCreated = true;
    av = new AnalysisViewer(this);
    connect(av, SIGNAL(statusChanged()),
	    this, SIGNAL(statusChanged()));
  }
  else if (child->inherits("AnalysisViewer"))
  {
    av = (AnalysisViewer *)child;
  }
  else
    av = analysisViewer;

  bool success = av->analyze(state, moves);

  if (success)
  {
    if (tabCreated)
    {
      if (!analysisViewer)
	analysisViewer = av;
      addTab(av, "Analyze position");
    }
    setCurrentIndex(indexOf(av));
  }
  else
  {
    if (tabCreated)
    {
      delete av;
    }
  }
}

void Viewer::checkmateSearch()
{
  TabChild *child = (TabChild *)currentWidget();
  const int node_limit = 100000;
  bool success = checkmateViewer->analyze(child->getState(), node_limit);
  if (success)
    setCurrentIndex(indexOf(checkmateViewer));
}

void Viewer::altCheckmateSearch()
{
  TabChild *child = (TabChild *)currentWidget();
  const int node_limit = 100000;
  bool success = checkmateViewer->analyze(child->getState(), node_limit, true);
  if (success)
    setCurrentIndex(indexOf(checkmateViewer));
}

void Viewer::checkmateSimulation()
{
  TabChild *child = (TabChild *)currentWidget();
  bool success = checkmateViewer->analyze(child->getState(), 0);
  if (success)
    setCurrentIndex(indexOf(checkmateViewer));
}

void Viewer::quiescenceSearch()
{
  TabChild *child = (TabChild *)currentWidget();
  std::vector<osl::Move> moves;
  osl::SimpleState state = child->getStateAndMovesToCurrent(moves);
  bool success = quiescenceViewer->analyze(state, moves);
  if (success)
    setCurrentIndex(indexOf(quiescenceViewer));
}

void Viewer::quiescenceSearchHalf()
{
  TabChild *child = (TabChild *)currentWidget();
  std::vector<osl::Move> moves;
  osl::SimpleState state = child->getStateAndMovesToCurrent(moves);
  bool success = quiescenceViewer->analyzeHalfDepth(state, moves);
  if (success)
    setCurrentIndex(indexOf(quiescenceViewer));
}

void Viewer::moveGenerateDialog()
{
  if (currentWidget()->inherits("BoardAndListTabChild"))
  {
    ((BoardAndListTabChild *)currentWidget())->moveGenerateDialog();
    return;
  }

  TabChild *child = (TabChild *)currentWidget();
  child->moveGenerateDialog();
}

void Viewer::toggleOrientation()
{
  TabChild *child = (TabChild *)currentWidget();
  child->toggleOrientation();
}

QString Viewer::getFilenameToSave()
{
  QString s = QFileDialog::getSaveFileName(this, 
                                           "Save moves up to current state",
                                           "",
					   "CSA (*.csa)");
  return s;
}

void Viewer::saveMovesToCurrent()
{
  QString s = getFilenameToSave();
  if (s.isNull())
    return;
  TabChild *child = (TabChild *)currentWidget();
  std::vector<osl::Move> moves;
  osl::SimpleState state = child->getStateAndMovesToCurrent(moves);
  std::ofstream os(s.toStdString().c_str());
  os << state;
  for (int i = 0; i < (int)moves.size(); i++)
  {
    os << osl::csa::show(moves[i]) << std::endl;
  }
}

void Viewer::autoSaveMoves()
{
  const QString fileName = 
    QFileDialog::getSaveFileName(this,
                                 tr("Auto Save file"), "", tr("Usi (*.usi)"));
  kifuViewer->setAutoSaveFile(fileName);
}

void Viewer::exportCurrent()
{
  QString s = getFilenameToSave();
  if (s.isNull())
    return;
  TabChild *child = (TabChild *)currentWidget();
  std::ofstream os(s.toStdString().c_str());
  os << child->getState();
}

void Viewer::moveTo(int index)
{
  TabChild *child = (TabChild *)currentWidget();
  if (child == kifuViewer)
  {
    kifuViewer->updateIndex(index);
  }
}

void Viewer::copy()
{
  TabChild *child = (TabChild *)currentWidget();
  child->copy();
}

void Viewer::copyBoardAndMoves()
{
  TabChild *child = (TabChild *)currentWidget();
  child->copyBoardAndMoves();
}

void Viewer::copyUsi()
{
  TabChild *child = (TabChild *)currentWidget();
  child->copyUsi();
}

void Viewer::copyBoardAndMovesUsi()
{
  TabChild *child = (TabChild *)currentWidget();
  child->copyBoardAndMovesUsi();
}

void Viewer::paste()
{
  QWidget *widget = currentWidget();
  if (widget->inherits("KifuViewer"))
  {
    QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    if (!text.isEmpty())
    {
      try
      {
        osl::CsaString csa(text.toUtf8().data());
        std::vector<osl::Move> board_moves;
        ((KifuViewer *)widget)->open(csa.initialState(), board_moves);
      }
      catch(...)
      {
      }
    }
  }  
}

QString Viewer::getFilename()
{
  TabChild *child = (TabChild *)currentWidget();
  QString filename = child->getFilename();
  if (filename.isNull())
    return "";

  QFileInfo fi(filename);
  return fi.fileName();
}

void Viewer::closeTab()
{
  QWidget *widget = currentWidget();
  if (widget->inherits("AnalysisViewer"))
  {
    removeTab(indexOf(widget));
    if (widget == analysisViewer)
    {
      analysisViewer = 0;
      for (int i = 0; i < count(); i++)
      {
	QWidget *child = this->widget(i);
	if (child->inherits("AnalysisViewer"))
	{
	  analysisViewer = (AnalysisViewer *)child;
	  break;
	}
      }
    }
    delete widget;
  }
  else if (widget != kifuViewer && widget->inherits("KifuViewer"))
  {
    removeTab(indexOf(widget));
    delete widget;
  }
}

bool Viewer::isSenteView() const
{
  if (currentWidget()->inherits("BoardTabChild"))
  {
    return ((BoardTabChild *)currentWidget())->isSenteView();
  }
  return true;
}

void Viewer::setOrientation(bool gote)
{
  if (currentWidget()->inherits("BoardTabChild"))
  {
    return ((BoardTabChild *)currentWidget())->setOrientation(!gote);
  }
}

void Viewer::setAnalyzeOnline(bool enable)
{
  KifuViewer::setAnalyzeOnline(enable);
  if (enable)
  {
    if (currentWidget()->inherits("KifuViewer")) {
      osl::SimpleState s;
      std::vector<osl::Move> m;
      s = ((KifuViewer *)currentWidget())->getStateAndMovesToCurrent(m);
      ((KifuViewer *)currentWidget())->prepareUpdateForAnalysis(s, m, 0, osl::Move());
    }
    else
      setCurrentIndex(indexOf(kifuViewer));
  }
}

void Viewer::notifyOrientation(int index)
{
  QWidget *w = widget(index);
  bool sente;
  if (w->inherits("BoardTabChild"))
    sente = ((BoardTabChild *)w)->isSenteView();
  else
    sente = true;

  emit orientationChanged(sente);
}

void Viewer::enableEffect(bool on)
{
  ((TabChild *)currentWidget())->enableEffect(on);
}

void Viewer::highlightLastMove(bool on)
{
  ((TabChild *)currentWidget())->highlightLastMove(on);
}
void Viewer::highlightBookMove(bool on)
{
  ((TabChild *)currentWidget())->highlightBookMove(on);
}
void Viewer::showArrowMove(bool on)
{
  ((TabChild *)currentWidget())->showArrowMove(on);
}

void Viewer::notifyEffect(int index)
{
  QWidget *w = widget(index);
  if (w->inherits("BoardTabChild"))
    emit effectChanged(((BoardTabChild *)w)->effectEnabled());
}

void Viewer::kifuAnalyze()
{
  if (((TabChild *)currentWidget())->inherits("KifuViewer"))
  {
    ((KifuViewer *)currentWidget())->analyze();
  }
}

void Viewer::resetIcon()
{
#if defined(__APPLE__) || defined(__FreeBSD__)
  qApp->setWindowIcon(QIcon(":/images/icon.png"));
#endif
}

void Viewer::setChatIcon()
{
#if defined(__APPLE__) || defined(__FreeBSD__)
  qApp->setWindowIcon(QIcon(":/images/icon-chat.png"));
#endif
}

void Viewer::chatReceived()
{
  if (!chatIcon)
  {
    chatIcon = true;
    setChatIcon();
  }
}

void Viewer::chatDisplayed()
{
  if (chatIcon)
  {
    chatIcon = false;
    resetIcon();
  }
}

void Viewer::editState()
{
  TabChild *child = (TabChild *)currentWidget();
  editor->setState(child->getState());
  setCurrentIndex(indexOf(editor));
}

void Viewer::testEvalDebug()
{
  TabChild *child = (TabChild *)currentWidget();
  OpenMidEndingEvalDebugDialog *dialog =
    new OpenMidEndingEvalDebugDialog(child->getState(), this);
  if (child->inherits("BoardTabChild"))
  {
    BoardTabChild *board_tab_child = (BoardTabChild *)child;
    connect(board_tab_child,
	    SIGNAL(statusChanged(const osl::SimpleState &,
				 const std::vector<osl::Move> &,
				 int, osl::Move)),
	    dialog, SLOT(setStatus(const osl::SimpleState &,
				   const std::vector<osl::Move> &,
				   int, osl::Move)));
  }
  dialog->show();
  dialog->raise();
}

void Viewer::setState1()
{
  TabChild *child = (TabChild *)currentWidget();
  state1.reset(new osl::SimpleState(child->getState()));
}

void Viewer::setState2()
{
  TabChild *child = (TabChild *)currentWidget();
  state2.reset(new osl::SimpleState(child->getState()));
}

void Viewer::showEvalDiff()
{
  if (state1.get() != NULL && state2.get() != NULL)
  {
    OpenMidEndingEvalDiffDialog *dialog =
      new OpenMidEndingEvalDiffDialog(osl::NumEffectState(*state1),
                                      osl::NumEffectState(*state2), this);
    dialog->show();
    dialog->raise();
  }
}

void Viewer::showProgressDebug()
{
  TabChild *child = (TabChild *)currentWidget();
  osl::NumEffectState state(child->getState());
  NewProgressDebugDialog *dialog = new NewProgressDebugDialog(state, this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
  dialog->raise();
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
