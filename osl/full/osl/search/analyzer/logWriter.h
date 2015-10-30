/* logWriter.h
 */
#ifndef _LOGWRITER_H
#define _LOGWRITER_H

#include "osl/basic_type.h"
#include <iosfwd>

namespace osl
{
  class MoveLogProb;
  namespace search
  {
    class SimpleHashRecord;
    namespace analyzer
    {
      /**
       * 探索ログ log を書き出す抽象クラス.
       * @see DotWriter
       * @see OutlineWriter (未定義)
       */
      class LogWriter
      {
      public:
	enum NodeType { NORMAL=0, IMPORTANT=1, ABNORMAL=2, };
	LogWriter();
	virtual ~LogWriter();
	/**
	 * @param important ユーザが指定した読筋を指定する場合 true
	 */
	virtual void showNode(Player turn, const SimpleHashRecord *record, 
			      int limit, NodeType type) const = 0;
	virtual void showNodeQuiescence(Player turn, 
					const SimpleHashRecord *record, 
					int limit, NodeType type) const = 0;
	virtual void showArc(const SimpleHashRecord *from,
			     const SimpleHashRecord *to,
			     const MoveLogProb& move, bool important) const = 0;
	virtual void showComment(const char * /*line*/) const {}
      };
    } // namespace analyzer
  } // namespace search
} // namespace osl

#endif /* _LOGWRITER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
