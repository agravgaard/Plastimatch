#include "oraTaskManager.h"

#include <stdexcept>
#include <iostream>

// Forward declarations
#include "oraTask.h"

namespace ora 
{

// Dummy task to terminate the endless task execution loop
Task * const endTask = 0;

// Initialize static members
QMutex TaskManager::instantiationMutex;
QMap<QString, TaskManager *> TaskManager::instances;

TaskManager *
TaskManager
::GetInstance(QString id)
{
  QMutexLocker locker(&TaskManager::instantiationMutex);
  QMap<QString, TaskManager *>::iterator it = instances.find(id);
  if (it == instances.end())
  {
    TaskManager *taskman = new TaskManager();
    it = instances.insert(id, taskman);
    taskman->start(QThread::NormalPriority);
  }
  return it.value();
}

bool TaskManager::DestroyInstance(QString id)
{
  QMutexLocker locker(&TaskManager::instantiationMutex);
  QMap<QString, TaskManager *>::iterator it = instances.find(id);
  if (it != instances.end())
  {
    TaskManager *taskman = instances.value(id);
    instances.remove(id);
    delete taskman;
    return true;
  }
  else // not found
  {
    return false;
  }
}


void
TaskManager::ExecuteTask(Task * task)
{
  // FIXME: Check if the redostack contains elements. A new action invalidates
  // the redo stack operations because of possible changed data
  if (!task)
    return;
  {
    QMutexLocker locker(&mutex);
    unfinishedTasks.enqueue(task);
    taskAdded.wakeOne();
  }
}


bool
TaskManager
::Undo()
{
  QMutexLocker locker(&mutex);
  if (!undoStack.isEmpty() && unfinishedTasks.isEmpty())
  {
    Task* task = undoStack.last();
    undoStack.pop_back();
    try {
      if(task->HasUnexecute())
      {
      if(task->Unexecute())
      {
        redoStack.append(task);
      }
      else
      {
        std::cout << "UNDO-FAIL: " << task->GetName().toStdString() << std::endl;
        // TODO: More informative and verbose messages on different kinds of fails
      }
      }
      else
      {
        // TODO: Task has no information to undo the changes, reexecute all tasks
      }
    } catch (std::runtime_error e) {
      std::cout << "UNDO-EXCEPTION: " << e.what() << std::endl;
      // TODO: Proper exception handling in GUI, maybe just forward exceptions
    }
    return true;
  }
  return false;
}

bool
TaskManager::Redo()
{
  QMutexLocker locker(&mutex);
  if (!redoStack.isEmpty() && unfinishedTasks.isEmpty())
  {
    Task* task = redoStack.last();
    redoStack.pop_back();
    // FIXME: Usa a different method to re-execute tasks from the redo stack
    // to avoid corruption of redo/undo operations
    ExecuteTask(task);
    return true;
  }
  return false;
}

unsigned int
TaskManager
::UndoSize() const
{
  return undoStack.size();
}

unsigned int
TaskManager
::RedoSize() const
{
  return redoStack.size();
}

bool
TaskManager
::Clean()
{
  QMutexLocker locker(&mutex);
  while (!unfinishedTasks.isEmpty())
    delete unfinishedTasks.dequeue();
  for (int i = 0; i < undoStack.size(); i++)
    delete undoStack[i];
  for (int i = 0; i < redoStack.size(); i++)
    delete redoStack[i];
  undoStack.clear();
  redoStack.clear();
  return true;
}


TaskManager
::TaskManager()
{

}

TaskManager
::~TaskManager()
{
  {
    QMutexLocker locker(&mutex);
    while (!unfinishedTasks.isEmpty())
      delete unfinishedTasks.dequeue();
    unfinishedTasks.enqueue(endTask);
    taskAdded.wakeOne();
  }
  wait();
  Clean();
}

void
TaskManager
::run()
{
  Task *task = 0;

  bool succ;

  forever
  {
    {
      QMutexLocker locker(&mutex);
      // Wait for tasks to process
      if (unfinishedTasks.isEmpty())
        taskAdded.wait(&mutex);
      // Get first task from queue
      task = unfinishedTasks.dequeue();
      if (task == endTask)
        break;
    }

    emit TaskProcessingStart(task);

    // Check if task has all input data and initialize it
    // Methods return false if they were canceled (exceptions are used for errors)
    if (!task->HasInput() && !task->AcquireInput())
    {
      emit TaskHasNoInputsDropped(task);
      emit TaskProcessingEnd(task);
      return; // TODO: is return ok? should not that be a continue with a preceding deletion of the task?
    }

    // If the task has no effect then do nothing
    if (task->HasNoEffect())
    {
      emit TaskHasNoEffectDropped(task);
      emit TaskProcessingEnd(task);
      return; // TODO: is return ok? should not that be a continue with a preceding deletion of the task?
    }

    // Execute the task
    emit TaskExecuteStart(task);

    succ = task->Execute();
    if (succ)
    {
      // TODO: Wait for the task to finish? (if it is a thread) with a WaitCondition
      if (!task->IsUnexecutable())
      {
        // the task is executed but cannot be unexecuted
        // the undostack is now invalid
        // TODO: All tasks that operate on these targets must be removed, or entire stack gets cleaned
      }
      else
      {
        // The task can be un-executed but it has no unexecute information
        // It is added to the stack and when an undo occurs the stack is
        // reexecuted from the beginning
        // if (task->HasUnexecute())

        // Add task to undo-stack
        undoStack.append(task);
      }
    }

    emit TaskExecuteEnd(task, succ);
    emit TaskProcessingEnd(task);

    // Optional auto-delete request of NOT-unexecutable tasks:
    if (!task->IsUnexecutable() && task->DeleteThisTaskAfterProcessing())
    {
      delete task;
    }

    {
      QMutexLocker locker(&mutex);
      if (unfinishedTasks.isEmpty())
        emit AllTasksDone();
    }
  }
}

}

