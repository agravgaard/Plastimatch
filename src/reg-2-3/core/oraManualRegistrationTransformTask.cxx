/*
 TRANSLATOR ora::ManualRegistrationTransformTask

 lupdate: Qt-based translation with NAMESPACE-support!
 */

#include "oraManualRegistrationTransformTask.h"

// ORAIFTools
#include <oraStringTools.h>

#include <itkMath.h>

#include <stdio.h>

namespace ora
{

ManualRegistrationTransformTask::ManualRegistrationTransformTask()
  : AbstractTransformTask()
{
  m_IsRotation = false;
}

ManualRegistrationTransformTask::~ManualRegistrationTransformTask()
{

}

std::string ManualRegistrationTransformTask::GetLogDescription()
{
  std::string s = "Manual registration (";
  if (m_IsRotation)
    s += "rotation), ";
  else
    s += "translation), ";
  s += ConvertParametersToString();
  s += ", time stamp: ";
  s += GetTimeStamp();
  return s;
}

std::string ManualRegistrationTransformTask::GetShortDescription()
{
  const double r2d = 180. / 3.14159265358979323846;
  std::ostringstream os;
  char buff[100];
  if (m_IsRotation)
  {
    os << ManualRegistrationTransformTask::tr("Manual rotation: ").toStdString();
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[0] * r2d * 10.) / 10.);
    os << ManualRegistrationTransformTask::tr("rx=").toStdString() << std::string(buff);
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[1] * r2d * 10.) / 10.);
    os << ManualRegistrationTransformTask::tr(" deg, ry=").toStdString() << std::string(buff);
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[2] * r2d * 10.) / 10.);
    os << ManualRegistrationTransformTask::tr(" deg, rz=").toStdString() << std::string(buff) <<
        ManualRegistrationTransformTask::tr(" deg").toStdString();
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[0] * r2d * 10.) / 10.);
    os << ManualRegistrationTransformTask::tr(" (delta: ").toStdString() << std::string(buff);
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[1] * r2d * 10.) / 10.);
    os << ManualRegistrationTransformTask::tr(" deg, ").toStdString() << std::string(buff);
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[2] * r2d * 10.) / 10.);
    os << ManualRegistrationTransformTask::tr(" deg, ").toStdString() << std::string(buff) <<
        ManualRegistrationTransformTask::tr(" deg)").toStdString();
  }
  else
  {
    os << ManualRegistrationTransformTask::tr("Manual translation: ").toStdString();
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[3]) / 10.);
    os << ManualRegistrationTransformTask::tr("tx=").toStdString() << std::string(buff);
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[4]) / 10.);
    os << ManualRegistrationTransformTask::tr(" cm, ty=").toStdString() << std::string(buff);
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[5]) / 10.);
    os << ManualRegistrationTransformTask::tr(" cm, tz=").toStdString() << std::string(buff) <<
        ManualRegistrationTransformTask::tr(" cm").toStdString();
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[3]) / 10.);
    os << ManualRegistrationTransformTask::tr(" (delta: ").toStdString() << std::string(buff);
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[4]) / 10.);
    os << ManualRegistrationTransformTask::tr(" cm, ").toStdString() << std::string(buff);
    sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[5]) / 10.);
    os << ManualRegistrationTransformTask::tr(" cm, ").toStdString() << std::string(buff) <<
        ManualRegistrationTransformTask::tr(" cm)").toStdString();
  }

  return os.str();
}

bool ManualRegistrationTransformTask::GetIsRotation()
{
  return m_IsRotation;
}

void ManualRegistrationTransformTask::SetIsRotation(bool rotation)
{
  m_IsRotation = rotation;
}

}
