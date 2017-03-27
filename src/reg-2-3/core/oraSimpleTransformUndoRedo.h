//
#ifndef ORASIMPLETRANSFORMUNDOREDO_H_
#define ORASIMPLETRANSFORMUNDOREDO_H_

#include <QObject>

#include <vector>
#include <string>
#include <stdio.h>
#include <sstream>
#include <math.h>

#include "oraAbstractTransformTask.h"

namespace ora
{

/** FIXME:
 *
 * @author phil 
 * @version 1.0
 */
class SimpleTransformUndoRedoManager : public QObject
{
  Q_OBJECT

  /*
   TRANSLATOR ora::SimpleTransformUndoRedoManager

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** Default constructor. **/
  SimpleTransformUndoRedoManager();
  /** Destructor. **/
  ~SimpleTransformUndoRedoManager();

  /** Get log-flag indicating whether or not to generate a string-based log-list
   * which tracks the stack-related task activities chronologically **/
  bool GetLogFlag()
  {
    return m_LogFlag;
  }
  /** Set log-flag indicating whether or not to generate a string-based log-list
   * which tracks the stack-related task activities chronologically. NOTE:
   * Flipping the flag-state deletes the current log list! **/
  void SetLogFlag(bool flag)
  {
    if (m_LogFlag != flag)
      m_LogList.clear();
    m_LogFlag = flag;
  }

  /** Get string-based log-list which tracks the stack-related task activities
   * chronologically **/
  std::vector<std::string> &GetLogList()
  {
    return m_LogList;
  }

  /** Get the number of current undo items w.r.t. current stack index. **/
  std::size_t GetNumberOfCurrentUndoItems();
  /** Get the (short) description of the i-th current undo item. **/
  std::string GetIthUndoItemDescription(std::size_t i);
  /** Get the number of current redo items w.r.t. current stack index. **/
  std::size_t GetNumberOfCurrentRedoItems();
  /** Get the (short) description of the i-th current redo item. **/
  std::string GetIthRedoItemDescription(std::size_t i);

  /** Report the specified task as DONE which causes the manager to add it to
   * its internal stack. This will usually trigger a UndoRedoStackModified()
   * signal. NOTE: This operation will normally clear the redo-stack. BTW: This
   * method ensures that the task's relative parameters are updated w.r.t. to
   * its predecessor in the stack!
   * @see UndoRedoStackModified()
   * @see Undo()
   * @see Redo() **/
  void ReportTask(AbstractTransformTask *task);

  /** Undo the next task in undo stack. If the task could be undone, it is
   * moved to the redo stack. During this operation, the TaskExecutionRequest()
   * signal is emitted which recognizes whether or not the undo operation was
   * successful. After a successful undo operation, the redo and undo stacks
   * are usually updated which triggers a UndoRedoStackModified() signal.
   * @param i if i>0, i undo steps are leapt over
   * @return true if the task could be successfully undone
   * @see TaskExecutionRequest()
   * @see UndoRedoStackModified()
   * @see Redo()
   * @see ReportTask() **/
  bool Undo(std::size_t i = 0);

  /** Go back in undo stack as far as possible and force something like an
   * 'initial position'. NOTE: This will, however, trigger only one
   * TaskExecutionRequest() signal!
   * @return true if at least one implicit undo could be applied
   * @see TaskExecutionRequest()
   * @see UndoRedoStackModified() **/
  bool Reset();

  /** Redo the next task in redo stack. If the task could be redone, it is
   * moved to the undo stack. During this operation, the TaskExecutionRequest()
   * signal is emitted which recognizes whether or not the redo operation was
   * successful. After a successful redo operation, the redo and undo stacks
   * are usually updated which triggers a UndoRedoStackModified() signal.
   * @param i if i>0, i redo steps are leapt over
   * @return true if the task could be successfully redone
   * @see TaskExecutionRequest()
   * @see UndoRedoStackModified()
   * @see Undo()
   * @see ReportTask() **/
  bool Redo(std::size_t i = 0);

  /** Destroy all managed tasks and set back. **/
  void CleanUp();

  /** Simulate the report of the specified task (but do not add it really!), and
   * compute the resultant relative parameters for the specified task. **/
  void ComputeSimulatedRelativeParameters(AbstractTransformTask *task);

signals:
  /** Emitted whenever the undo and/or redo stack was modified. **/
  void UndoRedoStackModified();
  /** Emitted whenever a task requests execution as a consequence of an undo/
   * redo operation.
   * @param task pointer to the task which requests to be executed/applied
   * @param success returned success which notifies the manager whether the
   * task could be executed or not! **/
  void TaskExecutionRequest(AbstractTransformTask *task, bool &success);

protected:
  /** Log-flag indicating whether or not to generate a string-based log-list
   * which tracks the stack-related task activities chronologically **/
  bool m_LogFlag;
  /** Internal stack pointer ("where am I?") **/
  int m_CurrentStackIndex;
  /** Main stack containing the applied transforms **/
  std::vector<AbstractTransformTask *> m_Stack;
  /** string-based log-list which tracks the stack-related task activities
   * chronologically **/
  std::vector<std::string> m_LogList;

};

}


#endif /* ORASIMPLETRANSFORMUNDOREDO_H_ */
