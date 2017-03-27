//

#ifndef ORAGLSLDRRRAYCASTMAPPER_H_
#define ORAGLSLDRRRAYCASTMAPPER_H_

#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkTimerLog.h>
#include <vtkColorTransferFunction.h>
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkTessellatedBoxSource.h>
#include <vtkPlaneCollection.h>
#include <vtkPlane.h>
#include <vtkClipConvexPolyData.h>
#include <vtkDensifyPolyData.h>
#include <vtksys/ios/sstream>
#include <vtkPerspectiveTransform.h>

#include "oraRGBTableManager.h"
#include "oraScalarFieldManager.h"

// activate (uncommented) or deactivate (commented) development mode:
//#define DEVELOPMENT
// development routines (defines):
#define PRINT_UNIFORM_VARIABLES_F
#define GL_ERROR_TO_STRING_F
#define PRINT_GL_ERRORS_F
#define CHECK_FB_STATUS_F
#define BUFF_TO_STRING_F
#define DISPLAY_BUFFERS_F
#define DISPLAY_FB_ATTACHMENTS_F
#define DISPLAY_FB_ATTACHMENT_F
// development routines (calls for developer):
#define PRINT_UNIFORM_VARIABLES(shader) ;
#define PRINT_GL_ERRORS(title)
#define CHECK_FB_STATUS()
#define BUFF_TO_STRING(buffer)
#define DISPLAY_BUFFERS()
#define DISPLAY_FB_ATTACHMENTS()
#define DISPLAY_FB_ATTACHMENT(attachment)
// mode-dependent override:
#ifdef DEVELOPMENT
#include "oraGLSLDRRRayCasterDevMethods.hxx"
#endif

namespace ora
{

/** \class GLSLDRRRayCastMapper
 * \brief GLSL-based ray-casting algorithm for fast DRR computation.
 *
 * A GLSL-based ray-casting algorithm for fast computation of digitally
 * reconstructed radiographs (DRR) using a computer's graphics card.
 *
 * It implements VTK's GPU volume mapper interface (vtkGPUVolumeRayCastMapper)
 * and has strongly been inspired by vtkOpenGLGPUVolumeRayCastMapper. It
 * contains a lot of code sections from this class. However, it extends it in a
 * lot of ways and strictly omits code sections that are irrelevant for DRR
 * computation or would slow down DRR computation.
 *
 * Due to the nature of this class, a valid openGL context in the form of an
 * initialized render window (vtkRenderWindow) with a valid renderer
 * (vtkRenderer) and active camera (vtkActiveCamera) must be referenced.
 *
 * The projection geometry can be set up using the setters and getters of this
 * class. It is NOT defined by the active camera; the camera is just for
 * internal purposes.
 *
 * At the moment the whole input volume to be ray-casted must fit into the
 * GPU's video memory - NO streaming support at the moment. The maximum fraction
 * of the video memory can explicitly be set. NOTE: for auto-detection of
 * available dedicated video memory extensions must be installed and activated:
 * DirectX-SDK on Windows, NV-CONTROL extension on Linux.
 *
 * Moreover, only single-component images are allowed.
 *
 * NOTE: An optional DRR mask can be defined (which must exactly fit the DRR
 * size). Pixels having values greater than 0 are computed, the others are
 * omitted. This may be useful for computing only specific parts of a DRR or
 * imitating a C-arm camera.
 *
 * A rescale slope and/or intercept can be defined in order to linearly
 * transform the current DRR on the fly. <br>
 * NOTE: If you use a mask and rescaling: the masked regions would get a value
 * of 0 (no absorption). However, the rescale intercept may change the masked
 * pixel values though!
 *
 * For the rest of the attributes refer to the methods' and members'
 * documentation.
 *
 * This class is also used for ITK-VTK-pipelining of GPU-based DRR-rendering
 * (or::ITKVTKDRRFilter).
 *
 * For development and debugging there is a header providing some dev-macros
 * and functionalities (oraGLSLDRRRayCasterDevMethods.hxx). The internal
 * define <code>#define DEVELOPMENT</code> should be commented for release
 * versions!
 *
 * @see vtkGPUVolumeRayCastMapper
 * @see vtkOpenGLGPUVolumeRayCastMapper
 * @see ora::RGBTableManager
 * @see ora::ScalarFieldManager
 * @see ora::ITKVTKDRRFilter
 * @see oraGLSLDRRRayCasterDevMethods.hxx
 *
 * @author phil 
 * @author VTK-community
 * @author Markus Neuner 
 * @version 2.3.2
 *
 * \ingroup Mappers
 */
class GLSLDRRRayCastMapper:
    public vtkGPUVolumeRayCastMapper
{
  /*
   TRANSLATOR ora::GLSLDRRRayCastMapper
   */

public:
  /** VTK standard instantiation **/
  static GLSLDRRRayCastMapper *New();

  /** type information **/
vtkTypeRevisionMacro(GLSLDRRRayCastMapper, vtkGPUVolumeRayCastMapper)
  ;

  /** object information **/
  void PrintSelf(ostream& os, vtkIndent indent);

  /**
   * Manually set amount of video memory to be used / available on GPU
   * (MaxMemoryInBytes * MaxMemoryFraction is the effectively available amount
   * of memory on the GPU). NOTE: at instantiation the amount of available
   * video memory is retrieved automatically (but can fail on some cards /
   * environments).
   **/
  vtkSetMacro(MaxMemoryInBytes, vtkIdType)
  /**
   * Get amount of video memory to be used / available on GPU
   * (MaxMemoryInBytes * MaxMemoryFraction is the effectively available amount
   * of memory on the GPU).
   **/
  vtkGetMacro(MaxMemoryInBytes, vtkIdType)

  /**
   * Set maximum fraction of available video memory to be used on GPU
   * (MaxMemoryInBytes * MaxMemoryFraction is the effectively available amount
   * of memory on the GPU); DEFAULT: 0.75 (75% of video memory).
   **/
  vtkSetClampMacro(MaxMemoryFraction, float, 0.1f, 1.0f)
  /**
   * Get maximum fraction of available video memory to be used on GPU
   * (MaxMemoryInBytes * MaxMemoryFraction is the effectively available amount
   * of memory on the GPU); DEFAULT: 0.75 (75% of video memory).
   **/
  vtkGetMacro(MaxMemoryFraction, float)

  /**
   * @return TRUE if GLSL-based ray-casting is supported on the this system
   * (hardware-dependent), FALSE otherwise (which means that this mapper will
   * not be able to compute DRRs on this system)
   */
  virtual bool IsDRRComputationSupported();

  /** Set render window required for openGL context. MUST BE SET! **/
  virtual void SetRenderWindow(vtkRenderWindow *renWin);
  /** Get render window required for openGL context. **/
  vtkRenderWindow *GetRenderWindow()
  {
    return this->RenderWindow;
  }

  /**
   * Initialize the DRR computation system (OpenGL checks and initialization).
   * @return TRUE if the DRR computation system could be initialized
   * successfully
   **/
  virtual bool InitializeSystem();
  /**
   * @return TRUE if the DRR computation system has been initialized
   * successfully
   **/
  virtual bool IsSystemInitialized();

  /**
   * Computes the DRR on the GPU and stores it as vtkImageData in LastDRR. <br>
   * NOTE: the system must be initialized before invoking this method!
   * @return TRUE if the DRR could successfully be computed
   **/
  virtual bool ComputeDRR();

  /**
   * Set the intensity transfer function (represented as RGB function) that
   * maps volume intensities to output intensities (models X-ray attenuation).
   * (First channel is of interest.)
   **/
  vtkSetObjectMacro(IntensityTF, vtkColorTransferFunction)
  /**
   * Get the intensity transfer function (represented as RGB function) that
   * maps volume intensities to output intensities (models X-ray attenuation).
   * (First channel is of interest.)
   **/
  vtkColorTransferFunction *GetIntensityTF()
  {
    return this->IntensityTF;
  }

  /**
   * Set the intensity transfer function's interpolation mode: TRUE ... linear
   * interpolation, FALSE ... nearest neighbor.
   **/
  vtkSetMacro(IntensityTFLinearInterpolation, bool)
  /**
   * Get the intensity transfer function's interpolation mode: TRUE ... linear
   * interpolation, FALSE ... nearest neighbor.
   **/
  vtkGetMacro(IntensityTFLinearInterpolation, bool)

  /** Get time (ms) it took to compute the last DRR. **/
  vtkGetMacro(LastDRRComputationTime, double)
  /** Get time (ms) it took to pre-process last DRR. **/
  vtkGetMacro(LastDRRPreProcessingTime, double)
  /** Get time (ms) it took to ray-cast last DRR. **/
  vtkGetMacro(LastDRRRayCastingTime, double)
  /** Get time (ms) it took to post-process last DRR. **/
  vtkGetMacro(LastDRRPostProcessingTime, double)
  /** Get time (ms) it took to transfer volume to GPU last time. **/
  vtkGetMacro(LastVolumeTransferTime, double)
  /** Get time (ms) it took to transfer mask to GPU last time. **/
  vtkGetMacro(LastMaskTransferTime, double)

  /**
   * Set rescale slope s. <br>
   * <b>V'=V*s + i</b> <br>
   * V'...resultant DRR pixel value, V...original DRR pixel, s...rescale slope,
   * i...rescale intercept<br>
   * As DRRs are summation images, the resultant intensities in the DRRs
   * strongly depend on the input volume's voxel intensities and its size and
   * ray step size. In some situations, it may however be of interest to
   * linearly transform the DRR pixel intensities into another range. For
   * example the slope and intercept properties can be used for visualizing
   * DRRs (only intensities in the range from 0.0 to 1.0 can be directly
   * visualized when using an interactive mode).
   * @see RescaleIntercept
   */
  vtkSetMacro(RescaleSlope, double)
  /** Get rescale slope s. **/
  vtkGetMacro(RescaleSlope, double)
  /**
   * Set rescale intercept i. <br>
   * <b>V'=V*s + i</b> <br>
   * V'...resultant DRR pixel value, V...original DRR pixel, s...rescale slope,
   * i...rescale intercept<br>
   * As DRRs are summation images, the resultant intensities in the DRRs
   * strongly depend on the input volume's voxel intensities and its size and
   * ray step size. In some situations, it may however be of interest to
   * linearly transform the DRR pixel intensities into another range. For
   * example the slope and intercept properties can be used for visualizing
   * DRRs (only intensities in the range from 0.0 to 1.0 can be directly
   * visualized when using an interactive mode).
   * @see RescaleSlope
   **/
  vtkSetMacro(RescaleIntercept, double)
  /** Get rescale intercept i. **/
  vtkGetMacro(RescaleIntercept, double)

  /**
   * Set transform defining a relative movement of the input volume outgoing
   * from its initial pose defined by the input volume's origin
   **/
  virtual void SetTransform(vtkTransform *trans)
  {
    if (trans != this->Transform)
    {
      this->Transform = trans;
      this->Modified();
    }
  }
  /**
   * Get transform defining a relative movement of the input volume outgoing
   * from its initial pose defined by the input volume's origin
   **/
  vtkTransform *GetTransform()
  {
    return this->Transform;
  }

  /**
   * Set the geometry properties that define the nature of ray-casting and the
   * resultant DRR.
   * @param sourcePos position of the ray-casting source in WCS (in mm)
   * @param size size of the DRR in Pixels
   * @param spacing pixel spacing of the DRR in mm/Pixel
   * @param origin origin of the DRR plane in WCS (in mm)
   * @param orientation of the plane in WCS (3x3-matrix where columns move
   * fastest); normalized!
   * @return TRUE if successfully taken over
   **/
  bool SetRayCastingGeometryProps(double sourcePos[3], double size[2],
      double spacing[2], double origin[3], double orientation[9]);

  /**
   * Set flag indicating that the computed DRR should be rendered in render
   * window although the image was copied to LastDRR (image data) property
   **/
  vtkSetMacro(DoScreenRenderingThoughLastDRRImageCopied, bool)
  /**
   * Get flag indicating that the computed DRR should be rendered in render
   * window although the image was copied to LastDRR (image data) property
   **/
  vtkGetMacro(DoScreenRenderingThoughLastDRRImageCopied, bool)

  /** Set last generated DRR reference. DRRs will be copied into this image. **/
  vtkSetObjectMacro(LastDRR, vtkImageData)
  /** Get last generated DRR reference. DRRs will be copied into this image. **/
  vtkImageData *GetLastDRR()
  {
    return this->LastDRR;
  }

  /** Set renderer active camera. **/
  vtkSetObjectMacro(PlaneViewCamera, vtkCamera)
  /** Get renderer active camera. **/
  virtual vtkCamera *GetPlaneViewCamera()
  {
    return this->PlaneViewCamera;
  }

  /**
   * Set volume orientation transform (as VTK does not support image
   * orientation).
   **/
  vtkSetObjectMacro(OrientationTransform, vtkTransform)
  /** Get volume orientation transform. **/
  virtual vtkTransform *GetOrientationTransform()
  {
    return this->OrientationTransform;
  }

  /**
   * UNSUPPORTED - no reduction!
   * @see vtkGPUVolumeRayCastMapper::GetReductionRatio()
   */
  virtual void GetReductionRatio(double ratio[3])
  {
  }

  /**
   * Set / get DRR mask. The pixels which are not equal to zero are interpreted
   * as effective DRR pixels - the other pixels are not computed! NOTE: The mask
   * size must exactly fit the DRR size - otherwise the mask won't be
   * recognized and the complete DRR is computed (default). Preferred pixel type
   * is UCHAR.
   **/
  vtkSetObjectMacro(DRRMask, vtkImageData)
  vtkImageData *GetDRRMask()
  {
    return this->DRRMask;
  }

  /**
   * Set / get flag determining whether the DRR output should be flipped along
   * its vertical direction immediately after computation. This can be very
   * important for debug purposes.
   **/
  vtkSetMacro(VerticalFlip, bool)
  virtual bool GetVerticalFlip()
  {
    return this->VerticalFlip;
  }
  vtkBooleanMacro(VerticalFlip, bool)

  /**
   * Force the ray-caster to release ALL texture managers from its internal map
   * and to unbind the associated textures from GPU. This can be useful in some
   * situations (e.g. if the volume input changes - would be unnecessary
   * memory!).
   * @return true if any texture has been released
   **/
  virtual bool ReleaseGPUTextures();

  /**
   * Force the ray-caster to release ONE specified texture manager (the one that
   * is associated with textureImage) from its internal map and to unbind the
   * associated texture from GPU. This may be useful for unloading decided
   * textures (e.g. masks) from memory.
   * @param textureImage the VTK image that is associated to (and stored as)
   * a texture
   * @return true if the texture has been released
   */
  virtual bool ReleaseGPUTexture(vtkImageData *textureImage);

  /** @return the string that describes why DRR computation is not supported
   * (if there are more reasons, these will be separated by newlines) **/
  char *&GetDRRComputationNotSupportedReasons()
  {
    return DRRComputationNotSupportedReasons;
  }

  vtkSetMacro(UnsharpMasking, bool)
  vtkGetMacro(UnsharpMasking, bool)
  vtkBooleanMacro(UnsharpMasking, bool)

  vtkSetMacro(UnsharpMaskingRadius, int)
  vtkGetMacro(UnsharpMaskingRadius, int)  

  vtkSetMacro(UseMappingIDs, bool)
  vtkGetMacro(UseMappingIDs, bool)
  vtkBooleanMacro(UseMappingIDs, bool)

  /** Generate a new ID for the next mappingmapping process. */
  void GenerateNextMappingID();

protected:
  /**
   * amount of video memory to be used / available on GPU
   * (MaxMemoryInBytes * MaxMemoryFraction is the effectively available amount
   * of memory on the GPU)
   **/
  vtkIdType MaxMemoryInBytes;
  /**
   * maximum fraction of available video memory to be used on GPU
   * (MaxMemoryInBytes * MaxMemoryFraction is the effectively available amount
   * of memory on the GPU); DEFAULT: 0.75 (75% of video memory)
   **/
  double MaxMemoryFraction;
  /** flag which stores whether GLSL-based DRR computation is possible **/
  bool DRRComputationSupported;
  /** string that describes why DRR computation is not supported (if there are
   * more reasons, these will be separated by newlines) **/
  char *DRRComputationNotSupportedReasons;
  /** flag which stores whether openGL extensions have been loaded **/
  bool LoadedExtensions;
  /** render window required for openGL context **/
  vtkSmartPointer<vtkRenderWindow> RenderWindow;
  /** flag indicating whether float textures are supported **/
  bool SupportFloatTextures;
  /** flag indicating whether pixel buffer objects are supported **/
  bool SupportPixelBufferObjects;
  /** flag indicating whether the system has been initialized successfully **/
  bool SystemInitialized;
  /** flag indicating whether openGL and GLSL objects have been created **/
  bool GLSLAndOpenGLObjectsCreated;
  /** openGL frame buffer object **/
  unsigned int FrameBufferObject;
  /** openGL depth buffer object **/
  unsigned int DepthRenderBufferObject;
  /**
   * 3D scalar texture + 1D color + 2D grabbed depth buffer + 1 2D color buffer
   * texture objects
   */
  unsigned int TextureObjects[3];
  /** GLSL shader source: fragment shader program **/
  unsigned int ProgramShader;
  /** GLSL shader source: main **/
  unsigned int FragmentMainShader;
  /** GLSL shader source: perspective projection **/
  unsigned int FragmentProjectionShader;
  /** GLSL shader source: ray-tracing (ray-casting) **/
  unsigned int FragmentTraceShader;
  /** Last generated DRR (by ComputeDRR()) **/
  vtkSmartPointer<vtkImageData> LastDRR;
  /** internal clocks for time measurement **/
  vtkSmartPointer<vtkTimerLog> Clocks[3];
  /** time (ms) it took to compute last DRR (incl. conversion into image) **/
  double LastDRRComputationTime;
  /** time (ms) it took to pre-process last DRR **/
  double LastDRRPreProcessingTime;
  /** time (ms) it took to ray-cast last DRR **/
  double LastDRRRayCastingTime;
  /** time (ms) it took to post-process last DRR **/
  double LastDRRPostProcessingTime;
  /** time (ms) it took to transfer volume to GPU (short if already buffered) **/
  double LastVolumeTransferTime;
  /** time (ms) it took to transfer mask to GPU (short if already buffered) **/
  double LastMaskTransferTime;

  /** Input image data's scalar range (important for transfer functions) **/
  double ScalarRange[2];
  /** Intensity transfer function conversion unit (-> GL 1D texture) **/
  RGBTableManager *IntensityTFTable;
  /** Intensity transfer function (represented as RGB function) **/
  vtkSmartPointer<vtkColorTransferFunction> IntensityTF;
  /**
   * Intensity transfer function linear interpolation mode (otherwise nearest
   * neighbor)
   **/
  bool IntensityTFLinearInterpolation;
  /** flag indicating that program was built **/
  bool BuiltProgram;
  /**
   * Rescale slope s. <br>
   * <b>V'=V*s + i</b> <br>
   * V'...resultant DRR pixel value, V...original DRR pixel, s...rescale slope,
   * i...rescale intercept<br>
   * As DRRs are summation images, the resultant intensities in the DRRs
   * strongly depend on the input volume's voxel intensities and its size and
   * ray step size. In some situations, it may however be of interest to
   * linearly transform the DRR pixel intensities into another range. For
   * example the slope and intercept properties can be used for visualizing
   * DRRs (only intensities in the range from 0.0 to 1.0 can be directly
   * visualized when using an interactive mode).
   * @see RescaleIntercept
   */
  double RescaleSlope;
  /**
   * Rescale intercept i. <br>
   * <b>V'=V*s + i</b> <br>
   * V'...resultant DRR pixel value, V...original DRR pixel, s...rescale slope,
   * i...rescale intercept<br>
   * As DRRs are summation images, the resultant intensities in the DRRs
   * strongly depend on the input volume's voxel intensities and its size and
   * ray step size. In some situations, it may however be of interest to
   * linearly transform the DRR pixel intensities into another range. For
   * example the slope and intercept properties can be used for visualizing
   * DRRs (only intensities in the range from 0.0 to 1.0 can be directly
   * visualized when using an interactive mode).
   * @see RescaleSlope
   **/
  double RescaleIntercept;
  /** store frame buffer for offscreen mode **/
  unsigned int SavedFrameBuffer;
  /** camera-setup that is defining the DRR plane **/
  vtkSmartPointer<vtkCamera> PlaneViewCamera;
  /** helper matrices to prevent frequent instantiation/deletion **/
  vtkSmartPointer<vtkMatrix4x4> Matrices[3];
  /**
   * Transform defining a relative movement of the input volume outgoing
   * from its initial pose defined by the input volume's origin
   **/
  vtkSmartPointer<vtkTransform> Transform;
  /** Current volume transform matrix (initial pose + rel. transform) **/
  vtkSmartPointer<vtkMatrix4x4> CurrentVolumeMatrix;
  /** Helper transform to prevent frequent instantiation/deletion **/
  vtkSmartPointer<vtkTransform> HTransform;
  /** Helper transform to realize image orientation (volume user transform!) **/
  vtkSmartPointer<vtkTransform> OrientationTransform;
  /** Scalar field texture indexing **/
  ScalarFieldTextureMapper *ScalarsTextures;
  /** Current scalar field texture **/
  ScalarFieldManager *CurrentScalarFieldTexture;
  /** DRR mask scalar field texture **/
  ScalarFieldManager *DRRMaskScalarFieldTexture;
  /** DRR mask specified by user **/
  vtkSmartPointer<vtkImageData> DRRMask;
  /** Position of the ray-casting source (X-ray source) **/
  double RayCastSourcePosition[3];
  /** Pixel spacing of the DRR plane in mm/Pixel **/
  double DRRSpacing[2];
  /** Origin of the DRR plane in mm **/
  double DRROrigin[3];
  /** Actual (demanded) DRR size in Pixels **/
  int DRRSize[2];
  /** Last DRR size in Pixels **/
  int LastDRRSize[2];
  /** Plane orientation matrix of DRR (columns move fastest) **/
  double DRROrientation[9];
  /** DRR plane's corner points (emerging from origin, orientation, ...) **/
  double DRRCorners[4][3];
  /** Ray-casting source point projection on DRR plane **/
  double SourceOnPlane[3];
  /** Helper near plane for DRR frustum computation **/
  vtkSmartPointer<vtkPlane> DRRFrustNearPlane;
  /** Box source for clipping **/
  vtkSmartPointer<vtkTessellatedBoxSource> BoxSource;
  /** Planes for clipping **/
  vtkSmartPointer<vtkPlaneCollection> Planes;
  /** Near plane **/
  vtkSmartPointer<vtkPlane> NearPlane;
  /** Inverse volume matrix **/
  vtkSmartPointer<vtkMatrix4x4> InvVolumeMatrix;
  /** Convex poly data clipper **/
  vtkSmartPointer<vtkClipConvexPolyData> Clip;
  /** Poly data densifier **/
  vtkSmartPointer<vtkDensifyPolyData> Densify;
  /** Clipped poly data **/
  vtkSmartPointer<vtkPolyData> ClippedBoundingBox;
  /**
   * Flag indicating that the computed DRR should be rendered in render window
   * although the image was copied to LastDRR (image data) property
   **/
  bool DoScreenRenderingThoughLastDRRImageCopied;
  /** openGL projection matrix computed by ComputeGLProjectionMatrix() **/
  vtkSmartPointer<vtkMatrix4x4> GLProjectionMatrix;
  /** openGL projection matrix helper transformation **/
  vtkSmartPointer<vtkPerspectiveTransform> GLProjectionTransform;
  /**
   * Flag determining whether the DRR output should be flipped along its
   * vertical direction immediately after computation. This can be very
   * important for debug purposes!
   **/
  bool VerticalFlip;
  /** Flag determining whether the DRR output should be unsharp masked. **/
  bool UnsharpMasking;
  /** Radius for unsharp masking (auto if <= 0). **/
  int UnsharpMaskingRadius;
  /** Helper variable for preventing the mapper from rendering the same scene
   * multiple times. ID of last mapping process. */
  int LastMappingID;
  /** Helper variable for preventing the mapper from rendering the same scene
   * multiple times. ID of current mapping process. */
  int CurrentMappingID;
  /** Flag indicating wehther mapping IDs are used or not. */
  bool UseMappingIDs;

  /** Default constructor. **/
  GLSLDRRRayCastMapper();
  /** Destructor **/
  virtual ~GLSLDRRRayCastMapper();

  /**
   * Compute internal GL projection matrix by taking current properties
   * (PlaneViewCamera ...) into account.
   * @see GLProjectionMatrix
   */
  void ComputeGLProjectionMatrix();

  /**
   * Automatically detects the available dedicated video / shared system memory
   * on first GPU. It sets the class properties. If the video memory size cannot
   * be determined, some default (128MB) is assumed. <br>
   * NOTE: this feature requires VTK compiled with VTK_USE_NVCONTROL=ON option
   * on LINUX-systems to successfully gain access to the GPU info list.
   **/
  void AutoDetectVideoMemory();

  /**
   * Try to load the required openGL extensions for GLSL-based ray-casting.
   * @return TRUE if successful
   */
  bool LoadExtensions();

  /**
   * Create the required openGL and GLSL objects required later for rendering.
   * @return TRUE if successful
   */
  bool CreateGLSLAndOpenGLObjects();

  /**
   * Verify compilation of the specified shader and print errors if there are
   * problems.
   * @param shader shader ID
   * @return TRUE if compilation is OK
   */
  bool VerifyCompilation(unsigned int shader);
  /**
   * Verify linkage of the specified shader and print errors if there are
   * problems.
   * @param shader shader ID
   * @return TRUE if linkage is OK
   */
  bool VerifyLinkage(unsigned int shader);

  /**
   * Pre-processing before real DRR-computation: <br>
   * - system (openGL/GLSL) initialization is necessary <br>
   * - frame buffer / texture allocation on demand <br>
   * - scalar range analysis <br>
   * - intensity transfer function update on demand <br>
   * - GLSL program (re-)compilation/linking <br>
   * - texture / frame buffer initialization and preparation <br>
   * @return TRUE if successful (-> can resume with real DRR-computation)
   * @see RayCasting()
   */
  bool PreProcessing();

  /**
   * Real DRR-computation (ray-casting): <br>
   * - transfer of input volume to GPU (if changed / necessary) <br>
   * - geometry initialization (camera, GLSL variables ...) <br>
   * - clipping box computation <br>
   * - conversion of geometry and GLSL-based rendering <br>
   * NOTE: can only be executed if preceding pre-processing was successful!
   * @return TRUE if successful (-> proceed with post-processing)
   * @see PreProcessing()
   * @see PostProcessing()
   */
  bool RayCasting();

  /**
   * Post-processing after real DRR-computation: <br>
   * - unbind textures <br>
   * - clean up rendering (remap frame buffer ...) <br>
   * - extract image from frame buffer <br>
   * NOTE: can only be executed if preceding ray-casting was successful!
   * @return TRUE if successful
   * @see RayCasting()
   */
  bool PostProcessing();

  /**
   * Allocate the frame buffers for DRR ray-casting if DRR size has changed
   * since last computation.
   */
  bool AllocateFrameBuffers();

  /**
   * Update intensity transfer function table from intensity transfer function.
   **/
  bool UpdateIntensityTransferFunction();

  /**
   * Build fragment shader program.
   * @return TRUE if successful
   */
  bool BuildProgram();

  /** Initialize the ray-caster's textures and variables. **/
  bool InitializeTexturesAndVariables();

  /** Initialize frame buffers. **/
  bool InitializeFrameBuffers();

  /** Prepare rendering in offscreen frame buffer. **/
  bool PrepareOffscreenFrameBufferRendering();

  /**
   * Compute current volume transformation matrix composed of input volume's
   * initial pose and the relative transformation. Result is stored in
   * CurrentVolumeMatrix.
   **/
  void ComputeVolumeTransformMatrix();

  /**
   * Transfer the input volume to GPU and initialize the according GLSL
   * variables. NOTE: this method checks whether the volume is already on the
   * GPU (-> performance!). Furthermore the input volume must fit on the GPU
   * video memory as a whole - we do not support streaming at the moment!
   **/
  bool TransferVolumeToGPU();

  /**
   * Transfer the DRR mask to GPU (if a mask is specified) and initialize the
   * according GLSL variables. NOTE: this method checks whether the mask is
   * already on the GPU (->performance!).
   */
  bool TransferMaskToGPU();

  /** Initialize DRR geometry (focal point, perspective transform ...). **/
  bool InitializeGeometry();

  /**
   * Clip input volume bounding box with near and far planes. Grab the output
   * poly data for rendering later.
   **/
  bool ClipBoundingBox();

  /** Convert clipped poly data and render the volume (ray-casting). **/
  bool ConvertAndRender();

  /** Unbindes a some texture objects after rendering. **/
  bool UnbindTextures();

  /** Clean-up some openGL things after rendering. **/
  bool CleanUpRender();

  /** Copy resultant (computed) texture to an image and/or to screen. **/
  bool CopyAndOrScreenRenderTexture();

  /** Release graphics resources when no longer needed. **/
  void ReleaseGraphicsResources();

  /** @see vtkGPUVolumeRayCastMapper#PreRender() **/
  virtual void PreRender(vtkRenderer *ren, vtkVolume *vol,
      double datasetBounds[6], double scalarRange[2],
      int numberOfScalarComponents, unsigned int numberOfLevels);

  /** @see vtkGPUVolumeRayCastMapper#RenderBlock() **/
  virtual void
      RenderBlock(vtkRenderer *ren, vtkVolume *vol, unsigned int level);

  /** @see vtkGPUVolumeRayCastMapper#PostRender() **/
  virtual void PostRender(vtkRenderer *ren, int numberOfScalarComponents);

  /** @see vtkGPUVolumeRayCastMapper#GPURender() **/
  virtual void GPURender(vtkRenderer *ren, vtkVolume *vol);

private:
  /** Purposely not implemented. **/
  GLSLDRRRayCastMapper(const GLSLDRRRayCastMapper &);
  /** Purposely not implemented. **/
  void operator=(const GLSLDRRRayCastMapper &);

  // optional development routines:
PRINT_UNIFORM_VARIABLES_F
GL_ERROR_TO_STRING_F
PRINT_GL_ERRORS_F
CHECK_FB_STATUS_F
BUFF_TO_STRING_F
DISPLAY_BUFFERS_F
DISPLAY_FB_ATTACHMENTS_F
DISPLAY_FB_ATTACHMENT_F

};

}

#endif /* ORAGLSLDRRRAYCASTMAPPER_H_ */
