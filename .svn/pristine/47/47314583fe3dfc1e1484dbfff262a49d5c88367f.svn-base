//
#ifndef ORAFLEXMAPCORRECTION_H_
#define ORAFLEXMAPCORRECTION_H_

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkSmartPointer.h>

#include "oraLinacMountedImagingDeviceProjectionProperties.h"

namespace ora
{

/** \class FlexMapCorrection
 * \brief Base class for flex map correction approaches of LINAC-mounted devices.
 *
 * Base class for flex map correction approaches of LINAC-mounted devices.
 * Basically flex map correction approaches are based on the LINAC gantry angle
 * position as it aims at correcting the idealized projection properties which
 * are due to mechanical flex (and possibly further instabilities). However,
 * concrete implementations can use further properties to determine the nature
 * of necessary correction.
 *
 * NOTE: Subclasses are expected to correct the <b>plane origin</b>, the
 * <b>plane orientation</b> and the <b>focal spot position</b> (or at least
 * a subset of these properties).
 *
 * NOTE: This class does not modify the referenced projection properties
 * directly, it rather updates its internal generic projection properties
 * representation which can be retrieved using GetCorrectedProjProps().
 *
 * NOTE: For compatibility reasons with ora::ProjectionProperties, this class
 * is templated over the pixel type of the fixed image and the mask pixel type.
 *
 * <b>Tests</b>:<br>
 * TestPortalImagingDeviceProjectionProperties.cxx <br>
 *
 * @see ora::LinacMountedImagingDeviceProjectionProperties
 * 
 * @author phil 
 * @version 1.2
 */
template<class TPixelType, class TMaskPixelType = unsigned char>
class FlexMapCorrection :
    public itk::Object
{
public:
  /** Standard class typedefs. **/
  typedef FlexMapCorrection Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Communication types. **/
  typedef LinacMountedImagingDeviceProjectionProperties<TPixelType,
      TMaskPixelType> LinacProjectionPropertiesType;
  typedef typename LinacProjectionPropertiesType::Pointer
      LinacProjectionPropertiesPointer;
  typedef ProjectionProperties<TPixelType, TMaskPixelType>
      GenericProjectionPropertiesType;
  typedef typename GenericProjectionPropertiesType::Pointer
      GenericProjectionPropertiesPointer;

  /** Run-time type information (and related methods). **/
  itkTypeMacro(FlexMapCorrection, itk::Object)

  itkSetObjectMacro(LinacProjProps, LinacProjectionPropertiesType)
  itkGetObjectMacro(LinacProjProps, LinacProjectionPropertiesType)

  /**
   * Do the correction of the projection properties w.r.t. the current LINAC
   * gantry angle. NOTE: This method DOES NOT modify the referenced projection
   * properties, it rather updates its internal projection properties which can
   * be retrieved using GetCorrectedProjProps()!
   * This method must be implemented in concrete subclasses!
   * @return TRUE if the referenced projection properties could successfully be
   * corrected and the internal corrected projection properties were accordingly
   * updated!
   * @see GetCorrectedProjProps()
   */
  virtual bool Correct() = 0;

  itkGetConstObjectMacro(CorrectedProjProps, GenericProjectionPropertiesType)

protected:
  /** Reference to the LINAC projection properties that should be corrected. **/
  LinacProjectionPropertiesPointer m_LinacProjProps;
  /** Internal corrected projection properties in generic representation. **/
  GenericProjectionPropertiesPointer m_CorrectedProjProps;

  /** Default constructor. **/
  FlexMapCorrection();
  /** Default constructor. **/
  virtual ~FlexMapCorrection();
  /** Standard object output. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  /** Purposely not implemented. **/
  FlexMapCorrection(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraFlexMapCorrection.txx"

#endif /* ORAFLEXMAPCORRECTION_H_ */
