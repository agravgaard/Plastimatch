#ifndef __itkVelocityFieldExponentialComposedWithDisplacementFieldFilter_txx_
#define __itkVelocityFieldExponentialComposedWithDisplacementFieldFilter_txx_

#include "itkVelocityFieldExponentialComposedWithDisplacementFieldFilter.h"

#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>

namespace itk
{
template< class TVelocityField, class TInputDisplacementField, class TOutputDisplacementField >
VelocityFieldExponentialComposedWithDisplacementFieldFilter< TVelocityField, TInputDisplacementField,
                                                             TOutputDisplacementField >
::VelocityFieldExponentialComposedWithDisplacementFieldFilter()
{
  m_NumberOfIntegrationSteps = 10;
  m_VelocityField = 0;
  m_ComputeInverse = false;
  m_VelocityFieldInterpolator = VelocityFieldInterpolatorType::New();
}

template< class TVelocityField, class TInputDisplacementField, class TOutputDisplacementField >
void
VelocityFieldExponentialComposedWithDisplacementFieldFilter< TVelocityField, TInputDisplacementField,
                                                             TOutputDisplacementField >
::BeforeThreadedGenerateData()
{
  if ( m_VelocityField.IsNull() )
    {
    itkExceptionMacro (<< "Velocity field must be set");
    }

  if ( m_NumberOfIntegrationSteps <= 0 )
    {
    itkExceptionMacro (<< "Number of integration step cannot be null or negative");
    }

  m_VelocityFieldInterpolator->SetInputImage (m_VelocityField);
}

template< class TVelocityField, class TInputDisplacementField, class TOutputDisplacementField >
void
VelocityFieldExponentialComposedWithDisplacementFieldFilter< TVelocityField, TInputDisplacementField,
                                                             TOutputDisplacementField >
::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, int threadId)
{
  typename InputImageType::ConstPointer inputField = this->GetInput();
  typename OutputImageType::Pointer     outputField = this->GetOutput();

  typedef ImageRegionConstIteratorWithIndex< InputImageType > InputImageIteratorType;
  typedef ImageRegionIteratorWithIndex< OutputImageType >     OutputImageIteratorType;

  InputImageIteratorType itIn(inputField,  outputRegionForThread);

  OutputImageIteratorType itOut(outputField, outputRegionForThread);

  double dt = 1.0 / static_cast< double >(m_NumberOfIntegrationSteps);

  while ( !itOut.IsAtEnd() )
    {
    InputPixelType vecIn = itIn.Value();

    IndexType indexIn = itOut.GetIndex();
    PointType pointIn;

    inputField->TransformIndexToPhysicalPoint (indexIn, pointIn);
    pointIn = pointIn + vecIn;

    OutputPixelType vecOut(0.0);

    for ( int i = 0; i < m_NumberOfIntegrationSteps; i++ )
      {
      PointType pointOut = pointIn + vecOut;

      OutputPixelType vec = m_VelocityFieldInterpolator->Evaluate (pointOut);
      if ( m_ComputeInverse )
        {
        vecOut -= vec * dt;
        }
      else
        {
        vecOut += vec * dt;
        }
      }

    itOut.Set (vecIn + vecOut);

    ++itIn;
    ++itOut;
    }
}

} // end namespace itk

#endif
