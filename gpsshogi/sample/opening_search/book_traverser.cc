#include "book_traverser.h"
#include "gpsshogi/redis/redis.h"
#include "gpsshogi/redis/searchResult.h"
#include "osl/hash/hashKey.h"
#include "osl/state/simpleState.h"
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <glog/logging.h>
#include <algorithm>
#include <deque>
#include <vector>

const std::string getStateKey(const osl::SimpleState& state) {
  const osl::record::CompactBoard cb(state);
  return gpsshogi::redis::compactBoardToString(cb);
}

const Node
Node::nextNode(const Node current_node,
               int next_state_index,
               const std::string& next_state_str,
               const osl::Move move) {
  Node::moves_t moves = current_node.moves;
  moves.push_back(move);
  return Node(next_state_index, next_state_str, moves);
}

void BookTraverser::selectNextMoves(const Node& node, WMoveContainer& moves) const
{
  const osl::SimpleState state(book.getBoard(node.state_index));
  moves = book.getMoves(node.state_index);
  std::sort(moves.begin(), moves.end(), osl::record::opening::WMoveSort());

  /*
   * 自分（the_player）の手番では、有望な手(weight>0)のみ抽出する
   * 相手はどんな手を指すか分からないので、特にfilterせずに、そのまま。
   */
  if (!moves.empty() && state.turn() == visitor.getPlayer()) {
    int min = 1;
    if (is_determinate) {
      min = moves.at(0).getWeight();
      if (node.getDepth() <= non_determinate_depth) {
        for (int i=1; i<=std::min(is_determinate, (int)moves.size()-1); ++i) {
          const int weight = moves.at(i).getWeight();
          if ((double)weight < (double)moves.at(i-1).getWeight()*ratio)
            break;
          min = weight;
        }
      }
    }
    // Do not play 0-weighted moves.
    if (min == 0) min = 1;

    WMoveContainer::iterator each = moves.begin();
    for (; each != moves.end(); ++each) {
      if (each->getWeight() < min)
        break;
    }
    moves.erase(each, moves.end());
  }
  DLOG(INFO) << boost::format("  #moves... %d\n") % moves.size();
}

void BookTraverser::traverse()
{
  LOG(INFO) << boost::format("Total states: %d") % book.getTotalState();
  bool states[book.getTotalState()]; // mark states that have been visited.
  memset(states, 0, sizeof(states));

  std::vector<Node> stateToVisit;

  LOG(INFO) << boost::format("Start index: %d") % book.getStartState();
  const Node root_node(book.getStartState(),
                       getStateKey(book.getBoard(book.getStartState())));
  stateToVisit.push_back(root_node);

  while (!stateToVisit.empty()) {
    const Node node = stateToVisit.back(); // DFS
    stateToVisit.pop_back();
    states[node.state_index] = true;
    DLOG(INFO) << boost::format("Visiting... %d") % node.state_index;

    /* この局面を処理する */
    const osl::SimpleState state(book.getBoard(node.state_index));
    if (state.turn() == osl::alt(visitor.getPlayer())) {
      // 黒の定跡を評価したい -> 黒の手が指されたあとの局面
      //                      -> 白手番の局面をサーバに登録する
      visitor.notify(node);
    }

    WMoveContainer moves;
    selectNextMoves(node, moves);
    /* leaf nodes */
    if (moves.empty() || node.getDepth() > max_depth) {
      continue;
    }
    // recursively search the tree
    BOOST_FOREACH(osl::record::opening::WMove& each_move, moves) {
      // consistancy check
      const osl::hash::HashKey hash(state);
      const int nextIndex = each_move.getStateIndex();
      const osl::SimpleState next_state(book.getBoard(nextIndex));
      const osl::hash::HashKey next_hash(next_state);
      if (next_hash != hash.newMakeMove(each_move.getMove()))
        throw std::string("Illegal move found.");

      if (!states[nextIndex]) {
	stateToVisit.push_back(Node::nextNode(node,
                                              nextIndex,
                                              getStateKey(next_state),
                                              each_move.getMove()));
      }
    } // each wmove
  } // while
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
