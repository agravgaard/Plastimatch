//
#include "BasicUnitTestIncludes.hxx"

#include "itkAmoebaOptimizerEx.h"

#include <vnl/vnl_vector_fixed.h>
#include <vnl/vnl_vector.h>
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_math.h>

/**
 * NOTE: Taken over from ITK test.
 *
 *  The objective function is the quadratic form:
 *
 *  1/2 x^T A x - b^T x
 *
 *  Where A is represented as an itkMatrix and
 *  b is represented as a itkVector
 *
 *  The system in this example is:
 *
 *     | 3  2 ||x|   | 2|   |0|
 *     | 2  6 ||y| + |-8| = |0|
 *
 *
 *   the solution is the vector | 2 -2 |
 *
 *   and the expected final value of the function is 10.0
 *
 */
class AmoebaCostFunction : public itk::SingleValuedCostFunction
{
public:

  typedef AmoebaCostFunction                    Self;
  typedef itk::SingleValuedCostFunction     Superclass;
  typedef itk::SmartPointer<Self>           Pointer;
  typedef itk::SmartPointer<const Self>     ConstPointer;
  itkNewMacro( Self );
  itkTypeMacro( AmoebaCostFunction, SingleValuedCostFunction );

  enum { SpaceDimension=2 };

  typedef Superclass::ParametersType              ParametersType;
  typedef Superclass::DerivativeType              DerivativeType;
  typedef Superclass::MeasureType                 MeasureType;

  typedef vnl_vector<double>                      VectorType;
  typedef vnl_matrix<double>                      MatrixType;


  AmoebaCostFunction():m_A(SpaceDimension,SpaceDimension),m_b(SpaceDimension)
  {
    m_A[0][0] =  3;
    m_A[0][1] =  2;
    m_A[1][0] =  2;
    m_A[1][1] =  6;

    m_b[0]    =  2;
    m_b[1]    = -8;
    m_Negate = false;
  }

  double GetValue( const ParametersType & parameters ) const
  {

    VectorType v( parameters.Size() );
    for(unsigned int i=0; i<SpaceDimension; i++)
    {
      v[i] = parameters[i];
    }
    VectorType Av = m_A * v;
    double val = ( inner_product<double>( Av , v ) )/2.0;
    val -= inner_product< double >( m_b , v );
    if( m_Negate )
    {
      val *= -1.0;
    }
    return val;
  }

  void GetDerivative( const ParametersType & parameters,
                      DerivativeType & derivative ) const
  {

    VectorType v( parameters.Size() );
    for(unsigned int i=0; i<SpaceDimension; i++)
    {
      v[i] = parameters[i];
    }
    std::cout << "GetDerivative( " << v << " ) = ";
    VectorType gradient = m_A * v  - m_b;
    std::cout << gradient << std::endl;
    derivative = DerivativeType(SpaceDimension);
    for(unsigned int i=0; i<SpaceDimension; i++)
    {
      if( !m_Negate )
      {
        derivative[i] = gradient[i];
      }
      else
      {
        derivative[i] = -gradient[i];
      }
    }
  }

  unsigned int GetNumberOfParameters(void) const
  {
    return SpaceDimension;
  }

  // Used to switch between maximization and minimization.
  void SetNegate(bool flag )
  {
    m_Negate = flag;
  }

private:
  MatrixType        m_A;
  VectorType        m_b;
  bool              m_Negate;
};

/**
 * NOTE: Taken over from ITK test.
 * Iteration event of the optimizer.
 */
class CommandIterationUpdateAmoeba : public itk::Command
{
public:
  typedef  CommandIterationUpdateAmoeba   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );
  unsigned long GetNumberOfIterations()
  {
    return m_IterationNumber;
  }
  bool StopAfter5Iterations;
protected:
  CommandIterationUpdateAmoeba()
  {
    m_IterationNumber=0;
    StopAfter5Iterations = false;
  }
public:
  typedef itk::AmoebaOptimizerEx OptimizerType;
  typedef OptimizerType* OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    OptimizerPointer optimizer = dynamic_cast<OptimizerPointer>(caller);
    if (m_FunctionEvent.CheckEvent(&event))
    {
      m_IterationNumber++;
      optimizer->GetCachedValue();
      optimizer->GetCachedCurrentPosition();
      optimizer->GetStopConditionDescription();
      if (StopAfter5Iterations && m_IterationNumber >= 5)
      {
        optimizer->StopOptimization(); // stop requested!
      }
    }
    else if( m_GradientEvent.CheckEvent( &event ) )
    {
      optimizer->GetCachedDerivative();
    }
  }
  void Execute(const itk::Object *caller, const itk::EventObject & event)
  {
    ;
  }
private:
  unsigned long m_IterationNumber;

  itk::FunctionEvaluationIterationEvent m_FunctionEvent;
  itk::GradientEvaluationIterationEvent m_GradientEvent;
};

/**
 * Tests base functionality of:
 *
 *   itk::AmoebaOptimizerEx and vnl_amoeba_ex.
 *
 * NOTE: Large parts of this test were taken over from the original ITK test
 * (itkAmoebaOptimizerTest.cxx) and fitted into our testing framework.
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see itk::AmoebaOptimizerEx
 * @see vnl_amoeba_ex
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL, helpLines);
    return EXIT_SUCCESS;
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting AmoebaOptimizerEx.\n")

  VERBOSE(<< "  * Core Amoeba-functionality ... ")
  lok = true; // initialize sub-section's success state
  typedef  itk::AmoebaOptimizerEx  OptimizerType;
  typedef  OptimizerType::InternalOptimizerType  vnlOptimizerType;
  OptimizerType::Pointer  itkOptimizer = OptimizerType::New();
  // set optimizer parameters
  itkOptimizer->SetMaximumNumberOfIterations(10);
  double xTolerance = 0.01;
  itkOptimizer->SetParametersConvergenceTolerance(xTolerance);
  double fTolerance = 0.001;
  itkOptimizer->SetFunctionConvergenceTolerance(fTolerance);
  AmoebaCostFunction::Pointer costFunction = AmoebaCostFunction::New();
  itkOptimizer->SetCostFunction(costFunction.GetPointer());
  vnlOptimizerType *vnlOptimizer = itkOptimizer->GetOptimizer();
  OptimizerType::ParametersType initialValue(2);       // constructor requires vector size
  initialValue[0] = 100; // We start not far from  | 2 -2 |
  initialValue[1] = -100;
  OptimizerType::ParametersType currentValue(2);
  currentValue = initialValue;
  itkOptimizer->SetInitialPosition( currentValue );
  try
  {
    vnlOptimizer->verbose = false;
    itkOptimizer->StartOptimization();
    itkOptimizer->SetMaximumNumberOfIterations(100);
    itkOptimizer->SetInitialPosition(itkOptimizer->GetCurrentPosition());
    itkOptimizer->StartOptimization();
  }
  catch( itk::ExceptionObject & e )
  {
    lok = false;
  }
  // check results:
  OptimizerType::ParametersType finalPosition;
  finalPosition = itkOptimizer->GetCurrentPosition();
  double trueParameters[2] = { 2, -2 };
  for (unsigned int j = 0; j < 2; j++)
  {
    if (vnl_math_abs(finalPosition[j] - trueParameters[j]) > xTolerance)
      lok = false;
  }
  // Get the final value of the optimizer
  OptimizerType::MeasureType finalValue = itkOptimizer->GetValue();
  if (vcl_fabs(finalValue+9.99998) > 0.01)
    lok = false;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Core Amoeba-functionality (maximization) ... ")
  lok = true; // initialize sub-section's success state
  { // add a block-scope to have local variables
    currentValue = initialValue;
    itkOptimizer->SetInitialPosition(currentValue);
    CommandIterationUpdateAmoeba::Pointer observer =
      CommandIterationUpdateAmoeba::New();
    itkOptimizer->AddObserver(itk::IterationEvent(), observer);
    itkOptimizer->AddObserver(itk::FunctionEvaluationIterationEvent(), observer);
    try
    {
      // These two following statement should compensate each other
      // and allow us to get to the same result as the test above.
      costFunction->SetNegate(true);
      itkOptimizer->MaximizeOn();
      itkOptimizer->StartOptimization();
      itkOptimizer->SetMaximumNumberOfIterations(100);
      itkOptimizer->SetInitialPosition( itkOptimizer->GetCurrentPosition() );
      itkOptimizer->StartOptimization();
    }
    catch( itk::ExceptionObject & e )
    {
      lok = false;
    }
    finalPosition = itkOptimizer->GetCurrentPosition();
    for( unsigned int j = 0; j < 2; j++ )
    {
      if (vnl_math_abs(finalPosition[j] - trueParameters[j]) > xTolerance)
        lok = false;
    }
    finalValue = itkOptimizer->GetValue();
    if (vcl_fabs(finalValue+9.99998) > 0.01)
      lok = false;
  }
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Amoeba stopping capability ... ")
  lok = true; // initialize sub-section's success state
  { // add a block-scope to have local variables
    itkOptimizer->RemoveAllObservers();
    currentValue = initialValue;
    itkOptimizer->SetInitialPosition(currentValue);
    CommandIterationUpdateAmoeba::Pointer observer =
      CommandIterationUpdateAmoeba::New();
    observer->StopAfter5Iterations = true; // STOP!
    itkOptimizer->AddObserver(itk::FunctionEvaluationIterationEvent(), observer);
    try
    {
      // These two following statement should compensate each other
      // and allow us to get to the same result as the test above.
      costFunction->SetNegate(true);
      itkOptimizer->MaximizeOn();
      itkOptimizer->StartOptimization();
      itkOptimizer->SetMaximumNumberOfIterations(100);
      itkOptimizer->SetInitialPosition( itkOptimizer->GetCurrentPosition() );
      itkOptimizer->StartOptimization();
    }
    catch( itk::ExceptionObject & e )
    {
      lok = false;
    }
    if (observer->GetNumberOfIterations() > 10) // some tolerance
      lok = false;
    if (itkOptimizer->GetStopConditionDescription() != "Manual stop")
      lok = false;
  }
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "Test result: ")
  if (ok)
  {
    VERBOSE(<< "OK\n\n")
    return EXIT_SUCCESS;
  }
  else
  {
    VERBOSE(<< "FAILURE\n\n")
    return EXIT_FAILURE;
  }
}
