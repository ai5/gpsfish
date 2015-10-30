/* piecePair.h
 */
#ifndef _PIECEPAIR_H
#define _PIECEPAIR_H

#include "eval/eval.h"
#include "eval/pieceFeature.h"
#include "osl/eval/piecePair.h"
namespace gpsshogi
{
  using namespace osl;

  typedef osl::eval::ml::PiecePair PiecePairFeature;
  class PiecePair : public PieceFeatureIncremental<PiecePairFeature>
  {
  public:
    PiecePair();
    ~PiecePair();
    void featuresNonUniq(const NumEffectState&, 
			 index_list_t&, int) const;
    size_t maxActive() const;

    void showSummary(std::ostream&) const;
    void showAll(std::ostream& os) const;
    void setRandom();
    void setWeightScale(const double*, double);
    const std::string name() const { return "PiecePair"; };
    const std::string describe(size_t local_index) const;
  };
}

#endif /* _PIECEPAIR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
