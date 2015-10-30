#ifndef GPSSOGI_OPENING_SEARCH_BOOK_TRAVERSER_H
#define GPSSOGI_OPENING_SEARCH_BOOK_TRAVERSER_H

#include "osl/move.h"
#include "osl/player.h"
#include "osl/state/simpleState.h"
#include "osl/record/opening/openingBook.h"
#include <string>
#include <vector>

const std::string getStateKey(const osl::SimpleState& state);

struct Node
{
  typedef std::vector<osl::Move> moves_t;

  int state_index;
  std::string state_key;
  moves_t moves;

  Node(int _state_index,
       const std::string& _state_key)
    : state_index(_state_index),
      state_key(_state_key)
  {}

  Node(int _state_index,
       const std::string& _state_key,
       const moves_t& _moves)
    : state_index(_state_index),
      state_key(_state_key),
      moves(_moves)
  {}

  // depth-1手目からdepth手目のstate。depth手目はまだ指されていない（これか
  // らdepth手目）
  int getDepth() const {
    return moves.size() + 1;
  }

  static const Node nextNode(const Node current_node,
                             int next_state_index,
                             const std::string& next_state_str,
                             const osl::Move move);
};


class AbstractVisitor
{
  const osl::Player player;
  osl::record::opening::WeightedBook *book;

public:
  AbstractVisitor(osl::Player _player)
    : player(_player)
  {}
  virtual ~AbstractVisitor() {}

  osl::Player getPlayer() const { return player; }
  virtual void notify(const Node& node) = 0;
  void setBook(osl::record::opening::WeightedBook *_book) {book = _book;}
};


class BookTraverser
{
public:
  typedef std::vector<osl::record::opening::WMove> WMoveContainer;

private:
  osl::record::opening::WeightedBook& book;
  AbstractVisitor& visitor;
  const int is_determinate;
  const int max_depth;
  const int non_determinate_depth;
  const double ratio;

public:
  BookTraverser(osl::record::opening::WeightedBook& _book,
                AbstractVisitor& _visitor,
                int _is_determinate,
                int _max_depth,
                int _non_determinate_depth,
                double _ratio)
    : book(_book),
      visitor(_visitor),
      is_determinate(_is_determinate),
      max_depth(_max_depth),
      non_determinate_depth(_non_determinate_depth),
      ratio(_ratio)
  {
    visitor.setBook(&book);
  }

  void traverse();

private:
  void selectNextMoves(const Node& node, WMoveContainer& moves) const;
};

#endif /* GPSSOGI_OPENING_SEARCH_BOOK_TRAVERSER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
