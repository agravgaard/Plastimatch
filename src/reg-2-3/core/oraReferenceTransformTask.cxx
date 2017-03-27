/*
 TRANSLATOR ora::ReferenceTransformTask

 lupdate: Qt-based translation with NAMESPACE-support!
 */
#include "oraReferenceTransformTask.h"

// ORAIFTools
#include <oraStringTools.h>

#include <itkMath.h>

#include <stdio.h>

namespace ora
{

ReferenceTransformTask::ReferenceTransformTask()
  : AbstractTransformTask()
{
}

ReferenceTransformTask::~ReferenceTransformTask()
{

}

std::string ReferenceTransformTask::GetLogDescription()
{
  std::string s = "Reference transform, ";
  s += ConvertParametersToString();
  s += ", time stamp: ";
  s += GetTimeStamp();
  return s;
}

std::string ReferenceTransformTask::GetShortDescription()
{
  const double r2d = 180. / 3.14159265358979323846;
  std::ostringstream os;
  char buff[100];
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[0] * r2d * 10.) / 10.);
  os << ReferenceTransformTask::tr("Reference-transform: rx=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[1] * r2d * 10.) / 10.);
  os << ReferenceTransformTask::tr(" deg, ry=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[2] * r2d * 10.) / 10.);
  os << ReferenceTransformTask::tr(" deg, rz=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[3]) / 10.);
  os << ReferenceTransformTask::tr(" deg, tx=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[4]) / 10.);
  os << ReferenceTransformTask::tr(" cm, ty=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[5]) / 10.);
  os << ReferenceTransformTask::tr(" cm, tz=").toStdString() << std::string(buff) <<
      ReferenceTransformTask::tr(" cm").toStdString();
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[0] * r2d * 10.) / 10.);
  os << ReferenceTransformTask::tr(" (delta: ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[1] * r2d * 10.) / 10.);
  os << ReferenceTransformTask::tr(" deg, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[2] * r2d * 10.) / 10.);
  os << ReferenceTransformTask::tr(" deg, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[3]) / 10.);
  os << ReferenceTransformTask::tr(" deg, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[4]) / 10.);
  os << ReferenceTransformTask::tr(" cm, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[5]) / 10.);
  os << ReferenceTransformTask::tr(" cm, ").toStdString() << std::string(buff) <<
      ReferenceTransformTask::tr(" cm)").toStdString();

  return os.str();
}


}
