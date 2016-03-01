//
#ifndef ORALINACMOUNTEDIMAGINGDEVICEPROJECTIONPROPERTIES_TXX_
#define ORALINACMOUNTEDIMAGINGDEVICEPROJECTIONPROPERTIES_TXX_

#include "oraLinacMountedImagingDeviceProjectionProperties.h"

#include "oraFlexMapCorrection.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace ora
{

template<class TPixelType, class TMaskPixelType>
LinacMountedImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::LinacMountedImagingDeviceProjectionProperties()
{
  this->Initialize();
}

template<class TPixelType, class TMaskPixelType>
void LinacMountedImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::Initialize()
{
  m_FlexMap = NULL;
  m_LinacGantryAngle = -1000;
  m_LinacGantryDirection = -1;
  m_SourceAxisDistance = -1;
  m_SourceFilmDistance = -1;
}

template<class TPixelType, class TMaskPixelType>
LinacMountedImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::~LinacMountedImagingDeviceProjectionProperties()
{
  m_FlexMap = NULL;
}

template<class TPixelType, class TMaskPixelType>
void LinacMountedImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "--- LINAC-mounted imaging device ---\n";
  os << indent << "Flex Map: " << itk::SmartPointer<FlexMapCorrection<
      TPixelType> >(m_FlexMap). GetPointer() << "\n";
  os << indent << "Linac Gantry Angle: " << m_LinacGantryAngle << " ("
      << (m_LinacGantryAngle / M_PI * 180.) << " deg)\n";
  os << indent << "Linac Gantry Direction: " << m_LinacGantryDirection << "\n";
  os << indent << "Source Axis Distance: " << m_SourceAxisDistance << "\n";
  os << indent << "Source Film Distance: " << m_SourceFilmDistance << "\n";
}

template<class TPixelType, class TMaskPixelType>
void LinacMountedImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::SetFlexMap(FlexMapCorrection<TPixelType, TMaskPixelType> *flexMap)
{
  if (this->m_FlexMap != flexMap)
  {
    this->m_FlexMap = flexMap;
    this->m_FlexMap->SetLinacProjProps(this); // automatically set reference
    this->Modified();
  }
}

template<class TPixelType, class TMaskPixelType>
FlexMapCorrection<TPixelType, TMaskPixelType> * LinacMountedImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::GetFlexMap()
{
  return this->m_FlexMap;
}

template<class TPixelType, class TMaskPixelType>
bool LinacMountedImagingDeviceProjectionProperties<TPixelType, TMaskPixelType>::IsGeometryValid()
{
  bool valid = true;

  valid = valid && (m_LinacGantryAngle >= -2. * M_PI && m_LinacGantryAngle
      <= 2. * M_PI);
  valid = valid && (m_LinacGantryDirection == 0 || m_LinacGantryDirection == 1);
  valid = valid && (m_SourceAxisDistance > 0.);
  valid = valid && (m_SourceFilmDistance > m_SourceAxisDistance);

  if (valid)
  {
    if (m_FlexMap && m_FlexMap->GetCorrectedProjProps()) // flexmap-result!
      return m_FlexMap->GetCorrectedProjProps()->IsGeometryValid();
    else
      return this->Superclass::IsGeometryValid();
  }
  else
  {
    return false;
  }
}

template<class TPixelType, class TMaskPixelType>
typename LinacMountedImagingDeviceProjectionProperties<TPixelType,
    TMaskPixelType>::PointType LinacMountedImagingDeviceProjectionProperties<
    TPixelType, TMaskPixelType>::GetSourceFocalSpotPosition()
{
  // (if flex map is set, assume that it is up-to-date!)
  if (m_FlexMap && m_FlexMap->GetCorrectedProjProps())
    return m_FlexMap->GetCorrectedProjProps()->GetSourceFocalSpotPosition();
  else
    return this->Superclass::GetSourceFocalSpotPosition();
}

template<class TPixelType, class TMaskPixelType>
typename LinacMountedImagingDeviceProjectionProperties<TPixelType,
    TMaskPixelType>::PointType LinacMountedImagingDeviceProjectionProperties<
    TPixelType, TMaskPixelType>::GetProjectionPlaneOrigin()
{
  // (if flex map is set, assume that it is up-to-date!)
  if (m_FlexMap && m_FlexMap->GetCorrectedProjProps())
    return m_FlexMap->GetCorrectedProjProps()->GetProjectionPlaneOrigin();
  else
    return this->Superclass::GetProjectionPlaneOrigin();
}

template<class TPixelType, class TMaskPixelType>
typename LinacMountedImagingDeviceProjectionProperties<TPixelType,
    TMaskPixelType>::MatrixType LinacMountedImagingDeviceProjectionProperties<
    TPixelType, TMaskPixelType>::GetProjectionPlaneOrientation()
{
  // (if flex map is set, assume that it is up-to-date!)
  if (m_FlexMap && m_FlexMap->GetCorrectedProjProps())
    return m_FlexMap->GetCorrectedProjProps()->GetProjectionPlaneOrientation();
  else
    return this->Superclass::GetProjectionPlaneOrientation();
}

template<class TPixelType, class TMaskPixelType>
typename LinacMountedImagingDeviceProjectionProperties<TPixelType,
    TMaskPixelType>::PointType LinacMountedImagingDeviceProjectionProperties<
    TPixelType, TMaskPixelType>::GetUncorrectedSourceFocalSpotPosition()
{
  return this->Superclass::GetSourceFocalSpotPosition();
}

template<class TPixelType, class TMaskPixelType>
typename LinacMountedImagingDeviceProjectionProperties<TPixelType,
    TMaskPixelType>::PointType LinacMountedImagingDeviceProjectionProperties<
    TPixelType, TMaskPixelType>::GetUncorrectedProjectionPlaneOrigin()
{
  return this->Superclass::GetProjectionPlaneOrigin();
}

template<class TPixelType, class TMaskPixelType>
typename LinacMountedImagingDeviceProjectionProperties<TPixelType,
    TMaskPixelType>::MatrixType LinacMountedImagingDeviceProjectionProperties<
    TPixelType, TMaskPixelType>::GetUncorrectedProjectionPlaneOrientation()
{
  return this->Superclass::GetProjectionPlaneOrientation();
}

}

#endif /* ORALINACMOUNTEDIMAGINGDEVICEPROJECTIONPROPERTIES_TXX_ */
