//
#ifndef ORAMULTIIMAGETOIMAGEMETRIC_TXX_
#define ORAMULTIIMAGETOIMAGEMETRIC_TXX_

#include "oraMultiImageToImageMetric.h"

#include "oraMetricEvents.hxx"

namespace ora
{

template<class TFixedImage, class TMovingImage>
MultiImageToImageMetric<TFixedImage, TMovingImage>::MultiImageToImageMetric() :
  Superclass()
{
  m_ReInitializeMetricsBeforeEvaluating = false;
  m_CurrentParameters.Fill(0);
  m_LastValues.Fill(0);
  m_LastDerivatives.Fill(0);
}

template<class TFixedImage, class TMovingImage>
MultiImageToImageMetric<TFixedImage, TMovingImage>::~MultiImageToImageMetric()
{
  for (std::size_t i = 0; i < m_InputMetrics.size(); i++)
    m_InputMetrics[i] = NULL;
  m_InputMetrics.clear();
}

template<class TFixedImage, class TMovingImage>
void MultiImageToImageMetric<TFixedImage, TMovingImage>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Input Metrics: (n=" << m_InputMetrics.size() << ")\n";
  for (std::size_t i = 0; i < m_InputMetrics.size(); i++)
  {
    if (m_InputMetrics[i])
    {
      os << indent << " - " << "metric[" << i << "]: "
          << m_InputMetrics[i].GetPointer() << "\n";
    }
  }
  os << indent << "Re-Initialize Metrics Before Evaluating: "
      << m_ReInitializeMetricsBeforeEvaluating << "\n";
  os << indent << "Current Parameters: " << m_CurrentParameters << "\n";
  os << indent << "Last Values: " << m_LastValues << "\n";
  os << indent << "Last Derivatives: " << m_LastDerivatives << "\n";

  // FIXME:
}

template<class TFixedImage, class TMovingImage>
typename MultiImageToImageMetric<TFixedImage, TMovingImage>::MeasureType MultiImageToImageMetric<
    TFixedImage, TMovingImage>::GetValue(const ParametersType &parameters) const
{
  std::size_t numMetrics = GetNumberOfMetricInputs();
  if (numMetrics <= 0)
  {
    MeasureType zero(0);
    return zero;
  }

  // copy current parameters ... so that they're available in event:
  m_CurrentParameters = parameters;
  this->InvokeEvent(ora::BeforeEvaluationEvent());

  // at this point, the moving images must be already up to date!

  // re-initialize the relevant metrics on demand:
  if (m_ReInitializeMetricsBeforeEvaluating)
  {
    for (std::size_t i = 0; i < numMetrics; ++i)
      m_InputMetrics[i]->Initialize();
  }

  MeasureType value(numMetrics);
  value.Fill(0);
  for (std::size_t i = 0; i < numMetrics; ++i) // metric value evaluations
  {
    value[i] = m_InputMetrics[i]->GetValue(parameters);
  }

  // make last value already available in event:
  m_LastValues = value;
  this->InvokeEvent(ora::AfterEvaluationEvent());

  return value;
}

template<class TFixedImage, class TMovingImage>
void MultiImageToImageMetric<TFixedImage, TMovingImage>::GetDerivative(
    const ParametersType &parameters, DerivativeType &derivative) const
{
  std::size_t numMetrics = GetNumberOfMetricInputs();
  if (numMetrics <= 0)
  {
    derivative.SetSize(0, 0);
    return;
  }

  // copy current parameters ... so that they're available in event:
  m_CurrentParameters = parameters;
  this->InvokeEvent(ora::BeforeEvaluationEvent());

  // at this point, the moving images must be already up to date!

  // re-initialize the relevant metrics on demand:
  if (m_ReInitializeMetricsBeforeEvaluating)
  {
    for (std::size_t i = 0; i < numMetrics; ++i)
      m_InputMetrics[i]->Initialize();
  }

  derivative.SetSize(numMetrics, m_InputMetrics[0]->GetNumberOfParameters());
  derivative.Fill(0);
  for (std::size_t i = 0; i < numMetrics; ++i) // metric derivative evaluations
  {
    itk::Array<double> subDerivative;
    m_InputMetrics[i]->GetDerivative(parameters, subDerivative);
    for (unsigned int c = 0; c < subDerivative.Size(); c++)
      derivative[i][c] = subDerivative[c];
  }

  // make last derivatives already available in event:
  m_LastDerivatives = derivative;
  this->InvokeEvent(ora::AfterEvaluationEvent());
}

template<class TFixedImage, class TMovingImage>
unsigned int MultiImageToImageMetric<TFixedImage, TMovingImage>::GetNumberOfValues() const
{
  return this->GetNumberOfMetricInputs();
}

template<class TFixedImage, class TMovingImage>
unsigned int MultiImageToImageMetric<TFixedImage, TMovingImage>::GetNumberOfParameters() const
{
  if (GetNumberOfMetricInputs() > 0)
  {
    return m_InputMetrics[0]->GetNumberOfParameters();
  }
  else
  {
    return 0;
  }
}

template<class TFixedImage, class TMovingImage>
bool MultiImageToImageMetric<TFixedImage, TMovingImage>::AddMetricInput(
    BaseMetricType *metric)
{
  if (metric)
  {
    m_InputMetrics.push_back(metric);
    this->Modified();
    return true;
  }
  else
  {
    return false;
  }
}

template<class TFixedImage, class TMovingImage>
std::size_t MultiImageToImageMetric<TFixedImage, TMovingImage>::GetNumberOfMetricInputs() const
{
  return m_InputMetrics.size();
}

template<class TFixedImage, class TMovingImage>
typename MultiImageToImageMetric<TFixedImage, TMovingImage>::BaseMetricType *MultiImageToImageMetric<
    TFixedImage, TMovingImage>::GetIthMetricInput(std::size_t i)
{
  if (i < GetNumberOfMetricInputs())
    return m_InputMetrics[i];
  else
    return NULL;
}

template<class TFixedImage, class TMovingImage>
bool MultiImageToImageMetric<TFixedImage, TMovingImage>::RemoveIthMetricInput(
    std::size_t i)
{
  if (i < GetNumberOfMetricInputs())
  {
    m_InputMetrics[i] = NULL;
    m_InputMetrics.erase(m_InputMetrics.begin() + i);
    return true;
  }
  else
  {
    return false;
  }
}

template<class TFixedImage, class TMovingImage>
void MultiImageToImageMetric<TFixedImage, TMovingImage>::RemoveAllMetricInputs()
{
  for (std::size_t i = 0; i < GetNumberOfMetricInputs(); i++)
    m_InputMetrics[i] = NULL;
  m_InputMetrics.clear();
}

template<class TFixedImage, class TMovingImage>
void MultiImageToImageMetric<TFixedImage, TMovingImage>::Initialize()
    throw (itk::ExceptionObject)
{
  ;
}

}

#endif /* ORAMULTIIMAGETOIMAGEMETRIC_TXX_ */
