//
#ifndef ORACOMPOSITEIMAGETOIMAGEMETRIC_H_
#define ORACOMPOSITEIMAGETOIMAGEMETRIC_H_

#include <itkSmartPointer.h>
#include <itkObjectFactory.h>
#include <itkImageToImageMetric.h>
#include <itkMultiThreader.h>
#include <itkSimpleFastMutexLock.h>
#include <itkImageFileWriter.h>

#include <vtkSmartPointer.h>
#include <vtkFunctionParser.h>

#include <vector>

namespace ora
{

/** \class CompositeImageToImageMetric
 * \brief A composite metric with n input metrics that are connected to a
 * single output.
 *
 * FIXME: class description
 * 
 * @see ora::BeforeMetricEvaluationEvent
 * @see ora::AfterMetricEvaluationEvent
 *
 * <b>Tests</b>:<br>
 * TestCompositeImageToImageMetric.cxx <br>
 * TestMultiResolutionNWay2D3DRegistrationMethod.cxx
 *
 * @author phil 
 * @author Markus 
 * @version 1.3.1
 *
 * \ingroup RegistrationMetrics
 */
template<class TFixedImage, class TMovingImage>
class CompositeImageToImageMetric :
    public itk::ImageToImageMetric<TFixedImage, TMovingImage>
{
public:
  /** Standard class typedefs. */
  typedef CompositeImageToImageMetric Self;
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(CompositeImageToImageMetric, ImageToImageMetric)

  /** Method for creation through the object factory. */
  itkNewMacro(Self)

  /** Inherited types. **/
  typedef typename Superclass::Pointer SuperclassPointer;
  typedef typename Superclass::ParametersType ParametersType;
  typedef typename Superclass::MeasureType MeasureType;
  typedef typename Superclass::DerivativeType DerivativeType;

  /**
   * Return the cost function value for the specified parameters. This method
   * composes the values of the sub-metrics according to the computation rules.
   * @see SetValueCompositeRule()
   **/
  virtual MeasureType GetValue(const ParametersType &parameters) const;

  /**
   * Return the cost function derivative for the specified parameters. This
   * method composes the derivatives of the sub-metrics according to the
   * computation rules.
   * @see SetDerivativeCompositeRule()
   **/
  virtual void GetDerivative(const ParametersType &parameters,
      DerivativeType &derivative) const;

  /**
   * Set the mathematical description of the n-way input to output mapping for
   * the metric value (scalar). The rule is expressed as a simple formula where
   * the specified variable names for the inputs' metric values can be used.
   * <br>For example: (assumed that there are 3 input-metrics m1,m2,m3) <br>
   * "0.5 * m1 + 0.7 * m2 + 1.0 * m3" (the composite metric's output will
   * be computed by summing the weighted source metric values). <br>
   * Furthermore the following operators and symbols can be used: <br>
   * +,-,*,/,log,sin,cos,(,)... <br>
   * if (qery,true-cmd,false-cmd) <br>
   * @see vtkFunctionParser
   **/
  itkSetMacro(ValueCompositeRule, std::string)
  itkGetMacro(ValueCompositeRule, std::string)

  /**
   * Set the mathematical description of the n-way input to output mapping for
   * the metric derivative (vector). The rule is expressed as a simple formula
   * where the specified variable names for the inputs' metric derivatives can
   * be used.
   * <br>For example: (assumed that there are 2 input-metrics with 3-dimensional
   * derivatives d1,d2) <br>
   * "d1[0] + d2[0] ### d1[1] + d2[1] ### d1[2] + d2[2]" is equivalent to
   * "d1[x] + d2[x]"
   * (the composite metric's output will be computed by summing the source
   * metric derivative components). <br>
   * There are two ways of specifying the output formula: <br>
   * The single component formulas can be separated by "###" sequences. Each
   * component of the derivative variables can be accessed by [] operator. If
   * "###" sequences are used there must be a sub-formula for each output
   * component. <br>
   * Another way of specifying the output formula is suitable for situations
   * where each component is computed equivalently (as in the example above).
   * In this case there must not be any "###" sequences. The generic components
   * are marked by "[x]" patterns. However explicit components can be used, e.g.
   * "d1[x] * d[0] + d2[x]" (whyever one needs to do that). <br>
   * Furthermore in both types of formulas the following operators and symbols
   * can be used: <br>
   * +,-,*,/,log,sin,cos,(,)... <br>
   * if (qery,true-cmd,false-cmd) <br>
   * @see vtkFunctionParser
   **/
  itkSetMacro(DerivativeCompositeRule, std::string)
  itkGetMacro(DerivativeCompositeRule, std::string)

  /**
   * Add an input metric input to this composite metric. NOTE: variable naming
   * restrictions are: <br>
   * - only alphanumeric names (a-z,A-Z,0-9) <br>
   * - underscore (_) is allowed in variable strings <br>
   * - names are case-sensitive <br>
   * - IMPLICIT restriction: you should not name your variables similar to
   * built-in functions/operators (e.g. log, sin ...); this is not checked here,
   * but will result in run-time exceptions later (during evaluation) ...
   * @param metric any valid image to image metric; NOTE: however it is possible
   * to add the same metric input multiple times (there is no replace mechanism
   * integrated!)
   * @param valueVariable string descriptor for the input metric's cost function
   * value that can be used for specifying the value composite rule later;
   * must not be empty
   * @param derivativeVariable string descriptor for the input metric's cost
   * function derivative that can be used for specifying the derivative
   * composite rule later; must not be empty
   * @return TRUE if the metric could be added successfully
   * @see SetValueCompositeRule()
   * @see SetDerivativeCompositeRule()
   */
  virtual bool AddMetricInput(Superclass *metric, std::string valueVariable,
      std::string derivativeVariable);
  /**
   * @return number of inputs (image to image metrics); equals the number of
   * value variables and derivative variables
   **/
  std::size_t GetNumberOfMetricInputs();
  /** @return the i-th input metric. NULL if i is out of input range. **/
  SuperclassPointer GetIthMetricInput(std::size_t i);
  /** @return the i-th value variable (string descriptor) **/
  std::string GetIthValueVariable(std::size_t i);
  /** @return the i-th derivative variable (string descriptor) **/
  std::string GetIthDerivativeVariable(std::size_t i);
  /**
   * Remove the i-th metric input and the according variables (value,
   * derivative).
   * @return TRUE if the metric input and variables could be deleted
   */
  bool RemoveIthMetricInputAndVariables(std::size_t i);
  /** Remove all metric inputs and the according variables (value, deriv.) **/
  void RemoveAllMetricInputsAndVariables();

  itkSetMacro(UseOptimizedValueComputation, bool)
  itkGetMacro(UseOptimizedValueComputation, bool)

  itkSetMacro(UseOptimizedDerivativeComputation, bool)
  itkGetMacro(UseOptimizedDerivativeComputation, bool)

  /**
   * Extract the variables indices (equals metric indices) found in a
   * computation rule (values or derivatives). The indices directly correlate
   * with the current vector of metrics/variables. The extracted indices are
   * important for cost function evaluation as the returned vector specifies
   * which of the sub-metrics must be evaluated to enable composite output.
   * @param values if TRUE the value composition rule is analyzed, otherwise
   * the derivative composition rule is considered.
   * @return the vector of referenced variable indices (NOTE: the indices are
   * sorted ascending)
   **/
  virtual std::vector<std::size_t>
  ExtractReferencedVariableIndices(bool values) const;

  itkSetMacro(ReInitializeMetricsBeforeEvaluating, bool)
  itkGetMacro(ReInitializeMetricsBeforeEvaluating, bool)

  itkSetMacro(OverrideNumberOfAvailableCPUs, int)
  itkGetMacro(OverrideNumberOfAvailableCPUs, int)

  /**
   * Get current parameters to be evaluated (value/derivative). This may be
   * especially useful in BeforeEvaluationEvent().
   **/
  itkGetMacro(CurrentParameters, ParametersType)
  /**
   * Set current parameters to be evaluated (value/derivative). This may be
   * especially useful in BeforeEvaluationEvent().
   **/
  itkSetMacro(CurrentParameters, ParametersType)

  /**
   * Get last stored composite value. Already available in
   * AfterEvaluationEvent().
   **/
  itkGetMacro(LastValue, MeasureType)
  /**
   * Get last stored composite derivative. Already available in
   * AfterEvaluationEvent().
   **/
  itkGetMacro(LastDerivative, DerivativeType)

  /**
   * This method does exactly nothing as the composited sub-metrics are
   * are initialized individually.
   */
  virtual void Initialize() throw (itk::ExceptionObject);

protected:
  /** Type of internal rule parser. **/
  typedef vtkSmartPointer<vtkFunctionParser> ParserPointer;
  /** Types for threading. **/
  typedef itk::MultiThreader ThreaderType;
  typedef ThreaderType::Pointer ThreaderPointer;
  typedef itk::SimpleFastMutexLock MutexType;

  /**
   * Internal structure used for connecting this class with the threading
   * library.
   */
  struct MetricEvaluationThreadStruct
  {
    ConstPointer CompositeMetric; /** pointer to composite metric (this) **/
    bool EvaluateValue; /** TRUE=value, FALSE=derivative **/
  };

  /**
   * parser for mathematically connecting the input metrics (values and
   * derivatives)
   **/
  ParserPointer m_Parser;
  /**
   * mathematical description of the n-way input to output mapping for the
   * metric value (scalar)
   **/
  std::string m_ValueCompositeRule;
  /**
   * mathematical description of the n-way input to output mapping for the
   * metric derivative (vector)
   **/
  std::string m_DerivativeCompositeRule;
  /** vector of input metrics that potentially contribute to the output **/
  std::vector<SuperclassPointer> m_InputMetrics;
  /** vector of value variables (string descriptors) w.r.t. input order **/
  mutable std::vector<std::string> m_ValueVariables;
  /** vector of derivative variables (string descriptors) w.r.t. input order **/
  mutable std::vector<std::string> m_DerivativeVariables;
  /**
   * flag determining whether or not to use optimized (CPU-parallelized)
   * computation of the composite metric value <br>
   * NOTE: using optimized value computation requires the metric's method
   * GetValue() to be thread-safe (it must be callable by several threads
   * at the same time)!
   **/
  bool m_UseOptimizedValueComputation;
  /**
   * flag determining whether or not to use optimized (CPU-parallelized)
   * computation of the composite metric derivative <br>
   * NOTE: using optimized derivative computation requires the metric's method
   * GetValue() to be thread-safe (it must be callable by several threads
   * at the same time)! This is generally not satisfied by metrics that
   * internally use the connected transform's Jacobian for derivative estimation
   * (the implementation of GetJacobian() is not thread-safe - returns reference
   * to internal member)!
   **/
  bool m_UseOptimizedDerivativeComputation;
  /** dynamically detected number of available CPUs **/
  int m_NumberOfAvailableCPUs;
  /**
   * manual override of available number of CPUs (if <= 0, the real number of
   * available CPUs is used)
   **/
  int m_OverrideNumberOfAvailableCPUs;
  /**
   * flag determining whether or not the sub-metrics should be re-initialized
   * before each evaluation (value and derivative evaluation)
   * \warning Activating this flag can potentially slow down performance
   * depending on the code of a metric's Initialize() method; it may be more
   * useful to place the pure code that is needed for updating a metric's state
   * manually by using ora::BeforeEvaluationEvent.
   * @see ora::BeforeMetricEvaluationEvent
   **/
  bool m_ReInitializeMetricsBeforeEvaluating;
  /** temporary helper holding indices of metrics to be evaluated **/
  mutable std::vector<std::size_t> m_MetricIndices;
  /** helper flags for threaded metric evaluation **/
  mutable std::vector<bool> m_ThreadedMetricEvaluationFlags;
  /** multi-threader **/
  ThreaderPointer m_Threader;
  /** mutex for metric-selection **/
  MutexType m_MetricMutex;
  /** helper parameters for threading **/
  mutable ParametersType m_CurrentParameters;
  /** Last stored composite value. **/
  mutable MeasureType m_LastValue;
  /** Last stored composite derivative. **/
  mutable DerivativeType m_LastDerivative;

  /** Default constructor **/
  CompositeImageToImageMetric();
  /** Destructor **/
  virtual ~CompositeImageToImageMetric();

  /** Print-out object information. **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Check whether or not the specified variable name is valid.
   * @param variable the variable name to be checked; Rules: <br>
   * - only alphanumeric names (a-z,A-Z,0-9) <br>
   * - underscore (_) is allowed in variable strings <br>
   * - names are case-sensitive <br>
   * @return TRUE if the variable is valid and can be added to the internal list
   */
  virtual bool IsVariableNameValid(std::string variable) const;

  /**
   * Static function used as a "callback" by the MultiThreader. The threading
   * library will call this routine for each thread, which will evaluate the
   * sub-metrics (value or gradient) and set the variables in the parser.
   * @param arg ThreadInfoStruct with a pointer to a
   * MetricEvaluationThreadStruct as user data is awaited
   * @see MetricEvaluationThreadStruct
   * @see m_Threader
   * @see m_MetricMutex
   * @see m_ThreadedMetricEvaluationFlags
   */
  static ITK_THREAD_RETURN_TYPE MetricEvaluationThreaderCallback(void *arg);

  typedef itk::ImageFileWriter<TMovingImage> WriterType;
  typedef typename WriterType::Pointer WriterPointer;

private:
  /** Purposely not implemented **/
  CompositeImageToImageMetric(const Self&);
  /** Purposely not implemented **/
  void operator=(const Self&);

};

}

#include "oraCompositeImageToImageMetric.txx"

#endif /* ORACOMPOSITEIMAGETOIMAGEMETRIC_H_ */
