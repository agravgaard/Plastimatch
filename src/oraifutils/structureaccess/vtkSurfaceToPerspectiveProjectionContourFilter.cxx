//
#include "vtkSurfaceToPerspectiveProjectionContourFilter.h"

#include <vtkPolyData.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkContourFilter.h>
#include <vtkImageData.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPolyDataCollection.h>
#include <vtkWindowToImageFilter.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>


vtkCxxRevisionMacro(vtkSurfaceToPerspectiveProjectionContourFilter, "1.0")
;
vtkStandardNewMacro(vtkSurfaceToPerspectiveProjectionContourFilter)
;

void vtkSurfaceToPerspectiveProjectionContourFilter::PrintSelf(ostream& os,
    vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Updating: " << Updating << std::endl;
  os << indent << "OutputPolyData: " << OutputPolyData << std::endl;
  os << indent << "ContourFilter: " << ContourFilter << std::endl;
  os << indent << "TransformFilter: " << TransformFilter << std::endl;
  os << indent << "Transform: " << Transform << std::endl;
}

vtkPolyData *vtkSurfaceToPerspectiveProjectionContourFilter::GetOutputPolyData()
{
  return OutputPolyData;
}

vtkSurfaceToPerspectiveProjectionContourFilter::vtkSurfaceToPerspectiveProjectionContourFilter() :
  vtkSurfaceToPerspectiveProjectionImageFilter()
{
  OutputPolyData = NULL;
  ContourFilter = vtkSmartPointer<vtkContourFilter>::New();
  ContourFilter->SetComputeGradients(false);
  ContourFilter->SetComputeNormals(false);
  ContourFilter->SetComputeScalars(false);
  ContourFilter->SetValue(0, 255);
  ContourFilter->SetArrayComponent(0);
  TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  Transform = vtkSmartPointer<vtkTransform>::New();
  Transform->PostMultiply();
  TransformFilter->SetTransform(Transform);
  TransformFilter->SetInput(ContourFilter->GetOutput());
  Updating = false;
}

vtkSurfaceToPerspectiveProjectionContourFilter::~vtkSurfaceToPerspectiveProjectionContourFilter()
{
  OutputPolyData = NULL;
  ContourFilter = NULL;
  TransformFilter = NULL;
  Transform = NULL;
}

int vtkSurfaceToPerspectiveProjectionContourFilter::FillOutputPortInformation(
    int port, vtkInformation* info)
{
  int ret = Superclass::FillOutputPortInformation(port, info);
  OutputPolyData = vtkSmartPointer<vtkPolyData>::New();
  return ret;
}

void vtkSurfaceToPerspectiveProjectionContourFilter::RequestData(
    vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  if (Updating) // prevent reentrancy
    return;

  Updating = true;

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *out = vtkImageData::SafeDownCast(outInfo->Get(
      vtkDataObject::DATA_OBJECT()));
  out->SetExtent(
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT()));
  out->AllocateScalars();

  if (this->Inputs->GetNumberOfItems() <= 0 || !this->ConnectedToRenWin)
    return;

  if (!this->UpdateCameraFromProjectionGeometry())
    return;

  // NOTE: all inputs should be added to renderer here.

  this->RenWin->Render();
  this->WindowImager->Modified();
  this->WindowImager->Update();

  if (!this->WindowImager->GetOutput())
    return;

  unsigned char *wip =
      (unsigned char *) this->WindowImager->GetOutput()->GetScalarPointer();
  unsigned char *wp = (unsigned char *) out->GetScalarPointer();
  int np = this->PlaneSizePixels[0] * this->PlaneSizePixels[1];
  int *rrws = RenWin->GetSize();
  if (rrws[0] * rrws[1] < np) // in situations where the renwin is not big enough
    np = rrws[0] * rrws[1];
  for (int i = 0; i < np; i++)
    wp[i] = wip[i * 3];

  // set image geometry:
  out->SetSpacing(this->PlaneSpacing[0], this->PlaneSpacing[1], 1.0);
  out->SetOrigin(this->PlaneOrigin);

  // contour extraction and adaption (from image without geometry!):
  ContourFilter->SetInput(this->WindowImager->GetOutput());
  ContourFilter->Update();
  Transform->Identity();
  Transform->Scale(this->PlaneSpacing[0], this->PlaneSpacing[1], 1);
  vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
  for (int d1 = 0; d1 < 3; d1++)
    for (int d2 = 0; d2 < 3; d2++)
      mat->SetElement(d1, d2, this->PlaneOrientation->GetElement(d1, d2));
  mat->Invert();
  Transform->Concatenate(mat);
  Transform->Translate(this->PlaneOrigin);
  TransformFilter->Update();
  OutputPolyData->ShallowCopy(TransformFilter->GetOutput());

  Updating = false;
}
