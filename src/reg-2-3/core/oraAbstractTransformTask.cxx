/*
 TRANSLATOR ora::AbstractTransformTask

 lupdate: Qt-based translation with NAMESPACE-support!
 */
#include "oraAbstractTransformTask.h"

// ORAIFTools
#include <oraStringTools.h>

namespace ora
{

AbstractTransformTask::AbstractTransformTask()
{
  m_Parameters.SetSize(0);
  m_RelativeParameters.SetSize(0);
  CurrentORADateTimeString(m_TimeStamp);
}

AbstractTransformTask::~AbstractTransformTask()
{

}

void AbstractTransformTask::SetParameters(ParametersType pars)
{
  m_Parameters = pars;
}
AbstractTransformTask::ParametersType AbstractTransformTask::GetParameters()
{
  return m_Parameters;
}

AbstractTransformTask::ParametersType AbstractTransformTask::GetRelativeParameters()
{
  return m_RelativeParameters;
}

bool AbstractTransformTask::ImpliesRelativeTransformation(const double EPSILON)
{
  for (unsigned int i = 0; i < m_RelativeParameters.Size(); i++)
  {
    if (fabs(m_RelativeParameters[i]) > EPSILON)
      return true;
  }
  return false;
}

std::string AbstractTransformTask::ConvertParametersToString()
{
  std::ostringstream os;
  os << "parameters: [";
  for (unsigned int i = 0; i < m_Parameters.Size(); i++)
    os << " " << m_Parameters[i];
  os << "]";
  if (m_RelativeParameters.Size() > 0)
  {
    os << ", relative: [";
    for (unsigned int i = 0; i < m_RelativeParameters.Size(); i++)
      os << " " << m_RelativeParameters[i];
    os << "]";
  }
  return os.str();
}

std::string AbstractTransformTask::GetTimeStamp()
{
  return m_TimeStamp;
}

bool AbstractTransformTask::ComputeRelativeTransform(AbstractTransformTask *task)
{
  if (task && task->GetParameters().Size() == m_Parameters.Size())
  {
    m_RelativeParameters.SetSize(m_Parameters.Size());
    ParametersType pars = task->GetParameters();
    for (unsigned int i = 0; i < m_Parameters.Size(); i++)
      m_RelativeParameters[i] = m_Parameters[i] - pars[i];
    return true;
  }
  else
  {
    return false;
  }
}

}
