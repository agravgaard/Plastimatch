//
#ifndef ORAPARAMETRIZABLEIDENTITYTRANSFORM_H_
#define ORAPARAMETRIZABLEIDENTITYTRANSFORM_H_

#include <itkMatrixOffsetTransformBase.h>
#include <itkEventObject.h>

namespace ora
{

/** Before parameters set event: is fired right before setting the parameters.**/
itkEventMacro(BeforeParametersSet, itk::AnyEvent)
/** Transform changed event: is fired when the parameters are changed. **/
itkEventMacro(TransformChanged, itk::AnyEvent)
/** After parameters set event: is fired right after setting the parameters.**/
itkEventMacro(AfterParametersSet, itk::AnyEvent)

/** \class ParametrizableIdentityTransform
 * \brief Has static identity matrix at its output, but can be parametrized arbitrarily.
 *
 * Produces a static identity matrix of specified dimension at its output, but
 * can be parametrized with arbitrary parameters of a specified size. This means
 * that the output of this transformation is completely independent of the
 * parametrization. The parameters are not modified by the transform, therefore,
 * the class simply returns the parameters previously set.
 *
 * NOTE: if the parameters are changed (really changed compared to the previous
 * parameters), the TransformChanged()-event is fired.
 *
 * NOTE: the BeforeParametersSet()-event is fired right before the parameters
 * are set using SetParameters(). The AfterParametersSet()-event is fired right
 * after the parameters are set using SetParameters(). These events could be
 * useful for synchronizing the transform access of a set of parametrizable
 * identity transforms to a common connected 3D transform in a threaded
 * environment! BTW: these events are fired regardless whether the parameters
 * are really changed or not!
 *
 * For example this transformation type can be useful for the purpose of 2D/3D-
 * registration where the planar images are registered per definition, and the
 * transformation solely defines the current 3D position of the optimizer.
 *
 * A connected transformation can be defined which will receive the set
 * parameters and will - if configured - deliver the Jacobian of this
 * transformation. This can be useful for 2D/3D-registration where the connected
 * transformation is typically the 3D transformation that influences DRR-
 * computation. Some metrics utilize the Jacobian of the transformation to
 * estimate the partial derivatives for a specified position, therefore, this
 * transformation can be configured to 'steal' the Jacobian from the connected
 * transformation. In this case a 2D input point is mapped to a 3D point with
 * 3rd dimension mapped to zero. NOTE: It is still a fundamental whether or not
 * it makes sense to use derivative-based optimizers with metrics that estimate
 * the partial derivatives from the Jacobian of the transformation!
 *
 * This class is templated over the scalar type (TScalarType) used for
 * coordinate value representation and the dimension of the transform
 * (Dimensions).
 *
 * @see ora::MultiResolutionNWay2D3DRegistrationMethod
 * @see ora::MatrixOffsetTransformBase
 *
 * @author phil 
 * @version 1.3
 *
 * \ingroup Transforms
 */
template<class TScalarType, unsigned int Dimensions>
class ParametrizableIdentityTransform:
    public itk::MatrixOffsetTransformBase<TScalarType, Dimensions, Dimensions>
{
public:
  /** Standard class typedefs. */
  typedef ParametrizableIdentityTransform Self;
  typedef itk::MatrixOffsetTransformBase<TScalarType, Dimensions, Dimensions>
      Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(Rigid2DTransform, MatrixOffsetTransformBase)

  /** New macro for creation of through a Smart Pointer */
  itkNewMacro(Self)

  /** Dimension of the domain space. */
  itkStaticConstMacro(SpaceDimension, unsigned int, Dimensions);

  /** Types from Superclass. */
  typedef TScalarType ScalarType;
  typedef typename Superclass::ParametersType ParametersType;
  typedef typename Superclass::JacobianType JacobianType;

  /** Generic 3D transformation type. **/
  typedef itk::Transform<TScalarType, 3, 3> Generic3DTransformType;
  typedef typename Generic3DTransformType::Pointer Generic3DTransformPointer;

  /** Standard matrix type for this class. */
  typedef itk::Matrix<TScalarType, itkGetStaticConstMacro(SpaceDimension),
      itkGetStaticConstMacro(SpaceDimension)> MatrixType;

  /** Standard vector types for this class. */
  typedef itk::Vector<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
      InputVectorType;
  typedef InputVectorType OutputVectorType;

  /** Standard covariant vector types for this class */
  typedef itk::CovariantVector<TScalarType,
      itkGetStaticConstMacro(SpaceDimension)> InputCovariantVectorType;
  typedef InputCovariantVectorType OutputCovariantVectorType;

  /** Standard vnl_vector types for this class. */
  typedef vnl_vector_fixed<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
      InputVnlVectorType;
  typedef InputVnlVectorType OutputVnlVectorType;

  /** Standard coordinate point type for this class */
  typedef itk::Point<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
      InputPointType;
  typedef InputPointType OutputPointType;

  /**
   * Base inverse transform type. This type should not be changed to the
   * concrete inverse transform type or inheritance would be lost.
   */
  typedef typename Superclass::InverseTransformBaseType
      InverseTransformBaseType;
  typedef typename InverseTransformBaseType::Pointer
      InverseTransformBasePointer;

  /** Method for transforming a point. **/
  OutputPointType TransformPoint(const InputPointType &point) const;

  /** Method for transforming a vector. **/
  OutputVectorType TransformVector(const InputVectorType &vector) const;
  /** Method for transforming a vnl_vector. **/
  OutputVnlVectorType TransformVector(const InputVnlVectorType &vector) const;
  /** Method for transforming a CovariantVector. **/
  OutputCovariantVectorType TransformCovariantVector(
      const InputCovariantVectorType &vector) const;

  /** Set the transformation matrix (has no effect, always identity!). **/
  void SetMatrix(const MatrixType &matrix);
  /** Set the transformation to an Identity (clear here). **/
  void SetIdentity();

  /** Set the offset (has no effect, always 0-vector!). **/
  void SetOffset(const OutputVectorType &offset);

  /** Set the center of transformation (has no effect, always 0-point!). **/
  void SetCenter(const InputPointType &center);

  /** Set the translation (has no effect, always 0-vector!). **/
  void SetTranslation(const OutputVectorType& translation);

  /**
   * Compute the Jacobian of the transformation
   *
   * This method computes the Jacobian matrix of the transformation.
   * given point or vector, returning the transformed point or
   * vector. The rank of the Jacobian will also indicate if the transform
   * is invertible at this point.
   *
   * The Jacobian can be expressed as a set of partial derivatives of the
   * output point components with respect to the parameters that defined
   * the transform:
   *
   * \f[
   *
   * J=\left[ \begin{array}{cccc}
   * \frac{\partial x_{1}}{\partial p_{1}} &
   * \frac{\partial x_{2}}{\partial p_{1}} &
   * \cdots  & \frac{\partial x_{n}}{\partial p_{1}}\\
   *    \frac{\partial x_{1}}{\partial p_{2}} &
   * \frac{\partial x_{2}}{\partial p_{2}} &
   * \cdots  & \frac{\partial x_{n}}{\partial p_{2}}\\
   *    \vdots  & \vdots  & \ddots  & \vdots \\
   *    \frac{\partial x_{1}}{\partial p_{m}} &
   * \frac{\partial x_{2}}{\partial p_{m}} &
   * \cdots  & \frac{\partial x_{n}}{\partial p_{m}}
   * \end{array}\right]
   *
   * \f]
   *
   * @see m_StealJacobianFromConnected3DTransform
   */
  const JacobianType& GetJacobian(const InputPointType &point) const;

  /**
   * Return an inverse of the identity transform - another identity transform.
   **/
  InverseTransformBasePointer GetInverseTransform() const;

  /**
   * Indicates that this transform is linear. That is, given two
   * points P and Q, and scalar coefficients a and b, then
   * T( a*P + b*Q ) = a * T(P) + b * T(Q)
   */
  bool IsLinear() const;

  /** Get the Fixed Parameters. */
  const ParametersType& GetFixedParameters() const;

  /** Set the fixed parameters and update internal transformation. */
  void SetFixedParameters(const ParametersType &paras);

  /** Get the Parameters (just reflection of set parameters!). */
  const ParametersType& GetParameters() const;

  /**
   * Set the parameters. These parameters do not have an effect on this
   * transform, the parameters are delegated to the connected 3D transform.
   * NOTE: if the parameters are changed (really changed compared to the
   * previous parameters), the TranformChanged()-event is fired.
   */
  void SetParameters(const ParametersType &paras);

  virtual void SetConnected3DTransform(Generic3DTransformType *transform);
  itkGetObjectMacro(Connected3DTransform, Generic3DTransformType)

  itkSetMacro(StealJacobianFromConnected3DTransform, bool)
  itkGetMacro(StealJacobianFromConnected3DTransform, bool)

  /**
   * Set number of connected transform's parameters. Will be initialized with
   * Dimensions * (Dimensions + 1).
   * NOTE: This member is not changeable as soon as there is a connected
   * transform set. In this case the number of parameters will automatically
   * be taken over and fixed as long as there is a reference to the connected
   * transform!
   * NOTE: Setting a NEW number of transform parameters will cause the
   * parameters to be set to a zero vector.
   */
  virtual void SetNumberOfConnectedTransformParameters(unsigned int numPars);
  itkGetMacro(NumberOfConnectedTransformParameters, unsigned int)

protected:
  /** Type for thread-safety. **/
  typedef itk::SimpleFastMutexLock MutexType;

  /**
   * Optional connected transform which will automatically receive the set
   * parameters, and which will be used to return the Jacobian (if configured).
   * @see m_StealJacobianFromConnected3DTransform
   **/
  Generic3DTransformPointer m_Connected3DTransform;
  /**
   * Flag determining whether the returned Jacobian of this transform is
   * constant (FALSE) or is 'stolen' from the connected transformation (TRUE
   * and connected transformation must be set); <br> NOTE: The transformation
   * will not automatically receive the parameters after setting it using
   * SetConnectTransform(); you will manually have to set the parameters of this
   * class again to be sure that the connected transform has the same
   * parameters!
   * @see m_Connected3DTransform
   */
  bool m_StealJacobianFromConnected3DTransform;
  /**
   * Number of connected transform's parameters. Will be initialized with
   * Dimensions * (Dimensions + 1).
   */
  unsigned int m_NumberOfConnectedTransformParameters;

  /** Default constructor **/
  ParametrizableIdentityTransform();
  /** Destructor **/
  virtual ~ParametrizableIdentityTransform();

  /** Print-out object information. **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  /** Purposely not implemented **/
  ParametrizableIdentityTransform(const Self&);
  /** Purposely not implemented **/
  void operator=(const Self&);

};

}

#include "oraParametrizableIdentityTransform.txx"

#endif /* ORAPARAMETRIZABLEIDENTITYTRANSFORM_H_ */
