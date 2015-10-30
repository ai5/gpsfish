#ifndef _GPS_GPSSHELL_IGNORELIST_H
#define _GPS_GPSSHELL_IGNORELIST_H

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>
#include <vector>

  namespace gpsshell
  {
    /**
     * Read and hold an ignore list file.
     */
    class IgnoreList
    {
    public:
      /**
       * A set of a csa file path, n-th move and comment message.
       */
      typedef std::tuple<boost::filesystem::path, size_t, std::string> index_t;

    private:
      boost::filesystem::path index_path; /**< current ignore list file */
      std::vector<index_t> lines;         /**< contents of the file */
      size_t current;                     /**< current line of the file */

    public:
      /**
       * Constructor.
       */
      IgnoreList()
        : current(0)
      {}

      /**
       * See if this object includes valid lines (i.e. not empty).
       */
      bool isEmpty() const 
      {
        return lines.empty();
      }

      /**
       * See if the current index can move forward.
       */
      bool hasNext() const
      {
        return ! isEmpty() &&
               (current < lines.size()-1);
      }

      /**
       * See if the current index can move backward.
       */
      bool hasPrev() const
      {
        return ! isEmpty() && (current > 0);
      }

      /**
       * Addvance the current index to the next index.
       */
      void next();

      /**
       * Get back the current index to the previous index.
       */
      void prev();

      /**
       * Move to the first index.
       */
      void first();

      /**
       * Move to the last index.
       */
      void last();

      /**
       * Return the current index (starting with zero) of the line.
       */
      index_t getCurrentIndex() const 
      {
        assert(!isEmpty());
        return lines.at(current);
      }

      /**
       * Open an ignore list file and read the contents.
       * @param a file path.
       */
      void openFile(const std::string& file);
    };
  }

#endif /* _GPS_GPSSHELL_IGNORELIST_H */
