#include "osl/misc/filePath.h"

#ifndef MINIMAL

std::string osl::misc::
file_string(const boost::filesystem::path& path)
{
#if BOOST_FILESYSTEM_VERSION >=3
  return path.string();
#else
  return path.file_string();
#endif
}

#endif /* MINIMAL */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
