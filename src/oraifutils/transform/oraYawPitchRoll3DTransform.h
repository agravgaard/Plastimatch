

#ifndef ORAYAWPITCHROLL3DTRANSFORM_H_
#define ORAYAWPITCHROLL3DTRANSFORM_H_


#include <itkSmartPointer.h>
#include <itkMatrixOffsetTransformBase.h>


namespace ora
{


// NOTE: for some weird reasons I had to place these constants here instead
// of class scope (did not compile on MinGW, only on linux).

/** value of PI **/
static const double PI = 3.14159265358979323846;
/** value of PI/2 **/
static const double PIH = 1.57079632679489661923;


/** \class YawPitchRoll3DTransform
 * \brief Implements Yaw-Pitch-Roll transformation in 3D space (incl. translations).
 *
 * Implements Yaw-Pitch-Roll transformation in 3D space with additional
 * translations. It is therefore suitable for modeling rigid transformations in
 * 3D space given a fixed coordinate system. The transformation performs roll
 * first, then pitch and finally yaw. These rotations can be centered around an
 * arbitrary rotational center in 3D space.
 *
 * NOTE: <b>There are singularities at pitch=-PI/2 and pitch=+PI/2. In both of
 * these cases yaw will be mapped to 0 and roll will hold the angle. This is due
 * to ambiguities when yaw and roll 'coincide'.</b>
 *
 * CONVENTION: <br>
 * Yaw, pitch and roll define the rotations around a fixed right-handed (world)
 * coordinate system where <b>roll</b> is the rotation around the x-axis
 * (angle gamma in radians), <b>pitch</b> is the rotation around the y-axis
 * (angle beta in radians) and <b>yaw</b> is the rotation around the z-axis
 * (angle alpha in radians). <b>Due to the nature of this transformation type
 * roll, yaw and pitch have different range limitations. While yaw and roll are
 * allowed to equal real angles within [-PI;+PI], pitch is limited to the
 * range [-PI/2;+PI/2]!</b>
 *
 * NOTE: <br>
 * The center of rotation, however, lies in the point defined by Center.
 *
 * The parameters of this transformation are ordered as follows: <br>
 * roll, pitch, gamma, x-translation, y-translation, z-translation <b>
 * (all angles in radians).
 *
 * @see itk::MatrixOffsetTransformBase
 *
 * @author phil 
 * @version 1.0
 */
template <class TScalarType>
class YawPitchRoll3DTransform
  : public itk::MatrixOffsetTransformBase<TScalarType, 3, 3>
{
public:
  /** Standard class typedefs. */
  typedef YawPitchRoll3DTransform Self;
  typedef itk::MatrixOffsetTransformBase<TScalarType, 3, 3> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** inherited types **/
  typedef typename Superclass::MatrixType MatrixType;
  typedef typename Superclass::ParametersType ParametersType;
  typedef typename Superclass::OutputVectorType OutputVectorType;


  /** Run-time type information (and related methods). */
  itkTypeMacro(Self, Superclass);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /**
   * Tool method for projecting an arbitrary angle into negative/positive
   * PI-range ]-PI;+PI]. NOTE: 180 degrees are always represented by +180
   * degrees, not by -180 degrees!
   * @param angle angle in radians (possibly outside ]-PI;+PI])
   * @return angle in radians projected into ]-PI;+PI]-range
   **/
  static const TScalarType ProjectAngleToNegativePositivePIRange(
      TScalarType angle);

  /**
   * @return object's modified time depending on the modified times of its
   * internal components.
   * @see itk::Object#GetMTime()
   */
  virtual unsigned long GetMTime() const;

  /** Print description of this object. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Get current yaw (angle alpha, around z-axis) in radians. Range limitation:
   * [-PI;+PI].
   **/
  itkGetMacro(Yaw, TScalarType)
  /**
   * Set current yaw (angle alpha, around z-axis) in radians. Range limitation:
   * [-PI;+PI].
   **/
  virtual void SetYaw(TScalarType yaw);

  /**
   * Get current pitch (angle beta, around y-axis) in radians. Range limitation:
   * [-PI/2;+PI/2].
   **/
  itkGetMacro(Pitch, TScalarType)
  /**
   * Set current pitch (angle beta, around y-axis) in radians. Range limitation:
   * [-PI/2;+PI/2].
   **/
  virtual void SetPitch(TScalarType pitch);

  /**
   * Get current roll (angle gamma, around x-axis) in radians. Range limitation:
   * [-PI;+PI].
   **/
  itkGetMacro(Roll, TScalarType)
  /**
   * Set current roll (angle gamma, around x-axis) in radians. Range limitation:
   * [-PI;+PI].
   **/
  virtual void SetRoll(TScalarType roll);

  /**
   * Set the complete transformation from a set of parameters at once. The first
   * three parameters define the rotation (roll - rotation around x-axis,
   * pitch - rotation around y-axis, yaw - rotation around z-axis) and the next
   * three parameters define the translation (x-translation, y-translation and
   * z-translation).
   * @see itk::MatrixOffsetTransformBase::SetParameters()
   */
  void SetParameters(const ParametersType &parameters);

  /**
   * Get the Transformation Parameters.
   * @see itk::MatrixOffsetTransformBase::GetParameters()
   */
  const ParametersType& GetParameters(void) const;

  /**
   * Set matrix of the internal itk::MatrixOffsetTransformBase. However, this
   * overridden method ensures that the internal Yaw/Pitch/Roll angles are
   * updated accordingly.
   * @see itk::MatrixOffsetTransformBase#SetMatrix()
   */
  virtual void SetMatrix(const MatrixType &matrix);

protected:
  /** epsilon value for floating point comparisons **/
  static const double EPSILON;

  /**
   * Roll (rotation around fixed x-axis in radians). Range limitation:
   * [-PI;+PI].
   **/
  TScalarType m_Roll;
  /**
   * Pitch (rotation around fixed y-axis in radians). Range limitation:
   * [-PI/2;+PI/2].
   **/
  TScalarType m_Pitch;
  /**
   * Yaw (rotation around fixed z-axis in radians). Range limitation:
   * [-PI;+PI].
   **/
  TScalarType m_Yaw;

  /** Default constructor. **/
  YawPitchRoll3DTransform();
  /** Default destructor. **/
  virtual ~YawPitchRoll3DTransform();

  /**
   * Update internal yaw, pitch and roll angles w.r.t. the singularities that
   * occur at pitch=PI/2 and pitch=-PI/2.
   */
  void UpdateYPR();

  /**
   * Compute internal rotation matrix from current parameters.
   * @see itk::MatrixOffsetTransformBase::ComputeMatrix()
   */
  void ComputeMatrix();

private:
  /** Purposely not implemented. **/
  YawPitchRoll3DTransform(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};


}


#include "oraYawPitchRoll3DTransform.txx"


#endif /* ORAYAWPITCHROLL3DTRANSFORM_H_ */
