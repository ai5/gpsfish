/* checkHashRecord.h
 */
#ifndef _CHECKHASHRECORD_H
#define _CHECKHASHRECORD_H

#include "osl/checkmate/proofDisproof.h"
#include "checkMoveList.h"
#include "checkAssert.h"
#include "twinList.h"
#include "twinTable.h"
#include "osl/checkmate/proofPieces.h"
#include "osl/checkmate/disproofPieces.h"
#include "osl/pieceStand.h"
#include "osl/move.h"

/**
 * @def CHECK_PROPAGATE_BY_DOMINANCE
 * 定義すると局面の優越関係を記憶し，詰/不詰を伝播させる 
 */
#define CHECK_PROPAGATE_BY_DOMINANCE

/**
 * @def CHECK_EXTRA_LIMIT_PROOF
 * 内部ノードの再展開抑制のための大きめの閾値 (Kishimoto2003?)
 */
#define CHECK_EXTRA_LIMIT_PROOF
/**
 * @def CHECK_EXTRA_LIMIT_DISPROOF
 * 内部ノードの再展開抑制のための大きめの閾値 (Kishimoto2003?)
 */
#define CHECK_EXTRA_LIMIT_DISPROOF

/**
 * @def CHECK_DELAY_UPWARDS
 * サイクル対策 (Kishimoto2003)
 */
#define CHECK_DELAY_UPWARDS

/**
 * @def CHECK_ABSORB_LOOP
 * LoopDetection の伝播制限 (bug 45)
 */
// #define CHECK_ABSORB_LOOP

namespace osl
{
  namespace checkmate
  {
    struct SameBoardList;
    /** 
     * 詰将棋でノードごとにテーブルに保持するデータ
     */
    struct CheckHashRecord
    {
    private:
      ProofDisproof proof_disproof;
    public:
      /** 手番に*否定的*な結果が判明した指手の中で最も都合が良い結果.
       * 但し， LoopDetection は除く
       * TODO: メモリ節約: proof/disproof のどちらかは省略可
       */
      ProofDisproof bestResultInSolved;
      /** 現在探索中のリスト */
      CheckMoveList moves;
      /** 詰の場合の詰ます手 finalByDominance の場合、別のノードの (solved)moves を指す場合がある*/
      CheckMove *bestMove;
      /** 親の一つ，合流の検出に使う */
      CheckHashRecord *parent;	// root はまとめられているので注意
      /** 優越関係で詰/不詰が分かった場合に記録 */
    private:
      const CheckHashRecord *final_by_dominance;
    public:
      /** 同じ盤面(持駒が違う)のリスト */
      SameBoardList *sameBoards;
      /** LoopDetection 関係(シミュレーション候補) */
      TwinList twins;
    private:
      /** 現在の局面の持駒 */
      PieceStand black_stand, white_stand;
      /** 証明駒または反証駒: 詰(不詰)の場合のみセットされる */
      PieceStand proof_disproof_pieces;
    public:
      /** root からの最短パスの距離 */
      signed short distance;
      /** 現在対象とする指手の種類 */
      MoveFilter filter;
      // この辺のbool は後でbit field にすることを試す
      /** 自身で合流の場合真 */
      bool isConfluence;
      /** 真なら sum の代わりにmax を取る (loop/dag 対策) */
      bool useMaxInsteadOfSum;
      /** 探索中に書き換えられる */
      bool isVisited;
      /** 柿木2005 */
      bool false_branch_candidate;
      bool false_branch;
    private:
      enum ProofPiecesType {
	UNSET = 0, PROOF, DISPROOF,
      };
      /** proof (disproof) pieces をセットしたかどうか. */
      char proof_pieces_type;
    public:
      /** proofDisproof の初期値. 優越関係が見つかったら上書きされる */
      static const ProofDisproof initialProofDisproof()
      {
	return ProofDisproof(1,1);
      }
      CheckHashRecord() 
	: proof_disproof(initialProofDisproof()), 
	  bestResultInSolved(ProofDisproof::Bottom()),
	  bestMove(0), parent(0), final_by_dominance(0), sameBoards(0),
	  distance(0), isConfluence(false),
	  useMaxInsteadOfSum(false), isVisited(false), 
	  false_branch_candidate(false), false_branch(false),
	  proof_pieces_type(UNSET)
      {
      }
      CheckHashRecord(PieceStand black, PieceStand white)
	: proof_disproof(initialProofDisproof()), 
	  bestResultInSolved(ProofDisproof::Bottom()),
	  bestMove(0), parent(0), final_by_dominance(0), sameBoards(0),
	  black_stand(black), white_stand(white),
	  distance(0), isConfluence(false),
	  useMaxInsteadOfSum(false), isVisited(false), 
	  false_branch_candidate(false), false_branch(false),
	  proof_pieces_type(UNSET)
      {
      }
      ~CheckHashRecord();

      void setProofDisproof(const ProofDisproof& pdp)
      {
	check_assert(! pdp.isFinal());
	check_assert(! proof_disproof.isFinal());
	proof_disproof = pdp;
      }
      void setProofDisproof(unsigned int proof, unsigned int disproof)
      {
	check_assert(proof && disproof);
	check_assert(! proof_disproof.isFinal());
	assert(proof < ProofDisproof::PROOF_LIMIT);
	assert(disproof < ProofDisproof::DISPROOF_LIMIT);
	setProofDisproof(ProofDisproof(proof, disproof));
      }
      const ProofDisproof& proofDisproof() const { return proof_disproof; }
      unsigned int proof() const { return proof_disproof.proof(); }
      unsigned int disproof() const { return proof_disproof.disproof(); }

      void setProofByDominance(unsigned int disproof, 
			       const CheckHashRecord *final_by_dominance)
      {
	assert(disproof > ProofDisproof::DISPROOF_LIMIT);
	assert(final_by_dominance);
	proof_disproof = ProofDisproof(0, disproof);
	check_assert(final_by_dominance->hasProofPieces());
	setProofPieces(final_by_dominance->proofPieces());
	setFinalByDominance(final_by_dominance->finalByDominance() 
			    ? final_by_dominance->finalByDominance() : final_by_dominance);
	check_assert(! finalByDominance()->finalByDominance());
	bestMove = final_by_dominance->bestMove;
	assert(isConsistent());
      }
      void setDisproofByDominance(unsigned int proof,
				  const CheckHashRecord *final_by_dominance)
      {
	assert(proof > ProofDisproof::PROOF_LIMIT);
	proof_disproof = ProofDisproof(proof, 0);
	assert(final_by_dominance->proofDisproof().isCheckmateFail());
	check_assert(final_by_dominance->hasDisproofPieces()
		     || (final_by_dominance->dump(), 0));
	setDisproofPieces(final_by_dominance->disproofPieces());
	setFinalByDominance
	  (final_by_dominance->finalByDominance() 
	   ? final_by_dominance->finalByDominance() : final_by_dominance);
	check_assert((! finalByDominance()->finalByDominance())
		     || (final_by_dominance->dump(),0));
	bestMove = final_by_dominance->bestMove;
      }
      void setFinalByDominance(const CheckHashRecord *dominance) 
      { 
	check_assert((dominance == 0) || dominance->final_by_dominance == 0);
	// 証明駒/反証駒をセットしてから
	check_assert((dominance == 0) || proof_pieces_type);
	final_by_dominance = dominance;
      }

      const PieceStand stand(Player player) const
      {
	return (player == BLACK) ? black_stand : white_stand;
      }
      const CheckHashRecord *finalByDominance() const 
      { 
	return final_by_dominance;
      }
      void setProofPieces(PieceStand new_stand)
      {
	check_assert(proof_pieces_type == UNSET);
	proof_pieces_type = PROOF;
	proof_disproof_pieces = new_stand;
      }
      /** bestMove をもとに証明駒を設定 */
      void setProofPiecesAttack(Player attacker)
      {
	check_assert(bestMove && bestMove->record);
	check_assert(bestMove->record->hasProofPieces());
	const PieceStand proof_pieces 
	  = ProofPieces::attack(bestMove->record->proofPieces(), bestMove->move,
				stand(attacker));
	setProofPieces(proof_pieces);
      }
      void setDisproofPieces(PieceStand new_stand)
      {
	check_assert(proof_pieces_type == UNSET);
	proof_pieces_type = DISPROOF;
	proof_disproof_pieces = new_stand;
      }
      /** bestMove をもとに反証駒を設定 */
      void setDisproofPiecesDefense(Player defense)
      {
	check_assert(bestMove && bestMove->record);
	check_assert(bestMove->record->hasDisproofPieces());
	const PieceStand disproof_pieces 
	  = DisproofPieces::defense(bestMove->record->disproofPieces(), bestMove->move,
				    stand(defense));
	setDisproofPieces(disproof_pieces);
      }
      bool hasProofPieces() const { return proof_pieces_type == PROOF; }
      bool hasDisproofPieces() const { return proof_pieces_type == DISPROOF; }
      const PieceStand proofPieces() const 
      {
	check_assert(proof_pieces_type == PROOF);
	return proof_disproof_pieces; 
      }
      const PieceStand disproofPieces() const 
      {
	check_assert(proof_pieces_type == DISPROOF);
	return proof_disproof_pieces; 
      }
      bool isConsistent() const;

      bool hasBestMove() const { return bestMove; }
      CheckMove* getBestMove()
      {
	check_assert(hasBestMove());
	return bestMove;
      }
      const CheckMove* getBestMove() const
      {
	check_assert(hasBestMove());
	return bestMove;
      }
      bool needMoveGeneration() const { return moves.empty(); }
      unsigned int add(unsigned int a, unsigned int b)
      {
	const unsigned int result = (useMaxInsteadOfSum) ? std::max(a,b) : a+b;
	check_assert(result >= a);
	check_assert(result >= b);
	return result;
      }
    private:
      template <Player Attacker>
      void propagateCheckmateRecursive();
      template <Player Attacker>
      void propagateNoCheckmateRecursive();
    public:
      /** 詰の時に proofDisproof を設定し，さらに優越関係がある局面に伝播 */
      template <Player Attacker>
      void propagateCheckmate(ProofDisproof pdp)
      {
	check_assert(pdp.proof()==0);
	proof_disproof = pdp;
#ifdef CHECK_PROPAGATE_BY_DOMINANCE
	if (sameBoards)
	  propagateCheckmateRecursive<Attacker>();
#endif
      }
      /** 不詰の時に proofDisproof を設定し，さらに優越関係がある局面に伝播 */
      template <Player Attacker>
      void propagateNoCheckmate(ProofDisproof pdp)
      {
	check_assert(pdp.disproof()==0);
	check_assert(! pdp.isLoopDetection());
	proof_disproof = pdp;
#ifdef CHECK_PROPAGATE_BY_DOMINANCE
	if (sameBoards)
	  propagateNoCheckmateRecursive<Attacker>();
#endif
      }
    public:
      /** 親が等しいことを確認 */
      void confirmParent(CheckHashRecord *parent)
      {
	check_assert(this->parent && parent);
	if (this->parent != parent)
	{
	  isConfluence = true;
	  this->parent = parent;	// 上書き
	  this->distance = std::min(parent->distance+1, (int)this->distance);
	}
      }
      /**
       * 攻撃側の moves を一通り眺め，各種(dis)proof number を集計し，最善の手を bestChild に保存する.
       * 
       * - NoEscape より Checkmate を選ぶ
       * @return 反証されていない子供の proof number のだいたいの平均
       * @param bestResultInSolved (bestChild==0) の時に，もっともましなpdp.
       *  LoopDetection を含む
       */
      unsigned int selectBestAttackMove(const PathEncoding& path,
					const TwinTable& table,
					unsigned int& currentBestProofNumber,
					unsigned int& currentSecondBestProofNumber,
					unsigned int& currentDisproofNumber,
					unsigned int& currentBestDisproofNumber,
					ProofDisproof& bestResultInSolved,
					CheckMove *&bestChild);
      // TODO: かっこ悪いので何とかする
      unsigned int selectBestAttackMoveMain(const PathEncoding& path,
					    const TwinTable& table,
					    unsigned int& currentBestProofNumber,
					    unsigned int& currentSecondBestProofNumber,
					    unsigned int& currentDisproofNumber,
					    unsigned int& currentBestDisproofNumber,
					    ProofDisproof& bestResultInSolved,
					    CheckMove *&bestChild);
      /**
       * 防御側の moves を一通り眺め，各種(dis)proof number を集計し，最善の手を bestChild に保存する.
       * 
       * finalでない時，proof number が同じならdisproof number が小さい方を選ぶ
       * - Checkmate より NoEscape を選ぶ (打歩詰の可能性があるから)
       * @param bestResultInSolved (bestChild==0) の時に，もっともましなpdp
       * @return 証明されていない子供の disproof number のだいたいの平均
       */
      unsigned int selectBestDefenseMove(const PathEncoding& path,
					 const TwinTable& table,
					 unsigned int& currentBestDisproofNumber,
					 unsigned int& currentSecondBestDisproofNumber,
					 unsigned int& currentProofNumber,
					 unsigned int& currentBestProofNumber,
					 ProofDisproof& bestResultInSolved,
					 CheckMove *&bestChild);
      /** @param dumpDepth 1 以上の時に bestMove を再帰的にdumpする. */
      void dump(std::ostream& os, int dumpDepth) const; 
      void dump(int dumpDepth=1) const; 

      void updateBestResultInSolvedAttack(const ProofDisproof& pdp)
      {
	check_assert(pdp.isCheckmateFail());
	check_assert(! pdp.isLoopDetection());
	bestResultInSolved = bestResultInSolved.betterForAttack(pdp);
      }
      void updateBestResultInSolvedDefense(const ProofDisproof& pdp)
      {
	check_assert(pdp.isCheckmateSuccess());
	bestResultInSolved = bestResultInSolved.betterForDefense(pdp);
      }
      /**
       * @param isAttack 攻方のノードかどうか
       */
      void addToSolved(CheckMove& move, const ProofDisproof& pdp,
		       bool isAttack)
      {
	check_assert(std::find(moves.begin(), moves.end(), move) != moves.end());
	check_assert(pdp.isFinal());
	check_assert(move.record == 0
		     || move.record->proofDisproof().isFinal());
	check_assert(! pdp.isLoopDetection());
	if (isAttack)
	  updateBestResultInSolvedAttack(pdp);
	else
	  updateBestResultInSolvedDefense(pdp);
	move.flags = MoveFlags::Solved;
      }
      void addToSolvedInAttack(CheckMove& move, const ProofDisproof& pdp)
      {
	check_assert(pdp.disproof() == 0);
	check_assert(! pdp.isLoopDetection());
	addToSolved(move, pdp, true);
      }
      void addToSolvedInDefense(CheckMove& move, const ProofDisproof& pdp)
      {
	check_assert(pdp.proof() == 0);
	addToSolved(move, pdp, false);
      }
      void setLoopDetection(const PathEncoding& path, const CheckMove& move,
			    const CheckHashRecord *loopTo)
      {
	check_assert(! proofDisproof().isFinal());
	proof_disproof = ProofDisproof::Unknown();
	twins.addLoopDetection(path, move, loopTo);
      }
      template <Player Attacker>
      void setLoopDetectionTryMerge(const PathEncoding& path, 
				    CheckMove& move,
				    const CheckHashRecord *loopTo)
      {
	check_assert(Attacker == alt(move.move.player()));
#ifdef CHECK_ABSORB_LOOP
	if (loopTo == this)
	{
	  check_assert(move.record);
	  const ProofDisproof& pdp = move.record->bestResultInSolved;
	  const ProofDisproof nocheck
	    = pdp.betterForAttack(ProofDisproof::NoCheckmate());
	  check_assert(nocheck.disproof() == 0);
	  bestMove = &move;
	  twins.clear();
	  propagateNoCheckmate<Attacker>(nocheck);
	  return;
	}
#endif
	setLoopDetection(path, move, loopTo);
      }
      void setLoopDetection(const PathEncoding& path, 
			    const CheckHashRecord *loopTo)
      {
	setLoopDetection(path, CheckMove(), loopTo);
      }
      template <Player Attacker>
      void setLoopDetectionInAttack(const PathEncoding& path)
      {
#ifdef CHECK_ABSORB_LOOP
	const CheckHashRecord *loopTo = 0;
	for (CheckMoveList::iterator p=moves.begin(); p!=moves.end(); ++p)
	{
	  if (! filter.isTarget(p->flags))
	    continue;
	  check_assert(p->record);
	  if (p->record->isVisited)
	  {
	    if (loopTo && (loopTo != p->record))
	      goto multiple_loops;
	    loopTo = p->record;
	    continue;
	  }
	  const TwinEntry *curLoop = p->findLoop(path);
	  check_assert(curLoop);
	  if (curLoop->loopTo == this)
	  {
	    p->flags.set(MoveFlags::Solved);
	    continue;
	  }
	  if ((curLoop->loopTo == 0)
	      || (loopTo && (loopTo != curLoop->loopTo)))
	    goto multiple_loops;
	  loopTo = curLoop->loopTo;
	}
	if (loopTo == 0)
	{
	  twins.clear();
	  const ProofDisproof nocheck
	    = bestResultInSolved.betterForAttack(ProofDisproof::NoCheckmate());
	  check_assert(nocheck.disproof() == 0);
	  propagateNoCheckmate<Attacker>(nocheck);
	  return;
	}
	setLoopDetection(path, CheckMove(), loopTo);
	return;
      multiple_loops:
#endif
	setLoopDetection(path, CheckMove(), 0);
	return;
      }
      /**
       * 全てのtwin entry を調べて，loopがあるかどうかを判定する.
       * path で this に到達した時に LoopDetection ならば TwinEntryを返す
       * TODO: 一手先の検索がほとんどなので，一手前のものを保存するのが良い?
       */
      const TwinEntry* findLoop(const PathEncoding& path, 
				const TwinTable& table) const
      {
	if (twins.empty())
	  return 0;

	TwinList::const_iterator pos = twins.find(path);
	if (pos != twins.end())
	  return &*pos;
	return table.findTwin(path);
      }
      /**
       * twin list を調べて，loopがあるかどうかを判定する.
       * simulation の場合は，twin list にあるループだけを対象とすれば良い
       */
      const TwinEntry* findLoopInList(const PathEncoding& path) const
      {
	TwinList::const_iterator pos = twins.find(path);
	if (pos != twins.end())
	  return &*pos;
	return 0;
      }
    };


    inline const TwinEntry*
    CheckMove::findLoop(const PathEncoding& path, const TwinTable& table) const
    {
      check_assert(record);
      PathEncoding new_path = path;
      new_path.pushMove(move);
      return record->findLoop(new_path, table);
    }
    inline const TwinEntry*
    CheckMove::findLoopInList(const PathEncoding& path) const
    {
      check_assert(record);
      PathEncoding new_path = path;
      new_path.pushMove(move);
      return record->findLoopInList(new_path);
    }
  } // namespace checkmate

  using checkmate::CheckHashRecord;
} // namespace osl

#endif /* _CHECKHASHRECORD_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
