

#include "oraTask.h"


namespace ora 
{

Task::Task()
{
  m_CustomName = "";
}

unsigned int Task::GetBytesCount() const
{
  return sizeof(this); // default: pure object size (should be overridden)
}

void Task::OverrideName(QString overrideName)
{
  m_CustomName = overrideName;
}

QString Task::GetName()
{
  return m_CustomName;
}

}

