#ifndef GPSSHOGI_DBM_TOKYO_CABINET_H
#define GPSSHOGI_DBM_TOKYO_CABINET_H
#include "gpsshogi/dbm/dbm.h"
#include <boost/noncopyable.hpp>
#include <exception>
#include <tcutil.h>
#include <tchdb.h>

namespace gpsshogi
{
  namespace dbm
  {
    class NotOpenedException : public std::exception
    {
    public:
      const char* what() const throw()
      {
	return "DBM not opened";
      }
    };

    class TokyoCabinetWrapper : public DBMWrapper, boost::noncopyable
    {
    public:
      TokyoCabinetWrapper(const std::string &filename,
			  int flag=0);
      ~TokyoCabinetWrapper();
      bool hasKey(const std::string &key);
      bool get(const std::string &key, std::string &value);
      bool put(const std::string &key, const std::string &value);
      // Not C++ iterator because there could exist only one iterator.
      void initIterator();
      bool next(std::string &key, std::string &value);

      /** @param new_bucket use tokyo cabinet default (=size()*2) if 0 */
      bool resize(size_t new_bucket=0);
      uint64_t size();
      uint64_t bucketSize();
      bool optimize();
    private:
      TCHDB *hdb;
      bool opened;
    };
  }
}
#endif // GPSSHOGI_DBM_TOKYO_CABINET_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

