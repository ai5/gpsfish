/* ntesukiMove.h
 */
#ifndef _NTESUKIMOVE_H
#define _NTESUKIMOVE_H

#include "osl/move.h"
#include "osl/ntesuki/ntesukiExceptions.h"

namespace osl
{
  namespace ntesuki
  {
    /**
     * Move に ntesuki 探索に関する情報を加えたもの
     *
     * - flag にノードの状態を保持するようにした
     */
    class NtesukiMove
    {
      enum Flags
      {
	NONE = 0,
	/* 2^0 */
	CHECK_FLAG = 1,			/** If this move is a check move. */ 
	/* 2^1 */
	PAWN_DROP_CHECKMATE_FLAG = 2,	/** If this move is a pawn drop checkmate foul. */
	/* 2^2 */
	/* 4 not used */
	/* 2^3 */
	IMMEDIATE_CHECKMATE = 8,	/** If this move is an immidiate checkmate. */
	/* 2^4 */
	TO_OLDER_CHILD = 16,		/** If the distance of the parent is larger than the child */
	/* 2^5 */
	NOPROMOTE = 32,			/** A promotable PAWN,ROOK,BISHOP move without promotion */
	/* 2^6 */
	INTERPOSE = 64,			/** Aigoma */
	/* 2^7 */
	ATTACK_FLAG = 128,		/** Attack move candidate */
	/* 2^8 */
	BY_SIMULATION = 256,		/** Value of the node after this move determined by simulation */
	/* 2^9 */
	LAME_LONG = 512,
	/* 2^10 */
	/* 2^11 */
	/* 2^12 */
	/* 2^13 */
	/* 2^14 */
	/* 2^15 */
	WHITE_SHIFT = 4,
	IS_SUCCESS_SHIFT = 16,		/** This move leads to checkmate success. */
	/* 16-19 BLACK SUCCESS */
	IS_SUCCESS_BLACK_SHIFT = IS_SUCCESS_SHIFT,
	/* 20-23 WHITE SUCCESS */
	IS_SUCCESS_WHITE_SHIFT = IS_SUCCESS_SHIFT + WHITE_SHIFT,
	IS_SUCCESS_BLACK_BASE = 1 << IS_SUCCESS_BLACK_SHIFT,
	IS_SUCCESS_WHITE_BASE = 1 << IS_SUCCESS_WHITE_SHIFT,
	IS_SUCCESS_BLACK_MASK = 0xf * IS_SUCCESS_BLACK_BASE,
	IS_SUCCESS_WHITE_MASK = 0xf * IS_SUCCESS_WHITE_BASE,
	IS_FAIL_SHIFT = 24,		/** This move leads to checkmate fail. */
	/* 24-27 BLACK FAIL */
	IS_FAIL_BLACK_SHIFT = IS_FAIL_SHIFT,
	/* 28-31 WHITE FAIL */
	IS_FAIL_WHITE_SHIFT = IS_FAIL_SHIFT + WHITE_SHIFT,
	IS_FAIL_BLACK_BASE = 1 << IS_FAIL_SHIFT,
	IS_FAIL_WHITE_BASE = 1 << (IS_FAIL_SHIFT + WHITE_SHIFT),
	IS_FAIL_BLACK_MASK = 0xf * IS_FAIL_BLACK_BASE,
	IS_FAIL_WHITE_MASK = 0xfLL * IS_FAIL_WHITE_BASE,
      };

      static std::string FlagsStr[];
      template <Player P>
      int is_success_flag(int pass_left) const;
      template <Player P>
      int is_fail_flag(int pass_left) const;

      osl::Move move;
      int flags;
      int order;
    public:
      unsigned short h_a_proof, h_a_disproof;	// 4 byte
      unsigned short h_d_proof, h_d_disproof;	// 4 byte
      
    public:
      NtesukiMove();
      NtesukiMove(osl::Move m);
      NtesukiMove(osl::Move m, Flags f);
      NtesukiMove(const NtesukiMove&);
      ~NtesukiMove();
      NtesukiMove operator=(const NtesukiMove&);

      /* static methods */
      static NtesukiMove INVALID();

      /* about the state of the node
       */
      /* if it is a check move */
      void setCheck();
      bool isCheck() const;

      /* if it is a check move */
      void setOrder(int o);
      int getOrder() const;

      /* interpose */
      void setInterpose();
      bool isInterpose() const;

      /* long move with no specific meaning */
      void setLameLong();
      bool isLameLong() const;

      /* by simulation */
      void setBySimulation();
      bool isBySimulation() const;

      /* nopromote */
      void setNoPromote();
      bool isNoPromote() const;

      /* if it is a move to an child with less depth */
      void setToOld();
      bool isToOld() const;

      /* if it is a immidiate checkmate move */
      template<Player P>
      void setImmediateCheckmate();
      bool isImmediateCheckmate() const;

      /* if it is a success/fail move */
      template<Player P>
      void setCheckmateSuccess(int pass_left);
      template<Player P>
      bool isCheckmateSuccess(int pass_left) const;
      bool isCheckmateSuccessSlow(Player P, int pass_left) const;

      template<Player P>
      void setCheckmateFail(int pass_left);
      template<Player P>
      bool isCheckmateFail(int pass_left) const;
      bool isCheckmateFailSlow(Player P, int pass_left) const;

      /* if it is a pawn drop checkmate move */
      void setPawnDropCheckmate();
      bool isPawnDropCheckmate() const;

      /* estimates used before the node after the move is expanded */
      void setHEstimates(unsigned short p_a, unsigned short d_a,
			 unsigned short p_d, unsigned short d_d);
      void setCEstimates(unsigned short p, unsigned short d);

      /* about the move itself */
      bool isValid() const;
      bool isInvalid() const;
      bool isNormal() const;
      bool isPass() const;
      bool isDrop() const;
      Square to() const;
      Ptype ptype() const;
      Move getMove() const;

      /* equality, only the equality of the move is checked
       * (the flags are not considered) */
      bool operator==(const NtesukiMove& rhs) const;
      bool operator!=(const NtesukiMove& rhs) const;

      /* output to stream */
      void flagsToStream(std::ostream& os) const;
      friend std::ostream& operator<<(std::ostream& os, const NtesukiMove& move);
    };

  }// ntesuki
}// osl
#endif /* _NTESUKIMOVE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
