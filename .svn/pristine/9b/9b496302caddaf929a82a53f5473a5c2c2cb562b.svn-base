//
#ifndef ORAGLSLRAYCASTINGDRRFILTER_H_
#define ORAGLSLRAYCASTINGDRRFILTER_H_

//ORAIFUTILS
#include "oraDRRFilter.h"

#include <itkMatrixOffsetTransformBase.h>

namespace ora
{
/** \class GLSLRayCastingDRRFilter
 * \brief FIXME
 *
 *
 * <b>Tests</b>:<br>
 * TestGLSLRayCastingDRRFilter.cxx <br>
 *
 * @see ora::DRRFilter
 *
 * @author phil
 * @version 1.0
 *
 * \ingroup ImageFilters
 **/
template<class TInputPixelType, class TOutputPixelType>
class GLSLRayCastingDRRFilter:
public ora::DRRFilter<TInputPixelType, TOutputPixelType>
{
public:
  /** Standard class typedefs. */
  typedef GLSLRayCastingDRRFilter Self;
  typedef ora::DRRFilter<TInputPixelType, TOutputPixelType> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Accessibility typedefs. */
  typedef typename Superclass::OutputImageType OutputImageType;
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;
  typedef typename Superclass::OutputImagePointer OutputImagePointer;
  typedef typename Superclass::OutputImagePixelType OutputImagePixelType;
  typedef typename Superclass::InputImageType InputImageType;
  typedef typename Superclass::InputImagePixelType InputImagePixelType;
  typedef typename Superclass::InputImageType InputImage;
  typedef typename Superclass::InputImagePointer InputImagePointer;
  typedef typename Superclass::GeometryType GeometryType;
  typedef typename Superclass::GeometryPointer GeometryPointer;
  typedef typename Superclass::MaskPixelType MaskPixelType;
  typedef typename Superclass::MaskImagePointer MaskImagePointer;
  typedef typename Superclass::ITFPointer ITFPointer;
  typedef itk::MatrixOffsetTransformBase<double,
			itkGetStaticConstMacro(InputImageDimension),
			itkGetStaticConstMacro(InputImageDimension)> TransformType;
  typedef typename TransformType::Pointer TransformPointer;
  typedef float MappedVolumePixelType;
  typedef itk::Image<MappedVolumePixelType, 3> MappedVolumeImageType;
  typedef MappedVolumeImageType::Pointer MappedVolumeImagePointer;

  /**Run-time type information (and related methods). */
  itkTypeMacro(Self, Superclass)

  /** Method for creation through the object factory */
  itkNewMacro(Self)

  /** Set the input image (3D volume) based on ITK image data.
   * @param input the ITK image data set to be used for DRR computation **/
  virtual void SetInput(InputImagePointer input);

  /** Generate information describing the output data.
  * A DRR filter usually produces an image with a different size than its input
  * image.
  * @see itk::ProcessObject#GenerateOutputInformaton() **/
  virtual void GenerateOutputInformation();

  /** @return TRUE if current settings are sufficient for computing a DRR **/
  virtual bool DRRCanBeComputed() const;
  /** @return TRUE (this implementation is purely GPU-based) **/
  virtual bool IsGPUBased() const;
  /** @return FALSE (this implementation is purely GPU-based) **/
  virtual bool IsCPUBased() const;
  /** @return FALSE (this implementation is GPU-based) **/
  virtual bool IsCPUMultiThreaded() const;
  /** @return TRUE (this implementation supports "on the fly" ITFs) **/
  virtual bool IsSupportingITFOnTheFly() const;
  /** @return TRUE (this implementation supports "off the fly" ITFs) **/
  virtual bool IsSupportingITFOffTheFly() const;
  /** @return TRUE (this implementation supports only rigid transforms) **/
  virtual bool IsSupportingRigidTransformation() const;
  /** @return FALSE (this implementation supports only rigid transforms) **/
  virtual bool IsSupportingAffineTransformation() const;
  /** @return FALSE (this implementation supports only rigid transforms) **/
  virtual bool IsSupportingElasticTransformation() const;
  /** @return TRUE (this implementation supports DRR masks; however, NULL is
   * also allowed) **/
  virtual bool IsSupportingDRRMasks() const;

  itkGetMacro(LastPreProcessingTime, double)
  itkGetMacro(LastPostProcessingTime, double)
  itkGetMacro(LastGPUPureProcessingTime, double)

  virtual void SetOffTheFlyITFMapping(bool flag);
  itkGetMacro(OffTheFlyITFMapping, bool)
  itkBooleanMacro(OffTheFlyITFMapping)

protected:
  typedef itk::RealTimeClock ClockType;
  typedef ClockType::Pointer ClockPointer;

  /** Stores pre-processing time of last DRR computation in milliseconds. **/
  double m_LastPreProcessingTime;
  /** Stores post-processing time of last DRR computation in milliseconds. **/
  double m_LastPostProcessingTime;
  /** Stores pure processing time of last DRR computation in milliseconds. **/
  double m_LastGPUPureProcessingTime;
  /** Tool clock for time measurements. **/
  ClockPointer m_PreProcessingClock;
  /** Tool clock for time measurements. **/
  ClockPointer m_PostProcessingClock;
  /** Tool clock for time measurements. **/
  ClockPointer m_PureProcessingClock;
  /** Current geometry. **/
  GeometryPointer m_CurrentGeometry;
  /** Transformation describing the current geometry transformation. **/
  TransformPointer m_GeometryTransform;
  /** Enables ITF mapping of the whole volume input before calculating the DRR. **/
  bool m_OffTheFlyITFMapping;
  /** Stores when the input was last remapped with the intensity transfer function. **/
  double m_LastITFCalculationTimeStamp;
  /** Stores the ITF mapped volume (in off-the-fly-ITF mode). FLOAT precision! **/
  MappedVolumeImagePointer m_MappedInput;
  /** Input Volumen update timestamp. **/
  double m_LastInputVolumeTimeStamp;

  /** Initiates the essential DRR computation on the GPU.
  * @see oraDRRFilter **/
  virtual void GenerateData();

  /** Update Geometry to be valid after input transformation **/
  virtual void UpdateCurrentImagingGeometry();


  /** Default constructor. **/
  GLSLRayCastingDRRFilter();
  /** Default destructor. **/
  virtual ~GLSLRayCastingDRRFilter();

private:
  /** Purposely not implemented. **/
  GLSLRayCastingDRRFilter(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraGLSLRayCastingDRRFilter.txx"

#endif /* ORAGLSLRAYCASTINGDRRFILTER_H_ */
