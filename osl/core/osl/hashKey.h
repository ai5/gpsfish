/* hashKey.h
 */
#ifndef OSL_HASH_KEY_H
#define OSL_HASH_KEY_H

#include "osl/basic_type.h"
#include "osl/bits/pieceStand.h"
#include "osl/simpleState.h"
#include <cstddef>

namespace osl
{
  namespace hash
  {
    struct BoardKey96 : public std::pair<uint64_t,uint32_t> 
    {
      BoardKey96() {}
      BoardKey96(const std::pair<uint64_t,uint32_t>& src)
	: std::pair<uint64_t,uint32_t>(src)
      {
      }
      uint32_t signature() const { return second; }
      size_t size() const { return 2; }
      uint64_t operator[](size_t i) const { return i ? first : second; }
    };
    class HashGenTable;
    struct HashKey128Layout
    {
      uint64_t board64;
      uint32_t board32, piece_stand;
    };
    /**
     * 手番を含んだ盤面の状態のハッシュ値を保持するためのクラス.
     * Board 96bit + 駒台(piece stand) 32bit
     */
    class HashKey128 : private HashKey128Layout
    {
      friend class HashGenTable;
    public:
      HashKey128() 
      {
	board64 = board32 = piece_stand = 0;
      }
      HashKey128(uint64_t h0, uint32_t h1, uint32_t s)
      {
	board64 = h0;
	board32 = h1;
	piece_stand = s;
      }
      HashKey128(const HashKey128Layout& src) : HashKey128Layout(src)
      {
      }
      const BoardKey96 boardKey() const {
	return std::make_pair(board64, board32); 
      }
      uint64_t boardKey64() const { return board64; }
      uint64_t signature() const { return board32; }
      /** 持駒も含んだ64bitのハッシュ */
      uint64_t hash64() const { return board64 + pieceStand64(); }
      uint64_t pieceStand64() const {
	return Stand_Hash.toUint64(pieceStand());
      }
      const PieceStand pieceStand() const{ return PieceStand(piece_stand); }
      const PieceStand blackStand() const { return PieceStand(piece_stand); }
      void setPieceStand(const PieceStand& p) { piece_stand=p.getFlags(); }

      /**
       * 駒台の情報を除いて同じかどうか.
       * 手番が異なるものは異なると定義する
       */
      bool isSameBoard(const HashKey128& key) const
      {
	return boardKey() == key.boardKey();
      }
      HashKey128& operator+=(const HashKey128& r)
      {
	board64 += r.board64;
	board32 += r.board32;
	PieceStand new_stand(piece_stand);
	new_stand.addAtmostOnePiece(r.pieceStand());
	piece_stand = new_stand.getFlags();
	return *this;
      }
      HashKey128& operator-=(const HashKey128& r)
      {
	board64 -= r.board64;
	board32 -= r.board32;
	PieceStand new_stand(piece_stand);
	new_stand.subAtmostOnePiece(r.pieceStand());
	piece_stand = new_stand.getFlags();
	return *this;
      }
      void add(Move move) { board64 += move.intValue(); }
      void changeTurn() { board64 ^= static_cast<uint64_t>(1); }
      void setPlayer(Player p) 
      { 
	board64 &= ~static_cast<uint64_t>(1);
	board64 |= playerToIndex(p);
      }
      bool playerBit() const { return board64 & 1; }
      bool isPlayerOfTurn(Player p) const
      {
	return playerBit() == playerToIndex(p);
      }
      Player turn() const { return isPlayerOfTurn(BLACK) ? BLACK : WHITE; }
      /**
       * 乱数で初期化.  
       * pieceStandには触らない
       */
      void setRandom();    
      size_t size() const { return 2; }
      uint64_t operator[](size_t i) const { return i ? board64 : board32; }
      struct StandHash
      {
	CArray<uint64_t, 19*3*3> HashMajorPawn;
	CArray<uint64_t, 5*5*5*5> HashPiece;
	StandHash();
	uint64_t toUint64(PieceStand stand) const 
	{
	  int major_pawn = stand.get(PAWN)*9
	    + stand.get(ROOK)*3 + stand.get(BISHOP);
	  int pieces = stand.get(GOLD)*125 + stand.get(SILVER)*25
	    + stand.get(KNIGHT)*5 + stand.get(LANCE);
	  return HashMajorPawn[major_pawn] + HashPiece[pieces];
	}
      };
      static const StandHash Stand_Hash;
    };
    inline bool operator==(const HashKey128& l, const HashKey128& r)
    {
      return l.boardKey() == r.boardKey() && l.pieceStand() == r.pieceStand();
    }
    inline bool operator!=(const HashKey128& l, const HashKey128& r)
    {
      return !(l==r);
    }
    /**
     * set等で使うためのみの不等号.  
     * full orderであること以外に深い意味はない
     */
    inline bool operator<(const HashKey128& l, const HashKey128& r)
    {
      if (l.pieceStand() < r.pieceStand())
	return true;
      else if (r.pieceStand() < l.pieceStand())
	return false;
      return l.boardKey() < r.boardKey();
    }

    typedef HashKey128 HashKeyBase;
    typedef BoardKey96 BoardKey;
    class HashKey : public HashKeyBase
    {
    public:
      HashKey() :HashKeyBase(){}
      HashKey(const SimpleState&);
      const HashKey newHashWithMove(Move move) const;
      const HashKey newMakeMove(Move) const;
      const HashKey newUnmakeMove(Move) const;

      void dumpContents(std::ostream& os) const;
      void dumpContentsCerr() const;
      static const HashKey readFromDump(const std::string&);
      static const HashKey readFromDump(std::istream&);
    };
    std::ostream& operator<<(std::ostream& os,const HashKey& h);

    class HashGenTable
    {
      static const CArray2d<HashKey128Layout,Square::SIZE,PTYPEO_SIZE> key;
    public:
      static void addHashKey(HashKey& hk,Square sq,PtypeO ptypeo) {
	assert(sq.isValid() && isValidPtypeO(ptypeo));
	hk += HashKey128(key[sq.index()][ptypeo-PTYPEO_MIN]);
      }
      static void subHashKey(HashKey& hk,Square sq,PtypeO ptypeo) {
	assert(sq.isValid() && isValidPtypeO(ptypeo));
	hk -= HashKey128(key[sq.index()][ptypeo-PTYPEO_MIN]);
      }
    };

  } // namespace hash
  using hash::HashKey;
  using hash::HashGenTable;
  using hash::BoardKey;
} // namespace osl

namespace std
{
  template <typename T> struct hash;
  template <>
  struct hash<osl::HashKey>{
    unsigned long operator()(const osl::HashKey& h) const {
      return h.signature();
    }
  };
  template<>
  struct hash<osl::BoardKey>
  {
    unsigned long operator()(const osl::BoardKey& h) const {
      return h.signature();
    }
  };
} // namespace stl

#endif /* OSL_HASH_KEY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
