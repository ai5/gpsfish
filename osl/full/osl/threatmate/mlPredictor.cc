/* mlPredictor.cc
 */
#include "osl/threatmate/mlPredictor.h"
#include "osl/progress.h"
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/bits/king8Info.h"
#include <cmath>

const double predictor_coef[5][9] = 
  {
    // all
    {-3.7460724,  1.5899403,  1.5218839,
      1.8912265,  2.2698081,  2.9564733,
      1.8364091,  0.4818582,  0.2046128}, 
    // ~ 100 nodes
    { -4.502173,   1.866922,   1.075071,
       1.785152,   2.904015,   3.546099,
       2.217401,   1.037181,   0.198456},
    // ~ 1,000 nodes
    {-4.2651299,  1.8346520,  1.4515673,
      1.8766168,  2.8202449,  3.2856799,
      2.0692834,  0.7330482,  0.2069689},
    // 1,000 ~ 10,000 nodes
    {-3.8450811,  1.5901365,  1.6039122,
      1.9947154,  2.2830698,  2.9788201,
      1.8998258,  0.4654985,  0.2174952},
    // 10,000  ~
    {-3.3149250,  1.3827221,  1.4877458,
      1.8048604,  1.6804844,  2.7207930,
      1.6221641,  0.3460561,  0.1866114,}
  };

double osl::threatmate::MlPredictor::predict(const NumEffectState& state, 
					     const Move move, size_t index){
  const Player turn = alt(state.turn());
  const Square oking = state.kingSquare(alt(turn));
  King8Info K(state.Iking8Info(alt(turn)));
  NewProgress tprogress(state);

  const double* coef = predictor_coef[index];
  double sum = coef[0];

  const int npiece = (int)(move.capturePtype());
  if (npiece) {
    for (int i=0; i<3; i++) 
      if (npiece%8 == i+5)
	sum += coef[i+1];
    if (npiece == 9)
      sum += coef[4];
  }

  int moveCandidate;
  if(turn==BLACK) moveCandidate=K.countMoveCandidate<BLACK>(state);
  else moveCandidate=K.countMoveCandidate<WHITE>(state);
  sum += coef[5] * ( Neighboring8Direct::hasEffect(state, newPtypeO(turn, move.ptype()), 
						   move.to(), oking) ) + 
    coef[6] * moveCandidate + 
    coef[7] * ( misc::BitOp::countBit(K.dropCandidate()) )+
    coef[8] * ( tprogress.progressAttack(alt(turn)).value() );
  
  return sum;
}
double osl::threatmate::MlPredictor::probability(const NumEffectState& state, 
					     const Move move, size_t index){
  return 1.0 / (1.0 + exp (- predict(state,move,index) ));
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
