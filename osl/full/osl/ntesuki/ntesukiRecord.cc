#include "osl/ntesuki/ntesukiRecord.h"
#include "osl/ntesuki/ntesukiRecord.tcc"
#include "osl/player.h"
#include <iostream>
#include <algorithm>


unsigned int osl::ntesuki::NtesukiRecord::
fixed_search_depth = 0;

unsigned int osl::ntesuki::NtesukiRecord::
inversion_cost = 0;

bool osl::ntesuki::NtesukiRecord::
use_dominance = false;

osl::ntesuki::NtesukiTable *
osl::ntesuki::NtesukiRecord::table = NULL;

osl::NumEffectState *
osl::ntesuki::NtesukiRecord::state = NULL;

osl::ntesuki::NtesukiMoveGenerator *
osl::ntesuki::NtesukiRecord::mg = NULL;

int osl::ntesuki::NtesukiRecord::
pass_count = 0;

bool osl::ntesuki::NtesukiRecord::
max_for_split = false;

bool osl::ntesuki::NtesukiRecord::
use_rzone_move_generation= false;

bool osl::ntesuki::NtesukiRecord::
delay_lame_long = false;

bool osl::ntesuki::NtesukiRecord::
use_9rzone = false;

unsigned int osl::ntesuki::NtesukiRecord::
split_count = 0;

unsigned int osl::ntesuki::NtesukiRecord::
confluence_count = 0;

/* constructor
 */
osl::ntesuki::NtesukiRecord::
NtesukiRecord(signed short distance,
	      const HashKey& key,
	      const PieceStand& white_stand,
	      RecordList* same_board_list)
  :  black_stand(key.getPieceStand()), white_stand(white_stand),
     distance(distance), key(key), same_board_list(same_board_list),
     rev_refcount(0),
     child_count(0), read_count(0), written_count(0),
     visited(false), by_simulation(false),
     by_fixed_black(false), by_fixed_white(false),
     already_set_up(false),
     final(false), is_split(false),
     do_oracle_attack(true),
     do_oracle_aunt(true),
     rzone_move_generation(use_rzone_move_generation)
{
  std::fill(values_black.begin(), values_black.end(), ProofDisproof(1, 1));
  std::fill(values_white.begin(), values_white.end(), ProofDisproof(1, 1));

  std::fill(read_interpose.begin(), read_interpose.end(), false);
  std::fill(read_check_defense.begin(), read_check_defense.end(), false);
  std::fill(read_non_attack.begin(), read_non_attack.end(), false);

  std::fill(is_ntesuki_black.begin(), is_ntesuki_black.end(), false);
  std::fill(is_ntesuki_white.begin(), is_ntesuki_white.end(), false);

  std::fill(propagated_oracle_black.begin(), propagated_oracle_black.end(), false);
  std::fill(propagated_oracle_white.begin(), propagated_oracle_white.end(), false);

  std::fill(use_old_black.begin(), use_old_black.end(), false);
  std::fill(use_old_white.begin(), use_old_white.end(), false);

  if (use_dominance)
  {
    lookup_same_board_list();
  }
}

void
osl::ntesuki::NtesukiRecord::
updateWithChild(osl::ntesuki::NtesukiRecord* child,
		int pass_left)
{
  for (unsigned int i = pass_left; i < SIZE; i++)
  {
    rzone<BLACK>()[i].update(child->rzone<BLACK>()[i]);
    rzone<WHITE>()[i].update(child->rzone<WHITE>()[i]);
  }
}
void
osl::ntesuki::NtesukiRecord::
lookup_same_board_list()
{
  for (RecordList::iterator it = same_board_list->begin();
       it != same_board_list->end(); it++)
  {
    if (&(*it) == this) continue;
    
    for (size_t pass_left = 0; pass_left < SIZE; pass_left++)
    {
      if (isDominatedByProofPieces<BLACK>(&(*it), pass_left))
      {
	PieceStand ps = it->getPDPieces<BLACK>(pass_left);
	TRY_DFPN;
	setResult<BLACK>(pass_left, it->getValue<BLACK>(pass_left),
			 it->getBestMove<BLACK>(pass_left), false, &ps);
	CATCH_DFPN;
	return;	
      }
      else if (isDominatedByProofPieces<WHITE>(&(*it), pass_left))
      {
	PieceStand ps = it->getPDPieces<WHITE>(pass_left);
	TRY_DFPN;
	setResult<WHITE>(pass_left, it->getValue<WHITE>(pass_left),
			 it->getBestMove<WHITE>(pass_left), false, &ps);
	CATCH_DFPN;
	return;
      }
    }
    for (int pass_left = (int)SIZE - 1; pass_left >= 0; pass_left--)
    {
      if (isDominatedByDisproofPieces<BLACK>(&(*it), pass_left))
      {
	PieceStand ps = it->getPDPieces<BLACK>(pass_left);
	TRY_DFPN;
	setResult<BLACK>(pass_left, it->getValue<BLACK>(pass_left),
			 it->getBestMove<BLACK>(pass_left), false, &ps);
	CATCH_DFPN;
	return;
      }
      else if (isDominatedByDisproofPieces<WHITE>(&(*it), pass_left))
      {
	PieceStand ps = it->getPDPieces<WHITE>(pass_left);
	const NtesukiMove& best_move = it->getBestMove<WHITE>(pass_left);
	TRY_DFPN;
	setResult<WHITE>(pass_left, it->getValue<WHITE>(pass_left),
			 best_move, false, &ps);
	CATCH_DFPN;
	return;
      }
    }
  }
}


/* =============================================================================
 * 'slow' accessors
 */
const osl::ntesuki::NtesukiResult
osl::ntesuki::NtesukiRecord::
getValueSlow(const Player player, int i) const
{
  if (BLACK == player)
    return  values<BLACK>()[i];
  else
    return  values<WHITE>()[i];
}

const osl::ntesuki::NtesukiResult
osl::ntesuki::NtesukiRecord::
getValueOfTurn(int i) const
{
  return getValueSlow(turn(), i);
}

const osl::ntesuki::NtesukiResult
osl::ntesuki::NtesukiRecord::
valueBeforeFinal() const
{
  return value_before_final;
}

const osl::ntesuki::NtesukiMove&
osl::ntesuki::NtesukiRecord::
getBestMoveSlow(Player P, int i) const
{
  if (BLACK == P)
    return getBestMove<BLACK>(i);
  else
    return getBestMove<WHITE>(i);
}

bool
osl::ntesuki::NtesukiRecord::
isByFixedSlow(Player P) const
{
  if (P == BLACK)
    return isByFixed<BLACK>();
  else
    return isByFixed<WHITE>();
}

osl::PieceStand
osl::ntesuki::NtesukiRecord::
getPDPiecesSlow(Player p, int pass_left) const
{
  if (p == BLACK)
    return pdpieces<BLACK>()[pass_left];
  else
    return pdpieces<WHITE>()[pass_left];
}

/* =============================================================================
 * explicit instantiation
 */
namespace osl
{
  namespace ntesuki
  {
    template
    void NtesukiRecord::
    setResult<BLACK>(int i,
		     const NtesukiResult& r,
		     const NtesukiMove& m,
		     bool bs,
		     const PieceStand* ps);
    template
    void NtesukiRecord::
    setResult<WHITE>(int i,
		     const NtesukiResult& r,
		     const NtesukiMove& m,
		     bool bs,
		     const PieceStand* ps);

    template
    bool NtesukiRecord::
    setUpNode<BLACK>();

    template
    bool NtesukiRecord::
    setUpNode<WHITE>();

    template
    void NtesukiRecord::
    generateMoves<BLACK>(NtesukiMoveList& moves,
			 int pass_left,
			 bool all_moves);

    template
    void NtesukiRecord::
    generateMoves<WHITE>(NtesukiMoveList& moves,
			 int pass_left,
			 bool all_moves);

    template
    bool NtesukiRecord::
    isNtesuki<BLACK>(int pass_left) const;

    template
    bool NtesukiRecord::
    isNtesuki<WHITE>(int pass_left) const;

    template
    void NtesukiRecord::
    setNtesuki<BLACK>(int pass_left);

    template
    void NtesukiRecord::
    setNtesuki<WHITE>(int pass_left);

    template
    bool NtesukiRecord::
    hasTriedPropagatedOracle<BLACK>(int pass_left) const;

    template
    bool NtesukiRecord::
    hasTriedPropagatedOracle<WHITE>(int pass_left) const;

    template
    void NtesukiRecord::
    triedPropagatedOracle<BLACK>(int pass_left);

    template
    void NtesukiRecord::
    triedPropagatedOracle<WHITE>(int pass_left);
    
    template
    bool NtesukiRecord::
    useOld<BLACK>(int pass_left) const;

    template
    bool NtesukiRecord::
    useOld<WHITE>(int pass_left) const;

    template
    void NtesukiRecord::
    setUseOld<BLACK>(int pass_left, bool b);

    template
    void NtesukiRecord::
    setUseOld<WHITE>(int pass_left, bool b);
    
    template
    PieceStand NtesukiRecord::
    getPDPieces<BLACK>(int pass_left) const;

    template
    PieceStand NtesukiRecord::
    getPDPieces<WHITE>(int pass_left) const;

    template
    void NtesukiRecord::
    setPDPieces<BLACK>(int pass_left,
		       const PieceStand p);
    template
    void NtesukiRecord::
    setPDPieces<WHITE>(int pass_left,
		       const PieceStand p);

    template
    bool NtesukiRecord::
    isLoopWithPath<BLACK>(int pass_left,
			  const PathEncoding& path) const;
    template
    bool NtesukiRecord::
    isLoopWithPath<WHITE>(int pass_left,
			  const PathEncoding& path) const;

    template
    void  NtesukiRecord::
    setLoopWithPath<BLACK>(int pass_left,
			   const PathEncoding& path);

    template
    void  NtesukiRecord::
    setLoopWithPath<WHITE>(int pass_left,
			   const PathEncoding& path);

    template
    const NtesukiResult NtesukiRecord::
    getValueWithPath<BLACK>(int max_pass_left, const PathEncoding path) const;

    template
    const NtesukiResult NtesukiRecord::
    getValueWithPath<WHITE>(int max_pass_left, const PathEncoding pat) const;

    template
    const NtesukiResult NtesukiRecord::
    getValueOr<BLACK>(int max_pass_left, const PathEncoding path,
		      IWScheme iwscheme) const;

    template
    const NtesukiResult NtesukiRecord::
    getValueOr<WHITE>(int max_pass_left, const PathEncoding pat,
		      IWScheme iwscheme) const;

    template
    const NtesukiResult NtesukiRecord::
    getValueAnd<BLACK>(int max_pass_left, const PathEncoding pat,
		       IWScheme iwscheme, PSScheme psscheme) const;

    template
    const NtesukiResult NtesukiRecord::
    getValueAnd<WHITE>(int max_pass_left, const PathEncoding pat,
		       IWScheme iwscheme, PSScheme psscheme) const;


    std::ostream&
    operator<<(std::ostream& os, const NtesukiRecord& record)
    {
      os << "player:\t"  << record.key.turn() << "\n"
	 << "visited:\t"  << record.isVisited() << "\n"
	 << "distance:\t"  << record.distance << "\n"
	 << "subtree:\t" << record.getChildCount() << "\n"
	 << record.key << "\nBS"
	 << record.black_stand << "\nWS"
	 << record.white_stand << "\n";


      for(size_t i = 0; i < NtesukiRecord::SIZE; ++i)
      {
	os << i << " B\tmove("
	   << record.getBestMove<BLACK>(i) << ")\t"
	   << record.getValue<BLACK>(i) << "\tdom("
	   << record.getPDPieces<BLACK>(i)
	   << "\n"
	   << i << " W\tmove("
	   << record.getBestMove<WHITE>(i) << ")\t"
	   << record.getValue<WHITE>(i) << "\tdom("
	   << record.getPDPieces<WHITE>(i)
	   << "\n";
      }

      return os;
    }

    std::ostream&
    operator<<(std::ostream& os,
	       const NtesukiRecord::IWScheme& s)
    {
      switch (s)
      {
      case NtesukiRecord::no_iw: os << "no_widening";
	break;
      case NtesukiRecord::strict_iw: os << "iterative_widening";
	break;
      case NtesukiRecord::pn_iw: os << "pnbased_widening";
	break;
      default:
	throw std::runtime_error("cannot parse string");
      }
      return os;
    }

    std::istream&
    operator>>(std::istream& is,
	       NtesukiRecord::IWScheme& s)
    {
      std::string token;
      is >> token;

      switch (token[0])
      {
      case 'n':
	s = NtesukiRecord::no_iw;
	break;
      case 'i':
	s = NtesukiRecord::strict_iw;
	break;
      case 'p':
	s = NtesukiRecord::pn_iw;
	break;
      default:
	throw std::runtime_error("cannot parse string");
      }
      return is;
    }

    std::ostream&
    operator<<(std::ostream& os,
	       const NtesukiRecord::PSScheme& s)
    {
      switch (s)
      {
      case NtesukiRecord::no_ps: os << "single_lambda";
	break;
      case NtesukiRecord::pn_ps: os << "dual_lambda";
	break;
      default:
	throw std::runtime_error("cannot parse string");
      }
      return os;
    }

    std::istream&
    operator>>(std::istream& is,
	       NtesukiRecord::PSScheme& s)
    {
      std::string token;
      is >> token;

      switch (token[0])
      {
      case 'n':
      case 's':
	s = NtesukiRecord::no_ps;
	break;
      case 'p':
      case 'd':
	s = NtesukiRecord::pn_ps;
	break;
      default:
	throw std::runtime_error("cannot parse string");
      }
      return is;
    }

    std::ostream&
    operator<<(std::ostream& os,
	       const NtesukiRecord::ISScheme& s)
    {
      switch (s)
      {
      case NtesukiRecord::no_is: os << "katagyoku";
	break;
      case NtesukiRecord::tonshi_is: os << "tonshi-only";
	break;
      case NtesukiRecord::delay_is: os << "delay-inversion";
	break;
      case NtesukiRecord::normal_is: os << "full-inversion";
	break;
      default:
	throw std::runtime_error("cannot parse string");
      }

      return os;
    }

    std::istream&
    operator>>(std::istream& is,
	       NtesukiRecord::ISScheme& s)
    {
      std::string token;
      is >> token;

      switch (token[0])
      {
      case 'n':
	s = NtesukiRecord::no_is;
	break;
      case 't':
	s = NtesukiRecord::tonshi_is;
	break;
      case 'd':
	s = NtesukiRecord::delay_is;
	break;
      case 'f':
	s = NtesukiRecord::normal_is;
	break;
      default:
	throw std::runtime_error("cannot parse string");
      }
      return is;
    }

  }
}
