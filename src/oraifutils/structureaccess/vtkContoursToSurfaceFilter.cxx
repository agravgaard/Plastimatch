#include "vtkContoursToSurfaceFilter.h"

#include <vector>
#include <algorithm>

#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <vtkMath.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkVoxelContoursToSurfaceFilter.h>
#include <vtkDecimatePro.h>
#include <vtkQuadricDecimation.h>


namespace ora
{

/**
 * Helper class maintaining some temporary 2D slice/contours information.
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 */
class ContourInfo
{
public:
  /** Contour's location in complete cell list **/
  vtkIdType ContourLocation;
  /** Contour's cell size **/
  vtkIdType ContourSize;
  /** Contour's projected coordinate on the normal **/
  double CoordinateAlongNormal;

  /** Default constructor **/
  ContourInfo()
  {
    CoordinateAlongNormal = 0;
    ContourLocation = 0;
    ContourSize = 0;
  }

  /** Comparator for ascending sort order w.r.t. to projected coordinate. **/
  static bool CompareAscending(ContourInfo *p1, ContourInfo *p2)
  {
    return (p1->CoordinateAlongNormal < p2->CoordinateAlongNormal);
  }

};

}

vtkCxxRevisionMacro(vtkContoursToSurfaceFilter, "1.0")
;
vtkStandardNewMacro(vtkContoursToSurfaceFilter)
;

vtkContoursToSurfaceFilter::vtkContoursToSurfaceFilter()
{
  RefNormal = NULL;
  Spacing[0] = 0;
  Spacing[1] = 0;
  Spacing[2] = 0;
  AppliedSpacing[0] = 0;
  AppliedSpacing[1] = 0;
  AppliedSpacing[2] = 0;
  SetMeshDecimationModeToNone();
  MeshReductionRatio = 0.8;
}

vtkContoursToSurfaceFilter::~vtkContoursToSurfaceFilter()
{
  if (RefNormal)
    delete[] RefNormal;
  RefNormal = NULL;
}

void vtkContoursToSurfaceFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  if (RefNormal)
    os << indent << "RefNormal: " << RefNormal[0] << "," << RefNormal[1] << ","
        << RefNormal[2] << std::endl;
  else
    os << indent << "RefNormal: NULL" << std::endl;
  os << indent << "RefPoint: " << RefPoint[0] << "," << RefPoint[1] << ","
      << RefPoint[2] << std::endl;
  os << indent << "Spacing: " << Spacing[0] << "," << Spacing[1] << ","
      << Spacing[2] << std::endl;
  os << indent << "MinimumDeltaZ: " << MinimumDeltaZ << std::endl;
  os << indent << "AppliedSpacing: " << AppliedSpacing[0] << ","
      << AppliedSpacing[1] << "," << AppliedSpacing[2] << std::endl;
  os << indent << "MeshDecimationMode: " << GetMeshDecimationModeAsString()
      << std::endl;
  os << indent << "MeshReductionRatio: " << MeshReductionRatio << std::endl;
}

void vtkContoursToSurfaceFilter::SetMeshDecimationModeToNone()
{
  SetMeshDecimationMode(VTK_CTSF_NO_DECIMATION);
}

void vtkContoursToSurfaceFilter::SetMeshDecimationModeToPro()
{
  SetMeshDecimationMode(VTK_CTSF_PRO_DECIMATION);
}

void vtkContoursToSurfaceFilter::SetMeshDecimationModeToQuadric()
{
  SetMeshDecimationMode(VTK_CTSF_QUADRIC_DECIMATION);
}

const char *vtkContoursToSurfaceFilter::GetMeshDecimationModeAsString()
{
  if (GetMeshDecimationMode() == VTK_CTSF_NO_DECIMATION)
    return "None";
  else if (GetMeshDecimationMode() == VTK_CTSF_PRO_DECIMATION)
    return "Pro";
  else if (GetMeshDecimationMode() == VTK_CTSF_QUADRIC_DECIMATION)
    return "Quadric";
  else
    return "";
}

bool vtkContoursToSurfaceFilter::CheckParallelity(vtkPolyData *pd)
{
  // auto-detect the tilt (relative to x/y-plane) of the polygons' normals -
  // all contours are assumed being parallel:
  vtkSmartPointer<vtkPolyDataNormals> normalFilter = vtkSmartPointer<
      vtkPolyDataNormals>::New();
  normalFilter->SetInput(pd);
  normalFilter->SplittingOn();
  normalFilter->ConsistencyOn();
  normalFilter->FlipNormalsOff();
  normalFilter->NonManifoldTraversalOn();
  normalFilter->ComputePointNormalsOff();
  normalFilter->ComputeCellNormalsOn(); // !!!
  normalFilter->AutoOrientNormalsOff();
  normalFilter->SetFeatureAngle(90);
  normalFilter->Update();

  vtkPolyData* contours = normalFilter->GetOutput();
  vtkSmartPointer<vtkDataArray> normalData =
      contours->GetCellData()->GetNormals();
  if (!normalData || normalData->GetNumberOfComponents() != 3
      || normalData->GetNumberOfTuples() <= 0)
    return false;

  // parallelity check:
  bool contoursParallel = true;
  if (RefNormal)
    delete[] RefNormal;
  RefNormal = NULL;
  for (vtkIdType i = 0; i < normalData->GetNumberOfTuples(); i++)
  {
    double *n = normalData->GetTuple3(i);
    if (RefNormal)
    {
      vtkMath::Normalize(n);
      double a = vtkMath::Dot(n, RefNormal);
      // fabs(a) because in theory it's uninteresting whether
      // they're counterclockwisely ordered:
      if (fabs(fabs(a) - 1.0) > 1e-4)
      {
        contoursParallel = false;
        break;
      }
    }
    else
    {
      RefNormal = new double[3]; // store reference normal for comparison
      RefNormal[0] = n[0];
      RefNormal[1] = n[1];
      RefNormal[2] = n[2];
      vtkMath::Normalize(RefNormal);
    }
  }

  return contoursParallel;
}

bool vtkContoursToSurfaceFilter::CheckLevelsAndOrder(vtkPolyData *pd,
    bool &needsCorrection)
{
  needsCorrection = false;
  if (!pd)
    return false;

  // check whether we have contours on at least 2 levels, and whether the
  // contours are correctly ordered (from - to + along normal direction):
  bool atLeastTwoLevels = false;
  needsCorrection = false;
  vtkCellArray *polys = pd->GetPolys();
  polys->InitTraversal();
  vtkIdType npolypts;
  vtkIdType *polypts;
  double *refLevel = NULL;
  double lastLevel = 0.;
  while (polys->GetNextCell(npolypts, polypts))
  {
    if (npolypts > 0)
    {
      double *p = pd->GetPoints()->GetPoint(polypts[0]);
      double t = vtkMath::Dot(p, RefNormal); // project onto normal direction
      if (!atLeastTwoLevels && refLevel)
      {
        if (fabs(t - *refLevel) > 1e-4)
          atLeastTwoLevels = true;
      }
      else if (!refLevel)
      {
        RefPoint[0] = p[0]; // store some reference point of the contours
        RefPoint[1] = p[1];
        RefPoint[2] = p[2];
        refLevel = new double;
        *refLevel = t; // store the reference coordinate for comparison
        lastLevel = t; // -> no level change here!
      }
      if (fabs(t - lastLevel) > 1e-4) // level-change
      {
        if (t < lastLevel)
        {
          needsCorrection = true;
          break;
        }
      }
      lastLevel = t; // store
    }
  }

  delete refLevel;
  return atLeastTwoLevels;
}

bool vtkContoursToSurfaceFilter::CorrectSortOrder(vtkPolyData *uncorrected,
    vtkPolyData *corrected)
{
  if (!uncorrected || !corrected)
    return false;

  corrected->SetPoints(uncorrected->GetPoints());

  // extract contours info:
  std::vector<ora::ContourInfo *> ca;
  vtkPoints *origPoints = uncorrected->GetPoints();
  vtkCellArray *polys = uncorrected->GetPolys();
  polys->InitTraversal();
  vtkIdType coff = 0;
  vtkIdType npolypts = 0;
  vtkIdType *polypts;
  while (polys->GetNextCell(npolypts, polypts))
  {
    if (npolypts > 0)
    {
      ora::ContourInfo *ci = new ora::ContourInfo();
      ca.push_back(ci);
      // determine projection coordinate (project onto normal direction):
      double t = vtkMath::Dot(origPoints->GetPoint(polypts[0]), RefNormal);
      ci->CoordinateAlongNormal = t;
      // store info:
      ci->ContourSize = npolypts;
      ci->ContourLocation = coff;
      coff += (npolypts + 1); // + size itself
    }
  }
  // sort w.r.t. ascending order along their coordinate on normal:
  std::sort(ca.begin(), ca.end(), ora::ContourInfo::CompareAscending);
  // generate new polys cell array according to sort order:
  vtkSmartPointer<vtkCellArray> newPolys = vtkSmartPointer<vtkCellArray>::New();
  newPolys->Allocate(1, npolypts);
  for (std::size_t u = 0; u < ca.size(); u++)
  {
    polys->GetCell(ca[u]->ContourLocation, npolypts, polypts);
    newPolys->InsertNextCell(npolypts);
    for (vtkIdType k = 0; k < npolypts; k++)
      newPolys->InsertCellPoint(polypts[k]);
  }
  corrected->SetPolys(newPolys);
  // clean
  for (std::size_t u = 0; u < ca.size(); u++)
    delete ca[u];
  ca.clear();

  return true;
}

bool vtkContoursToSurfaceFilter::ApplyForwardTransform(
    vtkPolyData *uncorrected, vtkPolyData *corrected,
    vtkTransform *forwardTransform, double *&bounds, double *&center)
{
  if (!uncorrected || !corrected || !forwardTransform)
    return false;

  // compute axis/angle rotation representation between the common polygon
  // normal and the world coordinate system's z-axis:
  double zaxis[3];
  zaxis[0] = 0;
  zaxis[1] = 0;
  zaxis[2] = 1;
  double zdn = vtkMath::Dot(zaxis, RefNormal);
  vtkSmartPointer<vtkTransformPolyDataFilter> correctionTransformFilter =
      vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  forwardTransform->Identity();
  forwardTransform->PostMultiply();
  if (fabs(fabs(zdn) - 1.0) > 1e-4) // there is indeed a discrepancy!
  {
    // -> compute rotation axis and angle around this axis:
    forwardTransform->Translate(RefPoint[0], RefPoint[1], RefPoint[2]);
    double angle = acos(zdn);
    double axis[3];
    vtkMath::Cross(zaxis, RefNormal, axis);
    vtkMath::Normalize(axis);
    // -> convert to rotation matrix:
    vtkSmartPointer<vtkMatrix4x4> rm = vtkSmartPointer<vtkMatrix4x4>::New();
    rm->Identity();
    double ca = cos(angle);
    double sa = sin(angle);
    double x = axis[0];
    double y = axis[1];
    double z = axis[2];

    rm->SetElement(0., 0., 1. + (1. - ca) * (x * x - 1));
    rm->SetElement(0., 1., -z * sa + (1. - ca) * x * y);
    rm->SetElement(0., 2., y * sa + (1. - ca) * x * z);
    rm->SetElement(1., 0., z * sa + (1. - ca) * x * y);
    rm->SetElement(1., 1., 1. + (1. - ca) * (y * y - 1.));
    rm->SetElement(1., 2., -x * sa + (1. - ca) * y * z);
    rm->SetElement(2., 0., -y * sa + (1. - ca) * x * z);
    rm->SetElement(2., 1., x * sa + (1. - ca) * y * z);
    rm->SetElement(2., 2., 1. + (1. - ca) * (z * z - 1.));
    forwardTransform->Concatenate(rm);
  }
  correctionTransformFilter->SetTransform(forwardTransform->GetInverse());
  correctionTransformFilter->SetInput(uncorrected);
  vtkPolyData *contours = correctionTransformFilter->GetOutput();
  correctionTransformFilter->Update();

  // convert to ijk-coordinates for the contour to surface filter:
  // (see example on VTK wiki)
  contours->GetBounds(bounds);
  contours->GetCenter(center);
  double origin[3] = { bounds[0], bounds[2], bounds[4] };
  if (Spacing[0] > 0.)
    AppliedSpacing[0] = Spacing[0];
  else
    AppliedSpacing[0] = 0.5;
  if (Spacing[1] > 0.)
    AppliedSpacing[1] = Spacing[1];
  else
    AppliedSpacing[1] = 0.5;
  if (Spacing[2] > 0.)
    AppliedSpacing[2] = Spacing[2];
  else
    AppliedSpacing[2] = MinimumDeltaZ - 0.01;

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkPoints* contourPoints = contours->GetPoints();
  int numPoints = contourPoints->GetNumberOfPoints();
  points->SetNumberOfPoints(numPoints);
  for (int i = 0; i < numPoints; ++i)
  {
    double pt[3];
    contourPoints->GetPoint(i, pt);
    pt[0] = static_cast<int> ((pt[0] - origin[0]) / AppliedSpacing[0] + 0.5);
    pt[1] = static_cast<int> ((pt[1] - origin[1]) / AppliedSpacing[1] + 0.5);
    pt[2] = static_cast<int> ((pt[2] - origin[2]) / AppliedSpacing[2] + 0.5);
    points->SetPoint(i, pt);
  }
  corrected->SetPolys(contours->GetPolys());
  corrected->SetPoints(points);

  return true;
}

bool vtkContoursToSurfaceFilter::ApplyBackwardTransform(
    vtkPolyData *uncorrected, vtkPolyData *corrected,
    vtkTransform *forwardTransform, double *center, double *bounds)
{
  if (!uncorrected || !corrected || !forwardTransform || !center || !bounds)
    return false;

  double scaleCenter[3];
  uncorrected->GetCenter(scaleCenter);
  double scaleBounds[6];
  uncorrected->GetBounds(scaleBounds);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
      vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInput(uncorrected);
  vtkSmartPointer<vtkTransform> transform =
      vtkSmartPointer<vtkTransform>::New();
  transformFilter->SetTransform(transform);
  transform->Concatenate(forwardTransform); // account for the z-aligment-correction
  transform->Translate(-scaleCenter[0], -scaleCenter[1], -scaleCenter[2]);
  transform->Scale((bounds[1] - bounds[0]) / (scaleBounds[1] - scaleBounds[0]),
      (bounds[3] - bounds[2]) / (scaleBounds[3] - scaleBounds[2]), (bounds[5]
          - bounds[4]) / (scaleBounds[5] - scaleBounds[4]));
  transform->Translate(center[0], center[1], center[2]);
  transformFilter->Update();

  corrected->SetPoints(transformFilter->GetOutput()->GetPoints());
  corrected->SetPolys(transformFilter->GetOutput()->GetPolys());

  return true;
}

int vtkContoursToSurfaceFilter::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  if (!inInfo)
  {
    vtkErrorMacro(<< "There is no input information.")
    return 0;
  }
  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(inInfo->Get(
      vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(
      vtkDataObject::DATA_OBJECT()));
  if (!input)
  {
    vtkErrorMacro(<< "There is no input object.")
    return 0;
  }

  // we really need the contours being parallel:
  if (!CheckParallelity(input))
  {
    vtkErrorMacro(<< "The input polygons are not parallel!")
    return 0;
  }

  // we need at least 2 different levels, and we must eventually correct the
  // sort order:
  bool needCorrection = false;
  if (!CheckLevelsAndOrder(input, needCorrection))
  {
    vtkErrorMacro(<< "The input polygons do not even comprise 2 different levels!")
    return 0;
  }

  // do a sort order correction if necessary; NOTE: this step really modifies
  // the input poly data, therefore, let's create new poly data if we correct:
  vtkSmartPointer<vtkPolyData> contours = input;
  if (needCorrection)
  {
    contours = vtkSmartPointer<vtkPolyData>::New();
    if (!CorrectSortOrder(input, contours))
    {
      vtkErrorMacro(<< "Could not correct the sort order of the input polygons!")
      return 0;
    }
  }
  // detect minimum delta Z:
  vtkCellArray *polys = contours->GetPolys();
  vtkPoints *points = contours->GetPoints();
  polys->InitTraversal();
  vtkIdType npolypts = 0;
  vtkIdType *polypts;
  double lastT = 0;
  int i = 0;
  MinimumDeltaZ = 1e10;
  while (polys->GetNextCell(npolypts, polypts))
  {
    if (npolypts > 0)
    {
      // determine projection coordinate (project onto normal direction):
      double t = vtkMath::Dot(points->GetPoint(polypts[0]), RefNormal);
      if (i > 0)
      {
        double diff = fabs(t - lastT);
        if (diff > 1e-4) // some diff (not on same plane)
        {
          if (diff < MinimumDeltaZ)
            MinimumDeltaZ = diff;
        }
        lastT = t;
      }
      else
      {
        lastT = t;
      }
      i++;
    }
  }

  // apply forward transform accounting for resolution and general tilt:
  vtkSmartPointer<vtkPolyData> transformedContours = vtkSmartPointer<
      vtkPolyData>::New();
  vtkSmartPointer<vtkTransform> forwardTransform =
      vtkSmartPointer<vtkTransform>::New();
  double *bounds = new double[6];
  double *center = new double[3];
  if (!ApplyForwardTransform(contours, transformedContours, forwardTransform,
      bounds, center))
  {
    vtkErrorMacro(<< "Could not apply forward transform correction!")
    return 0;
  }

  // generate surface from contours by implicit voxelation:
  vtkSmartPointer<vtkVoxelContoursToSurfaceFilter> contoursToSurface =
      vtkSmartPointer<vtkVoxelContoursToSurfaceFilter>::New();
  contoursToSurface->SetInput(transformedContours);
  contoursToSurface->SetSpacing(AppliedSpacing[0], AppliedSpacing[1],
      AppliedSpacing[2]);
  contoursToSurface->Update();

  // apply backward transform that maps back the triangulated surface:
  if (MeshDecimationMode == VTK_CTSF_NO_DECIMATION) // no mesh decimation
  {
    if (!ApplyBackwardTransform(contoursToSurface->GetOutput(), output,
        forwardTransform, center, bounds))
    {
      vtkErrorMacro(<< "Could not apply backward transform correction!")
      return 0;
    }
  }
  else // additional mesh decimation
  {
    vtkSmartPointer<vtkPolyData> undecimated =
        vtkSmartPointer<vtkPolyData>::New();
    if (!ApplyBackwardTransform(contoursToSurface->GetOutput(), undecimated,
        forwardTransform, center, bounds))
    {
      vtkErrorMacro(<< "Could not apply backward transform correction!")
      return 0;
    }

    vtkSmartPointer<vtkPolyDataAlgorithm> filter = NULL;
    if (MeshDecimationMode == VTK_CTSF_PRO_DECIMATION)
    {
      vtkSmartPointer<vtkDecimatePro> f =
          vtkSmartPointer<vtkDecimatePro>::New();
      f->SetTargetReduction(MeshReductionRatio);
      f->SetFeatureAngle(90.);
      filter = f;
    }
    else // if (MeshDecimationMode == VTK_CTSF_QUADRIC_DECIMATION)
    {
      vtkSmartPointer<vtkQuadricDecimation> f = vtkSmartPointer<
          vtkQuadricDecimation>::New();
      f->SetTargetReduction(MeshReductionRatio);
      f->AttributeErrorMetricOff();
      filter = f;
    }
    // apply decimation
    filter->SetInput(undecimated);
    filter->Update();
    // take over
    output->SetPoints(filter->GetOutput()->GetPoints());
    output->SetPolys(filter->GetOutput()->GetPolys());
  }
  delete[] bounds;
  delete[] center;

  return 1;
}
