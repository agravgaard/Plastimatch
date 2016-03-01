//
#ifndef ORAREG23CONTROLWINDOW_H
#define ORAREG23CONTROLWINDOW_H

#include <QtGui/QMainWindow>
#include <QMutex>
#include "ui_oraREG23ControlWindow.h"

// ORAIFMVC
#include <oraViewController.h>
// ORAIFGUIComponents
#include <oraXYPlotWidget.h>

// forward declarations:
namespace ora 
{
class REG23Model;
class Task;
class REG23TaskPresentationWidget;
class REG23RenderViewDialog;
class AbstractTransformTask;
}
class QTimer;
class QLabel;
class QToolButton;
class QCloseEvent;

namespace ora 
{

class REG23ControlWindow :
    public QMainWindow, public ViewController
{
Q_OBJECT

  /*
   TRANSLATOR ora::REG23ControlWindow

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** Default constructor **/
  REG23ControlWindow(QWidget *parent = 0);
  /** Destructor **/
  virtual ~REG23ControlWindow();

  /** Initialize the view. MUST BE CALLED FROM MAIN-THREAD! **/
  virtual void Initialize();

  /** @see ViewController#Update(int) **/
  virtual void Update(int id);

  /** Set the 'load config' button visible/invisible. **/
  virtual void SetLoadConfigVisible(bool visible);

  /** Add a desired initial render view geometry. The corresponding render
   * view won't be initially displayed if there is no geometry defined for it.
   * NOTE: the idx parameter is 0-based! **/
  virtual void AddRenderViewGeometry(int idx, int x, int y, int w, int h, bool visibility);

    /** Get GUI update interval in milliseconds **/
  int GetGUIUpdateIntervalMS()
  {
    return m_GUIUpdateIntervalMS;
  }
  /** Set GUI update interval in milliseconds **/
  void SetGUIUpdateIntervalMS(int ms);

  /** Set layout configuration file. If set the window configuration is saved to
   * this file when the application closes. @see OnLastWindowClosed() **/
  void SetLayoutConfigurationFile(QString fileName);

  /** Set whether windows of this application should stay on top of other
   * applications. **/
  void SetWindowsStayOnTop(bool stayOnTop);

  /** Set flag indicating whether "no-GUI" mode is active. **/
  void SetNoGUI(bool value)
  {
    m_NoGUI = value;
  }
  /** Get flag indicating whether "no-GUI" mode is active. **/
  bool GetNoGUI()
  {
    return m_NoGUI;
  }

protected:
  /** Helper struct for a render view geometry. **/
  struct RenderViewGeometry
  {
    int idx;
    int x;
    int y;
    int w;
    int h;
    bool visibility;
  };

  /** flag indicating that class is initializing **/
  bool m_Initializing;
  /** flag indicating that class has been initialized **/
  bool m_Initialized;
  /** casted version of the model reference **/
  REG23Model *m_CastedModel;
  /** Timer that establishes an event for image loading / framework initialization **/
  QTimer *m_MainTimer;
  /** Current running task **/
  Task *m_CurrentTask;
  /** Protect against task-user-interactions **/
  QMutex m_TaskMutex;
  /** Task status presentation widget (hand icon) **/
  REG23TaskPresentationWidget *m_StatusWidget;
  /** Map that connects the tool buttons with the render views **/
  QMap<QToolButton *, REG23RenderViewDialog *> m_ToolButtonWindowMap;
  /** Map that stores the last (or desired) render window positions **/
  QMap<REG23RenderViewDialog *, QRect *> m_LastRenderViewGeoms;
  /** Blocker flag **/
  bool m_LockRenderViewButtonsEffect;
  /** Initial render view geometries **/
  QVector<RenderViewGeometry> m_InitialRenderViewGeometries;
  /** Timer that manages GUI updates (therefore, the Initialize()-method must
   * be called from the MAIN-THREAD!) **/
  QTimer *m_GUIUpdateTimer;
  /** GUI update interval in milliseconds **/
  int m_GUIUpdateIntervalMS;
  /** Flag indicating that cost function should be visually updated. **/
  bool m_UpdateCostFunctionFlag;
  /** Basic transformation (rotation) output string **/
  QString m_RotationsText;
  /** Basic transformation (translation) output string **/
  QString m_TranslationsText;
  /** Current registration parameters (EULER, in raw-format) **/
  double m_CurrentRawParameters[6];
  /** Flag indicating that current parameters should be refreshed **/
  bool m_UpdateCurrentParameters;
  /** Current windows style **/
  QString m_CurrentStyle;
  /** Volume information format string (study) **/
  QString m_VolumeStudyInfoFormatString;
  /** Volume information format string (series) **/
  QString m_VolumeSeriesInfoFormatString;
  /** Menu for redo list **/
  QMenu *m_RedoMenu;
  /** Menu for undo list **/
  QMenu *m_UndoMenu;
  /** Layout configuration file. If set the window configuration is saved to
   * this file when the application closes. @see OnLastWindowClosed() **/
  QString m_LayoutConfigurationFile;
  /** Flag indicating that OK button initiated a close request. **/
  bool m_OKButtonPressed;
  /** Flag indicating whether app wins stay on top of others **/
  bool m_StayOnTop;
  /** Cost function evolution title format string**/
  QString m_CostFuncEvolTitleFormatString;
  /** Flag indicating that application shuts down. **/
  bool m_ApplicationIsShuttingDown;
  /** Initial window title. **/
  QString m_WindowTitle;
  /** Stored visibility of each curve. **/
  QVector<bool> m_StoredCurveVisibility;
  /** Flag indicating whether "no-GUI" mode is active. **/
  bool m_NoGUI;

  /** Enable/disable activation controls. **/
  void SetActivationControlsEnabled(bool enable);

  /** Create the render window buttons according to the loaded configuration. **/
  void CreateRenderWindowButtons();

  /** Fill the dedicated label with volume meta information from the loaded
   * volume after registration initialization. **/
  void FillVolumeInformation();

  /** On close request of window.
   * @see QWidget#closeEvent() **/
  virtual void closeEvent(QCloseEvent *event);

protected slots:
  void OnMainTimerTimeout();
  void OnOKButtonPressed();
  void OnCancelButtonPressed();
  void OnCancelStatusButtonPressed();
  void OnLoadConfigButtonPressed();
  void OnTaskManagerTaskHasNoInputsDropped(Task* task);
  void OnTaskStarted(bool execute);
  void OnTaskProgress(bool execute, double progress);
  void OnTaskFinished(bool execute);
  void OnLastImageLoaderTaskFinished(bool execute);
  void OnStartButtonPressed();
  void OnRenderViewToolButtonToggled(bool checked);
  void OnRenderViewFinished(int result);
  void OnGUIUpdateTimerTimeout();
  void OnUndoRedoManagerStackModified();
  void OnUndoRedoManagerTaskExecutionRequest(AbstractTransformTask *task,
      bool &success);
  void OnUndoMenuTriggered(QAction *action);
  void OnRedoMenuTriggered(QAction *action);
  void OnUndoToolButtonClicked();
  void OnRedoToolButtonClicked();
  void OnResetToolButtonClicked();
  void OnAboutButtonPressed();
  void OnHelpButtonPressed();
  void OnHelpLoadStarted();
  void OnHelpLoadFinished(bool ok);
  void OnHelpLoadProgress(int progress);
  void OnLastWindowClosed();
  void OnReferenceToolButtonClicked();
  void OnITFOptimizerToolButtonClicked();
  void OnSaveDRRsToolButtonClicked();
  void OnSaveBlendingToolButtonClicked();

private:
  Ui::REG23ControlWindowClass ui;
};

}

#endif // ORAREG23CONTROLWINDOW_H
