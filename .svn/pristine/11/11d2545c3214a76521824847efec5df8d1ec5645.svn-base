#ifndef __itkVelocityFieldExponentialComposedWithDisplacementFieldFilter_h_
#define __itkVelocityFieldExponentialComposedWithDisplacementFieldFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorLinearInterpolateNearestNeighborExtrapolateImageFunction.h"
// #include "itkVectorLinearInterpolateImageFunction.h"

namespace itk
{
/**
   \class VelocityFieldExponentialComposedWithDisplacementFieldFilter
   \brief Computes the composition of a displacement field \phi with the exponential
   of a velocity field u: exp(u)o\phi

   This is an alternative to using an exponential and a composition filter. Using both
   implies at least two resamplings, one for each filter. This class only resamples the
   velocity field once, which increases the result accuracy.

   Moreover, a forward Euler method is used to compute the exponential, as it was shown
   to be more accurate than the scaling and squaring method.

   \author Pierre Fillard, INRIA Paris
 */

template< class TVelocityField, class TInputDisplacementField, class TOutputDisplacementField >
class VelocityFieldExponentialComposedWithDisplacementFieldFilter:
  public ImageToImageFilter< TInputDisplacementField, TOutputDisplacementField >
{
public:
  typedef VelocityFieldExponentialComposedWithDisplacementFieldFilter             Self;
  typedef ImageToImageFilter< TInputDisplacementField, TOutputDisplacementField > Superclass;
  typedef SmartPointer< Self >                                                    Pointer;
  typedef SmartPointer< const Self >                                              ConstPointer;

  itkNewMacro  (Self);
  itkTypeMacro (VelocityFieldExponentialComposedWithDisplacementFieldFilter, ImageToImageFilter);

  typedef TVelocityField                        VelocityFieldType;
  typedef typename VelocityFieldType::PixelType VelocityPixelType;

  typedef TInputDisplacementField             InputImageType;
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename InputImageType::RegionType InputImageRegionType;
  typedef typename InputImageType::IndexType  IndexType;
  typedef typename InputImageType::PointType  PointType;

  typedef TOutputDisplacementField             OutputImageType;
  typedef typename OutputImageType::PixelType  OutputPixelType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  //typedef VectorLinearInterpolateImageFunction <VelocityFieldType>
  // VelocityFieldInterpolatorType;
  typedef VectorLinearInterpolateNearestNeighborExtrapolateImageFunction< VelocityFieldType >
  VelocityFieldInterpolatorType;

  itkSetObjectMacro (VelocityField, VelocityFieldType);
  itkGetObjectMacro (VelocityField, VelocityFieldType);

  /**
   * Set/Get the number of integration steps to compute the exponential
   */
  itkSetMacro (NumberOfIntegrationSteps, int);
  itkGetMacro (NumberOfIntegrationSteps, int);

  /**
   * If On, compute exp(-u)o\phi instead of exp(u)o\phi
   */
  itkSetMacro      (ComputeInverse, bool);
  itkGetConstMacro (ComputeInverse, bool);
  itkBooleanMacro  (ComputeInverse);
protected:
  VelocityFieldExponentialComposedWithDisplacementFieldFilter();
  ~VelocityFieldExponentialComposedWithDisplacementFieldFilter(){}

  void BeforeThreadedGenerateData(void);

  void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                            int threadId);

private:
  VelocityFieldExponentialComposedWithDisplacementFieldFilter (const Self &);
  void operator=(const Self &);

  typename VelocityFieldType::Pointer m_VelocityField;
  typename VelocityFieldInterpolatorType::Pointer m_VelocityFieldInterpolator;

  int  m_NumberOfIntegrationSteps;
  bool m_ComputeInverse;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkVelocityFieldExponentialComposedWithDisplacementFieldFilter.txx"
#endif

#endif
