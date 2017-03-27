//

#ifndef ORAITKVTKDRRFILTER_H_
#define ORAITKVTKDRRFILTER_H_

#include <itkImageToImageFilter.h>
#include <itkConceptChecking.h>
#include <itkMatrixOffsetTransformBase.h>
#include <itkMacro.h>
#include <itkSimpleFastMutexLock.h>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkColorTransferFunction.h>
#include <vtkTransform.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include "oraGLSLDRRRayCastMapper.h"
#include "oraITKVTKLinearTransformConnector.h"
#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
  #include "oraInvisibleXOpenGLRenderWindow.h"
#endif

#include <vector>
#include <map>

namespace ora
{

/** Macro for setting a 'geometry' attribute. **/
#define oraSetGeometryMacro(name, type) \
  virtual void Set##name(const type _arg) \
  { \
    if (this->m_##name != _arg) \
      { \
      this->m_##name = _arg; \
      this->ApplyGeometrySettings(); \
      this->Modified(); \
      } \
  }

/** \class ITKVTKDRRFilter
 * \brief Implements computation of digitally reconstructed radiographs (DRR) by
 * involving GPU shaders on the graphics card (openGL shading language, GLSL).
 *
 * Both images, the input image (volume) as well as the output image
 * (DRR), of this filter are assumed to be represented as 3D images. The DRR
 * is single-sliced.
 *
 * The filter is templated over the input pixel type and the output pixel type.
 *
 * In order to compute the DRR GLSL-based by ray-casting an internal openGL
 * context is required. Internally an own render window is created which is
 * used for that. Currently, on Linux system the render window (small) will be
 * visible during computation due to some VTK-insufficiencies (although off-
 * screen rendering is used).
 *
 * In addition an interactive mode is available where the generated DRRs are
 * directly visualized in the openGL context (VTK render window). Have a look
 * at the documentation of Interactive property of this class.
 *
 * So-called independent outputs are implemented. This means that the filter can
 * manage more than one output. DRR-computation refers to a specific output
 * while the others are unmodified. This can be used for 2D/3D-registration with
 * multiple images where DRRs with different projection geometry settings are
 * required. For performance it is better to compute the DRRs for registration
 * sequentially in order to avoid alternating expensive volume streaming to the
 * GPU.
 *
 * Moreover, this class is capable of defining DRR masks for each independent
 * output. These optional masks define whether or not a DRR pixel should be
 * computed (value greater than 0). This may especially be useful for stochastic
 * metric evaluations where only a subset of the DRR pixels is really used.
 *
 * In addition, the computed DRRs can be linearly rescaled on the fly; i.e.
 * applying a rescale slope and/or intercept to the DRRs directly on the GPU.<br>
 * NOTE: If you use masks and rescaling: the masked regions would get a value
 * of 0 (no absorption). However, the rescale intercept may change the masked
 * pixel values though!
 *
 * Have a look on the well-documented class members and methods to find out
 * how to configure this class and how to setup the desired DRR geometry.
 * 
 *
 * @see itk::ImageToImageFilter
 * @see ora::GLSLDRRRayCastMapper
 *
 * @author phil 
 * @version 2.6
 *
 * \ingroup ImageFilters
 */
template<class TInputPixelType, class TOutputPixelType>
class ITKVTKDRRFilter:
    public itk::ImageToImageFilter<itk::Image<TInputPixelType, 3>, itk::Image<
        TOutputPixelType, 3> >
{
  /*
   TRANSLATOR ora::ITKVTKDRRFilter
   */

public:
  /** Standard class typedefs. */
  typedef ITKVTKDRRFilter Self;
  typedef itk::ImageToImageFilter<itk::Image<TInputPixelType, 3>, itk::Image<
      TOutputPixelType, 3> > Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Accessibility typedefs. **/
  typedef itk::Image<TInputPixelType, 3> InputImageType;
  typedef typename InputImageType::Pointer InputImagePointer;
  typedef typename InputImageType::ConstPointer InputImageConstPointer;
  typedef typename InputImageType::PixelType InputImagePixelType;
  typedef itk::Image<TOutputPixelType, 3> OutputImageType;
  typedef typename OutputImageType::Pointer OutputImagePointer;
  typedef typename OutputImageType::ConstPointer OutputImageConstPointer;
  typedef typename OutputImageType::PixelType OutputImagePixelType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef unsigned char MaskPixelType;
  typedef itk::Image<MaskPixelType, 3> MaskImageType;
  typedef MaskImageType::Pointer MaskImagePointer;
  typedef itk::MatrixOffsetTransformBase<double,
      itkGetStaticConstMacro(InputImageDimension),
      itkGetStaticConstMacro(InputImageDimension)> TransformType;
  typedef typename TransformType::Pointer TransformPointer;
  typedef itk::Point<double, 3> PointType;
  typedef itk::Matrix<double, 3, 3> MatrixType;
  typedef itk::Size<2> SizeType;
  typedef itk::FixedArray<double, 2> SpacingType;
  typedef vtkSmartPointer<vtkImageData> VTKInputImagePointer;
  typedef vtkSmartPointer<vtkImageData> VTKOutputImagePointer;
  typedef vtkSmartPointer<vtkTransform> VTKTransformPointer;
  typedef vtkSmartPointer<vtkColorTransferFunction> TransferFunctionPointer;
  typedef itk::Array<double> TransferFunctionSpecificationType;
  typedef itk::FixedArray<int, 2> DiscretePositionType;
  typedef vtkSmartPointer<vtkRenderWindow> RenderWindowPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(Self, Superclass)
  ;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)
  ;

  /**
   * Set the input image (3D volume) from ITK image data. Internally the ITK
   * image's pipeline is connected to the VTK pipeline.
   * @param input the ITK image data set to be used for DRR computation
   * @see UpdateOrientationTransformation()
   */
  virtual void SetInput(InputImagePointer input);

  /**
   * As image orientation in space cannot be handled by VTK, it must be
   * imitated by a user transform. This method updates this transform w.r.t.
   * the current ITK input data set (ITK certainly supports image orientation).
   * This method is automatically called after each SetInput(). However, if
   * the image orientation or origin of the input changes, the user is
   * responsible for calling this method!!!
   * @see SetInput()
   */
  virtual void UpdateOrientationTransformation();

  /**
   * Set the input image (3D volume) directly from VTK image data.
   * Internally the VTK image will be connected to the filter's input via a
   * pipeline connection.
   * @param input the VTK image data set to be used for DRR computation
   */
  virtual void SetVTKInput(VTKInputImagePointer input);

  /**
   * Generate information describing the output data.
   * This filter produces an image with a different size than its input image.
   * @see itk::ProcessObject#GenerateOutputInformaton()
   */
  virtual void GenerateOutputInformation();

  /** Generate the information describing the requested input region in the
   * input image. Need a different input requested region than output requested
   * region.
   * @see itk::ImageToImageFilter#GenerateInputRequestedRegion()
   * @see itk::ImageSource#GenerateInputRequestedRegion()
   * @see itk::ProcessObject#GenerateInputRequestedRegion()
   * */
  virtual void GenerateInputRequestedRegion();

  /**
   * @return object's modified time depending on the modified times of its
   * internal components.
   * @see itk::Object#GetMTime()
   */
  virtual unsigned long GetMTime(void) const;

  /** Set DRR source focal spot position in WCS. **/
  oraSetGeometryMacro(SourceFocalSpotPosition, PointType)
  /** Get DRR source focal spot position in WCS. **/
  itkGetMacro(SourceFocalSpotPosition, PointType)

  /** Set DRR plane origin in WCS. **/
  oraSetGeometryMacro(DRRPlaneOrigin, PointType)
  /** Get DRR plane origin in WCS. **/
  itkGetMacro(DRRPlaneOrigin, PointType)

  /**
   * Set DRR plane orientation (first row of matrix defines normalized direction
   * of the DRR row, second row of matrix defines normalized direction of the
   * DRR column, third row is the normalized DRR plane normal towards the
   * focal point); this matrix is expected to be an orthogonal matrix.
   **/
  virtual void SetDRRPlaneOrientation(const MatrixType matrix);
  /**
   * Get DRR plane orientation (first row of matrix defines normalized direction
   * of the DRR row, second row of matrix defines normalized direction of the
   * DRR column, third row is the normalized DRR plane normal towards the
   * focal point).
   **/
  itkGetMacro(DRRPlaneOrientation, MatrixType)

  /** Set DRR size in pixels (row-direction, column-direction) **/
  oraSetGeometryMacro(DRRSize, SizeType)
  /** Get DRR size in pixels (row-direction, column-direction) **/
  itkGetMacro(DRRSize, SizeType)

  /** Set DRR spacing (spacing along row-dir., spacing along column-dir.) **/
  oraSetGeometryMacro(DRRSpacing, SpacingType)
  /** Get DRR spacing (spacing along row-dir., spacing along column-dir.) **/
  itkGetMacro(DRRSpacing, SpacingType)

  /** @return TRUE if current geometry settings are valid, FALSE otherwise **/
  virtual bool IsGeometryValid();

  /**
   * Set current volume transformation (ITK representation). This transformation
   * will internally be connected to the underlying VTK tranformation. Both will
   * represent the same linear transformation.
   **/
  virtual void SetTransform(TransformType *transform);
  /** Get current volume transformation (ITK representation) **/
  itkGetObjectMacro(Transform, TransformType)

  /** Get current volume transformation (VTK representation) **/
  virtual VTKTransformPointer GetVTKTransform()
  {
    return this->m_VTKTransform;
  }

  /** Get volume orientatino transformation (VTK representation) **/
  virtual VTKTransformPointer GetOrientationVTKTransform()
  {
    return this->m_OrientationVTKTransform;
  }

  /**
   * Set the intensity transfer function that maps volume intensities to
   * output intensities that contribute to DRR summation.
   * @param tf a flat array of function value pairs where the first values
   * specify the volume intensities and successive values specify the according
   * output intensities (therefore, the number of array elements must be even);
   * these 'points' are supporting points for a linearly interpolated transfer
   * function
   **/
  virtual void SetIntensityTransferFunction(
      TransferFunctionSpecificationType tf);
  /**
   * Get the intensity transfer function in flat representation.
   * @see SetIntensityTransferFunction()
   **/
  virtual TransferFunctionSpecificationType GetIntensityTransferFunction();
  /**
   * Get direct pointer to the internal transfer function in VTK style.
   * (NOTE: this object's Modified()-method calls should take place after
   * modifying the transfer function)
   **/
  virtual TransferFunctionPointer GetInternalIntensityTransferFunction()
  {
    return this->m_IntensityTF;
  }

  /** Set sampling distance along the casted rays **/
  virtual void SetSampleDistance(float distance);
  /** Get sampling distance along the casted rays **/
  itkGetMacro(SampleDistance, float)

  /**
   * Set override value for GPU's dedicated video memory size in MB (applied if
   * >0MB)
   **/
  virtual void SetOverrideVideoMemSizeMB(int videoMemSize);
  /**
   * Get override value for GPU's dedicated video memory size in MB (applied if
   * >0MB)
   **/
  itkGetMacro(OverrideVideoMemSizeMB, int)

  /**
   * Set override value for maximum fraction of GPU video memory to be used by
   * the ray-caster (applied if within [0.1;1.0])
   */
  virtual void SetOverrideMaxVideoMemFraction(float maxFraction);
  /**
   * Get override value for maximum fraction of GPU video memory to be used by
   * the ray-caster (applied if within [0.1;1.0])
   */
  itkGetMacro(OverrideMaxVideoMemFraction, float)

  /**
   * Returns the maximum size of GPU video memory [in MB] which can effectively
   * be used (corresponds to maximum internal volume size that does not require
   * streaming of the volume). This method already takes care of set override
   * values and maximum fractions! <br>
   * NOTE: this method requires the internal pipeline to be built!
   */
  virtual int GetEffectiveMaximumGPUMemory();

  /**
   * Set flag indicating whether or not interactive rendering should be
   * activated; interactive means that the generated DRR is immediately
   * visualized within the openGL context (VTK render window); internally off-
   * screen rendering must be deactivated for interactive mode and rendering the
   * image will require additional time and therefore slow-down DRR-generation;
   * this mode is meant for demonstration purposes only; NOTE: activating this
   * flag automatically requires the openGL context (VTK render window) to have
   * the same size as the generated DRR which will limit the maximum DRR size in
   * pixels (screen resolution dependent!). Moreover, note that this property
   * requires a call to BuildRenderPipeline() to show effect!
   * @see SetRescaleSlope()
   * @see SetRescaleIntercept()
   * @see SetContextTitle()
   * @see SetContextPosition()
   * @see SetCopyDRRToImageOutput()
   * @see BuildRenderPipeline()
   **/
  virtual void SetInteractive(bool interactive);
  /** Get flag indicating whether or not interactive rendering is activated. **/
  itkGetMacro(Interactive, bool)
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
   * @see m_RescaleIntercept
   */
  virtual void SetRescaleSlope(double slope);
  /**
   * Get rescale slope s.
   * @see m_RescaleIntercept
   */
  itkGetMacro(RescaleSlope, double)
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
   * @see m_RescaleSlope
   **/
  virtual void SetRescaleIntercept(double intercept);
  /**
   * Get rescale intercept i.
   * @see m_RescaleSlope
   */
  itkGetMacro(RescaleIntercept, double)
  /**
   * Set title of openGL context (VTK render window) to be displayed (especially
   * interesting for interactive mode when the context is visible
   * @see SetInteractive()
   */
  virtual void SetContextTitle(std::string title);
  /** Get title of openGL context (VTK render window) to be displayed. **/
  itkGetMacro(ContextTitle, std::string)
  /**
   * Set position of the openGL context (VTK render window) on screen;
   * interesting for interactive mode when the context is visible; however this
   * property is also considered if Interactive property is not activated as on
   * Linux systems the context (with minimal size) is displayed though off-
   * screen rendering is activated (bug)
   * @see SetInteractive()
   */
  virtual void SetContextPosition(DiscretePositionType pos);
  /** Set position of the openGL context (VTK render window) on screen. **/
  itkGetMacro(ContextPosition, DiscretePositionType)
  /**
   * Set flag controlling whether or not the generated DRR is copied to this
   * filter's image output; it could make sense if Interactive property is
   * activated, otherwise this flag is automatically forced to equal TRUE as it
   * would not make sense to render a DRR without displaying or copying it
   * @see SetInteractive()
   */
  virtual void SetCopyDRRToImageOutput(bool copy);
  /**
   * Get flag controlling whether or not the generated DRR is copied to this
   * filter's image output.
   */
  itkGetMacro(CopyDRRToImageOutput, bool)

  /**
   * Set number of independent DRR outputs. This means that there can be 
   * multiple DRR outputs which are independent from each other. Depending on 
   * the value of CurrentDRROutputIndex successive DRR computations will be
   * related to the specified output.
   * @see SetCurrentDRROutputIndex()
   */
  virtual void SetNumberOfIndependentOutputs(int numOutputs);
  /** Get number of independent DRR outputs. **/
  itkGetMacro(NumberOfIndependentOutputs, int)
  /**
   * Set index that determines to which DRR output successive DRR computations 
   * will relate. Range: 0 .. m_NumberOfIndependentOutputs-1
   * @see SetNumberOfIndependentOutputs()
   */
  virtual void SetCurrentDRROutputIndex(int currIndex);
  /**
   * Get index that determines to which DRR output successive DRR computations 
   * will relate.
   */
  itkGetMacro(CurrentDRROutputIndex, int)

  /**
   * Set an optional DRR mask for the current output index. NOTE: The mask is
   * required to match the size of the DRR (in pixels) exactly in order to be
   * considered!
   * @param mask the mask image (pixels having values >0 will be computed)
   **/
  virtual void SetDRRMask(MaskImagePointer mask);
  /** Get the optional DRR mask for the current output index. **/
  virtual MaskImagePointer GetDRRMask();

  /**
   * Time measurements of last DRR computation (mainly for debugging). The
   * following measurements are returned (by reference): <br>
   * @param volumeTransfer transfer of volume to GPU
   * @param maskTransfer transfer of optional DRR mask to GPU
   * @param drrComputation total DRR computation
   * @param preProcessing DRR computation (pre-processing part)
   * @param rayCasting DRR computation (ray-casting part)
   * @param postProcessing DRR computation (post-processing part)
   **/
  virtual void GetTimeMeasuresOfLastComputation(double &volumeTransfer,
      double &maskTransfer, double &drrComputation, double &preProcessing,
      double &rayCasting, double &postProcessing);

  /**
   * Set convenience flag indicating that the GetMTime()-method should NOT
   * consider the transformations' (volume AND orientation transformation)
   * MTimes. This could be useful if this class is integrated with more complex
   * frameworks that guarantee that the Superclass' MTime is modified if the
   * output should be updated! DEFAULT: FALSE (suitable for ordinary usage).
   */
  itkSetMacro(WeakMTimeBehavior, bool)
  itkGetMacro(WeakMTimeBehavior, bool)
  itkBooleanMacro(WeakMTimeBehavior)

  /**
   * Set external, custom render window (to show effect, BuildRenderPipeline()
   * must be called).
   **/
  itkSetMacro(ExternalRenderWindow, RenderWindowPointer)
  /** Get external, custom render window. **/
  itkGetMacro(ExternalRenderWindow, RenderWindowPointer)

  /** Build the internal VTK rendering pipeline for DRR computation. **/
  virtual void BuildRenderPipeline();

  /** Get VTK-event-object for Start/EndEvent watching.
   * @see SetFireStartEndEvents() **/
  vtkObject *GetEventObject()
  {
    return m_EventObject;
  }
  /** Set flag indicating whether or not VTK StartEvent and EndEvent are thrown
   * before each call to MakeCurrent() and Render(). This is especially useful
   * if this filter is used in a GUI-framework. The events are invoked on the
   * EventObject.
   * @see GetEventObject() **/
  virtual void SetFireStartEndEvents(bool fire)
  {
    m_FireStartEndEvents = fire;
  }
  /** Get flag indicating whether or not VTK StartEvent and EndEvent are thrown
   * before each call to MakeCurrent() and Render(). This is especially useful
   * if this filter is used in a GUI-framework. The events are invoked on the
   * EventObject.
   * @see GetEventObject() **/
  virtual bool GetFireStartEndEvents()
  {
    return m_FireStartEndEvents;
  }

  /** Get flag indicating that the filter should not try to resize the connected
   * render window to size (1,1) if Interactive==FALSE. */
  bool GetDoNotTryToResizeNonInteractiveWindows()
  {
    return m_DoNotTryToResizeNonInteractiveWindows;
  }
  /** Get flag indicating that the filter should not try to resize the connected
   * render window to size (1,1) if Interactive==FALSE. */
  void SetDoNotTryToResizeNonInteractiveWindows(bool flag)
  {
    m_DoNotTryToResizeNonInteractiveWindows = flag;
  }

  /** Concept checking */
#ifdef ITK_USE_CONCEPT_CHECKING
  /** input image type must have numeric pixel type **/
  itkConceptMacro(InputHasNumericTraitsCheck,
      (itk::Concept::HasNumericTraits<InputImagePixelType>));
  /** output image type must have numeric pixel type **/
  itkConceptMacro(OutputHasNumericTraitsCheck,
      (itk::Concept::HasNumericTraits<OutputImagePixelType>));
#endif

protected:
  /** internal types **/
  typedef vtkSmartPointer<vtkRenderer> RendererPointer;
  typedef GLSLDRRRayCastMapper GPURayCasterType;
  typedef vtkSmartPointer<GPURayCasterType> GPURayCasterPointer;
  typedef vtkSmartPointer<vtkCamera> CameraPointer;
  typedef ora::ITKVTKLinearTransformConnector TransformConnectorType;
  typedef TransformConnectorType::Pointer TransformConnectorPointer;
  typedef vtkSmartPointer<vtkCallbackCommand> CommandPointer;
  typedef vtkSmartPointer<vtkRenderWindowInteractor> InteractorPointer;
  typedef itk::SimpleFastMutexLock MutexType;
#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
  typedef vtkSmartPointer<InvisibleXOpenGLRenderWindow> InternalRenderWindowPointer;
#else
  typedef RenderWindowPointer InternalRenderWindowPointer;
#endif

  /** internal VTK input image data **/
  VTKInputImagePointer m_VTKInput;
  /** DRR source focal spot position in WCS **/
  PointType m_SourceFocalSpotPosition;
  /** DRR plane origin in WCS **/
  PointType m_DRRPlaneOrigin;
  /**
   * DRR plane orientation (first row of matrix defines normalized direction
   * of the DRR row, second row of matrix defines normalized direction of the
   * DRR column, third row is the normalized DRR plane normal towards the
   * focal point); this matrix is expected to be an orthogonal matrix.
   **/
  MatrixType m_DRRPlaneOrientation;
  /** Helper: rotated current plane orientation **/
  MatrixType m_DRRPlaneOrientationRotated;
  /** DRR size in pixels (row-direction, column-direction) **/
  SizeType m_DRRSize;
  /** DRR spacing (spacing along row-direction, spacing along column-dir.) **/
  SpacingType m_DRRSpacing;
  /** flag indicating whether current geometry settings are valid **/
  bool m_GeometryIsValid;
  /** Current volume transformation (ITK representation) **/
  TransformPointer m_Transform;
  /** Current volume transformation (VTK representation) **/
  VTKTransformPointer m_VTKTransform;
  /** Volume orientation transformation (VTK representation) **/
  VTKTransformPointer m_OrientationVTKTransform;
  /** vector of internal independent output images in VTK-format**/
  std::vector<VTKOutputImagePointer> m_VTKOutputImages;
  /** vector of internal DRR mask images in VTK-format **/
  std::vector<VTKOutputImagePointer> m_VTKDRRMasks;
  /** mapping between ITK masks and the internal VTK representation **/
  std::map<vtkImageData*, MaskImagePointer> m_VTKToITKMaskMap;
  /** VTK-base render pipeline: renderer **/
  RendererPointer m_Renderer;
  /** VTK-base render pipeline: render window **/
  RenderWindowPointer m_RenderWindow;
  /** External, custom render window **/
  RenderWindowPointer m_ExternalRenderWindow;
  /** VTK-base render pipeline: camera **/
  CameraPointer m_Camera;
  /** internal ray caster component which really computes DRRs **/
  GPURayCasterPointer m_GPURayCaster;
  /** internal transfer function for volume intensity mapping **/
  TransferFunctionPointer m_IntensityTF;
  /** sampling distance along the casted rays **/
  float m_SampleDistance;
  /** helper connecting an ITK and a VTK transform **/
  TransformConnectorPointer m_TransformConnector;
  /**
   * override value for GPU's dedicated video memory size in MB (applied if
   * >0MB)
   **/
  int m_OverrideVideoMemSizeMB;
  /**
   * override value for maximum fraction of GPU video memory to be used by the
   * ray-caster (applied if within [0.1;1.0])
   */
  float m_OverrideMaxVideoMemFraction;
  /**
   * flag indicating whether or not interactive rendering should be activated;
   * interactive means that the generated DRR is immediately visualized within
   * the openGL context (VTK render window); internally off-screen rendering
   * must be deactivated for interactive mode and rendering the image will
   * require additional time and therefore slow-down DRR-generation; this mode
   * is meant for demonstration purposes only; NOTE: activating this flag
   * automatically requires the openGL context (VTK render window) to have the
   * same size as the generated DRR which will limit the maximum DRR size in
   * pixels (screen resolution dependent!)
   * @see m_RescaleSlope
   * @see m_RescaleIntercept
   * @see m_ContextTitle
   * @see m_ContextPosition
   * @see m_CopyDRRToImageOutput
   **/
  bool m_Interactive;
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
   * @see m_RescaleIntercept
   */
  double m_RescaleSlope;
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
   * @see m_RescaleSlope
   **/
  double m_RescaleIntercept;
  /**
   * Title of openGL context (VTK render window) to be displayed (especially
   * interesting for interactive mode when the context is visible
   * @see m_Interactive
   */
  std::string m_ContextTitle;
  /**
   * Position of the openGL context (VTK render window) on screen; interesting
   * for interactive mode when the context is visible; however this property
   * is also considered if Interactive property is not activated as on Linux
   * systems the context (with minimal size) is displayed though off-screen
   * rendering is activated (bug)
   * @see m_Interactive
   */
  DiscretePositionType m_ContextPosition;
  /**
   * flag controlling whether or not the generated DRR is copied to this
   * filter's image output; it could make sense if Interactive property is
   * activated, otherwise this flag is automatically forced to equal TRUE as it
   * would not make sense to render a DRR without displaying or copying it
   * @see m_Interactive
   */
  bool m_CopyDRRToImageOutput;
  /**
   * Number of independent DRR outputs. This means that there can be multiple
   * DRR outputs which are independent from each other. Depending on the value 
   * of m_CurrentDRROutputIndex the DRR computation is related to the specified
   * output.
   * @see m_CurrentDRROutputIndex
   */
  int m_NumberOfIndependentOutputs;
  /**
   * Determines to which DRR output successive DRR computations will relate.
   * Range: 0 .. m_NumberOfIndependentOutputs-1
   * @see m_NumberOfIndependentOutputs
   */
  int m_CurrentDRROutputIndex;
  /**
   * A convenience flag indicating that the GetMTime()-method should NOT
   * consider the transformations' (volume AND orientation transformation)
   * MTimes. This could be useful if this class is integrated with more complex
   * frameworks that guarantee that the Superclass' MTime is modified if the
   * output should be updated! DEFAULT: FALSE (suitable for ordinary usage).
   */
  bool m_WeakMTimeBehavior;
  /** VTK-event-object for Start/EndEvents.
   * @see m_FireStartEndEvents **/
  vtkObject *m_EventObject;
  /** Flag indicating whether or not VTK StartEvent and EndEvent are thrown
   * before each call to MakeCurrent() and Render(). This is especially useful
   * if this filter is used in a GUI-framework. The events are invoked on the
   * EventObject.
   * @see m_EventObject **/
  bool m_FireStartEndEvents;
  /** Flag indicating that the filter should not try to resize the connected
   * render window to size (1,1) if Interactive==FALSE. */
  bool m_DoNotTryToResizeNonInteractiveWindows;

  /** Default constructor. **/
  ITKVTKDRRFilter();
  /** Default destructor. **/
  virtual ~ITKVTKDRRFilter();

  /** Print description of this object. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Can only operate on complete output region.
   * @see itk::ProcessObject#EnlargeOutputRequestedRegion()
   */
  virtual void EnlargeOutputRequestedRegion(itk::DataObject *data);

  /**
   * In general apply individual requested region to each of the outputs.
   * @see itk::ProcessObject#GenerateOutputRequestedRegion()
   */
  virtual void GenerateOutputRequestedRegion(itk::DataObject *data);

  /**
   * Connect an ITK image (output type) instance to an existing VTK image
   * object by connecting their pipeline.
   * @param vtkImage an existing VTK image object
   * @return the ITK image object (output type) if successful, NULL otherwise
   */
  virtual OutputImagePointer ConnectOutputITKImageToVTKImage(
      const VTKOutputImagePointer vtkImage);
  /**
   * Connect an ITK image (input type) instance to an existing VTK image
   * object by connecting their pipeline.
   * @param vtkImage an existing VTK image object
   * @return the ITK image object (input type) if successful, NULL otherwise
   */
  virtual InputImagePointer ConnectInputITKImageToVTKImage(
      const VTKOutputImagePointer vtkImage);

  /**
   * Connect a VTK image instance to an existing ITK image object by connecting
   * their pipeline (input image type).
   * @param itkImage an existing ITK image object (input image type)
   * @return the VTK image object if successful, NULL otherwise
   */
  virtual VTKInputImagePointer ConnectVTKImageToITKInputImage(
      InputImageConstPointer itkImage);
  /**
   * Connect a VTK image instance to an existing ITK image object by connecting
   * their pipeline (mask image type).
   * @param itkImage an existing ITK image object (mask image type)
   * @return the VTK image object if successful, NULL otherwise
   */
  virtual VTKInputImagePointer ConnectVTKImageToITKMaskImage(
      MaskImagePointer itkImage);

  /**
   * Apply the set geometry settings (e.g. DRR size, DRR plane orientation ...)
   * to the internal components. (Required before valid output information
   * can be generated).
   * @return TRUE if current geometry is completely valid (not including whether
   * current input is valid)
   **/
  virtual bool ApplyGeometrySettings();

  /** Destroy the internal VTK rendering pipeline for DRR computation. **/
  virtual void DestroyRenderPipeline();

  /** @return TRUE if the specified matrix is orthogonal **/
  bool IsOrthogonalMatrix(const MatrixType &matrix) const;

  /**
   * Initiate the real DRR computation. Internally this computation is
   * excessively parallized on the graphics card (GPU). Therefore, we do not
   * need to split the computation within this class.
   * @see itk::ProcessObject#GenerateData()
   **/
  virtual void GenerateData();

private:
  /** Purposely not implemented. **/
  ITKVTKDRRFilter(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraITKVTKDRRFilter.txx"

#endif /* ORAITKVTKDRRFILTER_H_ */
