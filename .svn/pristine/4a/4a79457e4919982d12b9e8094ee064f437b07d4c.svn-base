//
#ifndef ORALINACMOUNTEDIMAGINGDEVICEPROJECTIONPROPERTIES_H_
#define ORALINACMOUNTEDIMAGINGDEVICEPROJECTIONPROPERTIES_H_

#include "oraProjectionProperties.h"

namespace ora
{

// forward declaration
template <class TPixelType, class TMaskPixelType>
class FlexMapCorrection;

/** \class LinacMountedImagingDeviceProjectionProperties
 * \brief Base class for a LINAC-mounted imaging devices' projection geometries.
 *
 * A base class for a LINAC-mounted imaging devices' projection geometries
 * (e.g. portal imaging geometry). The imaging geometry is basically defined by
 * specifying the LINAC gantry angle. In order to define the projection, the source
 * to axis distance (SAD) and source to film distance (SFD) must be specified.
 * Here, axis relates to the mechanical iso-center.
 * NOTE: The set projection size and spacing will neither be influenced by this
 * class nor by the optional flex map correction.
 *
 * Moreover, this class defines the interface for an optional flex map
 * correction approach that is responsible for correcting the idealized
 * projection properties w.r.t. measured uncertainties which are due to the
 * mechanical flex encountered when rotating the LINAC gantry.
 *
 * For reasons of superclass-compatibility, this class is templated over the
 * fixed image pixel type and the mask image type.
 *
 * @see ora::ProjectionProperties
 * @see ora::FlexMapCorrection
 * 
 * @author phil 
 * @version 1.0
 */
template<class TPixelType, class TMaskPixelType = unsigned char>
class LinacMountedImagingDeviceProjectionProperties:
    public ProjectionProperties<TPixelType, TMaskPixelType>
{
public:
  /** Standard class typedefs. **/
  typedef LinacMountedImagingDeviceProjectionProperties Self;
  typedef ProjectionProperties<TPixelType, TMaskPixelType> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Superclass re-definitions **/
  typedef typename Superclass::PointType PointType;
  typedef typename Superclass::MatrixType MatrixType;
  typedef typename Superclass::SizeType SizeType;
  typedef typename Superclass::SpacingType SpacingType;

  /** Further typedefs. **/
  typedef double AngleType;
  typedef double DistanceType;

  /** Run-time type information (and related methods). **/
  itkTypeMacro(LinacMountedImagingDeviceProjectionProperties, itk::Object)

  /**
   * Updates internal projection properties using the actual set attributes.
   * This method must be called in order to ensure that the projection
   * properties are up-to-date!
   * NOTE: This method also handles the (optional) flex map correction if a
   * flex map correction is set.
   * @return TRUE if the update was successful
   * Must be implemented in subclasses.
   **/
  virtual bool Update() = 0;

  /**
   * In addition to the superclass implementation this method checks whether the
   * specified LINAC gantry angle and related props are valid.
   * @see ProjectionProperties#IsGeometryValid()
   */
  virtual bool IsGeometryValid();

  // re-define the pure projection-geometry-getters in order to implement the
  // automatic (optional) flex map correction!
  virtual PointType GetSourceFocalSpotPosition();
  virtual PointType GetProjectionPlaneOrigin();
  virtual MatrixType GetProjectionPlaneOrientation();
  // however, offer access to the uncorrected props (this may be useful for
  // concrete flex map implementations)
  PointType GetUncorrectedSourceFocalSpotPosition();
  PointType GetUncorrectedProjectionPlaneOrigin();
  MatrixType GetUncorrectedProjectionPlaneOrientation();


  itkSetMacro(LinacGantryAngle, AngleType)
  itkGetMacro(LinacGantryAngle, AngleType)

  itkSetMacro(LinacGantryDirection, int)
  itkGetMacro(LinacGantryDirection, int)

  itkSetMacro(SourceAxisDistance, DistanceType)
  itkGetMacro(SourceAxisDistance, DistanceType)

  itkSetMacro(SourceFilmDistance, DistanceType)
  itkGetMacro(SourceFilmDistance, DistanceType)

  /**
   * Set flex map correction method and automatically reference this LINAC-
   * mounted imaging device properties.
   **/
  virtual void SetFlexMap(FlexMapCorrection<TPixelType, TMaskPixelType> *flexMap);
  virtual FlexMapCorrection<TPixelType, TMaskPixelType> *GetFlexMap();

protected:
  /** Type of the fixed image positioned in 3D space. **/
  static const unsigned int FixedDimension = 3;
  typedef TPixelType FixedPixelType;
  typedef itk::Image<FixedPixelType, FixedDimension> FixedImageType;
  typedef typename FixedImageType::Pointer FixedImagePointer;
  typedef typename FixedImageType::RegionType FixedImageRegionType;

  /** Optional flex map correction approach. **/
  FlexMapCorrection<TPixelType, TMaskPixelType> *m_FlexMap;
  /** LINAC gantry angle in radians (allowed range: [-2*PI;+2*PI]. **/
  AngleType m_LinacGantryAngle;
  /** LINAC gantry angle rotation direction (0=clockwise, 1=counter-clockw.) **/
  int m_LinacGantryDirection;
  /**
   * Source to axis distance (SAD): distance of imaging device source to
   * mechanical iso-center of LINAC in mm. NOTE: SAD > 0 requested!
   **/
  DistanceType m_SourceAxisDistance;
  /**
   * Source to film distance (SFD): distance of imaging device source to
   * intersection point of imaging plane and the central axis of LINAC in mm.
   * NOTE: SFD > SAD requested!
   **/
  DistanceType m_SourceFilmDistance;

  /** Default constructor. **/
  LinacMountedImagingDeviceProjectionProperties();
  /** Default constructor. **/
  virtual ~LinacMountedImagingDeviceProjectionProperties();
  /** Standard object output. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  // make some of the props "unsettable" in order to force the user to use
  // the gantry angle to specify the geometry!
  itkSetMacro(SourceFocalSpotPosition, PointType)
  itkSetMacro(ProjectionPlaneOrigin, PointType)
  itkSetMacro(ProjectionPlaneOrientation, MatrixType)
  // make this method "unusable"!
  virtual void SetGeometryFromFixedImage(FixedImagePointer fixedImage,
        FixedImageRegionType fixedRegion)
  {
    Superclass::SetGeometryFromFixedImage(fixedImage, fixedRegion);
  }

  /** Initialize internal members. **/
  virtual void Initialize();

private:
  /** Purposely not implemented. **/
  LinacMountedImagingDeviceProjectionProperties(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraLinacMountedImagingDeviceProjectionProperties.txx"

#endif /* ORALINACMOUNTEDIMAGINGDEVICEPROJECTIONPROPERTIES_H_ */
