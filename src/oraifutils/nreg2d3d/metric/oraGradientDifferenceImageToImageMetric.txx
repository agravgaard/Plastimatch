//
#ifndef ORAGRADIENTDIFFERENCEIMAGETOIMAGEMETRIC_TXX_
#define ORAGRADIENTDIFFERENCEIMAGETOIMAGEMETRIC_TXX_

#include "oraGradientDifferenceImageToImageMetric.h"

#include <sstream>

#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageFileWriter.h>

namespace ora
{

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
GradientDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::GradientDifferenceImageToImageMetric() :
  Superclass()
{
  m_NoOverlapMetricValue = 10000.;
  m_NoOverlapReactionMode = 0; // throw exception
  m_IsComputingDerivative = false;
  m_DerivativeScales.SetSize(0);
  m_TransformMovingImageFilter = NULL;
  for (unsigned int d = 0; d < ImageDimension; d++)
  {
    m_MinFixedGradient[d] = 0;
    m_MaxFixedGradient[d] = 0;
    m_MinMovingGradient[d] = 0;
    m_MaxMovingGradient[d] = 0;
    m_FixedVariance[d] = 0;
  }
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
GradientDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::~GradientDifferenceImageToImageMetric()
{
  m_TransformMovingImageFilter = NULL;
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Derivative Scales: " << m_DerivativeScales << "\n";
  os << indent << "No Overlap Reaction Mode: " << m_NoOverlapReactionMode << "\n";
  os << indent << "No Overlap Metric Value: " << m_NoOverlapMetricValue << "\n";
  os << indent << "Is Computing Derivative: " << m_IsComputingDerivative << "\n";
  for (int d = 0; d < ImageDimension; d++)
  {
    os << indent << "Min Moving Gradient [" << d << "]: " << m_MinMovingGradient[d] << "\n";
    os << indent << "Max Moving Gradient [" << d << "]: " << m_MaxMovingGradient[d] << "\n";
    os << indent << "Min Fixed Gradient [" << d << "]: " << m_MinFixedGradient[d] << "\n";
    os << indent << "Max Fixed Gradient [" << d << "]: " << m_MaxFixedGradient[d] << "\n";
    os << indent << "Fixed Variance [" << d << "]: " << m_FixedVariance[d] << "\n";
    os << indent << "Fixed Sobel Operators [" << d << "]: " << m_FixedSobelOperators[d] << "\n";
    os << indent << "Fixed Sobel Filters [" << d << "]: " << m_FixedSobelFilters[d].GetPointer() << "\n";
    os << indent << "Moving Sobel Operators [" << d << "]: " << m_MovingSobelOperators[d] << "\n";
    os << indent << "Moving Sobel Filters [" << d << "]: " << m_MovingSobelFilters[d].GetPointer() << "\n";
  }
  os << indent << "Transform Moving Image Filter: " << m_TransformMovingImageFilter.GetPointer() << "\n";
  os << indent << "Cast Fixed Image Filter: " << m_CastFixedImageFilter.GetPointer() << "\n";
  os << indent << "Cast Moving Image Filter: " << m_CastMovingImageFilter.GetPointer() << "\n";
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientDifferenceImageToImageMetric<TFixedImage, TMovingImage,
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
void GradientDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::Initialize() throw (itk::ExceptionObject)
{
  // NOTE: Removed check for Gradient computation because the gradient volume
  // is never used and requires a big amount of memory (CovariantVector<double>)
  // if (!this->GetComputeGradient())
  // {
  //  itkExceptionMacro(<< "Gradient difference image to image metric requires ComputeGradient set to TRUE!");
  // }

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
  for (unsigned int d = 0; d < ImageDimension; d++)
  {
    m_FixedSobelOperators[d].SetDirection(d);
    m_FixedSobelOperators[d].CreateDirectional();
    m_FixedSobelFilters[d] = SobelFilterType::New();
    m_FixedSobelFilters[d]->OverrideBoundaryCondition(&m_FixedCondition);
    m_FixedSobelFilters[d]->SetOperator(m_FixedSobelOperators[d]);
    m_FixedSobelFilters[d]->SetInput(m_CastFixedImageFilter->GetOutput());
    m_FixedSobelFilters[d]->UpdateLargestPossibleRegion();

    if (this->GetDebug())
    {
      typedef itk::ImageFileWriter<GradientImageType> WriterType;
      typename WriterType::Pointer w = WriterType::New();
      std::stringstream filename;
      filename << "debugoutput-gradientsobel-moving-image-" << d << ".mhd";
      w->SetFileName(filename.str().c_str());
      w->SetInput(m_FixedSobelFilters[d]->GetOutput());
      w->Update();
    }
  }
  ComputeVariance(); // update fixed image gradient variance

  // transformed moving gradient image:
  m_CastMovingImageFilter = CastMovingImageFilterType::New();
  m_CastMovingImageFilter->SetInput(m_TransformMovingImageFilter->GetOutput());
  m_CastMovingImageFilter->ReleaseDataFlagOn();
  for (unsigned int d = 0; d < ImageDimension; d++)
  {
    m_MovingSobelOperators[d].SetDirection(d);
    m_MovingSobelOperators[d].CreateDirectional();
    m_MovingSobelFilters[d] = SobelFilterType::New();
    m_MovingSobelFilters[d]->OverrideBoundaryCondition(&m_MovingCondition);
    m_MovingSobelFilters[d]->SetOperator(m_MovingSobelOperators[d]);
    m_MovingSobelFilters[d]->SetInput(m_CastMovingImageFilter->GetOutput());
    m_MovingSobelFilters[d]->UpdateLargestPossibleRegion();
    if (this->GetDebug())
    {
      typedef itk::ImageFileWriter<GradientImageType> WriterType;
      typename WriterType::Pointer w = WriterType::New();
      std::stringstream filename;
      filename << "debugoutput-gradientsobel-moving-image-" << d << ".mhd";
      w->SetFileName(filename.str().c_str());
      w->SetInput(m_MovingSobelFilters[d]->GetOutput());
      w->Update();
    }
  }

}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::ComputeVariance() const
{
  GradientPixelType mean[FixedImageDimension];
  GradientPixelType gradient;
  double nPixels = 0;
  for (unsigned int d = 0; d < ImageDimension; d++)
  {
    typedef itk::ImageRegionConstIteratorWithIndex<GradientImageType>
        IteratorType;

    IteratorType it(m_FixedSobelFilters[d]->GetOutput(),
        this->GetFixedImageRegion());

    // compute mean gradients:
    nPixels = 0;
    gradient = it.Get();
    mean[d] = 0;
    m_MinMovingGradient[d] = gradient;
    m_MaxMovingGradient[d] = gradient;
    while (!it.IsAtEnd())
    {
      gradient = it.Get();
      mean[d] += gradient;
      if (gradient > m_MaxFixedGradient[d])
      {
        m_MaxFixedGradient[d] = gradient;
      }
      if (gradient < m_MinFixedGradient[d])
      {
        m_MinFixedGradient[d] = gradient;
      }
      nPixels += 1;
      ++it;
    }
    if (nPixels > 0)
    {
      mean[d] /= nPixels;
    }

    // compute gradient variance:
    it.GoToBegin();
    m_FixedVariance[d] = 0;
    while (!it.IsAtEnd())
    {
      gradient = it.Get();
      gradient -= mean[d];
      m_FixedVariance[d] += (gradient * gradient);
      ++it;
    }
    m_FixedVariance[d] /= nPixels;
  }
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::ComputeMovingGradientRange() const
{
  GradientPixelType gradient;
  for (unsigned int d = 0; d < ImageDimension; d++)
  {
    typedef itk::ImageRegionConstIteratorWithIndex<GradientImageType>
        IteratorType;

    IteratorType it(m_MovingSobelFilters[d]->GetOutput(),
        this->GetFixedImageRegion());

    gradient = it.Get();
    m_MinMovingGradient[d] = gradient;
    m_MaxMovingGradient[d] = gradient;
    while (!it.IsAtEnd())
    {
      gradient = it.Get();
      if (gradient > m_MaxMovingGradient[d])
      {
        m_MaxMovingGradient[d] = gradient;
      }
      if (gradient < m_MinMovingGradient[d])
      {
        m_MinMovingGradient[d] = gradient;
      }
      ++it;
    }
  }
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
typename GradientDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::MeasureType GradientDifferenceImageToImageMetric<
    TFixedImage, TMovingImage, TGradientPixelType>::ComputeMeasure(
    const ParametersType &parameters) const
{
  this->SetTransformParameters(parameters); // take over transform

  m_TransformMovingImageFilter->UpdateLargestPossibleRegion();

  // update the gradient images:
  for (unsigned int d = 0; d < ImageDimension; d++)
  {
    m_MovingSobelFilters[d]->UpdateLargestPossibleRegion();
  }
  // and update the gradient range:
  ComputeMovingGradientRange();  // FIXME: Is this really required because the values are never used

  MeasureType measure = itk::NumericTraits<MeasureType>::Zero;
  for (unsigned int d = 0; d < ImageDimension; d++)
  {
    if (m_FixedVariance[d] == itk::NumericTraits<GradientPixelType>::Zero)
    {
      continue;
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
    IteratorType fit(m_FixedSobelFilters[d]->GetOutput(),
        this->GetFixedImageRegion());
    IteratorType mit(m_MovingSobelFilters[d]->GetOutput(),
        this->GetFixedImageRegion());

    //m_FixedSobelFilters[d]->UpdateLargestPossibleRegion(); // FIXME: do we need this?
    //m_MovingSobelFilters[d]->UpdateLargestPossibleRegion();

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
      // and calculate the gradient difference
      diff = fgradient - mgradient;
      measure += m_FixedVariance[d] / (m_FixedVariance[d] + diff * diff);

      ++fit;
      ++mit;
    }
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
typename GradientDifferenceImageToImageMetric<TFixedImage, TMovingImage,
    TGradientPixelType>::MeasureType GradientDifferenceImageToImageMetric<
    TFixedImage, TMovingImage, TGradientPixelType>::GetValue(
    const ParametersType &parameters) const
{
  MeasureType currentMeasure = ComputeMeasure(parameters);

  return currentMeasure;
}

template<class TFixedImage, class TMovingImage, class TGradientPixelType>
void GradientDifferenceImageToImageMetric<TFixedImage, TMovingImage,
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

#endif /* ORAGRADIENTDIFFERENCEIMAGETOIMAGEMETRIC_TXX_ */
