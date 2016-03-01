//

#ifndef ORATRANSLATIONSCALEFLEXMAPCORRECTION_TXX_
#define ORATRANSLATIONSCALEFLEXMAPCORRECTION_TXX_

#include "oraTranslationScaleFlexMapCorrection.h"

namespace ora
{
template<class TPixelType, class TMaskPixelType>
const double TranslationScaleFlexMapCorrection<TPixelType, TMaskPixelType>::PI2 = M_PI * 2.0;

template<class TPixelType, class TMaskPixelType>
TranslationScaleFlexMapCorrection<TPixelType, TMaskPixelType>::TranslationScaleFlexMapCorrection() :
  Superclass()
{
  // as long as an interpolator is NULL, there is NO correction for this
  // parameter!
  m_XTranslationCW = NULL;
  m_XTranslationCCW = NULL;
  m_YTranslationCW = NULL;
  m_YTranslationCCW = NULL;
  m_ZTranslationCW = NULL;
  m_ZTranslationCCW = NULL;
  m_EssentialsOnly = true;
  m_UseSplineInterpolation = false;
}

template<class TPixelType, class TMaskPixelType>
TranslationScaleFlexMapCorrection<TPixelType, TMaskPixelType>::~TranslationScaleFlexMapCorrection()
{
  if (m_XTranslationCW != NULL)
  {
    delete m_XTranslationCW;
    m_XTranslationCW = NULL;
  }
  if (m_XTranslationCCW != NULL)
  {
    delete m_XTranslationCCW;
    m_XTranslationCCW = NULL;
  }
  if (m_YTranslationCW != NULL)
  {
    delete m_YTranslationCW;
    m_YTranslationCW = NULL;
  }
  if (m_YTranslationCCW != NULL)
  {
    delete m_YTranslationCCW;
    m_YTranslationCCW = NULL;
  }
  if (m_ZTranslationCW != NULL)
  {
    delete m_ZTranslationCW;
    m_ZTranslationCW = NULL;
  }
  if (m_ZTranslationCCW != NULL)
  {
    delete m_ZTranslationCCW;
    m_ZTranslationCCW = NULL;
  }
}

template<class TPixelType, class TMaskPixelType>
void TranslationScaleFlexMapCorrection<TPixelType, TMaskPixelType>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "X CW correction: " << (m_XTranslationCW != NULL) << "\n";
  os << indent << "X CCW correction: " << (m_XTranslationCCW != NULL) << "\n";
  os << indent << "Y CW correction: " << (m_YTranslationCW != NULL) << "\n";
  os << indent << "Y CCW correction: " << (m_YTranslationCCW != NULL) << "\n";
  os << indent << "Z CW correction: " << (m_ZTranslationCW != NULL) << "\n";
  os << indent << "Z CCW correction: " << (m_ZTranslationCCW != NULL) << "\n";
  os << indent << "Use Spline Interpolation: " << m_UseSplineInterpolation << "\n";
  os << indent << "Essentials Only: " << m_EssentialsOnly << "\n";
}

template<class TPixelType, class TMaskPixelType>
double TranslationScaleFlexMapCorrection<TPixelType, TMaskPixelType>::ProjectAngleInto0To2PIRange(
    double angle)
{
  while (angle < 0 || angle >= PI2)
  {
    if (angle < 0)
      angle += PI2;
    else if (angle >= PI2)
      angle -= PI2;
  }
  return angle;
}

template<class TPixelType, class TMaskPixelType>
bool TranslationScaleFlexMapCorrection<TPixelType, TMaskPixelType>::Correct()
{
  if (!this->m_LinacProjProps)
    return false;

  typename LinacProjectionPropertiesType::PointType orig =
      this->m_LinacProjProps->GetUncorrectedProjectionPlaneOrigin();
  typename LinacProjectionPropertiesType::MatrixType orient =
      this->m_LinacProjProps->GetUncorrectedProjectionPlaneOrientation();
  typename LinacProjectionPropertiesType::PointType focus =
      this->m_LinacProjProps->GetUncorrectedSourceFocalSpotPosition();

  // NOTE: this simple flex map correction will only have influence on
  // plane origin at the end!
  InterpolateFunction1D *xcorr = NULL;
  InterpolateFunction1D *ycorr = NULL;
  InterpolateFunction1D *zcorr = NULL;
  if (this->m_LinacProjProps->GetLinacGantryDirection() == 0) // CW
  {
    xcorr = m_XTranslationCW;
    ycorr = m_YTranslationCW;
    zcorr = m_ZTranslationCW;
  }
  else // CCW
  {
    xcorr = m_XTranslationCCW;
    ycorr = m_YTranslationCCW;
    zcorr = m_ZTranslationCCW;
  }
  double *v;
  double corr;
  int d;
  double angle = ProjectAngleInto0To2PIRange(
      this->m_LinacProjProps->GetLinacGantryAngle());
  if (xcorr)
  {
    v = orient[0]; // row-dir
    corr = xcorr->Interpolate(angle);
    for (d = 0; d < 3; d++)
      orig[d] += (v[d] * corr);
  }
  if (ycorr)
  {
    v = orient[1]; // col-dir
    corr = ycorr->Interpolate(angle);
    for (d = 0; d < 3; d++)
      orig[d] += (v[d] * corr);
  }
  if (zcorr)
  {
    v = orient[2]; // plane-normal
    corr = zcorr->Interpolate(angle);
    for (d = 0; d < 3; d++)
      orig[d] += (v[d] * corr);
  }
  // apply corrected origin (and rest of essentials)
  this->m_CorrectedProjProps->SetProjectionPlaneOrigin(orig);
  this->m_CorrectedProjProps->SetProjectionPlaneOrientation(orient);
  this->m_CorrectedProjProps->SetSourceFocalSpotPosition(focus);

  if (m_EssentialsOnly)
    return true;

  this->m_CorrectedProjProps->SetProjectionSize(
      this->m_LinacProjProps-> GetProjectionSize());
  this->m_CorrectedProjProps->SetProjectionSpacing(
      this->m_LinacProjProps-> GetProjectionSpacing());
  this->m_CorrectedProjProps->SetSamplingDistance(
      this->m_LinacProjProps-> GetSamplingDistance());
  // NOTE: same pointer!
  this->m_CorrectedProjProps->SetITF(this->m_LinacProjProps->GetITF());
  this->m_CorrectedProjProps->SetSamplingDistance(
      this->m_LinacProjProps-> GetSamplingDistance());
  this->m_CorrectedProjProps->SetRescaleSlope(
      this->m_LinacProjProps-> GetRescaleSlope());
  this->m_CorrectedProjProps->SetRescaleIntercept(
      this->m_LinacProjProps-> GetRescaleIntercept());
  // NOTE: same pointer!
  this->m_CorrectedProjProps->SetDRRMask(this->m_LinacProjProps->GetDRRMask());

  return true;
}

template<class TPixelType, class TMaskPixelType>
void TranslationScaleFlexMapCorrection<TPixelType, TMaskPixelType>::GenerateXYList(
    const std::string &str, std::vector<double> &xys)
{
  std::string::size_type lastPos = str.find_first_not_of(",", 0);
  std::string::size_type pos = str.find_first_of(",", lastPos);
  while (std::string::npos != pos || std::string::npos != lastPos)
  {
    xys.push_back(atof(str.substr(lastPos, pos - lastPos).c_str()));
    lastPos = str.find_first_not_of(",", pos);
    pos = str.find_first_of(",", lastPos);
  }
}

}

#endif /* ORATRANSLATIONSCALEFLEXMAPCORRECTION_TXX_ */
