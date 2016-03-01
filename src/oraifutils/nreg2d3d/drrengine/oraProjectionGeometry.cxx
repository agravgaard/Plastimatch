//
#include "oraProjectionGeometry.h"

//std
#include <math.h>
#include <vector>

#include <itkMatrix.h>
#include <vnl/algo/vnl_svd.h>

namespace ora
{

const double ProjectionGeometry::F_EPSILON = 1e-6;

ProjectionGeometry::ProjectionGeometry()
{
  for (int d = 0; d < 3; d++)
  {
    m_SourcePosition[d] = 0;
    m_DetectorOrigin[d] = 0;
    m_DetectorRowOrientation[d] = 0;
    m_DetectorColumnOrientation[d] = 0;
  }
  for (int d = 0; d < 2; d++)
  {
    m_DetectorSpacing[d] = 0;
    m_DetectorSize[d] = 0;
  }
  for (int d = 0; d < 12; d++)
    m_ProjectionMatrix[d] = 0;
  m_LastValidationTimeStamp = 0;
  m_GeometryValid = false;
  m_LastMatrixTimeStamp = 0;
}

ProjectionGeometry::~ProjectionGeometry()
{
  ;
}

// Helper that prints the specified array components to os separated by ",".
template<typename T> void PrintArray(std::ostream &os, int count, const T *arr)
{
  for (int i = 0; i < count; i++)
  {
    if (i < (count - 1))
      os << arr[i] << ",";
    else
      os << arr[i];
  }
}

void ProjectionGeometry::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << "Source position: "; PrintArray<double>(os, 3, m_SourcePosition); os << std::endl;
  os << "Detector origin: "; PrintArray<double>(os, 3, m_DetectorOrigin); os << std::endl;
  os << "Detector row orientation: "; PrintArray<double>(os, 3, m_DetectorRowOrientation); os << std::endl;
  os << "Detector column orientation: "; PrintArray<double>(os, 3, m_DetectorColumnOrientation); os << std::endl;
  os << "Detector spacing: "; PrintArray<double>(os, 2, m_DetectorSpacing); os << std::endl;
  os << "Detector size: "; PrintArray<int>(os, 2, m_DetectorSize); os << std::endl;
  os << "Projection matrix: "; PrintArray<double>(os, 12, m_ProjectionMatrix); os << std::endl;
  os << "Geometry valid: " << m_GeometryValid << std::endl;
}

bool ProjectionGeometry::IsGeometryValid() const
{
  if (this->GetMTime() > m_LastValidationTimeStamp)
  {
    m_LastValidationTimeStamp = this->GetMTime();

    m_GeometryValid = false;
    // 1. orthogonal row/column vectors required:
    double dot = m_DetectorRowOrientation[0] * m_DetectorColumnOrientation[0] +
        m_DetectorRowOrientation[1] * m_DetectorColumnOrientation[1] +
        m_DetectorRowOrientation[2] * m_DetectorColumnOrientation[2];
    if (fabs(dot) < F_EPSILON)
    {
      // 2. detector size and spacing >0 required:
      bool valid = true;
      for (int d = 0; d < 2; d++)
      {
        if (m_DetectorSize[d] <= 0)
          valid = false;
        if (m_DetectorSpacing[d] < F_EPSILON)
          valid = false;
      }
      if (valid)
      {
        // 3. source position out-of-plane required:
        double n[3];
        n[0] = m_DetectorRowOrientation[1] * m_DetectorColumnOrientation[2] -
            m_DetectorRowOrientation[2] * m_DetectorColumnOrientation[1];
        n[1] = m_DetectorRowOrientation[2] * m_DetectorColumnOrientation[0] -
            m_DetectorRowOrientation[0] * m_DetectorColumnOrientation[2];
        n[2] = m_DetectorRowOrientation[0] * m_DetectorColumnOrientation[1] -
            m_DetectorRowOrientation[1] * m_DetectorColumnOrientation[0];
        double dist = fabs(n[0] * (m_SourcePosition[0] - m_DetectorOrigin[0]) +
            n[1] * (m_SourcePosition[1] - m_DetectorOrigin[1]) +
            n[2] * (m_SourcePosition[2] - m_DetectorOrigin[2]));
        if (dist > F_EPSILON)
        {
          m_GeometryValid = true; // DONE!
        }
      }
    }
    return m_GeometryValid;
  }
  else // up-to-date
  {
    return m_GeometryValid;
  }
}

double *ProjectionGeometry::Compute3x4ProjectionMatrix() const
{
  if (this->GetMTime() > m_LastMatrixTimeStamp)
  {
    /* Solve the homogeneous system of linear equations for the 3*4 perspective
     * projection matrix M. Each correspondence between a 3D point $X = (x, y, z)^T$
     * and a 2D image point $x = (u, v)^T$ gives one equation:
     * $(\alpha u, \alpha v, \alpha)^T = M (x, y, z, 1)^T
     *
     * This means every known point correspondence gives two linear equations
     * with 12 unknowns each. At n known point correspondences a linear system
     * can be defined by a 2n*12 matrix:
     * $Am=\left[\begin{array}{cccccccccccc}
       x_{1} & y_{1} & z_{1} & 1 & 0 & 0 & 0 & 0 & -u_{1}x_{1} & -u_{1}y_{1} & -u_{1}z_{1} & -u_{1}\\
       0 & 0 & 0 & 0 & x_{1} & y_{1} & z_{1} & 1 & -v_{1}x_{1} & -v_{1}y_{1} & -v_{1}z_{1} & -v_{1}\\
       &  &  &  &  &  &  &  & \vdots\end{array}\right]\left[\begin{array}{c}
       m_{11}\\
       m_{12}\\
       \vdots\\
       m_{34}\end{array}\right]=0$
     *
     * With 6 corresponding points the system is over-determined, and can be
     * solved for m. The linear system Am=0 then has 11 unknowns in 12 equations.
     * There is always a trivial solution m=0 because A*0=0. We are interested
     * in m!=0, which implies that A has to have a rank of at least 11.
     * The solution is to find the right zero space m, because A maps to zero.
     * The zero space can be found by singular value decomposition (SVD).
     * SVD is a linear algebraic technique to solve linear equations in a least
     * square sense (closest possible solution) and works on general matrices
     * (even singular and close to singular matrices).
     * Any m*n matrix A, with m>=n can be decomposed into three matrices A=UDV^T.
     * Matrix U has orthonormal columns, D is a non-negative diagonal matrix and
     * V^T has orthonormal rows. The solution is the column of V with the smallest
     * singular value $\sigma_{n}$ in D. The smallest singular value would be zero if
     * all data is exact, which is generally not the case in floating point arithmetics.
     */

    // Typedefs
    typedef itk::Point<double, 3> Point3DType;
    typedef itk::Vector<double, 3> Vector3DType;

    // Define points in 3d space to determine the projection matrix from 3D/2d
    // point correspondences
    // Use 27 points around [0,0,0] for better numerical stability
    std::vector<Point3DType> points3D;
    std::vector<Point3DType>::iterator pointIt;
    for (int x = -1; x <= 1; ++x)
    {
      for (int y = -1; y <= 1; ++y)
      {
        for (int z = -1; z <= 1; ++z)
        {
          Point3DType point;
          point[0] = x;
          point[1] = y;
          point[2] = z;
          points3D.push_back(point);
        }
      }
    }

    // Determine the intersection of the point-source line with the DRR plane
    // DRR plane normal
    Vector3DType colOrientation = m_DetectorColumnOrientation;
    colOrientation.Normalize();
    Vector3DType rowOrientation = m_DetectorRowOrientation;
    rowOrientation.Normalize();
    Vector3DType detectorNormal = itk::CrossProduct(rowOrientation, colOrientation);
    detectorNormal.Normalize();
    // Required 3D points
    Point3DType detectorOrigin = m_DetectorOrigin;
    Point3DType sourcePosition = m_SourcePosition;

    // Compute intersection
    std::vector<Point3DType> points3DProjected;
    for (pointIt = points3D.begin(); pointIt != points3D.end(); ++pointIt)
    {
      double u = (detectorNormal * (detectorOrigin - sourcePosition)) /
          (detectorNormal * (*pointIt - sourcePosition));
      Point3DType detectorIntersection = sourcePosition + u * (*pointIt - sourcePosition);
      points3DProjected.push_back(detectorIntersection);
    }

    // Transform to pixel coordinates by a change of the basis
    typedef itk::Matrix<double, 3, 3> Matrix3x3Type;
    Matrix3x3Type planeBasis;
    rowOrientation *= m_DetectorSpacing[0];
    planeBasis(0, 0) = rowOrientation[0];
    planeBasis(1, 0) = rowOrientation[1];
    planeBasis(2, 0) = rowOrientation[2];
    colOrientation *= m_DetectorSpacing[1];
    planeBasis(0, 1) = colOrientation[0];
    planeBasis(1, 1) = colOrientation[1];
    planeBasis(2, 1) = colOrientation[2];
    planeBasis(0, 2) = detectorNormal[0];
    planeBasis(1, 2) = detectorNormal[1];
    planeBasis(2, 2) = detectorNormal[2];
    Matrix3x3Type planeBasisInv;
    planeBasisInv = planeBasis.GetInverse();

    std::vector<Point3DType> points2D;
    for (pointIt = points3DProjected.begin(); pointIt != points3DProjected.end(); ++pointIt)
    {
      Point3DType point = *pointIt;
      point[0] -= detectorOrigin[0];
      point[1] -= detectorOrigin[1];
      point[2] -= detectorOrigin[2];
      point = planeBasisInv * point;
      points2D.push_back(point);
    }

    // Create 2n*12 matrix A of the linear system
    itk::Matrix<double, 2*27, 12> A;
    for (unsigned int i = 0; i < points3D.size(); ++i)
    {
      const double x = points3D[i][0];
      const double y = points3D[i][1];
      const double z = points3D[i][2];
      const double u = points2D[i][0];
      const double v = points2D[i][1];
      A(2*i, 0) = x;
      A(2*i, 1) = y;
      A(2*i, 2) = z;
      A(2*i, 3) = 1.0;
      A(2*i, 4) = 0.0;
      A(2*i, 5) = 0.0;
      A(2*i, 6) = 0.0;
      A(2*i, 7) = 0.0;
      A(2*i, 8) = -u*x;
      A(2*i, 9) = -u*y;
      A(2*i, 10) = -u*z;
      A(2*i, 11) = -u;
      A(2*i+1, 0) = 0.0;
      A(2*i+1, 1) = 0.0;
      A(2*i+1, 2) = 0.0;
      A(2*i+1, 3) = 0.0;
      A(2*i+1, 4) = x;
      A(2*i+1, 5) = y;
      A(2*i+1, 6) = z;
      A(2*i+1, 7) = 1.0;
      A(2*i+1, 8) = -v*x;
      A(2*i+1, 9) = -v*y;
      A(2*i+1, 10) = -v*z;
      A(2*i+1, 11) = -v;
    }

    // Compute singular value decomposition of A
    vnl_svd<double> svd(A.GetVnlMatrix());

    // Get last column of V (has smallest singular value) and convert it to a matrix
    if (svd.rank() < 12)
      return NULL;
    svd.nullvector().copy_out(m_ProjectionMatrix);

    m_LastMatrixTimeStamp = this->GetMTime();
    return m_ProjectionMatrix;
  }
  else // up-to-date
  {
    return m_ProjectionMatrix;
  }
}

// Helper for setting a member variable and updating thiss' modified state.
template<typename T> inline void TakeOverModified(int count,
    T *member, const T *var, ProjectionGeometry *thiss)
{
  bool modified = false;
  for (int i = 0; i < count; i++)
  {
	if (vnl_math_abs(member[i] - var[i]) > ProjectionGeometry::F_EPSILON)
      modified = true;
    member[i] = var[i]; // take over
  }
  if (modified)
    thiss->Modified(); // signal
}

// Helper for normalizing and setting a member variable and updating thiss'
// modified state.
template<typename T> inline void NormalizeTakeOverModified3D(
    T *member, const T *var, ProjectionGeometry *thiss)
{
  double ivar[3] = {0, 0, 0};
  double norm = sqrt(var[0] * var[0] + var[1] * var[1] + var[2] * var[2]);
  if (norm != 0)
  {
    ivar[0] = var[0] / norm;
    ivar[1] = var[1] / norm;
    ivar[2] = var[2] / norm;
  }
  bool modified = false;
  for (int i = 0; i < 3; i++)
  {
    if (fabs(member[i] - ivar[i]) > ProjectionGeometry::F_EPSILON)
      modified = true;
    member[i] = ivar[i]; // take over
  }
  if (modified)
    thiss->Modified(); // signal
}

void ProjectionGeometry::SetSourcePosition(const double position[3])
{
  TakeOverModified<double>(3, m_SourcePosition, position, this);
}

const double *ProjectionGeometry::GetSourcePosition() const
{
  return m_SourcePosition;
}

void ProjectionGeometry::SetDetectorOrigin(const double origin[3])
{
  TakeOverModified<double>(3, m_DetectorOrigin, origin, this);
}

const double *ProjectionGeometry::GetDetectorOrigin() const
{
  return m_DetectorOrigin;
}

void ProjectionGeometry::SetDetectorOrientation(const double row[3],
		const double column[3])
{
  NormalizeTakeOverModified3D<double>(m_DetectorRowOrientation, row, this);
  NormalizeTakeOverModified3D<double>(m_DetectorColumnOrientation, column, this);
}

void ProjectionGeometry::SetDetectorRowOrientation(const double row[3])
{
  NormalizeTakeOverModified3D<double>(m_DetectorRowOrientation, row, this);
}

void ProjectionGeometry::SetDetectorColumnOrientation(const double column[3])
{
  NormalizeTakeOverModified3D<double>(m_DetectorColumnOrientation, column, this);
}

const double *ProjectionGeometry::GetDetectorRowOrientation() const
{
  return m_DetectorRowOrientation;
}

const double *ProjectionGeometry::GetDetectorColumnOrientation() const
{
  return m_DetectorColumnOrientation;
}

void ProjectionGeometry::SetDetectorPixelSpacing(const double spacing[2])
{
  TakeOverModified<double>(2, m_DetectorSpacing, spacing, this);
}

const double *ProjectionGeometry::GetDetectorPixelSpacing() const
{
  return m_DetectorSpacing;
}

void ProjectionGeometry::SetDetectorSize(const int size[2])
{
  TakeOverModified<int>(2, m_DetectorSize, size, this);
}

const int *ProjectionGeometry::GetDetectorSize() const
{
  return m_DetectorSize;
}

}
