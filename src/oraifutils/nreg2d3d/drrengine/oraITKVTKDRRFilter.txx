#ifndef ORAITKVTKDRRFILTER_TXX_
#define ORAITKVTKDRRFILTER_TXX_

#include "oraITKVTKDRRFilter.h"

#include <itkVTKImageImport.h>
#include <itkVTKImageExport.h>
#include <itksys/SystemTools.hxx>

#include <vtkImageExport.h>
#include <vtkImageImport.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkInteractorObserver.h>
#include <vtkCallbackCommand.h>

namespace ora
{

template<class TInputPixelType, class TOutputPixelType>
ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::ITKVTKDRRFilter()
{
  this->m_WeakMTimeBehavior = false; // consider transformations' MTimes!
  this->m_SourceFocalSpotPosition.Fill(0);
  this->m_DRRPlaneOrigin.Fill(0);
  this->m_DRRPlaneOrientation.SetIdentity();
  this->m_DRRPlaneOrientationRotated.SetIdentity();
  this->m_DRRSize.Fill(0);
  this->m_DRRSpacing.Fill(0);
  this->m_GeometryIsValid = false;
  this->m_Transform = NULL;
  this->m_IntensityTF = TransferFunctionPointer::New();
  this->m_IntensityTF->AddRGBPoint(0, 0, 0, 0); // some default
  this->m_IntensityTF->AddRGBPoint(1, 1, 1, 1);
  this->m_VTKTransform = VTKTransformPointer::New();
  this->m_OrientationVTKTransform = VTKTransformPointer::New();
  this->m_Camera = NULL;
  this->m_SampleDistance = 1.0;
  this->m_TransformConnector = TransformConnectorType::New();
  this->m_OverrideVideoMemSizeMB = 0;
  this->m_OverrideMaxVideoMemFraction = 0.0;

  this->m_Interactive = false; // default visual style
  this->m_RescaleSlope = 1.0;
  this->m_RescaleIntercept = 0.0;
  SetContextTitle("");
  this->m_ContextPosition[0] = 0;
  this->m_ContextPosition[1] = 0;
  this->m_CopyDRRToImageOutput = true;

  this->m_VTKOutputImages.clear();
  this->m_VTKDRRMasks.clear();
  this->m_VTKToITKMaskMap.clear();
  this->m_NumberOfIndependentOutputs = 0;
  this->SetNumberOfIndependentOutputs(1);
  this->m_CurrentDRROutputIndex = 0;

  this->m_ExternalRenderWindow = NULL;

  this->m_FireStartEndEvents = false;
  this->m_EventObject = vtkObject::New();

  this->m_DoNotTryToResizeNonInteractiveWindows = false;

  // Must be invoked externally: this->BuildRenderPipeline();
}

template<class TInputPixelType, class TOutputPixelType>
ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::~ITKVTKDRRFilter()
{
  this->DestroyRenderPipeline();

  this->m_Transform = NULL;
  std::map<vtkImageData*, MaskImagePointer>::iterator it =
      this->m_VTKToITKMaskMap.begin();
  while (it != this->m_VTKToITKMaskMap.end())
  {
    (*it).second = NULL;
    ++it;
  }
  this->m_VTKToITKMaskMap.clear();
  for (std::size_t i = 0; i < this->m_VTKOutputImages.size(); i++)
  {
    this->m_VTKOutputImages[i] = NULL;
    this->m_VTKDRRMasks[i] = NULL;
  }
  this->m_VTKOutputImages.clear();
  this->m_VTKDRRMasks.clear();
  this->m_IntensityTF = NULL;
  this->m_VTKTransform = NULL;
  this->m_OrientationVTKTransform = NULL;
  this->m_TransformConnector = NULL;

  this->m_EventObject->Delete();
  this->m_EventObject = NULL;
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Weak MTime Behavior: " << this->m_WeakMTimeBehavior << "\n";
  os << indent << "Transform: " << this->m_Transform.GetPointer() << std::endl;
  os << indent << "VTK Transform: " << this->m_VTKTransform.GetPointer()
      << std::endl;
  os << indent << "Orientation VTK Transform: "
      << this->m_OrientationVTKTransform.GetPointer() << std::endl;
  os << indent << "Transform Connector: "
      << this->m_TransformConnector.GetPointer() << std::endl;
  os << indent << "Focal Spot Position: " << this->m_SourceFocalSpotPosition
      << std::endl;
  os << indent << "DRR Plane Origin: " << this->m_DRRPlaneOrigin << std::endl;
  os << indent << "DRR Plane Orientation: " << std::endl
      << this->m_DRRPlaneOrientation;
  os << indent << "DRR Plane Orientation (rotated): " << std::endl
      << this->m_DRRPlaneOrientationRotated;
  os << indent << "DRR Size: " << this->m_DRRSize << std::endl;
  os << indent << "DRR Spacing: " << this->m_DRRSpacing << std::endl;
  os << indent << "(Geometry Valid: " << this->m_GeometryIsValid << ")"
      << std::endl;
  os << indent << "Current DRR Output Index: " << this->m_CurrentDRROutputIndex
      << std::endl;
  os << indent << "VTK Output Images (n=" << this->m_NumberOfIndependentOutputs
      << "):\n";
  for (std::size_t x = 0; x < this->m_VTKOutputImages.size(); x++)
    os << indent << " " << x << ": " << this->m_VTKOutputImages[x].GetPointer()
        << std::endl;
  os << indent << "VTK DRR Masks (n=" << this->m_NumberOfIndependentOutputs
      << "):\n";
  for (std::size_t x = 0; x < this->m_VTKDRRMasks.size(); x++)
    os << indent << " " << x << ": " << this->m_VTKDRRMasks[x].GetPointer()
        << std::endl;
  os << indent << "Render Window: " << this->m_RenderWindow.GetPointer()
      << std::endl;
  os << indent << "Renderer: " << this->m_Renderer.GetPointer() << std::endl;
  os << indent << "Camera: " << this->m_Camera.GetPointer() << std::endl;
  os << indent << "GPU Ray Caster: " << this->m_GPURayCaster.GetPointer()
      << std::endl;
  os << indent << "Intensity Transfer Function: "
      << this->m_IntensityTF.GetPointer() << std::endl;
  os << indent << "Override Video Memory Size (MB): "
      << this->m_OverrideVideoMemSizeMB << std::endl;
  os << indent << "Override Maximum Fraction of Video Memory Size: "
      << this->m_OverrideMaxVideoMemFraction << std::endl;
  os << indent << "Interactive (visual): " << this->m_Interactive << std::endl;
  os << indent << "Rescale Slope: " << this->m_RescaleSlope << "\n";
  os << indent << "Rescale Intercept: " << this->m_RescaleIntercept << "\n";
  os << indent << "Context Title (visual): " << this->m_ContextTitle
      << std::endl;
  os << indent << "Context Position (visual): " << this->m_ContextPosition[0]
      << "," << this->m_ContextPosition[1] << std::endl;
  os << indent << "Copy DRR To Image Output (visual): "
      << this->m_CopyDRRToImageOutput << std::endl;
  os << indent << "External Render Window: "
      << this->m_ExternalRenderWindow.GetPointer() << std::endl;
  os << indent << "Event Object: " << this->m_EventObject << std::endl;
  os << indent << "Fire Start/End events: " << this->m_FireStartEndEvents <<
      std::endl;
  os << indent << "DoNotTryToResizeNonInteractiveWindows: " <<
      this->m_DoNotTryToResizeNonInteractiveWindows << std::endl;
}

template<class TInputPixelType, class TOutputPixelType>
int ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::GetEffectiveMaximumGPUMemory()
{
  float memsize = 0;

  if (this->m_GPURayCaster)
  {
    memsize = static_cast<float> (this->m_GPURayCaster->GetMaxMemoryInBytes())
        * this->m_GPURayCaster->GetMaxMemoryFraction();
    memsize /= (1024 * 1024);
  }

  return static_cast<int> (memsize);
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::BuildRenderPipeline()
{
  DestroyRenderPipeline();

  // render window and renderer
  if (!this->m_ExternalRenderWindow) // build our own window
  {
    this->m_RenderWindow = NULL;
    if (!this->m_Interactive) // off-screen
    {
      this->m_RenderWindow = InternalRenderWindowPointer::New();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
      this->m_RenderWindow->SetOffScreenRendering(false); // pseudo-off-screen
#else
      this->m_RenderWindow->SetOffScreenRendering(true);
#endif
    }
    else // on-screen
    {
      this->m_RenderWindow = RenderWindowPointer::New();
    }
    this->m_Renderer = RendererPointer::New();
    this->m_RenderWindow->AddRenderer(this->m_Renderer);
    this->m_RenderWindow->SetSize(1, 1); // preliminary size
    this->m_RenderWindow->SetPosition(this->m_ContextPosition[0],
        this->m_ContextPosition[1]);
    this->m_RenderWindow->SetWindowName(this->m_ContextTitle.c_str());
    // initially, render (expected from main thread):
    if (m_FireStartEndEvents)
      m_EventObject->InvokeEvent(vtkCommand::StartEvent);

    this->m_RenderWindow->MakeCurrent();
    this->m_RenderWindow->Render();
      // unbind context again:
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
    if (!this->m_Interactive) // pseudo-off-screen
    {
      // flags: 0 ... SW_HIDE (seems to cause rendering problems later)
      //        6 ... SW_MINIMIZE (seems to work)
      ShowWindow((HWND__*)this->m_RenderWindow->GetGenericWindowId(), 6);
    }
    wglMakeCurrent((HDC__*)this->m_RenderWindow->GetGenericDisplayId(), NULL);
#else
    glXMakeCurrent((Display*)this->m_RenderWindow->GetGenericDisplayId(), None,
        NULL);
#endif
    if (m_FireStartEndEvents)
      m_EventObject->InvokeEvent(vtkCommand::EndEvent);
  }
  else
  {
    // EXPECTED: a render window with a renderer and a render window interactor
    this->m_RenderWindow = NULL;
    this->m_RenderWindow = this->m_ExternalRenderWindow;
    // first renderer is used as internal renderer!
    this->m_Renderer = this->m_RenderWindow->GetRenderers()->GetFirstRenderer();
  }

  // volume mapper
  this->m_GPURayCaster = GPURayCasterPointer::New();
  this->m_GPURayCaster->SetUseMappingIDs(true); // prevent multiple rendering of same DRR
  this->m_GPURayCaster->SetVerticalFlip(false); // we are in WCS, it's OK!
  this->m_GPURayCaster->SetInput(this->m_VTKInput);
  this->m_GPURayCaster->SetRenderWindow(this->m_RenderWindow);
  this->m_GPURayCaster->SetSampleDistance(this->m_SampleDistance);
  this->m_GPURayCaster->SetLastDRR(NULL);
  if (this->m_OverrideVideoMemSizeMB > 0)
    this->m_GPURayCaster->SetMaxMemoryInBytes(this->m_OverrideVideoMemSizeMB);
  if (this->m_OverrideMaxVideoMemFraction >= 0.1
      && this->m_OverrideMaxVideoMemFraction <= 1.0)
    this->m_GPURayCaster->SetMaxMemoryFraction(
        this->m_OverrideMaxVideoMemFraction);

  // volume property
  vtkSmartPointer<vtkVolumeProperty> volprop = vtkSmartPointer<
      vtkVolumeProperty>::New();
  volprop->SetIndependentComponents(true); // basically true for DRR computation
  volprop->SetColor(this->m_IntensityTF); // intensity transfer function
  this->m_GPURayCaster->SetIntensityTF(this->m_IntensityTF);
  this->m_GPURayCaster->SetIntensityTFLinearInterpolation(true);
  vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
  volume->SetMapper(this->m_GPURayCaster);
  // NOTE: this is a new extension!
  // the image orientation (which is normally not handled by VTK) is set
  // via a separate user transform!
  this->m_GPURayCaster->SetOrientationTransform(this->m_OrientationVTKTransform);
  // this transform is the transformation applied to the volume excluding
  // image orientation in space:
  this->m_GPURayCaster->SetTransform(this->m_VTKTransform);

  // add to pipeline
  this->m_Renderer->AddVolume(volume);

  // camera
  this->m_Camera = this->m_Renderer->GetActiveCamera();
  this->m_Camera->SetPosition(0, 0, 5); // some initial settings
  this->m_Camera->SetFocalPoint(0, 0, 0);
  this->m_Camera->SetViewUp(0, 1, 0);
  this->m_Camera->SetClippingRange(0.001, 1.0);
  this->m_Camera->SetParallelProjection(false);
  this->m_GPURayCaster->SetPlaneViewCamera(this->m_Camera);

  // interactivity
  this->SetInteractive(this->m_Interactive);
  this->SetRescaleSlope(this->m_RescaleSlope);
  this->SetRescaleIntercept(this->m_RescaleIntercept);
  this->SetCopyDRRToImageOutput(this->m_CopyDRRToImageOutput);
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetOverrideVideoMemSizeMB(
    int videoMemSize)
{
  if (this->m_OverrideVideoMemSizeMB != videoMemSize)
  {
    this->m_OverrideVideoMemSizeMB = videoMemSize;
    if (this->m_OverrideVideoMemSizeMB > 0)
      this->m_GPURayCaster->SetMaxMemoryInBytes(this->m_OverrideVideoMemSizeMB
          * 1024 * 1024);
    this->Modified();
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetOverrideMaxVideoMemFraction(
    float maxFraction)
{
  if (this->m_OverrideMaxVideoMemFraction != maxFraction)
  {
    this->m_OverrideMaxVideoMemFraction = maxFraction;
    if (this->m_OverrideMaxVideoMemFraction >= 0.1
        && this->m_OverrideMaxVideoMemFraction <= 1.0)
      this->m_GPURayCaster->SetMaxMemoryFraction(
          this->m_OverrideMaxVideoMemFraction);
    else
      this->m_GPURayCaster->SetMaxMemoryFraction(0.75); // default
    this->Modified();
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetSampleDistance(
    float distance)
{
  if (this->m_SampleDistance == distance || !this->m_GPURayCaster)
    return;

  this->m_SampleDistance = distance;
  this->m_GPURayCaster->SetSampleDistance(this->m_SampleDistance);

  this->Modified();
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetTransform(
    TransformType *transform)
{
  if (transform == this->m_Transform)
    return;

  this->m_Transform = transform;
  this->m_TransformConnector->SetVTKTransform(this->m_VTKTransform);
  this->m_TransformConnector->SetITKTransform(this->m_Transform);

  this->Modified();
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::DestroyRenderPipeline()
{
  if (this->m_RenderWindow != this->m_ExternalRenderWindow)
  {
    if (this->m_Renderer)
      this->m_Renderer->RemoveAllViewProps();
    if (this->m_RenderWindow)
      this->m_RenderWindow->RemoveRenderer(this->m_Renderer);
  }
  this->m_GPURayCaster = NULL;
  this->m_Renderer = NULL;
  this->m_RenderWindow = NULL;
  this->m_Camera = NULL;
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetIntensityTransferFunction(
    TransferFunctionSpecificationType tf)
{
  if ((tf.Size() % 2) != 0)
    return;

  this->m_IntensityTF->RemoveAllPoints();
  for (unsigned int i = 0; i < tf.Size(); i += 2)
    //                               volume intensity, output intensity
    this->m_IntensityTF->AddRGBPoint(tf[i], tf[i + 1], 0, 0);
}

template<class TInputPixelType, class TOutputPixelType>
typename ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::TransferFunctionSpecificationType ITKVTKDRRFilter<
    TInputPixelType, TOutputPixelType>::GetIntensityTransferFunction()
{
  TransferFunctionSpecificationType tf(this->m_IntensityTF->GetSize());

  double x[6];
  unsigned int j = 0;
  for (int i = 0; i < this->m_IntensityTF->GetSize(); i++)
  {
    this->m_IntensityTF->GetNodeValue(i, x);
    tf[j++] = x[0]; // volume intensity (location)
    tf[j++] = x[1]; // output intensity (red channel)
  }

  return tf;
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetInput(
    InputImagePointer input)
{
  if (input == this->GetInput())
    return;

  this->Superclass::SetInput(NULL);
  this->m_VTKInput = NULL;
  if (!input)
  {
    this->UpdateOrientationTransformation();
    return;
  }

  // connect a new VTK image to existing ITK image:
  this->Superclass::SetInput(input);

  this->UpdateOrientationTransformation(); // update based on ITK-info!
  this->m_VTKInput = this->ConnectVTKImageToITKInputImage(this->GetInput());

  if (this->m_GPURayCaster)
  {
    this->m_GPURayCaster->ReleaseGPUTextures(); // simple release all textures
    this->m_GPURayCaster->SetInput(this->m_VTKInput);
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::UpdateOrientationTransformation()
{
  InputImagePointer input = const_cast<InputImageType*> (this->GetInput());
  if (input)
  {
    typedef typename InputImageType::PointType PointType;
    typedef typename InputImageType::DirectionType DirectionType;
    PointType io = input->GetOrigin();
    DirectionType id = input->GetDirection();
    // convert rotation to 4x4 homogeneous matrix
    vtkSmartPointer<vtkMatrix4x4> rotMatrix =
        vtkSmartPointer<vtkMatrix4x4>::New();
    rotMatrix->Identity();
    for (unsigned int i = 0; i < 3; i++) // override the inner 3x3-matrix
      for (unsigned int j = 0; j < 3; j++)
        rotMatrix->SetElement(i, j, id[j][i]); // column vs. row based!

    // -> adapt the internal VTK transform that models the image orientation
    this->m_OrientationVTKTransform->Identity();
    this->m_OrientationVTKTransform->PostMultiply();
    // - orientation
    this->m_OrientationVTKTransform->Translate(-io[0], -io[1], -io[2]);
    this->m_OrientationVTKTransform->Concatenate(rotMatrix);
    this->m_OrientationVTKTransform->Translate(io[0], io[1], io[2]);
  }
  else
  {
    this->m_OrientationVTKTransform->Identity(); // default
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetVTKInput(
    VTKInputImagePointer input)
{
  if (input == this->m_VTKInput)
    return;

  this->Superclass::SetInput(NULL);
  this->m_VTKInput = NULL;
  if (!input)
    return;

  // connect a new ITK image to existing VTK image:
  this->m_VTKInput = input;
  this->Superclass::SetInput(this->ConnectInputITKImageToVTKImage(
      this->m_VTKInput));

  if (this->m_GPURayCaster)
    this->m_GPURayCaster->SetInput(this->m_VTKInput);
}

template<class TInputPixelType, class TOutputPixelType>
unsigned long ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::GetMTime(void) const
{
  unsigned long latest = this->Superclass::GetMTime();

  if (!this->m_WeakMTimeBehavior)
  {
    if (this->m_Transform)
    {
      if (latest < this->m_Transform->GetMTime())
        latest = this->m_Transform->GetMTime();
    }
    if (this->m_OrientationVTKTransform)
    {
      if (latest < this->m_OrientationVTKTransform->GetMTime())
        latest = this->m_OrientationVTKTransform->GetMTime();
    }
  }

  return latest;
}

template<class TInputPixelType, class TOutputPixelType>
bool ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::IsOrthogonalMatrix(
    const MatrixType &matrix) const
{
  bool orthogonal = true;

  // M * M == I required
  typename MatrixType::InternalMatrixType test =
      matrix.GetVnlMatrix() * matrix.GetTranspose();
  if(!test.is_identity(1e-3)) // we have to be very tolerant ... (had issues)
  {
    orthogonal = false;
  }

  if (!orthogonal)
  {
    itkDebugMacro(<< "The specified matrix is not orthogonal:\n" << matrix)
  }

  return orthogonal;
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetDRRPlaneOrientation(
    const MatrixType matrix)
{
  if (this->m_DRRPlaneOrientation != matrix)
  {
    if (this->IsOrthogonalMatrix(matrix)) // require orthogonal matrix
    {
      this->m_DRRPlaneOrientation = matrix;
      this->m_DRRPlaneOrientationRotated[0][0] = matrix[0][0];
      this->m_DRRPlaneOrientationRotated[1][0] = matrix[0][1];
      this->m_DRRPlaneOrientationRotated[2][0] = matrix[0][2];
      this->m_DRRPlaneOrientationRotated[0][1] = matrix[1][0];
      this->m_DRRPlaneOrientationRotated[1][1] = matrix[1][1];
      this->m_DRRPlaneOrientationRotated[2][1] = matrix[1][2];
      this->m_DRRPlaneOrientationRotated[0][2] = matrix[2][0];
      this->m_DRRPlaneOrientationRotated[1][2] = matrix[2][1];
      this->m_DRRPlaneOrientationRotated[2][2] = matrix[2][2];
      this->ApplyGeometrySettings();
      this->Modified();
    }
  }
}

template<class TInputPixelType, class TOutputPixelType>
bool ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::ApplyGeometrySettings()
{
  // check validity of geometry setup:
  this->m_GeometryIsValid = true;
  unsigned int i, j;
  for (i = 0; i < 2; i++)
  {
    if (this->m_DRRSize[i] <= 0)
      this->m_GeometryIsValid = false;
    if (this->m_DRRSpacing[i] <= 0.0)
      this->m_GeometryIsValid = false;
  }

  // must have orthogonal orientation matrix:
  if (!IsOrthogonalMatrix(this->m_DRRPlaneOrientation))
    this->m_GeometryIsValid = false;

  // focal spot must be above DRR plane (in dir. of plane normal):
  if (this->m_GeometryIsValid)
  {
    double n[3]; // plane normal
    vtkMath::Cross(this->m_DRRPlaneOrientation[0],
        this->m_DRRPlaneOrientation[1], n); // plane axes are normalized
    double fs[3]; // spot
    for (i = 0; i < 3; i++)
      fs[i] = m_SourceFocalSpotPosition[i];
    double fscoord = vtkMath::Dot(n, fs); // projection of fs
    double po[3];
    for (i = 0; i < 3; i++)
      po[i] = m_DRRPlaneOrigin[i];
    double pocoord = vtkMath::Dot(n, po); // projection of po
    if (fabs(fscoord - pocoord) < 1e-6)
      this->m_GeometryIsValid = false;
  }

  if (!this->m_GeometryIsValid) // early exit if geometry is invalid
    return this->m_GeometryIsValid;

  // check whether the internal VTK output image (DRR in background) must
  // be reallocated (is worth being checked because it needs considerable time):
  bool needNewVTKOutputImage = false;
  VTKOutputImagePointer voutimage =
      this->m_VTKOutputImages[this->m_CurrentDRROutputIndex];
  if (!voutimage)
    needNewVTKOutputImage = true;
  if (!needNewVTKOutputImage)
  {
    int *dims = voutimage->GetDimensions();
    if (dims[0] != (int) this->m_DRRSize[0] || dims[1]
        != (int) this->m_DRRSize[1] || dims[2] != 1)
      needNewVTKOutputImage = true;

    double *spac = voutimage->GetSpacing();
    if (spac[0] != this->m_DRRSpacing[0] || spac[1] != this->m_DRRSpacing[1]
        || spac[2] != 1)
      needNewVTKOutputImage = true;
  }

  // create new VTK output image if required
  if (needNewVTKOutputImage)
  {
    this->m_VTKOutputImages[this->m_CurrentDRROutputIndex] = NULL;
    this->m_VTKOutputImages[this->m_CurrentDRROutputIndex]
        = VTKOutputImagePointer::New();
    voutimage = this->m_VTKOutputImages[this->m_CurrentDRROutputIndex];
    voutimage->SetDimensions(this->m_DRRSize[0], this->m_DRRSize[1], 1);
    voutimage->SetOrigin(this->m_DRRPlaneOrigin[0], this->m_DRRPlaneOrigin[1],
        this->m_DRRPlaneOrigin[2]);
    voutimage->SetSpacing(this->m_DRRSpacing[0], this->m_DRRSpacing[1], 1);
    voutimage->SetNumberOfScalarComponents(1);

    // choose scalar type according to output pixel type:
    if (typeid(OutputImagePixelType) == typeid(float))
      voutimage->SetScalarTypeToFloat();
    else if (typeid(OutputImagePixelType) == typeid(double))
      voutimage->SetScalarTypeToDouble();
    else if (typeid(OutputImagePixelType) == typeid(int))
      voutimage->SetScalarTypeToInt();
    else if (typeid(OutputImagePixelType) == typeid(unsigned int))
      voutimage->SetScalarTypeToUnsignedInt();
    else if (typeid(OutputImagePixelType) == typeid(long))
      voutimage->SetScalarTypeToLong();
    else if (typeid(OutputImagePixelType) == typeid(unsigned long))
      voutimage->SetScalarTypeToUnsignedLong();
    else if (typeid(OutputImagePixelType) == typeid(short))
      voutimage->SetScalarTypeToShort();
    else if (typeid(OutputImagePixelType) == typeid(unsigned short))
      voutimage->SetScalarTypeToUnsignedShort();
    else if (typeid(OutputImagePixelType) == typeid(unsigned char))
      voutimage->SetScalarTypeToUnsignedChar();
    else if (typeid(OutputImagePixelType) == typeid(char))
      voutimage->SetScalarTypeToChar();
    else // unknown scalar type
    {
      this->m_VTKOutputImages[this->m_CurrentDRROutputIndex] = NULL;
      voutimage = NULL;
      this->m_GeometryIsValid = false;
      return this->m_GeometryIsValid;
    }

    // reserve the scalar array
    voutimage->AllocateScalars();
  }

  // be sure that origin is up to date (this is usually fast)
  voutimage->SetOrigin(this->m_DRRPlaneOrigin[0], this->m_DRRPlaneOrigin[1],
      this->m_DRRPlaneOrigin[2]);

  // set up the GPU ray caster:
  if (!this->m_GPURayCaster) // need it
  {
    this->m_GeometryIsValid = false;
    return this->m_GeometryIsValid;
  }

  // set up the DRR plane and projection settings
  double fs[3];
  for (i = 0; i < 3; i++)
    fs[i] = this->m_SourceFocalSpotPosition[i];
  double sz[3];
  for (i = 0; i < 3; i++)
    sz[i] = this->m_DRRSize[i];
  double sp[3];
  for (i = 0; i < 3; i++)
    sp[i] = this->m_DRRSpacing[i];
  double po[3];
  for (i = 0; i < 3; i++)
    po[i] = this->m_DRRPlaneOrigin[i];
  double orient[9];
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      orient[i * 3 + j] = this->m_DRRPlaneOrientation[i][j];
  this->m_GeometryIsValid = this->m_GPURayCaster->SetRayCastingGeometryProps(
      fs, sz, sp, po, orient);

  this->m_GPURayCaster->SetLastDRR(
      this->m_VTKOutputImages[this->m_CurrentDRROutputIndex]);

  // NOTE: the DRR mask is only set if the size of the mask matches the DRR
  // plane size (in pixels)!
  bool sizeMatch = false;
  if (this->m_VTKDRRMasks[this->m_CurrentDRROutputIndex])
  {
    int *mdims =
        this->m_VTKDRRMasks[this->m_CurrentDRROutputIndex]-> GetDimensions();
    if ((unsigned int) mdims[0] == this->m_DRRSize[0]
        && (unsigned int) mdims[1] == this->m_DRRSize[1])
      sizeMatch = true;
  }
  if (sizeMatch)
  {
    this->m_GPURayCaster->SetDRRMask(
        this->m_VTKDRRMasks[this->m_CurrentDRROutputIndex]);
  }
  else
  {
    this->m_GPURayCaster->SetDRRMask(NULL);
  }

  int wsize[2];
  if (this->m_Interactive)
  {
    wsize[0] = this->m_DRRSize[0];
    wsize[1] = this->m_DRRSize[1];
  }
  else
  {
    wsize[0] = 1;
    wsize[1] = 1;
  }
  int *csize = this->m_RenderWindow->GetSize();
  if (!m_DoNotTryToResizeNonInteractiveWindows &&
      !m_Interactive &&
      (csize[0] != wsize[0] || csize[1] != wsize[1]))
  {
    if (m_FireStartEndEvents)
      m_EventObject->InvokeEvent(vtkCommand::StartEvent);
#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
    XLockDisplay((Display*)this->m_RenderWindow->GetGenericDisplayId());
#endif
    this->m_RenderWindow->MakeCurrent();
    this->m_RenderWindow->SetSize(wsize[0], wsize[1]);
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
    wglMakeCurrent((HDC__*)this->m_RenderWindow->GetGenericDisplayId(), NULL);
#else
    glXMakeCurrent((Display*)this->m_RenderWindow->GetGenericDisplayId(), None,
        NULL);
    XUnlockDisplay((Display*)this->m_RenderWindow->GetGenericDisplayId());
#endif
    if (m_FireStartEndEvents)
      m_EventObject->InvokeEvent(vtkCommand::EndEvent);
  }

  return this->m_GeometryIsValid;
}

template<class TInputPixelType, class TOutputPixelType>
bool ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::IsGeometryValid()
{
  return this->m_GeometryIsValid;
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetNumberOfIndependentOutputs(
    int numOutputs)
{
  if (numOutputs <= 0)
    return;

  if (numOutputs != this->GetNumberOfIndependentOutputs())
  {
    bool resizeOutputsBefore = (numOutputs
        > this->GetNumberOfIndependentOutputs());

    if (resizeOutputsBefore)
    {
      this->SetNumberOfOutputs(numOutputs);
      this->SetNumberOfRequiredOutputs(numOutputs);
    }

    // adjust the size of VTK output images (preserve existing images):
    while ((int) this->m_VTKOutputImages.size() > numOutputs)
    {
      // remove unnecessary filter output:
      int idx = static_cast<int> (this->m_VTKOutputImages.size()) - 1;
      OutputImagePointer output = this->GetOutput(idx);
      this->RemoveOutput(output);
      this->m_VTKOutputImages[idx] = NULL;
      this->m_VTKOutputImages.pop_back();
      this->m_VTKDRRMasks[idx] = NULL;
      this->m_VTKDRRMasks.pop_back();
    }
    while ((int) this->m_VTKOutputImages.size() < numOutputs)
    {
      this->m_VTKOutputImages.push_back(NULL);
      this->m_VTKDRRMasks.push_back(NULL); // no mask by default
      // add required filter output:
      int idx = static_cast<int> (this->m_VTKOutputImages.size()) - 1;
      OutputImagePointer output =
          static_cast<OutputImageType *> (this->MakeOutput(idx).GetPointer());
      this->SetNthOutput(idx, output.GetPointer());
    }

    if (!resizeOutputsBefore)
    {
      this->SetNumberOfOutputs(numOutputs);
      this->SetNumberOfRequiredOutputs(numOutputs);
    }

    this->m_NumberOfIndependentOutputs = numOutputs;
    if (this->m_CurrentDRROutputIndex >= numOutputs)
      this->SetCurrentDRROutputIndex(0);

    this->ApplyGeometrySettings(); // be sure that image is OK
    this->Modified();
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetCurrentDRROutputIndex(
    int currIndex)
{
  if (currIndex < 0 || currIndex >= this->m_NumberOfIndependentOutputs)
    return;

  if (this->m_CurrentDRROutputIndex != currIndex)
  {
    this->m_CurrentDRROutputIndex = currIndex;
    this->ApplyGeometrySettings(); // be sure that image is OK
    this->Modified();
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::GenerateOutputInformation()
{
  if (this->m_VTKOutputImages.size() <= 0 || !this->m_GeometryIsValid)
    return;

  // simply connect the pipelines:
  this->GraftNthOutput(this->m_CurrentDRROutputIndex,
      this->ConnectOutputITKImageToVTKImage(
          this->m_VTKOutputImages[this->m_CurrentDRROutputIndex]));

  // regions size and index, spacing and origin are implicitly set via pipeline
  // callbacks; HOWEVER, VTK images do not contain any orientation information;
  // therefore, we have to set this attribute manually:
  OutputImagePointer output = this->GetOutput(this->m_CurrentDRROutputIndex);
  output->SetDirection(this->m_DRRPlaneOrientationRotated);
  // obviously the origin is not taken over automatically
  output->SetOrigin(this->m_DRRPlaneOrigin);

  return;
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::EnlargeOutputRequestedRegion(
    itk::DataObject *data)
{
  this->Superclass::EnlargeOutputRequestedRegion(data);
  data->SetRequestedRegionToLargestPossibleRegion(); // largest possible!

  return;
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::GenerateOutputRequestedRegion(
    itk::DataObject *data)
{
  for (int x = 0; x < this->m_NumberOfIndependentOutputs; x++)
  {
    OutputImagePointer output = this->GetOutput(x);
    if (output)
    {
      output->SetRequestedRegionToLargestPossibleRegion();
      output->SetBufferedRegion(output->GetLargestPossibleRegion());
    }
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::GenerateInputRequestedRegion()
{
  this->Superclass::GenerateInputRequestedRegion();

  // get pointer to the input
  if (!this->GetInput())
    return;
  InputImagePointer inputPtr = const_cast<InputImageType*> (this->GetInput());

  // request complete input image
  inputPtr->SetRequestedRegionToLargestPossibleRegion();

  return;
}

template<class TInputPixelType, class TOutputPixelType>
typename ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::VTKInputImagePointer ITKVTKDRRFilter<
    TInputPixelType, TOutputPixelType>::ConnectVTKImageToITKInputImage(
    InputImageConstPointer itkImage)
{
  if (!itkImage)
    return NULL;

  typedef itk::VTKImageExport<InputImageType> ITKExporterType;
  typedef typename ITKExporterType::Pointer ITKExporterPointer;
  typedef vtkSmartPointer<vtkImageImport> VTKImporterPointer;

  // connect VTK-pipeline to ITK-pipeline:
  ITKExporterPointer itkExporter = ITKExporterType::New();

  itkExporter->SetInput(itkImage); // export the ITK image object

  VTKImporterPointer vtkImporter = VTKImporterPointer::New();

  // most important: connect the callbacks of both pipelines
  vtkImporter->SetUpdateInformationCallback(
      itkExporter->GetUpdateInformationCallback());
  vtkImporter->SetPipelineModifiedCallback(
      itkExporter->GetPipelineModifiedCallback());
  vtkImporter->SetWholeExtentCallback(itkExporter->GetWholeExtentCallback());
  vtkImporter->SetSpacingCallback(itkExporter->GetSpacingCallback());
  vtkImporter->SetOriginCallback(itkExporter->GetOriginCallback());
  vtkImporter->SetScalarTypeCallback(itkExporter->GetScalarTypeCallback());
  vtkImporter->SetNumberOfComponentsCallback(
      itkExporter->GetNumberOfComponentsCallback());
  vtkImporter->SetPropagateUpdateExtentCallback(
      itkExporter->GetPropagateUpdateExtentCallback());
  vtkImporter->SetUpdateDataCallback(itkExporter->GetUpdateDataCallback());
  vtkImporter->SetDataExtentCallback(itkExporter->GetDataExtentCallback());
  vtkImporter->SetBufferPointerCallback(itkExporter->GetBufferPointerCallback());
  vtkImporter->SetCallbackUserData(itkExporter->GetCallbackUserData());

  // import the VTK image object
  vtkImporter->Update(); // update immediately

  VTKInputImagePointer vtkImage = VTKInputImagePointer::New();
  vtkImage->ShallowCopy(vtkImporter->GetOutput());

  return vtkImage;
}

template<class TInputPixelType, class TOutputPixelType>
typename ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::VTKInputImagePointer ITKVTKDRRFilter<
    TInputPixelType, TOutputPixelType>::ConnectVTKImageToITKMaskImage(
    MaskImagePointer itkImage)
{
  if (!itkImage)
    return NULL;

  typedef itk::VTKImageExport<MaskImageType> ITKExporterType;
  typedef typename ITKExporterType::Pointer ITKExporterPointer;
  typedef vtkSmartPointer<vtkImageImport> VTKImporterPointer;

  // connect VTK-pipeline to ITK-pipeline:
  ITKExporterPointer itkExporter = ITKExporterType::New();

  itkExporter->SetInput(itkImage); // export the ITK image object

  VTKImporterPointer vtkImporter = VTKImporterPointer::New();

  // most important: connect the callbacks of both pipelines
  vtkImporter->SetUpdateInformationCallback(
      itkExporter->GetUpdateInformationCallback());
  vtkImporter->SetPipelineModifiedCallback(
      itkExporter->GetPipelineModifiedCallback());
  vtkImporter->SetWholeExtentCallback(itkExporter->GetWholeExtentCallback());
  vtkImporter->SetSpacingCallback(itkExporter->GetSpacingCallback());
  vtkImporter->SetOriginCallback(itkExporter->GetOriginCallback());
  vtkImporter->SetScalarTypeCallback(itkExporter->GetScalarTypeCallback());
  vtkImporter->SetNumberOfComponentsCallback(
      itkExporter->GetNumberOfComponentsCallback());
  vtkImporter->SetPropagateUpdateExtentCallback(
      itkExporter->GetPropagateUpdateExtentCallback());
  vtkImporter->SetUpdateDataCallback(itkExporter->GetUpdateDataCallback());
  vtkImporter->SetDataExtentCallback(itkExporter->GetDataExtentCallback());
  vtkImporter->SetBufferPointerCallback(itkExporter->GetBufferPointerCallback());
  vtkImporter->SetCallbackUserData(itkExporter->GetCallbackUserData());

  // import the VTK image object
  vtkImporter->Update(); // update immediately

  VTKInputImagePointer vtkImage = VTKInputImagePointer::New();
  vtkImage->ShallowCopy(vtkImporter->GetOutput());

  return vtkImage;
}

template<class TInputPixelType, class TOutputPixelType>
typename ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::OutputImagePointer ITKVTKDRRFilter<
    TInputPixelType, TOutputPixelType>::ConnectOutputITKImageToVTKImage(
    const VTKOutputImagePointer vtkImage)
{
  if (!vtkImage)
    return NULL;

  typedef vtkImageExport VTKExporterType;
  typedef vtkSmartPointer<VTKExporterType> VTKExporterPointer;
  typedef itk::VTKImageImport<OutputImageType> ITKImporterType;
  typedef typename ITKImporterType::Pointer ITKImporterPointer;

  // connect ITK-pipeline to VTK-pipeline:
  VTKExporterPointer vtkExporter = VTKExporterPointer::New();
  ITKImporterPointer itkImporter = ITKImporterType::New();

  vtkExporter->SetInput(vtkImage); // export the VTK image object

  // most important: connect the callbacks of both pipelines
  itkImporter->SetUpdateInformationCallback(
      vtkExporter->GetUpdateInformationCallback());
  itkImporter->SetPipelineModifiedCallback(
      vtkExporter->GetPipelineModifiedCallback());
  itkImporter->SetWholeExtentCallback(vtkExporter->GetWholeExtentCallback());
  itkImporter->SetSpacingCallback(vtkExporter->GetSpacingCallback());
  itkImporter->SetOriginCallback(vtkExporter->GetOriginCallback());
  itkImporter->SetScalarTypeCallback(vtkExporter->GetScalarTypeCallback());
  itkImporter->SetNumberOfComponentsCallback(
      vtkExporter->GetNumberOfComponentsCallback());
  itkImporter->SetPropagateUpdateExtentCallback(
      vtkExporter->GetPropagateUpdateExtentCallback());
  itkImporter->SetUpdateDataCallback(vtkExporter->GetUpdateDataCallback());
  itkImporter->SetDataExtentCallback(vtkExporter->GetDataExtentCallback());
  itkImporter->SetBufferPointerCallback(vtkExporter->GetBufferPointerCallback());
  itkImporter->SetCallbackUserData(vtkExporter->GetCallbackUserData());

  // import the ITK image object
  itkImporter->Update(); // update immediately

  return itkImporter->GetOutput();
}

template<class TInputPixelType, class TOutputPixelType>
typename ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::InputImagePointer ITKVTKDRRFilter<
    TInputPixelType, TOutputPixelType>::ConnectInputITKImageToVTKImage(
    const VTKOutputImagePointer vtkImage)
{
  if (!vtkImage)
    return NULL;

  typedef vtkImageExport VTKExporterType;
  typedef vtkSmartPointer<VTKExporterType> VTKExporterPointer;
  typedef itk::VTKImageImport<InputImageType> ITKImporterType;
  typedef typename ITKImporterType::Pointer ITKImporterPointer;

  // connect ITK-pipeline to VTK-pipeline:
  VTKExporterPointer vtkExporter = VTKExporterPointer::New();
  ITKImporterPointer itkImporter = ITKImporterType::New();

  vtkExporter->SetInput(vtkImage); // export the VTK image object

  // most important: connect the callbacks of both pipelines
  itkImporter->SetUpdateInformationCallback(
      vtkExporter->GetUpdateInformationCallback());
  itkImporter->SetPipelineModifiedCallback(
      vtkExporter->GetPipelineModifiedCallback());
  itkImporter->SetWholeExtentCallback(vtkExporter->GetWholeExtentCallback());
  itkImporter->SetSpacingCallback(vtkExporter->GetSpacingCallback());
  itkImporter->SetOriginCallback(vtkExporter->GetOriginCallback());
  itkImporter->SetScalarTypeCallback(vtkExporter->GetScalarTypeCallback());
  itkImporter->SetNumberOfComponentsCallback(
      vtkExporter->GetNumberOfComponentsCallback());
  itkImporter->SetPropagateUpdateExtentCallback(
      vtkExporter->GetPropagateUpdateExtentCallback());
  itkImporter->SetUpdateDataCallback(vtkExporter->GetUpdateDataCallback());
  itkImporter->SetDataExtentCallback(vtkExporter->GetDataExtentCallback());
  itkImporter->SetBufferPointerCallback(vtkExporter->GetBufferPointerCallback());
  itkImporter->SetCallbackUserData(vtkExporter->GetCallbackUserData());

  // import the ITK image object
  itkImporter->Update(); // update immediately

  return itkImporter->GetOutput();
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::GenerateData()
{
  // possibly multi-threaded GL environment:
  if (m_FireStartEndEvents)
    m_EventObject->InvokeEvent(vtkCommand::StartEvent);
  this->m_GPURayCaster->GenerateNextMappingID(); // prevent multiple rendering
#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
  XLockDisplay((Display*)this->m_RenderWindow->GetGenericDisplayId());
#endif
  this->m_RenderWindow->MakeCurrent();
  this->m_RenderWindow->Render();
  // we have to unbind the render context in order to allow other threads to
  // render to it, too!
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
  wglMakeCurrent((HDC__*)this->m_RenderWindow->GetGenericDisplayId(), NULL);
#else
  glXMakeCurrent((Display*)this->m_RenderWindow->GetGenericDisplayId(), None,
      NULL);
  XUnlockDisplay((Display*)this->m_RenderWindow->GetGenericDisplayId());
#endif
  if (m_FireStartEndEvents)
    m_EventObject->InvokeEvent(vtkCommand::EndEvent);
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetInteractive(
    bool interactive)
{
  this->m_Interactive = interactive;
  if (interactive)
  {
    if (this->m_GPURayCaster)
      this->m_GPURayCaster->SetDoScreenRenderingThoughLastDRRImageCopied(true);
    this->SetCopyDRRToImageOutput(this->m_CopyDRRToImageOutput);
  }
  else // -> force some props
  {
    if (this->m_GPURayCaster)
    {
      this->m_GPURayCaster->SetDoScreenRenderingThoughLastDRRImageCopied(false);
      this->m_GPURayCaster->SetLastDRR(
          this->m_VTKOutputImages[this->m_CurrentDRROutputIndex]);
    }
    this->SetCopyDRRToImageOutput(true);
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetRescaleSlope(
    double slope)
{
  if (this->m_RescaleSlope != slope)
  {
    this->m_RescaleSlope = slope;
    if (this->m_GPURayCaster)
      this->m_GPURayCaster->SetRescaleSlope(this->m_RescaleSlope);
    this->Modified();
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetRescaleIntercept(
    double intercept)
{
  if (this->m_RescaleIntercept != intercept)
  {
    this->m_RescaleIntercept = intercept;
    if (this->m_GPURayCaster)
      this->m_GPURayCaster->SetRescaleIntercept(this->m_RescaleIntercept);
    this->Modified();
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetContextTitle(
    std::string title)
{
  this->m_ContextTitle = title;
  if (this->m_ContextTitle.length() <= 0)
    this->m_ContextTitle = "ORA GLSL Ray-Caster, (c) radART, 2009-2010";
  if (this->m_RenderWindow)
    this->m_RenderWindow->SetWindowName(this->m_ContextTitle.c_str());
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetContextPosition(
    DiscretePositionType pos)
{
  this->m_ContextPosition = pos;
  if (this->m_RenderWindow)
    this->m_RenderWindow->SetPosition(this->m_ContextPosition[0],
        this->m_ContextPosition[1]);
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetCopyDRRToImageOutput(
    bool copy)
{
  this->m_CopyDRRToImageOutput = copy;
  if (this->m_Interactive)
  {
    if (this->m_GPURayCaster)
    {
      if (copy)
        this->m_GPURayCaster->SetLastDRR(
            this->m_VTKOutputImages[this->m_CurrentDRROutputIndex]);
      else
        this->m_GPURayCaster->SetLastDRR(NULL);
    }
  }
  else
  {
    if (this->m_GPURayCaster)
      this->m_GPURayCaster->SetLastDRR(
          this->m_VTKOutputImages[this->m_CurrentDRROutputIndex]);
    this->m_CopyDRRToImageOutput = true;
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::SetDRRMask(
    MaskImagePointer mask)
{
  if (this->m_CurrentDRROutputIndex < 0 || this->m_CurrentDRROutputIndex
      >= this->GetNumberOfIndependentOutputs())
    return;

  std::map<vtkImageData*, MaskImagePointer>::iterator it =
      this->m_VTKToITKMaskMap.find(
          this->m_VTKDRRMasks[this->m_CurrentDRROutputIndex]);
  if (it != this->m_VTKToITKMaskMap.end()) // mapping exists
  {
    if (it->second == mask) // no change!
    {
      // NOTE: we need this however!
      this->ApplyGeometrySettings();
      this->Modified();
      return;
    }

    // release texture memory if previously set
    if (this->m_GPURayCaster
        && this->m_VTKDRRMasks[this->m_CurrentDRROutputIndex])
      this->m_GPURayCaster->ReleaseGPUTexture(
          this->m_VTKDRRMasks[this->m_CurrentDRROutputIndex]);

    this->m_VTKToITKMaskMap.erase(it); // remove old mapping
  }

  this->m_VTKDRRMasks[this->m_CurrentDRROutputIndex] = NULL; // clear mask
  // add new mapping
  if (mask)
  {
    this->m_VTKDRRMasks[this->m_CurrentDRROutputIndex]
        = this->ConnectVTKImageToITKMaskImage(mask); // ITK-VTK-connection
    this->m_VTKToITKMaskMap[this->m_VTKDRRMasks[this->m_CurrentDRROutputIndex]]
        = mask; // store mapping
  }

  this->ApplyGeometrySettings();
  this->Modified();
}

template<class TInputPixelType, class TOutputPixelType>
typename ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::MaskImagePointer ITKVTKDRRFilter<
    TInputPixelType, TOutputPixelType>::GetDRRMask()
{
  if (this->m_CurrentDRROutputIndex >= 0 && this->m_CurrentDRROutputIndex
      < this->GetNumberOfIndependentOutputs())
  {
    std::map<vtkImageData*, MaskImagePointer>::iterator it =
        this->m_VTKToITKMaskMap.find(
            this->m_VTKDRRMasks[this->m_CurrentDRROutputIndex]);
    if (it != this->m_VTKToITKMaskMap.end())
      return it->second;
    else
      return NULL;
  }
  else
  {
    return NULL;
  }
}

template<class TInputPixelType, class TOutputPixelType>
void ITKVTKDRRFilter<TInputPixelType, TOutputPixelType>::GetTimeMeasuresOfLastComputation(
    double &volumeTransfer, double &maskTransfer, double &drrComputation,
    double &preProcessing, double &rayCasting, double &postProcessing)
{
  // init
  volumeTransfer = 0;
  maskTransfer = 0;
  drrComputation = 0;
  preProcessing = 0;
  rayCasting = 0;
  postProcessing = 0;
  if (this->m_GPURayCaster)
  {
    volumeTransfer = this->m_GPURayCaster->GetLastVolumeTransferTime();
    maskTransfer = this->m_GPURayCaster->GetLastMaskTransferTime();
    drrComputation = this->m_GPURayCaster->GetLastDRRComputationTime();
    // more detail
    preProcessing = this->m_GPURayCaster->GetLastDRRPreProcessingTime();
    rayCasting = this->m_GPURayCaster->GetLastDRRRayCastingTime();
    postProcessing = this->m_GPURayCaster->GetLastDRRPostProcessingTime();
  }
}

}

#endif /* ORAITKVTKDRRFILTER_TXX_ */

