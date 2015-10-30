/* ratio.h
 */
#ifndef _RATIO_H
#define _RATIO_H

#include "osl/stat/average.h"

namespace osl
{
  namespace stat
  {
    class Ratio
    {
      Average ave;
      const char *name;
      bool show_on_destructor;
    public:
      Ratio(const char *n=0, bool show=false) : name(n), show_on_destructor(show)
      {
      }
      ~Ratio();
      void add(bool success) { ave.add(success ? 1.0 : 0.0); }
      double ratio() const { return ave.average(); }
      void show() const;
      void clear() { ave.clear(); }
    };
  } // namespace stat
} // namespace osl


#endif /* _RATIO_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
