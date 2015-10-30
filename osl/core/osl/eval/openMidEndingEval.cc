#include "osl/eval/openMidEndingEval.h"
#include "osl/eval/piecePair.h"
#include "osl/eval/piecePairKing.h"
#include "osl/eval/kingTable.h"
#include "osl/eval/majorPiece.h"
#include "osl/eval/minorPiece.h"
#include "osl/eval/mobility.h"
#include "osl/eval/pieceStand.h"
#include "osl/eval/pin.h"
#include "osl/eval/king8.h"
#include "osl/eval/progress.h"
#include "osl/random.h"
#include "osl/bits/binaryIO.h"
#include "osl/bits/pieceStand.h"
#include "osl/oslConfig.h"

#include <fstream>

osl::eval::ml::OpenMidEndingPtypeTable::
OpenMidEndingPtypeTable()
{
  const CArray<int, PTYPE_SIZE> values = {{
      0, 0,
      583, 493, 491, 467, 1279, 1351,
      PtypeEvalTraits<KING>::val, 585,
      128, 318, 361, 540, 959, 1059
    }};
  reset(values);
}
#ifndef MINIMAL
const char * osl::eval::ml::
OpenMidEndingEvalDebugInfo::name(ProgressIndependentFeature f)
{
  static const CArray<const char *,PROGRESS_INDEPENDENT_FEATURE_LIMIT> table = {{
      "PIECE",
      "BISHOP_EXCHANGE_SILVER_KING",
      "ENTER_KING_DEFENSE",
      "KING25_EFFECT_ATTACK",
      "PIECE_PAIR",
      "PIECE_PAIR_KING",
    }};
  return table[f];
}
const char *osl::eval::ml::
OpenMidEndingEvalDebugInfo::name(StageFeature f)
{
  static const CArray<const char *,STAGE_FEATURE_LIMIT> table = {{
          "KING_PIECE_RELATIVE",
          "PIECE_STAND",
          "KING25_EFFECT_EACH",
          "PTYPEX",
          "PTYPEY",
          "ROOK_MOBILITY",
          "BISHOP_MOBILITY",
          "LANCE_MOBILITY",
          "ROOK_EFFECT",
          "BISHOP_EFFECT",
          "PIECE_STAND_COMBINATION",
          "PIECE_STAND_TURN",
          "ROOK_PAWN",
          "PAWN_DROP",
          "PIECE_STAND_Y",
          "KNIGHT_CHECK",
          "PAWN_ADVANCE",
          "PAWN_PTYPEO",
          "PROMOTED_MINOR_PIECE",
          "KING_PIECE_RELATIVE_NOSUPPORT",
          "NON_PAWN_ATTACKED",
          "NON_PAWN_ATTACKED_PTYPE",
          "PTYPE_YY",
          "KING3PIECES",
          "BISHOP_HEAD",
          "KNIGHT_HEAD",
          "ROOK_PROMOTE_DEFENSE",
          "PTYPE_COUNT",
          "LANCE_EFFECT_PIECE",
          "PTYPE_Y_PAWN_Y",
          "BISHOP_AND_KING",
          "PIECE_FORK_TURN",
          "ROOK_SILVER_KNIGHT",
          "BISHOP_SILVER_KNIGHT",
          "KING25_EFFECT_SUPPORTED",
          "KING_ROOK_BISHOP",
          "KING_X_BLOCKED3",
          "GOLD_RETREAT",
          "SILVER_RETREAT",
          "ALL_GOLD",
          "ALL_MAJOR",
          "KING25_EFFECT_DEFENSE",
          "ANAGUMA_EMPTY",
          "NO_PAWN_ON_STAND",
          "NON_PAWN_PIECE_STAND",
          "PIN_PTYPE_ALL",
          "KING_MOBILITY",
          "GOLD_AND_SILVER_NEAR_KING",
          "PTYPE_COMBINATION",
          "KING25_BOTH_SIDE",
          "KING25_MOBILITY",
          "BISHOP_STAND_FILE5",
          "MAJOR_CHECK_WITH_CAPTURE",
          "SILVER_ADVANCE26",
          "KING25_EFFECT3",
          "BISHOP_BISHOP_PIECE",
          "ROOK_ROOK",
          "ROOK_ROOK_PIECE",
          "KING25_EFFECT_COUNT_COMBINATION",
          "NON_PAWN_ATTACKED_PTYPE_PAIR",
          "ATTACK_MAJORS_IN_BASE",
    }};
  return table[f];
}
#endif

volatile osl::eval::ml::OpenMidEndingEval::LoadStatus
osl::eval::ml::OpenMidEndingEval::initialized_flag = osl::eval::ml::OpenMidEndingEval::Zero;
static std::mutex initialize_mutex;
osl::eval::ml::Weights
osl::eval::ml::OpenMidEndingEval::piece_pair_weights;
namespace
{
#ifndef MINIMAL
  template <class Eval>
  static void setRandomOne()
  {
    osl::eval::ml::Weights weights(Eval::DIM);
    for (size_t i = 0; i < weights.dimension(); ++i)
    {
      weights.setValue(i, (osl::misc::random() % 1024)-512);
    }
    Eval::setUp(weights);
  }
  template <class Eval>
  static void setRandomOne(int stage)
  {
    osl::eval::ml::Weights weights(Eval::DIM);
    for (size_t i = 0; i < weights.dimension(); ++i)
    {
      weights.setValue(i, (osl::misc::random() % 1024)-512);
    }
    Eval::setUp(weights, stage);
  }
#endif
  template <class Eval, class Reader>
  static int setUpOneWithDim(Reader& p, int dim)
  {
    osl::eval::ml::Weights weights(dim);
    // std::cerr << typeid(Eval).name() << " " << dim << "\n";
    for (size_t i = 0; i < weights.dimension(); ++i)
    {
      if (! p.hasNext())
	break;
      int val = p.read();
      weights.setValue(i, val);
    }
    Eval::setUp(weights);
    return weights.dimension();
  }
  template <class Eval, class Reader>
  static int setUpOne(Reader& p)
  {
    return setUpOneWithDim<Eval>(p, Eval::DIM);
  }
  template <class Eval, class Reader>
  static int setUpOne(Reader& p, int stage)
  {
    osl::eval::ml::Weights weights(Eval::DIM);
    // std::cerr << typeid(Eval).name() << " " << Eval::DIM << "\n";
    for (size_t i = 0; i < weights.dimension(); ++i)
    {
      if (!p.hasNext())
	break;
      int val = p.read();
      weights.setValue(i, val);
    }
    Eval::setUp(weights,stage);
    return weights.dimension();
  }
}

namespace osl
{
  struct IntArrayReader
  {
    size_t cur, length;
    const int *array;
    IntArrayReader(const int *a, size_t l) : cur(0), length(l), array(a)
    {
    }
    bool hasNext() const { return cur < length; }
    bool failed() const { return false; }
    int read() { return array[cur++]; }
  };
}

void osl::eval::ml::
OpenMidEndingEval::resetWeights(const int *w, size_t length)
{
  IntArrayReader reader(w, length);
  doResetWeights(reader);
}

bool osl::eval::ml::OpenMidEndingEval::setUp(const char *filename)
{
  std::lock_guard<std::mutex> lk(initialize_mutex);
  if (initialized_flag == Loaded)
    return true;
  typedef osl::misc::BinaryElementReader<int> reader_t;
  std::ifstream is(filename, std::ios_base::binary);
  reader_t reader(is);
  if (! reader.hasNext()) {
    initialized_flag = Zero;
    std::cerr << "file " << filename << std::endl;
    return false;
  }
  doResetWeights(reader);
  return initialized_flag == Loaded;
}

template <class Reader>
void osl::eval::ml::
OpenMidEndingEval::doResetWeights(Reader& reader)
{
  size_t read_count = 0;

  // flat
  CArray<int, PTYPE_SIZE> piece_values = {{0}};
  Weights weights(PTYPE_SIZE);
  for (int i = 0; i < PTYPE_SIZE; ++i)
  {
    if (! reader.hasNext())
      break;
    int val = reader.read();
    if (i == KING) {
      assert(val == 0);
      val = PtypeEvalTraits<KING>::val;
    }
    weights.setValue(i, val);
    piece_values[i] = val;
    ++read_count;
  }
  PieceEval::setUp(weights);
  Piece_Value.reset(piece_values);

  PiecePair::init();
  piece_pair_weights.resetDimension(PiecePair::DIM);
  for (size_t i = 0; i < piece_pair_weights.dimension(); ++i)
  {
    if (! reader.hasNext())
      break;
    int val = reader.read();
    piece_pair_weights.setValue(i, val);
    ++read_count;
  }
  PiecePair::sanitize(piece_pair_weights);
  PiecePair::compile(piece_pair_weights);

  read_count += setUpOne<King25EffectAttack>(reader);
  read_count += setUpOne<King25EffectYAttack>(reader);
  read_count += setUpOne<PiecePairKing>(reader);
  read_count += setUpOne<BishopExchangeSilverKing>(reader);
  read_count += setUpOne<EnterKingDefense>(reader);

  // opening
  read_count += setUpOne<PieceStand>(reader,0);
  read_count += setUpOne<King25EffectEachBothOpening>(reader);
  read_count += setUpOne<PawnDrop>(reader,0);
  read_count += setUpOne<NoPawnOnStand>(reader,0);
  read_count += setUpOne<GoldRetreat>(reader,0);
  read_count += setUpOne<SilverRetreat>(reader,0);
  read_count += setUpOne<KnightAdvance>(reader,0);
  read_count += setUpOne<AllMajor>(reader,0);
  read_count += setUpOne<KingXBlocked>(reader,0);
  read_count += setUpOne<KingXBlockedY>(reader,0);
  read_count += setUpOne<AllGold>(reader,0);
  read_count += setUpOne<PtypeX>(reader,0);
  read_count += setUpOne<PtypeY>(reader,0);
  read_count += setUpOne<AnagumaEmpty>(reader,0);
  read_count += setUpOne<NonPawnPieceStand>(reader,0);
  read_count += setUpOne<King25EffectDefense>(reader,0);
  read_count += setUpOne<King25EffectYDefense>(reader,0);
  read_count += setUpOne<RookMobility>(reader,0);
  read_count += setUpOne<BishopMobility>(reader,0);
  read_count += setUpOne<LanceMobility>(reader,0);
  read_count += setUpOne<RookEffect>(reader,0);
  read_count += setUpOne<BishopEffect>(reader,0);
  read_count += setUpOne<PawnAdvance>(reader,0);
  read_count += setUpOne<PawnDropY>(reader,0);
  read_count += setUpOne<KnightCheck>(reader,0);

  // midgame
  read_count += setUpOne<PieceStand>(reader,1);
  read_count += setUpOne<King25EffectEachBothMidgame>(reader);
  read_count += setUpOne<PawnDrop>(reader,1);
  read_count += setUpOne<NoPawnOnStand>(reader,1);
  read_count += setUpOne<GoldRetreat>(reader,1);
  read_count += setUpOne<SilverRetreat>(reader,1);
  read_count += setUpOne<KnightAdvance>(reader,1);
  read_count += setUpOne<AllMajor>(reader,1);
  read_count += setUpOne<KingXBlocked>(reader,1);
  read_count += setUpOne<KingXBlockedY>(reader,1);
  read_count += setUpOne<AllGold>(reader,1);
  read_count += setUpOne<PtypeX>(reader,1);
  read_count += setUpOne<PtypeY>(reader,1);
  read_count += setUpOne<AnagumaEmpty>(reader,1);
  read_count += setUpOne<NonPawnPieceStand>(reader,1);
  read_count += setUpOne<King25EffectDefense>(reader,1);
  read_count += setUpOne<King25EffectYDefense>(reader,1);
  read_count += setUpOne<RookMobility>(reader,1);
  read_count += setUpOne<BishopMobility>(reader,1);
  read_count += setUpOne<LanceMobility>(reader,1);
  read_count += setUpOne<RookEffect>(reader,1);
  read_count += setUpOne<BishopEffect>(reader,1);
  read_count += setUpOne<PawnAdvance>(reader,1);
  read_count += setUpOne<PawnDropY>(reader,1);
  read_count += setUpOne<KnightCheck>(reader,1);

#ifdef EVAL_QUAD
  // midgame2
  read_count += setUpOne<PieceStand>(reader,2);
  read_count += setUpOne<King25EffectEachBothMidgame2>(reader);
  read_count += setUpOne<PawnDrop>(reader,2);
  read_count += setUpOne<NoPawnOnStand>(reader,2);
  read_count += setUpOne<GoldRetreat>(reader,2);
  read_count += setUpOne<SilverRetreat>(reader,2);
  read_count += setUpOne<KnightAdvance>(reader,2);
  read_count += setUpOne<AllMajor>(reader,2);
  read_count += setUpOne<KingXBlocked>(reader,2);
  read_count += setUpOne<KingXBlockedY>(reader,2);
  read_count += setUpOne<AllGold>(reader,2);
  read_count += setUpOne<PtypeX>(reader,2);
  read_count += setUpOne<PtypeY>(reader,2);
  read_count += setUpOne<AnagumaEmpty>(reader,2);
  read_count += setUpOne<NonPawnPieceStand>(reader,2);
  read_count += setUpOne<King25EffectDefense>(reader,2);
  read_count += setUpOne<King25EffectYDefense>(reader,2);
  read_count += setUpOne<RookMobility>(reader,2);
  read_count += setUpOne<BishopMobility>(reader,2);
  read_count += setUpOne<LanceMobility>(reader,2);
  read_count += setUpOne<RookEffect>(reader,2);
  read_count += setUpOne<BishopEffect>(reader,2);
  read_count += setUpOne<PawnAdvance>(reader,2);
  read_count += setUpOne<PawnDropY>(reader,2);
  read_count += setUpOne<KnightCheck>(reader,2);
#endif

  // endgame
  read_count += setUpOne<PieceStand>(reader,EndgameIndex);
  read_count += setUpOne<King25EffectEachBothEnding>(reader);
  read_count += setUpOne<PawnDrop>(reader,EndgameIndex);
  read_count += setUpOne<NoPawnOnStand>(reader,EndgameIndex);
  read_count += setUpOne<GoldRetreat>(reader,EndgameIndex);
  read_count += setUpOne<SilverRetreat>(reader,EndgameIndex);
  read_count += setUpOne<KnightAdvance>(reader,EndgameIndex);
  read_count += setUpOne<AllMajor>(reader,EndgameIndex);
  read_count += setUpOne<KingXBlocked>(reader,EndgameIndex);
  read_count += setUpOne<KingXBlockedY>(reader,EndgameIndex);
  read_count += setUpOne<AllGold>(reader,EndgameIndex);
  read_count += setUpOne<PtypeX>(reader,EndgameIndex);
  read_count += setUpOne<PtypeY>(reader,EndgameIndex);
  read_count += setUpOne<AnagumaEmpty>(reader,EndgameIndex);
  read_count += setUpOne<NonPawnPieceStand>(reader,EndgameIndex);
  read_count += setUpOne<King25EffectDefense>(reader,EndgameIndex);
  read_count += setUpOne<King25EffectYDefense>(reader,EndgameIndex);
  read_count += setUpOne<RookMobility>(reader,EndgameIndex);
  read_count += setUpOne<BishopMobility>(reader,EndgameIndex);
  read_count += setUpOne<LanceMobility>(reader,EndgameIndex);
  read_count += setUpOne<RookEffect>(reader,EndgameIndex);
  read_count += setUpOne<BishopEffect>(reader,EndgameIndex);
  read_count += setUpOne<PawnAdvance>(reader,EndgameIndex);
  read_count += setUpOne<PawnDropY>(reader,EndgameIndex);
  read_count += setUpOne<KnightCheck>(reader,EndgameIndex);

  // triple
  read_count += setUpOne<KingPieceRelative>(reader,0);
  read_count += setUpOne<KingPieceRelative>(reader,1);
#ifdef EVAL_QUAD
  read_count += setUpOne<KingPieceRelative>(reader,2);
#endif
  read_count += setUpOne<KingPieceRelative>(reader,EndgameIndex);
  read_count += setUpOne<NonPawnPieceStandTurn>(reader);
  read_count += setUpOne<King25EffectEachXY>(reader);
  read_count += setUpOne<RookPawnY>(reader);
  read_count += setUpOne<RookEffectPiece>(reader);
  read_count += setUpOne<BishopEffectPiece>(reader);
  read_count += setUpOne<PieceStandY>(reader);
  read_count += setUpOne<RookEffectPieceKingRelative>(reader);
  read_count += setUpOne<BishopEffectPieceKingRelative>(reader);
  read_count += setUpOne<RookPawnYX>(reader);
  read_count += setUpOne<PawnPtypeOPtypeO>(reader);
  read_count += setUpOne<PromotedMinorPieces>(reader);
  read_count += setUpOne<KingPieceRelativeNoSupport>(reader);
  read_count += setUpOne<NonPawnAttacked>(reader);
  read_count += setUpOne<PtypeYY>(reader);
  read_count += setUpOne<PawnPtypeOPtypeOY>(reader);
  read_count += setUpOne<PawnDropX>(reader);
  read_count += setUpOne<King3Pieces>(reader);
  read_count += setUpOne<King3PiecesXY>(reader);
  read_count += setUpOne<King25EffectEachKXY>(reader);
  read_count += setUpOne<BishopHead>(reader);
  read_count += setUpOne<BishopHeadKingRelative>(reader);
  read_count += setUpOne<KnightCheckY>(reader);
  read_count += setUpOne<KnightHead>(reader);
  read_count += setUpOne<RookPromoteDefense>(reader);
  read_count += setUpOne<PawnDropPawnStand>(reader);
  read_count += setUpOne<PawnDropPawnStandX>(reader);
  read_count += setUpOne<PawnDropPawnStandY>(reader);
  read_count += setUpOne<KnightHeadOppPiecePawnOnStand>(reader);
  read_count += setUpOne<KingXBothBlocked>(reader);
  read_count += setUpOne<KingXBothBlockedY>(reader);
  read_count += setUpOne<KingRookBishop>(reader);
  read_count += setUpOne<PromotedMinorPiecesY>(reader);
  read_count += setUpOne<King25EffectSupported>(reader);
  read_count += setUpOne<King25EffectSupportedY>(reader);
  read_count += setUpOne<NonPawnAttackedKingRelative>(reader);
  read_count += setUpOne<NonPawnAttackedPtype>(reader);
  read_count += setUpOne<PtypeCount>(reader);
  read_count += setUpOne<KingXBlocked3>(reader);
  read_count += setUpOne<KingXBlocked3Y>(reader);
  read_count += setUpOne<PtypeCountXY>(reader);
  read_count += setUpOne<PtypeCountXYAttack>(reader);
  read_count += setUpOne<LanceEffectPieceKingRelative>(reader);
  read_count += setUpOne<KingMobility>(reader);
  read_count += setUpOne<KingMobilitySum>(reader);
  read_count += setUpOne<PtypeYPawnY>(reader);
  read_count += setUpOne<GoldAndSilverNearKing>(reader);
  read_count += setUpOne<PtypeCombination>(reader);
  read_count += setUpOne<PieceStandCombinationBoth>(reader);
  read_count += setUpOne<King25BothSide>(reader);
  read_count += setUpOne<King25BothSideX>(reader);
  read_count += setUpOne<King25BothSideY>(reader);
  read_count += setUpOne<GoldAndSilverNearKingCombination>(reader);
  read_count += setUpOne<KingMobilityWithRook>(reader);
  read_count += setUpOne<KingMobilityWithBishop>(reader);
  read_count += setUpOne<NumPiecesBetweenBishopAndKingSelf>(reader);
  read_count += setUpOne<NumPiecesBetweenBishopAndKingOpp>(reader);
  read_count += setUpOne<NumPiecesBetweenBishopAndKingAll>(reader);
  read_count += setUpOne<King25Effect3>(reader);
  read_count += setUpOne<SilverHeadPawnKingRelative>(reader);
  read_count += setUpOne<GoldKnightKingRelative>(reader);
  read_count += setUpOne<RookMobilitySum>(reader);
  read_count += setUpOne<RookMobilityX>(reader);
  read_count += setUpOne<RookMobilityY>(reader);
  read_count += setUpOne<RookMobilitySumKingX>(reader);
  read_count += setUpOne<RookMobilityXKingX>(reader);
  read_count += setUpOne<PinPtype>(reader);
  read_count += setUpOne<PinPtypeDistance>(reader);
  read_count += setUpOne<BishopMobilityEach>(reader);
  read_count += setUpOne<BishopBishopPiece>(reader);
  read_count += setUpOne<NonPawnPieceStandCombination>(reader);
  read_count += setUpOne<CanCheckNonPawnPieceStandCombination>(reader);
  read_count += setUpOne<King25Effect3Y>(reader);
  read_count += setUpOne<RookRook>(reader);
  read_count += setUpOne<RookRookPiece>(reader);
  read_count += setUpOne<PinPtypePawnAttack>(reader);
  read_count += setUpOne<King25Mobility>(reader);
  read_count += setUpOne<King25MobilityX>(reader);
  read_count += setUpOne<King25MobilityY>(reader);
  read_count += setUpOne<King25EffectCountCombination>(reader);
  read_count += setUpOne<GoldSideMove>(reader);
  read_count += setUpOne<King25EffectCountCombinationY>(reader);
  read_count += setUpOne<RookPromoteDefenseRookH>(reader);
  read_count += setUpOne<BishopHeadX>(reader);
  read_count += setUpOne<PawnDropNonDrop>(reader);
  read_count += setUpOne<PawnStateKingRelative>(reader);
  read_count += setUpOne<SilverFork>(reader);
  read_count += setUpOne<BishopRookFork>(reader);
  read_count += setUpOne<BishopStandFile5>(reader);
  read_count += setUpOne<KnightFork>(reader);
  read_count += setUpOne<NonPawnAttackedPtypePair>(reader);
  read_count += setUpOne<MajorCheckWithCapture>(reader);
  read_count += setUpOne<SilverAdvance26>(reader);
  read_count += setUpOne<RookSilverKnight>(reader);
  read_count += setUpOne<BishopSilverKnight>(reader);
  read_count += setUpOne<AttackMajorsInBase>(reader);
  read_count += setUpOne<CheckShadowPtype>(reader);
  read_count += setUpOne<Promotion37>(reader);

  initialized_flag = reader.failed() ? Zero : Loaded;
  if (initialized_flag != Loaded)
  {
    std::cerr << "Failed to load OpenMidEndingEval data "
	      << ' ' << read_count << std::endl;
  }
}

std::string osl::eval::ml::OpenMidEndingEval::defaultFilename()
{
  std::string filename = OslConfig::home();
  filename += "/data/eval.bin";
  return filename;
}

bool osl::eval::ml::OpenMidEndingEval::setUp()
{
  return setUp(defaultFilename().c_str());  
}

osl::eval::ml::
OpenMidEndingEval::OpenMidEndingEval(const NumEffectState &state, bool use_limit)
  : progress(state), use_progress_independent_value_limit(use_limit)
{
  assert(initialized_flag != Zero);
  
  pawns.fill(0);
  black_pawn_count = 0;
  turn = state.turn();
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit; ++i)
  {
    const Piece pawn = state.pieceOf(i);
    if (pawn.owner() == BLACK)
      ++black_pawn_count;
    if (pawn.isOnBoard() && !pawn.isPromoted())
      pawns[pawn.owner()][pawn.square().x() - 1] =
	pawn.square().y();
  }
  black_major_count = 0;
  black_gold_count = 0;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit; ++i)
  {
    if (state.pieceOf(i).owner() == BLACK)
      ++black_major_count;
  }
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit; ++i)
  {
    if (state.pieceOf(i).owner() == BLACK)
      ++black_major_count;
  }
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit; ++i)
  {
    if (state.pieceOf(i).owner() == BLACK)
      ++black_gold_count;
  }
  updateGoldSilverNearKing(state);

  ptype_count.fill(0);
  ptypeo_mask=0u;
  ptype_board_count.fill(0);
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.ptype() == KING)
      continue;
    ++ptype_count[piece.owner()][piece.ptype()];
    ptypeo_mask |= 1<<(piece.ptypeO()-PTYPEO_MIN);
    if (piece.isOnBoard())
      ++ptype_board_count[piece.owner()][piece.ptype()];
  }
  non_pawn_stand_count.fill(0);
  for (Ptype ptype: osl::PieceStand::order)
  {
    if (ptype == PAWN)
      continue;
    non_pawn_stand_count[BLACK] +=
      state.countPiecesOnStand(osl::BLACK, ptype);
    non_pawn_stand_count[WHITE] += 
      state.countPiecesOnStand(osl::WHITE, ptype);
  }
  progress_independent_value = PieceEval::eval(state);
  piece_stand_value = PieceStand::eval(state);
  piece_pair_value = PiecePair::eval(state, piece_pair_weights);
  piece_pair_king_value = PiecePairKing::eval(state);
  RookMobilityAll::eval(state, rook_mobility);
  BishopMobilityAll::eval(state, bishop_mobility);
  LanceMobilityAll::eval(state, lance_mobility);
  knight_advance = KnightAdvance::eval(state);

  rook_effect = RookEffectBase::eval(state);
  bishop_effect = BishopEffectBase::eval(state);

  King25EffectEachBoth::eval(state, king25_effect_each);

  King25EffectBoth::countEffectAndPiecesBoth<BLACK>(state, effect25[WHITE],
						    effect25_supported[WHITE],
						    black_attack_effect, black_attack_piece,
						    white_defense_effect, white_defense_piece,
						    black_attack_supported_piece,
						    white_vertical,
						    white_king_vertical);
  King25EffectBoth::countEffectAndPiecesBoth<WHITE>(state, effect25[BLACK],
						    effect25_supported[BLACK],
						    white_attack_effect, white_attack_piece,
						    black_defense_effect, black_defense_piece,
						    white_attack_supported_piece,
						    black_vertical,
						    black_king_vertical);
  recalculated_value =
    BishopExchangeSilverKing::eval(state) + 
    EnterKingDefense::eval(state) + 
    King25EffectAttack::eval(state,
			     black_attack_effect,
			     black_attack_piece,
			     white_attack_effect, white_attack_piece);
  recalculated_value +=
    King25EffectYAttack::eval(state,
			      black_attack_effect,
			      black_attack_piece,
			      white_attack_effect, white_attack_piece);
  kingx_blocked = KingXBothBlocked::eval(state);
  {
    MultiInt result_supported =
      King25EffectSupported::eval(black_attack_piece,
				  white_attack_piece,
				  black_attack_supported_piece,
				  white_attack_supported_piece);
    MultiInt result_supported_y =
      King25EffectSupportedY::eval(black_attack_piece,
				   white_attack_piece,
				   black_attack_supported_piece,
				   white_attack_supported_piece,
				   state.kingSquare<BLACK>().y(),
				   state.kingSquare<WHITE>().y());

    recalculated_stage_value = result_supported + result_supported_y;
    king_rook_bishop[BLACK]=KingRookBishop::evalOne<BLACK>(state);
    king_rook_bishop[WHITE]=KingRookBishop::evalOne<WHITE>(state);
    recalculated_stage_value+=king_rook_bishop[BLACK]-king_rook_bishop[WHITE];
    recalculated_stage_value+=KingXBlocked3::eval(state);
  }

  kingx_blocked += KingXBlocked::eval(state)+KingXBlockedY::eval(state);
  const MultiInt silver_retreat = SilverFeatures::eval(state);
  const MultiInt gold_retreat = GoldFeatures::eval(state);
  recalculated_stage_value += knight_advance;
  recalculated_stage_value += silver_retreat + gold_retreat;
  recalculated_stage_value += AllGold::eval(black_gold_count);
  recalculated_stage_value += AllMajor::eval(black_major_count);
  recalculated_stage_value += 
    King25EffectDefense::eval(state,black_defense_effect,black_defense_piece,
			      white_defense_effect, white_defense_piece);
  recalculated_stage_value += 
    King25EffectYDefense::eval(state,
			       black_defense_effect,
			       black_defense_piece,
			       white_defense_effect, white_defense_piece);
  recalculated_stage_value += AnagumaEmpty::eval(state);
  recalculated_stage_value += kingx_blocked[BLACK] + kingx_blocked[WHITE];

  recalculated_stage_value += NoPawnOnStand::eval(state, black_pawn_count);
  recalculated_stage_value += NonPawnPieceStand::eval(non_pawn_stand_count[BLACK], non_pawn_stand_count[WHITE]);
  recalculated_stage_value += PinPtypeAll::eval(state);
  recalculated_stage_value += KingMobility::eval(state) + KingMobilitySum::eval(state);
  recalculated_stage_value += GoldAndSilverNearKing::eval(state,
							  gs_near_king_count);
  recalculated_stage_value += PtypeCombination::eval(ptypeo_mask);
  recalculated_stage_value += PieceStandCombinationBoth::eval(state);
  king25_both_side[BLACK]=King25BothSide::evalOne<BLACK>(state,black_vertical);
  king25_both_side[WHITE]=King25BothSide::evalOne<WHITE>(state,white_vertical);
  recalculated_stage_value += king25_both_side[BLACK]-king25_both_side[WHITE];
  recalculated_stage_value += King25Mobility::eval(state,
						   black_king_vertical,
						   white_king_vertical);
  recalculated_stage_value += BishopStandFile5::eval(state);
  recalculated_stage_value += MajorCheckWithCapture::eval(state);
  recalculated_stage_value += SilverAdvance26::eval(state);

  king_table_value = KingPieceRelative::eval(state);

  pawn_drop = PawnDropBoth::eval(state);

  ptypex = PtypeX::eval(state);

  ptypey = PtypeY::eval(state);

  can_check[BLACK] =
    CanCheckNonPawnPieceStandCombination::canCheck<BLACK>(state);
  can_check[WHITE] =
    CanCheckNonPawnPieceStandCombination::canCheck<WHITE>(state);
  piece_stand_combination = NonPawnPieceStandCombination::eval(state,
							       can_check);
  NonPawnPieceStandTurn::eval(state, piece_stand_turn);
  rook_pawn = RookPawnY::eval(state, pawns);
  piece_stand_y = PieceStandY::eval(state);

  pawn_advance = PawnAdvance::eval(state);
  knight_check = KnightCheck::eval(state);
  pawn_ptypeo = PawnPtypeOPtypeO::eval(state);

  promoted_minor_piece = PromotedMinorPieces::eval(state);

  effected_mask[BLACK] =
    effected_mask_for_attacked[BLACK] =
    state.effectedMask(BLACK);
  effected_mask[WHITE] =
    effected_mask_for_attacked[WHITE] =
    state.effectedMask(WHITE);
  mask_t black_ppawn =
    effected_mask_for_attacked[BLACK].selectBit<PAWN>() &
    state.promotedPieces().getMask<PAWN>();
  mask_t white_ppawn =
    effected_mask_for_attacked[WHITE].selectBit<PAWN>() &
    state.promotedPieces().getMask<PAWN>();
  effected_mask_for_attacked[BLACK].clearBit<PAWN>();
  effected_mask_for_attacked[WHITE].clearBit<PAWN>();
  effected_mask_for_attacked[BLACK].orMask(PtypeFuns<PAWN>::indexNum, black_ppawn);
  effected_mask_for_attacked[WHITE].orMask(PtypeFuns<PAWN>::indexNum, white_ppawn);
  nosupport = KingPieceRelativeNoSupport::eval(state);
  NonPawnAttacked::eval(state, non_pawn_attacked);
  NonPawnAttackedPtype::eval(state, attacked_mask, non_pawn_attacked_ptype);
  knight_head = KnightHead::eval(state);

  ptype_yy = PtypeYY::eval(state);
  king3pieces = King3Pieces::eval(state);
  bishop_head = BishopHead::eval(state);
  rook_promote_defense = RookPromoteDefense::eval(state);
  PtypeCount::eval(state, ptype_count, ptype_board_count, ptype_count_value);
  lance_effect_piece = LanceEffectPieceKingRelative::eval(state);
  ptype_y_pawn_y = PtypeYPawnY::eval(state, pawns);
  bishop_and_king = NumPiecesBetweenBishopAndKing::eval(state);
  recalculated_stage_value += King25Effect3::eval(state, effect25);
  recalculated_stage_value += BishopBishopPiece::eval(state);
  recalculated_stage_value += RookRook::eval(state);
  recalculated_stage_value += RookRookPiece::eval(state);
  recalculated_stage_value += King25EffectCountCombination::eval(state, effect25);
  recalculated_stage_value += NonPawnAttackedPtypePair::eval(state);
  rook_silver_knight = RookSilverKnight::eval(state);
  bishop_silver_knight = BishopSilverKnight::eval(state);
  recalculated_stage_value += AttackMajorsInBase::eval(state);
  recalculated_stage_value += CheckShadowPtype::eval(state);
  recalculated_stage_value += Promotion37::eval(state);
  piece_fork_turn = SilverFork::eval(state, silver_drop);
  piece_fork_turn += BishopRookFork::eval(state, bishop_drop, rook_drop);
  piece_fork_turn += KnightFork::eval(state, knight_fork_squares, knight_drop);
  invalidateCache();
}

int osl::eval::ml::
OpenMidEndingEval::expect(const NumEffectState &state, Move move) const
{
  if (move.isPass())
    return value();
  int value;
  if(move.player()==BLACK)
    value = PieceEval::evalWithUpdate<BLACK>(state, move, progress_independent_value);
  else
    value = PieceEval::evalWithUpdate<WHITE>(state, move, progress_independent_value);

#ifdef USE_TEST_PROGRESS
  return roundUp(value * NewProgress::maxProgress() +
		 openingValue() * (NewProgress::maxProgress() - progress.progress()) + 
		 endgameValue() * progress.progress());
#else
  return roundUp(value * 16 +
		 openingValue() * (16 - progress.progress16().value()) + 
		 endgameValue() * progress.progress16().value());
#endif
}

void osl::eval::ml::
OpenMidEndingEval::update(const NumEffectState &new_state, Move last_move)
{
  turn = alt(turn);
  assert(new_state.turn() == turn);
  if (last_move.isPass())
  {
    invalidateCache();
    return;
  }
  if(last_move.player()==BLACK)
    updateSub<BLACK>(new_state,last_move);
  else
    updateSub<WHITE>(new_state,last_move);
}
template<osl::Player P>
void osl::eval::ml::
OpenMidEndingEval::updateSub(const NumEffectState &new_state, Move last_move)
{
  assert(last_move.player()==P);
  const Square opp_king =
    new_state.kingSquare<alt(P)>();
  const Square self_king =
    new_state.kingSquare<P>();
  Ptype captured = last_move.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    Ptype base = unpromote(captured);
    if (base == PAWN)
    {
      if (P == BLACK)
	++black_pawn_count;
      else
	--black_pawn_count;
    }
    else
    {
      ++non_pawn_stand_count[P];
    }
    if (captured == PAWN)
    {
      pawns[alt(P)][last_move.to().x() - 1] = 0;
    }
    if (isMajorBasic(base))
    {
      if (P == BLACK)
	++black_major_count;
      else
	--black_major_count;
    }
    if (base == GOLD)
    {
      if (P == BLACK)
	++black_gold_count;
      else
	--black_gold_count;
    }
    if (base == GOLD || base == SILVER)
    {
      const int y_diff = std::abs(last_move.to().y() - opp_king.y());
      const int x_diff = std::abs(last_move.to().x() - opp_king.x());
      if (y_diff <= 2 && x_diff <= 3)
      {
	--gs_near_king_count[alt(P)][std::max(x_diff, y_diff) - 1];
      }
    }
  }
  const Ptype base_ptype = unpromote(last_move.ptype());
  {
    if (base_ptype == GOLD || base_ptype == SILVER)
    {
      if (!last_move.isDrop())
      {
	const int y_diff = std::abs(last_move.from().y() - self_king.y());
	const int x_diff = std::abs(last_move.from().x() - self_king.x());
	if (y_diff <= 2 && x_diff <= 3)
	{
	  --gs_near_king_count[P][std::max(x_diff, y_diff) - 1];
	}
      }
      {
	const int y_diff = std::abs(last_move.to().y() - self_king.y());
	const int x_diff = std::abs(last_move.to().x() - self_king.x());
	if (y_diff <= 2 && x_diff <= 3)
	{
	  ++gs_near_king_count[P][std::max(x_diff, y_diff) - 1];
	}
      }
    }
    if (base_ptype == KING)
    {
      updateGoldSilverNearKing(new_state);
    }
  }
  if (last_move.isDrop() && last_move.ptype() != PAWN)
  {
    --non_pawn_stand_count[P];
  }
  if (last_move.ptype() == PPAWN && last_move.isPromotion())
  {
    pawns[P][last_move.from().x() - 1] = 0;
  }
  if (last_move.ptype() == PAWN)
  {
    pawns[P][last_move.to().x() - 1] = last_move.to().y();
  }
  const Square kb = new_state.kingSquare<BLACK>(), kw = new_state.kingSquare<WHITE>();
  {
    BoardMask mask = new_state.changedEffects();
    mask.set(last_move.from());
    mask.set(last_move.to());
    const bool update_black = mask.anyInRange(Board_Mask_Table5x5.mask(kw)); // black attack to white
    const bool update_white = mask.anyInRange(Board_Mask_Table5x5.mask(kb));
    if (update_black ||
	(effect25_supported[WHITE] & new_state.effectedMask(BLACK)) !=
	effect25_supported[WHITE] ||
	(~effect25_supported[WHITE] & effect25[WHITE] & ~new_state.effectedMask(BLACK)) !=
	(~effect25_supported[WHITE] & effect25[WHITE])){
      King25EffectBoth::countEffectAndPiecesBoth<BLACK>(
	new_state, effect25[WHITE], effect25_supported[WHITE],
	black_attack_effect, black_attack_piece,
	white_defense_effect, white_defense_piece,
	black_attack_supported_piece, white_vertical, white_king_vertical);
      king25_both_side[WHITE]=King25BothSide::evalOne<WHITE>(new_state,white_vertical);
    }
    if (update_white ||
	(effect25_supported[BLACK] & new_state.effectedMask(WHITE)) !=
	effect25_supported[BLACK] ||
	(~effect25_supported[BLACK] & effect25[BLACK] & ~new_state.effectedMask(WHITE)) !=
	(~effect25_supported[BLACK] & effect25[BLACK])){
      King25EffectBoth::countEffectAndPiecesBoth<WHITE>(
	new_state, effect25[BLACK], effect25_supported[BLACK],
	white_attack_effect, white_attack_piece,
	black_defense_effect, black_defense_piece,
	white_attack_supported_piece, black_vertical, black_king_vertical);
      king25_both_side[BLACK]=King25BothSide::evalOne<BLACK>(new_state,black_vertical);
    }
  }
#ifdef USE_TEST_PROGRESS
  progress.updateSub<P>(new_state, last_move);
#else
  progress.update(new_state, last_move);
#endif

  progress_independent_value =
    PieceEval::evalWithUpdate<P>(new_state, last_move, progress_independent_value);
  piece_stand_value =
    PieceStand::evalWithUpdate<P>(new_state, last_move,
				      piece_stand_value);
  if (new_state.longEffectChanged<ROOK>() || last_move.ptype() == KING)
  {
    RookMobilityAll::eval(new_state, rook_mobility);
    rook_effect = RookEffectBase::eval(new_state);
  }
  if (new_state.longEffectChanged<BISHOP>())
  {
    BishopMobilityAll::eval(new_state, bishop_mobility);
    bishop_effect = BishopEffectBase::eval(new_state);
  }
  else if (last_move.ptype() == KING)
  {
    bishop_effect = BishopEffectBase::eval(new_state);
  }
  if (new_state.longEffectChanged<LANCE>() || last_move.ptype() == KING)
  {
    LanceMobilityAll::eval(new_state, lance_mobility);
    lance_effect_piece = LanceEffectPieceKingRelative::eval(new_state);
  }

  if (new_state.anyEffectChanged<KNIGHT>()) {
    knight_advance = KnightAdvance::eval(new_state);
  }
  KingXBlockedBoth::evalWithUpdateBang(new_state, last_move, kingx_blocked);
  const MultiInt silver_features = SilverFeatures::eval(new_state);
  const MultiInt gold_retreat = GoldFeatures::eval(new_state);
  recalculated_stage_value = silver_features+gold_retreat;
  recalculated_stage_value += AllGold::eval(black_gold_count);
  recalculated_stage_value += AllMajor::eval(black_major_count);
  
  King25EffectEachBoth::evalWithUpdate(new_state, last_move,
				       king25_effect_each);
  
  recalculated_value =
    BishopExchangeSilverKing::eval(new_state) + 
    EnterKingDefense::eval(new_state) + 
    King25EffectAttack::eval(new_state,
			     black_attack_effect,
			     black_attack_piece,
			     white_attack_effect, white_attack_piece);
  recalculated_value +=
    King25EffectYAttack::eval(new_state,
			      black_attack_effect,
			      black_attack_piece,
			      white_attack_effect, white_attack_piece);

  recalculated_stage_value += 
    King25EffectDefense::eval(new_state,black_defense_effect,black_defense_piece,
			      white_defense_effect, white_defense_piece);
  recalculated_stage_value += 
    King25EffectYDefense::eval(new_state,
			       black_defense_effect,
			       black_defense_piece,
			       white_defense_effect, white_defense_piece);
  recalculated_stage_value += knight_advance;
  recalculated_stage_value += AnagumaEmpty::eval(new_state);
  recalculated_stage_value += kingx_blocked[BLACK] + kingx_blocked[WHITE];
  recalculated_stage_value += NoPawnOnStand::eval(new_state, black_pawn_count);
  recalculated_stage_value += NonPawnPieceStand::eval(non_pawn_stand_count[BLACK], non_pawn_stand_count[WHITE]);
  recalculated_stage_value += PinPtypeAll::eval(new_state);
  recalculated_stage_value += KingMobility::eval(new_state) + KingMobilitySum::eval(new_state);
  recalculated_stage_value += GoldAndSilverNearKing::eval(new_state,
							  gs_near_king_count);
  recalculated_stage_value += PieceStandCombinationBoth::eval(new_state);
  
  {
    MultiInt result_supported =
      King25EffectSupported::eval(black_attack_piece,
				  white_attack_piece,
				  black_attack_supported_piece,
				  white_attack_supported_piece);
    MultiInt result_supported_y =
      King25EffectSupportedY::eval(black_attack_piece,
				   white_attack_piece,
				   black_attack_supported_piece,
				   white_attack_supported_piece,
				   new_state.kingSquare<BLACK>().y(),
				   new_state.kingSquare<WHITE>().y());
    recalculated_stage_value += result_supported + result_supported_y;
    if(isMajorNonPieceOK(last_move.ptype()) || 
       isMajorNonPieceOK(last_move.capturePtype())){ // rook or bishop
      king_rook_bishop[BLACK]=KingRookBishop::evalOne<BLACK>(new_state);
      king_rook_bishop[WHITE]=KingRookBishop::evalOne<WHITE>(new_state);
    }
    else if(last_move.ptype() == KING){
      king_rook_bishop[P]=KingRookBishop::evalOne<P>(new_state);
    }
    recalculated_stage_value +=king_rook_bishop[BLACK]-king_rook_bishop[WHITE];
    recalculated_stage_value += KingXBlocked3::eval(new_state);
    recalculated_stage_value += king25_both_side[BLACK]-king25_both_side[WHITE];
    recalculated_stage_value += King25Mobility::eval(new_state,
						     black_king_vertical,
						     white_king_vertical);
  }
  king_table_value = KingPieceRelative::evalWithUpdate<P>
    (new_state, last_move, king_table_value);
  piece_pair_value = PiecePair::evalWithUpdateCompiled(new_state,
						       last_move,
						       piece_pair_value);
  PiecePairKing::evalWithUpdateBang<P>(new_state, last_move,
				       piece_pair_king_value);
  pawn_drop = PawnDropBoth::evalWithUpdate<P>(new_state,
					      last_move, pawn_drop);

  ptypex = PtypeX::evalWithUpdate<P>(new_state, last_move, ptypex);
  ptypey = PtypeY::evalWithUpdate<P>(new_state, last_move, ptypey);
  CArray<bool, 2> can_check_new;
  can_check_new[BLACK] =
    CanCheckNonPawnPieceStandCombination::canCheck<BLACK>(new_state);
  can_check_new[WHITE] =
    CanCheckNonPawnPieceStandCombination::canCheck<WHITE>(new_state);
  piece_stand_combination =
    NonPawnPieceStandCombination::evalWithUpdate(new_state,
						 last_move,
						 piece_stand_combination,
						 can_check,
						 can_check_new);
  can_check = can_check_new;
  NonPawnPieceStandTurn::evalWithUpdateBang<P>(new_state,
					    last_move,
					    piece_stand_turn);
  rook_pawn = RookPawnY::eval(new_state, pawns);
  piece_stand_y = PieceStandY::evalWithUpdate<P>(new_state, last_move,
						 piece_stand_y);
  PawnAdvanceAll::evalWithUpdateBang<P>(new_state,
					last_move,
					pawn_advance);

  knight_check = KnightCheck::eval(new_state);
  pawn_ptypeo = PawnPtypeOPtypeO::template evalWithUpdate<P>(new_state, last_move,
 						 pawns,
 						 pawn_ptypeo);

  promoted_minor_piece =
    PromotedMinorPieces::evalWithUpdate(new_state,
					last_move,
					promoted_minor_piece);

  nosupport = KingPieceRelativeNoSupport::evalWithUpdate(new_state, last_move,
							 effected_mask,
							 nosupport);
  NonPawnAttacked::evalWithUpdateBang<P>(new_state,
				      last_move,
				      effected_mask_for_attacked,
				      non_pawn_attacked);
  NonPawnAttackedPtype::evalWithUpdateBang<P>(
    new_state, last_move, effected_mask_for_attacked,
    attacked_mask, non_pawn_attacked_ptype);
  effected_mask[BLACK] =
    effected_mask_for_attacked[BLACK] =
    new_state.effectedMask(BLACK);
  effected_mask[WHITE] =
    effected_mask_for_attacked[WHITE] =
    new_state.effectedMask(WHITE);
  mask_t black_ppawn =
    effected_mask_for_attacked[BLACK].selectBit<PAWN>() &
    new_state.promotedPieces().template getMask<PAWN>();
  mask_t white_ppawn =
    effected_mask_for_attacked[WHITE].selectBit<PAWN>() &
    new_state.promotedPieces().template getMask<PAWN>();
  effected_mask_for_attacked[BLACK].clearBit<PAWN>();
  effected_mask_for_attacked[WHITE].clearBit<PAWN>();
  effected_mask_for_attacked[BLACK].orMask(PtypeFuns<PAWN>::indexNum, black_ppawn);
  effected_mask_for_attacked[WHITE].orMask(PtypeFuns<PAWN>::indexNum, white_ppawn);

  ptype_yy = PtypeYY::evalWithUpdate(new_state, last_move, ptype_yy);
  king3pieces = King3Pieces::evalWithUpdate(new_state, last_move, king3pieces);
  bishop_head = BishopHead::eval(new_state);
  knight_head = KnightHead::eval(new_state);
  rook_promote_defense = RookPromoteDefense::eval(new_state);
  PtypeCount::evalWithUpdateBang<P>(new_state,
				 last_move, ptype_count, ptype_board_count,
				 ptype_count_value,ptypeo_mask);
  PtypeYPawnY::evalWithUpdateBang<P>(new_state, last_move,pawns, ptype_y_pawn_y);
  recalculated_stage_value += PtypeCombination::eval(ptypeo_mask);
  bishop_and_king = NumPiecesBetweenBishopAndKing::eval(new_state);
  recalculated_stage_value += King25Effect3::eval(new_state, effect25);
  recalculated_stage_value += BishopBishopPiece::eval(new_state);
  recalculated_stage_value += RookRook::eval(new_state);
  recalculated_stage_value += RookRookPiece::eval(new_state);
  recalculated_stage_value += King25EffectCountCombination::eval(new_state, effect25);
  recalculated_stage_value += BishopStandFile5::eval(new_state);
  recalculated_stage_value += MajorCheckWithCapture::eval(new_state);
  recalculated_stage_value += SilverAdvance26::eval(new_state);
  if (base_ptype == ROOK || last_move.ptype() == SILVER ||
      last_move.ptype() == KNIGHT ||
      captured == ROOK || captured == PROOK || captured == SILVER ||
      captured == KNIGHT ||
      (last_move.isPromotion() &&
       (base_ptype == SILVER || base_ptype == KNIGHT)))
  {
    rook_silver_knight = RookSilverKnight::eval(new_state);
  }
  if (base_ptype == BISHOP || last_move.ptype() == SILVER ||
      last_move.ptype() == KNIGHT ||
      captured == BISHOP || captured == PBISHOP || captured == SILVER ||
      captured == KNIGHT ||
      (last_move.isPromotion() &&
       (base_ptype == SILVER || base_ptype == KNIGHT)))
  {
    bishop_silver_knight = BishopSilverKnight::eval(new_state);
  }
  recalculated_stage_value += AttackMajorsInBase::eval(new_state);
  recalculated_stage_value += CheckShadowPtype::eval(new_state);
#ifdef USE_TEST_PROGRESS
  recalculated_stage_value += progress.rawData().promotion37_eval;
  recalculated_stage_value += progress.rawData().non_pawn_ptype_attacked_pair_eval[BLACK]
    + progress.rawData().non_pawn_ptype_attacked_pair_eval[WHITE];
#else
  recalculated_stage_value += Promotion37::eval(new_state);
  recalculated_stage_value += NonPawnAttackedPtypePair::eval(new_state);
#endif
  piece_fork_turn = SilverFork::eval(new_state, silver_drop);
  piece_fork_turn += BishopRookFork::eval(new_state, bishop_drop, rook_drop);
  piece_fork_turn += KnightFork::evalWithUpdate<P>(new_state, last_move, knight_fork_squares, knight_drop);
  invalidateCache();
}

#ifndef MINIMAL
osl::eval::ml::OpenMidEndingEvalDebugInfo
osl::eval::ml::OpenMidEndingEval::debugInfo(const NumEffectState &state)
{
  OpenMidEndingEvalDebugInfo debug_info;
  debug_info.value = value();
  debug_info.progress = progress16().value();
  debug_info.progress_independent_values[OpenMidEndingEvalDebugInfo::PIECE] = progress_independent_value;
  debug_info.progress_independent_values[OpenMidEndingEvalDebugInfo::BISHOP_EXCHANGE_SILVER_KING]
    = BishopExchangeSilverKing::eval(state);
  debug_info.progress_independent_values[OpenMidEndingEvalDebugInfo::ENTER_KING_DEFENSE]
    = EnterKingDefense::eval(state);
  int black_attack_effect, black_attack_piece, black_defense_effect, black_defense_piece,
    white_attack_effect, white_attack_piece, white_defense_effect, white_defense_piece;
  CArray<int, 5> black_vertical, white_vertical,
    black_king_vertical, white_king_vertical;
  King25EffectBoth::countEffectAndPiecesBoth<BLACK>(state, effect25[WHITE],
						    effect25_supported[WHITE],
						    black_attack_effect, black_attack_piece,
						    white_defense_effect, white_defense_piece,
						    black_attack_supported_piece,
						    white_vertical,
						    white_king_vertical);
  King25EffectBoth::countEffectAndPiecesBoth<WHITE>(state, effect25[BLACK],
						    effect25_supported[BLACK],
						    white_attack_effect, white_attack_piece,
						    black_defense_effect, black_defense_piece,
						    white_attack_supported_piece,
						    black_vertical,
						    black_king_vertical);
  debug_info.progress_independent_values[OpenMidEndingEvalDebugInfo::KING25_EFFECT_ATTACK] =
    King25EffectBoth::eval(state,
			   black_attack_effect,
			   black_attack_piece,
			   white_attack_effect, white_attack_piece,
			   black_defense_effect, black_defense_piece,
			   white_defense_effect, white_defense_piece) +
    King25EffectY::eval(state,
			black_attack_effect,
			black_attack_piece,
			white_attack_effect, white_attack_piece,
			black_defense_effect, black_defense_piece,
			white_defense_effect, white_defense_piece);
  debug_info.progress_independent_values[OpenMidEndingEvalDebugInfo::PIECE_PAIR] = piece_pair_value;
  debug_info.progress_independent_values[OpenMidEndingEvalDebugInfo::PIECE_PAIR_KING] = piece_pair_king_value[BLACK] + piece_pair_king_value[WHITE];

  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING_PIECE_RELATIVE] =
    king_table_value;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PIECE_STAND] =
    piece_stand_value;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING25_EFFECT_EACH] =
    king25_effect_each[BLACK] + king25_effect_each[WHITE];
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PTYPEX] = ptypex;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PTYPEY] = ptypey;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ROOK_MOBILITY] = rook_mobility;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::BISHOP_MOBILITY] = bishop_mobility;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::LANCE_MOBILITY] = lance_mobility;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ROOK_EFFECT] = rook_effect;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::BISHOP_EFFECT] = bishop_effect;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PIECE_STAND_COMBINATION] = piece_stand_combination;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PIECE_STAND_TURN] = piece_stand_turn[turn];
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ROOK_PAWN] = rook_pawn;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PAWN_DROP] = pawn_drop;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PIECE_STAND_Y] = piece_stand_y;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KNIGHT_CHECK] = knight_check;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PAWN_ADVANCE] = pawn_advance;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PAWN_PTYPEO] = pawn_ptypeo;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PROMOTED_MINOR_PIECE] = promoted_minor_piece;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING_PIECE_RELATIVE_NOSUPPORT] = nosupport;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::NON_PAWN_ATTACKED] = non_pawn_attacked[turn];
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::NON_PAWN_ATTACKED_PTYPE] = non_pawn_attacked_ptype[turn];
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PTYPE_YY] = ptype_yy;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING3PIECES] = king3pieces;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::BISHOP_HEAD] = bishop_head;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KNIGHT_HEAD] = knight_head;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ROOK_PROMOTE_DEFENSE] = rook_promote_defense;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PTYPE_COUNT] = ptype_count_value;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::LANCE_EFFECT_PIECE] = lance_effect_piece;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PTYPE_Y_PAWN_Y] = ptype_y_pawn_y;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::BISHOP_AND_KING] = bishop_and_king;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PIECE_FORK_TURN] = piece_fork_turn[turn];
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ROOK_SILVER_KNIGHT] = rook_silver_knight;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::BISHOP_SILVER_KNIGHT] = bishop_silver_knight;
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING25_EFFECT_SUPPORTED] =
    King25EffectSupported::eval(black_attack_piece,
                                white_attack_piece,
                                black_attack_supported_piece,
                                white_attack_supported_piece) +
    King25EffectSupportedY::eval(black_attack_piece,
                                 white_attack_piece,
                                 black_attack_supported_piece,
                                 white_attack_supported_piece,
                                 state.kingSquare<BLACK>().y(),
                                 state.kingSquare<WHITE>().y());
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING_ROOK_BISHOP] =
    king_rook_bishop[BLACK] - king_rook_bishop[WHITE];
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING_X_BLOCKED3] =
    KingXBlocked3::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::GOLD_RETREAT] =
    GoldFeatures::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::SILVER_RETREAT] =
    SilverFeatures::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ALL_GOLD] =
    AllGold::eval(black_gold_count);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ALL_MAJOR] =
    AllMajor::eval(black_major_count);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING25_EFFECT_DEFENSE] =
    King25EffectDefense::eval(state, black_defense_effect, black_defense_piece,
			      white_defense_effect, white_defense_piece) +
    King25EffectYDefense::eval(state,
			       black_defense_effect,
			       black_defense_piece,
			       white_defense_effect, white_defense_piece);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ANAGUMA_EMPTY] =
    AnagumaEmpty::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::NO_PAWN_ON_STAND] =
    NoPawnOnStand::eval(state, black_pawn_count);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::NON_PAWN_PIECE_STAND] =
    NonPawnPieceStand::eval(non_pawn_stand_count[BLACK], non_pawn_stand_count[WHITE]);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PIN_PTYPE_ALL] =
    PinPtypeAll::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING_MOBILITY] =
    KingMobility::eval(state) + KingMobilitySum::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::GOLD_AND_SILVER_NEAR_KING] =
    GoldAndSilverNearKing::eval(state,
                                gs_near_king_count);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::PTYPE_COMBINATION] =
    PtypeCombination::eval(ptypeo_mask);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING25_BOTH_SIDE] =
    king25_both_side[BLACK] - king25_both_side[WHITE];
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING25_MOBILITY] =
    King25Mobility::eval(state,
                         black_king_vertical,
                         white_king_vertical);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::BISHOP_STAND_FILE5] =
    BishopStandFile5::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::MAJOR_CHECK_WITH_CAPTURE] =
    MajorCheckWithCapture::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::SILVER_ADVANCE26] =
    SilverAdvance26::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING25_EFFECT3] =
    King25Effect3::eval(state, effect25);                                 
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::BISHOP_BISHOP_PIECE] =
    BishopBishopPiece::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ROOK_ROOK] =
    RookRook::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ROOK_ROOK_PIECE] =
    RookRookPiece::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::KING25_EFFECT_COUNT_COMBINATION] =
    King25EffectCountCombination::eval(state, effect25);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::NON_PAWN_ATTACKED_PTYPE_PAIR] =
    NonPawnAttackedPtypePair::eval(state);
  debug_info.stage_values[OpenMidEndingEvalDebugInfo::ATTACK_MAJORS_IN_BASE] =
    AttackMajorsInBase::eval(state);

  return debug_info;
}

#define DEBUGPRINT(x) std::cerr << "  " << #x << " " << x << "\n"
void osl::eval::ml::OpenMidEndingEval::
debug() const
{
  DEBUGPRINT(king_table_value[0]);
  DEBUGPRINT(piece_stand_value[0]);
  DEBUGPRINT(king25_effect_each[BLACK][0] + king25_effect_each[WHITE][0]);
  DEBUGPRINT(ptypex[0]);
  DEBUGPRINT(ptypey[0]);
  DEBUGPRINT(rook_mobility[0]);
  DEBUGPRINT(bishop_mobility[0]);
  DEBUGPRINT(lance_mobility[0]);
  DEBUGPRINT(rook_effect[0]);
  DEBUGPRINT(bishop_effect[0]);
  DEBUGPRINT(piece_stand_combination[0]); 
  DEBUGPRINT(piece_stand_turn[turn][0]);
  DEBUGPRINT(rook_pawn[0]);
  DEBUGPRINT(pawn_drop[0]);
  DEBUGPRINT(piece_stand_y[0]);
  DEBUGPRINT(knight_check[0]);
  DEBUGPRINT(pawn_advance[0]);
  DEBUGPRINT(pawn_ptypeo[0]);
  DEBUGPRINT(promoted_minor_piece[0]);
  DEBUGPRINT(nosupport[0]);
  DEBUGPRINT(non_pawn_attacked[turn][0]);
  DEBUGPRINT(non_pawn_attacked_ptype[turn][0]);
  DEBUGPRINT(ptype_yy[0]);
  DEBUGPRINT(king3pieces[0]);
  DEBUGPRINT(bishop_head[0]);
  DEBUGPRINT(knight_head[0]);
  DEBUGPRINT(rook_promote_defense[0]);
  DEBUGPRINT(ptype_count_value[0]);
  DEBUGPRINT(lance_effect_piece[0]);
  DEBUGPRINT(ptype_y_pawn_y[0]);
  DEBUGPRINT(bishop_and_king[0]);
  DEBUGPRINT(recalculated_stage_value[0]);
}

void osl::eval::ml::OpenMidEndingEval::
setRandom()
{
  std::lock_guard<std::mutex> lk(initialize_mutex);
  initialized_flag = Random;
  
  setRandomOne<King25EffectAttack>();
  setRandomOne<King25EffectYAttack>();

  // opening
  setRandomOne<PieceStand>(0);
  setRandomOne<Pin>(0);
  setRandomOne<King25EffectEachBothOpening>();
  setRandomOne<PawnDrop>(0);
  setRandomOne<NoPawnOnStand>(0);
  setRandomOne<GoldRetreat>(0);
  setRandomOne<SilverRetreat>(0);
  setRandomOne<KnightAdvance>(0);
  setRandomOne<AllMajor>(0);
  setRandomOne<KingXBlocked>(0);
  setRandomOne<KingXBlockedY>(0);
  setRandomOne<AllGold>(0);
  setRandomOne<PtypeX>(0);
  setRandomOne<PtypeY>(0);
  setRandomOne<AnagumaEmpty>(0);
  setRandomOne<NonPawnPieceStand>(0);
  setRandomOne<King25EffectDefense>(0);
  setRandomOne<King25EffectYDefense>(0);
  setRandomOne<RookMobility>(0);
  setRandomOne<BishopMobility>(0);
  setRandomOne<LanceMobility>(0);
  setRandomOne<RookEffect>(0);
  setRandomOne<BishopEffect>(0);
  setRandomOne<PawnAdvance>(0);
  setRandomOne<PawnDropY>(0);
  setRandomOne<KnightCheck>(0);

  // midgame
  setRandomOne<PieceStand>(1);
  setRandomOne<Pin>(1);
  setRandomOne<King25EffectEachBothMidgame>();
  setRandomOne<PawnDrop>(1);
  setRandomOne<NoPawnOnStand>(1);
  setRandomOne<GoldRetreat>(1);
  setRandomOne<SilverRetreat>(1);
  setRandomOne<KnightAdvance>(1);
  setRandomOne<AllMajor>(1);
  setRandomOne<KingXBlocked>(1);
  setRandomOne<KingXBlockedY>(1);
  setRandomOne<AllGold>(1);
  setRandomOne<PtypeX>(1);
  setRandomOne<PtypeY>(1);
  setRandomOne<AnagumaEmpty>(1);
  setRandomOne<NonPawnPieceStand>(1);
  setRandomOne<King25EffectDefense>(1);
  setRandomOne<King25EffectYDefense>(1);
  setRandomOne<RookMobility>(1);
  setRandomOne<BishopMobility>(1);
  setRandomOne<LanceMobility>(1);
  setRandomOne<RookEffect>(1);
  setRandomOne<BishopEffect>(1);
  setRandomOne<PawnAdvance>(1);
  setRandomOne<PawnDropY>(1);
  setRandomOne<KnightCheck>(1);

#ifdef EVAL_QUAD
  // midgame2
  setRandomOne<PieceStand>(2);
  setRandomOne<Pin>(2);
  setRandomOne<King25EffectEachBothEnding>();
  setRandomOne<PawnDrop>(2);
  setRandomOne<NoPawnOnStand>(2);
  setRandomOne<GoldRetreat>(2);
  setRandomOne<SilverRetreat>(2);
  setRandomOne<KnightAdvance>(2);
  setRandomOne<AllMajor>(2);
  setRandomOne<KingXBlocked>(2);
  setRandomOne<KingXBlockedY>(2);
  setRandomOne<AllGold>(2);
  setRandomOne<PtypeX>(2);
  setRandomOne<PtypeY>(2);
  setRandomOne<AnagumaEmpty>(2);
  setRandomOne<NonPawnPieceStand>(2);
  setRandomOne<King25EffectDefense>(2);
  setRandomOne<King25EffectYDefense>(2);
  setRandomOne<RookMobility>(2);
  setRandomOne<BishopMobility>(2);
  setRandomOne<LanceMobility>(2);
  setRandomOne<RookEffect>(2);
  setRandomOne<BishopEffect>(2);
  setRandomOne<PawnAdvance>(2);
  setRandomOne<PawnDropY>(2);
  setRandomOne<KnightCheck>(2);
#endif
  // endgame
  setRandomOne<PieceStand>(EndgameIndex);
  setRandomOne<Pin>(EndgameIndex);
  setRandomOne<King25EffectEachBothMidgame>();
  setRandomOne<PawnDrop>(EndgameIndex);
  setRandomOne<NoPawnOnStand>(EndgameIndex);
  setRandomOne<GoldRetreat>(EndgameIndex);
  setRandomOne<SilverRetreat>(EndgameIndex);
  setRandomOne<KnightAdvance>(EndgameIndex);
  setRandomOne<AllMajor>(EndgameIndex);
  setRandomOne<KingXBlocked>(EndgameIndex);
  setRandomOne<KingXBlockedY>(EndgameIndex);
  setRandomOne<AllGold>(EndgameIndex);
  setRandomOne<PtypeX>(EndgameIndex);
  setRandomOne<PtypeY>(EndgameIndex);
  setRandomOne<AnagumaEmpty>(EndgameIndex);
  setRandomOne<NonPawnPieceStand>(EndgameIndex);
  setRandomOne<King25EffectDefense>(EndgameIndex);
  setRandomOne<King25EffectYDefense>(EndgameIndex);
  setRandomOne<RookMobility>(EndgameIndex);
  setRandomOne<BishopMobility>(EndgameIndex);
  setRandomOne<LanceMobility>(EndgameIndex);
  setRandomOne<RookEffect>(EndgameIndex);
  setRandomOne<BishopEffect>(EndgameIndex);
  setRandomOne<PawnAdvance>(EndgameIndex);
  setRandomOne<PawnDropY>(EndgameIndex);
  setRandomOne<KnightCheck>(EndgameIndex);

  // both
  setRandomOne<KingPieceRelative>(0);
  setRandomOne<KingPieceRelative>(1);
#ifdef EVAL_QUAD
  setRandomOne<KingPieceRelative>(2);
#endif
  setRandomOne<KingPieceRelative>(EndgameIndex);
  setRandomOne<NonPawnPieceStandCombination>();
  setRandomOne<NonPawnPieceStandTurn>();
  setRandomOne<King25EffectEachXY>();
  setRandomOne<RookPawnY>();
  setRandomOne<RookEffectPiece>();
  setRandomOne<BishopEffectPiece>();
  setRandomOne<PieceStandY>();
  setRandomOne<RookEffectPieceKingRelative>();
  setRandomOne<BishopEffectPieceKingRelative>();
  setRandomOne<RookPawnYX>();
  setRandomOne<PawnPtypeOPtypeO>();
  setRandomOne<CanCheckNonPawnPieceStandCombination>();
  setRandomOne<PromotedMinorPieces>();
  setRandomOne<KingPieceRelativeNoSupport>();
  setRandomOne<NonPawnAttacked>();
  setRandomOne<PtypeYY>();
  setRandomOne<PawnPtypeOPtypeOY>();
  setRandomOne<PawnDropX>();
  setRandomOne<King3Pieces>();
  setRandomOne<King3PiecesXY>();
  setRandomOne<King25EffectEachKXY>();
  setRandomOne<BishopHead>();
  setRandomOne<BishopHeadKingRelative>();
  setRandomOne<KnightCheckY>();
  setRandomOne<KnightHead>();
  setRandomOne<RookPromoteDefense>();
  setRandomOne<PawnDropPawnStand>();
  setRandomOne<PawnDropPawnStandX>();
  setRandomOne<PawnDropPawnStandY>();
  setRandomOne<King25Effect2>();
  setRandomOne<King25EffectY2>();
  setRandomOne<KnightHeadOppPiecePawnOnStand>();
  setRandomOne<KingXBothBlocked>();
  setRandomOne<KingXBothBlockedY>();
  setRandomOne<KingRookBishop>();
  setRandomOne<PromotedMinorPiecesY>();
  setRandomOne<King25EffectSupported>();
  setRandomOne<King25EffectSupportedY>();
  setRandomOne<NonPawnAttackedKingRelative>();
  setRandomOne<NonPawnAttackedPtype>();
  setRandomOne<PtypeCount>();
  setRandomOne<KingXBlocked3>();
  setRandomOne<KingXBlocked3Y>();
  setRandomOne<PtypeCountXY>();
  setRandomOne<PtypeCountXYAttack>();
  setRandomOne<LanceEffectPieceKingRelative>();
  setRandomOne<KingMobility>();
  setRandomOne<KingMobilitySum>();
  setRandomOne<MajorCheckWithCapture>();
  setRandomOne<RookSilverKnight>();
  setRandomOne<BishopSilverKnight>();
}
#endif


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
