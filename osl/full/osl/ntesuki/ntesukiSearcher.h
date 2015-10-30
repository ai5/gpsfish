/* ntesukiSearcher.h
 */
#ifndef _NTESUKI_SEACHER_H
#define _NTESUKI_SEACHER_H
#include "osl/state/numEffectState.h"
#include "osl/ntesuki/ntesukiTable.h"
#include "osl/ntesuki/ntesukiRecord.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"

#include "osl/ntesuki/ntesukiExceptions.h"
#include "osl/container/moveVector.h"
#include "osl/ntesuki/ntesukiSimulationSearcher.h"

#include <osl/stl/vector.h>

namespace osl
{
  namespace ntesuki
  {
    class
    NtesukiSearcher
    {
    public:
      typedef NumEffectState State;

    private:
      State& state;

      /** 手生成器. */
      NtesukiMoveGenerator *mg;
      
      /** トランスポジションテーブル. */
      NtesukiTable table;

      /** シミュレーション探索器. */
      NtesukiSimulationSearcher simulator;

      /** 現在までに何ノード読んだか. */
      unsigned int node_count;

      /** 最大何ノードまで読むか. */
      unsigned int read_node_limit;
      
      /** 経過をどこまで表示するか. */
      bool verbose;

      /** 探索を途中で強制的に終了させるための flag. */
      volatile int *stop_flag;

      /** ルートから現在探索中のノードまでの path の hash値. */
      PathEncoding path;

      /** ルートから現在探索中のノードまでの move. */
      typedef std::vector<Move> moves_t;
      moves_t moves_played;
      
      /** ルートから現在探索中のノードまでの局面. */
      typedef std::vector<NtesukiRecord *> nodes_t;
      nodes_t nodes_played;

      /** λオーダ */
      unsigned int max_pass;


    public:
      /**
       * 探索のふるまいを制御する変数.
       */
      static bool delay_non_pass;
      static bool ptt_invalid_defense;

      static bool delay_interpose;
      static bool delay_nopromote;
      static bool delay_non_attack;
      static bool read_attack_only;
      static bool ptt_non_attack;

      static bool ptt_siblings_fail;
      static bool ptt_siblings_success;

      static bool ptt_uncle;
      static bool ptt_aunt;

      static unsigned int dynamic_widening_width;

    private:
      /** Iterative widening の scheme */
      NtesukiRecord::IWScheme iwscheme;
      /** Player selection の scheme */
      NtesukiRecord::PSScheme psscheme;
      /** Inversion searching の scheme */
      NtesukiRecord::ISScheme isscheme;

      /** 詰めろな手に関する cost */
      int tsumero_cost;

      /** 詰めろな手に関する証明数の予測値 */
      int tsumero_estimate;

      /** GC の際にテーブル をどこまで小さくするか */
      double gc_ratio;

      template<class Search,Player T> class AttackHelper;
      template<class Search,Player T> class DefenseHelper;
      template<class Search,Player T> class CallSimulationAttack;
      template<class Search,Player T> class CallSimulationDefense;
      template<class Search,Player T> class CallSimulationDefenseDisproof;

      /* statistic information */
      unsigned int blockByAttackBack;
      unsigned int blockByPass;
      unsigned int attack_node_count;
      unsigned int attack_node_under_attack_count;
      unsigned int attack_node_moves_count;
      unsigned int defense_node_count;
      unsigned int defense_node_under_attack_count;
      unsigned int defense_node_moves_count;
      unsigned int pass_count, pass_success_count;
      unsigned int pass_attack_count, pass_attack_success_count;
      unsigned int sibling_defense_count, sibling_defense_success_count;
      unsigned int sibling_attack_count, sibling_attack_success_count;
      unsigned int isshogi_defense_count, isshogi_defense_success_count;
      unsigned int isshogi_attack_count, isshogi_attack_success_count;
      unsigned int immediate_win, immediate_lose;
      unsigned int attack_back_count;
      unsigned int proof_without_inversion_count, proof_AND_count, disproof_by_inversion_count;

    public:
      static const int NtesukiNotFound = -1;
      static const int ReadLimitReached = -2;
      static const int TableLimitReached = -3;
    private:
      static const unsigned int INITIAL_PROOF_LIMIT =
      ProofDisproof::PROOF_LIMIT / 4;
      static const unsigned int INITIAL_DISPROOF_LIMIT =
      ProofDisproof::DISPROOF_LIMIT / 4;

      /**
       * 攻撃側の処理
       */
      template <Player T>
      NtesukiResult attack(NtesukiRecord* record,
			   const NtesukiRecord* oracle_attack,
			   const NtesukiRecord* oracle_defense,
			   unsigned int proofLimit,
			   unsigned int disproofLimit,
			   int pass_left,
			   const Move last_move);
      template <Player T>
      void attackWithOrder(NtesukiRecord* record,
			   const NtesukiRecord* oracle_attack,
			   const NtesukiRecord* oracle_defense,
			   unsigned int proofLimit,
			   unsigned int disproofLimit,
			   int pass_left,
			   const Move last_move);

      template <Player T>
      NtesukiMove* selectMoveAttack(NtesukiRecord* record,
				    unsigned int& best_proof,
				    unsigned int& sum_disproof,
				    unsigned int& second_proof,
				    unsigned int& best_disproof,
				    unsigned int& step_cost,
				    NtesukiMoveList& moves,
				    const int pass_left);

      /**
       * 防御に関する計算
       */
      template <Player T>
      NtesukiResult defense(NtesukiRecord* record,
			    const NtesukiRecord* oracle_attack,
			    const NtesukiRecord* oracle_defense,
			    unsigned int proofLimit,
			    unsigned int disproofLimit,
			    int pass_left,
			    const Move last_move);


      template <Player T>
      void defenseWithPlayer(NtesukiRecord* record,
			     const NtesukiRecord* oracle_attack,
			     const NtesukiRecord* oracle_defense,
			     unsigned int proofLimit,
			     unsigned int disproofLimit,
			     int pass_left,
			     const Move last_move);

      template <Player T>
      NtesukiMove* selectMoveDefense(NtesukiRecord* record,
				     unsigned int& best_disproof,
				     unsigned int& sum_proof,
				     unsigned int& second_disproof,
				     unsigned int& best_proof,
				     unsigned int& step_cost,
				     NtesukiMoveList& moves,
				     const int pass_left,
				     const Move last_move);

      /**
       * 受け手番で，ある手が Success だとわかった場合，
       * 他の手も同様に Success にならないか Simulaition で調べる.
       * 現在 $\lambda^n$ 探索中の場合には，原則 $\lambda^(n-1)$ move の場合には調べない.
       *
       * e.g. 必至探索中には王手は調べない.
       */
      template <Player T> void simulateSiblingsSuccess(NtesukiRecord *record,
						       NtesukiRecord *record_best,
						       int pass_left,
						       unsigned int& success_count,
						       unsigned int& total_count);

      /**
       * 攻め手番で，ある手が Fail だとわかった場合，
       * 他の手も同様に Fail にならないか Simulaition で調べる.
       */
      template <Player T> void simulateSiblingsFail(NtesukiRecord *record,
						    NtesukiRecord *record_best,
						    int pass_left,
						    unsigned int& success_count,
						    unsigned int& total_count);
      /**
       * 攻めになっていない手を除く.
       */
      template <Player T> void handleNonAttack(NtesukiRecord *record,
					       int pass_left);
      /**
       * 頓死がないか調べる.
       */
      template <Player T> void handleTonshi(NtesukiRecord *record,
					    int pass_left,
					    const Move last_move);
      /**
       * 無駄合候補が本当に無駄合が調べる.
       */
      template <Player T> void handleInterpose(NtesukiRecord *record,
					       int pass_left);

    public:
      /* ======================
       * 外から呼ばれる関数
       */
      NtesukiSearcher(State& state,
		      NtesukiMoveGenerator *mg,
		      unsigned int table_limit,
		      volatile int *stop_flag,
		      bool verbose,
		      int maxPass = NtesukiRecord::SIZE,
		      NtesukiRecord::IWScheme iwscheme = NtesukiRecord::pn_iw,
		      NtesukiRecord::PSScheme psscheme = NtesukiRecord::no_ps,
		      NtesukiRecord::ISScheme isscheme = NtesukiRecord::no_is,
		      int tsumero_cost = 0,
		      int tsumero_estimate = 0,
		      double gc_ratio = 0.33);
      ~NtesukiSearcher();


      template <Player T> int search();

      int searchSlow(Player attacker, int rnl = 160000)
      {
	read_node_limit = rnl;
	if (attacker == BLACK)
	  return search<BLACK>();
	else
	  return search<WHITE>();
      }

      NtesukiTable& getTable();
      int getNodeCount() const { return node_count; }
      bool exceedReadNodeLimit() const {return node_count > read_node_limit;}

    };
  } //ntesuki
} //osl

#endif /* _NTESUKI_SEACHER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
