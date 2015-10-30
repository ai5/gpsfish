#ifndef _OPENING_BOOK_CONVERTER_H
#define _OPENING_BOOK_CONVERTER_H

#include "osl/book/openingBook.h"

class OBState
{
  int OBMoveIndex;
  int nOBMove;
  int blackWinCount;
  int whiteWinCount;

 public:
  OBState(int startIndex, int nMove, int blackWin, int whiteWin) :
    OBMoveIndex(startIndex), nOBMove(nMove),
    blackWinCount(blackWin), whiteWinCount(whiteWin) {}
    int getOBMoveIndex() const { return OBMoveIndex; }
    int getNOBMove() const { return nOBMove; }
    int getBlackWinCount() const { return blackWinCount; }
    int getWhiteWinCount() const { return whiteWinCount; }
};

class OpeningBookConverter
{
  std::vector<OBState> states;
  std::vector<osl::book::OBMove> moves;
 public:
  OpeningBookConverter(const char* filename);
  ~OpeningBookConverter() {};
  void write(const char* filename);
  void writeInNewFormat(const char* filename);
  void writeInNewEditFormat(const char* filename);
 private:
  int readInt(std::ifstream& ifs);
  void writeInt(std::ofstream& ofs, int n);
  void writeInNewFormat(std::ofstream& ofs);
};

#endif // _OPENING_BOOK_CONVERTER_H
