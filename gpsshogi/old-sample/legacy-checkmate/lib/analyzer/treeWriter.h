/* treeWriter.h
 */
#ifndef _CHECK_TREEWRITER_H
#define _CHECK_TREEWRITER_H

#include "osl/move.h"
#include <boost/scoped_ptr.hpp>
#include <string>
#include <iosfwd>

namespace osl
{
  namespace container
  {
    class MoveVector;
  }
  namespace checkmate
  {
    class CheckHashRecord;
    class CheckMove;
    using container::MoveVector;
    
    namespace analyzer
    {
      class RecordSet;
      /**
       * CheckTableAnalyzer で探索木を出力
       */
      class TreeWriter
      {
	int depth;
      public:
	TreeWriter();
	virtual ~TreeWriter();
	void incDepth() { ++depth; }
	void decDepth() { --depth; }
	virtual const std::string header() const { return ""; }
	int getDepth() const { return depth; }
	virtual void showRecord(const CheckHashRecord */*record*/) {}
	/**
	 * move をたどる前に呼ばれる
	 */
	virtual void showMove(const CheckHashRecord */*from*/,
			      const CheckMove& /*move*/) {}
	/**
	 * move から帰った時に呼ばれる
	 */
	virtual void showMoveAfter(const CheckHashRecord * /*from*/,
				   const CheckMove& /*move*/) {}
	virtual void showMoves(const MoveVector& /*moves*/) {}
	virtual void writeln(const char * /*msg*/) {}
      };

      /**
       * dot (www.graphviz.org) 用.
       * log の出力にはむかないと思われる．
       */
      class DotWriter : public TreeWriter
      {
	std::ostream& os;
	std::unique_ptr<RecordSet> visited;
	/**
	 * 小さい証明数/反証数のノードを無視
	 */
	size_t minimumPdp;
      public:
	explicit DotWriter(std::ostream&, size_t min=0, const char *graphname=0);
	~DotWriter();
	const std::string header() const;
	void showRecord(const CheckHashRecord *record);
	void showMove(const CheckHashRecord *from, const CheckMove& move);
	void showMoves(const MoveVector& moves);
	void writeln(const char *msg);
      };
    

      /**
       * Emacs の outline-mode 用
       */
      class OutlineWriter : public TreeWriter
      {
      public:
	OutlineWriter();
	virtual ~OutlineWriter();
	const std::string header() const;
      };

      class TreeStreamWriter : public OutlineWriter
      {
      protected:
	std::ostream *os;
	bool simpleMove;
      public:
	explicit TreeStreamWriter(std::ostream *, bool simpleMove);
	~TreeStreamWriter();
	void showRecord(const CheckHashRecord *record);
	void showMove(const CheckMove& m);
	void showMove(const CheckHashRecord *from, const CheckMove& move);
	void showMoves(const MoveVector& moves);
	void writeln(const char *msg);
      };

    } // namespace analyzer
  } // namespace checkmate
} // namespace osl


#endif /* _CHECK_TREEWRITER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
