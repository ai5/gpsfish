/* ntesukiSimulationSearcher.h
 */
#ifndef __NTESUKI_SIMULATION_SEARCHER_H
#define __NTESUKI_SIMULATION_SEARCHER_H
#include "osl/ntesuki/ntesukiTable.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "ntesukiExceptions.h"

typedef osl::state::NumEffectState state_t;

namespace osl
{
  namespace ntesuki
  {
    class
    NtesukiSimulationSearcher
    {
      /** 現在までに何ノード読んだか */
      unsigned int node_count;
      /** 経過をどこまで表示するか */
      bool verbose;

      /** Simulation 結果の統計 */
      unsigned int proof_count;
      unsigned int proof_success_count;
      unsigned int light_proof_success_count;
      unsigned int disproof_count;
      unsigned int disproof_success_count;
    public:
      bool debug;
    private:
      NtesukiResult result;
      state_t& state;
      NtesukiMoveGenerator *mg;
      PathEncoding& path;
      NtesukiTable& table;
      NtesukiRecord::ISScheme isscheme;

      /*
       * helpers
       */
      template <class Searcher, Player P> class AttackHelperProof;
      template <class Searcher, Player P> class DefenseHelperProof;
      template <class Searcher, Player P> class AttackHelperDisproof;
      template <class Searcher, Player P> class DefenseHelperDisproof;

      /* Utilities
       */
      template<Player P>
      bool
      isSafeMove(const Move move,
		 int pass_left);

      template <Player P>
      Move
      adjustMove(Move candidate) const
      {
	assert(candidate.isValid());
	if (! candidate.isDrop())
	{
	  const Piece p=state.pieceOnBoard(candidate.to());
	  candidate=setCapture(candidate,p);
	}
	return candidate;
      }
      
      /**
       * Proof 攻撃に関する計算
       */
      template <Player P>
      void attackForProof(NtesukiRecord* record,
			  const NtesukiRecord* record_orig,
			  const unsigned int passLeft,
			  const Move last_move);
      /**
       * Disproof 防御に関する計算
       */
      template <Player P>
      void defenseForProof(NtesukiRecord* record,
			   const NtesukiRecord* record_orig,
			   const unsigned int passLeft,
			   const Move last_move);
      /**
       * Disproof 攻撃に関する計算
       */
      template <Player P>
      void attackForDisproof(NtesukiRecord* record,
			     const NtesukiRecord* record_orig,
			     const unsigned int passLeft,
			     const Move last_move);
      /**
       * Disproof 防御に関する計算
       */
      template <Player P>
      void defenseForDisproof(NtesukiRecord* record,
			      const NtesukiRecord* record_orig,
			      const unsigned int passLeft,
			      const Move last_move);

    public:
      NtesukiSimulationSearcher(state_t& state,
				NtesukiMoveGenerator *mg,
				PathEncoding& path,
				NtesukiTable& table,
				NtesukiRecord::ISScheme isscheme,
				bool verbose = false);
      ~NtesukiSimulationSearcher();

      /**
       * Start simulation to proof, P as Attacker.
       * @return true, if checkmate is proven
       */
      template <Player P>
      bool
      startFromAttackProof(NtesukiRecord* record,
			   const NtesukiRecord* record_orig,
			   const unsigned int passLeft,
			   const Move last_move);

      /**
       * Start simulation to proof, P as Defender.
       * @return true, if checkmate is proven
       */
      template <Player P>
      bool
      startFromDefenseProof(NtesukiRecord* record,
			    const NtesukiRecord* record_orig,
			    const unsigned int passLeft,
			    const Move last_move);

      /**
       * Start simulation to disproof, P as Attacker.
       * @return true, if nocheckmate is proven
       */
      template <Player P>
      bool
      startFromAttackDisproof(NtesukiRecord* record,
			      const NtesukiRecord* record_orig,
			      const unsigned int passLeft,
			      const Move last_move);

      /**
       * Start simulation to disproof, P as Defender.
       * @return true, if nocheckmate is proven
       */
      template <Player P>
      bool
      startFromDefenseDisproof(NtesukiRecord* record,
			       const NtesukiRecord* record_orig,
			       const unsigned int passLeft,
			       const Move last_move);

      unsigned int nodeCount() const { return node_count; }
    };
  } //ntesuki
} //osl
#endif /* _NTESUKI_SIMULATION_SEARCHER_H */

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
