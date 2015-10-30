/* show-eval.cc
 */
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include "osl/record/csaRecord.h"
#include "osl/stat/variance.h"
#include "osl/oslConfig.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace osl;
using namespace osl::eval;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " csa-filename"
       << endl;
  exit(1);
}

void show(const char *filename);
void finish();

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;

  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "e:vh")) != EOF)
  {
    switch(c)
    {
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag)
    usage(program_name);

  eval::ml::OpenMidEndingEval::setUp();
  progress::ml::NewProgress::setUp();
  
  for (int i=0; i<argc; ++i)
  {
    show(argv[i]);
  }
  finish();
}

using namespace osl::eval::ml;
using osl::stat::Variance;
CArray<CArray<Variance,4>, OpenMidEndingEvalDebugInfo::STAGE_FEATURE_LIMIT>
stage_features;
CArray<Variance, OpenMidEndingEvalDebugInfo::PROGRESS_INDEPENDENT_FEATURE_LIMIT>
progress_independent_features;

void show(const NumEffectState& state)
{
  OpenMidEndingEval eval(state);
  const OpenMidEndingEvalDebugInfo info = eval.debugInfo(state);
  for (int i=0; i<OpenMidEndingEvalDebugInfo::STAGE_FEATURE_LIMIT; ++i)
    for (int s=0; s<4; ++s)
      stage_features[i][s].add(info.stage_values[i][s]);
  for (int i=0; i<OpenMidEndingEvalDebugInfo::PROGRESS_INDEPENDENT_FEATURE_LIMIT; ++i)
    progress_independent_features[i].add(info.progress_independent_values[i]);}

void finish()
{
  for (int i=0; i<OpenMidEndingEvalDebugInfo::STAGE_FEATURE_LIMIT; ++i)
    for (int s=0; s<4; ++s)
      std::cout << OpenMidEndingEvalDebugInfo::name((OpenMidEndingEvalDebugInfo::StageFeature)i)
		<< s
		<< " " << stage_features[i][s].average()
		<< " " << sqrt(stage_features[i][s].variance()) << "\n";
  for (int i=0; i<OpenMidEndingEvalDebugInfo::PROGRESS_INDEPENDENT_FEATURE_LIMIT; ++i)
    std::cout << OpenMidEndingEvalDebugInfo::name((OpenMidEndingEvalDebugInfo::ProgressIndependentFeature)i)
	      << " " << progress_independent_features[i].average()
	      << " " << sqrt(progress_independent_features[i].variance()) << "\n";
}

void show(const char *filename)
{
  CsaFile file(filename);
  const auto moves = file.load().moves();
  NumEffectState state(file.load().initialState());
  for (unsigned int i=0; i<moves.size(); i++)
  {
    show(state);
    const Move m = moves[i];
    state.makeMove(m);
  }
  show(state);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
