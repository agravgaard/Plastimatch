//
#ifndef VTKSURFACETOPERSPECTIVEPROJECTIONIMAGEFILTER_H_
#define VTKSURFACETOPERSPECTIVEPROJECTIONIMAGEFILTER_H_

#include <map>

#include <vtkAlgorithm.h>
#include <vtkSmartPointer.h>

// forward declarations:
class vtkPolyData;
class vtkImageData;
class vtkRenderWindow;
class vtkRenderer;
class vtkCamera;
class vtkInformation;
class vtkMatrix3x3;
class vtkPolyDataCollection;
class vtkPolyDataMapper;
class vtkActor;
class vtkActorCollection;
class vtkWindowToImageFilter;


/** \class vtkSurfaceToPerspectiveProjectionImageFilter
 * \brief Generates a perspective projection (binary) image from input poly data surface(s).
 *
 * Generates a perspective projection (binary) image from input poly data
 * surface(s). A set of input surfaces can be added (poly data representation).
 *
 * In order to render the binary output image, an external render context
 * (render window) is required. This render window is automatically switched
 * to off-screen mode. Use the ConnectToRenderWindow() method to set this window.
 *
 * Moreover, the perspective projection geometry must be set: image plane size
 * and pose, and projection source position. NOTE: At the moment, only a
 * homogeneous spacing in row and column direction of the image plane is
 * supported!
 *
 * @see vtkAlgorithm
 *
 * <b>Tests</b>:<br>
 * TestSurfaceToPerspectiveProjectionImageFilter.cxx <br>
 *
 * @author phil 
 * @version 1.1
 */
class vtkSurfaceToPerspectiveProjectionImageFilter :
    public vtkAlgorithm
{
public:
  /** Standard new macro. **/
  static vtkSurfaceToPerspectiveProjectionImageFilter *New();

vtkTypeRevisionMacro(vtkSurfaceToPerspectiveProjectionImageFilter, vtkAlgorithm)
  ;

  /** Print object information. **/
  void PrintSelf(ostream& os, vtkIndent indent);

  /** Add an input poly data surface. **/
  void AddInput(vtkPolyData *input);
  /** Remove a specified input poly data surface. **/
  void RemoveInput(vtkPolyData *input);
  /** Remove all input poly data surfaces. **/
  void RemoveAllInputs();
  /** Get number of input poly data surfaces. **/
  int GetNumberOfInputs();

  /**
   * Get the output data object for a port on this algorithm. Here, we output
   * the perspective projection image of the input poly data surface in the
   * form of a binary image object.
   **/
  vtkImageData* GetOutput();

  /**
   * Connect to an external render window that is internally used for the
   * projection of the surface onto a plane.
   * @warning All renderers will be removed from the render window, and its
   * mode will be set to off-screen rendering!
   * @see DisconnectFromRenderWindow()
   **/
  bool ConnectToRenderWindow(vtkRenderWindow *renWin);
  /**
   * Disconnect from the render window, release it.
   * @see ConnectToRenderWindow()
   **/
  void DisconnectFromRenderWindow();

  vtkSetVector3Macro(SourcePosition, double)
  vtkGetVector3Macro(SourcePosition, double)

  vtkSetVector2Macro(PlaneSizePixels, int)
  vtkGetVector2Macro(PlaneSizePixels, int)

  virtual void SetPlaneSpacing(double sx, double sy);
  virtual void SetPlaneSpacing(double *s);
  vtkGetVector2Macro(PlaneSpacing, double)

  vtkSetVector3Macro(PlaneOrigin, double)
  vtkGetVector3Macro(PlaneOrigin, double)

  vtkSetMacro(UseThreadSafeRendering, bool)
  vtkGetMacro(UseThreadSafeRendering, bool)
  vtkBooleanMacro(UseThreadSafeRendering, bool)

  virtual void SetPlaneOrientation(vtkMatrix3x3 *);
  vtkGetObjectMacro(PlaneOrientation, vtkMatrix3x3)

  /** @see vtkAlgorithm#ProcessRequest() **/
  virtual int ProcessRequest(vtkInformation*, vtkInformationVector**,
      vtkInformationVector*);

  /** @return true if current projection geometry is valid **/
  bool IsProjectionGeometryValid();

protected:
  /** Source poly data surface(s). **/
  vtkSmartPointer<vtkPolyDataCollection> Inputs;
  /** Render window to be used for projection. **/
  vtkRenderWindow *RenWin;
  /** Internal camera that emulates the perspective projection geometry **/
  vtkSmartPointer<vtkCamera> Cam;
  /** Internal renderer that is used for rendering the projected image **/
  vtkSmartPointer<vtkRenderer> Ren;
  /** Flag indicating that we are connected to a render window. **/
  bool ConnectedToRenWin;
  /** Projection property: source position. **/
  double SourcePosition[3];
  /** Projection property: plane size in pixels. **/
  int PlaneSizePixels[2];
  /**
   * Projection property: plane spacing in mm/pixel. NOTE: Internally, only a
   * common spacing is supported (sx == sy). Therefore, if two different
   * spacing values for x and y are provided, the smaller is applied to both
   * directions!
   **/
  double PlaneSpacing[2];
  /** Projection property: plane position in mm. **/
  double PlaneOrigin[3];
  /**
   * Projection property: plane orientation (1st row: row vector, 2nd row:
   * column vector, 3rd row: slicing vector).
   **/
  vtkMatrix3x3 *PlaneOrientation;
  /** A map associating an input with its actor **/
  std::map<vtkPolyData *, vtkActor *> InputActorMap;
  /** Collection of current actors **/
  vtkSmartPointer<vtkActorCollection> Actors;
  /** Window to image filter **/
  vtkSmartPointer<vtkWindowToImageFilter> WindowImager;
  /** This flag indicates whether thread-safe rendering should be used **/
  bool UseThreadSafeRendering;

  /** Default constructor **/
  vtkSurfaceToPerspectiveProjectionImageFilter();
  /** Destructor **/
  ~vtkSurfaceToPerspectiveProjectionImageFilter();

  /**
   * vtkImageData output.
   * @see vtkAlgorithm#FillOutputPortInformation()
   **/
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  /** @see vtkAlgorithm#RequestData() **/
  virtual void RequestData(vtkInformation *, vtkInformationVector **,
      vtkInformationVector *);

  /** @see vtkAlgorithm#RequestInformation() **/
  virtual void RequestInformation(vtkInformation*, vtkInformationVector**,
      vtkInformationVector*);

  /** @return true if the internal camera could be successfully updated **/
  bool UpdateCameraFromProjectionGeometry();

private:
  /** Purposely not implemented. **/
  vtkSurfaceToPerspectiveProjectionImageFilter(
      const vtkSurfaceToPerspectiveProjectionImageFilter&);
  /** Purposely not implemented. **/
  void operator=(const vtkSurfaceToPerspectiveProjectionImageFilter&);

};

#endif /* VTKSURFACETOPERSPECTIVEPROJECTIONIMAGEFILTER_H_ */
