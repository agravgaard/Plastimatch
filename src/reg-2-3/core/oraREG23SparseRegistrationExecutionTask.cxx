//
#include "oraREG23SparseRegistrationExecutionTask.h"

#include <QLocale>

#include "oraREG23Model.h"
// ORAIFTools
#include <oraStringTools.h>

#include <itkCommand.h>

namespace ora
{

REG23SparseRegistrationExecutionTask::REG23SparseRegistrationExecutionTask() :
  REG23RegistrationExecutionTask()
{
  m_NumberOfIterations = -1;
}

REG23SparseRegistrationExecutionTask::~REG23SparseRegistrationExecutionTask()
{
  ;
}

void REG23SparseRegistrationExecutionTask::OnInternalFilterProgressReceptor(
    itk::Object *caller, const itk::EventObject &event)
{
  itk::ProcessObject *filter = dynamic_cast<itk::ProcessObject *>(caller);

  if(!itk::ProgressEvent().CheckEvent(&event) || !filter)
    return;

  if (filter)
  {
    double p = m_CurrentProgressStart;
    // filter->GetProgress() in [0;1]
    p += m_CurrentProgressSpan * filter->GetProgress();
    // NOTE: filter is buggy - not in [0;1] if multi-threading is activated!
    if (p > (m_CurrentProgressStart + m_CurrentProgressSpan))
      p = m_CurrentProgressStart + m_CurrentProgressSpan;
    TaskProgressInfo(m_CurrentProgressDirection, p);

    if (m_CancelRequest)
      filter->SetAbortGenerateData(true); // may be handled by filter
  }
}

bool REG23SparseRegistrationExecutionTask::Execute()
{
  REG23Model *tm = this->m_TargetModel;
  this->m_CancelRequest = false;
  this->m_CurrentProgressDirection = true;

  itk::RealTimeClock::Pointer timer = itk::RealTimeClock::New();
  m_StartTime = timer->GetTimeStamp();

  double p = 0.;
  emit
  TaskStarted(true);
  emit
  TaskProgressInfo(true, 0);
  bool succ = true;

  if (!m_CancelRequest)
  {
    tm->SetExecuteSparsePreRegistration(false); // set back - at least once started

    if (tm->m_SparsePreRegistrationType == "CROSSCORRELATION")
    {
      typedef itk::MemberCommand<REG23SparseRegistrationExecutionTask> ITKCommandType;
      typedef ITKCommandType::TMemberFunctionPointer MemberPointer;

      ITKCommandType::Pointer cmd = ITKCommandType::New();
      cmd->SetCallbackFunction(this,
          (MemberPointer)&REG23SparseRegistrationExecutionTask::OnInternalFilterProgressReceptor);

      // OK, the arguments were checked for correctness, extract them now:
      // CrossCorrelation(<num-it>,<x-max>,<y-max>)
      REG23Model::ParametersType prevPars = tm->GetCurrentParameters(); // store
      std::string s = TrimF(tm->m_Config->ReadString("Registration",
          "SparsePreRegistration", ""));
      std::vector<std::string> toks;
      s = s.substr(17, s.length());
      s = s.substr(0, s.length() - 1); // cut last ")"
      Tokenize(s, toks, ",");
      int numIt = atoi(toks[0].c_str());
      double maxTRE = atof(toks[1].c_str());

      bool fastSearch = false;
      bool radiusSearch = false;
      for (unsigned int i = 2; i < toks.size(); ++i) {
        if (ToUpperCaseF(toks[i]) == "FAST")
          fastSearch = true;
        if (ToUpperCaseF(toks[i]) == "RADIUS")
          radiusSearch = true;
      }

      int numViews = 0;
      for (std::size_t i = 1; i <= tm->m_FixedImageFileNames.size(); i++)
      {
        s = TrimF(tm->m_Config->ReadString("Registration",
            "CrossCorrelationView" + StreamConvert(i), ""));
        if (s.length() > 0)
          numViews++;
      }
      m_CurrentProgressSpan = 100. / (double)(numViews * numIt);

      m_CurrentProgressStart = -m_CurrentProgressSpan;
      bool unsharpMask;
      std::vector<double> patchConfig;
      std::vector<double> fregConfig;
      std::vector<double> args;
      int index = 0;
      double tx = 0, ty = 0;
      REG23Model::ParametersType pars;
      ITKVTKImage *ivi = tm->GetVolumeImage();
      for (int j = 1; j <= numIt; j++)
      {
        for (std::size_t i = 1; i <= tm->m_FixedImageFileNames.size(); i++)
        {
          if (m_CancelRequest)
            break;

          // CrossCorrelationView{x}=<usharp>,<p-off-x>,<p-off-y>,<p-sz-x>,<p-sz-y>,<f-x->,<f-x+>,<f-y->,<f-y+>,<structure-uid>
          s = TrimF(tm->m_Config->ReadString("Registration",
              "CrossCorrelationView" + StreamConvert(i), ""));
          if (s.length() <= 0) // view not configured
            continue;

          toks.clear();
          Tokenize(s, toks, ",");
          index = i - 1; // zero-based view index
          unsharpMask = static_cast<bool>(atoi(toks[0].c_str()));
          patchConfig.clear();
          for (int k = 1; k <= 4; k++)
            patchConfig.push_back(static_cast<double>(atof(toks[k].c_str())));
          fregConfig.clear();
          for (int k = 5; k <= 8; k++)
            fregConfig.push_back(static_cast<double>(atof(toks[k].c_str())));
          std::string structureUID = toks[9];

          m_CurrentProgressStart += m_CurrentProgressSpan;

          // call the method using the parsed arguments:
          // ARGS-FORMAT: Default: {x-, x+, y-, y+, xoff, yoff, w, h} = {-30, 30, -30, 30, -30, -30, 61, 61}
          args.clear();
          args.push_back(fregConfig[0]);
          args.push_back(fregConfig[1]);
          args.push_back(fregConfig[2]);
          args.push_back(fregConfig[3]);
          args.push_back(patchConfig[0]);
          args.push_back(patchConfig[1]);
          args.push_back(patchConfig[2]);
          args.push_back(patchConfig[3]);
          TEMPLATE_CALL_COMP(ivi->GetComponentType(),
              succ = tm->ComputeCrossCorrelationInitialTransform,
              index, tx, ty, unsharpMask, fastSearch, radiusSearch, args, structureUID, cmd)
          if (fastSearch && (!succ || vcl_sqrt(tx * tx + ty * ty) > maxTRE))
          {
            TEMPLATE_CALL_COMP(ivi->GetComponentType(),
                succ = tm->ComputeCrossCorrelationInitialTransform,
                index, tx, ty, unsharpMask, false, radiusSearch, args, structureUID, cmd)
          }

          if (succ)
          {
            // after each computation, update the transform and model:
            // - compute resultant 3D translation vector:
            tx = -tx; // invert result!
            ty = -ty;
            pars = tm->ComputeTransformationFromPhysicalInPlaneTranslation(
                index, tx, ty);
            // - update model and images
            tm->OverrideAndApplyCurrentParameters(pars);
            TEMPLATE_CALL_COMP(ivi->GetComponentType(),
                tm->ComputeCurrentMovingImages)

            m_NumberOfIterations = j; // update
            p = m_CurrentProgressStart + m_CurrentProgressSpan;
            emit TaskProgressInfo(true, p);
          }
          else
          {
            break;
          }
        }
        if (!succ)
          break;
      }

      // OK, now we have to check whether we're in tolerance and whether to
      // apply the result:
      bool applyTransform = true;
      if (maxTRE > 0)
      {
        REG23Model::TransformType::InputPointType zero;
        zero.Fill(0);
        REG23Model::TransformType::OutputPointType p = tm->m_Transform->TransformPoint(zero);
        if (p.EuclideanDistanceTo(zero) > maxTRE)
          applyTransform = false;
      }
      if (!applyTransform)
      {
        // restore old parameters
        tm->OverrideAndApplyCurrentParameters(prevPars);
        TEMPLATE_CALL_COMP(ivi->GetComponentType(),
            tm->ComputeCurrentMovingImages)
      }
    }
    else // NOT IMPLEMENTED
    {
      p = 100.; // nothing to do
      emit TaskProgressInfo(true, p);
    }
  }

  emit
  TaskFinished(true);

  succ &= (!m_CancelRequest);
  return succ;
}

QString REG23SparseRegistrationExecutionTask::GetName()
{
  QString nam = this->Task::GetName(); // returns this->m_CustomName
  if (nam.length() <= 0) // default
  {
    nam = "Sparse Registration";
  }
  if (this->m_IncludeRegistrationTimeInName)
  {
    double currentRegistrationTime = this->GetElapsedTimeSinceStart();
    QLocale loc;
    QString fs = loc.toString(currentRegistrationTime, 'f', 2);
    nam += this->m_RegistrationTimeFormatString.arg(fs);
  }
  return nam;
}

}
