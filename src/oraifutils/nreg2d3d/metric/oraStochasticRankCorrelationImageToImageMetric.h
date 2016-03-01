//
#ifndef ORASTOCHASTICRANKCORRELATIONIMAGETOIMAGEMETRIC_H_
#define ORASTOCHASTICRANKCORRELATIONIMAGETOIMAGEMETRIC_H_

#include <itkImageToImageMetric.h>
#include <itkArray.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkResampleImageFilter.h>
#include <itkEventObject.h>
#include <itkConceptChecking.h>

#include "oraAverageRanksImageToImageFilter.h"

namespace ora
{

/**
 * Helper struct for DEBUGGING.
 * @see StochasticRankCorrelationImageToImageMetric#SetExtractSampleDistribution()
 **/
typedef struct
{
  double FixedIntensity;
  double FixedRankIntensity;
  double MovingIntensity;
  double MovingRankIntensity;
} SampleDistributionEntry;

/**
 * Event is fired right after stochastic mask creation (according to configured
 * sample coverage) or after check of user-specified stochastic mask.
 **/
itkEventMacro(AfterMaskCreation, itk::AnyEvent)

/** \class StochasticRankCorrelationImageToImageMetric
 * \brief An implementation of Stochastic Rank Correlation (SRC) metric.
 *
 * An ITK-based implementation of Stochastic Rank Correlation (SRC) metric for
 * image registration as proposed by Birkfellner et al. <br><br>
 * <i>Birkfellner W, Stock M, Figl M, Gendrin C, Hummel J, Dong S, Kettenbach J,
 * Georg D, Bergmann H.: Stochastic rank correlation: a robust merit function
 * for 2D/3D registration of image data obtained at different energies.
 * Med Phys. 2009; 36(8): 3420-3428</i>.
 *
 * In contrast to cross-correlation (Pearson's product moment correlation
 * coefficient; linear relationship), this non-parametric metric assesses the
 * statistical relationship between 2 variables using a monotonic function. It
 * is the implementation of Spearman's rank correlation coefficient. In order to
 * map the source images' intensities onto an ordinal scale, the intensity
 * histograms are extracted. Subsequently, an average rank (derived from the
 * cumulative histogram) is assigned to each bin, yielding an intensity (metric
 * scale) to rank (ordinal scale) lookup table. The word "stochastic" means that
 * only a subset of the image pixels is considered for histogram creation. Once
 * the lookup tables for the moving and the fixed image are determined, the
 * metric value M_SRC (rank correlation coefficient) is computed as follows:<br>
 * M_SRC = 6 * sum_1_to_N ( delta_I_N^2 ) / ( N * (N^2 - 1) ). <br>
 * N is the number of sampled pixels (defined on the fixed image grid) and
 * delta_I_N^2 denote the square differences of mapped fixed image pixel ranks
 * and moving image ranks.
 *
 * As can be seen from the formula above, this metric yields a MINIMIZATION
 * problem!
 *
 * Birkfellner et al. reported 5 % of the pixels of the fixed image (randomly
 * sampled) as a sufficient subset for 2D/3D registration of a spine reference
 * data set. The 'sample coverage' may however depend on the concrete field of
 * application.
 *
 * NOTE: In contrast to the approach presented by Birkfellner et al., this
 * implementation does not generate ranks by sampling and sorting the discrete
 * pixel intensities. It uses the generic ITK histogram implementation with
 * configurable minimum / maximum intensities and number of bins, and then
 * generates a simple 1D-histogram. The ranks are inherently derived by
 * generating the cumulative histogram "on the fly", and calculating the
 * "average rank" for each bin. Therefore, this approach works on input images
 * with an arbitrary (not necessarily ordinal) pixel type.
 *
 * Furthermore, this metric supports a fixed image region, and a fixed as well
 * as a moving image mask for measure evaluation.
 *
 * The stochastic factor (sample coverage) is either defined as percentage or as
 * user-specified mask input image (pixels > 0 define pixels to be considered).
 *
 * The cost function derivative is computed using finite distances (step-by-step
 * transform parameter perturbation). The derivative step length scales can be
 * set individually. This may for example be important to distinguish between
 * rotational and translational parameters.
 *
 * This class is templated over the fixed image type (TFixedImage),
 * the moving image type (TMovingImage), and the rank image type
 * (TRankImage). The input image dimension must equal the output image
 * dimension. The rank image pixel type must be large enough to cover the rank
 * range (i.e. the number of pixels found in the fixed / moving image in worst
 * case).
 *
 * NOTE: Changing fixed / moving histogram properties (minimum intensity,
 * maximum intensity, number of bins), the fixed image mask or the fixed image
 * region will show an effect AFTER calling Initialize()! Modifying one or more
 * of these properties without calling Initialize() won't have any impact!
 *
 * NOTE: Each call to Initialize() will internally generate a stochastic mask if
 * sample coverage (percentage) is used, or check (and potentially correct) the
 * user-specified input mask against fixed image region and mask otherwise. This
 * internal stochastic mask, which defines the sample pixels, can only be
 * retrieved during the AfterMaskCreation()-event using the GetStochasticMask()-
 * method.
 *
 * NOTE: Ranking of the fixed image during Initialize() is achieved by utilizing
 * a multi-threaded convenience filter (ora::AverageRanksImageToImageFilter).
 *
 * NOTE: At the moment, this metric generates a 'rank image representation' of
 * the fixed image which has the same number of pixels as the fixed image. This
 * image is kept until the object is destroyed or Initialize() is called once
 * again. Be aware of this additional memory requirement (may be relevant for
 * image registration with more than 2 dimensions)!
 *
 * NOTE: In order to correct for tied ranks, "Horn"-correction can be activated
 * (UseHornTiedRanksCorrection-flag).<br>
 * "D. Horn, A correction for the effect of tied ranks on the value of
 *   the rank difference correlation coefficient, Journal of Educational
 *   Psychology, Volume 33, Issue 9, December 1942, Pages 686-690."
 *
 * NOTE: Since a minimum/maximum intensity for histogram extraction (moving
 * AND fixed image) can be specified, some "mapped" (transformed and
 * interpolated) moving image pixel can effectively be assigned a 0-rank (e.g.
 * because they're outside the specified intensity range). The
 * MovingZeroRanksContributeToMeasure-flag controls whether these 0-ranks are
 * considered during computation of SRC coefficient (metric measure) or not.
 *
 * <b>Events:</b><br>
 * - <b>AfterMaskCreation</b>: fired right after stochastic mask creation
 * (according to configured sample coverage) or after check of user-specified
 * stochastic mask.<br>
 *
 * HINT: Have a look at the referenced tests in order to get a quick overview of
 * this metric and on how to use it!
 *
 * <b>Tests</b>:<br>
 * TestStochasticRankCorrelationMetric.cxx
 * TestAverageRanksFilter.cxx
 *
 * @see ora::AverageRanksImageToImageFilter
 *
 * @author phil 
 * @version 1.4
 *
 * \ingroup RegistrationMetrics
 */
template<class TFixedImage, class TMovingImage, class TRankImage>
class StochasticRankCorrelationImageToImageMetric:
    public itk::ImageToImageMetric<TFixedImage, TMovingImage>
{
public:
  /** Standard class typedefs. */
  typedef StochasticRankCorrelationImageToImageMetric Self;
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Accessibility typedefs. **/
  typedef TFixedImage FixedImageType;
  typedef typename FixedImageType::PixelType FixedPixelType;
  typedef typename FixedImageType::Pointer FixedImagePointer;

  typedef typename FixedImageType::ConstPointer FixedImageConstPointer;
  typedef TMovingImage MovingImageType;
  typedef typename MovingImageType::PixelType MovingPixelType;
  typedef typename MovingImageType::Pointer MovingImagePointer;
  typedef typename MovingImageType::ConstPointer MovingImageConstPointer;
  typedef TRankImage RankImageType;
  typedef typename RankImageType::PixelType RankPixelType;
  typedef typename RankImageType::Pointer RankImagePointer;
  typedef unsigned char InternalMaskPixelType;
#ifdef ITK_USE_OPTIMIZED_REGISTRATION_METHODS
  typedef typename Superclass::FixedImageMaskConstPointer FixedImageMaskPointer;
  typedef typename Superclass::MovingImageMaskConstPointer MovingImageMaskPointer;
#else
#ifdef ITK_LEGACY_REMOVE
  typedef typename Superclass::FixedImageMaskConstPointer FixedImageMaskPointer;
  typedef typename Superclass::MovingImageMaskConstPointer MovingImageMaskPointer;
#else
  typedef typename Superclass::FixedImageMaskPointer FixedImageMaskPointer;
  typedef typename Superclass::MovingImageMaskPointer MovingImageMaskPointer;
#endif // ITK_LEGACY_REMOVE
#endif // ITK_USE_OPTIMIZED_REGISTRATION_METHODS

  /** ImageDimension */
  itkStaticConstMacro(ImageDimension, unsigned int,
      FixedImageType::ImageDimension);
  itkStaticConstMacro(FixedImageDimension, unsigned int,
      FixedImageType::ImageDimension);
  itkStaticConstMacro(MovingImageDimension, unsigned int,
      MovingImageType::ImageDimension);
  itkStaticConstMacro(RankImageDimension, unsigned int,
      RankImageType::ImageDimension);

  /** Inherited types. **/
  typedef typename Superclass::Pointer SuperclassPointer;
  typedef typename Superclass::ParametersType ParametersType;
  typedef typename Superclass::MeasureType MeasureType;
  typedef typename Superclass::DerivativeType DerivativeType;
  typedef typename Superclass::TransformType TransformType;

  /** Sampling types. **/
  typedef unsigned long SeedValueType;
  typedef itk::Array<SeedValueType> SeedsType;

  /** Scales type. */
  typedef itk::Array<double> ScalesType;

  /** Average ranking image filter **/
  typedef AverageRanksImageToImageFilter<FixedImageType, RankImageType,
    InternalMaskPixelType> RankFilterType;
  typedef typename RankFilterType::Pointer RankFilterPointer;
  typedef InternalMaskPixelType MaskPixelType;
  typedef typename RankFilterType::MaskImageType MaskImageType;
  typedef typename RankFilterType::MaskImagePointer MaskImagePointer;
  typedef typename RankFilterType::MaskImageConstPointer MaskImageConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(StochasticRankCorrelationImageToImageMetric, ImageToImageMetric)

  /** Method for creation through the object factory. */
  itkNewMacro(Self)

  /**
   * Get pointer to (already stochastically sampled) fixed average-ranked image.
   * NOTE: all pixels of the image that are different from zero-rank contribute
   * to cost function evaluation! This image is available and actual after a
   * call to Initialize();
   * @see Initialize()
   **/
  itkGetObjectMacro(FixedRankImage, RankImageType)

  /**
   * Set stochastic factor: how many samples to extract from the image domain?
   * Range: ]0;100] (%). NOTE: The word image domain here relates to the image
   * considering fixed image region and fixed image mask (if set). NOTE: If
   * StochasticUserMask is supplied, this attribute will be ignored.
   **/
  itkSetClampMacro(SampleCoverage, double, 0.00001, 100.0)
  /** Get stochastic factor. **/
  itkGetMacro(SampleCoverage, double)

  /**
   * Get stochastic mask that was generated according to SampleCoverage
   * property. OR, if user-specified stochastic mask is set, this method will
   * return a reference to this mask (eventually modified according to fixed
   * image region and fixed image mask). This property can also return NULL: if
   * the fixed image region is same size as fixed image's largest possible
   * region, the fixed image mask is not set, no user-specified mask is set and
   * the sample coverage is 100 %. NOTE: This mask is only reliably
   * available during AfterMaskCreation() event!
   **/
  itkGetConstObjectMacro(StochasticMask, MaskImageType)

  /**
   * Set optional user-supplied stochastic mask. If supplied, SampleCoverage
   * will be ignored and the pixels having a value >0 will be selected for metric
   * evaluation. NOTE: This mask is required to have the same size as the fixed
   * image (largest possible region). NOTE: If a fixed image mask and/or fixed
   * image region are also set, only the pixels of the stochastic mask within
   * the fixed image mask and/or fixed image region will contribute to metric
   * evaluation. This will potentially MODIFY this set mask image!
   */
  virtual void SetStochasticUserMask(MaskImageType *mask);
  /** Get optional user-supplied stochastic mask. **/
  itkGetObjectMacro(StochasticUserMask, MaskImageType)

  /**
   * Set the derivative step length scales for each parameter dimension.
   * Internally the derivative is computed by using finite distances. The
   * finite distances are specified by these scales. <br>
   * NOTE: the scales are overridden with a 1-vector in SetTransform() if the
   * scales vector length does not match the number of transform parameters!
   */
  itkSetMacro(DerivativeScales, ScalesType)
  /** Get the derivative step length scales for each parameter dimension. **/
  itkGetConstReferenceMacro(DerivativeScales, ScalesType)
  ;

  /**
   * Set seeds for deterministic behavior of stochastic sampling. Size must
   * equal the number of image dimensions. If seed value is 0 for a dimension,
   * the sampling behavior is non-deterministic (random).
   * @see SetSampleCoverage()
   * @see GenerateStochasticMaskImage()
   * @see Initialize()
   **/
  virtual void SetRandomSeeds(SeedsType seeds);
  /** Get seeds for deterministic behavior of stochastic sampling. **/
  itkGetMacro(RandomSeeds, SeedsType)

  /** Set number of histogram bins for histogram extraction of fixed image. **/
  itkSetMacro(FixedNumberOfHistogramBins, unsigned int)
  /** Get number of histogram bins for histogram extraction of fixed image. **/
  itkGetMacro(FixedNumberOfHistogramBins, unsigned int)
  /** Set minimum intensity for histogram extraction of fixed image. **/
  itkSetMacro(FixedHistogramMinIntensity, FixedPixelType)
  /** Get minimum intensity for histogram extraction of fixed image. **/
  itkGetMacro(FixedHistogramMinIntensity, FixedPixelType)
  /** Set maximum intensity for histogram extraction of fixed image. **/
  itkSetMacro(FixedHistogramMaxIntensity, FixedPixelType)
  /** Get maximum intensity for histogram extraction of fixed image. **/
  itkGetMacro(FixedHistogramMaxIntensity, FixedPixelType)
  /** Set flag indicating that intensities at the ends should be clipped. **/
  itkSetMacro(FixedHistogramClipAtEnds, bool)
  /** Get flag indicating that intensities at the ends should be clipped. **/
  itkGetMacro(FixedHistogramClipAtEnds, bool)
  itkBooleanMacro(FixedHistogramClipAtEnds)

  /** Set number of histogram bins for histogram extraction of moving image. **/
  itkSetMacro(MovingNumberOfHistogramBins, unsigned int)
  /** Get number of histogram bins for histogram extraction of moving image. **/
  itkGetMacro(MovingNumberOfHistogramBins, unsigned int)
  /** Set minimum intensity for histogram extraction of moving image. **/
  itkSetMacro(MovingHistogramMinIntensity, MovingPixelType)
  /** Get minimum intensity for histogram extraction of moving image. **/
  itkGetMacro(MovingHistogramMinIntensity, MovingPixelType)
  /** Set maximum intensity for histogram extraction of moving image. **/
  itkSetMacro(MovingHistogramMaxIntensity, MovingPixelType)
  /** Get maximum intensity for histogram extraction of moving image. **/
  itkGetMacro(MovingHistogramMaxIntensity, MovingPixelType)
  /** Set flag indicating that intensities at the ends should be clipped. **/
  itkSetMacro(MovingHistogramClipAtEnds, bool)
  /** Get flag indicating that intensities at the ends should be clipped. **/
  itkGetMacro(MovingHistogramClipAtEnds, bool)
  itkBooleanMacro(MovingHistogramClipAtEnds)

  /**
   * Set flag.
   * Determine what should happen if moving and fixed image do not overlap for
   * a given transformation?<br>
   * 0 ... throw an exception<br>
   * 1 ... return the configured NoOverlapMetricValue<br>
   * DEFAULT: 0.
   **/
  itkSetMacro(NoOverlapReactionMode, int)
  /**
   * Get flag.
   * Determine what should happen if moving and fixed image do not overlap for
   * a given transformation?<br>
   * 0 ... throw an exception<br>
   * 1 ... return the configured NoOverlapMetricValue<br>
   * DEFAULT: 0.
   **/
  itkGetMacro(NoOverlapReactionMode, int)
  /**
   * Set value that is returned if the moving and fixed image do not overlap for
   * a given transformation and m_NoOverlapReactionMode==1.
   **/
  itkSetMacro(NoOverlapMetricValue, MeasureType)
  /**
   * Get value that is returned if the moving and fixed image do not overlap for
   * a given transformation and m_NoOverlapReactionMode==1.
   **/
  itkGetMacro(NoOverlapMetricValue, MeasureType)

  /** Set the transform and adapt the derivative scales if necessary. **/
  virtual void SetTransform(TransformType *transform);

    /**
   * Set "use Horn correction"-flag for tied ranks.
   * @see "D. Horn, A correction for the effect of tied ranks on the value of
   *   the rank difference correlation coefficient, Journal of Educational
   *   Psychology, Volume 33, Issue 9, December 1942, Pages 686-690."
   **/
  itkSetMacro(UseHornTiedRanksCorrection, bool)
  /** Get "use Horn correction"-flag for tied ranks. **/
  itkGetMacro(UseHornTiedRanksCorrection, bool)
  itkBooleanMacro(UseHornTiedRanksCorrection)

  /**
   * Set flag indicating whether pixels of the moving image that gain a 0-rank
   * (e.g. because they're out of histogram min/max-intensity) should contribute
   * to SRC coefficient or not (default: TRUE).
   */
  itkSetMacro(MovingZeroRanksContributeToMeasure, bool)
  /**
   * Get flag indicating whether pixels of the moving image that gain a 0-rank
   * (e.g. because they're out of histogram min/max-intensity) should contribute
   * to SRC coefficient or not (default: TRUE).
   */
  itkGetMacro(MovingZeroRanksContributeToMeasure, bool)
  itkBooleanMacro(MovingZeroRanksContributeToMeasure)

  /**
   * DEBUGGING: Use it really only for debugging! If TRUE, the sampled fixed
   * and moving image and rank image intensities are internally cached.
   * @see GetSampleDistribution()
   */
  virtual void SetExtractSampleDistribution(bool value)
  {
    m_SampleDistribution.clear();
    if (m_ExtractSampleDistribution != value)
    {
      m_ExtractSampleDistribution = value;
      this->Modified();
    }
  }
  itkGetMacro(ExtractSampleDistribution, bool)

  /**
   * Initialize the metric. The fixed image is stochastically sampled according
   * to configuration (sample coverage - NOTE: this can be a bit problematic
   * if the fixed image mask masks a lot of pixels and coverage is around 50) or
   * the user-specified stochastic input mask is checked against fixed image
   * region and fixed image mask.
   * Based on the sampled image information, its histogram is average-ranked.
   * From this point in time, this image is the internal reference image which
   * is used for cost function evaluation.
   **/
  virtual void Initialize() throw (itk::ExceptionObject);

  /**
   * This method returns the value of the stochastic rank correlation cost
   * function corresponding to the specified transformation parameters.
   * @see itk::SingleValuedCostFunction#GetValue()
   **/
  virtual MeasureType GetValue(const ParametersType &parameters) const;

  /**
   * This method returns the derivative of the stochastic rank correlation cost
   * function corresponding to the specified transformation parameters.
   * Derivative estimation is based on finite distances.
   * @see itk::SingleValuedCostFunction#GetDerivative()
   **/
  virtual void GetDerivative(const ParametersType &parameters,
      DerivativeType &derivative) const;

  /**
   * DEBUGGING: Contains the extracted sample distribution after each call
   * to GetValue() - BUT only if ExtractSampleDistribution is set to TRUE.
   * @see SetExtractSampleDistribution()
   */
  std::vector<SampleDistributionEntry> * GetSampleDistribution()
  {
    return &m_SampleDistribution;
  }

  /** Concept checking */
  /** fixed image type must have numeric pixel type **/
  itkConceptMacro(FixedImagePixelTypeHasNumericTraitsCheck,
      (itk::Concept::HasNumericTraits<FixedPixelType>));
  /** moving image type must have numeric pixel type **/
  itkConceptMacro(MovingImagePixelTypeHasNumericTraitsCheck,
      (itk::Concept::HasNumericTraits<MovingPixelType>));
  /** fixed and moving image must share the same dimension **/
  itkConceptMacro(FixedMovingDimensionCheck,
      (itk::Concept::SameDimension<itkGetStaticConstMacro(FixedImageDimension),
          itkGetStaticConstMacro(MovingImageDimension)>));
  /** rank image type must have numeric pixel type **/
  itkConceptMacro(RankImagePixelTypeHasNumericTraitsCheck,
        (itk::Concept::HasNumericTraits<RankPixelType>));
  /** fixed and rank image must share the same dimension **/
  itkConceptMacro(FixedRankDimensionCheck,
      (itk::Concept::SameDimension<itkGetStaticConstMacro(FixedImageDimension),
          itkGetStaticConstMacro(RankImageDimension)>));

protected:
  typedef itk::ImageRegionIteratorWithIndex<MaskImageType> MaskIteratorType;
  typedef itk::ImageRegionConstIteratorWithIndex<FixedImageType>
        FixedIteratorType;
  typedef itk::ImageRegionConstIteratorWithIndex<RankImageType>
      FixedRankIteratorType;
  typedef typename RankFilterType::HistogramType HistogramType;
  typedef typename RankFilterType::HistogramPointer HistogramPointer;

  /** Fixed (already stochastically sampled) fixed average-ranked image. **/
  RankImagePointer m_FixedRankImage;
  /**
   * Mask for stochastic rank correlation (contains the pixels that should
   * really contribute to metric). If StochasticUserMask is set, this property
   * will temporarily reference this user mask.
   **/
  MaskImagePointer m_StochasticMask;
  /**
   * Optional user-supplied stochastic mask. If supplied, SampleCoverage will be
   * ignored and the pixels having a value >0 will be selected for metric
   * evaluation. NOTE: This mask is required to have the same size as the fixed
   * image (largest possible region). NOTE: If a fixed image mask and/or fixed
   * image region are also set, only the pixels of the stochastic mask within
   * the fixed image mask and/or fixed image region will contribute to metric
   * evaluation. This will potentially MODIFY this set mask image!
   */
  MaskImagePointer m_StochasticUserMask;
  /**
   * Stochastic factor: how many samples to extract from the image domain?
   * range: ]0;100] (%) NOTE: The word image domain here relates to the image
   * considering fixed image region and fixed image mask (if set). NOTE: If
   * StochasticUserMask is supplied, this attribute will be ignored.
   **/
  double m_SampleCoverage;
  /**
   * Seeds for deterministic behavior of stochastic sampling. Size must equal
   * the number of image dimensions. If seed value is 0 for a dimension, the
   * sampling behavior is non-deterministic (random).
   **/
  SeedsType m_RandomSeeds;
  /**
   * Set the derivative step length scales for each parameter dimension.
   * Internally the derivative is computed by using finite distances. The
   * finite distances are specified by these scales.
   */
  ScalesType m_DerivativeScales;
  /** Number of histogram bins for histogram extraction of fixed image. **/
  unsigned int m_FixedNumberOfHistogramBins;
  /** Minimum intensity for histogram extraction of fixed image. **/
  FixedPixelType m_FixedHistogramMinIntensity;
  /** Maximum intensity for histogram extraction of fixed image. **/
  FixedPixelType m_FixedHistogramMaxIntensity;
  /** Flag indicating that intensities at the ends should be clipped. **/
  bool m_FixedHistogramClipAtEnds;
  /** Number of histogram bins for histogram extraction of moving image. **/
  unsigned int m_MovingNumberOfHistogramBins;
  /** Minimum intensity for histogram extraction of moving image. **/
  MovingPixelType m_MovingHistogramMinIntensity;
  /** Maximum intensity for histogram extraction of moving image. **/
  MovingPixelType m_MovingHistogramMaxIntensity;
  /** Flag indicating that intensities at the ends should be clipped. **/
  bool m_MovingHistogramClipAtEnds;
  /** Stores the current moving rank histogram **/
  mutable HistogramPointer m_MovingRankHistogram;
  /**
   * Determine what should happen if moving and fixed image do not overlap for
   * a given transformation?<br>
   * 0 ... throw an exception<br>
   * 1 ... return the configured NoOverlapMetricValue<br>
   * DEFAULT: 0.
   **/
  int m_NoOverlapReactionMode;
  /**
   * Value that is returned if the moving and fixed image do not overlap for
   * a given transformation and m_NoOverlapReactionMode==1.
   **/
  MeasureType m_NoOverlapMetricValue;
  /**
   * Use Horn correction for tied ranks.
   * @see "D. Horn, A correction for the effect of tied ranks on the value of
   *   the rank difference correlation coefficient, Journal of Educational
   *   Psychology, Volume 33, Issue 9, December 1942, Pages 686-690."
   **/
  bool m_UseHornTiedRanksCorrection;
  /**
   * Horn correction sums for tied rank correction.
   * 1st element: fixed image
   * 2nd element: moving image
   * @see "D. Horn, A correction for the effect of tied ranks on the value of
   *   the rank difference correlation coefficient, Journal of Educational
   *   Psychology, Volume 33, Issue 9, December 1942, Pages 686-690."
   */
  mutable double m_HornCorrectionSums[2];
  /** A helper which counts the exact number of samples in fixed (rank)image.**/
  unsigned int m_NumberOfFixedSamplePixels;
  /**
   * Flag indicating whether pixels of the moving image that gain a 0-rank (e.g.
   * because they're out of histogram min/max-intensity) should contribute to
   * SRC coefficient or not (default: TRUE).
   */
  bool m_MovingZeroRanksContributeToMeasure;
  /** Flag indicating that GetValue() is called due to derivative computation **/
  mutable bool m_IsComputingDerivative;
  /**
   * DEBUGGING: Use it really only for debugging! If TRUE, the sampled fixed
   * and moving image and rank image intensities are internally cached.
   * @see m_SampleDistribution
   */
  bool m_ExtractSampleDistribution;
  /**
   * DEBUGGING: Contains the extracted sample distribution after each call
   * to GetValue() - BUT only if m_ExtractSampleDistribution is set to TRUE.
   * @see m_ExtractSampleDistribution
   */
  mutable std::vector<SampleDistributionEntry> m_SampleDistribution;

  /** "After histogram extraction" event entry point for fixed ranker. **/
  static void FixedAfterHistogramExtractionEvent(itk::Object *obj,
    const itk::EventObject &ev, void *cd);

  /** Default constructor **/
  StochasticRankCorrelationImageToImageMetric();
  /** Destructor **/
  virtual ~StochasticRankCorrelationImageToImageMetric();

  /** Print-out object information. **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Generate stochastic mask image corresponding to set pixel coverage.
   * @return TRUE if successful
   **/
  bool GenerateStochasticMaskImage();

  /**
   * Extracts the moving rank histogram according to current settings.
   * Stored in m_MovingRankHistogram.
   **/
  void ExtractMovingRankHistogram() const;

  /**
   * DEBUGGING: Extract the sample distribution (fixed image pixels and
   * fixed rank image pixels) if m_ExtractSampleDistribution == TRUE.
   */
  void ExtractFixedSampleDistribution() const;

private:
  /** Purposely not implemented **/
  StochasticRankCorrelationImageToImageMetric(const Self&);
  /** Purposely not implemented **/
  void operator=(const Self&);

};

}

#include "oraStochasticRankCorrelationImageToImageMetric.txx"

#endif /* ORASTOCHASTICRANKCORRELATIONIMAGETOIMAGEMETRIC_H_ */
