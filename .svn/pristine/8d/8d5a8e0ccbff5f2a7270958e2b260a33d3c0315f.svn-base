//
#ifndef ORADRRITFLIKELIHOODCOSTFUNCTION_H_
#define ORADRRITFLIKELIHOODCOSTFUNCTION_H_

#include <itkImageToImageMetric.h>
#include <itkHistogram.h>
#include <itkImageRegionConstIterator.h>

namespace ora
{

/** \class DRRITFLikelihoodCostFunction
 * FIXME
 *
 * min, max, bins, cut-off is set for fixed image
 * --> sampled moving image values are binned into histogram of fixed image!
 *
 * FIXME: histogram is extracted from WHOLE fixed image - regardless whether or
 * not a fixed image mask is set; BUT: the fixed image mask affects measure
 * evaluation (only the corresponding moving image pixels are considered)
 *
 * FIXME: metric must be maximized
 *
 * NOTE: The initial idea with the ML-approach is from Markus, but was modified!
 *
 * FIXME: template parameters ...
 *
 * @see itk::ImageToImageMetric
 *
 * @author phil 
 * @author markus 
 * @version 1.0
 */
template<class TFixedImage, class TMovingImage>
class DRRITFLikelihoodCostFunction :
    public itk::ImageToImageMetric<TFixedImage, TMovingImage>
{
public:
  /** Standard class typedefs. */
  typedef DRRITFLikelihoodCostFunction Self;
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Accessibility types **/
  typedef TFixedImage FixedImageType;
  typedef typename FixedImageType::Pointer FixedImagePointer;
  typedef typename FixedImageType::ConstPointer FixedImageConstPointer;
  typedef typename FixedImageType::PixelType FixedPixelType;
  typedef TMovingImage MovingImageType;
  typedef typename MovingImageType::Pointer MovingImagePointer;
  typedef typename MovingImageType::ConstPointer MovingImageConstPointer;
  typedef typename MovingImageType::PixelType MovingPixelType;
  typedef typename Superclass::ParametersType ParametersType;
  typedef typename Superclass::MeasureType MeasureType;
  typedef typename Superclass::DerivativeType DerivativeType;
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

  /** Scales type. */
  typedef itk::Array<double> ScalesType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DRRITFLikelihoodCostFunction, ImageToImageMetric)

  /** Method for creation through the object factory. */
  itkNewMacro(Self)

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

  /**
   * Set/get flag indicating that moving image intensities outside the declared
   * fixed image intensity range (m_FixedHistogramMinIntensity,
   * m_FixedHistogramMaxIntensity) are mapped to a zero-probability (do not
   * contribute to resulting measure).
   * @see m_FixedHistogramMinIntensity
   * @see m_FixedHistogramMaxIntensity
   **/
  itkSetMacro(MapOutsideIntensitiesToZeroProbability, bool)
  itkGetMacro(MapOutsideIntensitiesToZeroProbability, bool)
  itkBooleanMacro(MapOutsideIntensitiesToZeroProbability)


  /** Initialize the metric. The histogram of the fixed image is extracted. **/
  virtual void Initialize() throw (itk::ExceptionObject);

  /**
   * This method returns the value of the cost function corresponding to the
   * specified transformation parameters.
   * @see itk::SingleValuedCostFunction#GetValue()
   **/
  virtual MeasureType GetValue(const ParametersType &parameters) const;

  /**
   * This method returns the derivative of the cost
   * function corresponding to the specified transformation parameters.
   * Derivative estimation is based on finite distances.
   * @see itk::SingleValuedCostFunction#GetDerivative()
   **/
  virtual void GetDerivative(const ParametersType &parameters,
      DerivativeType &derivative) const;

  // FIXME: concept checks

protected:
  typedef itk::Statistics::Histogram<double> HistogramType;
  typedef HistogramType::Pointer HistogramPointer;
  typedef itk::ImageRegionConstIterator<FixedImageType> IteratorType;

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
  /** Histogram of fixed image. **/
  HistogramPointer m_FixedHistogram;
  /**
   * Flag indicating that moving image intensities outside the declared fixed
   * image intensity range (m_FixedHistogramMinIntensity,
   * m_FixedHistogramMaxIntensity) are mapped to a zero-probability (do not
   * contribute to resulting measure).
   * @see m_FixedHistogramMinIntensity
   * @see m_FixedHistogramMaxIntensity
   **/
  bool m_MapOutsideIntensitiesToZeroProbability;

  /** Default constructor. **/
  DRRITFLikelihoodCostFunction();
  /** Destructor. **/
  virtual ~DRRITFLikelihoodCostFunction();

  /** Print-out object information. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /** "After histogram extraction" event entry point for fixed ranker. **/
  static void AfterHistogramExtractionEvent(itk::Object *obj,
      const itk::EventObject &ev, void *cd);

  /** Extract the histogram from the whole fixed image. **/
  virtual void ExtractFixedImageHistogram();

private:
  /** Purposely not implemented **/
  DRRITFLikelihoodCostFunction(const Self&);
  /** Purposely not implemented **/
  void operator=(const Self&);

};

}

#include "oraDRRITFLikelihoodCostFunction.txx"

#endif /* ORADRRITFLIKELIHOODCOSTFUNCTION_H_ */
