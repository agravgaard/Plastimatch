//
#ifndef ORAGRADIENTMAGNITUDEDIFFERENCEIMAGETOIMAGEMETRIC_H_
#define ORAGRADIENTMAGNITUDEDIFFERENCEIMAGETOIMAGEMETRIC_H_

#include <itkImageToImageMetric.h>
#include <itkResampleImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkSobelOperator.h>
#include <itkNeighborhoodOperatorImageFilter.h>
#include <itkZeroFluxNeumannBoundaryCondition.h>
#include <itkGradientMagnitudeImageFilter.h>

namespace ora
{

/** \class GradientMagnitudeDifferenceImageToImageMetric
 * \brief Gradient magnitude difference image metric supporting masks.
 *
 * An simplified version of gradient difference metric (as implemented in
 * itk::GradientMagnitudeDifferenceImageToImageMetric), but with support of fixed and
 * moving image masks and the usage of the gradient magnitude instead of
 * directional gradients.
 *
 * This metric yields an effective maximization problem.
 *
 * FIXME: rest of comments
 *
 * @see itk::GradientDifferenceImageToImageMetric
 * @see itk::ImageToImageMetric
 *
 * @author Markus 
 * @version 1.1
 */
template<class TFixedImage, class TMovingImage, class TGradientPixelType>
class GradientMagnitudeDifferenceImageToImageMetric :
    public itk::ImageToImageMetric<TFixedImage, TMovingImage>
{
public:
  /** Standard class typedefs. */
  typedef GradientMagnitudeDifferenceImageToImageMetric Self;
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
  typedef TGradientPixelType GradientPixelType;
  itkStaticConstMacro(ImageDimension, unsigned int,
      FixedImageType::ImageDimension);
  itkStaticConstMacro(FixedImageDimension, unsigned int,
      FixedImageType::ImageDimension);
  itkStaticConstMacro(MovingImageDimension, unsigned int,
      MovingImageType::ImageDimension);
  typedef itk::Image<GradientPixelType, ImageDimension> GradientImageType;
  typedef typename GradientImageType::Pointer GradientImagePointer;
  typedef typename GradientImageType::ConstPointer GradientImageConstPointer;

  /** Inherited types. **/
  typedef typename Superclass::Pointer SuperclassPointer;
  typedef typename Superclass::ParametersType ParametersType;
  typedef typename Superclass::MeasureType MeasureType;
  typedef typename Superclass::DerivativeType DerivativeType;
  typedef typename Superclass::TransformType TransformType;

  /** Scales type. */
  typedef itk::Array<double> ScalesType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(GradientMagnitudeDifferenceImageToImageMetric, ImageToImageMetric)

  /** Method for creation through the object factory. */
  itkNewMacro(Self)

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

  /** Set the transform and adapt the derivative scales if necessary. **/
  virtual void SetTransform(TransformType *transform);

  /**
   * Initialize the metric.
   * FIXME: describe in more detail
   **/
  virtual void Initialize() throw (itk::ExceptionObject);

  /**
   * This method returns the value of the gradient magnitude difference cost
   * function corresponding to the specified transformation parameters.
   * @see itk::SingleValuedCostFunction#GetValue()
   **/
  virtual MeasureType GetValue(const ParametersType &parameters) const;

  /**
   * This method returns the derivative of the gradient difference cost
   * function corresponding to the specified transformation parameters.
   * Derivative estimation is based on finite distances.
   * @see itk::SingleValuedCostFunction#GetDerivative()
   **/
  virtual void GetDerivative(const ParametersType &parameters,
      DerivativeType &derivative) const;

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

protected:
  /** Internal types */
  typedef itk::Image<FixedPixelType, itkGetStaticConstMacro(ImageDimension)>
      TransformedMovingImageType;
  typedef itk::ResampleImageFilter<MovingImageType, TransformedMovingImageType>
      TransformMovingImageFilterType;
  typedef typename TransformMovingImageFilterType::Pointer
      TransformMovingImageFilterPointer;
  typedef itk::CastImageFilter<FixedImageType, GradientImageType>
      CastFixedImageFilterType;
  typedef typename CastFixedImageFilterType::Pointer
      CastFixedImageFilterPointer;
  typedef itk::CastImageFilter<MovingImageType, GradientImageType>
      CastMovingImageFilterType;
  typedef typename CastMovingImageFilterType::Pointer
      CastMovingImageFilterPointer;
  typedef itk::GradientMagnitudeImageFilter<GradientImageType, GradientImageType>
      GradientMagnitudeType;

  /**
   * Set the derivative step length scales for each parameter dimension.
   * Internally the derivative is computed by using finite distances. The
   * finite distances are specified by these scales.
   */
  ScalesType m_DerivativeScales;
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
  /** Flag indicating that GetValue() is called due to derivative computation **/
  mutable bool m_IsComputingDerivative;
  /** The range of the moving image gradients (min) **/
  mutable GradientPixelType m_MinMovingGradient;
  /** The range of the moving image gradients (max) **/
  mutable GradientPixelType m_MaxMovingGradient;
  /** The range of the fixed image gradients (min) */
  mutable GradientPixelType m_MinFixedGradient;
  /** The range of the fixed image gradients (max) **/
  mutable GradientPixelType m_MaxFixedGradient;
  /** The variance of the fixed image gradients. **/
  mutable GradientPixelType m_FixedVariance;
  /** The filter for transforming the moving image. **/
  TransformMovingImageFilterPointer m_TransformMovingImageFilter;
  /** The caster of the fixed image **/
  CastFixedImageFilterPointer m_CastFixedImageFilter;
  /** The caster of the moving image **/
  CastMovingImageFilterPointer m_CastMovingImageFilter;
  /** The gradient magnitudes of the fixed image **/
  typename GradientMagnitudeType::Pointer m_FixedGradientMagnitudeFilter;
  /** The gradient magnitudes of the moving image **/
  typename GradientMagnitudeType::Pointer m_MovingGradientMagnitudeFilter;

  /** Default constructor **/
  GradientMagnitudeDifferenceImageToImageMetric();
  /** Destructor **/
  virtual ~GradientMagnitudeDifferenceImageToImageMetric();

  /** Print-out object information. **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /** Compute gradient variance of fixed gradient image.  **/
  void ComputeVariance() const;

  /** Compute gradient range of transformed moving gradient image.  **/
  void ComputeMovingGradientRange() const;

  /** Compute pure measure on pre-computed gradient images.  **/
  MeasureType ComputeMeasure(const ParametersType &parameters) const;

private:
  /** Purposely not implemented **/
  GradientMagnitudeDifferenceImageToImageMetric(const Self&);
  /** Purposely not implemented **/
  void operator=(const Self&);

};

}

#include "oraGradientMagnitudeDifferenceImageToImageMetric.txx"

#endif /* ORAGRADIENTMAGNITUDEDiffERENCEIMAGETOIMAGEMETRIC_H_ */
