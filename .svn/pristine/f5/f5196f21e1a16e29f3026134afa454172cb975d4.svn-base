#ifndef ORATASKMANAGER_H_
#define ORATASKMANAGER_H_

#include <QMutex>
#include <QQueue>
#include <QVector>
#include <QThread>
#include <QWaitCondition>
#include <QMap>


// Forward declarations
namespace ora
{
class Task;
}

namespace ora
{

/** This class takes care of undo/redo functionality, which is provided by
 * execution or un-execution of individual tasks.
 *
 * This class implements an "ID-based singleton" pattern. Retrieve a specific
 * task manager by calling TaskManager::GetInstance("your-ID").
 *
 * @author Markus 
 * @author phil 
 * @version 1.2
 */
class TaskManager: public QThread
{
Q_OBJECT

public:
  /**
   * Get the singleton instance for a specified ID.
   * @param id a string literal that UNIQUELY identifies a specified singleton
   */
  static TaskManager *GetInstance(QString id);
  /**
   * Destroy the singleton instance with a specified ID.
   * @param id a string literal that UNIQUELY identifies a specified singleton
   * @return TRUE if the specified singleton could be destroyed
   */
  static bool DestroyInstance(QString id);

  /** Schedules the execution of a task.
   * @see #unfinishedTasks
   */
  void ExecuteTask(Task * task);

  /* Unexecute the last executed task. */
  bool Undo();

  /* Reexecute the last unexecuted task. */
  bool Redo();

  /* Get the number of tasks on the undo stack. */
  unsigned int UndoSize() const;

  /* Get the number of tasks on the redo stack. */
  unsigned int RedoSize() const;

  /* Clear undo and redo stack. */
  bool Clean();

signals:
  /** The next task (argument) of the queue is processed. **/
  void TaskProcessingStart(Task *task);
  /** The specified task (argument) is no longer processed. This could be due
   * to errors or due to the planned task end. **/
  void TaskProcessingEnd(Task *task);
  /** The specified task (argument) was started. **/
  void TaskExecuteStart(Task *task);
  /** The specified task (argument) finished. The second argument specifies
   * whether the task was successful or not. **/
  void TaskExecuteEnd(Task *task, bool success);
  /** The specified task (argument) was dropped because it has no effect. **/
  void TaskHasNoEffectDropped(Task *task);
  /** The specified task (argument) could not be executed because it does not
   * have valid input(s) and has not been able to acquire it (them). **/
  void TaskHasNoInputsDropped(Task *task);
  /** All tasks currently in queue are done. **/
  void AllTasksDone();

protected:
  /** Starts the task execution loop. */
  void run();

private:
  TaskManager(); // no instance from outside
  TaskManager(const TaskManager&); // no copy
  TaskManager& operator=(const TaskManager&); // no assignment
  ~TaskManager();

  /** ID-based singleton instances. */
  static QMap<QString, TaskManager *> instances;

  /** Mutex for concurrent singleton instantiation/access. */
  static QMutex instantiationMutex;

  /** Queue that stores all unfinished tasks that should be executed. Tasks
   * get added to the queue by ExecuteTask().
   * @see ExecuteTask()
   */
  QQueue<Task *> unfinishedTasks;

  /** List of tasks that can be un-executed.
   * Oldest are at front, newest at back.
   */
  QVector<Task *> undoStack;

  /** List of tasks that can be re-executed.
   * Oldest are at front, newest at back.
   */
  QVector<Task *> redoStack;

  /** The execution loop waits for tasks that get added by ExecuteTask(). */
  QWaitCondition taskAdded;

  /** The execution loop waits for the task to get finished.
   * NOTE: currently not used */
  QWaitCondition taskFinished;

  /** Mutex for access serialization between threads. */
  QMutex mutex;

private slots:
  // TODO: use this slot eventually later for synchronization of asyn tasks.
  //    void TaskFinished();
};

} // ora

#endif
