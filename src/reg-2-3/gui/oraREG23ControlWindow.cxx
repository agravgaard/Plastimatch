/*
 TRANSLATOR ora::REG23ControlWindow

 lupdate: Qt-based translation with NAMESPACE-support!
 */

#include <QStyleFactory>
#include <QTimer>
#include <QLabel>
#include <QToolButton>
#include <QMessageBox>
#include <QMutexLocker>
#include <QMenu>
#include <QCloseEvent>
#include <QLocale>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QtWebKit/QWebView>

#include "oraREG23ControlWindow.h"
#include "oraREG23Model.h"
#include "oraREG23RenderViewDialog.h"
#include "oraREG23RegistrationInitializationTask.h"
#include "oraREG23RegistrationExecutionTask.h"
#include "oraREG23SparseRegistrationExecutionTask.h"
#include "oraREG23TaskPresentationWidget.h"
#include "oraSimpleTransformUndoRedo.h"
#include "oraInitialTransformTask.h"
#include "oraAutoRegistrationTransformTask.h"
#include "oraSparseAutoRegistrationTransformTask.h"
#include "oraReferenceTransformTask.h"
#include "oraREG23AboutDialog.h"

// ORAIFModel
#include <oraTask.h>
#include <oraImageImporterTask.h>

#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkMath.h>

namespace ora 
{

REG23ControlWindow::REG23ControlWindow(QWidget *parent) :
  QMainWindow(parent), ViewController()
{
  ui.setupUi(this);
  m_NoGUI = false;
  m_CastedModel = NULL;
  m_Initializing = false;
  m_Initialized = false;
  m_MainTimer = NULL;
  m_CurrentTask = NULL;
  m_StatusWidget = NULL;
  m_LockRenderViewButtonsEffect = false;
  m_UpdateCostFunctionFlag = false;
  m_GUIUpdateIntervalMS = 200;
  for (unsigned int i = 0; i < 6; i++)
    m_CurrentRawParameters[i] = 0;
  m_UpdateCurrentParameters = false;
  m_CurrentStyle = "plastique"; // default
  m_VolumeStudyInfoFormatString = "";
  m_VolumeSeriesInfoFormatString = "";
  m_UndoMenu = NULL;
  m_RedoMenu = NULL;
  m_OKButtonPressed = false;
  m_StayOnTop = false;
  m_ApplicationIsShuttingDown = false;
  m_WindowTitle = this->windowTitle();
  m_StoredCurveVisibility.clear();
}

REG23ControlWindow::~REG23ControlWindow()
{
  if (m_MainTimer)
    delete m_MainTimer;
  m_MainTimer = NULL;
  if (m_GUIUpdateTimer)
    delete m_GUIUpdateTimer;
  m_GUIUpdateTimer = NULL;
  if (m_UndoMenu)
    delete m_UndoMenu;
  if (m_RedoMenu)
    delete m_RedoMenu;
}

void REG23ControlWindow::Initialize()
{
  m_Initializing = true;

  // -> set graphical style:
  QApplication::setStyle(QStyleFactory::create(m_CurrentStyle));
  QApplication::setPalette(QApplication::style()->standardPalette());

  m_CastedModel = dynamic_cast<REG23Model *>(this->m_Model); // concrete type

  m_MainTimer = new QTimer();
  m_MainTimer->setSingleShot(true);
  m_MainTimer->setInterval(100); // some delay
  this->connect(m_MainTimer, SIGNAL(timeout()), this,
      SLOT(OnMainTimerTimeout()));
  m_MainTimer->start();

  // connect important task manager events:
  this->connect(m_CastedModel->GetTaskManager(),
      SIGNAL(TaskHasNoInputsDropped(Task*)), this,
      SLOT(OnTaskManagerTaskHasNoInputsDropped(Task*)), Qt::BlockingQueuedConnection);

  // set the status bar visible
  // (old code <- created and added stask pres. widget to status bar)
//  m_StatusWidget = new REG23TaskPresentationWidget(this);
//  m_StatusWidget->setVisible(true);
//  statusBar()->addPermanentWidget(m_StatusWidget, true);
//  statusBar()->setSizeGripEnabled(false);
//  statusBar()->setVisible(true);
  m_StatusWidget = ui.ProgressPresentationWidget;
  m_StatusWidget->SetCancelButtonToolTip(REG23ControlWindow::tr(
      "Stop current running process."));
  m_StatusWidget->SetStartButtonToolTip(REG23ControlWindow::tr(
      "Start a new auto-registration process."));

  // hidden render window:
  ui.HiddenVTKWidget->setVisible(false);
  m_CastedModel->SetToolRenderWindow(ui.HiddenVTKWidget->GetRenderWindow());
  ui.HiddenVTKRenderWidget->setVisible(false);
  vtkRenderWindow *drrRenWin = ui.HiddenVTKRenderWidget->GetRenderWindow();
  vtkSmartPointer<vtkRenderer> drrRen = vtkSmartPointer<vtkRenderer>::New();
  drrRenWin->GetRenderers()->RemoveAllItems();
  drrRenWin->AddRenderer(drrRen);
  m_CastedModel->SetDRRToolRenderWindow(drrRenWin);

  // cost function widgets:
  XYPlotWidget *xyp = ui.CostFunctionWidget;
  xyp->SetXAxisLabel(REG23ControlWindow::tr("Iterations").toStdString());
  xyp->SetXTicksNumberFormat('i');
  xyp->SetYAxisLabel(REG23ControlWindow::tr("Cost Function").toStdString());
  xyp->SetYTicksNumberFormat('f');
  xyp->SetXTicksNumberPrecision(3);
  xyp->ClearCurve(0);
  xyp->SetCustomText(XYPlotWidget::CTT_CSV_SAVE_DLG_DEF_FILE,
      REG23ControlWindow::tr("curves.csv"));
  xyp->SetCustomText(XYPlotWidget::CTT_CSV_SAVE_DLG_FILE_FILTER,
      REG23ControlWindow::tr("CSV sheet (*.csv)"));
  xyp->SetCustomText(XYPlotWidget::CTT_CSV_SAVE_DLG_TITLE,
      REG23ControlWindow::tr("Export curves to CSV sheet ..."));
  xyp->SetCustomText(XYPlotWidget::CTT_CSV_TT,
      REG23ControlWindow::tr("Export all curves to a single CSV sheet."));
  xyp->SetCustomText(XYPlotWidget::CTT_VIS_CMB_TT,
      REG23ControlWindow::tr("Specify which curves should be displayed."));
  xyp->SetCustomText(XYPlotWidget::CTT_ZOOM_FULL_TT,
      REG23ControlWindow::tr("Fit display to extent of all visible curves."));
  xyp->SetCustomText(XYPlotWidget::CTT_ZOOM_IN_TT,
      REG23ControlWindow::tr("Zoom in."));
  xyp->SetCustomText(XYPlotWidget::CTT_ZOOM_OUT_TT,
      REG23ControlWindow::tr("Zoom out."));
  xyp->setVisible(m_CastedModel->GetRenderCostFunction());
  ui.CostFunctionTitleLabel->setVisible(m_CastedModel->GetRenderCostFunction());

  // transformation widgets:
  m_RotationsText = ui.RotationsLabel->text();
  m_TranslationsText = ui.TranslationsLabel->text();
  ui.RotationsLabel->setText(m_RotationsText.arg("--").arg("--").arg("--"));
  ui.TranslationsLabel->setText(m_TranslationsText.arg("--").arg("--").arg("--"));

  // GUI updates:
  m_GUIUpdateTimer = new QTimer(this);
  m_GUIUpdateTimer->setInterval(m_GUIUpdateIntervalMS);
  this->connect(m_GUIUpdateTimer, SIGNAL(timeout()),
      this, SLOT(OnGUIUpdateTimerTimeout()));
  m_GUIUpdateTimer->setSingleShot(false);
  m_GUIUpdateTimer->start();

  // events
  this->connect(ui.OKButton, SIGNAL(pressed()), this, SLOT(OnOKButtonPressed()));
  this->connect(ui.CancelButton, SIGNAL(pressed()), this,
      SLOT(OnCancelButtonPressed()));
  this->connect(ui.LoadConfigButton, SIGNAL(pressed()), this,
      SLOT(OnLoadConfigButtonPressed()));
  this->connect(m_StatusWidget, SIGNAL(UserCancelRequest()), this,
      SLOT(OnCancelStatusButtonPressed()));
  this->connect(m_StatusWidget, SIGNAL(UserStartRequest()), this,
      SLOT(OnStartButtonPressed()));
  SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
  this->connect(undoRedoManager, SIGNAL(UndoRedoStackModified()), this,
      SLOT(OnUndoRedoManagerStackModified()));
  this->connect(undoRedoManager, SIGNAL(TaskExecutionRequest(AbstractTransformTask*,bool &)), this,
      SLOT(OnUndoRedoManagerTaskExecutionRequest(AbstractTransformTask*,bool&)));

  m_UndoMenu = new QMenu(ui.UndoToolButton);
  this->connect(m_UndoMenu, SIGNAL(triggered(QAction*)), this,
      SLOT(OnUndoMenuTriggered(QAction*)));
  ui.UndoToolButton->setMenu(m_UndoMenu);
  m_RedoMenu = new QMenu(ui.RedoToolButton);
  this->connect(m_RedoMenu, SIGNAL(triggered(QAction*)), this,
      SLOT(OnRedoMenuTriggered(QAction*)));
  ui.RedoToolButton->setMenu(m_RedoMenu);

  this->connect(ui.UndoToolButton, SIGNAL(clicked()), this,
      SLOT(OnUndoToolButtonClicked()));
  this->connect(ui.RedoToolButton, SIGNAL(clicked()), this,
      SLOT(OnRedoToolButtonClicked()));
  this->connect(ui.ResetToolButton, SIGNAL(clicked()), this,
      SLOT(OnResetToolButtonClicked()));

  this->connect(ui.AboutButton, SIGNAL(pressed()), this,
      SLOT(OnAboutButtonPressed()));
  this->connect(ui.HelpButton, SIGNAL(pressed()), this,
      SLOT(OnHelpButtonPressed()));

  // Science mode buttons:
  this->connect(ui.ReferenceToolButton, SIGNAL(clicked()), this,
      SLOT(OnReferenceToolButtonClicked()));
  ui.ReferenceToolButton->setVisible(false);
  ui.ITFOptimizerToolButton->setVisible(false);
  this->connect(ui.ITFOptimizerToolButton, SIGNAL(clicked()), this,
      SLOT(OnITFOptimizerToolButtonClicked()));
  ui.SaveDRRsToolButton->setVisible(false);
  this->connect(ui.SaveDRRsToolButton, SIGNAL(clicked()), this,
      SLOT(OnSaveDRRsToolButtonClicked()));
  ui.SaveBlendingToolButton->setVisible(false);
  this->connect(ui.SaveBlendingToolButton, SIGNAL(clicked()), this,
      SLOT(OnSaveBlendingToolButtonClicked()));

  OnUndoRedoManagerStackModified();

  m_VolumeStudyInfoFormatString = ui.VolumeStudyInfoLabel->text();
  m_VolumeSeriesInfoFormatString = ui.VolumeSeriesInfoLabel->text();
  m_CostFuncEvolTitleFormatString = ui.CostFunctionTitleLabel->text();
  ui.CostFunctionTitleLabel->setText(m_CostFuncEvolTitleFormatString.arg("--").arg("--"));
  FillVolumeInformation();

  if (m_CastedModel->IsScientificMode())
    this->setWindowTitle(m_WindowTitle + " " +
        REG23ControlWindow::tr("[Scientific Mode]"));

  m_Initializing = false;
  m_Initialized = true;
}

void REG23ControlWindow::Update(int id)
{
  if (id == REG23Model::UPDATE_COST_FUNCTION)
  {
    m_UpdateCostFunctionFlag = true;
  }
  else if (id == REG23Model::UPDATE_PARAMETERS)
  {
    REG23Model::ParametersType pars = m_CastedModel->GetCurrentParameters();
    for (unsigned int i = 0; i < pars.size(); i++)
      m_CurrentRawParameters[i] = pars[i];
    m_UpdateCurrentParameters = true;
  }
}

void REG23ControlWindow::SetLoadConfigVisible(bool visible)
{
  ui.LoadConfigButton->setVisible(visible);
}

void REG23ControlWindow::OnMainTimerTimeout()
{
  if (m_Initialized)
  {
    // initial configuration loading (not a task):
    m_StatusWidget->ShowMessage(REG23ControlWindow::tr("Loading configuration ..."));
    std::string errSect, errKey, errMsg;
    m_CastedModel->LoadConfiguration(errSect, errKey, errMsg);
    if (!m_CastedModel->HaveValidConfiguration())
    {
      if (m_CastedModel->IsScientificMode())
        this->setWindowTitle(m_WindowTitle + " " +
            REG23ControlWindow::tr("[Scientific Mode]"));

      if (!m_NoGUI)
        QMessageBox::critical(this, REG23ControlWindow::tr("Configuration Error"),
          REG23ControlWindow::tr("The specified configuration file appeared to be invalid!\nError occurred here: [%1]\\%2\nError description: %3").
          arg(QString::fromStdString(errSect)).
          arg(QString::fromStdString(errKey)).
          arg(QString::fromStdString(errMsg)));
      if (!ui.LoadConfigButton->isVisible())
        OnCancelButtonPressed(); // exit application (user cannot select another)
      m_StatusWidget->ShowMessage(REG23ControlWindow::tr("Configuration error detected."));
      return;
    }
    m_StoredCurveVisibility.clear();
    ui.ReferenceToolButton->setVisible(m_CastedModel->IsScientificMode());
    ui.ITFOptimizerToolButton->setVisible(m_CastedModel->IsScientificMode());
    ui.SaveDRRsToolButton->setVisible(m_CastedModel->IsScientificMode());
    ui.SaveBlendingToolButton->setVisible(m_CastedModel->IsScientificMode());
    ui.CostFunctionWidget->SetCSVExportButtonVisibility(m_CastedModel->IsScientificMode());
    ui.CostFunctionWidget->SetVisibilityComboVisibility(m_CastedModel->IsScientificMode());
    m_StatusWidget->ShowMessage("");

    std::string nonSupportReasons = "";
    if (!m_CastedModel->IsHardwareAdequate(nonSupportReasons))
    {
      QString message = REG23ControlWindow::tr("The hardware (graphics card, GPU) appears to be invalid for REG23. The application cannot resume!\nPossible reasons:\n");
      message += QString::fromStdString(nonSupportReasons);
      if (!m_NoGUI)
        QMessageBox::critical(this, REG23ControlWindow::tr("Hardware Error"), message);
      OnCancelButtonPressed(); // exit application (application makes no sense)
      return;
    }

    // change application style if necessary:
    std::string configStyle = m_CastedModel->GetConfiguredGraphicsStyle();
    if (configStyle.length() > 0 && configStyle != m_CurrentStyle.toStdString())
    {
      // -> apply immediately
      m_CurrentStyle = QString::fromStdString(configStyle);
      QApplication::setStyle(QStyleFactory::create(m_CurrentStyle));
      QApplication::setPalette(QApplication::style()->standardPalette());
    }

    CreateRenderWindowButtons();
    XYPlotWidget *xyp = ui.CostFunctionWidget;
    xyp->setVisible(m_CastedModel->GetRenderCostFunction());
    ui.CostFunctionTitleLabel->setVisible(m_CastedModel->GetRenderCostFunction());

    if (m_CastedModel->HaveValidImageFileNames())
    {
      m_StatusWidget->ActivateProgressBar(false);

      m_CastedModel->SendInitialParametersNotification(); // request initial pars

      ImageImporterTask *vtask = m_CastedModel->GenerateVolumeImporterTask();
      vtask->OverrideName(REG23ControlWindow::tr("Importing volume ..."));
      std::vector<ImageImporterTask *> ftasks =
          m_CastedModel->GenerateFixedImageImporterTasks();
      if (vtask && ftasks.size() > 0)
      {
        m_StatusWidget->SetCancelButtonToolTip(REG23ControlWindow::tr(
            "Stop initialization process."));
        this->connect(vtask, SIGNAL(TaskStarted(bool)), this,
            SLOT(OnTaskStarted(bool)), Qt::BlockingQueuedConnection);
        this->connect(vtask, SIGNAL(TaskFinished(bool)), this,
            SLOT(OnTaskFinished(bool)), Qt::BlockingQueuedConnection);
        m_CastedModel->GetTaskManager()->ExecuteTask(vtask);
        ImageImporterTask *last = NULL;
        for (std::size_t i = 0; i < ftasks.size(); i++)
        {
          last = ftasks[i];
          ftasks[i]->OverrideName(REG23ControlWindow::tr(
              "Importing fixed image %1 of %2 ..."). arg(i + 1).arg(
              ftasks.size()));
          this->connect(ftasks[i], SIGNAL(TaskStarted(bool)), this,
              SLOT(OnTaskStarted(bool)), Qt::BlockingQueuedConnection);
          this->connect(ftasks[i], SIGNAL(TaskFinished(bool)), this,
              SLOT(OnTaskFinished(bool)), Qt::BlockingQueuedConnection);
          m_CastedModel->GetTaskManager()->ExecuteTask(ftasks[i]);
        }
        this->connect(last, SIGNAL(TaskFinished(bool)), this,
            SLOT(OnLastImageLoaderTaskFinished(bool)), Qt::BlockingQueuedConnection);
      }
      else
      {
        if (vtask)
          delete vtask;
        if (!m_NoGUI)
          QMessageBox::critical(this, REG23ControlWindow::tr("Image import"),
            REG23ControlWindow::tr("Image import could not be established."));
      }
    }
    else
    {
      if (!m_NoGUI)
        QMessageBox::critical(this, REG23ControlWindow::tr("Images"),
          REG23ControlWindow::tr("No or invalid images configured."));
    }
  }
  else // -> again, as initialization has not been completed
  {
    m_MainTimer->start();
  }
}

void REG23ControlWindow::OnLastImageLoaderTaskFinished(bool execute)
{
  // -> initiate Initialization
  REG23RegistrationInitializationTask *task =
      m_CastedModel->GenerateInitializationTask();
  if (task && m_CastedModel->GetTaskManager())
  {
    FillVolumeInformation(); // study, series ...

    task->SetNames(
        REG23ControlWindow::tr("Pre-processing images ..."),
        REG23ControlWindow::tr("Generating mask images ..."),
        REG23ControlWindow::tr("Initializing framework ..."),
        REG23ControlWindow::tr("Post-processing images ..."));
    this->connect(task, SIGNAL(TaskStarted(bool)), this,
        SLOT(OnTaskStarted(bool)), Qt::BlockingQueuedConnection);
    this->connect(task, SIGNAL(TaskFinished(bool)), this,
        SLOT(OnTaskFinished(bool)), Qt::BlockingQueuedConnection);
    this->connect(task, SIGNAL(TaskProgressInfo(bool,double)), this,
        SLOT(OnTaskProgress(bool,double)), Qt::BlockingQueuedConnection);
    m_CastedModel->GetTaskManager()->ExecuteTask(task);
    m_StatusWidget->SetCancelButtonToolTip(REG23ControlWindow::tr(
        "Stop initialization process."));
  }
  else
  {
    if (!m_NoGUI)
      QMessageBox::critical(
        this,
        REG23ControlWindow::tr("Registration initialization"),
        REG23ControlWindow::tr(
            "The registration initialization could not be initiated due to failures in preceding image loading processes!"));
  }
}

void REG23ControlWindow::OnOKButtonPressed()
{
  bool ok = true;
  if (this->m_CastedModel)
  {
    ok = ok && m_CastedModel->SaveTransformToFile(); // optionally generate file
  }
  else
  {
    ok = false;
  }
  if (!ok)
  {
    if (!m_NoGUI)
      QMessageBox::critical(
      this,
      REG23ControlWindow::tr("Registration result storage"),
      REG23ControlWindow::tr(
          "The registration result could not be stored! Please restart the application!"));
  }
  m_OKButtonPressed = true;
  this->close();
}

void REG23ControlWindow::OnCancelButtonPressed()
{
  this->close(); // rest of code is implemented in closeEvent()!
}

void REG23ControlWindow::OnLoadConfigButtonPressed()
{
  std::cerr << "FIXME: Load file, load config, ev. report errors, start timer"
      << std::endl;
}

void REG23ControlWindow::OnTaskStarted(bool execute)
{
  QMutexLocker lock(&m_TaskMutex);
  Task *task = (Task *) sender();
  m_StatusWidget->ShowMessage(task->GetName()); // display task name
  m_StatusWidget->ActivateCancelButton(task->IsCancelable());
  m_StatusWidget->ActivateProgressBar(task->SupportsProgressInformation());
  m_StatusWidget->SetProgress(0);
  m_CurrentTask = task;
  REG23RegistrationExecutionTask *testExec =
      dynamic_cast<REG23RegistrationExecutionTask *>(m_CurrentTask);
  if (testExec) // -> initialize cost function display
  {
    XYPlotWidget *xyp = ui.CostFunctionWidget;
    int endcount = 1; // only cost function
    if (m_CastedModel->IsScientificMode()) // all parameters in science mode
      endcount = xyp->GetNumberOfCurves();
    m_StoredCurveVisibility.clear();
    for (int xx = 0; xx < endcount; xx++)
    {
      // save current visibility state:
      m_StoredCurveVisibility.push_back(xyp->GetCurveVisibility(xx));
      xyp->ClearCurve(xx);
    }
    if (!m_CastedModel->IsScientificMode())
    {
      m_StoredCurveVisibility.clear();
      m_StoredCurveVisibility.push_back(true);
    }
    xyp->setVisible(m_CastedModel->GetRenderCostFunction());
    ui.CostFunctionTitleLabel->setVisible(m_CastedModel->GetRenderCostFunction());
  }
  SetActivationControlsEnabled(false);
  OnUndoRedoManagerStackModified();
}

void REG23ControlWindow::OnTaskProgress(bool execute, double progress)
{
  QMutexLocker lock(&m_TaskMutex);
  Task *task = (Task *) sender();
  m_StatusWidget->ShowMessage(task->GetName()); // display current task name
  m_StatusWidget->SetProgress(progress);
}

void REG23ControlWindow::FillVolumeInformation()
{
  QString s1 = "";
  QString s2 = "";
  QString s3 = "";
  QString s4 = "";
  QString s5 = "";
  QString s6 = "";
  QString s7 = "";
  QString arg1 = "";
  QString arg2 = "";
  QString arg3 = "";
  if (m_CastedModel)
  {
    ITKVTKImage *volume = m_CastedModel->GetVolumeImage();
    if (volume && volume->GetMetaInfo())
    {
      // %1 ... ORA study string, %2 ... (study description), %3 ... ORA series
      // string
      ITKVTKImageMetaInformation::Pointer mi = volume->GetMetaInfo();
      if (mi && mi->GetVolumeMetaInfo())
      {
        s1 = QString::fromStdString(mi->GetVolumeMetaInfo()->GetDICOMModality());
        s2 = QString::fromStdString(mi->GetVolumeMetaInfo()->GetDICOMStudyID());
        s3 = QString::fromStdString(mi->GetVolumeMetaInfo()->GetDICOMStudyDate());
        s4 = QString::fromStdString(mi->GetVolumeMetaInfo()->GetDICOMStudyTime());
        s5 = QString::fromStdString(mi->GetVolumeMetaInfo()->GetDICOMStudyDescription());
        s6 = QString::fromStdString(mi->GetVolumeMetaInfo()->GetDICOMSeriesDescription());
        s7 = QString::fromStdString(mi->GetVolumeMetaInfo()->GetDICOMSeriesNumber());
      }
      // ORA study string:
      arg1 = s1.trimmed();
      if (s2.trimmed().length() > 0)
        arg1 += " " + s2.trimmed();
      if (s3.trimmed().length() > 0)
        arg1 += " " + s3.trimmed();
      if (s4.trimmed().length() > 0)
        arg1 += " " + s4.trimmed();
      // optional study description:
      if (s5.trimmed().length() > 0)
        arg2 = "[" + s5.trimmed() + "]";
      // ORA series string:
      arg3 = s6.trimmed();
      if (s7.trimmed().length() > 0)
        arg3 += " " + s7.trimmed();
    }
  }
  if (arg1.length() <= 0)
    arg1 = "-";
  if (arg3.length() <= 0)
    arg3 = "-";
  QString text = m_VolumeStudyInfoFormatString;
  ui.VolumeStudyInfoLabel->setText(text.arg(arg1));
  ui.VolumeStudyInfoLabel->setToolTip(arg2);
  ui.VolumeStudyInfoLabel->update();
  ui.VolumeStudyInfoLabel->repaint();
  text = m_VolumeSeriesInfoFormatString;
  ui.VolumeSeriesInfoLabel->setText(text.arg(arg3));
  ui.VolumeSeriesInfoLabel->update();
  ui.VolumeSeriesInfoLabel->repaint();
}

void REG23ControlWindow::OnTaskManagerTaskHasNoInputsDropped(Task* task)
{
  if (task)
  {
    // dynamic_cast succeeds only if type matches (if we've virtual methods):
    ImageImporterTask *testImporterTask =
        dynamic_cast<ImageImporterTask *>(task);
    REG23RegistrationInitializationTask *testInitTask =
        dynamic_cast<REG23RegistrationInitializationTask *>(task);
    REG23RegistrationExecutionTask *testExecTask =
        dynamic_cast<REG23RegistrationExecutionTask *>(task);
    if (testImporterTask)
    {
      if (!m_NoGUI)
      {
        QMessageBox::critical(this, REG23ControlWindow::tr("Image import failed"),
          REG23ControlWindow::tr("Obviously at least one image is not available on disk. Cannot resume with registration!"));
      }
      else
      {
        OnCancelButtonPressed(); // exit application in "no-GUI"-mode
      }
    }
    else if (testInitTask)
    {
      if (!m_NoGUI)
      {
        QMessageBox::critical(this, REG23ControlWindow::tr("Image import failed"),
          REG23ControlWindow::tr("Obviously the import of at least one image failed. Cannot resume with registration!"));
      }
      else
      {
        OnCancelButtonPressed(); // exit application in "no-GUI"-mode
      }
    }
    else if (testExecTask)
    {
      if (!m_NoGUI)
      {
        QMessageBox::critical(this, REG23ControlWindow::tr("Initialization failed"),
          REG23ControlWindow::tr("Obviously the initialization of the registration (pre-processing, mask-generation ...) failed. Cannot resume with registration!"));
      }
      else
      {
        OnCancelButtonPressed(); // exit application in "no-GUI"-mode
      }
    }

    if (testImporterTask || testInitTask || testExecTask)
    {
      // -> close render views if open:
      QMap<QToolButton *, REG23RenderViewDialog *>::iterator it;
      for (it = m_ToolButtonWindowMap.begin(); it != m_ToolButtonWindowMap.end(); ++it)
        it.key()->setChecked(false);
    }
  }
}

void REG23ControlWindow::OnTaskFinished(bool execute)
{
  QMutexLocker lock(&m_TaskMutex);
  Task *storeTask = m_CurrentTask;
  m_CurrentTask = NULL;
  m_StatusWidget->ActivateCancelButton(false);
  m_StatusWidget->ActivateProgressBar(false);
  SetActivationControlsEnabled(true);
  m_StatusWidget->ShowMessage("");

  // initialization finished:
  if (storeTask && m_CastedModel)
  {
    // dynamic_cast succeeds only if type matches (if we've virtual methods):
    REG23RegistrationInitializationTask *testInitTask =
        dynamic_cast<REG23RegistrationInitializationTask *>(storeTask);
    if (testInitTask)
    {
      // model is ready for auto-registration after a SUCCESSFUL initialization!
      if (!m_CastedModel->IsReadyForAutoRegistration() && !m_ApplicationIsShuttingDown)
      {
        if (!m_NoGUI)
        {
          QMessageBox::critical(this, REG23ControlWindow::tr("Initialization failed"),
            REG23ControlWindow::tr("Obviously the initialization of the registration (pre-processing, mask-generation ...) failed. Cannot resume with registration!"));
        }
        else
        {
          OnCancelButtonPressed(); // exit application in "no-GUI"-mode
        }
        return;
      }

      std::vector<std::string> omtypes = m_CastedModel->GetOptimizerAndMetricTypeStrings();
      if (omtypes.size() > 0)
      {
        QString s = "", s1 = "";
        if (omtypes.size() > 1)
          s1 = QString::fromStdString(omtypes[1]);
        for (std::size_t k = 2; k < omtypes.size(); k++)
        {
          if (omtypes[k] != s1.toStdString())
            s += "/" + QString::fromStdString(omtypes[k]);
        }
        s = s1 + s;

        ui.CostFunctionTitleLabel->setText(m_CostFuncEvolTitleFormatString.
            arg(QString::fromStdString(omtypes[0])).arg(s));
      }
      else
      {
        ui.CostFunctionTitleLabel->setText(m_CostFuncEvolTitleFormatString.
            arg("-").arg("-"));
      }

      InitialTransformTask *urt = new InitialTransformTask();
      InitialTransformTask::ParametersType pars(6);
      for (std::size_t k = 0; k < pars.Size(); k++)
        pars[k] = m_CurrentRawParameters[k];
      urt->SetParameters(pars);
      SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
      undoRedoManager->ReportTask(urt); // add to undo/redo queue

      FillVolumeInformation(); // study, series ...

      // be sure that window/level on views has been reset (sometimes W/L is not
      // correctly updated):
      QList<REG23RenderViewDialog *> views = m_ToolButtonWindowMap.values();
      for (int k = 0; k < views.size(); k++)
        views[k]->Update(REG23RenderViewDialog::FORCE_RESET_WINDOW_LEVEL);
      this->update(); // refresh this window also ...
      this->repaint();

      double divergence = m_CastedModel->ComputeFixedImagesAcquisitionTimeDivergence();
      if (m_CastedModel->GetShowWarningOnDivergingFixedImageAcquisitionTimes() &&
          divergence > m_CastedModel->GetMaxFixedImageAcquisitionDivergingTimeMin())
      {
        QLocale loc;
        QString s1 = loc.toString(divergence, 'f', 1);
        QString s2 = loc.toString(m_CastedModel->GetMaxFixedImageAcquisitionDivergingTimeMin(), 'f', 1);
        if (!m_NoGUI)
          QMessageBox::warning(this,
            REG23ControlWindow::tr("Diverging reference X-ray acquisition times"),
            REG23ControlWindow::tr("The reference X-ray images appear to have diverging acquisition times (%1 min) which are out of tolerance (max. %2 min)!\nPlease check whether or not the right X-rays are selected!").arg(s1).arg(s2));
      }

      // after registration framework initialization, we usually like to have
      // a sparse pre-registration (if configured and valid):
      m_CastedModel->SetExecuteSparsePreRegistration(true);

      // optional auto-start
      if (m_CastedModel->GetStartRegistrationAutomatically())
      {
        m_StatusWidget->ClickStartButton(); // call registration automatically
      }
    }
  }

  if (storeTask && m_CastedModel)
  {
    REG23SparseRegistrationExecutionTask *testSparseExecTask =
        dynamic_cast<REG23SparseRegistrationExecutionTask *>(storeTask);
    REG23RegistrationExecutionTask *testExecTask =
        dynamic_cast<REG23RegistrationExecutionTask *>(storeTask);
    if (testSparseExecTask) // sparse pre-registration finished
    {
      double regTime = testSparseExecTask->GetElapsedTimeSinceStart();
      bool userCancel = testSparseExecTask->GetCancelRequestFlag();
      SparseAutoRegistrationTransformTask *urt = new SparseAutoRegistrationTransformTask();
      urt->SetUserCancel(userCancel);
      urt->SetRegistrationTime(regTime);
      urt->SetNumberOfIterations(testSparseExecTask->GetNumberOfIterations());
      SparseAutoRegistrationTransformTask::ParametersType pars(6);
      for (std::size_t k = 0; k < pars.Size(); k++)
        pars[k] = m_CurrentRawParameters[k];
      urt->SetParameters(pars);
      SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
      undoRedoManager->ComputeSimulatedRelativeParameters(urt);
      if (urt->ImpliesRelativeTransformation()) // really implies transform
        undoRedoManager->ReportTask(urt); // add to undo/redo queue
      else // not really a notable relative transform - forget it
        delete urt;

      if (!m_NoGUI) // GUI mode
      {
        // update overall registration time and show it!
        QString s;
        if (!userCancel)
          s = REG23ControlWindow::tr("Sparse pre-registration completed in %1 s.");
        else
          s = REG23ControlWindow::tr("Sparse pre-registration canceled after %1 s.");
        QLocale loc;
        QString fs = loc.toString(regTime, 'f', 2);
        m_StatusWidget->ShowMessage(s.arg(fs));
      }
      // If there was no user-cancel, we will resume with the auto-registration:
      if (!userCancel)
      {
        m_StatusWidget->ClickStartButton(); // call registration automatically
      }
    }
    else if (testExecTask) // auto-registration execution finished
    {
      double regTime = testExecTask->GetElapsedTimeSinceStart();
      bool userCancel = testExecTask->GetCancelRequestFlag();
      AutoRegistrationTransformTask *urt = new AutoRegistrationTransformTask();
      urt->SetUserCancel(userCancel);
      urt->SetRegistrationTime(regTime);
      m_CastedModel->AcquireFunctionLock();
      const QVector<QVector<QPointF> > data = m_CastedModel->
          GetFunctionValuesThreadUnsafe();
      int numIterations = -1;
      if (data.size() > 0 && data[0].size() > 0)
        numIterations = (int)data[0][data[0].size() - 1].x();
      m_CastedModel->ReleaseFunctionLock();
      urt->SetNumberOfIterations(numIterations);
      AutoRegistrationTransformTask::ParametersType pars(6);
      for (std::size_t k = 0; k < pars.Size(); k++)
        pars[k] = m_CurrentRawParameters[k];
      urt->SetParameters(pars);
      SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
      undoRedoManager->ComputeSimulatedRelativeParameters(urt);
      if (urt->ImpliesRelativeTransformation()) // really implies transform
        undoRedoManager->ReportTask(urt); // add to undo/redo queue
      else // not really a notable relative transform - forget it
        delete urt;

      if (!m_NoGUI) // GUI mode
      {
        // update overall registration time and show it!
        QString s;
        if (!userCancel)
          s = REG23ControlWindow::tr("Auto-registration completed in %1 s.");
        else
          s = REG23ControlWindow::tr("Auto-registration canceled after %1 s.");
        QLocale loc;
        QString fs = loc.toString(regTime, 'f', 2);
        m_StatusWidget->ShowMessage(s.arg(fs));
      }
      else // no-GUI mode
      {
        ui.OKButton->click(); // exit with accept
      }
    }
  }
}

void REG23ControlWindow::OnCancelStatusButtonPressed()
{
  QMutexLocker lock(&m_TaskMutex);
  if (m_CurrentTask)
    m_CurrentTask->Cancel(); // implemented or not
}

void REG23ControlWindow::SetActivationControlsEnabled(bool enable)
{
  ui.AboutButton->setEnabled(enable);
  ui.HelpButton->setEnabled(enable);
  // ui.CancelButton->setEnabled(enable); ... NOTE: always available
  ui.LoadConfigButton->setEnabled(enable);
  if (enable && m_CastedModel && m_CastedModel->IsReadyForAutoRegistration())
    m_StatusWidget->ActivateStartButton(true);
  else
    m_StatusWidget->ActivateStartButton(false);

  // NOTE: IsReadyForAutoRegistration() returns TRUE if all internal components
  // have been successfully initialized which means that the last registration
  // initialization task since last config-load successfully finished. This is
  // enough to justify an "OK" because a user may use REG23 for pure
  // manual registration.
  if (enable && m_CastedModel && m_CastedModel->HaveValidConfiguration() &&
      m_CastedModel->IsReadyForAutoRegistration())
    ui.OKButton->setEnabled(true);
  else
    ui.OKButton->setEnabled(false);

  ui.ITFOptimizerToolButton->setEnabled(enable);
  ui.SaveDRRsToolButton->setEnabled(enable);
  ui.SaveBlendingToolButton->setEnabled(enable);
  ui.CostFunctionWidget->SetCSVExportButtonEnabled(enable);
  ui.CostFunctionWidget->SetVisibilityComboEnabled(enable);
}

void REG23ControlWindow::OnStartButtonPressed()
{
  // -> initiate sparse pre-registration or auto-registration (if configured and
  // requested):
  REG23SparseRegistrationExecutionTask *preTask = m_CastedModel->
      GenerateSparseExecutionTask();
  if (preTask)
  {
    preTask->OverrideName(REG23ControlWindow::tr("Sparsely pre-registering ..."));
    preTask->SetRegistrationTimeFormatString(REG23ControlWindow::tr(" [%1 s]"));
    preTask->SetIncludeRegistrationTimeInName(true);
    this->connect(preTask, SIGNAL(TaskStarted(bool)), this,
        SLOT(OnTaskStarted(bool)), Qt::BlockingQueuedConnection);
    this->connect(preTask, SIGNAL(TaskFinished(bool)), this,
        SLOT(OnTaskFinished(bool)), Qt::BlockingQueuedConnection);
    this->connect(preTask, SIGNAL(TaskProgressInfo(bool,double)), this,
        SLOT(OnTaskProgress(bool,double)), Qt::BlockingQueuedConnection);
    m_CastedModel->GetTaskManager()->ExecuteTask(preTask);
    m_StatusWidget->SetCancelButtonToolTip(REG23ControlWindow::tr(
        "Stop sparse pre-registration process."));
  }
  else // pre-registration is not requested or not configured -> auto-reg.
  {
    // -> initiate auto-registration
    REG23RegistrationExecutionTask *task =
        m_CastedModel->GenerateExecutionTask();
    if (task && m_CastedModel->GetTaskManager())
    {
      task->OverrideName(REG23ControlWindow::tr("Auto-registering ..."));
      task->SetRegistrationTimeFormatString(REG23ControlWindow::tr(" [%1 s]"));
      task->SetIncludeRegistrationTimeInName(true);
      this->connect(task, SIGNAL(TaskStarted(bool)), this,
          SLOT(OnTaskStarted(bool)), Qt::BlockingQueuedConnection);
      this->connect(task, SIGNAL(TaskFinished(bool)), this,
          SLOT(OnTaskFinished(bool)), Qt::BlockingQueuedConnection);
      this->connect(task, SIGNAL(TaskProgressInfo(bool,double)), this,
          SLOT(OnTaskProgress(bool,double)), Qt::BlockingQueuedConnection);
      m_CastedModel->GetTaskManager()->ExecuteTask(task);
      m_StatusWidget->SetCancelButtonToolTip(REG23ControlWindow::tr(
          "Stop auto-registration process."));
    }
    else
    {
      if (!m_NoGUI)
        QMessageBox::critical(
          this,
          REG23ControlWindow::tr("Automatic registration"),
          REG23ControlWindow::tr(
              "Automatic registration could not be initiated due to failures in preceding initialization processes!"));
    }
  }
}

void REG23ControlWindow::CreateRenderWindowButtons()
{
  if (!m_Initialized || !m_CastedModel->HaveValidConfiguration())
    return;

  QFrame *fr = ui.RenWinButtonsFrame;
  int cc = fr->children().size();
  for (int i = cc - 1; i >= 0; i--) // remove children first
  {
    QToolButton *tb = dynamic_cast<QToolButton*>(fr->children().at(i));
    if (tb)
    {
      if (m_ToolButtonWindowMap.value(tb))
      {
        REG23RenderViewDialog *vd = m_ToolButtonWindowMap.value(tb);
        m_CastedModel->Unregister(vd);
        if (m_LastRenderViewGeoms.value(vd))
          delete m_LastRenderViewGeoms.value(vd);
        this->disconnect(vd, SIGNAL(finished(int)));
        delete vd; // delete window
      }
      this->disconnect(tb, SIGNAL(toggled(bool)));
      fr->layout()->removeWidget(tb);
    }
  }
  m_ToolButtonWindowMap.clear();
  m_LastRenderViewGeoms.clear();
  std::vector<std::string> vns = this->m_CastedModel->GetViewNames();
  for (std::size_t i = 0; i < vns.size(); i++)
  {
    QString caption;
    if (vns[i].length() > 0)
      caption = QString::fromStdString(vns[i]);
    else
      caption = REG23ControlWindow::tr("View %1").arg(i + 1);
    QToolButton *tb = new QToolButton(ui.RenWinButtonsFrame);
    tb->setText(caption);
    tb->setToolTip(
        REG23ControlWindow::tr("Activate / deactivate view window (%1)").
        arg(caption));
    fr->layout()->addWidget(tb);
    tb->setVisible(true);
    tb->setEnabled(true);
    tb->setCheckable(true);
    tb->setChecked(false);
    tb->setMinimumHeight(35);
    this->connect(tb, SIGNAL(toggled(bool)),
        this, SLOT(OnRenderViewToolButtonToggled(bool)));
    // -> add render window:
    REG23RenderViewDialog *vd = new REG23RenderViewDialog(NULL); // not childs
    vd->SetFixedImageIndex(i);
    this->connect(vd, SIGNAL(finished(int)),
        this, SLOT(OnRenderViewFinished(int)));
    vd->setWindowTitle(caption);
    vd->SetBaseWindowTitle(caption);
    vd->SetModel(this->m_Model);
    m_CastedModel->Register(vd);
    vd->Initialize();
    m_ToolButtonWindowMap.insert(tb, vd);

    // show or do not show windows depending on initial render view geometry:
    RenderViewGeometry *geom = NULL;
    for (int k = 0; k < m_InitialRenderViewGeometries.size(); k++)
    {
      if (m_InitialRenderViewGeometries[k].idx == (int)i)
        geom = &m_InitialRenderViewGeometries[k];
    }
    if (geom)
    {
      QRect *geomr = new QRect(geom->x, geom->y, geom->w, geom->h);
      m_LastRenderViewGeoms.insert(vd, geomr);
      tb->click();
      // Click again to hide window
      if (m_NoGUI || !geom->visibility)
        tb->click();
    }
  }

  SetWindowsStayOnTop(m_StayOnTop);

  if (m_NoGUI)
    this->setVisible(false);
}

void REG23ControlWindow::OnRenderViewToolButtonToggled(bool checked)
{
  if (m_LockRenderViewButtonsEffect)
    return;
  QToolButton *tb = dynamic_cast<QToolButton*>(sender());
  if (tb)
  {
    if (m_ToolButtonWindowMap.value(tb))
    {
      REG23RenderViewDialog *d = m_ToolButtonWindowMap.value(tb);
      if (checked)
      {
        if (m_LastRenderViewGeoms.value(d))
        {
          QRect *geom = m_LastRenderViewGeoms.value(d);
          d->setGeometry(geom->left(), geom->top(), geom->width(), geom->height());
          // TODO: Make clean function to handle this.
          QRect corr;
          corr.setX(geom->x() + geom->x() - d->pos().x());
          corr.setY(geom->y() + geom->y() - d->pos().y());
          corr.setWidth(geom->width());
          corr.setHeight(geom->height());
          d->setGeometry(corr);
        }
        d->show();
      }
      else
      {
        d->accept();
      }
    }
  }
}

void REG23ControlWindow::OnRenderViewFinished(int result)
{
  REG23RenderViewDialog *d = dynamic_cast<REG23RenderViewDialog*>(sender());
  if (d)
  {
    if (m_ToolButtonWindowMap.key(d))
    {
      QRect *geom = NULL;
      if (m_LastRenderViewGeoms.value(d))
        geom = m_LastRenderViewGeoms.value(d);
      if (!geom)
      {
        geom = new QRect();
        m_LastRenderViewGeoms.insert(d, geom);
      }
      geom->setLeft(d->pos().x());
      geom->setTop(d->pos().y());
      geom->setWidth(d->geometry().width());
      geom->setHeight(d->geometry().height());
      // NOTE: Do not uncheck the toggle buttons if application is shutting
      // down - otherwise the visibility state cannot be stored in file!
      if (!m_ApplicationIsShuttingDown)
      {
        m_LockRenderViewButtonsEffect = true;
        m_ToolButtonWindowMap.key(d)->setChecked(false);
        m_LockRenderViewButtonsEffect = false;
      }
    }
  }
}

void REG23ControlWindow::AddRenderViewGeometry(int idx, int x, int y, int w, int h, bool visibility)
{
  if (idx < 0 || w <= 0 || h <= 0)
    return;
  RenderViewGeometry g;
  g.idx = idx;
  g.x = x;
  g.y = y;
  g.w = w;
  g.h = h;
  g.visibility = visibility;
  m_InitialRenderViewGeometries.push_back(g);
}

void REG23ControlWindow::OnGUIUpdateTimerTimeout()
{
  // NOTE: Obviously, this call helps to process pending events of the Qt-event-
  // loop which is exceptionally important if we used multi-threaded openGL
  // render calls.
  QCoreApplication::processEvents();

  if (m_UpdateCostFunctionFlag)
  {
    m_UpdateCostFunctionFlag = false; // set back!
    XYPlotWidget *xyp = ui.CostFunctionWidget;
    // lock the cost function values for the whole rendering period (not only
    // for the acquisition-period - the returned vector is just a reference!):
    m_CastedModel->AcquireFunctionLock();
    QVector<QVector<QPointF> > funcs =
        m_CastedModel->GetFunctionValuesThreadUnsafe();
    m_CastedModel->ReleaseFunctionLock();

    QString funcName = "";
    int endcount = 1; // only cost function
    if (m_CastedModel->IsScientificMode()) // all parameters in science mode
      endcount = funcs.size();
    if (m_StoredCurveVisibility.size() != endcount)
      m_StoredCurveVisibility.clear();
    for (int xx = 0; xx < endcount; xx++)
    {
      if (xx == 0)
        funcName = REG23ControlWindow::tr("metric[-]");
      else if (xx == 1)
        funcName = REG23ControlWindow::tr("rotX[deg]");
      else if (xx == 2)
        funcName = REG23ControlWindow::tr("rotY[deg]");
      else if (xx == 3)
        funcName = REG23ControlWindow::tr("rotZ[deg]");
      else if (xx == 4)
        funcName = REG23ControlWindow::tr("transX[cm]");
      else if (xx == 5)
        funcName = REG23ControlWindow::tr("transY[cm]");
      else if (xx == 6)
        funcName = REG23ControlWindow::tr("transZ[cm]");
      else
        funcName = "";
      if (m_StoredCurveVisibility.size() != endcount)
        m_StoredCurveVisibility.push_back((xx == 0)); // initialize
      xyp->SetCurveData(xx, funcs[xx], true, funcName, m_StoredCurveVisibility[xx]);
    }

  }
  if (m_UpdateCurrentParameters)
  {
    QLocale loc;
    QString pf[6];
    for (unsigned int i = 0; i < 3; i++) // rotations
    {
      pf[i] = loc.toString(m_CurrentRawParameters[i] /
          vtkMath::Pi() * 180., 'f', 1);
      if (pf[i].left(1) != "-") // add an explicit "+"-sign
        pf[i] = "+" + pf[i];
    }
    for (unsigned int i = 3; i < 6; i++) // translations
    {
      pf[i] = loc.toString(m_CurrentRawParameters[i] / 10., 'f', 1);
      if (pf[i].left(1) != "-") // add an explicit "+"-sign
        pf[i] = "+" + pf[i];
    }
    ui.RotationsLabel->setText(m_RotationsText.arg(pf[0]).arg(pf[1]).arg(pf[2]));
    ui.RotationsLabel->update();
    ui.RotationsLabel->repaint();
    ui.TranslationsLabel->setText(m_TranslationsText.arg(pf[3]).arg(pf[4]).arg(pf[5]));
    ui.TranslationsLabel->update();
    ui.TranslationsLabel->repaint();
    m_UpdateCurrentParameters = false;
  }
}

void REG23ControlWindow::OnUndoRedoManagerStackModified()
{
  if (m_CastedModel && m_CastedModel->IsReadyForManualRegistration())
  {
    SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
    if (undoRedoManager->GetNumberOfCurrentUndoItems() > 0)
    {
      ui.UndoToolButton->setEnabled(true);
      ui.ResetToolButton->setEnabled(true);
      std::size_t n = undoRedoManager->GetNumberOfCurrentUndoItems();
      m_UndoMenu->clear();
      for (std::size_t i = 0; i < n; i++)
      {
        QString text = QString::fromStdString(
            undoRedoManager->GetIthUndoItemDescription(i));
        QAction *action = m_UndoMenu->addAction(text);
        QVariant var;
        var.setValue<int>(i);
        action->setData(var);
      }
    }
    else
    {
      ui.UndoToolButton->setEnabled(false);
      ui.ResetToolButton->setEnabled(false);
    }
    if (undoRedoManager->GetNumberOfCurrentRedoItems() > 0)
    {
      ui.RedoToolButton->setEnabled(true);
      std::size_t n = undoRedoManager->GetNumberOfCurrentRedoItems();
      m_RedoMenu->clear();
      for (std::size_t i = 0; i < n; i++)
      {
        QString text = QString::fromStdString(
            undoRedoManager->GetIthRedoItemDescription(i));
        QAction *action = m_RedoMenu->addAction(text);
        QVariant var;
        var.setValue<int>(i);
        action->setData(var);
      }
    }
    else
    {
      ui.RedoToolButton->setEnabled(false);
    }
    if (m_CastedModel->IsScientificMode()) // science mode
    {
      if (m_CastedModel->GetReferenceParameters().GetSize() > 0)
        ui.ReferenceToolButton->setEnabled(true);
      else // no reference transfrom defined
        ui.ReferenceToolButton->setEnabled(false);
    }
  }
  else
  {
    ui.UndoToolButton->setEnabled(false);
    ui.RedoToolButton->setEnabled(false);
    ui.ResetToolButton->setEnabled(false);
    if (m_CastedModel->IsScientificMode()) // science mode
      ui.ReferenceToolButton->setEnabled(false);
  }
}

void REG23ControlWindow::OnUndoRedoManagerTaskExecutionRequest(
    AbstractTransformTask *task, bool &success)
{
  if (task)
  {
    // apply requested task parameters and update GUI accordingly:
    AbstractTransformTask::ParametersType pars = task->GetParameters();
    m_CastedModel->OverrideAndApplyCurrentParameters(pars);
    ITKVTKImage *ivi = m_CastedModel->GetVolumeImage();
    TEMPLATE_CALL_COMP(ivi->GetComponentType(),
        m_CastedModel->ComputeCurrentMovingImages)
    success = true;
  }
  else
  {
    success = false;
  }
}

void REG23ControlWindow::OnUndoMenuTriggered(QAction *action)
{
  if (action)
  {
    QVariant var = action->data();
    bool ok = false;
    int i = var.toInt(&ok);
    if (m_CastedModel && m_CastedModel->IsReadyForManualRegistration())
    {
      SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
      undoRedoManager->Undo(i);
    }
  }
}

void REG23ControlWindow::OnRedoMenuTriggered(QAction *action)
{
  if (action)
  {
    QVariant var = action->data();
    bool ok = false;
    int i = var.toInt(&ok);
    if (m_CastedModel && m_CastedModel->IsReadyForManualRegistration())
    {
      SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
      undoRedoManager->Redo(i);
    }
  }
}

void REG23ControlWindow::OnUndoToolButtonClicked()
{
  if (m_CastedModel && m_CastedModel->IsReadyForManualRegistration())
  {
    SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
    undoRedoManager->Undo();
  }
}

void REG23ControlWindow::OnRedoToolButtonClicked()
{
  if (m_CastedModel && m_CastedModel->IsReadyForManualRegistration())
  {
    SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
    undoRedoManager->Redo();
  }
}

void REG23ControlWindow::OnResetToolButtonClicked()
{
  if (m_CastedModel && m_CastedModel->IsReadyForManualRegistration())
  {
    SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
    undoRedoManager->Reset();
  }
}

void REG23ControlWindow::OnAboutButtonPressed()
{
  REG23AboutDialog ad(this);
  ad.exec();
}

void REG23ControlWindow::OnHelpButtonPressed()
{
  QWebView *helpView = new QWebView(this);
  helpView->setWindowFlags(Qt::Window);
  helpView->setAttribute(Qt::WA_DeleteOnClose, true);

  QString language = QLocale::system().name(); // <language-code>_<country-code>
  // <language-code> code only (country-code may be too specific ...)
  QChar qc('_');
  int p;
  if ((p = language.indexOf(qc)) > -1)
    language = language.mid(0, p);

  QString helpFile = qApp->applicationDirPath() + QDir::separator()
  + "REG23_ShortReference_" + language + ".html";
  if (!QFile::exists(helpFile))
  {
    language = "de";  // default, TODO: create english help
    helpFile = qApp->applicationDirPath() + QDir::separator()
    + "REG23_ShortReference_" + language + ".html";
    if (!QFile::exists(helpFile))
      return;
  }

  this->connect(helpView, SIGNAL(loadStarted()), this,
      SLOT(OnHelpLoadStarted()));
  this->connect(helpView, SIGNAL(loadFinished(bool)), this,
      SLOT(OnHelpLoadFinished(bool)));
  this->connect(helpView, SIGNAL(loadProgress(int)), this,
      SLOT(OnHelpLoadProgress(int)));
  helpView->load(QUrl::fromLocalFile(helpFile));
  helpView->show();
}

void REG23ControlWindow::OnHelpLoadStarted()
{
  QApplication::setOverrideCursor(Qt::BusyCursor);
}

void REG23ControlWindow::OnHelpLoadFinished(bool ok)
{
  QApplication::restoreOverrideCursor();
  m_StatusWidget->ShowMessage("");
}

void REG23ControlWindow::OnHelpLoadProgress(int progress)
{
  m_StatusWidget->ShowMessage(REG23ControlWindow::tr("Loading help %1%").arg(progress));
}

void REG23ControlWindow::SetLayoutConfigurationFile(QString fileName)
{
  if (fileName.isNull() || fileName.isEmpty())
    return;
  m_LayoutConfigurationFile = fileName;
}

void REG23ControlWindow::OnLastWindowClosed()
{
  if (m_LayoutConfigurationFile.isNull() || m_LayoutConfigurationFile.isEmpty()
      || m_NoGUI)
    return;

  // Save window geometries
  ora::IniAccess config(m_LayoutConfigurationFile.toStdString());
  std::string section = "Layout" + ora::StreamConvert(m_ToolButtonWindowMap.size());
  std::string key = "ControlWindowGeometry";
  QRect geom(this->pos().x(), this->pos().y(), this->size().width(), this->size().height());
  QString value = QString("%1 %2 %3 %4").arg(geom.left()).arg(geom.top()).arg(
      geom.width()).arg(geom.height());
  bool result = config.WriteString(section, key, value.toStdString());
  if (!result)
    return;
  QMap<QToolButton *, REG23RenderViewDialog *>::const_iterator it;
  for (it = m_ToolButtonWindowMap.constBegin(); it
      != m_ToolButtonWindowMap.constEnd(); ++it)
  {
    int index = it.value()->GetFixedImageIndex() + 1; // config starts at 1
    key = "RenderViewGeometry" + ora::StreamConvert(index);
    geom = QRect(it.value()->pos().x(), it.value()->pos().y(),
        it.value()->size().width(), it.value()->size().height());
    // Window visibility
    int vis = 0;
    if (it.key()->isChecked())
      vis = 1;
    // Construct configuration string
    value = QString("%1 %2 %3 %4 %5").arg(geom.left()).arg(geom.top()).arg(
        geom.width()).arg(geom.height()).arg(vis);
    result = config.WriteString(section, key, value.toStdString());
    if (!result)
      return;
  }
  config.SetAddORACheckSum(true);
  result = config.Update();
}

void REG23ControlWindow::closeEvent(QCloseEvent *event)
{
  if (!m_OKButtonPressed) // Cancel or so ...
  {
    // NOTE: new approach -> terminate running processes automatically (see below)!
//    if (m_StatusWidget->IsCancelButtonActivated())
//    {
//      if (!m_NoGUI)
//        QMessageBox::critical(this, REG23ControlWindow::tr("Processes running!"),
//          REG23ControlWindow::tr("There is at least one process running; cannot close the program. Please cancel or finish the process before!"));
//      event->ignore();
//      return;
//    }
    bool doClose = true;
    if (m_CastedModel && m_CastedModel->HaveValidConfiguration()) // only ask here
    {
      std::string nonSupportReasons = "";
      if (m_CastedModel->GetShowWarningOnCancel() &&
          /* hardware support cannot be queried during registration (!) */
          (m_CastedModel->GetRegistrationIsRunning() || m_CastedModel->IsHardwareAdequate(nonSupportReasons)) &&
          (!m_NoGUI && QMessageBox::question(this, REG23ControlWindow::tr("Are you sure?"),
          REG23ControlWindow::tr("Are you sure that you want to 'decline' the actual registration result?"),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No))
        doClose = false;
      if (m_NoGUI)
        doClose = true;
    }
    if (doClose)
    {
      m_ApplicationIsShuttingDown = true; // signal!
      if (m_StatusWidget->IsCancelButtonActivated()) // processes running
      {
        m_StatusWidget->ClickCancelButton(); // auto-terminate process
        while (m_StatusWidget->IsCancelButtonActivated())
        {
          itksys::SystemTools::Delay(100);
          QApplication::instance()->processEvents();
        } // wait until cancel button is disabled (processes stopped)
      }
      event->accept();
      // close children as well
      QList<REG23RenderViewDialog *> views = m_ToolButtonWindowMap.values();
      for (int k = 0; k < views.size(); k++)
      {
        views[k]->StoreWindowLevel(); // stores window/level settings
        views[k]->close();
      }
    }
    else
    {
      event->ignore();
    }
  }
  else
  {
    m_ApplicationIsShuttingDown = true; // signal!
    // close children as well
    QList<REG23RenderViewDialog *> views = m_ToolButtonWindowMap.values();
    for (int k = 0; k < views.size(); k++)
    {
      views[k]->StoreWindowLevel(); // stores window/level settings
      views[k]->close();
    }
  }
  m_OKButtonPressed = false; // set back
}

void REG23ControlWindow::SetWindowsStayOnTop(bool stayOnTop)
{
  m_StayOnTop = stayOnTop;

  Qt::WindowFlags flags = this->windowFlags();
  bool vis = this->isVisible();
  if (m_StayOnTop)
  {
    this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
  }
  else
  {
    if ((flags & Qt::WindowStaysOnTopHint) == Qt::WindowStaysOnTopHint)
      this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
  }
  this->setVisible(vis);

  QList<REG23RenderViewDialog *> views = m_ToolButtonWindowMap.values();
  for (int k = 0; k < views.size(); k++)
  {
    vis = views[k]->isVisible();
    flags = views[k]->windowFlags();
    if (m_StayOnTop)
    {
      views[k]->setWindowFlags(flags | (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
    }
    else
    {
      if ((flags & Qt::WindowStaysOnTopHint) == Qt::WindowStaysOnTopHint)
        views[k]->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
    }
    views[k]->setVisible(vis);
  }
}

void REG23ControlWindow::OnReferenceToolButtonClicked()
{
  if (m_CastedModel->GetReferenceParameters().GetSize() <= 0)
    return; // no reference transform defined
  ReferenceTransformTask *urt = new ReferenceTransformTask();
  urt->SetParameters(m_CastedModel->GetReferenceParameters());
  SimpleTransformUndoRedoManager *undoRedoManager = m_CastedModel->GetUndoRedoManager();
  undoRedoManager->ReportTask(urt); // add to undo/redo queue
  // apply transform:
  m_CastedModel->OverrideAndApplyCurrentParameters(m_CastedModel->GetReferenceParameters());
  ITKVTKImage *ivi = m_CastedModel->GetVolumeImage();
  TEMPLATE_CALL_COMP(ivi->GetComponentType(),
      m_CastedModel->ComputeCurrentMovingImages)
}

void REG23ControlWindow::OnITFOptimizerToolButtonClicked()
{
  if (m_CastedModel->IsReadyForManualRegistration())
  {
    QString dir = QFileDialog::getExistingDirectory(this,
        REG23ControlWindow::tr("Select the ITF-optimization storage directory ..."));
    if (dir.length() > 0)
    {
      // finally, really do it!
      bool ok = false;
      ITKVTKImage *ivi = m_CastedModel->GetVolumeImage();
      TEMPLATE_CALL_COMP(ivi->GetComponentType(),
          ok = m_CastedModel->GenerateITFOptimizerConfiguration, dir.toStdString())
      if (!ok)
      {
        if (!m_NoGUI)
          QMessageBox::critical(this, REG23ControlWindow::tr("ITF-optimizer configuration error"),
            REG23ControlWindow::tr("One or more errors occured during generating and storing ITF-optimization configuration!"));
      }
    }
  }
}

void REG23ControlWindow::OnSaveDRRsToolButtonClicked()
{
  if (m_CastedModel->IsReadyForManualRegistration())
  {
    QString dir = QFileDialog::getExistingDirectory(this,
        REG23ControlWindow::tr("Select the DRR storage directory ..."));
    if (dir.length() > 0)
    {
      bool ok = false;
      QString pattern = QInputDialog::getText(this,
          REG23ControlWindow::tr("File name pattern"),
          REG23ControlWindow::tr("DRR pattern (%d for view index):"),
          QLineEdit::Normal, REG23ControlWindow::tr("drr%d.mhd"), &ok);
      if (ok && !pattern.isEmpty())
      {
        if (!pattern.contains("%d", Qt::CaseSensitive))
        {
          if (!m_NoGUI)
            QMessageBox::critical(this, REG23ControlWindow::tr("No pattern"),
              REG23ControlWindow::tr("No %d (view index) was detected in pattern!"));
          return;
        }
        QStringList sl;
        sl.append("3D FLOAT DRRs");
        sl.append("2D USHORT DRRs");
        sl.append("2D FLOAT DRRs");
        sl.append("2D UCHAR DRRs");
        int defaultIndex = 1;
        if (pattern.endsWith(".mhd", Qt::CaseInsensitive))
          defaultIndex = 0;
        QString sel = QInputDialog::getItem(this,
            REG23ControlWindow::tr("Image dimensionality and data type"),
            REG23ControlWindow::tr("Selection:"), sl, defaultIndex, false, &ok);
        if (ok)
        {
          defaultIndex = -1;
          for (int i = 0; i < sl.size(); i++)
          {
            if (sel == sl[i])
              defaultIndex = i;
          }
          // finally, really do it!
          ITKVTKImage *ivi = m_CastedModel->GetVolumeImage();
          TEMPLATE_CALL_COMP(ivi->GetComponentType(),
              ok = m_CastedModel->GenerateAndSaveDRRs, dir.toStdString(),
                    pattern.toStdString(), defaultIndex)
          if (!ok)
          {
            if (!m_NoGUI)
              QMessageBox::critical(this, REG23ControlWindow::tr("DRR storage error"),
                REG23ControlWindow::tr("One or more errors occured during generating and storing DRRs!"));
          }
        }
      }
    }
  }
}

void REG23ControlWindow::OnSaveBlendingToolButtonClicked()
{
  if (!m_CastedModel || !m_CastedModel->IsReadyForManualRegistration())
    return;

  QString dir = QFileDialog::getExistingDirectory(this,
  REG23ControlWindow::tr("Select the image storage directory ..."));
  if (dir.length() <= 0)
    return;

  bool ok = false;
  QString
    pattern =
        QInputDialog::getText(
            this,
            REG23ControlWindow::tr("File name pattern"),
            REG23ControlWindow::tr(
                "Image pattern:\n - %1 for view index [0-n]\n - %2 for blending value [0-100]\n - %3 for index"),
            QLineEdit::Normal, REG23ControlWindow::tr("image_%1_%3_%2.png"), &ok);
  if (!ok || pattern.isEmpty())
    return;

  if (!pattern.contains("%1", Qt::CaseSensitive))
  {
    if (!m_NoGUI)
      QMessageBox::critical(this, REG23ControlWindow::tr("No pattern"),
        REG23ControlWindow::tr("No '%1' (view index) was detected in pattern!"));
    return;
  }
  if (!pattern.contains("%2", Qt::CaseSensitive))
  {
    if (!m_NoGUI)
      QMessageBox::critical(
        this,
        REG23ControlWindow::tr("No pattern"),
        REG23ControlWindow::tr(
            "No '%2' (blending value) was detected in pattern!"));
    return;
  }
  if (!pattern.endsWith(".png", Qt::CaseInsensitive))
  {
    if (!m_NoGUI)
      QMessageBox::critical(this, REG23ControlWindow::tr("Invalid file extension"),
        REG23ControlWindow::tr("Only PNG is supported!"));
    return;
  }

  QString settings = QInputDialog::getText(this,
      REG23ControlWindow::tr("Settings"),
      REG23ControlWindow::tr("Blending values [int]\n 'Min[0-50];Max[50-100];Step[1-50]':"),
      QLineEdit::Normal, REG23ControlWindow::tr("20;80;10"), &ok);
  if (!ok || pattern.isEmpty())
  {
    if (!m_NoGUI)
      QMessageBox::critical(this, REG23ControlWindow::tr("Invalid settings"),
        REG23ControlWindow::tr("No settings provided!"));
    return;
  }

  QStringList settingsList = settings.split(";", QString::SkipEmptyParts,
      Qt::CaseSensitive);
  if (settingsList.size() != 3)
  {
    if (!m_NoGUI)
      QMessageBox::critical(
        this,
        REG23ControlWindow::tr("Invalid settings"),
        REG23ControlWindow::tr("Invalid number of settings provided (%1)!").arg(
            settingsList.size()));
    return;
  }
  int minValue = settingsList[0].toInt(&ok);
  int maxValue = settingsList[1].toInt(&ok);
  int stepValue = settingsList[2].toInt(&ok);
  if (!ok || minValue < 0 || minValue > 100 || maxValue < 0 || maxValue > 100 ||
      minValue > maxValue || stepValue < 1 || minValue > 50 || maxValue < 50)
  {
    if (!m_NoGUI)
      QMessageBox::critical(this, REG23ControlWindow::tr("Invalid settings"),
        REG23ControlWindow::tr("Invalid settings provided (%1)!").arg(settings));
    return;
  }

  QApplication::setOverrideCursor(Qt::BusyCursor);
  QMap<QToolButton *, REG23RenderViewDialog *>::iterator it;
  for (it = m_ToolButtonWindowMap.begin(); it != m_ToolButtonWindowMap.end(); ++it)
  {
    unsigned int index = 1;

    // 50 to minValue
    for (int value = 50; value >= minValue; value -= stepValue)
    {
      ok = it.value()->StoreRenderWindowImage(dir, pattern, index, value, 1);
      ++index;
      if (!ok)
      {
        if (!m_NoGUI)
          QMessageBox::critical(
            this,
            REG23ControlWindow::tr("Image storage error"),
            REG23ControlWindow::tr(
                "One or more errors occured during generating and storing images!"));
        break;
      }
    }
    // minValue to maxValue
    for (int value = minValue; value <= maxValue; value += stepValue)
    {
      ok = it.value()->StoreRenderWindowImage(dir, pattern, index, value, 1);
      ++index;
      if (!ok)
      {
        if (!m_NoGUI)
          QMessageBox::critical(
            this,
            REG23ControlWindow::tr("Image storage error"),
            REG23ControlWindow::tr(
                "One or more errors occured during generating and storing images!"));
        break;
      }
    }
    // maxValue to 50
    for (int value = maxValue; value >= 50; value -= stepValue)
    {
      ok = it.value()->StoreRenderWindowImage(dir, pattern, index, value, 1);
      ++index;
      if (!ok)
      {
        if (!m_NoGUI)
          QMessageBox::critical(
            this,
            REG23ControlWindow::tr("Image storage error"),
            REG23ControlWindow::tr(
                "One or more errors occured during generating and storing images!"));
        break;
      }
    }
  }
  QApplication::restoreOverrideCursor();
}

}
