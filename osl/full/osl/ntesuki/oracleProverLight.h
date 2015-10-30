/* oracleProverLight.h
 */
#ifndef _NTESUKI_ORACLE_PROVER_LIGHT_H
#define _NTESUKI_ORACLE_PROVER_LIGHT_H

#include "osl/ntesuki/ntesukiTable.h"
#include "osl/state/numEffectState.h"
#include "osl/hash/hashKey.h"
#include "osl/player.h"
#include "osl/pathEncoding.h"

namespace osl
{
  namespace ntesuki
  {
    class OracleProverLight
    {
    public:
      typedef NtesukiTable table_t;
      typedef NumEffectState state_t;
      typedef NtesukiMoveGenerator gen_t;
    private:
      state_t& state;
      gen_t *mg;
      PathEncoding path;
      table_t& table;
      NtesukiRecord::ISScheme isscheme;
      int fixed_search_depth;

      template<class Searcher, Player P> class AttackHelper;
      template<class Searcher, Player P> class DefenseHelper;

    public:
      explicit OracleProverLight(state_t& s,
				 gen_t *g,
				 PathEncoding p,
				 table_t& t,
				 NtesukiRecord::ISScheme isscheme = NtesukiRecord::no_is)
	: state(s), mg(g), path(p), table(t), isscheme(isscheme)
      {
      }
      
      template <Player P>
      bool startFromAttack(NtesukiRecord* record,
			   const NtesukiRecord* record_orig,
			   const unsigned int pass_left);
      template <Player P>
      bool startFromDefense(NtesukiRecord* record,
			    const NtesukiRecord* record_orig,
			    const unsigned int pass_left);

    private:
      template <Player P>
      bool attack(const NtesukiRecord *oracle,
		  const unsigned int pass_left);
      template <Player P>
      bool defense(const NtesukiRecord *oracle,
		   const unsigned int pass_left);

    };
  } // namespace ntesuki
} // namespace osl

#endif /* _NTESUKI_ORACLE_PROVER_LIGHT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
