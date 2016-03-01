

#ifndef ORAYAWPITCHROLL3DTRANSFORM_TXX_
#define ORAYAWPITCHROLL3DTRANSFORM_TXX_


#include "oraYawPitchRoll3DTransform.h"



namespace ora
{

template <class TScalarType>
const double YawPitchRoll3DTransform<TScalarType>::EPSILON = 1e-6;

template <class TScalarType>
const TScalarType
YawPitchRoll3DTransform<TScalarType>
::ProjectAngleToNegativePositivePIRange(TScalarType angle)
{
  const double TPI = 2 * PI;
  if (angle < 0)
  {
    while (angle < -TPI)
      angle += TPI;
  }
  else // angle >= 0
  {
    while (angle > TPI)
      angle -= TPI;
  }

  if (fabs(angle - TPI) < EPSILON || fabs(angle + TPI) < EPSILON)
    angle = 0;

  if (angle < -PI)
    angle += TPI;
  if (angle > PI)
    angle -= TPI;

  // special case: 180 degrees are always represented by +180 degrees, not
  // by -180 degrees!
  if (fabs(angle + PI) < EPSILON)
    angle = PI;

  return angle;
}


template <class TScalarType>
YawPitchRoll3DTransform<TScalarType>
::YawPitchRoll3DTransform()
  : itk::MatrixOffsetTransformBase<TScalarType, 3, 3>()
{
  this->m_Roll = static_cast<TScalarType>(0);
  this->m_Pitch = static_cast<TScalarType>(0);
  this->m_Yaw = static_cast<TScalarType>(0);

  ParametersType pars;
  pars.SetSize(6);
  pars[0] = this->m_Roll;
  pars[1] = this->m_Pitch;
  pars[2] = this->m_Yaw;
  const OutputVectorType &vec = this->GetTranslation();
  pars[3] = vec[0];
  pars[4] = vec[1];
  pars[5] = vec[2];
  this->SetParameters(pars);
}

template <class TScalarType>
YawPitchRoll3DTransform<TScalarType>
::~YawPitchRoll3DTransform()
{

}

template <class TScalarType>
void
YawPitchRoll3DTransform<TScalarType>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "--- Yaw, Pitch, Roll Transform ---\n";
  os << indent << "Yaw (angle alpha): " << this->m_Yaw << "rad (" <<
    (this->m_Yaw / PI * 180.) << "deg)\n";
  os << indent << "Pitch (angle beta): " << this->m_Pitch << "rad (" <<
    (this->m_Pitch / PI * 180.) << "deg)\n";
  os << indent << "Roll (angle gamma): " << this->m_Roll << "rad (" <<
    (this->m_Roll / PI * 180.) << "deg)\n";
  os << indent << "Center of Rotation: " << this->GetCenter()[0] << "," <<
    this->GetCenter()[1] << "," << this->GetCenter()[2] << std::endl;
  os << indent << "Translation x: " << this->GetTranslation()[0] << std::endl;
  os << indent << "Translation y: " << this->GetTranslation()[1] << std::endl;
  os << indent << "Translation z: " << this->GetTranslation()[2] << std::endl;
  os << indent << "Parameters: " << this->m_Parameters << std::endl;
}

template <class TScalarType>
unsigned long
YawPitchRoll3DTransform<TScalarType>
::GetMTime() const
{
  return this->Superclass::GetMTime();
}

template <class TScalarType>
void
YawPitchRoll3DTransform<TScalarType>
::UpdateYPR()
{
  // current rotation matrix
  const MatrixType& m = this->GetMatrix();

  // pitch (beta angle) - important for singularities
  double pitch = atan2(-m[2][0], sqrt(m[2][1] * m[2][1] + m[2][2] * m[2][2]));
  double yaw = 0;
  double roll = 0;

  // roll (gamma), yaw (alpha)
  if (fabs(pitch - PIH) >= EPSILON && fabs(pitch + PIH) >= EPSILON)
  {
    yaw = atan2(m[1][0], m[0][0]);
    roll = atan2(m[2][1], m[2][2]);
  }
  else if (fabs(pitch - PIH) < EPSILON) // beta=PI/2 (singularity)
  {
    yaw = 0;
    roll = atan2(m[0][1], m[1][1]);
  }
  else // beta=-PI/2 (singularity)
  {
    yaw = 0;
    roll = -atan2(m[0][1], m[1][1]);
  }

  this->m_Yaw = ProjectAngleToNegativePositivePIRange(
      static_cast<TScalarType>(yaw));
  this->m_Pitch = ProjectAngleToNegativePositivePIRange(
      static_cast<TScalarType>(pitch));
  this->m_Roll = ProjectAngleToNegativePositivePIRange(
      static_cast<TScalarType>(roll));
}

template <class TScalarType>
void
YawPitchRoll3DTransform<TScalarType>
::SetParameters(const ParametersType &parameters)
{
  if (parameters.GetSize() != 6)
    return;

  bool modified = false;
  if (parameters.GetSize() != this->m_Parameters.GetSize()) // first call
    modified = true;

  // be sure that the range criteria are met:
  ParametersType pars = parameters;
  // roll [-PI;+PI]:
  pars[0] = ProjectAngleToNegativePositivePIRange(pars[0]);
  // pitch [-PI/2;+PI/2]:
  pars[1] = ProjectAngleToNegativePositivePIRange(pars[1]);
  if (pars[1] > PIH)
    pars[1] = PIH;
  else if (pars[1] < -PIH)
    pars[1] = -PIH;
  // yaw [-PI;+PI]:
  pars[2] = ProjectAngleToNegativePositivePIRange(pars[2]);

  for (unsigned int i = 0; i < 6; i++)
  {
    if (pars[i] != this->m_Parameters[i])
    {
      modified = true;
      break;
    }
  }

  if (modified)
  {
    this->m_Parameters = pars;

    // compute matrix and offset from current parameters
    this->ComputeMatrix();
    //set translation, this->ComputeOffset() is called internally
    OutputVectorType transl;
    transl[0] = pars[3];
    transl[1] = pars[4];
    transl[2] = pars[5];
    this->SetTranslation(transl);

    // update yaw, pitch, roll
    this->UpdateYPR();

    this->Modified();
  }
}

template <class TScalarType>
const typename YawPitchRoll3DTransform<TScalarType>::ParametersType&
YawPitchRoll3DTransform<TScalarType>
::GetParameters() const
{
  this->m_Parameters[0] = this->m_Roll;
  this->m_Parameters[1] = this->m_Pitch;
  this->m_Parameters[2] = this->m_Yaw;
  this->m_Parameters[3] = this->GetTranslation()[0];
  this->m_Parameters[4] = this->GetTranslation()[1];
  this->m_Parameters[5] = this->GetTranslation()[2];
  return this->m_Parameters;
}

template <class TScalarType>
void
YawPitchRoll3DTransform<TScalarType>
::ComputeMatrix()
{
  MatrixType m = this->GetMatrix();

  double ca = cos(this->m_Parameters[2]);
  double cb = cos(this->m_Parameters[1]);
  double cg = cos(this->m_Parameters[0]);
  double sa = sin(this->m_Parameters[2]);
  double sb = sin(this->m_Parameters[1]);
  double sg = sin(this->m_Parameters[0]);

  m[0][0] = ca * cb;
  m[0][1] = ca * sb * sg - sa * cg;
  m[0][2] = ca * sb * cg + sa * sg;

  m[1][0] = sa * cb;
  m[1][1] = sa * sb * sg + ca * cg;
  m[1][2] = sa * sb * cg - ca * sg;

  m[2][0] = -sb;
  m[2][1] = cb * sg;
  m[2][2] = cb * cg;

  this->SetMatrix(m);
}

template <class TScalarType>
void
YawPitchRoll3DTransform<TScalarType>
::SetYaw(TScalarType yaw)
{
  ParametersType newPars = this->GetParameters();
  newPars[2] = yaw;
  this->SetParameters(newPars); // internal clamping
}

template <class TScalarType>
void
YawPitchRoll3DTransform<TScalarType>
::SetPitch(TScalarType pitch)
{
  ParametersType newPars = this->GetParameters();
  newPars[1] = pitch;
  this->SetParameters(newPars); // internal clamping
}

template <class TScalarType>
void
YawPitchRoll3DTransform<TScalarType>
::SetRoll(TScalarType roll)
{
  ParametersType newPars = this->GetParameters();
  newPars[0] = roll;
  this->SetParameters(newPars); // internal clamping
}

template <class TScalarType>
void
YawPitchRoll3DTransform<TScalarType>
::SetMatrix(const MatrixType &matrix)
{
  // do not call this->Superclass::SetMatrix() in order to postpone the
  // this->Modified()-invocation:
  this->SetVarMatrix(matrix);
  this->ComputeOffset();
  this->UpdateYPR(); // force update of angles
  this->Modified(); // now we can signal modification
}


}


#endif /* ORAYAWPITCHROLL3DTRANSFORM_TXX_ */
