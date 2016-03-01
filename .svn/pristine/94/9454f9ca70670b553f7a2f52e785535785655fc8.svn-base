

#include "oraITKVTKMetaInformationXMLFile.h"

#include <iostream>

#include <itkMetaDataObject.h>
#include <itkIOCommon.h>
#include <itksys/SystemTools.hxx>

// ORAIFTools
#include "oraStringTools.h"
#include "SimpleDebugger.h"

#ifdef USE_QT
#include <QCryptographicHash>
#include <qstring.h>
#else
// NOTE: Must use vtksys because itksys is compiled without MD5 support
#include <vtksys/MD5.h>
#endif


/** accessibility macro (volume) **/
#define WriteSimpleValidityDependentElement(member, _type, tag) \
if (vmi->Get##member##Valid()) \
  WriteSimpleElement<_type>(this, tag, \
    currIndent, output, vmi->Get##member()); \
else \
  WriteSimpleElement<std::string>(this, tag, currIndent, output, "");

/** accessibility macro (slice) **/
#define WriteSimpleValidityDependentElementSlice(member, _type, tag) \
if (smi->Get##member##Valid()) \
  WriteSimpleElement<_type>(this, tag, \
    levelIndent, output, smi->Get##member()); \
else \
  WriteSimpleElement<std::string>(this, tag, levelIndent, output, "");



/**
 * external XML utilities
 * @see itkXMLFile.cxx
 **/
extern "C"
{
  static void itkXMLParserStartElement(void* parser, const char *name,
                                       const char **atts)
  {
    // Begin element handler that is registered with the XML_Parser.
    // This just casts the user data to a itkXMLParser and calls
    // StartElement.
    static_cast<itk::XMLReaderBase*>(parser)->StartElement(name, atts);
  }

  static void itkXMLParserEndElement(void* parser, const char *name)
  {
    // End element handler that is registered with the XML_Parser.  This
    // just casts the user data to a itkXMLParser and calls EndElement.
    static_cast<itk::XMLReaderBase*>(parser)->EndElement(name);
  }

  static void itkXMLParserCharacterDataHandler(void* parser, const char* data,
                                               int length)
  {
    // Character data handler that is registered with the XML_Parser.
    // This just casts the user data to a itkXMLParser and calls
    // CharacterDataHandler.
    static_cast<itk::XMLReaderBase*>(parser)->CharacterDataHandler(data, length);
  }
}


namespace ora 
{


ITKVTKMetaInformationXMLFileReader
::ITKVTKMetaInformationXMLFileReader()
  : Superclass()
{
  m_Data.str("");
  SetOutputObject(NULL);
  m_IgnoreInvalidHash = false;
  m_TempOutput = NULL;
  m_TempSlice = NULL;
  m_OriginalReadFileVersion = "";
}

ITKVTKMetaInformationXMLFileReader
::~ITKVTKMetaInformationXMLFileReader()
{
  SetOutputObject(NULL);
  m_TempOutput = NULL;
  m_TempSlice = NULL;
}

int
ITKVTKMetaInformationXMLFileReader
::CanReadFile(const char* name)
{
  if (itksys::SystemTools::FileExists(name) &&
      itksys::SystemTools::FileLength(name) > 0)
    return true;
  else
    return false;
}

void
ITKVTKMetaInformationXMLFileReader
::StartElement(const char * name, const char **atts)
{
  if (strcmp(name, "ORA_IMAGE_META_INFO") == 0) // start new output object
  {
    bool versionAwaited = false;
    for (int i = 0; atts[i] != 0; i++)
    {
      if (atts[i] != 0)
      {
        if (versionAwaited)
        {
          m_OriginalReadFileVersion = std::string(atts[i]);
          break;
        }
        if (strcmp(atts[i], "version") == 0)
          versionAwaited = true;
      }
    }

    m_TempSlice = NULL;
    m_TempOutput = NULL;
    m_TempOutput = ITKVTKImageMetaInformation::New();
    SetOutputObject(NULL);
  }
  // VOLUME_META_INFO: Volume Info Object needs not to be manually created
  // SLICES_META_INFO: Slices Vector needs not to be manually created
  else if (strcmp(name, "SLICE") == 0 && m_TempOutput) // add new slice object
  {
    m_TempSlice = SliceMetaInformation::New(); // store ref for easier access
    m_TempOutput->GetSlicesMetaInformation()->push_back(m_TempSlice);
  }
}

void
ITKVTKMetaInformationXMLFileReader
::EndElement(const char *name)
{
  std::string data = TrimF(m_Data.str()); // forget about blanks

  // decode encoded characters (${nn}-patterns):
  std::size_t p = data.find("${");
  if (p != std::string::npos)
  {
    std::size_t q;
    std::string s;
    int n;
    while (p != std::string::npos)
    {
      q = data.find("}", p + 1);
      if (q != std::string::npos)
      {
        s = data.substr(p + 2, q - p - 2);
        if (s.length() > 0)
        {
          n = atoi(s.c_str());
          if (n >= 0 && n <= 255) // unsigned char
          {
            s = " ";
            s[0] = (char)n;
            data.erase(p, q - p + 1);
            data.insert(p, s);
          }
        }
      }
      p = data.find("${", p + 1); // next search
    }
  }

  if (strcmp(name, "ORA_IMAGE_META_INFO") == 0) // finalize object
  {
    SetOutputObject(m_TempOutput); // set as valid output
    m_TempSlice = NULL;
  }
  else if (m_TempOutput)
  {
    VolumeMetaInformation::Pointer vmi = m_TempOutput->GetVolumeMetaInfo();

    // <BASIC>
    if (strcmp(name, "MHD_File") == 0)
    {
      vmi->SetMHDFileName(data);
    }
    else if (strcmp(name, "Source") == 0)
    {
      vmi->SetSource(data);
    }
    else if (strcmp(name, "Source_Version") == 0)
    {
      vmi->SetSourceVersion(data);
    }
    else if (strcmp(name, "Number_Of_Intensity_Components") == 0)
    {
      vmi->SetNumberOfComponents((unsigned int)atoi(data.c_str()));
    }
    else if (strcmp(name, "Direction") == 0)
    {
      vmi->SetDirection(ArrayToDirectionType(data));
    }
    else if (strcmp(name, "Origin") == 0 && !m_TempSlice)
    {
      vmi->SetOrigin(ArrayToPointType(data));
    }
    else if (strcmp(name, "Spacing") == 0)
    {
      vmi->SetSpacing(ArrayToFixedArray(data));
    }
    else if (strcmp(name, "Size") == 0)
    {
      vmi->SetSize(ArrayToFixedArrayIntType(data));
    }
    else if (strcmp(name, "Anatomical_Orientation") == 0)
    {
      vmi->SetAnatomicalOrientation(ArrayToStringArrayType(data));
    }
    else if (strcmp(name, "Frame_Of_Reference_UID") == 0)
    {
      vmi->SetFrameOfReferenceUID(data);
    }
    // </BASIC>

    // <IMAGE_INTENSITY>
    else if (strcmp(name, "WL_Level") == 0)
    {
      vmi->SetWLLevel(atof(data.c_str()));
    }
    else if (strcmp(name, "WL_Window") == 0)
    {
      vmi->SetWLWindow(atof(data.c_str()));
    }
    else if (strcmp(name, "Rescale_Slope") == 0)
    {
      vmi->SetRescaleSlope(atof(data.c_str()));
    }
    else if (strcmp(name, "Rescale_Intercept") == 0)
    {
      vmi->SetRescaleIntercept(atof(data.c_str()));
    }
    else if (strcmp(name, "Rescale_Unit") == 0)
    {
      vmi->SetRescaleUnit(data);
    }
    else if (strcmp(name, "Rescale_Descriptor") == 0)
    {
      vmi->SetRescaleDescriptor(data);
    }
    else if (strcmp(name, "Rescale_Digits") == 0)
    {
      vmi->SetRescaleDigits((unsigned char)atoi(data.c_str()));
    }
    else if (strcmp(name, "Gamma") == 0)
    {
      vmi->SetGamma(atof(data.c_str()));
    }
    else if (strcmp(name, "ORA_Palette_ID") == 0)
    {
      vmi->SetORAPaletteID(data);
    }
    // </IMAGE_INTENSITY>

    // <DICOM>
    else if (strcmp(name, "Study_Instance_UID") == 0)
    {
      vmi->SetDICOMStudyInstanceUID(data);
    }
    else if (strcmp(name, "Study_ID") == 0)
    {
      vmi->SetDICOMStudyID(data);
    }
    else if (strcmp(name, "Study_Date") == 0)
    {
      vmi->SetDICOMStudyDate(data);
    }
    else if (strcmp(name, "Study_Time") == 0)
    {
      vmi->SetDICOMStudyTime(data);
    }
    else if (strcmp(name, "Study_Description") == 0)
    {
      vmi->SetDICOMStudyDescription(data);
    }
    else if (strcmp(name, "Series_Instance_UID") == 0)
    {
      vmi->SetDICOMSeriesInstanceUID(data);
    }
    else if (strcmp(name, "Series_Number") == 0)
    {
      vmi->SetDICOMSeriesNumber(data);
    }
    else if (strcmp(name, "Series_Description") == 0)
    {
      vmi->SetDICOMSeriesDescription(data);
    }
    else if (strcmp(name, "Modality") == 0)
    {
      vmi->SetDICOMModality(data);
    }
    else if (strcmp(name, "SOP_Class_UID") == 0)
    {
      vmi->SetDICOMSOPClassUID(data);
    }
    else if (strcmp(name, "Device_Serial") == 0)
    {
      vmi->SetDICOMDeviceSerial(data);
    }
    else if (strcmp(name, "Manufacturer") == 0)
    {
      vmi->SetDICOMManufacturer(data);
    }
    else if (strcmp(name, "Creator_UID") == 0)
    {
      vmi->SetDICOMCreatorUID(data);
    }
    else if (strcmp(name, "Accession_Number") == 0)
    {
      vmi->SetDICOMAccessionNumber(data);
    }
    else if (strcmp(name, "Bits_Allocated") == 0)
    {
      vmi->SetBitsAllocated((unsigned int)atoi(data.c_str()));
    }
    else if (strcmp(name, "Bits_Stored") == 0)
    {
      vmi->SetBitsStored((unsigned int)atoi(data.c_str()));
    }
    else if (strcmp(name, "Complementary_Study_UID") == 0)
    {
      vmi->SetComplementaryStudyUID(data);
    }
    else if (strcmp(name, "Complementary_Series_UID") == 0)
    {
      vmi->SetComplementarySeriesUID(data);
    }
    // </DICOM>

    // <PATIENT>
    else if (strcmp(name, "Patient_Position") == 0)
    {
      vmi->SetPatientPosition(data);
    }
    else if (strcmp(name, "Patient_ID") == 0)
    {
      vmi->SetPatientID(data);
    }
    else if (strcmp(name, "Patient_Name") == 0)
    {
      vmi->SetPatientName(data);
    }
    else if (strcmp(name, "Patient_BirthDate") == 0)
    {
      vmi->SetPatientBirthDate(data);
    }
    else if (strcmp(name, "Patient_Sex") == 0)
    {
      vmi->SetPatientSex(data);
    }
    // </PATIENT>

    // <RT>
    else if (strcmp(name, "RT_Image_Plane") == 0)
    {
      vmi->SetRTIPlane(data);
    }
    else if (strcmp(name, "RT_Image_Description") == 0)
    {
      vmi->SetRTIDescription(data);
    }
    else if (strcmp(name, "RT_Image_Label") == 0)
    {
      vmi->SetRTILabel(data);
    }
    else if (strcmp(name, "Xray_Image_Receptor_Angle") == 0)
    {
      vmi->SetXRayImageReceptorAngleByString(data);
    }
    else if (strcmp(name, "Table_Height") == 0)
    {
      vmi->SetTableHeightByString(data);
    }
    else if (strcmp(name, "Source_Film_Distance") == 0)
    {
      vmi->SetSourceFilmDistanceByString(data);
    }
    else if (strcmp(name, "Source_Axis_Distance") == 0)
    {
      vmi->SetSourceAxisDistanceByString(data);
    }
    else if (strcmp(name, "Couch_Isocentric") == 0)
    {
      vmi->SetCouchByString(data);
    }
    else if (strcmp(name, "Collimator") == 0)
    {
      vmi->SetCollimatorByString(data);
    }
    else if (strcmp(name, "Gantry") == 0)
    {
      vmi->SetGantryByString(data);
    }
    else if (strcmp(name, "RT_Patient_Plan") == 0)
    {
      vmi->SetPatientPlan(data);
    }
    else if (strcmp(name, "RT_Patient_Beam") == 0)
    {
      vmi->SetPatientBeam(data);
    }
    else if (strcmp(name, "Machine") == 0)
    {
      vmi->SetMachine(data);
    }
    else if (strcmp(name, "Isocenter") == 0)
    {
      std::vector<std::string> tok;
      TokenizeIncludingEmptySpaces(data, tok, ",");
      if (tok.size() >= 3)
        vmi->SetIsocenterByStringWithSlope(tok[0], tok[1], tok[2], 1.0);
      else
        vmi->SetIsocenterByStringWithSlope("", "", "", 1.0);
    }
    // </RT>

    else if (m_TempSlice)
    {
      if (strcmp(name, "SLICE") == 0)
      {
        m_TempSlice = NULL;
      }

      // <SLICE>
      else if (strcmp(name, "Slice_Location") == 0)
      {
        m_TempSlice->SetSliceLocationByString(data);
      }
      else if (strcmp(name, "Origin") == 0)
      {
        m_TempSlice->SetOrigin(ArrayToPointType(data));
      }
      else if (strcmp(name, "DICOM_Instance_Number") == 0)
      {
        m_TempSlice->SetDICOMInstanceNumber(data);
      }
      else if (strcmp(name, "DICOM_SOP_Instance_UID") == 0)
      {
        m_TempSlice->SetDICOMSOPInstanceUID(data);
      }
      else if (strcmp(name, "Complementary_Instance_UID") == 0)
      {
        m_TempSlice->SetComplementaryInstanceUID(data);
      }
      else if (strcmp(name, "ORA_Acquisition_Date") == 0)
      {
        m_TempSlice->SetORAAcquisitionDate(data);
      }
      else if (strcmp(name, "ORA_Acquisition_Type_ID") == 0)
      {
        m_TempSlice->SetORAAcquisitionTypeID(data);
      }
      else if (strcmp(name, "ORA_Acquisition_Type") == 0)
      {
        m_TempSlice->SetORAAcquisitionType(data);
      }
      else if (strcmp(name, "Image_Frames") == 0)
      {
        m_TempSlice->SetImageFramesByString(data);
      }
      else if (strcmp(name, "Image_Measurement_Time") == 0)
      {
        m_TempSlice->SetImageMeasurementTimeByString(data);
      }
      else if (strcmp(name, "Image_Dose_Rate") == 0)
      {
        m_TempSlice->SetImageDoseRateByString(data);
      }
      else if (strcmp(name, "Image_Norm_Dose_Rate") == 0)
      {
        m_TempSlice->SetImageNormDoseRateByString(data);
      }
      else if (strcmp(name, "Image_Output") == 0)
      {
        m_TempSlice->SetImageOutputByString(data);
      }
      else if (strcmp(name, "Image_Norm_Output") == 0)
      {
        m_TempSlice->SetImageNormOutputByString(data);
      }
      // </SLICE>
    }
  }
}

void
ITKVTKMetaInformationXMLFileReader
::CharacterDataHandler(const char *inData, int inLength)
{
  // simply store values
  m_Data.str(""); // set back
  for (int i = 0; i < inLength; i++)
    m_Data << inData[i];
}

void
ITKVTKMetaInformationXMLFileReader
::parse(void)
{
  XML_Parser Parser = XML_ParserCreate(0);

  // explicitly set ASCII-compatible UTF-8 encoding
  XML_SetEncoding(static_cast<XML_Parser>(Parser), "utf-8");

  XML_SetElementHandler(Parser, &itkXMLParserStartElement,
      &itkXMLParserEndElement);
  XML_SetCharacterDataHandler(Parser, &itkXMLParserCharacterDataHandler);
  XML_SetUserData(Parser,this);

  // open file
  std::ifstream inputstream;
  // (read in binary mode to prevent CR/LF-translation)
  inputstream.open(m_Filename.c_str(), std::ios::binary | std::ios::in);
  if(inputstream.fail())
  {
    SimpleDebugger deb;
    SimpleErrorMacro2Object(deb, << "Cannot open " << m_Filename << ".");
    return;
  }

  // default stream parser just reads a block at a time.
  std::streamsize filesize = itksys::SystemTools::
    FileLength(m_Filename.c_str());
  char *buffer = new char [filesize];
  inputstream.read(buffer, filesize);
  if(static_cast<std::streamsize>(inputstream.gcount()) != filesize)
  {
    SimpleDebugger deb;
    SimpleErrorMacro2Object(deb, << "File " << m_Filename << " could not " <<
        "completely be loaded.");
    return;
  }

  // check MD5 checksum first
  // - extract written MD5
  char *pos = strstr(buffer, "<MD5>");
  bool hashOK = false;
  if (pos)
  {
    char fileHashBuff[32];
    for (int i = 0; i < 32; i++)
      fileHashBuff[i] = *(pos + 5 + i);
    std::string fileHash;
    fileHash.assign(fileHashBuff, 32);
    // replace the MD5-hash with zero-hash
    for (int i = 0; i < 32; i++)
      *(pos + 5 + i) = '0';

    // compute MD5-hash
    std::string md5Hash = "";
    md5Hash = ITKVTKMetaInformationXMLFileWriter::ComputeHashMD5(buffer, filesize);

    // compare with original hash
    if (md5Hash == fileHash)
      hashOK = true;
    // set back original MD5-hash
    for (int i = 0; i < 32; i++)
      *(pos + 5 + i) = fileHashBuff[i];
  }
  if(!hashOK && !m_IgnoreInvalidHash)
  {
    SimpleDebugger deb;
    SimpleErrorMacro2Object(deb, << "The MD5-hash of " << m_Filename <<
        " is incorrect or missing. Cannot resume loading.");
    return;
  }
  else if (!hashOK && m_IgnoreInvalidHash) // warning
  {
    SimpleDebugger deb;
    DetailedDebugMacro2Object(deb, << "The MD5-hash of " << m_Filename <<
        " is incorrect, but it is ignored. Resume.");
  }

  // do the real parsing
  bool result = XML_Parse(Parser, buffer, inputstream.gcount(), false);
  delete[] buffer;
  if(!result)
  {
    SimpleDebugger deb;
    SimpleErrorMacro2Object(deb, << "XML-Parser-ERROR: " <<
        XML_ErrorString(XML_GetErrorCode(Parser)));
    return;
  }
  XML_ParserFree(Parser);


  // VERSION-SPECIFIC FIXES

  // 1.2: version 1.2 is known to contain doubly slices; let's remove them:
  if (m_OriginalReadFileVersion == "1.2")
  {
    std::vector<SliceMetaInformation::Pointer> *smi = this->GetOutputObject()->
        GetSlicesMetaInformation();
    // the slices are sequentially sorted, therefore we can simply remove half
    // of the slices (provided that the number of slices is even):
    unsigned int numSlices = smi->size();
    if ((numSlices % 2) == 0)
    {
      int x = numSlices / 2;
      for (int i = 1; i <= x; i++)
      {
        (*smi)[numSlices - i] = NULL;
        smi->pop_back();
      }
    }
    else
    {
      SimpleDebugger deb;
      InformativeDebugMacro2Object(deb, << "WARNING: doubly slices from " <<
          "version 1.2 could not be removed! File: " << m_Filename)
    }
  }

  return;
}

VolumeMetaInformation::DirectionType
ITKVTKMetaInformationXMLFileReader
::ArrayToDirectionType(std::string dir)
{
  std::vector<std::string> tok;
  ora::TokenizeIncludingEmptySpaces(dir, tok, ",");
  VolumeMetaInformation::DirectionType direction;
  if (tok.size() >= 9)
  {
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        direction[i][j] = atof(tok[i * 3 + j].c_str());
  }
  else
    direction.Fill(0);
  return direction;
}

VolumeMetaInformation::PointType
ITKVTKMetaInformationXMLFileReader
::ArrayToPointType(std::string point)
{
  std::vector<std::string> tok;
  ora::TokenizeIncludingEmptySpaces(point, tok, ",");
  VolumeMetaInformation::PointType pointt;
  if (tok.size() >= 3)
  {
    for (int i = 0; i < 3; i++)
      pointt[i] = atof(tok[i].c_str());
  }
  else
    pointt.Fill(0);
  return pointt;
}

VolumeMetaInformation::SpacingType
ITKVTKMetaInformationXMLFileReader
::ArrayToFixedArray(std::string farray)
{
  std::vector<std::string> tok;
  ora::TokenizeIncludingEmptySpaces(farray, tok, ",");
  VolumeMetaInformation::SpacingType fixeda;
  if (tok.size() >= 3)
  {
    for (int i = 0; i < 3; i++)
      fixeda[i] = atof(tok[i].c_str());
  }
  else
    fixeda.Fill(0);
  return fixeda;
}

VolumeMetaInformation::SizeType
ITKVTKMetaInformationXMLFileReader
::ArrayToFixedArrayIntType(std::string farray)
{
  std::vector<std::string> tok;
  ora::TokenizeIncludingEmptySpaces(farray, tok, ",");
  VolumeMetaInformation::SizeType fixeda;
  if (tok.size() >= 3)
  {
    for (int i = 0; i < 3; i++)
      fixeda[i] = atoi(tok[i].c_str());
  }
  else
    fixeda.Fill(0);
  return fixeda;
}

VolumeMetaInformation::StringArrayType
ITKVTKMetaInformationXMLFileReader
::ArrayToStringArrayType(std::string sarray)
{
  std::vector<std::string> tok;
  ora::TokenizeIncludingEmptySpaces(sarray, tok, "|");
  VolumeMetaInformation::StringArrayType sa;
  sa.Fill("");
  for (unsigned int i = 0; i < tok.size() && i < sa.Size(); i++)
    sa[i] = tok[i];
  return sa;
}

void
ITKVTKMetaInformationXMLFileReader
::Update()
{
  // parsing with MD5-check
  this->parse();
}




ITKVTKMetaInformationXMLFileWriter
::ITKVTKMetaInformationXMLFileWriter()
  : Superclass()
{

}

ITKVTKMetaInformationXMLFileWriter
::~ITKVTKMetaInformationXMLFileWriter()
{

}

int
ITKVTKMetaInformationXMLFileWriter
::CanWriteFile(const char *name)
{
  if (name && std::string(name).length() > 0)
    return true;

  return false;
}

// Helper-template procedure for simple XML-entries (start tag, write value,
// and end tag):
template <typename T>
void
WriteSimpleElement(ITKVTKMetaInformationXMLFileWriter *thiss,
  const char *tag, itk::Indent &indent, std::ostream &os, T value)
{
  // XML / HTML conformant conversion (encode chars with ASCII > 127):
  std::ostringstream tmp;
  tmp.str();
  tmp << value;
  std::string cvalue = tmp.str();
  std::string fvalue = "";
#ifdef USE_QT
  QString qvalue = "";
  QString s;
  for (unsigned i = 0; i < cvalue.length(); ++i)
  {
    if ((int)(unsigned char)cvalue[i] <= 0x7F) // standard ASCII
    {
      qvalue.append(QChar(cvalue[i]));
    }
    else // UTF-8 -> encode properly
    {
      s = "";
      s.setNum((int)(unsigned char)cvalue[i]);
      s = "${" + s + "}";
      qvalue.append(s);
    }
  }
  fvalue = qvalue.toStdString();
#else
  //FIXME: Encoding of UTF-8
  fvalue = cvalue;
#endif

  os << indent;
  thiss->WriteStartElement(tag, os);
  os << fvalue;
  thiss->WriteEndElement(tag, os);
  os << std::endl;
}

std::string
ITKVTKMetaInformationXMLFileWriter
::DirectionTypeToArray(VolumeMetaInformation::DirectionType dir)
{
  std::ostringstream result;
  result.str("");
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
    {
      if (i == 0 && j == 0)
        result << dir[0][0];
      else
        result << "," << dir[i][j];
    }
  return result.str();
}

std::string
ITKVTKMetaInformationXMLFileWriter
::PointTypeToArray(VolumeMetaInformation::PointType point)
{
  std::ostringstream result;
  result.str("");
  for (int i = 0; i < 3; ++i)
  {
    if (i == 0)
      result << point[0];
    else
      result << "," << point[i];
  }
  return result.str();
}

std::string
ITKVTKMetaInformationXMLFileWriter
::FixedArrayToArray(VolumeMetaInformation::SpacingType array)
{
  std::ostringstream result;
  result.str("");
  for (int i = 0; i < 3; ++i)
  {
    if (i == 0)
      result << array[0];
    else
      result << "," << array[i];
  }
  return result.str();
}

std::string
ITKVTKMetaInformationXMLFileWriter
::FixedArrayToArray(VolumeMetaInformation::SizeType array)
{
  std::ostringstream result;
  result.str("");
  for (int i = 0; i < 3; ++i)
  {
    if (i == 0)
      result << array[0];
    else
      result << "," << array[i];
  }
  return result.str();
}

std::string
ITKVTKMetaInformationXMLFileWriter
::StringArrayTypeToArray(VolumeMetaInformation::StringArrayType array)
{
  std::ostringstream result;
  result.str("");
  for (int i = 0; i < 3; ++i)
  {
    if (i == 0)
      result << array[0];
    else
      result << "|" << array[i];
  }
  return result.str();
}

void
ITKVTKMetaInformationXMLFileWriter
::WriteStartElement(const char *const tag, std::ostream &os)
{
  os << '<' << tag << '>';
}

void
ITKVTKMetaInformationXMLFileWriter
::WriteEndElement(const char *const tag, std::ostream &os)
{
  os << '<' << '/'  << tag << '>';
}

int
ITKVTKMetaInformationXMLFileWriter
::WriteFile()
{
  if(this->m_InputObject == 0)
  {
    SimpleDebugger deb;
    SimpleErrorMacro2Object(deb, << "No meta-information to write.")
  }
  if(this->m_Filename.length() == 0)
  {
    SimpleDebugger deb;
    SimpleErrorMacro2Object(deb, << "Invalid destination file name.")
  }

  std::ostringstream output; // write to simple string buffer first
  itk::Indent rootIndent;
  itk::Indent currIndent;

  WriteStartElement("?xml version=\"1.0\" standalone=\"yes\" ?", output);
  output << std::endl;
  WriteStartElement("!DOCTYPE ORAIMAGEMETAINFO", output);
  output << std::endl;

  // write out meta-information:

  std::string orastarttag = "ORA_IMAGE_META_INFO version=\"";
  orastarttag += m_InputObject->GetFileVersion() + "\" creator=\"oraif\"";
  WriteStartElement(orastarttag.c_str(), output); output << std::endl;

    currIndent = rootIndent.GetNextIndent(); output << currIndent;
    // prepare tag for MD5 hash
    WriteStartElement("HASH", output); output << std::endl;

      std::ostringstream os;
      itk::Indent hashIndent = currIndent.GetNextIndent();
      os << hashIndent;
      long md5Pos = output.tellp();
      std::string checkstr = "MD5";
      md5Pos += os.str().length() + checkstr.length() + 2;
      WriteSimpleElement<std::string>(this, checkstr.c_str(), hashIndent,
          output, "00000000000000000000000000000000"); // reserve 16 bytes (hex)

    output << currIndent;
    WriteEndElement("HASH", output); output << std::endl;

    // volume information:
    output << currIndent;
    WriteStartElement("VOLUME_META_INFO", output); output << std::endl;

    if (m_InputObject->GetVolumeMetaInfo())
    {
      VolumeMetaInformation::Pointer vmi = m_InputObject->GetVolumeMetaInfo();
      currIndent = currIndent.GetNextIndent();

      itk::Indent levelIndent = currIndent;

      // basic image information
      output << levelIndent;
      WriteStartElement("BASIC", output); output << std::endl;
        currIndent = levelIndent.GetNextIndent();

        WriteSimpleElement<std::string>(this, "MHD_File",
            currIndent, output, vmi->GetMHDFileName());
        WriteSimpleElement<std::string>(this, "Source",
            currIndent, output, vmi->GetSource());
        WriteSimpleElement<std::string>(this, "Source_Version",
            currIndent, output, vmi->GetSourceVersion());
        WriteSimpleElement<unsigned int>(this, "Number_Of_Intensity_Components",
            currIndent, output, vmi->GetNumberOfComponents());
        WriteSimpleElement<std::string>(this, "Direction",
          currIndent, output, DirectionTypeToArray(vmi->GetDirection()));
        WriteSimpleElement<std::string>(this, "Origin",
            currIndent, output, PointTypeToArray(vmi->GetOrigin()));
        WriteSimpleElement<std::string>(this, "Spacing",
            currIndent, output, FixedArrayToArray(vmi->GetSpacing()));
        WriteSimpleElement<std::string>(this, "Size",
            currIndent, output, FixedArrayToArray(vmi->GetSize()));
        WriteSimpleElement<std::string>(this, "Anatomical_Orientation",
            currIndent, output,
            StringArrayTypeToArray(vmi->GetAnatomicalOrientation()));
        WriteSimpleElement<std::string>(this, "Frame_Of_Reference_UID",
          currIndent, output, vmi->GetFrameOfReferenceUID());

      output << levelIndent;
      WriteEndElement("BASIC", output); output << std::endl;

      // image intensity information
      output << levelIndent;
      WriteStartElement("IMAGE_INTENSITY", output); output << std::endl;
        currIndent = levelIndent.GetNextIndent();

        WriteSimpleElement<double>(this, "WL_Level",
            currIndent, output, vmi->GetWLLevel());
        WriteSimpleElement<double>(this, "WL_Window",
            currIndent, output, vmi->GetWLWindow());
        WriteSimpleElement<double>(this, "Rescale_Slope",
            currIndent, output, vmi->GetRescaleSlope());
        WriteSimpleElement<double>(this, "Rescale_Intercept",
            currIndent, output, vmi->GetRescaleIntercept());
        WriteSimpleElement<std::string>(this, "Rescale_Unit",
            currIndent, output, vmi->GetRescaleUnit());
        WriteSimpleElement<std::string>(this, "Rescale_Descriptor",
            currIndent, output, vmi->GetRescaleDescriptor());
        WriteSimpleElement<int>(this, "Rescale_Digits",
            currIndent, output, (int)vmi->GetRescaleDigits());
        WriteSimpleElement<double>(this, "Gamma",
            currIndent, output, vmi->GetGamma());
        WriteSimpleElement<std::string>(this, "ORA_Palette_ID",
            currIndent, output, vmi->GetORAPaletteID());

      output << levelIndent;
      WriteEndElement("IMAGE_INTENSITY", output); output << std::endl;

      // pure DICOM information
      output << levelIndent;
      WriteStartElement("DICOM", output); output << std::endl;
        currIndent = levelIndent.GetNextIndent();

        WriteSimpleElement<std::string>(this, "Study_Instance_UID",
            currIndent, output, vmi->GetDICOMStudyInstanceUID());
        WriteSimpleElement<std::string>(this, "Study_ID",
            currIndent, output, vmi->GetDICOMStudyID());
        WriteSimpleElement<std::string>(this, "Study_Date",
            currIndent, output, vmi->GetDICOMStudyDate());
        WriteSimpleElement<std::string>(this, "Study_Time",
            currIndent, output, vmi->GetDICOMStudyTime());
        WriteSimpleElement<std::string>(this, "Study_Description",
            currIndent, output, vmi->GetDICOMStudyDescription());
        WriteSimpleElement<std::string>(this, "Series_Instance_UID",
            currIndent, output, vmi->GetDICOMSeriesInstanceUID());
        WriteSimpleElement<std::string>(this, "Series_Number",
            currIndent, output, vmi->GetDICOMSeriesNumber());
        WriteSimpleElement<std::string>(this, "Series_Description",
            currIndent, output, vmi->GetDICOMSeriesDescription());
        WriteSimpleElement<std::string>(this, "Modality",
            currIndent, output, vmi->GetDICOMModality());
        WriteSimpleElement<std::string>(this, "SOP_Class_UID",
            currIndent, output, vmi->GetDICOMSOPClassUID());
        WriteSimpleElement<std::string>(this, "Device_Serial",
            currIndent, output, vmi->GetDICOMDeviceSerial());
        WriteSimpleElement<std::string>(this, "Manufacturer",
            currIndent, output, vmi->GetDICOMManufacturer());
        WriteSimpleElement<std::string>(this, "Creator_UID",
            currIndent, output, vmi->GetDICOMCreatorUID());
        WriteSimpleElement<std::string>(this, "Accession_Number",
            currIndent, output, vmi->GetDICOMAccessionNumber());
        WriteSimpleElement<unsigned short>(this, "Bits_Allocated",
            currIndent, output, vmi->GetBitsAllocated());
        WriteSimpleElement<unsigned short>(this, "Bits_Stored",
            currIndent, output, vmi->GetBitsStored());
        WriteSimpleElement<std::string>(this, "Complementary_Study_UID",
            currIndent, output, vmi->GetComplementaryStudyUID());
        WriteSimpleElement<std::string>(this, "Complementary_Series_UID",
            currIndent, output, vmi->GetComplementarySeriesUID());

      output << levelIndent;
      WriteEndElement("DICOM", output); output << std::endl;

      // patient information
      output << levelIndent;
      WriteStartElement("PATIENT", output); output << std::endl;
        currIndent = levelIndent.GetNextIndent();

        WriteSimpleElement<std::string>(this, "Patient_Position",
            currIndent, output, vmi->GetPatientPosition());
        WriteSimpleElement<std::string>(this, "Patient_ID",
            currIndent, output, vmi->GetPatientID());
        WriteSimpleElement<std::string>(this, "Patient_Name",
            currIndent, output, vmi->GetPatientName());
        WriteSimpleElement<std::string>(this, "Patient_BirthDate",
            currIndent, output, vmi->GetPatientBirthDate());
        WriteSimpleElement<std::string>(this, "Patient_Sex",
            currIndent, output, vmi->GetPatientSex());

      output << levelIndent;
      WriteEndElement("PATIENT", output); output << std::endl;

      // RT related information
      output << levelIndent;
      WriteStartElement("RT", output); output << std::endl;
        currIndent = levelIndent.GetNextIndent();

        WriteSimpleElement<std::string>(this, "RT_Image_Plane",
            currIndent, output, vmi->GetRTIPlane());
        WriteSimpleElement<std::string>(this, "RT_Image_Description",
            currIndent, output, vmi->GetRTIDescription());
        WriteSimpleElement<std::string>(this, "RT_Image_Label",
            currIndent, output, vmi->GetRTILabel());
        WriteSimpleValidityDependentElement(XRayImageReceptorAngle, double,
            "Xray_Image_Receptor_Angle")
        WriteSimpleValidityDependentElement(TableHeight, double, "Table_Height")
        WriteSimpleValidityDependentElement(SourceFilmDistance, double,
            "Source_Film_Distance")
        WriteSimpleValidityDependentElement(SourceAxisDistance, double,
            "Source_Axis_Distance")
        WriteSimpleValidityDependentElement(Couch, double, "Couch_Isocentric")
        WriteSimpleValidityDependentElement(Collimator, double, "Collimator")
        WriteSimpleValidityDependentElement(Gantry, double, "Gantry")
        WriteSimpleElement<std::string>(this, "RT_Patient_Plan",
            currIndent, output, vmi->GetPatientPlan());
        WriteSimpleElement<std::string>(this, "RT_Patient_Beam",
            currIndent, output, vmi->GetPatientBeam());
        WriteSimpleElement<std::string>(this, "Machine",
            currIndent, output, vmi->GetMachine());
        if (vmi->GetIsocenterValid())
          WriteSimpleElement<std::string>(this, "Isocenter",
            currIndent, output, PointTypeToArray(vmi->GetIsocenter()));
        else
          WriteSimpleElement<std::string>(this, "Isocenter",
            currIndent, output, "");

      output << levelIndent;
      WriteEndElement("RT", output); output << std::endl;
    }

      // slices information:
      currIndent = currIndent.GetNextIndent(); output << currIndent;
      WriteStartElement("SLICES_META_INFO", output); output << std::endl;
      itk::Indent storeIndent = currIndent;
      currIndent = currIndent.GetNextIndent();

      if (m_InputObject->GetSlicesMetaInformation())
      {
        std::vector<SliceMetaInformation::Pointer> *smiv = m_InputObject->
          GetSlicesMetaInformation();

        for (unsigned int i = 0; i < smiv->size(); ++i)
        {
          SliceMetaInformation::Pointer smi = (*smiv)[i];
          output << currIndent;
          WriteStartElement("SLICE", output); output << std::endl;
          itk::Indent levelIndent(currIndent.GetNextIndent());

          WriteSimpleValidityDependentElementSlice(SliceLocation, double,
              "Slice_Location")
          WriteSimpleElement<std::string>(this, "Origin",
            levelIndent, output, PointTypeToArray(smi->GetOrigin()));

          WriteSimpleElement<std::string>(this, "DICOM_Instance_Number",
            levelIndent, output, smi->GetDICOMInstanceNumber());
          WriteSimpleElement<std::string>(this, "DICOM_SOP_Instance_UID",
            levelIndent, output, smi->GetDICOMSOPInstanceUID());
          WriteSimpleElement<std::string>(this, "Complementary_Instance_UID",
            levelIndent, output, smi->GetComplementaryInstanceUID());

          WriteSimpleElement<std::string>(this, "ORA_Acquisition_Date",
            levelIndent, output, smi->GetORAAcquisitionDate());
          WriteSimpleElement<std::string>(this, "ORA_Acquisition_Type_ID",
            levelIndent, output, smi->GetORAAcquisitionTypeID());
          WriteSimpleElement<std::string>(this, "ORA_Acquisition_Type",
            levelIndent, output, smi->GetORAAcquisitionType());

          WriteSimpleValidityDependentElementSlice(ImageFrames, int,
              "Image_Frames")
          WriteSimpleValidityDependentElementSlice(ImageMeasurementTime, double,
              "Image_Measurement_Time")
          WriteSimpleValidityDependentElementSlice(ImageDoseRate, double,
              "Image_Dose_Rate")
          WriteSimpleValidityDependentElementSlice(ImageNormDoseRate, double,
              "Image_Norm_Dose_Rate")
          WriteSimpleValidityDependentElementSlice(ImageOutput, double,
              "Image_Output")
          WriteSimpleValidityDependentElementSlice(ImageNormOutput, double,
              "Image_Norm_Output")

          output << currIndent;
          WriteEndElement("SLICE", output); output << std::endl;
        }
      }

      output << storeIndent;
      WriteEndElement("SLICES_META_INFO", output); output << std::endl;

    currIndent = rootIndent.GetNextIndent(); output << currIndent;
    WriteEndElement("VOLUME_META_INFO", output); output << std::endl;

  WriteEndElement("ORA_IMAGE_META_INFO", output); output << std::endl;

  // generate MD5-hash:
  std::string filecontent = output.str();
  std::string md5Hash = "";
  char *filedata = const_cast<char*> (filecontent.c_str());
  md5Hash = ITKVTKMetaInformationXMLFileWriter::ComputeHashMD5(filedata, filecontent.length());

  // write MD5-hash after "<indent><MD5>":
  output.seekp(md5Pos);
  output << md5Hash;

  // -> really write to file!
  std::ofstream foutput;
  // (write in binary mode to prevent CR/LF-translation)
  foutput.open(m_Filename.c_str(), std::ios::binary | std::ios::out);
  if(foutput.fail())
  {
    SimpleDebugger deb;
    SimpleErrorMacro2Object(deb, << "Cannot write file (" <<
        m_Filename << ")")
  }
  foutput << output.str();
  foutput.close();

  return true;
}

std::string
ITKVTKMetaInformationXMLFileWriter
::ComputeHashMD5(char *data, const unsigned int &datasize)
{
  char *md5result;
  const unsigned int md5length = 16;
#ifdef USE_QT
  QCryptographicHash md5qt(QCryptographicHash::Md5);
  md5qt.addData(data, datasize);
  md5result = md5qt.result().data();
#else
  vtksysMD5 *md5 = vtksysMD5_New();
  vtksysMD5_Initialize(md5);
  vtksysMD5_Append(md5, reinterpret_cast<unsigned char*>(data), datasize);
  unsigned char md5resTemp[16];
  vtksysMD5_Finalize(md5, md5resTemp);
  vtksysMD5_Delete(md5);
  md5result = reinterpret_cast<char*>(md5resTemp);
#endif

  char hexbuf[3];
  std::string md5Hash = "";
  std::string s(md5result, md5length);
  for (std::string::const_iterator it=s.begin() ; it < s.end(); it++ )
  {
    sprintf(hexbuf, "%02x", 128 + *it);
    md5Hash.append(hexbuf);
  }
  // DEBUG output:
  /*
  printf("          0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15\n"
         "md5sum:  [%i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i]\n"
         "md5Hash: [%s]\n",
         md5result[0], md5result[1], md5result[2], md5result[3], md5result[4],
         md5result[5], md5result[6], md5result[7], md5result[8], md5result[9],
         md5result[10], md5result[11], md5result[12], md5result[13], md5result[14],
         md5result[15], md5Hash.c_str());
  */
  return md5Hash;
}

}

