//
#ifndef ORACOMPOSITEIMAGETOIMAGEMETRIC_TXX_
#define ORACOMPOSITEIMAGETOIMAGEMETRIC_TXX_

#include "oraCompositeImageToImageMetric.h"

#include "oraMetricEvents.hxx"

#include <itksys/SystemTools.hxx>

#include <algorithm>
#include <locale>

namespace ora
{

template<class TFixedImage, class TMovingImage>
CompositeImageToImageMetric<TFixedImage, TMovingImage>::CompositeImageToImageMetric() :
  Superclass()
{
  m_Parser = ParserPointer::New();
  m_Parser->SetReplaceInvalidValues(false);
  m_ValueCompositeRule = "";
  m_DerivativeCompositeRule = "";
  m_UseOptimizedValueComputation = true;
  m_UseOptimizedDerivativeComputation = false; // GetJacobian() not thread-safe
  m_ReInitializeMetricsBeforeEvaluating = false;
  m_Threader = ThreaderType::New();
  m_LastValue = 0;
  m_LastDerivative.SetSize(0);

  // determine the number of logically available CPUs (platform-dependent):
#ifdef ITK_USE_SPROC
  m_NumberOfAvailableCPUs = prctl(PR_MAXPPROCS);
#endif

#ifdef ITK_USE_PTHREADS
#ifdef _SC_NPROCESSORS_ONLN
  m_NumberOfAvailableCPUs = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_SC_NPROC_ONLN)
  m_NumberOfAvailableCPUs = sysconf(_SC_NPROC_ONLN);
#else
  //num = 1;
#endif
#if defined(__SVR4) && defined(sun) && defined(PTHREAD_MUTEX_NORMAL)
  pthread_setconcurrency(m_NumberOfAvailableCPUs);
#endif
#endif

#if defined(_WIN32)
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  m_NumberOfAvailableCPUs = sysInfo.dwNumberOfProcessors;
#endif

#ifndef ITK_USE_WIN32_THREADS
#ifndef ITK_USE_SPROC
#ifndef ITK_USE_PTHREADS
  m_NumberOfAvailableCPUs = 1;
#endif
#endif
#endif

#ifdef __APPLE__
  size_t dataLen = sizeof(int); // 'num' is an 'int'
  int result = sysctlbyname ("hw.logicalcpu", &m_NumberOfAvailableCPUs,
      &dataLen, NULL, 0);
  if (result == -1)
  m_NumberOfAvailableCPUs = 1;
#endif

  if (m_NumberOfAvailableCPUs < 1)
    m_NumberOfAvailableCPUs = 1;
  m_OverrideNumberOfAvailableCPUs = 0;
}

template<class TFixedImage, class TMovingImage>
CompositeImageToImageMetric<TFixedImage, TMovingImage>::~CompositeImageToImageMetric()
{
  m_Parser = NULL;
  for (std::size_t i = 0; i < m_InputMetrics.size(); i++)
    m_InputMetrics[i] = NULL;
  m_InputMetrics.clear();
  m_ValueVariables.clear();
  m_DerivativeVariables.clear();
  m_ThreadedMetricEvaluationFlags.clear();
  m_Threader = NULL;
  m_MetricIndices.clear();
}

template<class TFixedImage, class TMovingImage>
void CompositeImageToImageMetric<TFixedImage, TMovingImage>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Parser: " << (m_Parser ? m_Parser.GetPointer() : 0) << "\n";
  os << indent << "Value Composite Rule: " << m_ValueCompositeRule << "\n";
  os << indent << "Derivative Composite Rule: " << m_DerivativeCompositeRule
      << "\n";
  os << indent << "Input Metrics: (n=" << m_InputMetrics.size() << ")\n";
  for (std::size_t i = 0; i < m_InputMetrics.size(); i++)
  {
    if (m_InputMetrics[i])
    {
      os << indent << " - " << "metric[" << i << "]: "
          << m_InputMetrics[i].GetPointer() << " (v=\"" << m_ValueVariables[i]
          << "\", d=\"" << m_DerivativeVariables[i] << "\")\n";
    }
  }
  os << indent << "Use Optimized Value Computation: "
      << m_UseOptimizedValueComputation << "\n";
  os << indent << "Use Optimized Derivative Computation: "
      << m_UseOptimizedDerivativeComputation << "\n";
  os << indent << "Number Of Available CPUs: " << m_NumberOfAvailableCPUs
      << "\n";
  os << indent << "Override Number Of Available CPUs: "
      << m_OverrideNumberOfAvailableCPUs << "\n";
  os << indent << "Re-Initialize Metrics Before Evaluating: "
      << m_ReInitializeMetricsBeforeEvaluating << "\n";

  // FIXME
}

template<class TFixedImage, class TMovingImage>
ITK_THREAD_RETURN_TYPE CompositeImageToImageMetric<TFixedImage, TMovingImage>::MetricEvaluationThreaderCallback(
    void *arg)
{
  // access the pointer to the composite metric ...
  if (!arg)
    return ITK_THREAD_RETURN_VALUE;
  MetricEvaluationThreadStruct
      *metstruct =
          (MetricEvaluationThreadStruct *) (((ThreaderType::ThreadInfoStruct *) (arg))->UserData);
  if (!metstruct)
    return ITK_THREAD_RETURN_VALUE;
  ConstPointer thiss = metstruct->CompositeMetric;
  if (!thiss)
    return ITK_THREAD_RETURN_VALUE;

  // thread-safely search for the next metric to be evaluated:
  SuperclassPointer metric = NULL;
  std::string variable = "";
  thiss->m_MetricMutex.Lock();
  for (std::size_t i = 0; i < thiss->m_ThreadedMetricEvaluationFlags.size(); i++)
  {
    if (!thiss->m_ThreadedMetricEvaluationFlags[i])
    {
      metric = thiss->m_InputMetrics[thiss->m_MetricIndices[i]];
      if (metstruct->EvaluateValue)
        variable = thiss->m_ValueVariables[thiss->m_MetricIndices[i]];
      else
        variable = thiss->m_DerivativeVariables[thiss->m_MetricIndices[i]];
      thiss->m_ThreadedMetricEvaluationFlags[i] = true; // mark as processed
      break;
    }
  }
  thiss->m_MetricMutex.Unlock();

  if (!metric) // no metric to evaluate -> finished ...
    return ITK_THREAD_RETURN_VALUE;

  if (metstruct->EvaluateValue)
  {
    // single evaluation
    double value = metric->GetValue(thiss->m_CurrentParameters);
    // thread-safely save in parser variable:
    thiss->m_MetricMutex.Lock();
    thiss->m_Parser->SetScalarVariableValue(variable.c_str(), value);
    thiss->m_MetricMutex.Unlock();
  }
  else // derivative
  {
    // single evaluation
    DerivativeType deriv;
    metric->GetDerivative(thiss->m_CurrentParameters, deriv);
    // thread-safely save in parser variable:
    thiss->m_MetricMutex.Lock();
    std::ostringstream os;
    for (unsigned int d = 0; d < deriv.Size(); d++) // set all components
    {
      os.str("");
      os << variable << "[" << d << "]";
      thiss->m_Parser->SetScalarVariableValue(os.str().c_str(), deriv[d]);
    }
    thiss->m_MetricMutex.Unlock();
  }

  return ITK_THREAD_RETURN_VALUE;
}

template<class TFixedImage, class TMovingImage>
typename CompositeImageToImageMetric<TFixedImage, TMovingImage>::MeasureType CompositeImageToImageMetric<
    TFixedImage, TMovingImage>::GetValue(const ParametersType &parameters) const
{
  // extract the relevant metric indices:
  m_MetricIndices.clear();
  m_MetricIndices = this->ExtractReferencedVariableIndices(true);

  // copy current parameters ... so that they're available in event:
  m_CurrentParameters = parameters;
  this->InvokeEvent(BeforeEvaluationEvent());

  // at this point, the moving images must be already up to date!

  // re-initialize the relevant metrics on demand:
  if (m_ReInitializeMetricsBeforeEvaluating)
  {
    for (std::size_t i = 0; i < m_MetricIndices.size(); i++)
      m_InputMetrics[i]->Initialize();
  }

  MeasureType value = 0;
  // parallelized:
  if (m_UseOptimizedValueComputation && (m_NumberOfAvailableCPUs > 1
      || m_OverrideNumberOfAvailableCPUs > 0))
  {
    // set the parser rule and variables, evaluate the sub-metrics parallelized:
    m_Parser->RemoveAllVariables();
    m_Parser->SetFunction(m_ValueCompositeRule.c_str());
    m_ThreadedMetricEvaluationFlags.clear();
    for (std::size_t i = 0; i < m_MetricIndices.size(); i++) // initialize flags
      m_ThreadedMetricEvaluationFlags.push_back(false);
    // set up multi-threaded metric evaluation:
    MetricEvaluationThreadStruct metstruct;
    metstruct.CompositeMetric = this;
    metstruct.EvaluateValue = true;
    if (m_OverrideNumberOfAvailableCPUs <= 0)
      m_Threader->SetNumberOfThreads(m_NumberOfAvailableCPUs);
    else
      m_Threader->SetNumberOfThreads(m_OverrideNumberOfAvailableCPUs);
    m_Threader->SetSingleMethod(MetricEvaluationThreaderCallback, &metstruct);
    // multi-threaded execution (includes setting of variables); do as often
    // as needed:
    bool ready;
    do
    {
      m_Threader->SingleMethodExecute();
      ready = true;
      for (std::size_t i = 0; i < m_MetricIndices.size(); i++) // check flags
      {
        if (!m_ThreadedMetricEvaluationFlags[i])
          ready = false;
      }
    }
    while (!ready);
  }
  else // sequential implementation
  {
    // set the parser rule and variables, evaluate the sub-metrics sequentially:
    m_Parser->RemoveAllVariables();
    m_Parser->SetFunction(m_ValueCompositeRule.c_str());
    for (std::size_t i = 0; i < m_MetricIndices.size(); i++)
    {
      double partialResult = m_InputMetrics[m_MetricIndices[i]]->GetValue(
          parameters);
      m_Parser->SetScalarVariableValue(
          m_ValueVariables[m_MetricIndices[i]].c_str(), partialResult);
    }
  }
  // evaluate the rule:
  value = static_cast<MeasureType> (m_Parser->GetScalarResult());

  // make last value already available in event:
  m_LastValue = value;
  this->InvokeEvent(AfterEvaluationEvent());

  return value;
}

template<class TFixedImage, class TMovingImage>
void CompositeImageToImageMetric<TFixedImage, TMovingImage>::GetDerivative(
    const ParametersType &parameters, DerivativeType &derivative) const
{
  // extract the relevant metric indices:
  m_MetricIndices.clear();
  m_MetricIndices = this->ExtractReferencedVariableIndices(false);

  // copy current parameters ... so that they're available in event:
  m_CurrentParameters = parameters;
  this->InvokeEvent(BeforeEvaluationEvent());

  // at this point, the moving images must be already up to date!

  // re-initialize the relevant metrics on demand:
  if (m_ReInitializeMetricsBeforeEvaluating)
  {
    for (std::size_t i = 0; i < m_MetricIndices.size(); i++)
      m_InputMetrics[i]->Initialize();
  }

  // initialize:
  derivative = DerivativeType(parameters.Size());
  derivative.Fill(itk::NumericTraits<typename DerivativeType::ValueType>::Zero);

  // decompose rule:
  std::vector<std::string> derivativeRules;
  derivativeRules.clear();
  if (m_DerivativeCompositeRule.find("[x]") == std::string::npos) // with ###
  {
    std::size_t p = 0;
    std::size_t pold = 0;
    do
    {
      p = m_DerivativeCompositeRule.find("###", p + 1);
      if (p != std::string::npos)
      {
        derivativeRules.push_back(m_DerivativeCompositeRule.substr(pold, p
            - pold));
        pold = p + 3; // store
      }
      else if (pold < m_DerivativeCompositeRule.length())
      {
        derivativeRules.push_back(m_DerivativeCompositeRule.substr(pold));
      }
    } while (p != std::string::npos);

    if (derivativeRules.size() != parameters.Size()) // require match
      return;
  }
  else // generic (using [x], without ###-s)
  {
    unsigned int i = 0;
    std::string tmp;
    std::ostringstream os;
    while (i < parameters.Size()) // replace [x] with each sub-index
    {
      tmp = m_DerivativeCompositeRule;
      os.str("");
      os << "[" << i << "]";
      std::string::size_type p = 0;
      p = tmp.find("[x]");
      while (p != std::string::npos)
      {
        tmp.replace(p, 3, os.str());
        p = tmp.find("[x]", p + 1);
      }
      derivativeRules.push_back(tmp);
      i++;
    }
  }

  // parallelized:
  if (m_UseOptimizedDerivativeComputation && (m_NumberOfAvailableCPUs > 1
      || m_OverrideNumberOfAvailableCPUs > 0))
  {
    // set the parser rule and variables, evaluate the sub-metrics parallelized:
    m_Parser->RemoveAllVariables();
    m_ThreadedMetricEvaluationFlags.clear();
    for (std::size_t i = 0; i < m_MetricIndices.size(); i++) // initialize flags
      m_ThreadedMetricEvaluationFlags.push_back(false);
    // set up multi-threaded metric evaluation:
    MetricEvaluationThreadStruct metstruct;
    metstruct.CompositeMetric = this;
    metstruct.EvaluateValue = false; // derivatives!
    if (m_OverrideNumberOfAvailableCPUs <= 0)
      m_Threader->SetNumberOfThreads(m_NumberOfAvailableCPUs);
    else
      m_Threader->SetNumberOfThreads(m_OverrideNumberOfAvailableCPUs);
    m_Threader->SetSingleMethod(MetricEvaluationThreaderCallback, &metstruct);
    // multi-threaded execution (includes setting of variables); do as often
    // as needed:
    bool ready;
    do
    {
      m_Threader->SingleMethodExecute();
      ready = true;
      for (std::size_t i = 0; i < m_MetricIndices.size(); i++) // check flags
      {
        if (!m_ThreadedMetricEvaluationFlags[i])
          ready = false;
      }
    } while (!ready);
  }
  else // sequential implementation
  {
    // set the parser rule and variables, evaluate the sub-metrics sequentially:
    m_Parser->RemoveAllVariables();
    m_Parser->SetFunction(m_DerivativeCompositeRule.c_str());
    std::ostringstream os;
    for (std::size_t i = 0; i < m_MetricIndices.size(); i++)
    {
      m_InputMetrics[m_MetricIndices[i]]->GetDerivative(parameters, derivative);
      for (unsigned int j = 0; j < derivative.Size(); j++)
      {
        os.str("");
        os << m_DerivativeVariables[m_MetricIndices[i]] << "[" << j << "]";
        m_Parser->SetScalarVariableValue(os.str().c_str(), derivative[j]);
      }
    }
  }
  // evaluate the sub-rules:
  for (std::size_t i = 0; i < derivativeRules.size(); i++)
  {
    m_Parser->SetFunction(derivativeRules[i].c_str());
    derivative[i]
        = static_cast<typename DerivativeType::ValueType> (m_Parser->GetScalarResult());
  }

  // make last derivative available in event:
  m_LastDerivative = derivative;
  this->InvokeEvent(AfterEvaluationEvent());
}

template<class TFixedImage, class TMovingImage>
bool CompositeImageToImageMetric<TFixedImage, TMovingImage>::AddMetricInput(
    Superclass *metric, std::string valueVariable,
    std::string derivativeVariable)
{
  if (metric && valueVariable.length() > 0 && derivativeVariable.length() > 0)
  {
    m_InputMetrics.push_back(metric);
    m_ValueVariables.push_back(valueVariable);
    m_DerivativeVariables.push_back(derivativeVariable);
    return true;
  }
  else
  {
    return false;
  }
}

template<class TFixedImage, class TMovingImage>
std::size_t CompositeImageToImageMetric<TFixedImage, TMovingImage>::GetNumberOfMetricInputs()
{
  return m_InputMetrics.size();
}

template<class TFixedImage, class TMovingImage>
typename CompositeImageToImageMetric<TFixedImage, TMovingImage>::SuperclassPointer CompositeImageToImageMetric<
    TFixedImage, TMovingImage>::GetIthMetricInput(std::size_t i)
{
  if (i < GetNumberOfMetricInputs())
    return m_InputMetrics[i];
  else
    return NULL;
}

template<class TFixedImage, class TMovingImage>
std::string CompositeImageToImageMetric<TFixedImage, TMovingImage>::GetIthValueVariable(
    std::size_t i)
{
  if (i < GetNumberOfMetricInputs())
    return m_ValueVariables[i];
  else
    return "";
}

template<class TFixedImage, class TMovingImage>
std::string CompositeImageToImageMetric<TFixedImage, TMovingImage>::GetIthDerivativeVariable(
    std::size_t i)
{
  if (i < GetNumberOfMetricInputs())
    return m_DerivativeVariables[i];
  else
    return "";
}

template<class TFixedImage, class TMovingImage>
bool CompositeImageToImageMetric<TFixedImage, TMovingImage>::RemoveIthMetricInputAndVariables(
    std::size_t i)
{
  if (i < GetNumberOfMetricInputs())
  {
    m_InputMetrics[i] = NULL;
    m_InputMetrics.erase(m_InputMetrics.begin() + i);
    m_ValueVariables.erase(m_ValueVariables.begin() + i);
    m_DerivativeVariables.erase(m_DerivativeVariables.begin() + i);
    return true;
  }
  else
  {
    return false;
  }
}

template<class TFixedImage, class TMovingImage>
void CompositeImageToImageMetric<TFixedImage, TMovingImage>::RemoveAllMetricInputsAndVariables()
{
  for (std::size_t i = 0; i < GetNumberOfMetricInputs(); i++)
    m_InputMetrics[i] = NULL;
  m_InputMetrics.clear();
  m_ValueVariables.clear();
  m_DerivativeVariables.clear();
}

template<class TFixedImage, class TMovingImage>
bool CompositeImageToImageMetric<TFixedImage, TMovingImage>::IsVariableNameValid(
    std::string variable) const
{
  if (variable.length() > 0)
  {
    std::size_t c = 0;
    std::locale loc;
    for (std::size_t i = 0; i < variable.length(); i++)
      if (std::isalnum(variable[i], loc))
        c++;
    bool alphaNumeric = (c == variable.size());

    if (!alphaNumeric) // allow "_"
    {
      c = 0;
      for (std::size_t i = 0; i < variable.length(); i++)
        if (std::isalnum(variable[i], loc) || variable[i] == '_')
          c++;
      alphaNumeric = (c == variable.size());
    }

    return alphaNumeric;
  }
  else
  {
    return false;
  }
}

template<class TFixedImage, class TMovingImage>
std::vector<std::size_t> CompositeImageToImageMetric<TFixedImage, TMovingImage>::ExtractReferencedVariableIndices(
    bool values) const
{
  std::vector<std::size_t> indices;
  std::string rule = m_ValueCompositeRule;
  std::vector<std::string> *vars = &m_ValueVariables;

  if (!values)
  {
    rule = m_DerivativeCompositeRule;
    vars = &m_DerivativeVariables;
  }

  for (std::size_t i = 0; i < vars->size(); i++) // locate variables in rule
  {
    std::size_t p = 0;
    int x = 0;
    do
    {
      if (x == 0)
        p = rule.find((*vars)[i]);
      else
        p = rule.find((*vars)[i], p + 1);
      x++;
      if (p != std::string::npos)
      {
        // check whether or not it is really this variable:
        bool frontOK = true;
        std::string s = "-";
        if (p > 0)
        {
          s[0] = rule[p - 1];
          if (IsVariableNameValid(s))
            frontOK = false;
        }
        bool backOK = true;
        if (p < (rule.length() - (*vars)[i].length()))
        {
          s[0] = rule[p + (*vars)[i].length()];
          if (IsVariableNameValid(s))
            backOK = false;
        }
        if (backOK && frontOK) // variable found in rule
        {
          indices.push_back(i);
          break;
        }
      }
    } while (p != std::string::npos);
  }

  return indices;
}

template<class TFixedImage, class TMovingImage>
void CompositeImageToImageMetric<TFixedImage, TMovingImage>::Initialize()
    throw (itk::ExceptionObject)
{
  ;
}

}

#endif /* ORACOMPOSITEIMAGETOIMAGEMETRIC_TXX_ */
