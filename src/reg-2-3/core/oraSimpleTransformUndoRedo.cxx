/*
 TRANSLATOR ora::SimpleTransformUndoRedoManager

 lupdate: Qt-based translation with NAMESPACE-support!
 */
#include "oraSimpleTransformUndoRedo.h"

#include "oraInitialTransformTask.h"

// ORAIFTools
#include <oraStringTools.h>

namespace ora
{

SimpleTransformUndoRedoManager::SimpleTransformUndoRedoManager()
{
  m_CurrentStackIndex = -1;
  m_LogFlag = true;
}

SimpleTransformUndoRedoManager::~SimpleTransformUndoRedoManager()
{
  CleanUp();
}

void SimpleTransformUndoRedoManager::CleanUp()
{
  for (std::size_t i = 0; i < m_Stack.size(); i++)
  {
    if (m_Stack[i])
      delete m_Stack[i];
  }
  m_Stack.clear();
  m_CurrentStackIndex = -1;
  emit UndoRedoStackModified();
}

void SimpleTransformUndoRedoManager::ReportTask(AbstractTransformTask *task)
{
  if (!task)
    return;
  // remove previous redo-items:
  while (m_CurrentStackIndex >= 0 &&
         (int)m_Stack.size() > (m_CurrentStackIndex + 1))
  {
    delete m_Stack[m_Stack.size() - 1];
    m_Stack.erase(m_Stack.begin() + m_Stack.size() - 1);
  }
  m_Stack.push_back(task);
  m_CurrentStackIndex++; // new position!
  if (m_CurrentStackIndex > 0) // compute relative parameters
    task->ComputeRelativeTransform(m_Stack[m_CurrentStackIndex - 1]);
  if (m_LogFlag) // log
  {
    std::string s = "EXECUTE: ";
    s += task->GetLogDescription();
    m_LogList.push_back(s);
  }
  emit UndoRedoStackModified(); // signal!
}

void SimpleTransformUndoRedoManager::ComputeSimulatedRelativeParameters(
    AbstractTransformTask *task)
{
  if (!task)
    return;
  int sim = m_CurrentStackIndex + 1;
  if (sim > 0) // simulate computation of relative parameters
    task->ComputeRelativeTransform(m_Stack[sim - 1]);
}

bool SimpleTransformUndoRedoManager::Undo(std::size_t i)
{
  if (GetNumberOfCurrentUndoItems() <= i)
    return false;
  AbstractTransformTask *task = m_Stack[m_CurrentStackIndex - i - 1];
  if (m_LogFlag) // log
  {
    std::string s = "UNDO: ";
    s += m_Stack[m_CurrentStackIndex - i]->GetLogDescription();
    m_LogList.push_back(s);
  }
  bool succ = false;
  emit TaskExecutionRequest(task, succ); // request execution of task
  if (succ)
  {
    m_CurrentStackIndex = m_CurrentStackIndex - i - 1; // simply move index!
    emit UndoRedoStackModified(); // signal!
    return true;
  }
  else
  {
    if (m_LogFlag) // log
    {
      std::string s = "UNDO-FAILURE!";
      m_LogList.push_back(s);
    }
    return false;
  }
}

bool SimpleTransformUndoRedoManager::Reset()
{
  std::size_t n = GetNumberOfCurrentUndoItems();
  if (n == 0)
    return false;
  AbstractTransformTask *task = m_Stack[m_CurrentStackIndex - (n - 1) - 1];
  if (m_LogFlag) // log
  {
    std::string s = "RESET: ";
    s += task->GetLogDescription();
    s += ", N=" + StreamConvert(n);
    m_LogList.push_back(s);
  }
  bool succ = false;
  emit TaskExecutionRequest(task, succ); // request execution of task
  if (succ)
  {
    m_CurrentStackIndex = m_CurrentStackIndex - (n - 1) - 1; // simply move index!
    emit UndoRedoStackModified(); // signal!
    return true;
  }
  else
  {
    if (m_LogFlag) // log
    {
      std::string s = "RESET-FAILURE!";
      m_LogList.push_back(s);
    }
    return false;
  }
}

bool SimpleTransformUndoRedoManager::Redo(std::size_t i)
{
  if (GetNumberOfCurrentRedoItems() <= i)
    return false;
  AbstractTransformTask *task = m_Stack[m_CurrentStackIndex + i + 1];
  if (m_LogFlag) // log
  {
    std::string s = "REDO: ";
    s += task->GetLogDescription();
    m_LogList.push_back(s);
  }
  bool succ = false;
  emit TaskExecutionRequest(task, succ); // request execution of task
  if (succ)
  {
    m_CurrentStackIndex = m_CurrentStackIndex + i + 1; // simply move index!
    emit UndoRedoStackModified(); // signal!
    return true;
  }
  else
  {
    if (m_LogFlag) // log
    {
      std::string s = "REDO-FAILURE!";
      m_LogList.push_back(s);
    }
    return false;
  }
}

std::size_t SimpleTransformUndoRedoManager::GetNumberOfCurrentUndoItems()
{
  if (m_CurrentStackIndex <= 0)
    return 0; // nothing to undo
  std::size_t i = (std::size_t)m_CurrentStackIndex;
  std::size_t c = 0;
  do
  {
    i--;
    c++;
    InitialTransformTask *testInit = dynamic_cast<InitialTransformTask *>(
        m_Stack[i]);
    if (testInit) // initial transform task cannot be undone!
      break;
  } while (i > 0);
  return c;
}

std::string SimpleTransformUndoRedoManager::GetIthUndoItemDescription(
    std::size_t i)
{
  if (i >= GetNumberOfCurrentUndoItems())
    return "";
  std::string s = m_Stack[m_CurrentStackIndex - i]->GetShortDescription();
  return s;
}

std::size_t SimpleTransformUndoRedoManager::GetNumberOfCurrentRedoItems()
{
  if (m_CurrentStackIndex < 0)
    return 0;
  std::size_t c = m_Stack.size() - (std::size_t)m_CurrentStackIndex - 1;
  return c;
}

std::string SimpleTransformUndoRedoManager::GetIthRedoItemDescription(
    std::size_t i)
{
  if (i >= GetNumberOfCurrentRedoItems())
    return "";
  std::string s = m_Stack[m_CurrentStackIndex + i + 1]->GetShortDescription();
  return s;
}

}
