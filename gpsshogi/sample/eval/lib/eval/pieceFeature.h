/* pieceFeature.h
 */
#ifndef _PIECEFEATURE_H
#define _PIECEFEATURE_H

#include "eval/eval.h"
namespace gpsshogi
{
  template <class Feature>
  class PieceFeature : public EvalComponent, protected Feature
  {
  public:
    enum { DIM = Feature::DIM };
    PieceFeature();
    ~PieceFeature();
    
    int eval(const NumEffectState&) const;
    int pieceValue(const NumEffectState& state, Piece p) const;
  };

  template <class Feature>
  class PieceFeatureIncremental : public PieceFeature<Feature>
  {
  public:
    int evalWithUpdate(const NumEffectState& state, Move moved, int last_value) const;
  };
}

template <class Feature>
gpsshogi::
PieceFeature<Feature>::PieceFeature()
  : EvalComponent(DIM)
{
}

template <class Feature>
gpsshogi::
PieceFeature<Feature>::~PieceFeature()
{
}

template <class Feature>
int gpsshogi::
PieceFeature<Feature>::pieceValue(const NumEffectState& state, Piece p) const
{
  return Feature::pieceValue(state, p, *this);
}

template <class Feature>
int gpsshogi::
PieceFeature<Feature>::eval(const NumEffectState& state) const
{
  return Feature::eval(state, *this);
}

template <class Feature>
int gpsshogi::
PieceFeatureIncremental<Feature>::evalWithUpdate(const NumEffectState& state, Move moved, int last_value) const
{
  return Feature::evalWithUpdate(state, moved, last_value, *this);
}


#endif /* _PIECEFEATURE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
