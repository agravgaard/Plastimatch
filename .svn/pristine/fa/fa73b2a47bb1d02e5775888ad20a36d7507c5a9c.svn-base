//
#include "vtkPerspectiveOverlayProjectionInteractorStyle.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPlane.h>
#include <vtkMatrix3x3.h>
#include <vtkMath.h>
#include <vtkRenderWindow.h>

// default VTK config:
vtkCxxRevisionMacro(vtkPerspectiveOverlayProjectionInteractorStyle, "1.2")
vtkStandardNewMacro(vtkPerspectiveOverlayProjectionInteractorStyle)

vtkPerspectiveOverlayProjectionInteractorStyle::vtkPerspectiveOverlayProjectionInteractorStyle()
  : vtk2DSceneImageInteractorStyle()
{
  ReferenceImagePlane = vtkPlane::New();
  CORPlane = vtkPlane::New();
  ReferenceImageOrientation = vtkMatrix3x3::New();
  ImagingSourcePosition[0] = 0;
  ImagingSourcePosition[1] = 0;
  ImagingSourcePosition[2] = 0;
  CenterOfRotation3D[0] = 0;
  CenterOfRotation3D[1] = 0;
  CenterOfRotation3D[2] = 0;
  CenterOfRotation2D[0] = 0;
  CenterOfRotation2D[1] = 0;
  CenterOfRotation2D[2] = 0;
  InitialTransformationPosition[0] = 0;
  InitialTransformationPosition[1] = 0;
  RegionSensitiveTransformNature = false;
  TransformNatureRegion[0] = 0;
  TransformNatureRegion[1] = 0;
  TransformNatureRegion[2] = 0;
  TransformNatureRegion[3] = 0;
}

vtkPerspectiveOverlayProjectionInteractorStyle::~vtkPerspectiveOverlayProjectionInteractorStyle()
{
  ReferenceImagePlane->Delete();
  ReferenceImagePlane = NULL;
  CORPlane->Delete();
  CORPlane = NULL;
  ReferenceImageOrientation->Delete();
  ReferenceImageOrientation = NULL;
}

void vtkPerspectiveOverlayProjectionInteractorStyle::OnLeftButtonDown()
{
  if (this->PseudoAltKeyFlag) // panning implementation!
  {
    Superclass::OnLeftButtonDown();
    return;
  }

  if (RegionSensitiveTransformNature)
  {
    // left inside region: translation
    // left outside region: rotation
    // left+CTRL: rotation inside AND outside region
    // left+SHIFT: translation inside AND outside region
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    if ((x >= TransformNatureRegion[0] && x <= TransformNatureRegion[2] &&
        y >= TransformNatureRegion[1] && y <= TransformNatureRegion[3] &&
        !this->Interactor->GetControlKey()) ||
        (this->Interactor->GetShiftKey() && !this->Interactor->GetControlKey()))
      this->StartPerspectiveTranslation();
    else
      this->StartPerspectiveRotation();
  }
  else
  {
    // left: translation
    // left+CTRL: rotation
    if (this->Interactor->GetControlKey())
      this->StartPerspectiveRotation();
    else
      this->StartPerspectiveTranslation();
  }
}

void vtkPerspectiveOverlayProjectionInteractorStyle::OnLeftButtonUp()
{
  this->EndPan();
  this->EndPerspectiveRotation();
  this->EndPerspectiveTranslation();
}

void vtkPerspectiveOverlayProjectionInteractorStyle::StartPerspectiveTranslation()
{
  if (this->State != VTKIS_NONE)
    return;
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  InitialTransformationPosition[0] = x;
  InitialTransformationPosition[1] = y;
  this->StartState(VTKIS_PERSPECTIVE_TRANSLATION);
}

void vtkPerspectiveOverlayProjectionInteractorStyle::EndPerspectiveTranslation()
{
  if (this->State != VTKIS_PERSPECTIVE_TRANSLATION)
    return;
  this->StopState();
}

void vtkPerspectiveOverlayProjectionInteractorStyle::StartPerspectiveRotation()
{
  if (this->State != VTKIS_NONE)
    return;
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  InitialTransformationPosition[0] = x;
  InitialTransformationPosition[1] = y;
  this->StartState(VTKIS_PERSPECTIVE_ROTATION);
}

void vtkPerspectiveOverlayProjectionInteractorStyle::EndPerspectiveRotation()
{
  if (this->State != VTKIS_PERSPECTIVE_ROTATION)
    return;
  this->StopState();
}

void vtkPerspectiveOverlayProjectionInteractorStyle::DefineReferenceImageGeometry(
    double *origin, vtkMatrix3x3 *orientation, double *sourcePosition,
    double *centerOfRotation)
{
  if (!origin || !orientation || !sourcePosition || !centerOfRotation)
    return;
  ReferenceImageOrientation->DeepCopy(orientation);
  ReferenceImagePlane->SetOrigin(origin);
  double n[3];
  n[0] = orientation->GetElement(2, 0);
  n[1] = orientation->GetElement(2, 1);
  n[2] = orientation->GetElement(2, 2);
  ReferenceImagePlane->SetNormal(n);
  ImagingSourcePosition[0] = sourcePosition[0];
  ImagingSourcePosition[1] = sourcePosition[1];
  ImagingSourcePosition[2] = sourcePosition[2];
  CenterOfRotation3D[0] = centerOfRotation[0];
  CenterOfRotation3D[1] = centerOfRotation[1];
  CenterOfRotation3D[2] = centerOfRotation[2];
  CORPlane->SetOrigin(CenterOfRotation3D);
  // NOTE: the COR-plane is not parallel to the reference image, it is rather
  // perpendicular to the direction of projection!
  n[0] = ImagingSourcePosition[0] - CenterOfRotation3D[0];
  n[1] = ImagingSourcePosition[1] - CenterOfRotation3D[1];
  n[2] = ImagingSourcePosition[2] - CenterOfRotation3D[2];
  vtkMath::Normalize(n);
  CORPlane->SetNormal(n); // parallel to reference image plane!
  // extract position of projected center of rotation:
  double t = 0;
  ReferenceImagePlane->IntersectWithLine(ImagingSourcePosition,
      CenterOfRotation3D, t, CenterOfRotation2D);
}

double *vtkPerspectiveOverlayProjectionInteractorStyle::GetProjectedCenterOfRotationPosition()
{
  return CenterOfRotation2D;
}

void vtkPerspectiveOverlayProjectionInteractorStyle::ComputeCenterOfRotationInViewportCoordinates(
    double center[2])
{
  double v1[3];
  v1[0] = ReferenceImageOrientation->GetElement(0, 0);
  v1[1] = ReferenceImageOrientation->GetElement(0, 1);
  v1[2] = ReferenceImageOrientation->GetElement(0, 2);
  double v2[3];
  v2[0] = ReferenceImageOrientation->GetElement(1, 0);
  v2[1] = ReferenceImageOrientation->GetElement(1, 1);
  v2[2] = ReferenceImageOrientation->GetElement(1, 2);
  double cs[3];
  this->Magnifier->GetOutputSpacing(cs);
  // pixel position within image actor:
  double llc[3];
  this->Magnifier->GetResliceAxesOrigin(llc);
  double offset[3];
  this->Magnifier->GetOutputOrigin(offset);
  llc[0] += offset[0] * v1[0] + offset[1] * v2[0];
  llc[1] += offset[0] * v1[1] + offset[1] * v2[1];
  llc[2] += offset[0] * v1[2] + offset[1] * v2[2];
  center[0] = vtkMath::Dot(CenterOfRotation2D, v1) - vtkMath::Dot(llc, v1);
  center[1] = vtkMath::Dot(CenterOfRotation2D, v2) - vtkMath::Dot(llc, v2);
  center[0] /= cs[0];
  center[1] /= cs[1];

  // -> add actor offset within viewport:
  center[0] += this->ImageActor->GetPosition()[0];
  center[1] += this->ImageActor->GetPosition()[1];
}

void vtkPerspectiveOverlayProjectionInteractorStyle::OnMouseMove()
{
  switch (this->State)
  {
    case VTKIS_PERSPECTIVE_TRANSLATION:
    case VTKIS_PERSPECTIVE_ROTATION:
      break;
    default:
      this->vtk2DSceneImageInteractorStyle::OnMouseMove(); // forward
      return; // early exit
  }

  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  double informationTuple5[5];
  double informationTuple4[4];
  double dx, dy, t;
  double tCOR[3];
  double COR2D[3];
  double v1[3];
  v1[0] = ReferenceImageOrientation->GetElement(0, 0);
  v1[1] = ReferenceImageOrientation->GetElement(0, 1);
  v1[2] = ReferenceImageOrientation->GetElement(0, 2);
  double v2[3];
  v2[0] = ReferenceImageOrientation->GetElement(1, 0);
  v2[1] = ReferenceImageOrientation->GetElement(1, 1);
  v2[2] = ReferenceImageOrientation->GetElement(1, 2);
  double n[3];
  ReferenceImagePlane->GetNormal(n);
  double cs[2];
  this->GetCurrentPixelSpacing(cs);
  double corVP[2];
  double vv1[3];
  double vv2[3];
  switch (this->State)
  {
    case VTKIS_PERSPECTIVE_TRANSLATION:
      this->FindPokedRenderer(x, y);
      dx = x - InitialTransformationPosition[0]; // in pixels!
      dy = y - InitialTransformationPosition[1];
      informationTuple5[0] = dx; // in pixels (in-plane, viewport)
      informationTuple5[1] = dy;
      dx *= cs[0]; // -> convert distance into mm (w.r.t. current spacing)
      dy *= cs[1];
      COR2D[0] = CenterOfRotation2D[0] + v1[0] * dx + v2[0] * dy;
      COR2D[1] = CenterOfRotation2D[1] + v1[1] * dx + v2[1] * dy;
      COR2D[2] = CenterOfRotation2D[2] + v1[2] * dx + v2[2] * dy;
      CORPlane->IntersectWithLine(COR2D, ImagingSourcePosition, t, tCOR);
      // in mm (3D translation vector, WCS):
      informationTuple5[2] = tCOR[0] - CenterOfRotation3D[0];
      informationTuple5[3] = tCOR[1] - CenterOfRotation3D[1];
      informationTuple5[4] = tCOR[2] - CenterOfRotation3D[2];
      this->InvokeEvent(TranslationEvent, informationTuple5);
      break;
    case VTKIS_PERSPECTIVE_ROTATION:
      informationTuple4[0] = n[0]; // rotation axis = ref. img. plane normal!
      informationTuple4[1] = n[1];
      informationTuple4[2] = n[2];
      ComputeCenterOfRotationInViewportCoordinates(corVP);
      COR2D[0] = corVP[0]; // -> pixel position in viewport
      COR2D[1] = corVP[1];
      COR2D[2] = 0;
      vv1[0] = InitialTransformationPosition[0] - COR2D[0];
      vv1[1] = InitialTransformationPosition[1] - COR2D[1];
      vv1[2] = 0;
      vtkMath::Normalize(vv1);
      vv2[0] = x - COR2D[0];
      vv2[1] = y - COR2D[1];
      vv2[2] = 0;
      vtkMath::Normalize(vv2);
      informationTuple4[3] = vtkMath::DegreesFromRadians(
       atan2(vv2[1], vv2[0]) - atan2(vv1[1], vv1[0]));
      this->InvokeEvent(RotationEvent, informationTuple4);
      break;
  }
}

void vtkPerspectiveOverlayProjectionInteractorStyle::OnKeyPress()
{
  // Get the keypress
  vtkRenderWindowInteractor *rwi = this->Interactor;
  std::string key = rwi->GetKeySym();

  bool mod = false;

  if (ReferenceImageOrientation)
  {
    double v1[3];
    v1[0] = ReferenceImageOrientation->GetElement(0, 0);
    v1[1] = ReferenceImageOrientation->GetElement(0, 1);
    v1[2] = ReferenceImageOrientation->GetElement(0, 2);
    double v2[3];
    v2[0] = ReferenceImageOrientation->GetElement(1, 0);
    v2[1] = ReferenceImageOrientation->GetElement(1, 1);
    v2[2] = ReferenceImageOrientation->GetElement(1, 2);
    double n[3];
    n[0] = ReferenceImageOrientation->GetElement(2, 0);
    n[1] = ReferenceImageOrientation->GetElement(2, 1);
    n[2] = ReferenceImageOrientation->GetElement(2, 2);
    double f = 1.0;
    if (rwi->GetShiftKey()) // SHIFT=acceleration!
      f *= 5.;
    double informationTuple5[5];
    double informationTuple4[5];
    informationTuple5[0] = 0; // in-plane translation is INVALID on key-press!
    informationTuple5[1] = 0;
    informationTuple4[0] = n[0];
    informationTuple4[1] = n[1];
    informationTuple4[2] = n[2];
    if(key.compare("Up") == 0)
    {
      informationTuple5[2] = v2[0] * f;
      informationTuple5[3] = v2[1] * f;
      informationTuple5[4] = v2[2] * f;
      this->InvokeEvent(TranslationEvent, informationTuple5);
      mod = true;
    }
    else if(key.compare("Down") == 0)
    {
      informationTuple5[2] = v2[0] * -f;
      informationTuple5[3] = v2[1] * -f;
      informationTuple5[4] = v2[2] * -f;
      this->InvokeEvent(TranslationEvent, informationTuple5);
      mod = true;
    }
    else if(!rwi->GetControlKey() && key.compare("Left") == 0)
    {
      informationTuple5[2] = v1[0] * -f;
      informationTuple5[3] = v1[1] * -f;
      informationTuple5[4] = v1[2] * -f;
      this->InvokeEvent(TranslationEvent, informationTuple5);
      mod = true;
    }
    else if(!rwi->GetControlKey() && key.compare("Right") == 0)
    {
      informationTuple5[2] = v1[0] * f;
      informationTuple5[3] = v1[1] * f;
      informationTuple5[4] = v1[2] * f;
      this->InvokeEvent(TranslationEvent, informationTuple5);
      mod = true;
    }
    else if(rwi->GetControlKey() && key.compare("Left") == 0)
    {
      informationTuple4[3] = f;
      this->InvokeEvent(RotationEvent, informationTuple4);
      mod = true;
    }
    else if(rwi->GetControlKey() && key.compare("Right") == 0)
    {
      informationTuple4[3] = -f;
      this->InvokeEvent(RotationEvent, informationTuple4);
      mod = true;
    }
  }

  if (!mod) // forward event
    this->vtk2DSceneImageInteractorStyle::OnKeyPress();
}
