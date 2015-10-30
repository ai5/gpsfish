#include "kifuFile.h"
#include "osl/record/csaRecord.h"
#include "osl/record/kisen.h"
#include "osl/record/kakinoki.h"
#include "osl/record/ki2.h"
#include "osl/record/ki2IOError.h"
#include "osl/record/usiRecord.h"

#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qregexp.h>
#include <QDateTime>
#include <QTextStream>

#include <sstream>

void KifuFile::getRecordData(const osl::record::Record &record)
{
  moves.clear();
  time.clear();
  searchInfo.clear();
    
  std::vector<std::string> rawComments;
  record.load(moves, time, rawComments, searchInfo);
  QTextCodec *codec = QTextCodec::codecForName("EUC-JP");
  const std::string black = record.player[osl::BLACK];
  const std::string white = record.player[osl::WHITE];
  playerName[playerToIndex(osl::BLACK)] = codec->toUnicode(black.c_str(), black.length());
  playerName[playerToIndex(osl::WHITE)] = codec->toUnicode(white.c_str(), white.length());
  std::string s = record.initial_comment;
  if (! s.empty()) 
  {
    std::istringstream is(s);
    std::string line;
    while (std::getline(is, line))
      initial_comment.push_back(codec->toUnicode(line.c_str(),
						 line.size()));
  }
  for (size_t i = 0; i < rawComments.size(); i++)
  {
    const std::string& comment = rawComments[i];
    comments.push_back(codec->toUnicode(comment.c_str(),
					comment.length()));
  }

  boost::gregorian::date date = record.start_date;
  if (! date.is_special())
    year = date.year();
}



CsaFile::CsaFile(QString f)
  : KifuFile(f), loaded(false)
{
}

bool CsaFile::loadData()
{
  if (!loaded)
  {
    std::unique_ptr<osl::CsaFile> csaFile;
    try
    {
      csaFile.reset(new osl::CsaFile(filename.toStdString()));
    }
    catch (osl::CsaIOError&)
    {
      return false;
    }
    state = csaFile->initialState();
    getRecordData(csaFile->load());
    QFileInfo fileInfo(filename);
    lastLoadedTime = fileInfo.lastModified().toTime_t();
    loaded = true;
  }
  return true;
}

bool CsaFile::reloadIfChanged()
{
  QFile f(filename);
  QFileInfo fileInfo(f);
  uint last_time = fileInfo.lastModified().toTime_t();
  if (last_time > lastLoadedTime)
    lastLoadedTime = last_time;
  else
    return false;
  if (! f.open(QIODevice::ReadOnly))
    return false;

  QTextStream stream(&f);
  QString content = stream.readAll();
  content.truncate(content.lastIndexOf('\n') + 1);
  std::istringstream is(content.toStdString());
  osl::CsaFile csa(is);
  getRecordData(csa.load());
  return true;
}

KifuFile *CsaFile::nextFile() const
{
  QFileInfo fileInfo(filename);
  QDir dir = fileInfo.absolutePath();

  QString name = fileInfo.fileName();
  QStringList files = dir.entryList(QStringList() << "*.csa");

  for (size_t i = 0; i < (size_t)files.size(); i++)
  {
    if (files[i] == name && i + 1 < (size_t)files.size())
    {
      return new CsaFile(dir.absolutePath() + "/" + files[i+1]);
    }
  }
  return NULL;
}

KifuFile *CsaFile::prevFile() const
{
  QFileInfo fileInfo(filename);
  QDir dir = fileInfo.absolutePath();

  QString name = fileInfo.fileName();
  QStringList files = dir.entryList(QStringList() << "*.csa");

  for (size_t i = 0; i < (size_t)files.size(); i++)
  {
    if (files[i] == name && i > 0)
    {
      return new CsaFile(dir.absolutePath() + "/" + files[i-1]);
    }
  }
  return NULL;
}


KakinokiFile::KakinokiFile(QString f)
  : KifuFile(f), loaded(false)
{
}

bool KakinokiFile::loadData()
{
  if (!loaded)
  {
    std::unique_ptr<osl::KakinokiFile> file;
    try
    {
      file.reset(new osl::KakinokiFile(filename.toStdString()));
    }
    catch (osl::KakinokiIOError&)
    {
      return false;
    }
    state = file->initialState();
    getRecordData(file->load());
    loaded = true;
  }
  return true;
}

KifuFile *KakinokiFile::nextFile() const
{
  QFileInfo fileInfo(filename);
  QDir dir = fileInfo.absolutePath();

  QString name = fileInfo.fileName();
  QStringList files = dir.entryList(QStringList() << "*.kif");

  for (size_t i = 0; i < (size_t)files.size(); i++)
  {
    if (files[i] == name && i + 1 < (size_t)files.size())
    {
      return new KakinokiFile(dir.absolutePath() + "/" + files[i+1]);
    }
  }
  return NULL;
}

KifuFile *KakinokiFile::prevFile() const
{
  QFileInfo fileInfo(filename);
  QDir dir = fileInfo.absolutePath();

  QString name = fileInfo.fileName();
  QStringList files = dir.entryList(QStringList() << "*.kif");

  for (size_t i = 0; i < (size_t)files.size(); i++)
  {
    if (files[i] == name && i > 0)
    {
      return new KakinokiFile(dir.absolutePath() + "/" + files[i-1]);
    }
  }
  return NULL;
}


Ki2File::Ki2File(QString f)
  : KifuFile(f), loaded(false)
{
}

bool Ki2File::loadData()
{
  if (!loaded)
  {
    std::unique_ptr<osl::Ki2File> file;
    try
    {
      file.reset(new osl::Ki2File(filename.toStdString()));
    }
    catch (osl::Ki2IOError&)
    {
      return false;
    }
    state = file->initialState();
    getRecordData(file->load());
    loaded = true;
  }
  return true;
}

KifuFile *Ki2File::nextFile() const
{
  QFileInfo fileInfo(filename);
  QDir dir = fileInfo.absolutePath();

  QString name = fileInfo.fileName();
  QStringList files = dir.entryList(QStringList() << "*.kif");

  for (size_t i = 0; i < (size_t)files.size(); i++)
  {
    if (files[i] == name && i + 1 < (size_t)files.size())
    {
      return new Ki2File(dir.absolutePath() + "/" + files[i+1]);
    }
  }
  return NULL;
}

KifuFile *Ki2File::prevFile() const
{
  QFileInfo fileInfo(filename);
  QDir dir = fileInfo.absolutePath();

  QString name = fileInfo.fileName();
  QStringList files = dir.entryList(QStringList() << "*.kif");

  for (size_t i = 0; i < (size_t)files.size(); i++)
  {
    if (files[i] == name && i > 0)
    {
      return new Ki2File(dir.absolutePath() + "/" + files[i-1]);
    }
  }
  return NULL;
}


KisenFile::KisenFile(QString f, int i)
  : KifuFile(f), index(i), loaded(false)
{
}

bool KisenFile::loadData()
{
  if (!loaded)
  {
    try
    {
      if (filename.endsWith(".kpf"))
      {
	osl::record::KisenPlusFile kisenPlusFile(filename.toStdString());
	kisenPlusFile.load(index, moves, time);
	state = kisenPlusFile.initialState();
      }
      else
      {
	osl::record::KisenFile kisenFile(filename.toStdString());
	moves = kisenFile.moves(index);
	time =  std::vector<int>(moves.size(), 1);
	state = kisenFile.initialState();
      }
    }
    catch (osl::CsaIOError&)
    {
      return false;
    }
    QString ipxName(filename);
    ipxName.replace(QRegExp("\\.(kif|kpf)$"), ".ipx");
    osl::record::KisenIpxFile ipx(ipxName.toStdString());
    QTextCodec *codec = QTextCodec::codecForName("euc-jp");
    std::string black = ipx.player(index, osl::BLACK)
      + ipx.title(index, osl::BLACK);
    std::string white = ipx.player(index, osl::WHITE)
      + ipx.title(index, osl::WHITE);
    playerName[playerToIndex(osl::BLACK)] = codec->toUnicode(black.c_str(), black.length());
    playerName[playerToIndex(osl::WHITE)] = codec->toUnicode(white.c_str(), white.length());
    boost::gregorian::date date = ipx.startDate(index);
    if (! date.is_special())
      year = date.year();
  }
  return true;
}

KifuFile *KisenFile::nextFile() const
{
  try
  {
    if (filename.endsWith(".kpf"))
    {
      osl::record::KisenPlusFile kisenPlusFile(filename.toStdString());
      if (index < (int)kisenPlusFile.size() - 1)
      {
	return new KisenFile(filename, index + 1);
      }
    }
    else
    {
      osl::record::KisenFile kisenFile(filename.toStdString());
      if (index < (int)kisenFile.size() - 1)
      {
	return new KisenFile(filename, index + 1);
      }
    }
  }
  catch (osl::CsaIOError&)
  {
  }
  return NULL;
}

KifuFile *KisenFile::prevFile() const
{
  try
  {
    if (filename.endsWith(".kpf"))
    {
      osl::record::KisenPlusFile kisenPlusFile(filename.toStdString());
      if (index > 0)
      {
	return new KisenFile(filename, index - 1);
      }
    }
    else
    {
      osl::record::KisenFile kisenFile(filename.toStdString());
      if (index > 0)
      {
	return new KisenFile(filename, index - 1);
      }
    }
  }
  catch (osl::CsaIOError&)
  {
  }
  return NULL;
}


UsiFile::UsiFile(QString f)
  : KifuFile(f), loaded(false)
{
}

bool UsiFile::loadData()
{
  if (!loaded)
  {
    std::unique_ptr<osl::UsiFile> file;
    try
    {
      file.reset(new osl::UsiFile(filename.toStdString()));
    }
    catch (osl::usi::ParseError&)
    {
      return false;
    }
    state = file->initialState();
    getRecordData(file->load());
    loaded = true;
  }
  return true;
}

KifuFile *UsiFile::nextFile() const
{
  QFileInfo fileInfo(filename);
  QDir dir = fileInfo.absolutePath();

  QString name = fileInfo.fileName();
  QStringList files = dir.entryList(QStringList() << "*.usi");

  for (size_t i = 0; i < (size_t)files.size(); i++)
  {
    if (files[i] == name && i + 1 < (size_t)files.size())
    {
      return new UsiFile(dir.absolutePath() + "/" + files[i+1]);
    }
  }
  return NULL;
}

KifuFile *UsiFile::prevFile() const
{
  QFileInfo fileInfo(filename);
  QDir dir = fileInfo.absolutePath();

  QString name = fileInfo.fileName();
  QStringList files = dir.entryList(QStringList() << "*.usi");

  for (size_t i = 0; i < (size_t)files.size(); i++)
  {
    if (files[i] == name && i > 0)
    {
      return new UsiFile(dir.absolutePath() + "/" + files[i-1]);
    }
  }
  return NULL;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
