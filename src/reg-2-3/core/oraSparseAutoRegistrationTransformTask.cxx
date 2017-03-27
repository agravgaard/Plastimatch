/*
 TRANSLATOR ora::SparseAutoRegistrationTransformTask

 lupdate: Qt-based translation with NAMESPACE-support!
 */
#include "oraSparseAutoRegistrationTransformTask.h"

// ORAIFTools
#include <oraStringTools.h>

#include <itkMath.h>

#include <stdio.h>

namespace ora
{

SparseAutoRegistrationTransformTask::SparseAutoRegistrationTransformTask()
  : AutoRegistrationTransformTask()
{
  m_RegistrationType = RT_UNKNOWN;
}

SparseAutoRegistrationTransformTask::~SparseAutoRegistrationTransformTask()
{

}


void SparseAutoRegistrationTransformTask::SetRegistrationType(RegistrationType regType)
{
  m_RegistrationType = regType;
}

SparseAutoRegistrationTransformTask::RegistrationType SparseAutoRegistrationTransformTask::GetRegistrationType()
{
  return m_RegistrationType;
}

std::string SparseAutoRegistrationTransformTask::GetLogDescription()
{
  char timebuff[100];
  sprintf(timebuff, "%.2f", m_RegistrationTime);
  std::string s = "";
  if (m_RegistrationType == RT_CROSS_CORRELATION)
    s = "Cross-Correlation Sparse Auto-registration, ";
  else
    s = "Sparse Auto-registration, ";
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

std::string SparseAutoRegistrationTransformTask::GetShortDescription()
{
  const double r2d = 180. / 3.14159265358979323846;
  std::ostringstream os;
  char buff[100];
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[0] * r2d * 10.) / 10.);
  os << SparseAutoRegistrationTransformTask::tr("Sparse Auto-registration: rx=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[1] * r2d * 10.) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" deg, ry=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[2] * r2d * 10.) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" deg, rz=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[3]) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" deg, tx=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[4]) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" cm, ty=").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_Parameters[5]) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" cm, tz=").toStdString() << std::string(buff) <<
      SparseAutoRegistrationTransformTask::tr(" cm").toStdString();
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[0] * r2d * 10.) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" (delta: ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[1] * r2d * 10.) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" deg, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[2] * r2d * 10.) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" deg, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[3]) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" deg, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[4]) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" cm, ").toStdString() << std::string(buff);
  sprintf(buff, "%.1f", itk::Math::Round<int, double>(this->m_RelativeParameters[5]) / 10.);
  os << SparseAutoRegistrationTransformTask::tr(" cm, ").toStdString() << std::string(buff) <<
      SparseAutoRegistrationTransformTask::tr(" cm)").toStdString();

  return os.str();
}

}
