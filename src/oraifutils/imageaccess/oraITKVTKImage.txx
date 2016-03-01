
#ifndef ORAITKVTKIMAGE_TXX_
#define ORAITKVTKIMAGE_TXX_

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

// #FIXME: CAUSES COMPILATION ERROR: COMPILE_FLAGS "${UNDEFINE_FLAGS} -DUSE_XRAWIO_FOR_ALL -DFAST_XIO_COMPRESSION")

#include "oraITKVTKImage.h"

#include <typeinfo>

#if defined(USE_XRAWIO_FOR_ALL)
  #include "oraXRawImageIO.h"
#endif
#if !defined(USE_ITK_FOR_WRITING) && !defined(USE_XRAWIO_FOR_ALL)
  #include "vtkMetaImageWriter2.h"
#endif
#include "oraITKVTKMetaInformationXMLFile.h"

// ORAIFTools
#include "oraStringTools.h"
#include "oraSimpleMacros.h"
#include "SimpleDebugger.h"

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
#include <vtkType.h>
#if !defined(USE_ITK_FOR_WRITING) && !defined(USE_XRAWIO_FOR_ALL)
  #include <vtkmetaio/metaImage.h>
#endif


namespace ora 
{


template <typename TComponentType>
itk::ImageIOBase::IOComponentType
ITKVTKImage
::GetComponentTypeFromTemplate()
{
  const std::type_info *type = &typeid(TComponentType);

  if (*type == typeid(unsigned char))
    return itk::ImageIOBase::UCHAR;
  else if (*type == typeid(char))
    return itk::ImageIOBase::CHAR;
  else if (*type == typeid(unsigned short))
    return itk::ImageIOBase::USHORT;
  else if (*type == typeid(short))
    return itk::ImageIOBase::SHORT;
  else if (*type == typeid(unsigned int))
    return itk::ImageIOBase::UINT;
  else if (*type == typeid(int))
    return itk::ImageIOBase::INT;
  else if (*type == typeid(unsigned long))
    return itk::ImageIOBase::ULONG;
  else if (*type == typeid(long))
    return itk::ImageIOBase::LONG;
  else if (*type == typeid(float))
    return itk::ImageIOBase::FLOAT;
  else if (*type == typeid(double))
    return itk::ImageIOBase::DOUBLE;
  else
    return itk::ImageIOBase::UNKNOWNCOMPONENTTYPE;
}


template <typename TComponentType>
ITKVTKImage *
ITKVTKImage
::LoadFromOrdinaryImageUsingITK(const std::string fileName, const ITKPixelType &pt, const ITKComponentType &ct)
{
  if (!itksys::SystemTools::FileExists(fileName.c_str(), true))
    return NULL;

  // ITK image type
  typedef itk::Image<TComponentType, ITKVTKImage::Dimensions> InputImageType;
  typedef typename InputImageType::Pointer InputImageTypePointer;

  typedef itk::ImageFileReader<InputImageType> ReaderType;
  typedef typename ReaderType::Pointer ReaderPointer;

  ReaderPointer reader = ReaderType::New();
  InputImageTypePointer inputImage = NULL;

  reader->SetFileName(fileName.c_str());
  try
  {
    reader->Update(); // read ITK-image incorporating generic object-factory
    inputImage = reader->GetOutput();
  }
  catch (itk::ExceptionObject &e)
  {
    SimpleDebugger deb;
    InformativeDebugMacro2Object(deb, << "ERROR reading ITK-image: " << e)
    inputImage = NULL;
  }
  reader = NULL;

  ITKVTKImage *ivi = NULL;

  if (inputImage) // image has successfully been read -> create minimal meta-object
  {
    ivi = new ITKVTKImage(pt, ct); // create image
    // Cast image to base type
    ITKVTKImage::ITKImagePointer ip = static_cast<ITKVTKImage::ITKImageType *>(inputImage.GetPointer());
    ivi->SetITKImage(ip, false); // set image

    ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
    // extract minimal meta-info from ITK-image
    ExtractMinimalMetaInfoFromITKImage<TComponentType>(ip, mi);
    // set full file name
    mi->GetVolumeMetaInfo()->SetFullFileName(fileName);
    // Set mhd filename if filetype is mhd
    if (  itksys::SystemTools::GetFilenameLastExtension(fileName) == ".mhd")
      mi->GetVolumeMetaInfo()->SetMHDFileName(fileName);
    ivi->SetMetaInfo(mi);

    inputImage = NULL;
    ip = NULL;
    mi = NULL;
  }

  return ivi;
}


template <typename TComponentType>
void
ITKVTKImage
::ExtractMinimalMetaInfoFromITKImage(ITKImagePointer &ip,
    ITKVTKImageMetaInformation::Pointer mi)
{
  if (!mi || !ip)
    return;

  // Cast to original ITK image type
  typedef itk::Image<TComponentType, ITKVTKImage::Dimensions> InputImageType;
  typedef typename InputImageType::Pointer InputImageTypePointer;
  typename InputImageType::Pointer i = static_cast<InputImageType * >(ip.GetPointer());

  mi->SetFileVersion(ITKVTKImageMetaInformation::HOTTEST_FILE_VERSION);
  VolumeMetaInformation::Pointer vi = mi->GetVolumeMetaInfo();
  if (vi)
  {
    VolumeMetaInformation::DirectionType dir;
    typename InputImageType::DirectionType idir = i->GetDirection();
    for (int x = 0; x < 3; x++)
      for (int y = 0; y < 3; y++)
        dir[x][y] = idir[x][y];
    vi->SetDirection(dir);

    VolumeMetaInformation::PointType orig;
    typename InputImageType::PointType iorig = i->GetOrigin();
    for (int x = 0; x < 3; x++)
      orig[x] = iorig[x];
    vi->SetOrigin(orig);

    VolumeMetaInformation::SpacingType spac;
    typename InputImageType::SpacingType ispac = i->GetSpacing();
    for (int x = 0; x < 3; x++)
      spac[x] = ispac[x];
    vi->SetSpacing(spac);

    VolumeMetaInformation::SizeType sz;
    typename InputImageType::SizeType isz = i->GetLargestPossibleRegion().
      GetSize();
    for (int x = 0; x < 3; x++)
      sz[x] = isz[x];
    vi->SetSize(sz);

    vi->SetNumberOfComponents(i->GetNumberOfComponentsPerPixel());

    // -> compute anatomical descriptions of the main directions (DICOM LPS)
    char *orient = NULL;
    VolumeMetaInformation::StringArrayType orientation;
    for (int x = 0; x < 3; ++x)
    {
      double d[3];
      for (int y = 0; y < 3; y++)
        d[y] = i->GetDirection()[x][y];
      orient = GetOrientationFromVector(d, true);
      orientation[x] = std::string(orient);
      delete [] orient;
    }
    vi->SetAnatomicalOrientation(orientation);

    // slice information:
    std::vector<SliceMetaInformation::Pointer> *sis = mi->
    GetSlicesMetaInformation();
    if (sis)
    {
      // using slicing direction for slice origin calculation:
      double sd[3];
      for (int y = 0; y < 3; y++)
        sd[y] = i->GetDirection()[2][y];
      vtkMath::Normalize(sd); // normalized slicing direction
      double o[3]; // volume origin
      for (int y = 0; y < 3; y++)
        o[y] = orig[y];

      sis->clear();
      // add minimal slice information for each 'slice' in 3rd dimension:
      for (unsigned int x = 0; x < sz[2]; ++x)
      {
        SliceMetaInformation::Pointer si = SliceMetaInformation::New();

        SliceMetaInformation::PointType so;
        for (int y = 0; y < 3; y++)
          so[y] = o[y] + sd[y] * spac[2] * x;
        si->SetOrigin(so);

        sis->push_back(si);
      }
    }
  }
}

template <typename TComponentType>
ITKVTKImage *
ITKVTKImage
::LoadFromOrdinaryImageUsingVTK(const std::string fileName, const ITKPixelType &pt, const ITKComponentType &ct)
{
  if (!itksys::SystemTools::FileExists(fileName.c_str(), true))
    return NULL;

  typedef vtkImageReader2* VTKReaderPointer;
  typedef vtkImageData* VTKImagePointer;

  // NOTE: the capability of reading meta-images with the VTK-reader should
  // be globally registered (@see ora::ImageModel)
  VTKReaderPointer reader = vtkImageReader2Factory::CreateImageReader2(
      fileName.c_str());
  VTKImagePointer inputImage = NULL;

  if (reader) // appropriate reader found
  {
    reader->SetFileName(fileName.c_str());
    inputImage = reader->GetOutput();
    try
    {
      reader->Update();
    }
    catch (...)
    {
      inputImage = NULL;
      SimpleDebugger deb;
      InformativeDebugMacro2Object(deb, << "ERROR (VTK image reading): " <<
          "Unknown error.")
    }
  }
  else
  {
    SimpleDebugger deb;
    InformativeDebugMacro2Object(deb, << "Obviously there is no VTK-reader " <<
        "for " << fileName)
  }

  ITKVTKImage *ivi = NULL;

  // TODO: VTK type to ITK type? Get type from vtk image data without the ITK dummy reader
  // img->GetScalarType()
  // VTK_VOID            0
  // VTK_BIT             1
  // VTK_CHAR            2
  // VTK_SIGNED_CHAR    15
  // VTK_UNSIGNED_CHAR   3
  // VTK_SHORT           4
  // VTK_UNSIGNED_SHORT  5
  // VTK_INT             6
  // VTK_UNSIGNED_INT    7
  // VTK_LONG            8
  // VTK_UNSIGNED_LONG   9
  // VTK_FLOAT          10
  // VTK_DOUBLE         11
  // VTK_ID_TYPE        12
  // /* These types are enabled if VTK_TYPE_USE_LONG_LONG is defined.  */
  // VTK_LONG_LONG          16
  // VTK_UNSIGNED_LONG_LONG 17
  // /* This type is enabled if VTK_TYPE_USE___INT64 is defined.  */
  // VTK___INT64            18
  // /* This type is enabled if VTK_TYPE_USE___INT64 and
  // VTK_TYPE_CONVERT_UI64_TO_DOUBLE are both defined.  */
  // VTK_UNSIGNED___INT64   19

  if (inputImage) // image has successfully been read -> create minimal meta-object
  {
    // FIX to avoid signed char runtime error of itk::VTKImageExport when
    // VTK reader reads char pixel type as signed char
    if (inputImage->GetScalarType() == VTK_SIGNED_CHAR)
      inputImage->SetScalarType(VTK_CHAR);

    ivi = new ITKVTKImage(pt, ct); // create image
    ivi->SetVTKImage(inputImage, false); // set image

    // extract minimal meta-info from ITK-image
    ITKImagePointer image;
    TEMPLATE_CALL_COMP(ivi->GetComponentType(), image = ivi->GetAsITKImage, )

    ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
    ExtractMinimalMetaInfoFromITKImage<TComponentType>(image, mi);
    ivi->SetMetaInfo(mi);

    image = NULL;
    mi = NULL;
  }

  reader->Delete(); // delete here because SetVTKImage() internally calls Update()
  reader = NULL;

  return ivi;
}


template <typename TComponentType>
ITKVTKImage::ITKImagePointer
ITKVTKImage
::GetAsITKImage()
{
  if (m_ITKImage)
  {
    return m_ITKImage;
  }
  else
  {
    if (!m_VTKImage) // makes no sense without VTK image
      return NULL;

    // ITK image type
    typedef itk::Image<TComponentType, ITKVTKImage::Dimensions> InputImageType;
    typedef typename InputImageType::Pointer InputImageTypePointer;

    typedef vtkImageExport VTKExporterType;
    typedef itk::VTKImageImport<InputImageType> ITKImporterType;

    // try to connect ITK-pipeline to VTK-pipeline:
    VTKExporterType *vtkExporter = VTKExporterType::New();

    vtkExporter->SetInput(m_VTKImage);

    typename ITKImporterType::Pointer itkImporter = ITKImporterType::New();

    itkImporter->SetUpdateInformationCallback(
      vtkExporter->GetUpdateInformationCallback());
    itkImporter->SetPipelineModifiedCallback(
      vtkExporter->GetPipelineModifiedCallback());
    itkImporter->SetWholeExtentCallback(
      vtkExporter->GetWholeExtentCallback());
    itkImporter->SetSpacingCallback(
      vtkExporter->GetSpacingCallback());
    itkImporter->SetOriginCallback(
      vtkExporter->GetOriginCallback());
    itkImporter->SetScalarTypeCallback(
      vtkExporter->GetScalarTypeCallback());
    itkImporter->SetNumberOfComponentsCallback(
      vtkExporter->GetNumberOfComponentsCallback());
    itkImporter->SetPropagateUpdateExtentCallback(
      vtkExporter->GetPropagateUpdateExtentCallback());
    itkImporter->SetUpdateDataCallback(
      vtkExporter->GetUpdateDataCallback());
    itkImporter->SetDataExtentCallback(
      vtkExporter->GetDataExtentCallback());
    itkImporter->SetBufferPointerCallback(
      vtkExporter->GetBufferPointerCallback());
    itkImporter->SetCallbackUserData(
      vtkExporter->GetCallbackUserData());

    // Get output
    InputImageTypePointer img = itkImporter->GetOutput();
    itkImporter->Update(); // update !
    img->DisconnectPipeline();

    // Cast image to base type
    m_ITKImage = static_cast<ITKVTKImage::ITKImageType *>(img.GetPointer());

    itkImporter = NULL;
    vtkExporter->Delete();

    return m_ITKImage;
  }
}

template <typename TComponentType>
void
ITKVTKImage
::OverrideBasicImageGeometry(double spacing[3], double origin[3],
     double orientation[9])
{
  typedef itk::Image<TComponentType, ITKVTKImage::Dimensions> InputImageType;

  if (!m_ITKImage)
    GetAsITKImage<TComponentType>(); // generate representation on demand
  if (!m_ITKImage) // not available
    return;

  if (m_VTKImage) // be sure the VTK image is reset
    m_VTKImage->Delete();
  m_VTKImage = NULL; // set back

  typename InputImageType::Pointer i = static_cast<InputImageType *>(m_ITKImage.GetPointer());
  typename InputImageType::PointType iorig;
  iorig[0] = origin[0];
  iorig[1] = origin[1];
  iorig[2] = origin[2];
  i->SetOrigin(iorig);
  typename InputImageType::DirectionType iorient;
  iorient[0][0] = orientation[0]; iorient[0][1] = orientation[3]; iorient[0][2] = orientation[6];
  iorient[1][0] = orientation[1]; iorient[1][1] = orientation[4]; iorient[1][2] = orientation[7];
  iorient[2][0] = orientation[2]; iorient[2][1] = orientation[5]; iorient[2][2] = orientation[8];
  i->SetDirection(iorient);
  typename InputImageType::SpacingType isp;
  isp[0] = spacing[0];
  isp[1] = spacing[1];
  isp[2] = spacing[2];
  i->SetSpacing(isp);
}

template <typename TComponentType>
bool
ITKVTKImage
::ExtractBasicImageGeometry(double spacing[3], int size[3], double origin[3],
    double orientation[9])
{
  typedef itk::Image<TComponentType, ITKVTKImage::Dimensions> InputImageType;

  if (!m_ITKImage)
    GetAsITKImage<TComponentType>(); // generate representation on demand
  if (!m_ITKImage)
    return false;

  typename InputImageType::Pointer i = static_cast<InputImageType *>(m_ITKImage.GetPointer());
  typename InputImageType::PointType iorig = i->GetOrigin();
  typename InputImageType::DirectionType iorient = i->GetDirection();
  typename InputImageType::SizeType isz = i->GetLargestPossibleRegion().GetSize();
  typename InputImageType::SpacingType isp = i->GetSpacing();
  int d;
  for (d = 0; d < 3; d++)
  {
    spacing[d] = isp[d];
    size[d] = isz[d];
    origin[d] = iorig[d];
  }
  orientation[0] = iorient[0][0];
  orientation[1] = iorient[1][0];
  orientation[2] = iorient[2][0];
  orientation[3] = iorient[0][1];
  orientation[4] = iorient[1][1];
  orientation[5] = iorient[2][1];
  orientation[6] = iorient[0][2];
  orientation[7] = iorient[1][2];
  orientation[8] = iorient[2][2];
  return true; // ok, valid
}

template <typename TComponentType>
typename ITKVTKImage::VTKImagePointer
ITKVTKImage
::GetAsVTKImage()
{
  if (m_VTKImage)
  {
    return m_VTKImage;
  }
  else
  {
    if (!m_ITKImage) // makes no sense without ITK image
      return NULL;

    // ITK image type
    typedef itk::Image<TComponentType, ITKVTKImage::Dimensions> InputImageType;
    typedef typename InputImageType::Pointer InputImageTypePointer;
    // Cast to real ITK image type
    InputImageTypePointer img = static_cast<InputImageType * >(m_ITKImage.GetPointer());

    typedef itk::VTKImageExport<InputImageType> ITKExporterType;
    typedef vtkImageImport VTKImporterType;

    // try to connect VTK-pipeline to ITK-pipeline:
    typename ITKExporterType::Pointer itkExporter = ITKExporterType::New();
    itkExporter->SetInput(img);

    VTKImporterType *vtkImporter = VTKImporterType::New();

    vtkImporter->SetUpdateInformationCallback(
      itkExporter->GetUpdateInformationCallback());
    vtkImporter->SetPipelineModifiedCallback(
      itkExporter->GetPipelineModifiedCallback());
    vtkImporter->SetWholeExtentCallback(
      itkExporter->GetWholeExtentCallback());
    vtkImporter->SetSpacingCallback(
      itkExporter->GetSpacingCallback());
    vtkImporter->SetOriginCallback(
      itkExporter->GetOriginCallback());
    vtkImporter->SetScalarTypeCallback(
      itkExporter->GetScalarTypeCallback());
    vtkImporter->SetNumberOfComponentsCallback(
      itkExporter->GetNumberOfComponentsCallback());
    vtkImporter->SetPropagateUpdateExtentCallback(
      itkExporter->GetPropagateUpdateExtentCallback());
    vtkImporter->SetUpdateDataCallback(
      itkExporter->GetUpdateDataCallback());
    vtkImporter->SetDataExtentCallback(
      itkExporter->GetDataExtentCallback());
    vtkImporter->SetBufferPointerCallback(
      itkExporter->GetBufferPointerCallback());
    vtkImporter->SetCallbackUserData(
      itkExporter->GetCallbackUserData());

    vtkImporter->Update(); // update !

    m_VTKImage = vtkImageData::New();
    m_VTKImage->ShallowCopy(vtkImporter->GetOutput());

    vtkImporter->Delete();
    itkExporter = NULL;

    return m_VTKImage;
  }
}


template <typename TComponentType>
bool
ITKVTKImage
::SaveImageAsORAMetaImage(std::string fileName,
    bool compress, std::string intermediateDirectory)
{
  EnsureStringEndsWith(fileName, ".mhd");
  Trim(intermediateDirectory);

  if (fileName.length() < 5 || !m_MetaInfo)
    return false;

#if defined(USE_XRAWIO_FOR_ALL)
  if (compress)
  {
#if defined(FAST_XIO_COMPRESSION)
    fileName = fileName.substr(0, fileName.length() - 3) + "xmhd";
#else
    fileName = fileName.substr(0, fileName.length() - 3) + "zmhd";
#endif
  }
#endif

#if !defined(USE_ITK_FOR_WRITING) && !defined(USE_XRAWIO_FOR_ALL)
  vtkImageData *image = NULL;
  TEMPLATE_CALL_COMP(this->GetComponentType(), image = this->GetAsVTKImage, )
#endif
  ITKImagePointer itkBaseImage;
  TEMPLATE_CALL_COMP(this->GetComponentType(), itkBaseImage = this->GetAsITKImage, )
  // Cast to original ITK image type
  typedef itk::Image<TComponentType, ITKVTKImage::Dimensions> InputImageType;
  typedef typename InputImageType::Pointer InputImageTypePointer;
  InputImageTypePointer itkImage = static_cast<InputImageType * >(itkBaseImage.GetPointer());
  itkBaseImage = NULL;

#if !defined(USE_ITK_FOR_WRITING) && !defined(USE_XRAWIO_FOR_ALL)
  if (!image || !itkImage)
#else
  if (!itkImage)
#endif
    return false;

  // the VTK implementation is slightly faster than the ITK implementation.
  // (a modified version is used which makes the metaimage-object available)
  // however, the ITK implementation on the other hand seems to be more stable

  // EXTENSION (2010-07-01): XRawImageIO is really fast and (hopefully) more
  // reliable than both implementations!

#if defined(USE_ITK_FOR_WRITING) || defined(USE_XRAWIO_FOR_ALL)
  typename itk::ImageFileWriter<InputImageType>::Pointer w =
      itk::ImageFileWriter<InputImageType>::New();
#else
  vtkMetaImageWriter2 *w = vtkMetaImageWriter2::New();
#endif
#if defined(USE_XRAWIO_FOR_ALL)
  typename ora::XRawImageIO<TComponentType, Dimensions>::Pointer xio =
      ora::XRawImageIO<TComponentType, Dimensions>::New();
  xio->SetAutoExtensionMode(true);
  w->SetImageIO(xio); // set the extended IO (with AutoExtensionMode)
#endif

  if (intermediateDirectory.length() > 0) // intermediate buffer
  {
    std::string fn = intermediateDirectory +
        itksys::SystemTools::GetFilenameName(fileName);
    w->SetFileName(fn.c_str());
  }
  else
  {
    w->SetFileName(fileName.c_str());
  }

#ifdef USE_ITK_FOR_WRITING
  w->SetUseCompression(compress);
#else
  w->SetCompression(compress);
#endif
// NOTE: done above via file-naming!

  try
  {
#if defined(USE_ITK_FOR_WRITING) || defined(USE_XRAWIO_FOR_ALL)
    w->SetInput(itkImage);
#else
    w->SetInputAndInitializeMetaImage(image);
#endif

#if !defined(USE_ITK_FOR_WRITING)
    // manipulate VTK metaimage-object here:

    // - image direction (TransformMatrix)
    vtkmetaio::MetaImage *mi = w->GetMetaImagePtr();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        mi->TransformMatrix(i, j, itkImage->GetDirection()[i][j]);

    // - AnatomicalOrientation (just apply only the first character of each
    // component - metaimage format does not seem to support minor components)
    char *orient = NULL;
    for (int i = 0; i < 3; ++i)
    {
      double dir[3];
      for (int j = 0; j < 3; j++)
        dir[j] = itkImage->GetDirection()[i][j];
      orient = GetOrientationFromVector(dir, false);
      mi->AnatomicalOrientation(i, orient[0]);
      delete [] orient;
    }
    w->WriteMetaImage();
#endif

#if defined(USE_ITK_FOR_WRITING) || defined(USE_XRAWIO_FOR_ALL)
    w->Update();
#endif

#if defined(USE_XRAWIO_FOR_ALL)
    std::string mhdFN = fileName;
    if (compress)
      mhdFN = mhdFN.substr(0, mhdFN.length() - 4) + "mhd"; // back from ?mhd
    m_MetaInfo->GetVolumeMetaInfo()->SetMHDFileName(
              itksys::SystemTools::GetFilenameName(mhdFN));
#else
    m_MetaInfo->GetVolumeMetaInfo()->SetMHDFileName(
              itksys::SystemTools::GetFilenameName(fileName));
#endif

    std::string xmlFileName = fileName;
    std::string ext = itksys::SystemTools::GetFilenameExtension(xmlFileName);
    xmlFileName = xmlFileName.substr(0, xmlFileName.length() - ext.length());
    xmlFileName += "." + ORA_EXTENDED_META_IMAGE_EXTENSION;
    std::string usedXMLFileName = xmlFileName;
    if (intermediateDirectory.length() > 0) // intermediate buffer
    {
      usedXMLFileName = intermediateDirectory +
              itksys::SystemTools::GetFilenameName(xmlFileName);
    }
    if (!m_MetaInfo->WriteToXMLFile(usedXMLFileName))
    {
      SimpleDebugger deb;
      SimpleErrorMacro2Object(deb, << "Unknown error during ORA.XML " <<
          "writing (" << usedXMLFileName << ").");
#if !defined(USE_ITK_FOR_WRITING) && !defined(USE_XRAWIO_FOR_ALL)
      w->Delete();
#else
      w = NULL;
#endif
      return false;
    }

    if (intermediateDirectory.length() > 0) // intermediate buffer
    {
      // copy files to real destination
      bool succ = true;

      std::string fn = w->GetFileName();
#if defined(USE_XRAWIO_FOR_ALL)
      if (compress)
      {
        fn = fn.substr(0, fn.length() - 4) + "mhd"; // convert back from ?mhd
        fileName = fileName.substr(0, fileName.length() - 4) +
            "mhd"; // convert back from ?mhd
      }
#endif
      succ &= itksys::SystemTools::CopyFileAlways(fn.c_str(), fileName.c_str());

      if (compress)
      {
#if !defined(USE_XRAWIO_FOR_ALL)
        ReplaceString(fn, ".mhd", ".zraw");
        ReplaceString(fileName, ".mhd", ".zraw");
#else
#if defined(FAST_XIO_COMPRESSION)
        ReplaceString(fn, ".mhd", ".xraw");
        ReplaceString(fileName, ".mhd", ".xraw");
#else
        ReplaceString(fn, ".mhd", ".zraw");
        ReplaceString(fileName, ".mhd", ".zraw");
#endif
#endif
      }
      else
      {
        ReplaceString(fn, ".mhd", ".raw");
        ReplaceString(fileName, ".mhd", ".raw");
      }
      succ &= itksys::SystemTools::CopyFileAlways(fn.c_str(), fileName.c_str());
      succ &= itksys::SystemTools::CopyFileAlways(usedXMLFileName.c_str(),
          xmlFileName.c_str());
      // delete buffered files
#if defined(USE_XRAWIO_FOR_ALL)
      std::string realFN = std::string(w->GetFileName());
      if (compress)
        realFN = realFN.substr(0, realFN.length() - 4) + "mhd"; // back from ?mhd
      succ &= itksys::SystemTools::RemoveFile(realFN.c_str());
#else
      succ &= itksys::SystemTools::RemoveFile(w->GetFileName());
#endif
      succ &= itksys::SystemTools::RemoveFile(fn.c_str());
      succ &= itksys::SystemTools::RemoveFile(usedXMLFileName.c_str());
      if (!succ)
      {
        SimpleDebugger deb;
        SimpleErrorMacro2Object(deb, << "Error during copying / deleting " <<
            "from intermediate file buffer.");
#if !defined(USE_ITK_FOR_WRITING) && !defined(USE_XRAWIO_FOR_ALL)
        w->Delete();
#else
        w = NULL;
#endif
        return false;
      }
    }
  }
  catch (...)
  {
    SimpleDebugger deb;
    SimpleErrorMacro2Object(deb, << "Unknown error during MHD " <<
      "writing (" << w->GetFileName() << ").");
#if !defined(USE_ITK_FOR_WRITING) && !defined(USE_XRAWIO_FOR_ALL)
    w->Delete();
#else
    w = NULL;
#endif
    return false;
  }

#if !defined(USE_ITK_FOR_WRITING) && !defined(USE_XRAWIO_FOR_ALL)
  w->Delete();
#else
  w = NULL;
#endif

  return true;
}


}

#endif /* ORAITKVTKIMAGE_TXX_ */

