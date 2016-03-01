

#ifndef ORATASK_H_
#define ORATASK_H_

#include <QObject>
#include <QString>


namespace ora 
{


/** This is an interface for classes that modify target objects.
 *
 * Subclasses are responsible to store all information that is required to
 * execute, un-execute and re-execute the task.
 *
 * NOTE: The task itself should not be executed directly by calling its Execute()
 * method, instead use the TaskManager::ExecuteTask method. It keeps track of the
 * executed tasks and enables undo/redo functionality.
 *
 * @author Markus 
 * @author phil 
 * @version 1.2
 */
class Task : public QObject
{
  Q_OBJECT

public:
  /** Minimum constructor. **/
  Task();

  /** Returns true if the task can be un-executed and re-executed.
   * Certain tasks cannot be un-executed (e.g. saving a file).
   * If the action is not un-executable then it is not re-executable also.
   */
  virtual bool IsUnexecutable() const = 0;

  /** Returns true if re-execution after un-execution is possible. */
  virtual bool IsReexecuteable() const = 0;

  /** Returns true if executing the action will produce no effect on the target
   * object (e.g. Set pixel value 154 to 154).
   */
  virtual bool HasNoEffect() const = 0;

  /** Returns true if the data (parameters) required to execute the task
   * on the target are valid and available (e.g. preconditions, null-pointer,
   * do files exist). */
  virtual bool HasInput() const = 0;

  /** Collects data (parameters) required by the task for execution (e.g.
   * loading files from disk).
   * Returns false if the collecting process was canceled.
   */
  virtual bool AcquireInput() = 0;

  /** Executes the task. Returns false if the action was canceled.
   * Canceling an action is not an error. Use exceptions for errors.
   */
  virtual bool Execute() = 0;

  /** Returns true if the un-execute information was constructed. Un-execute
   * information must be available immediately after execution.
   * Un-execute information may not exists after execution (e.g. memory
   * requirements are too high at storing entire images).
   */
  virtual bool HasUnexecute() const = 0;

  /** Un-executes the task. Un-execute should not be called before Execute
   * (no un-execute information available).
   */
  virtual bool Unexecute() = 0;

  /** Returns the name of the task. The name may depend on the current state
   * of the task. This class may or may not be re-implemented in subclasses.
   */
  virtual QString GetName();
  /** Optionally provide a specific (more 'natural') name for a task. This
   * method may or may not be re-implemented and adapted in subclasses. **/
  virtual void OverrideName(QString overrideName);

  /** Returns the number of bytes used by the task object. Might be useful
   * to estimate the memory usage. The default return value is the pure size
   * of the object which does not really account for the real size of the
   * object as most members usually are pointers. */
  virtual unsigned int GetBytesCount() const;

  /** Returns whether or not this task supports the TaskProgressInfo signal. **/
  virtual bool SupportsProgressInformation() const = 0;

  /** Returns whether or not this task is cancelable. **/
  virtual bool IsCancelable() const = 0;

  /**
   * Should be implemented if IsCancelable() returns TRUE. Otherwise this method
   * won't have any effect.
   **/
  virtual void Cancel() { } ;

  /** If a task is NOT unexecutable, it is not pushed onto the undo-stack.
   * Therefore, such tasks can indicate that they want to be deleted by the
   * task manager after execution. NOTE: This is done by the task manager after
   * emitting the TaskProcessingEnd()-signal. Furthermore, a return value of
   * TRUE only shows an effect on tasks that are NOT unexecutable! **/
  virtual bool DeleteThisTaskAfterProcessing() = 0;

signals:
  /** Task started. Should be emitted in subclasses.
   * @param execute TRUE if invoked at the start of execution, false if invoked
   * at the start of unexecution
   **/
  void TaskStarted(bool execute);
  /** Task finished. Should be emitted in subclasses.
   * @param execute TRUE if invoked at the end of execution, false if invoked
   * at the end of unexecution
   **/
  void TaskFinished(bool execute);
  /**
   * Task progress information. Could optionally be emitted in subclasses.
   * @param execute TRUE if invoked during execution, false if invoked
   * during unexecution
   * @param p (estimated) progress in percent [0;100]
   * @see SupportsProgressInformation()
   **/
  void TaskProgressInfo(bool execute, double p);

protected:
  /** Custom task name (valid if not empty). **/
  QString m_CustomName;

};

}


#endif /* ORATASK_H_ */
