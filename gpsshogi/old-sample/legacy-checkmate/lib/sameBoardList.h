/* sameBoardList.h
 */
#ifndef _SAMEBOARDLIST_H
#define _SAMEBOARDLIST_H

#include "checkHashRecord.h"
#include "checkAssert.h"
#include "osl/checkmate/proofDisproof.h"
#ifdef CHECKMATE_DEBUG
#  include "osl/stat/ratio.h"
#endif
#include "osl/stl/slist.h"

namespace osl
{
  namespace checkmate
  {
    /**
     * 同じ盤面(持駒は異なってよい)に関する CheckHashRecord のリスト.
     * ArrayCheckHashTable や DominanceTable が使う．
     */
    class SameBoardList 
    {
    public:
      typedef slist<CheckHashRecord> list_t;
      typedef list_t::iterator iterator;
      typedef list_t::const_iterator const_iterator;
    private:
      list_t colleagues;
    public:
      ~SameBoardList();
      void clear();
      iterator begin() { return colleagues.begin(); }
      iterator end() { return colleagues.end(); }
      const_iterator begin() const { return colleagues.begin(); }
      const_iterator end()   const { return colleagues.end(); }

      bool empty() const { return colleagues.empty(); }
      size_t size() const { return colleagues.size(); }
      size_t confirmNoVisitedRecords() const;
      CheckHashRecord *find(const PieceStand& black_stand)
      {
	for (iterator p=begin(); p!=end(); ++p)
	{
	  if (p->stand(BLACK) == black_stand)
	    return &*p;
	}
	return 0; 
      }
      const CheckHashRecord *find(const PieceStand& black_stand) const
      {
	for (const_iterator p=begin(); p!=end(); ++p)
	{
	  if (p->stand(BLACK) == black_stand)
	    return &*p;
	}
	return 0; 
      }
    private:
      inline static
      void setMoreProvable(unsigned int& proofLL, unsigned int& disproofUL,
			   const CheckHashRecord *& final_by_dominance, 
			   CheckHashRecord& record);
      inline static
      void setLessProvable(unsigned int& proofUL, unsigned int& disproofLL, 
			   const CheckHashRecord *& final_by_dominance,
			   CheckHashRecord& record);
    public:
      template <Player Attacker>
      CheckHashRecord *allocate(const PieceStand& black_stand,
				const PieceStand& white_stand,
				const PathEncoding& path,
				size_t& counter);
      CheckHashRecord *allocateSlow(Player attacker,
				    const PieceStand& black_stand,
				    const PieceStand& white_stand,
				    const PathEncoding& path,
				    size_t& counter);
      /**
       * 盤面が同じで攻方の持駒が多くisVisitedな CheckHashRecord があれば
       * 返す．
       */
      template <Player Attacker>
      const CheckHashRecord *
      findIneffectiveDropLoop(const PieceStand& black_stand) const
      {
	for (list_t::const_iterator p=begin(); p!=end(); ++p)
	{
	  if (p->stand(BLACK) == black_stand)
	    continue;
	  if (p->stand(BLACK).template hasMoreThan<Attacker>(black_stand))
	  {
	    if (p->isVisited)	// わざわざ持駒を少なくした
	    {
	      return &*p;
	    }
	  }
	}
	return 0;
      }

      /**
       * @param isAttack 手番が攻方か受方か
       */
      template <bool isAttack>
      void updateSlow(Player attacker, 
		      CheckHashRecord& record, const PathEncoding& path)
      {
	if (attacker == BLACK)
	{
	  check_assert(path.turn() == (isAttack ? BLACK : WHITE));
	  update<isAttack,BLACK>(record, path);
	}
	else
	{
	  check_assert(path.turn() == (isAttack ? WHITE : BLACK));
 	  update<isAttack,WHITE>(record, path);
	}
      }
      void updateSlow(bool is_attack, Player attacker, 
		      CheckHashRecord& record, const PathEncoding& path);
      template <bool isAttack, Player Attacker>
      void update(CheckHashRecord& record, const PathEncoding& path);
    };
  } // namespace checkmate
} // namespace osl

#endif /* _SAMEBOARDLIST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
