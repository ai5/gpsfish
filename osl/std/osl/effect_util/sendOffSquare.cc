/* sendOffSquare.cc
 */
#include "osl/effect_util/sendOffSquare.h"
#include "osl/bits/ptypeTable.h"
#include "osl/oslConfig.h"

osl::effect_util::SendOffSquare::Table osl::effect_util::SendOffSquare::table;

static osl::SetUpRegister _initializer([](){ 
    osl::effect_util::SendOffSquare::init();
});

void osl::effect_util::SendOffSquare::
Table::init()
{
  normal[0] = Offset( 1, 1);
  normal[1] = Offset( 1, 0);
  normal[2] = Offset( 1,-1);
  normal[3] = Offset( 0, 1);
  normal[4] = Offset( 0,-1);
  normal[5] = Offset(-1, 1);
  normal[6] = Offset(-1, 0);
  normal[7] = Offset(-1,-1);

  const Square center(5,5);
  const PtypeO king = newPtypeO(BLACK, KING);
  for (int i=0; i<8; ++i)
  {
    const Offset king_square = normal[i];
    for (int j=0; j<8; ++j)
    {
      const Offset target = normal[j];
      if (i==j)
	continue;
      const int dx = king_square.dx() - target.dx();
      const int dy = king_square.dy() - target.dy();
      const EffectContent effect
	= Ptype_Table.getEffect(king, Offset32(dx, dy));
      if (! effect.hasEffect())
      {
	reverse[i].push_back(j);
      }
    }
  }

  for (int i=0; i<256; ++i)
  {
    unsigned int val = i;
    while (val)
    {
      const int j = misc::BitOp::takeOneBit(val);

      for (int p: reverse[j])
      {
	if (! reverse_all[i].isMember(p))
	  reverse_all[i].push_back(p);
      }
    }
  }
}

template <osl::Player Attack>
osl::effect_util::SendOffSquare::SendOff8
#if (defined __GNUC__) && (! defined GPSONE) && (! defined GPSUSIONE)
__attribute__ ((used))
#endif
  osl::effect_util::
SendOffSquare::find(const NumEffectState& state, Square king_square,
		      Square8& out)
{
  assert(out.empty());
  int flags=0;
  for (int i=0; i<8; ++i)
  {
    testSquare<Attack>(state, king_square+table.normal[i], i, flags);
  }
  SendOff8 data = 0;
  for (int i: table.reverse_all[flags])
  {
    const Square candidate = king_square + table.normal[i];
    if (! state.pieceAt(candidate).isEdge()
	&& state.countEffect(alt(Attack), candidate) == 1) {
      out.push_back(candidate);
      data |= (1<<i);
    }
  }
  return data;
}

void osl::effect_util::
SendOffSquare::unpack(SendOff8 flags8, Square king_square,
			Square8& out)
{
  assert(out.empty());
  unsigned int flags = flags8;
  while (flags) {
    const int i = misc::BitOp::takeOneBit(flags);
    const Square candidate = king_square + table.normal[i];
    out.push_back(candidate);
  }
}

osl::effect_util::SendOffSquare::SendOff8 osl::effect_util::
SendOffSquare::find(Player attack, const NumEffectState& state, 
		      Square king_square,
		      Square8& out)
{
  if (attack == BLACK)
    return find<BLACK>(state, king_square, out);
  else
    return find<WHITE>(state, king_square, out);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
