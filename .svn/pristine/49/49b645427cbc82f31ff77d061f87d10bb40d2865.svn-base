

#ifndef ORAQFILETOOLS_H_
#define ORAQFILETOOLS_H_

#include <string>
#include <vector>


namespace ora
{


/**
 * Encapsulates some general methods for file and file system access in open
 * radART world. This class is (partially) Qt-based and therefore separated
 * from oraFileTools.h.
 *
 * @see oraFileTools.h
 *
 * @author phil 
 * @version 1.4
 */
class GeneralFileTools
{
public:
  /**
   * Find files that match a specified pattern in a specified directory and
   * optionally in its sub-directories.
   * @param directory name of the base directory
   * @param filePattern an expression specifying the file pattern
   * @param recursive if TRUE, the sub-directories are also browsed (don't
   * worry, certainly it's not recursively done internally)
   * @param dirPattern an expression specifying the directory pattern for
   * sub-directory search (relevant if recursive is TRUE)
   * @return the list of found files matching the pattern (the files also
   * contain the absolute file path)
   */
  static std::vector<std::string> FindFilesInDirectory(std::string directory,
      std::string pattern = "*", bool recursive = false,
      std::string dirPattern = "*");

  /**
   * Delete files in a specified path with a specified pattern if they are
   * older than a specified threshold.
   * @param path the path where the files are located
   * @param pattern the file pattern which should be considered (e.g. "*.log")
   * @param maxAge the maximum age of the file specified in fractions of a day
   * @return the number of deleted files
   */
  static int DeleteOlderFiles(std::string path, std::string pattern,
      double maxAge);

};


}


#endif /* ORAQFILETOOLS_H_ */
