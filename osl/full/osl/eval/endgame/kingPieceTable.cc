/**
 * kingPieceTable.cc
 */
#include "osl/eval/endgame/kingPieceTable.h"
#include "osl/container/pieceValues.h"
#include <cstdio>
#include <iostream>
#if defined(_WIN32)
#   include <stdlib.h>
#endif

void osl::eval::endgame::
KingPieceTable::saveText(const char *filename) const
{
  FILE *fp = fopen(filename, "w");
  if (! fp)
    return;
  for (int x=1; x<=9; ++x) {
    for (int y=1; y<=9; ++y) {
      Square sq(x,y);
      for (int i=0; i<2; ++i) {
	for (int x2=0; x2<=9; ++x2) {
	  for (int y2=(x2 == 0) ? 0 : 1; y2<=9; ++y2) {
	    Square sq2(x2,y2);
	    if (x2 == 0 && y2 == 0)
	      sq2 = Square::STAND();
	    for (int j=0; j<PTYPE_SIZE; ++j)
	      fprintf(fp, "%d\n", data[sq.index()*2+i][sq2.index()*PTYPE_SIZE+j]);
	    if (sq2.isPieceStand())
	      break;
	  }
	}
      }
    }
  }
  fclose(fp);
}

void osl::eval::endgame::
KingPieceTable::loadText(const char *filename)
{
  CArray<int, EffectiveDimension> w;
  FILE *fp = fopen(filename, "r");
  if (! fp) {
    std::cerr << "open failed " << filename << "\n";
    return;
  }
  for (int i=0; i<EffectiveDimension; ++i) {
    if (fscanf(fp, "%d", &w[i]) != 1) {
      std::cerr << "read failed " << i << "\n";
    }
  }
  fclose(fp);
  resetWeights(&w[0]);
}

void osl::eval::endgame::
KingPieceTable::resetWeights(const int *w)
{
#ifndef NDEBUG
  const int *src = w;
#endif
  for (int x=1; x<=9; ++x) {
    for (int y=1; y<=9; ++y) {
      Square sq(x,y);
      for (int i=0; i<2; ++i) {
	for (int x2=0; x2<=9; ++x2) {
	  for (int y2=(x2 == 0) ? 0 : 1; y2<=9; ++y2) {
	    Square sq2(x2,y2);
	    if (x2 == 0 && y2 == 0)
	      sq2 = Square::STAND();
	    for (int j=0; j<PTYPE_SIZE; ++j) {
	      assert(effectiveIndexOf(sq, indexToPlayer(i), sq2, (Ptype)j)
		     == w-src);
	      data[sq.index()*2+i][sq2.index()*PTYPE_SIZE+j] = *w++;
	    }
	    if (sq2.isPieceStand())
	      break;
	  }
	}
      }
    }
  }
  assert(w == src+dimension());
}

void osl::eval::endgame::
KingPieceTable::randomize()
{
  for (int x=1; x<=9; ++x) {
    for (int y=1; y<=9; ++y) {
      Square sq(x,y);
      for (int i=0; i<2; ++i) {
	for (int x2=0; x2<=9; ++x2) {
	  for (int y2=(x2 == 0) ? 0 : 1; y2<=9; ++y2) {
	    Square sq2(x2,y2);
	    if (x2 == 0 && y2 == 0)
	      sq2 = Square::STAND();
	    for (int j=0; j<PTYPE_SIZE; ++j) {
	      data[sq.index()*2+i][sq2.index()*PTYPE_SIZE+j]
		= (
#ifndef _WIN32
                   random()
#else
                   rand()
#endif
                   %1024)-512;
	    }
	    if (sq2.isPieceStand())
	      break;
	  }
	}
      }
    }
  }
}

void osl::eval::endgame::
KingPieceTable::clear()
{
  data.fill(0);
}

bool osl::eval::endgame::
operator==(const KingPieceTable& l, KingPieceTable& r)
{
  return l.data == r.data;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
