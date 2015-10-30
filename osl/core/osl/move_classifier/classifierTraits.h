/* classifierTraits.h
 */
#ifndef OSL_CLASSIFIERTRAITS_H
#define OSL_CLASSIFIERTRAITS_H

namespace osl
{
  namespace move_classifier 
  {
    template <class T> struct ClassifierTraits
    {
      static const bool drop_suitable = true;
    };
  }
}

#endif /* OSL_CLASSIFIERTRAITS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
