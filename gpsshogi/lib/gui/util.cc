#include "gpsshogi/gui/util.h"
#include "osl/eval/pieceEval.h"
#include "osl/record/ki2.h"
#include "osl/record/kanjiCode.h"
#include <qtextcodec.h>
#include <string>

QString gpsshogi::gui::
Util::getKanjiPiece(osl::Ptype p)
{
  static const std::string kanjiPiece[] =
    {"", "", "と", "杏", "圭", "全", "馬", "竜", "玉", "金", "歩", "香",
     "桂", "銀", "角", "飛"};
  return QString::fromUtf8(kanjiPiece[p].c_str(), kanjiPiece[p].length());
}

QString gpsshogi::gui::
Util::moveToString(osl::Move m)
{
  static const std::string kanjiNumber[] =
    {"零", "一", "二", "三", "四", "五", "六", "七", "八", "九"};
  static const std::string kanjiPlayer[] = {"▲", "△"};
  if (m.isPass())
    return "PASS";
  else if (m.isInvalid())
    return "INVALID";
  const osl::Square pos = m.to();
  const osl::Square fromPos = m.from();
  const std::string player = kanjiPlayer[playerToIndex(m.player())];

  QString result = QString::fromUtf8(player.c_str(), player.length());
  result.append(QString("%1%2%3").arg(pos.x())
		.arg(QString::fromUtf8(kanjiNumber[pos.y()].c_str()))
		.arg(Util::getKanjiPiece(m.ptype())));

  if (fromPos.isOnBoard())
  {
    result.append(QString(" (%1%2)").
		  arg(fromPos.x()).
		  arg(QString::fromUtf8(kanjiNumber[fromPos.y()].c_str())));
  }
  return result;
}

QString gpsshogi::gui::
Util::moveToString(osl::Move m, const osl::NumEffectState& state,
		   osl::Move last_move)
{
  std::string euc = osl::ki2::show(m, state, last_move);
  for (size_t i=0; euc.size() < 14; ++i)
    euc += K_SPACE;
  QTextCodec *codec = QTextCodec::codecForName("EUC-JP");
  return codec->toUnicode(euc.c_str(), euc.length());  
}

int gpsshogi::gui::
Util::compare(osl::Move m1, osl::Move m2)
{
  if ((! m1.isNormal()) && (! m2.isNormal()))
      return 0;
    else if (! m1.isNormal())
      return -1;
    else if (! m2.isNormal())
      return 1;

    if (m1.to().x() < m2.to().x())
      return -1;
    else if (m1.to().x() > m2.to().x())
      return 1;
    else
    {
      if (m1.to().y() < m2.to().y())
	return -1;
      else if (m1.to().y() > m2.to().y())
	return 1;
      else
      {
	int x1 = osl::eval::Ptype_Eval_Table.value(m1.ptype()) + osl::eval::Ptype_Eval_Table.value(osl::unpromote(m1.ptype()));
	int x2 = osl::eval::Ptype_Eval_Table.value(m2.ptype()) + osl::eval::Ptype_Eval_Table.value(osl::unpromote(m2.ptype()));
	if (x1 < x2)
	  return -1;
	else if (x1 > x2)
	  return 1;
	else return 0;
      }
    }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
