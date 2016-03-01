/*
 TRANSLATOR ora::REG23RenderViewDialog

 lupdate: Qt-based translation with NAMESPACE-support!
 */

#include <QTimer>
#include <QResizeEvent>
#include <QBitmap>
#include <QCursor>
#include <QToolTip>
#include <QMenu>
#include <QAction>
#include <QColorDialog>
#include <QString>
#include <QDir>

#include "oraREG23RenderViewDialog.h"

#include "oraREG23Model.h"
#include "oraSimpleTransformUndoRedo.h"
#include "oraManualRegistrationTransformTask.h"
#include "oraSparseAutoRegistrationTransformTask.h"
// ORAIFImageAccess
#include <oraITKVTKImage.h>
// ORAIFImageTools
#include <oraVTKUnsharpMaskingImageFilter.h>

#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkCallbackCommand.h>
#include <vtkProperty2D.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPlane.h>
#include <vtkTransform.h>
#include <vtkTransform2D.h>
#include <vtkPointData.h>
#include <vtkImageConvolve.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

namespace ora
{

const double REG23RenderViewDialog::TW_FXMIN = 0.2;
const double REG23RenderViewDialog::TW_FXMAX = 0.8;
const double REG23RenderViewDialog::TW_FYMIN = 0.2;
const double REG23RenderViewDialog::TW_FYMAX = 0.8;

REG23RenderViewDialog::REG23RenderViewDialog(QWidget *parent)
    : QDialog(parent), ViewController()
{
  m_FixedImageStoredWL = NULL;
  m_FixedImageUMStoredWL = NULL;
  m_FixedImageReceivedOnce = false;
  m_FixedWLAction = NULL;
  m_MovingWLAction = NULL;
	ui.setupUi(this);
	m_FixedImageIndex = -1;
	m_Initialized = false;
	m_CurrentMovingImage = NULL;
	m_UpdatedMovingImage = false;
	m_CurrentMovingImageIsInitial = false;
	m_GUIUpdateIntervalMS = 200; // 5 Hz default
	m_FixedImage = NULL;
	m_FixedImageUM = NULL;
	m_UpdatedFixedImage = false;
	m_MaskImage = NULL;
	m_UpdatedMaskImage = false;
	m_UpdatedWindowTitle = false;
	m_UpdatedResetWindowLevel = false;
  m_WindowTitle = "";
  m_BlockUMToggle = false;
	// generate a small empty dummy image for situations where we do not have an
	// input image:
	GenerateEmptyImage();
  m_FixedLUT = NULL;
  m_FixedColorMapper = NULL;
  m_Blender = NULL;
  m_Renderer = NULL;
  m_Style = NULL;
  m_MovingLUT = NULL;
  m_MovingColorMapper = NULL;
  m_MaskLUT = NULL;
  m_MaskColorMapper = NULL;
  m_OverlayMapper2D = NULL;
  m_OverlayActor2D = NULL;
  m_OverlayMagnifier = NULL;
  m_StretchWindowLevel = false;
  m_FixedImageOrientation = vtkMatrix3x3::New();
  m_FixedImageOrientation->Identity();
  m_CurrentRotationDirection = 0;
  m_TransformationInitiatedByMouse = false;
  m_BaseWindowTitle = "";
  m_LastStartedInteractionState = VTKIS_NONE;
  m_PreviousWLChannel = 0; // overlay
  QIcon *ico = NULL;
  ico = new QIcon(":u23rvd/img/fixed-image.png");
  m_WLIcons.push_back(ico);
  ico = new QIcon(":u23rvd/img/fixed-image-wl.png");
  m_WLIcons.push_back(ico);
  ico = new QIcon(":u23rvd/img/moving-image.png");
  m_WLIcons.push_back(ico);
  ico = new QIcon(":u23rvd/img/moving-image-wl.png");
  m_WLIcons.push_back(ico);
  m_BlockWLActionToggle = true;
  m_FixedWLMenu = new QMenu(ui.FixedImageToolButton);
  m_FixedWLAction = m_FixedWLMenu->addAction(
      REG23RenderViewDialog::tr("Windowing relates exclusively to X-ray image."));
  m_FixedWLAction->setCheckable(true);
  m_FixedWLAction->setChecked(false);
  this->connect(m_FixedWLAction, SIGNAL(toggled(bool)), this,
      SLOT(OnFixedWLMenuToggled(bool)));
  ui.FixedImageToolButton->setMenu(m_FixedWLMenu);
  m_MovingWLMenu = new QMenu(ui.MovingImageToolButton);
  m_MovingWLAction = m_MovingWLMenu->addAction(
      REG23RenderViewDialog::tr("Windowing relates exclusively to DRR image."));
  m_MovingWLAction->setCheckable(true);
  m_MovingWLAction->setChecked(false);
  this->connect(m_MovingWLAction, SIGNAL(toggled(bool)), this,
      SLOT(OnMovingWLMenuToggled(bool)));
  ui.MovingImageToolButton->setMenu(m_MovingWLMenu);
  m_BlockWLActionToggle = false;

  m_DefaultCursor = new QCursor(Qt::CrossCursor);
  m_ZoomCursor = NULL;
  CursorFromImageName(":u23rvd/img/zoom-cursor.png",
      ":u23rvd/img/zoom-cursor-mask.png", m_ZoomCursor, 13, 13);
  m_PanCursor = NULL;
  CursorFromImageName(":u23rvd/img/pan-cursor.png",
      ":u23rvd/img/pan-cursor-mask.png", m_PanCursor, 15, 16);
  m_RotateCursor = NULL;
  CursorFromImageName(":u23rvd/img/rotate-cursor1.png",
      ":u23rvd/img/rotate-cursor1-mask.png", m_RotateCursor, 17, 13);
  m_TranslateCursor = NULL;
  CursorFromImageName(":u23rvd/img/translate-cursor1.png",
      ":u23rvd/img/translate-cursor1-mask.png", m_TranslateCursor, 16, 16);
  m_WindowLevelCursor = NULL;
  CursorFromImageName(":u23rvd/img/window-level-cursor1.png",
      ":u23rvd/img/window-level-cursor1-mask.png", m_WindowLevelCursor, 15, 17);
  AdaptWindowLevelIconsTextsAndMenus();
}

void REG23RenderViewDialog::GenerateEmptyImage()
{
  m_EmptyImage = NULL;
  m_EmptyImage = vtkSmartPointer<vtkImageData>::New();
  m_EmptyImage->SetNumberOfScalarComponents(1);
  m_EmptyImage->SetScalarTypeToFloat();
  m_EmptyImage->SetDimensions(10, 10, 1);
  m_EmptyImage->SetSpacing(1, 1, 1);
  m_EmptyImage->AllocateScalars();
  float *pixels = static_cast<float *>(m_EmptyImage->GetScalarPointer());
  for (int i = 0; i < 100; i++)
    pixels[i] = 0;
}

REG23RenderViewDialog::~REG23RenderViewDialog()
{
  if (m_FixedImageStoredWL)
    delete[] m_FixedImageStoredWL;
  m_FixedImageStoredWL = NULL;
  if (m_FixedImageUMStoredWL)
    delete[] m_FixedImageUMStoredWL;
  m_FixedImageUMStoredWL = NULL;
  DestroyRenderPipeline();
  for (int i = 0; i < m_WLIcons.size(); i++)
    delete m_WLIcons[i];
  m_WLIcons.clear();
  if (m_CurrentMovingImage)
    m_CurrentMovingImage->Delete();
    m_CurrentMovingImage = NULL;
  if (m_GUIUpdateTimer)
    delete m_GUIUpdateTimer;
  m_GUIUpdateTimer = NULL;
  m_FixedImage = NULL;
  m_FixedImageUM = NULL;
  m_MaskImage = NULL;
  m_EmptyImage = NULL;
  if (m_DefaultCursor)
    delete m_DefaultCursor;
  if (m_ZoomCursor)
    delete m_ZoomCursor;
  if (m_PanCursor)
    delete m_PanCursor;
  if (m_TranslateCursor)
    delete m_TranslateCursor;
  if (m_RotateCursor)
    delete m_RotateCursor;
  if (m_WindowLevelCursor)
    delete m_WindowLevelCursor;
  DestroyWidgets();
  delete m_FixedWLMenu;
  m_FixedWLMenu = NULL;
  delete m_MovingWLMenu;
  m_MovingWLMenu = NULL;
  m_FixedImageOrientation->Delete();
}

void REG23RenderViewDialog::Initialize()
{
  m_Initializing = true;

  m_CastedModel = dynamic_cast<REG23Model *>(this->m_Model); // concrete type

  m_GUIUpdateTimer = new QTimer(this);
  m_GUIUpdateTimer->setInterval(m_GUIUpdateIntervalMS);
  this->connect(m_GUIUpdateTimer, SIGNAL(timeout()),
      this, SLOT(OnGUIUpdateTimerTimeout()));
  m_GUIUpdateTimer->setSingleShot(false);

  this->connect(ui.ZoomFullToolButton, SIGNAL(pressed()),
      this, SLOT(OnZoomFullToolButtonPressed()));
  this->connect(ui.WindowLevelFullToolButton, SIGNAL(pressed()),
      this, SLOT(OnWindowLevelFullToolButtonPressed()));
  QToolButton *tb = ui.TransformationWidgetsToolButton;
  tb->setCheckable(true);
  tb->setChecked(m_CastedModel->GetTransformationWidgetsVisible());
  tb = ui.MaskToolButton;
  tb->setCheckable(true);
  tb->setChecked(m_CastedModel->GetDisplayMasks());

  this->connect(ui.FixedMovingSlider, SIGNAL(valueChanged(int)),
      this, SLOT(OnFixedMovingSliderValueChanged(int)));
  this->connect(ui.FixedMovingSlider, SIGNAL(DoubleClick()),
      this, SLOT(OnFixedMovingSliderDoubleClick()));
  this->connect(ui.FixedMovingSlider, SIGNAL(RequestSliderInformationToolTipText(QString&,int)),
        this, SLOT(OnFixedMovingSliderRequestSliderInformationToolTipText(QString&,int)));
  this->connect(ui.FixedImageToolButton, SIGNAL(released()),
      this, SLOT(OnFixedImageToolButtonPressed()));
  this->connect(ui.MovingImageToolButton, SIGNAL(released()),
      this, SLOT(OnMovingImageToolButtonPressed()));
  this->connect(ui.MaskToolButton, SIGNAL(toggled(bool)),
      this, SLOT(OnMaskToolButtonToggled(bool)));

  this->connect(ui.UnsharpMaskToolButton, SIGNAL(toggled(bool)), this,
      SLOT(OnUnsharpMaskToolButtonToggled(bool)));
  if (m_CastedModel->IsUnsharpMaskingEnabled(m_FixedImageIndex + 1))
  {
    ui.UnsharpMaskToolButton->setEnabled(true);
    ui.UnsharpMaskToolButton->setCheckable(true);
    m_BlockUMToggle = true;
    if (m_CastedModel->IsUnsharpMaskingActivated(m_FixedImageIndex + 1))
      ui.UnsharpMaskToolButton->setChecked(true);
    else
      ui.UnsharpMaskToolButton->setChecked(false);
    m_BlockUMToggle = false;
  }
  else
  {
    ui.UnsharpMaskToolButton->setEnabled(false);
  }

  // Science mode buttons:
  if (m_CastedModel->IsScientificMode())
  {
    ui.CrossCorrelationInitialTransformButton->setVisible(true);
    this->connect(ui.CrossCorrelationInitialTransformButton, SIGNAL(clicked()), this,
        SLOT(OnCrossCorrelationInitialTransformButtonClicked()));
  }
  else
  {
    ui.CrossCorrelationInitialTransformButton->setVisible(false);
  }

  BuildRenderPipeline();
  BuildRotationWidget();
  BuildTranslationWidget();

  EnableDisableFixedMovingSlider();
  AdaptWindowLevelIconsTextsAndMenus();

  m_Initializing = false;
  m_Initialized = true;
}

void REG23RenderViewDialog::Update(int id)
{
  if (m_Initialized && (id == REG23Model::UPDATE_MOVING_IMAGES ||
      id == REG23Model::UPDATE_INITIAL_MOVING_IMAGES ||
      id == REG23Model::UPDATE_CURRENT_MOVING_IMAGES))
  {
    ITKVTKImage *movingImage = m_CastedModel->GetCurrentMovingImage(
        m_FixedImageIndex);
    if (movingImage && (m_CastedModel->GetCurrentIteration() >= 1 ||
        id == REG23Model::UPDATE_INITIAL_MOVING_IMAGES ||
        id == REG23Model::UPDATE_CURRENT_MOVING_IMAGES))
    {
      m_CurrentMovingImageMutex.lock();
      if (m_CurrentMovingImage)
        m_CurrentMovingImage->Delete();
      m_CurrentMovingImage = NULL;
      m_CurrentMovingImage = vtkImageData::New();
      // really copy the image!
      m_CurrentMovingImage->DeepCopy(movingImage->
          GetAsVTKImage<REG23Model::DRRPixelType>());
      if (m_FixedImage) // be sure that moving image has same origin!
        m_CurrentMovingImage->SetOrigin(m_FixedImage->GetOrigin());
      if (id == REG23Model::UPDATE_INITIAL_MOVING_IMAGES)
        m_CurrentMovingImageIsInitial = true;
      m_UpdatedMovingImage = true;
      m_CurrentMovingImageMutex.unlock();
    }
    if (id == REG23Model::UPDATE_INITIAL_MOVING_IMAGES)
    {
      m_StretchWindowLevel = true; // -> adapt window/level
      // moreover, extract image orientation and position (at this point,
      // the source position should be fixed by model):
      if (m_Style)
      {
        ITKVTKImage *fixedImage = m_CastedModel->GetFixedImage(
            m_FixedImageIndex);
        ITKVTKImage::ITKImagePointer ibase = fixedImage->
            GetAsITKImage<REG23Model::DRRPixelType>();
        typedef itk::Image<REG23Model::DRRPixelType,
          ITKVTKImage::Dimensions> OriginalImageType;
        OriginalImageType::Pointer icast = static_cast<OriginalImageType * >(
            ibase.GetPointer());
        OriginalImageType::PointType iorigin = icast->GetOrigin();
        OriginalImageType::DirectionType idirection = icast->GetDirection();
        double origin[3];
        origin[0] = iorigin[0];
        origin[1] = iorigin[1];
        origin[2] = iorigin[2];
        m_FixedImageOrientation->SetElement(0, 0, idirection[0][0]); // row-direction
        m_FixedImageOrientation->SetElement(0, 1, idirection[1][0]);
        m_FixedImageOrientation->SetElement(0, 2, idirection[2][0]);
        m_FixedImageOrientation->SetElement(1, 0, idirection[0][1]); // column-direction
        m_FixedImageOrientation->SetElement(1, 1, idirection[1][1]);
        m_FixedImageOrientation->SetElement(1, 2, idirection[2][1]);
        m_FixedImageOrientation->SetElement(2, 0, idirection[0][2]); // slicing-direction
        m_FixedImageOrientation->SetElement(2, 1, idirection[1][2]);
        m_FixedImageOrientation->SetElement(2, 2, idirection[2][2]);
        double *source = m_CastedModel->GetSourcePosition(m_FixedImageIndex);
        double cor[3];
        m_CastedModel->GetCenterOfRotation(cor);
        m_Style->DefineReferenceImageGeometry(origin, m_FixedImageOrientation,
            source, cor);
      }
    }
  }
  else if (m_Initialized && id == REG23Model::UPDATE_INPUT_IMAGES)
  {
    ITKVTKImage *fixedImage = m_CastedModel->GetFixedImage(m_FixedImageIndex, false);
    if (fixedImage)
    {
      m_FixedImageMutex.lock();
      m_FixedImageUM = NULL; // set back unsharp mask representation, too
      m_FixedImage = NULL;
      m_FixedImage = vtkSmartPointer<vtkImageData>::New();
      // really copy the image!
      m_FixedImage->DeepCopy(fixedImage->
          GetAsVTKImage<REG23Model::DRRPixelType>());
      m_UpdatedFixedImage = true;
      m_StretchWindowLevel = true; // -> adapt window/level
      // -> update window title with image information:
      if (fixedImage->GetMetaInfo())
      {
        ITKVTKImageMetaInformation::Pointer mi = fixedImage->GetMetaInfo();
        std::vector<SliceMetaInformation::Pointer> *smis = mi->
            GetSlicesMetaInformation();
        SliceMetaInformation::Pointer smi = NULL;
        if (smis && smis->size() > 0)
          smi = (*smis)[0];
        if (smi)
        {
          // %1 ... base window title
          // %2 ... acquisition type
          // %3 ... acquisition date
          QString s1 = QString::fromStdString(smi->GetORAAcquisitionType());
          QString s2 = QString::fromStdString(smi->GetORAAcquisitionDate());
          QString title = "";
          if (s1.length() > 0 && s2.length() > 0)
            title = REG23RenderViewDialog::tr("%1: %2 (%3)").
              arg(m_BaseWindowTitle).arg(s1).arg(s2);
          else if (s1.length() > 0)
            title = REG23RenderViewDialog::tr("%1: %2").
              arg(m_BaseWindowTitle).arg(s1);
          else
            title = REG23RenderViewDialog::tr("%1").
              arg(m_BaseWindowTitle);
          // NOTE: cannot set window title from a thread, need to do this in
          // GUI-thread!
          m_WindowTitle = title;
          m_UpdatedWindowTitle = true; // signal
        }
      }
      m_FixedImageMutex.unlock();
    }
  }
  else if (m_Initialized && id == REG23Model::UPDATE_MASK_IMAGES)
  {
    //ITKVTKImage *maskImage = m_CastedModel->GetMaskImage(m_FixedImageIndex);
    ITKVTKImage *maskImage = m_CastedModel->GetMaskImageContour(m_FixedImageIndex);
    if (maskImage)
    {
      m_MaskImageMutex.lock();
      m_MaskImage = NULL;
      m_MaskImage = vtkSmartPointer<vtkImageData>::New();
      // really copy the image!
      m_MaskImage->DeepCopy(maskImage->
          GetAsVTKImage<REG23Model::MaskPixelType>());
      delete maskImage;
      if (m_FixedImage) // be sure that moving image has same origin!
        m_MaskImage->SetOrigin(m_FixedImage->GetOrigin());
      m_UpdatedMaskImage = true;
      m_MaskImageMutex.unlock();
    }
  }
  else if (m_Initialized && id == FORCE_RESET_WINDOW_LEVEL)
  {
    m_UpdatedResetWindowLevel = true;
  }
}

void REG23RenderViewDialog::OnGUIUpdateTimerTimeout()
{
  m_CurrentMovingImageMutex.lock();
  if (m_UpdatedMovingImage)
  {
    if (m_CurrentMovingImage)
      ExchangeCurrentMovingImageInRenderPipeline(m_CurrentMovingImage,
          m_CurrentMovingImageIsInitial);
    if (m_CurrentMovingImageIsInitial)
      m_CurrentMovingImageIsInitial = false; // set back
    m_UpdatedMovingImage = false; // set back!
  }
  m_CurrentMovingImageMutex.unlock();

  m_FixedImageMutex.lock();
  if (m_UpdatedFixedImage)
  {
    if (m_FixedImage)
    {
      if (m_CastedModel->IsUnsharpMaskingEnabled(m_FixedImageIndex + 1) &&
          ui.UnsharpMaskToolButton->isChecked())
      {
        OnUnsharpMaskToolButtonToggled(true);
      }
      else
      {
        ExchangeFixedImageInRenderPipeline(m_FixedImage, false);
      }
      m_FixedImageReceivedOnce = true; // signal that!
    }
    m_UpdatedFixedImage = false; // set back!
  }
  m_FixedImageMutex.unlock();

  m_MaskImageMutex.lock();
  if (m_UpdatedMaskImage)
  {
    if (m_MaskImage)
      ExchangeMaskImageInRenderPipeline(m_MaskImage);
    m_UpdatedMaskImage = false; // set back!
  }
  m_MaskImageMutex.unlock();

  if (m_UpdatedWindowTitle)
  {
    this->setWindowTitle(m_WindowTitle);
    m_UpdatedWindowTitle = false;
  }

  if (m_UpdatedResetWindowLevel)
  {
    if (m_Style)
    {
      m_Style->SetCurrentWindowLevelChannel(0); // main
      m_Style->InvokeEvent(vtkCommand::ResetWindowLevelEvent);
      m_Style->SetCurrentWindowLevelChannel(m_PreviousWLChannel); // restore
      m_Renderer->GetRenderWindow()->Render();
    }
    m_UpdatedResetWindowLevel = false;
  }
}

void REG23RenderViewDialog::SetGUIUpdateIntervalMS(int ms)
{
  if (ms < 0)
    return;
  m_GUIUpdateIntervalMS = ms;
  if (m_GUIUpdateTimer)
    m_GUIUpdateTimer->setInterval(m_GUIUpdateIntervalMS);
}

void REG23RenderViewDialog::setVisible(bool visible)
{
  if (visible && m_GUIUpdateTimer && !m_GUIUpdateTimer->isActive())
    m_GUIUpdateTimer->start();
  this->QDialog::setVisible(visible);
}

void REG23RenderViewDialog::done(int r)
{
  if (m_GUIUpdateTimer && m_GUIUpdateTimer->isActive())
    m_GUIUpdateTimer->stop();
  this->QDialog::done(r);
}

void REG23RenderViewDialog::StyleCallback(vtkObject *caller, unsigned long eid,
    void *clientdata, void *calldata)
{
  REG23RenderViewDialog *dlg = reinterpret_cast<REG23RenderViewDialog *>(
      clientdata);
  if (clientdata)
  {
    if (eid == vtkCommand::StartInteractionEvent ||
        eid == vtkCommand::EndInteractionEvent)
    {
      if (eid == vtkCommand::StartInteractionEvent)
        dlg->m_LastStartedInteractionState = dlg->m_Style->GetState();
      dlg->AdaptRenderWidgetCursor();
      if (eid == vtkCommand::EndInteractionEvent)
      {
        dlg->AppendManualTransformationUndoRedoInfo();
        dlg->m_LastStartedInteractionState = VTKIS_NONE; // set back
      }
    }
    else if (eid == vtkPerspectiveOverlayProjectionInteractorStyle::TranslationEvent)
    {
      double *translation = reinterpret_cast<double *>(calldata);
      if (dlg->m_TransformationInitiatedByMouse) // no widget-update if keyboard
      {
        double dx = translation[0]; // dx in-plane (in pixels)
        double dy = translation[1]; // dy in-plane (in pixels)
        dlg->AdaptTranslationWidget(false, dx, dy);
      }
      else // "fake" initial pars
      {
        // store initial transform parameters:
        REG23Model::ParametersType pars = dlg->m_CastedModel->GetCurrentParameters();
        for (unsigned int k = 0; k < pars.Size(); k++)
          dlg->m_InitialTransformParameters[k] = pars[k];
      }
      double transl3D[3];
      transl3D[0] = translation[2];
      transl3D[1] = translation[3];
      transl3D[2] = translation[4];
      dlg->UpdateDRRAccordingToTranslationVector(transl3D);
      if (!dlg->m_TransformationInitiatedByMouse) // keyboard -> for undo/redo
      {
        dlg->m_LastStartedInteractionState = VTKIS_PERSPECTIVE_TRANSLATION;
        dlg->m_Style->InvokeEvent(vtkCommand::EndInteractionEvent);
      }
    }
    else if (eid == vtkPerspectiveOverlayProjectionInteractorStyle::RotationEvent)
    {
      double *rot = reinterpret_cast<double *>(calldata);
      double angle = rot[3]; // dr in degrees
      if (dlg->m_TransformationInitiatedByMouse) // no widget-update if keyboard
      {
        dlg->AdaptRotationWidget(false, angle);
      }
      else // "fake" initial rotation angle
      {
        dlg->m_LastManualRotationAngle = 0; // reset
      }
      double axis[3];
      axis[0] = rot[0];
      axis[1] = rot[1];
      axis[2] = rot[2];
      dlg->UpdateDRRAccordingToRotation(axis, angle);
      if (!dlg->m_TransformationInitiatedByMouse) // keyboard -> for undo/redo
      {
        dlg->m_LastStartedInteractionState = VTKIS_PERSPECTIVE_ROTATION;
        dlg->m_Style->InvokeEvent(vtkCommand::EndInteractionEvent);
      }
    }
  }
}

void REG23RenderViewDialog::AdaptRenderWidgetCursor()
{
  if (m_LastStartedInteractionState == VTKIS_PERSPECTIVE_TRANSLATION ||
      m_LastStartedInteractionState == VTKIS_PERSPECTIVE_ROTATION)
  {
    if (m_TranslationWidgetActor1->GetVisibility())
      m_TranslationWidgetActor1->SetVisibility(false);
    if (m_TranslationWidgetActor2->GetVisibility())
      m_TranslationWidgetActor2->SetVisibility(false);
    if (m_RotationWidgetActor1->GetVisibility())
      m_RotationWidgetActor1->SetVisibility(false);
    if (m_RotationWidgetActor2->GetVisibility())
      m_RotationWidgetActor2->SetVisibility(false);
  }
  m_TransformationInitiatedByMouse = false;
  QVTKWidget *rw = ui.RenderVTKWidget;
  if (!m_Style)
  {
    rw->setCursor(*m_DefaultCursor);
    return;
  }
  if (m_Style->GetState() == VTKIS_ZOOM)
  {
    rw->setCursor(*m_ZoomCursor);
  }
  else if (m_Style->GetState() == VTKIS_PAN)
  {
    rw->setCursor(*m_PanCursor);
  }
  else if (m_Style->GetState() == VTKIS_WINDOW_LEVEL)
  {
    rw->setCursor(*m_WindowLevelCursor);
  }
  else if (m_Style->GetState() == VTKIS_PERSPECTIVE_TRANSLATION)
  {
    if (!m_CastedModel->IsReadyForManualRegistration())
      return; // not allowed at the moment
    // store initial transform parameters:
    REG23Model::ParametersType pars = m_CastedModel->GetCurrentParameters();
    for (unsigned int k = 0; k < pars.Size(); k++)
      m_InitialTransformParameters[k] = pars[k];
    // adapt widget and cursor:
    m_TransformationInitiatedByMouse = true;
    AdaptTranslationWidget(true, 0, 0);
    rw->setCursor(*m_TranslateCursor);
  }
  else if (m_Style->GetState() == VTKIS_PERSPECTIVE_ROTATION)
  {
    if (!m_CastedModel->IsReadyForManualRegistration())
      return; // not allowed at the moment
    m_LastManualRotationAngle = 0; // reset
    m_CurrentRotationDirection = 0;
    // adapt widget and cursor:
    m_TransformationInitiatedByMouse = true;
    AdaptRotationWidget(true, 0);
    rw->setCursor(*m_RotateCursor);
  }
  else
  {
    rw->setCursor(*m_DefaultCursor);
  }
}

void REG23RenderViewDialog::BuildRenderPipeline()
{
  DestroyRenderPipeline();
  QVTKWidget *rw = ui.RenderVTKWidget;

  if (m_CastedModel)
    m_CastedModel->ConnectRenderWindowToGlobalLock(rw->GetRenderWindow());

  // (during build, we simply connect the empty image)

  // "VIEWER"
  vtkRenderWindow *renWin = rw->GetRenderWindow();
  if (!renWin)
    return;
  m_Renderer = vtkSmartPointer<vtkRenderer>::New();
  m_Renderer->SetBackground(rw->palette().color(rw->backgroundRole()).redF(),
      rw->palette().color(rw->backgroundRole()).greenF(),
      rw->palette().color(rw->backgroundRole()).blueF());
  renWin->AddRenderer(m_Renderer);
  m_Style = vtkSmartPointer<vtkPerspectiveOverlayProjectionInteractorStyle>::New();
  m_Style->SetWindowLevelMouseSensitivity(m_CastedModel->GetWindowingSensitivity());
  m_Style->SetRealTimeMouseSensitivityAdaption(m_CastedModel->GetRealTimeAdaptiveWindowing());
  m_Style->SetMotionFactor(50); // ITKSnap-like acceleration
  m_Style->UseMinimumMaximumSpacingForZoomOn();
  m_Style->SetMinimumSpacingForZoom(0.01); // 10 um
  m_Style->SetMaximumSpacingForZoom(10); // 10 mm
  m_Style->SetRenderer(m_Renderer);
  renWin->GetInteractor()->SetInteractorStyle(m_Style);
  vtkSmartPointer<vtkCallbackCommand> styleCB =
      vtkSmartPointer<vtkCallbackCommand>::New();
  styleCB->SetCallback(StyleCallback);
  styleCB->SetClientData(this);
  m_Style->AddObserver(vtkCommand::StartInteractionEvent, styleCB);
  m_Style->AddObserver(vtkCommand::EndInteractionEvent, styleCB);
  m_Style->AddObserver(
      vtkPerspectiveOverlayProjectionInteractorStyle::TranslationEvent, styleCB);
  m_Style->AddObserver(
      vtkPerspectiveOverlayProjectionInteractorStyle::RotationEvent, styleCB);
  m_Style->SetRegionSensitiveTransformNature(
      m_CastedModel->GetRegionSensitiveTransformNature());
  AdaptTransformNatureRegion();

  // FIXED IMAGE
  // - LUT
  m_FixedLUT = vtkSmartPointer<vtkLookupTable>::New();
  m_FixedLUT->SetNumberOfColors(4096);
  if (m_FixedImage)
    m_FixedLUT->SetTableRange(m_FixedImage->GetScalarRange());
  else
    m_FixedLUT->SetTableRange(m_EmptyImage->GetScalarRange());
  m_FixedLUT->SetHueRange(0, 0); // RED
//  m_FixedLUT->SetHueRange(0.3333333, 0.3333333); // GREEN
  m_FixedLUT->SetSaturationRange(1, 1);
  m_FixedLUT->SetValueRange(0, 1);
  m_FixedLUT->SetAlphaRange(1, 1);
  m_FixedLUT->Build();
  // - color mapper
  m_FixedColorMapper = vtkSmartPointer<vtkImageMapToColors>::New();
  if (m_FixedImage)
    m_FixedColorMapper->SetInput(m_FixedImage);
  else
    m_FixedColorMapper->SetInput(m_EmptyImage);
  m_FixedColorMapper->SetLookupTable(m_FixedLUT);
  m_FixedColorMapper->SetOutputFormatToRGB();
  double resetWL[2];
  resetWL[0] = 0; // dummy
  resetWL[1] = 0;
  m_Style->AddWindowLevelChannel(m_FixedColorMapper, resetWL);

  // MOVING IMAGE
  m_MovingLUT = vtkSmartPointer<vtkLookupTable>::New();
  m_MovingLUT->SetNumberOfColors(4096);
  if (m_CurrentMovingImage)
    m_MovingLUT->SetTableRange(m_CurrentMovingImage->GetScalarRange());
  else
    m_MovingLUT->SetTableRange(m_EmptyImage->GetScalarRange());
  m_MovingLUT->SetHueRange(0.3333333, 0.3333333); // GREEN
//  m_MovingLUT->SetHueRange(0.6666666, 0.6666666); // BLUE
  m_MovingLUT->SetSaturationRange(1, 1);
  m_MovingLUT->SetValueRange(0, 1);
  m_MovingLUT->SetAlphaRange(1, 1);
  m_MovingLUT->Build();
  // - color mapper
  m_MovingColorMapper = vtkSmartPointer<vtkImageMapToColors>::New();
  if (m_CurrentMovingImage)
    m_MovingColorMapper->SetInput(m_CurrentMovingImage);
  else
    m_MovingColorMapper->SetInput(m_EmptyImage);
  m_MovingColorMapper->SetLookupTable(m_MovingLUT);
  m_MovingColorMapper->SetOutputFormatToRGB();
  m_Style->AddWindowLevelChannel(m_MovingColorMapper, resetWL);

  // MASK IMAGE
  m_MaskLUT = vtkSmartPointer<vtkLookupTable>::New();
  if (m_MaskImage)
    m_MaskLUT->SetTableRange(m_MaskImage->GetScalarRange());
  else
    m_MaskLUT->SetTableRange(m_EmptyImage->GetScalarRange());
  double maskCol[3];
  m_CastedModel->GetMasksColorHSV(maskCol);
  m_MaskLUT->SetValueRange(0, maskCol[2]);
  m_MaskLUT->SetHueRange(maskCol[0], maskCol[0]);
  m_MaskLUT->SetSaturationRange(maskCol[1], maskCol[1]);
  m_MaskLUT->SetAlphaRange(1, 1);
  m_MaskLUT->Build();
  // - color mapper
  m_MaskColorMapper = vtkSmartPointer<vtkImageMapToColors>::New();
  if (m_MaskImage)
    m_MaskColorMapper->SetInput(m_MaskImage);
  else
    m_MaskColorMapper->SetInput(m_EmptyImage);
  m_MaskColorMapper->SetLookupTable(m_MaskLUT);
  m_MaskColorMapper->SetOutputFormatToRGB();

  // OVERLAY
  m_Blender = vtkSmartPointer<vtkImageBlend>::New();
  m_Blender->AddInputConnection(0, m_FixedColorMapper->GetOutputPort());
  m_Blender->AddInputConnection(0, m_MovingColorMapper->GetOutputPort());
  m_Blender->AddInputConnection(0, m_MaskColorMapper->GetOutputPort());
  m_Blender->SetOpacity(0, 0.5);
  if (m_CurrentMovingImage)
    m_Blender->SetOpacity(1, 0.5);
  else
    m_Blender->SetOpacity(1, 0.0);
  OnMaskToolButtonToggled(ui.MaskToolButton->isChecked()); // set opacity dependent on flags
  m_OverlayActor2D = vtkSmartPointer<vtkActor2D>::New();
  m_OverlayMapper2D = vtkSmartPointer<vtkImageMapper>::New();
  m_OverlayActor2D->SetMapper(m_OverlayMapper2D);
  m_OverlayActor2D->SetVisibility(false);
  m_OverlayMagnifier = vtkSmartPointer<vtkImageReslice>::New();
  m_OverlayMagnifier->SetInterpolationModeToLinear(); // enough for 2D-zoom
  m_OverlayMagnifier->SetInputConnection(m_Blender->GetOutputPort());
  m_OverlayMapper2D->SetInputConnection(m_OverlayMagnifier->GetOutputPort());
  m_Renderer->AddActor2D(m_OverlayActor2D);
  m_Style->SetReferenceImage(m_Blender->GetOutput());
  vtkSmartPointer<vtkMatrix4x4> imageOrientation =
        vtkSmartPointer<vtkMatrix4x4>::New();
  imageOrientation->Identity(); // 2D in-plane
  //m_Style->SetImageAxesOrientation(imageOrientation);
  m_Style->SetImageActor(m_OverlayActor2D);
  m_Style->SetMagnifier(m_OverlayMagnifier);
  m_Style->SetImageMapper(m_OverlayMapper2D);

  m_Style->FitImageToRenderWindow();
  rw->GetRenderWindow()->GetInteractor()->Render();
  m_Style->FitImageToRenderWindow();
}

void REG23RenderViewDialog::StretchOverlayWindowLevelOnDemand()
{
  if (m_StretchWindowLevel && m_Style && m_Blender->GetOutput())
  {
    m_Style->SetCurrentWindowLevelChannel(0); // main
    m_Style->InvokeEvent(vtkCommand::ResetWindowLevelEvent);
    m_Style->SetCurrentWindowLevelChannel(m_PreviousWLChannel); // restore
    m_StretchWindowLevel = false;
  }
}

void REG23RenderViewDialog::ExchangeFixedImageInRenderPipeline(
    vtkImageData *newFixedImage, bool unsharpMaskImage)
{
  AdaptTransformNatureRegion(); // just be sure
  vtkImageData *img = newFixedImage;
  if (!img)
  {
    img = m_EmptyImage;
    m_OverlayActor2D->SetVisibility(false);
    m_Blender->SetOpacity(0, 0.5);
  }
  else
  {
    m_OverlayActor2D->SetVisibility(true);
    QSlider *slider = ui.FixedMovingSlider;
    int value = slider->value();
    double drrOpacity = (double)(100 - value) / 100.;
    double xrayOpacity = 1 - drrOpacity;
    m_Blender->SetOpacity(0, xrayOpacity);
  }
  double *sr = new double[2];
  if (m_CastedModel)
  {
    int isFixedImage = 0; // neither fixed image nor unsharp mask image
    if ((vtkImageData *)m_FixedColorMapper->GetInput() == m_FixedImage)
      isFixedImage = 1; // fixed image
    else if ((vtkImageData *)m_FixedColorMapper->GetInput() == m_FixedImageUM)
      isFixedImage = 2; // unsharp mask image
    if (isFixedImage != 0 && unsharpMaskImage && m_FixedImageUMStoredWL)
    {
      sr[0] = m_FixedImageUMStoredWL[1] - m_FixedImageUMStoredWL[0] / 2.;
      sr[1] = m_FixedImageUMStoredWL[1] + m_FixedImageUMStoredWL[0] / 2.;
    }
    else if (isFixedImage != 0 && !unsharpMaskImage && m_FixedImageStoredWL)
    {
      sr[0] = m_FixedImageStoredWL[1] - m_FixedImageStoredWL[0] / 2.;
      sr[1] = m_FixedImageStoredWL[1] + m_FixedImageStoredWL[0] / 2.;
    }
    else
    {
      double *sr2 = m_CastedModel->GetWindowLevelMinMaxForFixedImage(img,
          (std::size_t)m_FixedImageIndex, unsharpMaskImage);
      sr[0] = sr2[0];
      sr[1] = sr2[1];
    }
  }
  if (!sr)
    sr = img->GetScalarRange();
  double *rsr = img->GetScalarRange();
  m_Style->OverrideResetWindowLevelByMinMax(1, rsr); // stretch to WHOLE range!
  m_FixedLUT->SetTableRange(sr);
  m_FixedLUT->Build();
  m_FixedColorMapper->SetInput(img);
  m_FixedColorMapper->Update();
  m_Blender->Update();
  m_OverlayMagnifier->Update();
  QVTKWidget *rw = ui.RenderVTKWidget;
  StretchOverlayWindowLevelOnDemand();
  rw->GetRenderWindow()->GetInteractor()->Render();
  for (int i = 1; i <= 2; i++)
  {
    m_Style->FitImageToRenderWindow();
    rw->GetRenderWindow()->GetInteractor()->Render();
  }
  EnableDisableFixedMovingSlider();
  delete[] sr;
}

void REG23RenderViewDialog::ExchangeMaskImageInRenderPipeline(
    vtkImageData *newMaskImage)
{
  AdaptTransformNatureRegion(); // just be sure
  vtkImageData *img = newMaskImage;
  if (!img)
    img = m_EmptyImage;
  OnMaskToolButtonToggled(ui.MaskToolButton->isChecked());
  m_MaskLUT->SetTableRange(img->GetScalarRange());
  m_MaskLUT->Build();
  m_MaskColorMapper->SetInput(img);
  m_MaskColorMapper->Update();
  m_Blender->Update();
  m_OverlayMagnifier->Update();

  QVTKWidget *rw = ui.RenderVTKWidget;
  StretchOverlayWindowLevelOnDemand();
  rw->GetRenderWindow()->GetInteractor()->Render();

  EnableDisableFixedMovingSlider();
}

void REG23RenderViewDialog::ExchangeCurrentMovingImageInRenderPipeline(
    vtkImageData *newMovingImage, bool isInitialImage)
{
  AdaptTransformNatureRegion(); // just be sure
  vtkImageData *img = newMovingImage;
  if (!img)
  {
    img = m_EmptyImage;
    m_Blender->SetOpacity(1, 0.0);
  }
  else
  {
    QSlider *slider = ui.FixedMovingSlider;
    int value = slider->value();
    double drrOpacity = (double)(100 - value) / 100.;
    m_Blender->SetOpacity(1, drrOpacity);
  }
  double sr[2];
  img->GetScalarRange(sr);
  m_Style->OverrideResetWindowLevelByMinMax(2, sr);
  if (isInitialImage)
  {
    m_MovingLUT->SetTableRange(sr);
    m_MovingLUT->Build();
  }
  m_MovingColorMapper->SetInput(img);
  m_MovingColorMapper->Update();
  m_Blender->Update();
  m_OverlayMagnifier->Update();

  QVTKWidget *rw = ui.RenderVTKWidget;
  StretchOverlayWindowLevelOnDemand();
  rw->GetRenderWindow()->GetInteractor()->Render();

  EnableDisableFixedMovingSlider();
}

void REG23RenderViewDialog::DestroyRenderPipeline()
{
  QVTKWidget *rw = ui.RenderVTKWidget;
  if (m_CastedModel)
    m_CastedModel->DisconnectRenderWindowFromGlobalLock(rw->GetRenderWindow());

  vtkRenderWindow *renWin = rw->GetRenderWindow();
  if (renWin && m_Renderer)
    renWin->RemoveRenderer(m_Renderer);
  m_Renderer = NULL;
  m_FixedLUT = NULL;
  m_FixedColorMapper = NULL;
  m_Blender = NULL;
  m_Style = NULL;
  m_MovingLUT = NULL;
  m_MovingColorMapper = NULL;
  m_OverlayActor2D = NULL;
  m_OverlayMapper2D = NULL;
}

void REG23RenderViewDialog::OnZoomFullToolButtonPressed()
{
  if (!m_Style)
    return;
  m_Style->FitImageToRenderWindow();
  QVTKWidget *rw = ui.RenderVTKWidget;
  rw->GetRenderWindow()->GetInteractor()->Render();
}

void REG23RenderViewDialog::OnWindowLevelFullToolButtonPressed()
{
  if (!m_Style)
    return;
  m_Style->InvokeEvent(vtkCommand::ResetWindowLevelEvent);
}

void REG23RenderViewDialog::resizeEvent(QResizeEvent *)
{
  if (!m_Style)
    return;
  m_Style->RestoreViewSettings();
  AdaptTransformNatureRegion();
}

void REG23RenderViewDialog::AdaptTransformNatureRegion()
{
  int *sz = m_Renderer->GetRenderWindow()->GetSize();
  double r[4];
  r[0] = TW_FXMIN * (double)sz[0]; /* in pixels */ \
  r[1] = TW_FYMIN * (double)sz[1];
  r[2] = TW_FXMAX * (double)sz[0];
  r[3] = TW_FYMAX * (double)sz[1];
  m_Style->SetTransformNatureRegion(r);
}

void REG23RenderViewDialog::CursorFromImageName(QString imageName,
    QString maskName, QCursor *&cursor, int hotx, int hoty)
{
  QPixmap bmpm(imageName, 0, Qt::MonoOnly);
  QBitmap bm(bmpm);
  QPixmap bmmaskpm(maskName, 0, Qt::MonoOnly);
  QBitmap bmmask(bmmaskpm);
  cursor = new QCursor(bm, bmmask, hotx, hoty);
}

void REG23RenderViewDialog::BuildRotationWidget()
{
  // passive part:
  vtkPoints *points;
  vtkSmartPointer<vtkCellArray> cells;
  vtkIdType pts[2];
  const int numpts1 = 6; // a long line and a cross (2 short lines)
  points = vtkPoints::New(VTK_DOUBLE);
  points->SetNumberOfPoints(numpts1);
  for (int i = 0; i < numpts1; i++)
    points->SetPoint(i, 0.0, 0.0, 0.0);
  // the widget's shape:
  cells = vtkSmartPointer<vtkCellArray>::New();
  cells->Allocate(cells->EstimateSize(numpts1, 2));
  // long line and 2 short lines of cross:
  for (int i = 0; i < numpts1; i += 2)
  {
    pts[0] = i; pts[1] = i + 1;
    cells->InsertNextCell(2, pts);
  }
  m_RotationWidgetPolyData1 = vtkSmartPointer<vtkPolyData>::New();
  m_RotationWidgetPolyData1->SetPoints(points);
  m_RotationWidgetPolyData1->SetLines(cells);
  points->Delete();
  // active part:
  const int numpts2 = 2; // a long line
  points = vtkPoints::New(VTK_DOUBLE);
  points->SetNumberOfPoints(numpts1);
  for (int i = 0; i < numpts2; i++)
    points->SetPoint(i, 0.0, 0.0, 0.0);
  // the widget's shape:
  cells = vtkSmartPointer<vtkCellArray>::New();
  cells->Allocate(cells->EstimateSize(numpts2, 2));
  // long line:
  pts[0] = 0; pts[1] = 1;
  cells->InsertNextCell(2, pts);
  m_RotationWidgetPolyData3 = vtkSmartPointer<vtkPolyData>::New();
  m_RotationWidgetPolyData3->SetPoints(points);
  m_RotationWidgetPolyData3->SetLines(cells);
  points->Delete();
  // active part (arc):
  m_RotationWidgetPolyData2 = vtkSmartPointer<vtkArcSource>::New();
  m_RotationWidgetPolyData2->SetCenter(0, 0, 0);
  m_RotationWidgetPolyData2->SetPoint1(0, 0, 0);
  m_RotationWidgetPolyData2->SetPoint2(0, 0, 0);
  // combined active part:
  m_RotationWidgetAppender = vtkSmartPointer<vtkAppendPolyData>::New();
  m_RotationWidgetAppender->AddInput(m_RotationWidgetPolyData2->GetOutput());
  m_RotationWidgetAppender->AddInput(m_RotationWidgetPolyData3);
  // representation pipeline:
  m_RotationWidgetMapper1 = NULL;
  m_RotationWidgetMapper1 = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_RotationWidgetMapper1->SetInput(m_RotationWidgetPolyData1);
  m_RotationWidgetActor1 = NULL;
  m_RotationWidgetActor1 = vtkSmartPointer<vtkActor2D>::New();
  m_RotationWidgetActor1->SetMapper(m_RotationWidgetMapper1);
  vtkSmartPointer<vtkProperty2D> prop1 = vtkSmartPointer<vtkProperty2D>::New();
  prop1->SetColor(0.2, 0.2, 1.0);
  prop1->SetLineWidth(1.0);
  prop1->SetLineStipplePattern(0xF0F0F0F0);
  m_RotationWidgetActor1->SetProperty(prop1);
  m_RotationWidgetActor1->SetVisibility(false);

  m_RotationWidgetMapper2 = NULL;
  m_RotationWidgetMapper2 = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_RotationWidgetMapper2->SetInput(m_RotationWidgetAppender->GetOutput());
  m_RotationWidgetActor2 = NULL;
  m_RotationWidgetActor2 = vtkSmartPointer<vtkActor2D>::New();
  m_RotationWidgetActor2->SetMapper(m_RotationWidgetMapper2);
  vtkSmartPointer<vtkProperty2D> prop2 = vtkSmartPointer<vtkProperty2D>::New();
  prop2->SetColor(0.2, 0.2, 1.0);
  prop2->SetLineWidth(1.0);
  m_RotationWidgetActor2->SetProperty(prop2);
  m_RotationWidgetActor2->SetVisibility(false);
  m_Renderer->AddActor2D(m_RotationWidgetActor1);
  m_Renderer->AddActor2D(m_RotationWidgetActor2);
}

void REG23RenderViewDialog::BuildTranslationWidget()
{
  vtkPoints *points;
  vtkSmartPointer<vtkCellArray> cells;
  vtkIdType pts[2];
  for (int j = 0; j < 2; j++)
  {
    const int numpts = 4; // a rectangle
    points = vtkPoints::New(VTK_DOUBLE);
    points->SetNumberOfPoints(numpts);
    for (int i = 0; i < numpts; i++)
      points->SetPoint(i, 0.0, 0.0, 0.0);
    // the widget's shape:
    cells = vtkSmartPointer<vtkCellArray>::New();
    cells->Allocate(cells->EstimateSize(numpts, 2));
    for (int i = 1; i <= (numpts - 1); i++)
    {
      pts[0] = i - 1; pts[1] = i;
      cells->InsertNextCell(2, pts);
    }
    pts[0] = numpts - 1; pts[1] = 0;
    cells->InsertNextCell(2, pts);
    if (j == 0)
    {
      m_TranslationWidgetPolyData1 = vtkSmartPointer<vtkPolyData>::New();
      m_TranslationWidgetPolyData1->SetPoints(points);
      m_TranslationWidgetPolyData1->SetLines(cells);
    }
    else
    {
      m_TranslationWidgetPolyData2 = vtkSmartPointer<vtkPolyData>::New();
      m_TranslationWidgetPolyData2->SetPoints(points);
      m_TranslationWidgetPolyData2->SetLines(cells);
    }
    points->Delete();
  }
  // representation pipeline:
  m_TranslationWidgetMapper1 = NULL;
  m_TranslationWidgetMapper1 = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_TranslationWidgetMapper1->SetInput(m_TranslationWidgetPolyData1);
  m_TranslationWidgetActor1 = NULL;
  m_TranslationWidgetActor1 = vtkSmartPointer<vtkActor2D>::New();
  m_TranslationWidgetActor1->SetMapper(m_TranslationWidgetMapper1);
  vtkSmartPointer<vtkProperty2D> prop1 = vtkSmartPointer<vtkProperty2D>::New();
  prop1->SetColor(0.2, 0.2, 1.0);
  prop1->SetLineWidth(1.0);
  prop1->SetLineStipplePattern(0xF0F0F0F0);
  m_TranslationWidgetActor1->SetProperty(prop1);
  m_TranslationWidgetActor1->SetVisibility(false);
  m_TranslationWidgetMapper2 = NULL;
  m_TranslationWidgetMapper2 = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_TranslationWidgetMapper2->SetInput(m_TranslationWidgetPolyData2);
  m_TranslationWidgetActor2 = NULL;
  m_TranslationWidgetActor2 = vtkSmartPointer<vtkActor2D>::New();
  m_TranslationWidgetActor2->SetMapper(m_TranslationWidgetMapper2);
  vtkSmartPointer<vtkProperty2D> prop2 = vtkSmartPointer<vtkProperty2D>::New();
  prop2->SetColor(0.2, 0.2, 1.0);
  prop2->SetLineWidth(1.0);
  m_TranslationWidgetActor2->SetProperty(prop2);
  m_TranslationWidgetActor2->SetVisibility(false);
  m_Renderer->AddActor2D(m_TranslationWidgetActor1);
  m_Renderer->AddActor2D(m_TranslationWidgetActor2);
}

void REG23RenderViewDialog::DestroyWidgets()
{
  m_TranslationWidgetActor1->SetMapper(NULL);
  m_TranslationWidgetActor1 = NULL;
  m_TranslationWidgetActor2->SetMapper(NULL);
  m_TranslationWidgetActor2 = NULL;
  m_TranslationWidgetMapper1->SetInput(NULL);
  m_TranslationWidgetMapper1 = NULL;
  m_TranslationWidgetMapper2->SetInput(NULL);
  m_TranslationWidgetMapper2 = NULL;
  m_TranslationWidgetPolyData1 = NULL;
  m_TranslationWidgetPolyData2 = NULL;

  m_RotationWidgetActor1->SetMapper(NULL);
  m_RotationWidgetActor1 = NULL;
  m_RotationWidgetActor2->SetMapper(NULL);
  m_RotationWidgetActor2 = NULL;
  m_RotationWidgetMapper1->SetInput(NULL);
  m_RotationWidgetMapper1 = NULL;
  m_RotationWidgetMapper2->SetInput(NULL);
  m_RotationWidgetMapper2 = NULL;
  m_RotationWidgetAppender->RemoveAllInputs();
  m_RotationWidgetAppender = NULL;
  m_RotationWidgetPolyData1 = NULL;
  m_RotationWidgetPolyData2 = NULL;
  m_RotationWidgetPolyData3 = NULL;
}

void REG23RenderViewDialog::AdaptRotationWidget(bool initialize, double dr)
{
  if (!m_CastedModel->IsReadyForManualRegistration())
    return; // not allowed at the moment

  const double fcross = 0.05; // relative cross line length
  const double crossminlen = 30; // minimum cross line length
  QToolButton *tb = ui.TransformationWidgetsToolButton;
  if (tb->isChecked())
  {
    // passive widget part:
    vtkPoints *ppoints = m_RotationWidgetPolyData1->GetPoints();
    if (initialize)
    {
      int x = m_Style->GetInteractor()->GetEventPosition()[0];
      int y = m_Style->GetInteractor()->GetEventPosition()[1];
      double p[3];
      p[2] = 0;
      int *sz = m_Renderer->GetRenderWindow()->GetSize();
      double crosslen = fcross * (double)sz[0];
      if ((fcross * (double)sz[1]) > crosslen)
        crosslen = fcross * (double)sz[1];
      if (crosslen < crossminlen)
        crosslen = crossminlen;
      double corVP[2];
      m_Style->ComputeCenterOfRotationInViewportCoordinates(corVP);
      p[0] = corVP[0];
      p[1] = corVP[1];
      ppoints->SetPoint(0, p);
      double v[3];
      v[0] = x - corVP[0];
      v[1] = y - corVP[1];
      v[2] = 0;
      vtkMath::Normalize(v);
      double len, len2;
      len = sqrt(pow(corVP[0] - 0, 2) + pow(corVP[1] - 0, 2));
      len2 = sqrt(pow(corVP[0] - sz[0], 2) + pow(corVP[1] - 0, 2));
      if (len2 > len)
        len = len2;
      len2 = sqrt(pow(corVP[0] - sz[0], 2) + pow(corVP[1] - sz[1], 2));
      if (len2 > len)
        len = len2;
      len2 = sqrt(pow(corVP[0], 2) + pow(corVP[1] - sz[1], 2));
      if (len2 > len)
        len = len2;
      p[0] = corVP[0] + v[0] * len;
      p[1] = corVP[1] + v[1] * len;
      ppoints->SetPoint(1, p);
      p[0] = corVP[0];
      p[1] = corVP[1] - crosslen / 2.;
      ppoints->SetPoint(2, p);
      p[0] = corVP[0];
      p[1] = corVP[1] + crosslen / 2.;
      ppoints->SetPoint(3, p);
      p[0] = corVP[0] - crosslen / 2.;
      p[1] = corVP[1];
      ppoints->SetPoint(4, p);
      p[0] = corVP[0] + crosslen / 2.;
      p[1] = corVP[1];
      ppoints->SetPoint(5, p);
      ppoints->GetData()->Modified();
      m_RotationWidgetPolyData1->Modified();
      // -> initialize arc:
      m_RotationWidgetPolyData2->SetCenter(corVP[0], corVP[1], 0);
      m_RotationWidgetPolyData2->SetPoint1(x, y, 0);
    }
    // active widget part:
    vtkSmartPointer<vtkTransform2D> t2d = vtkSmartPointer<vtkTransform2D>::New();
    t2d->Translate(m_RotationWidgetPolyData2->GetCenter()[0],
        m_RotationWidgetPolyData2->GetCenter()[1]);
    t2d->Rotate(-dr);
    t2d->Translate(-m_RotationWidgetPolyData2->GetCenter()[0],
        -m_RotationWidgetPolyData2->GetCenter()[1]);
    double ip[2];
    ip[0] = m_RotationWidgetPolyData2->GetPoint1()[0];
    ip[1] = m_RotationWidgetPolyData2->GetPoint1()[1];
    double op[2];
    t2d->TransformPoints(ip, op, 1);
    m_RotationWidgetPolyData2->SetPoint2(op[0], op[1], 0);
    double dr360 = dr;
    while (dr360 < 0)
      dr360 += 360;
    while (dr360 > 360)
      dr360 -= 360;
    double dr360old = m_LastManualRotationAngle;
    while (dr360old < 0)
      dr360old += 360;
    while (dr360old > 360)
      dr360old -= 360;
    if (m_CurrentRotationDirection == 0 ||
        fabs(dr360 - dr360old) > 300) // -> (re-)initialize rotation direction
    {
      if (dr360 > 300)
        m_CurrentRotationDirection = 1; // CW
      else
        m_CurrentRotationDirection = -1; // CCW
    }
    bool neg;
    int res;
    if (m_CurrentRotationDirection == -1) // CCW
    {
      neg = (dr360 > 180);
      res = static_cast<int>(dr360 + 10);
    }
    else // CW
    {
      neg = (dr360 < 180);
      res = static_cast<int>(370 - dr360);
    }
    m_RotationWidgetPolyData2->SetResolution(res);
    m_RotationWidgetPolyData2->SetNegative(neg);
    vtkPoints *apoints = m_RotationWidgetPolyData3->GetPoints();
    double p[3];
    p[0] = m_RotationWidgetPolyData2->GetCenter()[0];
    p[1] = m_RotationWidgetPolyData2->GetCenter()[1];
    p[2] = 0;
    apoints->SetPoint(0, p);
    double v[3];
    v[0] = op[0] - p[0];
    v[1] = op[1]- p[1];
    v[2] = 0;
    vtkMath::Normalize(v);
    double pr1[3];
    ppoints->GetPoint(0, pr1);
    double pr2[3];
    ppoints->GetPoint(1, pr2);
    double len = sqrt(pow(pr1[0] - pr2[0], 2) + pow(pr1[1] - pr2[1], 2));
    p[0] = p[0] + v[0] * len;
    p[1] = p[1] + v[1] * len;
    p[2] = 0;
    apoints->SetPoint(1, p);
    apoints->GetData()->Modified();
    m_RotationWidgetPolyData3->Modified();

    if (!m_RotationWidgetActor1->GetVisibility())
      m_RotationWidgetActor1->SetVisibility(true);
    if (!m_RotationWidgetActor2->GetVisibility())
      m_RotationWidgetActor2->SetVisibility(true);
    m_Renderer->GetRenderWindow()->Render();
  }
}

void REG23RenderViewDialog::AdaptTranslationWidget(bool initialize, double dx,
    double dy)
{
  if (!m_CastedModel->IsReadyForManualRegistration())
    return; // not allowed at the moment

  QToolButton *tb = ui.TransformationWidgetsToolButton;
  if (tb->isChecked())
  {
    // passive widget part:
    vtkPoints *ppoints = m_TranslationWidgetPolyData1->GetPoints();
    if (initialize)
    {
      // NOTE: Something to think about: the factors are used for specifying the
      // approximate rectangle -> we back-project these corner points onto
      // a plane which covers the 3D center of rotation and is parallel to
      // the image plane. These back-projected corner points are the reference.
      int *sz = m_Renderer->GetRenderWindow()->GetSize();
      double p[3];
// shorter code:
#define COMPUTE_3D_POINT(xfac, yfac, idx) \
      p[0] = xfac * (double)sz[0]; /* in pixels */ \
      p[1] = yfac * (double)sz[1]; \
      p[2] = 0; /* ignored */ \
      ppoints->SetPoint(idx, p);

      COMPUTE_3D_POINT(TW_FXMIN, TW_FYMIN, 0)
      COMPUTE_3D_POINT(TW_FXMAX, TW_FYMIN, 1)
      COMPUTE_3D_POINT(TW_FXMAX, TW_FYMAX, 2)
      COMPUTE_3D_POINT(TW_FXMIN, TW_FYMAX, 3)
      ppoints->GetData()->Modified();
      m_TranslationWidgetPolyData1->Modified();
    }
    // active widget part:
    vtkPoints *apoints = m_TranslationWidgetPolyData2->GetPoints();
    double p[3];
    for (vtkIdType i = 0; i < ppoints->GetNumberOfPoints(); i++)
    {
      ppoints->GetPoint(i, p);
      p[0] += dx;
      p[1] += dy;
      apoints->SetPoint(i, p);
    }
    apoints->GetData()->Modified();
    m_TranslationWidgetPolyData2->Modified();

    if (!m_TranslationWidgetActor1->GetVisibility())
      m_TranslationWidgetActor1->SetVisibility(true);
    if (!m_TranslationWidgetActor2->GetVisibility())
      m_TranslationWidgetActor2->SetVisibility(true);
    m_Renderer->GetRenderWindow()->Render();
  }
}

void REG23RenderViewDialog::UpdateDRRAccordingToTranslationVector(
    double translation[3])
{
  if (!m_CastedModel->IsReadyForManualRegistration())
    return; // not allowed at the moment

  REG23Model::ParametersType pars(6);
  for (int k = 0; k < 6; k++)
    pars[k] = m_InitialTransformParameters[k];
  pars[3] += translation[0]; // add offset
  pars[4] += translation[1];
  pars[5] += translation[2];
  m_CastedModel->OverrideAndApplyCurrentParameters(pars);

  ITKVTKImage *ivi = m_CastedModel->GetVolumeImage();
  TEMPLATE_CALL_COMP(ivi->GetComponentType(),
      m_CastedModel->ComputeCurrentMovingImages)
}

void REG23RenderViewDialog::UpdateDRRAccordingToRotation(double axis[3],
    double angle)
{
  if (!m_CastedModel->IsReadyForManualRegistration())
    return; // not allowed at the moment

  double dangle = angle - m_LastManualRotationAngle;
  m_LastManualRotationAngle = angle;

  vtkMath::Normalize(axis);
  m_CastedModel->ConcatenateAndApplyRelativeAxisAngleRotation(axis,
      vtkMath::RadiansFromDegrees(dangle));

  ITKVTKImage *ivi = m_CastedModel->GetVolumeImage();
  TEMPLATE_CALL_COMP(ivi->GetComponentType(),
      m_CastedModel->ComputeCurrentMovingImages)
}

void REG23RenderViewDialog::OnFixedMovingSliderValueChanged(int value)
{
  if (ui.FixedMovingSlider->isEnabled())
  {
    double drrOpacity = (double)(100 - value) / 100.;
    double xrayOpacity = 1 - drrOpacity;
    m_Blender->SetOpacity(0, xrayOpacity);
    m_Blender->SetOpacity(1, drrOpacity);
    m_Blender->Update();
    m_OverlayMagnifier->Update();
    m_Style->SetCurrentWindowLevelChannel(0); // main
    m_Style->InvokeEvent(vtkCommand::ResetWindowLevelEvent);
    m_Style->SetCurrentWindowLevelChannel(m_PreviousWLChannel); // restore
    m_Renderer->GetRenderWindow()->Render();
    if (value == 0) // full DRR
      m_Style->SetCurrentWindowLevelChannel(2); // DRR channel
    else if (value == 100) // full X-ray
      m_Style->SetCurrentWindowLevelChannel(1); // X-ray channel
    else
      m_Style->SetCurrentWindowLevelChannel(m_PreviousWLChannel); // restore
    AdaptWindowLevelIconsTextsAndMenus();
  }
}

void REG23RenderViewDialog::OnFixedImageToolButtonPressed()
{
  QSlider *sl = ui.FixedMovingSlider;
  sl->setValue(100);
}

void REG23RenderViewDialog::OnMovingImageToolButtonPressed()
{
  QSlider *sl = ui.FixedMovingSlider;
  sl->setValue(0);
}

void REG23RenderViewDialog::OnFixedMovingSliderDoubleClick()
{
  QSlider *sl = ui.FixedMovingSlider;
  sl->setValue(50);
}

void REG23RenderViewDialog::OnFixedMovingSliderRequestSliderInformationToolTipText(
    QString &text, int value)
{
  text = REG23RenderViewDialog::tr("X-ray: %1 %, DRR: %2 %").arg(100 - value).
      arg(value);
}

void REG23RenderViewDialog::EnableDisableFixedMovingSlider()
{
  bool enabled = false;
  if (m_Blender && m_FixedColorMapper && m_MovingColorMapper &&
      m_FixedImage && m_CurrentMovingImage)
    enabled = true;
  ui.FixedMovingSlider->setEnabled(enabled);
  ui.FixedImageToolButton->setEnabled(enabled);
  ui.MovingImageToolButton->setEnabled(enabled);
}

void REG23RenderViewDialog::OnMaskToolButtonToggled(bool value)
{
  if (m_MaskImage && m_Blender && m_Blender->GetNumberOfInputs() >= 3)
  {
    QVTKWidget *rw = ui.RenderVTKWidget;
    if (value)
      m_Blender->SetOpacity(2, 0.5);
    else
      m_Blender->SetOpacity(2, 0.0);
    m_StretchWindowLevel = true;
    StretchOverlayWindowLevelOnDemand();
    rw->GetRenderWindow()->GetInteractor()->Render();
  }
}

void REG23RenderViewDialog::OnCrossCorrelationInitialTransformButtonClicked()
{
  QCursor cwait(Qt::WaitCursor);
  QApplication::setOverrideCursor(cwait);

  // Compute transformation from moving image to fixed image
  // The result is stored as x/y-translation in [mm]
  double tx = 0;
  double ty = 0;
  bool success = false;
  vtkSmartPointer<vtkTimerLog> tl = vtkSmartPointer<vtkTimerLog>::New();
  tl->StartTimer();
  ITKVTKImage *ivi = m_CastedModel->GetVolumeImage();
  std::vector<double> args;
  args.push_back(-40);
  args.push_back(40);
  args.push_back(-40);
  args.push_back(40);
  args.push_back(-40);
  args.push_back(40);
  args.push_back(81);
  args.push_back(81);
  std::string structureUID = "SkelettStructure1";
  TEMPLATE_CALL_COMP(ivi->GetComponentType(),
      success = m_CastedModel->ComputeCrossCorrelationInitialTransform,
      m_FixedImageIndex, tx, ty, true, true, false, args, structureUID)
  if (!success || vcl_sqrt(tx * tx + ty * ty) > 100)
  {
      TEMPLATE_CALL_COMP(ivi->GetComponentType(),
      success = m_CastedModel->ComputeCrossCorrelationInitialTransform,
      m_FixedImageIndex, tx, ty, true, false, false, args, structureUID)
  }

  if (success)
  {
    // compute resultant 3D translation vector:
    tx = -tx; // invert result!
    ty = -ty;
    REG23Model::ParametersType pars = m_CastedModel->
        ComputeTransformationFromPhysicalInPlaneTranslation(m_FixedImageIndex,
            tx, ty);
    tl->StopTimer();
    // update model and user-interface:
    m_CastedModel->OverrideAndApplyCurrentParameters(pars);
    TEMPLATE_CALL_COMP(ivi->GetComponentType(),
        m_CastedModel->ComputeCurrentMovingImages)

    double regTime = tl->GetElapsedTime();
    bool userCancel = false;
    SparseAutoRegistrationTransformTask *urt = new SparseAutoRegistrationTransformTask();
    urt->SetUserCancel(userCancel);
    urt->SetRegistrationTime(regTime);
    urt->SetNumberOfIterations(1);
    urt->SetRegistrationType(SparseAutoRegistrationTransformTask::RT_CROSS_CORRELATION);
    urt->SetParameters(pars);
    SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
    undoRedoManager->ComputeSimulatedRelativeParameters(urt);
    if (urt->ImpliesRelativeTransformation()) // really implies transform
      undoRedoManager->ReportTask(urt); // add to undo/redo queue
    else // not really a notable relative transform - forget it
      delete urt;
  }

  QApplication::restoreOverrideCursor();
}

void REG23RenderViewDialog::AppendManualTransformationUndoRedoInfo()
{
  if (!m_CastedModel || !m_CastedModel->IsReadyForManualRegistration())
    return;

  ManualRegistrationTransformTask *urt = new ManualRegistrationTransformTask();
  urt->SetParameters(m_CastedModel->GetCurrentParameters());
  if (m_LastStartedInteractionState == VTKIS_PERSPECTIVE_ROTATION)
    urt->SetIsRotation(true);
  else
    urt->SetIsRotation(false);
  SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
  undoRedoManager->ComputeSimulatedRelativeParameters(urt);
  if (urt->ImpliesRelativeTransformation()) // does really imply a transform
  {
    // each relevant manual transformation makes automatic sparse
    // pre-registration obsolete:
    m_CastedModel->SetExecuteSparsePreRegistration(false);

    undoRedoManager->ReportTask(urt);
  }
  else
  {
    delete urt; // delete again
  }
}

void REG23RenderViewDialog::AdaptWindowLevelIconsTextsAndMenus()
{
  if (!m_FixedWLAction || !m_MovingWLAction || !m_Style)
    return;

  QIcon *ico1 = NULL;
  QIcon *ico2 = NULL;

  m_BlockWLActionToggle = true;
  QString selectedChannelText;
  if (m_Style->GetCurrentWindowLevelChannel() == 0)
  {
    ico1 = m_WLIcons[0];
    ico2 = m_WLIcons[2];

    m_FixedWLAction->setChecked(false);
    m_MovingWLAction->setChecked(false);
    selectedChannelText = REG23RenderViewDialog::tr("OVERLAY");
  }
  else if (m_Style->GetCurrentWindowLevelChannel() == 1) // X-ray
  {
    ico1 = m_WLIcons[1];
    ico2 = m_WLIcons[2];
    m_FixedWLAction->setChecked(true);
    m_MovingWLAction->setChecked(false);
    selectedChannelText = REG23RenderViewDialog::tr("X-RAY");
  }
  else //if (m_Style->GetCurrentWindowLevelChannel() == 2) // DRR
  {
    ico1 = m_WLIcons[0];
    ico2 = m_WLIcons[3];
    m_FixedWLAction->setChecked(false);
    m_MovingWLAction->setChecked(true);
    selectedChannelText = REG23RenderViewDialog::tr("DRR");
  }

  ui.WindowLevelFullToolButton->setToolTip(REG23RenderViewDialog::tr(
      "Set window/level of %1 to full intensity range (or press CTRL+R).").arg(
          selectedChannelText));

  if (ui.FixedMovingSlider->value() == 0 ||
      ui.FixedMovingSlider->value() == 100)
  {
    m_FixedWLAction->setEnabled(false); // fixed state!
    m_MovingWLAction->setEnabled(false);
  }
  else
  {
    m_FixedWLAction->setEnabled(true); // state is changeable
    m_MovingWLAction->setEnabled(true);
  }

  m_BlockWLActionToggle = false;

  QToolButton *tb = ui.FixedImageToolButton;
  tb->setIcon(*ico1);
  tb = ui.MovingImageToolButton;
  tb->setIcon(*ico2);
}

void REG23RenderViewDialog::OnFixedWLMenuToggled(bool checked)
{
  if (!m_BlockWLActionToggle)
  {
    if (checked)
      m_PreviousWLChannel = 1;
    else if (!checked && m_PreviousWLChannel == 1)
      m_PreviousWLChannel = 0;
    m_Style->SetCurrentWindowLevelChannel(m_PreviousWLChannel);
    AdaptWindowLevelIconsTextsAndMenus();
  }
}

void REG23RenderViewDialog::OnMovingWLMenuToggled(bool checked)
{
  if (!m_BlockWLActionToggle)
  {
    if (checked)
      m_PreviousWLChannel = 2;
    else if (!checked && m_PreviousWLChannel == 2)
      m_PreviousWLChannel = 0;
    m_Style->SetCurrentWindowLevelChannel(m_PreviousWLChannel);
    AdaptWindowLevelIconsTextsAndMenus();
  }
}

void REG23RenderViewDialog::OnUnsharpMaskToolButtonToggled(bool checked)
{
  if (m_BlockUMToggle)
    return;

  if (m_FixedImage)
  {
    if (m_FixedImageReceivedOnce) // not valid before first retrieval!
    {
      int isFixedImage = 0; // neither fixed image nor unsharp mask image
      if ((vtkImageData *)m_FixedColorMapper->GetInput() == m_FixedImage)
        isFixedImage = 1; // fixed image
      else if ((vtkImageData *)m_FixedColorMapper->GetInput() == m_FixedImageUM)
        isFixedImage = 2; // unsharp mask image
      if (checked && isFixedImage == 1) // fixed image -> unsharp mask image
      {
        // store current window/level
        if (!m_FixedImageStoredWL)
          m_FixedImageStoredWL = new double[2];
        double tr[2];
        m_FixedLUT->GetTableRange(tr);
        m_FixedImageStoredWL[0] = tr[1] - tr[0];
        m_FixedImageStoredWL[1] = tr[0] + m_FixedImageStoredWL[0] / 2.;
      }
      else if (!checked && isFixedImage == 2) // unsharp mask image -> fixed image
      {
        // store current window/level
        if (!m_FixedImageUMStoredWL)
          m_FixedImageUMStoredWL = new double[2];
        double tr[2];
        m_FixedLUT->GetTableRange(tr);
        m_FixedImageUMStoredWL[0] = tr[1] - tr[0];
        m_FixedImageUMStoredWL[1] = tr[0] + m_FixedImageUMStoredWL[0] / 2.;
      }
    }
    if (checked)
    {
      if (!m_FixedImageUM) // not created, yet
      {
        m_FixedImageUM = vtkSmartPointer<vtkImageData>::New();
        vtkSmartPointer<VTKUnsharpMaskingImageFilter> f =
            vtkSmartPointer<VTKUnsharpMaskingImageFilter>::New();
        f->SetInput(m_FixedImage);
        f->SetAutoRadius(true);
        f->Update();
        m_FixedImageUM->DeepCopy(f->GetOutput());
        f = NULL;
      }
      ExchangeFixedImageInRenderPipeline(m_FixedImageUM, true);
    }
    else
    {
      ExchangeFixedImageInRenderPipeline(m_FixedImage, false);
    }
  }
  else
  {
    ExchangeFixedImageInRenderPipeline(NULL, false);
  }
}

void REG23RenderViewDialog::StoreWindowLevel()
{
  if (m_CastedModel && m_CastedModel->IsReadyForAutoRegistration())
  {
    double fwl[2];
    if ((vtkImageData *)m_FixedColorMapper->GetInput() == m_FixedImage)
    {
      double tr[2];
      m_FixedLUT->GetTableRange(tr); // -> extract directly from current LUT
      fwl[0] = tr[1] - tr[0];
      fwl[1] = tr[0] + fwl[0] / 2.;
    }
    else if (m_FixedImageStoredWL)
    {
      fwl[0] = m_FixedImageStoredWL[0]; // -> take the stored fixed values
      fwl[1] = m_FixedImageStoredWL[1];
    }
    else if (m_CastedModel->IsUnsharpMaskingEnabled(m_FixedImageIndex + 1))
    {
      // -> extract from initial window/level (has not been stored, yet!)
      double *tr = m_CastedModel->GetWindowLevelMinMaxForFixedImage(m_FixedImage,
          (std::size_t)m_FixedImageIndex, false);
      if (tr)
      {
        fwl[0] = tr[1] - tr[0];
        fwl[1] = tr[0] + fwl[0] / 2.;
      }
      else
      {
        return;
      }
    }
    else // no values available - ERROR ?!
    {
      return;
    }
    double uwl[2];
    if (m_CastedModel->IsUnsharpMaskingEnabled(m_FixedImageIndex + 1))
    {
      if ((vtkImageData *)m_FixedColorMapper->GetInput() == m_FixedImageUM)
      {
        double tr[2];
        m_FixedLUT->GetTableRange(tr); // -> extract directly from current LUT
        uwl[0] = tr[1] - tr[0];
        uwl[1] = tr[0] + uwl[0] / 2.;
      }
      else if (m_FixedImageUMStoredWL)
      {
        uwl[0] = m_FixedImageUMStoredWL[0]; // -> take the stored unsharp values
        uwl[1] = m_FixedImageUMStoredWL[1];
      }
      else
      {
        // -> extract from initial window/level (has not been stored, yet!)
        double *tr = m_CastedModel->GetWindowLevelMinMaxForFixedImage(
            m_FixedImageUM, (std::size_t)m_FixedImageIndex, true);
        if (tr)
        {
          uwl[0] = tr[1] - tr[0];
          uwl[1] = tr[0] + uwl[0] / 2.;
        }
        else
        {
          return;
        }
      }
    } // else: not written into file, value do not matter ...
    m_CastedModel->StoreWindowLevelToFile(m_FixedImageIndex, fwl, uwl);
  }
}

bool REG23RenderViewDialog::StoreRenderWindowImage(const QString &outDir,
    const QString &outPattern, const unsigned int &index,
    const int &blendvalue, const int &magnification)
{
  if (!itksys::SystemTools::FileExists(outDir.toStdString().c_str())
      || outPattern.length() <= 0 || !m_CastedModel
      || !m_CastedModel->IsReadyForManualRegistration())
    return false;

  if (blendvalue >= 0 && blendvalue <= 100)
  {
    ui.FixedMovingSlider->setValue(blendvalue);
  }

  QString filename = outDir;
  filename += QDir::separator();
  filename += outPattern;
  filename = filename.arg(m_FixedImageIndex + 1, 3, 10, QChar('0'));
  filename = filename.arg(ui.FixedMovingSlider->value(), 3, 10, QChar('0'));
  if(index > 0)
    filename = filename.arg(index, 3, 10, QChar('0'));

  // Screenshot
  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<
      vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(m_Renderer->GetRenderWindow());
  windowToImageFilter->SetMagnification(magnification);
  // also record the alpha (transparency) channel
  // windowToImageFilter->SetInputBufferTypeToRGBA();
  windowToImageFilter->Update();

  vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
  writer->SetFileName(filename.toStdString().c_str());
  writer->SetInput(windowToImageFilter->GetOutput());
  writer->Write();

  return true;
}

}
