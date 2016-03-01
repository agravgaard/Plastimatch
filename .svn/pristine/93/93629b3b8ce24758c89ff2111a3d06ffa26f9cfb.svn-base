//
#ifndef ORAREG23REGISTRATIONEXECUTIONTASK_H_
#define ORAREG23REGISTRATIONEXECUTIONTASK_H_

// ORAIFModel
#include <oraTask.h>

#include <itkObject.h>
#include <itkEventObject.h>

namespace ora
{

// forward declaration:
class REG23Model;

/**
 * // FIXME:
 *
 * @see Task
 *
 * @author phil
 * @author Markus
 * @version 1.2
 */
class REG23RegistrationExecutionTask : public Task
{
public:
  /** Default constructor **/
  REG23RegistrationExecutionTask();
  /** Destructor **/
  virtual ~REG23RegistrationExecutionTask();

  /**
   * This implementation does not support unexecution.
   * @see Task#IsUnexecutable()
   */
  virtual bool IsUnexecutable() const
  {
    return false;
  }

  /**
   * This task can be reexecuted. Reexecuting means that the registration
   * is newly executed.
   * @see Task#IsReexecuteable()
   */
  virtual bool IsReexecuteable() const
  {
    return false;
  }

  /**
   * Certainly, this task has an effect - it modifies the model transformation.
   * This property is statically set here.
   * @see Task#HasNoEffect()
   */
  virtual bool HasNoEffect() const
  {
    return false;
  }

  /**
   * Returns true if the registration model seems to be completely initialized
   * and ready for execution.
   * @see Task#HasInput()
   */
  virtual bool HasInput() const;

  /**
   * Does nothing, simply returns false. The input is either set or not when the
   * task is added to the task manager.
   * @see Task#AcquireInput()
   */
  virtual bool AcquireInput()
  {
    return false;
  }

  /** Executes registration.
   * @return TRUE if the execution succeeded
   * @see Task#Execute()
   */
  virtual bool Execute();

  /** Returns true if the un-execute information was constructed (successful
   * previous execution).
   * @see Task#HasUnexecute()
   */
  virtual bool HasUnexecute() const
  {
    return m_UnexecuteInfoAvailable;
  }

  /** No unexecution implemented!
   * @see Task#Unexecute()
   */
  virtual bool Unexecute()
  {
    return false;
  }

  /**
   * The name of the task is "Registration".
   * @see Task#GetName()
   */
  virtual QString GetName();

  /**
   * This task supports progress information (although this might be an
   * estimation due to optimization convergency).
   * @see Task#SupportsProgressInformation()
   */
  bool SupportsProgressInformation() const
  {
    return true;
  }

  /**
   * This task supports cancel.
   * @see Task#IsCancelable()
   */
  bool IsCancelable() const
  {
    return true;
  }

  /**
   * A cancel will stop execution at some exposed steps.
   * @see Task#Cancel()
   **/
  virtual void Cancel()
  {
    m_CancelRequest = true;
  }

  /**
   * @return the state of the cancel-flag
   */
  virtual bool GetCancelRequestFlag()
  {
    return m_CancelRequest;
  }

  /** Set the target model which holds registration framework. **/
  void SetTargetModel(REG23Model *model)
  {
    m_TargetModel = model;
  }
  /** Get target model. **/
  REG23Model *GetTargetModel()
  {
    return m_TargetModel;
  }

  /** Get flag indicating whether or not the registration time should be part of
   * the name. **/
  bool GetIncludeRegistrationTimeInName()
  {
    return m_IncludeRegistrationTimeInName;
  }
  /** Set flag indicating whether or not the registration time should be part of
   * the name. **/
  void SetIncludeRegistrationTimeInName(bool flag)
  {
    m_IncludeRegistrationTimeInName = flag;
  }

  /** Get format string for the registration time that is appended to the name
   * string if m_IncludeRegistrationTimeInName flag is set. Use a "%1" sub-term
   * for the registration time itself. **/
  QString GetRegistrationTimeFormatString()
  {
    return m_RegistrationTimeFormatString;
  }
  /** Set format string for the registration time that is appended to the name
   * string if m_IncludeRegistrationTimeInName flag is set **/
  void SetRegistrationTimeFormatString(QString s)
  {
    m_RegistrationTimeFormatString = s;
  }

  /** @return the elapsed time since last start of the task **/
  double GetElapsedTimeSinceStart();

  /** Delete this task after execution. **/
  bool DeleteThisTaskAfterProcessing()
  {
    return true;
  }

protected:
  /** Execution done, unexecution information is now available. **/
  bool m_UnexecuteInfoAvailable;
  /** The model that is initialized/deinitialized **/
  REG23Model *m_TargetModel;
  /** Flag indicating a cancel request **/
  bool m_CancelRequest;
  /** Flag indicating whether or not the registration time should be part of
   * the name.
   * @see m_RegistrationTimeFormatString
   **/
  bool m_IncludeRegistrationTimeInName;
  /** Start time of actual execution **/
  double m_StartTime;
  /** Format string for the registration time that is appended to the name
   * string if m_IncludeRegistrationTimeInName flag is set
   * @see m_IncludeRegistrationTimeInName **/
  QString m_RegistrationTimeFormatString;

  /** Callback for NReg2D/3D registration events. **/
  void OnRegistrationCallback(itk::Object *obj, const itk::EventObject &ev);
  /** Callback for NReg2D/3D optimizer events. **/
  void OnOptimizerCallback(itk::Object *obj, const itk::EventObject &ev);

};


}


#endif /* ORAREG23REGISTRATIONEXECUTIONTASK_H_ */
