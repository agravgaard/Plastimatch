//
#include "oraREG23RegistrationInitializationTask.h"

#include "oraREG23Model.h"

// ORAIFModel
#include <oraImageConsumer.h>
// ORAIFTools
#include <oraSimpleMacros.h>
// ORAIFStructureAccess
#include <oraVTKStructure.h>

namespace ora
{

REG23RegistrationInitializationTask::REG23RegistrationInitializationTask() :
    Task()
{
  m_CancelRequest = false;
  m_UnexecuteInfoAvailable = false;
  m_PreProcessingName = "";
  m_MaskGenerationName = "";
  m_PostProcessingName = "";
  m_RegInitName = "";
}

REG23RegistrationInitializationTask::~REG23RegistrationInitializationTask()
{
  ;
}

bool REG23RegistrationInitializationTask::HasInput() const
{
  // NOTE: We've access to the internals of the model since we're friends!
  if (!m_TargetModel)
    return false;
  if (!m_TargetModel->m_ValidConfiguration)
    return false;
  if (!m_TargetModel->m_Volume || !m_TargetModel->m_Volume->ProduceImage())
    return false;
  if (m_TargetModel->m_FixedImages.size() <= 0)
    return false;
  for (std::size_t i = 0; i < m_TargetModel->m_FixedImages.size(); i++)
  {
    if (!m_TargetModel->m_FixedImages[i]
        || !m_TargetModel->m_FixedImages[i]->ProduceImage())
      return false;
  }
  return true;
}

bool REG23RegistrationInitializationTask::Execute()
{
  m_CancelRequest = false;
  this->m_CustomName = m_PreProcessingName; // next step
  REG23Model *tm = this->m_TargetModel;
  emit
  TaskStarted(true);
  double p = 0., pstart, pend, phelp;
  emit
  TaskProgressInfo(true, p);
  bool succ = true;

  // (optional) image pre-processing:
  if (!m_CancelRequest)
  {
    pstart = 0.;
    pend = 33.33;
    phelp = 1.0 + (double)tm->m_FixedImages.size();
    QString key = "Volume";
    // - volume
    ITKVTKImage *ivi = tm->m_Volume->ProduceImage();
    ITKVTKImage *pivi = NULL;
    ITKVTKImage::ITKImagePointer itkBaseImage;
    bool error = false;
    if (tm->m_PPVolume)
      delete tm->m_PPVolume;
    tm->m_PPVolume = NULL;
    TEMPLATE_CALL_COMP(ivi->GetComponentType(),
                       itkBaseImage = ivi->GetAsITKImage, )
    TEMPLATE_CALL_COMP(ivi->GetComponentType(), pivi = tm->PreProcessImage,
                       itkBaseImage, key.toStdString(), error,
                       ivi->GetMetaInfo())
    if (pivi) // successful image pre-processing
    {
      tm->m_PPVolume = new ImageConsumer(true); // delete @ end (new image)
      tm->m_PPVolume->ConsumeImage(pivi);
    }
    else if (!pivi && !error) // no image pre-processing necessary
    {
      tm->m_PPVolume = new ImageConsumer(false); // DON'T delete @ end (image ref)
      tm->m_PPVolume->ConsumeImage(ivi);
    }
    else // error on image pre-processing
    {
      emit TaskFinished(true); // throw in every case!
      return false; // error!
    }
    p = pstart + (pend - pstart) * 1. / phelp;
    emit TaskProgressInfo(true, p);
    // - fixed images
    for (std::size_t i = 0; i < tm->m_PPFixedImages.size(); i++)
      if (tm->m_PPFixedImages[i])
        delete tm->m_PPFixedImages[i];
    tm->m_PPFixedImages.clear();
    for (std::size_t i = 0; i < tm->m_FixedImages.size(); i++)
    {
      if (m_CancelRequest) // user-break
        break;
      key = "FixedImage" + QString::fromStdString(StreamConvert(i + 1));
      ivi = tm->m_FixedImages[i]->ProduceImage();
      TEMPLATE_CALL_COMP(ivi->GetComponentType(),
                         itkBaseImage = ivi->GetAsITKImage, )
      TEMPLATE_CALL_COMP(ivi->GetComponentType(), pivi = tm->PreProcessImage,
                         itkBaseImage, key.toStdString(), error,
                         ivi->GetMetaInfo())
      if (pivi) // successful image pre-processing
      {
        ImageConsumer *fic = new ImageConsumer(true); // delete @ end (new image)
        fic->ConsumeImage(pivi);
        tm->m_PPFixedImages.push_back(fic);
      }
      else if (!pivi && !error) // no image pre-processing necessary
      {
        ImageConsumer *fic = new ImageConsumer(false); // DON'T delete @ end (image ref)
        fic->ConsumeImage(ivi);
        tm->m_PPFixedImages.push_back(fic);
      }
      else // error on image pre-processing
      {
        emit TaskFinished(true); // throw in every case!
        return false; // error!
      }
      p = pstart + (pend - pstart) * ((double)i + 2.) / phelp;
      emit TaskProgressInfo(true, p);
    }

    // model notifies observers that input images are pre-processed & ready:
    tm->SendInputImagesCompleteNotification();

    this->m_CustomName = m_MaskGenerationName; // next step
    emit TaskProgressInfo(true, p);
  }

  // (optional) auto-mask generation:
  if (!m_CancelRequest)
  {
    pstart = 33.33;
    pend = 66.66;

    // - fix source positions (auto-determination where needed):
    if (!tm->FixAutoSourcePositions())
    {
      emit TaskFinished(true); // throw in every case!
      return false; // error
    }

    // if the "intelligent mask mode" is ON, this method tries to load previously
    // generated and stored 2D masks instead of re-generating them (faster!);
    // however, this is only possible if certain criteria are met!
    bool intelligentMasksLoaded = tm->LoadIntelligentMasks();

    if (!intelligentMasksLoaded)
    {
      // - load structures:
      std::size_t numstructs = tm->m_Structures->GetNumberOfStructures();
      phelp = (double)numstructs;
      for (std::size_t i = 1; i <= numstructs; i++)
      {
        if (m_CancelRequest) // user-break
          break;
        std::string ek = "", em = "";
        if (!tm->ParseAndProcessStructureEntry(i, false, ek, em))
        {
          emit TaskFinished(true); // throw in every case!
          return false; // error
        }
        p = pstart + (pend - pstart) * 0.5 * (double)i / phelp;
        emit TaskProgressInfo(true, p);
      }
    }
    else // intelligent masks were loaded
    {
      // NOTE: In future, if we support silhouette-projections one day, we must
      // load the structures though! We must keep that in mind!
      // For sparse pre-registration, which partially also requires a reference
      // structure, there is a built-in solution ("-stored-structure-props")!

      p = pstart + (pend - pstart) * 0.5;
      emit TaskProgressInfo(true, p);
    }

    if (!m_CancelRequest)
    {
      if (!intelligentMasksLoaded)
      {
        // - now, really generate the mask images:
        phelp = (double)tm->m_FixedImages.size();
        for (std::size_t i = 0; i < tm->m_FixedImages.size(); i++)
        {
          if (m_CancelRequest) // user-break
            break;
          ITKVTKImage *imask = NULL;
          std::string ek = "", em = "";
          if (!tm->ParseAndProcessMaskEntry(i + 1, false, ek, em, imask))
          {
            emit TaskFinished(true); // throw in every case!
            return false; // error
          }
          tm->m_MaskImages[i]->ConsumeImage(imask); // NOTE: can be NULL(=no mask)
          p = pstart + (pend - pstart) * 0.5 +
              (pend - pstart) * 0.5 * ((double)i + 1.) / phelp;
          emit TaskProgressInfo(true, p);
        }
      }

      // model notifies observers (BUT only if there are mask images):
      tm->SendMaskImagesCompleteNotification();

      this->m_CustomName = m_PostProcessingName; // next step
      p = 66.66;
      emit TaskProgressInfo(true, p);
    }
  }

  // (optional) fixed image post-processing:
  if (!m_CancelRequest)
  {
    pstart = 66.66;
    pend = 90.0;
    phelp = 1.0 + (double)tm->m_PPFixedImages.size();
    QString key = "FixedImage";
    ITKVTKImage *ivi = NULL;
    ITKVTKImage *iviMask = NULL;
    ITKVTKImage *pivi = NULL;
    ITKVTKImage::ITKImagePointer itkBaseImage;
    bool error = false;
    for (std::size_t i = 0; i < tm->m_PPFixedImages.size(); i++)
    {
      if (m_CancelRequest) // user-break
        break;
      key = "FixedImage" + QString::fromStdString(StreamConvert(i + 1));
      ivi = tm->m_PPFixedImages[i]->ProduceImage();
      iviMask = tm->m_MaskImages[i]->ProduceImage(); // NOTE: can be NULL(=no mask)
      TEMPLATE_CALL_COMP(ivi->GetComponentType(),
                         itkBaseImage = ivi->GetAsITKImage, )
      TEMPLATE_CALL_COMP(ivi->GetComponentType(), pivi = tm->PostProcessImage,
                         itkBaseImage, key.toStdString(), error, iviMask,
                         ivi->GetMetaInfo())
      if (pivi) // successful image post-processing
      {
        ImageConsumer *fic = new ImageConsumer(true); // delete @ end (new image)
        fic->ConsumeImage(pivi);
        // set image (The pre-processe done is kept)
        tm->m_PPFixedImagesPost[i] = fic;
      }
      else if (!pivi && !error) // no image post-processing necessary
      {
        // Indicate that no post-processing is applied/required with NULL
        tm->m_PPFixedImagesPost[i] = NULL;
      }
      else // error on image post-processing
      {
        emit TaskFinished(true); // throw in every case!
        return false; // error!
      }
      p = pstart + (pend - pstart) * ((double)i + 2.) / phelp;
      emit TaskProgressInfo(true, p);
    }

    // model notifies observers that input images are post-processed & ready:
    tm->SendInputImagesCompleteNotification();

    this->m_CustomName = m_RegInitName; // next step
    emit TaskProgressInfo(true, p);
  }

  // registration component instantiation and composition:
  if (!m_CancelRequest)
  {
    bool succeeded = false;
    ITKVTKImage *ivi = tm->m_Volume->ProduceImage();
    TEMPLATE_CALL_COMP(ivi->GetComponentType(),
                       succeeded = tm->InitializeInternal, )
    if (!succeeded)
    {
      emit TaskFinished(true); // throw in every case!
      return false; // error
    }
    p = 100.;
    emit TaskProgressInfo(true, p);
  }

  // set flag that sparse pre-registration should be executed (if configured)
  m_TargetModel->SetExecuteSparsePreRegistration(true);

  emit
  TaskFinished(true); // throw in every case!

  succ &= (!m_CancelRequest);
  return succ;
}

QString REG23RegistrationInitializationTask::GetName()
{
  QString nam = this->Task::GetName(); // returns this->m_CustomName
  if (nam.length() <= 0) // default
  {
    nam = "Initialize Registration";
  }
  return nam;
}

void REG23RegistrationInitializationTask::SetNames(
  QString preProcessingName, QString maskGenerationName,
  QString regInitName, QString postProcessingName)
{
  m_PreProcessingName = preProcessingName;
  m_MaskGenerationName = maskGenerationName;
  m_RegInitName = regInitName;
  m_PostProcessingName = postProcessingName;
}

}
