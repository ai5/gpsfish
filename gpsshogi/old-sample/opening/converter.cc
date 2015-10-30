// Convert from DBM to WeightedBook format

#include <stdexcept>
#include <sstream>
#include <map>
#include "osl/record/record.h"
#include "osl/record/compactBoard.h"
#include "osl/apply_move/applyMove.h"
#include "editor.h"

typedef struct data
{
  data(int state_index, osl::vector<osl::record::opening::WMove> moves,
       int blackWinCount, int whiteWinCount)
    : state_index(state_index), moves(moves),
      blackWinCount(blackWinCount), whiteWinCount(whiteWinCount) {
  }
  data() : state_index(-1), blackWinCount(0), whiteWinCount(0) {
  }

  int state_index;
  osl::vector<osl::record::opening::WMove> moves;
  int blackWinCount;
  int whiteWinCount;
} data;

void convertToOpening(const char *db_name, const char *opening_name)
{
  DBM *db = dbm_open(const_cast<char *>(db_name), O_RDONLY, 00644);
  if (!db)
    throw std::runtime_error("Failed to open DBM");
  std::map<std::string, data> indices;

  int n_moves = 0;
  datum key = dbm_firstkey(db);

  // Record index nubmers
  while (key.dptr != NULL)
  {
    datum val = dbm_fetch(db, key);
    assert(val.dptr != NULL);

    std::string key_string((char *)key.dptr, key.dsize);

    std::string str((char *)val.dptr, val.dsize);
    std::istringstream iss(str);
    State state;
    iss >> state;

    data d(0, state.getMoves(),
	   state.getBlackWinCount(), state.getWhiteWinCount());
    indices[key_string] = d;

    n_moves += d.moves.size();

    key = dbm_nextkey(db);
  }

  dbm_close(db);

  int n_states = 0;
  for (std::map<std::string, data>::iterator it = indices.begin();
       it != indices.end(); it++)
  {
    it->second.state_index = n_states++;
  }

  int start_index;
  {
    osl::SimpleState hirateState(osl::HIRATE);
    osl::record::CompactBoard hirateBoard(hirateState);
    std::ostringstream oss(std::ostringstream::out);
    oss << hirateBoard;
    const std::string& key_string = oss.str();

    start_index = indices[key_string].state_index;
  }

  std::ofstream ofs(opening_name);
  osl::record::writeInt(ofs, 1);		// version number
  osl::record::writeInt(ofs, n_states);
  osl::record::writeInt(ofs, n_moves);
  osl::record::writeInt(ofs, start_index);

  n_moves = 0;
  // Write states
  for (std::map<std::string, data>::const_iterator it = indices.begin();
       it != indices.end(); it++)
  {
    int moves = it->second.moves.size();
    osl::record::writeInt(ofs, n_moves);
    osl::record::writeInt(ofs, moves);
    osl::record::writeInt(ofs, it->second.blackWinCount);
    osl::record::writeInt(ofs, it->second.whiteWinCount);

    n_moves += moves;
  }

  // Write moves
  for (std::map<std::string, data>::const_iterator it = indices.begin();
       it != indices.end(); it++)
  {
    osl::record::CompactBoard board;
    std::istringstream iss(it->first);
    iss >> board;
    
    osl::vector<osl::record::opening::WMove> moves = it->second.moves;
    for (unsigned int i = 0; i < moves.size(); i++)
    {

      osl::SimpleState newState(board.getState());
      osl::ApplyMoveOfTurn::doMove(newState, moves[i].getMove());
      osl::record::CompactBoard board(newState);
      std::ostringstream oss(std::ostringstream::out);
      oss << board;

      const std::string& key_string = oss.str();
      int nextIndex = -1; 

      std::map<std::string, data>::iterator it = indices.find(key_string);
      if (it != indices.end())
      {
	nextIndex = it->second.state_index;
      }
  
      osl::record::opening::WMove wmove(moves[i].getMove(),
					nextIndex,
					moves[i].getWeight());
      ofs << wmove;
    }
  }

  // Write compactBoard
  for (std::map<std::string, data>::const_iterator it = indices.begin();
       it != indices.end(); it++)
  {
    ofs.write(it->first.data(), it->first.length());
  }
}
