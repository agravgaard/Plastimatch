//
#ifndef ORAPORTALIMAGINGDEVICEPROJECTIONPROPERTIES_H_
#define ORAPORTALIMAGINGDEVICEPROJECTIONPROPERTIES_H_

#include "oraLinacMountedImagingDeviceProjectionProperties.h"

#include <itkVersorRigid3DTransform.h>

namespace ora
{

/** \class PortalImagingDeviceProjectionProperties
 * \brief Projection geometry of a LINAC portal imaging device (MV imaging).
 *
 * Represents the projection geometry of a LINAC portal imaging device (MV
 * imaging). It is defined by <br>
 * - the projection size (with projection of the iso-center as center),<br>
 * - the projection spacing,<br>
 * - the LINAC gantry angle (which equals the imaging source gantry angle),<br>
 * - the source to axis distance (SAD),<br>
 * - and the source to film distance (SFD).
 *
 * This geometry will basically be defined in an IEC-based machine coordinate
 * system where the row-direction of the imaging plane is the x-axis (for gantry
 * angle 0), and the column-direction of the imaging plane is the y-axis (for
 * all gantry angles).
 *
 * NOTE: Optionally a flex map correction approach can be connected to this
 * class which accounts for mechanical instabilities. This may moreover
 * require the gantry rotation direction as further input.
 *
 * For reasons of superclass-compatibility, this class is templated over the
 * fixed image pixel type and the mask pixel type.
 *
 * <b>Tests</b>:<br>
 * TestPortalImagingDeviceProjectionProperties.cxx <br>
 * TestProjectionProperties.cxx
 *
 * @see ora::LinacMountedImagingDeviceProjectionProperties
 * @see ora::FlexMapCorrection
 *
 * @author phil 
 * @version 1.0
 */
template<class TPixelType, class TMaskPixelType = unsigned char>
class PortalImagingDeviceProjectionProperties :
    public LinacMountedImagingDeviceProjectionProperties<TPixelType,
        TMaskPixelType>
{
public:
  /** Standard class typedefs. **/
  typedef PortalImagingDeviceProjectionProperties Self;
  typedef LinacMountedImagingDeviceProjectionProperties<TPixelType,
      TMaskPixelType> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. **/
  itkNewMacro(Self)

  /** Run-time type information (and related methods). **/
  itkTypeMacro(PortalImagingDeviceProjectionProperties, itk::Object)

  /**
   * Concrete implementation of the update of the portal imaging device
   * projection geometry from actual set properties.
   * NOTE: This method updates "idealized" projection geometry of this class. If
   * a flex map correction is connected, the Correct()-method of the flex map
   * will be called which causes the flex map output to be updated. This output
   * will then substitute the essential internal projection geometry properties
   * (origin, orientation, focal spot position).
   * @see ora::LinacMountedImagingDeviceProjectionProperties#Update()
   **/
  virtual bool Update();

protected:
  /** Transformation type. **/
  typedef itk::VersorRigid3DTransform<double> TransformType;
  typedef TransformType::Pointer TransformPointer;

  /** Helper transform that rotates the imaging device around iso-center. **/
  TransformPointer m_RotationTransform;
  /** The axis where the imaging device is rotated around. **/
  TransformType::AxisType m_RotationAxis;

  /** Default constructor. **/
  PortalImagingDeviceProjectionProperties();
  /** Default constructor. **/
  virtual ~PortalImagingDeviceProjectionProperties();
  /** Standard object output. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  /** Purposely not implemented. **/
  PortalImagingDeviceProjectionProperties(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraPortalImagingDeviceProjectionProperties.txx"

#endif /* ORAPORTALIMAGINGDEVICEPROJECTIONPROPERTIES_H_ */
