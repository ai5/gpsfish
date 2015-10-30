#include "mainWindow.h"
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>

MainWindow::
MainWindow() : QVBox()
{
  setupMenu();
}

MainWindow::
~MainWindow()
{
}

void MainWindow::
showBoard(QString str)
{
  board = str;
  QMessageBox::information(this, "QT Shogi Client", str);
}

void MainWindow::
setupMenu()
{
  QPopupMenu *file = new QPopupMenu(this);
  file->insertItem("&Quit", this, SLOT(quitClicked()), CTRL+Key_Q );
  file->insertItem("&Save", this, SLOT(saveClicked()), CTRL+Key_S );

  QMenuBar *menu = new QMenuBar(this);
  menu->insertItem("&File", file);
}

void MainWindow::
quitClicked(void)
{
  emit(quit());
}

void MainWindow::
saveClicked(void)
{
  if (board.isEmpty())
  {
    QMessageBox::warning(this, "QT Shogi Client", "No Board Yet!");
    return;
  }
  QString filename = QFileDialog::getSaveFileName(QString::null, "CSA files (*.csa);;All files(*.*)", this);
  if (filename.isEmpty()) return;

  QFile f(filename);
  if (f.exists())
  {
    if (QMessageBox::question(this, "QT Shogi Client", "File exists, overwrite?",
			      QMessageBox::Ok | QMessageBox::Default,
			      QMessageBox::Cancel | QMessageBox::Escape)
	==  QMessageBox::Cancel)
      return;
    
  }

  if (!f.open(IO_WriteOnly))
  {
    QMessageBox::critical(this, "QT Shogi Client", "Could not save file!");
    return;
  }

  QTextStream t(&f);
  t << board;
  f.close();
}

