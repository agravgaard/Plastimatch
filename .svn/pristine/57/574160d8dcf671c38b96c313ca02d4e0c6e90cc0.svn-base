//
#ifndef ORAIMAGEIMPORTERTASK_H_
#define ORAIMAGEIMPORTERTASK_H_

#include "oraTask.h"

namespace ora
{

// forward declarations:
class ImageConsumer;
class IniAccess;

/** \class ImageImporterTask
 * \brief A task that imports and orients images (ITK images, ORA images).
 *
 * This is a concrete implementation of a task that imports and orients images
 * (ITK images, ORA images). In order to correctly orient ORA images a valid
 * ORA-connectivity must be established (access to ORA patient and site data).
 *
 * The image is imported to the connected target image consumer.
 *
 * @see Task
 * @see ImageConsumer
 *
 * @author phil 
 * @version 1.0
 */
class ImageImporterTask : public Task
{
public:
  // FIXME: remove pixel type later
  typedef unsigned short PixelType;

  /** Import mode. **/
  typedef enum
  {
    /** import a volume (ORA.XML or ITK image) incl. optional re-orientation **/
    IM_VOLUME = 0,
    /** import a planar image (RTI or ITK image) **/
    IM_PLANAR_IMAGE
  } ImportMode;

  /** Default constructor **/
  ImageImporterTask();
  /** Destructor **/
  virtual ~ImageImporterTask();

  /**
   * This task can be unexecuted as it loads and orients an image, and adds it
   * to the target image consumer. Unexecuting means that the image is removed
   * from the target image consumer.
   * @see Task#IsUnexecutable()
   */
  virtual bool IsUnexecutable() const
  {
    return true && m_ForcedUnexecutableState;
  }

  /**
   * This method allows the unexecutable state to be externally modified
   * (overridden). This could make sense if an external system does not want
   * the task to be unexecutable.
   */
  virtual void ForceUnexecutableState(bool state)
  {
    m_ForcedUnexecutableState = state;
  }

  /**
   * This task can be reexecuted. Reexecuting means that the image is loaded
   * and oriented again, and then simply added to the target image consumer
   * again.
   * @see Task#IsReexecuteable()
   */
  virtual bool IsReexecuteable() const
  {
    return IsUnexecutable() && true;
  }

  /**
   * Certainly, this task has an effect - it adds the image to the target image
   * consumer. This property is statically set here.
   * @see Task#HasNoEffect()
   */
  virtual bool HasNoEffect() const
  {
    return false;
  }

  /**
   * Returns true if a valid file name and (if required) the ORA-connectivity
   * parameters are OK.
   * @see Task#HasInput()
   */
  virtual bool HasInput() const;

  /**
   * Does nothing, simply returns false. The input is either set or not when the
   * task is added to the task manager.
   * @see Task#AcquireInput()
   */
  virtual bool AcquireInput()
  {
    return false;
  }

  /** Executes importing and orienting of ITK or ORA images. Add the image to
   * the target image consumer.
   * @return TRUE if the execution succeeded
   * @see Task#Execute()
   */
  virtual bool Execute();

  /** Returns true if the un-execute information was constructed (successful
   * previous execution).
   * @see Task#HasUnexecute()
   */
  virtual bool HasUnexecute() const
  {
    return m_UnexecuteInfoAvailable;
  }

  /** Unexecutes the task. Remove the image from the target image consumer.
   * @see Task#Unexecute()
   */
  virtual bool Unexecute();

  /**
   * The name of the task is constructed by a base name "Import image" and the
   * additional file name information.
   * @see Task#GetName()
   */
  virtual QString GetName();

  /**
   * This task does not support progress information.
   * @see Task#SupportsProgressInformation()
   */
  bool SupportsProgressInformation() const
  {
    return false;
  }

  /**
   * This task does not support cancel (image loading is usually a bulk-op).
   * @see Task#IsCancelable()
   */
  bool IsCancelable() const
  {
    return false;
  }

  /**
   * Basically this method returns TRUE. It shows, however, only an effect
   * if the unexecutable state is forced to FALSE!
   */
  bool DeleteThisTaskAfterProcessing()
  {
    return true;
  }

  /** Set image consumer which should receive the imported image. **/
  void SetTargetImageConsumer(ImageConsumer *consumer)
  {
    m_TargetImageConsumer = consumer;
  }
  /** Get image consumer which should receive the imported image. **/
  ImageConsumer *GetTargetImageConsumer()
  {
    return m_TargetImageConsumer;
  }

  /** Set ORA site info file object (part of ORA-connectivity) **/
  void SetSiteInfo(IniAccess *siteInfo)
  {
    m_SiteInfo = siteInfo;
  }
  /** Get ORA site info file object (part of ORA-connectivity) **/
  IniAccess *GetSiteInfo()
  {
    return m_SiteInfo;
  }

  /** Set ORA patient UID (part of ORA-connectivity) **/
  void SetPatientUID(QString uid)
  {
    m_PatientUID = uid;
  }
  /** Get ORA patient UID (part of ORA-connectivity) **/
  QString GetPatientUID()
  {
    return m_PatientUID;
  }

  /** Set file name of the image to be imported (ITK image, ORA image) **/
  void SetImageFileName(QString fileName)
  {
    m_ImageFileName = fileName;
  }
  /** Get file name of the image to be imported (ITK image, ORA image) **/
  QString GetImageFileName()
  {
    return m_ImageFileName;
  }

  /** Set relative iso-center position of the volume to be converted. **/
  void SetVolumeIsoCenter(double *ic)
  {
    m_VolumeIsoCenter = ic;
  }
  /** Get relative iso-center position of the volume to be converted. **/
  double *GetVolumeIsoCenter()
  {
    return m_VolumeIsoCenter;
  }

  /** Set the current import mode. **/
  void SetImportMode(ImportMode im)
  {
    m_ImportMode = im;
  }
  void SetImportModeToVolume()
  {
    SetImportMode(IM_VOLUME);
  }
  void SetImportModeToPlanarImage()
  {
    SetImportMode(IM_PLANAR_IMAGE);
  }
  /** Get the current import mode. **/
  ImportMode GetImportMode()
  {
    return m_ImportMode;
  }

protected:
  /** Execution done, unexecution information is now available. **/
  bool m_UnexecuteInfoAvailable;
  /** Image consumer which should receive the imported image. **/
  ImageConsumer *m_TargetImageConsumer;
  /** ORA site info file object (part of ORA-connectivity) **/
  IniAccess *m_SiteInfo;
  /** ORA patient UID (part of ORA-connectivity) **/
  QString m_PatientUID;
  /** File name of the image to be imported (ITK image, ORA image) **/
  QString m_ImageFileName;
  /**
   * Relative iso-center position of the volume to be converted. Convention
   * follows the convention of beam-info-file. Values are in mm! This prop is
   * optional (and only important for volume conversion).
   **/
  double *m_VolumeIsoCenter;
  /** Import mode. **/
  ImportMode m_ImportMode;
  /** Forced unexecutable state **/
  bool m_ForcedUnexecutableState;

private:
  ImageImporterTask(const ImageImporterTask&);  // no copy
  ImageImporterTask& operator=(const ImageImporterTask&); // no assignment

};


}


#endif /* ORAIMAGEIMPORTERTASK_H_ */
