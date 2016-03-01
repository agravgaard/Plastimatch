//
#ifndef ORAPORTALIMAGINGDEVICEPROJECTIONPROPERTIES_TXX_
#define ORAPORTALIMAGINGDEVICEPROJECTIONPROPERTIES_TXX_

#include "oraPortalImagingDeviceProjectionProperties.h"

#include <itkVector.h>

namespace ora
{

template<class TPixelType, class TMaskPixelType>
PortalImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::PortalImagingDeviceProjectionProperties()
{
  this->Initialize();
  m_RotationTransform = TransformType::New();
  m_RotationAxis[0] = 0; // y-axis
  m_RotationAxis[1] = 1;
  m_RotationAxis[2] = 0;
}

template<class TPixelType, class TMaskPixelType>
PortalImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::~PortalImagingDeviceProjectionProperties()
{
  m_RotationTransform = NULL;
}

template<class TPixelType, class TMaskPixelType>
void PortalImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "--- Portal imaging device ---\n";
  os << indent << "Rotation Transform: " << m_RotationTransform.GetPointer()
      << "\n";
}

template<class TPixelType, class TMaskPixelType>
bool PortalImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::Update()
{
  // check the basic pre-requisites for computation:
  bool valid = true;
  valid = valid && (this->m_LinacGantryAngle >= -2. * M_PI
      && this->m_LinacGantryAngle <= 2. * M_PI);
  valid = valid && (this->m_SourceAxisDistance > 0.);
  // not common, but possible:
  //   valid = valid && (this->m_SourceFilmDistance > this->m_SourceAxisDistance);
  // not really a pre-requisite for this, but for flex map correction:
  if (this->m_FlexMap)
  {
    valid = valid && (this->m_LinacGantryDirection == 0
        || this->m_LinacGantryDirection == 1);
  }
  // furthermore, we need valid projection size and valid projection spacing
  valid = valid && (this->m_ProjectionSpacing[0] > 0.
      && this->m_ProjectionSpacing[1] > 0.);
  valid = valid && (this->m_ProjectionSize[0] > 0 && this->m_ProjectionSize[1]
      > 0);

  if (!valid)
    return false;

  // OK, let's update the orientation, origin and focal spot position
  // - first define the props for gantry angle 0:
  typedef itk::Vector<double, 3> VectorType;
  this->m_SourceFocalSpotPosition[0] = 0;
  this->m_SourceFocalSpotPosition[1] = 0;
  this->m_SourceFocalSpotPosition[2] = this->m_SourceAxisDistance;
  VectorType r; // row-direction
  r[0] = 1.;
  r[1] = 0.;
  r[2] = 0.;
  VectorType c; // column-direction
  c[0] = 0.;
  c[1] = 1.;
  c[2] = 0.;
  VectorType n; // plane normal
  n[0] = 0.;
  n[1] = 0.;
  n[2] = 1.;
  this->m_ProjectionPlaneOrigin[0] = -(this->m_ProjectionSpacing[0]
      * (double) this->m_ProjectionSize[0]) / 2.;
  this->m_ProjectionPlaneOrigin[1] = -(this->m_ProjectionSpacing[1]
      * (double) this->m_ProjectionSize[1]) / 2.;
  this->m_ProjectionPlaneOrigin[2] = this->m_SourceAxisDistance
      - this->m_SourceFilmDistance;
  // - then rotate the virtual plane around the iso-center by the specified
  // gantry angle
  this->m_RotationTransform->SetIdentity();
  this->m_RotationTransform->SetRotation(this->m_RotationAxis,
      this->m_LinacGantryAngle);
  this->m_SourceFocalSpotPosition = this->m_RotationTransform->TransformPoint(
      this->m_SourceFocalSpotPosition);
  this->m_ProjectionPlaneOrigin = this->m_RotationTransform->TransformPoint(
      this->m_ProjectionPlaneOrigin);
  r = this->m_RotationTransform->TransformVector(r);
  c = this->m_RotationTransform->TransformVector(c); // (no change)
  n = this->m_RotationTransform->TransformVector(n);
  for (int d = 0; d < 3; d++)
  {
    this->m_ProjectionPlaneOrientation[0][d] = r[d];
    this->m_ProjectionPlaneOrientation[1][d] = c[d];
    this->m_ProjectionPlaneOrientation[2][d] = n[d];
  }

  // finally, account for the flex map if defined
  if (valid && this->m_FlexMap)
  {
    valid = this->m_FlexMap->Correct();
  }

  return valid;
}

}

#endif /* ORAPORTALIMAGINGDEVICEPROJECTIONPROPERTIES_TXX_ */
