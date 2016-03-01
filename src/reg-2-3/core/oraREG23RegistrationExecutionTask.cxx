//
#include "oraREG23RegistrationExecutionTask.h"

#include <QLocale>

#include "oraREG23Model.h"

#include <itkRealTimeClock.h>

namespace ora
{

REG23RegistrationExecutionTask::REG23RegistrationExecutionTask() :
  Task()
{
  m_CancelRequest = false;
  m_UnexecuteInfoAvailable = false;
  m_IncludeRegistrationTimeInName = false;
  m_RegistrationTimeFormatString = "(%1 s)";
  m_StartTime = 0;
}

REG23RegistrationExecutionTask::~REG23RegistrationExecutionTask()
{
  ;
}

bool REG23RegistrationExecutionTask::HasInput() const
{
  return this->m_TargetModel->IsReadyForAutoRegistration();
}

void REG23RegistrationExecutionTask::OnRegistrationCallback(itk::Object *obj,
    const itk::EventObject &ev)
{
  REG23Model *tm = this->m_TargetModel;
  if (tm->m_RegistrationIsRunning)
  {
    bool forceUpdate = false;
    if (std::string(ev.GetEventName()) == "EndEvent")
      forceUpdate = true;
    ITKVTKImage *ivi = tm->m_Volume->ProduceImage();
    TEMPLATE_CALL_COMP(ivi->GetComponentType(),
                       tm->UpdateCurrentRegistrationParameters, m_CancelRequest,
                       forceUpdate)
  }
}

void REG23RegistrationExecutionTask::OnOptimizerCallback(itk::Object *obj,
    const itk::EventObject &ev)
{
  REG23Model *tm = this->m_TargetModel;
  if (tm->m_RegistrationIsRunning)
  {
    tm->IncrementIteration();
    ITKVTKImage *ivi = tm->m_Volume->ProduceImage();
    TEMPLATE_CALL_COMP(ivi->GetComponentType(),
                       tm->UpdateCurrentRegistrationParameters, m_CancelRequest)
    // - update progress
    double progress = (double)tm->m_CurrentIteration /
        (double)tm->m_NumberOfIterations * 100.;
    emit TaskProgressInfo(true, progress);
  }
}

bool REG23RegistrationExecutionTask::Execute()
{
  m_CancelRequest = false;
  REG23Model *tm = this->m_TargetModel;

  // do the real registration (NOTE: the progress is updated via the
  // callback commands of the registration framework)
  itk::RealTimeClock::Pointer timer = itk::RealTimeClock::New();
  m_StartTime = timer->GetTimeStamp();

  emit
  TaskStarted(true);
  double p = 0.;
  emit
  TaskProgressInfo(true, p);
  bool succ = true;

  if (!m_CancelRequest)
  {
    // connect the callback commands
    typedef REG23Model::CommandType::TMemberFunctionPointer MemberPointer;
    tm->GetRegistrationCommand()->SetCallbackFunction(this,
        (MemberPointer)&REG23RegistrationExecutionTask::OnRegistrationCallback);
    tm->GetOptimizerCommand()->SetCallbackFunction(this,
        (MemberPointer)&REG23RegistrationExecutionTask::OnOptimizerCallback);

    ITKVTKImage *ivi = tm->m_Volume->ProduceImage();
    TEMPLATE_CALL_COMP(ivi->GetComponentType(),
                       succ = tm->ExecuteRegistration, )
    if (succ)
    {
      p = 100.;
      emit TaskProgressInfo(true, p);
    }
  }

  emit
  TaskFinished(true); // throw in every case!

  succ &= (!m_CancelRequest);
  return succ;
}

QString REG23RegistrationExecutionTask::GetName()
{
  QString nam = this->Task::GetName(); // returns this->m_CustomName
  if (nam.length() <= 0) // default
  {
    nam = "Registration";
  }
  if (m_IncludeRegistrationTimeInName)
  {
    double currentRegistrationTime = GetElapsedTimeSinceStart();
    QLocale loc;
    QString fs = loc.toString(currentRegistrationTime, 'f', 2);
    nam += m_RegistrationTimeFormatString.arg(fs);
  }
  return nam;
}

double REG23RegistrationExecutionTask::GetElapsedTimeSinceStart()
{
  itk::RealTimeClock::Pointer timer = itk::RealTimeClock::New();
  double currentRegistrationTime = timer->GetTimeStamp() - m_StartTime;
  if (currentRegistrationTime < 0)
    currentRegistrationTime = 0;
  return currentRegistrationTime;
}

}
