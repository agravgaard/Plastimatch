

#ifndef ORASETUPERROR_H_
#define ORASETUPERROR_H_

#include <vector>
#include <time.h>

// ORAIFTransform
#include "oraYawPitchRoll3DTransform.h"

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkVector.h>
#include <itkPoint.h>
#include <itkMatrix.h>


namespace ora
{


/** Macro for setting a property and updating the internal transform. **/
#define oraSetAndUpdateTransformMacro(name, type) \
  virtual void Set##name (const type _arg) \
  { \
    itkDebugMacro("setting " #name " to " << _arg); \
    if (this->m_##name != _arg) \
    { \
      this->m_##name = _arg; \
      this->UpdateInternalTransform(); \
      this->Modified(); \
    } \
  }

/** \class PlanarViewIdentifier
 * \brief Minimal helper class for identifying an ORA planar view image.
 *
 * Minimal helper class for identifying an ORA planar view image (by its name
 * and an acquistion date).
 *
 * @author phil 
 * @version 1.0
 **/
class PlanarViewIdentifier
{
public:
  /** Acquisition date as string. **/
  std::string AcquisitionDateTimeStr;
  /** Acquisition date as time struct. **/
  tm AcquisitionDateTime;
  /** Planar view file name. **/
  std::string FileName;
  /** Planar view's UID **/
  std::string ViewUID;

  /** Default constructor. **/
  PlanarViewIdentifier();
  /** Extended constructor initializing the props - syncs string vs. tm! **/
  PlanarViewIdentifier(std::string fileName, std::string acquDT);

private:
  /** purposely not implemented **/
  PlanarViewIdentifier(const PlanarViewIdentifier&);
  /** purposely not implemented **/
  void operator=(const PlanarViewIdentifier&);
};

/** \class SetupError
 * \brief Describes an ORA patient setup error.
 *
 * This class describes an ORA patient setup error and, therefore, implicitly
 * defines a relative transform related to the planned patient position.
 *
 * @author phil 
 * @version 1.0
 */
class SetupError
  : public itk::Object
{
public:
  /** some relevant typedefs **/
  typedef SetupError Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  /** Setup type **/
  typedef enum
  {
    ST_UNKNOWN = 0,
    ST_MARKER = 1,
    ST_RIGID_TRANSFORM = 2
  } SetupType;

  /** geometric types **/
  typedef itk::Vector<double, 3> Vector3DType;
  typedef itk::Point<double, 3> Point3DType;
  typedef itk::Matrix<double, 3, 3> Matrix3x3Type;
  typedef ora::YawPitchRoll3DTransform<double> TransformType;
  typedef TransformType::Pointer TransformPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(SetupError, itk::Object);

  itkSetMacro(PlanUID, std::string)
  itkGetMacro(PlanUID, std::string)

  itkSetEnumMacro(Method, SetupType)
  itkGetEnumMacro(Method, SetupType)

  itkSetMacro(OperatorSign, std::string)
  itkGetMacro(OperatorSign, std::string)

  void SetDateTime(std::string dateTime);
  itkGetMacro(DateTime, std::string)
  //itkGetMacro(CDateTime, tm)
  // Type tm does not support operator <<
  virtual tm GetCDateTime ()
  {
    itkDebugMacro("returning " << "CDateTime of " << asctime(&this->m_CDateTime));
    return this->m_CDateTime;
  }

  oraSetAndUpdateTransformMacro(TotalTranslation, Vector3DType)
  itkGetMacro(TotalTranslation, Vector3DType)

  oraSetAndUpdateTransformMacro(Rotation, Matrix3x3Type)
  itkGetMacro(Rotation, Matrix3x3Type)

  oraSetAndUpdateTransformMacro(Dilation, Vector3DType)
  itkGetMacro(Dilation, Vector3DType)

  oraSetAndUpdateTransformMacro(AverageIGRTTranslation, Vector3DType)
  itkGetMacro(AverageIGRTTranslation, Vector3DType)

  /**
   * Set the raw center of rotation position. NOTE: this method will set back
   * validity of the corrected center of rotation (which is fully defined in
   * WCS).
   * @see m_CenterOfRotation
   **/
  virtual void SetCenterOfRotation(Point3DType cor);
  itkGetMacro(CenterOfRotation, Point3DType)
  /**
   * Correct the center of rotation by transforming it into WCS (relation to
   * WCS zero-point!). NOTE: Furthermore, this method triggers the update of the
   * internal transform.
   * @param isoCenter relative position of the iso-center (WCS zero-point)
   * @see m_CenterOfRotation
   * @see UpdateInternalTransform()
   */
  virtual void CorrectCenterOfRotation(Point3DType isoCenter);
  /**
   * Get the corrected center of rotation position fully defined in WCS.
   * @param isValid returned indicating whether the returned coordinates are
   * trustworthy or not
   * @return the corrected center of rotation coordinates
   */
  virtual Point3DType GetCorrectedCenterOfRotation(bool &isValid);

  oraSetAndUpdateTransformMacro(RotationAngles, Vector3DType)
  itkGetMacro(RotationAngles, Vector3DType)

  oraSetAndUpdateTransformMacro(RealTranslation, Vector3DType)
  itkGetMacro(RealTranslation, Vector3DType)

  /**
   * Load the setup error from a typical open radART (study) setup error entry
   * (SetupErrors.inf or StudySetupErrors.inf). NOTE: the referenced plan UID
   * must be set externally as it is not contained in the entry!
   * @param oraString the open radART setup error entry
   * @return true if successful
   */
  bool LoadFromORAString(std::string oraString);

  /** @return TRUE if the other setup equals self **/
  bool Equals(SetupError::Pointer other);

  /**
   * Get internal transformation representation (as yaw-pitch-roll
   * transformation).
   **/
  itkGetObjectMacro(Transform, TransformType)

  /**
   * Find the best matching planar view from a vector of planar views (minimal
   * description). Matching is determined on a simple heuristic: setup error and
   * image date/times must not exceed maxSec seconds, the closest image wins.
   * @param planarViews pointer to the planar views which should be investigated
   * @param maxSec maximum number of seconds between setup error and view
   * @return the index in the vector of the best matching view or -1 if there is
   * no matching view (w.r.t. the internal heuristic)
   **/
  int FindBestMatchingPlanarView(
      std::vector<PlanarViewIdentifier *> *planarViews,
      unsigned int maxSec = 1800);

  /** @return true if the setup error emerges obviously from a study setup error
   * file **/
  bool IsStudySetupError()
  {
    if (m_ReferenceImageFilename.length() > 0)
      return true;
    return false;
  }

  itkSetMacro(ReferenceImageFilename, std::string)
  itkGetMacro(ReferenceImageFilename, std::string)

  oraSetAndUpdateTransformMacro(AllowRotation, bool)
  itkGetMacro(AllowRotation, bool)

protected:
  /** Determine whether or not the transformation should contain the rotational
   * components. This is because open-radART setup error file contain
   * rotation matrices in some cases, but these have not been applied - the
   * translations only have been applied! Rotations are currently only supported
   * for the abIGRT-protocol! **/
  bool m_AllowRotation;
  /** (Only for study setup errors) The name of one of the reference images
   * where the setup error was derived from **/
  std::string m_ReferenceImageFilename;
  /** UID of the referenced plan **/
  std::string m_PlanUID;
  /** Setup type **/
  SetupType m_Method;
  /** operator's sign **/
  std::string m_OperatorSign;
  /** date / time (string) **/
  std::string m_DateTime;
  /** date / time (c-time tm) **/
  tm m_CDateTime;
  /** total translation (mm) **/
  Vector3DType m_TotalTranslation;
  /** rotation (direction cosines) **/
  Matrix3x3Type m_Rotation;
  /** dilation **/
  Vector3DType m_Dilation;
  /** adjusted average IGRT position (mm) **/
  Vector3DType m_AverageIGRTTranslation;
  /**
   * center of rotation (especially for gold markers) (mm);<br>
   * <b>NOTE</b>: This point is defined in a coordinate system where the axes
   * are WCS-aligned, but the origin of this coordinate system relates to the
   * absolute patient coordinate system zero-point. In order to generate the
   * "corrected center of rotation" fully defined in WCS, one needs to specify
   * the relative iso-center position of the referenced plan. This can be
   * achieved using the CorrectCenterOfRotation()-method and the corrected
   * center of rotation can be retrieved by GetCorrectedCenterOfRotation().
   **/
  Point3DType m_CenterOfRotation;
  /**
   * corrected center of rotation (fully defined in WCS)
   * @see m_CenterOfRotation
   */
  Point3DType m_CorrectedCenterOfRotation;
  /** indicates whether the content of m_CorrectedCenterOfRotation is valid **/
  bool m_CenterOfRotationIsUpToDate;
  /** rotation angles (gold markers) (deg) **/
  Vector3DType m_RotationAngles;
  /** real applied translation (from couch position) (mm) **/
  Vector3DType m_RealTranslation;
  /**
   * Internal transformation representation (as yaw-pitch-roll transformation).
   */
  TransformPointer m_Transform;

  /** Default constructor. **/
  SetupError();
  /** Default destructor. **/
  ~SetupError();

  /**
   * Updates the internal transform from current set geometric settings. NOTE:
   * If the center of rotation is up to date (if the isValid-flag is true when
   * calling GetCorrectedCenterOfRotation(isValid)), this point is taken for
   * transform-composition! Otherwise the native center of rotation is taken.
   * @see m_Transform
   * @see m_CenterOfRotation
   **/
  void UpdateInternalTransform();

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  /** purposely not implemented **/
  SetupError(const Self&);
  /** purposely not implemented **/
  void operator=(const Self&);

};


/** \class SetupErrorCollection
 * \brief Manages a set of ORA setup error object.
 *
 * This class manages a set of ORA setup errors typically found in a setup
 * error info file. Moreover this class supports finding setup errors by
 * specified criteria (e.g. date of setup).
 *
 * @author phil 
 * @version 1.0
 */
class SetupErrorCollection
  : public itk::Object
{
public:
  /** some relevant typedefs **/
  typedef SetupErrorCollection Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(SetupErrorCollection, itk::Object);

  /**
   * Create a setup error collection from a typical open radART setup errors
   * file.
   * @param setupErrorsFile the open radART setup errors file
   * @return the created setup error collection if successful, NULL otherwise
   * @see ora::SetupError
   */
  static Pointer CreateFromFile(std::string setupErrorsFile);

  /**
   * Get class-wide static default setup error ('no error', rigid identity)
   * which can be retrieved without instantiation; SINGLETON.
   **/
  static SetupError::Pointer GetDefaultSetupError();

  /**
   * Add a new setup error.
   * @param se the setup error to be added
   * @return TRUE if successful (NOTE: doubletts are not allowed)
   */
  bool AddSetupError(SetupError::Pointer se);

  /** @return the number of currently managed setup errors **/
  unsigned int GetNumberOfSetupErrors();

  /** Clear all managed setup errors. **/
  void Clear();

  /** @return a direct reference to the internal setup errors **/
  const std::vector<SetupError::Pointer> *GetSetupErrors();

  /**
   * Remove the specified setup error from the internal list.
   * @param se the setup error to be removed
   * @return TRUE if found and removed
   **/
  bool RemoveSetupError(SetupError::Pointer se);

  /**
   * Remove the specified setup error(s) from the internal list. NOTE: all
   * setup errors that fit the specified criteria are removed!
   * @param dateTime date / time of setup (must not be empty!)
   * @param method setup method (if ST_UNKNOWN, this criterion is not checked)
   * @param operatorSign operator signature (if empty, this criterion is not
   * checked)
   * @return the number of removed setup error objects
   **/
  unsigned int RemoveSetupErrors(std::string dateTime,
      SetupError::SetupType method = SetupError::ST_UNKNOWN,
      std::string operatorSign = "");

  /**
   * Find the specified setup error. NOTE: if more setups fit the criteria, it
   * is unspecified which of the matching set is returned!
   * @param dateTime date / time of setup (must not be empty!)
   * @param method setup method (if ST_UNKNOWN, this criterion is not checked)
   * @param operatorSign operator signature (if empty, this criterion is not
   * checked)
   * @return the found setup error or NULL if not found
   */
  SetupError::Pointer FindSetupError(std::string dateTime,
      SetupError::SetupType method = SetupError::ST_UNKNOWN,
      std::string operatorSign = "");

  /**
   * Find a setup error that best matches the specified criteria. It might be
   * common to specify a certain date/time entry and find a setup error that
   * is closest to that date/time.
   * @param dateTime date/time to match (NOTE: can also be a date-only
   * specification)
   * @param mustBeSameDay restrict the setup errors to those which share the
   * same date-part (if TRUE)
   * @param planUID optional plan UID specification (considerered if not
   * empty)
   * @param method optional registration type specification (not considered if
   * ST_UNKNOWN)
   * @param operatorSign optional operator signature (not considered if emtpy)
   * @return the best matching setup error or NULL if there is no matching one
   */
  SetupError::Pointer FindBestMatchingSetupError(std::string dateTime,
      bool mustBeSameDay, std::string planUID = "",
      SetupError::SetupType method = SetupError::ST_UNKNOWN,
      std::string operatorSign = "");

protected:
  /** internal list of managed setup errors **/
  std::vector<SetupError::Pointer> m_SetupErrors;

  /**
   * class-wide static default setup error ('no error', rigid identity) which
   * can be retrieved without instantiation; SINGLETON
   **/
  static SetupError::Pointer m_DefaultSetupError;

  /** Default constructor. **/
  SetupErrorCollection();
  /** Default destructor. **/
  ~SetupErrorCollection();

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  /** purposely not implemented **/
  SetupErrorCollection(const Self&);
  /** purposely not implemented **/
  void operator=(const Self&);

};



}


#endif /* ORASETUPERROR_H_ */
