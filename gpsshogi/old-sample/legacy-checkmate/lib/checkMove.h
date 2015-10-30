/* checkMove.h
 */
#ifndef _CHECKMOVE_H
#define _CHECKMOVE_H

#include "osl/checkmate/proofDisproof.h"
#include "osl/move.h"
#include "osl/pathEncoding.h"
#include <iosfwd>
namespace osl
{
  namespace checkmate
  {
    class CheckHashRecord;
    class TwinEntry;
    class TwinTable;
    class MoveFlags
    {
      unsigned char flags;
    public:
      enum Constant {
	/** 手番に*否定的*な結果が判明した指手 */
	Solved=1,
	/** 歩以外の中合いリスト. 守備側で詰があった時だけ使う， */
	BlockingBySacrifice=2,
	/** 歩飛角の不成の指手. 攻撃側で打歩詰があったときだけ使う. */
	NoPromote=4,
	/** 捨駒の攻撃 最初は読まない */
	SacrificeAttack=8,
	/** 深さの浅い子供へのリンク.
	 * ループなどのもと．他の指手が全て詰/不詰の時だけ使用 */
	Upward=16,
	/** この手で一手詰. この先にrecordがあるとは限らない */
	ImmediateCheckmate=32,
      };
      MoveFlags() : flags(0) {}
      MoveFlags& operator=(Constant flag)
      {
	flags = static_cast<unsigned char>(flag);
	return *this;
      }
      void set(Constant flag)
      {
	assert((flags & flag) == 0);
	flags |= flag;
      }
      void unset(Constant flag)
      {
	assert(flags & flag);
	flags ^= flag;
      }
      int isSet(Constant flag) const
      {
	return flags & flag;
      }
      int getFlags() const { return flags; }
    };
    class MoveFilter
    {
      unsigned char mask;
    public:
      MoveFilter() : mask(31)
      {
      }
      bool isTarget(MoveFlags flags) const
      {
	return (mask & flags.getFlags()) == 0;
      }
      void addTarget(MoveFlags::Constant flag)
      {
	assert(! isTarget(flag));
	mask ^= flag;
      }
      bool isTarget(MoveFlags::Constant flag) const
      {
	return (mask & flag) == 0;
      }
      int getMask() const { return mask; }
    };
    std::ostream& operator<<(std::ostream& os, MoveFlags);
    std::ostream& operator<<(std::ostream& os, MoveFilter);
    /** 
     * 詰将棋で使う指手
     */
    struct CheckMoveCore
    {
      /** move を指した後のrecord */
      CheckHashRecord *record;	// 8 byte
      Move move;		// 4 byte
      explicit CheckMoveCore(Move m=Move::INVALID(), CheckHashRecord *r=0) 
	: record(r), move(m)
      {
      }
    };
    /** 
     * 詰将棋で使う指手
     */
    struct CheckMove : public CheckMoveCore
    {
      MoveFlags flags;		// 1 byte
      /** cost */
      signed char cost_proof, cost_disproof; // 2 byte
      /** H. record == 0 の時だけ使う */
      unsigned short h_proof, h_disproof;	// 4 byte
      explicit CheckMove(Move m=Move::INVALID(), CheckHashRecord *r=0) 
	: CheckMoveCore(m, r),
	  cost_proof(0), cost_disproof(0), h_proof(1), h_disproof(1)
      {
      }
      void addCost(unsigned& proof, unsigned int& disproof) const
      {
	assert(proof && disproof);
	assert(cost_proof >= 0);
	assert(cost_disproof >= 0);
	proof += cost_proof;
	disproof += cost_disproof;
	assert(proof < ProofDisproof::PROOF_LIMIT);
	assert(disproof < ProofDisproof::DISPROOF_LIMIT);
      }
      /**
       * path の後に this を指した後に LoopDetection かどうか
       * CheckHashRecord.h で定義
       * @return 見つからなければ0
       */
      inline const TwinEntry* findLoop(const PathEncoding& path,
				       const TwinTable& table) const;
      inline const TwinEntry* findLoopInList(const PathEncoding& path) const;
    };
    inline bool operator==(const CheckMove& l, const CheckMove& r)
    {
      if (l.move == r.move)
      {
	assert(l.record == r.record);
	return true;
      }
      assert(l.record != r.record || (l.record == 0 && r.record == 0));
      return false;
    }
  } // namespace checkmate
} // namespace osl


#endif /* _CHECKMOVE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
