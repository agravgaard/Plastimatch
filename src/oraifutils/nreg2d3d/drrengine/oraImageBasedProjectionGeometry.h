//
#ifndef ORAIMAGEBASEDPROJECTIONGEOMETRY_H_
#define ORAIMAGEBASEDPROJECTIONGEOMETRY_H_

#include "oraProjectionGeometry.h"
//ITK
#include <itkImage.h>

namespace ora
{

/** \class ImageBasedProjectionGeometry
 * \brief Convenience class for defining projection geometries outgoing from oriented planar images in 3D space.
 *
 * This is a convenience class which helps to define a projection geometry
 * based on existing oriented planar images (single-sliced 3D images) in 3D
 * space.
 *
 * Basically, only the detector geometry (origin, orientation, spacing, size) is
 * defined by the image. However, there are further methods enabling to derive
 * the source position by inputting further information.
 *
 * @see ora::ProjectionGeometry
 *
 * @author phil
 * @author jeanluc
 * @version 1.0
 *
 * \ingroup ImageFilters
 **/
template<class TPixelType>
class ImageBasedProjectionGeometry:
	public ProjectionGeometry
{
public:
	/** Standard class typedefs. */
	typedef ImageBasedProjectionGeometry Self;
	typedef ProjectionGeometry Superclass;
	typedef itk::SmartPointer<Self> Pointer;
	typedef itk::SmartPointer<const Self> ConstPointer;

	/** Accessibility typedefs **/
	typedef itk::Image<TPixelType, 3> ImageType;
	typedef typename ImageType::Pointer ImagePointer;
	typedef typename ImageType::RegionType RegionType;

  /**Run-time type information (and related methods). */
  itkTypeMacro(Self, Superclass)

  /** Method for creation through the object factory */
  itkNewMacro(Self)

  /** Extract the detector geometry (origin, orientation, spacing, size) from the
   * specified input image (single-sliced planar image in 3D space). NOTE: The
   * source position of the projection geometry can certainly not be extracted
   * from the image, it must be specified separately.
   * @param image the image which implicitly defines the projection plane
   * (detector)
   * @return TRUE if successful (this does however not inherently mean that
   * the projection geometry is valid as the source position is not defined
   * by the image) **/
  virtual bool ExtractDetectorGeometryFromImage(ImagePointer image);

  /** Extract the detector geometry (origin, orientation, spacing, size) from
   * the specified input image (single-sliced planar image in 3D space) and a
   * dedicated image region. NOTE: The source position of the projection
   * geometry can certainly not be extracted from the image and its region, it
   * must be specified separately.
   * @param image the image which implicitly defines the projection plane
   * (detector)
   * @param region the image region which defines the start index and the size
   * of the image which implicitly defines the projection portion of interest
   * @return TRUE if successful (this does however not inherently mean that
   * the projection geometry is valid as the source position is not defined
   * by the image) **/
  virtual bool ExtractDetectorGeometryFromImageAndRegion(ImagePointer image,
  		const RegionType &region);

  /** Extract the detector geometry (origin, orientation, spacing, size) from the
   * specified input image (single-sliced planar image in 3D space) and compute
   * the source position by assuming that the central axis is orthonormal to
   * the image plane.
   * @param image the image which implicitly defines the projection plane
   * (detector)
   * @param cx defines the relative position (x-direction) of the central-axis/plane
   * intersection within the detector plane in physical units,
   * starting from the image origin (detector origin) and following the row
   * vector direction
   * @param cy defines the relative position (y-direction) of the central-axis/plane
   * intersection within the detector plane in physical units,
   * starting from the image origin (detector origin) and following the column
   * vector direction
   * @param SFD the source-to-film distance (distance from source position to
   * the detector plane along the central axis)
   * @param inverse [optional; default=FALSE] by default, the SFD is applied
   * along the implicit plane normal (cross-product of row and column vector);
   * if this flag is TRUE, the SFD is applied along the inverse direction of the
   * plane normal
   * @return TRUE if successful (the return value TRUE includes the overall
   * projection geometry validation) **/
  virtual bool ExtractDetectorGeometryFromImageAndAssumeSourcePosition(
  		ImagePointer image, double cx, double cy, double SFD, bool inverse = false);

  /** Extract the detector geometry (origin, orientation, spacing, size) from the
   * specified input image (single-sliced planar image in 3D space) and its
   * region, and compute the source position by assuming that the central axis
   * is orthonormal to the image plane. NOTE: The relative distance (cx, cy)
   * relates to the ORIGINAL image origin (not to the corrected one which
   * results from the image region)!
   * @param image the image which implicitly defines the projection plane
   * (detector)
   * @param region the image region which defines the start index and the size
   * of the image which implicitly defines the projection portion of interest
   * @param cx defines the relative position (x-direction) of the central-axis/plane
   * intersection within the detector plane in physical units,
   * starting from the image origin (detector origin) and following the row
   * vector direction
   * @param cy defines the relative position (y-direction) of the central-axis/plane
   * intersection within the detector plane in physical units,
   * starting from the image origin (detector origin) and following the column
   * vector direction
   * @param SFD the source-to-film distance (distance from source position to
   * the detector plane along the central axis)
   * @param inverse [optional; default=FALSE] by default, the SFD is applied
   * along the implicit plane normal (cross-product of row and column vector);
   * if this flag is TRUE, the SFD is applied along the inverse direction of the
   * plane normal
   * @return TRUE if successful (the return value TRUE includes the overall
   * projection geometry validation) **/
  virtual bool ExtractDetectorGeometryFromImageAndRegionAndAssumeSourcePosition(
  		ImagePointer image, const RegionType &region, double cx, double cy,
  		double SFD, bool inverse = false);


protected:
  /** Default constructor. **/
  ImageBasedProjectionGeometry();
  /** Default destructor. **/
  virtual ~ImageBasedProjectionGeometry();

	/** Print description of this object. **/
	virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  /** Purposely not implemented. **/
  ImageBasedProjectionGeometry(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraImageBasedProjectionGeometry.txx"

#endif /* ORAIMAGEBASEDPROJECTIONGEOMETRY_H_ */
