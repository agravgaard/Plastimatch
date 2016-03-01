//
#ifndef ORAMULTIIMAGETOIMAGEMETRIC_H_
#define ORAMULTIIMAGETOIMAGEMETRIC_H_

#include <itkSmartPointer.h>
#include <itkObjectFactory.h>
#include <itkObject.h>
#include <itkMultipleValuedCostFunction.h>
#include <itkImageToImageMetric.h>

#include <vector>

namespace ora
{

/** \class MultiImageToImageMetric
 * \brief A multi-metric with n input metrics that are connected to multiple
 * outputs.
 *
 * FIXME: class description
 *
 * FIXME: does not support multi-threading
 *
 * @see BeforeMetricEvaluationEvent
 * @see AfterMetricEvaluationEvent
 *
 * <b>Tests</b>:<br>
 * TestMultiImageToImageMetric.cxx <br>
 * TestMultiResolutionNWay2D3DRegistrationMethod.cxx
 *
 * @author phil 
 * @version 1.1
 *
 * \ingroup RegistrationMetrics
 */
template<class TFixedImage, class TMovingImage>
class MultiImageToImageMetric :
    public itk::MultipleValuedCostFunction
{
public:
  /** Standard class typedefs. */
  typedef MultiImageToImageMetric Self;
  typedef itk::MultipleValuedCostFunction Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(MultiImageToImageMetric, MultipleValuedCostFunction)

  /** Method for creation through the object factory. */
  itkNewMacro(Self)

  /** Derived superclass types. **/
  typedef Superclass::DerivativeType DerivativeType;
  typedef Superclass::MeasureType MeasureType;
  typedef Superclass::ParametersType ParametersType;
  typedef Superclass::ParametersValueType ParametersValueType;

  /** Image metric type. **/
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> BaseMetricType;
  typedef typename BaseMetricType::Pointer BaseMetricPointer;

  /**
   * This method returns the value of the cost function corresponding
   * to the specified parameters.
   * @see itk::MultipleValuedCostFunction::GetValue()
   */
  virtual MeasureType GetValue(const ParametersType &parameters) const;

  /**
   * This method returns the derivative of the cost function corresponding
   * to the specified parameters.
   * @see itk::MultipleValuedCostFunction::GetDerivative()
   */
  virtual void GetDerivative(const ParametersType &parameters,
      DerivativeType &derivative) const;

  /**
   * @return the number of values that are computed by the multivalued cost
   * function.
   * @see itk::MultipleValuedCostFunction::GetNumberOfValues()
   */
  virtual unsigned int GetNumberOfValues() const;

  /**
   * @return the number of parameters that are used to specify the cost function
   * position.
   * @see itk::MultipleValuedCostFunction::GetNumberOfParameters()
   */
  virtual unsigned int GetNumberOfParameters() const;

  /**
   * Add an input metric input to this multi-metric's internal metric list (at
   * the end). This metric will be mapped to the i-th output of the metric.
   * @param metric any valid image to image metric; NOTE: however it is possible
   * to add the same metric input multiple times (there is no replace mechanism
   * integrated!)
   * @return TRUE if the metric could be added successfully
   */
  virtual bool AddMetricInput(BaseMetricType *metric);
  /** @return number of inputs (image to image metrics) **/
  std::size_t GetNumberOfMetricInputs() const;
  /** @return the i-th input metric. NULL if i is out of input range. **/
  BaseMetricType *GetIthMetricInput(std::size_t i);
  /**
   * Remove the i-th metric input from internal list.
   * @return TRUE if the metric input could be deleted
   */
  bool RemoveIthMetricInput(std::size_t i);
  /** Remove all metric inputs. **/
  void RemoveAllMetricInputs();

  itkSetMacro(ReInitializeMetricsBeforeEvaluating, bool)
  itkGetMacro(ReInitializeMetricsBeforeEvaluating, bool)

  /**
   * Get current parameters to be evaluated (values/derivatives). This may be
   * especially useful in BeforeEvaluationEvent().
   **/
  itkGetMacro(CurrentParameters, ParametersType)
  /**
   * Set current parameters to be evaluated (values/derivatives). This may be
   * especially useful in BeforeEvaluationEvent().
   **/
  itkSetMacro(CurrentParameters, ParametersType)

  /**
   * Get last stored multi-metric values. Already available in
   * AfterEvaluationEvent().
   **/
  itkGetMacro(LastValues, MeasureType)
  /**
   * Get last stored multi-metric derivatives. Already available in
   * AfterEvaluationEvent().
   **/
  itkGetMacro(LastDerivatives, DerivativeType)

  /**
   * This method does exactly nothing as the composited sub-metrics are
   * are initialized individually.
   */
  virtual void Initialize() throw (itk::ExceptionObject);

protected:
  /** vector of input metrics that potentially contribute to the output **/
  std::vector<BaseMetricPointer> m_InputMetrics;
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
  /** Current multi-metric evaluation parameters. **/
  mutable ParametersType m_CurrentParameters;
  /** Last stored multi-metric values. **/
  mutable MeasureType m_LastValues;
  /** Last stored multi-metric derivatives. **/
  mutable DerivativeType m_LastDerivatives;

  /** Default constructor **/
  MultiImageToImageMetric();
  /** Destructor **/
  virtual ~MultiImageToImageMetric();

  /** Print-out object information. **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  /** Purposely not implemented **/
  MultiImageToImageMetric(const Self&);
  /** Purposely not implemented **/
  void operator=(const Self&);

};

}

#include "oraMultiImageToImageMetric.txx"

#endif /* ORAMULTIIMAGETOIMAGEMETRIC_H_ */
