//
#include "oraImageImporterTask.h"

#include "oraImageConsumer.h"
#include "oraImageOrienter.h"

// ORAIFTools
#include "oraIniAccess.h"
#include "oraStringTools.h"

#include <itksys/SystemTools.hxx>


namespace ora
{

ImageImporterTask::ImageImporterTask()
  : Task()
{
  m_UnexecuteInfoAvailable = false;
  m_TargetImageConsumer = NULL;
  m_SiteInfo = NULL;
  m_PatientUID = "";
  m_ImageFileName = "";
  m_VolumeIsoCenter = NULL;
  m_ImportMode = IM_PLANAR_IMAGE;
  m_ForcedUnexecutableState = true;
}

ImageImporterTask::~ImageImporterTask()
{
  m_TargetImageConsumer = NULL;
  m_SiteInfo = NULL;
  m_VolumeIsoCenter = NULL;
}

bool ImageImporterTask::HasInput() const
{
  bool ok = true;
  if (!itksys::SystemTools::FileExists(m_ImageFileName.toStdString().c_str()))
    ok = false;
  std::string ext = itksys::SystemTools::GetFilenameExtension(
      m_ImageFileName.toStdString());
  if (ToLowerCaseF(ext) == ".rti" ||
      ToLowerCaseF(ext) == ".rti.org" || ToLowerCaseF(ext) == ".ora.xml") // -> ORA
  {
    if (!m_SiteInfo || m_PatientUID.length() <= 0) // ORA-connectivity
      ok = false;
  }
  if (!m_TargetImageConsumer) // target required
    ok = false;
  return ok;
}

bool ImageImporterTask::Execute()
{
  emit
  TaskStarted(true);
  ImageOrienter<PixelType> orienter;
  // apply ORA-connectivity settings if applicable:
  if (m_SiteInfo && m_PatientUID.length() > 0)
  {
    std::string s = m_SiteInfo->ReadString("Paths", "PatientsFolder", "", true);
    EnsureStringEndsWith(s, DSEP);
    s += m_PatientUID.toStdString() + DSEP;
    s += m_SiteInfo->ReadString("Paths", "FramesOfReferenceFile", "");
    if (itksys::SystemTools::FileExists(s.c_str()))
      orienter.SetFramesInfo(s);
    else
      orienter.SetFramesInfo("");
  }
  bool succ = false;

  ImageOrienter<PixelType>::ImageType *image = NULL;
  if (m_ImportMode == IM_VOLUME) // --- VOLUME IMPORT ---
  {
    double isoc[3];
    if (!m_VolumeIsoCenter)
    {
      isoc[0] = 0;
      isoc[1] = 0;
      isoc[2] = 0;
    }
    else
    {
      isoc[0] = m_VolumeIsoCenter[0];
      isoc[1] = m_VolumeIsoCenter[1];
      isoc[2] = m_VolumeIsoCenter[2];
    }
    image = orienter.ConvertVolumeObject(m_ImageFileName.toStdString(), isoc);
  }
  else // IM_PLANAR_IMAGE // --- PLANAR IMAGE IMPORT ---
  {
    image = orienter.ConvertPlanarImageObject(m_ImageFileName.toStdString());
  }
  if (image)
  {
    if (m_TargetImageConsumer)
      m_TargetImageConsumer->ConsumeImage(image); // set image!
    m_UnexecuteInfoAvailable = true;
    succ = true;
  }

  emit
  TaskFinished(true);
  return succ;
}

bool ImageImporterTask::Unexecute()
{
  emit TaskStarted(false);
  if (m_TargetImageConsumer)
    m_TargetImageConsumer->UnconsumeImage();
  m_UnexecuteInfoAvailable = false;
  emit
  TaskFinished(false);
  return false;
}

QString ImageImporterTask::GetName()
{
  QString nam = this->Task::GetName();
  if (nam.length() <= 0)
  {
    nam = "Import Image (";
    nam += m_ImageFileName + ")";
  }
  return nam;
}

}
