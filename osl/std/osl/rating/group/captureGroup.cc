/* captureGroup.cc
 */
#include "osl/rating/group/captureGroup.h"

osl::rating::CaptureGroup::CaptureGroup() : Group("Capture")
{
  see_range.push_back(-Capture::INF);
  see_range.push_back(-1050);
  see_range.push_back(-850);
  see_range.push_back(-650);
  see_range.push_back(-450);
  see_range.push_back(-250);
  see_range.push_back(-50);
  see_range.push_back(51);
  see_range.push_back(251);
  see_range.push_back(451);
  see_range.push_back(651);
  see_range.push_back(851);
  see_range.push_back(1051);
  see_range.push_back(Capture::INF);
  for (size_t i=0; i<see_range.size()-1; ++i)
    for (int p=0; p<8; ++p)	// progress8
      push_back(new Capture(see_range[i],see_range[i+1]));
}

osl::rating::DropCapturedGroup::DropCapturedGroup() : Group("DropCaptured")
{
  for (int pt=PTYPE_BASIC_MIN; pt<=PTYPE_MAX; ++pt) {
    for (int p=0; p<8; ++p)	// progress8
      push_back(new DropCaptured(static_cast<Ptype>(pt)));
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
