/* twinEntry.h
 */
#ifndef _TWINENTRY_H
#define _TWINENTRY_H

#include "checkMove.h"
#include "osl/pathEncoding.h"

namespace osl
{
  namespace checkmate
  {
    class CheckHashRecord;
    struct TwinEntry
    {
      PathEncoding path;
      CheckMoveCore move;
      /**
       * 一番浅いloop先のアドレス，複数あれば0.
       * LoopDetection を NoCheckmate に切り替えるために使用
       */
      const CheckHashRecord *loopTo;
      TwinEntry(const PathEncoding& p, const CheckMoveCore& m,
		const CheckHashRecord *l=0)
	: path(p), move(m), loopTo(l)
      {
      }
    };

    /**
     * 同じシミュレーションを2度行なわないための記録と，TwinEntryを同時に表に格納するためのクラス.
     * - LoopDetection発見時 -> TwinEntry を使用
     * - LoopDetection未発見時 -> TwinEntry::path と age を使用
     */
    struct TwinAgeEntry
    {
      TwinEntry entry;
      /**
       * 前回シミュレーションした時のtwinsの長さ
       */
      int age;
      /**
       * TwinEntry を作成
       */
      TwinAgeEntry(const PathEncoding& p, const CheckMoveCore& m,
		   const CheckHashRecord *l=0)
	: entry(p, m, l), age(-1)
      {
      }
      /**
       * age メモを作成
       */
      TwinAgeEntry(const PathEncoding& path) : entry(path, CheckMoveCore()), age(0)
      {
      }
      bool hasTwinEntry() const { return age < 0; }
      void setTwinEntry(const TwinEntry& e) 
      {
	assert(entry.path == e.path);
	entry.move = e.move;
	age = -1;
      }
    };

  } // namespace checkmate
} // namespace osl


#endif /* _TWINENTRY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
