/* proofTreeTraverser.cc
 */
#include "analyzer/proofTreeTraverser.h"
#include "analyzer/disproofTreeTraverser.h"
#include "checkHashRecord.h"
#include "corruptCheckTable.h"
#include "osl/record/csa.h"
#include <iostream>

osl::checkmate::analyzer::
ProofTreeTraverser::ProofTreeTraverser(TreeWriter& w, const TwinTable& t)
  : TreeTraverser(w, t)
{
}
osl::checkmate::analyzer::
ProofTreeTraverser::~ProofTreeTraverser()
{
}

void osl::checkmate::analyzer::
ProofTreeTraverser::orNode(Move, const CheckHashRecord *record,
			   const HashKey& key, const PathEncoding& path)
{
  assert(! analyzerStack.findNotLast(record));
  assert(! record->isVisited);
  // stack 中に優越関係がある局面があれば，不詰と判断することが妥当だが，
  // 優越関係を使わずに探索した場合は，そのような証明木を得ることがあるので
  // ここでは確認しない．

  // 証明木の条件
  if (! record->proofDisproof().isCheckmateSuccess())
    throw CorruptCheckTable(record, "! isCheckmateSuccess in proof");

  // 指手がない時は信用
  if (record->moves.empty())
  {
    leaves.insert(record);
    return;	// isproof で，前の手が逃げる手になっていなかった時とか
  }
  
  if (! visited.insert(record).second)
  {
    writer.writeln("confluence");
    if (! record->isConfluence)
      writer.writeln("UNRECOGNIZED confluence!");
    return;
  }

  if (record->finalByDominance())
  {
    writer.writeln("finalByDominance");
    return;
  }
  if (! record->hasBestMove())
    throw CorruptCheckTable(record, "best move not recorded");

  const CheckMoveList& l = record->moves;
  // ここはOrNode なのでどれか一つを選ぶ
  for (CheckMoveList::const_iterator p=l.begin(); p!=l.end(); ++p)
  {
    const PathEncoding newPath(path, p->move);
    if (p->move == record->bestMove->move)
    {
      writer.showMove(record, *p);
      if (p->flags.isSet(MoveFlags::ImmediateCheckmate))
	return;
      assert(p->record);
      if (! p->record)
	return;
      writer.incDepth();
      
      const HashKey newKey = key.newHashWithMove(p->move);
      analyzerStack.push_back(CheckStackEntry(&*p, "P-AND", p->record, 
					      newKey, newPath));
      ProofTreeTraverser::andNode(p->move, p->record, newKey, newPath);
      analyzerStack.pop_back();

      writer.decDepth();
      writer.showMoveAfter(record, *p);
      return;
    }
  }

  // 証明に必要な指手を見つけられなかった
  std::cerr << "corrupt proof tree\n";
  throw CorruptCheckTable(record, "best move not found in proof or-node");
}


void osl::checkmate::analyzer::
ProofTreeTraverser::andNode(Move, const CheckHashRecord *record,
			    const HashKey& key, const PathEncoding& path)
{
  if (! visited.insert(record).second)
  {
    writer.writeln("confluence");
    if (! record->isConfluence)
      writer.writeln("UNRECOGNIZED confluence!");
    return;
  }

  assert(record->proofDisproof().isCheckmateSuccess());
  assert(! analyzerStack.findNotLast(record));
  assert(! record->isVisited);
  // TODO: 追及 path によっては詰になるノードを詰と扱って弊害があるグ
  // ラフはあるか? あるとするとアルゴリズムの変更が必要
  // 825棋譜の104手付近でおこる
  // assert(! record->findLoop(path));

  // stack 中に優越関係がある局面があれば，不詰と判断することが妥当だが，
  // 優越関係を使わずに探索した場合は，そのような証明木を得ることがあるので
  // ここでは確認しない．

  if (record->finalByDominance())
  {
    leaves.insert(record);
    writer.writeln("finalByDominance");
    return;
  }  
  
  const CheckMoveList& l = record->moves;
  if (l.empty())
  {
    leaves.insert(record);
    return;
  }
  
  for (CheckMoveList::const_iterator p=l.begin(); p!=l.end(); ++p)
  {
    const bool loopToStack = p->record && findLoopToStack(p->record->twins);
    if (loopToStack)
    {
      std::cerr << "ineffective proof solution\n";
    }
    if (p->flags.isSet(MoveFlags::ImmediateCheckmate))
      continue;
    if ((! p->record) || (! p->flags.isSet(MoveFlags::Solved)))
    {
      csaShow(std::cerr, p->move);
      std::cerr << "\nUnexamined move in proof tree?\n";
      record->dump();
      writer.writeln("UNEXAMINED move!");
      writer.showMove(record, *p);
      writer.writeln("possible BUG\n");
      throw CorruptCheckTable(record, "unexamined move in proof and-node");
    }
#ifndef NDEBUG
    const TwinEntry *loopInNextMove = p->findLoop(path, table);
    assert(! loopInNextMove);
#endif
    writer.showMove(record, *p);
    writer.incDepth();

    PathEncoding newPath(path, p->move);
    const HashKey newKey = key.newHashWithMove(p->move);
    analyzerStack.push_back(CheckStackEntry(&*p, "P-OR ", p->record, 
					    newKey, newPath));
    ProofTreeTraverser::orNode(p->move, p->record, newKey, newPath);
    analyzerStack.pop_back();

    writer.decDepth();
    writer.showMoveAfter(record, *p);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
