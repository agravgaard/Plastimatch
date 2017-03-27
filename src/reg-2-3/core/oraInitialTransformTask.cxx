/*
 TRANSLATOR ora::InitialTransformTask

 lupdate: Qt-based translation with NAMESPACE-support!
 */
#include "oraInitialTransformTask.h"

// ORAIFTools
#include <oraStringTools.h>

#include <itkMath.h>

#include <stdio.h>

namespace ora
{

InitialTransformTask::InitialTransformTask()
  : AbstractTransformTask()
{

}

InitialTransformTask::~InitialTransformTask()
{

}

std::string InitialTransformTask::GetLogDescription()
{
  std::string s = "Initialization, ";
  s += ConvertParametersToString();
  s += ", time stamp: ";
  s += GetTimeStamp();
  return s;
}

std::string InitialTransformTask::GetShortDescription()
{
  const double r2d = 180. / 3.14159265358979323846;
  std::ostringstream os;
  char buff[100];
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[0] * r2d * 10.) / 10.);
  os << InitialTransformTask::tr("Initial state: rx=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[1] * r2d * 10.) / 10.);
  os << InitialTransformTask::tr(" deg, ry=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[2] * r2d * 10.) / 10.);
  os << InitialTransformTask::tr(" deg, rz=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[3]) / 10.);
  os << InitialTransformTask::tr(" deg, tx=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[4]) / 10.);
  os << InitialTransformTask::tr(" cm, ty=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[5]) / 10.);
  os << InitialTransformTask::tr(" cm, tz=").toStdString() << std::string(buff) <<
      InitialTransformTask::tr(" cm").toStdString();
  return os.str();
}

}
