//
#include "vtkImplicitTriangulatedPolyData.h"

#include <math.h>

#include <vtkPolygon.h>
#include <vtkPlane.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>


vtkImplicitTriangulatedPolyData::vtkImplicitTriangulatedPolyData()
{
  TriangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  TriangleFilter->PassVertsOff();
  TriangleFilter->PassLinesOff();
  InputPD = NULL;
  CellLocator = vtkSmartPointer<vtkCellLocator>::New();
  UsePointNormals = false;
}

void vtkImplicitTriangulatedPolyData::SetInput(vtkPolyData *input)
{
  if (InputPD != input)
  {
    // be sure that input poly data is triangulated!
    TriangleFilter->SetInput(input);
    TriangleFilter->Update();
    // build cell locator
    InputPD = TriangleFilter->GetOutput();
    CellLocator->SetDataSet(InputPD);
    CellLocator->BuildLocator();
    this->Modified();
  }
}

unsigned long vtkImplicitTriangulatedPolyData::GetMTime()
{
  unsigned long mTime = this->vtkImplicitFunction::GetMTime();
  if (InputPD != NULL)
  {
    InputPD->Update();
    unsigned long inputMTime = InputPD->GetMTime();
    mTime = (inputMTime > mTime ? inputMTime : mTime);
  }
  return mTime;
}

vtkImplicitTriangulatedPolyData::~vtkImplicitTriangulatedPolyData()
{
  TriangleFilter = NULL;
  InputPD = NULL;
  CellLocator = NULL;
}

double vtkImplicitTriangulatedPolyData::EvaluateFunction(double x[3])
{
  if (!InputPD || InputPD->GetNumberOfCells() == 0)
  {
    vtkErrorMacro(<< "Insufficient input poly data!");
    return VTK_LARGE_FLOAT; // some value
  }

  // find the closest point and closest cell to query-point x[3]:
  double cp[3];
  double sdist; // squared distance to the closest point
  vtkIdType cID; // the cell ID of the cell containing the closest point
  int sID; // some unused sub ID
  CellLocator->FindClosestPoint(x, cp, cID, sID, sdist);
  vtkCell *cell = InputPD->GetCell(cID);
  if (cell)
  {
    vtkPoints *pts = cell->GetPoints();
    double n[3]; // normal

    if (!UsePointNormals) // compute closest cell's normal
    {
      vtkPolygon::ComputeNormal(pts, n);
    }
    else // closest point's normal (existing, not computed)
    {
      // FIXME: there is a bug that causes a crash!
      const double epsilon = 1e-6;
      vtkIdType numPts = pts->GetNumberOfPoints();
      double pt[3];
      vtkIdType pid = 0; // default
      for (vtkIdType i = 0; i < numPts; i++)
      {
        pts->GetPoint(i, pt);
        if (fabs(pt[0] - cp[0]) < epsilon &&
            fabs(pt[1] - cp[1]) < epsilon &&
            fabs(pt[2] - cp[2]) < epsilon)
        {
          pid = i;
          break;
        }
      }
      vtkSmartPointer<vtkDoubleArray> pointNormals =
          vtkDoubleArray::SafeDownCast(InputPD->GetPointData()->GetNormals());
      pointNormals->GetTuple(pid, n);
    }
//    double c[3]; // compute center of polygon (triangle)
//    vtkIdType numPts = pts->GetNumberOfPoints();
//    c[0] = c[1] = c[2] = 0.0;
//    double p0[3];
//    for (int i = 0; i < numPts; i++)
//    {
//      pts->GetPoint(i, p0);
//      c[0] += p0[0];
//      c[1] += p0[1];
//      c[2] += p0[2];
//    }
//    c[0] *= static_cast<double>(numPts);
//    c[1] *= static_cast<double>(numPts);
//    c[2] *= static_cast<double>(numPts);
//    double pp[3]; // projected x[3] onto closest cell
//    vtkPlane::ProjectPoint(x, cp, n, pp);

    double v[3]; // compute vector from x[3] to projected point ...
    v[0] = cp[0] - x[0];
    v[1] = cp[1] - x[1];
    v[2] = cp[2] - x[2];
    double f = vtkMath::Dot(v, n); // ... and look whether we're in/outside
    if (f <= 0)
      return sqrt(vtkMath::Distance2BetweenPoints(x, cp));
    else
      return -sqrt(vtkMath::Distance2BetweenPoints(x, cp));
    return f;
  }
  else
  {
    return VTK_LARGE_FLOAT; // some value
  }
}

void vtkImplicitTriangulatedPolyData::EvaluateGradient(double x[3], double n[3])
{
  if (!InputPD || InputPD->GetNumberOfCells() == 0)
  {
    vtkErrorMacro(<< "Insufficient input poly data!");
    n[0] = VTK_LARGE_FLOAT; // some value
    n[1] = VTK_LARGE_FLOAT;
    n[2] = VTK_LARGE_FLOAT;
    return;
  }

  // find the closest point and closest cell to query-point x[3]:
  double cp[3];
  double sdist; // squared distance to the closest point
  vtkIdType cID; // the cell ID of the cell containing the closest point
  int sID; // some unused sub ID
  CellLocator->FindClosestPoint(x, cp, cID, sID, sdist);
  vtkCell *cell = InputPD->GetCell(cID);
  if (cell)
  {
    vtkPolygon::ComputeNormal(cell->GetPoints(), n); // simply return cell-normal
  }
  else
  {
    n[0] = VTK_LARGE_FLOAT; // some value
    n[1] = VTK_LARGE_FLOAT;
    n[2] = VTK_LARGE_FLOAT;
  }
}

void vtkImplicitTriangulatedPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImplicitFunction::PrintSelf(os, indent);
  
  // FIXME:
}
