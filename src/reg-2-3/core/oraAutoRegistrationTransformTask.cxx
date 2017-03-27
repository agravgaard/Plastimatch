/*
 TRANSLATOR ora::AutoRegistrationTransformTask

 lupdate: Qt-based translation with NAMESPACE-support!
 */
#include "oraAutoRegistrationTransformTask.h"

// ORAIFTools
#include <oraStringTools.h>

#include <itkMath.h>

#include <stdio.h>

namespace ora
{

AutoRegistrationTransformTask::AutoRegistrationTransformTask()
  : AbstractTransformTask()
{
  m_RegistrationTime = -1;
  m_UserCancel = false;
  m_NumberOfIterations = -1;
}

AutoRegistrationTransformTask::~AutoRegistrationTransformTask()
{

}

std::string AutoRegistrationTransformTask::GetLogDescription()
{
  char timebuff[100];
  sprintf(timebuff, "%.2f", m_RegistrationTime);
  std::string s = "Auto-registration, ";
  s += ConvertParametersToString();
  s += ", registration time: ";
  s += std::string(timebuff) + " s";
  s += ", user-cancel: ";
  s += StreamConvert(m_UserCancel);
  s += ", iterations: ";
  s += StreamConvert(m_NumberOfIterations);
  s += ", time stamp: ";
  s += GetTimeStamp();
  return s;
}

std::string AutoRegistrationTransformTask::GetShortDescription()
{
  const double r2d = 180. / 3.14159265358979323846;
  std::ostringstream os;
  char buff[100];
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[0] * r2d * 10.) / 10.);
  os << AutoRegistrationTransformTask::tr("Auto-registration: rx=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[1] * r2d * 10.) / 10.);
  os << AutoRegistrationTransformTask::tr(" deg, ry=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[2] * r2d * 10.) / 10.);
  os << AutoRegistrationTransformTask::tr(" deg, rz=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[3]) / 10.);
  os << AutoRegistrationTransformTask::tr(" deg, tx=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[4]) / 10.);
  os << AutoRegistrationTransformTask::tr(" cm, ty=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[5]) / 10.);
  os << AutoRegistrationTransformTask::tr(" cm, tz=").toStdString() << std::string(buff) <<
      AutoRegistrationTransformTask::tr(" cm").toStdString();
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[0] * r2d * 10.) / 10.);
  os << AutoRegistrationTransformTask::tr(" (delta: ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[1] * r2d * 10.) / 10.);
  os << AutoRegistrationTransformTask::tr(" deg, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[2] * r2d * 10.) / 10.);
  os << AutoRegistrationTransformTask::tr(" deg, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[3]) / 10.);
  os << AutoRegistrationTransformTask::tr(" deg, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[4]) / 10.);
  os << AutoRegistrationTransformTask::tr(" cm, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[5]) / 10.);
  os << AutoRegistrationTransformTask::tr(" cm, ").toStdString() << std::string(buff) <<
      AutoRegistrationTransformTask::tr(" cm)").toStdString();

  return os.str();
}

double AutoRegistrationTransformTask::GetRegistrationTime()
{
  return m_RegistrationTime;
}

void AutoRegistrationTransformTask::SetRegistrationTime(double regTime)
{
  m_RegistrationTime = regTime;
}

void AutoRegistrationTransformTask::SetUserCancel(bool cancel)
{
  m_UserCancel = cancel;
}

bool AutoRegistrationTransformTask::GetUserCancel()
{
  return m_UserCancel;
}

void AutoRegistrationTransformTask::SetNumberOfIterations(int num)
{
  m_NumberOfIterations = num;
}

int AutoRegistrationTransformTask::GetNumberOfIterations()
{
  return m_NumberOfIterations;
}

}
