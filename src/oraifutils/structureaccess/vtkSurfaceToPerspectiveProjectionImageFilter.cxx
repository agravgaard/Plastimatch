//
#include "vtkSurfaceToPerspectiveProjectionImageFilter.h"

#include <vtkObjectFactory.h>

#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkInformation.h>
#include <vtkMatrix3x3.h>
#include <vtkPlane.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkMath.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataCollection.h>
#include <vtkActorCollection.h>
#include <vtkProperty.h>
#include <vtkWindowToImageFilter.h>
#include <vtkMatrix4x4.h>
#include <vtkgl.h>

#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif


vtkCxxRevisionMacro(vtkSurfaceToPerspectiveProjectionImageFilter, "1.1")
;
vtkStandardNewMacro(vtkSurfaceToPerspectiveProjectionImageFilter)
;

vtkSetObjectImplementationMacro(vtkSurfaceToPerspectiveProjectionImageFilter, PlaneOrientation, vtkMatrix3x3)

void vtkSurfaceToPerspectiveProjectionImageFilter::PrintSelf(ostream& os,
    vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "#Inputs: " << Inputs->GetNumberOfItems() << std::endl;
  os << indent << "#InputActorMap: " << InputActorMap.size() << std::endl;
  os << indent << "#Actors: " << Actors->GetNumberOfItems() << std::endl;
  os << indent << "RenWin: " << RenWin << std::endl;
  os << indent << "Cam: " << Cam << std::endl;
  os << indent << "Ren: " << Ren << std::endl;
  os << indent << "ConnectedToRenWin: " << ConnectedToRenWin << std::endl;
  os << indent << "SourcePosition: " << SourcePosition[0] << ","
      << SourcePosition[1] << "," << SourcePosition[2] << std::endl;
  os << indent << "PlaneSizePixels: " << PlaneSizePixels[0] << ","
      << PlaneSizePixels[1] << std::endl;
  os << indent << "PlaneSpacing: " << PlaneSpacing[0] << "," << PlaneSpacing[1]
      << std::endl;
  os << indent << "PlaneOrigin: " << PlaneOrigin[0] << "," << PlaneOrigin[1]
      << "," << PlaneOrigin[2] << std::endl;
  os << indent << "PlaneOrientation: " << PlaneOrientation << std::endl;
  os << indent << "WindowImager: " << WindowImager << std::endl;
  os << indent << "UseThreadSafeRendering: " << UseThreadSafeRendering << std::endl;
}

vtkSurfaceToPerspectiveProjectionImageFilter::vtkSurfaceToPerspectiveProjectionImageFilter()
{
  Inputs = vtkSmartPointer<vtkPolyDataCollection>::New();
  RenWin = NULL;
  Cam = vtkSmartPointer<vtkCamera>::New();
  Ren = vtkSmartPointer<vtkRenderer>::New();
  Ren->SetActiveCamera(Cam);
  Ren->RemoveAllLights();
  Ren->SetBackground(0, 0, 0); // black
  this->SetNumberOfOutputPorts(1);
  this->SetNumberOfInputPorts(0);
  SourcePosition[0] = SourcePosition[1] = SourcePosition[2] = 0;
  PlaneSizePixels[0] = PlaneSizePixels[1] = 0;
  PlaneSpacing[0] = PlaneSpacing[1] = 0;
  PlaneOrigin[0] = PlaneOrigin[1] = PlaneOrigin[2] = 0;
  PlaneOrientation = NULL;
  InputActorMap.clear();
  Actors = vtkSmartPointer<vtkActorCollection>::New();
  WindowImager = vtkSmartPointer<vtkWindowToImageFilter>::New();
  UseThreadSafeRendering = false;
}

vtkSurfaceToPerspectiveProjectionImageFilter::~vtkSurfaceToPerspectiveProjectionImageFilter()
{
  DisconnectFromRenderWindow();
  RemoveAllInputs();
  Inputs = NULL;
  RenWin = NULL;
  Ren->SetActiveCamera(NULL);
  Cam = NULL;
  Ren = NULL;
  Actors = NULL;
  WindowImager = NULL;
}

void vtkSurfaceToPerspectiveProjectionImageFilter::AddInput(vtkPolyData *input)
{
  if (input && !InputActorMap[input])
  {
    Inputs->AddItem(input);
    vtkSmartPointer<vtkPolyDataMapper> m =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    m->SetInput(input);
    m->SetScalarVisibility(false);
    vtkSmartPointer<vtkActor> a = vtkSmartPointer<vtkActor>::New();
    a->SetMapper(m);
    a->GetProperty()->SetDiffuse(0); // -> white objects
    a->GetProperty()->SetDiffuseColor(1, 1, 1);
    a->GetProperty()->SetAmbient(1);
    a->GetProperty()->SetAmbientColor(1, 1, 1);
    a->GetProperty()->SetSpecular(0);
    a->GetProperty()->SetSpecularColor(1, 1, 1);
    a->GetProperty()->SetShading(0);
    a->GetProperty()->SetOpacity(1.0);
    Actors->AddItem(a);
    InputActorMap[input] = a;
    Ren->AddActor(a);
    this->Modified();
  }
}

void vtkSurfaceToPerspectiveProjectionImageFilter::RemoveInput(
    vtkPolyData *input)
{
  if (input)
  {
    Inputs->RemoveItem(input);
    if (InputActorMap[input])
    {
      Ren->RemoveActor(InputActorMap[input]);
      Actors->RemoveItem(InputActorMap[input]);
      InputActorMap.erase(input);
      this->Modified();
    }
  }
}

void vtkSurfaceToPerspectiveProjectionImageFilter::RemoveAllInputs()
{
  Ren->RemoveAllViewProps();
  Inputs->RemoveAllItems();
  Actors->RemoveAllItems();
  InputActorMap.clear();
  this->Modified();
}

int vtkSurfaceToPerspectiveProjectionImageFilter::GetNumberOfInputs()
{
  return Inputs->GetNumberOfItems();
}

bool vtkSurfaceToPerspectiveProjectionImageFilter::ConnectToRenderWindow(
    vtkRenderWindow *renWin)
{
  if (renWin != RenWin)
  {
    if (RenWin)
      RenWin->UnRegister(this);
    RenWin = renWin;
    if (RenWin)
      RenWin->Register(this);
    this->Modified();
  }

  if (RenWin)
  {
    RenWin->GetRenderers()->RemoveAllItems();
    RenWin->SetOffScreenRendering(true);
    RenWin->AddRenderer(Ren);
    WindowImager->SetInput(RenWin);
    WindowImager->ShouldRerenderOff();
    ConnectedToRenWin = true;
  }

  return ConnectedToRenWin;
}

void vtkSurfaceToPerspectiveProjectionImageFilter::DisconnectFromRenderWindow()
{
  if (RenWin)
  {
    RenWin->RemoveRenderer(Ren);
    RenWin->UnRegister(this);
    RenWin = NULL;
    this->Modified();
    ConnectedToRenWin = false;
  }
}

vtkImageData* vtkSurfaceToPerspectiveProjectionImageFilter::GetOutput()
{
  return vtkImageData::SafeDownCast(this->GetOutputDataObject(0));
}

int vtkSurfaceToPerspectiveProjectionImageFilter::FillOutputPortInformation(
    int port, vtkInformation* info)
{
  // now add our info
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
  return 1;
}

bool vtkSurfaceToPerspectiveProjectionImageFilter::IsProjectionGeometryValid()
{
  if (!PlaneOrientation)
    return false;

  // M * M == I required
  vtkSmartPointer<vtkMatrix3x3> transpose =
      vtkSmartPointer<vtkMatrix3x3>::New();
  vtkMatrix3x3::Transpose(PlaneOrientation, transpose);
  vtkSmartPointer<vtkMatrix3x3> test = vtkSmartPointer<vtkMatrix3x3>::New();
  vtkMatrix3x3::Multiply3x3(PlaneOrientation, transpose, test);
  vtkSmartPointer<vtkMatrix3x3> identity = vtkSmartPointer<vtkMatrix3x3>::New();
  identity->Identity();
  for (int d1 = 0; d1 < 3; d1++)
  {
    for (int d2 = 0; d2 < 3; d2++)
    {
      if (fabs(test->GetElement(d1, d2) - identity->GetElement(d1, d2)) > 1e-3)
        return false;
    }
  }

  for (int d = 0; d < 2; d++)
  {
    if (PlaneSizePixels[d] <= 0)
      return false;
    if (PlaneSpacing[d] <= 0.0)
      return false;
  }

  double fs[3];
  double n[3];
  double p0[3];
  for (int d = 0; d < 3; d++)
  {
    fs[d] = SourcePosition[d];
    n[d] = PlaneOrientation->GetElement(2, d);
    p0[d] = PlaneOrigin[d];
  }
  if (vtkPlane::DistanceToPlane(fs, n, p0) < 1e-3)
    return false;

  return true;
}

void vtkSurfaceToPerspectiveProjectionImageFilter::RequestInformation(
    vtkInformation * vtkNotUsed(request), vtkInformationVector** vtkNotUsed( inputVector ),
    vtkInformationVector *outputVector)
{
  if (Inputs->GetNumberOfItems() <= 0)
  {
    vtkErrorMacro(<< "At least one input poly data surface must be specified!");
    return;
  }
  if (!ConnectedToRenWin)
  {
    vtkErrorMacro(<< "A render window must be connected!");
    return;
  }
  if (!IsProjectionGeometryValid())
  {
    vtkErrorMacro(<< "The projection geometry is invalid / not fully defined!");
    return;
  }

  // whole extent:
  int wExtent[6];
  wExtent[0] = 0;
  wExtent[1] = PlaneSizePixels[0] - 1;
  wExtent[2] = 0;
  wExtent[3] = PlaneSizePixels[1] - 1;
  wExtent[4] = 0;
  wExtent[5] = 0;
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wExtent, 6);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), wExtent, 6);
  // pixel type:
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_CHAR, 1);
}

int vtkSurfaceToPerspectiveProjectionImageFilter::ProcessRequest(
    vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  // generate the data
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
  {
    this->RequestData(request, inputVector, outputVector);
    return 1;
  }

  // execute information
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
  {
    this->RequestInformation(request, inputVector, outputVector);
    return 1;
  }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

bool vtkSurfaceToPerspectiveProjectionImageFilter::UpdateCameraFromProjectionGeometry()
{
  if (!IsProjectionGeometryValid())
    return false;

  if (!Cam)
    return false;

  if (PlaneSpacing[0] != PlaneSpacing[1])
    return false;

  int i;
  // first calculate the positions of the plane's corner points:
  double v1[3]; // plane x-direction (orientation is normalized)
  v1[0] = PlaneOrientation->GetElement(0, 0);
  v1[1] = PlaneOrientation->GetElement(0, 1);
  v1[2] = PlaneOrientation->GetElement(0, 2);
  double v2[3]; // plane y-direction
  v2[0] = PlaneOrientation->GetElement(1, 0);
  v2[1] = PlaneOrientation->GetElement(1, 1);
  v2[2] = PlaneOrientation->GetElement(1, 2);
  double w = (double) PlaneSizePixels[0] * PlaneSpacing[0]; // plane width
  double h = (double) PlaneSizePixels[1] * PlaneSpacing[1]; // plane height
  double corners[4][3];
  i = 0;
  while (i < 3)
  {
    corners[0][i] = PlaneOrigin[i];
    corners[1][i] = PlaneOrigin[i] + v1[i] * w;
    corners[2][i] = corners[1][i] + v2[i] * h;
    corners[3][i] = PlaneOrigin[i] + v2[i] * h;
    i++;
  }
  double center[3];
  // center in image plane:
  for (i = 0; i < 3; i++)
    center[i] = PlaneOrigin[i] + v1[i] * w / 2. + v2[i] * h / 2.;
  // -> compute viewing angle (vertical or horizontal):
  double qw = vtkMath::Dot(corners[0], v1) - vtkMath::Dot(center, v1);
  qw = fabs(qw);
  double qh = vtkMath::Dot(corners[0], v2) - vtkMath::Dot(center, v2);
  qh = fabs(qh);
  bool hordir;
  double va;
  double n[3]; // plane normal
  n[0] = PlaneOrientation->GetElement(2, 0);
  n[1] = PlaneOrientation->GetElement(2, 1);
  n[2] = PlaneOrientation->GetElement(2, 2);
  double pd = vtkPlane::DistanceToPlane(SourcePosition, n, PlaneOrigin);
  if (qh >= qw) // vertical
  {
    hordir = false;
    va = 2 * atan2(qh, pd) / vtkMath::Pi() * 180.;
  }
  else // horizontal
  {
    hordir = true;
    va = 2 * atan2(qw, pd) / vtkMath::Pi() * 180.;
  }
  // -> clipping range:
  double clipr[2];
  clipr[0] = 1.0; // static near plane, far plane behind image plane
  clipr[1] = vtkPlane::DistanceToPlane(SourcePosition, n, PlaneOrigin) + 1.0;
  // -> virtual source position (perpendicular to image plane):
  double virtualSourcePosition[3];
  vtkPlane::ProjectPoint(center, SourcePosition, n, virtualSourcePosition);

  // -> adapt render window size:
  RenWin->SetSize(PlaneSizePixels[0], PlaneSizePixels[1]);

  // -> apply the settings to camera:
  // compute view shear that explains non-centered source positions and/or
  // tilted image planes:
  // NOTE: it is important that dx and dy are computed w.r.t. the image plane
  // main directions, not relative to the WCS!!!
  double t1 = vtkMath::Dot(SourcePosition, v1);
  double t2 = vtkMath::Dot(virtualSourcePosition, v1);
  double dxdz = (t1 - t2) / pd;
  t1 = vtkMath::Dot(SourcePosition, v2);
  t2 = vtkMath::Dot(virtualSourcePosition, v2);
  double dydz = (t1 - t2) / pd;
  Cam->SetViewShear(dxdz, dydz, 1.0);
  Cam->SetPosition(virtualSourcePosition);
  Cam->SetFocalPoint(center);
  Cam->SetViewUp(v2);
  Cam->SetUseHorizontalViewAngle(hordir);
  Cam->SetViewAngle(va);
  Cam->SetClippingRange(clipr[0], clipr[1]);

  return true;
}

void vtkSurfaceToPerspectiveProjectionImageFilter::RequestData(vtkInformation* vtkNotUsed(request),
    vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *out = vtkImageData::SafeDownCast(outInfo->Get(
      vtkDataObject::DATA_OBJECT()));
  out->SetExtent(
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT()));
  out->AllocateScalars();

  if (Inputs->GetNumberOfItems() <= 0 || !ConnectedToRenWin)
    return;

  if (!UpdateCameraFromProjectionGeometry())
    return;

  // NOTE: all inputs should be added to renderer here.

  if (!UseThreadSafeRendering)
  {
    RenWin->Render();
    WindowImager->Modified();
    WindowImager->Update();
  }
  else
  {
#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
    XLockDisplay((Display*)this->RenWin->GetGenericDisplayId());
#endif
    RenWin->MakeCurrent();
    RenWin->Render();
    WindowImager->Modified();
    WindowImager->Update();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
    wglMakeCurrent((HDC__*) RenWin->GetGenericDisplayId(), NULL);
#else
    glXMakeCurrent((Display*) RenWin->GetGenericDisplayId(), None, NULL);
    XUnlockDisplay((Display*)this->RenWin->GetGenericDisplayId());
#endif
  }

  if (!WindowImager->GetOutput())
    return;
  unsigned char *wip =
      (unsigned char *) WindowImager->GetOutput()->GetScalarPointer();
  unsigned char *wp = (unsigned char *) out->GetScalarPointer();
  int np = PlaneSizePixels[0] * PlaneSizePixels[1];
  int *rrws = RenWin->GetSize();
  if (rrws[0] * rrws[1] < np) // in situations where the renwin is not big enough
    np = rrws[0] * rrws[1];
  for (int i = 0; i < np; i++)
  {
    wp[i] = wip[i * 3];
  }
  out->SetSpacing(PlaneSpacing[0], PlaneSpacing[1], 1.0);
  out->SetOrigin(PlaneOrigin);
}

void vtkSurfaceToPerspectiveProjectionImageFilter::SetPlaneSpacing(double sx,
    double sy)
{
  if (sx < sy)
    PlaneSpacing[0] = PlaneSpacing[1] = sx;
  else
    PlaneSpacing[0] = PlaneSpacing[1] = sy;
  this->Modified();
}

void vtkSurfaceToPerspectiveProjectionImageFilter::SetPlaneSpacing(double *s)
{
  if (s[0] < s[1])
    PlaneSpacing[0] = PlaneSpacing[1] = s[0];
  else
    PlaneSpacing[0] = PlaneSpacing[1] = s[1];
  this->Modified();
}

