#include "osl/ntesuki/oracleProverLight.h"
#include "osl/ntesuki/oracleProverLight.tcc"
#include "osl/ntesuki/ntesukiMoveGenerator.h"

typedef NumEffectState state_t;

osl::ntesuki::OracleProverLight::
OracleProverLight(state_t& s,
		  gen_t *g,
		  PathEncoding p,
		  table_t& t,
		  NtesukiRecord::ISScheme ischeme);

template bool osl::ntesuki::OracleProverLight::
startFromAttack<BLACK>(NtesukiRecord *record,
		       const NtesukiRecord* record_orig,
		       const unsigned int pass_left);
template bool osl::ntesuki::OracleProverLight::
startFromAttack<WHITE>(NtesukiRecord *record,
		       const NtesukiRecord* record_orig,
		       const unsigned int pass_left);


template bool osl::ntesuki::OracleProverLight::
startFromDefense<BLACK>(NtesukiRecord *record,
			const NtesukiRecord* record_orig,
			const unsigned int pass_left);
template bool osl::ntesuki::OracleProverLight::
startFromDefense<WHITE>(NtesukiRecord *record,
			const NtesukiRecord* record_orig,
			const unsigned int pass_left);


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
