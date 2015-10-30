/* defenseSimulation.h
 */
#ifndef _DEFENSE_SIMULATION_H
#define _DEFENSE_SIMULATION_H

#include "osl/state/numEffectState.h"

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
    struct DefenseSimulation
    {
      /** 
       * oracle で反証された時に，drop で oracle.move.ptype() が同じ兄弟が
       * 反証できるかどうかをシミュレーションする．
       * @param key  state のkey
       * @param path state のpath
       */
      template <class Table>
      static void disproofDropSibling(NumEffectState& state, 
				      const HashKey& key,
				      const PathEncoding& path,
				      CheckHashRecord *record, Table& table,
				      const CheckMove& oracle,
				      size_t& node_count);
      /**
       * NoPromote 用:
       * target を指した後，guide と同様に反証できるかどうかを確かめる
       * @param state target を指す前
       * @param new_key target を指した後
       * @param new_path target を指した後
       */
      template <class Table>
      static bool disproofNoPromote(NumEffectState& state, 
				    const HashKey& new_key,
				    const PathEncoding& new_path,
				    CheckHashRecord *record, Table& table, 
				    CheckMove& target, 
				    const CheckMove& guide, size_t& node_count);
    private:
      /**
       * target を指した後，guide と同様に反証できるかどうかを確かめる
       */
      template <class Table>
      static bool disproof(NumEffectState& state, 
			   const HashKey& new_key,
			   const PathEncoding& new_path, 
			   Table& table, const CheckMove& target, 
			   CheckHashRecord *guide, size_t& node_count);

    };
  } // namespace checkmate
} // namespace osl

#endif /* _DEFENSE_SIMULATION_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
