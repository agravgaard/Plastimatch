//
#ifndef ORAITKDRRFILTER_H_
#define ORAITKDRRFILTER_H_

//STL
#include <vector>
//ITK
#include <itkConceptChecking.h>
#include <itkMatrixOffsetTransformBase.h>
#include <itkMacro.h>
#include <itkRealTimeClock.h>
#include <itkVector.h>
#include <itkPoint.h>
#include <itkSize.h>
#include <itkFixedArray.h>
//ORAIFUTILS
#include "oraDRRFilter.h"

namespace ora
{
/** \class CPUSiddonDDRFilter
 * \brief Implements computation of digitally reconstructed radiographs (DRRs)
 * for the CPU
 *
 * This filter is a multi-threaded implementation of the ora::DRRFilter interface
 * for the CPU.
 * It calculates digitally reconstructed radiographw from given input using the Siddon
 * ray casting algorithm.
 * NOTE: This filter only supports rigid transformation as it is implemented with
 * moving geometry. This means that the imaging geometry is (inversely)
 * transformed instead of transforming and interpolating each voxel.
 *
 * Both images, the input image (volume) as well as the output image
 * (DRR), of this filter are assumed to be represented as 3D images. The DRR
 * is single-sliced.
 *
 * The filter is templated over the input pixel type and the output pixel type.
 *
 * So-called independent outputs are implemented. This means that the filter can
 * manage more than one output. DRR-computation refers to a specific output
 * while the others are unmodified. This can be used for 2D/3D-registration with
 * multiple images where DRRs with different projection geometry settings are
 * required.
 *
 * Moreover, this class is capable of defining DRR masks for each independent
 * output. These optional masks define whether or not a DRR pixel should be
 * computed (value greater than 0). This may especially be useful for stochastic
 * metric evaluations where only a subset of the DRR pixels is really used.
 *
 * Additionally, this class supports intensity transfer functions so that volume
 * intensities can be mapped to volume output intensities before they are summed up.
 * The intensity transfer function can be modified on the fly. In addition, there
 * is an off-the-fly ITF mode which implies that the ITF is applied to the
 * whole volume (whenever the volume or the ITF are changed) instead of computing
 * the ITF mapping for each voxel (on-the-fly). However, this implementation
 * tolerates NULL-ITF-pointers (no ITF mapping) which means that the raw input
 * volume intensities are summed up.
 *
 * Have a look on the well-documented class members and methods to find out
 * how to configure this class and how to setup the desired DRR geometry. Please
 * also refer to the design document.
 *
 * <b>Tests</b>:<br>
 * TestCPUSiddonDRRFilter.cxx <br>
 *
 * @see ora::DRRFilter
 *
 * @author phil
 * @author jeanluc
 * @version 1.0
 *
 * \ingroup ImageFilters
 **/
template<class TInputPixelType, class TOutputPixelType>
class CPUSiddonDRRFilter:
public ora::DRRFilter<TInputPixelType, TOutputPixelType>
{
public:
  /** Standard class typedefs. */
  typedef CPUSiddonDRRFilter Self;
  typedef ora::DRRFilter<TInputPixelType, TOutputPixelType> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Accessibility typedefs. */
  typedef itk::Point<double, 3> PointType;
  typedef itk::Vector<double, 3> VectorType;
  typedef itk::Size<2> SizeType;
  typedef itk::FixedArray<double, 2> SpacingType;
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

  /** Internal floating point comparison accuracy **/
  static const double EPSILON;


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
  /** @return FALSE (this implementation is purely CPU-based) **/
  virtual bool IsGPUBased() const;
  /** @return TRUE (this implementation is purely CPU-based) **/
  virtual bool IsCPUBased() const;
  /** @return TRUE (this implementation is CPU-based and multi-threaded) **/
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

  itkGetMacro(LastCPUPreProcessingTime, double)
  itkGetMacro(LastCPUPostProcessingTime, double)
  itkGetMacro(LastCPUPureProcessingTime, double)

  virtual void SetOffTheFlyITFMapping(bool flag);
  itkGetMacro(OffTheFlyITFMapping, bool)
  itkBooleanMacro(OffTheFlyITFMapping)

protected:
  typedef itk::RealTimeClock ClockType;
  typedef ClockType::Pointer ClockPointer;

  /** Stores pre-processing time of last DRR computation in milliseconds. **/
  double m_LastCPUPreProcessingTime;
  /** Stores post-processing time of last DRR computation in milliseconds. **/
  double m_LastCPUPostProcessingTime;
  /** Stores pure processing time of last DRR computation in milliseconds. **/
  double m_LastCPUPureProcessingTime;
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

  /** Executes the DRR computation. This method must be overridden in concrete
  * subclasses that are CPU-threaded.
  * @see GenerateData()
  * @see BeforeThreadedGenerateData()
  * @see AfterThreadedGenerateData()
  * @see oraDRRFilter **/
  virtual void ThreadedGenerateData(
      const OutputImageRegionType& outputRegionForThread, int threadId);
  /** CPU-threaded DRR computation pre-processing entry point.
  * @see GenerateData()
  * @see ThreadedGenerateData()
  * @see AfterThreadedGenerateData()
  * @see oraDRRFilter **/
  virtual void BeforeThreadedGenerateData();
  /** CPU-threaded DRR computation post-processing entry point.
  * @see GenerateData()
  * @see ThreadedGenerateData()
  * @see BeforeThreadedGenerateData()
  * @see oraDRRFilter **/
  virtual void AfterThreadedGenerateData();
  /** Update Geometry to be valid after input transformation **/
  virtual void UpdateCurrentImagingGeometry();


  /** Default constructor. **/
  CPUSiddonDRRFilter();
  /** Default destructor. **/
  virtual ~CPUSiddonDRRFilter();

private:
  /** Purposely not implemented. **/
  CPUSiddonDRRFilter(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};
}

#include "oraCPUSiddonDRRFilter.txx"

#endif /* ORACPUSIDDONDDRFILTER_H_ */

