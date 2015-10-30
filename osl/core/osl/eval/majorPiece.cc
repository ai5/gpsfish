#include "osl/eval/majorPiece.h"
#include "osl/mobility/rookMobility.h"
#include <algorithm>
using osl::MultiInt;

template <bool Opening, osl::Ptype MajorBasic>
osl::CArray<int, 18>
osl::eval::ml::MajorY<Opening, MajorBasic>::table;

template <bool Opening, osl::Ptype MajorBasic>
void osl::eval::ml::
MajorY<Opening, MajorBasic>::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
}


template <bool Opening>
int osl::eval::ml::RookPawn<Opening>::weight;

template <bool Opening>
void osl::eval::ml::
RookPawn<Opening>::setUp(const Weights &weights)
{
  weight = weights.value(0);
}

template <bool Opening>
int osl::eval::ml::RookPawn<Opening>::eval(const NumEffectState &state)
{
  int result = 0;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() && !piece.square().canPromote(piece.owner()) &&
	!state.isPawnMaskSet(piece.owner(), piece.square().x()))
    {
      if (piece.owner() == BLACK)
	result += weight;
      else
	result -= weight;
    }
  }
  return result;
}


osl::CArray<MultiInt, 180> osl::eval::ml::RookPawnY::table;
osl::CArray<MultiInt, 1620> osl::eval::ml::
RookPawnY::y_attack_table;
osl::CArray<MultiInt, 1620> osl::eval::ml::
RookPawnY::y_defense_table;

void osl::eval::ml::
RookPawnYX::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s) 
    {  
      RookPawnY::y_attack_table[i][s] = weights.value(i + ONE_DIM * 2 * s);
      RookPawnY::y_defense_table[i][s] = weights.value(i + ONE_DIM * 2 * s + ONE_DIM);
    }
  }
}


void osl::eval::ml::
RookPawnY::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

MultiInt osl::eval::ml::
RookPawnY::eval(const NumEffectState &state,
		const CArray2d<int, 2, 9> &pawns)
{
  MultiInt result;
  const CArray<Square,2> kings = {
    state.kingSquare<BLACK>(),
    state.kingSquare<WHITE>(),
  };
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard())
    {
      if (piece.owner() == BLACK)
      {
	const int pawn_y =
	  pawns[BLACK][piece.square().x() - 1];
	result +=
	  table[index(piece, pawn_y)] +
	  y_attack_table[indexY(kings[WHITE], piece, pawn_y)] +
	  y_defense_table[indexY(kings[BLACK], piece, pawn_y)];
      }
      else
      {
	int y = pawns[WHITE][piece.square().x() - 1];
	if (y != 0)
	  y = 10 - y;
	result -=
	  table[index(piece, y)] +
	  y_attack_table[indexY(kings[BLACK], piece, y)] +
	  y_defense_table[indexY(kings[WHITE], piece, y)];
      }
    }
  }
  return result;
}


MultiInt osl::eval::ml::AllMajor::weight;

void osl::eval::ml::
AllMajor::setUp(const Weights &weights,int stage)
{
  weight[stage] = weights.value(0);
}


template <bool Opening>
osl::CArray<int, 32> osl::eval::ml::MajorGoldSilverAttacked<Opening>::table;

template <bool Opening>
void osl::eval::ml::
MajorGoldSilverAttacked<Opening>::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
}

template <bool Opening>
int osl::eval::ml::
MajorGoldSilverAttacked<Opening>::index(
  const NumEffectState &state, Piece piece)
{
  return piece.ptype() + (state.turn() == piece.owner() ? 0 : PTYPE_SIZE);
}

template <bool Opening>
template <osl::Ptype PTYPE>
int osl::eval::ml::
MajorGoldSilverAttacked<Opening>::evalOne(const NumEffectState &state)
{
  int result = 0;
  for (int i = PtypeTraits<PTYPE>::indexMin;
       i < PtypeTraits<PTYPE>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() &&
	state.hasEffectAt(alt(piece.owner()), piece.square()))
    {
      const int weight = table[index(state, piece)];
      if (piece.owner() == BLACK)
	result += weight;
      else
	result -= weight;
    }
  }
  return result;
}

template <bool Opening>
int osl::eval::ml::
MajorGoldSilverAttacked<Opening>::eval(const NumEffectState &state)
{
  int result = 0;
  result += evalOne<ROOK>(state);
  result += evalOne<BISHOP>(state);
  result += evalOne<GOLD>(state);
  result += evalOne<SILVER>(state);

  return result;
}

osl::CArray<MultiInt, 612> osl::eval::ml::RookEffectBase::attack_table;
osl::CArray<MultiInt, 612> osl::eval::ml::RookEffectBase::defense_table;
osl::CArray<MultiInt, 32> osl::eval::ml::RookEffectBase::piece_table;

osl::CArray<MultiInt, 23104> osl::eval::ml::RookEffectBase::attack_u;
osl::CArray<MultiInt, 23104> osl::eval::ml::RookEffectBase::attack_d;
osl::CArray<MultiInt, 23104> osl::eval::ml::RookEffectBase::attack_r;
osl::CArray<MultiInt, 23104> osl::eval::ml::RookEffectBase::attack_l;
osl::CArray<MultiInt, 23104> osl::eval::ml::RookEffectBase::defense_u;
osl::CArray<MultiInt, 23104> osl::eval::ml::RookEffectBase::defense_d;
osl::CArray<MultiInt, 23104> osl::eval::ml::RookEffectBase::defense_r;
osl::CArray<MultiInt, 23104> osl::eval::ml::RookEffectBase::defense_l;
osl::CArray<MultiInt, 722> osl::eval::ml::RookEffectBase::attack_nospace;
osl::CArray<MultiInt, 722> osl::eval::ml::RookEffectBase::defense_nospace;


template<osl::Player P>
inline
MultiInt osl::eval::ml::RookEffectBase::evalOne(
 const NumEffectState& state,
 Square rook,
 Square myKing,
 Square opKing,
 Square up,
 Square dp,
 Square rp,
 Square lp,
 bool isP)
{
  MultiInt result;
  PtypeO uPtypeO=state.pieceAt(up).ptypeO();
  PtypeO dPtypeO=state.pieceAt(dp).ptypeO();
  PtypeO rPtypeO=state.pieceAt(rp).ptypeO();
  PtypeO lPtypeO=state.pieceAt(lp).ptypeO();
  if(P==WHITE){
    uPtypeO=(PtypeO)(static_cast<int>(uPtypeO)^(~15));
    dPtypeO=(PtypeO)(static_cast<int>(dPtypeO)^(~15));
    rPtypeO=(PtypeO)(static_cast<int>(rPtypeO)^(~15));
    lPtypeO=(PtypeO)(static_cast<int>(lPtypeO)^(~15));
    up=up.rotate180EdgeOK();
    dp=dp.rotate180EdgeOK();
    rp=rp.rotate180EdgeOK();
    lp=lp.rotate180EdgeOK();
    rook=rook.rotate180();
    myKing=myKing.rotate180();
    opKing=opKing.rotate180();
  }
  assert((myKing.y()-dp.y())<(myKing.y()-up.y()));
  assert((myKing.x()-lp.x())<(myKing.x()-rp.x()));
  result+=attack_u[index1(opKing,up,uPtypeO,isP)]+
    attack_d[index1(opKing,dp,dPtypeO,isP)]+
    attack_l[index1(opKing,lp,lPtypeO,isP)]+
    attack_r[index1(opKing,rp,rPtypeO,isP)]+
    defense_u[index1(myKing,up,uPtypeO,isP)]+
    defense_d[index1(myKing,dp,dPtypeO,isP)]+
    defense_l[index1(myKing,lp,lPtypeO,isP)]+
    defense_r[index1(myKing,rp,rPtypeO,isP)]+
    attack_nospace[index2(opKing,rook,isP)]+
    defense_nospace[index2(myKing,rook,isP)];
  return result;
}

MultiInt osl::eval::ml::
RookEffectBase::eval(const NumEffectState &state)
{
  const CArray<Square,2> kings = { 
    state.kingSquare(BLACK), 
    state.kingSquare(WHITE), 
  };
  MultiInt result;
  for (int i = PtypeTraits<ROOK>::indexMin; i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (! p.isOnBoard()) continue;
    const Square pos=p.square();
    Square up=state.mobilityOf(U,i);
    Square dp=state.mobilityOf(D,i);
    Square lp=state.mobilityOf(L,i);
    Square rp=state.mobilityOf(R,i);
    const bool isP=p.isPromoted();
    if(p.owner()==BLACK)
      result+=evalOne<BLACK>(state,pos,kings[0],kings[1],up,dp,rp,lp,isP);
    else
      result-=evalOne<WHITE>(state,pos,kings[1],kings[0],dp,up,lp,rp,isP);
  }
  return result;
}

void osl::eval::ml::RookEffect::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    attack_table[i][stage] = weights.value(i);
    defense_table[i][stage] = weights.value(i + ONE_DIM);
  }
}

void osl::eval::ml::RookEffectPiece::setUp(const Weights &weights)
{
  for (size_t i = 0; i < 32; ++i)
  {
    for (int s=0; s<NStages; ++s)
      RookEffectBase::piece_table[i][s] = weights.value(i + 32*s);
  }
}

void osl::eval::ml::
RookEffectPieceKingRelative::setUp(const Weights &weights)
{
  CArray<MultiInt, 19584> piece_attack_table;
  CArray<MultiInt, 19584> piece_defense_table;
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
    {
      piece_attack_table[i][s] = weights.value(i + ONE_DIM*2*s);
      piece_defense_table[i][s] = weights.value(i + ONE_DIM*2*s + ONE_DIM);
    }
  }
  for(int isP=0;isP<2;isP++)
    for(int y_diff=-9;y_diff<=9;y_diff++)
      for(int x_diff= -9;x_diff<=9;x_diff++){
	int i2=index2(x_diff,y_diff,isP);
	if(abs(x_diff)<9 && abs(y_diff)<9){
	  attack_nospace[i2]= 
	    -(attack_table[index(abs(x_diff),y_diff,true,isP)]+
	      attack_table[index(abs(x_diff),y_diff,false,isP)]);
	  defense_nospace[i2]= 
	    -(defense_table[index(abs(x_diff),y_diff,true,isP)]+
	      defense_table[index(abs(x_diff),y_diff,false,isP)]);
	}
	for(int ptypeo= PTYPEO_MIN;ptypeo<=PTYPEO_MAX;ptypeo++){
	  if(getPtype((PtypeO)ptypeo)==(int)PTYPE_EMPTY) continue;
	  int i1=index1(x_diff,y_diff,(PtypeO)ptypeo,isP);
	  int indexPieceH,indexPieceV;
	  int table_ptypeo=ptypeo;
	  if(getPtype((PtypeO)ptypeo)==PTYPE_EDGE) table_ptypeo=PTYPEO_EDGE;
	  if(getPtype((PtypeO)ptypeo)==PTYPE_EDGE || abs(x_diff)==9 || abs(y_diff)==9){
	    indexPieceH= 0+0+(PTYPEO_EDGE-PTYPEO_MIN)*17*9+4896+isP*9792;
	    indexPieceV= 0+0+(PTYPEO_EDGE-PTYPEO_MIN)*17*9+ isP*9792;
	  }
	  else{
	    indexPieceH= index0(abs(x_diff),-y_diff,(PtypeO)ptypeo,true,isP);
	    indexPieceV= index0(abs(x_diff),-y_diff,(PtypeO)ptypeo,false,isP);
	  }
	  attack_u[i1]=piece_attack_table[indexPieceV]+piece_table[table_ptypeo-PTYPEO_MIN];
	  defense_u[i1]=piece_defense_table[indexPieceV];
	  attack_d[i1]=piece_attack_table[indexPieceV]+piece_table[table_ptypeo-PTYPEO_MIN];
	  defense_d[i1]=piece_defense_table[indexPieceV];
	  if(abs(x_diff)<=8){
	    for(int y_diff_1=y_diff+1;y_diff_1<=8;y_diff_1++){
	      int i=index(abs(x_diff),y_diff_1,false,isP);
	      attack_u[i1]+=attack_table[i];
	      defense_u[i1]+=defense_table[i];
	    }
	    for(int y_diff_1=std::max(-8,y_diff);y_diff_1<=8;y_diff_1++){
	      int i=index(abs(x_diff),y_diff_1,false,isP);
	      attack_d[i1]-=attack_table[i];
	      defense_d[i1]-=defense_table[i];
	    }
	  }
	  attack_l[i1]=piece_attack_table[indexPieceH]+piece_table[table_ptypeo-PTYPEO_MIN];
	  defense_l[i1]=piece_defense_table[indexPieceH];
	  attack_r[i1]=piece_attack_table[indexPieceH]+piece_table[table_ptypeo-PTYPEO_MIN];
	  defense_r[i1]=piece_defense_table[indexPieceH];
	  if(abs(y_diff)<=8){
	    for(int x_diff_1=x_diff+1;x_diff_1<=8;x_diff_1++){
	      int i=index(abs(x_diff_1),y_diff,true,isP);
	      attack_r[i1]+=attack_table[i];
	      defense_r[i1]+=defense_table[i];
	    }
	    for(int x_diff_1=std::max(-8,x_diff);x_diff_1<=8;x_diff_1++){
	      int i=index(abs(x_diff_1),y_diff,true,isP);
	      attack_l[i1]-=attack_table[i];
	      defense_l[i1]-=defense_table[i];
	    }
	  }
	}
      }
}



osl::CArray<MultiInt, 256> osl::eval::ml::RookPromoteDefense::promote_defense_table;
osl::CArray<MultiInt, 144> osl::eval::ml::RookPromoteDefense::promote_defense_rook_table;

void osl::eval::ml::RookPromoteDefense::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      promote_defense_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::RookPromoteDefenseRookH::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      RookPromoteDefense::promote_defense_rook_table[i][s] =
	weights.value(i + ONE_DIM*s);
  }
}

MultiInt osl::eval::ml::
RookPromoteDefense::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i){
    const Piece rook = state.pieceOf(i);
    if(rook.isOnBoardNotPromoted()){
      if(rook.owner()==BLACK){
	Square rookPos=rook.square();
	if(rookPos.y()>=4){
	  Square pos=state.mobilityOf(U,i);
	  const Piece attacked = state.pieceAt(pos);
	  if (attacked.canMoveOn<BLACK>()){
	    const NumBitmapEffect effect = state.effectSetAt(pos);
	    if (effect.countEffect(WHITE) == 1){
	      PieceMask mask = effect & state.piecesOnBoard(WHITE);
	      const Piece effect_piece = state.pieceOf(mask.takeOneBit());
	      const int index = attacked.ptype() * 16 + effect_piece.ptype();
	      result += promote_defense_table[index];
	      if (effect_piece.ptype() == ROOK &&
		  effect_piece.square().x() != rookPos.x())
	      {
		result +=
		  promote_defense_rook_table[
		    attacked.ptype() * 9 +
		    mobility::RookMobility::countHorizontalAll<BLACK>(state,
								      rook)];
	      }
	    }
	  }
	}
      }
      else{
	Square rookPos=rook.square();
	if(rookPos.y()<=6){
	  Square pos=state.mobilityOf(D,i);
	  const Piece attacked = state.pieceAt(pos);
	  if (attacked.canMoveOn<WHITE>()){
	    const NumBitmapEffect effect = state.effectSetAt(pos);
	    if (effect.countEffect(BLACK) == 1){
	      PieceMask mask = effect & state.piecesOnBoard(BLACK);
	      const Piece effect_piece = state.pieceOf(mask.takeOneBit());
	      const int index = attacked.ptype() * 16 + effect_piece.ptype();
	      result -= promote_defense_table[index];
	      if (effect_piece.ptype() == ROOK &&
		  effect_piece.square().x() != rookPos.x())
	      {
		result -=
		  promote_defense_rook_table[
		    attacked.ptype() * 9 +
		    mobility::RookMobility::countHorizontalAll<WHITE>(state,
								      rook)];
	      }
	    }
	  }
	}
      }
    }
  }
  return result;
}



osl::CArray<MultiInt, 612> osl::eval::ml::BishopEffectBase::attack_table;
osl::CArray<MultiInt, 612> osl::eval::ml::BishopEffectBase::defense_table;
osl::CArray<MultiInt, 32> osl::eval::ml::BishopEffectBase::piece_table;
osl::CArray<MultiInt, 23104> osl::eval::ml::BishopEffectBase::attack_ul;
osl::CArray<MultiInt, 23104> osl::eval::ml::BishopEffectBase::attack_ur;
osl::CArray<MultiInt, 23104> osl::eval::ml::BishopEffectBase::attack_dl;
osl::CArray<MultiInt, 23104> osl::eval::ml::BishopEffectBase::attack_dr;
osl::CArray<MultiInt, 23104> osl::eval::ml::BishopEffectBase::defense_ul;
osl::CArray<MultiInt, 23104> osl::eval::ml::BishopEffectBase::defense_ur;
osl::CArray<MultiInt, 23104> osl::eval::ml::BishopEffectBase::defense_dl;
osl::CArray<MultiInt, 23104> osl::eval::ml::BishopEffectBase::defense_dr;
osl::CArray<MultiInt, 722> osl::eval::ml::BishopEffectBase::attack_nospace;
osl::CArray<MultiInt, 722> osl::eval::ml::BishopEffectBase::defense_nospace;


template<osl::Player P>
inline
MultiInt osl::eval::ml::BishopEffectBase::evalOne(
 const NumEffectState& state,
 Square bishop,
 Square myKing,
 Square opKing,
 Square ulp,
 Square urp,
 Square dlp,
 Square drp,
 bool isP)
{
  MultiInt result;
  PtypeO ulPtypeO=state.pieceAt(ulp).ptypeO();
  PtypeO urPtypeO=state.pieceAt(urp).ptypeO();
  PtypeO dlPtypeO=state.pieceAt(dlp).ptypeO();
  PtypeO drPtypeO=state.pieceAt(drp).ptypeO();
  if(P==WHITE){
    ulPtypeO=(PtypeO)(static_cast<int>(ulPtypeO)^(~15));
    urPtypeO=(PtypeO)(static_cast<int>(urPtypeO)^(~15));
    dlPtypeO=(PtypeO)(static_cast<int>(dlPtypeO)^(~15));
    drPtypeO=(PtypeO)(static_cast<int>(drPtypeO)^(~15));
    ulp=ulp.rotate180EdgeOK();
    urp=urp.rotate180EdgeOK();
    dlp=dlp.rotate180EdgeOK();
    drp=drp.rotate180EdgeOK();
    bishop=bishop.rotate180();
    myKing=myKing.rotate180();
    opKing=opKing.rotate180();
  }
  result+=attack_ul[index1(opKing,ulp,ulPtypeO,isP)]+
    attack_ur[index1(opKing,urp,urPtypeO,isP)]+
    attack_dl[index1(opKing,dlp,dlPtypeO,isP)]+
    attack_dr[index1(opKing,drp,drPtypeO,isP)]+
    defense_ul[index1(myKing,ulp,ulPtypeO,isP)]+
    defense_ur[index1(myKing,urp,urPtypeO,isP)]+
    defense_dl[index1(myKing,dlp,dlPtypeO,isP)]+
    defense_dr[index1(myKing,drp,drPtypeO,isP)]+
    attack_nospace[index2(opKing,bishop,isP)]+
    defense_nospace[index2(myKing,bishop,isP)];
  return result;
}

MultiInt osl::eval::ml::
BishopEffectBase::eval(const NumEffectState &state)
{
  const CArray<Square,2> kings = {{ 
    state.kingSquare(BLACK), 
    state.kingSquare(WHITE), 
  }};

  MultiInt result;
  for (int i = PtypeTraits<BISHOP>::indexMin; i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
    {
      const Piece p = state.pieceOf(i);
      if (! p.isOnBoard()) continue;
      const Square pos=p.square();
      Square ulp=state.mobilityOf(UL,i);
      Square urp=state.mobilityOf(UR,i);
      Square dlp=state.mobilityOf(DL,i);
      Square drp=state.mobilityOf(DR,i);
      const bool isP=p.isPromoted();
      if(p.owner()==BLACK)
	result+=evalOne<BLACK>(state,pos,kings[0],kings[1],ulp,urp,dlp,drp,isP);
      else
	result-=evalOne<WHITE>(state,pos,kings[1],kings[0],drp,dlp,urp,ulp,isP);
    }
  return result;
}

void osl::eval::ml::BishopEffect::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    attack_table[i][stage] = weights.value(i);
    defense_table[i][stage] = weights.value(i + ONE_DIM);
  }
}

void osl::eval::ml::BishopEffectPiece::setUp(const Weights &weights)
{
  for (size_t i = 0; i < 32; ++i)
  {
    for (int s=0; s<NStages; ++s)
      BishopEffectBase::piece_table[i][s] = weights.value(i + 32*s);
  }
}


void osl::eval::ml::
BishopEffectPieceKingRelative::setUp(const Weights &weights)
{
  CArray<MultiInt, 19584> piece_attack_table;
  CArray<MultiInt, 19584> piece_defense_table;
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s) 
    {
      piece_attack_table[i][s] = weights.value(i + ONE_DIM * 2 * s);
      piece_defense_table[i][s] = weights.value(i + ONE_DIM * 2 * s + ONE_DIM);
    }
  }
  for(int isP=0;isP<2;isP++)
    for(int y_diff=-9;y_diff<=9;y_diff++)
      for(int x_diff= -9;x_diff<=9;x_diff++){
	int i2=index2(x_diff,y_diff,isP);
	if(abs(x_diff)<9 && abs(y_diff)<9){
	  attack_nospace[i2]= 
	    -(attack_table[index(x_diff,y_diff,true,isP)]+
	      attack_table[index(x_diff,y_diff,false,isP)]);
	  defense_nospace[i2]= 
	    -(defense_table[index(x_diff,y_diff,true,isP)]+
	      defense_table[index(x_diff,y_diff,false,isP)]);
	}
	for(int ptypeo= PTYPEO_MIN;ptypeo<=PTYPEO_MAX;ptypeo++){
	  if(getPtype((PtypeO)ptypeo)==(int)PTYPE_EMPTY) continue;
	  int i1=index1(x_diff,y_diff,(PtypeO)ptypeo,isP);
	  int indexPieceUR,indexPieceUL;
	  int table_ptypeo=ptypeo;
	  if(getPtype((PtypeO)ptypeo)==PTYPE_EDGE) table_ptypeo=PTYPEO_EDGE;
	  if(getPtype((PtypeO)ptypeo)==PTYPE_EDGE || abs(x_diff)==9 || abs(y_diff)==9){
	    indexPieceUR= 0+0+(PTYPEO_EDGE-PTYPEO_MIN)*17*9+4896+isP*9792;
	    indexPieceUL= 0+0+(PTYPEO_EDGE-PTYPEO_MIN)*17*9+ isP*9792;
	  }
	  else{
	    indexPieceUR= index0(x_diff,y_diff,(PtypeO)ptypeo,true,isP);
	    indexPieceUL= index0(x_diff,y_diff,(PtypeO)ptypeo,false,isP);
	  }
	  attack_ul[i1]=piece_attack_table[indexPieceUL]+piece_table[table_ptypeo-PTYPEO_MIN];
	  defense_ul[i1]=piece_defense_table[indexPieceUL];
	  attack_dr[i1]=piece_attack_table[indexPieceUL]+piece_table[table_ptypeo-PTYPEO_MIN];
	  defense_dr[i1]=piece_defense_table[indexPieceUL];
	  {
	    int y_diff_1=y_diff+1, x_diff_1=x_diff-1;
	    for(;y_diff_1<=8 && x_diff_1>=-8;y_diff_1++,x_diff_1--){
	      if(std::abs(x_diff_1)<=8 && std::abs(y_diff_1)<=8){
		int i=index(x_diff_1,y_diff_1,false,isP);
		attack_ul[i1]+=attack_table[i];
		defense_ul[i1]+=defense_table[i];
	      }
	    }
	  }
	  {
	    int y_diff_1=y_diff, x_diff_1=x_diff;
	    for(;y_diff_1<=8 && x_diff_1>=-8;y_diff_1++,x_diff_1--){
	      if(std::abs(x_diff_1)<=8 && std::abs(y_diff_1)<=8){
		int i=index(x_diff_1,y_diff_1,false,isP);
		attack_dr[i1]-=attack_table[i];
		defense_dr[i1]-=defense_table[i];
	      }
	    }
	  }
	  attack_ur[i1]=piece_attack_table[indexPieceUR]+piece_table[table_ptypeo-PTYPEO_MIN];
	  defense_ur[i1]=piece_defense_table[indexPieceUR];
	  attack_dl[i1]=piece_attack_table[indexPieceUR]+piece_table[table_ptypeo-PTYPEO_MIN];
	  defense_dl[i1]=piece_defense_table[indexPieceUR];
	  {
	    int y_diff_1=y_diff+1, x_diff_1=x_diff+1;
	    for(;y_diff_1<=8 && x_diff_1<=8;y_diff_1++,x_diff_1++){
	      if(std::abs(x_diff_1)<=8 && std::abs(y_diff_1)<=8){
		int i=index(x_diff_1,y_diff_1,true,isP);
		attack_ur[i1]+=attack_table[i];
		defense_ur[i1]+=defense_table[i];
	      }
	    }
	  }
	  {
	    int y_diff_1=y_diff, x_diff_1=x_diff;
	    for(;y_diff_1<=8 && x_diff_1<=8;y_diff_1++,x_diff_1++){
	      if(std::abs(x_diff_1)<=8 && std::abs(y_diff_1)<=8){
		int i=index(x_diff_1,y_diff_1,true,isP);
		attack_dl[i1]-=attack_table[i];
		defense_dl[i1]-=defense_table[i];
	      }
	    }
	  }
	}
      }
}

osl::CArray<MultiInt, 32> osl::eval::ml::BishopHead::table;
osl::CArray<MultiInt, 4896> osl::eval::ml::BishopHead::king_table;
osl::CArray<MultiInt, 160> osl::eval::ml::BishopHead::x_table;

void osl::eval::ml::BishopHead::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
BishopHeadKingRelative::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      BishopHead::king_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for(int x_diff=0;x_diff<=8;x_diff++)
    for(int y_diff=-8;y_diff<=8;y_diff++){
      for(int i=0;i<32;i++)
	BishopHead::king_table[(i*9+x_diff)*17+y_diff+8]+=BishopHead::table[i];
    }
  const PtypeO PTYPEO_EMPTY_R=newPtypeO(WHITE,PTYPE_EMPTY);
  const PtypeO PTYPEO_EDGE_R=newPtypeO(BLACK,PTYPE_EDGE);
  for(int x_diff=0;x_diff<=8;x_diff++)
    for(int y_diff=-8;y_diff<=8;y_diff++){
      BishopHead::king_table[(ptypeOIndex(PTYPEO_EMPTY_R)*9+x_diff)*17+y_diff+8]=
	BishopHead::king_table[(ptypeOIndex(PTYPEO_EMPTY)*9+x_diff)*17+y_diff+8];
      BishopHead::king_table[(ptypeOIndex(PTYPEO_EDGE_R)*9+x_diff)*17+y_diff+8]=
	BishopHead::king_table[(ptypeOIndex(PTYPEO_EDGE)*9+x_diff)*17+y_diff+8];
    }
}

void osl::eval::ml::BishopHeadX::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      BishopHead::x_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

MultiInt osl::eval::ml::
BishopHead::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i){
    const Piece p = state.pieceOf(i);
    if (p.isOnBoardNotPromoted()){
      const Square pos=p.square();
      if (p.owner() == BLACK){
	if(pos.y()>=2){
	  const Square up = pos+DirectionPlayerTraits<U,BLACK>::offset();
	  if (!state.hasEffectAt(BLACK, up)){
	    const Square king = state.kingSquare(BLACK);
	    const PtypeO ptypeo = state.pieceAt(up).ptypeO();
	    const int index_k = indexK(BLACK, ptypeo,
				       std::abs(pos.x() - king.x()),
				       pos.y() - king.y());
	    result += king_table[index_k];
	    result += x_table[indexX<BLACK>(ptypeo, pos.x())];
	  }
	}
      }
      else if(pos.y()<=8) {
	const Square up = pos+DirectionPlayerTraits<U,WHITE>::offset();
	if (!state.hasEffectAt(WHITE, up)){
	  const Square king = state.kingSquare(WHITE);
	  const PtypeO ptypeo = state.pieceAt(up).ptypeO();
	  const int index_k = indexK(WHITE, ptypeo,
				     std::abs(pos.x() - king.x()),
				     pos.y() - king.y());
	  result -= king_table[index_k];
	  result -= x_table[indexX<WHITE>(ptypeo, pos.x())];
	}
      }
    }
  }
  return result;
}


osl::CArray<MultiInt, 374544> osl::eval::ml::KingRookBishop::table;

void osl::eval::ml::KingRookBishop::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template<osl::Player P>
MultiInt osl::eval::ml::
KingRookBishop::evalOne(const NumEffectState &state)
{
  const Square king=state.kingSquare(P);
  MultiInt result;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece rook = state.pieceOf(i);
    if (!rook.isOnBoard())
    {
      continue;
    }
    for (int j = PtypeTraits<BISHOP>::indexMin;
	 j < PtypeTraits<BISHOP>::indexLimit;
	 ++j)
    {
      const Piece bishop = state.pieceOf(j);
      if (!bishop.isOnBoard())
      {
	continue;
      }
      result += table[index<P>(king, rook, bishop)];
    }
  }
  return result;
}

MultiInt osl::eval::ml::
KingRookBishop::eval(const NumEffectState &state)
{
  return evalOne<BLACK>(state)-evalOne<WHITE>(state);
}


osl::CArray<MultiInt, 9> osl::eval::ml::NumPiecesBetweenBishopAndKing::self_table;
osl::CArray<MultiInt, 9> osl::eval::ml::NumPiecesBetweenBishopAndKing::opp_table;
osl::CArray<MultiInt, 9> osl::eval::ml::NumPiecesBetweenBishopAndKing::all_table;

void osl::eval::ml::
NumPiecesBetweenBishopAndKingSelf::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      NumPiecesBetweenBishopAndKing::self_table[i][s] =
	weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
NumPiecesBetweenBishopAndKingOpp::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      NumPiecesBetweenBishopAndKing::opp_table[i][s] =
	weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
NumPiecesBetweenBishopAndKingAll::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      NumPiecesBetweenBishopAndKing::all_table[i][s] =
	weights.value(i + ONE_DIM*s);
  }
}

osl::MultiInt osl::eval::ml::
NumPiecesBetweenBishopAndKing::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece bishop = state.pieceOf(i);
    if (!bishop.isOnBoard())
    {
      continue;
    }
    int self, opp, all;
    countBetween(state,
		 state.kingSquare(alt(bishop.owner())),
		 bishop, self, opp, all);
    if (bishop.owner() == BLACK)
    {
      result += (self_table[self] + opp_table[opp] + all_table[all]);
    }
    else
    {
      result -= (self_table[self] + opp_table[opp] + all_table[all]);
    }
  }
  return result;
}

void osl::eval::ml::
NumPiecesBetweenBishopAndKing::countBetween(
  const NumEffectState &state, Square king, Piece bishop,
  int &self_count, int &opp_count, int &total_count)
{
  assert(bishop.isOnBoard());
  if ((king.x() + king.y() != bishop.square().x() + bishop.square().y()) &&
	(king.x() - king.y() != bishop.square().x() - bishop.square().y()))
  {
    self_count = opp_count = total_count = 8;
    return;
  }
  Direction dir;
  assert(king.x() != bishop.square().x());
  assert(king.y() != bishop.square().y());
  if (king.x() < bishop.square().x())
  {
    if (king.y() < bishop.square().y())
    {
	dir = UR;
    }
    else
    {
	dir = DR;
    }
  }
  else
  {
    if (king.y() < bishop.square().y())
    {
	dir = UL;
    }
    else
    {
	dir = DL;
    }
  }
  const Player player = bishop.owner();
  const Direction move_dir = (player == BLACK ? dir : inverse(dir));
  self_count = opp_count = total_count = 0;
  for (Square pos = state.mobilityOf(dir, bishop.number());
	 pos != king; pos = Board_Table.nextSquare(player, pos, move_dir))
  {
    assert(pos.isOnBoard());
    const Piece piece = state.pieceAt(pos);
    if (!piece.isEmpty())
    {
      ++total_count;
      if (piece.owner() == player)
	++self_count;
      else
	++opp_count;
    }
  }
}


osl::CArray<MultiInt, 64>
osl::eval::ml::BishopBishopPiece::table;

void osl::eval::ml::
BishopBishopPiece::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::MultiInt osl::eval::ml::
BishopBishopPiece::eval(const NumEffectState &state)
{
  MultiInt result;
  const Piece bishop1 = state.pieceOf(PtypeTraits<BISHOP>::indexMin);
  const Piece bishop2 = state.pieceOf(PtypeTraits<BISHOP>::indexMin + 1);
  if (!bishop1.isOnBoard() || !bishop2.isOnBoard() ||
      bishop1.owner() == bishop2.owner())
    return result;
  if (bishop1.square().x() + bishop1.square().y() !=
      bishop2.square().x() + bishop2.square().y() &&
      bishop1.square().x() - bishop1.square().y() !=
      bishop2.square().x() - bishop2.square().y())
    return result;

  if (state.hasEffectByPtype<BISHOP>(bishop2.owner(), bishop1.square()))
    return result;

  Direction dir;
  if (bishop1.square().x() < bishop2.square().x())
  {
    if (bishop1.square().y() < bishop2.square().y())
    {
	dir = UR;
    }
    else
    {
	dir = DR;
    }
  }
  else
  {
    if (bishop1.square().y() < bishop2.square().y())
    {
	dir = UL;
    }
    else
    {
	dir = DL;
    }
  }
  Square p1 = state.mobilityOf(inverse(dir), bishop1.number());
  Square p2 = state.mobilityOf(dir, bishop2.number());
  if (p1 == p2)
  {
    const Piece p = state.pieceAt(p1);
    const bool black_with_support =
      state.hasEffectAt<BLACK>(bishop1.owner() == BLACK ?
			       bishop1.square() : bishop2.square());
    const bool white_with_support =
      state.hasEffectAt<WHITE>(bishop1.owner() == WHITE ?
			       bishop1.square() : bishop2.square());
    if (p.owner() == BLACK)
    {
      result += table[index(p.ptype(), black_with_support,
			    white_with_support)];
    }
    else
    {
      result -= table[index(p.ptype(), white_with_support,
			    black_with_support)];
    }
  }
  return result;
}

osl::CArray<MultiInt, 800>
osl::eval::ml::RookRook::table;

void osl::eval::ml::
RookRook::setUp(const Weights &weights)
{
  CArray<MultiInt, 800> orig_table;
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      orig_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for (int owner = 0; owner < 2; ++owner)
  {
    const bool same_player = (owner == 0);
    for (int y1 = 0; y1 < 10; ++y1)
    {
      for (int y2 = 0; y2 < 10; ++y2)
      {
	for (int promoted1 = 0; promoted1 < 2; ++promoted1)
	{
	  for (int promoted2 = 0; promoted2 < 2; ++promoted2)
	  {
	    if (same_player)
	    {
	      int y1p = y1;
	      int y2p = y2;
	      int promoted1p = promoted1;
	      int promoted2p = promoted2;
	      if (y1 > y2 || (y1 == y2 && !promoted1 && promoted2))
	      {
		std::swap(y1p, y2p);
		std::swap(promoted1p, promoted2p);
	      }
	      table[index(same_player, promoted1, promoted2,
			  y1, y2)] =
		orig_table[index(same_player, promoted1p, promoted2p,
				 y1p, y2p)];
	    }
	    else
	    {
	      if (y1 + y2 > 10 || y1 == 0 ||
		  (y1 + y2 == 10 && promoted1))
	      {
		const int idx = index(same_player, promoted1, promoted2,
				      y1, y2);
		table[idx] = orig_table[idx];
	      }
	      else
	      {
		table[index(same_player, promoted1, promoted2,
			    y1, y2)] =
		  -orig_table[index(same_player, promoted2, promoted1,
				    (10 - y2) % 10, (10 - y1) % 10)];
	      }
	    }
	  }
	}
      }
    }
  }
}

osl::MultiInt osl::eval::ml::
RookRook::eval(const NumEffectState &state)
{
  MultiInt result;
  Piece rook1 = state.pieceOf(PtypeTraits<ROOK>::indexMin);
  Piece rook2 = state.pieceOf(PtypeTraits<ROOK>::indexMin + 1);
  if (rook1.owner() == rook2.owner())
  {
    if (rook1.owner() == BLACK)
    {
      result += table[index<true, BLACK>(rook1, rook2)];
    }
    else
    {
      result -= table[index<true, WHITE>(rook1, rook2)];
    }
  }
  else
  {
    if (rook1.owner() != BLACK)
    {
      std::swap(rook1, rook2);
    }
    result += table[index<false, BLACK>(rook1, rook2)];
  }
  return result;
}


osl::CArray<MultiInt, 128>
osl::eval::ml::RookRookPiece::table;

void osl::eval::ml::
RookRookPiece::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::MultiInt osl::eval::ml::
RookRookPiece::eval(const NumEffectState &state)
{
  MultiInt result;
  const Piece rook1 = state.pieceOf(PtypeTraits<ROOK>::indexMin);
  const Piece rook2 = state.pieceOf(PtypeTraits<ROOK>::indexMin + 1);
  if (!rook1.isOnBoard() || !rook2.isOnBoard() ||
      rook1.owner() == rook2.owner())
    return result;

  if (state.hasEffectByPtype<ROOK>(rook2.owner(), rook1.square()))
    return result;

  Direction dir;
  bool vertical = false;
  if (rook1.square().x() == rook2.square().x())
  {
    vertical = true;
    if (rook1.square().y() < rook2.square().y())
    {
	dir = D;
    }
    else
    {
	dir = U;
    }
  }
  else if (rook1.square().y() == rook2.square().y())
  {
    if (rook1.square().x() < rook2.square().x())
    {
	dir = L;
    }
    else
    {
	dir = R;
    }
  }
  else
  {
    return result;
  }

  Square p1 = state.mobilityOf(dir, rook1.number());
  Square p2 = state.mobilityOf(inverse(dir), rook2.number());
  assert(p1.isOnBoard() && p2.isOnBoard());
  if (p1 == p2)
  {
    const Piece p = state.pieceAt(p1);
    const bool black_with_support =
      state.hasEffectAt<BLACK>(rook1.owner() == BLACK ?
			       rook1.square() : rook2.square());
    const bool white_with_support =
      state.hasEffectAt<WHITE>(rook1.owner() == WHITE ?
			       rook1.square() : rook2.square());
    if (p.owner() == BLACK)
    {
      result += table[index(p.ptype(), black_with_support,
			    white_with_support, vertical)];
    }
    else
    {
      result -= table[index(p.ptype(), white_with_support,
			    black_with_support, vertical)];
    }
  }
  return result;
}


osl::CArray<MultiInt, 32>
osl::eval::ml::BishopStandFile5::table;

void osl::eval::ml::
BishopStandFile5::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::MultiInt osl::eval::ml::
BishopStandFile5::eval(const NumEffectState &state)
{
  MultiInt result;
  if (state.hasPieceOnStand<BISHOP>(BLACK))
  {
    result += table[ptypeOIndex(state.pieceAt(Square(5, 3)).ptypeO())];
  }
  if (state.hasPieceOnStand<BISHOP>(WHITE))
  {
    PtypeO ptypeO = state.pieceAt(Square(5, 7)).ptypeO();
    ptypeO = altIfPiece(ptypeO);
    result -= table[ptypeOIndex(ptypeO)];
  }
  return result;
}



osl::CArray<MultiInt, osl::eval::ml::MajorCheckWithCapture::ONE_DIM>
osl::eval::ml::MajorCheckWithCapture::table;

void osl::eval::ml::
MajorCheckWithCapture::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <osl::Player Owner>
osl::MultiInt osl::eval::ml::
MajorCheckWithCapture::addOne(const NumEffectState &state)
{
  const Square king = state.kingSquare(Owner);
  PieceMask pieces = state.effectedMask(alt(Owner));
  pieces &= state.piecesOnBoard(Owner);
  pieces &= ~state.effectedMask(Owner);
  MultiInt sum;
  while (pieces.any()) {
    const Piece p = state.pieceOf(pieces.takeOneBit());
    const Square sq = p.square();
    if (state.hasLongEffectAt<ROOK>(alt(Owner), sq)
	&& state.hasEffectIf(newPtypeO(BLACK,ROOK), sq, king)) {
      if (Owner == BLACK)
	sum += table[index(p.ptype(), true, sq.canPromote(alt(Owner)))];
      else
	sum -= table[index(p.ptype(), true, sq.canPromote(alt(Owner)))];
    }
    if (state.hasLongEffectAt<BISHOP>(alt(Owner), sq)
	&& state.hasEffectIf(newPtypeO(BLACK,BISHOP), sq, king)) {
      if (Owner == BLACK)
	sum += table[index(p.ptype(), false, sq.canPromote(alt(Owner)))];
      else
	sum -= table[index(p.ptype(), false, sq.canPromote(alt(Owner)))];
    }
  }
  return sum;
}

osl::MultiInt osl::eval::ml::
MajorCheckWithCapture::eval(const NumEffectState &state)
{
  return addOne<BLACK>(state) + addOne<WHITE>(state);
}


osl::CArray<MultiInt, osl::eval::ml::RookSilverKnight::ONE_DIM>
osl::eval::ml::RookSilverKnight::table;

void osl::eval::ml::
RookSilverKnight::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::MultiInt osl::eval::ml::
RookSilverKnight::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece rook = state.pieceOf(i);
    if (!rook.isOnBoard())
    {
      continue;
    }
    for (int i = PtypeTraits<SILVER>::indexMin;
	 i < PtypeTraits<SILVER>::indexLimit;
	 ++i)
    {
      const Piece silver = state.pieceOf(i);
      if (!silver.isOnBoard() || silver.isPromoted() ||
          silver.owner() != rook.owner())
      {
        continue;
      }
      for (int i = PtypeTraits<KNIGHT>::indexMin;
           i < PtypeTraits<KNIGHT>::indexLimit;
           ++i)
      {
        const Piece knight = state.pieceOf(i);
        if (!knight.isOnBoard() || knight.isPromoted() ||
            knight.owner() != rook.owner())
        {
          continue;
        }

        if (rook.owner() == BLACK)
        {
          if (rook.square().x() > 5)
          {
            result += table[index(9 - rook.square().x(), rook.square().y() - 1,
				  9 - silver.square().x(), silver.square().y() - 1,
				  9 - knight.square().x(), knight.square().y() - 1)];
          }
          else
          {
            result += table[index(rook.square().x() - 1, rook.square().y() - 1,
				  silver.square().x() - 1, silver.square().y() - 1,
				  knight.square().x() - 1, knight.square().y() - 1)];
          }
        }
        else
        {
          if (rook.square().x() >= 5)
          {
	    result -= table[index(9 - rook.square().x(), 9 - rook.square().y(),
				  9 - silver.square().x(), 9 - silver.square().y(),
				  9 - knight.square().x(), 9 - knight.square().y())];
          }
          else
          {
	    result -= table[index(rook.square().x() - 1, 9 - rook.square().y(),
				  silver.square().x() - 1, 9 - silver.square().y(),
				  knight.square().x() - 1, 9 - knight.square().y())];
          }
        }
      }
    }
  }
  return result;
}


osl::CArray<MultiInt, osl::eval::ml::BishopSilverKnight::ONE_DIM>
osl::eval::ml::BishopSilverKnight::table;

void osl::eval::ml::
BishopSilverKnight::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::MultiInt osl::eval::ml::
BishopSilverKnight::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece bishop = state.pieceOf(i);
    if (!bishop.isOnBoard())
    {
      continue;
    }
    for (int i = PtypeTraits<SILVER>::indexMin;
	 i < PtypeTraits<SILVER>::indexLimit;
	 ++i)
    {
      const Piece silver = state.pieceOf(i);
      if (!silver.isOnBoard() || silver.isPromoted() ||
          silver.owner() != bishop.owner())
      {
        continue;
      }
      for (int i = PtypeTraits<KNIGHT>::indexMin;
           i < PtypeTraits<KNIGHT>::indexLimit;
           ++i)
      {
        const Piece knight = state.pieceOf(i);
        if (!knight.isOnBoard() || knight.isPromoted() ||
            knight.owner() != bishop.owner())
        {
          continue;
        }

        if (bishop.owner() == BLACK)
        {
          if (bishop.square().x() > 5)
          {
            result += table[index(9 - bishop.square().x(), bishop.square().y() - 1,
				  9 - silver.square().x(), silver.square().y() - 1,
				  9 - knight.square().x(), knight.square().y() - 1)];
          }
          else
          {
            result += table[index(bishop.square().x() - 1, bishop.square().y() - 1,
				  silver.square().x() - 1, silver.square().y() - 1,
				  knight.square().x() - 1, knight.square().y() - 1)];
          }
        }
        else
        {
          if (bishop.square().x() >= 5)
          {
	    result -= table[index(9 - bishop.square().x(), 9 - bishop.square().y(),
				  9 - silver.square().x(), 9 - silver.square().y(),
				  9 - knight.square().x(), 9 - knight.square().y())];
          }
          else
          {
	    result -= table[index(bishop.square().x() - 1, 9 - bishop.square().y(),
				  silver.square().x() - 1, 9 - silver.square().y(),
				  knight.square().x() - 1, 9 - knight.square().y())];
          }
        }
      }
    }
  }
  return result;
}


osl::CArray<MultiInt, osl::eval::ml::AttackMajorsInBase::ONE_DIM>
osl::eval::ml::AttackMajorsInBase::table;

void osl::eval::ml::
AttackMajorsInBase::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i) {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
    if (i > 0)
      table[i] += table[0];
  }
}

template <osl::Player P>
void osl::eval::ml::
AttackMajorsInBase::addOne(const NumEffectState &state, Piece rook, MultiInt& result)
{
  Square sq = rook.square();
  if (state.hasEffectAt(alt(P), sq)
      || sq.squareForBlack(P).y() < 8)
    return;
  typedef std::pair<Offset,Square> pair_t;
  const CArray<pair_t, 7> bishop_attack =
    {{
	pair_t(Offset::make<P,U>(), sq.neighbor<P, UL>()),
	pair_t(Offset::make<P,U>(), sq.neighbor<P, UR>()),
	pair_t(Offset::make<P,L>(), sq.neighbor<P, UL>()),
	pair_t(Offset::make<P,R>(), sq.neighbor<P, UR>()),
	pair_t(Offset::make<P,D>(), sq.neighbor<P, UL>()),
	pair_t(Offset::make<P,D>(), sq.neighbor<P, UR>()),
	pair_t(Offset::make<P,U>(), sq.neighbor<P, U>()),
      }};
  const bool has_gold = state.hasPieceOnStand(alt(P), GOLD);
  const bool rook_support = state.hasEffectAt(P, sq);
  for (pair_t pair: bishop_attack) {
    const Square attack_square = pair.second;
    if (! state[attack_square].isEmpty()
	|| state.countEffect(P, attack_square) > 1)
      continue;
    const Square bishop_square = attack_square + pair.first;
    Piece p = state[bishop_square];
    if (! p.isPlayerPtype(P,BISHOP)
	|| state.hasEffectAt(alt(P), bishop_square))
      continue;
    int a = state.countEffect(alt(P), attack_square) + has_gold;
    if (a <= state.countEffect(P, attack_square))
      continue;
    const int i = index(state.findCheapAttack(P, attack_square).ptype(),
			state.findCheapAttack(alt(P), attack_square).ptype(),
			has_gold, rook_support,
			state.hasEffectNotBy(P, rook, bishop_square));
    if (P == BLACK)
      result += table[i];
    else
      result -= table[i];
  }
}

osl::MultiInt osl::eval::ml::
AttackMajorsInBase::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i=0; i<state.nthLimit<ROOK>(); ++i) {
    const Piece rook = state.nth<ROOK>(i);
    if (! rook.isOnBoard() || rook.isPromoted())
      continue;
    Player P = rook.owner();
    if (P == BLACK)
      addOne<BLACK>(state, rook, result);    
    else
      addOne<WHITE>(state, rook, result);    
  }
  return result;
}


namespace osl
{
  namespace eval
  {
    namespace ml
    {
      template class MajorY<true, ROOK>;
      template class MajorY<false, ROOK>;
      template class MajorY<true, BISHOP>;
      template class MajorY<false, BISHOP>;
      template class RookPawn<true>;
      template class RookPawn<false>;
      template class MajorGoldSilverAttacked<false>;
      template MultiInt KingRookBishop::evalOne<BLACK>(const NumEffectState &state);
      template MultiInt KingRookBishop::evalOne<WHITE>(const NumEffectState &state);
    }
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
