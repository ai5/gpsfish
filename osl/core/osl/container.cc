#include "osl/container.h"
#include "osl/eval/ptypeEval.h"
#include <iostream>

std::ostream& osl::operator<<(std::ostream& os,MoveVector const& mv)
{
  os<< "MoveVector" << std::endl;
  for (Move m: mv) {
    os << m << std::endl;
  }
  return os<<std::endl;
}
bool osl::operator<(const MoveVector& l, const MoveVector& r)
{ 
  return std::lexicographical_compare(l.begin(), l.end(),
				      r.begin(), r.end()); 
}

namespace osl
{
  struct PieceBasicLessThan
  {
    bool operator()(Piece p0,Piece p1){
      const Ptype ptype0=unpromote(p0.ptype());
      const Ptype ptype1=unpromote(p1.ptype());
      return (eval::Ptype_Eval_Table.value(ptype0)
	      < eval::Ptype_Eval_Table.value(ptype1));
    }
  };
  struct PiecePtypeMoreThan
  {
    bool operator()(Piece p0,Piece p1){
      const PtypeO ptypeo0=p0.ptypeO();
      const PtypeO ptypeo1=p1.ptypeO();
      return (abs(eval::Ptype_Eval_Table.captureValue(ptypeo0))
	      > abs(eval::Ptype_Eval_Table.captureValue(ptypeo1)));
    }
  };
} // namespace osl

void osl::PieceVector::sortByBasic()
{
  std::sort(begin(),end(),PieceBasicLessThan());
}

void osl::PieceVector::sortByPtype()
{
  std::sort(begin(),end(),PiecePtypeMoreThan());
}

#ifndef MINIMAL
std::ostream& osl::operator<<(std::ostream& os,PieceVector const& pv)
{
  os << "PieceVector";
  for (Piece p: pv) {
    os << " " << p;
  }
  return os << std::endl;
}
#endif

struct osl::PtypeOSquareVector::PtypeOSquareLessThan
{
  bool operator()(const std::pair<PtypeO,Square>& l,
		  const std::pair<PtypeO,Square>& r) {
    const int vall = abs(eval::Ptype_Eval_Table.captureValue(l.first));
    const int valr = abs(eval::Ptype_Eval_Table.captureValue(r.first));
    if (vall != valr)
      return vall < valr;
    return l.second.uintValue() < r.second.uintValue();
  }
};

void osl::PtypeOSquareVector::sort()
{
  std::sort(begin(),end(),PtypeOSquareLessThan());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
