//
#ifndef ORAPROJECTIONPROPERTIES_H_
#define ORAPROJECTIONPROPERTIES_H_

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkImage.h>
#include <itkFixedArray.h>
#include <itkArray.h>

#include <vtkSmartPointer.h>
#include <vtkColorTransferFunction.h>

namespace ora
{

/** \class ProjectionProperties
 * \brief Defines the properties of a projection image (DRR).
 *
 * Defines the properties of a projection image and provides tools to specify
 * and validate these properties as easy as possible. E.g. the geometry can
 * directly be derived from the pose and shape of the reference (fixed) image in
 * 3D space.
 *
 * This class is templated over the fixed image's pixel type, and (optionally)
 * over the mask image pixel type.
 *
 * This class is meant as raw implementation of a 2D/3D-registration scenario
 * without any special properties. It is therefore suited as base class for more
 * specific modality-dependent projection properties classes.
 *
 * In addition, this class is capable of defining further projection-specific
 * properties especially for 2D/3D-registration. A DRR-mask which defines the
 * pixels to be evaluated, and intensity rescaling parameters (slope, intercept)
 * that describe a linear transformation can be defined.
 *
 * <b>Tests</b>:<br>
 * TestProjectionProperties.cxx <br>
 * TestMultiResolutionNWay2D3DRegistrationMethod.cxx
 * 
 * @see ora::MultiResolutionNWay2D3DRegistrationMethod
 * @see ora::ITKVTKDRRFilter
 *
 * @author phil 
 * @version 1.2.1
 */
template<class TPixelType, class TMaskPixelType = unsigned char>
class ProjectionProperties:
    public itk::Object
{
public:
  /** Standard class typedefs. **/
  typedef ProjectionProperties Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. **/
  itkNewMacro(Self)

  /** Run-time type information (and related methods). **/
  itkTypeMacro(ProjectionProperties, itk::Object)

  /** Type of the fixed image positioned in 3D space. **/
  static const unsigned int FixedDimension = 3;
  typedef TPixelType FixedPixelType;
  typedef itk::Image<FixedPixelType, FixedDimension> FixedImageType;
  typedef typename FixedImageType::Pointer FixedImagePointer;
  typedef typename FixedImageType::RegionType FixedImageRegionType;
  typedef TMaskPixelType MaskPixelType;
  typedef itk::Image<MaskPixelType, FixedDimension> MaskImageType;
  typedef typename MaskImageType::Pointer MaskImagePointer;

  /** Type of geometric properties. **/
  typedef itk::Point<double, 3> PointType;
  typedef itk::Matrix<double, 3, 3> MatrixType;
  typedef itk::Size<2> SizeType;
  typedef itk::FixedArray<double, 2> SpacingType;
  typedef itk::FixedArray<double, 3> Spacing3DType;

  /** Type of further properties. **/
  typedef vtkSmartPointer<vtkColorTransferFunction> TransferFunctionPointer;
  typedef itk::Array<double> TransferFunctionSpecificationType;

  itkSetMacro(SourceFocalSpotPosition, PointType)
  itkGetConstMacro(SourceFocalSpotPosition, PointType)

  itkSetMacro(ProjectionPlaneOrigin, PointType)
  itkGetConstMacro(ProjectionPlaneOrigin, PointType)

  itkSetMacro(ProjectionPlaneOrientation, MatrixType)
  itkGetConstMacro(ProjectionPlaneOrientation, MatrixType)

  itkSetMacro(ProjectionSize, SizeType)
  itkGetConstMacro(ProjectionSize, SizeType)

  itkSetMacro(ProjectionSpacing, SpacingType)
  itkGetConstMacro(ProjectionSpacing, SpacingType)

  virtual void SetSamplingDistance(float distance);
  itkGetConstMacro(SamplingDistance, float)

  itkSetMacro(ITF, TransferFunctionPointer)
  itkGetMacro(ITF, TransferFunctionPointer)

  itkSetMacro(RescaleSlope, double)
  itkGetConstMacro(RescaleSlope, double)

  itkSetMacro(RescaleIntercept, double)
  itkGetConstMacro(RescaleIntercept, double)

  itkSetObjectMacro(DRRMask, MaskImageType)
  itkGetObjectMacro(DRRMask, MaskImageType)

  /**
   * Set projection geometry (partially) from fixed image information. Plane
   * origin, plane orientation, size and spacing can be taken over from the
   * image and the specified fixed image region (which defines the rectangular
   * region of interest). <br> NOTE: the focal spot position must however be
   * specified separately.
   * @param fixedImage the fixed image in terms of a single-sliced volume
   * positioned in 3D-space that defines plane orientation, the plane spacing
   * and implicitly the plane origin
   * @param fixedRegion the fixed image region of interest that defines
   * implicitly the plane origin and the plane size
   * @see IsGeometryValid()
   * @see AreAllPropertiesValid()
   */
  virtual void SetGeometryFromFixedImage(FixedImagePointer fixedImage,
      FixedImageRegionType fixedRegion);

  /**
   * Automatically compute and set sampling distance for ray-casting from
   * volume information (spacing).
   * @param volumeSpacing spacing in row-, column- and slicing-direction of
   * volume
   * @param mode determines the method of sampling distance computation: <br>
   * 0 ... half of smallest spacing component (Shannon theorem) <br>
   * 1 ... smallest spacing component (empirical, but usually enough) <br>
   * 2 ... largest spacing component (empirical, usually low quality) <br>
   * 3 ... half of largest spacing component (sparse Shannon theorem) <br>
   * @return the computed sampling distance (which was internally set, you do
   * not have to set it afterwards); -1 is returned if it could not be computed
   * and set
   * @see AreRayCastingPropertiesValid()
   * @see AreAllPropertiesValid()
   */
  virtual float ComputeAndSetSamplingDistanceFromVolume(
      Spacing3DType volumeSpacing, int mode);

  itkGetMacro(LastSamplingDistanceComputationMode, int)

  /**
   * Set the internal intensity transfer function (ITF) that simulates X-ray
   * attenuation from a sequence of node pairs (input volume intensity followed
   * by resultant output contribution intensity in the range [0;1]).
   * @param nodePairs flat sequence of input volume intensities and
   * resultant output contribution intensities (i1,o1,i2,o2 ...); therefore the
   * number of values in nodePairs must be even!<br>
   * NOTE: the output contribution intensities that simulate attenuation must
   * internally lie in the range [0;1]. However, if the specified output
   * intensities in nodePairs parameter lie outside this range, the output range
   * is automatically rescaled to the range [0;1] w.r.t. the specified maximum
   * output intensity value. Furthermore, negative output values are clamped to
   * zero output.
   * @return TRUE if successful
   * @see AreRayCastingPropertiesValid()
   * @see AreAllPropertiesValid()
   */
  virtual bool SetIntensityTransferFunctionFromNodePairs(
      TransferFunctionSpecificationType nodePairs);

  /**
   * @return TRUE if current projection geometry is valid (complete) <br>
   * NOTE: plane orientation must be orthogonal, projection size must be greater
   * than zero in each dimension, projection plane spacing must be greater than
   * zero in each dimension and the focal spot must not lie inside the plane
   * @see AreAllPropertiesValid()
   */
  virtual bool IsGeometryValid() const;

  /**
   * @return TRUE if all ray-casting properties of this class are valid; this
   * requires sampling distance greater than zero and a valid intensity transfer
   * function (at least two nodes)
   * @see AreAllPropertiesValid()
   */
  virtual bool AreRayCastingPropertiesValid() const;

  /**
   * @return TRUE if mask properties of this class are valid; this
   * requires the mask size (first two dimensions) to equal the projection size.
   * @see AreAllPropertiesValid()
   */
  virtual bool AreMaskPropertiesValid() const;

  /**
   * @return TRUE if all properties of this class are valid (geometry and ray-
   * casting and mask properties)
   * @see IsGeometryValid()
   * @see AreRayCastingPropertiesValid()
   * @see AreMaskPropertiesValid()
   */
  virtual bool AreAllPropertiesValid() const;

protected:
  /** Projection source focal spot position in WCS. In mm. **/
  PointType m_SourceFocalSpotPosition;
  /** Projection plane origin in WCS. In mm. **/
  PointType m_ProjectionPlaneOrigin;
  /**
   * Projection plane orientation (first row of matrix defines normalized
   * direction of the projection row, second row of matrix defines normalized
   * direction of the projection column, third row is the normalized projection
   * plane normal towards the focal point); this matrix is expected to be
   * orthogonal.
   **/
  MatrixType m_ProjectionPlaneOrientation;
  /** Projection size in pixels (row-direction, column-direction). In pixels. **/
  SizeType m_ProjectionSize;
  /**
   * Projection spacing (spacing along row-direction, spacing along column-dir).
   * In mm / pixel.
   **/
  SpacingType m_ProjectionSpacing;
  /** Sampling distance along the projection rays. In mm. **/
  float m_SamplingDistance;
  /**
   * Intensity transfer function that maps sampled volume intensities to
   * output values that contribute to the final projection image. It simulates
   * attenuation. NOTE: the output node values must be in the range [0;1].
   **/
  TransferFunctionPointer m_ITF;
  /**
   * Stores the last used sampling distance computation mode. If sampling
   * distance is set manually, this member will be set to -1.
   * @see ComputeAndSetSamplingDistanceFromVolume
   */
  int m_LastSamplingDistanceComputationMode;
  /**
   * Rescale slope for linear intensity rescaling. This is the parameter s in
   * equation V'=V*s+i where V' is the resultant DRR pixel value and V is the
   * original DRR pixel value. Default: 1.0.
   * @see m_RescaleIntercept
   **/
  double m_RescaleSlope;
  /**
   * Rescale intercept for linear intensity rescaling. This is the parameter i in
   * equation V'=V*s+i where V' is the resultant DRR pixel value and V is the
   * original DRR pixel value. Default: 0.0.
   * @see m_RescaleSlope
   **/
  double m_RescaleIntercept;
  /**
   * Optional DRR mask image that defines which DRR pixels should be computed.
   * The corresponding DRR pixels where the mask pixels have values greater
   * than 0 are computed. NOTE: In order to define a valid projection geometry,
   * the mask size must match the projection size (first two dimensions). The
   * spacing and origin of the mask are basically ignored - the pixels are
   * important!
   */
  MaskImagePointer m_DRRMask;

  /** Default constructor. **/
  ProjectionProperties();
  /** Default constructor. **/
  virtual ~ProjectionProperties();
  /** Standard object output. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /** @return TRUE if the specified matrix is orthogonal **/
  bool IsOrthogonalMatrix(const MatrixType &matrix) const;

private:
  /** Purposely not implemented. **/
  ProjectionProperties(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraProjectionProperties.txx"

#endif /* ORAPROJECTIONPROPERTIES_H_ */
