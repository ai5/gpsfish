/* bookInMemory.cc
 */
#include "osl/book/bookInMemory.h"
#include "osl/book/openingBook.h"
#include "osl/numEffectState.h"
#include "osl/oslConfig.h"
#include <memory>
#include <map>

osl::book::
BookInMemory::BookInMemory(const std::string& filename)
{
  readAll(filename);
}

osl::book::
BookInMemory::~BookInMemory()
{
}

int osl::book::
BookInMemory::readRecursive(const HashKey& key, int index, WeightedBook& book, int depth, int /*parent_visit*/)
{
  const int depth_threshold = 60, visit_threshold = 4, scale = 16;
  const int visit = book.blackWinCount(index)+book.whiteWinCount(index);
  if (depth > depth_threshold || table.find(key) != table.end()
      || visit < visit_threshold)
    return visit;
  const std::vector<book::WMove>& moves = book.moves(index);
  FixedCapacityVector<std::pair<int, Move>, 40> children;
  for (WMove move: moves)
  {
    const HashKey child = key.newMakeMove(move.move);
    const int cv = readRecursive(child, move.stateIndex(), book, depth+1, visit);
    if (cv < visit_threshold || cv*scale < visit || move.weight == 0)
      continue;
    children.push_back(std::make_pair(cv, move.move));
    if (children.size() == children.capacity())
      break;
  }
  std::sort(children.begin(), children.end());
  std::reverse(children.begin(), children.end());
  if (! children.empty()) {
    moves_t& store = table[key];
    store.fill(Move());
    for (size_t i=0; i<children.size(); ++i) {
      store[i] = children[i].second;
      if (i+1 == store.size())
	break;
    }
  }
  return visit;
}

void osl::book::
BookInMemory::readAll(const std::string& filename)
{
  WeightedBook book(OslConfig::openingBook(filename));
  int index = book.startState();
  const NumEffectState state;
  readRecursive(HashKey(state), index, book, 0, 0);
}

void osl::book::
BookInMemory::find(const HashKey& key, MoveVector& out) const
{
  table_t::const_iterator p = table.find(key);
  if (p == table.end())
    return;
  for (Move move: p->second)
    if (move.isNormal())
      out.push_back(move);
}

const osl::book::BookInMemory&
osl::book::
BookInMemory::instance(const std::string& filename)
{
  static std::map<std::string,std::shared_ptr<BookInMemory> > table;
  std::shared_ptr<BookInMemory> &book = table[filename];
  if (! book)
    book.reset(new BookInMemory(filename));
  return *book;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
