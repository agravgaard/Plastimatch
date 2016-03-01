

#include "oraITKVTKImageMetaInformation.h"

#include <iostream>

#include "oraITKVTKMetaInformationXMLFile.h"

// ORAIFTools
#include "oraStringTools.h"


namespace ora 
{


VolumeMetaInformation
::VolumeMetaInformation()
{
  m_MHDFileName = "";
  m_Source = "";
  m_SourceVersion = "";
  m_FullFileName = "";

  m_PatientPosition = "";
  m_PatientID = "";
  m_PatientName = "";
  m_PatientBirthDate = "";
  m_PatientSex = "";
  m_PatientPlan = "";
  m_PatientBeam = "";
  m_Direction.SetIdentity();
  m_Origin.Fill(0.);
  m_Spacing.Fill(0.);
  m_Size.Fill(0);
  m_AnatomicalOrientation.Fill("");
  m_FrameOfReferenceUID = "";
  m_DICOMStudyInstanceUID = "";
  m_DICOMStudyID = "";
  m_DICOMStudyDate = "";
  m_DICOMStudyTime = "";
  m_DICOMStudyDescription = "";
  m_DICOMSeriesInstanceUID = "";
  m_DICOMSeriesNumber = "";
  m_DICOMSeriesDescription = "";
  m_DICOMModality = "";
  m_DICOMSOPClassUID = "";
  m_DICOMDeviceSerial = "";
  m_DICOMManufacturer = "";
  m_DICOMCreatorUID = "";
  m_DICOMAccessionNumber = "";
  m_BitsAllocated = 0;
  m_BitsStored = 0;
  m_WLLevel = 0;
  m_WLWindow = -1;
  m_RescaleIntercept = 0;
  m_RescaleSlope = 1.;
  m_RescaleUnit = "";
  m_RescaleDescriptor = "";
  m_RescaleDigits = 0;
  m_Gamma = 0.;
  m_NumberOfComponents = 1;
  m_RTIPlane = "";
  m_RTIDescription = "";
  m_RTILabel = "";
  m_ORAPaletteID = "";
  m_ComplementaryStudyUID = "";
  m_ComplementarySeriesUID = "";
  m_XRayImageReceptorAngle = 0;
  m_XRayImageReceptorAngleValid = false;
  m_TableHeight = 0;
  m_TableHeightValid = false;
  m_SourceFilmDistance = 0;
  m_SourceFilmDistanceValid = false;
  m_SourceAxisDistance = 0;
  m_SourceAxisDistanceValid = false;
  m_Couch = 0;
  m_CouchValid = false;
  m_Collimator = 0;
  m_CollimatorValid = false;
  m_Gantry = 0;
  m_GantryValid = false;
  m_Machine = "";
  m_Isocenter.Fill(0);
  m_IsocenterValid = false;

  m_Parent = NULL;
}

VolumeMetaInformation
::~VolumeMetaInformation()
{
  m_Parent = NULL;
}

void
VolumeMetaInformation
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "BASIC" << std::endl;
    os << indent << " MHD File Name = " << m_MHDFileName << std::endl;
    os << indent << " Source = " << m_Source << std::endl;
    os << indent << " Source Version = " << m_SourceVersion << std::endl;
    os << indent << " Full File Name = " << m_FullFileName << std::endl;
    os << indent << " Number of Intensity Components = " <<
      m_NumberOfComponents << std::endl;
    os << indent << " Direction = ";
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        os << m_Direction[i][j] << " ";
    os << std::endl;
    os << indent << " Origin = ";
    for (int i = 0; i < 3; ++i)
      os << m_Origin[i] << " ";
    os << std::endl;
    os << indent << " Spacing = ";
    for (int i = 0; i < 3; ++i)
      os << m_Spacing[i] << " ";
    os << std::endl;
    os << indent << " Size = ";
    for (int i = 0; i < 3; ++i)
      os << m_Size[i] << " ";
    os << std::endl;
    os << indent << " Anatomical Orientation = ";
    for (int i = 0; i < 3; ++i)
      os << m_AnatomicalOrientation[i] << " ";
    os << std::endl;
    os << indent << " Frame of Reference UID = " <<
      m_FrameOfReferenceUID << std::endl;
  os << indent << "/BASIC" << std::endl;

  os << indent << "IMAGE_INTENSITY" << std::endl;
    os << indent << " Level (W/L) = " << m_WLLevel << std::endl;
    os << indent << " Window (W/L) = " << m_WLWindow << std::endl;
    os << indent << " Rescale Slope = " << m_RescaleSlope << std::endl;
    os << indent << " Rescale Intercept = " << m_RescaleIntercept << std::endl;
    os << indent << " Rescale Unit = " << m_RescaleUnit << std::endl;
    os << indent << " Rescale Descriptor = " << m_RescaleDescriptor << std::endl;
    os << indent << " Rescale Digits = " << m_RescaleDigits << std::endl;
    os << indent << " Gamma Correction = " << m_Gamma << std::endl;
    os << indent << " ORA Palette ID = " << m_ORAPaletteID << std::endl;
  os << indent << "/IMAGE_INTENSITY" << std::endl;

  os << indent << "DICOM" << std::endl;
    os << indent << " Study Instance UID = " <<
      m_DICOMStudyInstanceUID << std::endl;
    os << indent << " Study ID = " << m_DICOMStudyID << std::endl;
    os << indent << " Study Date = " << m_DICOMStudyDate << std::endl;
    os << indent << " Study Time = " << m_DICOMStudyTime << std::endl;
    os << indent << " Study Description = " <<
      m_DICOMStudyDescription << std::endl;
    os << indent << " Series Instance UID = " <<
      m_DICOMSeriesInstanceUID << std::endl;
    os << indent << " Series Number = " << m_DICOMSeriesNumber << std::endl;
    os << indent << " Series Description = " <<
      m_DICOMSeriesDescription << std::endl;
    os << indent << " Modality = " << m_DICOMModality << std::endl;
    os << indent << " SOP Class UID = " << m_DICOMSOPClassUID << std::endl;
    os << indent << " Device Serial = " << m_DICOMDeviceSerial << std::endl;
    os << indent << " Manufacturer = " << m_DICOMManufacturer << std::endl;
    os << indent << " Creator UID = " << m_DICOMCreatorUID << std::endl;
    os << indent << " Accession Number = " <<
      m_DICOMAccessionNumber << std::endl;
    os << indent << " Bits Allocated = " << m_BitsAllocated << std::endl;
    os << indent << " Bits Stored = " << m_BitsStored << std::endl;
    os << indent << " Complementary Study Instance UID = " <<
      m_ComplementaryStudyUID << std::endl;
    os << indent << " Complementary Series Instance UID = " <<
      m_ComplementarySeriesUID << std::endl;
  os << indent << "/DICOM" << std::endl;

  os << indent << "PATIENT" << std::endl;
    os << indent << " Position = " << m_PatientPosition << std::endl;
    os << indent << " ID = " << m_PatientID << std::endl;
    os << indent << " Name = " << m_PatientName << std::endl;
    os << indent << " Date of Birth = " << m_PatientBirthDate << std::endl;
    os << indent << " Sex = " << m_PatientSex << std::endl;
  os << indent << "/PATIENT" << std::endl;

  os << indent << "RT" << std::endl;
    os << indent << " RT Image Plane = " << m_RTIPlane << std::endl;
    os << indent << " RT Image Description = " << m_RTIDescription << std::endl;
    os << indent << " RT Image Label = " << m_RTILabel << std::endl;
    if (m_XRayImageReceptorAngleValid)
      os << indent << " X-ray Image Receptor Angle = " <<
        m_XRayImageReceptorAngle << std::endl;
    else
      os << indent << " X-ray Image Receptor Angle = <INVALID>" << std::endl;
    if (m_TableHeightValid)
      os << indent << " Table Height = " << m_TableHeight << std::endl;
    else
      os << indent << " Table Height = <INVALID>" << std::endl;
    if (m_SourceFilmDistanceValid)
      os << indent << " Source to Film Distance = " << m_SourceFilmDistance <<
        std::endl;
    else
      os << indent << " Source to Film Distance = <INVALID>" << std::endl;
    if (m_SourceAxisDistanceValid)
      os << indent << " Source to Axis Distance = " << m_SourceAxisDistance <<
        std::endl;
    else
      os << indent << " Source to Axis Distance = <INVALID>" << std::endl;
    if (m_CouchValid)
      os << indent << " Couch (isocentric) = " << m_Couch << std::endl;
    else
      os << indent << " Couch (isocentric) = <INVALID>" << std::endl;
    if (m_CollimatorValid)
      os << indent << " Collimator = " << m_Collimator << std::endl;
    else
      os << indent << " Collimator = <INVALID>" << std::endl;
    if (m_GantryValid)
      os << indent << " Gantry = " << m_Gantry << std::endl;
    else
      os << indent << " Gantry = <INVALID>" << std::endl;
    os << indent << " RT Patient Plan = " << m_PatientPlan << std::endl;
    os << indent << " RT Patient Beam = " << m_PatientBeam << std::endl;
    os << indent << " Machine = " << m_Machine << std::endl;
    if (m_IsocenterValid)
    {
      os << indent << " Isocenter = ";
      for (int i = 0; i < 3; ++i)
        os << m_Isocenter[i] << " ";
      os << std::endl;
    }
    else
      os << indent << " Isocenter = <INVALID>" << std::endl;
  os << indent << "/RT" << std::endl;
}

void
VolumeMetaInformation
::Anonymize()
{
  this->SetPatientBirthDate("");
  this->SetPatientID("");
  this->SetPatientName("");
  this->SetDICOMStudyDescription("");
  this->SetDICOMSeriesDescription("");
  this->SetDICOMDeviceSerial("");
  this->SetDICOMManufacturer("");
  this->SetDICOMCreatorUID("");
  this->SetDICOMAccessionNumber("");
}

void
VolumeMetaInformation
::ComputeAnatomicalOrientationForPatientBasedSystem(char anatomicalX,
    char anatomicalNegX, char anatomicalY, char anatomicalNegY,
    char anatomicalZ, char anatomicalNegZ)
{
  double vector[3];
  for (int d = 0; d < 3; d++)
  {
    for (int i = 0; i < 3; i++)
      vector[i] = m_Direction[d][i];

    char orientation[4];
    char *optr = orientation;
    *optr = '\0';

    char orientationX = vector[0] < 0 ? anatomicalNegX : anatomicalX;
    char orientationY = vector[1] < 0 ? anatomicalNegY : anatomicalY;
    char orientationZ = vector[2] < 0 ? anatomicalNegZ : anatomicalZ;
    double absX = fabs(vector[0]);
    double absY = fabs(vector[1]);
    double absZ = fabs(vector[2]);

    for (int i = 0; i < 3; ++i)
    {
      if (absX > .0001 && absX > absY && absX > absZ)
      {
        *optr++ = orientationX;
        absX = 0;
      }
      else if (absY > .0001 && absY > absX && absY > absZ)
      {
        *optr++ = orientationY;
        absY = 0;
      }
      else if (absZ > .0001 && absZ > absX && absZ > absY)
      {
        *optr++ = orientationZ;
        absZ = 0;
      }
      else
        break;
      *optr = '\0';
    }

    this->m_AnatomicalOrientation[d] = std::string(orientation);
  }
}

VolumeMetaInformation::FORPointer
VolumeMetaInformation
::GetFrameOfReference()
{
  if (!this->m_Parent)
    return NULL;

  std::string forUID = TrimF(this->m_FrameOfReferenceUID);
  if (forUID.length() <= 0)
    return NULL;

  if (!m_Parent->GetFORColl())
    return NULL;

  return m_Parent->GetFORColl()->FindFOR(forUID); // can still return NULL!
}


SliceMetaInformation
::SliceMetaInformation()
{
  m_ComplementaryInstanceUID = "";
  m_SliceLocation = 0;
  m_SliceLocationValid = false;
  m_Origin.Fill(0);
  m_DICOMInstanceNumber = "";
  m_DICOMSOPInstanceUID = "";
  m_ORAAcquisitionDate = "";
  m_ORAAcquisitionTypeID = "";
  m_ImageFrames = 0;
  m_ImageFramesValid = false;
  m_ImageMeasurementTime = 0;
  m_ImageMeasurementTimeValid = false;
  m_ImageDoseRate = 0;
  m_ImageDoseRateValid = false;
  m_ImageNormDoseRate = 0;
  m_ImageNormDoseRateValid = false;
  m_ImageOutput = 0;
  m_ImageOutputValid = false;
  m_ImageNormOutput = 0;
  m_ImageNormOutputValid = false;
  m_ORAAcquisitionType = "";
}

SliceMetaInformation
::~SliceMetaInformation()
{

}

void
SliceMetaInformation
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "SLICE" << std::endl;
    if (m_SliceLocationValid)
      os << indent << " Slice Location = " << m_SliceLocation << std::endl;
    else
      os << indent << " Slice Location = <INVALID>" << std::endl;
    os << indent << " Origin = ";
    for (int i = 0; i < 3; ++i)
      os << m_Origin[i] << " ";
    os << std::endl;
    os << indent << " DICOM Instance Number = " <<
      m_DICOMInstanceNumber << std::endl;
    os << indent << " DICOM SOP Instance UID = " <<
      m_DICOMSOPInstanceUID << std::endl;
    os << indent << " Complementary SOP Instance UID = " <<
      m_ComplementaryInstanceUID << std::endl;
    os << indent << " ORA Acquisition Date = " <<
      m_ORAAcquisitionDate << std::endl;
    os << indent << " ORA Acquisition Type ID = " <<
      m_ORAAcquisitionTypeID << std::endl;
    os << indent << " ORA Acquisition Type = " <<
      m_ORAAcquisitionType << std::endl;
    if (m_ImageFramesValid)
      os << indent << " Image Frames = " << m_ImageFrames << std::endl;
    else
      os << indent << " Image Frames = <INVALID>" << std::endl;
    if (m_ImageMeasurementTimeValid)
      os << indent << " Image Measurement Time = " <<
        m_ImageMeasurementTime << std::endl;
    else
      os << indent << " Image Measurement Time = <INVALID>" << std::endl;
    if (m_ImageDoseRateValid)
      os << indent << " Image Dose Rate = " << m_ImageDoseRate << std::endl;
    else
      os << indent << " Image Dose Rate = <INVALID>" << std::endl;
    if (m_ImageNormDoseRateValid)
      os << indent << " Image Norm Dose Rate = " <<
        m_ImageNormDoseRateValid << std::endl;
    else
      os << indent << " Image Norm Dose Rate = <INVALID>" << std::endl;
    if (m_ImageOutputValid)
      os << indent << " Image Output = " << m_ImageOutput << std::endl;
    else
      os << indent << " Image Output = <INVALID>" << std::endl;
    if (m_ImageNormOutputValid)
      os << indent << " Image Norm Output = " << m_ImageNormOutput << std::endl;
    else
      os << indent << " Image Norm Output = <INVALID>" << std::endl;
  os << indent << "/SLICE" << std::endl;
}

void
SliceMetaInformation
::Anonymize()
{
  // NOTE: nothing to do at the moment - all anonymized
}


// current hottest file version:
// - CHANGE LOG:
// BEGIN - 2009-11-16: version 1.0 (first format)
//    INDICATION: volumes had wrong direction due to a wrong sort order by ORA
// 2009-11-16 - 2009-11-22: version 1.1 (version 1.1 marks volumes with fixed
//    direction)
//    INDICATION: volumes had weird image origins resulting from DICOM
//    conversion and unusual voxel handling by ORA
// 2009-11-22 - : version 1.2 (version 1.2 marks volumes with additional patient
//    sex, source and source version fields; the source and source version fields
//    are for correcting the weird image origins specifically)
// 2009-12-01 - : version 1.3 (version 1.3 marks volumes with the correct number
//    of meta slice information items in the ORA XML file - before, the slice
//    information was doubly stored due to image series reader / image IO)
const std::string ITKVTKImageMetaInformation::HOTTEST_FILE_VERSION = "1.3";


bool
ITKVTKImageMetaInformation
::CheckFileForHottestFileVersion(const char *fileName, std::string &version)
{
  version = ""; // init
  if (itksys::SystemTools::FileExists(fileName, true))
  {
    // open file
    std::ifstream inputstream;

    // (read in binary mode to prevent CR/LF-translation)
    inputstream.open(fileName, std::ios::binary | std::ios::in);
    if(inputstream.fail())
    {
      SimpleDebugger deb;
      SimpleErrorMacro2Object(deb, << "Cannot open " << fileName << ".");
      return false;
    }
    // read the first 200 bytes (at maximum)
    std::streamsize filesize = itksys::SystemTools::FileLength(fileName);
    if (filesize > 200)
      filesize = 200;
    char *buffer = new char [filesize + 1];
    inputstream.read(buffer, filesize);
    if(static_cast<std::streamsize>(inputstream.gcount()) != filesize)
    {
      SimpleDebugger deb;
      SimpleErrorMacro2Object(deb, << "File " << fileName << " could not " <<
        "be loaded (event not first bytes for version check).");
      return false;
    }

    std::string s;
    s.assign(buffer, filesize);
    std::string::size_type p = s.find("<ORA_IMAGE_META_INFO version=\"");
    if (p != std::string::npos)
    {
      s = s.substr(p + 30);
      std::string::size_type p2 = s.find("\"");
      if (p2 != std::string::npos)
      {
        version = s.substr(0, p2);
        if (version == HOTTEST_FILE_VERSION) // clear match
        {
          return true;
        }
        else // maybe compatible though ...
        {
          // decompose into major and minor
          std::size_t p = version.find(".");
          if (p != std::string::npos)
          {
            std::string major = version.substr(0, p);
            ora::Trim(major);
            std::string minor = version.substr(p + 1);
            ora::Trim(minor);

            // COMPATIBILITY EXCEPTIONS:

            // NOTE: 1.3 is fully backward-compatible to 1.2 as the meta info
            // reader is capable of annihilating the doubly-slice-info bug:
            if (major == "1" && minor == "2" && HOTTEST_FILE_VERSION == "1.3")
              return true;

            return false;
          }
          else
            return false;
        }
      }
      else
        return false;
    }
    else
      return false;

    inputstream.close();
  }
  else
    return false;
}

ITKVTKImageMetaInformation
::ITKVTKImageMetaInformation()
  : SimpleDebugger()
{
  m_VolumeMetaInfo = VolumeMetaInformation::New();
  m_VolumeMetaInfo->SetParent(this);
  m_SlicesMetaInfo = new std::vector<SliceMetaInformation::Pointer>();
  m_FORColl = FORCollType::New();
  m_ImageList = ImgLstType::New();

  m_FileVersion = HOTTEST_FILE_VERSION;
}

ITKVTKImageMetaInformation
::~ITKVTKImageMetaInformation()
{
  m_VolumeMetaInfo = NULL;
  for (unsigned int i = 0; i < m_SlicesMetaInfo->size(); ++i)
    (*m_SlicesMetaInfo)[i] = NULL;
  delete m_SlicesMetaInfo;
  m_FORColl = NULL;
  m_ImageList = NULL;
}

void
ITKVTKImageMetaInformation
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Frame of Reference Collection:" << std::endl;
  if (m_FORColl)
    m_FORColl->Print(os, indent.GetNextIndent());
  else
      os << indent << "NULL" << std::endl;

  os << indent << "Image List:" << std::endl;
  if (m_ImageList)
    m_ImageList->Print(os, indent.GetNextIndent());
  else
    os << indent << "NULL" << std::endl;

  os << indent << "Volume Meta Information:" << std::endl;
  if (m_VolumeMetaInfo)
    m_VolumeMetaInfo->Print(os, indent.GetNextIndent());
  else
    os << indent << "NULL" << std::endl;

  os << indent << "Slices Meta Information(n=" << m_SlicesMetaInfo->size() <<
    "):" << std::endl;
  if (m_SlicesMetaInfo)
  {
    for (unsigned int i = 0; i < m_SlicesMetaInfo->size(); ++i)
      (*m_SlicesMetaInfo)[i]->Print(os, indent.GetNextIndent());
  }
  else
    os << indent << "NULL" << std::endl;
}

bool
ITKVTKImageMetaInformation
::WriteToXMLFile(const std::string fileName)
{
  ITKVTKMetaInformationXMLFileWriter::Pointer mixw =
    ITKVTKMetaInformationXMLFileWriter::New();

  mixw->SetFilename(fileName);
  mixw->SetObject(this);

  return mixw->WriteFile();
}

void
ITKVTKImageMetaInformation
::Anonymize()
{
  if (m_VolumeMetaInfo)
    m_VolumeMetaInfo->Anonymize();
  if (m_SlicesMetaInfo)
  {
    for (unsigned int i = 0; i < m_SlicesMetaInfo->size(); ++i)
      (*m_SlicesMetaInfo)[i]->Anonymize();
  }
}


}

