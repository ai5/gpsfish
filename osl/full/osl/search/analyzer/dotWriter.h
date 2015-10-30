/* dotWriter.h
 */
#ifndef _DOTWRITER_H
#define _DOTWRITER_H

#include "osl/search/analyzer/logWriter.h"
#include <memory>
namespace osl
{
  namespace hash
  {
    class HashKey;
  }
  namespace search
  {
    class SimpleHashTable;

    namespace analyzer
    {
      class RecordSet;
      /**
       * 探索ログ dot (www.graphviz.org) 用を書き出す.
       */
      class DotWriter : public LogWriter
      {
	/**
	 * 既に書いたノードを保存.
	 * CAVEAT: from, と to を両方書くと from -> to のエッジも書かない
	 */
	std::unique_ptr<RecordSet> written;
	std::ostream& os;
      public:
	explicit DotWriter(std::ostream& os);
	~DotWriter();
	void showNode(Player turn, const SimpleHashRecord *record, 
		      int limit, NodeType type) const;
	void showNodeQuiescence(Player turn, 
				const SimpleHashRecord *record, 
				int limit, NodeType type) const;
	void showArc(const SimpleHashRecord *from,
		     const SimpleHashRecord *to,
		     const MoveLogProb& move, bool important) const;
	void showComment(const char *line) const;
      };
    } // namespace analyzer
  } // namespace search
} // namespace osl

#endif /* _DOTWRITER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
