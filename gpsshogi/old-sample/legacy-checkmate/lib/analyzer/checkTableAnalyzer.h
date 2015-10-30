/* checkTableAnalyzer.h
 */
#ifndef _CHECKTABLEANALYZER_H
#define _CHECKTABLEANALYZER_H

#include <iosfwd>
#include <cstddef>
namespace osl
{
  namespace hash
  {
    class HashKey;
  }
  class PathEncoding;
  namespace checkmate
  {
    class CheckHashRecord;
    class TwinTable;
    namespace analyzer
    {
      using hash::HashKey;
      /**
       * 詰将棋探索後のテーブルの分析
       *
       * 探索した木の大きさと、見つけた証明に必要な木の大きさ
       * (最小証明木とは限らない)、
       * TODO: 証明に必要な持駒などを求める
       */
      class CheckTableAnalyzer
      {
	const TwinTable& table;
	bool useOutlineFormat;
      public:
	explicit CheckTableAnalyzer(const TwinTable&, bool outline=true);
	~CheckTableAnalyzer();
    
	/**
	 * record を root とする木を書き出す
	 */
	void showTree(const CheckHashRecord *record, std::ostream& os,
		      int maxDepth, bool expandFinalState=true, 
		      bool showTerminalMoves=false, size_t threshold=0) const;
	/**
	 * テーブルの中でstate から到達可能な局面の数を調べる
	 */
	size_t treeSize(const CheckHashRecord *record) const;
	/**
	 * record の証明するのに必要な局面の数を調べる
	 */
	size_t proofTreeSize(const CheckHashRecord *record, 
			     const HashKey& key, 
			     const PathEncoding& path, 
			     bool orNode) const;
	size_t proofTreeSize(const CheckHashRecord *record, 
			     const HashKey& key, 
			     const PathEncoding& path, 
			     bool orNode, size_t &leaf_size) const;
	/**
	 * record の反証するのに必要な局面の数を調べる
	 */
	size_t disproofTreeSize(const CheckHashRecord *record, 
				const HashKey& key, 
				const PathEncoding& path, 
				bool orNode, bool isPartialStack=false) const;
	size_t disproofTreeSize(const CheckHashRecord *record, 
				const HashKey& key, 
				const PathEncoding& path, 
				bool orNode, size_t& leaf_size, 
				bool isPartialStack=false) const;
	/**
	 * @return state が 詰みでも不詰でもなければ 0 が帰る
	 */
	size_t proofOrDisproofTreeSize(const CheckHashRecord *record, 
				       const HashKey& key, 
				       const PathEncoding& path, 
				       bool orNode, bool isPartialStack=false) const;
	size_t proofOrDisproofTreeSize(const CheckHashRecord *record, 
				       const HashKey& key, 
				       const PathEncoding& path,
				       bool orNode, size_t& leaf_size, 
				       bool isPartialStack=false) const;

	/**
	 * テーブルの中でstate の証明するのに必要な木を表示する
	 */
	size_t showProofTree(const CheckHashRecord *record, 
			     const HashKey& key, const PathEncoding& path, 
			     bool orNode, std::ostream& os,
			     bool isPartialStack=false) const;
      };
    } // namespace analyzer
  } // namespace checkmate
  using checkmate::analyzer::CheckTableAnalyzer;
} // namespace osl

#endif /* _CHECKTABLEANALYZER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
