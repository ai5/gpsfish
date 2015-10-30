/* ptypeEvalTraits.h
 */
#ifndef EVAL_PTYPEEVALTRAITS_H
#define EVAL_PTYPEEVALTRAITS_H

#include "osl/basic_type.h"
namespace osl
{
  namespace eval
  {
    template<Ptype T>
    struct PtypeEvalTraits;
  
    template<>
    struct PtypeEvalTraits<PAWN>{
      static const int val=128;
    };

    template<>
    struct PtypeEvalTraits<PPAWN>{
      static const int val=768;
    };

    template<>
    struct PtypeEvalTraits<LANCE>{
      static const int val=512;
    };

    template<>
    struct PtypeEvalTraits<PLANCE>{
      static const int val=768;
    };

    template<>
    struct PtypeEvalTraits<KNIGHT>{
      static const int val=512;
    };

    template<>
    struct PtypeEvalTraits<PKNIGHT>{
      static const int val=768;
    };

    template<>
    struct PtypeEvalTraits<SILVER>{
      static const int val=704;
    };

    template<>
    struct PtypeEvalTraits<PSILVER>{
      static const int val=768;
    };

    template<>
    struct PtypeEvalTraits<GOLD>{
      static const int val=768;
    };

    template<>
    struct PtypeEvalTraits<BISHOP>{
      static const int val=1024;
    };

    template<>
    struct PtypeEvalTraits<PBISHOP>{
      static const int val=1472;
    };

    template<>
    struct PtypeEvalTraits<ROOK>{
      static const int val=1216;
    };

    template<>
    struct PtypeEvalTraits<PROOK>{
      static const int val=1664;
    };

    template<>
    struct PtypeEvalTraits<KING>{
      static const int val=12800;
    };
  
  } // namespace eval
} // namespace osl


#endif /* EVAL_PTYPEEVALTRAITS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
