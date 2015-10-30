/* piecePair.cc
 */
#include "osl/eval/piecePair.h"

osl::eval::ml::PiecePair::IndexTable osl::eval::ml::PiecePair::plain_table;
osl::CArray<osl::eval::ml::PiecePair::IndexTable, 10> osl::eval::ml::PiecePair::x_table, 
osl::eval::ml::PiecePair::y_table;
const osl::CArray<osl::Offset, 12> osl::eval::ml::PiecePair::offsets = {
  // positive offset [0,5]
  DirectionPlayerTraits<UUL, BLACK>::offset(), 
  DirectionPlayerTraits<UL, BLACK>::offset(), 
  DirectionPlayerTraits<L, BLACK>::offset(), 
  DirectionPlayerTraits<DL, BLACK>::offset(), 
  DirectionPlayerTraits<UUR, WHITE>::offset(), 
  DirectionPlayerTraits<D, BLACK>::offset(), 
  // negative offset [6,11]
  DirectionPlayerTraits<UUL, WHITE>::offset(), 
  DirectionPlayerTraits<DR, BLACK>::offset(), 
  DirectionPlayerTraits<R, BLACK>::offset(), 
  DirectionPlayerTraits<UR, BLACK>::offset(), 
  DirectionPlayerTraits<UUR, BLACK>::offset(), 
  DirectionPlayerTraits<U, BLACK>::offset(), 
    };

namespace osl
{
  namespace eval
  {
    namespace ml
    {
  namespace ppair
  {
    CArray<int, 0x200> offset_index;
    PiecePair::IndexTable& plain_table = PiecePair::plain_table;
    CArray<PiecePair::IndexTable, 10>& x_table = PiecePair::x_table;
    CArray<PiecePair::IndexTable, 10>& y_table = PiecePair::y_table;

    void makeOffsetIndex()
    {
      offset_index.fill(-1);
      for (size_t i=0; i<PiecePair::offsets.size(); ++i) {
	offset_index[PiecePair::offsets[i].index()] = i;
      }
    }
    inline int inv(int offset_id)
    {
      assert(offset_id >= 0 && offset_id < 12);
      return (offset_id + 6) % 12;
    }
    inline int swaplr(int offset_id)
    {
      assert(offset_id >= 0 && offset_id < 12);
      if (offset_id == 11) 
	return 11;
      return 10 - offset_id;
    }
    inline int swapud(int offset_id)
    {
      assert(offset_id >= 0 && offset_id < 12);
      return swaplr(inv(offset_id));
    }    
    int pindex(Player player, Ptype ptype) { return PiecePair::IndexTable::pindex(player, ptype); }
    void makeTable()
    {
      int index = 0;
      for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
	for (int ip1=ip0; ip1<=PTYPE_MAX; ++ip1) {
	  const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	  // same player
	  {
#ifndef NDEBUG
	    const int pi0 = pindex(BLACK, p0), pi1 = pindex(BLACK, p1);
	    assert(plain_table[0][pi0][pi1] == 0);
#endif
	    ++index;
	    plain_table.fillSame(index,  0, p0, p1); // UUL
	    plain_table.fillSame(index, 10, p0, p1); // UUR
	    if (p0 != p1) {
	      ++index;
	      plain_table.fillSame(index,  0, p1, p0); // UUL
	      plain_table.fillSame(index, 10, p1, p0); // UUR
	    }

	    ++index;
	    plain_table.fillSame(index, 1, p0, p1); // UL
	    plain_table.fillSame(index, 9, p0, p1); // UR
	    if (p0 != p1) {
	      ++index;
	      plain_table.fillSame(index, 1, p1, p0); // UR
	      plain_table.fillSame(index, 9, p1, p0); // UL
	    }
	    
	    ++index;
	    plain_table.fillSame(index, 2, p0, p1); // L
	    plain_table.fillSame(index, 8, p0, p1); // R
	    if (p0 != p1) { // use the same index as L
	      plain_table.fillSame(index, 2, p1, p0); // L
	      plain_table.fillSame(index, 8, p1, p0); // R
	    }
	    
	    ++index;
	    plain_table.fillSame(index, 11, p0, p1); // U
	    if (p0 != p1) {
	      plain_table.fillSame(index, 11, p1, p0); // U
	    }
	  }
	  // different player
	  {
	    // UUL, UUR
	    ++index;
	    plain_table.fillDiffer(index, 0, p0, p1); // UUL
	    plain_table.fillDiffer(index, 10, p0, p1); // UUR
	    ++index;
	    plain_table.fillDiffer(index, inv(0), p0, p1); // UUL^-1
	    plain_table.fillDiffer(index, inv(10), p0, p1); // UUR^-1

	    // UL, UR
	    ++index;
	    plain_table.fillDiffer(index, 1, p0, p1); // UL
	    plain_table.fillDiffer(index, 9, p0, p1); // UR
	    ++index;
	    // DR, DL
	    plain_table.fillDiffer(index, inv(1), p0, p1); // DR
	    plain_table.fillDiffer(index, inv(9), p0, p1); // DL

            // LR
	    ++index;
	    plain_table.fillDiffer(index, 2, p0, p1); // L
	    plain_table.fillDiffer(index, inv(2), p0, p1); // R, use the same index as L

	    // UD
	    ++index;
	    plain_table.fillDiffer(index, 11, p0, p1); // U

	    ++index;
	    plain_table.fillDiffer(index, inv(11), p0, p1); // D
	  }
	}
      }  
      assert(index+1 == PiecePair::plain_table_size);
    }
    void makeTableX()
    {
      // currently only for same player
      int index = 0;
      // make leftside
      for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
	for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
	  const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	  const int pi0 = pindex(BLACK, p0),  pi1 = pindex(BLACK, p1);
	  for (int x=1; x<=5; ++x) {
	    // (UUL, DDL), (UL, DL)
	    for (int d=0; d<2; ++d) {
	      ++index;
	      x_table[x][d][pi0][pi1] = index;
	      x_table[x][swapud(d)][pi0][pi1] = index;
	    }
	    // L
	    ++index;
	    x_table[x][2][pi0][pi1] = index;
	    // U, D
	    ++index;
	    x_table[x][11][pi0][pi1] = index;
	    x_table[x][inv(11)][pi1][pi0] = index;
	    ++index;
	    x_table[x][5][pi0][pi1] = index;
	    x_table[x][inv(5)][pi1][pi0] = index;
	  } // x
	}
      }
      // make rightside
      for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
	for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
	  const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	  const int pi0 = pindex(BLACK, p0),  pi1 = pindex(BLACK, p1);
	  for (int x=2; x<=5; ++x) {
	    // (UUL, DDL), (UL, DL) => (DDR, UUR), (DR, UR)
	    for (int d=0; d<2; ++d) {
	      x_table[x-1][inv(d)][pi1][pi0]         = x_table[x][d][pi0][pi1];
	      x_table[x-1][inv(swapud(d))][pi1][pi0] = x_table[x][swapud(d)][pi0][pi1];
	    }
	    // L => R
	    x_table[x-1][swaplr(2)][pi1][pi0] = x_table[x][2][pi0][pi1];
	  }
	  // flip col 5
	  for (size_t d=0; d<PiecePair::offsets.size(); ++d) {
	    if (swaplr(d) == (int)d || x_table[5][d][pi0][pi1] == 0)
	      continue;
	    x_table[5][swaplr(d)][pi0][pi1] = x_table[5][d][pi0][pi1];
	  }
	}
      }
      // mirror to [6,9]
      for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
	for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
	  const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	  const int pi0 = pindex(BLACK, p0), pi1 = pindex(BLACK, p1);
	  for (int x=6; x<=9; ++x) {
	    for (size_t d=0; d<PiecePair::offsets.size(); ++d) {
	      x_table[x][d][pi0][pi1] = x_table[10-x][swaplr(d)][pi0][pi1];
	    }
	  } // x
	}
      }
      // make white player
      for (int x=1; x<=9; ++x) {
	for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
	  for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
	    const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	    const int pi0 = pindex(BLACK, p0), pi1 = pindex(BLACK, p1);
	    const int pi0w = pindex(WHITE, p0), pi1w = pindex(WHITE, p1);
	    for (size_t d=0; d<PiecePair::offsets.size(); ++d) {
	      assert(x_table[x][d][pi0][pi1]);
	      x_table[10-x][inv(d)][pi0w][pi1w] = -x_table[x][d][pi0][pi1];
	    }
	  }
	}
      }
      assert(PiecePair::x_table_size == index+1);
      for (int x=1; x<=9; ++x)
	x_table[x].amplify(PiecePair::plain_table_size);
    }
    int wrap9(int y) 
    {
      return (y-1)%9 + 1;
    }
    void makeTableY()
    {
      // only for same player
      int index = 0;
      // for upside direction
      for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
	for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
	  const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	  const int pi0 = pindex(BLACK, p0), pi1 = pindex(BLACK, p1);
	  // same player
	  for (int y=1; y<=9; ++y) {
	    for (int d=0; d<2; ++d) { // (UUL, UUR), (UL, UR)
	      ++index;
	      y_table[y][d][pi0][pi1] = index;
	      y_table[y][swaplr(d)][pi0][pi1] = index;
	    }
	    // (L, R)
	    ++index;
	    y_table[y][2][pi0][pi1] = index;
	    y_table[y][2][pi1][pi0] = index;
	    y_table[y][swaplr(2)][pi0][pi1] = index;
	    y_table[y][swaplr(2)][pi1][pi0] = index;
	    // U
	    ++index;
	    y_table[y][11][pi0][pi1] = index;
	  } // y
	}
      }
      // flip for downside direction
      for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
	for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
	  const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	  const int pi0 = pindex(BLACK, p0), pi1 = pindex(BLACK, p1);
	  for (int y=1; y<=9; ++y) {
	    // (UUL, UUR), 
	    y_table[wrap9(y+2)][inv(0)][pi1][pi0]         = y_table[y][0][pi0][pi1];
	    y_table[wrap9(y+2)][inv(swaplr(0))][pi1][pi0] = y_table[y][swaplr(0)][pi0][pi1];
	    // (UL, UR)
	    y_table[wrap9(y+1)][inv(1)][pi1][pi0]         = y_table[y][1][pi0][pi1];
	    y_table[wrap9(y+1)][inv(swaplr(1))][pi1][pi0] = y_table[y][swaplr(1)][pi0][pi1];
	    // U
	    y_table[wrap9(y+1)][inv(11)][pi1][pi0] = y_table[y][11][pi0][pi1];
	  } // y
	}
      }
      // make white player
      for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
	for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
	  const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	  const int pi0 = pindex(BLACK, p0), pi1 = pindex(BLACK, p1);
	  const int pi0w = pindex(WHITE, p0), pi1w = pindex(WHITE, p1);
	  for (int y=1; y<=9; ++y) {
	    for (size_t d=0; d<PiecePair::offsets.size(); ++d) {
	      y_table[10-y][inv(d)][pi0w][pi1w] = -y_table[y][d][pi0][pi1];
	    }
	  }
	}
      }
      assert(PiecePair::y_table_size == index+1);
      for (int y=1; y<=9; ++y)
	y_table[y].amplify(PiecePair::plain_table_size+PiecePair::x_table_size);
    }

    CArray3d<int, PTYPEO_SIZE, 12, PTYPEO_SIZE> x_values[10], y_values[10]; // plain_values は xに折込
  }
      using namespace ppair;
    }
  }
}

/* ------------------------------------------------------------------------- */
osl::eval::ml::
PiecePair::IndexTable::IndexTable()
{
  fill(0);
}

void osl::eval::ml::
PiecePair::IndexTable::amplify(int base)
{
  for (size_t d=0; d<offsets.size(); ++d) {
    for (int ip0=0; ip0<PTYPEO_SIZE; ++ip0) {
      for (int ip1=0; ip1<PTYPEO_SIZE; ++ip1) {
	signed short& target = (*this)[d][ip0][ip1];
	if (target > 0) {
	  target += base;
	}
	else if (target < 0)
	{
	  target -= base;
	}
      }
    }
  }
}
void osl::eval::ml::
PiecePair::IndexTable::fillBW(int index, int dir, Ptype p0, Ptype p1) 
{
  const int pi0 = pindex(BLACK, p0), pi1 = pindex(BLACK, p1);
  const int pi0w = pindex(WHITE, p0), pi1w = pindex(WHITE, p1);

  (*this)[dir][pi0][pi1] = index; // normal
  (*this)[inv(dir)][pi0w][pi1w] = -index; // white
}
void osl::eval::ml::
PiecePair::IndexTable::fillSame(int index, int dir, Ptype p0, Ptype p1) 
{
  fillBW(index, dir, p0, p1);
  fillBW(index, inv(dir), p1, p0); // swapped order
}
void osl::eval::ml::
PiecePair::IndexTable::fillDiffer(int index, int dir, Ptype p0, Ptype p1) 
{
  const int pi0 = pindex(BLACK, p0), pi1 = pindex(BLACK, p1);
  const int pi0w = pindex(WHITE, p0), pi1w = pindex(WHITE, p1);

  (*this)[inv(dir)][pi0][pi1w] = index;
  (*this)[dir][pi1w][pi0] = index; // swapped piece
  (*this)[inv(dir)][pi1][pi0w] = -index; // swapped player
  (*this)[dir][pi0w][pi1] = -index; // swapped player, swapped piece
}
/* ------------------------------------------------------------------------- */


void osl::eval::ml::
PiecePair::init()
{
  static bool initialized = false;
  if (initialized)
    return;
  initialized = true;
  makeOffsetIndex();
  makeTable();
  makeTableX();
  makeTableY();
}

void osl::eval::ml::
PiecePair::compile(const Weights& weights)
{
  for (int i=1; i<=9; ++i) {
    x_values[i].fill(0);
    y_values[i].fill(0);
  }
  for (size_t d=0; d<offsets.size(); ++d) {
    for (int ip0=0; ip0<PTYPEO_SIZE; ++ip0) {
      for (int ip1=0; ip1<PTYPEO_SIZE; ++ip1) {
	int plain = 0;
	if (plain_table[d][ip0][ip1] > 0)
	  plain = weights.value(plain_table[d][ip0][ip1]);
	else if (plain_table[d][ip0][ip1] < 0)
	  plain = -weights.value(-plain_table[d][ip0][ip1]);
	for (int i=1; i<=9; ++i) {
	  x_values[i][ip0][d][ip1] = plain;
	  if (x_table[i][d][ip0][ip1] > 0)
	    x_values[i][ip0][d][ip1] += weights.value(x_table[i][d][ip0][ip1]);
	  else if (x_table[i][d][ip0][ip1] < 0)
	    x_values[i][ip0][d][ip1] += -weights.value(-x_table[i][d][ip0][ip1]);
	  if (y_table[i][d][ip0][ip1] > 0)
	    y_values[i][ip0][d][ip1] = weights.value(y_table[i][d][ip0][ip1]);
	  else if (y_table[i][d][ip0][ip1] < 0)
	    y_values[i][ip0][d][ip1] = -weights.value(-y_table[i][d][ip0][ip1]);
	}
      }
    }
  }  
}

void osl::eval::ml::
PiecePair::sanitize(Weights& values)
{
  values.setValue(0,0);
  for (int x=1; x<=9; ++x) {
    for (int y=1; y<=9; ++y) {
      const Square pos1(x,y);
      for (size_t i=0; i<offsets.size(); ++i) {
	const Square pos0 = pos1+offsets[i];
	if (! pos0.isOnBoard())
	  continue;
	for (int p=PTYPE_PIECE_MIN; p<=PTYPE_MAX; ++p) {
	  const Ptype ptype = static_cast<Ptype>(p);
	  assert(isPiece(ptype));
	  index_t idx = index(i, pos0, newPtypeO(BLACK, ptype), pos1, newPtypeO(WHITE, ptype));
	  values.setValue(abs(idx[0]), 0);
	  idx = index(i, pos0, newPtypeO(WHITE, ptype), pos1, newPtypeO(BLACK, ptype));
	  values.setValue(abs(idx[0]), 0);
	}
      }
    }
  }
}

osl::eval::ml::PiecePair::index_t osl::eval::ml::
PiecePair::index(int offset_id, Square pos0, PtypeO p0, Square pos1, PtypeO p1)
{
  assert(pos0 != pos1);
  assert(! pos0.isPieceStand() && ! pos1.isPieceStand());

  assert(pos0 - pos1 == offsets[offset_id]);
  index_t ret = {
    plain_table[offset_id][ptypeOIndex(p0)][ptypeOIndex(p1)],
    x_table[pos0.x()][offset_id][ptypeOIndex(p0)][ptypeOIndex(p1)],
    y_table[pos0.y()][offset_id][ptypeOIndex(p0)][ptypeOIndex(p1)],
  };
  assert(abs(ret[0]) < plain_table_size);
  assert(abs(ret[1]) < plain_table_size + x_table_size);
  assert(abs(ret[2]) < plain_table_size + x_table_size + y_table_size);
  assert(ret[1] == 0 || abs(ret[1]) > plain_table_size);
  assert(ret[2] == 0 || abs(ret[2]) > plain_table_size + x_table_size);
  return ret;
}

osl::eval::ml::PiecePair::index_t osl::eval::ml::
PiecePair::index(int offset_id, Piece p, Piece q)
{
  assert(p.isPiece());
  assert(q.isPiece());
  assert(p != q);
  assert(p.isOnBoard() && q.isOnBoard());
  return index(offset_id, p.square(), p.ptypeO(), q.square(), q.ptypeO());
}

int osl::eval::ml::
PiecePair::eval(const NumEffectState& state, const Weights& values)
{
  int ret = 0;
  for (int i=0; i<Piece::SIZE; i++) {
    const Piece p = state.pieceOf(i);
    ret += pieceValueDouble(state, p, values);
  }
  return ret/2;
}

int osl::eval::ml::
PiecePair::evalWithUpdate(const NumEffectState& state, Move moved, int last_value, const Weights& values)
{
  if (moved.isPass())
    return last_value;
  
  int ret = last_value;
  const Square from = moved.from();
  const Square to = moved.to();  

  // adjust from
  if (! from.isPieceStand()) {
    for (size_t i=0; i<offsets.size(); ++i) {
      const Square target = from + offsets[i];
      const Piece p = state.pieceAt(target);
      if (! p.isPiece() || p.square() == to)
	continue;
      assert(!target.isPieceStand());
      ret -= value(i, p, from, moved.oldPtypeO(), values);      
    }  
  }
  
  // adjust to
  if (! moved.isCapture()) 
  {  
    for (size_t i=0; i<offsets.size(); ++i) {
      const Square target = to + offsets[i];
      const Piece p = state.pieceAt(target);
      if (! p.isPiece())
	continue;
      assert(!target.isPieceStand());
      ret += value(i, p, to, moved.ptypeO(), values);      
    }  
    return ret;
  }

  // adjust with capture
  for (size_t i=0; i<offsets.size(); ++i) {
    const Square target = to + offsets[i];
    const Piece p = state.pieceAt(target);
    if (! p.isPiece())
      continue;
    assert(!target.isPieceStand());
    ret += value(i, p, to, moved.ptypeO(), values);      
    if (p.square() == to)
      continue;
    ret -= value(i, p, to, moved.capturePtypeO(), values);      
  }
  const Offset diff = to - from;
  int capture_i = offset_index[diff.index()];
  if (capture_i >= 0)
    ret -= value(capture_i, to, moved.capturePtypeO(), from, moved.oldPtypeO(), values);

  return ret;
}

int osl::eval::ml::
PiecePair::valueCompiled(int offset_id, Square pos0, PtypeO p0, Square pos1, PtypeO p1)
{
  assert(pos0 != pos1);
  assert(! pos0.isPieceStand() && ! pos1.isPieceStand());
  assert(pos0 - pos1 == offsets[offset_id]);

  return x_values[pos0.x()][ptypeOIndex(p0)][offset_id][ptypeOIndex(p1)]
    + y_values[pos0.y()][ptypeOIndex(p0)][offset_id][ptypeOIndex(p1)];
}

template <int Direction, int Offset>
inline int osl::eval::ml::
PiecePair::sum12One(const Piece *base_ptr,const int *xbase,const int *ybase)
{
  const Piece p = *(base_ptr-Offset);
  PtypeO p1=p.ptypeO();
  return 
      *(xbase+(&x_values[0][0][1][0]-&x_values[0][0][0][0])*Direction+p1)
    + *(ybase+(&y_values[0][0][1][0]-&y_values[0][0][0][0])*Direction+p1);
}
inline int osl::eval::ml::
PiecePair::sum12(NumEffectState const& state,Square base,PtypeO ptypeO)
{
  const int *xbase= &x_values[base.x()][ptypeOIndex(ptypeO)][0][ptypeOIndex((PtypeO)0)];
  const int *ybase= &y_values[base.y()][ptypeOIndex(ptypeO)][0][ptypeOIndex((PtypeO)0)];
  const Piece* base_ptr= state.getPiecePtr(base);
  return 
    sum12One<4,18>(base_ptr,xbase,ybase)+
    + sum12One<3,17>(base_ptr,xbase,ybase)
    + sum12One<2,16>(base_ptr,xbase,ybase)
    + sum12One<1,15>(base_ptr,xbase,ybase)
    + sum12One<0,14>(base_ptr,xbase,ybase)
    + sum12One<5,1>(base_ptr,xbase,ybase)
    + sum12One<11,-1>(base_ptr,xbase,ybase)
    + sum12One<6,-14>(base_ptr,xbase,ybase)
    + sum12One<7,-15>(base_ptr,xbase,ybase)
    + sum12One<8,-16>(base_ptr,xbase,ybase)
    + sum12One<9,-17>(base_ptr,xbase,ybase)
    + sum12One<10,-18>(base_ptr,xbase,ybase);
}

template<int Direction, int Offset>
inline int osl::eval::ml::
PiecePair::adjust12One(const Piece *base_ptr,const int *xbase1,const int *ybase1,const int *xbase2,const int *ybase2)
{
  const Piece p = *(base_ptr-Offset);
  PtypeO p1=p.ptypeO();
  return 
      *(xbase1+(&x_values[0][0][1][0]-&x_values[0][0][0][0])*Direction+p1)
    + *(ybase1+(&y_values[0][0][1][0]-&y_values[0][0][0][0])*Direction+p1)
    - *(xbase2+(&x_values[0][0][1][0]-&x_values[0][0][0][0])*Direction+p1)
    - *(ybase2+(&y_values[0][0][1][0]-&y_values[0][0][0][0])*Direction+p1);
}

inline int osl::eval::ml::
PiecePair::adjust12(NumEffectState const& state,Square base,PtypeO pos,PtypeO neg)
{
  const int *xbase1= &x_values[base.x()][ptypeOIndex(pos)][0][ptypeOIndex((PtypeO)0)];
  const int *xbase2= &x_values[base.x()][ptypeOIndex(neg)][0][ptypeOIndex((PtypeO)0)];
  const int *ybase1= &y_values[base.y()][ptypeOIndex(pos)][0][ptypeOIndex((PtypeO)0)];
  const int *ybase2= &y_values[base.y()][ptypeOIndex(neg)][0][ptypeOIndex((PtypeO)0)];
  const Piece* base_ptr= state.getPiecePtr(base);
  return
    adjust12One<4,18>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<3,17>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<2,16>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<1,15>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<0,14>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<5,1>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<11,-1>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<6,-14>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<7,-15>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<8,-16>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<9,-17>(base_ptr,xbase1,ybase1,xbase2,ybase2)
    + adjust12One<10,-18>(base_ptr,xbase1,ybase1,xbase2,ybase2);
}

int osl::eval::ml::
PiecePair::evalWithUpdateCompiled(const NumEffectState& state, Move moved, int last_value)
{
  int ret = last_value;
  const Square from = moved.from();
  const Square to = moved.to();  

  // adjust from
  if (from.isPieceStand()) {
    ret+=sum12(state,to,moved.ptypeO());
    return ret;
  }
  else{
    ret-=sum12(state,from,moved.oldPtypeO());
    // adjust to
    if (! moved.isCapture()) {
      ret+=sum12(state,to,moved.ptypeO());
      const Offset diff = to-from;
      int capture_i = offset_index[diff.index()];
      if (capture_i >= 0){
	PtypeO ptypeO=moved.ptypeO();
	const int *xbase= &x_values[to.x()][ptypeOIndex(ptypeO)][0][ptypeOIndex((PtypeO)0)];
	const int *ybase= &y_values[to.y()][ptypeOIndex(ptypeO)][0][ptypeOIndex((PtypeO)0)];
	PtypeO p1=moved.oldPtypeO();
	ret+=
	  *(xbase+(&x_values[0][0][1][0]-&x_values[0][0][0][0])*capture_i+p1)
	  + *(ybase+(&y_values[0][0][1][0]-&y_values[0][0][0][0])*capture_i+p1);
      }
      return ret;
    }
    else{
      // adjust with capture
      ret+=adjust12(state,to,moved.ptypeO(),moved.capturePtypeO());
      const Offset diff = to-from;
      int capture_i = offset_index[diff.index()];
      if (capture_i >= 0){
	Square base=to;
	PtypeO ptypeO1=moved.ptypeO();
	PtypeO ptypeO2=moved.capturePtypeO();
	const int *xbase1= &x_values[base.x()][ptypeOIndex(ptypeO1)][0][ptypeOIndex((PtypeO)0)];
	const int *xbase2= &x_values[base.x()][ptypeOIndex(ptypeO2)][0][ptypeOIndex((PtypeO)0)];
	const int *ybase1= &y_values[base.y()][ptypeOIndex(ptypeO1)][0][ptypeOIndex((PtypeO)0)];
	const int *ybase2= &y_values[base.y()][ptypeOIndex(ptypeO2)][0][ptypeOIndex((PtypeO)0)];
	PtypeO p1=moved.oldPtypeO();
	ret+=
	  *(xbase1+(&x_values[0][0][1][0]-&x_values[0][0][0][0])*capture_i+p1)
	  + *(ybase1+(&y_values[0][0][1][0]-&y_values[0][0][0][0])*capture_i+p1)
	  - *(xbase2+(&x_values[0][0][1][0]-&x_values[0][0][0][0])*capture_i+p1)
	  - *(ybase2+(&y_values[0][0][1][0]-&y_values[0][0][0][0])*capture_i+p1);
      }
      return ret;
    }
  }
}

int osl::eval::ml::
PiecePair::pieceValueDouble(const NumEffectState& state, Piece p, const Weights& values)
{
  if (! p.isOnBoard())
    return 0;
  int ret = 0;
  for (size_t i=0; i<offsets.size(); ++i) {
    const Square target = p.square() + offsets[i];
    const Piece q = state.pieceAt(target);
    if (! q.isPiece()|| p == q)
      continue;
    assert(!target.isPieceStand()); 
    assert(p.isOnBoard() && q.isOnBoard());
    int v = value(i, q, p, values);
    ret += v;
  }
  return ret;
}

int osl::eval::ml::
PiecePair::pieceValue(const NumEffectState& state, Piece p, const Weights& values)
{
  return pieceValueDouble(state, p, values)/2;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
