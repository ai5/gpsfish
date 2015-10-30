/* recordTracer.h
 */
#ifndef GAME_PLAYING_RECORDTRACER_H
#define GAME_PLAYING_RECORDTRACER_H

#include "osl/game_playing/openingBookTracer.h"
#include <stack>
#include <vector>

namespace osl
{
  namespace game_playing
  {
    /**
     * vector<Moves>の追跡
     */
    class RecordTracer : public OpeningBookTracer
    {
    public:
      typedef std::vector<Move> moves_t;
    private:
      const moves_t moves;
      std::stack<int> state_index;
      bool verbose;
    public:
      explicit RecordTracer(const moves_t& moves, bool verbose=false);
      RecordTracer(const RecordTracer&);
      ~RecordTracer();
      OpeningBookTracer* clone() const;

      void update(Move);
      const Move selectMove() const;

      int stateIndex() const { return state_index.top(); }
      bool isOutOfBook() const;
      void popMove();

      static const RecordTracer kisenRecord(const char *filename, int id,
					    unsigned int num_moves,
					    bool verbose);
    };
  } // namespace game_playing
} // namespace osl

#endif /* _RECORDTRACER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
