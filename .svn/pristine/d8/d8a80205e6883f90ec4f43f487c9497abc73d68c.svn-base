

#ifndef ORACOMPLEMENTARYMETAFILECACHE_H_
#define ORACOMPLEMENTARYMETAFILECACHE_H_


#include <itkObject.h>
#include <itkObjectFactory.h>

// Forward declarations
namespace ora
{
class IniAccess;
}


namespace ora 
{


/**
 * Represents a cache of loaded complementary meta information files of RTI
 * files.
 * @author phil 
 * @version 1.0
 */
class ComplementaryMetaFileCache
  : public itk::Object
{
public:
  /** some relevant typedefs **/
  typedef ComplementaryMetaFileCache Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ComplementaryMetaFileCache, itk::Object);

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Create a complementary meta file cache from a list of RTI images (slices).
   * @param rtiList list of RTI image files where a complementary meta image
   * file must be available with the file extension '.inf'
   * @return the pointer to the created cache if successful, NULL otherwise
   */
  static Pointer CreateFromRTIList(std::vector<std::string> rtiList);

  /**
   * Find a specified cached meta file in the internal map.
   * @param fileName of the cached meta file
   * @return the cached meta file if found, NULL otherwise
   **/
  IniAccess *FindEntry(const std::string fileName);

  /**
   * Add a cached meta file to the internal map.
   * @param cachedMetaFile the cached meta file to be added
   * @return TRUE if the cached meta file could be added
   */
  bool AddEntry(IniAccess *cachedMetaFile);

  /**
   * Remove a cached meta file from the list.
   * @param cachedMetaFile the cached meta file to be removed
   * @return TRUE if the cached meta file could be removed
   */
  bool RemoveEntry(IniAccess *cachedMetaFile);

  /**
   * Remove a cached meta file from the list.
   * @param cachedMetaFile the file name of the cached meta file to be removed
   * @return TRUE if the cached meta file could be removed
   */
  bool RemoveEntry(std::string fileName);

  /**
   * Clear the complete list.
   */
  void Clear();

  /**
   * @return the file names of the contained cached meta files.
   */
  std::vector<std::string> GetCachedMetaFileEntries();

  /**
   * @return the pointers of the contained cached meta files
   */
  std::vector<IniAccess *> GetCachedMetaFileEntryPointers();

protected:
  /** Complementary meta file map type **/
  typedef std::map<std::string, IniAccess *> MapType;

  /**
   * map of complementary RTI meta image files where the first component is the
   * file name and the second component is a pointer to the according
   * ora::IniAccess instance
   */
  MapType m_Map;

  /** Default constructor **/
  ComplementaryMetaFileCache();
  /** Default destructor **/
  ~ComplementaryMetaFileCache();

private:
  /** purposely not implemented **/
  ComplementaryMetaFileCache(const Self &);
  /** purposely not implemented **/
  void operator=(const Self &);

};


}


#endif /* ORACOMPLEMENTARYMETAFILECACHE_H_ */
