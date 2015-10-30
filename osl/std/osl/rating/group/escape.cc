/* escape.cc
 */
#include "osl/rating/group/escape.h"

osl::rating::
FromEffectGroup::FromEffectGroup() : Group("FromEffect")
{
  for (int a=0; a<3; ++a)
    for (int d=0; d<3; ++d)
      for (int p=0; p<8; ++p)	// progress8
	push_back(new FromEffect(a, d));
}

osl::rating::
PtypeAttackedGroup::PtypeAttackedGroup() : Group("PtypeAttacked")
{
  for (int s=PTYPE_PIECE_MIN; s<=PTYPE_MAX; ++s) {
    for (int a=PTYPE_MIN; a<=PTYPE_MAX; ++a) {
      for (int p=0; p<8; ++p)	// progress8
	push_back(new PtypeAttacked(static_cast<Ptype>(s), static_cast<Ptype>(a)));
    }
  }
}

osl::rating::
ImmediateEscapeGroup::ImmediateEscapeGroup() : Group("ImmediateEscape")
{
  for (int s=PTYPE_PIECE_MIN; s<=PTYPE_MAX; ++s) {
    for (int a=PTYPE_PIECE_MIN; a<=PTYPE_MAX; ++a) {
      for (int p=0; p<8; ++p)	// progress8
	push_back(new ImmediateEscape(static_cast<Ptype>(s), static_cast<Ptype>(a)));
    }
  }
}

osl::rating::
KingEscapeGroup::KingEscapeGroup() : Group("KingEscape")
{
  for (int s=PTYPE_PIECE_MIN; s<=PTYPE_MAX; ++s)
    push_back(new KingEscape(static_cast<Ptype>(s)));
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
