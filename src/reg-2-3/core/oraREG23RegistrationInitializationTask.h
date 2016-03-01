//
#ifndef ORAREG23REGISTRATIONINITIALIZATIONTASK_H_
#define ORAREG23REGISTRATIONINITIALIZATIONTASK_H_

// ORAIFModel
#include <oraTask.h>

namespace ora
{

// forward declaration:
class REG23Model;

/**
 * For the sake of simplicity, three main subtasks were composed here: image
 * pre-processing, auto-mask generation, registration components instantiation.
 *
 * // FIXME:
 *
 * @see Task
 *
 * @author phil
 * @author Markus
 * @version 1.2
 */
class REG23RegistrationInitializationTask : public Task
{
public:
  /** Default constructor **/
  REG23RegistrationInitializationTask();
  /** Destructor **/
  virtual ~REG23RegistrationInitializationTask();

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
   * framework is newly initialized.
   * @see Task#IsReexecuteable()
   */
  virtual bool IsReexecuteable() const
  {
    return true;
  }

  /**
   * Certainly, this task has an effect - it initializes and configures the
   * registration framework. This property is statically set here.
   * @see Task#HasNoEffect()
   */
  virtual bool HasNoEffect() const
  {
    return false;
  }

  /**
   * Returns true if the images and configuration of the registration model
   * seem to be valid.
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

  /** Executes registration framework configuration and initialization.
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
   * The name of the task is "Initialize registration".
   * @see Task#GetName()
   */
  virtual QString GetName();
  /**
   * Set the individual task names of the steps involved in registration
   * initialization. <br>
   * NOTE: The OverrideName()-method has no effect in this class!
   * @param preProcessingName task name for image pre-processing step
   * @param maskGenerationName task name for auto mask generation step
   * @param regInitName task name for pure registration components init step
   * @param postProcessingName task name for image post-processing step
   * @see Task#OverrideName()
   */
  virtual void SetNames(QString preProcessingName,
      QString maskGenerationName, QString regInitName,
      QString postProcessingName);
  /**
   * Disabled. Use the SetNames()-method instead.
   * @see SetNames()
   **/
  virtual void OverrideName(QString overrideName) {};

  /**
   * This task supports progress information.
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

  /** Delete this task after execution. **/
  bool DeleteThisTaskAfterProcessing()
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

  /** Set the target model which is initialized/deinitialized. **/
  void SetTargetModel(REG23Model *model)
  {
    m_TargetModel = model;
  }
  /** Get target model. **/
  REG23Model *GetTargetModel()
  {
    return m_TargetModel;
  }

protected:
  /** Execution done, unexecution information is now available. **/
  bool m_UnexecuteInfoAvailable;
  /** The model that is initialized/deinitialized **/
  REG23Model *m_TargetModel;
  /** Flag indicating a cancel request **/
  bool m_CancelRequest;
  /** Specific task name for pre-processing step **/
  QString m_PreProcessingName;
  /** Specific task name for mask generation step **/
  QString m_MaskGenerationName;
  /** Specific task name for post-processing step **/
  QString m_PostProcessingName;
  /** Specific task name for registration initialization step **/
  QString m_RegInitName;

};

}

#endif /* ORAREG23REGISTRATIONINITIALIZATIONTASK_H_ */
