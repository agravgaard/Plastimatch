//
#ifndef ORAREG23SPARSEREGISTRATIONEXECUTIONTASK_H_
#define ORAREG23SPARSEREGISTRATIONEXECUTIONTASK_H_

#include "oraREG23RegistrationExecutionTask.h"

#include <itkObject.h>
#include <itkEventObject.h>

namespace ora
{

// forward declaration:
class REG23Model;

/**
 * // FIXME:
 *
 * @see REG23RegistrationExecutionTask
 *
 * @author phil
 * @author Markus
 * @version 1.2
 */
class REG23SparseRegistrationExecutionTask : public REG23RegistrationExecutionTask
{
public:
  /** Default constructor **/
  REG23SparseRegistrationExecutionTask();
  /** Destructor **/
  virtual ~REG23SparseRegistrationExecutionTask();

 /** Executes registration.
   * @return TRUE if the execution succeeded
   * @see Task#Execute()
   */
  virtual bool Execute();

  /**
   * The name of the task is "Registration".
   * @see Task#GetName()
   */
  virtual QString GetName();

  /** The number of iterations (only available AFTER Execute()!) **/
  virtual int GetNumberOfIterations()
  {
    return m_NumberOfIterations;
  }

protected:
  /** The number of iterations (only available AFTER Execute()!) **/
  int m_NumberOfIterations;
  /** Helper for ITK-based observer progress **/
  double m_CurrentProgressStart;
  /** Helper for ITK-based observer progress **/
  double m_CurrentProgressSpan;
  /** Helper for ITK-based observer progress (TRUE=EXECUTE, FALSE=UNEXECUTE) **/
  bool m_CurrentProgressDirection;

  /** Entry-point for internal ITK filter observer. **/
  void OnInternalFilterProgressReceptor(itk::Object *caller,
      const itk::EventObject &event);

};


}


#endif /* ORAREG23SPARSEREGISTRATIONEXECUTIONTASK_H_ */
