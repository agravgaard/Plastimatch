

#ifndef ORACOMPACTRTISLICEMETAINFO_H_
#define ORACOMPACTRTISLICEMETAINFO_H_

#include "oraImageList.h"

#include <itkPoint.h>
#include <itkVector.h>
#include <itkMatrix.h>
#include <itkFixedArray.h>

// Forward declarations
namespace ora
{
class IniAccess;
}


namespace ora 
{


/**
 * A helper class for compact open radART RTI slice meta information
 * encapsulation for easy slice sorting based on geometric constraints.
 * @author phil 
 * @version 1.0
 */
class CompactRTISliceMetaInfo
{
public:
  /** Relevant typedefs **/
  typedef itk::Point<double, 3> PointType;
  typedef itk::Vector<double, 3> VectorType;
  typedef itk::Matrix<double, 3, 3> MatrixType;
  typedef itk::FixedArray<double, 2> SizeType;
  typedef std::vector<CompactRTISliceMetaInfo *> MetaVectorType;
  typedef std::vector<MetaVectorType *> MetaVectorVectorType;

  /** Default constructor **/
  CompactRTISliceMetaInfo();

  /** Default destructor **/
  ~CompactRTISliceMetaInfo();

  /**
   * Create a new compact RTI slice meta information object from cached
   * information.
   * @param cachedMetaFile valid pre-cached meta information file belonging to a
   * specified RTI slice
   * @param imageList the image list containing the available images of the
   * belonging patient
   * @return the created object if successful, NULL otherwise
   */
  static CompactRTISliceMetaInfo *CreateFromCachedInformation(
      IniAccess *cachedMetaFile, ImageList::Pointer imageList);

  /**
   * Create and return a vector of compact RTI slice meta information objects
   * from cached image list information only.
   * @param imageList the image list containing the available images of the
   * belonging patient
   * @param metaSliceList returned vector (by reference - vector must be
   * created)
   * @return TRUE if successful (any valid meta information loaded)
   */
  static bool CreateFromCachedInformation(
        ImageList::Pointer imageList, MetaVectorType &metaSliceList);

  // it's easier to directly access the properties here:

  /** file name of the RTI slice (*.rti) **/
  std::string FileName;
  /** Frame of Reference UID **/
  std::string FORUID;
  /** Slice acquisition type **/
  std::string AcquisitionType;
  /**
   * Size of the slice in mm (1st item: in row direction, 2nd item: in column
   * direction
   **/
  SizeType SliceSize;
  /**
   * Origin of the slice in mm in a specified reference coordinate system
   * (typically the coordinate system of the acquisition modality related to
   * the according FOR's origin)
   **/
  PointType SliceOrigin;
  /**
   * Slicing direction defined by the slice's orientation (the cross product
   * of the row and column vectors: r x c) related to the DICOM LPS
   * patient positioning <br>
   * <b>NOTE:</b> be sure that the vector is set in normalized form!
   **/
  VectorType SlicingDirectionNorm;
  /**
   * Row direction of the image related to the DICOM LPS
   * patient positioning <br>
   * <b>NOTE:</b> be sure that the vector is set in normalized form!
   **/
  VectorType RowDirectionNorm;
  /**
   * A vector of meta-vector which should be avoided during insertion. This is
   * a helper for iterative slice stacking.
   */
  MetaVectorVectorType AvoidTheseMetaVectorsForInsertion;

  /**
   * Computes the slice position w.r.t. the slicing direction if it has not
   * already been calculated.
   * @param force force the computation of the slice position even if already
   * computed (e.g. when the slicing direction or the slice origin changed)
   * @return the safe computed slice position
   */
  double ComputeSlicePositionIfNecessary(bool force = false);

  /**
   * Computes a 'clean' representation of the slice file name enabling better
   * separation during stacking.
   * @param force force the computation of the clean file name (e.g. when the
   * file name changed)
   * @return the safe computed clean file name
   */
  std::string GenerateCleanFileNameIfNecessary(bool force = false);

protected:
  /** Indicates whether computed slice position is valid (already computed) **/
  bool m_ComputedSlicePositionValid;
  /**
   * Helper: computed slice position on slicing direction (for faster execution
   * of iterative processes)
   */
  double m_ComputedSlicePosition;
  /** Clean file name for RTI-comparsion and better separation **/
  std::string m_CleanFileName;

};


}


#endif /* ORACOMPACTRTISLICEMETAINFO_H_ */
