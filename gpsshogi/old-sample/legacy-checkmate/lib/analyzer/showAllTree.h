/* showAllTree.h
 */
#ifndef _CHECK_SHOWALLTREE_H
#define _CHECK_SHOWALLTREE_H

#include <iosfwd>
namespace osl
{
  namespace checkmate
  {
    class CheckHashRecord;

    namespace analyzer
    {
      class TreeWriter;
      class RecordSet;
      /**
       * record を root とする木を書き出す
       */
      class ShowAllTree
      {
	std::ostream& os;
	int maxDepth;
	bool expandFinalState;
	bool showTerminalMoves;
      public:
	ShowAllTree(std::ostream& os, int maxDepth, 
		    bool expandFinalState=true, bool showTerminalMoves=false);
	void showOutline(const CheckHashRecord *record) const;
	void showDot(const CheckHashRecord *record, size_t threshold) const;
	static bool isTerminal(const CheckHashRecord *record);
      private:
	void show(TreeWriter& writer, const CheckHashRecord *record) const;
	void show(const CheckHashRecord *record, TreeWriter& writer,
		  RecordSet& visited) const;
      };
    } // namespace analyzer
  } // namespace checkmate
} // namespace osl


#endif /* _CHECK_SHOWALLTREE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
