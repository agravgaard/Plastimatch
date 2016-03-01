

#ifndef ORAFILETOOLS_H_
#define ORAFILETOOLS_H_


#include <vector>
#include <map>
#include <string>


namespace ora
{

/** OS-dependent directory separator. **/
#if (defined(_WIN32) || defined(_WIN64)) && !defined(__CYGWIN__)
  const std::string DSEP = "\\";
#else
  const std::string DSEP = "/";
#endif

/** \class UnixUNCConverter
 *  \brief Class for conversion between UNC network descriptions and UNIX paths.
 *
 * Class for conversion between UNC network descriptions and UNIX absolute
 * paths. This is especially interesting for mixed UNIX and Windows
 * environments. Implements the singleton design pattern.
 *
 * @author phil 
 * @version 1.2
 */
class UnixUNCConverter
{
public:
  /** @return Get singleton instance. **/
  static UnixUNCConverter *GetInstance();

  /**
   * Discard file drive information from a windows path (typically C:\ or
   * \\server\share\ from a windows path).
   * @param path path information (typically windows UNC)
   * @return the path with discarded drive information or empty string if
   * any error occurred
   **/
  static std::string DiscardWinFileDriveInformation(std::string path);

  /**
   * Add a translation pair to the internal translation table.
   * NOTE: the UNC-part is not case-sensitive.
   * @param unc UNC entry, e.g. "\\192.132.100.22\sharename"
   * @param unx UNIX entry, e.g. "/mount/sharename"
   */
  virtual void AddTranslationPair(std::string unc, std::string unx);

  /**
   * Clears all current translation pairs.
   */
  virtual void ClearTranslationPairs();

  /**
   * Lists all current translation pairs on the specified output stream.
   * @param os the output stream
   */
  virtual void ListTranslationPairs(std::ostream &os);

  /**
   * Load configured translation pairs from an INI file which contains a
   * section named [UNCvsUNIX] and the keys uncN=UNC-share and unixN=UNIX-mount.
   * @param fileName file name of translation pair file
   * @return true if successful ([UNCvsUNIX] found in specified file)
   */
  virtual bool LoadTranslationPairsFromFile(std::string fileName);

  /**
   * Ensure compatibility with UNIX environments. This means that paths which
   * contain known (from translation pairs) UNC-fragments are converted into
   * UNIX compatible paths. Additionally, the general path compatibility with
   * UNIX systems is ensured.
   * @param general path (or file) specification containing eventually UNC-
   * fragments
   * @return string compatible with UNIX environment (mounts instead of shares)
   */
  std::string EnsureUNIXCompatibility(std::string path);

  /**
   * Ensure compatibility with UNC environments. This means that paths which
   * contain known (from translation pairs) UNIX-mount-fragments are converted
   * into UNC compatible paths. Additionally, the general path compatibility
   * with Windows systems is ensured.
   * @param general path (or file) specification containing eventually UNIX-
   * mount-fragments
   * @return string compatible with UNC environment (shares instead of mounts)
   */
  std::string EnsureUNCCompatibility(std::string path);

  /**
   * Ensure compatibility with the compiled application's operating system.
   * (EnsureUNCCompatibility is compiled on Windows,
   *  EnsureUNIXCompatibility is compiled on UNIX)
   * @see EnsureUNCCompatibility()
   * @see EnsureUNIXCompatibility()
   */
  std::string EnsureOSCompatibility(std::string path);

  /**
   * Get specific UNCvsUNIX-section-name where the translation pairs are
   * stored (keys: unc{x}, unix{x}).
   */
  std::string GetUNCvsUNIXSectionName()
  {
    return m_UNCvsUNIXSectionName;
  }
  /**
   * Set specific UNCvsUNIX-section-name where the translation pairs are
   * stored (keys: unc{x}, unix{x}).
   */
  void SetUNCvsUNIXSectionName(std::string sectionName)
  {
    m_UNCvsUNIXSectionName = sectionName;
  }


protected:
  /** Internal types **/
  typedef std::map<std::string, std::string> ConversionTableType;

  /** UNC to UNIX translation table **/
  ConversionTableType m_UNCToUnixTable;
  /** UNIX to UNC translation table **/
  ConversionTableType m_UnixToUNCTable;
  /** specific UNCvsUNIX-section-name **/
  std::string m_UNCvsUNIXSectionName;

  /** Hidden constructor **/
  UnixUNCConverter();

  /** Hidden destructor **/
  virtual ~UnixUNCConverter();

private:
  /** Singleton instance of UnixUNCConverter. **/
  static UnixUNCConverter *m_Instance;

};


}


#endif /* ORAFILETOOLS_H_ */
