//
#ifndef ORAPROJECTIONPROPERTIES_TXX_
#define ORAPROJECTIONPROPERTIES_TXX_

#include "oraProjectionProperties.h"

#include <vtkPlane.h>

namespace ora
{

template<class TPixelType, class TMaskPixelType>
ProjectionProperties<TPixelType, TMaskPixelType>::ProjectionProperties()
{
  this->m_SourceFocalSpotPosition.Fill(0);
  this->m_ProjectionPlaneOrigin.Fill(0);
  this->m_ProjectionPlaneOrientation.Fill(0);
  this->m_ProjectionSize.Fill(0);
  this->m_ProjectionSpacing.Fill(0);
  this->m_SamplingDistance = -1;
  this->m_ITF = NULL;
  this->m_LastSamplingDistanceComputationMode = -1;
  this->m_RescaleSlope = 1.;
  this->m_RescaleIntercept = 0.;
  this->m_DRRMask = NULL;
}

template<class TPixelType, class TMaskPixelType>
ProjectionProperties<TPixelType, TMaskPixelType>::~ProjectionProperties()
{
  this->m_ITF = NULL;
}
template<class TPixelType, class TMaskPixelType>
void ProjectionProperties<TPixelType, TMaskPixelType>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Projection Plane Origin: " << m_ProjectionPlaneOrigin
      << "\n";
  os << indent << "Source Focal Spot Position: " << m_SourceFocalSpotPosition
      << "\n";
  os << indent << "Projection Plane Orientation: \n"
      << m_ProjectionPlaneOrientation << "\n";
  os << indent << "Projection Size: " << m_ProjectionSize << "\n";
  os << indent << "Projection Spacing: " << m_ProjectionSpacing << "\n";
  os << indent << "Sampling Distance: " << m_SamplingDistance << "\n";
  os << indent << "Last Sampling Distance Computation Mode: "
      << m_LastSamplingDistanceComputationMode << "\n";
  os << indent << "Rescale Slope: " << m_RescaleSlope << "\n";
  os << indent << "Rescale Intercept: " << m_RescaleIntercept << "\n";
  os << indent << "DRR Mask: " << m_DRRMask.GetPointer() << "\n";

  os << indent << "Intensity Transfer Function: ";
  if (m_ITF != NULL)
  {
    os << "(n=" << m_ITF->GetSize() << ") ";
    double x[6];
    for (int i = 0; i < m_ITF->GetSize(); i++)
    {
      m_ITF->GetNodeValue(i, x);
      os << " " << x[0] << "," << x[1];
    }
    os << "\n";
  }
  else
  {
    os << "not set\n";
  }
}

template<class TPixelType, class TMaskPixelType>
void ProjectionProperties<TPixelType, TMaskPixelType>::SetGeometryFromFixedImage(
    FixedImagePointer fixedImage, FixedImageRegionType fixedRegion)
{
  if (!fixedImage)
    return;
  typename FixedImageRegionType::SizeType sz = fixedRegion.GetSize();
  if (sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0)
    return;
  if (!this->IsOrthogonalMatrix(fixedImage->GetDirection()))
    return;

  // projection plane orientation:
  // NOTE: direction is column-based, but our orientation is row-based!!!
  for (unsigned int d = 0; d < FixedDimension; d++)
    for (unsigned int c = 0; c < FixedDimension; c++)
      this->m_ProjectionPlaneOrientation[d][c]
          = fixedImage->GetDirection()[c][d];

  // projection plane origin (mm):
  typename FixedImageRegionType::IndexType idx = fixedRegion.GetIndex();
  for (unsigned int d = 0; d < FixedDimension; d++)
    this->m_ProjectionPlaneOrigin[d] = fixedImage->GetOrigin()[d];
  for (unsigned int d = 0; d < FixedDimension; d++)
  {
    for (unsigned int c = 0; c < FixedDimension; c++)
    {
      this->m_ProjectionPlaneOrigin[c]
          += static_cast<SpacingType::ValueType> (idx[d])
              * fixedImage->GetSpacing()[d]
              * this->m_ProjectionPlaneOrientation[d][c];
    }
  }

  // projection plane spacing (mm / pixel):
  for (unsigned int d = 0; d < 2; d++)
    this->m_ProjectionSpacing[d] = fixedImage->GetSpacing()[d];

  // projection plane size (pixels):
  for (unsigned int d = 0; d < 2; d++)
    this->m_ProjectionSize[d] = sz[d];
}

template<class TPixelType, class TMaskPixelType>
void ProjectionProperties<TPixelType, TMaskPixelType>::SetSamplingDistance(
    float distance)
{
  this->m_LastSamplingDistanceComputationMode = -1; // manual mode
  if (this->m_SamplingDistance != distance)
  {
    this->m_SamplingDistance = distance;
    this->Modified();
  }
}

template<class TPixelType, class TMaskPixelType>
float ProjectionProperties<TPixelType, TMaskPixelType>::ComputeAndSetSamplingDistanceFromVolume(
    Spacing3DType volumeSpacing, int mode)
{
  this->m_LastSamplingDistanceComputationMode = -1;
  if (mode < 0 || mode > 3)
    return -1;
  bool spacValid = true;
  float smallest = 9e20;
  float largest = -9e20;
  for (int d = 0; d < 3; d++)
  {
    if (volumeSpacing[d] <= 0)
      spacValid = false;
    if (volumeSpacing[d] > largest)
      largest = volumeSpacing[d];
    if (volumeSpacing[d] < smallest)
      smallest = volumeSpacing[d];
  }
  if (!spacValid)
    return -1;

  float dist = -1;
  if (mode == 0) // Shannon: smallest
  {
    dist = smallest / 2.f;
  }
  else if (mode == 1) // empirical: smallest
  {
    dist = smallest;
  }
  else if (mode == 2) // empirical: largest
  {
    dist = largest;
  }
  else if (mode == 3) // Shannon: largest
  {
    dist = largest / 2.f;
  }

  this->m_SamplingDistance = dist;
  this->m_LastSamplingDistanceComputationMode = mode; // store

  return dist;
}

template<class TPixelType, class TMaskPixelType>
bool ProjectionProperties<TPixelType, TMaskPixelType>::SetIntensityTransferFunctionFromNodePairs(
    TransferFunctionSpecificationType nodePairs)
{
  if ((nodePairs.Size() % 2) != 0)
    return false;

  double maxContribution = -1;
  unsigned int i;
  for (i = 1; i < nodePairs.Size(); i += 2)
  {
    if (nodePairs[i] < 0.) // clamp negative contributions
      nodePairs[i] = 0;
    if (nodePairs[i] > maxContribution)
      maxContribution = nodePairs[i];
  }
  if (maxContribution > 1.) // need global rescaling
  {
    for (i = 1; i < nodePairs.Size(); i += 2) // w.r.t. max. contribution
      nodePairs[i] /= maxContribution;
  }

  if (!this->m_ITF) // create on demand
    this->m_ITF = TransferFunctionPointer::New();

  this->m_ITF->RemoveAllPoints(); // add points to ITF (R-channel)
  for (i = 0; i < nodePairs.Size(); i += 2)
    //                               volume intensity, output intensity
    this->m_ITF->AddRGBPoint(nodePairs[i], nodePairs[i + 1], 0, 0);

  return true;
}

template<class TPixelType, class TMaskPixelType>
bool ProjectionProperties<TPixelType, TMaskPixelType>::IsGeometryValid() const
{
  if (!this->IsOrthogonalMatrix(this->m_ProjectionPlaneOrientation))
    return false;

  for (int d = 0; d < 2; d++)
  {
    if (this->m_ProjectionSize[d] <= 0)
      return false;
    if (this->m_ProjectionSpacing[d] <= 0.0)
      return false;
  }

  double fs[3];
  double n[3];
  double p0[3];
  for (int d = 0; d < 3; d++)
  {
    fs[d] = this->m_SourceFocalSpotPosition[d];
    n[d] = this->m_ProjectionPlaneOrientation[2][d];
    p0[d] = this->m_ProjectionPlaneOrigin[d];
  }
  if (vtkPlane::DistanceToPlane(fs, n, p0) < 1e-3)
    return false;

  return true;
}

template<class TPixelType, class TMaskPixelType>
bool ProjectionProperties<TPixelType, TMaskPixelType>::AreRayCastingPropertiesValid() const
{
  if (!this->m_ITF || this->m_ITF->GetSize() < 2)
    return false;
  if (this->m_SamplingDistance <= 1e-5)
    return false;

  bool itfvalid = true;
  double x[6];
  for (int i = 0; i < this->m_ITF->GetSize(); i++) // output range: [0;1]
  {
    this->m_ITF->GetNodeValue(i, x);
    if (x[1] < 0. || x[1] > 1.)
      itfvalid = false;
  }
  if (!itfvalid)
    return false;

  return true;
}

template<class TPixelType, class TMaskPixelType>
bool ProjectionProperties<TPixelType, TMaskPixelType>::AreMaskPropertiesValid() const
{
  if (!this->m_DRRMask) // no mask set
    return true;

  // NOTE: only the pixels are considered; i.e. size must match!
  typename MaskImageType::SizeType maskSize =
      this->m_DRRMask->GetLargestPossibleRegion().GetSize();
  if (maskSize[0] == this->m_ProjectionSize[0] && maskSize[1]
      == this->m_ProjectionSize[1])
    return true;
  else
    return false;
}

template<class TPixelType, class TMaskPixelType>
bool ProjectionProperties<TPixelType, TMaskPixelType>::AreAllPropertiesValid() const
{
  return this->IsGeometryValid() && this->AreRayCastingPropertiesValid()
      && this->AreMaskPropertiesValid();
}

template<class TPixelType, class TMaskPixelType>
bool ProjectionProperties<TPixelType, TMaskPixelType>::IsOrthogonalMatrix(
    const MatrixType &matrix) const
{
  bool orthogonal = true;

  // check whether plane orientation is orthogonal:
  // 1. is a quadratic (3x3) matrix and real
  // 2. check if the determinant is 1 or -1
  const double epsilon = 1e-5;
  const MatrixType::InternalMatrixType m = matrix.GetVnlMatrix();
  double det = fabs(vnl_determinant<double> (m));
  if (fabs(det - 1.0) > epsilon)
    orthogonal = false;
  if (det < 1e-5) // cannot compute inverse if det=0!
    return false;
  // 3. check if the inverse equals the transposed matrix
  MatrixType::InternalMatrixType mt = matrix.GetTranspose();
  MatrixType::InternalMatrixType mi = matrix.GetInverse();
  int i, j;
  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 3; j++)
    {
      if (fabs(mt[i][j] - mi[i][j]) > epsilon)
        orthogonal = false;
    }
  }

  return orthogonal;
}

}

#endif /* ORAPROJECTIONPROPERTIES_TXX_ */
