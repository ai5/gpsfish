/* openingBookTracer.h
 */
#ifndef _OPENINGBOOKTRACER_H
#define _OPENINGBOOKTRACER_H

#include "osl/basic_type.h"

namespace osl
{
  namespace game_playing
  {
    /**
     * 定跡の追跡
     */
    class OpeningBookTracer
    {
    protected:
      bool verbose;
    public:
      OpeningBookTracer() : verbose(false) {}
      virtual ~OpeningBookTracer();
      /** new したものを返す */
      virtual OpeningBookTracer* clone() const = 0;
      /** 指した手に対応して状態を更新する． */
      virtual void update(Move)=0;
      /**
       * 良い手を探す．状態は更新しない．
       * @return 定跡をはずれたら Move::INVALID()
       */
      virtual const Move selectMove() const=0;
      virtual bool isOutOfBook() const=0;
      /**
       * 一手前の状態に戻す
       */
      virtual void popMove()=0;
      bool isVerbose() const { return verbose; }
    };

    /**
     * 定跡無し
     */
    class NullBook : public OpeningBookTracer
    {
    public:
      ~NullBook();
      OpeningBookTracer* clone() const 
      {
	return new NullBook();
      }
      
      void update(Move);
      const Move selectMove() const;
      bool isOutOfBook() const;
      void popMove();
    };
    
  } // namespace game_playing
} // namespace osl

#endif /* _OPENINGBOOKTRACER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
