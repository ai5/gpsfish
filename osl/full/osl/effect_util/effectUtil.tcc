#ifndef _EFFECTUTIL_TCC
#define _EFFECTUTIL_TCC

#include "osl/effect_util/effectUtil.h"

template <class EvalT>
struct osl::effect_util::EffectUtil::FindThreat
{
  const NumEffectState& state;
  Player target;
  int attacker_value;
  PieceVector& supported, & unsupported;
  FindThreat(const NumEffectState& st, Player t, int a,
	     PieceVector& s, PieceVector& u)
    : state(st), target(t), attacker_value(a), supported(s), unsupported(u)
  {
  }
  void operator()(Square pos)
  {
    const Piece cur = state.pieceOnBoard(pos);
    assert(cur.isPiece());
    if (cur.owner() != target)
      return;
    if (state.hasEffectAt(target, pos))
    {
      if (abs(EvalT::captureValue(cur.ptypeO()))
	  > attacker_value)
	supported.push_back(cur);
    }
    else
    {
      unsupported.push_back(cur);
    }
  }
};

template <class EvalT>
void osl::EffectUtil::
findThreat(const NumEffectState& state, Square position,
	   PtypeO ptypeo, PieceVector& out)
{
  PieceVector supported, unsupported;
  const int attacker_value = abs(EvalT::captureValue(ptypeo));
  FindThreat<EvalT> f(state, alt(getOwner(ptypeo)), attacker_value, 
		      supported, unsupported);
  state.forEachEffectOfPtypeO<FindThreat<EvalT>, false>
    (position, ptypeo, f);

  unsupported.sortByPtype();
  supported.sortByPtype();
  PieceVector::iterator u=unsupported.begin(), s=supported.begin();

  if (u!=unsupported.end())
  {
    while ((s!=supported.end()) 
	   && ((abs(EvalT::captureValue(s->ptypeO()))
		- attacker_value)
	       > abs(EvalT::captureValue(u->ptypeO()))))
    {
      out.push_back(*s);
      ++s;
    }
  }
  out.push_back(u, unsupported.end());
  out.push_back(s, supported.end());
}

#endif /* _EFFECTUTIL_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
