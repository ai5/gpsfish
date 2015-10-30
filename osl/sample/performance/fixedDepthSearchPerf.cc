#include "osl/csa.h"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include "osl/state/numEffectState.h"
#include "osl/misc/perfmon.h"
#include <time.h>
#include <sys/time.h>
#include <iostream>
using namespace osl;
using namespace osl::checkmate;

int main()
{
  SimpleState state=CsaString(
"P1-KY *  *  * -KY * -FU-KE * \n"
"P2 *  *  *  * -OU *  *  *  * \n"
"P3 *  *  * -FU-FU *  *  * -KY\n"
"P4-FU *  * -GI *  *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6+FU *  * +RY * +HI+FU *  * \n"
"P7 * +FU * +FU+FU+FU *  *  * \n"
"P8 *  * +OU * -TO *  *  *  * \n"
"P9+KY *  *  *  *  *  * +KE * \n"
"P+00KI00GI00GI00GI00KE00KE00FU00FU00FU00KI\n"
"P-00KA00KA00KI00FU00FU00FU00FU00KI\n"
"+\n").getInitialState();
  NumEffectState eState(state);
  ProofDisproof pdp;
  osl::misc::PerfMon perfmon;
  FixedDepthSearcher searcher(eState);
  perfmon.restart();
  searcher.setCount(0);
  for(int i=0;i<1000;i++)
    {
      Move dummy;
    pdp=searcher.hasCheckmateMove<BLACK>(1,dummy);
  }
  perfmon.stop("total", searcher.getCount());
  std::cerr << pdp << std::endl;
  return 0;
}
