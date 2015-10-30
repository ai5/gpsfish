/* ntesukiMove.cc
 */
#include "osl/ntesuki/ntesukiMove.h"
#include "osl/ntesuki/ntesukiMove.tcc"
#include <iostream>


/* Constructors and destructors
 */
osl::ntesuki::NtesukiMove::
NtesukiMove()
  : move(Move::INVALID()), flags(0), order(-1)  {};

osl::ntesuki::NtesukiMove::
NtesukiMove(osl::Move m)
  : move(m), flags(0), order(-1),
    h_a_proof(1), h_a_disproof(1),
    h_d_proof(1), h_d_disproof(1)
{};

osl::ntesuki::NtesukiMove::
NtesukiMove(osl::Move m, Flags f)
  : move(m), flags(f), order(-1),
    h_a_proof(1), h_a_disproof(1),
    h_d_proof(1), h_d_disproof(1)
{};

osl::ntesuki::NtesukiMove::
NtesukiMove(const NtesukiMove& m)
  : move(m.move), flags(m.flags), order(m.order),
    h_a_proof(1), h_a_disproof(1),
    h_d_proof(1), h_d_disproof(1)
{};

osl::ntesuki::NtesukiMove
osl::ntesuki::NtesukiMove::
operator=(const NtesukiMove& m)
{
  if (this == &m) return *this;

  move = m.move;
  flags = m.flags;
  order = m.order;
  h_a_proof = m.h_a_proof;
  h_a_disproof = m.h_a_disproof;
  h_d_proof = m.h_d_proof;
  h_d_disproof = m.h_d_disproof;

  return *this;
};

osl::ntesuki::NtesukiMove::
~NtesukiMove()
{
}

/* static methods */
osl::ntesuki::NtesukiMove osl::ntesuki::NtesukiMove::
INVALID() { return NtesukiMove(Move::INVALID()); }

/* about the state of the node */
void osl::ntesuki::NtesukiMove::
setCheck()
{
  ntesuki_assert(!(flags & CHECK_FLAG));
  flags |= CHECK_FLAG;
};

bool osl::ntesuki::NtesukiMove::
isCheck() const
{
  return flags & CHECK_FLAG;
}

void osl::ntesuki::NtesukiMove::
setOrder(int o)
{
  ntesuki_assert(order == -1);
  order = o;
};

int osl::ntesuki::NtesukiMove::
getOrder() const
{
  //ntesuki_assert(order != -1);
  return order;
}

void osl::ntesuki::NtesukiMove::
setNoPromote()
{
  ntesuki_assert(!(flags & NOPROMOTE));
  flags |= NOPROMOTE;
};

bool osl::ntesuki::NtesukiMove::
isNoPromote() const
{
  return flags & NOPROMOTE;
}

void osl::ntesuki::NtesukiMove::
setInterpose()
{
  ntesuki_assert(!(flags & INTERPOSE));
  flags |= INTERPOSE;
};

bool osl::ntesuki::NtesukiMove::
isInterpose() const
{
  return flags & INTERPOSE;
}

void osl::ntesuki::NtesukiMove::
setLameLong()
{
  ntesuki_assert(!(flags & LAME_LONG));
  flags |= LAME_LONG;
}

bool osl::ntesuki::NtesukiMove::
isLameLong() const
{
  return flags & LAME_LONG;
}

void osl::ntesuki::NtesukiMove::
setToOld()
{
  flags |= TO_OLDER_CHILD;
};

bool osl::ntesuki::NtesukiMove::
isToOld() const
{
  return flags & TO_OLDER_CHILD;
}

/* setImmediateCheckmate is defiend in .tcc
 */

bool osl::ntesuki::NtesukiMove::
isImmediateCheckmate()const
{
  return flags & IMMEDIATE_CHECKMATE;
};

void osl::ntesuki::NtesukiMove::
setBySimulation()
{
  flags |= BY_SIMULATION;
};

bool osl::ntesuki::NtesukiMove::
isBySimulation() const
{
  return flags & BY_SIMULATION;
}

/* Pawn drop checkmates
 */
void osl::ntesuki::NtesukiMove::
setPawnDropCheckmate()
{
  flags |= PAWN_DROP_CHECKMATE_FLAG;
};

bool osl::ntesuki::NtesukiMove::
isPawnDropCheckmate() const
{
  return 	(flags & PAWN_DROP_CHECKMATE_FLAG) == PAWN_DROP_CHECKMATE_FLAG;
}

void osl::ntesuki::NtesukiMove::
setHEstimates(unsigned short p_a, unsigned short d_a,
	      unsigned short p_d, unsigned short d_d)
{
  h_a_proof = p_a;
  h_a_disproof = d_a;
  h_d_proof = p_d;
  h_d_disproof = d_d;
}

bool osl::ntesuki::NtesukiMove::
isCheckmateSuccessSlow(Player P, int pass_left) const
{
  if (P == BLACK)
    return isCheckmateSuccess<BLACK>(pass_left);
  else
    return isCheckmateSuccess<WHITE>(pass_left);
}

bool osl::ntesuki::NtesukiMove::
isCheckmateFailSlow(Player P, int pass_left) const
{
  if (P == BLACK)
    return isCheckmateFail<BLACK>(pass_left);
  else
    return isCheckmateFail<WHITE>(pass_left);
}

/* about the move */
bool osl::ntesuki::NtesukiMove::
isValid() const { return move.isValid(); }
bool osl::ntesuki::NtesukiMove::
isInvalid() const { return move.isInvalid(); }
bool osl::ntesuki::NtesukiMove::
isNormal() const { return move.isNormal(); }
bool osl::ntesuki::NtesukiMove::
isPass() const { return move.isPass(); }
bool osl::ntesuki::NtesukiMove::
isDrop() const { return move.isDrop(); }
osl::Square osl::ntesuki::NtesukiMove::
to() const { return move.to(); }
osl::Ptype osl::ntesuki::NtesukiMove::
ptype() const { return move.ptype(); }
osl::Move osl::ntesuki::NtesukiMove::
getMove() const { return move; }

/* for moves */
bool osl::ntesuki::NtesukiMove::
operator==(const NtesukiMove& rhs) const
{
  return move == rhs.move;
};
bool osl::ntesuki::NtesukiMove::
operator!=(const NtesukiMove& rhs) const
{
  return move != rhs.move;
};
/* output to stream
 */
void osl::ntesuki::NtesukiMove::
flagsToStream(std::ostream& os) const
{
  int tmp = flags;
  for (int i = 0; i < 32; ++i)
  {
    if (1 == (tmp % 2))
      os << " " << NtesukiMove::FlagsStr[i];
    tmp = tmp >> 1;
  }
}

std::ostream& osl::ntesuki::
operator<<(std::ostream& os, const osl::ntesuki::NtesukiMove& move)
{
  os << "(" << move.getMove();
  os << "o=" << move.getOrder() << " ";
  move.flagsToStream(os);
  return os << ")";
}

namespace osl
{
  namespace ntesuki
  {
    template void NtesukiMove::setCheckmateSuccess<BLACK>(int pass_left);
    template void NtesukiMove::setCheckmateSuccess<WHITE>(int pass_left);
    template void NtesukiMove::setCheckmateFail<BLACK>(int pass_left);
    template void NtesukiMove::setCheckmateFail<WHITE>(int pass_left);
    template void NtesukiMove::setImmediateCheckmate<BLACK>();
    template void NtesukiMove::setImmediateCheckmate<WHITE>();
  }
}

std::string
osl::ntesuki::NtesukiMove::FlagsStr[] =
  {
    /* 2^0 CHECK_FLAG*/ 
    "CHECK", 
    /* 2^1 PAWN_DROP_CHECKMATE_FLAG = 2 */
    "PAWN_CHECKMATE", 
    /* 2^2 */
    "(BUG)", 
    /* 2^3 IMMEDIATE_CHECKMATE */
    "IMMEDIATE", 
    /* 2^4 TO_OLDER_CHILD */
    "OLD_CHILD", 
    /* 2^5 NOPROMOTE */
    "NOPROMOTE", 
    /* 2^6 INTERPOSE */
    "INTERPOSE", 
    /* 2^7 */
    "ATTACK", 
    /* 2^8 */
    "BY_SIMULATION", 
    /* 2^9 */
    "LAME_LONG", 
    /* 2^10 */
    "(BUG)", 
    /* 2^11 */
    "(BUG)", 
    /* 2^12 */
    "(BUG)", 
    /* 2^13 */
    "(BUG)", 
    /* 2^14 */
    "(BUG)", 
    /* 2^15 */
    "(BUG)", 
    /* 16-19 BLACK SUCCESS */
    "BLACK_SUCC_1", 
    "BLACK_SUCC_2", 
    "BLACK_SUCC_3", 
    "BLACK_SUCC_4", 
    /* 20-23 WHITE SUCCESS */
    "WHITE_SUCC_1", 
    "WHITE_SUCC_2", 
    "WHITE_SUCC_3", 
    "WHITE_SUCC_4", 
    /* 24-27 BLACK FAIL */
    "BLACK_FAIL_1", 
    "BLACK_FAIL_2", 
    "BLACK_FAIL_3", 
    "BLACK_FAIL_4", 
    /* 28-31 WHITE FAIL */
    "WHITE_FAIL_1", 
    "WHITE_FAIL_2", 
    "WHITE_FAIL_3", 
    "WHITE_FAIL_4", 
  };
