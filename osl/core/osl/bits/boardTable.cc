/* directionTable.cc
 */
#include "osl/bits/boardTable.h"
#include "osl/bits/directionTraits.h"

template <osl::Direction Dir>
void osl::BoardTable::setDirections(){
  const int blackDx=DirectionTraits<Dir>::blackDx;
  const int blackDy=DirectionTraits<Dir>::blackDy;
  Offset offset=Offset(blackDx,blackDy);
#ifndef MINIMAL
  space_counts[Offset32Wide(0,0).index()]=0;
#endif
  for(int i=1;i<=8;i++){
    int dx=i*blackDx;
    int dy=i*blackDy;
    Offset32 offset32(dx,dy);
    directions[offset32.index()]=Dir;
    short_offsets[offset32.index()]=offset;
    short_offsets_not_knight[offset32.index()]=offset;
    short8Dir[Offset(dx,dy).intValue()-Offset::ONBOARD_OFFSET_MIN]=
      longToShort(Dir);
    short8Offset[Offset(dx,dy).intValue()-Offset::ONBOARD_OFFSET_MIN]=
      offset.intValue();
  }
#ifndef MINIMAL
  for(int i=1;i<=10;i++){
    int dx=i*blackDx;
    int dy=i*blackDy;
    Offset32Wide offset32w(dx,dy);
    space_counts[offset32w.index()]=i-1;
  }
#endif
}
template <osl::Direction Dir>
void osl::BoardTable::setKnightDirections(){
  int dx=DirectionTraits<Dir>::blackDx;
  int dy=DirectionTraits<Dir>::blackDy;
  Offset32 offset32=Offset32(dx,dy);
  Offset offset=Offset(dx,dy);
  short_offsets[offset32.index()]=offset;
  short_offsets[(-offset32).index()]= -offset;
}

void osl::BoardTable::init(){
  short8Dir.fill(DIRECTION_INVALID_VALUE);
  short8Offset.fill();
  directions.fill();
  short_offsets_not_knight.fill();
#ifndef MINIMAL
  space_counts.fill(-1);
#endif
  setDirections<LONG_UL>();
  setDirections<LONG_U>();
  setDirections<LONG_UR>();
  setDirections<LONG_L>();
  setDirections<LONG_R>();
  setDirections<LONG_DL>();
  setDirections<LONG_D>();
  setDirections<LONG_DR>();
  setKnightDirections<UUL>();
  setKnightDirections<UUR>();
}

osl::BoardTable::BoardTable(){
  init();
  assert(! getOffset(BLACK, UL).zero());
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
