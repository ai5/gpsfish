/* effectContent.h
 */
#ifndef OSL_EFFECTCONTENT_H
#define OSL_EFFECTCONTENT_H

#include "osl/basic_type.h"

namespace osl
{
  class EffectContent
  {
    int effect;
    EffectContent(int value) : effect(value)
    {
    }
  public:
    EffectContent() : effect(0)
    {
    }
    explicit EffectContent(Offset offset)
      : effect(offset.intValue() << 1)
    {
    }
    static const EffectContent DIRECT() { return EffectContent(1); }
    /**
     * 隣だが，offsetも与える
     */
    static const EffectContent DIRECT(Offset offset) { 
      return EffectContent((offset.intValue() << 1)+1); 
    }
    /**
     * 短い利きがあるか，間がemptyなら長い利きがある
     */
    bool hasEffect() const { return effect; }
    /**
     * 短い利きがある．長い利きの隣も含む
     */
    bool hasUnblockableEffect() const { return (effect & 1); }
    /**
     * 返り値が0なら長い利きがない, 
     * 0以外なら辿るのに必要なoffset
     * (2005/3/25 に仕様変更 - 長い利きだが隣の場合もoffsetを返す)
     */
    const Offset offset() const { return Offset::makeDirect(effect >> 1); }
    /**
     * 2005/3/25に変更.
     */
    bool hasBlockableEffect() const { 
      return (effect & (-effect) & ~1) != 0;
    }
    int intValue() const { return effect; }
  };

  inline bool operator==(EffectContent l, EffectContent r)
  {
    return l.intValue() == r.intValue();
  }
  inline bool operator!=(EffectContent l, EffectContent r)
  {
    return ! (l == r);
  }
  inline bool operator<(EffectContent l, EffectContent r)
  {
    return l.intValue() < r.intValue();
  }
  
} // namespace osl

#endif /* OSL_EFFECTCONTENT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
