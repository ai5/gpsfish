#include "analyzer/proofTreeDepth.h"
#include "checkHashRecord.h"
#include "osl/stl/hash_map.h"
#include "osl/stl/pointerHash.h"

/**
 * 深さを記憶するテーブル.
 * -1 は探索中
 */
struct osl::checkmate::analyzer::ProofTreeDepth::Table
  : public osl::hash_map<const CheckHashRecord*, int>
{
};

osl::checkmate::analyzer::
ProofTreeDepth::ProofTreeDepth()
  : table(new Table())
{
}

osl::checkmate::analyzer::
ProofTreeDepth::~ProofTreeDepth()
{
}

int osl::checkmate::analyzer::
ProofTreeDepth::depth(const CheckHashRecord *record, bool is_or_node) const
{
  if (! record)
  {
    assert(is_or_node);		// ImmediateCheckmate
    return 1;
  }
  assert(record);
  assert(record->proofDisproof().isCheckmateSuccess());
  return (is_or_node ? orNode(record) : andNode(record));
}

int osl::checkmate::analyzer::
ProofTreeDepth::orNode(const CheckHashRecord *record) const
{
  assert(record);
  assert(record->proofDisproof().isCheckmateSuccess());
  const Table::const_iterator p = table->find(record);
  if (p != table->end())
    return p->second;
  (*table)[record] = -1;

  if (record->bestMove && (! record->bestMove->record))
  {
    // ImmediateCheckmate
    assert(record->bestMove->flags.isSet(MoveFlags::ImmediateCheckmate));
    (*table)[record] = 1;
    return 1;
  }
  const CheckMoveList& l = record->finalByDominance()
    ? record->finalByDominance()->moves
    : record->moves;
  for (CheckMoveList::const_iterator p=l.begin(); p!=l.end(); ++p)
  {
    if (! p->record)
      continue;
    if (! p->record->proofDisproof().isCheckmateSuccess())
      continue;
    const int depth = andNode(p->record);
    if (depth >= 0)
    {
      (*table)[record] = depth+1;
      return (*table)[record];
    }
  }
  if (! l.empty())
    return -1;			// ループ迷いこみ

  (*table)[record] = 0;
  return 0;	// or node で指手がなくて詰 => 逃げてない
}

int osl::checkmate::analyzer::
ProofTreeDepth::andNode(const CheckHashRecord *record) const
{
  assert(record);
  assert(record->proofDisproof().isCheckmateSuccess());
  const Table::const_iterator p = table->find(record);
  if (p != table->end())
    return p->second;
  (*table)[record] = -1;

  int result = 0;	// and node で指手がなくて詰 => 逃げられない

  const CheckMoveList& l = record->finalByDominance()
    ? record->finalByDominance()->moves
    : record->moves;
  for (CheckMoveList::const_iterator p=l.begin(); p!=l.end(); ++p)
  {
    if (! p->record)
      continue;
    const int depth = orNode(p->record);
    if (depth < 0)
      return depth;		// loop found
    result = std::max(depth+1, result);
  }
  
  (*table)[record] = result;
  return result;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
