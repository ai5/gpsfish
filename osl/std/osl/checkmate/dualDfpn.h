/* dualDfpn.h
 */
#ifndef OSL_DUALDFPN_H
#define OSL_DUALDFPN_H
#include "osl/numEffectState.h"
#include "osl/checkmate/proofDisproof.h"
#include "osl/hashKey.h"
#include "osl/pathEncoding.h"
#include "osl/container/moveStack.h"
#include <memory>
#include <cstddef>
#include <limits>

#ifdef OSL_SMP
#  ifndef OSL_DFPN_SMP
#    define OSL_DFPN_SMP
#  endif
#endif

namespace osl
{
  class RepetitionCounter;
  namespace checkmate
  {
    class Dfpn;
    class DfpnTable;
    /** 一般用詰み探索: 先手後手の詰みを別々に管理 */
    class DualDfpn
    {
      struct Shared;
      struct Local;
      struct OraclePool;
      std::shared_ptr<Shared> shared;
      std::unique_ptr<Local> local;
    public:
      explicit DualDfpn(uint64_t ignored=std::numeric_limits<uint64_t>::max());
      DualDfpn(const DualDfpn& src);
      ~DualDfpn();

      void setRootPlayer(Player);
      template <Player P>
      ProofDisproof findProof(int node_limit, const NumEffectState& state, 
			      const HashKey& key, const PathEncoding& path,
			      Move& best_move, Move last_move=Move::INVALID());
      /**
       * 詰みを発見. 
       * 別々のスレッドから呼び出し可能
       * @return 相手玉が詰み
       */
      template <Player P>
      bool isWinningState(int node_limit, const NumEffectState& state, 
			  const HashKey& key, const PathEncoding& path,
			  Move& best_move, Move last_move=Move::INVALID())
      {
	return findProof(node_limit, state, key, path, best_move, last_move)
	  .isCheckmateSuccess();
      }
      bool isWinningState(int node_limit, const NumEffectState& state, 
			  const HashKey& key, const PathEncoding& path,
			  Move& best_move, Move last_move=Move::INVALID());
      ProofDisproof findProof(int node_limit, const NumEffectState& state, 
			      const HashKey& key, const PathEncoding& path,
			      Move& best_move, Move last_move=Move::INVALID());
#ifdef OSL_DFPN_SMP
      /** 
       * 詰みを発見. 
       * 同時に動作可能なスレッドは一つ
       */
      template <Player P>
      bool isWinningStateParallel(int node_limit, const NumEffectState& state, 
				  const HashKey& key, const PathEncoding& path,
				  Move& best_move, Move last_move=Move::INVALID());
      bool isWinningStateParallel(int node_limit, const NumEffectState& state, 
				  const HashKey& key, const PathEncoding& path,
				  Move& best_move, Move last_move=Move::INVALID());
#endif
      template <Player P>
      bool isLosingState(int node_limit, const NumEffectState& state, 
			 const HashKey& key, const PathEncoding& path,
			 Move last_move=Move::INVALID());
      bool isLosingState(int node_limit, const NumEffectState& state, 
			 const HashKey& key, const PathEncoding& path,
			 Move last_move=Move::INVALID());

      void runGC(bool verbose=false, size_t memory_use_ratio_1000=0);

      // debug
      void setVerbose(int level=1);
      int distance(Player attack, const HashKey& key);
      size_t mainNodeCount() const;
      size_t totalNodeCount() const;
      void writeRootHistory(const RepetitionCounter& counter,
			    const MoveStack& moves,
			    const SimpleState& state, Player attack);
      const DfpnTable& table(Player) const;
    private:
      Dfpn& prepareDfpn(Player attack);
      Dfpn& prepareDfpnSmall(Player attack);
    };
  }
  using checkmate::DualDfpn;
}

#endif /* OSL_DUALDFPN_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
