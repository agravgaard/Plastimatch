//
#ifndef ORAREG23RENDERVIEWDIALOG_H
#define ORAREG23RENDERVIEWDIALOG_H

#include <QtGui/QDialog>
#include <QMutex>
#include <QVector>

#include "ui_oraREG23RenderViewDialog.h"

#include "vtkPerspectiveOverlayProjectionInteractorStyle.h"

// ORAIFMVC
#include <oraViewController.h>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkImageBlend.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkImageMapper.h>
#include <vtkActor2D.h>
#include <vtkImageReslice.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkArcSource.h>
#include <vtkAppendPolyData.h>

// forward declarations:
namespace ora 
{
class REG23Model;
}
class vtkImageData;
class QTimer;
class QResizeEvent;
class QCursor;
class QMenu;
class QAction;

namespace ora 
{

class REG23RenderViewDialog :
    public QDialog, public ViewController
{
Q_OBJECT

  /*
   TRANSLATOR ora::REG23RenderViewDialog

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** UPDATE ID (MVC) for forcing the render view to update window/level. **/
  static const int FORCE_RESET_WINDOW_LEVEL = 75201;

  /** Default constructor **/
  REG23RenderViewDialog(QWidget *parent = 0);
  /** Destructor **/
  ~REG23RenderViewDialog();

  /** @see ViewController#Update(int) **/
  virtual void Update(int id);

  /** Set according 0-based fixed image index **/
  void SetFixedImageIndex(int idx)
  {
    m_FixedImageIndex = idx;
  }
  /** Get according 0-based fixed image index **/
  int GetFixedImageIndex()
  {
    return m_FixedImageIndex;
  }

  /** Initialize the view. MUST BE CALLED FROM MAIN-THREAD! **/
  virtual void Initialize();

  /** Get GUI update interval in milliseconds **/
  int GetGUIUpdateIntervalMS()
  {
    return m_GUIUpdateIntervalMS;
  }
  /** Set GUI update interval in milliseconds **/
  void SetGUIUpdateIntervalMS(int ms);

  /** Activate GUI timer.
   * @see QWidget#setVisible() **/
  virtual void setVisible(bool visible);

  /** Get basic window title (normally the view-name). **/
  QString GetBaseWindowTitle()
  {
    return m_BaseWindowTitle;
  }
  /** Set basic window title (normally the view-name). **/
  void SetBaseWindowTitle(QString baseTitle)
  {
    m_BaseWindowTitle = baseTitle;
  }

  /** Depending on the model's window/level storage strategy, the current
   * fixed image and unsharp mask image window/level settings are stored
   * to file in the respective fixed image's source folder. **/
  void StoreWindowLevel();

  /** Saves a screenshot of the renderwindow.
   *
   * @param outputDir Output directory where the screenshot is saved.
   * @param outPattern Pattern with '%1' for fixed image index [0-n] and '%2' for blending value [0-100].
   * @param index Additional pattern value (replaced if index > 0).
   * @param blendvalue Sets the blending slider value if in range [0-100]. Default -1.
   * @param magnification Magnification of screenshot (resolution) [1-2048]. Default 1.
   * @return True if successful, else false.
   */
  bool StoreRenderWindowImage(const QString &outputDir, const QString &outPattern,
      const unsigned int &index = 0, const int &blendvalue = -1,
      const int &magnification = 1);

public Q_SLOTS:
  /** Deactivate GUI timer.
   * @see QDialog#done()
   **/
  virtual void done(int r);

protected:
  /** Factor that defines transform widget bounds. **/
  static const double TW_FXMIN;
  /** Factor that defines transform widget bounds. **/
  static const double TW_FXMAX;
  /** Factor that defines transform widget bounds. **/
  static const double TW_FYMIN;
  /** Factor that defines transform widget bounds. **/
  static const double TW_FYMAX;

  /** According 0-based fixed image index **/
  int m_FixedImageIndex;
  /** casted version of the model reference **/
  REG23Model *m_CastedModel;
  /** flag indicating that class is initializing **/
  bool m_Initializing;
  /** flag indicating that class has been initialized **/
  bool m_Initialized;
  /** Protect access to current moving image **/
  QMutex m_CurrentMovingImageMutex;
  /** Holds the copied current moving image for visualization (VTK-format) **/
  vtkImageData *m_CurrentMovingImage;
  /** Flag indicating that the moving image was updated and should be processed/
   * visualized or whatever; as soon as this was done, the flag should be set
   * back under protection of the m_CurrentMovingImageMutex (also querying this
   * flag should be m_CurrentMovingImageMutex-protected) **/
  bool m_UpdatedMovingImage;
  /** Flag indicating that the next moving image to be displayed is the initial
   * one. **/
  bool m_CurrentMovingImageIsInitial;
  /** Flag indicating that window/level of the overlay should be auto-adjusted
   * (stretched over the whole scalar range) **/
  bool m_StretchWindowLevel;
  /** Timer that manages GUI updates (therefore, the Initialize()-method must
   * be called from the MAIN-THREAD!) **/
  QTimer *m_GUIUpdateTimer;
  /** GUI update interval in milliseconds **/
  int m_GUIUpdateIntervalMS;
  /** Protect access to fixed image **/
  QMutex m_FixedImageMutex;
  /** Hold copied fixed image for visualization (VTK-format) **/
  vtkSmartPointer<vtkImageData> m_FixedImage;
  /** Hold copied fixed image in unsharp masking representation for
   * visualization (VTK-format). Created on demand. **/
  vtkSmartPointer<vtkImageData> m_FixedImageUM;
  /** Flag indicating that the fixed image was updated and should be processed/
   * visualized or whatever; as soon as this was done, the flag should be set
   * back under protection of the m_FixedImageMutex (also querying this
   * flag should be m_FixedImageMutex-protected) **/
  bool m_UpdatedFixedImage;
  /** Protect access to mask image **/
  QMutex m_MaskImageMutex;
  /** Hold copied mask image for visualization (VTK-format) **/
  vtkSmartPointer<vtkImageData> m_MaskImage;
  /** Flag indicating that the mask image was updated and should be processed/
   * visualized or whatever; as soon as this was done, the flag should be set
   * back under protection of the m_MaskImageMutex (also querying this
   * flag should be m_MaskImageMutex-protected) **/
  bool m_UpdatedMaskImage;
  /** Flag indicating that the window title was updated (this is not possible
   * from a thread!) **/
  bool m_UpdatedWindowTitle;
  /** Window title to be updated if m_UpdatedWindowTitle == TRUE **/
  QString m_WindowTitle;
  /** Flag indicating that window/level should be updated. **/
  bool m_UpdatedResetWindowLevel;
  /** Dummy image for situation when we do not have a fixed/moving image to
   * satisfy the pipeline. **/
  vtkSmartPointer<vtkImageData> m_EmptyImage;
  /** Lookup-table for fixed image coloring. **/
  vtkSmartPointer<vtkLookupTable> m_FixedLUT;
  /** Filter for fixed image coloring. **/
  vtkSmartPointer<vtkImageMapToColors> m_FixedColorMapper;
  /** Blender that blends the moving image over the fixed image. **/
  vtkSmartPointer<vtkImageBlend> m_Blender;
  /** Central renderer. **/
  vtkSmartPointer<vtkRenderer> m_Renderer;
  /** Interactor style that manages 2D perspective overlay interaction. **/
  vtkSmartPointer<vtkPerspectiveOverlayProjectionInteractorStyle> m_Style;
  /** Lookup-table for moving image coloring. **/
  vtkSmartPointer<vtkLookupTable> m_MovingLUT;
  /** Filter for moving image coloring. **/
  vtkSmartPointer<vtkImageMapToColors> m_MovingColorMapper;
  /** Lookup-table for mask image coloring. **/
  vtkSmartPointer<vtkLookupTable> m_MaskLUT;
  /** Filter for mask image coloring. **/
  vtkSmartPointer<vtkImageMapToColors> m_MaskColorMapper;
  /** 2D mapper for overlay image presentation **/
  vtkSmartPointer<vtkImageMapper> m_OverlayMapper2D;
  /** 2D actor for overlay image presentation **/
  vtkSmartPointer<vtkActor2D> m_OverlayActor2D;
  /** Resampler for zooming capability of overlay image **/
  vtkSmartPointer<vtkImageReslice> m_OverlayMagnifier;
  /** 2D mapper for rotation "widget" presentation (passive part) **/
  vtkSmartPointer<vtkPolyDataMapper2D> m_RotationWidgetMapper1;
  /** 2D mapper for rotation "widget" presentation (active part) **/
  vtkSmartPointer<vtkPolyDataMapper2D> m_RotationWidgetMapper2;
  /** 2D actor for rotation "widget" presentation (passive part) **/
  vtkSmartPointer<vtkActor2D> m_RotationWidgetActor1;
  /** 2D actor for rotation "widget" presentation (active part) **/
  vtkSmartPointer<vtkActor2D> m_RotationWidgetActor2;
  /** Poly data 1 that makes up the rotation "widget" (passive part) **/
  vtkSmartPointer<vtkPolyData> m_RotationWidgetPolyData1;
  /** Poly data 2 that makes up the rotation "widget" (active part) **/
  vtkSmartPointer<vtkArcSource> m_RotationWidgetPolyData2;
  /** Poly data 3 that makes up the rotation "widget" (active part) **/
  vtkSmartPointer<vtkPolyData> m_RotationWidgetPolyData3;
  /** Append filter for active parts of the rotation "widget" **/
  vtkSmartPointer<vtkAppendPolyData> m_RotationWidgetAppender;
  /** Helper that stores last manual rotation angle **/
  double m_LastManualRotationAngle;
  /** Helper: current rotation direction of the rotation "widget" **/
  int m_CurrentRotationDirection;
  /** 2D mapper for translation "widget" presentation (passive part) **/
  vtkSmartPointer<vtkPolyDataMapper2D> m_TranslationWidgetMapper1;
  /** 2D mapper for translation "widget" presentation (active part) **/
  vtkSmartPointer<vtkPolyDataMapper2D> m_TranslationWidgetMapper2;
  /** 2D actor for translation "widget" presentation **/
  vtkSmartPointer<vtkActor2D> m_TranslationWidgetActor1;
  /** 2D actor for translation "widget" presentation **/
  vtkSmartPointer<vtkActor2D> m_TranslationWidgetActor2;
  /** Poly data 1 that makes up the translation "widget" (passive part) **/
  vtkSmartPointer<vtkPolyData> m_TranslationWidgetPolyData1;
  /** Poly data 2 that makes up the translation "widget" (active part) **/
  vtkSmartPointer<vtkPolyData> m_TranslationWidgetPolyData2;
  /** Helper for initial parameters during manual registration. **/
  double m_InitialTransformParameters[6];
  /** Fixed image orientation **/
  vtkMatrix3x3 *m_FixedImageOrientation;
  /** Default cursor for render widget **/
  QCursor *m_DefaultCursor;
  /** Zooming cursor for render widget **/
  QCursor *m_ZoomCursor;
  /** Panning cursor for render widget **/
  QCursor *m_PanCursor;
  /** Rotation cursor for render widget **/
  QCursor *m_RotateCursor;
  /** Translation cursor for render widget **/
  QCursor *m_TranslateCursor;
  /** Windowing cursor for render widget **/
  QCursor *m_WindowLevelCursor;
  /** Flag indicating whether a manual transformation was initiated by mouse or
   * by key **/
  bool m_TransformationInitiatedByMouse;
  /** Basic window title (normally the view-name). **/
  QString m_BaseWindowTitle;
  /** Store last started interaction state (afer StartInteractionEvent) **/
  int m_LastStartedInteractionState;
  /** Previous window/level channel (0..overlay,1..fixed,2..moving) **/
  int m_PreviousWLChannel;
  /** Window/level-dependent icons: 0..fixed,1..fixed-w/l,2..moving,
   * 3..moving-w/l **/
  QVector<QIcon *> m_WLIcons;
  /** Window/level popup for fixed image button. **/
  QMenu *m_FixedWLMenu;
  /** Window/level popup for fixed image button. **/
  QAction *m_FixedWLAction;
  /** Window/level popup for moving image button. **/
  QMenu *m_MovingWLMenu;
  /** Window/level popup for moving image button. **/
  QAction *m_MovingWLAction;
  /** Helper flag for re-entrancy avoidance. **/
  bool m_BlockWLActionToggle;
  /** Helper flag for re-entrancy avoidance. **/
  bool m_BlockUMToggle;
  /** Stored window level of fixed image (since last unsharped masking
   * representation mode) **/
  double *m_FixedImageStoredWL;
  /** Stored window level of fixed image (since last unsharped masking
   * representation mode) **/
  double *m_FixedImageUMStoredWL;
  /** At least one fixed image has been received. **/
  bool m_FixedImageReceivedOnce;

  /** Callback command for style **/
  static void StyleCallback(vtkObject *caller, unsigned long eid,
      void *clientdata, void *calldata);

  /** @see QDialog#resizeEvent() **/
  void resizeEvent(QResizeEvent *);

  /** Establish the render pipeline for overlay imaging. **/
  void BuildRenderPipeline();
  /** Destroy the render pipeline for overlay imaging. **/
  void DestroyRenderPipeline();

  /** Generates an emtpy dummy image and stores it in m_EmptyImage. **/
  void GenerateEmptyImage();

  /** Auto-adjust the window/level if m_StretchWindowLevel==TRUE. **/
  void StretchOverlayWindowLevelOnDemand();

  /** Exchange the current connected fixed image with the specified new
   * fixed image. If NULL is specified, the empty image (m_EmptyImage)
   * is added to the pipeline.
   * @param newFixedImage the new fixed image which should be integrated with
   * the render pipeline
   * @param unsharpMaskImage flag indicating whether the fixed image has
   * unsharp mask representation or not
   **/
  void ExchangeFixedImageInRenderPipeline(vtkImageData *newFixedImage,
      bool unsharpMaskImage);
  /** Exchange the current connected moving image with the specified new
   * moving image. If NULL is specified, the empty image (m_EmptyImage)
   * is added to the pipeline.
   * @param newMovingImage the new moving image
   * @param isInitialImage is initial image if TRUE
   **/
  void ExchangeCurrentMovingImageInRenderPipeline(vtkImageData *newMovingImage,
      bool isInitialImage = false);
  /** Exchange the current connected mask image with the specified new
   * mask image. If NULL is specified, the empty image (m_EmptyImage)
   * is added to the pipeline.
   **/
  void ExchangeMaskImageInRenderPipeline(vtkImageData *newMaskImage);

  /** Create a new cursor from the specified imageName (in resources) and
   * maskName (in resources) and store the object in cursor variable.
   */
  void CursorFromImageName(QString imageName, QString maskName,
      QCursor *&cursor, int hotx = -1, int hoty = -1);

  /** Adapts render widget cursor to style's current interaction state. **/
  void AdaptRenderWidgetCursor();

  /** Instantiate and initialize the components for the rotation "widget". **/
  void BuildRotationWidget();
  /** Instantiate and initialize the components for the translation "widget". **/
  void BuildTranslationWidget();
  /** Destroy rotation and translation "widget" components. **/
  void DestroyWidgets();
  /** Adapt poly data of translation "widget" (only if tool button is checked).
   * @param bool if TRUE, the passive widget part is initialized according to
   * viewport size; if FALSE, the passive widget is not changed
   * @param dx relative x-offset of active widget part (in-plane)
   * @param dy relative y-offset of active widget part (in-plane)
   **/
  void AdaptTranslationWidget(bool initialize, double dx, double dy);
  /** Adapt poly data of rotation "widget" (only if tool button is checked).
   * @param bool if TRUE, the passive widget part is initialized according to
   * viewport size; if FALSE, the passive widget is not changed
   * @param dr relative rotation of active widget part in degrees (in-plane)
   **/
  void AdaptRotationWidget(bool initialize, double dr);

  /** Update current DRR according to specified 3D translation vector.
   * Translation is in mm. **/
  void UpdateDRRAccordingToTranslationVector(double translation[3]);
  /** Update current DRR according to specified 3D rotation (axis, angle).
   * Angle is in degrees. **/
  void UpdateDRRAccordingToRotation(double axis[3], double angle);

  /** Set the transform nature region on the interactor style. **/
  void AdaptTransformNatureRegion();

  /** Enable or disable the blending slider dependent on the current input
   * images (only enable if both a valid moving and fixed image is currently
   * set). **/
  void EnableDisableFixedMovingSlider();

  /** Append undo/redo task info due to manual transformation **/
  void AppendManualTransformationUndoRedoInfo();

  /** Update window/level-related icons, texts and menus in dependency of
   * the m_CurrentWLChannel member. **/
  void AdaptWindowLevelIconsTextsAndMenus();

protected slots:
  void OnGUIUpdateTimerTimeout();
  void OnZoomFullToolButtonPressed();
  void OnWindowLevelFullToolButtonPressed();
  void OnFixedMovingSliderValueChanged(int value);
  void OnFixedMovingSliderDoubleClick();
  void OnFixedMovingSliderRequestSliderInformationToolTipText(QString &text,
      int value);
  void OnFixedImageToolButtonPressed();
  void OnMovingImageToolButtonPressed();
  void OnMaskToolButtonToggled(bool value);
  void OnCrossCorrelationInitialTransformButtonClicked();
  void OnFixedWLMenuToggled(bool checked);
  void OnMovingWLMenuToggled(bool checked);
  void OnUnsharpMaskToolButtonToggled(bool checked);

private:
  Ui::REG23RenderViewDialogClass ui;
};

}

#endif // ORAREG23RENDERVIEWDIALOG_H
