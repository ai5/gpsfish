#include "osl/basic_type.h"
// #include "osl/liberty8Table.h"
#include "osl/bits/pieceTable.h"
#include "osl/bits/boardTable.h"
#include "osl/bits/ptypeTable.h"
// #include "osl/hashKey.h"
#if 0
#include "osl/move_generator/addEffect8Table.h"
#include "osl/progress/ptypeProgress.h"
#include "osl/progress/effect5x3Table.h"
#include "osl/pathEncoding.h"
#include "osl/effect/moveSignature.h"

#include "osl/centering3x3.h"
#endif

#include "osl/bits/centering5x3.h"
#include "osl/checkmate/immediateCheckmateTable.h"

#if 0
#include "osl/effect_util/neighboring8Effect.h"
#include "osl/effect_util/sendOffSquare.h"

#endif
#include "osl/eval/openMidEndingEval.h"
#include "osl/bits/boardMask.h"

namespace osl
{
  const CArray<Offset,DIRECTION_SIZE> BoardTable::offsets = { {
    DirectionTraits<static_cast<Direction>(0)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(1)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(2)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(3)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(4)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(5)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(6)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(7)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(8)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(9)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(10)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(11)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(12)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(13)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(14)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(15)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(16)>::blackOffset(),
    DirectionTraits<static_cast<Direction>(17)>::blackOffset()
  } };
  const CArray<int,DIRECTION_SIZE> BoardTable::dxs = { {
    DirectionTraits<static_cast<Direction>(0)>::blackDx,
    DirectionTraits<static_cast<Direction>(1)>::blackDx,
    DirectionTraits<static_cast<Direction>(2)>::blackDx,
    DirectionTraits<static_cast<Direction>(3)>::blackDx,
    DirectionTraits<static_cast<Direction>(4)>::blackDx,
    DirectionTraits<static_cast<Direction>(5)>::blackDx,
    DirectionTraits<static_cast<Direction>(6)>::blackDx,
    DirectionTraits<static_cast<Direction>(7)>::blackDx,
    DirectionTraits<static_cast<Direction>(8)>::blackDx,
    DirectionTraits<static_cast<Direction>(9)>::blackDx,
    DirectionTraits<static_cast<Direction>(10)>::blackDx,
    DirectionTraits<static_cast<Direction>(11)>::blackDx,
    DirectionTraits<static_cast<Direction>(12)>::blackDx,
    DirectionTraits<static_cast<Direction>(13)>::blackDx,
    DirectionTraits<static_cast<Direction>(14)>::blackDx,
    DirectionTraits<static_cast<Direction>(15)>::blackDx,
    DirectionTraits<static_cast<Direction>(16)>::blackDx,
    DirectionTraits<static_cast<Direction>(17)>::blackDx
  } };
  const CArray<int,DIRECTION_SIZE> BoardTable::dys = { {
    DirectionTraits<static_cast<Direction>(0)>::blackDy,
    DirectionTraits<static_cast<Direction>(1)>::blackDy,
    DirectionTraits<static_cast<Direction>(2)>::blackDy,
    DirectionTraits<static_cast<Direction>(3)>::blackDy,
    DirectionTraits<static_cast<Direction>(4)>::blackDy,
    DirectionTraits<static_cast<Direction>(5)>::blackDy,
    DirectionTraits<static_cast<Direction>(6)>::blackDy,
    DirectionTraits<static_cast<Direction>(7)>::blackDy,
    DirectionTraits<static_cast<Direction>(8)>::blackDy,
    DirectionTraits<static_cast<Direction>(9)>::blackDy,
    DirectionTraits<static_cast<Direction>(10)>::blackDy,
    DirectionTraits<static_cast<Direction>(11)>::blackDy,
    DirectionTraits<static_cast<Direction>(12)>::blackDy,
    DirectionTraits<static_cast<Direction>(13)>::blackDy,
    DirectionTraits<static_cast<Direction>(14)>::blackDy,
    DirectionTraits<static_cast<Direction>(15)>::blackDy,
    DirectionTraits<static_cast<Direction>(16)>::blackDy,
    DirectionTraits<static_cast<Direction>(17)>::blackDy
  } };
}

namespace osl
{
  const PieceTable Piece_Table;
  const BoardTable Board_Table;
  // PtypeTable depends on BoardTable
  const PtypeTable Ptype_Table;
#if 0
  // BoardTable, PtypeTable -> Liberty8Table
  const effect::Liberty8Table effect::Liberty8_Table;
#endif
#ifndef DFPNSTATONE
  const eval::PtypeEvalTable eval::Ptype_Eval_Table;
  eval::ml::OpenMidEndingPtypeTable eval::ml::OpenMidEndingEval::Piece_Value;
#endif

#if 0
#ifndef MINIMAL
  const effect::MoveSignatureTable effect::Move_Signature_Table;
#endif
  const PathEncodingTable Path_Encoding_Table;

#endif
  const Centering5x3::Table Centering5x3::table;

#if 0
  const effect_util::Neighboring8Effect::Table Neighboring8Effect::table;
#endif
  const container::BoardMaskTable5x5 container::Board_Mask_Table5x5;
  const container::BoardMaskTable3x3 container::Board_Mask_Table3x3;
  const container::BoardMaskTable5x3Center container::Board_Mask_Table5x3_Center;
  const checkmate::ImmediateCheckmateTable checkmate::Immediate_Checkmate_Table;
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
