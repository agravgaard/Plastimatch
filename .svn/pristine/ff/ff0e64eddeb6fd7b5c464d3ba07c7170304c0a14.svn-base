

#include "itkRTIImageIO.h"

#include <fstream>
#define _USE_MATH_DEFINES  // For MSVC
#include <math.h>
#include <iterator>

// ORAIFTools
#include "oraIniAccess.h"
#include "SimpleDebugger.h"
#include "oraFileTools.h"

#include <itksys/SystemTools.hxx>
#include <itkByteSwapper.h>
#include <itkVersorRigid3DTransform.h>


#define READ_DICOM_KEY(key, objectPointer, prop) \
{ \
  std::string s = ini.ReadString("DICOMi", key, ""); \
  if (s.length() == 0) \
    s = ini.ReadString("DICOMe", key, ""); \
  if (objectPointer->Get##prop().length() <= 0) \
    objectPointer->Set##prop(s); \
}


namespace itk
{


RTIImageIO
::RTIImageIO()
{
  m_Header = NULL;
  m_MetaInfo = NULL;
  SetComponentType(itk::ImageIOBase::USHORT);
  SetNumberOfComponents(1);
  SetPixelType(itk::ImageIOBase::SCALAR);
  m_ComplementaryMetaFile = "";
  m_SequencerHelpFlag = true;
  m_SequenceNumber = 0;
  SetImageSeriesReaderMode(false);
}

RTIImageIO
::~RTIImageIO()
{
  m_MetaInfo = NULL;
  if (m_Header)
    delete m_Header;
}

void
RTIImageIO
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << std::endl;
  os << "Complementary Meta File: " << m_ComplementaryMetaFile << std::endl;
  os << std::endl;
  os << "RTI-HEADER: (" << sizeof(_RTI_HEADER) << ")" << std::endl;
  if (m_Header)
  {
    os << indent << "BmpTypeMajor=" << (int)m_Header->GetBmpTypeMajor() <<
      std::endl;
    os << indent << "BmpTypeMinor=" << (int)m_Header->GetBmpTypeMinor() <<
      std::endl;
    os << indent << "BmpTypeRevision=" << (int)m_Header->GetBmpTypeRevision() <<
      std::endl;
    os << indent << "BmpTypeReserved=" << (int)m_Header->GetBmpTypeReserved() <<
      std::endl;
    os << indent << "Cols=" << m_Header->GetCols() << std::endl;
    os << indent << "Rows=" << m_Header->GetRows() << std::endl;
    os << indent << "BitsAlloc=" << m_Header->GetBitsAlloc() << std::endl;
    os << indent << "BitsStored=" << m_Header->GetBitsStored() << std::endl;
    os << indent << "Center64=" << m_Header->GetCenter64() << std::endl;
    os << indent << "Width64=" << m_Header->GetWidth64() << std::endl;
    os << indent << "RescaleIntercept=" << m_Header->GetRescaleIntercept() <<
      std::endl;
    os << indent << "RescaleSlope=" << m_Header->GetRescaleSlope() << std::endl;
    os << indent << "RescaleUnit[20]=\"" << m_Header->GetRescaleUnit() << "\"" <<
      std::endl;
    os << indent << "RescaleDescriptor[20]=\"" <<
      m_Header->GetRescaleDescriptor() << "\"" << std::endl;
    os << indent << "RescaleDigits=" << (int)m_Header->GetRescaleDigits() <<
      std::endl;
    os << indent << "GammaSingle=" << m_Header->GetGammaSingle() << std::endl;
  }
  else
    os << indent << "... header not loaded ..." << std::endl;
}

bool
RTIImageIO
::CanReadFile(const char *fileName)
{
  const std::string fn(fileName);

  if (m_Header) // obviously already loaded with success
    return true;

  // check file extension (could be discussed ...):
  if (fn.compare("") == 0)
  {
    itkDebugMacro(<< "No RTI file name specified.");

    return false; // file name not set
  }

  std::string fnlow;
  transform(fn.begin(), fn.end(), back_inserter(fnlow), (int(*)(int))tolower);

  std::string ext = itksys::SystemTools::GetFilenameExtension(fnlow);
  bool rtiExt = false;
  if (ext == ".rti" || (ext.length() >= 4 && ext.substr(ext.length() - 4, 4) == ".rti"))
    rtiExt = true;
  bool rtiOrgExt = false;
  if (ext == ".rti.org" || (ext.length() >= 8 && ext.substr(ext.length() - 8, 8) == ".rti.org"))
    rtiOrgExt = true;
  if (!rtiExt && !rtiOrgExt)
  {
    itkDebugMacro(<< "RTI file extension does not match *.rti or *.rti.org!");

    return false;
  }

  if (!itksys::SystemTools::FileExists(fn.c_str()))
  {
    itkDebugMacro(<< "RTI file does not exist.");

    return false;
  }

  // pre-buffer the header and check version:
  std::fstream fs;
  if (m_Header)
    delete(m_Header);
  m_Header = new _RTI_HEADER;

  try
  {
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
    fs.open(fn.c_str(), std::ios::in | std::ios::binary);
#else
    fs.open(fn.c_str(), std::ios::in);
#endif
    fs.seekg(0, std::ios::beg);
    fs.read((char *)m_Header, sizeof(_RTI_HEADER));
    fs.close();
  }
  catch (std::fstream::failure e)
  {
    itkDebugMacro(<< "Error during header streaming.");
    delete m_Header;
    m_Header = NULL;

    return false;
  }
  if (fs.fail())
  {
    itkDebugMacro(<< "Error during header streaming.");
    delete m_Header;
    m_Header = NULL;

    return false;
  }

  // >= 1.2 !
  if (m_Header->GetBmpTypeMajor() < 1 ||
      (m_Header->GetBmpTypeMajor() == 1 && m_Header->GetBmpTypeMinor() < 2))
  {
    itkDebugMacro(<< "RTI header version conflict.");
    delete m_Header;
    m_Header = NULL;

    return false;
  }

  // currently only 16bits supported (allocated bits!):
  if (m_Header->GetBitsAlloc() != 16)
  {
    itkDebugMacro(<< "Only 16-bit images (unsigned short) supported.");
    delete m_Header;
    m_Header = NULL;

    return false;
  }

  return true;
}

void
RTIImageIO
::ReadImageInformation()
{
  // if RTIImageIO is set forcefully CanReadFile() is not invoked ...
  if (!CanReadFile(m_FileName.c_str()))
  {
    itkDebugMacro(<< "Cannot read RTI information - problems with image type.");
    return;
  }
  // NOTE: INTERNALLY we treat 2D images as 3D images with 1 slice
  // (with spacing 1.0) -> to keep our chance to build up a volume later!

  SetNumberOfDimensions(3);
  // NOTE: the information in the rti-file directly is more reliable
  // (caused by a bug of myself!)
  SetDimensions(0, m_Header->GetCols());
  SetDimensions(1, m_Header->GetRows());
  SetDimensions(2, 1);

  if (m_ComplementaryMetaFile.length() > 4 &&
      itksys::SystemTools::FileExists(m_ComplementaryMetaFile.c_str()))
  {
    ora::IniAccess ini(m_ComplementaryMetaFile);

    // image spacing:

    // use Metrics\Left ... to implicitly calculate spacing; do not use
    // the (eventually) available Image\OrigPixelSpacing entries:
    float le = ini.ReadValue<float>("Metrics", "Left", 0.f);
    float ri = ini.ReadValue<float>("Metrics", "Right", 0.f);
    float w = (ri - le) * 10.f; // cm -> mm !!!
    float to = ini.ReadValue<float>("Metrics", "Top", 0.f);
    float bo = ini.ReadValue<float>("Metrics", "Bottom", 0.f);
    float h = (to - bo) * 10.f; // cm -> mm !!!

    if (w < 0.f) // allow top/left/right/left variations
      w *= -1.f;
    if (h < 0.f)
      h *= -1.f;

    if (w > 0.f && h > 0.f &&
        GetDimensions(0) > 0 && GetDimensions(1) > 0) // defined
    {
      SetSpacing(0, w / GetDimensions(0));
      SetSpacing(1, h / GetDimensions(1));
    }
    else // default
    {
      SetSpacing(0, 1.0);
      SetSpacing(1, 1.0);
    }
    SetSpacing(2, 1.0);


    // EXTENSION (2010-06-29): image position and orientation of planar images
    // are not reliable! Therefore use the SAD and SFD properties together with
    // the gantry and metric-props to determine the image position and
    // orientation based on the related projection geometry.

    bool applyProjectiveGeometry = false;
    std::string fileVersion = ini.ReadString("File", "Version", "");
    double sfd = ini.ReadValue<double>("Metrics", "SFD", 0);
    double sad = ini.ReadValue<double>("Metrics", "SAD", 0);
    double gantry = ini.ReadValue<double>("Geometry", "Gantry", -1000);
    // prerequisite 1: gantry (=imaging device source) is defined
    if (gantry > -999.9)
    {
      gantry = gantry / 180 * M_PI; // deg -> rad
      // prerequisite 2: SAD (=source axis distance) is defined
      if (sad > 1e-6)
      {
        sad *= 10.; // cm -> mm
        // prerequisite 3: SFD (=source to film distance) is defined
        if (sfd > 1e-6)
        {
          sfd *= 10.; // cm -> mm
          // prerequisite 4: metric must be defined
          if (w > 0.f && h > 0.f)
          {
            // prerequisite 5: no file version string available (relates to BUG)
            if (fileVersion.length() == 0)
            {
              applyProjectiveGeometry = true;
            }
          }
        }
      }
    }

    // image position:

    if (!applyProjectiveGeometry) // non-projective image (or slice)
    {
      // "x-off,y-off,z-off"
      std::vector<std::string> origin;
      std::string originStr = ini.ReadString("Metrics", "ImagePosition", "");

      // NOTE: open radART convention treats the image position as the 'left
      // lower' corner of the first voxel transmitted; in contrast, ITK's image
      // origin is the center of the first voxel transmitted - correct this
      // below after detection of the image orientation

      ora::Tokenize(originStr, origin, ",");
      if (origin.size() == 3)
      {
        SetOrigin(0, atof(origin[0].c_str()) * 10.0f); // cm -> mm
        SetOrigin(1, atof(origin[1].c_str()) * 10.0f);
        SetOrigin(2, atof(origin[2].c_str()) * 10.0f);
      }
      else
      {
        SetOrigin(0, 0.0);
        SetOrigin(1, 0.0);
        SetOrigin(2, 0.0);
      }
    }

    // image orientation:

    std::vector<double> dirRow0(3, 0);
    std::vector<double> dirRow1(3, 0);
    std::vector<double> dirRow2(3, 0);

    if (!applyProjectiveGeometry) // non-projective image (or slice)
    {
      // "R11,R12,R13,R21,R22,R23"
      std::vector<std::string> direction;
      std::string directionStr = ini.ReadString("Metrics",
          "ImageOrientation", "");

      // the IO's direction cosines are stored in COLUMNS (not in rows) !!!

      ora::Tokenize(directionStr, direction, ",");
      std::vector<double> dir0, dir1, dir2;

      if (direction.size() == 6)
      {
        for (int i = 0; i < 3; i++)
          dirRow0[i] = atof(direction[i].c_str());

        for (int i = 0; i < 3; i++)
          dirRow1[i] = atof(direction[3 + i].c_str());

        dirRow2[0] = dirRow0[1] * dirRow1[2] - dirRow0[2] * dirRow1[1];
        dirRow2[1] = dirRow0[2] * dirRow1[0] - dirRow0[0] * dirRow1[2];
        dirRow2[2] = dirRow0[0] * dirRow1[1] - dirRow0[1] * dirRow1[0];
      }
      else
      {
        dirRow0[0] = 1.;
        dirRow0[1] = 0.;
        dirRow0[2] = 0.;

        dirRow1[0] = 0.;
        dirRow1[1] = 1.;
        dirRow1[2] = 0.;

        dirRow2[0] = 0.;
        dirRow2[1] = 0.;
        dirRow2[2] = 1.;
      }

      // BUG FIX (2011-03-02): The rows need not being converted into columns
      // here, that's inherent! I do not understand how I could lose sight of
      // this dramatic bug?!
      SetDirection(0, dirRow0);
      SetDirection(1, dirRow1);
      SetDirection(2, dirRow2);
    }

    if (applyProjectiveGeometry) // projective image (e.g. X-ray)
    {
      typedef itk::VersorRigid3DTransform<double> RotationTransformType;

      // modeled gantry rotation around y-axis of machine coordinate system:
      RotationTransformType::Pointer rtrans = RotationTransformType::New();
      RotationTransformType::AxisType rax;
      rax[0] = 0;
      rax[1] = 1;
      rax[2] = 0;
      rtrans->SetRotation(rax, gantry);

      // basic pixel row vector:
      RotationTransformType::OutputVectorType prow;
      prow[0] = 1;
      prow[1] = 0;
      prow[2] = 0;
      // basic pixel column vector:
      RotationTransformType::OutputVectorType pcol;
      pcol[0] = 0;
      pcol[1] = 1;
      pcol[2] = 0;
      // intersection point of image plane and central axis (@ gantry angle 0):
      RotationTransformType::OutputPointType cax;
      cax[0] = 0;
      cax[1] = 0;
      cax[2] = -(sfd - sad);

      // obtain new vector directions / points by transforming them:
      RotationTransformType::OutputVectorType eprow = rtrans->
          TransformVector(prow);
      RotationTransformType::OutputVectorType epcol = rtrans->
          TransformVector(pcol);
      RotationTransformType::OutputPointType ecax = rtrans->
          TransformPoint(cax);

      // Image Position

      // compute image position:
      RotationTransformType::OutputPointType origin;
      origin = ecax;
      origin = origin + eprow * le * 10.; // le: cm -> mm
      origin = origin + epcol * bo * 10.; // bo: cm -> mm

      for (int i = 0; i < 3; i++)
        SetOrigin(i, origin[i]);

      // Image Orientation

      for (int i = 0; i < 3; i++)
        dirRow0[i] = eprow[i];

      for (int i = 0; i < 3; i++)
        dirRow1[i] = epcol[i];

      dirRow2[0] = dirRow0[1] * dirRow1[2] - dirRow0[2] * dirRow1[1];
      dirRow2[1] = dirRow0[2] * dirRow1[0] - dirRow0[0] * dirRow1[2];
      dirRow2[2] = dirRow0[0] * dirRow1[1] - dirRow0[1] * dirRow1[0];

      std::vector<double> dirCol; // convert direction rows to direction columns
      for (int i = 0; i < 3; i++)
      {
        dirCol.clear();
        if (i == 0)
        {
          dirCol.push_back(dirRow0[0]);
          dirCol.push_back(dirRow0[1]);
          dirCol.push_back(dirRow0[2]);
        }
        else if (i == 1)
        {
          dirCol.push_back(dirRow1[0]);
          dirCol.push_back(dirRow1[1]);
          dirCol.push_back(dirRow1[2]);
        }
        else if (i == 2)
        {
          dirCol.push_back(dirRow2[0]);
          dirCol.push_back(dirRow2[1]);
          dirCol.push_back(dirRow2[2]);
        }

        SetDirection(i, dirCol);
      }
    }

    // correct the voxel center (open radART DICOM-converter makes corrections
    // just in-plane, therefore, we only need to re-correct these dimensions):
    for (int i = 0; i < 3; i++)
      SetOrigin(i, GetOrigin(i) +
          dirRow0[i] * GetSpacing(0) / 2. +
          dirRow1[i] * GetSpacing(1) / 2.);

    // Additional meta information if requested:
    if (m_MetaInfo)
    {
      if (m_SequencerHelpFlag)
        m_SequenceNumber = -1;

      m_SequenceNumber++; // 0-based current image number within sequence

      if (m_SequencerHelpFlag)
      {
        m_MetaInfo->SetFileVersion(MetaInfoType::HOTTEST_FILE_VERSION);

        // volume related:
        ora::VolumeMetaInformation::Pointer volMI = m_MetaInfo->
          GetVolumeMetaInfo();
        if (volMI)
        {
          // *** internal information (image object and header data):

          // - set source and its version
          volMI->SetSource("ORA-RTI");
          volMI->SetSourceVersion(ini.ReadString("File", "FileFormat", ""));

          ora::VolumeMetaInformation::DirectionType dir;
          for (int i = 0; i < 3; i++)
          {
            dir[0][i] = dirRow0[i];
            dir[1][i] = dirRow1[i];
            dir[2][i] = dirRow2[i];
          }
          volMI->SetDirection(dir);

          ora::VolumeMetaInformation::PointType orig;
          for (int i = 0; i < 3; i++)
            orig[i] = GetOrigin(i);
          volMI->SetOrigin(orig);

          // just preliminary (must externally be reset)
          ora::VolumeMetaInformation::SpacingType spac;
          for (int i = 0; i < 3; i++)
            spac[i] = GetSpacing(i);
          volMI->SetSpacing(spac);

          // just preliminary (must externally be reset)
          ora::VolumeMetaInformation::SizeType sz;
          for (int i = 0; i < 3; i++)
            sz[i] = GetDimensions(i);
          volMI->SetSize(sz);

          //-> external: volMI->SetAnatomicalOrientation();

          volMI->SetNumberOfComponents(GetNumberOfComponents());

          // find image-list entry for first slice (-> take FOR!)
          if (m_MetaInfo->GetImageList())
          {
            MetaInfoType::ImgLstEntryPointer ile = m_MetaInfo->
              GetImageList()->FindEntry(m_FileName);
            if (ile)
            {
              MetaInfoType::ImgLstEntryType::FORPointer refFOR = ile->
                GetReferencedFOR();
              if (refFOR)
                volMI->SetFrameOfReferenceUID(refFOR->GetUID());
            }
          }

          // MHDFileName is set at another point

          volMI->SetBitsAllocated(m_Header->GetBitsAlloc());

          volMI->SetBitsStored(m_Header->GetBitsStored());

          volMI->SetWLLevel((double)m_Header->GetCenter64());

          volMI->SetWLWindow((double)m_Header->GetWidth64());

          volMI->SetRescaleIntercept(m_Header->GetRescaleIntercept());

          volMI->SetRescaleSlope(m_Header->GetRescaleSlope());

          volMI->SetRescaleUnit(ora::TrimF(m_Header->GetRescaleUnit()));

          volMI->SetRescaleDescriptor(ora::TrimF(m_Header->
              GetRescaleDescriptor()));

          volMI->SetRescaleDigits(m_Header->GetRescaleDigits());

          volMI->SetGamma(m_Header->GetGammaSingle());


          // *** [DICOM] section of info file

          READ_DICOM_KEY("StudyInstUID", volMI, DICOMStudyInstanceUID)

          READ_DICOM_KEY("StudyID", volMI, DICOMStudyID)

          READ_DICOM_KEY("StudyDate", volMI, DICOMStudyDate)

          READ_DICOM_KEY("StudyTime", volMI, DICOMStudyTime)

          READ_DICOM_KEY("StudyDescr", volMI, DICOMStudyDescription)

          READ_DICOM_KEY("SeriesInstUID", volMI, DICOMSeriesInstanceUID)

          READ_DICOM_KEY("SeriesNo", volMI, DICOMSeriesNumber)

          READ_DICOM_KEY("SeriesDescr", volMI, DICOMSeriesDescription)

          READ_DICOM_KEY("Modality", volMI, DICOMModality)

          READ_DICOM_KEY("SOPClassUID", volMI, DICOMSOPClassUID)

          READ_DICOM_KEY("DeviceSerial", volMI, DICOMDeviceSerial)

          READ_DICOM_KEY("Manufacturer", volMI, DICOMManufacturer)

          READ_DICOM_KEY("CreatorUID", volMI, DICOMCreatorUID)

          READ_DICOM_KEY("AccessionNo", volMI, DICOMAccessionNumber)


          // *** [Patient] section of info file

          volMI->SetPatientPosition(ini.ReadString("Patient",
              "PatPosition", ""));

          volMI->SetPatientID(ini.ReadString("Patient", "PatID", ""));

          volMI->SetPatientName(ini.ReadString("Patient", "Name", ""));

          volMI->SetPatientBirthDate(ini.ReadString("Patient",
              "BirthDate", ""));

          volMI->SetPatientSex(ini.ReadString("Patient", "Sex", ""));

          volMI->SetPatientPlan(ini.ReadString("Patient", "Plan", ""));

          volMI->SetPatientBeam(ini.ReadString("Patient", "Beam", ""));


          // *** [Image] section of info file

          volMI->SetRTIPlane(ini.ReadString("Image", "RTImagePlane", ""));

          volMI->SetRTIDescription(ini.ReadString("Image",
              "RTImageDescription", ""));

          volMI->SetRTILabel(ini.ReadString("Image", "RTImageLabel", ""));

          volMI->SetXRayImageReceptorAngleByString(ini.ReadString("Image",
              "XRayImageReceptorAngle", ""));

          volMI->SetORAPaletteID(ini.ReadString("Image", "Palette", ""));

          volMI->SetComplementaryStudyUID(ini.ReadString("Image",
              "CompStudy", ""));

          volMI->SetComplementarySeriesUID(ini.ReadString("Image",
              "CompSeries", ""));


          // *** [Metrics] section of info file

          volMI->SetTableHeightByStringWithSlope(ini.ReadString("Metrics",
              "TableHeight", ""), 10.);

          volMI->SetSourceFilmDistanceByStringWithSlope(ini.ReadString(
              "Metrics", "SFD", ""), 10.);

          volMI->SetSourceAxisDistanceByStringWithSlope(ini.ReadString(
              "Metrics", "SAD", ""), 10.);


          // *** [Geometry] section of info file

          volMI->SetCouchByString(ini.ReadString("Geometry", "Couch", ""));

          volMI->SetCollimatorByString(ini.ReadString("Geometry",
              "Collimator", ""));

          volMI->SetGantryByString(ini.ReadString("Geometry", "Gantry", ""));

          volMI->SetMachine(ini.ReadString("Geometry", "Machine", ""));

          volMI->SetIsocenterByStringWithSlope(ini.ReadString("Geometry",
              "IsoCtrX", ""), ini.ReadString("Geometry", "IsoCtrY", ""),
              ini.ReadString("Geometry", "IsoCtrZ", ""), 10.);
        }

        m_SequencerHelpFlag = false; // set back
      }

      std::vector<ora::SliceMetaInformation::Pointer> *sliceMIs =
        m_MetaInfo->GetSlicesMetaInformation();
      if (sliceMIs)
      {
        // in image series reader mode:
        // clear previous slice MIs as long as Read() has not been called
        if (m_ImageSeriesReaderMode && !m_ImageSeriesReaderHelpFlag)
          sliceMIs->clear();

        ora::SliceMetaInformation::Pointer sliceMI =
          ora::SliceMetaInformation::New();
        sliceMIs->push_back(sliceMI);

        sliceMI->SetComplementaryInstanceUID(ini.ReadString("Image",
            "CompInstance", ""));

        sliceMI->SetSliceLocationByStringWithSlope(ini.ReadString("Metrics",
            "SliceLocation", ""), 10);

        ora::SliceMetaInformation::PointType orig;
        for (int i = 0; i < 3; i++)
          orig[i] = GetOrigin(i);
        sliceMI->SetOrigin(orig);

        READ_DICOM_KEY("InstanceNo", sliceMI, DICOMInstanceNumber)

        READ_DICOM_KEY("SOPInstanceUID", sliceMI, DICOMSOPInstanceUID)

        sliceMI->SetORAAcquisitionDate(ini.ReadString("Info",
            "AcquDate", ""));

        sliceMI->SetORAAcquisitionTypeID(ini.ReadString("Info",
            "AcquTypeID", ""));

        sliceMI->SetORAAcquisitionType(ini.ReadString("Info",
            "AcquType", ""));

        sliceMI->SetImageFramesByString(ini.ReadString("Image",
            "Frames", ""));

        sliceMI->SetImageMeasurementTimeByString(ini.ReadString("Image",
            "MeasurementTime", ""));

        sliceMI->SetImageDoseRateByString(ini.ReadString("Image",
            "DoseRate", ""));

        sliceMI->SetImageNormDoseRateByString(ini.ReadString("Image",
            "NormDoseRate", ""));

        sliceMI->SetImageOutputByString(ini.ReadString("Image",
            "Output", ""));

        sliceMI->SetImageNormOutputByString(ini.ReadString("Image",
            "NormOutput", ""));
      }
    }

  }
  else // apply defaults
  {
    SetSpacing(0, 1.);
    SetSpacing(1, 1.);
    SetSpacing(2, 1.);

    SetOrigin(0, 0.);
    SetOrigin(1, 0.);
    SetOrigin(2, 0.);

    std::vector<double> dir;
    dir.push_back(1.);
    dir.push_back(0.);
    dir.push_back(0.);
    SetDirection(0, dir);
    dir[0] = 0.;
    dir[1] = 1.;
    SetDirection(1, dir);
    dir[1] = 0.;
    dir[2] = 1.;
    SetDirection(2, dir);
  }
}

void
RTIImageIO
::Read(void* buffer)
{
  m_ImageSeriesReaderHelpFlag = true; // mark that Read() was called!

  // NOTE:
  // unfortunately there is a small bug in open radART RTI images
  // version <=1.2 (caused by myself!) -> DICOMConverter generates images
  // with one byte less than expected (cause is a auto-clipping-procedure);
  // therefore itk::RawImageIO fails, so here is a workaround Read()-method:

  std::ifstream fs;

  // open the file:
#ifdef _WIN32
  fs.open(m_FileName.c_str(), std::ios::in | std::ios::binary);
#else
  fs.open(m_FileName.c_str(), std::ios::in);
#endif

  if (fs.fail())
  {
    itkExceptionMacro(<< "Could not open file: " << m_FileName);
    try
    {
      fs.close();
    }
    catch (std::fstream::failure e)
    {
      ;
    }

    return;
  }

  // get the file size in bytes:
  long filestart, fileend;
  unsigned long filelength;
  filestart = fs.tellg();
  fs.seekg (0, std::ios::end);
  fileend = fs.tellg();
  filelength = fileend - filestart;
  itkDebugMacro(<< "image file size: " << filelength << " bytes");

  fs.seekg((long)sizeof(_RTI_HEADER), std::ios::beg); // overcome header offset
  if (fs.fail())
  {
    itkExceptionMacro(<< "file seek (header-offset) failed");
    try
    {
      fs.close();
    }
    catch (std::fstream::failure e)
    {
      ;
    }

    return;
  }

  // find out whether the file contains the version <=1.2 bug or not:
  unsigned long imageSize = static_cast<unsigned long>(
    this->GetImageSizeInBytes());
  bool rtiBug = false;

  if (filelength < (imageSize + sizeof(_RTI_HEADER)))
  {
    // obviously the bug: -> tolerate one missing byte
    if (filelength == (imageSize + sizeof(_RTI_HEADER) - 1))
    {
      imageSize -= 1;
      rtiBug = true;
    }
    else // too buggy ...
    {
      itkExceptionMacro(<< "RTI image is too buggy");
      try
      {
        fs.close();
      }
      catch (std::fstream::failure e)
      {
        ;
      }

      return;
    }
  }
  if (rtiBug)
  {
    itkDebugMacro(<< "RTI-bug detected and successfully overcome.");
  }
  else
  {
    itkDebugMacro(<< "No RTI-bug detected.");
  }

  const std::streamsize byToBeRead = static_cast< std::streamsize>(imageSize);
  fs.read(static_cast<char *>(buffer), byToBeRead);
  const std::streamsize byRead = fs.gcount();

#ifdef __APPLE_CC__
  // fail() is broken in the Mac. It returns true when reaches eof().
  if (byRead != byToBeRead)
#else
  if ((byRead != byToBeRead) || fs.fail())
#endif
  {
    itkExceptionMacro(<< "image buffer could not be read");
    try
    {
      fs.close();
    }
    catch (std::fstream::failure e)
    {
      ;
    }

    return;
  }

  if (rtiBug) // -> apply a virtual last byte (equal to pixel before)
  {
    imageSize += 1;
    buffer = (void *)realloc(buffer, imageSize);
    *((char *)((long)buffer + imageSize - 1)) =
      *((char *)((long)buffer + imageSize - 3));
  }

  itkDebugMacro(<< "RTI image data successfully read.");

  // swap (stored in little endian byte order):
  typedef ByteSwapper<unsigned short> InternalByteSwapperType;
  InternalByteSwapperType::SwapRangeFromSystemToLittleEndian(
    (unsigned short *)buffer, GetImageSizeInComponents());

  // close file at the end:
  try
  {
    fs.close();
  }
  catch (std::fstream::failure e)
  {
    ;
  }
}

bool
RTIImageIO
::CanWriteFile(const char*)
{
  itkDebugMacro(<< "RTI writing currently not supported.");

  return false;
}

void
RTIImageIO
::WriteImageInformation()
{
  itkDebugMacro(<< "RTI writing currently not supported.");

  return;
}

void
RTIImageIO
::Write(const void* buffer)
{
  itkDebugMacro(<< "RTI writing currently not supported.");

  return;
}


}
