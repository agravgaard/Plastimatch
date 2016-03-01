

#ifndef ORARTITOMHDCONVERTER_H_
#define ORARTITOMHDCONVERTER_H_

// ORAIFTools
#include "SimpleDebugger.h"
#include "oraITKVTKImageMetaInformation.h"
#include "oraFileTools.h"

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkSmartPointer.h>
#include <itkImage.h>


// Forward declarations
namespace ora
{
class CompactRTISliceMetaInfo;
}


namespace ora 
{


/**
 * FIXME
 * Generates a 3D image in (extended) metaimage format out of a series of
 * 2D RTI images which meet the following criteria:<br>
 * <p>
 * (1) the RTI images are 16-bit streams with valid header data,<br>
 * (2) the RTI images are all of same size,<br>
 * (3) and the RTI images are coplanar.<br>
 * </p>
 * The resultant image is templated over its pixel type which can - certainly -
 * lead to information loss.
 * @author phil 
 * @version 1.2
 */
template <typename TPixelType>
class RTIToMHDConverter
  : public itk::Object, public SimpleDebugger
{
public:
  /** Public standard types **/
  typedef RTIToMHDConverter<TPixelType> Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  typedef itk::Image<TPixelType, 3> ImageType;
  typedef typename ImageType::Pointer ImagePointer;

  typedef ora::ITKVTKImageMetaInformation MetaInfoType;
  typedef MetaInfoType::Pointer MetaInfoPointer;

  typedef std::vector<std::string> StrLstType;
  typedef std::vector<StrLstType *> StrLstVecType;

  typedef std::vector<CompactRTISliceMetaInfo *> MetaVectorType;
  typedef std::vector<MetaVectorType *> MetaVectorVectorType;

   /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(RTIToMHDConverter, itk::Object);

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Set the slice list to the values specified in the argument. Basically the
   * sort order of this file name list controls the order of slice -> volume
   * creation. Using the string-based or meta-information-based sort-mechanisms
   * of this class, the sort order can be automatically adjusted.
   * @see SortSlicesByFileNames(bool)
   * @see ReSortSliceListByMetaInformation(bool)
   */
  void SetSliceList(StrLstType &sliceList)
  {
    m_SliceList.clear();
    m_SliceList.assign(sliceList.begin(), sliceList.end());
    m_MetaInfo = NULL; // set it back!
    ClearAvailableSliceStacks();
  }
  /** Get a pointer to the internal slice list allowing manipulations **/
  StrLstType *GetSliceList()
  {
    m_MetaInfo = NULL; // set it back because manipulations are possible
    ClearAvailableSliceStacks();
    return &m_SliceList;
  }
  /**
   * Set the slice list to the values specified in the image list file
   * (m_ImageListFile). Therefore, the image list file must be set to a
   * valid value.
   * @see SetSliceList(StrLstType)
   */
  void SetSliceListFromImageListFile();
  /**
   * Set the slice list to the values which result from recursively browsing
   * the specified base folder.
   * @param baseFolder the base folder which will be recursively browsed for
   * available RTI files
   */
  void SetSliceListByBaseFolder(std::string baseFolder);

  /**
   * Sort the set slice (file name) list by the file names w.r.t. open radART's
   * naming conventions.<br>
   * NOTE: this method really operates on the file names and does not extract
   * any meta-information from the RTI-headers for sorting!
   * @param ascending if TRUE then the sort order is ascending, otherwise
   * descending
   **/
  virtual void SortSlicesByFileNames(bool ascending);

  /**
   * Sort the set slice (file name) list with respect to the configured
   * meta information.<br>
   * NOTE: this method applies to the complete slice list (out of image list
   * file) - for a sub-slice-list explicitly set use
   * ReSortSliceListByMetaInformation(bool)!!! <br>
   * NOTE: this method operates on the meta information of the images which
   * requires reading of the complementary inf-files! This may cause network
   * traffic and additional hard disk operations which require more time!
   * The slice list is cleared at the end of this method - the slices must then
   * explicitly be selected using the available slice stacks information!
   * @param ascending if TRUE then the sort order is ascending, otherwise
   * descending
   **/
  virtual void SortSlicesByMetaInformation(bool ascending);

  /**
   * Sort the set slice list (sub-slice-list set by using SetSliceList()) w.r.t.
   * the meta information found in the image list file. <br>
   * NOTE: internally, only the meta information found in image list is
   * used (not each single meta info file)! <br>
   * NOTE: be careful, calling this method 'destroys' (misuses) the
   * available slice stacks information which can be retrieved using
   * GetAvailableSliceStacks()! If you use this information after calling
   * this method, be prepared to update this information!
   * @param ascending if TRUE then the sort order is ascending, otherwise
   * descending
   */
  virtual void ReSortSliceListByMetaInformation(bool ascending);

  /**
   * Explicitly select a specified available slice stack from a generated list.
   * @param stack a pointer to the stack which should become selected; if
   * NULL is applied the current selection is cleared
   * @return TRUE if the slice stack has successfully been selected!
   **/
  virtual bool SelectAvailableSliceStack(StrLstType *stack);

  /**
   * Explicitly select a specified available slice stack from a generated list.
   * @param index the index of the stack that should become selected within the
   * available slice stack list
   * @return TRUE if the slice stack has successfully been selected!
   **/
  virtual bool SelectAvailableSliceStack(unsigned int index);

  /**
   * Build a 3D image volume from the set slice list. The slice list is
   * expected to be sorted w.r.t. to some meaningful criteria when this
   * method is called.<br>
   * NOTE: the internal volume image is set back in this method; therefore,
   * only successful application of this method guarantees a valid image!
   * @param expectedMinimumNumberOfSlices [default: 0] specifies the expected
   * minimum number of slices that the volume must have (after validation of
   * slice position, parallelism etc.); if this minimum number of slices is
   * not fulfilled, the method does not create a volume and returns immediately
   * FALSE; if this parameter is euqal or less than 0 (default), the number of
   * slices is ignored
   * @return TRUE if an image could successfully be created
   */
  virtual bool BuildImageFromSlices(int expectedMinimumNumberOfSlices = 0);

  /**
   * Get a pointer to the loaded image. <br>
   * NOTE: disconnected from pipeline!
   */
  ImagePointer GetImage()
  {
    return m_Image;
  }

  /**
   * Get a pointer to the loaded image meta information. <br>
   * NOTE: the object is not deleted if this object is destroyed. It must
   * externally be freed.
   */
  MetaInfoPointer GetMetaInfo()
  {
    return m_MetaInfo;
  }

  /**
   * Save the loaded (built) volume image as (extended) metaimage.
   * @param fileName file name of the volume image; if the file extension
   * differs from "mhd", this extension is automatically appended
   * @param compress if TRUE, the image is compressed
   * @param intermediateDirectory if this parameter is not empty the image is
   * first stored (streamed) to the specfied directory, then copied to the
   * real destination, then the image is deleted from the intermediate
   * directory; this can be useful when the image is stored to network resources
   * as this can be buggy during streaming; this parameter MUST HAVE A
   * TRAILING SEPARATOR!
   * @return TRUE if successful
   */
  virtual bool SaveImage(std::string fileName, bool compress,
      std::string intermediateDirectory);

  /** Get patient's image list file (necessary for complete image-build). **/
  virtual std::string GetImageListFile()
  {
    return m_ImageListFile;
  }
  /** Set patient's image list file (necessary for complete image-build). **/
  virtual void SetImageListFile(std::string imageListFile)
  {
    m_MetaInfo = NULL; // set it back!
    ClearAvailableSliceStacks();
    m_ImageListFile = UnixUNCConverter::GetInstance()->EnsureOSCompatibility(
      imageListFile);
  }

  /** Get patient's frame of reference file (necessary for complete image-build). **/
  virtual std::string GetFrameOfReferenceFile()
  {
    return m_FrameOfReferenceFile;
  }
  /** Set patient's frame of reference file (necessary for complete image-build). **/
  virtual void SetFrameOfReferenceFile(std::string frameOfReferenceFile)
  {
    m_MetaInfo = NULL; // set it back!
    ClearAvailableSliceStacks();
    m_FrameOfReferenceFile = UnixUNCConverter::GetInstance()->
        EnsureOSCompatibility(frameOfReferenceFile);
  }

  /**
   * Initialize the converter for RTI to (extended) MHD conversion.
   * @param preCacheMetaInfoFiles if TRUE then the corresponding meta
   * information files are pre-cached (e.g. necessary for sorting based on
   * meta information using more than just the image list information)
   */
  virtual void Initialize(bool preCacheMetaInfoFiles);

  /**
   * Get a list of acquisition types (case-sensitive) for single RTI image
   * files whereof a volume can be calculated (referred to as 'stackable').
   * Direct pointer to internal list which can be manipulated.
   **/
  StrLstType *GetStackableAcquisitionTypes()
  {
    return &m_StackableAcquisitionTypes;
  }

  /**
   * Get a list of logically available slice stacks which were calculated by
   * sorting based on meta information. The information provided are full-path
   * file names. Synchronized with the information retrievable from
   * GetAvailableSliceStacksMeta().
   * @see GetAvailableSliceStacksMeta()
   * @see SortSlicesByMetaInformation(bool)
   */
  StrLstVecType *GetAvailableSliceStacks()
  {
    return &m_AvailableSliceStacks;
  }
  /**
   * Get a list of logically available slice stack meta information which was
   * calculated by sorting based on meta information. The information provided
   * is slice-based, but can be used for slice-/stack-selection (GUI-feed).
   * Synchronized with the information retrievable from
   * GetAvailableSliceStacks().
   * @see GetAvailableSliceStacks()
   * @see SortSlicesByMetaInformation(bool)
   */
  MetaVectorVectorType *GetAvailableSliceStacksMeta()
  {
    return &m_AvailableSliceStacksMeta;
  }

  /**
   * Set the meta information mode flag.
   * Use only the information found in the image list for sorting based on
   * meta information. This mode is expected to accelerate the process of
   * sorting as the meta information files are not pre-cached.
   */
  void SetUseImageListInformationOnlyForMetaSorting(bool useImageListOnly)
  {
    m_UseImageListInformationOnlyForMetaSorting = useImageListOnly;
  }
  /**
   * Get the meta information mode flag.
   * Use only the information found in the image list for sorting based on
   * meta information. This mode is expected to accelerate the process of
   * sorting as the meta information files are not pre-cached.
   */
  bool GetUseImageListInformationOnlyForMetaSorting()
  {
    return m_UseImageListInformationOnlyForMetaSorting;
  }

  /** Clean up internal members, free resources (image, meta info). **/
  void CleanUp();

protected:
  /** Internal typedefs **/
  typedef std::vector<IniAccess *> CacheVectorType;
  typedef MetaVectorType::iterator MetaIteratorType;
  typedef CacheVectorType::iterator CacheIteratorType;

  /** contains a list of coplanar, sorted slice file names (RTI-files) **/
  StrLstType m_SliceList;
  /** internal built image **/
  ImagePointer m_Image;
  /** internal built image meta information **/
  ITKVTKImageMetaInformation::Pointer m_MetaInfo;
  /** patient's image list file (necessary for complete image-build) **/
  std::string m_ImageListFile;
  /** patient's frame of reference file (necessary for complete image-build) **/
  std::string m_FrameOfReferenceFile;
  /**
   * a list of acquisition types (case-sensitive) for single RTI image
   * files whereof a volume can be calculated (referred to as 'stackable')
   **/
  StrLstType m_StackableAcquisitionTypes;
  /**
   * A list of slice lists which logically build up volumes. Available after
   * invoking sorting based on meta information.
   **/
  StrLstVecType m_AvailableSliceStacks;
  /**
   * A list of slice lists which logically build up volumes. Available after
   * invoking sorting based on meta information. This vector is synchronized
   * with m_AvailableSliceStacks.
   **/
  MetaVectorVectorType m_AvailableSliceStacksMeta;
  /**
   * Use only the information found in the image list for sorting based on
   * meta information. This mode is expected to accelerate the process of
   * sorting as the meta information files are not pre-cached.
   */
  bool m_UseImageListInformationOnlyForMetaSorting;

  /**
   * Implements the 'smaller than' paradigm for RTI slice file name strings
   * following the open radART naming convention. The numeric parts of the
   * given image file names are considered. <br>
   * NOTE: this sort-mechanism is solely based on the image file names which may
   * result in inadequate sort orders if the considered file set is inadequate.
   * @param fn1 file name to be compared to fn2
   * @param fn2 file name to be compared to fn1
   * @return TRUE if fn1 is smaller than fn2 (the numeric parts of a typical
   * RTI-image file name are considered and evaluated)
   */
  static bool RTIFileNameStringSmallerThanOperator(const std::string &fn1,
      const std::string &fn2);
  /**
   * A comparator for RTI file name sorting. ASCENDING sort mode.
   * @param fileName1 is compared to the other file name
   * @param fileName2 is compared to the other file name
   * @return TRUE if fileName1 is smaller than fileName2
   * @see RTIFileNameStringSmallerThanOperator(std::string, std::string)
   */
  static bool CompareRTIFileNamesAscending(const std::string &fileName1,
    const std::string &fileName2);
  /**
   * A comparator for RTI file name sorting. DESCENDING sort mode.
   * @param fileName1 is compared to the other file name
   * @param fileName2 is compared to the other file name
   * @return TRUE if fileName1 is not less than fileName2
   * @see RTIFileNameStringSmallerThanOperator(std::string, std::string)
   */
  static bool CompareRTIFileNamesDescending(const std::string &fileName1,
    const std::string &fileName2);

  /**
   * Implements the 'smaller than' paradigm for RTI slice meta information
   * comparison through file names. The main sort criterion is the slice origin
   * w.r.t to the slicing direction.
   * @param i1 information object to be compared to i2
   * @param i2 information object to be compared to i1
   * @return TRUE if i1 is smaller than i2 (which means that it the projection
   * of the slice origin onto the slicing direction of i1 is smaller than that
   * of i2)
   */
  static bool RTIFileMetaInfoSmallerThanOperator(CompactRTISliceMetaInfo *i1,
      CompactRTISliceMetaInfo *i2);
  /**
   * A comparator for RTI meta information sorting. ASCENDING sort mode.
   * @param i1 information object to be compared to i2
   * @param i2 information object to be compared to i1
   * @return TRUE if i1 is smaller than i2 (which means that it the projection
   * of the slice origin onto the slicing direction of i1 is smaller than that
   * of i2)
   * @see RTIFileMetaInfoSmallerThanOperator(CompactRTISliceMetaInfo*,
   * CompactRTISliceMetaInfo*)
   */
  static bool CompareRTIFileMetaInfoAscending(CompactRTISliceMetaInfo *i1,
    CompactRTISliceMetaInfo *i2);
  /**
   * A comparator for RTI meta information sorting. DESCENDING sort mode.
   * @param i1 information object to be compared to i2
   * @param i2 information object to be compared to i1
   * @return TRUE if i1 is not less than i2 (which means that it the projection
   * of the slice origin onto the slicing direction of i1 is not less than that
   * of i2)
   * @see RTIFileMetaInfoSmallerThanOperator(CompactRTISliceMetaInfo*,
   * CompactRTISliceMetaInfo*)
   */
  static bool CompareRTIFileMetaInfoDescending(CompactRTISliceMetaInfo *i1,
    CompactRTISliceMetaInfo *i2);

  /** internal default constructor **/
  RTIToMHDConverter();

  /** internal destructor **/
  virtual ~RTIToMHDConverter();

  /**
   * Validates the configured slice file names. Invalid slices are automatically
   * removed from the internal list. Additionally the valid slice file names are
   * converted between UNC and UNIX format if necessary.
   * @param checkMetadataAsWell if TRUE the availability of the accompanying
   * metadata files (INF-files) is checked; invalid slices are removed from the
   * list
   * @return TRUE if at least one of the configured slices is OK
   */
  bool ValidateSlices(bool checkMetaDataAsWell);

  /**
   * @param acquisitionType explored acquisition type
   * @return TRUE if the acquisition type is stackable, FALSE otherwise
   */
  bool IsStackable(std::string acquisitionType);

  /**
   * Clear the list of available slice stacks.
   */
  void ClearAvailableSliceStacks();

  /**
   * Generate the (sorted) available slice stacks by inspecting the
   * compactMetaVector of the actual slice list. The separation criteria are:<br>
   * - Acquisition Type <br>
   * - FOR UID <br>
   * - Slicing Direction (+ Row direction) - Image Orientation <br>
   * - Slice Origin <br>
   * - Slice Size <br>
   * Additionally the stacks are sorted following their image origin positions
   * (w.r.t. the slicing direction).
   * @param compactMetaVector a vector describing the slice list in compact
   * representation optimized for separation and sorting
   */
  void GenerateSortedAvailableSliceStacks(MetaVectorType &compactMetaVector,
      bool ascending);

private:
  /** purposely not implemented **/
  RTIToMHDConverter(const Self&);

  /** purposely not implemented **/
  void operator=(const Self&);

};


}


#include "oraRTIToMHDConverter.txx"


#endif /* ORARTITOMHDCONVERTER_H_ */
