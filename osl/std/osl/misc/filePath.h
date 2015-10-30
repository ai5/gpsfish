/* filePath.h
 */
#ifndef OSL_MISC_CARRAY_H
#define OSL_MISC_CARRAY_H

#ifndef MINIMAL

#include <boost/filesystem/path.hpp>
#include <string>

namespace osl
{
  namespace misc
  {
    // Converts boost::filesystem::path to a string representation.
    // This function hides incompatibility among various boost::filesystem
    // versions.
    std::string file_string(const boost::filesystem::path& path);
  }
}

#endif /* MINIMAL */
#endif /* OSL_MISC_CARRAY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
