/* kanjiPrint.h
 */
#ifndef RECORD_KANJIPRINT_H
#define RECORD_KANJIPRINT_H

#include "osl/simpleState.h"
#include <memory>
#include <iosfwd>
#include <string>

namespace osl
{
  class Move;

  namespace record
  {
    class Color;
    std::ostream& operator<<(std::ostream& os, const Color& c);
    /**
     * カラーコードを保持するクラス。
     */
    class Color
    {
    public:
      Color() : name(""), valid(false) {}
      Color(const std::string& value, const std::string& name, const bool valid=true);
      ~Color();
    private:
      std::string value;    
      std::string name;
      bool valid;
    public:
      bool isValid() const {return valid;}
      const std::string& getName() const {return name;}
      bool operator==(const Color& rhs) const
      {
        return (this->valid == rhs.valid) &&
               (this->value == rhs.value); 
      }
      bool operator!=(const Color& rhs) const
      {
        return !(*this == rhs);
               
      }

      /**
       * 文字列に対応するColor objectを返す
       */
      static const Color colorFor(const std::string& str);

      /** 色指定しない（デフォルトのまま）ことを示す特別なオブジェクト */
      static const Color NONE;
      static const Color Black;      
      static const Color Red;        
      static const Color Green;      
      static const Color Brown;      
      static const Color Blue;       
      static const Color Purple;     
      static const Color Cyan;       
      static const Color LightGray;  
      static const Color DarkGray;   
      static const Color LightRed;   
      static const Color LightGreen; 
      static const Color Yellow;     
      static const Color LightBlue;  
      static const Color LightPurple;
      static const Color LightCyan;  
      static const Color White;      
   
      friend std::ostream& operator<<(std::ostream& os, const Color& c);
    };


    /**
     * shellの文字出力にて、色を変える。
     */
    class ChangeShellColor
    {
    private:
      std::ostream& os;
      const Color color;

      void escColSet() const;
      void escColReSet() const;
    public:
      ChangeShellColor(std::ostream& os, const Color& color)
        : os(os), color(color) {escColSet();}
      ~ChangeShellColor() {escColReSet();}
    };

    /** 漢数字を返す（持ち駒の数などで用いる） */
    std::string kanjiNumber(const int n);

    /**
     * 駒の文字を管理するAbstract class。
     */
    class Characters
    {
    public:
      static const CArray<std::string, 32> stand; 

      virtual ~Characters();

      /** 段数の文字を返す */
      virtual const std::string& getDan(const size_t index) const = 0; 
      /** 筋の文字を返す。駒の文字幅に合わせる必要がある。 */
      virtual const std::string& getSuji(const size_t index) const = 0; 
      /** 盤面上の駒を返す */
      virtual const std::string& getPiece(const size_t index) const = 0; 

      /** 持ち駒の漢字文字を返す。1文字を期待する */
      const std::string& getStand(const size_t index) const 
      {
        return stand[index];
      }

      const std::string& stand_kanji(const PtypeO& ptypeo) const
      {
        return getStand(piece_index(ptypeo));
      }

      const std::string& kanji(const PtypeO& ptypeo) const
      {
        return getPiece(piece_index(ptypeo));
      }
      const std::string& kanji(Ptype ptype) const
      {
        return getPiece(newPtypeO(BLACK, ptype));
      }
    private:
      size_t piece_index(const PtypeO& ptypeo) const
      {
#ifndef NDEBUG
        static const size_t NPieces = PTYPEO_MAX - PTYPEO_MIN+2;
#endif
        const size_t index = ptypeo - PTYPEO_MIN;
        assert(index < NPieces);
        return index;
      }
    };

    /** 持ち駒用（標準文字）  */
    struct StandardCharacters : public Characters
    {
      /** 段数の文字 */
      static const CArray<std::string,10> dan;
      /** 筋の文字。駒の文字幅に合わせる必要がある。 */
      static const CArray<std::string,10> suji;
      /** 盤面上の駒の漢字文字 */
      static const CArray<std::string,32> pieces;

      const std::string& getDan(const size_t index) const {return dan[index];} 
      const std::string& getSuji(const size_t index) const {return suji[index];} 
      const std::string& getPiece(const size_t index) const {return pieces[index];} 
    };

    /** ロシア文字（激指フォント用）*/
    struct RussianCharacters : public Characters
    {
      static const CArray<std::string,10> dan;
      static const CArray<std::string,10> suji;
      static const CArray<std::string,32> pieces;

      const std::string& getDan(const size_t index) const {return dan[index];} 
      const std::string& getSuji(const size_t index) const {return suji[index];} 
      const std::string& getPiece(const size_t index) const {return pieces[index];} 
    };

    /** 柿木形式（KIF）。頭にv */
    struct KIFCharacters : public Characters
    {
      static const CArray<std::string,10> dan;
      static const CArray<std::string,10> suji;
      static const CArray<std::string,32> pieces;

      const std::string& getDan(const size_t index) const {return dan[index];} 
      const std::string& getSuji(const size_t index) const {return suji[index];} 
      const std::string& getPiece(const size_t index) const {return pieces[index];} 
    };

    /**
     * 局面を漢字でカラーで表示する.
     */
    class KanjiPrint
    {
    private:
      std::ostream& os;
      const std::shared_ptr<Characters> pieces;
      Color black_color;
      Color white_color;
      Color last_move_color;

    public:
      explicit KanjiPrint(std::ostream& os, 
                          const std::shared_ptr<Characters> pieces=std::shared_ptr<Characters>(new StandardCharacters()))
        : os(os), pieces(pieces), 
          black_color(Color::NONE), 
          white_color(Color::NONE),
          last_move_color(Color::NONE) {}
      ~KanjiPrint() {}
    
      /**
       * 出力
       * @param state a state to show. 
       * @param last_move a color last_move is available unless it is null. 
       */
      void print(const SimpleState& state, 
                 const Move *last_move=NULL) const;
     
      void setBlackColor(const Color& c) {black_color = c;}
      void setWhiteColor(const Color& c) {white_color = c;}
      void setLastMoveColor(const Color& c) {last_move_color = c;}
    };
  } // namespace record
} // namespace osl

#endif /* RECORD_KANJIPRINT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
