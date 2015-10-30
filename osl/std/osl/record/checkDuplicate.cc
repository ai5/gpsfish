#include "osl/record/checkDuplicate.h"
#include "osl/numEffectState.h"
#include <boost/format.hpp>
#include <iostream>

std::pair<osl::HashKey,osl::PathEncoding> osl::record::
CheckDuplicate::getLastState(const std::vector<Move>& moves)
{
  NumEffectState state;
  PathEncoding path(BLACK);
  
  for (Move move: moves) {
    state.makeMove(move);
    path.pushMove(move);
    assert(state.isConsistent(true));
  }
  return std::make_pair(HashKey(state), path);
}

osl::record::CheckDuplicate::DUPLICATE_RESULT osl::record::
CheckDuplicate::regist(const std::vector<Move>& moves)
{
  const std::pair<HashKey, PathEncoding> pair = getLastState(moves);
  return regist(pair.first, pair.second);
}

osl::record::CheckDuplicate::DUPLICATE_RESULT osl::record::
CheckDuplicate::regist(const HashKey& key, 
                       const PathEncoding& moves)
{
  ++regist_counter;

  std::vector<PathEncoding>& rs = keys[key];
  if (rs.empty())
  {
    // not found. i.e. a new key
    rs.push_back(moves);
    return NO_DUPLICATE;
  }
  else
  {
    if (std::find(rs.begin(), rs.end(), moves)
	== rs.end())
    {
      // new moves
      ++duplicated_hash_counter;
      rs.push_back(moves);
      return HASH_DUPLICATE;
    }
    else
    {
      // hit 
      ++duplicated_moves_counter;
      return MOVES_DUPLICATE;
    }
  }
}

void osl::record::
CheckDuplicate::print(std::ostream& out) const
{
  out << boost::format("Trials %d, Unique %d, Duplicates Hash %d, Duplicated moves %d\n")
    % regist_counter 
    % keys.size() 
    % duplicated_hash_counter 
    % duplicated_moves_counter;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
