//
#ifndef ORAAVERAGERANKSIMAGETOIMAGEFILTER_H_
#define ORAAVERAGERANKSIMAGETOIMAGEFILTER_H_

#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkConceptChecking.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <itkSimpleFastMutexLock.h>
#include <itkEventObject.h>
#include <itkHistogram.h>

namespace ora
{

/** Event is fired right after histogram extraction. **/
itkEventMacro(AfterHistogramExtraction, itk::AnyEvent)
/**
 * Event is fired right after histogram transformation (but if and only if
 * a mask image is set and UseHistogramTransformation is activated).
 */
itkEventMacro(AfterHistogramTransformation, itk::AnyEvent)
/**
 * Event is fired right after average rank generation from the cumulative
 * histogram. The histogram then consists of averaged ranks.
 */
itkEventMacro(AfterHistogramRankGeneration, itk::AnyEvent)

/** \class AverageRanksImageToImageFilter
 * \brief Computes the average-ranked image from a source image.
 *
 * Computes the average-ranked image (or the according averaged rank map only)
 * from a specified source image.
 * For this purpose the intensity histogram from the source image is extracted
 * and the according average ranks are determined. This is achieved by finding
 * the cumulative mean rank of each intensity-class.
 *
 * This filter is multi-threaded.
 *
 * This filter is templated over the input image type (TInputImage),
 * the output image type (TOutputImage), and the mask pixel type
 * (TMaskPixel). The input image dimension must equal the output image
 * dimension. Furthermore the input image is requested to have a pixel type
 * with numeric traits. Note that the internal histogram is extracted from the
 * input image using a user-specified number of bins and minimum and maximum
 * bounds. The output pixel type must be large enough to cover the rank range
 * (i.e. the number of pixels found in the input image in worst case).
 *
 * The output image is generated optionally (flag: DoNotGenerateOutput) as it
 * can make sense to retrieve the averaged rank map (i.e. the lookup-table for
 * transforming input intensities to output intensities) only.
 *
 * Furthermore a mask can be specified that explicitly marks the pixels that
 * should be used for histogram estimation. Depending on the
 * GenerateOutputForMaskedPixelsOnly-flag, the average ranks are applied to ALL
 * pixels of the input image or just to the masked pixels in order to generate
 * the output image.
 *
 * Events:<br>
 * - <b>AfterHistogramExtraction</b>: fired right after histogram extraction;
 * the extracted histogram is accessible through GetAveragedRankHistogram()<br>
 * - <b>AfterHistogramTransformation</b>: fired right after histogram
 * transformation (but if and only if a mask image is set and
 * UseHistogramTransformation is activated)<br>
 * - <b>AfterHistogramRankGeneration</b>: fired right after average rank
 * generation from the cumulative histogram. The histogram then consists of
 * averaged ranks <br>
 *
 * <b>Tests</b>:<br>
 * TestAverageRanksFilter.cxx
 *
 * @author phil 
 * @version 1.4
 *
 * \ingroup ImageFilters
 */
template<class TInputImage, class TOutputImage,
    class TMaskPixel = unsigned char>
class AverageRanksImageToImageFilter:
    public itk::ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef AverageRanksImageToImageFilter Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputImage> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Accessibility typedefs. **/
  typedef typename TInputImage::PixelType InputPixelType;
  typedef typename TOutputImage::PixelType OutputPixelType;
  typedef typename TOutputImage::RegionType OutputImageRegionType;
  typedef itk::Statistics::Histogram<double> HistogramType; // 1D histogram
  typedef HistogramType::Pointer HistogramPointer;

  /** ImageDimension enumeration */
  itkStaticConstMacro(InputImageDimension, unsigned int,
      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
      TOutputImage::ImageDimension);

  /** Masking **/
  typedef TMaskPixel MaskPixelType;
  typedef itk::Image<MaskPixelType, InputImageDimension> MaskImageType;
  typedef typename MaskImageType::Pointer MaskImagePointer;
  typedef typename MaskImageType::ConstPointer MaskImageConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(Self, Superclass)
  ;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)
  ;

  /** @return pointer to the internally generated averaged rank histogram **/
  virtual HistogramPointer GetAveragedRankHistogram();

  /**
   * Set flag indicating that output should not be generated although this is an
   * image to image filter. The reason is that it can make sense to generate the
   * rank map alone without generating the output image itself.
   **/
  itkSetMacro(DoNotGenerateOutput, bool)
  /**
   * Get flag indicating that output should not be generated although this is an
   * image to image filter.
   **/
  itkGetMacro(DoNotGenerateOutput, bool)

  /**
   * Set image mask that explicitly marks the pixels that should contribute to
   * histogram estimation. Depending on the GenerateOutputForUnmaskedPixelsOnly-
   * flag, the generated output image comprises ALL pixels or just the masked
   * pixels of the input image. However, the output ranks information is
   * generated with the information from the mask image only. If mask
   * is NULL, all pixels are taken for histogram computation. <br>
   * NOTE: the mask must have the same size (largest possible region) as the
   * input image.
   **/
  virtual void SetMaskImage(MaskImagePointer mask);
  /**
   * Get image mask that explicitly marks the pixels that should contribute to
   * histogram estimation.
   **/
  itkGetObjectMacro(MaskImage, MaskImageType)

  /**
   * Set flag indicating whether the whole image or just the masked fractions
   * of it should be output. This is solely interesting if a mask image is set.
   * The rest of pixels is consistently set to zero-rank.
   **/
  itkSetMacro(GenerateOutputForMaskedPixelsOnly, bool)
  /**
   * Get flag indicating whether the whole image or just the unmasked fractions
   * of it should be output.
   **/
  itkGetMacro(GenerateOutputForMaskedPixelsOnly, bool)

  /**
   * Set flag indicating whether histogram transformation (if masks are used)
   * should be used in order to back-transform the sampled histogram to its
   * estimated original rank-range.
   */
  itkSetMacro(UseHistogramTransformation, bool)
  /**
   * Get flag indicating whether histogram transformation (if masks are used)
   * should be used in order to back-transform the sampled histogram to its
   * estimated original rank-range.
   */
  itkGetMacro(UseHistogramTransformation, bool)

  /** Set number of histogram bins for histogram extraction. **/
  itkSetMacro(NumberOfHistogramBins, unsigned int)
  /** Get number of histogram bins for histogram extraction. **/
  itkGetMacro(NumberOfHistogramBins, unsigned int)
  /** Set minimum intensity for histogram extraction. **/
  itkSetMacro(HistogramMinIntensity, InputPixelType)
  /** Get minimum intensity for histogram extraction. **/
  itkGetMacro(HistogramMinIntensity, InputPixelType)
  /** Set maximum intensity for histogram extraction. **/
  itkSetMacro(HistogramMaxIntensity, InputPixelType)
  /** Get maximum intensity for histogram extraction. **/
  itkGetMacro(HistogramMaxIntensity, InputPixelType)
  /** Set flag indicating that intensities at the ends should be clipped. **/
  itkSetMacro(HistogramClipAtEnds, bool)
  /** Get flag indicating that intensities at the ends should be clipped. **/
  itkGetMacro(HistogramClipAtEnds, bool)
  itkBooleanMacro(HistogramClipAtEnds)

  /** Concept checking */
  /** input image type must have numeric pixel type **/
  itkConceptMacro(InputHasNumericTraitsCheck,
      (itk::Concept::HasNumericTraits<InputPixelType>));
  /** input and output image must share the same dimension **/
    itkConceptMacro(ImageDimensionCheck,
        (itk::Concept::SameDimension<itkGetStaticConstMacro(InputImageDimension),
            itkGetStaticConstMacro(OutputImageDimension)>));

protected:
  /** Iterator type for input image. **/
  typedef itk::ImageRegionConstIterator<TInputImage>
      InputImageRegionConstIterator;
  /** Iterator type for output image. **/
  typedef itk::ImageRegionIterator<TOutputImage> OutputImageRegionIterator;

  /** Internal histogram map (averaged rank histogram at the end). **/
  HistogramPointer m_RankHistogram;
  /** Temporary internal histograms (one for each thread). **/
  std::vector<HistogramPointer> m_Histograms;
  /** Number of internal temporary histograms. **/
  unsigned int m_NumberOfHistograms;
  /**
   * Flag indicating that output should not be generated although this is an
   * image to image filter. The reason is that it can make sense to generate the
   * rank map alone without generating the output image itself.
   **/
  bool m_DoNotGenerateOutput;
  /** Internal help counter counting the number of 'rounds' already passed. **/
  bool m_RoundCounter;
  /**
   * Image mask that explicitly marks the pixels that should contribute to
   * histogram estimation. The generated output image however comprises ALL
   * pixels, but generated with the information from the mask image. If this
   * member is NULL, all pixels are taken for histogram computation.
   **/
  MaskImagePointer m_MaskImage;
  /**
   * Flag indicating whether the whole image or just the masked fractions of
   * it should be output. This is solely interesting if a mask image is set.
   **/
  bool m_GenerateOutputForMaskedPixelsOnly;
  /** Internal helper for storing the total sum of extracted frequencies. **/
  double m_HistogramSum;
  /**
   * Flag indicating whether histogram transformation (if masks are used) should
   * be used in order to back-transform the sampled histogram to its original
   * rank-range.
   */
  bool m_UseHistogramTransformation;
  /** Number of histogram bins for histogram extraction. **/
  unsigned int m_NumberOfHistogramBins;
  /** Minimum intensity for histogram extraction. **/
  InputPixelType m_HistogramMinIntensity;
  /** Maximum intensity for histogram extraction. **/
  InputPixelType m_HistogramMaxIntensity;
  /** Flag indicating that intensities at the ends should be clipped. **/
  bool m_HistogramClipAtEnds;

  /** Default constructor. **/
  AverageRanksImageToImageFilter();
  /** Default destructor. **/
  virtual ~AverageRanksImageToImageFilter();

  /** Print description of this object. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /** Entry point before threaded execution. **/
  virtual void BeforeThreadedGenerateData();

  /** Threaded implementation of histogram ranking. **/
  virtual void ThreadedGenerateData(
      const OutputImageRegionType& outputRegionForThread, int threadId);

  /** Entry point after threaded execution. **/
  virtual void AfterThreadedGenerateData();

  /** Merge the partial histograms generated by the single threads. **/
  virtual void MergePartialHistograms();

  /**
   * In cases where a mask image was defined, this is important as the number
   * of effectively sampled pixels should be transformed to the original number
   * of pixels given by mask size in order to generate ranks later that are
   * comparable to the real ranks!
   **/
  virtual void TransformHistogram();

  /**
   * Take the histogram and replace the frequencies with the resultant average
   * ranks. Internally the histogram is sorted from lower to higher key values
   * (weak ordering) due to std::map implementation. The average ranks are
   * generated in this order w.r.t. the frequencies found within the bins.
   **/
  virtual void GenerateAverageRanks();

private:
  /** Purposely not implemented. **/
  AverageRanksImageToImageFilter(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraAverageRanksImageToImageFilter.txx"

#endif /* ORAAVERAGERANKSIMAGETOIMAGEFILTER_H_ */
