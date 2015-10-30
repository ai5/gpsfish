#include "osl/ntesuki/ntesukiSimulationSearcherProof.tcc"
#include "osl/ntesuki/ntesukiSimulationSearcherDisproof.tcc"
#include "osl/ntesuki/ntesukiMoveGenerator.h"

typedef NumEffectState state_t;

/* Constructor/ Destructor
 */
osl::ntesuki::NtesukiSimulationSearcher::
NtesukiSimulationSearcher(state_t& state,
			  NtesukiMoveGenerator *mg,
			  PathEncoding&
			  path,
			  NtesukiTable& table,
			  NtesukiRecord::ISScheme isscheme,
			  bool verbose)
  : node_count(0),
    verbose(verbose),
    proof_count(0),
    proof_success_count(0),
    light_proof_success_count(0),
    disproof_count(0),
    disproof_success_count(0),
    debug(false),
    state(state),
    mg(mg),
    path(path),
    table(table),
    isscheme(isscheme)
{
}

osl::ntesuki::NtesukiSimulationSearcher::
~NtesukiSimulationSearcher()
{
  if (verbose)
    std::cerr << "~NtesukiSimulationSeacher:\t("
	      << node_count
	      << ")\tproof("
	      << light_proof_success_count << "/"
	      << proof_success_count << "/"
	      << proof_count
	      << ")\tdisproof("
	      << disproof_success_count << "/"
	      << disproof_count
	      << ")" << std::endl;
}

template bool osl::ntesuki::NtesukiSimulationSearcher::
startFromAttackProof<BLACK>(NtesukiRecord *record,
			    const NtesukiRecord* record_orig,
			    const unsigned int passLeft,
			    const Move last_move);

template bool osl::ntesuki::NtesukiSimulationSearcher::
startFromAttackProof<WHITE>(NtesukiRecord *record,
			    const NtesukiRecord* record_orig,
			    const unsigned int passLeft,
			    const Move last_move);

template bool osl::ntesuki::NtesukiSimulationSearcher::
startFromDefenseProof<BLACK>(NtesukiRecord *record,
			     const NtesukiRecord* record_orig,
			     const unsigned int passLeft,
			     const Move last_move);

template bool osl::ntesuki::NtesukiSimulationSearcher::
startFromDefenseProof<WHITE>(NtesukiRecord *record,
			     const NtesukiRecord* record_orig,
			     const unsigned int passLeft,
			     const Move last_move);

template bool osl::ntesuki::NtesukiSimulationSearcher::
startFromAttackDisproof<BLACK>(NtesukiRecord *record,
			       const NtesukiRecord* record_orig,
			       const unsigned int passLeft,
			       const Move last_move);

template bool osl::ntesuki::NtesukiSimulationSearcher::
startFromAttackDisproof<WHITE>(NtesukiRecord *record,
			       const NtesukiRecord* record_orig,
			       const unsigned int passLeft,
			       const Move last_move);

template bool osl::ntesuki::NtesukiSimulationSearcher::
startFromDefenseDisproof<BLACK>(NtesukiRecord *record,
				const NtesukiRecord* record_orig,
				const unsigned int passLeft,
				const Move last_move);

template bool osl::ntesuki::NtesukiSimulationSearcher::
startFromDefenseDisproof<WHITE>(NtesukiRecord *record,
				const NtesukiRecord* record_orig,
				const unsigned int passLeft,
				const Move last_move);

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
