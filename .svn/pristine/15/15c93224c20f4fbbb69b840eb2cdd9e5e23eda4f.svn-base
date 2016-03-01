//
#ifndef ORAPROJECTIONGEOMETRY_H_
#define ORAPROJECTIONGEOMETRY_H_

//ITK
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace ora
{

/** \class ProjectionGeometry
 * \brief Defines the nature of a perspective projection suitable for DRR computation.
 *
 * This class implements the basic definition of a perspective projection
 * geometry as proposed in "plastimatch digitally reconstructed radiographs
 * (DRR) application programming interface (API)" (design document).
 *
 * Please,
 * refer to this design document in order to retrieve more information on the
 * assumptions and restrictions regarding projection geometry definition!
 *
 * <b>Tests</b>:<br>
 * TestProjectionGeometry.cxx <br>
 *
 * @author phil
 * @author Markus
 * @version 1.1
 */
class ProjectionGeometry : public itk::Object
{
public:
  /** Finite epsilon for floating point comparisons. **/
  static const double F_EPSILON;

  /** Standard class typedefs. **/
  typedef ProjectionGeometry Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. **/
  itkNewMacro(Self)

  /** Run-time type information (and related methods). **/
  itkTypeMacro(ProjectionGeometry, itk::Object)

  /** Set the source position (idealized focal spot where the rays emerge from)
   * in physical units. **/
  void SetSourcePosition(const double position[3]);
  /** Get the source position (idealized focal spot where the rays emerge from)
   * in physical units. **/
  const double *GetSourcePosition() const;

  /** Set the detector origin (center of the first transmitted pixel position)
   * in physical units. **/
  void SetDetectorOrigin(const double origin[3]);
  /** Get the detector origin (center of the first transmitted pixel position)
   * in physical units. **/
  const double *GetDetectorOrigin() const;

  /** Set the detector orientation (direction cosines of the "horizontal" row
   * vector and the "vertical" column vector). These vectors are internally
   * normalized if necessary. **/
  void SetDetectorOrientation(const double row[3], const double column[3]);
  /** Set a part of the detector orientation (direction cosines of the
   * "horizontal" row vector). This vector is internally
   * normalized if necessary. **/
  void SetDetectorRowOrientation(const double row[3]);
  /** Set a part of the detector orientation (direction cosines of the
   * "vertical" column vector). This vector is internally
   * normalized if necessary. **/
  void SetDetectorColumnOrientation(const double column[3]);
  /** Get a part of the detector orientation (direction cosines of the
   * "horizontal" row vector). Normalized vector. **/
  const double *GetDetectorRowOrientation() const;
  /** Get a part of the detector orientation (direction cosines of the
   * "vertical" column vector). Normalized vector. **/
  const double *GetDetectorColumnOrientation() const;

  /** Set the detector pixel spacing (distance from one pixel to another)
   * along row and column direction in physical units.  **/
  void SetDetectorPixelSpacing(const double spacing[2]);
  /** Get the detector pixel spacing (distance from one pixel to another)
   * along row and column direction in physical units.  **/
  const double *GetDetectorPixelSpacing() const;

  /** Set the detector size along row and column direction in pixels.  **/
  void SetDetectorSize(const int size[2]);
  /** Get the detector size along row and column direction in pixels.  **/
  const int *GetDetectorSize() const;

  /** @return TRUE if the geometry is basically valid (applicable in terms of
   * DRR computation);<br>
   * 1. row and column vector must be orthogonal<br>
   * 2. detector size and spacing must be greater than 0<br>
   * 3. source position must not lie within the detector plane **/
  virtual bool IsGeometryValid() const;

  /** Computes and returns the homogeneous 3x4-projection-matrix from the
   * currently configured projection geometry settings. The returned flat
   * array contains the matrix components where the column moves fastest. **/
  virtual double *Compute3x4ProjectionMatrix() const;

protected:
  /** Source position (idealized focal spot where the rays emerge from)
   * in physical units. **/
  double m_SourcePosition[3];
  /** Detector origin (center of the first transmitted pixel position)
   * in physical units. **/
  double m_DetectorOrigin[3];
  /** A part of the detector orientation (direction cosines of the
   * "horizontal" row vector). **/
  double m_DetectorRowOrientation[3];
  /** A part of the detector orientation (direction cosines of the
   * "vertical" column vector). **/
  double m_DetectorColumnOrientation[3];
  /** Detector pixel spacing (distance from one pixel to another)
   * along row and column direction in physical units.  **/
  double m_DetectorSpacing[2];
  /** Detector size along row and column direction in pixels.  **/
  int m_DetectorSize[2];
  /** Update helper. **/
  mutable unsigned long int m_LastMatrixTimeStamp;
  /** Homogeneous 3x4-projection-matrix according to the currently configured
   * projection geometry settings. **/
  mutable double m_ProjectionMatrix[12];
  /** Update helper. **/
  mutable unsigned long int m_LastValidationTimeStamp;
  /** Storage for validation. **/
  mutable bool m_GeometryValid;

  /** Default constructor. **/
  ProjectionGeometry();
  /** Default constructor. **/
  virtual ~ProjectionGeometry();
  /** Standard object output. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  /** Purposely not implemented. **/
  ProjectionGeometry(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#endif /* ORAPROJECTIONGEOMETRY_H_ */
