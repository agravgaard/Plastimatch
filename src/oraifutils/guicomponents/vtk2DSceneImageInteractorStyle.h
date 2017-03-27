//
#ifndef VTK2DSCENEIMAGEINTERACTORSTYLE_H_
#define VTK2DSCENEIMAGEINTERACTORSTYLE_H_

#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkActor2D.h>
#include <vtkImageMapper.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkImageMapToColors.h>
#include <vtkSmartPointer.h>
#include <vtkCommand.h>

#include <vector>

// forward declaration:
class vtkRenderWindow;

// further motion flags:
#define VTKIS_CURSOR 50

/**
 * Interactor style dedicated for usage with 2D scene pipelines that are based
 * on vtkActor2D and vtkImageMapper. It enables basic zooming, panning, image
 * fitting (adapting image zoom to viewport), flipping along x/y axes, and
 * color windowing.
 *
 * The following external component setup is assumed:<br>
 * An input image (or a specified image filter output) is connected to a
 * vtkImageReslice instance (the magnifier for zooming capability). The output
 * of the magnifier is connected to an instance of vtkImageMapper which is
 * connected to a vtkActor2D instance itself. The reference to this actor must
 * be set to this interactor style. Moreover, the magnifier instance must be
 * set. In addition, the pointer to the reference image (e.g. the input of
 * vtkImageReslice) must be set; this is important for zooming and image
 * fitting. In order to support color windowing, the vtkImageMapper pointer must
 * be set.
 *
 * Moreover, this interactor style supports color windowing of further "channels"
 * different from the main input image mapper. These optional further window/
 * level "channels" can be dynamically added/removed. Depending on the
 * CurrentWindowLevelChannel property, color windowing relates either to the
 * main image mapper or to the specified optional window/level channel.
 *
 * Windowing is related relatively to contrast c=1.0. Near c=1.0 the
 * sensitivity is minimal. In opposite, sensitivity is nonlinearly increasing
 * with increasing deviation from c=1.0. Window and level sensitivities are
 * adapted in parallel; i.e. as soon as the contrast reaches its maximum,
 * leveling is also sensitive.
 *
 * NOTE: This class is meant for 2D images (3D images with one slice). If a 3D
 * image with more than 1 slices is set, the actual slice is set to the center
 * slice along the current viewing direction. If you would like to have
 * slicing capabilities for 3D volumes, you should refer to the subclass
 * <code>vtk3DSlicingInteractorStyle</code> which is dedicated to interaction
 * with 3D volumes.
 *
 * Key/mouse bindings:<br>
 * - ZOOMING: (a) right mouse-button and up/down-movement (zooming w.r.t.
 * reslice plane center), (b) '+'/'-'-keys (zoom-in, zoom-out), (c) 'r'-key
 * (fit zoom to actual viewport), (d - if configured to do so) mouse-wheel
 * up/down (zoom-in, zoom-out)<br>
 * - PANNING: left mouse-button + ALT-key and left/right/up/down-movement
 * (panning of the image)<br>
 * - COLOR WINDOWING: (a) middle mouse-button (up-movement: increase
 * brightness, down-movement: decrease brightness, left-movement: decrease
 * contrast, right-movement: increase contrast), (b) 'r'-key+ctrl (reset window
 * level to whole image range) <br>
 * - RESLICING INTERPOLATION: 'i'-key alters reslicing interpolation method
 * (nearest-neighbor->linear->cubic->nearest-neighbor->...) if the magnifier
 * component is already set
 * - FLIPPING: 'x'-key flips the image along the x-axis by changing the reslice
 * direction internally; 'y'-key flips the image along the y-axis by changing
 * the reslice direction internally
 * - WINDOW/LEVEL CHANNEL: 'w'-key alters the current window level channel
 * (main channel->channel 1->channel 2->...->main channel->...)
 * - CURSORING: (if support flag is enabled) left mouse-button without ALT-key
 * or external ALT-flag set (down starts cursoring, movement while down button
 * triggers cursoring events, up ends cursoring)
 *
 * Moreover, this class fires events (see EventIDs enum) which can be listend
 * from external source.
 *
 * @see vtkInteractorStyleImage
 * @see vtkActor2D
 * @see vtkImageMapper
 * @see vtkImageReslice
 * @see vtk3DSlicingInteractorStyle
 *
 * @author phil
 * @version 2.3
 */
class vtk2DSceneImageInteractorStyle :
  public vtkInteractorStyleImage
{
public:
  /** Special event IDs for this class. **/
  typedef enum
  {
    StartZooming = vtkCommand::UserEvent + 1,
    Zooming,
    EndZooming,
    StartPanning,
    Panning,
    EndPanning,
    InterpolationModeChanged,
    FlippingModeChanged,
    WindowLevelChannelChanged,
    ImageFittedToRenderWindow,
    ImagePortionFittedToRenderWindow,
    StartCursoring,
    Cursoring,
    EndCursoring,
    ViewRestored /* this event ID should always stay the last one! */
  } EventIds;

  /** Standard new **/
  static vtk2DSceneImageInteractorStyle* New();
  vtkTypeRevisionMacro(vtk2DSceneImageInteractorStyle, vtkInteractorStyleImage);

  /** Fit the whole image (maximum extent w.r.t. current reslice plane) to the
   * render window in order to provide the maximum possible view. **/
  virtual void FitImageToRenderWindow();

  /** Fit a specified portion of the image (lying on the current reslice plane)
   * to the render window in order to provide the maximum possible view for this
   * image portion. NOTE: Both points which specify the image portion must lie
   * on the reslice plane (only processed internally if this criterion is met)!
   * @param point1 specifies the outer corner position of the "lower left" pixel
   * of the reslice image portion defined in WCS
   * @param point2 specifies the outer corner position of the "upper right" pixel
   * of the reslice image portion defined in WCS
   * @param adaptForZooming if TRUE, the current view is adapted so that zooming
   * afterwards does not feel unintuitive - the whole render window is filled,
   * but w.r.t. to desired view portion and resultant zooming factor **/
  virtual void FitImagePortionToRenderWindow(double point1[3], double point2[3],
      bool adaptForZooming);

  /** Fit a specified portion of the image (lying on the current reslice plane)
   * to the render window in order to provide the maximum possible view for this
   * image portion. NOTE: Both points which specify the image portion are
   * expressed as relative 2D values lying on the image plane. These values
   * relate to the image origin of the maximum image plane w.r.t. to current
   * reslicing orientation.
   * @param point1 specifies the reslice origin of the image portion w.r.t.
   * current reslice origin and orientation in the current reslice plane (2D
   * vector in physical units)
   * @param point2 specifies the "right upper" point of the image portion w.r.t.
   * current reslice origin and orientation in the current reslice plane (2D
   * vector in physical units)
   * @param adaptForZooming if TRUE, the current view is adapted so that zooming
   * afterwards does not feel unintuitive - the whole render window is filled,
   * but w.r.t. to desired view portion and resultant zooming factor **/
  virtual void FitRelativeImagePortionToRenderWindow(double point1[2],
      double point2[2], bool adaptForZooming);

  void SetRenderer(vtkRenderer *ren);
  vtkGetObjectMacro(Renderer, vtkRenderer)

  vtkSetObjectMacro(ImageActor, vtkActor2D)
  vtkGetObjectMacro(ImageActor, vtkActor2D)

  virtual void SetMagnifier(vtkImageReslice *magnifier);
  vtkGetObjectMacro(Magnifier, vtkImageReslice)

  vtkSetObjectMacro(ImageMapper, vtkImageMapper)
  vtkGetObjectMacro(ImageMapper, vtkImageMapper)

  virtual void SetReferenceImage(vtkImageData *image);
  vtkGetObjectMacro(ReferenceImage, vtkImageData)

  vtkSetMacro(SupportColorWindowing, bool)
  vtkGetMacro(SupportColorWindowing, bool)
  vtkBooleanMacro(SupportColorWindowing, bool)

  vtkSetMacro(UseMinimumMaximumSpacingForZoom, bool)
  vtkGetMacro(UseMinimumMaximumSpacingForZoom, bool)
  vtkBooleanMacro(UseMinimumMaximumSpacingForZoom, bool)

  vtkSetMacro(MinimumSpacingForZoom, double)
  vtkGetMacro(MinimumSpacingForZoom, double)

  vtkSetMacro(MaximumSpacingForZoom, double)
  vtkGetMacro(MaximumSpacingForZoom, double)

  /** Restore the last positioning scheme (based on a very simple strategy which
   * simply adapts the zoom factor to the render window width). May be useful
   * to involve when render window size is altered. **/
  void RestoreViewSettings();

  /** Mouse scroll event binding: zoom in. **/
  virtual void OnMouseWheelForward();
  /** Mouse scroll event binding: zoom out. **/
  virtual void OnMouseWheelBackward();
  /** Mouse move bindings: realize the operations here. **/
  virtual void OnMouseMove();
  /** Right mouse button event bindings:
   * (NO MODIFIER): center-based zooming<br>
   * (SHIFT): click-point-based zooming<br>
   * (CONTROL): Window/Level<br>
   **/
  virtual void OnRightButtonDown();
  /** Right mouse button event bindings:
   * (NO MODIFIER): center-based zooming<br>
   * (SHIFT): click-point-based zooming<br>
   * (CONTROL): Window/Level<br>
   **/
  virtual void OnRightButtonUp();
  /** Middle mouse button event bindings:
   * (NO MODIFIER): Window/Level<br>
   **/
  virtual void OnMiddleButtonDown();
  /** Middle mouse button event bindings:
   * (NO MODIFIER): Window/Level<br>
   **/
  virtual void OnMiddleButtonUp();
  /** Left mouse button event bindings:
   * (ALT): panning
   **/
  virtual void OnLeftButtonDown();
  /** Left mouse button event bindings:
   * (ALT): panning
   **/
  virtual void OnLeftButtonUp();

  /** A key goes down (for pseudo-ALT-flag). **/
  virtual void OnKeyDown();
  /** A key goes up (for pseudo-ALT-flag). **/
  virtual void OnKeyUp();

  /** Return the current size of a pixel w.r.t. to current magnification.
   * If the pixel spacing cannot be returned, FALSE is returned - otherwise,
   * if x and y pixel spacing can be returned, TRUE is returned. **/
  bool GetCurrentPixelSpacing(double spacing[2]);

  /** Get the array of optional further LUTs for other window/level "channels" **/
  const std::vector<vtkImageMapToColors *> &GetWindowLevelChannels()
  {
    return WindowLevelChannels;
  }
  /** Add an optional further mapper as other window/level "channel" and return its
   * 1-based index. In addition, reset window and level values must be defined
   * (2-component double array, 1st component is window, 2nd is level).
   * NOTE: We assume the mapper's lookup table being an instance of
   * vtkLookupTable or an instance of a subclass of vtkLookupTable. **/
  int AddWindowLevelChannel(vtkImageMapToColors *channel, double *resetWindowLevel);
  /** Override the reset window/level values of the specified further window/
   * level "channel".
   * @return TRUE if the values were successfully overridden **/
  bool OverrideResetWindowLevel(int index, double *resetWindowLevel);
  /** Override the reset window/level values of the specified further window/
   * level "channel" by specifying minimum and maximum of the intensity range.
   * @return TRUE if the values were successfully overridden **/
  bool OverrideResetWindowLevelByMinMax(int index, double *resetMinMax);
  /** Remove the specified LUT from window/level "channel"s and return TRUE if
   * successful. **/
  bool RemoveWindowLevelChannel(vtkImageMapToColors *channel);
  /** Return the 1-based index of the specified window/level "channel". 0 is
   * returned if the channel is not found in the internal array. */
  int GetIndexOfWindowLevelChannel(vtkImageMapToColors *channel);
  /** Remove all window/level channels. **/
  void RemoveAllWindowLevelChannels();

  vtkSetObjectMacro(MainWindowLevelChannel, vtkImageMapToColors)
  vtkGetObjectMacro(MainWindowLevelChannel, vtkImageMapToColors)

  vtkSetVector2Macro(MainWindowLevelChannelResetWindowLevel, double)
  vtkGetVector2Macro(MainWindowLevelChannelResetWindowLevel, double)

  void SetCurrentWindowLevelChannel(int channel);
  vtkGetMacro(CurrentWindowLevelChannel, int)

  vtkSetMacro(WindowLevelMouseSensitivity, double)
  vtkGetMacro(WindowLevelMouseSensitivity, double)

  vtkSetMacro(RealTimeMouseSensitivityAdaption, bool)
  vtkGetMacro(RealTimeMouseSensitivityAdaption, bool)
  vtkBooleanMacro(RealTimeMouseSensitivityAdaption, bool)

  /** Set the interpolation mode of the magnifier component (if set). **/
  void SetInterpolationMode(int mode);
  /** Get the interpolation mode of the magnifier component (if set).
   * @return -1 if there is no magnifier, interpolation mode otherwise **/
  int GetInterpolationMode();

  /** Flip the image along the row-direction visually by rotating around the
   * column-direction by 180 degrees. NOTE: This method changes the reslice
   * orientation!
   * @param resliceMatrix specifies the matrix to be changed **/
  virtual void FlipImageAlongRowDirection(vtkMatrix4x4 *resliceMatrix = NULL);
  /** Flip the image along the column-direction visually by rotating around the
   * row-direction by 180 degrees. NOTE: This method changes the reslice
   * orientation!
   * @param resliceMatrix specifies the matrix to be changed **/
  virtual void FlipImageAlongColumnDirection(vtkMatrix4x4 *resliceMatrix = NULL);

  vtkSetMacro(UseMouseWheelForZoomingInOut, bool)
  vtkGetMacro(UseMouseWheelForZoomingInOut, bool)
  vtkBooleanMacro(UseMouseWheelForZoomingInOut, bool)

  vtkSetMacro(ExternalPseudoAltKeyFlag, bool)
  vtkGetMacro(ExternalPseudoAltKeyFlag, bool)

  /** Reset internal flip-states to FALSE. E.g. if input data have
   * changed. This method does not trigger the related eventl. **/
  virtual void ResetFlipStates();
  vtkGetMacro(FlippedAlongRow, bool)
  vtkGetMacro(FlippedAlongColumn, bool)

  /** @see vtkInteractorStyle#StartZoom() **/
  virtual void StartZoom();
  /** @see vtkInteractorStyle#EndZoom() **/
  virtual void EndZoom();
  /** @see vtkInteractorStyle#StartPan() **/
  virtual void StartPan();
  /** @see vtkInteractorStyle#EndPan() **/
  virtual void EndPan();

  vtkSetMacro(SupportCursoring, bool)
  vtkGetMacro(SupportCursoring, bool)
  vtkBooleanMacro(SupportCursoring, bool)
  /** Start cursoring mode (if the cursoring-support flag is set). **/
  virtual void StartCursor();
  /** End cursoring mode (if the cursoring-support flag is set). **/
  virtual void EndCursor();
  vtkGetVector2Macro(CurrentInPlaneCursorPosition, int)
  vtkGetVector3Macro(CurrentWorldCursorPosition, double)

  /** Maps discrete in-plane coordinates (x,y) in pixels into real world
   * coordinates (x,y,z) in physical units.
   * \warning NOTE: This method is not really implemented in this class as this
   * class is designed for 2D images. It should be overridden in subclasses
   * that support image orientation in higher dimensions! **/
  virtual void MapInPlaneCoordinatesIntoWorldCoordinates(const int ic[2],
  		double wc[3]);

  /** Maps real world coordinates (x,y,z) in physical units into discrete
   * in-plane coordinates (x,y) in pixels.
   * \warning NOTE: This method is not really implemented in this class as this
   * class is designed for 2D images. It should be overridden in subclasses
   * that support image orientation in higher dimensions! **/
  virtual void MapWorldCoordinatesIntoInPlaneCoordinates(const double wc[3],
  		int ic[2]);

protected:
  /** "Epsilon" for accurate internal floating point comparisons. **/
  static const double F_EPSILON;
  /** "Epsilon" for rather weak internal floating point comparisons. **/
  static const double F_WEAK_EPSILON;

  /** Flag is true if we think that the ALT key is pressed. The ALT key is not
   * really trackable using the interactor style. BUT: if the returned key code
   * is 0 and neither CTRL nor SHIFT are active, it's quite likely that ALT is
   * pressed. **/
  bool PseudoAltKeyFlag;
  /** Additional flag indicating that ALT is currently pressed which can be
   * set externally.  **/
  bool ExternalPseudoAltKeyFlag;
  /** Connected renderer **/
  vtkRenderer *Renderer;
  /** Reference to the renderer's render window **/
  vtkRenderWindow *RenderWindow;
  /** Connected reference (fixed) image for resetting the camera **/
  vtkImageData *ReferenceImage;
  /** Magnifier that is connected to the image mapper's input and internally
   * realizes zooming **/
  vtkImageReslice *Magnifier;
  /** Image reslicing axes orientation defined in the image's coordinate system.
   * Within this class, this matrix - which describes row/column-directions for
   * reslicing - is set to identity because we do not allow any reslicing out of
   * plane for 2D images. It's however used in order to implement generalism
   * for N-D implementations (sub-classes). This matrix is row-based: first
   * row defines the 'x'-axis orientation, second row defines the 'y'-axis
   * orientation, third columns is the cross product of the first and second
   * row. All row vectors are expected to be normalized! **/
  vtkSmartPointer<vtkMatrix4x4> ResliceOrientation;
  /** Image actor used for pixel rendering (2D actor) **/
  vtkActor2D *ImageActor;
  /** Image mapper used for pixel rendering (2D mapper) **/
  vtkImageMapper *ImageMapper;
  /** Holds current magnification factor **/
  double CurrentMagnification;
  /** Holds current reslice left-lower-corner in world coordinates **/
  double CurrentResliceLLC[3];
  /** Holds current reslice right-upper-corner in world coordinates **/
  double CurrentResliceRUC[3];
  /** Holds current reslice spacing **/
  double CurrentResliceSpacing[3];
  /** Holds current reslice extent **/
  int CurrentResliceExtent[6];
  /** Flag indicating whether or not to support window/level. Needs a valid
   * pointer to the 2D image mapper.
   * @see ImageMapper **/
  bool SupportColorWindowing;
  /** Helper for color windowing. **/
  double InitialWL[2];
  /** Helper for color windowing. **/
  double CurrentWL[2];
  /** Helper for color windowing. **/
  double CurrentSR[2];
  /** Helper for color windowing. **/
  int WLStartPosition[2];
  /** Current window/level sensitivity factor. **/
  double CurrentWLFactor;
  /** Mouse sensitivity for color windowing: a value <1.0 decreases sensitivity,
   * a value >1.0 increases sensitivity. **/
  double WindowLevelMouseSensitivity;
  /** Flag indicating whether mouse sensitivity for window/level should be
   * adapted in dependence of contrast in real-time during windowing. If TRUE,
   * sensitivity is real-time-adapted, if FALSE, sensitivity is adapted each
   * time the user initiates a new windowing (by pressing the mouse down). **/
  bool RealTimeMouseSensitivityAdaption;
  /** Array of optional further LUTs for other window/level "channels" **/
  std::vector<vtkImageMapToColors *> WindowLevelChannels;
  /** Array corresponding to WindowLevelChannels with optional "reset" window/
   * level values for each channel (NULL if not specified -> reset window/level
   * will then fit to the whole image range!). **/
  std::vector<double *> WindowLevelChannelsResetWindowLevels;
  /** Optional LUT for main window/level channel. If set, the main windowing is
   * executed on this channel instead of the image mapper. **/
  vtkImageMapToColors *MainWindowLevelChannel;
  /** "Reset" window/level values for the main channel which is required if
   * MainWindowLevel is set. **/
  double MainWindowLevelChannelResetWindowLevel[2];
  /** Indicate which window/level "channel" is currently selected. The index
   * starts with 1 and ends with number of window/level "channels". An index
   * of 0 or another invalid index implicitly selects the image mapper
   * window/level "channel" or the main channel (if set), respectively! **/
  int CurrentWindowLevelChannel;
  /** Flag indicating whether the minimum/maximum spacing values should be used
   * for limiting the zoom **/
  bool UseMinimumMaximumSpacingForZoom;
  /** Optional minimum spacing which limits zoom-in. Only effective if
   * UseMinimumMaximumSpacingForZoom is ON. If this attribute is smaller or
   * equal to 0, it will be ignored. **/
  double MinimumSpacingForZoom;
  /** Optional maximum spacing which limits zoom-out. Only effective if
   * UseMinimumMaximumSpacingForZoom is ON. If this attribute is smaller or
   * equal to 0, it will be ignored. **/
  double MaximumSpacingForZoom;
  /** If true, the mouse wheel foward/backward events trigger zoom in/out. If
   * false, mouse wheel movements do not have any effect. **/
  bool UseMouseWheelForZoomingInOut;
  /** Help flag **/
  bool SecondaryTriggeredMouseWheel;
  /** Flag indicating that image is flipped along row **/
  bool FlippedAlongRow;
  /** Flag indicating that image is flipped along column **/
  bool FlippedAlongColumn;
  /** Flag indicating that zooming event should not be invoked now **/
  bool DoNotInvokeZoomingEvent;
  /** Flag indicating whether the cursoring events are fired. Cursoring events
   * are triggered by left mouse button clicks and movements. NOTE: This class
   * does not support any real cursoring capabilities. If activated (OFF by
   * default), the cursoring events (see EventIDs enum) are fired and can be
   * listened. **/
  bool SupportCursoring;
  /** Holds the current discrete cursor position coordinates (x, y) in pixels
   * in-plane. This property is only reliable during cursoring events! **/
  int CurrentInPlaneCursorPosition[2];
  /** Holds the current real cursor position coordinates (x, y, y) in physical
   * units in world coordinate system. This property is only reliable during
   * cursoring events!
   * \warning NOTE: This member is not really implemented in this class as this
   * class is designed for 2D images. It is expected to be filled in subclasses
   * that support image orientation in higher dimensions! **/
  double CurrentWorldCursorPosition[3];

  /** Callback for color windowing interactions. **/
  static void WindowLevelCallback(vtkObject *caller, unsigned long eid,
      void *clientdata, void *calldata);

  /** Default constructor **/
  vtk2DSceneImageInteractorStyle();
  /** Hidden default destructor. **/
  virtual ~vtk2DSceneImageInteractorStyle();

  /** @see vtkInteractorStyle#OnChar() **/
  virtual void OnChar();
  /** @see vtkInteractorStyle#OnKeyPress() **/
  virtual void OnKeyPress();

  /** Zoom in/out by a specified factor. A factor>1.0 zooms in, a factor<1.0
   * zooms out. **/
  virtual void Zoom(double factor);

  /** Pan the image by the specified offset (in pixels). **/
  virtual void Pan(int dx, int dy);

  /** Apply current geometry to the image. It is defined by the image origin,
   * the spacing and the image extent (all in the world coordinate system). **/
  void ApplyCurrentGeometry(double *origin, double *spacing,
      int *extent);

  /** Internal implementation of color windowing (ORA style). **/
  virtual void WindowLevelInternal();
  /** Internal implementation of color windowing start. **/
  virtual void StartWindowLevelInternal();
  /** Internal implementation of color windowing reset. **/
  virtual void ResetWindowLevelInternal();

  /** Compute current window/level factors that control w/l-sensitivity. **/
  virtual void ComputeCurrentWindowLevelFactors();

  /** Compute the 8 corner points of the current reference image in world
   * coordinate system **/
  void ComputeVolumeCorners(double corners[8][3]);

  /** Compute adjusted spacing w.r.t. the current reslicing direction **/
  virtual void ComputeAdjustedSpacing(double spacing[3]);

  /** Compute the adjusted reslicing parameters w.r.t. the current reslicing
   * direction so that we retrieve the whole data set. This method returns
   * the centered origin along the slicing direction.
   * @return the center-coordinate along the slicing direction **/
  virtual double ComputeAdjustedMaximumReslicingParameters(double origin[3],
      double spacing[3], int extent[6]);

  /** Alters reslicing interpolation mode if the magnifier component is set.
   * Nearest-neighbor->linear->cubic->nearest-neighbor->...<br>
   * Does NOT re-render automatically! **/
  void AlterInterpolationMode();

  /** Alters current window level channel. **/
  void AlterWindowLevelChannel();

  /** Update the reslice orientation matrix internally. This method has no
   * effect in this class' implementation, but it is supposed to be implement
   * in more complex subclasses. **/
  virtual void UpdateResliceOrientation();

  /** This method is called after changing magnifier props (origin, axes, output
   * properties). In this class this method is simply empty. In more complex
   * subclasses it may be a useful 'entry point'. **/
  virtual void AfterChangingMagnifier();

private:
  // purposely not implemented
  vtk2DSceneImageInteractorStyle(const vtk2DSceneImageInteractorStyle &);
  // purposely not implemented
  void operator=(const vtk2DSceneImageInteractorStyle &);

};

#endif /* VTK2DSCENEIMAGEINTERACTORSTYLE_H_ */
