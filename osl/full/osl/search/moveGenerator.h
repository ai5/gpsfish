/* moveGenerator.h
 */
#ifndef OSL_MOVEGENERATOR_H
#define OSL_MOVEGENERATOR_H

#include "osl/search/simpleHashRecord.h"
#include "osl/search/searchState2.h"
#include "osl/rating/ratingEnv.h"
#include "osl/progress.h"
#include "osl/container/moveLogProbVector.h"
#include "osl/container/moveStack.h"

namespace osl
{
  namespace search
  {
    namespace analyzer
    {
      class CategoryMoveVector;
    }
    class SearchState2;
    class MoveMarker
    {
      typedef uint8_t value_t;
      CArray2d<value_t,Offset::BOARD_HEIGHT*9,Piece::SIZE*2+PTYPE_SIZE> marker;
      value_t cur;
    public:
      MoveMarker();
      void clear();
      static unsigned int pieceIndex(const NumEffectState& state, Move m)
      {
	if (m.isPass() || m.isDrop()) 
	  return Piece::SIZE*2+m.ptype();
	int base = state.pieceOnBoard(m.from()).number();
	if (m.isPromotion())
	  return base+ Piece::SIZE;
	return base;
      }
      static unsigned int toIndex(Move m)
      {
	return m.to().index()-Square::onBoardMin().index();
      }
      void registerMove(const NumEffectState& state, Move m)
      {
	marker[toIndex(m)][pieceIndex(state,m)] = cur;
      }
      bool registerIfNew(const NumEffectState& state, Move m);
      bool registered(const NumEffectState& state, Move m) const;
    };
    class MoveGenerator
    {
      enum State { 
	INITIAL, KING_ESCAPE, TAKE_BACK, BREAK_THREATMATE, CAPTURE, TACTICAL_FINISH, 
	TESUJI, ALL, FINISH 
      };
      typedef void (MoveGenerator::*generator_t)(const SearchState2&);
      static const CArray2d<generator_t, 2, FINISH> Generators;
      static const CArray<const char *, FINISH> GeneratorNames;
      MoveLogProbVector moves;
      int cur_state;
      size_t cur_index;
      const SimpleHashRecord *record;
      int limit;
      int tried;
      MoveMarker marker;
      RatingEnv env;
      Progress32 progress;
      Move eval_suggestion;
#ifndef MINIMAL
      bool in_quiesce;
#endif
      bool in_pv;
    public:
      MoveGenerator();
      template <class EvalT>
      void init(int limit, const SimpleHashRecord *record, const EvalT&,
		const NumEffectState&, bool in_pv, Move hash_move, bool quiesce=false);
      /** @param P turn */
      template <Player P>
      const MoveLogProb nextTacticalMove(const SearchState2& state) 
      {
	assert(cur_state < TACTICAL_FINISH);
	if (cur_index < moves.size()) {
	  ++tried;
	  return moves[cur_index++];
	}
	return nextTacticalMoveWithGeneration<P>(state);
      }
      template <Player P>
      const MoveLogProb nextMove(const SearchState2& state) 
      {
	assert(cur_state >= TACTICAL_FINISH);
	if (cur_index < moves.size()) {
	  ++tried;
	  return moves[cur_index++];
	}
	if (cur_state < FINISH)
	  return nextMoveWithGeneration<P>(state);
	return MoveLogProb();
      }

      /** killer move など */
      void registerMove(const NumEffectState& state, Move m)
      {
	++tried;
	if (! m.isNormal())
	  return;
	marker.registerMove(state, m);
      }
      
      int triedMoves() const { return tried; }
      const PieceMask& myPins() const { return env.my_pin; }
      void dump() const;

      // construct直後に呼ぶこと
      void generateAll(Player P, const SearchState2& state, 
		       analyzer::CategoryMoveVector&);
      template <Player P>
      void generateAll(const SearchState2&, MoveLogProbVector&);
      void generateAll(Player P, const SearchState2& state, MoveLogProbVector& out);

      const MoveLogProbVector& generated() const { return moves; }
      static int captureValue(Ptype);
      template <Player P>
      void quiesceCapture(const NumEffectState&, Square);
    private:
      template <Player P>
      const MoveLogProb nextMoveWithGeneration(const SearchState2&) ;
      template <Player P>
      const MoveLogProb nextTacticalMoveWithGeneration(const SearchState2&) ;
      template <Player P>
      void generateKingEscape(const SearchState2& state);
      template <Player P>
      void generateTakeBack(const SearchState2& state);
      template <Player P>
      void generateBreakThreatmate(const SearchState2& state);
      template <Player P>
      void generateCapture(const SearchState2& state);
      template <Player P>
      void generateTesuji(const SearchState2& state);
      template <Player P>
      void generateAllExp(const SearchState2& state);
      template <Player P>
      void generateAll(const SearchState2& state);
      template <Player P>
      void addCapture(const NumEffectState&, const RatingEnv&, const MoveVector&);
    public:
      /** call this before any use of instance of MoveGenerator */
      static void initOnce();
    };
  }
}


#endif /* OSL_MOVEGENERATOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
