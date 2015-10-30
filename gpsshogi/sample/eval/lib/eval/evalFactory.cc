#include "eval/evalFactory.h"

#include "eval/eval.h"
#include "eval/progress.h"
#include "eval/progressFeature.h"
#include "eval/openMidEnding.h"
#include "eval/progressEval.h"
#include "osl/eval/openMidEndingEval.h"

gpsshogi::Eval *
gpsshogi::EvalFactory::newEval(const std::string &name)
{
  if (name == "piece") {
    return new gpsshogi::PieceEval();
  }
  else if (name == "frich2") {
    return new gpsshogi::RichEval(2, true);
  }
  else if (name == "progresseval") {
    return new gpsshogi::KProgressEval();
  }
  else if (name == "stableprogresseval") {
    return new gpsshogi::StableProgressEval();
  }
#if (defined LEARN_TEST_PROGRESS)
  else if (name == "handprogress") {
    return new gpsshogi::HandProgressFeatureEval();
  }
  else if (name == "effectprogress") {
    return new gpsshogi::EffectProgressFeatureEval();
  }
  else if (name == "stableeffectprogress") {
    return new gpsshogi::StableEffectProgressFeatureEval();
  }
#endif
  else if (name == "openmidendingtest0") {
    return new gpsshogi::OpenMidEndingForTest(0);
  }
  else if (name == "openmidendingtest1") {
    return new gpsshogi::OpenMidEndingForTest(1);
  }
  else if (name == "kopenmidending") {
    return new gpsshogi::KOpenMidEnding();
  }
  else if (name == "stableopenmidending") {
    return new gpsshogi::StableOpenMidEnding();
  }
  else if (name == "oslopenmidending") {
    return new gpsshogi::OslOpenMidEnding();
  }
  return NULL;
}
