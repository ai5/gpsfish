/* problems.cc
 */
#include "problems.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>

const osl::Problem osl::problems1[] = 
{
  {				// 合法手無し
    "P1 *  *  *  *  *  * -KI-KE-OU\n"
    "P2 *  *  *  *  *  * -KE-KA-KY\n"
    "P3 *  *  *  *  *  * -FU+KE-FU\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7-FU-GI *  *  *  *  *  *  * \n"
    "P8 * -FU *  *  *  *  *  *  * \n"
    "P9+OU *  *  *  *  *  *  *  * \n"
    "P+00AL\n"
    "-",
    { "%TORYO", }
  },
  {
    "P1+KA+TO+TO+HI * +TO-KI-KE-OU\n"
    "P2+KA+TO+TO+KY+FU+TO-FU-FU-KY\n"
    "P3+HI * +FU+FU *  * -KI-GI-FU\n"
    "P4+KE+KE+KY+KY *  *  *  *  * \n"
    "P5+FU+KE *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 * -GI * +GI * +FU+FU *  * \n"
    "P8 * -FU *  * +KI *  *  *  * \n"
    "P9+OU *  *  *  *  *  *  *  * \n"
    "P-00FU\n"
    "P+00AL\n"
    "-\n",
    { "-0097FU", }		// -0098FUは打歩詰
  },
  {
    "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
    "P2 * -HI *  *  *  * -KI *  * \n"
    "P3-FU * -FU-FU-FU-FU-KA * -FU\n"
    "P4 * -FU *  *  *  * -FU+FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "P+00FU\n"
    "+\n",
    { "+2423TO", }
  },
  {
    "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
    "P2 * -HI *  *  *  * -KI-KA * \n"
    "P3-FU * -FU-FU-FU-FU-FU-FU-FU\n"
    "P4 * -FU *  *  *  *  * +FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "-\n",
    { "-2324FU", }
  },
  {
    "P1-KY-KE-GI-KI-OU-KI * -KE-KY\n"
    "P2 *  *  *  *  *  *  * -GI * \n"
    "P3-FU *  * -FU-FU-FU * -FU-FU\n"
    "P4 *  * -FU *  *  * -FU *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 * -HI *  *  *  *  *  *  * \n"
    "P7+FU *  * +FU+FU+FU+FU+FU+FU\n"
    "P8 *  * +HI *  * +OU *  *  * \n"
    "P9+KY+KE+GI+KI * +KI+GI+KE+KY\n"
    "P+00KA00FU\n"
    "P-00KA00FU00FU\n"
    "+\n",
    { "+0095KA", }
  },
  {
    "P1-KY-KE-GI-KI-OU-KI * -KE-KY\n"
    "P2 * -HI *  *  * -GI * -KA * \n"
    "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
    "P4 *  *  *  *  *  * -FU *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  * +FU *  *  *  *  *  * \n"
    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "+\n",
    { "+8822UM", }
  },
};
const int osl::numProblems1 = sizeof(osl::problems1)/sizeof(osl::Problem);

const osl::Problem osl::problems3[] = 
{
  {
    "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
    "P2 * -HI *  *  *  * -KI-KA * \n"
    "P3-FU * -FU-FU-FU-FU-FU * -FU\n"
    "P4 * -FU *  *  *  *  * -FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "P-00FU\n"
    "+\n",
    { "+2824HI", }
  },
#if 0
  {
    // 無駄な王手をかけずに潔良く投了する
    // 3手読みが必要
    "P1 *  *  *  *  *  * -KI-KE-OU\n"
    "P2 *  *  *  *  *  * -KI-GI-KY\n"
    "P3 *  *  *  *  *  * -FU * -FU\n"
    "P4 *  *  *  *  *  *  * -FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7-KI-GI *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9+OU *  *  *  *  *  *  *  * \n"
    "P+00KE\n"
    "P-00AL\n"
    "+\n",
    { "%TORYO", }
  },
#endif
};
const int osl::numProblems3 = sizeof(osl::problems3)/sizeof(osl::Problem);


//	"P1 *  *  *  *  *  *  *  *  * \n"
//	"P2 *  *  *  *  *  *  *  *  * \n"
//	"P3 *  *  *  *  *  *  *  *  * \n"
//	"P4 *  *  *  *  *  *  *  *  * \n"
//	"P5 *  *  *  *  *  *  *  *  * \n"
//	"P6 *  *  *  *  *  *  *  *  * \n"
//	"P7 *  *  *  *  *  *  *  *  * \n"
//	"P8 *  *  *  *  *  *  *  *  * \n"
//	"P9 *  *  *  *  *  *  *  *  * \n"
//	"P+\n"
//	"P-\n"
//	"+\n",

bool osl::Problem::acceptable(Move m) const
{
  std::stringstream ss;
  ss << csa::show(m);
  for (int i=0; i<MaxExpected; ++i)
  {
    if (! expected[i])
      return false;
    if (ss.str() == expected[i])
      return true;
    std::cerr << ss.str() << " != " << expected[i] << "\n";
  }
  return false;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
