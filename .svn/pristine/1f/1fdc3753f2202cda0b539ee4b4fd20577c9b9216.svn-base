//
#ifndef ORAPARAMETRIZABLEIDENTITYTRANSFORM_TXX_
#define ORAPARAMETRIZABLEIDENTITYTRANSFORM_TXX_

#include "oraParametrizableIdentityTransform.h"

namespace ora
{

template<class TScalarType, unsigned int Dimensions>
ParametrizableIdentityTransform<TScalarType, Dimensions>::ParametrizableIdentityTransform() :
  Superclass(SpaceDimension, Dimensions * (Dimensions + 1)) // preliminary #parameters
{
  this->m_NumberOfConnectedTransformParameters = Dimensions * (Dimensions + 1);

  // initialize constant Jacobian matrix and parameters vector:
  this->m_Jacobian = JacobianType(SpaceDimension,
      this->m_NumberOfConnectedTransformParameters);
  this->m_Jacobian.Fill(0.0);
  this->m_Parameters.SetSize(this->m_NumberOfConnectedTransformParameters);
  this->m_Parameters.Fill(0);

  this->m_Connected3DTransform = NULL;
  this->m_StealJacobianFromConnected3DTransform = true;
}
template<class TScalarType, unsigned int Dimensions>
ParametrizableIdentityTransform<TScalarType, Dimensions>::~ParametrizableIdentityTransform()
{
  ;
}

template<class TScalarType, unsigned int Dimensions>
void ParametrizableIdentityTransform<TScalarType, Dimensions>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Space Dimension: " << SpaceDimension << std::endl;
  os << indent << "Number Of Connected Transform Parameters: "
      << m_NumberOfConnectedTransformParameters << std::endl;
  os << indent << "Parameters: " << this->m_Parameters << std::endl;
  os << indent << "Connected 3D Transform: "
      << (this->m_Connected3DTransform ? this->m_Connected3DTransform.GetPointer()
          : 0) << std::endl;
  os << indent << "Steal Jacobian From Connected 3D Transform: "
      << this->m_StealJacobianFromConnected3DTransform << std::endl;
}

/** Method for transforming a point. **/
template<class TScalarType, unsigned int Dimensions>
typename ParametrizableIdentityTransform<TScalarType, Dimensions>::OutputPointType ParametrizableIdentityTransform<
    TScalarType, Dimensions>::TransformPoint(const InputPointType &point) const
{
  return point;
}

template<class TScalarType, unsigned int Dimensions>
typename ParametrizableIdentityTransform<TScalarType, Dimensions>::OutputVectorType ParametrizableIdentityTransform<
    TScalarType, Dimensions>::TransformVector(const InputVectorType &vector) const
{
  return vector;
}

template<class TScalarType, unsigned int Dimensions>
typename ParametrizableIdentityTransform<TScalarType, Dimensions>::OutputVnlVectorType ParametrizableIdentityTransform<
    TScalarType, Dimensions>::TransformVector(const InputVnlVectorType &vector) const
{
  return vector;
}

template<class TScalarType, unsigned int Dimensions>
typename ParametrizableIdentityTransform<TScalarType, Dimensions>::OutputCovariantVectorType ParametrizableIdentityTransform<
    TScalarType, Dimensions>::TransformCovariantVector(
    const InputCovariantVectorType &vector) const
{
  return vector;
}

template<class TScalarType, unsigned int Dimensions>
void ParametrizableIdentityTransform<TScalarType, Dimensions>::SetMatrix(
    const MatrixType &matrix)
{
  ;
}

template<class TScalarType, unsigned int Dimensions>
void ParametrizableIdentityTransform<TScalarType, Dimensions>::SetIdentity()
{
  ; // identity by default (after instantiation)
}

template<class TScalarType, unsigned int Dimensions>
void ParametrizableIdentityTransform<TScalarType, Dimensions>::SetOffset(
    const OutputVectorType &offset)
{
  ; // 0-vector by default (after instantiation)
}

template<class TScalarType, unsigned int Dimensions>
void ParametrizableIdentityTransform<TScalarType, Dimensions>::SetCenter(
    const InputPointType &center)
{
  ; // 0-point by default (after instantiation)
}

template<class TScalarType, unsigned int Dimensions>
void ParametrizableIdentityTransform<TScalarType, Dimensions>::SetTranslation(
    const OutputVectorType& translation)
{
  ; // 0-vector by default (after instantiation)
}

template<class TScalarType, unsigned int Dimensions>
const typename ParametrizableIdentityTransform<TScalarType, Dimensions>::JacobianType& ParametrizableIdentityTransform<
    TScalarType, Dimensions>::GetJacobian(const InputPointType &point) const
{
  if (!this->m_Connected3DTransform
      || !this->m_StealJacobianFromConnected3DTransform)
  {
    return this->m_Jacobian;
  }
  else
  {
    typedef typename Generic3DTransformType::InputPointType Input3DPointType;

    Input3DPointType ip3;
    ip3.Fill(0);
    for (unsigned int d = 0; d < SpaceDimension && d < 3; d++)
      ip3[d] = point[d];

    return this->m_Connected3DTransform->GetJacobian(ip3);
  }
}

template<class TScalarType, unsigned int Dimensions>
typename ParametrizableIdentityTransform<TScalarType, Dimensions>::InverseTransformBasePointer ParametrizableIdentityTransform<
    TScalarType, Dimensions>::GetInverseTransform() const
{
  return this->New().GetPointer();
}

template<class TScalarType, unsigned int Dimensions>
bool ParametrizableIdentityTransform<TScalarType, Dimensions>::IsLinear() const
{
  return true;
}

template<class TScalarType, unsigned int Dimensions>
const typename ParametrizableIdentityTransform<TScalarType, Dimensions>::ParametersType& ParametrizableIdentityTransform<
    TScalarType, Dimensions>::GetFixedParameters() const
{
  return this->m_FixedParameters;
}

template<class TScalarType, unsigned int Dimensions>
void ParametrizableIdentityTransform<TScalarType, Dimensions>::SetFixedParameters(
    const ParametersType &paras)
{
  ;
}

template<class TScalarType, unsigned int Dimensions>
const typename ParametrizableIdentityTransform<TScalarType, Dimensions>::ParametersType& ParametrizableIdentityTransform<
    TScalarType, Dimensions>::GetParameters() const
{
  return this->m_Parameters;
}

template<class TScalarType, unsigned int Dimensions>
void ParametrizableIdentityTransform<TScalarType, Dimensions>::SetParameters(
    const ParametersType &paras)
{
  if (paras.GetSize() == this->m_NumberOfConnectedTransformParameters)
  {
    this->InvokeEvent(ora::BeforeParametersSet()); // signal (for sync)!

    bool modified = false;
    for (unsigned int i = 0; i < this->m_NumberOfConnectedTransformParameters; i++)
    {
      if (this->m_Parameters[i] != paras[i])
      {
        this->m_Parameters[i] = paras[i];
        modified = true;
      }
    }
    // always set the parameters to the connected transform to be sure that
    // they are synchronized (even if this transformation is not modified):
    if (this->m_Connected3DTransform)
      this->m_Connected3DTransform->SetParameters(this->m_Parameters);
    if (modified)
    {
      this->Modified();
      this->InvokeEvent(ora::TransformChanged()); // signal that!
    }

    this->InvokeEvent(ora::AfterParametersSet()); // signal (for sync)!
  }
  else
  {
    itkDebugMacro(<< "Wrong parameters dimension; awaiting " <<
        this->m_NumberOfConnectedTransformParameters << " parameters.")
  }
}

template<class TScalarType, unsigned int Dimensions>
void ParametrizableIdentityTransform<TScalarType, Dimensions>::SetConnected3DTransform(
    Generic3DTransformType *transform)
{
  if (transform != this->m_Connected3DTransform)
  {
    this->m_Connected3DTransform = transform;
    if (this->m_Connected3DTransform) // -> fix the number of parameters
    {
      this->m_NumberOfConnectedTransformParameters
          = this->m_Connected3DTransform->GetNumberOfParameters();
      // re-initialize constant Jacobian matrix and parameters vector:
      this->m_Jacobian = JacobianType(SpaceDimension,
          this->m_NumberOfConnectedTransformParameters);
      this->m_Jacobian.Fill(0.0);
      this->m_Parameters.SetSize(this->m_NumberOfConnectedTransformParameters);
      this->m_Parameters.Fill(0);
    }
    this->Modified();
  }
}

template<class TScalarType, unsigned int Dimensions>
void ParametrizableIdentityTransform<TScalarType, Dimensions>::SetNumberOfConnectedTransformParameters(
    unsigned int numPars)
{
  if (!this->m_Connected3DTransform) // only if there is no connected transform
  {
    this->m_NumberOfConnectedTransformParameters = numPars;

    // re-initialize constant Jacobian matrix and parameters vector:
    this->m_Jacobian = JacobianType(SpaceDimension,
        this->m_NumberOfConnectedTransformParameters);
    this->m_Jacobian.Fill(0.0);
    this->m_Parameters.SetSize(this->m_NumberOfConnectedTransformParameters);
    this->m_Parameters.Fill(0);

    this->Modified();
  }
}

}

#endif /* ORAPARAMETRIZABLEIDENTITYTRANSFORM_TXX_ */
