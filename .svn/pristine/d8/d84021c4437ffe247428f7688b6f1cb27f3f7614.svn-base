//
#ifndef ORAGRADIENTMAGNITUDEDIFFERENCEIMAGETOIMAGEMETRIC_TXX_
#define ORAGRADIENTMAGNITUDEDIFFERENCEIMAGETOIMAGEMETRIC_TXX_

#include "oraGradientMagnitudeDifferenceImageToImageMetric.h"

#include <itkImageRegionConstIteratorWithIndex.h>

#include <itkImageFileWriter.h>

namespace ora
{

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
GradientMagnitudeDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::GradientMagnitudeDifferenceImageToImageMetric() :
  Superclass()
{
  m_NoOverlapMetricValue = 10000.;
  m_NoOverlapReactionMode = 0; // throw exception
  m_IsComputingDerivative = false;
  m_DerivativeScales.SetSize(0);
  m_TransformMovingImageFilter = NULL;
  m_MinFixedGradient = 0;
  m_MaxFixedGradient = 0;
  m_MinMovingGradient = 0;
  m_MaxMovingGradient = 0;
  m_FixedVariance = 0;
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
GradientMagnitudeDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::~GradientMagnitudeDifferenceImageToImageMetric()
{
  m_TransformMovingImageFilter = NULL;
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientMagnitudeDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Derivative Scales: " << m_DerivativeScales << "\n";
  os << indent << "No Overlap Reaction Mode: " << m_NoOverlapReactionMode << "\n";
  os << indent << "No Overlap Metric Value: " << m_NoOverlapMetricValue << "\n";
  os << indent << "Is Computing Derivative: " << m_IsComputingDerivative << "\n";

  os << indent << "Min Moving Gradient: " << m_MinMovingGradient << "\n";
  os << indent << "Max Moving Gradient: " << m_MaxMovingGradient << "\n";
  os << indent << "Min Fixed Gradient: " << m_MinFixedGradient << "\n";
  os << indent << "Max Fixed Gradient: " << m_MaxFixedGradient << "\n";
  os << indent << "Fixed Variance: " << m_FixedVariance << "\n";

  os << indent << "Transform Moving Image Filter: " << m_TransformMovingImageFilter.GetPointer() << "\n";
  os << indent << "Cast Fixed Image Filter: " << m_CastFixedImageFilter.GetPointer() << "\n";
  os << indent << "Cast Moving Image Filter: " << m_CastMovingImageFilter.GetPointer() << "\n";
  os << indent << "Fixed Gradient Magnitude Filter: " << m_FixedGradientMagnitudeFilter.GetPointer() << "\n";
  os << indent << "Moving Gradient Magnitude Filter: " << m_MovingGradientMagnitudeFilter.GetPointer() << "\n";
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientMagnitudeDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::SetTransform(TransformType *transform)
{
  if (transform && this->m_DerivativeScales.GetSize()
      != transform->GetNumberOfParameters())
  {
    this->m_DerivativeScales.SetSize(transform->GetNumberOfParameters());
    this->m_DerivativeScales.Fill(1.0);
  }
  Superclass::SetTransform(transform);
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientMagnitudeDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::Initialize() throw (itk::ExceptionObject)
{

  Superclass::Initialize();

  // filter for resampling moving image:
  m_TransformMovingImageFilter = TransformMovingImageFilterType::New();
  m_TransformMovingImageFilter->SetTransform(this->m_Transform);
  m_TransformMovingImageFilter->SetInterpolator(this->m_Interpolator);
  m_TransformMovingImageFilter->SetInput(this->m_MovingImage);
  m_TransformMovingImageFilter->SetDefaultPixelValue(0);
  m_TransformMovingImageFilter->SetSize(
      this->m_FixedImage->GetLargestPossibleRegion().GetSize());
  m_TransformMovingImageFilter->SetOutputOrigin(this->m_FixedImage->GetOrigin());
  m_TransformMovingImageFilter->SetOutputSpacing(
      this->m_FixedImage->GetSpacing());
  m_TransformMovingImageFilter->SetOutputDirection(
      this->m_FixedImage->GetDirection());
  m_TransformMovingImageFilter->ReleaseDataFlagOn();

  // fixed gradient image:
  m_CastFixedImageFilter = CastFixedImageFilterType::New();
  m_CastFixedImageFilter->SetInput(this->m_FixedImage);
  m_CastFixedImageFilter->ReleaseDataFlagOn();

  m_FixedGradientMagnitudeFilter = GradientMagnitudeType::New();
  m_FixedGradientMagnitudeFilter->SetInput(m_CastFixedImageFilter->GetOutput());
  m_FixedGradientMagnitudeFilter->UpdateLargestPossibleRegion();

  if (this->GetDebug())
  {
    typedef itk::ImageFileWriter<GradientImageType> WriterType;
    typename WriterType::Pointer w = WriterType::New();
    w->SetFileName("debugoutput-gradientmagnitude-fixed-image.mhd");
    w->SetInput(m_FixedGradientMagnitudeFilter->GetOutput());
    w->Update();
  }

  ComputeVariance(); // update fixed image gradient variance

  // transformed moving gradient image:
  m_CastMovingImageFilter = CastMovingImageFilterType::New();
  m_CastMovingImageFilter->SetInput(m_TransformMovingImageFilter->GetOutput());
  m_CastMovingImageFilter->ReleaseDataFlagOn();

  m_MovingGradientMagnitudeFilter = GradientMagnitudeType::New();
  m_MovingGradientMagnitudeFilter->SetInput(m_CastMovingImageFilter->GetOutput());
  m_MovingGradientMagnitudeFilter->UpdateLargestPossibleRegion();

  if (this->GetDebug())
  {
    typedef itk::ImageFileWriter<GradientImageType> WriterType;
    typename WriterType::Pointer w = WriterType::New();
    w->SetFileName("debugoutput-gradientmagnitude-moving-image.mhd");
    w->SetInput(m_MovingGradientMagnitudeFilter->GetOutput());
    w->Update();
  }
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientMagnitudeDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::ComputeVariance() const
{
  GradientPixelType mean;
  GradientPixelType gradient;
  double nPixels = 0;

  typedef itk::ImageRegionConstIteratorWithIndex<GradientImageType>
      IteratorType;

  IteratorType it(m_FixedGradientMagnitudeFilter->GetOutput(),
      this->GetFixedImageRegion());

  // compute mean gradient magnitude:
  nPixels = 0;
  gradient = it.Get();
  mean = 0;
  m_MinMovingGradient = gradient;
  m_MaxMovingGradient = gradient;
  while (!it.IsAtEnd())
  {
    gradient = it.Get();
    mean += gradient;
    if (gradient > m_MaxFixedGradient)
    {
      m_MaxFixedGradient = gradient;
    }
    if (gradient < m_MinFixedGradient)
    {
      m_MinFixedGradient = gradient;
    }
    nPixels += 1;
    ++it;
  }
  if (nPixels > 0)
  {
    mean /= nPixels;
  }

  // compute gradient variance:
  it.GoToBegin();
  m_FixedVariance = 0;
  while (!it.IsAtEnd())
  {
    gradient = it.Get();
    gradient -= mean;
    m_FixedVariance += (gradient * gradient);
    ++it;
  }
  m_FixedVariance /= nPixels;
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientMagnitudeDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::ComputeMovingGradientRange() const
{
  GradientPixelType gradient;

  typedef itk::ImageRegionConstIteratorWithIndex<GradientImageType>
      IteratorType;

  IteratorType it(m_MovingGradientMagnitudeFilter->GetOutput(),
      this->GetFixedImageRegion());

  gradient = it.Get();
  m_MinMovingGradient = gradient;
  m_MaxMovingGradient = gradient;
  while (!it.IsAtEnd())
  {
    gradient = it.Get();
    if (gradient > m_MaxMovingGradient)
    {
      m_MaxMovingGradient = gradient;
    }
    if (gradient < m_MinMovingGradient)
    {
      m_MinMovingGradient = gradient;
    }
    ++it;
  }
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
typename GradientMagnitudeDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::MeasureType GradientMagnitudeDifferenceImageToImageMetric<
    TFixedImage, TMovingImage, TGradientPixelType>::ComputeMeasure(
    const ParametersType &parameters) const
{
  this->SetTransformParameters(parameters); // take over transform

  m_TransformMovingImageFilter->UpdateLargestPossibleRegion();

  // update the gradient images:
  m_MovingGradientMagnitudeFilter->UpdateLargestPossibleRegion();

  // and update the gradient range:
  ComputeMovingGradientRange();  // FIXME: Is this really required because the values are never used

  MeasureType measure = itk::NumericTraits<MeasureType>::Zero;
  if (m_FixedVariance == itk::NumericTraits<GradientPixelType>::Zero)
  {
    // FIXME: Should we really return if variance is zero
    return measure;
  }

  // Iterate over the fixed and moving gradient images
  // calculating the similarity measure. In contrast to the original ITK
  // implementation, only the pixels within the fixed and moving image mask
  // are considered!
  typedef itk::ImageRegionConstIteratorWithIndex<GradientImageType>
      IteratorType;

  GradientPixelType mgradient;
  GradientPixelType fgradient;
  GradientPixelType diff;
  IteratorType fit(m_FixedGradientMagnitudeFilter->GetOutput(),
      this->GetFixedImageRegion());
  IteratorType mit(m_MovingGradientMagnitudeFilter->GetOutput(),
      this->GetFixedImageRegion());

//    m_FixedSobelFilters[d]->UpdateLargestPossibleRegion(); // FIXME: do we need this?
//    m_MovingSobelFilters[d]->UpdateLargestPossibleRegion();

  this->m_NumberOfPixelsCounted = 0;

  typename FixedImageType::IndexType index;
  typename Superclass::InputPointType inputPoint;
  typename Superclass::OutputPointType transformedPoint;
  FixedImageConstPointer fixedImage = this->m_FixedImage;
  TransformType const *transform = this->m_Transform;
  FixedImageMaskPointer fixedMask = this->m_FixedImageMask;
  MovingImageMaskPointer movingMask = this->m_MovingImageMask;
  this->m_Interpolator->SetInputImage(this->m_MovingImage);
  while (!fit.IsAtEnd())
  {
    index = fit.GetIndex();
    fixedImage->TransformIndexToPhysicalPoint(index, inputPoint);

    // NOTE: Yes, the moving image is already interpolated, but we have to
    // check against possibly set image masks.
    if (fixedMask && !fixedMask->IsInside(inputPoint))
    {
      ++fit;
      ++mit;
      continue;
    }
    transformedPoint = transform->TransformPoint(inputPoint);
    if (movingMask && !movingMask->IsInside(transformedPoint))
    {
      ++fit;
      ++mit;
      continue;
    }
    // FIXME: should we normalize the value somehow with the number of pixels?
//      if (!this->m_Interpolator->IsInsideBuffer(transformedPoint))
//      {
//        ++fit;
//        ++mit;
//        continue;
//      }
    if (this->m_Interpolator->IsInsideBuffer(transformedPoint))
    {
      this->m_NumberOfPixelsCounted++;
    }

    // Get the moving and fixed image gradients
    mgradient = mit.Get();
    fgradient = fit.Get();
    // and calculate the gradient magnitude difference
    // FIXME: same computation as is GradientDifferenceImagefilter was used
    // but this may be inappropriate
    diff = fgradient - mgradient;
    measure += m_FixedVariance / (m_FixedVariance + diff * diff);

    ++fit;
    ++mit;
  }

  // FIXME: should we normalize the value somehow with the number of pixels?

  if (this->m_NumberOfPixelsCounted <= 0)
  {
    if (m_NoOverlapReactionMode) // return specified metric value
    {
      measure = m_NoOverlapMetricValue;
    }
    else
    {
      itkExceptionMacro(<< "GD-ERROR: Fixed and moving image do not overlap!")
    }
  }

  return measure;
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
typename GradientMagnitudeDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::MeasureType GradientMagnitudeDifferenceImageToImageMetric<
    TFixedImage, TMovingImage, TGradientPixelType>::GetValue(
    const ParametersType &parameters) const
{
  MeasureType currentMeasure = ComputeMeasure(parameters);

  return currentMeasure;
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientMagnitudeDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::GetDerivative(const ParametersType &parameters,
    DerivativeType &derivative) const
{
  m_IsComputingDerivative = true; // yes, we're computing the derivative now
  ParametersType p = parameters;
  const unsigned int npars = this->GetNumberOfParameters();
  derivative = DerivativeType(npars);
  for (unsigned int i = 0; i < npars; i++)
  {
    p[i] -= this->m_DerivativeScales[i];
    const MeasureType v0 = this->GetValue(p);
    p[i] += 2 * this->m_DerivativeScales[i];
    const MeasureType v1 = this->GetValue(p);
    derivative[i] = (v1 - v0) / (2 * this->m_DerivativeScales[i]);
    p[i] = parameters[i];
  }
  m_IsComputingDerivative = false;
}

}

#endif /* ORAGRADIENTMAGNITUDEDIFFERENCEIMAGETOIMAGEMETRIC_TXX_ */
