/* sendOffSquare.h
 */
#ifndef OSL_SENDOFFPOSITION_H
#define OSL_SENDOFFPOSITION_H

#include "osl/numEffectState.h"
#include "osl/container/square8.h"
namespace osl
{
  namespace effect_util
  {
    struct Offset8 : public FixedCapacityVector<int,8>
    {
    };
    /** 送り金のような玉で取ると利きがはずれる駒が発生するマスを調査 */
    struct SendOffSquare
    {
      typedef uint8_t SendOff8;
      template <Player Attack>
      static bool onlyOneSupport(const NumEffectState& state, Square target)
      {
	const Piece p = state.pieceAt(target);
	if (! p.isOnBoardByOwner<alt(Attack)>())
	  return false;
	return state.hasEffectAt<Attack>(target) 
	  && (state.countEffect(alt(Attack), target) == 1);
      }
      template <Player Attack>
      static SendOff8 find(const NumEffectState& state, Square king_position,
			      Square8& out);
      static SendOff8 find(Player attack,
			      const NumEffectState& state, Square king_position,
			      Square8& out);
      static SendOff8 invalidData() { return 0xff; }
      static void unpack(SendOff8, Square king, Square8& out);
      struct Table
      {
	CArray<Offset,8> normal;
	CArray<Offset8,8> reverse;
	CArray<Offset8,256> reverse_all;
	void init();
      };
      static void init() { table.init(); }
    private:
      static Table table;
      template <Player Attack>
      static void testSquare(const NumEffectState& state, Square candidate,
			       int id, int& out)
      {
	if (onlyOneSupport<Attack>(state, candidate))
	{
	  out |= (1<<id);
	}
      }
    };
  } // namespace effect_util
  using effect_util::SendOffSquare;
} // namespace osl

#endif /* OSL_SENDOFFPOSITION_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
