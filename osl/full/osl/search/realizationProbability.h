/* realizationProbability.h
 */
#ifndef INCLUDED_REALIZATIONPROBABILITY
#define INCLUDED_REALIZATIONPROBABILITY

namespace osl
{
  namespace search
  {
    struct RealizationProbability
    {
      enum {
	DefaultProb = 500,
	ReSearch    = 100,
	KillerMove  = 200,
	TableMove   = 100,
      };
    };

    struct FullWidthMoveProbability
    {
      enum {
	DefaultProb = 500,
	ReSearch    = 500,
	KillerMove  = 500,
	TableMove   = 500,
      };
    };
  } // namespace search

  using search::RealizationProbability;
  using search::FullWidthMoveProbability;
} // namespace osl

#endif /* INCLUDED_REALIZATIONPROBABILITY */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
