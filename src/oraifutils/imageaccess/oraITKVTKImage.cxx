
#include "oraITKVTKImage.h"

// NOTE: Preprocessor-defines were moved to CMakeLists.txt as compile flag
// This pre-processor flag determines whether to use ITK or VTK for writing
// ORA-XML (extended metaimage) files.
//#define USE_ITK_FOR_WRITING
// This pre-processor flag determines whether to use XRawImageIO for both
// reading and writing instead of the standard ITK/VTK implementations
// (preferably yes!)
//#define USE_XRAWIO_FOR_ALL
// This pre-processor flag determines whether to use fast or default compression
// for XRawImageIO (preferably yes!)
//#define FAST_XIO_COMPRESSION

#include <typeinfo>
#include <fstream>

#if defined(USE_XRAWIO_FOR_ALL)
  #include "oraXRawImageIO.h"
#endif
#if !defined(USE_ITK_FOR_WRITING) && !defined(USE_XRAWIO_FOR_ALL)
  #include "vtkMetaImageWriter2.h"
#endif
#include "oraITKVTKMetaInformationXMLFile.h"

// ORAIFTools
#include "oraSimpleMacros.h"
#include "SimpleDebugger.h"
#include "oraStringTools.h"

#include <itkVTKImageExport.h>
#include <itkVTKImageImport.h>
#include <itkImageFileReader.h>
#include <itksys/SystemTools.hxx>
#if defined(USE_ITK_FOR_WRITING) || defined(USE_XRAWIO_FOR_ALL)
  #include <itkImageFileWriter.h>
#endif

#include <vtkImageImport.h>
#include <vtkImageExport.h>
#include <vtkMath.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#if !defined(USE_ITK_FOR_WRITING) && !defined(USE_XRAWIO_FOR_ALL)
  #include <vtkmetaio/metaImage.h>
#endif


namespace ora 
{


// constant initialization
const std::string
ITKVTKImage
::ORA_EXTENDED_META_IMAGE_EXTENSION = "ora.xml";


void
ITKVTKImage
::GetImageTypes(const std::string fileName,
    ITKPixelType &pixelType, ITKComponentType &componentType)
{
  typedef itk::Image<unsigned char, 3> ImageType;
  itk::ImageFileReader<ImageType>::Pointer imageReader = itk::ImageFileReader<
      ImageType>::New();
  imageReader->SetFileName(fileName.c_str());
  try
  {
    imageReader->UpdateOutputInformation();

    pixelType = imageReader->GetImageIO()->GetPixelType();
    componentType = imageReader->GetImageIO()->GetComponentType();
    // TODO: Maybe include dimension too
    // imageReader->GetImageIO()->GetNumberOfDimensions()
  }
  catch (itk::ExceptionObject &e)
  {
    SimpleDebugger deb;
    InformativeDebugMacro2Object(deb, << "ERROR reading ITK image types: "
        << fileName << " - " << e)
    pixelType = itk::ImageIOBase::UNKNOWNPIXELTYPE;
    componentType = itk::ImageIOBase::UNKNOWNCOMPONENTTYPE;
  }
}

ITKVTKImage *
ITKVTKImage
::LoadImageFromFile(const std::string fileName, IOType loadType)
{
  // TODO: Encapsulate in try-catch blocks for error handling
  // TODO: Add forceComponentType argument to force a certain type

  ITKVTKImage *ivi = NULL;
  ITKPixelType pixelType;
  ITKComponentType componentType;

  switch (loadType) {
    case ITKIO:
      // Get type information from file
      GetImageTypes(fileName, pixelType, componentType);

      // Read mhd file (call template function based on component type)
      TEMPLATE_CALL_COMP(componentType, ivi = LoadFromOrdinaryImageUsingITK,
          fileName, pixelType, componentType)
      break;
    case VTKIO:
      // Get type information from file
      GetImageTypes(fileName, pixelType, componentType);

      // Call template function based on component type
      // TODO: Currently type is read with ITK dummy reader. Maybe use VTK type information.
      TEMPLATE_CALL_COMP(componentType, ivi = LoadFromOrdinaryImageUsingVTK,
          fileName, pixelType, componentType)
      break;
    case ORAIO:
      ivi = LoadFromORAMetaImage(fileName, false);
      break;
    default:
      break;
  }
  return ivi;
}


ITKVTKImage *
ITKVTKImage
::LoadFromORAMetaImage(const std::string fileName, bool ignoreHash,
    bool forceOrientationFromMetaInfo)
{
  if (!itksys::SystemTools::FileExists(fileName.c_str(), true))
    return NULL;

  // first try to read the ORA-XML meta data file:
  ora::ITKVTKMetaInformationXMLFileReader::Pointer readerXML =
    ora::ITKVTKMetaInformationXMLFileReader::New();
  readerXML->SetFilename(fileName);
  readerXML->SetIgnoreInvalidHash(ignoreHash);
  ora::ITKVTKImageMetaInformation::Pointer mi = NULL;
  if (readerXML->CanReadFile(fileName.c_str()))
  {
    try
    {
      readerXML->Update();
      mi = readerXML->GetOutputObject();
    }
    catch (...)
    {
      mi = NULL;
      SimpleDebugger deb;
      InformativeDebugMacro2Object(deb, << "ERROR (ORA-XML meta reading): " <<
          "Unknown error.")
    }
  }

  ITKVTKImage *ivi = NULL;

  if (mi) // meta information available
  {
    VolumeMetaInformation::Pointer vi = mi->GetVolumeMetaInfo();
    if (vi)
    {
      // set full file name:
      vi->SetFullFileName(fileName);

      // now try to read the raw image information:
      std::string mhdFile = vi->GetMHDFileName(); // is expected to be relative
      std::string path = itksys::SystemTools::GetFilenamePath(fileName);
      if (path.length() > 0) // ensure that path ends with file separator
        EnsureStringEndsWith(path, "/");
      // Force MHD file to be in same directory as ORA.XML file.
      // Ignore absolute path from ORA.XML-MHDFileName.
      mhdFile = path + itksys::SystemTools::GetFilenameName(mhdFile);

      // Get type information from file
      ITKPixelType pixelType;
      ITKComponentType componentType;
      GetImageTypes(mhdFile, pixelType, componentType);

      if (forceOrientationFromMetaInfo)
      {
        // EXTENSION for FIXING A PREVIOUS BUG (2011-03-02): check whether
        // image direction of XML file and the MHD header are equal; if they're
        // unequal, trust the XML file and take it over -> re-write the original
        // MHD FILE!

        VolumeMetaInformation::DirectionType midir = mi->
            GetVolumeMetaInfo()->GetDirection();

        bool doCorrection = false;
        std::vector<std::string> mhdLines;

        // read metadata file for more information:
        std::ifstream ifs(mhdFile.c_str());
        std::string istri((std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>());
        std::vector<std::string> rawlines;
        Tokenize(istri, rawlines, "\n");
        for (std::size_t k = 0; k < rawlines.size(); k++)
        {
          istri = rawlines[k];
          ReplaceString(istri, "\r", "");
          rawlines[k] = istri;
        }
        ifs.close();

        for (std::size_t k = 0; k < rawlines.size(); k++)
        {
          std::string ln = rawlines[k];
          std::size_t p = ln.find('=', 0);
          if (p != std::string::npos)
          {
            std::string key = ln.substr(0, p);
            Trim(key);
            ToLowerCase(key);
            std::string value = ln.substr(p + 1, ln.length());
            Trim(value);
            // supported entries:
            if (key == "transformmatrix")
            {
              std::vector<std::string> toks;
              Tokenize(value, toks, " ");
              if (toks.size() == 9)
              {
                int c = 0;
                double f = 0;
                for (int i1 = 0; i1 < 3; i1++)
                {
                  for (int i2 = 0; i2 < 3; i2++)
                  {
                    f = atof(toks[c].c_str());
                    if (fabs(f - midir[i1][i2]) > 1e-6)
                    {
                      mhdLines.push_back("---xxx---");
                      doCorrection = true;
                      break;
                    }
                    c++;
                  }
                  if (doCorrection)
                    break;
                }
              }
              else
              {
                doCorrection = true;
              }
            }
            else
            {
              mhdLines.push_back(ln);
            }
          }
        }

        if (doCorrection)
        {
          std::ofstream outf;
          outf.open(mhdFile.c_str(), std::ios::out);
          if (outf.is_open())
          {
            // MHD header lines:
            for (std::size_t x = 0; x < mhdLines.size(); x++)
            {
              if (mhdLines[x] != std::string("---xxx---"))
              {
                outf << mhdLines[x] << std::endl;
              }
              else
              {
                // -> add corrected transformation matrix (orientation):
                std::string corrected = "";
                char buff[2048];
                sprintf(buff, "%.10f %.10f %.10f %.10f %.10f %.10f %.10f %.10f %.10f",
                    midir[0][0], midir[0][1], midir[0][2],
                    midir[1][0], midir[1][1], midir[1][2],
                    midir[2][0], midir[2][1], midir[2][2]);
                corrected = std::string(buff);
                outf << "TransformMatrix = " << corrected << std::endl;
              }
            }
            outf.close();
          }
        }
      }

      // Read mhd file
      TEMPLATE_CALL_COMP(componentType, ivi = LoadFromOrdinaryImageUsingITK,
          mhdFile, pixelType, componentType)

      if (ivi) // itk image successfully read
      {
        ivi->SetMetaInfo(mi); // set meta info
      }
    }
  }

  return ivi;
}

ITKVTKImage
::ITKVTKImage(ITKPixelType pt, ITKComponentType ct)
{
  this->m_PixelType = pt;
  this->m_ComponentType = ct;
  this->m_ITKImage = NULL;
  this->m_VTKImage = NULL;
  this->m_MetaInfo = NULL;
}


ITKVTKImage
::~ITKVTKImage()
{
  this->m_ITKImage = NULL;
  if (this->m_VTKImage)
    this->m_VTKImage->Delete();
  this->m_VTKImage = NULL;
  this->m_MetaInfo = NULL;
}


void
ITKVTKImage
::SetITKImage(ITKImagePointer itkImage, bool updateVTKImageImmediately)
{
  // TODO: To set a different image may interfere with the types and the image object
  // TODO: Disable setting the image data after object creation (by passing it to the constructor)
  if (m_VTKImage)
    m_VTKImage->Delete();
  m_VTKImage = NULL; // set back

  m_ITKImage = NULL;
  m_ITKImage = itkImage;
  if (m_ITKImage)
  {
    m_ITKImage->Update();
    m_ITKImage->DisconnectPipeline();

    // simply call VTK-getter -> connects VTK-pipeline with ITK-pipeline
    if (updateVTKImageImmediately)
      TEMPLATE_CALL_COMP(this->GetComponentType(), GetAsVTKImage, )
  }
}

void
ITKVTKImage
::SetVTKImage(VTKImagePointer vtkImage, bool updateITKImageImmediately)
{
  // TODO: To set a different image may interfere with the types and the image object
  // TODO: Disable setting the image data after object creation (by passing it to the constructor)
  m_ITKImage = NULL; // set back

  if (m_VTKImage)
    m_VTKImage->Delete();
  m_VTKImage = NULL;

  if (vtkImage)
  {
    m_VTKImage = VTKImageType::New();
    vtkImage->Update();
    m_VTKImage->ShallowCopy(vtkImage);

    // simply call ITK-getter -> connects ITK-pipeline with VTK-pipeline
    if (updateITKImageImmediately)
      TEMPLATE_CALL_COMP(this->GetComponentType(), GetAsITKImage, )
  }
}


}


