

#include "oraQVTKProgressEventAdaptor.h"

// Forward declarations
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkObject.h"


namespace ora
{


void
QVTKProgressEventAdaptor
::ProcessProgressEvents(vtkObject *object, unsigned long event,
    void *clientdata, void *calldata)
{
  QVTKProgressEventAdaptor *thiss =
    reinterpret_cast<QVTKProgressEventAdaptor *>(clientdata);

  if (thiss && event == vtkCommand::ProgressEvent)
  {
    double *progress = (double*)calldata; // expected in call data!
    thiss->EmitProgressSignal(*progress);
  }
}

QVTKProgressEventAdaptor
::QVTKProgressEventAdaptor()
  : QObject(), SimpleDebugger()
{
  m_CallbackCommand = vtkCallbackCommand::New();
  m_CurrentVTKObject = NULL;
  m_CurrentObserverTag = 0;
}

QVTKProgressEventAdaptor
::~QVTKProgressEventAdaptor()
{
  m_CallbackCommand->Delete();
  m_CallbackCommand = NULL;
}

bool
QVTKProgressEventAdaptor
::Register(vtkObject *object)
{
  if (object && m_CallbackCommand)
  {
    m_CurrentVTKObject = object;

    m_CallbackCommand->SetClientData(this);
    m_CallbackCommand->SetCallback(
        QVTKProgressEventAdaptor::ProcessProgressEvents);
    m_CurrentObserverTag = m_CurrentVTKObject->AddObserver(
        vtkCommand::ProgressEvent, m_CallbackCommand);

    return true;
  }

  return false;
}

void
QVTKProgressEventAdaptor
::Unregister()
{
  if (m_CallbackCommand && m_CurrentVTKObject)
  {
    m_CurrentVTKObject->RemoveObserver(m_CurrentObserverTag);
    m_CurrentObserverTag = 0;
  }
}

vtkCommand *
QVTKProgressEventAdaptor
::GetPreparedCommand()
{
  if (m_CallbackCommand)
  {
    m_CallbackCommand->SetClientData(this);
    m_CallbackCommand->SetCallback(
        QVTKProgressEventAdaptor::ProcessProgressEvents);

    return m_CallbackCommand;
  }
  else
    return NULL;
}

void
QVTKProgressEventAdaptor
::EmitProgressSignal(double progress)
{
  emit Progress(progress);
}


}
