#ifndef OSL_RECORD_KANJI_CODE_H
#define OSL_RECORD_KANJI_CODE_H

#include <string>

namespace osl
{
  namespace record
  {
    /**
     * Define Kanji characters by using EUC-JP codes.
     */
#define K_BLACK_SIGN   "\xA2\xA5" // ▲
#define K_WHITE_SIGN   "\xA2\xA4" // △
#define K_SPACE        "\xA1\xA1" // "　"
#define K_NAKAGURO     "\xA1\xA6" // ・
#define K_COLON        "\xA1\xA7" // ：
#define K_R1           "\xA3\xB1" // １
#define K_R2           "\xA3\xB2" // ２
#define K_R3           "\xA3\xB3" // ３
#define K_R4           "\xA3\xB4" // ４
#define K_R5           "\xA3\xB5" // ５
#define K_R6           "\xA3\xB6" // ６
#define K_R7           "\xA3\xB7" // ７
#define K_R8           "\xA3\xB8" // ８
#define K_R9           "\xA3\xB9" // ９
#define K_K1           "\xB0\xEC" // 一
#define K_K2           "\xC6\xF3" // 二
#define K_K3           "\xBB\xB0" // 三
#define K_K4           "\xBB\xCD" // 四
#define K_K5           "\xB8\xDE" // 五
#define K_K6           "\xCF\xBB" // 六
#define K_K7           "\xBC\xB7" // 七
#define K_K8           "\xC8\xAC" // 八
#define K_K9           "\xB6\xE5" // 九
#define K_K10          "\xBD\xBD" // 十
#define K_K11          "\xBD\xBD\xA3\xB1" // 十一
#define K_K12          "\xBD\xBD\xA3\xB2" // 十二
#define K_K13          "\xBD\xBD\xBB\xB0" // 十三
#define K_K14          "\xBD\xBD\xBB\xCD" // 十四
#define K_K15          "\xBD\xBD\xB8\xDE" // 十五
#define K_K16          "\xBD\xBD\xCF\xBB" // 十六
#define K_K17          "\xBD\xBD\xBC\xB7" // 十七
#define K_K18          "\xBD\xBD\xC8\xAC" // 十八
#define K_MIGI         "\xB1\xA6" // 右
#define K_HIDARI       "\xBA\xB8" // 左
#define K_UE           "\xBE\xE5" // 上
#define K_SHITA        "\xB2\xBC" // 下
#define K_SUGU         "\xC4\xBE" // 直
#define K_YORU         "\xB4\xF3" // 寄
#define K_HIKU         "\xB0\xFA" // 引
#define K_YUKU         "\xB9\xD4" // 行
#define K_ONAZI        "\xC6\xB1" // 同
#define K_NARU         "\xC0\xAE" // 成
#define K_FUNARI       "\xC9\xD4" K_NARU // 不成
#define K_UTSU         "\xC2\xC7" // 打
    // pieces
#define K_PAWN         "\xCA\xE2" // 歩
#define K_PAWN_R       "\xA7\xAE" // М
#define K_PPAWN        "\xA4\xC8" // と
#define K_PPAWN_R      "\xA7\xAF" // Н
#define K_LANCE        "\xB9\xE1" // 香
#define K_LANCE_R      "\xA7\xAC" // К
#define K_PLANCE       "\xB0\xC9" // 杏
#define K_PLANCE_R     "\xA7\xAD" // Л
#define K_PLANCE_D     K_NARU "\xB9\xE1" // 成香
#define K_KNIGHT       "\xB7\xCB" // 桂
#define K_KNIGHT_R     "\xA7\xAA" // И
#define K_PKNIGHT      "\xB7\xBD" // 圭
#define K_PKNIGHT_R    "\xA7\xAB" // Й
#define K_PKNIGHT_D    K_NARU "\xB7\xCB" // 成桂
#define K_SILVER       "\xB6\xE4" // 銀
#define K_SILVER_R     "\xA7\xA8" // Ж
#define K_PSILVER      "\xC1\xB4" // 全
#define K_PSILVER_R    "\xA7\xA9" // З
#define K_PSILVER_D    K_NARU "\xB6\xE4" // 成銀
#define K_GOLD         "\xB6\xE2" // 金
#define K_GOLD_R       "\xA7\xA7" // Ё
#define K_BISHOP       "\xB3\xD1" // 角
#define K_BISHOP_R     "\xA7\xA4" // Г
#define K_PBISHOP      "\xC7\xCF" // 馬
#define K_PBISHOP_R    "\xA7\xA5" // Д
#define K_ROOK         "\xC8\xF4" // 飛
#define K_ROOK_R       "\xA7\xA2" // Б
#define K_PROOK        "\xCE\xB6" // 龍
#define K_PROOK2       "\xCE\xB5" // 竜
#define K_PROOK_R      "\xA7\xA3" // В
#define K_KING         "\xB6\xCC" // 玉
#define K_KING2        "\xB2\xA6" // 王
#define K_KING_R       "\xA7\xA1" // А
    //
#define K_BLACK        "\xC0\xE8\xBC\xEA" // 先手
#define K_WHITE        "\xB8\xE5\xBC\xEA" // 後手
#define K_MOCHIGOMA    "\xBB\xFD\xB6\xF0" // 持駒
#define K_BLACK_STAND  K_BLACK K_MOCHIGOMA // 先手持駒
#define K_WHITE_STAND  K_WHITE K_MOCHIGOMA // 後手持駒
#define K_TEAIWARI     "\xBC\xEA\xB9\xE7\xB3\xE4" // 手合割
#define K_PASS         "\xA5\xD1\xA5\xB9" // パス
#define K_SENKEI       "\xC0\xEF\xB7\xBF"  // 戦型
#define K_TORYO        "\xc5\xea\xce\xbb"  // 投了
#define K_HENKA        "\xca\xd1\xb2\xbd" // 変化
#define K_KI	       "\xb4\xfd"	  // 棋
#define K_KISEN        K_KI "\xc0\xef" // 棋戦
#define K_KIFU         K_KI "\xc9\xe8" // 棋譜
#define K_TSUMERO      "\xb5\xcd\xa4\xe1\xa4\xed" // 詰めろ
#define K_KAISHI       "\xb3\xab\xbb\xcf" // 開始
#define K_NICHIJI      "\xc6\xfc\xbb\xfe" // 日時
#define K_TESUU        "\xbc\xea\xbf\xf4" // 手数
#define K_NASHI        "\xa4\xca\xa4\xb7" // なし
#define K_DAN          "\xc3\xca"	  // 段
#define K_KANMURI      "\xb4\xa7"	  // 冠
#define K_KURAI        "\xb0\xcc"	  // 位
#define K_SHOU         "\xbe\xad"	  // 将
#define K_SEI          "\xc0\xbb"	  // 聖
#define K_ZA           "\xba\xc2"	  // 座
#define K_MEIJIN       "\xcc\xbe\xbf\xcd"  // 名人
#define K_JORYUU       "\xbd\xf7\xce\xae"  // 女流
#define K_RESIGN       "\xc5\xea\xce\xbb"  // 投了
  } // namespace record
} // namespace osl


#endif /* OSL_RECORD_KANJI_CODE_H */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
