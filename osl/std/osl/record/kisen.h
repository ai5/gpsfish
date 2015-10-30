#ifndef OSL_KISEN_H
#define OSL_KISEN_H

#include "osl/record/record.h"
#include "osl/numEffectState.h"
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <string>
#include <iosfwd>
#include <fstream>
namespace osl
{
  namespace record
  {
    class KisenUtils{
    public:
      static Square convertSquare( int pos );
      static Move convertMove(SimpleState const& state,int c0,int c1 );
      static int convertSquare(Square pos);
    };
    /**
     * 「.kif」という拡張子を持つ棋泉形式ファイル.
     * 手のみの情報が入っている
     */
    class KisenFile{
    private:
      NumEffectState initial_state;
      std::ifstream ifs;
      size_t number_of_games;
      const std::string filename;
    public:
      static const size_t MaxMoves=256;
      explicit KisenFile(const std::string& filename);
      ~KisenFile();

      size_t size() const{ return number_of_games; }
      NumEffectState initialState() const{ return initial_state; }
      std::vector<Move> moves(size_t index);
      std::string fileName() const { return filename; }
      std::string ipxFileName() const { return ipxFileName(filename); }
      static std::string ipxFileName(const std::string&);
    };
    /**
     * 「.ipx」という拡張子を持つ棋泉形式ファイル.
     * 対局者に関する情報(「プレイヤー名」，「レーティング」)
     * が分かっている．
     *
     * 1 record = 256Bytesの固定長が並ぶファイル。
     *    - 001-014: 先手 名前 [14]
     *    - 015-028: 後手 名前 [14]
     *    - 029-036: 先手 肩書き [8]
     *    - 037-044: 後手 肩書き [8]
     *    - 045-084: unknown
     *    - 085-090: 開始日の年月日時分 [6]
     *      - 085-086: 開始日の年 [2]
     *      - 087    : 月 [1]
     *      - 088    : 日 [1]
     *      - 089    : 時 [1]
     *      - 090    : 分 [1]
     *    - 091-096: 終了 年月日時分 [6]
     *    - 097-118: unknown
     *    - 119    : 結果フラグ [1]
     *    - 120-212: unknown
     *    - 213-214: 先手rating [2]
     *    - 215-216: 後手rating [2]
     *    - 217-256: unknown
     */
    class KisenIpxFile{
      std::ifstream ifs;
      size_t number_of_games;
      const std::string file_name;
    public:
      enum{
	BY_PARITY=0,
	BLACK_WIN=1,
	WHITE_WIN=2,
	SENNNICHITE=3,
	JISHOGI=4,
	BLACK_WIN_256=5,
	WHITE_WIN_256=6,
	SENNNICHITE_256=7,
	JISHOGI_256=8,
      };
      explicit KisenIpxFile(std::string const& filename);
      ~KisenIpxFile();
      
      size_t size() const{ return number_of_games; }
      std::string player(size_t index,Player pl);
      std::string title(size_t index,Player pl);
      unsigned int rating(size_t index,Player pl);
      unsigned int result(size_t index);
      const std::string& fileName() const { return file_name; }
      /**
       * 開始日の年月日を返す
       */
      boost::gregorian::date startDate(size_t index);
    };
    /**
     * 「.kpf」という拡張子を持つ棋泉プラス形式ファイル.
     * 手や消費時間の情報が入っている
     */
    class KisenPlusFile{
    private:
      NumEffectState initial_state;
      std::ifstream ifs;
      size_t number_of_games;
    public:
      static const size_t maxMoves=256;
      explicit KisenPlusFile(const std::string& fileName);
      size_t size() const{ return number_of_games; }
      NumEffectState initialState() const{ return initial_state; }
      std::vector<Move> moves(size_t index);
      void load(size_t index, std::vector<Move>&, std::vector<int>&);
    };

    class KisenWriter
    {
    public:
      KisenWriter(std::ostream &ostream) : os(ostream) { }
      void save(const RecordMinimal&);
    private:
      std::ostream &os;
    };

    /**
     * 「.ipx」という拡張子を持つ棋泉形式ファイル.
     * 対局者に関する情報(「プレイヤー名」，「レーティング」)
     * が分かっている．書かれるのは分かっている部分のみ。
     * 勝敗情報は不完全。千日手や持将棋の情報が Record にないため。
     */
    class KisenIpxWriter
    {
    public:
      KisenIpxWriter(std::ostream &ostream) : os(ostream) { }
      void save(const Record &,
		int black_rating, int white_rating,
		const std::string &black_title,
		const std::string &white_title);
    private:
      void writeString(const std::string &name, size_t length);
      void writeRating(int rating);
      void writeStartDate(int year, int month, int day, int hour, int min);
      std::ostream &os;
    };
  } // namespace record
  using record::KisenFile;
  using record::KisenIpxFile;
  using record::KisenWriter;
  using record::KisenIpxWriter;
} // namespace osl
#endif // OSL_KISEN_H 
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
