//
#ifndef ORADRRITFLIKELIHOODCOSTFUNCTION_TXX_
#define ORADRRITFLIKELIHOODCOSTFUNCTION_TXX_

#include "oraDRRITFLikelihoodCostFunction.h"

#include "oraMathTools.h"

#include <itkCommand.h>

namespace ora
{

template<class TFixedImage, class TMovingImage>
DRRITFLikelihoodCostFunction<TFixedImage, TMovingImage>::DRRITFLikelihoodCostFunction() :
  Superclass()
{
  this->m_ComputeGradient = false;
  m_DerivativeScales.SetSize(0);
  m_FixedNumberOfHistogramBins = 256;
  m_FixedHistogramMinIntensity
      = itk::NumericTraits<FixedPixelType>::ZeroValue();
  m_FixedHistogramMaxIntensity = itk::NumericTraits<FixedPixelType>::max();
  m_FixedHistogramClipAtEnds = false;
  m_FixedHistogram = NULL;
  m_MapOutsideIntensitiesToZeroProbability = true;
}

template<class TFixedImage, class TMovingImage>
DRRITFLikelihoodCostFunction<TFixedImage, TMovingImage>::~DRRITFLikelihoodCostFunction()
{
  m_FixedHistogram = NULL;
}

template<class TFixedImage, class TMovingImage>
void DRRITFLikelihoodCostFunction<TFixedImage, TMovingImage>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  Superclass::Print(os, indent);

  os << indent << "Derivative Scales: " << m_DerivativeScales << "\n";
  os << indent << "Fixed Number Of Histogram Bins: "
      << m_FixedNumberOfHistogramBins << "\n";
  os << indent << "Fixed Histogram Min Intensity: "
      << m_FixedHistogramMinIntensity << "\n";
  os << indent << "Fixed Histogram Max Intensity: "
      << m_FixedHistogramMaxIntensity << "\n";
  os << indent << "Fixed Histogram Clip At Ends: "
      << m_FixedHistogramClipAtEnds << "\n";
  os << indent << "Fixed Histogram: " << m_FixedHistogram.GetPointer() << "\n";
  os << indent << "Map Outside Intensities To Zero Probability: "
      << m_MapOutsideIntensitiesToZeroProbability << "\n";
}

template<class TFixedImage, class TMovingImage>
void DRRITFLikelihoodCostFunction<TFixedImage, TMovingImage>::ExtractFixedImageHistogram()
{
  m_FixedHistogram = NULL;
  m_FixedHistogram = HistogramType::New();
  typename HistogramType::SizeType histSize;
  histSize.Fill(this->m_FixedNumberOfHistogramBins);
  typename HistogramType::MeasurementVectorType lbound;
  lbound.Fill(this->m_FixedHistogramMinIntensity);
  typename HistogramType::MeasurementVectorType ubound;
  ubound.Fill(this->m_FixedHistogramMaxIntensity);
  m_FixedHistogram->Initialize(histSize, lbound, ubound);
  m_FixedHistogram->SetClipBinsAtEnds(m_FixedHistogramClipAtEnds);

  // NOTE: the histogram is extracted from the fixed image; fixed image region
  // and fixed image mask are considered!

  FixedImageConstPointer fixedImage = this->m_FixedImage;
  HistogramPointer histogram = m_FixedHistogram;
  FixedImageMaskPointer fixedMask = this->m_FixedImageMask;
  histogram->SetToZero();
  IteratorType fi(fixedImage, this->GetFixedImageRegion());
  typename Superclass::InputPointType fixedPoint;
  typename FixedImageType::IndexType index;
  double v;
  fi.GoToBegin();
  if (fixedMask)
  {
    while (!fi.IsAtEnd())
    {
      index = fi.GetIndex();
      fixedImage->TransformIndexToPhysicalPoint(index, fixedPoint);
      if (fixedMask->IsInside(fixedPoint))
      {
        v = static_cast<double> (fi.Get());
        histogram->IncreaseFrequency(v, 1);
      }
      ++fi;
    }
  }
  else
  {
    while (!fi.IsAtEnd())
    {
      v = static_cast<double> (fi.Get());
      histogram->IncreaseFrequency(v, 1);
      ++fi;
    }
  }
}

template<class TFixedImage, class TMovingImage>
void DRRITFLikelihoodCostFunction<TFixedImage, TMovingImage>::Initialize()
    throw (itk::ExceptionObject)
{
  Superclass::Initialize();

  // extract histogram of fixed image considering set fixed image region and
  // set fixed image mask:
  ExtractFixedImageHistogram(); // -> m_FixedHistogram
}

template<class TFixedImage, class TMovingImage>
typename DRRITFLikelihoodCostFunction<TFixedImage, TMovingImage>::MeasureType DRRITFLikelihoodCostFunction<
    TFixedImage, TMovingImage>::GetValue(const ParametersType &parameters) const
{
  this->SetTransformParameters(parameters);

  this->m_NumberOfPixelsCounted = 0;

  // NOTE: basically, the fixed and moving image intensities should cover a
  // very similar intensity range, otherwise the metric value won't be very
  // meaningful.
  double measure = 0;

  FixedImageConstPointer fixedImage = this->m_FixedImage;
  HistogramPointer histogram = m_FixedHistogram;
  FixedImageMaskPointer fixedMask = this->m_FixedImageMask;
  MovingImageMaskPointer movingMask = this->m_MovingImageMask;
  typename Superclass::TransformPointer transform = this->m_Transform;
  typename Superclass::InterpolatorPointer interpolator = this->m_Interpolator;
  IteratorType fi(fixedImage, this->GetFixedImageRegion());
  typename Superclass::InputPointType inputPoint;
  typename Superclass::OutputPointType transformedPoint;
  typename FixedImageType::IndexType index;
  double v;
  fi.GoToBegin();
  while (!fi.IsAtEnd())
  {
    index = fi.GetIndex();
    fixedImage->TransformIndexToPhysicalPoint(index, inputPoint);

    // consider fixed image mask
    if (fixedMask && !fixedMask->IsInside(inputPoint))
    {
      ++fi;
      continue;
    }

    // consider moving image mask
    transformedPoint = transform->TransformPoint(inputPoint);
    if (movingMask && !movingMask->IsInside(transformedPoint))
    {
      ++fi;
      continue;
    }

    // interpolate and compute moving image value:
    if (interpolator->IsInsideBuffer(transformedPoint))
    {
      // simply sum up the logarithm of the probability of the investigated
      // moving pixel value expressed in terms of the fixed image distribution:

      v
          = static_cast<double> (this->m_Interpolator->Evaluate(
              transformedPoint));

      if (m_MapOutsideIntensitiesToZeroProbability && (v
          < m_FixedHistogramMinIntensity || v > m_FixedHistogramMaxIntensity))
      {
        ++fi;
        continue;
      }

      v = histogram->GetFrequency(histogram->GetIndex(v));
      measure += LogOnePlusX(v);

      this->m_NumberOfPixelsCounted++;
    }

    ++fi;
  }

  if (this->m_NumberOfPixelsCounted <= 0)
  {
    itkDebugMacro(<< "WARNING: Fixed and moving image do not overlap!")
  }
  else
  {
    measure /= static_cast<double> (this->m_NumberOfPixelsCounted);
  }

  return measure;
}

template<class TFixedImage, class TMovingImage>
void DRRITFLikelihoodCostFunction<TFixedImage, TMovingImage>::GetDerivative(
    const ParametersType &parameters, DerivativeType &derivative) const
{
  // simple finite distance approach:
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
}

}

#endif /* ORADRRITFLIKELIHOODCOSTFUNCTION_TXX_ */
