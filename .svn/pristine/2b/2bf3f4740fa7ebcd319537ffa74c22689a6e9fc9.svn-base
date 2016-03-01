//
#ifndef _ORAAVERAGERANKSIMAGETOIMAGEFILTER_TXX_
#define _ORAAVERAGERANKSIMAGETOIMAGEFILTER_TXX_

#include "oraAverageRanksImageToImageFilter.h"

#include <itkImageRegionConstIterator.h>
#include <itkNumericTraits.h>

namespace ora
{

template<class TInputImage, class TOutputImage, class TMaskPixel>
AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::AverageRanksImageToImageFilter() :
  Superclass()
{
  m_NumberOfHistograms = 0;
  m_DoNotGenerateOutput = false; // default: generate output image
  m_MaskImage = NULL; // default: take ALL pixels for hist.-computation
  m_GenerateOutputForMaskedPixelsOnly = false; // default: whole output
  m_HistogramSum = 0;
  m_UseHistogramTransformation = false; // default: no tranformation
  m_Histograms.clear();
  m_RankHistogram = HistogramType::New();
  m_NumberOfHistogramBins = 256;
  m_HistogramMinIntensity
      = itk::NumericTraits<InputPixelType>::ZeroValue();
  m_HistogramMaxIntensity = itk::NumericTraits<InputPixelType>::max();
  m_HistogramClipAtEnds = false;
}

template<class TInputImage, class TOutputImage, class TMaskPixel>
AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::~AverageRanksImageToImageFilter()
{
  m_MaskImage = NULL;
  m_RankHistogram = NULL;
  for (unsigned int i = 0; i < m_NumberOfHistograms; i++)
    m_Histograms[i] = NULL;
  m_Histograms.clear();
}

template<class TInputImage, class TOutputImage, class TMaskPixel>
void AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << "Rank Histogram: size="
      << m_RankHistogram->GetSize() << "\n";
  os << indent << "Histograms: N=" << m_NumberOfHistograms << "\n";
  os << indent << "Do Not Generate Output: " << m_DoNotGenerateOutput
      << "\n";
  os << indent << "Round Counter: " << m_RoundCounter << "\n";
  os << indent << "Mask Image: " << m_MaskImage << "\n";
  os << indent << "Generate Output For Masked Pixels Only: "
      << m_GenerateOutputForMaskedPixelsOnly << "\n";
  os << indent << "Histogram Sum: " << m_HistogramSum << "\n";
  os << indent << "Number Of Histogram Bins: " << m_NumberOfHistogramBins
      << "\n";
  os << indent << "Histogram Min Intensity: " << m_HistogramMinIntensity
      << "\n";
  os << indent << "Histogram Max Intensity: " << m_HistogramMaxIntensity
      << "\n";
  os << indent << "Histogram Clip At Ends: " << m_HistogramClipAtEnds
        << "\n";
}

template<class TInputImage, class TOutputImage, class TMaskPixel>
void AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::BeforeThreadedGenerateData()
{
  // re-initialize global average-ranked histogram:
  HistogramType::SizeType histSize;
  histSize.Fill(m_NumberOfHistogramBins);
  HistogramType::MeasurementVectorType lbound;
  lbound.Fill(m_HistogramMinIntensity);
  HistogramType::MeasurementVectorType ubound;
  ubound.Fill(m_HistogramMaxIntensity);
  m_RankHistogram->Initialize(histSize, lbound, ubound);
  m_RankHistogram->SetClipBinsAtEnds(m_HistogramClipAtEnds);

  // generate and initialize the partial histograms:
  if (static_cast<int> (m_NumberOfHistograms)
      != this->GetNumberOfThreads())
  {
    if (m_NumberOfHistograms > 0)
    {
      for (unsigned int i = 0; i < m_NumberOfHistograms; i++)
        m_Histograms[i] = NULL;
      m_Histograms.clear();
    }
    m_NumberOfHistograms = this->GetNumberOfThreads();
    for (unsigned int i = 0; i < m_NumberOfHistograms; i++)
      m_Histograms.push_back(HistogramType::New());
  }
  m_HistogramSum = 0;

  for (unsigned int i = 0; i < m_NumberOfHistograms; i++)
  {
    m_Histograms[i]->Initialize(histSize, lbound, ubound);
    m_Histograms[i]->SetClipBinsAtEnds(m_HistogramClipAtEnds);
  }

  // indicate 'round 1': histogram-extraction & average-ranking
  m_RoundCounter = 0;
}

template<class TInputImage, class TOutputImage, class TMaskPixel>
void AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::SetMaskImage(MaskImagePointer mask)
{
  if (!this->GetInput())
    return;

  if (mask != m_MaskImage)
  {
    if (mask) // verify mask size!
    {
      typename MaskImageType::RegionType mreg;
      typename TInputImage::RegionType ireg;
      mreg = mask->GetLargestPossibleRegion();
      ireg = this->GetInput()->GetLargestPossibleRegion();
      for (unsigned int d = 0; d < InputImageDimension; d++)
      {
        if (mreg.GetSize()[d] != ireg.GetSize()[d])
          return;
      }
      m_MaskImage = mask;
      this->Modified();
    }
    else // NULL is OK
    {
      m_MaskImage = NULL;
      this->Modified();
    }
  }
}

template<class TInputImage, class TOutputImage, class TMaskPixel>
void AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::ThreadedGenerateData(
    const OutputImageRegionType& outputRegionForThread, int threadId)
{
  InputImageRegionConstIterator it(this->GetInput(), outputRegionForThread);
  it.GoToBegin();
  if (m_RoundCounter == 0) // 'round 1': hist.-extraction & avg-ranking
  {
    // extract the histogram
    HistogramPointer ihist = m_Histograms[threadId];
    HistogramType::MeasurementVectorType s;
    if (!m_MaskImage) // no masking
    {
      while (!it.IsAtEnd())
      {
        s[0] = it.Get();
        ihist->IncreaseFrequency(s, 1);
        ++it;
      }
    }
    else // masking
    {
      typedef itk::ImageRegionConstIterator<MaskImageType> MaskIteratorType;
      MaskIteratorType mit(m_MaskImage, outputRegionForThread);
      mit.GoToBegin();
      while (!mit.IsAtEnd())
      {
        if (mit.Get() != 0) // unmasked pixel -> contributes to histogram
        {
          s[0] = it.Get();
          ihist->IncreaseFrequency(s, 1);
        }
        ++it;
        ++mit;
      }
    }
  }
  else // 'round 2': output image generation
  {
    OutputImageRegionIterator oit(this->GetOutput(), outputRegionForThread);
    oit.GoToBegin();
    HistogramPointer hist = m_Histograms[threadId];
    // -> simply replace the original intensities with the average ranks:
    if (!m_GenerateOutputForMaskedPixelsOnly || !m_MaskImage)
    {
      HistogramType::MeasurementVectorType mv;
      while (!it.IsAtEnd())
      {
        mv[0] = static_cast<HistogramType::MeasurementType> (it.Get());
        // NOTE: internally, the rank histogram is already casted
        oit.Set(hist->GetFrequency(hist->GetIndex(mv)));
        ++oit;
        ++it;
      }
    }
    else // generate output only at unmasked positions
    {
      typedef itk::ImageRegionConstIterator<MaskImageType> MaskIteratorType;
      MaskIteratorType mit(m_MaskImage, outputRegionForThread);
      mit.GoToBegin();
      HistogramType::MeasurementVectorType mv;
      mit.GoToBegin();
      while (!it.IsAtEnd())
      {
        if (mit.Get() != 0) // unmasked
        {
          mv[0] = static_cast<HistogramType::MeasurementType> (it.Get());
          // NOTE: internally, the rank histogram is already casted
          oit.Set(hist->GetFrequency(hist->GetIndex(mv)));
        }
        else
        {
          oit.Set(0);
        }
        ++oit;
        ++it;
        ++mit;
      }
    }
  }
}

template<class TInputImage, class TOutputImage, class TMaskPixel>
void AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::AfterThreadedGenerateData()
{
  if (m_RoundCounter == 0)
  {
    MergePartialHistograms();
    TransformHistogram();
    GenerateAverageRanks();

    // indicate 'round 2': output image generation
    m_RoundCounter++;

    if (!m_DoNotGenerateOutput)
    {
      // initiate second round:
      this->GetMultiThreader()->SingleMethodExecute();
    }
  }
}

template<class TInputImage, class TOutputImage, class TMaskPixel>
typename AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::HistogramPointer AverageRanksImageToImageFilter<
    TInputImage, TOutputImage, TMaskPixel>::GetAveragedRankHistogram()
{
  return m_RankHistogram;
}

template<class TInputImage, class TOutputImage, class TMaskPixel>
void AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::MergePartialHistograms()
{
  // merge histogram information from all threads:
  m_HistogramSum = 0;
  for (unsigned int i = 0; i < m_NumberOfHistograms; i++)
  {
    HistogramType::Iterator hit = m_Histograms[i]->Begin();
    HistogramType::Iterator hitend = m_Histograms[i]->End();

    for (; hit != hitend; ++hit)
      m_RankHistogram->IncreaseFrequency(hit.GetMeasurementVector(),
          hit.GetFrequency());
  }
  // store total number of frequencies (number of sampled pixels)
  m_HistogramSum = m_RankHistogram->GetTotalFrequency();

  this->Superclass::InvokeEvent(ora::AfterHistogramExtraction());
}

template<class TInputImage, class TOutputImage, class TMaskPixel>
void AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::TransformHistogram()
{
  if (m_MaskImage && m_UseHistogramTransformation)
  {
    double numHistPixels = m_HistogramSum;
    double numTotalPixels =
        m_MaskImage->GetLargestPossibleRegion(). GetNumberOfPixels();
    double f = numTotalPixels / numHistPixels; // lin. transformation factor
    HistogramType::Iterator hit = m_RankHistogram->Begin();
    HistogramType::Iterator hitend = m_RankHistogram->End();
    HistogramType::FrequencyType freq;
    for (; hit != hitend; ++hit)
    {
      freq = hit.GetFrequency();
      freq = static_cast<HistogramType::FrequencyType> (f * freq);
      hit.SetFrequency(freq);
    }
    this->Superclass::InvokeEvent(ora::AfterHistogramTransformation());
  } // else: no need for histogram transformation
}

template<class TInputImage, class TOutputImage, class TMaskPixel>
void AverageRanksImageToImageFilter<TInputImage, TOutputImage,
    TMaskPixel>::GenerateAverageRanks()
{
  // INFO:
  // We can compute the average rank of each grayscale level directly from
  // the histogram by simply taking the frequencies of each bin into account.
  HistogramType::FrequencyType icumf = 0;
  HistogramType::FrequencyType ifreq = 0;
  HistogramType::Iterator hit = m_RankHistogram->Begin();
  HistogramType::Iterator hitend = m_RankHistogram->End();
  for (; hit != hitend; ++hit)
  {
    ifreq = hit.GetFrequency();
    if (ifreq > 0)
    {
      hit.SetFrequency(static_cast<OutputPixelType> (icumf + ifreq / 2.));
      icumf += ifreq;
    } // else: ignore 0-bins - they get no rank
  }

  if (!m_DoNotGenerateOutput)
  {
    // NOTE: Accessing (even if read-only) GetFrequency() and GetIndex() of rank
    // histogram from different threads simultaneously causes bad output pixels.
    // Therefore, generate a rank-histogram copy for each thread.
    for (unsigned int i = 0; i < m_NumberOfHistograms; i++)
    {
      HistogramType::Iterator hit = m_Histograms[i]->Begin();
      HistogramType::Iterator hitend = m_Histograms[i]->End();
      for (; hit != hitend; ++hit)
      {
        hit.SetFrequency(m_RankHistogram->GetFrequency(
            hit.GetInstanceIdentifier()));
      }
    }
  }

  this->Superclass::InvokeEvent(ora::AfterHistogramRankGeneration());
}

}

#endif /* _ORAAVERAGERANKSIMAGETOIMAGEFILTER_TXX_ */
