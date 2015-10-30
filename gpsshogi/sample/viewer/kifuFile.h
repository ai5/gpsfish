#ifndef GPSSHOGI_SAMPLE_VIEWER_KIFUFILE_H
#define GPSSHOGI_SAMPLE_VIEWER_KIFUFILE_H
#include <qstring.h>
#include "osl/numEffectState.h"
#include "osl/record/searchInfo.h"
#include "osl/record/record.h"

class KifuFile
{
public:
  virtual KifuFile *nextFile() const = 0;
  virtual KifuFile *prevFile() const = 0;
  virtual ~KifuFile() {}
  const osl::SimpleState &getInitialState() {
    loadData();
    return state;
  }
  const std::vector<osl::Move> &getMoves() {
    loadData();
    return moves;
  }
  const std::vector<int> &getTime() {
    loadData();
    return time;
  }
  QString getPlayerName(osl::Player player) {
    loadData();
    return playerName[playerToIndex(player)];
  }
  const std::vector<QString> &getComments() {
    loadData();
    return comments;
  }
  const std::vector<osl::record::SearchInfo> &getSearchInfo() {
    loadData();
    return searchInfo;
  }
  const std::vector<QString>& getInitialComment() const 
  {  
    return initial_comment; 
  }
  virtual bool loadData() = 0;
  virtual QString getFilename() { return filename; }
  virtual int getYear() const { return year; }
  virtual bool reloadIfChanged() { return false; }
  void getRecordData(const osl::Record& record);
protected:
  KifuFile(QString f) : filename(f), year(0) {}

  QString filename;
  osl::CArray<QString,2> playerName;
  osl::SimpleState state;
  std::vector<osl::Move> moves;
  std::vector<int> time;
  std::vector<QString> comments;
  std::vector<osl::record::SearchInfo> searchInfo;
  std::vector<QString> initial_comment;
  int year;
};

class CsaFile : public KifuFile
{
public:
  CsaFile(QString filename);
  virtual ~CsaFile() {}
  KifuFile *nextFile() const;
  KifuFile *prevFile() const;
  bool loadData();
  bool reloadIfChanged();
private:
  bool loaded;
  uint lastLoadedTime;
};

class KakinokiFile : public KifuFile
{
public:
  KakinokiFile(QString filename);
  virtual ~KakinokiFile() {}
  KifuFile *nextFile() const;
  KifuFile *prevFile() const;
  bool loadData();
private:
  bool loaded;
};

class Ki2File : public KifuFile
{
public:
  Ki2File(QString filename);
  virtual ~Ki2File() {}
  KifuFile *nextFile() const;
  KifuFile *prevFile() const;
  bool loadData();
private:
  bool loaded;
};

class KisenFile : public KifuFile
{
public:
  KisenFile(QString filename, int index);
  virtual ~KisenFile() {}
  KifuFile *nextFile() const;
  KifuFile *prevFile() const;
  bool loadData();
private:
  int index;
  bool loaded;
};

class UsiFile : public KifuFile
{
public:
  UsiFile(QString filename);
  virtual ~UsiFile() {}
  KifuFile *nextFile() const;
  KifuFile *prevFile() const;
  bool loadData();
private:
  bool loaded;
};

#endif // GPSSHOGI_SAMPLE_VIEWER_KIFUFILE_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
