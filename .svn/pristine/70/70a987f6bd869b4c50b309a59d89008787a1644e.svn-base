//
#ifndef ORASTOCHASTICRANKCORRELATIONIMAGETOIMAGEMETRIC_TXX_
#define ORASTOCHASTICRANKCORRELATIONIMAGETOIMAGEMETRIC_TXX_

#include "oraStochasticRankCorrelationImageToImageMetric.h"

#include <itkRealTimeClock.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkImageFileWriter.h>
#include <itkCommand.h>
#include <itkMath.h>

#include <fstream>
#include <vector>

#include <vnl/vnl_random.h>

namespace ora
{

template<class TFixedImage, class TMovingImage, class TRankImage>
StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::StochasticRankCorrelationImageToImageMetric()
{
  m_FixedRankImage = NULL;
  m_SampleCoverage = 5.0; // 5 % default (see Birkfellner et al.)
  m_RandomSeeds.SetSize(ImageDimension);
  m_RandomSeeds.Fill(0); // non-deterministic by default
  m_DerivativeScales.SetSize(0);
  m_StochasticMask = NULL;
  m_FixedNumberOfHistogramBins = 256;
  m_FixedHistogramMinIntensity
      = itk::NumericTraits<FixedPixelType>::ZeroValue();
  m_FixedHistogramMaxIntensity = itk::NumericTraits<FixedPixelType>::max();
  m_FixedHistogramClipAtEnds = false;
  m_MovingNumberOfHistogramBins = 256;
  m_MovingHistogramMinIntensity
      = itk::NumericTraits<MovingPixelType>::ZeroValue();
  m_MovingHistogramMaxIntensity = itk::NumericTraits<MovingPixelType>::max();
  m_MovingHistogramClipAtEnds = false;
  m_MovingRankHistogram = NULL;
  m_NoOverlapMetricValue = 10000.;
  m_NoOverlapReactionMode = 0; // throw exception
  m_StochasticUserMask = NULL;
  m_UseHornTiedRanksCorrection = false; // OFF by default
  m_HornCorrectionSums[0] = m_HornCorrectionSums[1] = 0;
  m_MovingZeroRanksContributeToMeasure = true; // default: ON
  m_IsComputingDerivative = false;
  m_ExtractSampleDistribution = false;
  m_SampleDistribution.clear();
}

template<class TFixedImage, class TMovingImage, class TRankImage>
StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::~StochasticRankCorrelationImageToImageMetric()
{
  m_FixedRankImage = NULL;
  m_StochasticMask = NULL;
  m_MovingRankHistogram = NULL;
  m_StochasticUserMask = NULL;
}

template<class TFixedImage, class TMovingImage, class TRankImage>
void StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << "Fixed Rank Image: " << m_FixedRankImage.GetPointer() << "\n";
  os << indent << "Sample Coverage: " << m_SampleCoverage << " %\n";
  os << indent << "Stochastic User Mask: " << m_StochasticUserMask.GetPointer()
      << "\n";
  os << indent << "Random Seeds: " << m_RandomSeeds << "\n";
  os << indent << "Derivative Scales: " << m_DerivativeScales << "\n";
  os << indent << "Stochastic Mask: " << m_StochasticMask.GetPointer() << "\n";
  os << indent << "Moving Rank Histogram: "
      << m_MovingRankHistogram.GetPointer() << "\n";
  os << indent << "Fixed Number Of Histogram Bins: "
      << m_FixedNumberOfHistogramBins << "\n";
  os << indent << "Fixed Histogram Min Intensity: "
      << m_FixedHistogramMinIntensity << "\n";
  os << indent << "Fixed Histogram Max Intensity: "
      << m_FixedHistogramMaxIntensity << "\n";
  os << indent << "Fixed Histogram Clip At Ends: "
      << m_FixedHistogramClipAtEnds << "\n";
  os << indent << "Moving Number Of Histogram Bins: "
      << m_MovingNumberOfHistogramBins << "\n";
  os << indent << "Moving Histogram Min Intensity: "
      << m_MovingHistogramMinIntensity << "\n";
  os << indent << "Moving Histogram Max Intensity: "
      << m_MovingHistogramMaxIntensity << "\n";
  os << indent << "Moving Histogram Clip At Ends: "
      << m_MovingHistogramClipAtEnds << "\n";
  os << indent << "No Overlap Metric Value: " << m_NoOverlapMetricValue << "\n";
  os << indent << "No Overlap Reaction Mode: " << m_NoOverlapReactionMode
      << "\n";
  os << indent << "Use Horn Tied Ranks Correction: "
      << m_UseHornTiedRanksCorrection << "\n";
  os << indent << "Horn Correction Sums (fixed, moving): "
      << m_HornCorrectionSums[0] << ", " << m_HornCorrectionSums[1] << "\n";
  os << indent << "Moving Zero Ranks Contribute To Measure: " <<
      m_MovingZeroRanksContributeToMeasure << "\n";
  os << indent << "Is Computing Derivative: " << m_IsComputingDerivative
      << "\n";
  os << indent << "Extract Sample Distribution (DEBUGGING): "
      << m_ExtractSampleDistribution << "\n";
}

template<class TFixedImage, class TMovingImage, class TRankImage>
void StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::SetStochasticUserMask(MaskImageType *mask)
{
  if (!mask || !this->m_FixedImage)
  {
    if (this->m_StochasticUserMask)
    {
      this->m_StochasticUserMask = NULL;
      this->Modified();
    }
    return;
  }

  if (this->m_StochasticUserMask != mask)
  {
    typename FixedImageType::SizeType fsz =
        this->m_FixedImage->GetLargestPossibleRegion().GetSize();
    typename MaskImageType::SizeType msz =
        mask->GetLargestPossibleRegion().GetSize();
    for (unsigned int d = 0; d < ImageDimension; d++)
    {
      if (fsz[d] != msz[d])
        return;
    }
    this->m_StochasticUserMask = mask;
    this->Modified();
  }
}

template<class TFixedImage, class TMovingImage, class TRankImage>
bool StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::GenerateStochasticMaskImage()
{
  m_StochasticMask = NULL;
  if (!this->m_FixedImage || (this->m_SampleCoverage <= 0
      && !m_StochasticUserMask))
    return false;

  // NOTE: If we neither have a fixed image region different from the fixed
  // image's largest possible region, nor have a fixed image mask while having
  // full sample coverage, we do not have to provide a stochastic mask for the
  // ranker! Set stochastic mask to NULL!
  if (m_SampleCoverage >= 100 && !m_StochasticUserMask
      && this->GetFixedImageRegion()
          == this->m_FixedImage->GetLargestPossibleRegion()
      && !this->m_FixedImageMask)
    return true;

  if (!m_StochasticUserMask) // via sample coverage
  {
    // generate mask image object (same size as LARGEST possible fixed region):
    m_StochasticMask = MaskImageType::New();
    m_StochasticMask->SetOrigin(this->m_FixedImage->GetOrigin());
    m_StochasticMask->SetSpacing(this->m_FixedImage->GetSpacing());
    m_StochasticMask->SetRegions(this->m_FixedImage->GetLargestPossibleRegion());
    m_StochasticMask->SetDirection(this->m_FixedImage->GetDirection());
    m_StochasticMask->Allocate();

    if (m_SampleCoverage < 100) // we need to sample a portion of pixels
    {
      // prepare random samplers:
      std::vector<vnl_random *> samplers;
      itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
      SeedValueType seed = (SeedValueType) (clock->GetTimeStamp() * 1e6);
      for (unsigned int d = 0; d < ImageDimension; d++)
      {
        vnl_random *sampler = new vnl_random();
        if (this->m_RandomSeeds[d] == 0) // non-deterministic
        {
          sampler->reseed(seed);
          seed = static_cast<SeedValueType> (((seed % 1001) + 1)
              * 1.2349577273633e12);
        }
        else // deterministic
        {
          sampler->reseed(this->m_RandomSeeds[d]);
        }
        samplers.push_back(sampler);
      }

      // try to be as efficient in random mask generation as possible;
      // distinguish between <=50% and >50% coverage:
      MaskPixelType masked, unmasked;
      double endpoint;
      if (this->m_SampleCoverage <= 50.)
      {
        endpoint = this->m_SampleCoverage / 100.;
        masked = 0;
        unmasked = 1;
      }
      else // <=100
      {
        endpoint = 1.0 - this->m_SampleCoverage / 100.0;
        masked = 1;
        unmasked = 0;
      }
      m_StochasticMask->FillBuffer(masked); // initialize with masked pixels

      typename FixedImageType::RegionType fir = this->GetFixedImageRegion();
      double totalPixels = fir.GetNumberOfPixels();
      if (this->m_FixedImageMask) // we have to subtract the fixed masked pixels
      {
        FixedIteratorType fit(this->m_FixedImage, fir);
        fit.GoToBegin();
        FixedImageConstPointer fixedImage = this->m_FixedImage;
        FixedImageMaskPointer fixedMask = this->m_FixedImageMask;
        typename Superclass::InputPointType fixedPoint;
        typename FixedImageType::IndexType index;
        while (!fit.IsAtEnd())
        {
          index = fit.GetIndex();
          fixedImage->TransformIndexToPhysicalPoint(index, fixedPoint);
          if (!fixedMask->IsInside(fixedPoint))
            totalPixels -= 1;
          ++fit;
        }
      }
      double sampledPixels = 0;
      MaskIteratorType mit(m_StochasticMask, fir);
      typename MaskImageType::RegionType::IndexType idx;
      bool doIt;
      FixedImageMaskPointer fixedMask = this->m_FixedImageMask;
      FixedImageConstPointer fixedImage = this->m_FixedImage;
      typename Superclass::InputPointType fixedPoint;
      typename MaskImageType::RegionType::IndexType minIdx;
      typename MaskImageType::RegionType::IndexType maxIdx;
      for (unsigned int d = 0; d < ImageDimension; d++)
      {
        minIdx[d] = fir.GetIndex()[d];
        maxIdx[d] = minIdx[d] + fir.GetSize()[d] - 1;
      }
      while (sampledPixels / totalPixels < endpoint)
      {
        for (unsigned int d = 0; d < ImageDimension; d++)
        {
          idx[d] = static_cast<typename MaskImageType::RegionType::IndexValueType> (
                   itk::Math::Round<int, double>(samplers[d]->drand32(minIdx[d], maxIdx[d])));
        }
        mit.SetIndex(idx);
        doIt = (mit.Get() != unmasked);
        if (doIt && fixedMask)
        {
          fixedImage->TransformIndexToPhysicalPoint(idx, fixedPoint);
          if (!fixedMask->IsInside(fixedPoint))
            doIt = false;
        }
        if (doIt) // do not sample pixels twice or masked pixels (fixed mask)
        {
          mit.Set(unmasked);
          sampledPixels += 1;
        }
      }

      // free random samplers:
      for (unsigned int d = 0; d < ImageDimension; d++)
      {
        vnl_random *sampler = samplers[d];
        delete sampler;
      }
      samplers.clear();
      clock = NULL;
    }
    else // 100 % coverage
    {
      m_StochasticMask->FillBuffer(1); // unmasked
    }
  }
  else // via user-specified stochastic mask
  {
    m_StochasticMask = m_StochasticUserMask; // simply set reference
  }

  // ensure that we only have pixels within fixed image region & within fixed
  // image mask:
  typename MaskImageType::RegionType::IndexType idx;
  FixedImageMaskPointer fixedMask = this->m_FixedImageMask;
  FixedImageConstPointer fixedImage = this->m_FixedImage;
  typename Superclass::InputPointType fixedPoint;
  MaskIteratorType mit2(m_StochasticMask,
      m_StochasticMask->GetLargestPossibleRegion());
  mit2.GoToBegin();
  typename FixedImageType::RegionType fir = this->GetFixedImageRegion();
  while (!mit2.IsAtEnd())
  {
    idx = mit2.GetIndex();
    if (!fir.IsInside(idx))
    {
      mit2.Set(0); // eliminate - outside fixed image region
    }
    else if (fixedMask)
    {
      fixedImage->TransformIndexToPhysicalPoint(idx, fixedPoint);
      if (!fixedMask->IsInside(fixedPoint))
        mit2.Set(0); // eliminate - outside fixed image mask
    }
    ++mit2;
  }

  this->Superclass::InvokeEvent(ora::AfterMaskCreation());

  return true;
}

template<class TFixedImage, class TMovingImage, class TRankImage>
void StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::ExtractFixedSampleDistribution() const
{
  if (m_ExtractSampleDistribution)
  {
    m_SampleDistribution.clear();
    typedef itk::ImageRegionConstIterator<FixedImageType> FixedIteratorType;
    FixedIteratorType fi(this->m_FixedImage, this->GetFixedImageRegion());
    FixedRankIteratorType fri(this->m_FixedRankImage,
        this->GetFixedImageRegion());
    typename RankImageType::PixelType rv;
    FixedPixelType fv;
    fri.GoToBegin();
    fi.GoToBegin();
    while (!fri.IsAtEnd())
    {
      rv = fri.Get();
      if (rv > 0)
      {
        SampleDistributionEntry sde;
        sde.FixedRankIntensity = static_cast<double> (rv);
        fv = fi.Get();
        sde.FixedIntensity = static_cast<double> (fv);
        m_SampleDistribution.push_back(sde);
      }
      ++fri;
      ++fi;
    }
  }
}

template<class TFixedImage, class TMovingImage, class TRankImage>
void StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::FixedAfterHistogramExtractionEvent(itk::Object *obj,
    const itk::EventObject &ev, void *cd)
{
  double *horn = static_cast<double*>(cd);
  RankFilterPointer ranker = reinterpret_cast<RankFilterType *>(obj);
  if (horn && ranker)
  {
    horn[1] = 0; // fixed sample pixel count
    // update Horn correction sum:
    HistogramPointer frhist = ranker->GetAveragedRankHistogram();
    typename HistogramType::Iterator frhit = frhist->Begin();
    typename HistogramType::Iterator frhitend = frhist->End();
    horn[0] = 0.;
    double ifreq;
    for (; frhit != frhitend; ++frhit)
    {
      ifreq = frhit.GetFrequency();
      if (ifreq > 1) // generate horn correction sum
      {
        horn[0] = horn[0] + ifreq * (ifreq * ifreq - 1) / 12.;
      }
      horn[1] = horn[1] + ifreq;
    }
  }
}

template<class TFixedImage, class TMovingImage, class TRankImage>
void StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::Initialize() throw (itk::ExceptionObject)
{
  this->Superclass::Initialize();

  m_FixedRankImage = NULL;

  // generate mask that contains the pixels that contribute to metric (see
  // m_SampleCoverage):
  if (!GenerateStochasticMaskImage())
    return;

  typedef itk::CStyleCommand CommandType;
  typedef CommandType::Pointer CommandPointer;


  // generate the fixed rank image:
  RankFilterPointer ranker = RankFilterType::New();
  ranker->SetInput(this->m_FixedImage); // ranking filter
  ranker->SetMaskImage(m_StochasticMask);
  ranker->SetDoNotGenerateOutput(false); // need output here
  ranker->SetUseHistogramTransformation(false); // no transformation desired
  ranker->SetGenerateOutputForMaskedPixelsOnly(true); // unmasked -> 0-ranks
  ranker->SetNumberOfHistogramBins(m_FixedNumberOfHistogramBins);
  ranker->SetHistogramMinIntensity(m_FixedHistogramMinIntensity);
  ranker->SetHistogramMaxIntensity(m_FixedHistogramMaxIntensity);
  ranker->SetHistogramClipAtEnds(m_FixedHistogramClipAtEnds);
  m_FixedRankImage = ranker->GetOutput();
  CommandPointer cmd = CommandType::New(); // for Horn-correction-sum-extraction
  double tmp[2];
  cmd->SetClientData(&tmp[0]);
  cmd->SetCallback(FixedAfterHistogramExtractionEvent);
  ranker->AddObserver(AfterHistogramExtraction(), cmd);
  ranker->Update(); // generate the image
  ranker->RemoveAllObservers();
  m_HornCorrectionSums[0] = tmp[0];
  m_NumberOfFixedSamplePixels = static_cast<unsigned int>(tmp[1]);
  cmd = NULL;
  m_FixedRankImage->DisconnectPipeline();
  ranker = NULL;
  m_StochasticMask = NULL; // no longer needed!

  ExtractFixedSampleDistribution(); // for DEBUGGING

  // prepare moving image rank histogram:
  m_MovingRankHistogram = NULL;
  m_MovingRankHistogram = HistogramType::New();
  typename HistogramType::SizeType histSize;
  histSize.Fill(this->m_MovingNumberOfHistogramBins);
  typename HistogramType::MeasurementVectorType lbound;
  lbound.Fill(this->m_MovingHistogramMinIntensity);
  typename HistogramType::MeasurementVectorType ubound;
  ubound.Fill(this->m_MovingHistogramMaxIntensity);
  m_MovingRankHistogram->Initialize(histSize, lbound, ubound);
  m_MovingRankHistogram->SetClipBinsAtEnds(m_MovingHistogramClipAtEnds);
}

template<class TFixedImage, class TMovingImage, class TRankImage>
void StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::SetRandomSeeds(SeedsType seeds)
{
  if (seeds.GetSize() != (int) ImageDimension)
    return;

  bool modified = false;
  for (unsigned int d = 0; d < seeds.GetSize(); d++)
  {
    if (m_RandomSeeds[d] != seeds[d])
    {
      m_RandomSeeds[d] = seeds[d];
      modified = true;
    }
  }
  if (modified)
    this->Modified();
}

template<class TFixedImage, class TMovingImage, class TRankImage>
void StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::ExtractMovingRankHistogram() const
{
  if (!m_MovingRankHistogram)
    return;
  HistogramPointer histogram = m_MovingRankHistogram;
  histogram->SetToZero();
  // first extract the pure histogram corresponding to fixed rank image!
  // (NOTE: However, we have to check moving image mask - if set!)
  FixedRankIteratorType fi(this->m_FixedRankImage, this->GetFixedImageRegion());
  RankImagePointer rankImage = m_FixedRankImage;
  typename Superclass::TransformType const *transform = this->m_Transform;
  typedef typename RankImageType::PixelType RankPixelType;
  RankPixelType r;
  typename RankImageType::IndexType index;
  typename Superclass::InputPointType inputPoint;
  typename Superclass::OutputPointType transformedPoint;
  typename HistogramType::MeasurementVectorType mv;
  MovingImageMaskPointer movingMask = this->m_MovingImageMask;
  fi.GoToBegin();
  while (!fi.IsAtEnd())
  {
    r = fi.Get();
    if (r > 0) // only fixed non-zero ranks are processed (others are masked)
    {
      index = fi.GetIndex();
      rankImage->TransformIndexToPhysicalPoint(index, inputPoint);

      // fixed image mask is inherently supported by mask

      // moving image mask support
      transformedPoint = transform->TransformPoint(inputPoint);
      if (movingMask && !movingMask->IsInside(transformedPoint))
      {
        ++fi;
        continue;
      }

      // interpolate and compute moving image value:
      if (this->m_Interpolator->IsInsideBuffer(transformedPoint))
      {
        // add to histogram:
        mv[0] = this->m_Interpolator->Evaluate(transformedPoint);
        histogram->IncreaseFrequency(mv, 1);
      }
    }
    ++fi;
  }

  // OK, now generate the average-rank map from cumulative histogram:
  typename HistogramType::FrequencyType icumf = 0;
  typename HistogramType::FrequencyType ifreq = 0;
  typename HistogramType::Iterator hit = histogram->Begin();
  typename HistogramType::Iterator hitend = histogram->End();
  m_HornCorrectionSums[1] = 0.;
  for (; hit != hitend; ++hit)
  {
    ifreq = hit.GetFrequency();

    // generate average rank:
    if (ifreq > 0)
    {
      hit.SetFrequency(static_cast<RankPixelType> (icumf + ifreq / 2.));
      icumf += ifreq;
    } // else: ignore 0-bins - they get no rank

    // generate horn correction sum:
    if (ifreq > 1)
    {
      m_HornCorrectionSums[1] += ifreq * (ifreq * ifreq - 1) / 12.;
    }
  }
}

template<class TFixedImage, class TMovingImage, class TRankImage>
typename StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::MeasureType StochasticRankCorrelationImageToImageMetric<
    TFixedImage, TMovingImage, TRankImage>::GetValue(
    const ParametersType &parameters) const
{
  this->SetTransformParameters(parameters);

  this->m_NumberOfPixelsCounted = 0;

  // extract moving rank histogram first:
  ExtractMovingRankHistogram();

  // having the rank histogram, we can map the interpolated moving image values
  // through this lookup table and compute the squared differences:
  FixedRankIteratorType fi(this->m_FixedRankImage, this->GetFixedImageRegion());
  RankImagePointer rankImage = m_FixedRankImage;
  typename Superclass::TransformType const *transform = this->m_Transform;
  typedef typename RankImageType::PixelType RankPixelType;
  RankPixelType r;
  typename RankImageType::IndexType index;
  typename Superclass::InputPointType inputPoint;
  typename Superclass::OutputPointType transformedPoint;
  MovingImageMaskPointer movingMask = this->m_MovingImageMask;
  fi.GoToBegin();
  double measure = 0.;
  double v, d;
  HistogramPointer histogram = m_MovingRankHistogram;
  typename HistogramType::MeasurementVectorType mv;

  // NOTE: The distinction between debugging and not debugging is relevant,
  // as sample distribution extraction may decrease performance.
  if (m_IsComputingDerivative || !m_ExtractSampleDistribution)
  {
    while (!fi.IsAtEnd())
    {
      r = fi.Get();
      if (r > 0) // only fixed non-zero ranks are processed (others are masked)
      {
        index = fi.GetIndex();
        rankImage->TransformIndexToPhysicalPoint(index, inputPoint);

        // fixed image mask and region inherently supported by rank image (mask)

        // moving image mask support
        transformedPoint = transform->TransformPoint(inputPoint);
        if (movingMask && !movingMask->IsInside(transformedPoint))
        {
          ++fi;
          continue;
        }

        // interpolate and compute moving image value:
        if (this->m_Interpolator->IsInsideBuffer(transformedPoint))
        {
          // compute current squared difference between fixed voxel rank and
          // moving voxel rank, finally sum it up:
          mv[0] = this->m_Interpolator->Evaluate(transformedPoint);
          v = histogram->GetFrequency(histogram->GetIndex(mv));
          if (m_MovingZeroRanksContributeToMeasure || v > 0)
          {
            d = r - v;
            measure += (d * d);
            this->m_NumberOfPixelsCounted++;
          }
        }
      }
      ++fi;
    }
  }
  else // DEBUGGING: extract sample distribution as well ...
  {
    unsigned int cc = 0; // counter for sample distribution entries
    while (!fi.IsAtEnd())
    {
      r = fi.Get();
      if (r > 0) // only fixed non-zero ranks are processed (others are masked)
      {
        index = fi.GetIndex();
        rankImage->TransformIndexToPhysicalPoint(index, inputPoint);

        // fixed image mask and region inherently supported by rank image (mask)

        // moving image mask support
        transformedPoint = transform->TransformPoint(inputPoint);
        if (movingMask && !movingMask->IsInside(transformedPoint))
        {
          m_SampleDistribution[cc].MovingIntensity = 0;
          m_SampleDistribution[cc].MovingRankIntensity = 0;
          ++fi;
          ++cc;
          continue;
        }

        // interpolate and compute moving image value:
        if (this->m_Interpolator->IsInsideBuffer(transformedPoint))
        {
          // compute current squared difference between fixed voxel rank and
          // moving voxel rank, finally sum it up:
          mv[0] = this->m_Interpolator->Evaluate(transformedPoint);
          m_SampleDistribution[cc].MovingIntensity
              = static_cast<double> (mv[0]);
          v = histogram->GetFrequency(histogram->GetIndex(mv));
          m_SampleDistribution[cc].MovingRankIntensity
              = static_cast<double> (v);
          if (m_MovingZeroRanksContributeToMeasure || v > 0)
          {
            d = r - v;
            measure += (d * d);
            this->m_NumberOfPixelsCounted++;
          }
        }
        else
        {
          m_SampleDistribution[cc].MovingIntensity = 0;
          m_SampleDistribution[cc].MovingRankIntensity = 0;
        }
        ++cc;
      }
      ++fi;
    }
  }

  // finally normalize the squared differences according to Pearson's r:
  if (this->m_NumberOfPixelsCounted > 0)
  {
    // NOTE: We take the number of pixels counted - not the initial number of
    // fixed image pixel samples. Due to transformations, a set of pixels is
    // potentially removed. We account for that by taking
    // m_NumberOfPixelsCounted as "N". We did some tests and saw that this
    // simple approach increases the capture range (especially for rotations).
    double n = static_cast<double> (this->m_NumberOfPixelsCounted);
    double n3_minus_n = (n * (n * n - 1));

    // minimization problem:
    if (!m_UseHornTiedRanksCorrection) // standard Spearman rank correlation
    {
      measure = measure * 6.0 / n3_minus_n;
    }
    else // Spearman rank correlation correct with Horn-method
    {
      n3_minus_n = (n * (n * n - 1));

      // NOTE: There is a potentially "dangerous" problem with
      // m_HornCorrectionSums[0]. As mentioned above,
      // m_NumberOfPixelsCounted is likely to be smaller than
      // m_NumberOfFixedSamplePixels (due to transformations or intensity-
      // cuts). However, m_HornCorrectionSums[0] relates to the frequencies
      // of the original fixed histogram. Therefore,
      // (N(N^2-1) - m_HornCorrectionSums[0]) can be less or equal to 0 in
      // worse cases. In order to avoid a 0 in the denominator or a negative
      // value under the square-root, we have to adapt m_HornCorrectionSums[0]
      // to the real counted number of pixels.
      // - the reduction factor (real pixels to estimated pixels)
      double f = (double)this->m_NumberOfPixelsCounted /
          (double)m_NumberOfFixedSamplePixels;
      // - approximate the fixed Horn correction sum by multiplying it by f^3
      //   (this is usually a quite good approximation):
      double fhornsum = (double)m_HornCorrectionSums[0] * f * f * f;
      // - however, the approximation is not perfect, be sure:
      if (fhornsum < 1)
        fhornsum = 1.;

      // in order to make the values a bit more readable, we rearrange
      // the formula slightly (multiply by 12/(n^3-n)):
      double n3_minus_n_inv = 12 / n3_minus_n;
      double csum1 = fhornsum * n3_minus_n_inv;
      double csum2 = m_HornCorrectionSums[1] * n3_minus_n_inv;
      double num = 2 - csum1 - csum2 - n3_minus_n_inv * measure;
      double sqrt1 = 1 - csum1;
      double sqrt2 = 1 - csum2;
      double denom = 2 * sqrt(sqrt1) * sqrt(sqrt2);
      // (as we implement a minimization criterion; 1 - ...):
      measure = 1 - num / denom;
    }
  }
  else // obviously, no overlap of relevant voxels!!!
  {
    if (m_NoOverlapReactionMode) // return specified metric value
    {
      measure = m_NoOverlapMetricValue;
    }
    else
    {
      itkExceptionMacro(<< "SRC-ERROR: Fixed and moving image do not overlap!")
    }
  }

  return measure;
}

template<class TFixedImage, class TMovingImage, class TRankImage>
void StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::SetTransform(TransformType *transform)
{
  if (transform && this->m_DerivativeScales.GetSize()
      != transform->GetNumberOfParameters())
  {
    this->m_DerivativeScales.SetSize(transform->GetNumberOfParameters());
    this->m_DerivativeScales.Fill(1.0);
  }
  Superclass::SetTransform(transform);
}

template<class TFixedImage, class TMovingImage, class TRankImage>
void StochasticRankCorrelationImageToImageMetric<TFixedImage, TMovingImage,
    TRankImage>::GetDerivative(const ParametersType &parameters,
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

#endif /* ORASTOCHASTICRANKCORRELATIONIMAGETOIMAGEMETRIC_TXX_ */
