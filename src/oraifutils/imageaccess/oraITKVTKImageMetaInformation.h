
#ifndef ORAITKVTKIMAGEMETAINFORMATION_H_
#define ORAITKVTKIMAGEMETAINFORMATION_H_


#include <vector>
#include <string>

#include "oraFrameOfReference.h"
#include "oraImageList.h"
#include "oraComplementaryMetaFileCache.h"

// ORAIFTools
#include "SimpleDebugger.h"
#include "oraSimpleMacros.h"

#include <itkObject.h>
#include <itkMatrix.h>
#include <itkPoint.h>
#include <itkVector.h>
#include <itkFixedArray.h>


/** Setter/getter for complex validity-flag dependent floating point members. **/
#define ComplexFloatValiditySetterGetter(member) \
SimpleSetter(member, double); \
SimpleGetter(member, double); \
SimpleSetter(member##Valid, bool); \
SimpleGetter(member##Valid, bool); \
/** \
 * Set float member by string. \
 * @see m_##member for a more detailed description \
 */ \
virtual void Set##member##ByString(std::string _arg_##member) \
{ \
  if (_arg_##member.length() > 0) \
  { \
    m_##member = atof(_arg_##member.c_str()); \
    m_##member##Valid = true; \
  } \
  else \
    m_##member##Valid = false; \
} \
/** \
 * Set float member by string with additional slope. \
 * @see m_##member for a more detailed description \
 */ \
virtual void Set##member##ByStringWithSlope(std::string _arg_##member, \
    double slope) \
{ \
  if (_arg_##member.length() > 0) \
  { \
    m_##member = atof(_arg_##member.c_str()) * slope; \
    m_##member##Valid = true; \
  } \
  else \
    m_##member##Valid = false; \
}

/** Setter/getter for complex validity-flag dependent integer members. **/
#define ComplexIntegerValiditySetterGetter(member) \
SimpleSetter(member, int); \
SimpleGetter(member, int); \
SimpleSetter(member##Valid, bool); \
SimpleGetter(member##Valid, bool); \
/** \
 * Set integer member by string. \
 * @see m_##member for a more detailed description \
 */ \
virtual void Set##member##ByString(std::string _arg_##member) \
{ \
  if (_arg_##member.length() > 0) \
  { \
    m_##member = atoi(_arg_##member.c_str()); \
    m_##member##Valid = true; \
  } \
  else \
    m_##member##Valid = false; \
}


namespace ora 
{


// forward declaration (see below)
class ITKVTKImageMetaInformation;


/**
 * Helper class which comprises the typical meta-information of an open radART
 * volume image.
 * @author phil 
 * @version 1.0
 */
class VolumeMetaInformation
  : public itk::Object
{
public:
  /** standard typedefs **/
  typedef VolumeMetaInformation Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  typedef itk::Matrix<double, 3, 3> DirectionType;
  typedef itk::Point<double, 3> PointType;
  typedef itk::Vector<double, 3> VectorType;
  typedef itk::FixedArray<double, 3> SpacingType;
  typedef itk::FixedArray<unsigned int, 3> SizeType;
  typedef itk::FixedArray<std::string, 3> StringArrayType;
  typedef FrameOfReference FORType;
  typedef FORType::Pointer FORPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(VolumeMetaInformation, itk::Object);

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Computes the anatomical orientation property assuming that we have
   * a patient coordinate system (i.e. DICOM - LPS-system). The anatomical
   * characters can be: 'L' (left), 'R' (right), 'F' (feet), 'H' (head),
   * 'A' (anterior), 'P' (posterior)
   * @param anatomicalX anatomical character for x-direction
   * @param anatomicalNegX anatomical character for negative x-direction
   * @param anatomicalY anatomical character for y-direction
   * @param anatomicalNegY anatomical character for negative y-direction
   * @param anatomicalZ anatomical character for z-direction
   * @param anatomicalNegZ anatomical character for negative z-direction
   */
  void ComputeAnatomicalOrientationForPatientBasedSystem(char anatomicalX,
    char anatomicalNegX, char anatomicalY, char anatomicalNegY,
    char anatomicalZ, char anatomicalNegZ);

  // getters / setters for the class attributes

  SimpleSetter(MHDFileName, std::string);
  SimpleGetter(MHDFileName, std::string);

  SimpleSetter(Source, std::string);
  SimpleGetter(Source, std::string);

  SimpleSetter(SourceVersion, std::string);
  SimpleGetter(SourceVersion, std::string);

  SimpleSetter(FullFileName, std::string); /* not persistent ! */
  SimpleGetter(FullFileName, std::string);

  SimpleSetter(PatientPosition, std::string);
  SimpleGetter(PatientPosition, std::string);

  SimpleSetter(PatientID, std::string);
  SimpleGetter(PatientID, std::string);

  SimpleSetter(PatientName, std::string);
  SimpleGetter(PatientName, std::string);

  SimpleSetter(PatientBirthDate, std::string);
  SimpleGetter(PatientBirthDate, std::string);

  SimpleSetter(PatientSex, std::string);
  SimpleGetter(PatientSex, std::string);

  SimpleSetter(PatientPlan, std::string);
  SimpleGetter(PatientPlan, std::string);

  SimpleSetter(PatientBeam, std::string);
  SimpleGetter(PatientBeam, std::string);

  SimpleSetter(Direction, DirectionType);
  SimpleGetter(Direction, DirectionType);

  SimpleSetter(Origin, PointType);
  SimpleGetter(Origin, PointType);

  SimpleSetter(Spacing, SpacingType);
  SimpleGetter(Spacing, SpacingType);

  SimpleSetter(Size, SizeType);
  SimpleGetter(Size, SizeType);

  SimpleSetter(AnatomicalOrientation, StringArrayType);
  SimpleGetter(AnatomicalOrientation, StringArrayType);

  SimpleSetter(FrameOfReferenceUID, std::string);
  SimpleGetter(FrameOfReferenceUID, std::string);

  /**
   * Get the referenced frame of reference of the volume (return NULL if
   * there is no referenced FOR UID or the FOR UID could not be found in the
   * global FOR collection).
   */
  virtual FORPointer GetFrameOfReference();

  SimpleSetter(DICOMStudyInstanceUID, std::string);
  SimpleGetter(DICOMStudyInstanceUID, std::string);

  SimpleSetter(DICOMStudyID, std::string);
  SimpleGetter(DICOMStudyID, std::string);

  SimpleSetter(DICOMStudyDate, std::string);
  SimpleGetter(DICOMStudyDate, std::string);

  SimpleSetter(DICOMStudyTime, std::string);
  SimpleGetter(DICOMStudyTime, std::string);

  SimpleSetter(DICOMStudyDescription, std::string);
  SimpleGetter(DICOMStudyDescription, std::string);

  SimpleSetter(DICOMSeriesInstanceUID, std::string);
  SimpleGetter(DICOMSeriesInstanceUID, std::string);

  SimpleSetter(DICOMSeriesNumber, std::string);
  SimpleGetter(DICOMSeriesNumber, std::string);

  SimpleSetter(DICOMSeriesDescription, std::string);
  SimpleGetter(DICOMSeriesDescription, std::string);

  SimpleSetter(DICOMModality, std::string);
  SimpleGetter(DICOMModality, std::string);

  SimpleSetter(DICOMSOPClassUID, std::string);
  SimpleGetter(DICOMSOPClassUID, std::string);

  SimpleSetter(DICOMDeviceSerial, std::string);
  SimpleGetter(DICOMDeviceSerial, std::string);

  SimpleSetter(DICOMManufacturer, std::string);
  SimpleGetter(DICOMManufacturer, std::string);

  SimpleSetter(DICOMCreatorUID, std::string);
  SimpleGetter(DICOMCreatorUID, std::string);

  SimpleSetter(DICOMAccessionNumber, std::string);
  SimpleGetter(DICOMAccessionNumber, std::string);

  SimpleSetter(BitsAllocated, unsigned short);
  SimpleGetter(BitsAllocated, unsigned short);

  SimpleSetter(BitsStored, unsigned short);
  SimpleGetter(BitsStored, unsigned short);

  SimpleSetter(WLLevel, double);
  SimpleGetter(WLLevel, double);

  SimpleSetter(WLWindow, double);
  SimpleGetter(WLWindow, double);

  SimpleSetter(RescaleIntercept, double);
  SimpleGetter(RescaleIntercept, double);

  SimpleSetter(RescaleSlope, double);
  SimpleGetter(RescaleSlope, double);

  SimpleSetter(RescaleUnit, std::string);
  SimpleGetter(RescaleUnit, std::string);

  SimpleSetter(RescaleDescriptor, std::string);
  SimpleGetter(RescaleDescriptor, std::string);

  SimpleSetter(RescaleDigits, unsigned char);
  SimpleGetter(RescaleDigits, unsigned char);

  SimpleSetter(Gamma, double);
  SimpleGetter(Gamma, double);

  SimpleSetter(NumberOfComponents, unsigned int);
  SimpleGetter(NumberOfComponents, unsigned int);

  SimpleSetter(RTIPlane, std::string);
  SimpleGetter(RTIPlane, std::string);

  SimpleSetter(RTIDescription, std::string);
  SimpleGetter(RTIDescription, std::string);

  SimpleSetter(RTILabel, std::string);
  SimpleGetter(RTILabel, std::string);

  SimpleSetter(ORAPaletteID, std::string);
  SimpleGetter(ORAPaletteID, std::string);

  SimpleSetter(ComplementaryStudyUID, std::string);
  SimpleGetter(ComplementaryStudyUID, std::string);

  SimpleSetter(ComplementarySeriesUID, std::string);
  SimpleGetter(ComplementarySeriesUID, std::string);

  ComplexFloatValiditySetterGetter(XRayImageReceptorAngle);

  ComplexFloatValiditySetterGetter(TableHeight);

  ComplexFloatValiditySetterGetter(SourceFilmDistance);

  ComplexFloatValiditySetterGetter(SourceAxisDistance);

  ComplexFloatValiditySetterGetter(Couch);

  ComplexFloatValiditySetterGetter(Collimator);

  ComplexFloatValiditySetterGetter(Gantry);

  SimpleSetter(Machine, std::string);
  SimpleGetter(Machine, std::string);

  SimpleSetter(Isocenter, PointType);
  SimpleGetter(Isocenter, PointType);
  SimpleSetter(IsocenterValid, bool);
  SimpleGetter(IsocenterValid, bool);
  void SetIsocenterByStringWithSlope(std::string isoX, std::string isoY,
      std::string isoZ, double slope)
  {
    if (isoX.length() > 0 && isoY.length() > 0 && isoZ.length() > 0)
    {
      ora::VolumeMetaInformation::PointType iso;
      iso[0] = atof(isoX.c_str()) * slope;
      iso[1] = atof(isoY.c_str()) * slope;
      iso[2] = atof(isoZ.c_str()) * slope;
      SetIsocenter(iso);
      SetIsocenterValid(true);
    }
    else
      SetIsocenterValid(false);
  }

  SimpleSetter(Parent, ITKVTKImageMetaInformation*);
  SimpleGetter(Parent, ITKVTKImageMetaInformation*);

  /** Utility method for anonymizing this information object. **/
  void Anonymize();

protected:
  /** Default constructor **/
  VolumeMetaInformation();
  /** Default destructor **/
  virtual ~VolumeMetaInformation();

  /**
   * global DICOM conformant patient position identifier; one of the values:<br>
   * HFS ... head first supine <br>
   * HFP ... head first prone <br>
   * HFDR ... head first decubitus right <br>
   * FFDR ... feet first decubiturs right <br>
   * FFP ... feet first prone <br>
   * HFDL ... head first decubitus left <br>
   * FFDL ... feet first decubiturs left <br>
   * FFS ... feet first supine <br>
   **/
  std::string m_PatientPosition;
  /** patient identifier (type of ID depends on original data source) **/
  std::string m_PatientID;
  /** patient name **/
  std::string m_PatientName;
  /** patient's date of birth **/
  std::string m_PatientBirthDate;
  /** patient's sex **/
  std::string m_PatientSex;
  /** patient's plan (image refers to) **/
  std::string m_PatientPlan;
  /** patient's beam (image refers to) **/
  std::string m_PatientBeam;
  /**
   * volume's direction cosines (matrix-representation): <br>
   * row vector <br>
   * column vector <br>
   * slicing vector <br>
   * ... these three vectors MUST span a right-handed coordinate system
   **/
  DirectionType m_Direction;
  /**
   * volume's origin (point representation): <br>
   * coordinate along row direction <br>
   * coordinate along column direction <br>
   * coordinate along slicing direction <br>
   * ... this positions defines the CENTER of the first volume voxel
   */
  PointType m_Origin;
  /**
   * volume's voxel spacing: <br>
   * spacing along row direction <br>
   * spacing along column direction <br>
   * spacing along slicing direction
   */
  SpacingType m_Spacing;
  /*
   * volume's size (dimension) in each direction: <br>
   * size (pixels) in row direction <br>
   * size (pixels) in column direction <br>
   * size (pixels) in slicing direction <br>
   */
  SizeType m_Size;
  /**
   * volume's anatomical direction along its main directions: <br>
   * L ... patient left side <br>
   * R ... patient right side <br>
   * F ... patient feet direction (inferior) <br>
   * H ... patient head direction (superior) <br>
   * A ... anterior <br>
   * P ... posterior <br>
   * ... follows the DICOM Patient Orientation convention (including the
   * refinement C.7.6.1.1.1)
   */
  StringArrayType m_AnatomicalOrientation;
  /** (DICOM / open radART) frame of reference UID **/
  std::string m_FrameOfReferenceUID;
  /** DICOM: study instance UID **/
  std::string m_DICOMStudyInstanceUID;
  /** DICOM: study ID **/
  std::string m_DICOMStudyID;
  /** DICOM: study date **/
  std::string m_DICOMStudyDate;
  /** DICOM: study time **/
  std::string m_DICOMStudyTime;
  /** DICOM: study description **/
  std::string m_DICOMStudyDescription;
  /** DICOM: series instance UID **/
  std::string m_DICOMSeriesInstanceUID;
  /** DICOM: series number **/
  std::string m_DICOMSeriesNumber;
  /** DICOM: series description **/
  std::string m_DICOMSeriesDescription;
  /** DICOM: modality **/
  std::string m_DICOMModality;
  /** DICOM: SOP class UID **/
  std::string m_DICOMSOPClassUID;
  /** DICOM: device serial **/
  std::string m_DICOMDeviceSerial;
  /** DICOM: manufacturer **/
  std::string m_DICOMManufacturer;
  /** DICOM: creator UID **/
  std::string m_DICOMCreatorUID;
  /** DICOM: accession number **/
  std::string m_DICOMAccessionNumber;

  /** original number of bits allocated for intensities **/
  unsigned short m_BitsAllocated;
  /** original number of bits stored for intensities **/
  unsigned short m_BitsStored;
  /** preferred level (windowing) **/
  double m_WLLevel;
  /** preferred window (windowing) **/
  double m_WLWindow;
  /** rescale intercept **/
  double m_RescaleIntercept;
  /** rescale slope **/
  double m_RescaleSlope;
  /** rescale unit **/
  std::string m_RescaleUnit;
  /** rescale descriptor **/
  std::string m_RescaleDescriptor;
  /** number of rescale digits **/
  unsigned char m_RescaleDigits;
  /** preferred ora gamma-correction preset **/
  double m_Gamma;
  /** number of pixel intensity components **/
  unsigned int m_NumberOfComponents;

  /** RT Image Plane pose relative to beam axis (NORMAL or NON_NORMAL) **/
  std::string m_RTIPlane;
  /** RT Image Descriptor **/
  std::string m_RTIDescription;
  /** RT Image Label **/
  std::string m_RTILabel;
  /** ora-ID for palette to be applied (intensity lookup table) **/
  std::string m_ORAPaletteID;
  /** complementary study UID to relate this volume to **/
  std::string m_ComplementaryStudyUID;
  /** complementary series UID to relate this volume to **/
  std::string m_ComplementarySeriesUID;
  /** X-ray Image receptor angle in degrees **/
  double m_XRayImageReceptorAngle;
  /** valid-flag for X-ray Image receptor angle **/
  bool m_XRayImageReceptorAngleValid;
  /** table height in millimeters **/
  double m_TableHeight;
  /** valid-flag for table height **/
  bool m_TableHeightValid;
  /** source to film distance in millimeters **/
  double m_SourceFilmDistance;
  /** valid-flag for source to film distance **/
  bool m_SourceFilmDistanceValid;
  /** source to axis distance in millimeters **/
  double m_SourceAxisDistance;
  /** valid-flag for source to axis distance **/
  bool m_SourceAxisDistanceValid;
  /** (isocentric) couch rotation in degrees **/
  double m_Couch;
  /** valid-flag for (isocentric) couch rotation **/
  bool m_CouchValid;
  /** collimator rotation in degrees **/
  double m_Collimator;
  /** valid-flag for collimator rotation **/
  bool m_CollimatorValid;
  /** gantry rotation in degrees **/
  double m_Gantry;
  /** valid-flag for gantry rotation **/
  bool m_GantryValid;
  /** LinAc machine identification **/
  std::string m_Machine;
  /** LinAc isocenter position in WCS **/
  PointType m_Isocenter;
  /** valid-flag for LinAc isocenter position **/
  bool m_IsocenterValid;

  /** name of the related metaimage header file (relative path) **/
  std::string m_MHDFileName;
  /**
   * name of the volume source (a string expression); e.g. "ORA-RTI"
   * (when originally created from a set of RTI-images)
   **/
  std::string m_Source;
  /** additional version string for source **/
  std::string m_SourceVersion;

  /**
   * full file name of this image (this prop is NOT persistent and NOT stored
   * in extended ORA meta information; it is just a temporary information
   * in memory for saving tasks and so on)
   **/
  std::string m_FullFileName;

  /** a reference to the parent meta information object **/
  ITKVTKImageMetaInformation *m_Parent;

private:
  /** purposely not implemented **/
  VolumeMetaInformation(const Self&);

  /** purposely not implemented **/
  void operator=(const Self&);

};


/**
 * Helper class which comprises the typical meta-information of a slice of
 * an open radART volume image.
 * @author phil 
 * @version 1.0
 */
class SliceMetaInformation
  : public itk::Object
{
public:
  /** standard typedefs **/
  typedef SliceMetaInformation Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::Point<double, 3> PointType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(SliceMetaInformation, itk::Object);

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  SimpleSetter(ComplementaryInstanceUID, std::string);
  SimpleGetter(ComplementaryInstanceUID, std::string);

  ComplexFloatValiditySetterGetter(SliceLocation);

  SimpleSetter(Origin, PointType);
  SimpleGetter(Origin, PointType);

  SimpleSetter(DICOMInstanceNumber, std::string);
  SimpleGetter(DICOMInstanceNumber, std::string);

  SimpleSetter(DICOMSOPInstanceUID, std::string);
  SimpleGetter(DICOMSOPInstanceUID, std::string);

  SimpleSetter(ORAAcquisitionDate, std::string);
  SimpleGetter(ORAAcquisitionDate, std::string);

  SimpleSetter(ORAAcquisitionTypeID, std::string);
  SimpleGetter(ORAAcquisitionTypeID, std::string);

  ComplexIntegerValiditySetterGetter(ImageFrames);

  ComplexFloatValiditySetterGetter(ImageMeasurementTime);

  ComplexFloatValiditySetterGetter(ImageDoseRate);

  ComplexFloatValiditySetterGetter(ImageNormDoseRate);

  ComplexFloatValiditySetterGetter(ImageOutput);

  ComplexFloatValiditySetterGetter(ImageNormOutput);

  SimpleSetter(ORAAcquisitionType, std::string);
  SimpleGetter(ORAAcquisitionType, std::string);

  /** Utility method for anonymizing this information object. **/
  void Anonymize();

protected:
  /** complementary SOP instance UID to relate this slice to **/
  std::string m_ComplementaryInstanceUID;
  /** nominal slice location in mm **/
  double m_SliceLocation;
  /** valid-flag for slice location **/
  double m_SliceLocationValid;
  /**
   * slice's origin (point representation): <br>
   * coordinate along row direction <br>
   * coordinate along column direction <br>
   * coordinate along slicing direction <br>
   * ... this positions defines the CENTER of the first slice voxel
   */
  PointType m_Origin;
  /** DICOM instance number **/
  std::string m_DICOMInstanceNumber;
  /** DICOM SOP instance UID **/
  std::string m_DICOMSOPInstanceUID;
  /** open radART acquisition date **/
  std::string m_ORAAcquisitionDate;
  /** open radART acquisition type ID **/
  std::string m_ORAAcquisitionTypeID;
  /** number of image frames **/
  int m_ImageFrames;
  /** valid-flag for number of image frames **/
  bool m_ImageFramesValid;
  /** image measurement time in milliseconds **/
  double m_ImageMeasurementTime;
  /** valid-flag for image measurement time **/
  bool m_ImageMeasurementTimeValid;
  /** image dose rate **/
  double m_ImageDoseRate;
  /** valid-flag for image dose rate **/
  bool m_ImageDoseRateValid;
  /** norm image dose rate **/
  double m_ImageNormDoseRate;
  /** valid-flag for norm image dose rate **/
  bool m_ImageNormDoseRateValid;
  /** image output **/
  double m_ImageOutput;
  /** valid-flag for image output **/
  bool m_ImageOutputValid;
  /** norm image output **/
  double m_ImageNormOutput;
  /** valid-flag for norm image output **/
  bool m_ImageNormOutputValid;
  /** open radART acquisition type **/
  std::string m_ORAAcquisitionType;

  /** Default constructor **/
  SliceMetaInformation();
  /** Default destructor **/
  virtual ~SliceMetaInformation();

private:
  /** purposely not implemented **/
  SliceMetaInformation(const Self&);

  /** purposely not implemented **/
  void operator=(const Self&);

};


/**
 * This is a helper class comprising meta-information for a specified image in
 * open radART context. In general, this class relates to a 3D volume image
 * which originates from a set of 2D slices. Therefore, the image information is
 * split up into two categories of information: (a) general image meta-
 * information which relates to the complete volume, and optionally (b) a vector
 * of slice-specific meta-information originating from the source slices.<br>
 * The primary volume meta-information (a) is represented by the attributes of
 * this class. The slice-specific meta-information (b) is accessible via the
 * sub-item vector.
 * @author phil 
 * @version 1.1
 * @see ora::ITKVTKImage
 */
class ITKVTKImageMetaInformation
  : public itk::Object, public SimpleDebugger
{
public:
  /** standard typedefs **/
  typedef ITKVTKImageMetaInformation Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  /** important typedefs **/
  typedef FrameOfReferenceCollection FORCollType;
  typedef FORCollType::Pointer FORCollPointer;
  typedef ImageListEntry ImgLstEntryType;
  typedef ImgLstEntryType::Pointer ImgLstEntryPointer;
  typedef ImageList ImgLstType;
  typedef ImgLstType::Pointer ImgLstPointer;
  typedef ComplementaryMetaFileCache MetaCacheType;
  typedef MetaCacheType::Pointer MetaCachePointer;

  /**
   * Is the actual hottest file version. This version should be used when
   * generating a meta-information object with actual support.
   */
  static const std::string HOTTEST_FILE_VERSION;

  /**
   * This method reads a specified ORA XML (extended meta information format)
   * file and checks whether it has the hottest file version or not. <br>
   * NOTE: the file is opened and the version string is extracted - no other
   * compatibility checks are done! <br>
   * NOTE: this check is not only a pure version string check, it checks
   * specific versions and whether or not they are compatible.
   * @param fileName file name of the ORA XML file (extension *.ora.xml)
   * @param version returned version string of the file
   * @return TRUE if the inspected file has hottest version, FALSE otherwise
   * (use the returned version argument to analyze the discrepancy further);
   * if the file does not exist or is not an ORA XML file, FALSE will be
   * returned and version will be empty
   */
  static bool CheckFileForHottestFileVersion(const char *fileName,
      std::string &version);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ITKVTKImageMetaInformation, itk::Object);

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /** Get a pointer to the volume image's meta-information bundle. **/
  VolumeMetaInformation::Pointer GetVolumeMetaInfo()
  {
    return m_VolumeMetaInfo;
  }

  /** Get a pointer to the volume image's slices' meta-information bundles. **/
  std::vector<SliceMetaInformation::Pointer> *GetSlicesMetaInformation()
  {
    return m_SlicesMetaInfo;
  }

  /** Get the pointer to the internal Frame of Reference Collection. **/
  FORCollPointer GetFORColl()
  {
    return m_FORColl;
  }
  /** Set the internal Frame of Reference Collection. **/
  void SetFORColl(FORCollPointer forColl)
  {
    m_FORColl = NULL;
    m_FORColl = forColl;
  }

  /** Get the pointer to the internal Image List. **/
  ImgLstPointer GetImageList()
  {
    return m_ImageList;
  }
  /** Set the internal Image List. **/
  void SetImageList(ImgLstPointer imageList)
  {
    m_ImageList = imageList;
  }

  /** Set the internal image meta info cache. **/
  MetaCachePointer GetComplementaryMetaFileCache()
  {
    return m_MetaCache;
  }
  /** Set the internal image meta info cache. **/
  void SetComplementaryMetaFileCache(MetaCachePointer cache)
  {
    m_MetaCache = cache;
  }

  /**
   * Write the meta-information object contents to an XML file in the related
   * format. <br>
   * NOTE: existing files are completely overwritten!
   * @param fileName the file specification of the XML-file (usually the
   * name of the metaimage header file with an additional ".ora.xml"-extension)
   * @return TRUE if the file was successfully loaded from the file
   */
  bool WriteToXMLFile(const std::string fileName);

  /**
   * @return the number of pre-defined (original) image slices in the volume
   */
  unsigned int GetNumberOfPreDefinedVolumeSlices()
  {
    if (m_SlicesMetaInfo)
      return m_SlicesMetaInfo->size();
    else
      return 0;
  }

  // getters / setters for the class attributes

  SimpleSetter(FileVersion, std::string);
  SimpleGetter(FileVersion, std::string);

  /** Utility method for anonymizing this information object. **/
  void Anonymize();

protected:
  /** Volume's meta information **/
  VolumeMetaInformation::Pointer m_VolumeMetaInfo;
  /** Slices' meta information **/
  std::vector<SliceMetaInformation::Pointer> *m_SlicesMetaInfo;
  /** Relevant Frames of Reference **/
  FORCollPointer m_FORColl;
  /** List of relevant images **/
  ImgLstPointer m_ImageList;
  /** Cached image meta info files for the images **/
  MetaCachePointer m_MetaCache;
  /** File version **/
  std::string m_FileVersion;

  /** Default Constructor **/
  ITKVTKImageMetaInformation();
  /** Default Destructor **/
  virtual ~ITKVTKImageMetaInformation();

private:
  /** purposely not implemented **/
  ITKVTKImageMetaInformation(const Self&);

  /** purposely not implemented **/
  void operator=(const Self&);

};


}


#endif /* ORAITKVTKIMAGEMETAINFORMATION_H_ */
