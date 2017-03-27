#include "oraGLSLDRRRayCastMapper.h"

#include <vtkObjectFactory.h>
#include <vtkGPUInfoList.h>
#include <vtkGPUInfo.h>
#include <vtkgl.h>
#include <vtkOpenGLExtensionManager.h>
#include <vtksys/ios/sstream>
#include <vtkCommand.h>
#include <vtkAbstractMapper.h>
#include <vtkDataArray.h>
#include <vtkMath.h>
#include <vtkstd/map>
#include <vtkCellArray.h>
#include <vtkVolume.h>

#include <string>

#include "oraGLSLRayCastingCodeFragments.hxx"

#include "oraVTKUnsharpMaskingImageFilter.h"

namespace ora
{

vtkCxxRevisionMacro(GLSLDRRRayCastMapper, "2.3.2")

vtkStandardNewMacro(GLSLDRRRayCastMapper)

GLSLDRRRayCastMapper::GLSLDRRRayCastMapper() :
  vtkGPUVolumeRayCastMapper()
{
  this->MaxMemoryFraction = 0.75;
  this->AutoDetectVideoMemory(); // auto-detect available GPU video memory
  this->DRRComputationSupported = false;
  this->DRRComputationNotSupportedReasons = NULL;
  this->LoadedExtensions = false;
  this->RenderWindow = NULL;
  this->SupportFloatTextures = false;
  this->SupportPixelBufferObjects = false;
  this->SystemInitialized = false;
  this->GLSLAndOpenGLObjectsCreated = false;
  this->FrameBufferObject = 0;
  this->DepthRenderBufferObject = 0;
  int i = 0, j = 0;
  while (i < 3)
  {
    this->TextureObjects[i] = 0;
    i++;
  }
  i = 0;
  while (i < 9)
  {
    this->DRROrientation[i] = 0;
    i++;
  }
  i = 0;
  while (i < 4)
  {
    j = 0;
    while (j < 3)
    {
      this->DRRCorners[i][j] = 0;
      j++;
    }
    i++;
  }

  this->DRROrientation[0] = 1;
  this->DRROrientation[4] = 1;
  this->DRROrientation[8] = 1;
  this->ProgramShader = 0;
  this->FragmentMainShader = 0;
  this->FragmentProjectionShader = 0;
  this->FragmentTraceShader = 0;
  this->LastDRR = NULL;
  for (i = 0; i < 3; i++)
    this->Clocks[i] = vtkSmartPointer<vtkTimerLog>::New();
  this->LastDRRComputationTime = 0;
  i = 0;
  while (i < 2)
  {
    this->DRRSpacing[i] = 1.;
    this->DRRSize[i] = 0;
    this->LastDRRSize[i] = 0;
    this->ScalarRange[i] = 0;
    i++;
  }
  this->IntensityTFTable = NULL;
  this->IntensityTF = NULL;
  this->IntensityTFLinearInterpolation = true;
  this->BuiltProgram = false;
  this->RescaleIntercept = 0.;
  this->RescaleSlope = 1.;
  this->SavedFrameBuffer = 0;
  this->PlaneViewCamera = NULL;
  i = 0;
  while (i < 3)
  {
    this->Matrices[i] = vtkSmartPointer<vtkMatrix4x4>::New();
    this->RayCastSourcePosition[i] = 0;
    this->SourceOnPlane[i] = 0;
    this->DRROrigin[i] = 0;
    i++;
  }
  this->DRRFrustNearPlane = vtkSmartPointer<vtkPlane>::New();
  this->GLProjectionMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->GLProjectionTransform = vtkSmartPointer<vtkPerspectiveTransform>::New();
  this->Transform = vtkSmartPointer<vtkTransform>::New();
  this->CurrentVolumeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->InvVolumeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->HTransform = vtkSmartPointer<vtkTransform>::New();
  this->OrientationTransform = NULL;
  this->LastDRRPreProcessingTime = 0;
  this->LastDRRRayCastingTime = 0;
  this->LastDRRPostProcessingTime = 0;
  this->LastVolumeTransferTime = 0;
  this->LastMaskTransferTime = 0;
  this->SampleDistance = 1.;
  this->ScalarsTextures = new ScalarFieldTextureMapper();
  this->CurrentScalarFieldTexture = NULL;
  this->DRRMaskScalarFieldTexture = NULL;
  this->DRRMask = NULL;
  this->BoxSource = vtkSmartPointer<vtkTessellatedBoxSource>::New();
  this->Planes = vtkSmartPointer<vtkPlaneCollection>::New();
  this->NearPlane = vtkSmartPointer<vtkPlane>::New();
  this->Clip = vtkSmartPointer<vtkClipConvexPolyData>::New();
  this->Clip->SetInputConnection(this->BoxSource->GetOutputPort());
  this->Clip->SetPlanes(this->Planes);
  this->Densify = vtkSmartPointer<vtkDensifyPolyData>::New();
  this->Densify->SetInputConnection(this->Clip->GetOutputPort());
  this->Densify->SetNumberOfSubdivisions(2);
  this->ClippedBoundingBox = this->Densify->GetOutput();
  this->DoScreenRenderingThoughLastDRRImageCopied = false;
  this->VerticalFlip = false;
  this->UnsharpMasking = false;
  this->UnsharpMaskingRadius = 0;
  this->LastMappingID = -1;
  this->CurrentMappingID = 0;
  this->UseMappingIDsOff(); // do not use this feature by default
}

GLSLDRRRayCastMapper::~GLSLDRRRayCastMapper()
{
  ReleaseGraphicsResources(); // release basic resources

  this->RenderWindow = NULL;
  this->LastDRR = NULL;
  int i = 0;
  for (i = 0; i < 3; i++)
    this->Clocks[i] = NULL;
  if (this->IntensityTFTable)
    delete this->IntensityTFTable;
  this->IntensityTF = NULL;
  this->PlaneViewCamera = NULL;
  i = 0;
  while (i < 3)
  {
    Matrices[i] = NULL;
    i++;
  }
  this->Transform = NULL;
  this->CurrentVolumeMatrix = NULL;
  this->GLProjectionMatrix = NULL;
  this->GLProjectionTransform = NULL;
  this->InvVolumeMatrix = NULL;
  this->HTransform = NULL;
  this->OrientationTransform = NULL;
  if (this->ScalarsTextures)
    delete this->ScalarsTextures;
  this->DRRMask = NULL;
  this->BoxSource = NULL;
  this->Planes = NULL;
  this->NearPlane = NULL;
  this->Clip = NULL;
  this->Densify = NULL;
  this->ClippedBoundingBox = NULL;
  this->DRRFrustNearPlane = NULL;
  if (this->DRRComputationNotSupportedReasons)
    delete[] this->DRRComputationNotSupportedReasons;
  this->DRRComputationNotSupportedReasons = NULL;
}

void GLSLDRRRayCastMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Max Memory Fraction: " << this->MaxMemoryFraction
      << std::endl;
  os << indent << "Max Memory In Bytes: " << (vtkIdType) this->MaxMemoryInBytes
      << " (" << (this->MaxMemoryInBytes / 1024 / 1024) << "MB)" << std::endl;
  os << indent << "==> Effective Video Memory size: "
      << (vtkIdType) (this->MaxMemoryInBytes * this->MaxMemoryFraction) << " ("
      << (this->MaxMemoryInBytes * this->MaxMemoryFraction / 1024 / 1024)
      << "MB)" << std::endl;
  os << indent << "Render Window: " << this->RenderWindow.GetPointer()
      << std::endl;
  os << indent << "DRR Computation Supported: "
      << this->DRRComputationSupported << std::endl;
  os << indent << "DRR Computation not supported reasons: " <<
      this->DRRComputationNotSupportedReasons << std::endl;
  os << indent << "Loaded Extensions: " << this->LoadedExtensions << std::endl;
  os << indent << "Support Float Textures: " << this->SupportFloatTextures
      << std::endl;
  os << indent << "Support Pixel Buffer Objects: "
      << this->SupportPixelBufferObjects << std::endl;
  os << indent << "System Initialized: " << this->SystemInitialized
      << std::endl;
  os << indent << "GLSL And OpenGL Objects Created: "
      << this->GLSLAndOpenGLObjectsCreated << std::endl;
  os << indent << "FrameBuffer Object: " << this->FrameBufferObject
      << std::endl;
  os << indent << "Depth Render Buffer Object: "
      << this->DepthRenderBufferObject << std::endl;
  os << indent << "Texture Objects: " << this->TextureObjects[0] << " "
      << this->TextureObjects[1] << " " << this->TextureObjects[2] << std::endl;
  os << indent << "Program Shader: " << this->ProgramShader << std::endl;
  os << indent << "Fragment Main Shader: " << this->FragmentMainShader
      << std::endl;
  os << indent << "Fragment Projection Shader: "
      << this->FragmentProjectionShader << std::endl;
  os << indent << "Fragment Trace Shader: " << this->FragmentTraceShader
      << std::endl;
  os << indent << "Last DRR: " << this->LastDRR.GetPointer() << std::endl;
  os << indent << "Last DRR Computation Time: " << this->LastDRRComputationTime
      << "ms" << std::endl;
  os << indent << "Last DRR Pre-Processing Time: "
      << this->LastDRRPreProcessingTime << "ms" << std::endl;
  os << indent << "Last DRR Ray-Casting Time: " << this->LastDRRRayCastingTime
      << "ms" << std::endl;
  os << indent << "Last DRR Post-Processing Time: "
      << this->LastDRRPostProcessingTime << "ms" << std::endl;
  os << indent << "Last Volume Transfer Time: " << this->LastVolumeTransferTime
      << "ms" << std::endl;
  os << indent << "Last Mask Transfer Time: " << this->LastMaskTransferTime
      << "ms" << std::endl;
  os << indent << "Scalar Range: " << this->ScalarRange[0] << ","
      << this->ScalarRange[1] << std::endl;
  os << indent << "Intensity TF Table: " << this->IntensityTFTable << std::endl;
  os << indent << "Intensity TF: " << this->IntensityTF.GetPointer()
      << std::endl;
  os << indent << "Intensity TF Linear Interpolation: "
      << this->IntensityTFLinearInterpolation << std::endl;
  os << indent << "Built Program: " << this->BuiltProgram << std::endl;
  os << indent << "Rescale Slope: " << this->RescaleSlope << "\n";
  os << indent << "Rescale Intercept: " << this->RescaleIntercept << "\n";
  os << indent << "Saved Frame Buffer: " << this->SavedFrameBuffer << std::endl;
  os << indent << "Plane View Camera: " << this->PlaneViewCamera.GetPointer()
      << std::endl;
  os << indent << "Matrices: " << this->Matrices[0].GetPointer() << ","
      << this->Matrices[1].GetPointer() << ","
      << this->Matrices[2].GetPointer() << std::endl;
  os << indent << "Transform: " << this->Transform.GetPointer() << std::endl;
  os << indent << "Current Volume Matrix: "
      << this->CurrentVolumeMatrix.GetPointer() << std::endl;
  os << indent << "H Transform: " << this->HTransform.GetPointer() << std::endl;
  os << indent << "Orientation Transform: "
      << this->OrientationTransform.GetPointer() << std::endl;
  os << indent << "Sample Distance: " << this->SampleDistance << std::endl;
  os << indent << "Scalars Textures: " << this->ScalarsTextures << std::endl;
  os << indent << "Current Scalar Field Texture: "
      << this->CurrentScalarFieldTexture << std::endl;
  os << indent << "DRR Mask Scalar Field Texture: "
      << this->DRRMaskScalarFieldTexture << std::endl;
  os << indent << "DRR Mask: " << this->DRRMask << std::endl;
  os << indent << "Box Source: " << this->BoxSource.GetPointer() << std::endl;
  os << indent << "Planes: " << this->Planes.GetPointer() << std::endl;
  os << indent << "Near Plane: " << this->NearPlane.GetPointer() << std::endl;
  os << indent << "Inv Volume Matrix: " << this->InvVolumeMatrix.GetPointer()
      << std::endl;
  os << indent << "Clip: " << this->Clip.GetPointer() << std::endl;
  os << indent << "Densify: " << this->Densify.GetPointer() << std::endl;
  os << indent << "Clipped Bounding Box"
      << this->ClippedBoundingBox.GetPointer() << std::endl;
  os << indent << "Do Screen Rendering Though Last DRR Image Copied: "
      << this->DoScreenRenderingThoughLastDRRImageCopied << std::endl;
  os << indent << "Ray-Cast Source Position: "
      << this->RayCastSourcePosition[0] << ","
      << this->RayCastSourcePosition[1] << ","
      << this->RayCastSourcePosition[2] << std::endl;
  os << indent << "DRR Size: " << this->DRRSize[0] << "," << this->DRRSize[1]
      << std::endl;
  os << indent << "Last DRR Size: " << this->LastDRRSize[0] << ","
      << this->LastDRRSize[1] << std::endl;
  os << indent << "DRR Spacing: " << this->DRRSpacing[0] << ","
      << this->DRRSpacing[1] << std::endl;
  os << indent << "DRR Origin: " << this->DRROrigin[0] << ","
      << this->DRROrigin[1] << "," << this->DRROrigin[2] << std::endl;
  os << indent << "Source On Plane: " << this->SourceOnPlane[0] << ","
      << this->SourceOnPlane[1] << "," << this->SourceOnPlane[2] << std::endl;
  os << indent << "DRR Orientation:";
  for (int i = 0; i < 9; i++)
  {
    if (i % 3 != 0 || i == 0)
      os << " ";
    else
      os << " \\ ";
    os << this->DRROrientation[i];
  }
  os << std::endl;
  for (int i = 0; i < 4; i++)
  {
    os << indent << "DRR Corners[" << i << "]:";
    for (int j = 0; j < 3; j++)
    {
      os << " " << this->DRRCorners[i][j];
    }
    os << std::endl;
  }
  os << indent << "DRR Frustum Near Plane: "
      << this->DRRFrustNearPlane.GetPointer() << std::endl;
  os << indent << "Vertical Flip: " << this->VerticalFlip << std::endl;

#ifdef DEVELOPMENT
  vtkstd::cout << "\n\n--- DEVELOPMENT ---\n\n";
#endif
  PRINT_UNIFORM_VARIABLES(this->ProgramShader)
  // dev
  CHECK_FB_STATUS(); // dev
}

void GLSLDRRRayCastMapper::SetRenderWindow(vtkRenderWindow *renWin)
{
  if (renWin != this->RenderWindow)
  {
    this->RenderWindow = renWin;
    this->SystemInitialized = false; // set them back to be sure
    this->LoadedExtensions = false;
    this->SupportFloatTextures = false;
    this->SupportPixelBufferObjects = false;
    this->GLSLAndOpenGLObjectsCreated = false;
    this->BuiltProgram = false;
    this->Modified();
  }
}

void GLSLDRRRayCastMapper::AutoDetectVideoMemory()
{
  typedef vtkSmartPointer<vtkGPUInfoList> GPUInfoListType;

  this->MaxMemoryInBytes = 0;

  GPUInfoListType l = GPUInfoListType::New();
  l->Probe();
  if (l->GetNumberOfGPUs() > 0)
  {
    typedef vtkSmartPointer<vtkGPUInfo> GPUInfoType;

    // simply take first GPU
    GPUInfoType info = l->GetGPUInfo(0);
    this->MaxMemoryInBytes = info->GetDedicatedVideoMemory();
    if (this->MaxMemoryInBytes == 0)
      this->MaxMemoryInBytes = info->GetDedicatedSystemMemory();
    if (this->MaxMemoryInBytes == 0)
      this->MaxMemoryInBytes = info->GetSharedSystemMemory();
  }

  if (this->MaxMemoryInBytes == 0) // default: 512MB.
    this->MaxMemoryInBytes = 512 * 1024 * 1024;
}

bool GLSLDRRRayCastMapper::IsDRRComputationSupported()
{
  if (!this->DRRComputationSupported)
  {
    // -> check support:
    if (this->LoadedExtensions || LoadExtensions())
      this->DRRComputationSupported = true;
  }

  return this->DRRComputationSupported;
}

bool GLSLDRRRayCastMapper::LoadExtensions()
{
  if (!this->LoadedExtensions)
  {
    this->LoadedExtensions = true; // init
    if (this->DRRComputationNotSupportedReasons)
      delete[] this->DRRComputationNotSupportedReasons;
    this->DRRComputationNotSupportedReasons = NULL;

    if (!this->RenderWindow)
    {
      this->DRRComputationNotSupportedReasons = new char[10000];
      sprintf(this->DRRComputationNotSupportedReasons, "%s",
          "There is no render window set!");
      this->LoadedExtensions = false;
      return this->LoadedExtensions;
    }
    this->RenderWindow->MakeCurrent(); // ensure that it is current GL-context
    this->RenderWindow->Render();

    // only NVIDIA cards supported
    const char *gl_vendor = reinterpret_cast<const char *> (glGetString(
        GL_VENDOR));
    if (gl_vendor && strstr(gl_vendor, "ATI") != 0)
    {
      this->DRRComputationNotSupportedReasons = new char[10000];
      sprintf(this->DRRComputationNotSupportedReasons, "%s%s%s",
          "The GPU vendor description is obviously ATI: ",
          gl_vendor, "\n");
      this->LoadedExtensions = false;
      return this->LoadedExtensions;
    }

    // create extension manager
    typedef vtkSmartPointer<vtkOpenGLExtensionManager> GLExtMngrType;
    GLExtMngrType extensions = GLExtMngrType::New();
    extensions->SetRenderWindow(this->RenderWindow);

    // EXTENSION CHECKS

    // GL_ARB_draw_buffers requires OpenGL 1.3, so we must have OpenGL 1.3;
    // we don't need to check for some extensions that become part of OpenGL
    // core after 1.3. Among them:
    //   - texture_3d is in core OpenGL since 1.2
    //   - texture_edge_clamp is in core OpenGL since 1.2
    //     (GL_SGIS_texture_edge_clamp or GL_EXT_texture_edge_clamp (nVidia) )
    //   -  multi-texture is in core OpenGL since 1.3

    int supports_GL_1_3 = extensions->ExtensionSupported("GL_VERSION_1_3");
    // at least 1.3 support required
    if (!supports_GL_1_3)
    {
      this->DRRComputationNotSupportedReasons = new char[10000];
      sprintf(this->DRRComputationNotSupportedReasons, "%s",
          "OpenGL 1.3 is obviously not supported!\n");
      vtkDebugMacro(<< "GL-EXTENSIONS: Need at least OpenGL 1.3!")
      this->LoadedExtensions = false;
      return this->LoadedExtensions;
    }

    int supports_GL_2_0 = 0;
    // check for 2.0 support
    supports_GL_2_0 = extensions->ExtensionSupported("GL_VERSION_2_0");

    // some extensions that are supported in 2.0, but if we don't
    // have 2.0 we'll need to check further
    int supports_shading_language_100 = 1;
    int supports_shader_objects = 1;
    int supports_fragment_shader = 1;
    int supports_texture_non_power_of_two = 1;
    int supports_draw_buffers = 1;

    if (!supports_GL_2_0) // <2.0 obviously

    {
      supports_shading_language_100 = extensions->ExtensionSupported(
          "GL_ARB_shading_language_100");
      supports_shader_objects = extensions->ExtensionSupported(
          "GL_ARB_shader_objects");
      supports_fragment_shader = extensions->ExtensionSupported(
          "GL_ARB_fragment_shader");
      supports_texture_non_power_of_two = extensions->ExtensionSupported(
          "GL_ARB_texture_non_power_of_two");
      supports_draw_buffers = extensions->ExtensionSupported(
          "GL_ARB_draw_buffers");
    }

    // check for frame buffer objects
    int supports_GL_EXT_framebuffer_object = extensions->ExtensionSupported(
        "GL_EXT_framebuffer_object");

    // find out if we have OpenGL 1.4 support
    int supports_GL_1_4 = extensions->ExtensionSupported("GL_VERSION_1_4");

    // find out if we have the depth texture ARB extension
    int supports_GL_ARB_depth_texture = extensions->ExtensionSupported(
        "GL_ARB_depth_texture");

    // depth textures are support if we either have OpenGL 1.4
    // or if the depth texture ARB extension is supported
    int supports_depth_texture = supports_GL_1_4
        || supports_GL_ARB_depth_texture;

    if (!supports_shading_language_100)
    {
      this->DRRComputationNotSupportedReasons = new char[10000];
      sprintf(this->DRRComputationNotSupportedReasons, "%s",
          "GLSL 1.0 extension is obviously not supported!\nMoreover, no OpenGL 2.0 support available!\n");
      vtkDebugMacro(<< "GL-EXTENSIONS: shading_language_100 (or OpenGL 2.0)" <<
          " is required but not supported!")
      this->LoadedExtensions = false;
      return this->LoadedExtensions;
    }
    else
    {
      // GLSL version >=1.20 required
      const char *glsl_version = reinterpret_cast<const char *> (glGetString(
          vtkgl::SHADING_LANGUAGE_VERSION));
      int glslMajor, glslMinor;
      vtksys_ios::istringstream ist(glsl_version);
      ist >> glslMajor;
      char c;
      ist.get(c); // '.'
      ist >> glslMinor;
      if (glslMajor < 1 || (glslMajor == 1 && glslMinor < 20))
      {
        this->DRRComputationNotSupportedReasons = new char[10000];
        sprintf(this->DRRComputationNotSupportedReasons, "%s",
            "GLSL >=1.20 extension is obviously not supported!\n");
        vtkDebugMacro(<< "GL-EXTENSIONS: GLSL version >= 1.20 required - " <<
            "found version " << glsl_version)
        this->LoadedExtensions = false;
        return this->LoadedExtensions;
      }
    }

    if (!supports_shader_objects)
    {
      this->DRRComputationNotSupportedReasons = new char[10000];
      sprintf(this->DRRComputationNotSupportedReasons, "%s",
          "GLSL shader_objects extension is obviously not supported (or OpenGL 2.0)!\n");
      vtkDebugMacro(<< "GL-EXTENSIONS: shader_objects (or OpenGL 2.0) is " <<
          "required but not supported!")
      this->LoadedExtensions = false;
      return this->LoadedExtensions;
    }
    if (!supports_fragment_shader)
    {
      this->DRRComputationNotSupportedReasons = new char[10000];
      sprintf(this->DRRComputationNotSupportedReasons, "%s",
          "GLSL fragment_shader extension is obviously not supported (or OpenGL 2.0)!\n");
      vtkDebugMacro(<< "GL-EXTENSIONS: fragment_shader (or OpenGL 2.0) is " <<
          "required but not supported!")
      this->LoadedExtensions = false;
      return this->LoadedExtensions;
    }
    if (!supports_texture_non_power_of_two)
    {
      this->DRRComputationNotSupportedReasons = new char[10000];
      sprintf(this->DRRComputationNotSupportedReasons, "%s",
          "GLSL texture_non_power_of_two extension is obviously not supported (or OpenGL 2.0)!\n");
      vtkDebugMacro(<< "GL-EXTENSIONS: texture_non_power_of_two (or OpenGL " <<
          "2.0) is required but not supported!")
      this->LoadedExtensions = false;
      return this->LoadedExtensions;
    }
    if (!supports_draw_buffers)
    {
      this->DRRComputationNotSupportedReasons = new char[10000];
      sprintf(this->DRRComputationNotSupportedReasons, "%s",
          "GLSL draw_buffers extension is obviously not supported (or OpenGL 2.0)!\n");
      vtkDebugMacro(<< "GL-EXTENSIONS: draw_buffers (or OpenGL 2.0) is " <<
          "required but not supported!")
      this->LoadedExtensions = false;
      return this->LoadedExtensions;
    }
    if (!supports_depth_texture)
    {
      this->DRRComputationNotSupportedReasons = new char[10000];
      sprintf(this->DRRComputationNotSupportedReasons, "%s",
          "GLSL depth_texture extension is obviously not supported (or OpenGL 2.0)!\n");
      vtkDebugMacro(<< "GL-EXTENSIONS: depth_texture (or OpenGL 2.0) is " <<
          "required but not supported!")
      this->LoadedExtensions = false;
      return this->LoadedExtensions;
    }
    if (!supports_GL_EXT_framebuffer_object)
    {
      this->DRRComputationNotSupportedReasons = new char[10000];
      sprintf(this->DRRComputationNotSupportedReasons, "%s",
          "GLSL framebuffer_object extension is obviously not supported (or OpenGL 2.0)!\n");
      vtkDebugMacro(<< "GL-EXTENSIONS: framebuffer_object is " <<
          "required but not supported!")
      this->LoadedExtensions = false;
      return this->LoadedExtensions;
    }

    // EXTENSION LOADING

    // First load all 1.2 and 1.3 extensions (we know we
    // support at least up to 1.3)
    extensions->LoadExtension("GL_VERSION_1_2");
    extensions->LoadExtension("GL_VERSION_1_3");

    // Load the 2.0 extensions if supported
    if (supports_GL_2_0)
    {
      extensions->LoadExtension("GL_VERSION_2_0");
    }
    else // specifically load shader objects, fragment shader, draw buffer ext

    {
      extensions->LoadCorePromotedExtension("GL_ARB_shader_objects");
      extensions->LoadCorePromotedExtension("GL_ARB_fragment_shader");
      extensions->LoadCorePromotedExtension("GL_ARB_draw_buffers");
    }

    // load the framebuffer object extension
    extensions->LoadExtension("GL_EXT_framebuffer_object");

    // optional extension (does not fail if not present)
    // load it if supported which will allow us to store textures as floats
    this->SupportFloatTextures = extensions->ExtensionSupported(
        "GL_ARB_texture_float");
    if (this->SupportFloatTextures)
      extensions->LoadExtension("GL_ARB_texture_float");

    // optional extension (does not fail if not present)
    // used to minimize memory footprint when loading large 3D textures
    // of scalars.
    // VBO or 1.5 is required by PBO or 2.1
    int supports_GL_1_5 = extensions->ExtensionSupported("GL_VERSION_1_5");
    int supports_vertex_buffer_object = supports_GL_1_5
        || extensions->ExtensionSupported("GL_ARB_vertex_buffer_object");
    int supports_GL_2_1 = extensions->ExtensionSupported("GL_VERSION_2_1");
    this->SupportPixelBufferObjects = supports_vertex_buffer_object
        && (supports_GL_2_1 || extensions->ExtensionSupported(
            "GL_ARB_pixel_buffer_object"));
    if (this->SupportPixelBufferObjects)
    {
      if (supports_GL_1_5)
        extensions->LoadExtension("GL_VERSION_1_5");
      else
        extensions->LoadCorePromotedExtension("GL_ARB_vertex_buffer_object");

      if (supports_GL_2_1)
        extensions->LoadExtension("GL_VERSION_2_1");
      else
        extensions->LoadCorePromotedExtension("GL_ARB_pixel_buffer_object");
    }

    // NOTE: normally we should do a test compilation and linkage here as some
    // old cards support OpenGL 2.0 but not 'while'-statements in a fragment
    // shader (example: nVidia GeForce FX 5200); it does not fail when compiling
    // each shader source but at linking stage because the parser underneath
    // only checks for syntax during compilation and the actual native code
    // generation happens during linking stage.
    // -->
    // ... we accept that this code won't work on such cards!
  }

  return this->LoadedExtensions;
}

bool GLSLDRRRayCastMapper::IsSystemInitialized()
{
  return this->SystemInitialized;
}

bool GLSLDRRRayCastMapper::InitializeSystem()
{
  if (!this->SystemInitialized)
  {
    if (IsDRRComputationSupported()) // basic requirements

    {
      if (CreateGLSLAndOpenGLObjects())
      {
        this->SystemInitialized = true;
      }
    }
  }

  return this->SystemInitialized;
}

bool GLSLDRRRayCastMapper::VerifyCompilation(unsigned int shader)
{
  GLuint fs = static_cast<GLuint> (shader);
  GLint params;
  vtkgl::GetShaderiv(fs, vtkgl::COMPILE_STATUS, &params);

  if (params != GL_TRUE)
  {
    vtkErrorMacro(<< "Shader (" << shader << ") source compile error.");
    // include null terminator
    vtkgl::GetShaderiv(fs, vtkgl::INFO_LOG_LENGTH, &params);
    if (params > 0)
    {
      char *buffer = new char[params];
      vtkgl::GetShaderInfoLog(fs, params, 0, buffer);
      vtkErrorMacro(<< "\nDescription: " << buffer);
      delete[] buffer;
    }
    else
    {
      vtkErrorMacro(<< "\nNo Description available.");
    }
    return false;
  }
  else
    // silence
    return true;
}

bool GLSLDRRRayCastMapper::VerifyLinkage(unsigned int shader)
{
  GLint params;
  GLuint prog = static_cast<GLuint> (shader);
  vtkgl::GetProgramiv(prog, vtkgl::LINK_STATUS, &params);

  if (params != GL_TRUE)
  {
    vtkErrorMacro(<< "Shader (" << shader << ") linking error.");
    vtkgl::GetProgramiv(prog, vtkgl::INFO_LOG_LENGTH, &params);
    if (params > 0)
    {
      char *buffer = new char[params];
      vtkgl::GetProgramInfoLog(prog, params, 0, buffer);
      vtkErrorMacro(<< "\nDescription: " << buffer);
      delete[] buffer;
    }
    else
    {
      vtkErrorMacro(<< "\nNo Description available.");
    }
    return false;
  }
  else
    // silence
    return true;
}

bool GLSLDRRRayCastMapper::CreateGLSLAndOpenGLObjects()
{
  if (this->GLSLAndOpenGLObjectsCreated)
    return this->GLSLAndOpenGLObjectsCreated;

  // OpenGL objects:

  const int fbLF = 1; // framebuffer left front
  const int numFB = 2; // 2 framebuffers -> ping-pong

  // 2 * Frame buffers(2d textures) + colorMap (1d texture) +
  // dataset (3d texture) + grabbed depthMap (2d texture)
  const int numTextureObjects = numFB + fbLF;
  GLuint textureObjects[numTextureObjects];

  // create the various objects we will need - one frame buffer which will
  // contain a render buffer for depth and a texture for color.
  GLuint frameBufferObject;
  GLuint depthRenderBufferObject;
  vtkgl::GenFramebuffersEXT(1, &frameBufferObject); // color
  vtkgl::GenRenderbuffersEXT(1, &depthRenderBufferObject); // depth
  int i = 0;
  while (i < numTextureObjects)
  {
    textureObjects[i] = 0;
    i++;
  }

  // frame buffers(2d textures) + colorMap (1d texture) + dataset (3d texture)
  // + grabbed depth buffer (2d texture)
  glGenTextures(numTextureObjects, textureObjects);
  // color buffers
  GLint value;
  glGetIntegerv(vtkgl::FRAMEBUFFER_BINDING_EXT, &value);

  // store default frame buffer
  GLuint savedFrameBuffer = static_cast<GLuint> (value);

  vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, frameBufferObject);
  i = 0;
  while (i < numFB)
  {
    glBindTexture(GL_TEXTURE_2D, textureObjects[fbLF + i]);
    i++;
  }
  vtkgl::FramebufferTexture2DEXT(vtkgl::FRAMEBUFFER_EXT,
      vtkgl::COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureObjects[fbLF], 0);

  // depth buffer
  vtkgl::BindRenderbufferEXT(vtkgl::RENDERBUFFER_EXT, depthRenderBufferObject);
  vtkgl::FramebufferRenderbufferEXT(vtkgl::FRAMEBUFFER_EXT,
      vtkgl::DEPTH_ATTACHMENT_EXT, vtkgl::RENDERBUFFER_EXT,
      depthRenderBufferObject);

  // restore default frame buffer
  vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT, savedFrameBuffer);

  // save GL objects by static casting to standard C types. GL* types
  // are not allowed in VTK header files.
  this->FrameBufferObject = static_cast<unsigned int> (frameBufferObject);
  this->DepthRenderBufferObject
      = static_cast<unsigned int> (depthRenderBufferObject);
  i = 0;
  while (i < numTextureObjects)
  {
    this->TextureObjects[i] = static_cast<unsigned int> (textureObjects[i]);
    i++;
  }

  // GLSL objects:

  GLuint programShader = vtkgl::CreateProgram();
  GLuint fragmentMainShader = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);

  vtkgl::AttachShader(programShader, fragmentMainShader);
  vtkgl::DeleteShader(fragmentMainShader); // reference counting

  const char *mainSource = ora::GLSL_DRR_MAIN_CODE.c_str();
  vtkgl::ShaderSource(fragmentMainShader, 1,
      const_cast<const char **> (&mainSource), 0);
  vtkgl::CompileShader(fragmentMainShader);
  this->VerifyCompilation(static_cast<unsigned int> (fragmentMainShader));

  GLuint fragmentProjectionShader;
  GLuint fragmentTraceShader;

  fragmentProjectionShader = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);
  vtkgl::AttachShader(programShader, fragmentProjectionShader);
  vtkgl::DeleteShader(fragmentProjectionShader); // reference counting

  fragmentTraceShader = vtkgl::CreateShader(vtkgl::FRAGMENT_SHADER);
  vtkgl::AttachShader(programShader, fragmentTraceShader);
  vtkgl::DeleteShader(fragmentTraceShader); // reference counting

  this->ProgramShader = static_cast<unsigned int> (programShader);
  this->FragmentMainShader = static_cast<unsigned int> (fragmentMainShader);
  this->FragmentProjectionShader
      = static_cast<unsigned int> (fragmentProjectionShader);
  this->FragmentTraceShader = static_cast<unsigned int> (fragmentTraceShader);

  this->GLSLAndOpenGLObjectsCreated = true; // done

  return this->GLSLAndOpenGLObjectsCreated;
}

bool GLSLDRRRayCastMapper::ComputeDRR()
{
  bool success = false;

  if (UseMappingIDs) // prevent multiple processing of same mapping ID
  {
    if (CurrentMappingID == LastMappingID)
      return true; // already computed
    else
      LastMappingID = CurrentMappingID; // and resume processing
  }

  Clocks[0]->StartTimer();

  success = PreProcessing();

  if (success)
    success = RayCasting();

  if (success)
    success = PostProcessing();

  Clocks[0]->StopTimer();
  this->LastDRRComputationTime = Clocks[0]->GetElapsedTime() * 1000.;

  return success;
}

bool GLSLDRRRayCastMapper::AllocateFrameBuffers()
{
  bool success = false;

  if (this->DRRSize[0] <= 0 || this->DRRSize[1] <= 0)
    return success;

  if (this->DRRSize[0] != this->LastDRRSize[0] || this->DRRSize[1]
      != this->LastDRRSize[1])
  {
    const int fbLF = 1; // framebuffer left front
    const int dm = 0; // depth map
    const int numFB = 2; // 2 framebuffers -> ping-pong
    int i = 0;
    GLenum errorCode = glGetError();
    while (i < numFB && errorCode == GL_NO_ERROR)
    {
      glBindTexture(GL_TEXTURE_2D,
          static_cast<GLuint> (this->TextureObjects[fbLF + i]));
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, vtkgl::CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vtkgl::CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      // Here we are assuming that GL_ARB_texture_non_power_of_two is available
      if (this->SupportFloatTextures)
        glTexImage2D(GL_TEXTURE_2D, 0, vtkgl::RGBA16F_ARB, this->DRRSize[0],
            this->DRRSize[1], 0, GL_RGBA, GL_FLOAT, NULL);
      else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, this->DRRSize[0],
            this->DRRSize[1], 0, GL_RGBA, GL_FLOAT, NULL);
      errorCode = glGetError();
      i++;
    }

    if (errorCode == GL_NO_ERROR)
    {
      // grabbed depth buffer
      glBindTexture(GL_TEXTURE_2D,
          static_cast<GLuint> (this->TextureObjects[dm]));
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, vtkgl::CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vtkgl::CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, vtkgl::DEPTH_TEXTURE_MODE, GL_LUMINANCE);

      glTexImage2D(GL_TEXTURE_2D, 0, vtkgl::DEPTH_COMPONENT32,
          this->DRRSize[0], this->DRRSize[1], 0, GL_DEPTH_COMPONENT, GL_FLOAT,
          NULL);

      // set up depth render buffer
      GLint savedFrameBuffer;
      glGetIntegerv(vtkgl::FRAMEBUFFER_BINDING_EXT, &savedFrameBuffer);
      vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT,
          static_cast<GLuint> (this->FrameBufferObject));
      vtkgl::BindRenderbufferEXT(vtkgl::RENDERBUFFER_EXT,
          static_cast<GLuint> (this->DepthRenderBufferObject));
      vtkgl::RenderbufferStorageEXT(vtkgl::RENDERBUFFER_EXT,
          vtkgl::DEPTH_COMPONENT24, this->DRRSize[0], this->DRRSize[1]);
      vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT,
          static_cast<GLuint> (savedFrameBuffer));
      errorCode = glGetError();
    }

    success = (errorCode == GL_NO_ERROR);
    if (success)
    {
      // store size for next comparison
      i = 0;
      while (i < 2)
      {
        this->LastDRRSize[i] = this->DRRSize[i];
        i++;
      }
    }

    // NOTE: if decided to implement slabbing (volume-streaming) one day, we
    // have to allocate a further scalar buffer here for storing the values
    // which result from cropping!
  }
  else
    success = true;

  return success;
}

bool GLSLDRRRayCastMapper::UpdateIntensityTransferFunction()
{
  if (!this->IntensityTF)
    return false;

  if (!this->IntensityTFTable)
    this->IntensityTFTable = new RGBTableManager();

  vtkgl::ActiveTexture(vtkgl::TEXTURE1);

  this->IntensityTFTable->Update(this->IntensityTF, this->ScalarRange,
      this->IntensityTFLinearInterpolation);

  // restore default
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);

  return true;
}

bool GLSLDRRRayCastMapper::BuildProgram()
{
  if (this->BuiltProgram)
    return true;

  // projection code: perspective projection
  const char *projectionCode =
      ora::GLSL_DRR_PERSPECTIVE_PROJECTION_CODE.c_str();
  GLuint fs = static_cast<GLuint> (this->FragmentProjectionShader);
  vtkgl::ShaderSource(fs, 1, const_cast<const char **> (&projectionCode), 0);
  vtkgl::CompileShader(fs);
  this->VerifyCompilation(this->FragmentProjectionShader);
  PRINT_GL_ERRORS("after Projection-Shader") // dev

  // tracing code: ray-casting
  const char *tracingCode = ora::GLSL_DRR_RAY_CASTING_CODE.c_str();
  fs = static_cast<GLuint> (this->FragmentTraceShader);
  vtkgl::ShaderSource(fs, 1, const_cast<const char **> (&tracingCode), 0);
  vtkgl::CompileShader(fs);
  this->VerifyCompilation(this->FragmentTraceShader);
  PRINT_GL_ERRORS("after Tracing-Shader") // dev

  // link program
  vtkgl::LinkProgram(static_cast<GLuint> (this->ProgramShader));
  PRINT_GL_ERRORS("after Linkage") // dev

  this->BuiltProgram = VerifyLinkage(this->ProgramShader);

  return this->BuiltProgram;
}

bool GLSLDRRRayCastMapper::InitializeTexturesAndVariables()
{
  bool success = true;

  // intensity transfer function (1D texture)
  vtkgl::ActiveTexture(vtkgl::TEXTURE1);
  this->IntensityTFTable->Bind();
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);

  // input volume
  GLint dataSetTexture;
  dataSetTexture = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "dataSetTexture");
  if (dataSetTexture != -1)
  {
    vtkgl::Uniform1i(dataSetTexture, 0);
  }
  else
  {
    vtkErrorMacro(<< "dataSetTexture is not a uniform variable.");
    success = false;
  }

  // intensity transfer function
  GLint transferFunc;
  transferFunc = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "transferFunc");
  if (transferFunc != -1)
  {
    vtkgl::Uniform1i(transferFunc, 1);
  }
  else
  {
    vtkErrorMacro(<< "transferFunc is not a uniform variable.");
    success = false;
  }

  // rescale slope
  GLint rescaleSlope;
  rescaleSlope = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "rescaleSlope");
  if (rescaleSlope != -1)
  {
    vtkgl::Uniform1f(rescaleSlope, static_cast<GLfloat> (this->RescaleSlope));
  }
  else
  {
    vtkErrorMacro(<< "rescaleSlope is not a uniform variable.");
    success = false;
  }

  // rescale intercept
  GLint rescaleIntercept;
  rescaleIntercept = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "rescaleIntercept");
  if (rescaleIntercept != -1)
  {
    vtkgl::Uniform1f(rescaleIntercept,
        static_cast<GLfloat> (this->RescaleIntercept));
  }
  else
  {
    vtkErrorMacro(<< "rescaleIntercept is not a uniform variable.");
    success = false;
  }

  // depth texture
  const int dm = 0; // depth map
  vtkgl::ActiveTexture(vtkgl::TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint> (this->TextureObjects[dm]));
  GLint depthTexture;
  depthTexture = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "depthTexture");
  if (depthTexture != -1)
  {
    vtkgl::Uniform1i(depthTexture, 2);
  }
  else
  {
    vtkErrorMacro(<< "depthTexture is not a uniform variable.");
    success = false;
  }

  // DRR mask
  GLint drrMaskTexture;
  drrMaskTexture = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "drrMaskTexture");
  if (drrMaskTexture != -1)
  {
    vtkgl::Uniform1i(drrMaskTexture, 4); // -> TEXTURE4
  }
  else
  {
    vtkErrorMacro(<< "drrMaskTexture is not a uniform variable.");
    success = false;
  }

  vtkgl::ActiveTexture(vtkgl::TEXTURE0);

  // inverse size
  GLint invDRRSize;
  invDRRSize = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "invDRRSize");
  if (invDRRSize != -1)
  {
    vtkgl::Uniform2f(invDRRSize, static_cast<GLfloat> (1.0 / this->DRRSize[0]),
        static_cast<GLfloat> (1.0 / this->DRRSize[1]));
  }
  else
  {
    vtkErrorMacro(<< "invDRRSize is not a uniform variable.");
    success = false;
  }

  return success;
}

bool GLSLDRRRayCastMapper::InitializeFrameBuffers()
{
  GLint savedFrameBuffer;
  glGetIntegerv(vtkgl::FRAMEBUFFER_BINDING_EXT, &savedFrameBuffer);
  this->SavedFrameBuffer = static_cast<unsigned int> (savedFrameBuffer);
  vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT,
      static_cast<GLuint> (this->FrameBufferObject));

  GLenum buffer[2];
  buffer[0] = vtkgl::COLOR_ATTACHMENT0_EXT;
  buffer[1] = GL_NONE; // no cropping
  vtkgl::DrawBuffers(2, buffer);

  // initialize the second color buffer
  const int fbLF = 1; // frame buffer left front
  vtkgl::FramebufferTexture2DEXT(vtkgl::FRAMEBUFFER_EXT,
      vtkgl::COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, this->TextureObjects[fbLF],
      0);
  vtkgl::FramebufferTexture2DEXT(vtkgl::FRAMEBUFFER_EXT,
      vtkgl::COLOR_ATTACHMENT0_EXT + 1, GL_TEXTURE_2D,
      this->TextureObjects[fbLF + 1], 0);
  buffer[0] = vtkgl::COLOR_ATTACHMENT0_EXT;
  buffer[1] = vtkgl::COLOR_ATTACHMENT1_EXT;
  vtkgl::DrawBuffers(2, buffer);

  return true;
}

void GLSLDRRRayCastMapper::ComputeVolumeTransformMatrix()
{
  this->CurrentVolumeMatrix->Identity();

  if (this->GetInput())
  {
    this->HTransform->Push();
    this->HTransform->Identity();
    this->HTransform->PostMultiply();

    // image orientation matrix
    vtkSmartPointer<vtkMatrix4x4> om = vtkSmartPointer<vtkMatrix4x4>::New();
    om->Identity(); // default
    // consider image orientation if specified
    if (this->OrientationTransform)
    {
      om->DeepCopy(this->OrientationTransform->GetMatrix());
    }
    om->Invert();

    // concatenate pure image orientation:
    this->HTransform->Concatenate(om);

    // consider external relative transform:
    vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
    mat->DeepCopy(this->Transform->GetMatrix());
    this->HTransform->Concatenate(mat);

    mat = NULL;
    om = NULL;

    this->HTransform->PreMultiply();
    this->HTransform->GetMatrix(this->CurrentVolumeMatrix);
    this->HTransform->Pop();
  }
}

bool GLSLDRRRayCastMapper::SetRayCastingGeometryProps(double sourcePos[3],
    double size[2], double spacing[2], double origin[3], double orientation[9])
{
  if (size[0] <= 0 || size[1] <= 0 || spacing[0] <= 0 || spacing[1] <= 0)
    return false;

  int i;

  // first calculate the positions of the DRR plane's corner points:
  double *v1 = orientation; // plane x-direction (orientation is normalized)
  double *v2 = orientation + 3; // plane y-direction
  double w = size[0] * spacing[0]; // DRR width
  double h = size[1] * spacing[1]; // DRR height
  i = 0;
  while (i < 3)
  {
    this->DRRCorners[0][i] = origin[i];
    this->DRRCorners[1][i] = origin[i] + v1[i] * w;
    this->DRRCorners[2][i] = this->DRRCorners[1][i] + v2[i] * h;
    this->DRRCorners[3][i] = origin[i] + v2[i] * h;
    i++;
  }

  // project the ray-casting source point onto the plane and find the bounding
  // radius that determines the view angle:
  double *n = orientation + 6; // plane normal
  double sourceOnPlane[3];
  vtkPlane::ProjectPoint(sourcePos, origin, n, sourceOnPlane);
  double r = -1., x = -1.;
  int idx = -1;
  i = 0;
  while (i < 4)
  {
    x = vtkMath::Distance2BetweenPoints(sourceOnPlane, this->DRRCorners[i]);
    if (x > r) // r^2

    {
      r = x;
      idx = i;
    }
    i++;
  }
  if (idx < 0)
    return false;
  // -> compute viewing angle (vertical or horizontal):
  double qw = vtkMath::Dot(this->DRRCorners[idx], v1) - vtkMath::Dot(
      sourceOnPlane, v1);
  qw = fabs(qw);
  double qh = vtkMath::Dot(this->DRRCorners[idx], v2) - vtkMath::Dot(
      sourceOnPlane, v2);
  qh = fabs(qh);
  bool hordir;
  double va;
  double pd = vtkPlane::DistanceToPlane(sourcePos, n, origin);
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
  clipr[0] = 1.0; // static near plane, far plane behind DRR plane
  clipr[1] = vtkPlane::DistanceToPlane(sourcePos, n, origin) + 1.0;
  // set near plane for later processing:
  double npo[3];
  i = 0;
  while (i < 3)
  {
    npo[i] = sourcePos[i] - n[i] * clipr[0];
    i++;
  }
  this->DRRFrustNearPlane->SetOrigin(npo);
  this->DRRFrustNearPlane->SetNormal(n);

  // -> apply the settings to camera:
  this->PlaneViewCamera->SetPosition(sourcePos);
  this->PlaneViewCamera->SetFocalPoint(sourceOnPlane);
  this->PlaneViewCamera->SetViewUp(v2);
  this->PlaneViewCamera->SetUseHorizontalViewAngle(hordir);
  this->PlaneViewCamera->SetViewAngle(va);
  this->PlaneViewCamera->SetClippingRange(clipr[0], clipr[1]);

  // store values internally
  i = 0;
  while (i < 2)
  {
    this->DRRSize[i] = static_cast<int> (size[i]);
    this->DRRSpacing[i] = spacing[i];
    i++;
  }
  i = 0;
  while (i < 3)
  {
    this->RayCastSourcePosition[i] = sourcePos[i];
    this->DRROrigin[i] = origin[i];
    this->SourceOnPlane[i] = sourceOnPlane[i];
    i++;
  }
  i = 0;
  while (i < 9)
  {
    this->DRROrientation[i] = orientation[i];
    i++;
  }

  this->Modified();

  return true;
}

void GLSLDRRRayCastMapper::ComputeGLProjectionMatrix()
{
  // projective coordinates of source on near plane:
  double p[3];
  double t = 0;
  this->DRRFrustNearPlane->IntersectWithLine(this->SourceOnPlane,
      this->RayCastSourcePosition, t, p);
  double *v1 = this->DRROrientation; // (normalized)
  double *v2 = this->DRROrientation + 3;
  double v1src = vtkMath::Dot(p, v1);
  double v2src = vtkMath::Dot(p, v2);
  // project DRR corners onto near plane and compute min/max values (relative
  // to source position on near plane):
  this->DRRFrustNearPlane->IntersectWithLine(this->DRRCorners[0],
      this->RayCastSourcePosition, t, p);
  double xmin = vtkMath::Dot(p, v1) - v1src;
  double ymin = vtkMath::Dot(p, v2) - v2src;
  this->DRRFrustNearPlane->IntersectWithLine(this->DRRCorners[2],
      this->RayCastSourcePosition, t, p);
  double xmax = vtkMath::Dot(p, v1) - v1src;
  double ymax = vtkMath::Dot(p, v2) - v2src;
  // -> compute frustum:
  double cr[2]; // 0: near, 1: far
  this->PlaneViewCamera->GetClippingRange(cr);
  this->GLProjectionTransform->Identity();
  this->GLProjectionTransform->Frustum(xmin, xmax, ymin, ymax, cr[0], cr[1]);
  this->GLProjectionMatrix->DeepCopy(this->GLProjectionTransform->GetMatrix());
}

bool GLSLDRRRayCastMapper::PrepareOffscreenFrameBufferRendering()
{
  if (!this->PlaneViewCamera)
    return false;

  // FBO always starts at 0,0
  glViewport(0, 0, this->DRRSize[0], this->DRRSize[1]);
  glEnable(GL_SCISSOR_TEST); // scissor on the FBO, on the reduced part
  glScissor(0, 0, this->DRRSize[0], this->DRRSize[1]);

  // NOTE: we support the rescale intercept for background!
  glClearColor(this->RescaleIntercept, 0.0, 0.0, 0.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  if (this->DRRSize[0] > 0 && this->DRRSize[1] > 0)
  {
    ComputeGLProjectionMatrix();
    this->GLProjectionMatrix->Transpose();
    glLoadMatrixd(this->GLProjectionMatrix->Element[0]);
  }
  else
  {
    glLoadIdentity();
  }
  // push the model view matrix onto the stack, make sure we adjust the mode
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  this->ComputeVolumeTransformMatrix(); // update current volume matrix
  this->Matrices[0]->DeepCopy(this->CurrentVolumeMatrix);
  this->Matrices[0]->Transpose();

  // insert camera view transformation
  glMultMatrixd(this->Matrices[0]->Element[0]);
  glShadeModel(GL_SMOOTH);
  glDisable(GL_LIGHTING);
  glEnable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND); // very important, otherwise the first image looks dark

  return true;
}

bool GLSLDRRRayCastMapper::PreProcessing()
{
  bool success = false;

  this->Clocks[1]->StartTimer();

  if (!this->SystemInitialized)
    InitializeSystem();
  success = this->SystemInitialized; // need an initialized system

  if (success)
    success = this->GetInput(); // need an input volume

  // FRAME BUFFER ALLOCATION
  if (success)
    success = AllocateFrameBuffers();
  PRINT_GL_ERRORS("after AllocateFrameBuffers()") // dev

  // SCALAR RANGE EXTRACTION
  if (success)
  {
    int cf;
    vtkDataArray* scalars = vtkAbstractMapper::GetScalars(this->GetInput(), 0,
        0, -1, NULL, cf);
    // NOTE: we only accept single-component images!
    if (scalars->GetNumberOfComponents() == 1)
      scalars->GetRange(this->ScalarRange);
    else
      success = false;
  } PRINT_GL_ERRORS("after Scalar Range Extraction") // dev

  // UPDATE INTENSITY TRANSFER FUNCTION
  // (models X-ray attenuation)
  if (success)
    success = UpdateIntensityTransferFunction();
  PRINT_GL_ERRORS("after UpdateIntensityTransferFunction()") // dev

  // BUILD FRAGMENT SHADER GLSL PROGRAM
  const int dm = 0; // depth map
  if (success)
  {
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    vtkgl::ActiveTexture(vtkgl::TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint> (this->TextureObjects[dm]));
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, this->DRRSize[0],
        this->DRRSize[1]);
    vtkgl::ActiveTexture(vtkgl::TEXTURE0);
    PRINT_GL_ERRORS("after BuildProgram()-Preparation") // dev

    success = BuildProgram(); // compose the prog
    PRINT_GL_ERRORS("after BuildProgram()") // dev

    vtkgl::UseProgram(this->ProgramShader); // -> use shader
    // for active texture 0, data set
    PRINT_GL_ERRORS ("after BuildProgram()-Usage"); // dev
  }

  // INITIALIZE TEXTURES AND UNIFORM VARIABLES
  if (success)
    success = InitializeTexturesAndVariables();
  PRINT_GL_ERRORS("after InitializeTexturesAndVariables()"); // dev

  // INITIALIZE FRAME BUFFERS
  if (success)
    success = InitializeFrameBuffers();
  PRINT_GL_ERRORS("after InitializeFrameBuffers()"); // dev

  // PREPARE OFFSCREEN FRAME BUFFER RENDERING
  if (success)
  {
    success = PrepareOffscreenFrameBufferRendering();
    PRINT_GL_ERRORS("after PrepareOffscreenFrameBufferRendering()") // dev

    if (success)
    {
      // restore in case of composite with no cropping or streaming.
      GLenum buffer[2];
      buffer[0] = vtkgl::COLOR_ATTACHMENT0_EXT;
      buffer[1] = GL_NONE;
      vtkgl::DrawBuffers(2, buffer);
      vtkgl::FramebufferTexture2DEXT(vtkgl::FRAMEBUFFER_EXT,
          vtkgl::COLOR_ATTACHMENT0_EXT + 1, GL_TEXTURE_2D, 0, 0);
    }PRINT_GL_ERRORS ("after Restore (Pre-Processing)"); // dev
  }

  // otherwise, we were rendering back face to initialize the z-buffer
  glCullFace(GL_BACK);

  this->Clocks[1]->StopTimer();
  this->LastDRRPreProcessingTime = this->Clocks[1]->GetElapsedTime() * 1000.;
  PRINT_GL_ERRORS("after Pre-Processing") // dev

  return success;
}

bool GLSLDRRRayCastMapper::TransferVolumeToGPU()
{
  bool success = false;
  double bounds[6];

  this->Clocks[2]->StartTimer();

  this->GetInput()->GetBounds(bounds); // input is expected to exist!

  // load scalar field
  int textureExtent[6];
  this->GetInput()->GetExtent(textureExtent);
  // is this subvolume already on the GPU?
  // i.e. is the extent of the volume inside the loaded extent?
  // - find the texture
  vtkstd::map<vtkImageData *, ScalarFieldManager *>::iterator it =
      this->ScalarsTextures->Map.find(this->GetInput());
  ScalarFieldManager *texture;
  if (it == this->ScalarsTextures->Map.end())
    texture = NULL;
  else
    texture = (*it).second;

  int loaded = texture != 0 && texture->IsLoaded()
      && this->GetInput()->GetMTime() <= texture->GetBuildTime();

  vtkIdType *loadedExtent;
  if (loaded)
  {
    loadedExtent = texture->GetLoadedExtent();
    int i = 0;
    while (loaded && i < 6)
    {
      loaded = loaded && loadedExtent[i] <= textureExtent[i];
      ++i;
      loaded = loaded && loadedExtent[i] >= textureExtent[i];
      ++i;
    }
  }

  if (loaded)
  {
    this->CurrentScalarFieldTexture = texture;
    vtkgl::ActiveTexture(vtkgl::TEXTURE0);
    this->CurrentScalarFieldTexture->Bind();
  }

  if (!loaded)
  {
    // -> load the whole dataset at once (we do not support streaming)

    // make sure we rebind our texture object to texture0 even if we don't have
    // to load the data themselves because the binding might be changed by
    // another mapper between two rendering calls.
    vtkgl::ActiveTexture(vtkgl::TEXTURE0);

    // find the texture.
    it = this->ScalarsTextures->Map.find(this->GetInput());

    if (it == this->ScalarsTextures->Map.end())
    {
      texture = new ScalarFieldManager();
      this->ScalarsTextures->Map[this->GetInput()] = texture;
      texture->SetSupportFloatTextures(this->SupportFloatTextures);
    }
    else
    {
      texture = (*it).second;
    }
    texture->Update(this->GetInput(), textureExtent, 0, 0, -1, NULL, true,
        this->ScalarRange,
        static_cast<int> (static_cast<float> (this->MaxMemoryInBytes)
            * this->MaxMemoryFraction));

    loaded = texture->IsLoaded();
    this->CurrentScalarFieldTexture = texture;
  }

  success = loaded;

  if (success)
  {
    loadedExtent = this->CurrentScalarFieldTexture->GetLoadedExtent();

    // low bounds and high bounds are in texture coordinates
    float lowBounds[3];
    float highBounds[3];
    int i = 0;
    while (i < 3)
    {
      double delta = static_cast<double> (loadedExtent[i * 2 + 1]
          - loadedExtent[i * 2] + 1);
      lowBounds[i] = static_cast<float> ((loadedExtent[i * 2] + 0.5
          - static_cast<double> (loadedExtent[i * 2])) / delta);
      highBounds[i] = static_cast<float> ((loadedExtent[i * 2 + 1] + 0.5
          - static_cast<double> (loadedExtent[i * 2])) / delta);
      ++i;
    }

    // lower bounds
    GLint lb = vtkgl::GetUniformLocation(
        static_cast<GLuint> (this->ProgramShader), "lowBounds");
    if (lb != -1)
      vtkgl::Uniform3f(lb, lowBounds[0], lowBounds[1], lowBounds[2]);
    else
    {
      vtkErrorMacro(<< "lowBounds is not a uniform variable.");
      success = false;
    }

    // higher bounds
    GLint hb = vtkgl::GetUniformLocation(
        static_cast<GLuint> (this->ProgramShader), "highBounds");
    if (hb != -1)
      vtkgl::Uniform3f(hb, highBounds[0], highBounds[1], highBounds[2]);
    else
    {
      vtkErrorMacro(<< "highBounds is not a uniform variable.");
      success = false;
    }
  }

  this->Clocks[2]->StopTimer();
  this->LastVolumeTransferTime = this->Clocks[2]->GetElapsedTime() * 1000.;

  return success;
}

bool GLSLDRRRayCastMapper::TransferMaskToGPU()
{
  this->Clocks[2]->StartTimer();

  // first check whether we have a valid mask
  int dims[3];
  dims[0] = dims[1] = dims[2] = -1;
  if (this->DRRMask)
    this->DRRMask->GetDimensions(dims);
  if (dims[0] == this->DRRSize[0] && dims[1] == this->DRRSize[1])
  {
    bool success = false;

    // load scalar field
    int textureExtent[6];
    this->DRRMask->GetExtent(textureExtent);
    // is this mask already on the GPU?
    // i.e. is the extent of the mask inside the loaded extent?
    // - find the texture
    vtkstd::map<vtkImageData *, ScalarFieldManager *>::iterator it =
        this->ScalarsTextures->Map.find(this->DRRMask);
    ScalarFieldManager *texture;
    if (it == this->ScalarsTextures->Map.end())
      texture = NULL;
    else
      texture = (*it).second;

    int loaded = texture != 0 && texture->IsLoaded()
        && this->DRRMask->GetMTime() <= texture->GetBuildTime();

    vtkIdType *loadedExtent;
    if (loaded)
    {
      loadedExtent = texture->GetLoadedExtent();
      int i = 0;
      while (loaded && i < 6)
      {
        loaded = loaded && loadedExtent[i] <= textureExtent[i];
        ++i;
        loaded = loaded && loadedExtent[i] >= textureExtent[i];
        ++i;
      }
    }

    if (loaded)
    {
      this->DRRMaskScalarFieldTexture = texture;
      vtkgl::ActiveTexture(vtkgl::TEXTURE4); // TEXTURE 4 !!!
      this->DRRMaskScalarFieldTexture->Bind();
    }

    if (!loaded)
    {
      // make sure we rebind our texture object to texture4 even if we don't have
      // to load the data themselves because the binding might be changed by
      // another mapper between two rendering calls.
      vtkgl::ActiveTexture(vtkgl::TEXTURE4);

      // find the texture.
      it = this->ScalarsTextures->Map.find(this->DRRMask);

      if (it == this->ScalarsTextures->Map.end())
      {
        texture = new ScalarFieldManager();
        this->ScalarsTextures->Map[this->DRRMask] = texture;
        texture->SetSupportFloatTextures(false);
      }
      else
      {
        texture = (*it).second;
      }

      int cf = 0;
      vtkDataArray* scalars = vtkAbstractMapper::GetScalars(this->DRRMask, 0,
          0, -1, NULL, cf);
      // NOTE: we only accept single-component images!
      if (scalars->GetNumberOfComponents() == 1)
      {
        double range[2];
        scalars->GetRange(range);
        texture->Update(this->DRRMask, textureExtent, 0, 0, -1, NULL, true,
            range,
            static_cast<int> (static_cast<float> (this->MaxMemoryInBytes)
                * this->MaxMemoryFraction));

        loaded = texture->IsLoaded();
        this->DRRMaskScalarFieldTexture = texture;
      }
    }

    success = loaded;

    GLfloat useDRRMask = vtkgl::GetUniformLocation(
        static_cast<GLuint> (this->ProgramShader), "useDRRMask");
    if (useDRRMask != -1)
    {
      if (success) // set flag to TRUE ->  render the unmasked DRR content only
        vtkgl::Uniform1f(useDRRMask, static_cast<GLfloat> (1.0));
      else
        // set flag to FALSE ->  render the whole DRR
        vtkgl::Uniform1f(useDRRMask, static_cast<GLfloat> (0.0));
    }
    else
    {
      vtkErrorMacro(<< "useDRRMask is not a uniform variable.");
      success = false;
    }

    this->Clocks[2]->StopTimer();
    this->LastMaskTransferTime = this->Clocks[2]->GetElapsedTime() * 1000.;

    return success;
  }
  else // NO MASK
  {
    // set useDRRMask-flag to FALSE ->  render the whole DRR:
    GLfloat useDRRMask = vtkgl::GetUniformLocation(
        static_cast<GLuint> (this->ProgramShader), "useDRRMask");
    if (useDRRMask != -1)
    {
      vtkgl::Uniform1f(useDRRMask, static_cast<GLfloat> (0.0));
    }
    else
    {
      vtkErrorMacro(<< "useDRRMask is not a uniform variable.");

      this->Clocks[2]->StopTimer();
      this->LastMaskTransferTime = this->Clocks[2]->GetElapsedTime() * 1000.;

      return false;
    }

    this->Clocks[2]->StopTimer();
    this->LastMaskTransferTime = this->Clocks[2]->GetElapsedTime() * 1000.;

    return true; // is OK
  }
}

bool GLSLDRRRayCastMapper::InitializeGeometry()
{
  bool success = true;

  this->ComputeVolumeTransformMatrix(); // world to dataset
  vtkMatrix4x4 *datasetToWorld = this->Matrices[0];
  double *bounds = this->CurrentScalarFieldTexture->GetLoadedBounds();
  double dx = bounds[1] - bounds[0];
  double dy = bounds[3] - bounds[2];
  double dz = bounds[5] - bounds[4];

  vtkMatrix4x4::Invert(this->CurrentVolumeMatrix, datasetToWorld);

  // worldToTexture matrix is needed
  // - compute change-of-coordinate matrix from world space to texture space
  vtkMatrix4x4 *worldToTexture = this->Matrices[2];
  vtkMatrix4x4 *datasetToTexture = this->Matrices[1];
  // - set the matrix
  datasetToTexture->Zero();
  datasetToTexture->SetElement(0, 0, dx);
  datasetToTexture->SetElement(1, 1, dy);
  datasetToTexture->SetElement(2, 2, dz);
  datasetToTexture->SetElement(3, 3, 1.);
  datasetToTexture->SetElement(0, 3, bounds[0]);
  datasetToTexture->SetElement(1, 3, bounds[2]);
  datasetToTexture->SetElement(2, 3, bounds[4]);
  // - worldToTexture := worldToDataSet * dataSetToTexture
  vtkMatrix4x4::Multiply4x4(this->CurrentVolumeMatrix, datasetToTexture,
      worldToTexture);

  // -> fixed perspective projection

  // compute camera position in texture coordinates
  // position of the center of the camera in world frame
  double cameraPosWorld[4];
  // position of the center of the camera in the dataset frame
  // (the transform of the volume is taken into account)
  double cameraPosDataset[4];
  // position of the center of the camera in the texture frame
  // the coordinates are translated and rescaled
  double cameraPosTexture[4];

  // camera position NEEDS NOT to equal ray-cast source position
  // (-> tilted DRR planes possible!)
  cameraPosWorld[0] = this->RayCastSourcePosition[0];
  cameraPosWorld[1] = this->RayCastSourcePosition[1];
  cameraPosWorld[2] = this->RayCastSourcePosition[2];
  cameraPosWorld[3] = 1.0; // we use homogeneous coordinates.

  datasetToWorld->MultiplyPoint(cameraPosWorld, cameraPosDataset);

  // from homogeneous to cartesian coordinates
  if (cameraPosDataset[3] != 1.0)
  {
    double ratio = 1 / cameraPosDataset[3];
    cameraPosDataset[0] *= ratio;
    cameraPosDataset[1] *= ratio;
    cameraPosDataset[2] *= ratio;
  }

  cameraPosTexture[0] = (cameraPosDataset[0] - bounds[0]) / dx;
  cameraPosTexture[1] = (cameraPosDataset[1] - bounds[2]) / dy;
  cameraPosTexture[2] = (cameraPosDataset[2] - bounds[4]) / dz;

  // only make sense for the vectorial part of the homogeneous matrix.
  // coefMatrix=transposeWorldToTexture*worldToTexture
  // we re-cycle the datasetToWorld pointer with a different name
  vtkMatrix4x4 *transposeWorldToTexture = this->Matrices[1];
  // transposeWorldToTexture={^t}worldToTexture
  vtkMatrix4x4::Transpose(worldToTexture, transposeWorldToTexture);

  vtkMatrix4x4 *coefMatrix = this->Matrices[1];
  vtkMatrix4x4::Multiply4x4(transposeWorldToTexture, worldToTexture, coefMatrix);

  // camera position
  GLint sourcePosition = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "sourcePosition");
  if (sourcePosition != -1)
    vtkgl::Uniform3f(sourcePosition,
        static_cast<GLfloat> (cameraPosTexture[0]),
        static_cast<GLfloat> (cameraPosTexture[1]),
        static_cast<GLfloat> (cameraPosTexture[2]));
  else
  {
    vtkErrorMacro(<< "sourcePosition is not a uniform variable.");
    success = false;
  }

  // sample distance
  GLint sampleDistance = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "sampleDistance");
  if (sampleDistance != -1)
    vtkgl::Uniform1f(sampleDistance, this->SampleDistance);
  else
  {
    vtkErrorMacro(<< "sampleDistance is not a uniform variable.");
    success = false;
  }

  // matrix 1
  GLint matrix1 = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "matrix1");
  if (matrix1 != -1)
    vtkgl::Uniform3f(matrix1,
        static_cast<GLfloat> (coefMatrix->GetElement(0, 0)),
        static_cast<GLfloat> (coefMatrix->GetElement(1, 1)),
        static_cast<GLfloat> (coefMatrix->GetElement(2, 2)));
  else
  {
    vtkErrorMacro(<< "matrix1 is not a uniform variable.");
    success = false;
  }

  // matrix 2
  GLint matrix2 = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "matrix2");
  if (matrix2 != -1)
    vtkgl::Uniform3f(matrix2, static_cast<GLfloat> (2 * coefMatrix->GetElement(
        0, 1)), static_cast<GLfloat> (2 * coefMatrix->GetElement(1, 2)),
        static_cast<GLfloat> (2 * coefMatrix->GetElement(0, 2)));
  else
  {
    vtkErrorMacro(<< "matrix2 is not a uniform variable.");
    success = false;
  }

  // change-of-coordinate matrix from Eye space to texture space
  vtkMatrix4x4 *eyeToTexture = this->Matrices[1];
  vtkMatrix4x4 *eyeToWorld = this->PlaneViewCamera->GetViewTransformMatrix();
  vtkMatrix4x4::Multiply4x4(eyeToWorld, worldToTexture, eyeToTexture);

  GLfloat matrix[16];// used sometimes as 3x3, sometimes as 4x4.
  double *raw = eyeToTexture->Element[0];
  int index;
  int column;
  int row;
  eyeToTexture->Invert();
  index = 0;
  column = 0;
  while (column < 4)
  {
    row = 0;
    while (row < 4)
    {
      matrix[index] = static_cast<GLfloat> (raw[row * 4 + column]);
      ++index;
      ++row;
    }
    ++column;
  }

  GLint textureToEye = vtkgl::GetUniformLocation(
      static_cast<GLuint> (this->ProgramShader), "textureToEye");
  if (textureToEye != -1)
    vtkgl::UniformMatrix4fv(textureToEye, 1, GL_FALSE, matrix);
  else
  {
    vtkErrorMacro(<< "textureToEye is not a uniform variable.");
    success = false;
  }

  return success;
}

bool GLSLDRRRayCastMapper::ClipBoundingBox()
{
  bool success = false;

  double *bounds = this->CurrentScalarFieldTexture->GetLoadedBounds();
  this->BoxSource->SetBounds(bounds);
  this->BoxSource->SetLevel(0);
  this->BoxSource->QuadsOn();

  this->Planes->RemoveAllItems();

  double range[2];
  double camPos[4];
  double focalPoint[4];
  double direction[3];

  camPos[0] = this->RayCastSourcePosition[0];
  camPos[1] = this->RayCastSourcePosition[1];
  camPos[2] = this->RayCastSourcePosition[2];
  camPos[3] = 1.0; // we use homogeneous coordinates.

  // pass camera through inverse volume matrix
  // so that we are in the same coordinate system
  ComputeVolumeTransformMatrix();
  this->InvVolumeMatrix->DeepCopy(this->CurrentVolumeMatrix);
  this->InvVolumeMatrix->Invert();
  this->InvVolumeMatrix->MultiplyPoint(camPos, camPos);
  if (camPos[3])
  {
    camPos[0] /= camPos[3];
    camPos[1] /= camPos[3];
    camPos[2] /= camPos[3];
  }
  // (clipping range is approximately clipping range from source position)
  this->PlaneViewCamera->GetClippingRange(range);
  this->PlaneViewCamera->GetFocalPoint(focalPoint);
  focalPoint[3] = 1.0;
  this->InvVolumeMatrix->MultiplyPoint(focalPoint, focalPoint);
  if (focalPoint[3])
  {
    focalPoint[0] /= focalPoint[3];
    focalPoint[1] /= focalPoint[3];
    focalPoint[2] /= focalPoint[3];
  }

  // compute the normalized view direction
  direction[0] = focalPoint[0] - camPos[0];
  direction[1] = focalPoint[1] - camPos[1];
  direction[2] = focalPoint[2] - camPos[2];
  vtkMath::Normalize(direction);

  double nearPoint[3];
  double dist = range[1] - range[0];
  range[0] += dist / (2 << 16);
  range[1] -= dist / (2 << 16);

  nearPoint[0] = camPos[0] + range[0] * direction[0];
  nearPoint[1] = camPos[1] + range[0] * direction[1];
  nearPoint[2] = camPos[2] + range[0] * direction[2];

  this->NearPlane->SetOrigin(nearPoint);
  this->NearPlane->SetNormal(direction);
  this->Planes->AddItem(this->NearPlane);

  this->Clip->Update();
  this->Densify->Update();
  // this->ClippedBoundingBox is updated!

  success = true;

  return success;
}

bool GLSLDRRRayCastMapper::ConvertAndRender()
{
  bool success = false;

  vtkPoints *points = this->ClippedBoundingBox->GetPoints();
  vtkCellArray *polys = this->ClippedBoundingBox->GetPolys();
  vtkIdType npts;
  vtkIdType *pts;
  vtkIdType i, j;
  double pt[3];

  double center[3] = { 0, 0, 0 };
  double min[3] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MAX, VTK_DOUBLE_MAX };
  double max[3] = { VTK_DOUBLE_MIN, VTK_DOUBLE_MIN, VTK_DOUBLE_MIN };

  // first compute center point
  npts = points->GetNumberOfPoints();
  for (i = 0; i < npts; i++)
  {
    points->GetPoint(i, pt);
    for (j = 0; j < 3; j++)
    {
      min[j] = (pt[j] < min[j]) ? (pt[j]) : (min[j]);
      max[j] = (pt[j] > max[j]) ? (pt[j]) : (max[j]);
    }
  }
  center[0] = 0.5 * (min[0] + max[0]);
  center[1] = 0.5 * (min[1] + max[1]);
  center[2] = 0.5 * (min[2] + max[2]);

  double *loadedBounds = NULL;
  vtkIdType *loadedExtent = NULL;
  loadedBounds = this->CurrentScalarFieldTexture->GetLoadedBounds();
  loadedExtent = this->CurrentScalarFieldTexture->GetLoadedExtent();

  double *spacing = this->GetInput()->GetSpacing();
  double spacingSign[3];
  i = 0;
  while (i < 3)
  {
    if (spacing[i] < 0)
      spacingSign[i] = -1.0;
    else
      spacingSign[i] = 1.0;
    ++i;
  }
  int polyId = 0;
  polys->InitTraversal();
  while (polys->GetNextCell(npts, pts))
  {
    vtkIdType start, end, inc;

    // need to have at least a triangle
    if (npts > 2)
    {
      // check the cross product of the first two
      // vectors dotted with the vector from the
      // center to the second point. Is it positive or negative?
      double p1[3], p2[3], p3[3];
      double v1[3], v2[3], v3[3], v4[3];

      points->GetPoint(pts[0], p1);
      points->GetPoint(pts[1], p2);
      points->GetPoint(pts[2], p3);

      v1[0] = p2[0] - p1[0];
      v1[1] = p2[1] - p1[1];
      v1[2] = p2[2] - p1[2];

      v2[0] = p2[0] - p3[0];
      v2[1] = p2[1] - p3[1];
      v2[2] = p2[2] - p3[2];

      vtkMath::Cross(v1, v2, v3);
      vtkMath::Normalize(v3);

      v4[0] = p2[0] - center[0];
      v4[1] = p2[1] - center[1];
      v4[2] = p2[2] - center[2];
      vtkMath::Normalize(v4);

      double dot = vtkMath::Dot(v3, v4);
      if (dot < 0)
      {
        start = 0;
        end = npts;
        inc = 1;
      }
      else
      {
        start = npts - 1;
        end = -1;
        inc = -1;
      }

      glBegin(GL_TRIANGLE_FAN); // GL_POLYGON -> GL_TRIANGLE_FAN

      double vert[3];
      double tcoord[3];
      for (i = start; i != end; i += inc)
      {
        points->GetPoint(pts[i], vert);
        for (j = 0; j < 3; j++)
        {
          // loaded bounds take both cell data and point date cases into account
          // texcoords between 1/2N and 1-1/2N:
          double tmp; // between 0 and 1
          tmp = spacingSign[j] * (vert[j] - loadedBounds[j * 2])
              / (loadedBounds[j * 2 + 1] - loadedBounds[j * 2]);
          double delta = static_cast<double> (loadedExtent[j * 2 + 1]
              - loadedExtent[j * 2] + 1);
          tcoord[j] = (tmp * (delta - 1) + 0.5) / delta;
        }
        vtkgl::MultiTexCoord3dv(vtkgl::TEXTURE0, tcoord);
        glVertex3dv(vert);
      }
      glEnd();
    }

    // otherwise, we are rendering back face to initialize the z-buffer
    glFinish();

    ++polyId;
  }

  // in OpenGL copy texture to texture does not exist but
  // framebuffer to texture exists (and our FB is an FBO).
  // we have to copy and not just to switch color textures because the
  // colorbuffer has to accumulate color or values step after step.
  // Switching would not work because two different steps can draw different
  // polygons that don't overlap

  const int fbLF = 1; // frame buffer left front

  vtkgl::ActiveTexture(vtkgl::TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, this->TextureObjects[fbLF + 1]);
  glReadBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, this->DRRSize[0],
      this->DRRSize[1]);
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);

  success = true;

  return success;
}

bool GLSLDRRRayCastMapper::RayCasting()
{
  bool success = false;

  // NOTE: this method is called if and only if pre-processing stage was
  // successful (this means furthermore that Input is valid and the system
  // is initialized)

  this->Clocks[1]->StartTimer();

  // CHECK SAMPLING DISTANCE
  success = (this->SampleDistance > 0.);

  // TRANSFER DRR MASK TO GPU VIDEO MEMORY
  if (success)
    success = TransferMaskToGPU();
  PRINT_GL_ERRORS("after TransferMaskToGPU()") // dev

  // TRANSFER INPUT VOLUME TO GPU VIDEO MEMORY
  if (success)
    success = TransferVolumeToGPU();
  PRINT_GL_ERRORS("after TransferVolumeToGPU()") // dev

  // INITIALIZE GEOMETRY
  if (success)
    success = InitializeGeometry();
  PRINT_GL_ERRORS("after InitializeGeometry()") // dev

  // CLIP VOLUME BOUNDING BOX
  if (success)
    success = ClipBoundingBox();
  PRINT_GL_ERRORS("after ClipBoundingBox()") // dev

  // CONVERSION AND RENDERING
  if (success)
    success = ConvertAndRender();
  PRINT_GL_ERRORS("after ConvertAndRender()") // dev

  this->Clocks[1]->StopTimer();
  this->LastDRRRayCastingTime = this->Clocks[1]->GetElapsedTime() * 1000.;
  PRINT_GL_ERRORS("after Ray-Casting") // dev

  return success;
}

bool GLSLDRRRayCastMapper::UnbindTextures()
{
  bool success = false;

  // mask texture
  vtkgl::ActiveTexture(vtkgl::TEXTURE4);
  glBindTexture(vtkgl::TEXTURE_3D_EXT, 0);

  // depth texture
  vtkgl::ActiveTexture(vtkgl::TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, 0);

  vtkgl::ActiveTexture(vtkgl::TEXTURE1);
  glBindTexture(GL_TEXTURE_1D, 0);

  // back to active texture 0.
  vtkgl::ActiveTexture(vtkgl::TEXTURE0);
  glBindTexture(vtkgl::TEXTURE_3D_EXT, 0);

  success = true;

  return success;
}

bool GLSLDRRRayCastMapper::CleanUpRender()
{
  bool success = false;

  // do not longer use the program
  vtkgl::UseProgram(0);

  // set back
  glPopMatrix();
  glDisable(GL_CULL_FACE);

  // recover frame buffer
  vtkgl::BindFramebufferEXT(vtkgl::FRAMEBUFFER_EXT,
      static_cast<GLuint> (this->SavedFrameBuffer));
  this->SavedFrameBuffer = 0;

  // undo the viewport change we made to reduce resolution
  glViewport(0, 0, this->DRRSize[0], this->DRRSize[1]);
  glEnable(GL_SCISSOR_TEST);
  glScissor(0, 0, this->DRRSize[0], this->DRRSize[1]);

  success = true;

  return success;
}

bool GLSLDRRRayCastMapper::CopyAndOrScreenRenderTexture()
{
  bool success = false;

  const int fbLF = 1; // framebuffer left front

  // if LastDRR-object is instantiated convert actual rendered image to image data
  if (this->LastDRR)
  {
    // we just need to copy the data, not render it
    glBindTexture(GL_TEXTURE_2D, this->TextureObjects[fbLF + 1]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // store the real sum-FLOAT-values (not normalized) in a VTK image with
    // adequate dimension, one component and float scalar type (we solely
    // need the red channel as DRR computation only uses the red channel):
    float *outPtr = static_cast<float *> (this->LastDRR->GetScalarPointer());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, outPtr);

    // flip along vertical direction (y-direction):
    if (VerticalFlip)
    {
      const int w = this->DRRSize[0]; // image width
      const int wb = w * sizeof(float); // width in bytes
      const int h = this->DRRSize[1]; // image height
      const int hl = h / 2; // half lines
      float *line = new float[w]; // temp buffer
      void *pline = (void *) line;
      void *pbuff = 0;
      void *pbuff2 = 0;
      int n = w * h - w; // last line
      for (int k = 0, y = 0; y < hl; k += w, y++, n -= w)
      {
        pbuff = (void *) (outPtr + n);
        memcpy(pline, pbuff, wb); // store for swap
        pbuff2 = (void *) (outPtr + k);
        memcpy(pbuff, pbuff2, wb); // swap
        memcpy(pbuff2, pline, wb);
      }
      delete[] line;
    }

    if (UnsharpMasking)
    {
      VTKUnsharpMaskingImageFilter *usmf = VTKUnsharpMaskingImageFilter::New();
      usmf->SetInput(this->LastDRR);
      if (UnsharpMaskingRadius > 0)
      {
        usmf->SetRadius(UnsharpMaskingRadius);
        usmf->AutoRadiusOff();
      }
      else
      {
        usmf->AutoRadiusOn();
      }
      usmf->Update();
      // copy result
      this->LastDRR->CopyAndCastFrom(usmf->GetOutput(), usmf->GetOutput()->GetWholeExtent());
      usmf->SetInput(NULL);
      usmf->Delete();
      usmf = NULL;
    }

    if (!DoScreenRenderingThoughLastDRRImageCopied)
    {
      success = true;
      return success;
    }
  }

  glViewport(0, 0, this->DRRSize[0], this->DRRSize[1]);
  glEnable(GL_SCISSOR_TEST);
  glScissor(0, 0, this->DRRSize[0], this->DRRSize[1]);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, this->DRRSize[0], 0.0, this->DRRSize[1], -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glBindTexture(GL_TEXTURE_2D, this->TextureObjects[fbLF]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  // As we use replace mode, we don't need to set the color value
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glDisable(GL_DEPTH_TEST);

  double xOffset = 1.0 / this->DRRSize[0];
  double yOffset = 1.0 / this->DRRSize[1];
  glDepthMask(GL_FALSE);
  glEnable(GL_TEXTURE_2D); // fixed pipeline (no scale/bias program)

  glBegin(GL_QUADS);
  glTexCoord2f(static_cast<GLfloat> (xOffset), static_cast<GLfloat> (yOffset));
  glVertex2f(0.0, 0.0);
  glTexCoord2f(static_cast<GLfloat> (1.0 - xOffset),
      static_cast<GLfloat> (yOffset));
  glVertex2f(static_cast<GLfloat> (this->DRRSize[0]), 0.0);
  glTexCoord2f(static_cast<GLfloat> (1.0 - xOffset), static_cast<GLfloat> (1.0
      - yOffset));
  glVertex2f(static_cast<GLfloat> (this->DRRSize[0]),
      static_cast<GLfloat> (this->DRRSize[1]));
  glTexCoord2f(static_cast<GLfloat> (xOffset), static_cast<GLfloat> (1.0
      - yOffset));
  glVertex2f(0.0, static_cast<GLfloat> (this->DRRSize[1]));
  glEnd();

  // restore the default mode; used in overlay
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glDisable(GL_TEXTURE_2D);
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  success = true;

  return success;
}

bool GLSLDRRRayCastMapper::PostProcessing()
{
  bool success = false;

  // NOTE: this method is called if and only if ray-casting stage was
  // successful (this means furthermore that Input is valid and the system
  // is initialized and textures exist)

  this->Clocks[1]->StartTimer();

  // UNBIND RENDERING TEXTURE OBJECTS
  success = UnbindTextures();
  PRINT_GL_ERRORS("after UnbindTextures()") // dev

  // OPENGL CLEAN UP
  if (success)
    success = CleanUpRender();
  PRINT_GL_ERRORS("after CleanUpRender()") // dev

  // COPY (IMAGE) AND/OR RENDER COMPUTED TEXTURE TO SCREEN
  if (success)
    success = CopyAndOrScreenRenderTexture();
  PRINT_GL_ERRORS("after CopyAndOrScreenRenderTexture()") // dev

  // FINAL CLEAN UP
  glEnable(GL_DEPTH_TEST);
  glPopAttrib(); // restore the blending mode and function
  glFinish();

  this->Clocks[1]->StopTimer();
  this->LastDRRPostProcessingTime = this->Clocks[1]->GetElapsedTime() * 1000.;
  PRINT_GL_ERRORS("after Post-Processing") // dev

  return success;
}

void GLSLDRRRayCastMapper::ReleaseGraphicsResources()
{
  // release GLSL and openGL objects
  if (this->GLSLAndOpenGLObjectsCreated)
  {
    if (this->RenderWindow)
      this->RenderWindow->MakeCurrent();

    this->LastDRRSize[0] = 0;
    this->LastDRRSize[1] = 0;

    GLuint frameBufferObject = static_cast<GLuint> (this->FrameBufferObject);
    vtkgl::DeleteFramebuffersEXT(1, &frameBufferObject);
    GLuint depthRenderBufferObject =
        static_cast<GLuint> (this->DepthRenderBufferObject);
    vtkgl::DeleteRenderbuffersEXT(1, &depthRenderBufferObject);

    const int fbLF = 1; // framebuffer left front
    const int numFB = 2; // 2 framebuffers -> ping-pong
    const int numTextureObjects = numFB + fbLF;
    GLuint textureObjects[numTextureObjects];
    int i = 0;
    while (i < numTextureObjects)
    {
      textureObjects[i] = static_cast<GLuint> (this->TextureObjects[i]);
      ++i;
    }
    glDeleteTextures(numTextureObjects, textureObjects);

    GLuint programShader = static_cast<GLuint> (this->ProgramShader);
    vtkgl::DeleteProgram(programShader);
    this->ProgramShader = 0;

    GLuint fragmentMainShader = static_cast<GLuint> (this->FragmentMainShader);
    vtkgl::DeleteShader(fragmentMainShader);
    this->FragmentMainShader = 0;

    GLuint fragmentProjectionShader =
        static_cast<GLuint> (this->FragmentProjectionShader);
    vtkgl::DeleteShader(fragmentProjectionShader);
    this->FragmentProjectionShader = 0;

    GLuint fragmentTraceShader =
        static_cast<GLuint> (this->FragmentTraceShader);
    vtkgl::DeleteShader(fragmentTraceShader);
    this->FragmentTraceShader = 0;

    this->GLSLAndOpenGLObjectsCreated = false;
  }

  ReleaseGPUTextures();

  if (this->IntensityTFTable != 0)
  {
    delete this->IntensityTFTable;
    this->IntensityTFTable = 0;
  }

  this->SystemInitialized = false; // set back
}

bool GLSLDRRRayCastMapper::ReleaseGPUTextures()
{
  if (this->ScalarsTextures != 0)
  {
    if (!this->ScalarsTextures->Map.empty())
    {
      vtkstd::map<vtkImageData *, ScalarFieldManager *>::iterator it =
          this->ScalarsTextures->Map.begin();
      bool deletedOne = false;
      while (it != this->ScalarsTextures->Map.end())
      {
        ScalarFieldManager *texture = (*it).second;
        delete texture;
        ++it;
        deletedOne = true;
      }
      this->ScalarsTextures->Map.clear();
      return deletedOne;
    }
  }
  return false;
}

bool GLSLDRRRayCastMapper::ReleaseGPUTexture(vtkImageData *textureImage)
{
  if (this->ScalarsTextures != 0 && textureImage)
  {
    if (!this->ScalarsTextures->Map.empty())
    {
      vtkstd::map<vtkImageData *, ScalarFieldManager *>::iterator it =
          this->ScalarsTextures->Map.find(textureImage);
      if (it != this->ScalarsTextures->Map.end())
      {
        ScalarFieldManager *texture = (*it).second;
        delete texture;
        this->ScalarsTextures->Map.erase(it);
        return true;
      }
    }
  }
  return false;
}

void GLSLDRRRayCastMapper::GenerateNextMappingID()
{
  CurrentMappingID++;
  if (CurrentMappingID > 100000)
    CurrentMappingID = 0;
}

void GLSLDRRRayCastMapper::PreRender(vtkRenderer *ren, vtkVolume *vol,
    double datasetBounds[6], double scalarRange[2],
    int numberOfScalarComponents, unsigned int numberOfLevels)
{
  PreProcessing(); // just to do something (should never be called)
}

void GLSLDRRRayCastMapper::RenderBlock(vtkRenderer *ren, vtkVolume *vol,
    unsigned int level)
{
  RayCasting(); // just to do something (should never be called)
}

void GLSLDRRRayCastMapper::PostRender(vtkRenderer *ren,
    int numberOfScalarComponents)
{
  PostProcessing(); // just to do something (should never be called)
}

void GLSLDRRRayCastMapper::GPURender(vtkRenderer *ren, vtkVolume *vol)
{
  ComputeDRR(); // compute the DRR
}

}
