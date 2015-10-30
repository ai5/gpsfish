/* piecePair.cc
 */
#include "eval/piecePair.h"
#include "eval/indexCache.h"
#include "osl/bits/centering5x3.h"
#include "osl/stat/average.h"
#include "osl/csa.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>


gpsshogi::
PiecePair::PiecePair()
{
  PiecePairFeature::init();
}

gpsshogi::
PiecePair::~PiecePair()
{
}

void gpsshogi::
PiecePair::featuresNonUniq(const NumEffectState& state, 
			   index_list_t& out, int offset) const
{
  for (int i=0; i<Piece::SIZE; i++) {
    const Piece p = state.pieceOf(i);
    if (! p.isOnBoard())
      continue;
    for (size_t i=0; i<PiecePairFeature::offsets.size(); ++i) {
      const Square target = p.square() + offsets[i];
      const Piece q = state.pieceAt(target);
      if (! q.isPiece() || q.number() <= p.number()) // 反対向きの重複排除
	continue;
      assert(!target.isPieceStand() && p.isOnBoard() && q.isOnBoard());
      CArray<int,3> index = PiecePairFeature::index(i, q, p);
      for (size_t i=0; i<index.size(); ++i) {
	if (index[i] == 0)
	  continue;
	out.add(abs(index[i]) + offset, index[i] > 0 ? 1 : -1);	
      }
    }
  }
}

void gpsshogi::
PiecePair::showSummary(std::ostream& os) const
{
  osl::stat::Average a, a_abs;
  for (size_t i=0; i<values.size(); ++i) {
    if (! values[i]) continue;
    a.add(values[i]);
    a_abs.add(abs(values[i]));
  }
#ifndef __APPLE__
  int avsum = std::abs(values).sum();
#else
  int avsum = 0;
#endif
  os << "PiecePair min. " << values.min() 
     << " abs(ave.) " << avsum/(double)dimension()
     << " ave. " << values.sum()/(double)dimension() << " max. " << values.max() << "\n";
  os << "  ave. (nonzero) " << a.average() << " abs(ave., nonzero) " << a_abs.average()
     << " #nonzero " << a.numElements() << "\n";
}

void gpsshogi::
PiecePair::showAll(std::ostream& os) const
{
  showSummary(os);
}

size_t gpsshogi::
PiecePair::maxActive() const
{
  return 40*10*3;
}

void gpsshogi::
PiecePair::setRandom()
{
  EvalComponent::setRandom();
  PiecePairFeature::sanitize(*this);
}

void gpsshogi::
PiecePair::setWeightScale(const double *w, double scale)
{
  EvalComponent::setWeightScale(w, scale);
  PiecePairFeature::sanitize(*this);
}

std::string csaOwnerOrEmpty(osl::PtypeO ptypeo) 
{
  if (! osl::isPiece(ptypeo))
    return "?";
  return osl::csa::show(osl::getOwner(ptypeo));
}

const std::string gpsshogi::
PiecePair::describe(size_t local_index) const
{
  if (local_index < PiecePairFeature::plain_table_size) {
    static CArray<std::string, PiecePairFeature::plain_table_size>
      dictionary;
    static bool initialized = false;
    if (! initialized) {
      initialized = true;
      for (int i=0; i<12; ++i) {
	for (int j=PTYPE_MIN; j<=PTYPE_MAX; ++j) {
	  const PtypeO p0 = newPtypeO(BLACK,static_cast<Ptype>(j));
	  for (int k=PTYPEO_MIN; k<=PTYPEO_MAX; ++k) {
	    const PtypeO p1 = static_cast<PtypeO>(k);
	    const Offset o = PiecePairFeature::offsets[i];
	    const int index = PiecePairFeature::plain_table[i][ptypeOIndex(p0)][ptypeOIndex(p1)];
	    if (index < 0)
	      continue;
	    dictionary[index] = '(' + std::to_string(o.dx())
	      + ',' + std::to_string(o.dy())
	      + ") " + csaOwnerOrEmpty(p0) + csa::show(getPtype(p0))
	      +':' + csaOwnerOrEmpty(p1) + csa::show(getPtype(p1));
	  }
	}
      }
    }
    return "plain " + dictionary[local_index];
  }
  local_index -= PiecePairFeature::plain_table_size;
  if (local_index < PiecePairFeature::x_table_size)
    return "x_table";
  local_index -= PiecePairFeature::x_table_size;
  if (local_index < PiecePairFeature::y_table_size)
    return "y_table";
  return "PiecePair error";
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
