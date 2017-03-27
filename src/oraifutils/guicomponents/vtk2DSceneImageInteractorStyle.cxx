//
#include "vtk2DSceneImageInteractorStyle.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkCallbackCommand.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkMetaImageWriter.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkBox.h>
#include <vtkPlane.h>

#include <algorithm>
#include <math.h>

// default VTK config:
vtkCxxRevisionMacro(vtk2DSceneImageInteractorStyle, "2.3")
vtkStandardNewMacro(vtk2DSceneImageInteractorStyle)

const double vtk2DSceneImageInteractorStyle::F_EPSILON = 1e-6;
const double vtk2DSceneImageInteractorStyle::F_WEAK_EPSILON = 1e-4;

vtk2DSceneImageInteractorStyle::vtk2DSceneImageInteractorStyle()
  : vtkInteractorStyleImage()
{
  PseudoAltKeyFlag = false;
  DoNotInvokeZoomingEvent = false;
  ExternalPseudoAltKeyFlag = false;
  Renderer = NULL;
  RenderWindow = NULL;
  ReferenceImage = NULL;
  Magnifier = NULL;
  // default: in-plane reslicing
  ResliceOrientation = vtkSmartPointer<vtkMatrix4x4>::New();
  ResliceOrientation->Identity();
  ImageActor = NULL;
  ImageMapper = NULL;
  CurrentMagnification = -10000;
  CurrentResliceSpacing[0] = 0;
  CurrentResliceSpacing[1] = 0;
  CurrentResliceSpacing[2] = 0;
  CurrentResliceRUC[0] = 0;
  CurrentResliceRUC[1] = 0;
  CurrentResliceRUC[2] = 0;
  CurrentResliceLLC[0] = 0;
	CurrentResliceLLC[1] = 0;
	CurrentResliceLLC[2] = 0;
	CurrentResliceExtent[0] = 0;
	CurrentResliceExtent[1] = 0;
	CurrentResliceExtent[2] = 0;
	CurrentResliceExtent[3] = 0;
	CurrentResliceExtent[4] = 0;
	CurrentResliceExtent[5] = 0;
  SupportColorWindowing = true;
  InitialWL[0] = 0;
  InitialWL[1] = 0;
  WLStartPosition[0] = 0;
  WLStartPosition[1] = 0;
  CurrentWLFactor = 1.;
  CurrentWindowLevelChannel = 0; // relate window/level to image mapper!
  MainWindowLevelChannel = NULL;
  MainWindowLevelChannelResetWindowLevel[0] = 255;
  MainWindowLevelChannelResetWindowLevel[1] = 127.5;
  WindowLevelMouseSensitivity = 1.0; // default
  RealTimeMouseSensitivityAdaption = true; // default
  MinimumSpacingForZoom = -1;
  MaximumSpacingForZoom = -1;
  UseMinimumMaximumSpacingForZoom = false;
  UseMouseWheelForZoomingInOut = true;
  SecondaryTriggeredMouseWheel = false;
  SupportCursoring = false;
  CurrentInPlaneCursorPosition[0] = 0;
  CurrentInPlaneCursorPosition[1] = 0;
  CurrentWorldCursorPosition[0] = 0.0;
  CurrentWorldCursorPosition[1] = 0.0;
  CurrentWorldCursorPosition[2] = 0.0;
  ResetFlipStates();
  vtkSmartPointer<vtkCallbackCommand> cbc =
      vtkSmartPointer<vtkCallbackCommand>::New();
  cbc->SetCallback(WindowLevelCallback);
  cbc->SetClientData(this);
  this->AddObserver(vtkCommand::WindowLevelEvent, cbc);
  this->AddObserver(vtkCommand::StartWindowLevelEvent, cbc);
  this->AddObserver(vtkCommand::ResetWindowLevelEvent, cbc);
}

void vtk2DSceneImageInteractorStyle::WindowLevelCallback(vtkObject *caller,
    unsigned long eid, void *clientdata, void *calldata)
{
  vtk2DSceneImageInteractorStyle *thiss = reinterpret_cast<
      vtk2DSceneImageInteractorStyle *>(clientdata);
  if (thiss)
  {
    if (eid == vtkCommand::WindowLevelEvent)
    {
      thiss->WindowLevelInternal();
    }
    else if (eid == vtkCommand::StartWindowLevelEvent)
    {
      thiss->StartWindowLevelInternal();
    }
    else if (eid == vtkCommand::ResetWindowLevelEvent)
    {
      thiss->ResetWindowLevelInternal();
    }
  }
}

vtk2DSceneImageInteractorStyle::~vtk2DSceneImageInteractorStyle()
{
  RemoveAllWindowLevelChannels();
  Renderer = NULL;
  ReferenceImage = NULL;
  Magnifier = NULL;
  ResliceOrientation = NULL;
  ImageActor = NULL;
  ImageMapper = NULL;
}

void vtk2DSceneImageInteractorStyle::SetCurrentWindowLevelChannel(int channel)
{
  if (channel != CurrentWindowLevelChannel)
  {
    CurrentWindowLevelChannel = channel;
    this->InvokeEvent(vtk2DSceneImageInteractorStyle::WindowLevelChannelChanged, this);
    this->Modified();
  }
}

void vtk2DSceneImageInteractorStyle::SetMagnifier(vtkImageReslice *magnifier)
{
	if (magnifier != this->Magnifier)
	{
		this->Magnifier = magnifier;
		bool change = false;
		if (this->ResliceOrientation && this->Magnifier)
		{
		  this->UpdateResliceOrientation(); // ensure
		  vtkSmartPointer<vtkMatrix4x4> resliceAxes =
		      vtkSmartPointer<vtkMatrix4x4>::New();
		  // 'x'-axis:
		  resliceAxes->SetElement(0, 0, this->ResliceOrientation->GetElement(0, 0));
		  resliceAxes->SetElement(1, 0, this->ResliceOrientation->GetElement(0, 1));
		  resliceAxes->SetElement(2, 0, this->ResliceOrientation->GetElement(0, 2));
      // 'y'-axis:
      resliceAxes->SetElement(0, 1, this->ResliceOrientation->GetElement(1, 0));
      resliceAxes->SetElement(1, 1, this->ResliceOrientation->GetElement(1, 1));
      resliceAxes->SetElement(2, 1, this->ResliceOrientation->GetElement(1, 2));
      // 'z'-axis:
      resliceAxes->SetElement(0, 2, this->ResliceOrientation->GetElement(2, 0));
      resliceAxes->SetElement(1, 2, this->ResliceOrientation->GetElement(2, 1));
      resliceAxes->SetElement(2, 2, this->ResliceOrientation->GetElement(2, 2));
			this->Magnifier->SetResliceAxes(resliceAxes);
			change = true;
		}
		if (this->ReferenceImage && this->Magnifier)
		{
	    double aorigin[3], aspacing[3];
	    int aextent[6];
	    this->ComputeAdjustedMaximumReslicingParameters(aorigin, aspacing, aextent);
		  this->Magnifier->SetResliceAxesOrigin(aorigin);
		  change = true;
		}
		if (change)
		  this->AfterChangingMagnifier(); // provide entry point
		this->Modified();
	}
}

void vtk2DSceneImageInteractorStyle::SetReferenceImage(vtkImageData *image)
{
  if (this->ReferenceImage != image)
  {
    this->ReferenceImage = image;
    if (this->ReferenceImage && this->Magnifier)
    {
      double aorigin[3], aspacing[3];
      int aextent[6];
      this->ComputeAdjustedMaximumReslicingParameters(aorigin, aspacing, aextent);
      if (this->ResliceOrientation) // be sure that it is still valid!
      {
        this->Magnifier->SetResliceAxesOrigin(aorigin);
        this->AfterChangingMagnifier(); // provide entry point
      }
      if (FlippedAlongColumn || FlippedAlongRow)
        this->InvokeEvent(vtk2DSceneImageInteractorStyle::FlippingModeChanged, this);
      ResetFlipStates();
    }
    this->Modified();
  }
}

void vtk2DSceneImageInteractorStyle::SetRenderer(vtkRenderer *ren)
{
  if (ren != Renderer)
  {
    Renderer = ren;
    if (ren)
      RenderWindow = ren->GetRenderWindow();
    else
      RenderWindow = NULL;
    this->Modified();
  }
}

bool vtk2DSceneImageInteractorStyle::GetCurrentPixelSpacing(double spacing[2])
{
  if (ReferenceImage && CurrentMagnification != -10000)
  {
    double s[3];
    ReferenceImage->GetSpacing(s);
    spacing[0] = s[0] / CurrentMagnification;
    spacing[1] = s[1] / CurrentMagnification;
    return true;
  }
  else
  {
    return false;
  }
}

void vtk2DSceneImageInteractorStyle::ComputeVolumeCorners(double corners[8][3])
{
  if (ReferenceImage)
  {
  	ReferenceImage->UpdateInformation();
    double origin[3];
    ReferenceImage->GetOrigin(origin); // *center* (not corner) of first voxel!
    double spac[3];
    ReferenceImage->GetSpacing(spac);
    int we[6];
    ReferenceImage->GetWholeExtent(we);
    int dims[3];
    dims[0] = we[1] - we[0] + 1; // safe retrieval of image dimensions
    dims[1] = we[3] - we[2] + 1;
    dims[2] = we[5] - we[4] + 1;

    // -> here, we refer to the corner of the first voxel, so correct for it:
    origin[0] -= spac[0] / 2.;
    origin[1] -= spac[1] / 2.;
    origin[2] -= spac[2] / 2.;

    // width, height, depth:
    double w = spac[0] * (double)dims[0];
    double h = spac[1] * (double)dims[1];
    double d = spac[2] * (double)dims[2];

    // construct bounding volume box (on-grid):
    // p1
    corners[0][0] = origin[0];
    corners[0][1] = origin[1];
    corners[0][2] = origin[2];
    // p2
    corners[1][0] = origin[0] + w;
    corners[1][1] = origin[1];
    corners[1][2] = origin[2];
    // p3
    corners[2][0] = origin[0] + w;
    corners[2][1] = origin[1] + h;
    corners[2][2] = origin[2];
    // p4
    corners[3][0] = origin[0];
    corners[3][1] = origin[1] + h;
    corners[3][2] = origin[2];
    // p5
    corners[4][0] = origin[0];
    corners[4][1] = origin[1];
    corners[4][2] = origin[2] + d;
    // p6
    corners[5][0] = origin[0] + w;
    corners[5][1] = origin[1];
    corners[5][2] = origin[2] + d;
    // p7
    corners[6][0] = origin[0] + w;
    corners[6][1] = origin[1] + h;
    corners[6][2] = origin[2] + d;
    // p8
    corners[7][0] = origin[0];
    corners[7][1] = origin[1] + h;
    corners[7][2] = origin[2] + d;
  }
}

void vtk2DSceneImageInteractorStyle::ComputeAdjustedSpacing(double spacing[3])
{
	if (ReferenceImage)
	{
		// SPACING: take simply the smallest component isotropically
		double rspac[3];
		ReferenceImage->GetSpacing(rspac);
		spacing[0] = std::min(std::min(rspac[0], rspac[1]), rspac[2]);
		spacing[1] = spacing[0];
		spacing[2] = 1.0; // reslice PLANE!
	}
}

double vtk2DSceneImageInteractorStyle::ComputeAdjustedMaximumReslicingParameters(
    double origin[3], double spacing[3], int extent[6])
{
  if (ReferenceImage && ResliceOrientation)
  {
    double vc[8][3];
    ComputeVolumeCorners(vc); // get the volume corners in WCS

    this->UpdateResliceOrientation(); // ensure
    if (!ResliceOrientation) // possible after updating!
      return 0;

    // REAL MAXIMUM EXTENT:
    double *v;
    double minp, maxp;
    double maxExtent[6];
    double v1[3]; // row direction
    v1[0] = this->ResliceOrientation->GetElement(0, 0);
    v1[1] = this->ResliceOrientation->GetElement(0, 1);
    v1[2] = this->ResliceOrientation->GetElement(0, 2);
    double v2[3]; // column direction
    v2[0] = this->ResliceOrientation->GetElement(1, 0);
    v2[1] = this->ResliceOrientation->GetElement(1, 1);
    v2[2] = this->ResliceOrientation->GetElement(1, 2);
    double v3[3]; // slicing direction
    v3[0] = this->ResliceOrientation->GetElement(2, 0);
    v3[1] = this->ResliceOrientation->GetElement(2, 1);
    v3[2] = this->ResliceOrientation->GetElement(2, 2);
    for (int x = 0; x < 3; ++x)
    {
      if (x == 0) // normalized row-direction
        v = v1;
      else if (x == 1) // normalized column-direction
        v = v2;
      else // normalized slicing-direction
      	v = v3;
      minp = vtkMath::Dot(vc[0], v);
      maxp = minp;
      for (int i = 1; i < 8; ++i)
      {
        double pos = vtkMath::Dot(vc[i], v); // projection
        if (pos < minp)
          minp = pos;
        if (pos > maxp)
          maxp = pos;
      }
      // the plane extent is related to the WCS origin!
      maxExtent[x * 2 + 0] = minp; // store the direction extents
      maxExtent[x * 2 + 1] = maxp;
    }

    // SPACING: take simply the smallest component isotropically
    ComputeAdjustedSpacing(spacing);

    // ORIGIN: relate to the real maximum plane w.r.t. the reslicing matrix;
    // the discrepancy between plane pixel center and corner is also
    // considered; NOTE: We take the centered plane along the slicing direction!
    double centerCoordinate = (maxExtent[4] + maxExtent[5]) / 2.;
    origin[0] = (maxExtent[0] + spacing[0] / 2.) * v1[0] +
    		(maxExtent[2] + spacing[1] / 2.) * v2[0] +
    		v3[0] * centerCoordinate;
    origin[1] = (maxExtent[0] + spacing[0] / 2.) * v1[1] +
    		(maxExtent[2] + spacing[1] / 2.) * v2[1] +
    		v3[1] * centerCoordinate;
    origin[2] = (maxExtent[0] + spacing[0] / 2.) * v1[2] +
    		(maxExtent[2] + spacing[1] / 2.) * v2[2] +
    		v3[2] * centerCoordinate;

    // EXTENT: relate to the real maximum plane and isotropic spacing:
    extent[0] = 0;
    extent[1] = static_cast<int>(ceil((maxExtent[1] - maxExtent[0]) / spacing[0]) - 1);
    extent[2] = 0;
    extent[3] = static_cast<int>(ceil((maxExtent[3] - maxExtent[2]) / spacing[1]) - 1);
    extent[4] = 0; // reslice PLANE!
    extent[5] = 0;

    return centerCoordinate;
  }
  return 0;
}

void vtk2DSceneImageInteractorStyle::FitImagePortionToRenderWindow(
    double point1[3], double point2[3], bool adaptForZooming)
{
  if (!Magnifier || !ResliceOrientation || !RenderWindow ||
      !ReferenceImage || !ImageActor)
    return;
  // check geometric integrity:
  this->UpdateResliceOrientation(); // ensure
  double v3[3]; // slicing direction (plane normal)
  v3[0] = ResliceOrientation->GetElement(2, 0);
  v3[1] = ResliceOrientation->GetElement(2, 1);
  v3[2] = ResliceOrientation->GetElement(2, 2);
  double p0[3];
  this->Magnifier->GetResliceAxesOrigin(p0);

  // NOTE: Here, the error tolerance should not be too narrow. Especially, for
  // reformatted planes in 3D space, relatively large deviations can follow
  // (depending on the orientation axes accuracy)!
  if (vtkPlane::DistanceToPlane(point1, v3, p0) > F_WEAK_EPSILON)
    return; // point1 not on plane!
  if (vtkPlane::DistanceToPlane(point2, v3, p0) > F_WEAK_EPSILON)
    return; // point2 not on plane!

  double v1[3];
  v1[0] = ResliceOrientation->GetElement(0, 0);
  v1[1] = ResliceOrientation->GetElement(0, 1);
  v1[2] = ResliceOrientation->GetElement(0, 2);
  double v2[3];
  v2[0] = ResliceOrientation->GetElement(1, 0);
  v2[1] = ResliceOrientation->GetElement(1, 1);
  v2[2] = ResliceOrientation->GetElement(1, 2);

  double bwidth = vtkMath::Dot(point2, v1) - vtkMath::Dot(point1, v1);
  double bheight = vtkMath::Dot(point2, v2) - vtkMath::Dot(point1, v2);
  if (bwidth <= 0 || bheight <= 0)
    return; // point1/point2 incorrectly specified w.r.t. reslice orientation

  double aspacing[3];
  ComputeAdjustedSpacing(aspacing); // isotropic spacing
  double aorigin[3];
  aorigin[0] = point1[0];// - v1[0] * aspacing[0] / 2. - v2[0] * aspacing[1] / 2.;
  aorigin[1] = point1[1];// - v1[1] * aspacing[0] / 2. - v2[1] * aspacing[1] / 2.;
  aorigin[2] = point1[2];// - v1[2] * aspacing[0] / 2. - v2[2] * aspacing[1] / 2.;
  int aextent[6];
  aextent[0] = 0;
  aextent[1] = static_cast<int>(ceil(bwidth / aspacing[0])) - 1;
  aextent[2] = 0;
  aextent[3] = static_cast<int>(ceil(bheight / aspacing[1])) - 1;
  aextent[4] = 0;
  aextent[5] = 0; // plane
  int dims[3]; // dimensions retrieval
  dims[0] = aextent[1] - aextent[0] + 1;
  dims[1] = aextent[3] - aextent[2] + 1;
  dims[2] = aextent[5] - aextent[4] + 1;

  // NOTE: We do not need to consider the image spacing because this scene
  // is purely pixel-based!
  double width = (double)dims[0];
  double height = (double)dims[1];
  if (height <= 0)
    height = 1.0;
  if (width < 0)
    width = 1.0;
  int *sz = RenderWindow->GetSize();
  double w = (double)sz[0];
  double h = (double)sz[1];
  double vpaspect = w / h; // viewport
  double imgaspect = width / height; // image
  bool fitToHeight = false;
  // We need some logic on how to determine which dimension gets adapted!
  if (vpaspect >= 1)
  {
    if (imgaspect >= 1)
    {
      if (imgaspect <= vpaspect)
      {
        fitToHeight = true;
      }
      else
      {
        fitToHeight = false;
      }
    }
    else
    {
      fitToHeight = true;
    }
  }
  else // vpaspect < 1
  {
    if (imgaspect >= 1)
    {
      fitToHeight = false;
    }
    else if (imgaspect <= vpaspect)
    {
      fitToHeight = true;
    }
    else
    {
      fitToHeight = false;
    }
  }
  // calculate resulting plane height and width
  double planeHeight, planeWidth;
  CurrentMagnification = 1.; // common magnification - keep aspect ratio!
  if (fitToHeight)
  {
    CurrentMagnification = h / height;
    planeHeight = h;
    planeWidth = imgaspect * planeHeight;
  }
  else
  {
    CurrentMagnification = w / width;
    planeWidth = w;
    planeHeight = planeWidth / imgaspect;
  }

  // update reslice spacing:
  CurrentResliceSpacing[0] = aspacing[0] / CurrentMagnification;
  CurrentResliceSpacing[1] = aspacing[1] / CurrentMagnification;
  CurrentResliceSpacing[2] = 1; // not considered

  // update reslice extent:
  CurrentResliceExtent[0] = 0;
  CurrentResliceExtent[1] = static_cast<int>(ceil(dims[0] * CurrentMagnification)) - 1;
  CurrentResliceExtent[2] = 0;
  CurrentResliceExtent[3] = static_cast<int>(ceil(dims[1] * CurrentMagnification)) - 1;
  CurrentResliceExtent[4] = 0; // not considered
  CurrentResliceExtent[5] = 0;

  // determine whether we have to move the image actor:
  // (NOTE: the image actor snaps only to integer positions, not to floating
  // point pixel positions -> compute deviation and correct it later by
  // adapting the reslice origin accordingly)
  double deviation;
  double v[3];
  if (fitToHeight) // -> apply offset along row-direction if necessary
  {
    double offset = ((w - planeWidth) / 2.);
    double offsetr = vtkMath::Round(offset);
    deviation = (offset - offsetr) * CurrentResliceSpacing[0];
    ImageActor->SetPosition(offsetr, 0);
    v[0] = this->ResliceOrientation->GetElement(0, 0); // row direction
    v[1] = this->ResliceOrientation->GetElement(0, 1);
    v[2] = this->ResliceOrientation->GetElement(0, 2);
  }
  else // -> apply offset along column-direction if necessary
  {
    double offset = ((h - planeHeight) / 2.);
    double offsetr = vtkMath::Round(offset);
    deviation = (offset - offsetr) * CurrentResliceSpacing[1];
    ImageActor->SetPosition(0, offsetr);
    v[0] = this->ResliceOrientation->GetElement(1, 0); // column direction
    v[1] = this->ResliceOrientation->GetElement(1, 1);
    v[2] = this->ResliceOrientation->GetElement(1, 2);
  }

  // update reslice lower-left (WCS):
  CurrentResliceLLC[0] = aorigin[0] + deviation * v[0] /* correction */;
  CurrentResliceLLC[1] = aorigin[1] + deviation * v[1] /* correction */;
  CurrentResliceLLC[2] = aorigin[2] + deviation * v[2] /* correction */;

  // update reslice right-upper (WCS):
  // (effective width/height are dim-1 because RUC is the image origin of the
  // upper right pixel!)
  double ew = (double)CurrentResliceExtent[1] * CurrentResliceSpacing[0];
  double eh = (double)CurrentResliceExtent[3] * CurrentResliceSpacing[1];
  CurrentResliceRUC[0] = CurrentResliceLLC[0] + ew * v1[0] + eh * v2[0];
  CurrentResliceRUC[1] = CurrentResliceLLC[1] + ew * v1[1] + eh * v2[1];
  CurrentResliceRUC[2] = CurrentResliceLLC[2] + ew * v1[2] + eh * v2[2];

  // -> output origin is in-plane, not in WCS!
  double inplaneOrigin[3];
  double ref[3];
  Magnifier->GetResliceAxesOrigin(ref);
  inplaneOrigin[0] = vtkMath::Dot(CurrentResliceLLC, v1) -
      vtkMath::Dot(ref, v1);
  inplaneOrigin[1] = vtkMath::Dot(CurrentResliceLLC, v2) -
      vtkMath::Dot(ref, v2);
  inplaneOrigin[2] = 0;

  // apply settings to magnifier:
  Magnifier->SetOutputOrigin(inplaneOrigin);
  Magnifier->SetOutputSpacing(CurrentResliceSpacing);
  Magnifier->SetOutputExtent(CurrentResliceExtent);

  DoNotInvokeZoomingEvent = true;
  if (adaptForZooming)
    Zoom(1.0);
  DoNotInvokeZoomingEvent = false;

  this->AfterChangingMagnifier(); // provide entry point

  this->InvokeEvent(vtk2DSceneImageInteractorStyle::ImagePortionFittedToRenderWindow, this);
}

void vtk2DSceneImageInteractorStyle::FitRelativeImagePortionToRenderWindow(
    double point1[2], double point2[2], bool adaptForZooming)
{
  if (!Magnifier || !ResliceOrientation || !RenderWindow ||
      !ReferenceImage || !ImageActor)
    return;

  double origin[3];
  Magnifier->GetResliceAxesOrigin(origin);
  this->UpdateResliceOrientation(); // ensure
  double v1[3];
  v1[0] = ResliceOrientation->GetElement(0, 0);
  v1[1] = ResliceOrientation->GetElement(0, 1);
  v1[2] = ResliceOrientation->GetElement(0, 2);
  double v2[3];
  v2[0] = ResliceOrientation->GetElement(1, 0);
  v2[1] = ResliceOrientation->GetElement(1, 1);
  v2[2] = ResliceOrientation->GetElement(1, 2);

  double p1[3];
  p1[0] = origin[0] + point1[0] * v1[0] + point1[1] * v2[0];
  p1[1] = origin[1] + point1[0] * v1[1] + point1[1] * v2[1];
  p1[2] = origin[2] + point1[0] * v1[2] + point1[1] * v2[2];
  double p2[3];
  p2[0] = origin[0] + point2[0] * v1[0] + point2[1] * v2[0];
  p2[1] = origin[1] + point2[0] * v1[1] + point2[1] * v2[1];
  p2[2] = origin[2] + point2[0] * v1[2] + point2[1] * v2[2];

  FitImagePortionToRenderWindow(p1, p2, adaptForZooming); // 3D version
}

void vtk2DSceneImageInteractorStyle::FitImageToRenderWindow()
{
  if (RenderWindow && ReferenceImage && ImageActor && Magnifier)
  {
    double aorigin[3], aspacing[3];
    int aextent[6];
    this->ComputeAdjustedMaximumReslicingParameters(aorigin, aspacing, aextent);
    this->Magnifier->SetResliceAxesOrigin(aorigin); // be sure it is set

    this->UpdateResliceOrientation(); // ensure
    double v1[3];
    v1[0] = ResliceOrientation->GetElement(0, 0);
    v1[1] = ResliceOrientation->GetElement(0, 1);
    v1[2] = ResliceOrientation->GetElement(0, 2);
    double v2[3];
    v2[0] = ResliceOrientation->GetElement(1, 0);
    v2[1] = ResliceOrientation->GetElement(1, 1);
    v2[2] = ResliceOrientation->GetElement(1, 2);

    double p1[3]; // cover the whole image (maximum region)
    p1[0] = aorigin[0] - v1[0] * aspacing[0] / 2. - v2[0] * aspacing[1] / 2.;
    p1[1] = aorigin[1] - v1[1] * aspacing[0] / 2. - v2[1] * aspacing[1] / 2.;
    p1[2] = aorigin[2] - v1[2] * aspacing[0] / 2. - v2[2] * aspacing[1] / 2.;
    double p2[3];
    p2[0] = p1[0] + v1[0] * ((double)aextent[1] + 1.0) * aspacing[0] +
        v2[0] * ((double)aextent[3] + 1.0) * aspacing[1];
    p2[1] = p1[1] + v1[1] * ((double)aextent[1] + 1.0) * aspacing[0] +
        v2[1] * ((double)aextent[3] + 1.0) * aspacing[1];
    p2[2] = p1[2] + v1[2] * ((double)aextent[1] + 1.0) * aspacing[0] +
        v2[2] * ((double)aextent[3] + 1.0) * aspacing[1];

    FitImagePortionToRenderWindow(p1, p2, false);

    this->InvokeEvent(vtk2DSceneImageInteractorStyle::ImageFittedToRenderWindow, this);
  }
}

void vtk2DSceneImageInteractorStyle::RestoreViewSettings()
{
  if (Magnifier && CurrentMagnification != -10000 && RenderWindow)
  {
    DoNotInvokeZoomingEvent = true;
    int *sz = RenderWindow->GetSize();
    if (sz[0] <= 0 || sz[1] <= 0)
      return;
    double newWidth = static_cast<double>(sz[0]);
    double oldWidth = static_cast<double>(CurrentResliceExtent[1] -
        CurrentResliceExtent[0] + 1);
    double newHeight = static_cast<double>(sz[1]);
    double oldHeight = static_cast<double>(CurrentResliceExtent[3] -
        CurrentResliceExtent[2] + 1);
    double f = newWidth / oldWidth;
    double f2 = newHeight / oldHeight;
    if (oldWidth >= oldHeight)
      Zoom(f);
    else
      Zoom(f2);
    // if min/max constraints for zooming are set, these have to be verified
    // internally by using the appropriate zooming directions (yes, I know, this
    // will introduce a minimal error ...)
    if (UseMinimumMaximumSpacingForZoom)
    {
      if (f < 1.)
      	Zoom(1.00000001);
      else
      	Zoom(0.99999999);
    }
    DoNotInvokeZoomingEvent = false;
    this->InvokeEvent(vtk2DSceneImageInteractorStyle::ViewRestored, this);
  }
}

void vtk2DSceneImageInteractorStyle::OnKeyDown()
{
  if (this->Interactor->GetShiftKey() ||
      this->Interactor->GetControlKey())
    return;
  if (this->Interactor->GetAltKey()) // unfortunately never true!
  {
    PseudoAltKeyFlag = true;
    return;
  }
  char key = this->Interactor->GetKeyCode();
  if (key == 0) // it's likely that ALT was pressed
    PseudoAltKeyFlag = true;
}

void vtk2DSceneImageInteractorStyle::OnKeyUp()
{
  PseudoAltKeyFlag = false;
}

void vtk2DSceneImageInteractorStyle::OnLeftButtonDown()
{
  if (PseudoAltKeyFlag || ExternalPseudoAltKeyFlag) // PAN
  {
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                            this->Interactor->GetEventPosition()[1]);
    if (!this->CurrentRenderer)
      return;

    this->GrabFocus(this->EventCallbackCommand);
    this->StartPan();
  }
  else if (SupportCursoring)
  {
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->FindPokedRenderer(x, y);
    if (!this->CurrentRenderer)
      return;

    this->GrabFocus(this->EventCallbackCommand);
  	CurrentInPlaneCursorPosition[0] = x;
  	CurrentInPlaneCursorPosition[1] = y;
  	this->MapInPlaneCoordinatesIntoWorldCoordinates(CurrentInPlaneCursorPosition,
  			CurrentWorldCursorPosition);
    this->StartCursor();
  }
}

void vtk2DSceneImageInteractorStyle::OnLeftButtonUp()
{
  switch (this->State)
  {
    case VTKIS_PAN:
      this->EndPan();
      if (this->Interactor)
        this->ReleaseFocus();
      break;
    case VTKIS_CURSOR:
    	int x = 0, y = 0;
    	if (this->Interactor)
    	{
        x = this->Interactor->GetEventPosition()[0];
        y = this->Interactor->GetEventPosition()[1];
    	}
    	CurrentInPlaneCursorPosition[0] = x;
    	CurrentInPlaneCursorPosition[1] = y;
    	this->MapInPlaneCoordinatesIntoWorldCoordinates(CurrentInPlaneCursorPosition,
    			CurrentWorldCursorPosition);
      this->EndCursor();
      if (this->Interactor)
        this->ReleaseFocus();
      break;
  }
}

void vtk2DSceneImageInteractorStyle::OnChar()
{
  vtkRenderWindowInteractor *rwi = this->Interactor;

  switch (rwi->GetKeyCode())
  {
    case 'r':
    case 'R':
      if (!rwi->GetControlKey())
      {
        FitImageToRenderWindow();
        rwi->Render();
      }
      break;
    case 'i':
    case 'I':
    	AlterInterpolationMode();
    	if (Magnifier)
    	{
    		Magnifier->Modified();
    		rwi->Render();
    	}
    	break;
    case 'x':
    case 'X':
    	FlipImageAlongRowDirection(ResliceOrientation);
    	rwi->Render();
    	break;
    case 'y':
    case 'Y':
    	FlipImageAlongColumnDirection(ResliceOrientation);
    	rwi->Render();
    	break;
    case 'w':
    case 'W':
      AlterWindowLevelChannel();
      break;
  }

  // do not forward, deactivate the other keys
}

void vtk2DSceneImageInteractorStyle::OnKeyPress()
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  std::string key = rwi->GetKeySym();

  if (key.compare("r") == 0 && rwi->GetControlKey())
  {
    this->InvokeEvent(vtkCommand::ResetWindowLevelEvent);
    return;
  }

  if (key.compare("plus") == 0)
  {
    SecondaryTriggeredMouseWheel = true;
    OnMouseWheelForward(); // zoom-in
    SecondaryTriggeredMouseWheel = false;
    return; // no forwarding
  }
  else if (key.compare("minus") == 0)
  {
    SecondaryTriggeredMouseWheel = true;
    OnMouseWheelBackward(); // zoom-out
    SecondaryTriggeredMouseWheel = false;
    return; // no forwarding
  }

  // forward events
  vtkInteractorStyleImage::OnKeyPress();
}

void vtk2DSceneImageInteractorStyle::Zoom(double factor)
{
  if (CurrentMagnification != -10000 && ReferenceImage && RenderWindow)
  {
    double newMagnification = CurrentMagnification * factor;
    double refSpacing[3];
    double newSpacing[3];
    ComputeAdjustedSpacing(refSpacing);
    // -> new spacing due to magnification
    newSpacing[0] = refSpacing[0] / newMagnification;
    newSpacing[1] = refSpacing[1] / newMagnification;
    newSpacing[2] = 1; // not considered
    if (UseMinimumMaximumSpacingForZoom && MinimumSpacingForZoom > 0 &&
        factor > 1.) // max zoom
    {
      if (newSpacing[0] < MinimumSpacingForZoom ||
          newSpacing[1] < MinimumSpacingForZoom)
      {
        if (newSpacing[0] < newSpacing[1])
          newMagnification = refSpacing[0] / MinimumSpacingForZoom;
        else
          newMagnification = refSpacing[1] / MinimumSpacingForZoom;
        if (fabs(CurrentMagnification - newMagnification) < 1e-6)
          return; // no change - already at maximum zoom -> exit
        newSpacing[0] = refSpacing[0] / newMagnification;
        newSpacing[1] = refSpacing[1] / newMagnification;
        // update factor as well:
        factor = newMagnification / CurrentMagnification;
      }
    }
    if (UseMinimumMaximumSpacingForZoom && MaximumSpacingForZoom > 0 &&
        factor < 1.) // min zoom
    {
      if (newSpacing[0] > MaximumSpacingForZoom ||
          newSpacing[1] > MaximumSpacingForZoom)
      {
        if (newSpacing[0] > newSpacing[1])
          newMagnification = refSpacing[0] / MaximumSpacingForZoom;
        else
          newMagnification = refSpacing[1] / MaximumSpacingForZoom;
        if (fabs(CurrentMagnification - newMagnification) < 1e-6)
          return; // no change - already at maximum zoom -> exit
        newSpacing[0] = refSpacing[0] / newMagnification;
        newSpacing[1] = refSpacing[1] / newMagnification;
        // update factor as well:
        factor = newMagnification / CurrentMagnification;
      }
    }

    double aorigin[3], aspacing[3];
    int aextent[6];
    ComputeAdjustedMaximumReslicingParameters(aorigin, aspacing, aextent);

    double iwidth = ((double)aextent[1] + 1.0) * aspacing[0];
    double iheight = ((double)aextent[3] + 1.0) * aspacing[1];
    int newDims[2];
    newDims[0] = static_cast<int>(ceil(iwidth / newSpacing[0]));
    newDims[1] = static_cast<int>(ceil(iheight / newSpacing[1]));

    double center[3];
		center[0] = (CurrentResliceLLC[0] + CurrentResliceRUC[0]) / 2.;
		center[1] = (CurrentResliceLLC[1] + CurrentResliceRUC[1]) / 2.;
		center[2] = (CurrentResliceLLC[2] + CurrentResliceRUC[2]) / 2.;

		this->UpdateResliceOrientation(); // ensure
    double v1[3];
    v1[0] = ResliceOrientation->GetElement(0, 0); // image row direction
    v1[1] = ResliceOrientation->GetElement(0, 1);
    v1[2] = ResliceOrientation->GetElement(0, 2);
    double v2[3];
    v2[0] = ResliceOrientation->GetElement(1, 0); // image column direction
    v2[1] = ResliceOrientation->GetElement(1, 1);
    v2[2] = ResliceOrientation->GetElement(1, 2);

    int *sz = RenderWindow->GetSize();
    bool fakeRUC = false;
    double actorPos[2];
    if (newDims[0] >= sz[0]) // no image actor movement (horizontal)
    {
    	actorPos[0] = 0;
    	iwidth = sz[0] * newSpacing[0]; // max. width
    	CurrentResliceLLC[0] = center[0] - v1[0] * iwidth / 2.;
    	CurrentResliceLLC[1] = center[1] - v1[1] * iwidth / 2.;
    	CurrentResliceLLC[2] = center[2] - v1[2] * iwidth / 2.;
    	CurrentResliceExtent[1] = sz[0];
    }
    else // image actor movement
    {
    	actorPos[0] = (double)(sz[0] - newDims[0]) / 2.;
    	double deviation = (actorPos[0] - vtkMath::Round(actorPos[0])) * newSpacing[0];
    	actorPos[0] = vtkMath::Round(actorPos[0]);
    	CurrentResliceLLC[0] = center[0] - v1[0] * (iwidth / 2. + deviation);
    	CurrentResliceLLC[1] = center[1] - v1[1] * (iwidth / 2. + deviation);
    	CurrentResliceLLC[2] = center[2] - v1[2] * (iwidth / 2. + deviation);
    	CurrentResliceExtent[1] = newDims[0];
    	fakeRUC = true;
    }
    if (newDims[1] >= sz[1]) // no image actor movement (vertical)
    {
    	actorPos[1] = 0;
    	iheight = sz[1] * newSpacing[1]; // max. height
    	CurrentResliceLLC[0] = CurrentResliceLLC[0] - v2[0] * iheight / 2.;
    	CurrentResliceLLC[1] = CurrentResliceLLC[1] - v2[1] * iheight / 2.;
    	CurrentResliceLLC[2] = CurrentResliceLLC[2] - v2[2] * iheight / 2.;
    	CurrentResliceExtent[3] = sz[1];
    }
    else // image actor movement
    {
    	actorPos[1] = (double)(sz[1] - newDims[1]) / 2.;
    	double deviation = (actorPos[1] - vtkMath::Round(actorPos[1])) * newSpacing[1];
    	actorPos[1] = vtkMath::Round(actorPos[1]);
    	CurrentResliceLLC[0] = CurrentResliceLLC[0] - v2[0] * (iheight / 2. + deviation);
    	CurrentResliceLLC[1] = CurrentResliceLLC[1] - v2[1] * (iheight / 2. + deviation);
    	CurrentResliceLLC[2] = CurrentResliceLLC[2] - v2[2] * (iheight / 2. + deviation);
    	CurrentResliceExtent[3] = newDims[1];
    	fakeRUC = true;
    }

    CurrentMagnification = newMagnification;
    CurrentResliceSpacing[0] = newSpacing[0];
    CurrentResliceSpacing[1] = newSpacing[1];
    CurrentResliceSpacing[2] = newSpacing[2];
    if (!fakeRUC)
    {
			double ew = (double)CurrentResliceExtent[1] * CurrentResliceSpacing[0];
			double eh = (double)CurrentResliceExtent[3] * CurrentResliceSpacing[1];
			CurrentResliceRUC[0] = CurrentResliceLLC[0] + ew * v1[0] + eh * v2[0];
			CurrentResliceRUC[1] = CurrentResliceLLC[1] + ew * v1[1] + eh * v2[1];
			CurrentResliceRUC[2] = CurrentResliceLLC[2] + ew * v1[2] + eh * v2[2];
    }
    else // have to fake RUC in order to preserve the zooming center!
    {
    	double h[3];
    	h[0] = center[0] - CurrentResliceLLC[0];
    	h[1] = center[1] - CurrentResliceLLC[1];
    	h[2] = center[2] - CurrentResliceLLC[2];
    	CurrentResliceRUC[0] = center[0] + h[0];
    	CurrentResliceRUC[1] = center[1] + h[1];
    	CurrentResliceRUC[2] = center[2] + h[2];
    }

    ImageActor->SetPosition(actorPos);

    // -> output origin is in-plane, not in WCS!
    double ref[3];
    Magnifier->GetResliceAxesOrigin(ref);
    double inplaneOrigin[3];
    inplaneOrigin[0] = vtkMath::Dot(CurrentResliceLLC, v1) -
    		vtkMath::Dot(ref, v1);
    inplaneOrigin[1] = vtkMath::Dot(CurrentResliceLLC, v2) -
    		vtkMath::Dot(ref, v2);
    inplaneOrigin[2] = 0;

    // apply settings to magnifier:
    Magnifier->SetOutputOrigin(inplaneOrigin);
    Magnifier->SetOutputSpacing(CurrentResliceSpacing);
    Magnifier->SetOutputExtent(CurrentResliceExtent);

    this->AfterChangingMagnifier(); // provide entry point

    if (!DoNotInvokeZoomingEvent)
      this->InvokeEvent(vtk2DSceneImageInteractorStyle::Zooming, this);

    this->Interactor->Render();
  }
}

void vtk2DSceneImageInteractorStyle::OnMouseWheelForward()
{
  if (!SecondaryTriggeredMouseWheel && !UseMouseWheelForZoomingInOut)
    return;
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
      this->Interactor->GetEventPosition()[1]);
  if (!this->CurrentRenderer)
    return;
  double factor = this->MotionFactor * 0.2 * this->MouseWheelMotionFactor;
  factor = pow(1.1, factor);
  this->GrabFocus(this->EventCallbackCommand);
  this->StartZoom();
  Zoom(factor);
  this->EndZoom();
  this->ReleaseFocus();
  this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
}

void vtk2DSceneImageInteractorStyle::OnMouseWheelBackward()
{
  if (!SecondaryTriggeredMouseWheel && !UseMouseWheelForZoomingInOut)
      return;
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
      this->Interactor->GetEventPosition()[1]);
  if (!this->CurrentRenderer)
    return;
  double factor = this->MotionFactor * -0.2 * this->MouseWheelMotionFactor;
  factor = pow(1.1, factor);
  this->GrabFocus(this->EventCallbackCommand);
  this->StartZoom();
  Zoom(factor);
  this->EndZoom();
  this->ReleaseFocus();
  this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
}

void vtk2DSceneImageInteractorStyle::Pan(int dx, int dy)
{
  if (CurrentMagnification != -10000 && ReferenceImage && RenderWindow)
  {
    double aorigin[3], aspacing[3];
    int aextent[6];
    this->ComputeAdjustedMaximumReslicingParameters(aorigin, aspacing, aextent);

    this->UpdateResliceOrientation(); // ensure
    double v1[3];
    v1[0] = ResliceOrientation->GetElement(0, 0);
    v1[1] = ResliceOrientation->GetElement(0, 1);
    v1[2] = ResliceOrientation->GetElement(0, 2);
    double v2[3];
    v2[0] = ResliceOrientation->GetElement(1, 0);
    v2[1] = ResliceOrientation->GetElement(1, 1);
    v2[2] = ResliceOrientation->GetElement(1, 2);

    double ddx = -dx * CurrentResliceSpacing[0];
    double ddy = -dy * CurrentResliceSpacing[1];

    CurrentResliceLLC[0] += ddx * v1[0] + ddy * v2[0];
    CurrentResliceLLC[1] += ddx * v1[1] + ddy * v2[1];
    CurrentResliceLLC[2] += ddx * v1[2] + ddy * v2[2];

		double ew = (double)CurrentResliceExtent[1] * CurrentResliceSpacing[0];
		double eh = (double)CurrentResliceExtent[3] * CurrentResliceSpacing[1];
		CurrentResliceRUC[0] = CurrentResliceLLC[0] + ew * v1[0] + eh * v2[0];
		CurrentResliceRUC[1] = CurrentResliceLLC[1] + ew * v1[1] + eh * v2[1];
		CurrentResliceRUC[2] = CurrentResliceLLC[2] + ew * v1[2] + eh * v2[2];

    // -> output origin is in-plane, not in WCS!
	  double ref[3];
	  Magnifier->GetResliceAxesOrigin(ref);
    double inplaneOrigin[3];
    inplaneOrigin[0] = vtkMath::Dot(CurrentResliceLLC, v1) -
    		vtkMath::Dot(ref, v1);
    inplaneOrigin[1] = vtkMath::Dot(CurrentResliceLLC, v2) -
    		vtkMath::Dot(ref, v2);
    inplaneOrigin[2] = 0;

    // apply settings to magnifier:
    Magnifier->SetOutputOrigin(inplaneOrigin);
    Magnifier->SetOutputSpacing(CurrentResliceSpacing);
    Magnifier->SetOutputExtent(CurrentResliceExtent);

    this->AfterChangingMagnifier(); // provide entry point

    this->InvokeEvent(vtk2DSceneImageInteractorStyle::Panning, this);

    this->Interactor->Render();
  }
}

void vtk2DSceneImageInteractorStyle::OnMiddleButtonDown()
{
  if (SupportColorWindowing && ImageMapper)
  {
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->FindPokedRenderer(x, y);
    if (!this->CurrentRenderer)
      return;
    this->GrabFocus(this->EventCallbackCommand);
    this->WindowLevelStartPosition[0] = x;
    this->WindowLevelStartPosition[1] = y;
    this->StartWindowLevel();
  }
}

void vtk2DSceneImageInteractorStyle::OnMiddleButtonUp()
{
  this->EndWindowLevel();
}

void vtk2DSceneImageInteractorStyle::OnRightButtonDown()
{
  this->StartZoom();
}

void vtk2DSceneImageInteractorStyle::OnRightButtonUp()
{
  this->EndZoom();
}

void vtk2DSceneImageInteractorStyle::OnMouseMove()
{
  switch (this->State)
  {
    case VTKIS_PAN:
    case VTKIS_ZOOM:
    case VTKIS_CURSOR:
      break;
    default:
      this->vtkInteractorStyleImage::OnMouseMove(); // forward
      return; // early exit
  }

  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  double *center = NULL;
  int dy = 0;
  double dyf = 0;
  switch (this->State)
  {
    case VTKIS_PAN:
      this->FindPokedRenderer(x, y);
      Pan(x - this->Interactor->GetLastEventPosition()[0], y
          - this->Interactor->GetLastEventPosition()[1]);
      this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      break;
    case VTKIS_ZOOM:
      this->FindPokedRenderer(x, y);
      center = this->CurrentRenderer->GetCenter();
      dy = this->Interactor->GetEventPosition()[1]
          - this->Interactor->GetLastEventPosition()[1];
      dyf = this->MotionFactor * dy / center[1];
      Zoom(pow(1.1, dyf));
      this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
      break;
    case VTKIS_CURSOR:
    	CurrentInPlaneCursorPosition[0] = x;
    	CurrentInPlaneCursorPosition[1] = y;
    	this->MapInPlaneCoordinatesIntoWorldCoordinates(CurrentInPlaneCursorPosition,
    			CurrentWorldCursorPosition);
    	this->InvokeEvent(Cursoring, NULL);
    	break;
  }
}

void vtk2DSceneImageInteractorStyle::WindowLevelInternal()
{
  bool ok = SupportColorWindowing;
  if (CurrentWindowLevelChannel < 1 ||
      CurrentWindowLevelChannel > (int)WindowLevelChannels.size())
  {
    if (!ImageMapper || !ImageMapper->GetInput())
      ok = false;
  }
  else
  {
    if (!WindowLevelChannels[CurrentWindowLevelChannel - 1] ||
        !WindowLevelChannels[CurrentWindowLevelChannel - 1]->GetInput())
      ok = false;
  }
  if (!ok)
    return;

  if (RealTimeMouseSensitivityAdaption)
    ComputeCurrentWindowLevelFactors(); // re-compute sensitivity factors

  double fwin = CurrentWLFactor;
  double flev = CurrentWLFactor;
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];
  if (!RealTimeMouseSensitivityAdaption)
  {
    CurrentWL[0] = InitialWL[0] + fwin * (double)(WLStartPosition[0] - X);
    CurrentWL[1] = InitialWL[1] + flev * (double)(WLStartPosition[1] - Y);
  }
  else
  {
    CurrentWL[0] = CurrentWL[0] + fwin * (double)(WLStartPosition[0] - X);
    CurrentWL[1] = CurrentWL[1] + flev * (double)(WLStartPosition[1] - Y);
    WLStartPosition[0] = X; // update for next window/level step
    WLStartPosition[1] = Y;
  }

  if (CurrentWindowLevelChannel <= 0 ||
      CurrentWindowLevelChannel > (int)WindowLevelChannels.size()) // main chan.
  {
    // finally take over and render
    if (!MainWindowLevelChannel)
    {
      ImageMapper->SetColorWindow(CurrentWL[0]);
      ImageMapper->SetColorLevel(CurrentWL[1]);
    }
    else // LUT set
    {
      // assume vtkLookupTable or subclass:
      vtkLookupTable *lut = reinterpret_cast<vtkLookupTable *>(
          MainWindowLevelChannel->GetLookupTable());
      double r[2];
      r[0] = CurrentWL[1] - CurrentWL[0] / 2.; // min
      r[1] = CurrentWL[1] + CurrentWL[0] / 2.; // max
      lut->SetTableRange(r);
      lut->Build();
    }
  }
  else
  {
    vtkImageMapToColors *map = WindowLevelChannels[CurrentWindowLevelChannel - 1];
    // assume vtkLookupTable or subclass:
    vtkLookupTable *lut = reinterpret_cast<vtkLookupTable *>(map->GetLookupTable());
    double r[2];
    r[0] = CurrentWL[1] - CurrentWL[0] / 2.; // min
    r[1] = CurrentWL[1] + CurrentWL[0] / 2.; // max
    lut->SetTableRange(r);
    lut->Build();
  }

  this->Interactor->Render();
}

void vtk2DSceneImageInteractorStyle::StartWindowLevelInternal()
{
  bool ok = SupportColorWindowing;
  if (CurrentWindowLevelChannel < 1 ||
      CurrentWindowLevelChannel > (int)WindowLevelChannels.size())
  {
    if (!ImageMapper || !ImageMapper->GetInput())
      ok = false;
  }
  else
  {
    if (!WindowLevelChannels[CurrentWindowLevelChannel - 1] ||
        !WindowLevelChannels[CurrentWindowLevelChannel - 1]->GetInput())
      ok = false;
  }
  if (!ok)
    return;

  WLStartPosition[0] = this->Interactor->GetEventPosition()[0];
  WLStartPosition[1] = this->Interactor->GetEventPosition()[1];

  vtkImageData *input = NULL;
  if (CurrentWindowLevelChannel <= 0 ||
      CurrentWindowLevelChannel > (int)WindowLevelChannels.size()) // main chan.
  {
    if (!MainWindowLevelChannel)
    {
      InitialWL[0] = ImageMapper->GetColorWindow(); // store initial situation
      InitialWL[1] = ImageMapper->GetColorLevel();
      // for reliable scalar range during window/level:
      ImageMapper->GetInput()->UpdateInformation();
      ImageMapper->GetInput()->SetUpdateExtent
        (ImageMapper->GetInput()->GetWholeExtent());
      ImageMapper->GetInput()->Update();
      input = ImageMapper->GetInput();
    }
    else // LUT set
    {
      vtkLookupTable *lut = reinterpret_cast<vtkLookupTable *>(
          MainWindowLevelChannel->GetLookupTable());
      double r[2];
      lut->GetTableRange(r);
      InitialWL[0] = r[1] - r[0]; // window
      InitialWL[1] = r[0] + InitialWL[0] / 2.; // level
      // for reliable scalar range during window/level:
      MainWindowLevelChannel->GetInput()->UpdateInformation();
      MainWindowLevelChannel->GetInput()->SetUpdateExtent
        (MainWindowLevelChannel->GetInput()->GetWholeExtent());
      MainWindowLevelChannel->GetInput()->Update();
      input = reinterpret_cast<vtkImageData*>(MainWindowLevelChannel->GetInput());
    }
  }
  else // further window/level channel
  {
    vtkImageMapToColors *map = WindowLevelChannels[CurrentWindowLevelChannel - 1];
    // assume vtkLookupTable or subclass:
    vtkLookupTable *lut = reinterpret_cast<vtkLookupTable *>(map->GetLookupTable());
    double r[2];
    lut->GetTableRange(r);
    InitialWL[0] = r[1] - r[0]; // window
    InitialWL[1] = r[0] + InitialWL[0] / 2.; // level
    // for reliable scalar range during window/level:
    map->GetInput()->UpdateInformation();
    map->GetInput()->SetUpdateExtent
      (map->GetInput()->GetWholeExtent());
    map->GetInput()->Update();
    input = reinterpret_cast<vtkImageData*>(map->GetInput());
  }
  CurrentWL[0] = InitialWL[0];
  CurrentWL[1] = InitialWL[1];

  int components = input->GetPointData()->GetScalars()->GetNumberOfComponents();
  if (components == 3)
  {
    double sr0[2];
    double sr1[2];
    double sr2[2];
    input->GetPointData()->GetScalars()->GetRange(sr0, 0);
    input->GetPointData()->GetScalars()->GetRange(sr1, 1);
    input->GetPointData()->GetScalars()->GetRange(sr2, 2);
    CurrentSR[0] = std::min(sr0[0], std::min(sr1[0], sr2[0]));
    CurrentSR[1] = std::max(sr0[1], std::max(sr1[1], sr2[1]));
  }
  else
  {
    input->GetScalarRange(CurrentSR);
  }

  // compute sensitivity factors dependent on current scalar range
  // (CurrentSR), current window/level (CurrentWL) and current
  // mouse sensitivity control (WindowLevelMouseSensitivity):
  ComputeCurrentWindowLevelFactors();
}

void vtk2DSceneImageInteractorStyle::ComputeCurrentWindowLevelFactors()
{
  double contrast = 1 - CurrentWL[0] / (CurrentSR[1] - CurrentSR[0]);
  //double brightness = 1 - (CurrentWL[1] - CurrentSR[0]) / (CurrentSR[1] - CurrentSR[0]);
  double diffc = 1. - contrast;
  if (diffc > 2.)
    diffc = 2.;
  CurrentWLFactor = (exp(diffc) - 1.) / (exp(1.) - 1.) *
      WindowLevelMouseSensitivity + 0.01;
  // factor is optimized for a span width of 255 -> adapt to real image span
  // width:
  CurrentWLFactor = CurrentWLFactor / 255. * (CurrentSR[1] - CurrentSR[0]);
}

void vtk2DSceneImageInteractorStyle::ResetWindowLevelInternal()
{
  bool ok = SupportColorWindowing;
  if (CurrentWindowLevelChannel < 1 ||
      CurrentWindowLevelChannel > (int)WindowLevelChannels.size())
  {
    if (!ImageMapper || !ImageMapper->GetInput())
      ok = false;
  }
  else
  {
    if (!WindowLevelChannels[CurrentWindowLevelChannel - 1] ||
        !WindowLevelChannels[CurrentWindowLevelChannel - 1]->GetInput())
      ok = false;
  }
  if (!ok)
    return;

  this->StartWindowLevel();

  if (CurrentWindowLevelChannel <= 0 ||
      CurrentWindowLevelChannel > (int)WindowLevelChannels.size()) // main chan.
  {
    double sr[2];
    if (!MainWindowLevelChannel)
    {
      ImageMapper->GetInput()->UpdateInformation();
      ImageMapper->GetInput()->SetUpdateExtent
        (ImageMapper->GetInput()->GetWholeExtent());
      ImageMapper->GetInput()->Update();
      int components = ImageMapper->GetInput()->GetPointData()->GetScalars()->
          GetNumberOfComponents();
      if (components == 3)
      {
        double sr0[2];
        double sr1[2];
        double sr2[2];
        ImageMapper->GetInput()->GetPointData()->GetScalars()->GetRange(sr0, 0);
        ImageMapper->GetInput()->GetPointData()->GetScalars()->GetRange(sr1, 1);
        ImageMapper->GetInput()->GetPointData()->GetScalars()->GetRange(sr2, 2);
        sr[0] = std::min(sr0[0], std::min(sr1[0], sr2[0]));
        sr[1] = std::max(sr0[1], std::max(sr1[1], sr2[1]));
      }
      else
      {
        ImageMapper->GetInput()->GetScalarRange(sr);
      }
      ImageMapper->SetColorWindow(sr[1] - sr[0]);
      ImageMapper->SetColorLevel(sr[0] + (sr[1] - sr[0]) / 2.);
    }
    else // LUT set
    {
      vtkLookupTable *lut = reinterpret_cast<vtkLookupTable *>(
          MainWindowLevelChannel->GetLookupTable());
      double r[2];
      r[0] = MainWindowLevelChannelResetWindowLevel[1] -
          MainWindowLevelChannelResetWindowLevel[0] / 2.; // min
      r[1] = MainWindowLevelChannelResetWindowLevel[1] +
          MainWindowLevelChannelResetWindowLevel[0] / 2.; // max
      lut->SetTableRange(r);
      lut->Build();
    }
  }
  else // further window/level channel
  {
    double *wl = WindowLevelChannelsResetWindowLevels[CurrentWindowLevelChannel - 1];
    vtkImageMapToColors *map = WindowLevelChannels[CurrentWindowLevelChannel - 1];
    // assume vtkLookupTable or subclass:
    vtkLookupTable *lut = reinterpret_cast<vtkLookupTable *>(map->GetLookupTable());
    double r[2];
    r[0] = wl[1] - wl[0] / 2.; // min
    r[1] = wl[1] + wl[0] / 2.; // max
    lut->SetTableRange(r);
    lut->Build();
  }

  this->Interactor->Render();
  this->EndWindowLevel();
}

int vtk2DSceneImageInteractorStyle::AddWindowLevelChannel(
    vtkImageMapToColors *channel, double *resetWindowLevel)
{
  if (!resetWindowLevel)
    return 0;

  WindowLevelChannels.push_back(channel);
  double *wl = new double[2];
  wl[0] = resetWindowLevel[0];
  wl[1] = resetWindowLevel[1];
  WindowLevelChannelsResetWindowLevels.push_back(wl);

  return WindowLevelChannels.size();
}

bool vtk2DSceneImageInteractorStyle::RemoveWindowLevelChannel(
    vtkImageMapToColors *channel)
{
  std::vector<vtkImageMapToColors *>::iterator it;
  int idx = 0;
  for (it = WindowLevelChannels.begin(); it != WindowLevelChannels.end(); ++it)
  {
    idx++;
    if (*it == channel)
    {
      // -> adapt selection if necessary
      if (CurrentWindowLevelChannel >= 1 &&
          CurrentWindowLevelChannel <= (int)WindowLevelChannels.size())
      {
        if (idx == CurrentWindowLevelChannel)
          CurrentWindowLevelChannel = 0; // invalidate
        else if (CurrentWindowLevelChannel > idx)
          CurrentWindowLevelChannel--; // adjust
      }
      WindowLevelChannels.erase(it);
      if (WindowLevelChannelsResetWindowLevels[idx - 1])
        delete[] WindowLevelChannelsResetWindowLevels[idx - 1];
      WindowLevelChannelsResetWindowLevels.erase(
          WindowLevelChannelsResetWindowLevels.begin() + idx - 1);
      return true;
    }
  }
  return false;
}

int vtk2DSceneImageInteractorStyle::GetIndexOfWindowLevelChannel(
    vtkImageMapToColors *channel)
{
  std::vector<vtkImageMapToColors *>::iterator it;
  int idx = 0;
  for (it = WindowLevelChannels.begin(); it != WindowLevelChannels.end(); ++it)
  {
    idx++;
    if (*it == channel)
      return idx;
  }
  return 0;
}

void vtk2DSceneImageInteractorStyle::RemoveAllWindowLevelChannels()
{
  WindowLevelChannels.clear();
  CurrentWindowLevelChannel = 0;
  for (std::size_t i = 0; i < WindowLevelChannelsResetWindowLevels.size(); i++)
  {
    if (WindowLevelChannelsResetWindowLevels[i])
      delete[] WindowLevelChannelsResetWindowLevels[i];
  }
  WindowLevelChannelsResetWindowLevels.clear();
}

bool vtk2DSceneImageInteractorStyle::OverrideResetWindowLevel(int index,
    double *resetWindowLevel)
{
  if (!resetWindowLevel)
    return false;

  if (index >= 1 && index <= (int)WindowLevelChannels.size())
  {
    WindowLevelChannelsResetWindowLevels[index - 1][0] = resetWindowLevel[0];
    WindowLevelChannelsResetWindowLevels[index - 1][1] = resetWindowLevel[1];
    return true;
  }
  return false;
}

bool vtk2DSceneImageInteractorStyle::OverrideResetWindowLevelByMinMax(int index,
    double *resetMinMax)
{
  if (!resetMinMax)
    return false;
  double wl[2];
  wl[0] = resetMinMax[1] - resetMinMax[0]; // window
  wl[1] = resetMinMax[0] + wl[0] / 2.; // level
  return OverrideResetWindowLevel(index, wl);
}

void vtk2DSceneImageInteractorStyle::AlterInterpolationMode()
{
	int mode = GetInterpolationMode();
	if (mode != -1)
	{
		if (mode == VTK_RESLICE_NEAREST)
			mode = VTK_RESLICE_LINEAR;
		else if (mode == VTK_RESLICE_LINEAR)
			mode = VTK_RESLICE_CUBIC;
		else if (mode == VTK_RESLICE_CUBIC)
			mode = VTK_RESLICE_NEAREST;
		SetInterpolationMode(mode);
	}
}

void vtk2DSceneImageInteractorStyle::AlterWindowLevelChannel()
{
  int maxChannel = (int)WindowLevelChannels.size();
  int channel = CurrentWindowLevelChannel;
  channel++;
  if (channel < 0 || channel > maxChannel)
  {
    // out of range -> set to main channel:
    channel = 0;
  }
  SetCurrentWindowLevelChannel(channel);
}

void vtk2DSceneImageInteractorStyle::SetInterpolationMode(int mode)
{
	if (Magnifier && Magnifier->GetInterpolationMode() != mode)
	{
		Magnifier->SetInterpolationMode(mode);
		this->AfterChangingMagnifier(); // provide entry point
		this->InvokeEvent(vtk2DSceneImageInteractorStyle::InterpolationModeChanged, this);
	}
}

int vtk2DSceneImageInteractorStyle::GetInterpolationMode()
{
	if (Magnifier)
		return Magnifier->GetInterpolationMode();
	return -1;
}

void vtk2DSceneImageInteractorStyle::FlipImageAlongRowDirection(
    vtkMatrix4x4 *resliceMatrix)
{
	if (!this->Magnifier)
		return;
	if (!resliceMatrix)
		resliceMatrix = this->ResliceOrientation;

  vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
  double yax[3];
  yax[0] = resliceMatrix->GetElement(1, 0);
  yax[1] = resliceMatrix->GetElement(1, 1);
  yax[2] = resliceMatrix->GetElement(1, 2);
  t->RotateWXYZ(180, yax);
  double v[3];

  this->UpdateResliceOrientation(); // ensure

  v[0] = resliceMatrix->GetElement(0, 0);
  v[1] = resliceMatrix->GetElement(0, 1);
  v[2] = resliceMatrix->GetElement(0, 2);
  t->TransformVector(v, v);
  resliceMatrix->SetElement(0, 0, v[0]);
  resliceMatrix->SetElement(0, 1, v[1]);
  resliceMatrix->SetElement(0, 2, v[2]);

  v[0] = resliceMatrix->GetElement(1, 0);
  v[1] = resliceMatrix->GetElement(1, 1);
  v[2] = resliceMatrix->GetElement(1, 2);
  t->TransformVector(v, v);
  resliceMatrix->SetElement(1, 0, v[0]);
  resliceMatrix->SetElement(1, 1, v[1]);
  resliceMatrix->SetElement(1, 2, v[2]);

  v[0] = resliceMatrix->GetElement(2, 0);
  v[1] = resliceMatrix->GetElement(2, 1);
  v[2] = resliceMatrix->GetElement(2, 2);
  t->TransformVector(v, v);
  resliceMatrix->SetElement(2, 0, v[0]);
  resliceMatrix->SetElement(2, 1, v[1]);
  resliceMatrix->SetElement(2, 2, v[2]);

  vtkSmartPointer<vtkMatrix4x4> resliceAxes =
      vtkSmartPointer<vtkMatrix4x4>::New();
  // 'x'-axis:
  resliceAxes->SetElement(0, 0, resliceMatrix->GetElement(0, 0));
  resliceAxes->SetElement(1, 0, resliceMatrix->GetElement(0, 1));
  resliceAxes->SetElement(2, 0, resliceMatrix->GetElement(0, 2));
  // 'y'-axis:
  resliceAxes->SetElement(0, 1, resliceMatrix->GetElement(1, 0));
  resliceAxes->SetElement(1, 1, resliceMatrix->GetElement(1, 1));
  resliceAxes->SetElement(2, 1, resliceMatrix->GetElement(1, 2));
  // 'z'-axis:
  resliceAxes->SetElement(0, 2, resliceMatrix->GetElement(2, 0));
  resliceAxes->SetElement(1, 2, resliceMatrix->GetElement(2, 1));
  resliceAxes->SetElement(2, 2, resliceMatrix->GetElement(2, 2));
	this->Magnifier->SetResliceAxes(resliceAxes);
  double aorigin[3], aspacing[3];
  int aextent[6];
  this->ComputeAdjustedMaximumReslicingParameters(aorigin, aspacing, aextent);
	this->Magnifier->SetResliceAxesOrigin(aorigin);

  FitImageToRenderWindow();

  FlippedAlongRow = !FlippedAlongRow;
  this->InvokeEvent(vtk2DSceneImageInteractorStyle::FlippingModeChanged, this);
}

void vtk2DSceneImageInteractorStyle::FlipImageAlongColumnDirection(
    vtkMatrix4x4 *resliceMatrix)
{
	if (!this->Magnifier)
		return;
	if (!resliceMatrix)
		resliceMatrix = this->ResliceOrientation;

  vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
  double xax[3];
  xax[0] = resliceMatrix->GetElement(0, 0);
  xax[1] = resliceMatrix->GetElement(0, 1);
  xax[2] = resliceMatrix->GetElement(0, 2);
  t->RotateWXYZ(180, xax);
  double v[3];

  this->UpdateResliceOrientation(); // ensure
  v[0] = resliceMatrix->GetElement(0, 0);
  v[1] = resliceMatrix->GetElement(0, 1);
  v[2] = resliceMatrix->GetElement(0, 2);
  t->TransformVector(v, v);
  resliceMatrix->SetElement(0, 0, v[0]);
  resliceMatrix->SetElement(0, 1, v[1]);
  resliceMatrix->SetElement(0, 2, v[2]);

  v[0] = resliceMatrix->GetElement(1, 0);
  v[1] = resliceMatrix->GetElement(1, 1);
  v[2] = resliceMatrix->GetElement(1, 2);
  t->TransformVector(v, v);
  resliceMatrix->SetElement(1, 0, v[0]);
  resliceMatrix->SetElement(1, 1, v[1]);
  resliceMatrix->SetElement(1, 2, v[2]);

  v[0] = resliceMatrix->GetElement(2, 0);
  v[1] = resliceMatrix->GetElement(2, 1);
  v[2] = resliceMatrix->GetElement(2, 2);
  t->TransformVector(v, v);
  resliceMatrix->SetElement(2, 0, v[0]);
  resliceMatrix->SetElement(2, 1, v[1]);
  resliceMatrix->SetElement(2, 2, v[2]);

  vtkSmartPointer<vtkMatrix4x4> resliceAxes =
      vtkSmartPointer<vtkMatrix4x4>::New();
  // 'x'-axis:
  resliceAxes->SetElement(0, 0, resliceMatrix->GetElement(0, 0));
  resliceAxes->SetElement(1, 0, resliceMatrix->GetElement(0, 1));
  resliceAxes->SetElement(2, 0, resliceMatrix->GetElement(0, 2));
  // 'y'-axis:
  resliceAxes->SetElement(0, 1, resliceMatrix->GetElement(1, 0));
  resliceAxes->SetElement(1, 1, resliceMatrix->GetElement(1, 1));
  resliceAxes->SetElement(2, 1, resliceMatrix->GetElement(1, 2));
  // 'z'-axis:
  resliceAxes->SetElement(0, 2, resliceMatrix->GetElement(2, 0));
  resliceAxes->SetElement(1, 2, resliceMatrix->GetElement(2, 1));
  resliceAxes->SetElement(2, 2, resliceMatrix->GetElement(2, 2));
	this->Magnifier->SetResliceAxes(resliceAxes);
  double aorigin[3], aspacing[3];
  int aextent[6];
  this->ComputeAdjustedMaximumReslicingParameters(aorigin, aspacing, aextent);
	this->Magnifier->SetResliceAxesOrigin(aorigin);

  FitImageToRenderWindow();

  FlippedAlongColumn = !FlippedAlongColumn;
  this->InvokeEvent(vtk2DSceneImageInteractorStyle::FlippingModeChanged, this);
}

void vtk2DSceneImageInteractorStyle::UpdateResliceOrientation()
{
  // to be implemented in subclasses if required
}

void vtk2DSceneImageInteractorStyle::AfterChangingMagnifier()
{
  // to be implemented in subclasses if required
}

void vtk2DSceneImageInteractorStyle::StartZoom()
{
  this->vtkInteractorStyleImage::StartZoom();
  this->InvokeEvent(vtk2DSceneImageInteractorStyle::StartZooming, this);
}

void vtk2DSceneImageInteractorStyle::EndZoom()
{
  this->InvokeEvent(vtk2DSceneImageInteractorStyle::EndZooming, this);
  this->vtkInteractorStyleImage::EndZoom();
}

void vtk2DSceneImageInteractorStyle::StartPan()
{
  this->vtkInteractorStyleImage::StartPan();
  this->InvokeEvent(vtk2DSceneImageInteractorStyle::StartPanning, this);
}

void vtk2DSceneImageInteractorStyle::EndPan()
{
  this->InvokeEvent(vtk2DSceneImageInteractorStyle::EndPanning, this);
  this->vtkInteractorStyleImage::EndPan();
}

void vtk2DSceneImageInteractorStyle::StartCursor()
{
  if (this->State != VTKIS_NONE)
    return;
  this->StartState(VTKIS_CURSOR);
  this->InvokeEvent(vtk2DSceneImageInteractorStyle::StartCursoring, this);
}

void vtk2DSceneImageInteractorStyle::EndCursor()
{
  this->InvokeEvent(vtk2DSceneImageInteractorStyle::EndCursoring, this);
  if (this->State != VTKIS_CURSOR)
    return;
  this->StopState();
}

void vtk2DSceneImageInteractorStyle::ResetFlipStates()
{
	FlippedAlongColumn = false;
	FlippedAlongRow = false;
}

void vtk2DSceneImageInteractorStyle::MapInPlaneCoordinatesIntoWorldCoordinates(
		const int ic[2], double wc[3])
{
	// NOTE: Dummy implementation - should be overridden and implemented in
	// subclasses that support image orientation in higher dimensions!
	wc[0] = ic[0];
	wc[1] = ic[1];
	wc[2] = 0;
}

void vtk2DSceneImageInteractorStyle::MapWorldCoordinatesIntoInPlaneCoordinates(
		const double wc[3], int ic[2])
{
	// NOTE: Dummy implementation - should be overridden and implemented in
	// subclasses that support image orientation in higher dimensions!
	ic[0] = wc[0];
	ic[1] = wc[1];
}
