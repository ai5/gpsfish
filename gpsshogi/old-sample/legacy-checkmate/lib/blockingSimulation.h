/* blockingSimulation.h
 */
#ifndef _BLOCKING_SIMULATION_H
#define _BLOCKING_SIMULATION_H

#include "osl/state/numEffectState.h"
#include "osl/hash/hashKey.h"
namespace osl
{
  class PathEncoding;
  namespace checkmate
  {
    class CheckHashRecord;
    class CheckMove;
    /**
     * @param P 攻撃側
     */
    template <Player P>
    struct BlockingSimulation
    {
      /** 現在の局面と record で 中合の move を指した時に
       * 兄弟ノードから oracle を探してシミュレーションする 
       */
      template <class Table>
      static bool proof(NumEffectState& state, 
			const HashKey& new_key, const PathEncoding& new_path,
			const CheckHashRecord *record, Table& table,
			const CheckMove& move, size_t& node_count);
      /** 
       * oracle で詰んだ時に，oracle.move.to() が同じ兄弟が
       * 詰むかどうかをシミュレーションする．
       */
      template <class Table>
      static void proofSibling(NumEffectState& state, 
			       const HashKey& key, const PathEncoding& path,
			       CheckHashRecord *record, Table& table,
			       const CheckMove& oracle, size_t& node_count);
    private:
      /**
       * target を指した後，guide と同様につむかどうかを確かめる
       */
      template <class Table>
      static bool proof(NumEffectState& state, 
			const HashKey&, const PathEncoding&, Table& table, 
			const CheckMove& target, const CheckHashRecord *guide,
			size_t& node_count);

    };
  } // namespace checkmate
} // namespace osl

#endif /* _BLOCKING_SIMULATION_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
