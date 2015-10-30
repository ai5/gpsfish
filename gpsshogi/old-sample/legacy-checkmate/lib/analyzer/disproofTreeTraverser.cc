/* disproofTreeTraverser.cc
 */
#include "analyzer/disproofTreeTraverser.h"
#include "analyzer/proofTreeTraverser.h"
#include "checkHashRecord.h"
#include "corruptCheckTable.h"
#include "osl/checkmate/pawnCheckmateMoves.h"
#include "osl/record/csa.h"
#include <iostream>

osl::checkmate::analyzer::
DisproofTreeTraverser::DisproofTreeTraverser(TreeWriter& w, const TwinTable& t,
					     bool partial)
  : TreeTraverser(w, t), isPartialStack(partial)
{
}
osl::checkmate::analyzer::
DisproofTreeTraverser::~DisproofTreeTraverser()
{
}

void osl::checkmate::analyzer::
DisproofTreeTraverser::orNode(Move last_move, const CheckHashRecord *record,
			      const HashKey& key, const PathEncoding& path)
{
  if (! visited.insert(record).second)
  {
    writer.writeln("confluence");
    if (! record->isConfluence)
      writer.writeln("UNRECOGNIZED confluence!");
    leaves.insert(record);
    return;
  }

  if (analyzerStack.findNotLast(record) || record->isVisited)
  {
    writer.writeln("loop detection");
    leaves.insert(record);
    return;
  }
  if (record->finalByDominance())
  {
    writer.writeln("finalByDominance");
    leaves.insert(record);
    return;
  }
  if (! record->twins.empty())
  {
    // stack 中に優越関係がある局面があれば，不詰
    const Player attacker = alt(path.turn());
    if (analyzerStack.findCover(attacker, key, record) != analyzerStack.end())
    {
      writer.writeln("loop detection (drop)");
      leaves.insert(record);
      return;
    }
  }

  const bool loopFound = record->findLoop(path, table);
  const bool loopToStackFound = findLoopToStack(record->twins);
  if ((! record->proofDisproof().isCheckmateFail())
      && (! loopFound) && (! loopToStackFound)
      && (! record->proofDisproof().isPawnDropFoul(last_move)))
    throw CorruptCheckTable(record, "! isCheckmateFail in disproof");

  // 指手がない時は信用
  if (record->moves.empty())
  {
    leaves.insert(record);
    return;
  }

  const CheckMoveList& l = record->moves;
  // ここはOrNode なのでどれか一つを選ぶ
  for (CheckMoveList::const_iterator p=l.begin(); p!=l.end(); ++p)
  {
    if (! p->record)
      continue;
    const PathEncoding newPath(path, p->move);
    if (p->record->proofDisproof().isCheckmateFail()
	|| p->findLoop(path, table)
	|| p->record->isVisited
	|| findLoopToStack(p->record->twins)
	|| analyzerStack.findNotLast(p->record))
    {
      writer.showMove(record, *p);
      writer.incDepth();

      const HashKey newKey = key.newHashWithMove(p->move);
      analyzerStack.push_back(CheckStackEntry(&*p, "D-AND", p->record, 
					      newKey, newPath));
      DisproofTreeTraverser::andNode(p->move, p->record, newKey, newPath);
      analyzerStack.pop_back();
      writer.decDepth();
      return;
    }
  }

  // 反証に必要な指手を見つけられなかった
  if (isPartialStack && (loopFound || loopToStackFound))
    return; // 反証した時には p->record->isVisited だったと思われる
  std::cerr << "corrupt (dis)proof tree\n";
  throw CorruptCheckTable(record, "best move not found in disproof or-node");
}

void osl::checkmate::analyzer::
DisproofTreeTraverser::andNode(Move, const CheckHashRecord *record,
			       const HashKey& key, const PathEncoding& path)
{
  if (! visited.insert(record).second)
  {
    writer.writeln("confluence");
    if (! record->isConfluence)
      writer.writeln("UNRECOGNIZED confluence!");
    leaves.insert(record);
    return;
  }

  if (analyzerStack.findNotLast(record) || record->isVisited)
  {
    writer.writeln("loop detection");
    leaves.insert(record);
    return;
  }
  if (! record->twins.empty())
  {
    const Player attacker = path.turn();
    if (analyzerStack.findCover(attacker, key, record) != analyzerStack.end())
    {
      writer.writeln("loop detection (drop)");
      leaves.insert(record);
      return;
    }
  }

  assert(record->proofDisproof().isCheckmateFail()
	 || findLoopToStack(record->twins)
	 || record->findLoop(path, table));

  if (record->finalByDominance())
  {
    writer.writeln("finalByDominance");
    leaves.insert(record);
    return;
  }

  const CheckMoveList& l = record->moves;
  if (l.empty())
  {
    leaves.insert(record);
    return;
  }
  
  const bool nowLoopDetection = ! record->proofDisproof().isCheckmateFail();

  for (CheckMoveList::const_iterator p=l.begin(); p!=l.end(); ++p)
  {
    if ((record->proofDisproof() != ProofDisproof::PawnCheckmate())
	&& p->flags.isSet(MoveFlags::NoPromote))
      continue; // NoPromote は試す必要がない

    if (! p->record)
    {
      if (nowLoopDetection && isPartialStack)
	continue;		// ここで visited だったはず
    }

    if (analyzerStack.findLoop(p->record) != analyzerStack.end())
      continue;
    const bool loopToStack = findLoopToStack(p->record->twins);

    if (p->record->proofDisproof().isPawnDropFoul(p->move))
      continue;

    // 基本的に Solved マークがついているはず
    if ((! p->flags.isSet(MoveFlags::Solved))
	&& record->proofDisproof().isCheckmateFail()
	&& (! loopToStack))
    {
      if (isPartialStack)
	continue;		// 本来はloopToStack だった可能性あり
#ifdef CHECK_ABSORB_LOOP
      if (isCheckmateFail(p->record->proofDisproof()))
      {
	// CHECK_ABSORB_LOOP 関係で Solved をつけ忘れることがあるらしい
      }
      else
#endif
      {
	csaShow(std::cerr, p->move);
	std::cerr << "\nUnexamined move in disproof tree?\n";
	record->dump();
	if (p->record)
	  p->record->dump();
	writer.writeln("UNEXAMINED move!");
	writer.showMove(record, *p);
	writer.writeln("possible BUG\n");
	throw CorruptCheckTable(record, "unexamined move in disproof and-node");
      }
    }

    const TwinEntry *loopInNextMove = p->findLoop(path, table);
    // isCheckmateFail だったら子供もそうあるべき
    if (p->record->proofDisproof().isCheckmateFail())
    {
      if ((! p->record->proofDisproof().isCheckmateFail())
	  && (! loopToStack))
      {
	std::cerr << "path " << path << " -> " << PathEncoding(path, p->move)
		  << "\n";
	std::cerr << "checkmate fail parent " << record << "\n";
	record->dump();
	std::cerr << "! checkmate fail child\n";
	std::cerr << "loopInNextMove " << loopInNextMove << "\n";
	if (loopInNextMove)
	  std::cerr << "loopInNextMove->loopTo " << loopInNextMove->loopTo
		    << "\n";
	p->record->dump();
	throw CorruptCheckTable(record, "bad aggregation in and-node");
      }
    }
    else
    {
      // ループによる反証
      if ((! loopInNextMove)
	  && (! p->record->proofDisproof().isCheckmateFail())
	  && (! loopToStack))
      {
	// loop detection は isPartialStack の時は確認できなくてもしょうがない
	if (isPartialStack)
	  continue; // 反証した時には p->record->isVisited だったと思われる
	std::cerr << "not confirmed in loop " << p->move << "\n";
	record->dump();
	if (PawnCheckmateMoves::effectiveOnlyIfPawnCheckmate(p->move))
	  continue;		// GHI + NoPromote と信じる
	if (p->record)
	  p->record->dump();
	writer.showMove(record, *p);
	writer.writeln("possible BUG\n");
	throw CorruptCheckTable(record, "loop detection not confirmed in disproof and-node");
      }
    }
    
    writer.showMove(record, *p);
    writer.incDepth();

    PathEncoding newPath(path, p->move);
    const HashKey newKey = key.newHashWithMove(p->move);
    analyzerStack.push_back(CheckStackEntry(&*p, "D-OR ", p->record, 
					    newKey, newPath));
    DisproofTreeTraverser::orNode(p->move, p->record, newKey, newPath);
    analyzerStack.pop_back();
    writer.decDepth();
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
