//
#ifndef VTKPERSPECTIVEOVERLAYPROJECTIONINTERACTORSTYLE_H_
#define VTKPERSPECTIVEOVERLAYPROJECTIONINTERACTORSTYLE_H_

// ORAIFGUIComponents
#include <vtk2DSceneImageInteractorStyle.h>

#include <vtkCommand.h>

// new interactor states:
#define VTKIS_PERSPECTIVE_TRANSLATION 3333
#define VTKIS_PERSPECTIVE_ROTATION 3334

// forward declarations:
class vtkPlane;
class vtkMatrix3x3;

/** \class vtkPerspectiveOverlayProjectionInteractorStyle
 * Interactor style dedicated for usage with 2D overlay image scenes that are
 * based on vtkActor2D and vtkImageMapper. It enables basic zooming, panning,
 * image fitting (adapting image zoom to viewport), and color windowing. In
 * addition, it enables the modification of a virtual underlying 3D
 * transformation based on in-plane translations and rotations (spinning).
 *
 * Additional key-/button-bindings:<br>
 * - Left mouse-button: (a) if RegionSensitiveTransformNature==FALSE, it will
 * initiate a translation; (b) if RegionSensitiveTransformNature==TRUE and the
 * cursor position is inside the TransformNatureRegion, a translation will be
 * initiated; (c) if RegionSensitiveTransformNature==TRUE and the cursor
 * position is outside the TransformNatureRegion, a rotation will be initiated<br>
 * - Left mouse-button + CTRL: (a) if RegionSensitiveTransformNature==FALSE, it
 * will always initiate a rotation; (b) if RegionSensitiveTransformNature==TRUE
 * and the cursor position is inside or outside the TransformNatureRegion, a
 * rotation will be initiated<br>
 * - Left mouse-button + SHIFT: (a) if RegionSensitiveTransformNature==TRUE
 * and the cursor position is inside or outside the TransformNatureRegion, a
 * translation will be initiated<br>
 * - Left cursor key: translation by 1 mm (3D) to "left" <br>
 * - Left cursor key + SHIFT: translation by 5 mm (3D) to "left" <br>
 * - Left cursor key + CTRL: rotation by 1 deg (3D) in CCW direction <br>
 * - Left cursor key + SHIFT + CTRL: rotation by 5 deg (3D) in CCW direction<br>
 * - Right cursor key: translation by 1 mm (3D) to "right" <br>
 * - Right cursor key + SHIFT: translation by 5 mm (3D) to "right" <br>
 * - Right cursor key + CTRL: rotation by 1 deg (3D) in CW direction <br>
 * - Right cursor key + SHIFT + CTRL: rotation by 5 deg (3D) in CW direction<br>
 *
 * The following events are fired:<br>
 * - TranslationEvent: if the user translated the second image; the event
 * parameters contain both the resultant 2D and the 3D translation<br>
 * - RotationEvent: if the user rotated the second image; the event parameters
 * contain both the 3D rotation axis and the rotation angle<br>
 *
 * @see vtk2DSceneImageInteractorStyle
 *
 * @author phil 
 * @version 1.2
 */
class vtkPerspectiveOverlayProjectionInteractorStyle
  : public vtk2DSceneImageInteractorStyle
{
public:
    /** Custom event IDs **/
    enum EventIds
    {
      /** Event is fired during manual user-initiated translation: calldata
       * will contain a double-5-tuple where the first two items contain the
       * in-plane translation in pixels and the next 3 items are the components
       * of the according 3D translation vector w.r.t. to COR. **/
      TranslationEvent = vtkCommand::UserEvent + 3333,
      /** Event is fired during manual user-initiated rotation: calldata
       * will contain a double-4-tuple where the first three items contain the
       * components of the 3D rotation axis, and the fourth item is the rotation
       * in degrees around the COR. **/
      RotationEvent = vtkCommand::UserEvent + 3334
    };

  /** Standard new **/
  static vtkPerspectiveOverlayProjectionInteractorStyle* New();
  vtkTypeRevisionMacro(vtkPerspectiveOverlayProjectionInteractorStyle, vtk2DSceneImageInteractorStyle);

  /** Left mouse-button binding.
   * @see vtk2DSceneImageInteractorStyle#OnLeftButtonDown() **/
  virtual void OnLeftButtonDown();
  /** Left mouse-button binding.
   * @see vtk2DSceneImageInteractorStyle#OnLeftButtonUp() **/
  virtual void OnLeftButtonUp();

  /** Mouse move bindings: realize the operations here. **/
  virtual void OnMouseMove();

  /** Define the imaging geometry that describes how the reference image was
   * generated.
   * @param origin origin of the reference image plane
   * @param orientation orientation of the reference image plane axes (1st row
   * is row direction, 2nd row is column direction, 3rd row is slicing
   * direction)
   * @param sourcePosition position of the image source (perspective proj.)
   * @param centerOfRotation position of the 3D transform's center of rotation
   **/
  void DefineReferenceImageGeometry(double *origin, vtkMatrix3x3 *orientation,
      double *sourcePosition, double *centerOfRotation);

  /** Return the position of the projected 3D center of rotation onto
   * the reference image plan.
   * @see DefineReferenceImageGeometry() **/
  double *GetProjectedCenterOfRotationPosition();

  vtkGetVector3Macro(ImagingSourcePosition, double)

  vtkGetObjectMacro(ReferenceImagePlane, vtkPlane)

  /** Compute and return the center of rotation projected onto the viewport in
   * pixels. **/
  void ComputeCenterOfRotationInViewportCoordinates(double center[2]);

  vtkSetMacro(RegionSensitiveTransformNature, bool)
  vtkGetMacro(RegionSensitiveTransformNature, bool)
  vtkBooleanMacro(RegionSensitiveTransformNature, bool)

  vtkSetVector4Macro(TransformNatureRegion, double)
  vtkGetVector4Macro(TransformNatureRegion, double)

  /** @see vtk2DSceneImageInteractorStyle#OnKeyPress() **/
  virtual void OnKeyPress();

protected:
  /** Internal image plane defining pose of reference image **/
  vtkPlane *ReferenceImagePlane;
  /** Internal reference image orientation **/
  vtkMatrix3x3 *ReferenceImageOrientation;
  /** Internal image plane perpendicular to the direction of projection **/
  vtkPlane *CORPlane;
  /** Internal imaging source position **/
  double ImagingSourcePosition[3];
  /** Internal center of rotation (3D transform) **/
  double CenterOfRotation3D[3];
  /** Internal center of rotation (projected onto reference image plane) **/
  double CenterOfRotation2D[3];
  /** Helper for storing coordinates **/
  int InitialTransformationPosition[2];
  /** Flag for region-sensitive transformation nature detection (rot / transl) **/
  bool RegionSensitiveTransformNature;
  /** If RegionSensitiveTransformNature-flag is set, a rectangular region is
   * required that marks where mouse clicks should be interpreted as translation
   * (inside) and where as rotation (outside). This prop contains the lower left
   * corner and the upper right corner of the region. **/
  double TransformNatureRegion[4];

  /** Default constructor **/
  vtkPerspectiveOverlayProjectionInteractorStyle();
  /** Hidden default destructor. **/
  virtual ~vtkPerspectiveOverlayProjectionInteractorStyle();

  // FIXME: implement PrintSelf()

  /** Interactor mode entry point: perspective translation (in-plane translation
   * which is computed back into a 3D transformation. **/
  virtual void StartPerspectiveTranslation();
  /** Interactor mode exit point: perspective translation (in-plane translation
   * which is computed back into a 3D transformation. **/
  virtual void EndPerspectiveTranslation();
  /** Interactor mode entry point: perspective translation (in-plane rotation
   * around view-axis which is computed back into a 3D transformation. **/
  virtual void StartPerspectiveRotation();
  /** Interactor mode exit point: perspective translation (in-plane rotation
   * around view-axis which is computed back into a 3D transformation. **/
  virtual void EndPerspectiveRotation();


private:
  // purposely not implemented
  vtkPerspectiveOverlayProjectionInteractorStyle(const vtkPerspectiveOverlayProjectionInteractorStyle &);
  // purposely not implemented
  void operator=(const vtkPerspectiveOverlayProjectionInteractorStyle &);

};

#endif /* VTKPERSPECTIVEOVERLAYPROJECTIONINTERACTORSTYLE_H_ */
