#include "osl/ntesuki/ntesukiSearcher.tcc"
#include <climits>

bool osl::ntesuki::NtesukiSearcher::
delay_non_pass = false;

bool osl::ntesuki::NtesukiSearcher::
ptt_invalid_defense = false;

bool osl::ntesuki::NtesukiSearcher::
delay_interpose = false;

bool osl::ntesuki::NtesukiSearcher::
delay_nopromote = false;

bool osl::ntesuki::NtesukiSearcher::
delay_non_attack = false;

bool osl::ntesuki::NtesukiSearcher::
read_attack_only = false;

bool osl::ntesuki::NtesukiSearcher::
ptt_non_attack = false;

bool osl::ntesuki::NtesukiSearcher::
ptt_siblings_fail = false;

bool osl::ntesuki::NtesukiSearcher::
ptt_siblings_success = false;

bool osl::ntesuki::NtesukiSearcher::
ptt_uncle = false;

bool osl::ntesuki::NtesukiSearcher::
ptt_aunt = false;

unsigned int osl::ntesuki::NtesukiSearcher::
dynamic_widening_width = 0;

/* ===================
 * Constructor / Destructor
 */
osl::ntesuki::NtesukiSearcher::
NtesukiSearcher(State& state,
		NtesukiMoveGenerator *mg,
		unsigned int table_limit,
		volatile int *stop_flag,
		bool verbose,
		int max_pass,
		NtesukiRecord::IWScheme iwscheme,
		NtesukiRecord::PSScheme psscheme,
		NtesukiRecord::ISScheme isscheme,
		int tsumero_cost,
		int tsumero_estimate,
		double gc_ratio)
  : state(state),
    mg(mg),
    table(table_limit,
	  static_cast<unsigned int>(table_limit * gc_ratio),
	  verbose),
    simulator(state, mg, path, table, isscheme, verbose),
    node_count(0),
    verbose(verbose),
    stop_flag(stop_flag),
    path(state.turn()),
    /* control on search */
    max_pass(max_pass),
    iwscheme(iwscheme),
    psscheme(psscheme),
    isscheme(isscheme),
    tsumero_cost(tsumero_cost),
    tsumero_estimate(tsumero_estimate),
    gc_ratio(gc_ratio),
    /* statistical information */
    blockByAttackBack(0), blockByPass(0),
    attack_node_count(0),
    attack_node_under_attack_count(0),
    attack_node_moves_count(0),
    defense_node_count(0),
    defense_node_under_attack_count(0),
    defense_node_moves_count(0),
    pass_count(0), pass_success_count(0),
    pass_attack_count(0), pass_attack_success_count(0),
    sibling_defense_count(0), sibling_defense_success_count(0),
    sibling_attack_count(0), sibling_attack_success_count(0),
    isshogi_defense_count(0), isshogi_defense_success_count(0),
    isshogi_attack_count(0), isshogi_attack_success_count(0),
    immediate_win(0), immediate_lose(0),
    attack_back_count(0),
    proof_without_inversion_count(0), proof_AND_count(0), disproof_by_inversion_count(0)
{
  NtesukiRecord::table = &table;
  NtesukiRecord::state = &state;
  NtesukiRecord::mg = mg;
  NtesukiRecord::split_count = 0;
  NtesukiRecord::confluence_count = 0;
  
  if (this->max_pass > (int)NtesukiRecord::SIZE)
    this->max_pass = NtesukiRecord::SIZE;

  if (verbose)
  {
    std::cerr << "NtesukiSearcher \n"
	      << "IWScheme:\t" << iwscheme << "\n"
	      << "PSScheme:\t" << psscheme << "\n"
	      << "ISScheme:\t" << isscheme << "\n"
	      << "Fixed:\t" << NtesukiRecord::fixed_search_depth<< "\n"
	      << "Tsumero cost:\t" << tsumero_cost << "\n"
	      << "Tsumero estimate:\t" << tsumero_estimate << "\n"
	      << "Inversion cost:\t" << NtesukiRecord::inversion_cost << "\n"
      ;
    std::cerr << "enhancements: ";
    if(NtesukiSearcher::ptt_uncle)
      std::cerr << " PTT_UNCLE";
    if(NtesukiSearcher::ptt_siblings_fail)
      std::cerr << " PTT_SIBLINGS_FAIL";
    if(NtesukiSearcher::ptt_siblings_success)
      std::cerr << " PTT_SIBLINGS_SUCCESS";
    if(NtesukiSearcher::delay_non_pass)
      std::cerr << " DELAY_NON_PASS";
    if (NtesukiSearcher::ptt_invalid_defense)
      std::cerr << " PASS_SIMULATION";
    if (NtesukiSearcher::delay_interpose)
      std::cerr << " DELAY_INTERPOSE";
    if(NtesukiSearcher::delay_nopromote)
      std::cerr << " DELAY_NOPROMOTE";
    if(NtesukiSearcher::delay_non_attack)
      std::cerr << " DELAY_NON_ATTACK";
    if(NtesukiSearcher::read_attack_only)
      std::cerr << " READ_ATTACK_ONLY";
    if(NtesukiSearcher::ptt_non_attack)
      std::cerr << " PTT_NON_ATTACK";
    if (NtesukiRecord::use_dominance)
      std::cerr << " USE_DOMINANCE";
    std::cerr << "\n";
  }
}

osl::ntesuki::NtesukiSearcher::
~NtesukiSearcher()
{
  if (verbose)
  {
    std::cerr << "~NtesukiSearcher "
	      << table.size()
	      << "/" << node_count
	      << "/" << read_node_limit << "\t"
	      << "pass(" << pass_success_count << "/" << pass_count << ")\n"

	      << "attack_node\t"
	      << attack_node_under_attack_count << "/"
	      << attack_node_count << "\t"
	      << attack_node_moves_count << "moves\n"

	      << "defense_node\t"
	      << defense_node_under_attack_count << "/"
	      << defense_node_count << "\t"
	      << defense_node_moves_count << "moves\n"
            
	      << "immidate(" << immediate_win << ", "
	      << immediate_lose << ")\n"
      
	      << "attack_back(" << attack_back_count << ")\n"

	      << "sibling_success(" << sibling_defense_success_count
	      << "/" << sibling_defense_count << ")\n"
	      << "sibling_fail(" << sibling_attack_success_count
	      << "/" << sibling_attack_count << ")\n"
      
	      << "is_att(" << isshogi_attack_success_count
	      << "/" << isshogi_attack_count << ")\t"
	      << "is_def(" << isshogi_defense_success_count
	      << "/" << isshogi_defense_count << ")\n"

	      << "inversion_win(" << disproof_by_inversion_count
	      << "/" << proof_AND_count
	      << "/" << proof_without_inversion_count << ")\n"

	      << "DAG\t"
	      << NtesukiRecord::split_count << "/"
	      << NtesukiRecord::confluence_count << "\n"
      ;
  }
}

NtesukiTable& 
osl::ntesuki::NtesukiSearcher::
getTable()
{
  return table;
}

/* explicit instantiation
 */
namespace osl
{
  namespace ntesuki
  {
    template int NtesukiSearcher::search<BLACK>();
    template int NtesukiSearcher::search<WHITE>();
  }
}


