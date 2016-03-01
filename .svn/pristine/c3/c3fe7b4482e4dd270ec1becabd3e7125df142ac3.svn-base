#ifndef ORAIMAGEORIENTER_TXX_
#define ORAIMAGEORIENTER_TXX_

#include "oraImageOrienter.h"

#include <fstream>
#define _USE_MATH_DEFINES  // For MSVC
#include <math.h>

// ORAIFTools
#include "oraIniAccess.h"
#include "oraStringTools.h"
// ORAIFRegistration
#include "oraSetupError.h"
// ORAIFImageAccess
#include "oraFrameOfReference.h"
#include "oraRTIToMHDConverter.h"
#include "oraXRawImageIO.h"

#include <itksys/SystemTools.hxx>
#include <itkTranslationTransform.h>
#include <itkChangeInformationImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkVersorRigid3DTransform.h>
#include <itkEuler3DTransform.h>
#include <itkMath.h>

#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

// Forward declarations
#include "oraQProgressEmitter.h"
#include "oraITKVTKImage.h" 


namespace ora 
{

template<typename TPixelType>
ImageOrienter<TPixelType>::ImageOrienter() :
  SimpleDebugger()
{
  m_FramesInfo = "";
  m_SetupErrorInfo = "";
}

template<typename TPixelType>
ImageOrienter<TPixelType>::~ImageOrienter()
{

}

template<typename TPixelType>
typename ImageOrienter<TPixelType>::ImageType *ImageOrienter<TPixelType>::ConvertVolumeObject(
    std::string volumeFile, double *isocenter)
{
  // initial checks:
  if (m_FramesInfo.length() > 0 && !itksys::SystemTools::FileExists(
      m_FramesInfo.c_str()))
  {
    SimpleErrorMacro(<< "Invalid FramesInfo file.")
    return false;
  }
  if (!itksys::SystemTools::FileExists(volumeFile.c_str()))
  {
    SimpleErrorMacro(<< "Invalid source volume file.")
    return false;
  }

  bool useITKReader = true;
  if (ToUpperCaseF(itksys::SystemTools::GetFilenameExtension(volumeFile))
      == ".ORA.XML")
    useITKReader = false; // appears to be ora-specific

  ImageType *dataSet = NULL;
  if (!useITKReader)
    dataSet = ImageType::LoadFromORAMetaImage(volumeFile, true);
  else
    dataSet = ImageType::LoadImageFromFile(volumeFile, ImageType::ITKIO);
  if (!dataSet)
  {
    SimpleErrorMacro(<< "ERROR: Data set could not be read!\n")
    return NULL;
  }

  FrameOfReference::Pointer fraOfRef = NULL;
  // Get ITK image
  ImageType::ITKImagePointer itkBaseImage;
  TEMPLATE_CALL_COMP(dataSet->GetComponentType(), itkBaseImage = dataSet->GetAsITKImage, )
  // Cast to original ITK image type
  typedef itk::Image<TPixelType, ITKVTKImage::Dimensions> InputImageType;
  typedef typename InputImageType::Pointer InputImageTypePointer;
  InputImageTypePointer itkImage = static_cast<InputImageType * >(itkBaseImage.GetPointer());
  itkBaseImage = NULL;

  typename InputImageType::PointType itkOrigin;
  typename InputImageType::DirectionType itkDirection;
  itkOrigin = itkImage->GetOrigin();
  itkDirection = itkImage->GetDirection();
  if (m_FramesInfo.length() > 0) // optionally load FORs
  {
    FrameOfReferenceCollection::Pointer forColl =
        FrameOfReferenceCollection::CreateFromFile(m_FramesInfo);
    dataSet->GetMetaInfo()->SetFORColl(forColl);
    fraOfRef
        = dataSet->GetMetaInfo()-> GetVolumeMetaInfo()->GetFrameOfReference();
  }

  FrameOfReference::TransformType::ConstPointer itkTransform = NULL;
  if (fraOfRef)
    itkTransform = fraOfRef->GetAffineTransform();
  typedef itk::TranslationTransform<double, 3> TranslTransformType;
  TranslTransformType::Pointer isoTransform = TranslTransformType::New();
  TranslTransformType::OutputVectorType isoTransl;
  isoTransl[0] = isocenter[0] * 10. * -1;
  isoTransl[1] = isocenter[1] * 10. * -1;
  isoTransl[2] = isocenter[2] * 10. * -1;
  isoTransform->Translate(isoTransl, false);
  if (itkTransform)
    itkOrigin = itkTransform->TransformPoint(itkOrigin);
  itkOrigin = isoTransform->TransformPoint(itkOrigin);
  FrameOfReference::GenericTransformType::OutputVectorType vec;
  for (int d = 0; d < 3; d++)
  {
    vec[0] = itkDirection[0][d]; // column-based!
    vec[1] = itkDirection[1][d];
    vec[2] = itkDirection[2][d];
    if (itkTransform)
      vec = itkTransform->TransformVector(vec);
    vec = isoTransform->TransformVector(vec); // normally no effect
    itkDirection[0][d] = vec[0];
    itkDirection[1][d] = vec[1];
    itkDirection[2][d] = vec[2];
  }
  // -> take over orientation / pose changes:
  typedef itk::ChangeInformationImageFilter<InputImageType>
      ChangerType;
  typename ChangerType::Pointer changer = ChangerType::New();
  changer->SetInput(itkImage);
  changer->ChangeNone();
  changer->ChangeOriginOn();
  changer->SetOutputOrigin(itkOrigin);
  changer->ChangeDirectionOn();
  changer->SetOutputDirection(itkDirection);
  try
  {
    changer->Update();
  }
  catch (itk::ExceptionObject& e)
  {
    std::cerr << "ERROR during orienting: " << e << std::endl;
    delete dataSet;
    return NULL;
  }
  // Set the image data (up-cast to base-class)
  InputImageTypePointer changedImage = changer->GetOutput();
  ITKVTKImage::ITKImagePointer ip = static_cast<ITKVTKImage::ITKImageType *>(changedImage.GetPointer());
  dataSet->SetITKImage(ip, false); // set image: (ITK -> VTK if necessary)
  dataSet->GetMetaInfo()->GetVolumeMetaInfo()->SetOrigin(itkOrigin);
  dataSet->GetMetaInfo()->GetVolumeMetaInfo()->SetDirection(itkDirection);

  return dataSet;
}

template<typename TPixelType>
bool ImageOrienter<TPixelType>::ConvertVolume(std::string volumeFile,
    std::string beamInfoFile, std::string outputFile,
    std::string outputInfoFile, QProgressEmitter *progress, double *isocenter)
{
  VeryDetailedDebugMacro(<< "Start orientation of volume '" << volumeFile <<
      "' (beam-info: '" << beamInfoFile << "', output: '" <<
      outputFile << "')")

  // initial checks:
  if (!itksys::SystemTools::FileExists(m_FramesInfo.c_str()))
  {
    SimpleErrorMacro(<< "Invalid FramesInfo file.")
    return false;
  }
  if (!itksys::SystemTools::FileExists(volumeFile.c_str()))
  {
    SimpleErrorMacro(<< "Invalid source volume file.")
    return false;
  }
  if (!itksys::SystemTools::FileExists(beamInfoFile.c_str()) && !isocenter)
  {
    SimpleErrorMacro(<< "Invalid BeamInfo file / isocenter.")
    return false;
  }
  if (isocenter)
    outputInfoFile = ""; // auto-setback

  bool success = true;

  if (progress)
    progress->SetProgress(0);

  ImageType *dataSet = ImageType::LoadFromORAMetaImage(volumeFile,
      true); // ignore hash in this scientific scenario!
  if (!dataSet)
  {
    SimpleErrorMacro(<< "ERROR: Data set could not be read!\n")
    success = false;
  }

  if (success)
  {
    // load FORs
    FrameOfReferenceCollection::Pointer forColl =
        FrameOfReferenceCollection::CreateFromFile(m_FramesInfo);
    dataSet->GetMetaInfo()->SetFORColl(forColl);

    // Generate oriented ITK-image
    ImageType::ITKImagePointer itkBaseImage;
    TEMPLATE_CALL_COMP(dataSet->GetComponentType(), itkBaseImage = dataSet->GetAsITKImage, )
    // Cast to original ITK image type
    typedef itk::Image<TPixelType, ITKVTKImage::Dimensions> InputImageType;
    typedef typename InputImageType::Pointer InputImageTypePointer;
    InputImageTypePointer itkImage = static_cast<InputImageType * >(itkBaseImage.GetPointer());
    itkBaseImage = NULL;

    typename InputImageType::PointType itkOrigin;
    typename InputImageType::DirectionType itkDirection;
    itkOrigin = itkImage->GetOrigin();
    itkDirection = itkImage->GetDirection();
    FrameOfReference::Pointer fraOfRef =
        dataSet->GetMetaInfo()-> GetVolumeMetaInfo()->GetFrameOfReference();
    FrameOfReference::TransformType::ConstPointer itkTransform =
        fraOfRef-> GetAffineTransform();
    std::string forUID = fraOfRef->GetUID();
    IniAccess forFile(m_FramesInfo);
    std::string forDef = forFile.ReadString("FramesOfReference", forUID, "");
    typedef itk::TranslationTransform<double, 3> TranslTransformType;
    TranslTransformType::Pointer isoTransform = TranslTransformType::New();
    TranslTransformType::OutputVectorType isoTransl;
    double ic[3];
    std::string beamUID = "";
    std::string planUID = "";
    if (!isocenter) // and: beamInfoFile.length() > 0
    {
      IniAccess bi(beamInfoFile);
      ic[0] = bi.ReadValue<double> ("Beam", "BmIsoCtrX", 0);
      ic[1] = bi.ReadValue<double> ("Beam", "BmIsoCtrY", 0);
      ic[2] = bi.ReadValue<double> ("Beam", "BmIsoCtrZ", 0);
      beamUID = bi.ReadString("Beam", "BmUID", "");
      planUID = bi.ReadString("Plan", "PlnUID", "");
    }
    else // if (isocenter)
    {
      ic[0] = isocenter[0];
      ic[1] = isocenter[1];
      ic[2] = isocenter[2];
    }
    isoTransl[0] = ic[0] * 10. * -1;
    isoTransl[1] = ic[1] * 10. * -1;
    isoTransl[2] = ic[2] * 10. * -1;
    isoTransform->Translate(isoTransl, false);
    itkOrigin = itkTransform->TransformPoint(itkOrigin);
    itkOrigin = isoTransform->TransformPoint(itkOrigin);
    FrameOfReference::GenericTransformType::OutputVectorType vec;
    for (int d = 0; d < 3; d++)
    {
      vec[0] = itkDirection[0][d]; // column-based!
      vec[1] = itkDirection[1][d];
      vec[2] = itkDirection[2][d];
      vec = itkTransform->TransformVector(vec);
      vec = isoTransform->TransformVector(vec); // normally no effect
      itkDirection[0][d] = vec[0];
      itkDirection[1][d] = vec[1];
      itkDirection[2][d] = vec[2];
    }
    typedef itk::ChangeInformationImageFilter<InputImageType> ChangerType;
    typename ChangerType::Pointer changer = ChangerType::New();
    changer->SetInput(itkImage);
    changer->ChangeNone();
    changer->ChangeOriginOn();
    changer->SetOutputOrigin(itkOrigin);
    changer->ChangeDirectionOn();
    changer->SetOutputDirection(itkDirection);
    changer->Update();

    if (progress)
      progress->SetProgress(0.5);

    // make output directory:
    std::string outdir = itksys::SystemTools::GetFilenamePath(outputFile);
    if (!itksys::SystemTools::MakeDirectory(outdir.c_str()))
    {
      SimpleErrorMacro(<< "ERROR: Output directory could not be created: " <<
          outdir)
      success = false;
    }

    // write output file:
    if (success)
    {
      typedef itk::Image<unsigned short, 3> USHORTImageType;
      typedef itk::CastImageFilter<InputImageType,
          USHORTImageType> CasterType;
      typename CasterType::Pointer caster = CasterType::New();
      caster->SetInput(changer->GetOutput());
      caster->Update();

      typedef itk::ImageFileWriter<USHORTImageType> WriterType;
      WriterType::Pointer w = WriterType::New();

      if (itksys::SystemTools::GetFilenameLastExtension(outputFile) == ".xmhd")
      {
        typedef ora::XRawImageIO<USHORTImageType::PixelType,
            USHORTImageType::ImageDimension> XIOType;
        typename XIOType::Pointer xio = XIOType::New();
        xio->SetAutoExtensionMode(true);
        w->SetImageIO(xio); // set the extended IO (with AutoExtensionMode)
      }

      w->SetInput(caster->GetOutput());
      w->SetFileName(outputFile.c_str());
      w->SetUseCompression(true);
      try
      {
        w->Update();
      }
      catch (itk::ExceptionObject &e)
      {
        success = false;
        SimpleErrorMacro(<< "ERROR: Could not write oriented volume: " << e)
      }
      if (progress)
        progress->SetProgress(0.9);
      // optional output info file:
      if (success && outputInfoFile.length() > 0)
      {
        outdir = itksys::SystemTools::GetFilenamePath(outputInfoFile);
        if (!itksys::SystemTools::MakeDirectory(outdir.c_str()))
        {
          SimpleErrorMacro(<< "ERROR: Output directory for info file " <<
              "could not be created: " << outdir)
          success = false;
        }
        if (success)
        {
          std::string anonVolumeFile = volumeFile; // -> anonymize
          // search for a suspicious path-part ("*-##############-*) ... could
          // be patient UID:
          std::vector<std::string> pathParts;
          std::string sep = DSEP;
          Tokenize(anonVolumeFile, pathParts, sep);
          for (std::size_t i = 0; i < pathParts.size(); i++)
          {
            std::size_t p1, p2 = std::string::npos;
            if ((p1 = pathParts[i].find("-")) != std::string::npos)
              p2 = pathParts[i].find("-", p1 + 1);
            if (p1 != std::string::npos && p2 != std::string::npos) // susp.
            {
              if (IsStrictlyNumeric(pathParts[i].substr(p1 + 1, p2 - p1 - 1)))
              {
                // ok, a bad guy - replace it by some dummy
                ReplaceString(anonVolumeFile, pathParts[i], "anonymized");
              }
            }
          }
          IniAccess outinfo(outputInfoFile);
          std::string timestamp;
          CurrentORADateTimeString(timestamp);
          outinfo.WriteString("Info", "TimeStamp", timestamp);
          outinfo.WriteString("Info", "SourceVolume", anonVolumeFile);
          outinfo.WriteString("Info", "OrientedVolume", outputFile);
          outinfo.WriteString("Info", "FORUID", forUID);
          outinfo.WriteString("Info", "FORDefinition", forDef);
          outinfo.WriteString("Info", "PlanUID", planUID);
          outinfo.WriteString("Info", "BeamUID", beamUID);
          outinfo.WriteValue<double> ("Info", "OriginalIsocenterXcm", ic[0]);
          outinfo.WriteValue<double> ("Info", "OriginalIsocenterYcm", ic[1]);
          outinfo.WriteValue<double> ("Info", "OriginalIsocenterZcm", ic[2]);
          outinfo.Update();
        }
      }
    }
  }
  if (progress)
    progress->SetProgress(1.0);
  VeryDetailedDebugMacro(<< "Finish orientation of volume '" << volumeFile <<
      "' --> SUCCESS = " << success << ")")

  return success;
}

template<typename TPixelType>
bool ImageOrienter<TPixelType>::ConvertKVImages(std::string viewFolder,
    std::string outputFolder, std::string outputInfoFile,
    QProgressEmitter *progress, std::string configTemplate)
{
  VeryDetailedDebugMacro(<< "Start conversion of kVs in '" << viewFolder <<
      "' (setup-errors: '" << m_SetupErrorInfo << "', output: '" <<
      outputFolder << "')")

  // initial checks:
  if (!itksys::SystemTools::FileExists(viewFolder.c_str(), false))
  {
    SimpleErrorMacro(<< "Invalid view folder.")
    return false;
  }
  if (!itksys::SystemTools::FileExists(m_SetupErrorInfo.c_str(), true))
  {
    SimpleErrorMacro(<< "Invalid setup-errors file.")
    return false;
  }
  bool produceConfigs = false;
  if (configTemplate.length() > 0 && !itksys::SystemTools::FileExists(
      m_SetupErrorInfo.c_str(), true))
  {
    InformativeDebugMacro(<< "Warning: TMR configuration template specified, "
        << "but not accessible (" << configTemplate << "). However, going " <<
        "to resume!")
  }
  else if (configTemplate.length() > 0 && itksys::SystemTools::FileExists(
      m_SetupErrorInfo.c_str(), true))
  {
    produceConfigs = true;
  }

  bool success = true;

  QDir kVDir(QString::fromStdString(viewFolder));
  QStringList kVRTIFilters;
  kVRTIFilters << "*.rti";
  kVDir.setNameFilters(kVRTIFilters);
  QStringList kVList = kVDir.entryList();

  QDir kVDir2(QString::fromStdString(viewFolder));
  QStringList kVRTIFilters2;
  kVRTIFilters2 << "*.rti.org";
  kVDir2.setNameFilters(kVRTIFilters2);
  QStringList kVList2 = kVDir.entryList();

  for (int xx = 0; xx < kVList2.size(); xx++)
    kVList.push_back(kVList2[xx]);

  std::string viewName = "XXX";
  std::vector<std::string> summaryEntries;
  std::vector<std::string> producedConfigs;

  typedef RTIToMHDConverter<PixelType> ConverterType;
  typename ConverterType::Pointer rticonv = ConverterType::New();
  typename ConverterType::StrLstType sliceList;
  typedef typename ConverterType::ImageType ImageType;
  typedef typename ConverterType::ImagePointer ImagePointer;
  typedef typename ConverterType::MetaInfoPointer MetaInfoPointer;

  std::string inpath = viewFolder;
  if (inpath[inpath.length() - 1] != '/' && inpath[inpath.length() - 1] != '\\')
    inpath += "/";
  std::string outpath = outputFolder;
  if (outpath[outpath.length() - 1] != '/' && outpath[outpath.length() - 1]
      != '\\')
    outpath += "/";

  SetupErrorCollection::Pointer sec = SetupErrorCollection::CreateFromFile(
      m_SetupErrorInfo);
  if (!sec)
  {
    SimpleErrorMacro(<< "Could not extract setup error collection: " <<
        m_SetupErrorInfo)
    return false;
  }
  std::string summaryfile = outpath + "summary.txt";
  std::ofstream summary(summaryfile.c_str());
  summary << "ITER\trx\try\trz\ttx\tty\ttz\tFILENAME\n";
  int iteration = 1;

  std::string viewFile = viewFolder;
  if (viewFile[viewFile.length() - 1] != '\\'
      && viewFile[viewFile.length() - 1] != '/')
    viewFile += "/";
  viewFile += "ViewInfo.inf";
  IniAccess vi(viewFile);
  std::string planUID = vi.ReadString("Plan", "PlnUID", "");
  viewName = vi.ReadString("View", "ViName", "");
  viewName = CleanStringForSimpleFileName(viewName);
  std::string outinfodir = "";
  IniAccess *oif = NULL;
  if (outputInfoFile.length() > 0)
  {
    outinfodir = itksys::SystemTools::GetFilenamePath(outputInfoFile);
    if (!itksys::SystemTools::MakeDirectory(outinfodir.c_str()))
    {
      SimpleErrorMacro(<< "ERROR: Output directory for info file " <<
          "could not be created: " << outinfodir)
      success = false;
    }
    else
    {
      oif = new IniAccess(outputInfoFile);
      std::string timestamp;
      CurrentORADateTimeString(timestamp);
      oif->WriteString("Info", "TimeStamp", timestamp);
      oif->WriteString("Info", "PlanUID", planUID);
      oif->WriteString("Info", "ViewUID", vi.ReadString("View", "ViUID", ""));
      oif->WriteString("Info", "Gantry", vi.ReadString("View", "ViGantry", ""));
      std::ostringstream os;
      os << "originX originY originZ orientation11 orientation12 "
          << "orientation13 orientation21 orientation22 orientation23 "
          << "orientation31 orientation32 orientation33 spacingX spacingY "
          << "spacingZ sizeX(pix) sizeY(pix) sizeZ(pix) acquisition-date";
      oif->WriteString("kV-list", "__HEADER__", os.str());
      oif->Update();
    }
  }

  // NOTE: furthermore we collect the encountered geometry-settings for easier
  // TMR-script creation later

  std::vector<MinmalKVGeometryType *> minKVInfo;

  for (int i = 0; i < kVList.size() && success; i++) // kV by kV ...
  {
    std::string actualKV = inpath + kVList[i].toStdString();
    std::string newKV = outpath + kVList[i].toStdString().substr(0,
        kVList[i].toStdString().length() - 3) + "mhd";

    if (progress)
    {
      double p = (double) i / (double) kVList.size();
      p *= 90; // -> rounded to 1 % resolution
      p = itk::Math::Round<int, double>(p);
      p /= 100;
      progress->SetProgress(p);
    }

    // first convert the image into implicit ORA.XML format in memory:
    sliceList.clear();
    sliceList.push_back(actualKV);
    rticonv->SetSliceList(sliceList);
    if (!rticonv->BuildImageFromSlices())
    {
      SimpleErrorMacro(<< "ERROR building kV image: " << actualKV)
      success = false;
      break; // exit
    }

    // now analyze the image and generate origin and orientation:
    ImagePointer itkImage = rticonv->GetImage();
    MetaInfoPointer mi = rticonv->GetMetaInfo();
    if (!itkImage || !mi)
    {
      SimpleErrorMacro(<< "ERROR extracting image and meta-info: " << actualKV)
      success = false;
      break; // exit
    }

    SetupError::Point3DType isoc = mi->GetVolumeMetaInfo()->GetIsocenter();
    double xch = isoc[2];
    isoc[2] = isoc[1]; // must swap Y and Z coordinate!
    isoc[1] = xch;

    // check minimal info:
    if (oif) // only if output-info is requested
    {
      typename ImageType::PointType itkOrigin;
      itkOrigin = itkImage->GetOrigin();
      typename ImageType::SizeType itkSize;
      itkSize = itkImage->GetLargestPossibleRegion().GetSize();
      typename ImageType::SpacingType itkSpacing;
      itkSpacing = itkImage->GetSpacing();
      double itkKVSize[2];
      itkKVSize[0] = itkSpacing[0] * itkSize[0];
      itkKVSize[1] = itkSpacing[1] * itkSize[1];
      bool found = false;
      for (std::size_t k = 0; k < minKVInfo.size(); k++)
      {
        if (minKVInfo[k])
        {
          // compare with 0.01 mm resolution:
          // orientation and gantry angle are assumed to remain stable
          if (fabs(minKVInfo[k]->Origin[0] - itkOrigin[0]) < 0.01 && fabs(
              minKVInfo[k]->Origin[1] - itkOrigin[1]) < 0.01 && fabs(
              minKVInfo[k]->Origin[2] - itkOrigin[2]) < 0.01 && fabs(
              minKVInfo[k]->Size[0] - itkKVSize[0]) < 0.01 && fabs(
              minKVInfo[k]->Size[1] - itkKVSize[1]) < 0.01 && fabs(
              minKVInfo[k]->Spacing[0] - itkSpacing[0]) < 0.01 && fabs(
              minKVInfo[k]->Spacing[1] - itkSpacing[1]) < 0.01)
          {
            minKVInfo[k]->NoHits++; // one hit more
            std::string kvmhd = kVList[i].toStdString();
            kvmhd = kvmhd.substr(0, kvmhd.length()
                - itksys::SystemTools::GetFilenameExtension(kvmhd).length())
                + ".mhd";
            minKVInfo[k]->KVList->push_back(kvmhd);
            found = true;
            break;
          }
        }
      }
      if (!found) // -> add new encountered geometry
      {
        MinmalKVGeometryType *mkvg = new MinmalKVGeometryType();
        mkvg->Origin[0] = itkOrigin[0];
        mkvg->Origin[1] = itkOrigin[1];
        mkvg->Origin[2] = itkOrigin[2];
        mkvg->Size[0] = itkKVSize[0];
        mkvg->Size[1] = itkKVSize[1];
        mkvg->Spacing[0] = itkSpacing[0];
        mkvg->Spacing[1] = itkSpacing[1];
        mkvg->SAD = mi->GetVolumeMetaInfo()->GetSourceAxisDistance();
        mkvg->SFD = mi->GetVolumeMetaInfo()->GetSourceFilmDistance();
        mkvg->Gantry = mi->GetVolumeMetaInfo()->GetGantry() / 180. * M_PI;
        mkvg->NoHits = 1;
        mkvg->KVList = new std::vector<std::string>();
        std::string kvmhd = kVList[i].toStdString();
        kvmhd = kvmhd.substr(0, kvmhd.length()
            - itksys::SystemTools::GetFilenameExtension(kvmhd).length())
            + ".mhd";
        mkvg->KVList->push_back(kvmhd);
        minKVInfo.push_back(mkvg);
      }
    }

    // NOTE: We have to flip the image raw data along its vertical axis, but
    // have to keep the geometrical settings!
    PixelType *pixels = itkImage->GetPixelContainer()->GetBufferPointer();
    const int w = itkImage->GetLargestPossibleRegion().GetSize()[0]; // img width
    const int wb = w * sizeof(PixelType); // width in bytes
    const int h = itkImage->GetLargestPossibleRegion().GetSize()[1]; // img height
    const int hl = h / 2; // half lines
    PixelType *line = new PixelType[w]; // temp buffer
    void *pline = (void *) line;
    void *pbuff = 0;
    void *pbuff2 = 0;
    int n = w * h - w; // last line
    for (int k = 0, y = 0; y < hl; k += w, y++, n -= w)
    {
      pbuff = (void *) (pixels + n);
      memcpy(pline, pbuff, wb); // store for swap
      pbuff2 = (void *) (pixels + k);
      memcpy(pbuff, pbuff2, wb); // swap
      memcpy(pbuff2, pline, wb);
    }
    delete [] line;

    // generate oriented ITK-image:
    // NOTE: the RTI converter has already been fixed to generate right-oriented
    // planar images!
    typedef itk::Image<unsigned short, 3> USHORTImageType;
    typedef itk::CastImageFilter<ImageType, USHORTImageType> CasterType;
    typename CasterType::Pointer caster = CasterType::New();
    caster->SetInput(itkImage);
    caster->Update();

    typedef itk::ImageFileWriter<USHORTImageType> WriterType;
    WriterType::Pointer wr = WriterType::New();
    wr->SetInput(caster->GetOutput());
    wr->SetFileName(newKV.c_str());
    wr->SetUseCompression(/*true*/false);
    try
    {
      wr->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      success = false;
      SimpleErrorMacro(<< "ERROR: Could not write oriented kV image: " << e)
      break; // exit
    }

    std::vector<SliceMetaInformation::Pointer> *sis =
        mi->GetSlicesMetaInformation();
    std::string acqDT = "";
    if (sis->size() > 0)
    {
      acqDT = (*sis)[0]->GetORAAcquisitionDate();
    }

    // write a line in the summary file:
    double rx = 0.; // YPR-transform
    double ry = 0.;
    double rz = 0.;
    double tx = 0.;
    double ty = 0.;
    double tz = 0.;

    SetupError::Pointer se = sec->FindBestMatchingSetupError(acqDT, true,
        planUID);
    if (se)
    {
      // correct the center of rotation with the iso-center!
      se->CorrectCenterOfRotation(isoc); // inherent update of transformation!
      SetupError::TransformPointer trans = se->GetTransform();
      // in summary file we relate to a rotation center of 0,0,0 -> therefore
      // take the matrix and offset information as reference:
      SetupError::TransformType::MatrixType m = trans->GetMatrix();
      SetupError::TransformType::OutputVectorType o = trans->GetOffset();

      SetupError::TransformPointer temp = SetupError::TransformType::New();
      temp->SetMatrix(m);
      temp->SetOffset(o);
      SetupError::TransformType::ParametersType pars = temp->GetParameters();

      // take over the YPR-transform parameters in WCS:
      rx = pars[0]; // roll
      ry = pars[1]; // pitch
      rz = pars[2]; // yaw
      tx = pars[3]; // tx
      ty = pars[4]; // tz
      tz = pars[5]; // ty

      summary << iteration << "\t" << rx << "\t" << ry << "\t" << rz << "\t"
          << tx << "\t" << ty << "\t" << tz << "\t" << newKV << "\n";

      std::ostringstream summaryEntry; // reduced
      summaryEntry.str("");
      summaryEntry << rx << " " << ry << " " << rz << " " << tx << " " << ty
          << " " << tz << "||" << newKV;
      summaryEntries.push_back(summaryEntry.str());

      iteration++;
    }
    else
    {
      // ... do not add to summary!
      VeryDetailedDebugMacro(<< "WARNING: No setup error for '" <<
          newKV << "' (date: " << acqDT << ")")
    }

    // write output info file if requested:
    if (oif)
    {
      std::string key = itksys::SystemTools::GetFilenameName(newKV);
      std::ostringstream os;
      os.str("");
      // compose info string:
      // originX originY originZ orientation11 ... orientation33 spacingX
      //    spacingY spacingZ sizeX(pix) sizeY(pix) sizeZ(pix) acquisition-date
      // - origin
      os << itkImage->GetOrigin()[0] << " " << itkImage->GetOrigin()[1] << " "
          << itkImage->GetOrigin()[2] << " ";
      // - direction (in row-order)
      os << itkImage->GetDirection()[0][0] << " "
          << itkImage->GetDirection()[1][0] << " "
          << itkImage->GetDirection()[2][0] << " "
          << itkImage->GetDirection()[0][1] << " "
          << itkImage->GetDirection()[1][1] << " "
          << itkImage->GetDirection()[2][1] << " "
          << itkImage->GetDirection()[0][2] << " "
          << itkImage->GetDirection()[1][2] << " "
          << itkImage->GetDirection()[2][2] << " ";
      // - spacing
      os << itkImage->GetSpacing()[0] << " " << itkImage->GetSpacing()[1]
          << " " << itkImage->GetSpacing()[2] << " ";
      // - size (pixels)
      os << itkImage->GetLargestPossibleRegion().GetSize()[0] << " "
          << itkImage->GetLargestPossibleRegion().GetSize()[1] << " "
          << itkImage->GetLargestPossibleRegion().GetSize()[2] << " ";
      // - acquisition date
      os << acqDT;
      oif->WriteString("kV-list", key, os.str());
    }
  }
  if (progress)
    progress->SetProgress(0.9);

  summary.close();
  std::string baseoutfolder;
  if (oif)
  {
    QDir qbasefolder(outputFolder.c_str());
    qbasefolder.cdUp();
    qbasefolder.cdUp();
    baseoutfolder = qbasefolder.absolutePath().toStdString();
    if (baseoutfolder[baseoutfolder.length() - 1] != '/'
        && baseoutfolder[baseoutfolder.length() - 1] != '\\')
      baseoutfolder += "/";
    std::string scriptsfolder = baseoutfolder;
    std::vector<std::string> availableCTs;
    if (produceConfigs)
    {
      if (itksys::SystemTools::MakeDirectory(scriptsfolder.c_str()))
      {
        QDirIterator it(qbasefolder, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
          QFileInfo info = it.next();
          if (info.isDir() && !info.absoluteFilePath().endsWith(".")
              && info.absoluteFilePath().length()
                  > (int) baseoutfolder.length())
          {
            std::string ctfolder = info.absoluteFilePath().toStdString();
            if (ctfolder[ctfolder.length() - 1] != '/'
                && ctfolder[ctfolder.length() - 1] != '\\')
              ctfolder += "/";
            std::string ctfile = ctfolder + "Volume.mhd";
            if (itksys::SystemTools::FileExists(ctfile.c_str(), true))
            {
              std::string id = ctfolder.substr(baseoutfolder.length(),
                  baseoutfolder.length());
              if (id[id.length() - 1] == '/' || id[id.length() - 1] == '\\')
                id = id.substr(0, id.length() - 1);
              // do not know why, but we get the folders multiple times ...
              bool foundID = false;
              for (unsigned int n = 0; n < availableCTs.size(); n++)
              {
                if (availableCTs[n] == id)
                {
                  foundID = true;
                  break;
                }
              }
              if (!foundID)
                availableCTs.push_back(id);
            }
          }
        }
      }
      else
      {
        produceConfigs = false; // cannot ...
      }
    }

    // write out and clean up geometry things
    std::ostringstream geomsect;
    for (std::size_t k = 0; k < minKVInfo.size(); k++)
    {
      if (progress)
      {
        double p = 0.9;
        p += (double) k / (double) minKVInfo.size() * 0.1;
        progress->SetProgress(p);
      }
      if (minKVInfo[k])
      {
        typedef itk::VersorRigid3DTransform<double> RotationTransformType;
        // modeled gantry rotation around y-axis of machine coordinate system:
        RotationTransformType::Pointer rtrans = RotationTransformType::New();
        RotationTransformType::AxisType rax;
        rax[0] = 0;
        rax[1] = 1;
        rax[2] = 0;
        rtrans->SetRotation(rax, minKVInfo[k]->Gantry);
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
        // focal spot position (@ gantry angle 0):
        RotationTransformType::OutputPointType fs;
        fs[0] = 0;
        fs[1] = 0;
        fs[2] = minKVInfo[k]->SAD;
        // obtain new vector directions / points by transforming them:
        RotationTransformType::OutputVectorType eprow =
            rtrans-> TransformVector(prow);
        RotationTransformType::OutputVectorType epcol =
            rtrans-> TransformVector(pcol);
        RotationTransformType::OutputPointType efs =
            rtrans-> TransformPoint(fs);

        geomsect.str("");
        geomsect << "encountered-projection-geometry-" << (k + 1);
        oif->WriteValue<int> (geomsect.str(),
            "number_of_images_that_share_this_config", minKVInfo[k]->NoHits);
        oif->WriteString(geomsect.str(), "advanced_config", "ON");
        oif->WriteValue<double> (geomsect.str(), "focal_point[0]", efs[0]);
        oif->WriteValue<double> (geomsect.str(), "focal_point[1]", efs[1]);
        oif->WriteValue<double> (geomsect.str(), "focal_point[2]", efs[2]);
        oif->WriteValue<double> (geomsect.str(), "projection_spacing[0]",
            minKVInfo[k]->Spacing[0]);
        oif->WriteValue<double> (geomsect.str(), "projection_spacing[1]",
            minKVInfo[k]->Spacing[1]);
        oif->WriteValue<double> (geomsect.str(), "projection_size[0]",
            minKVInfo[k]->Size[0]);
        oif->WriteValue<double> (geomsect.str(), "projection_size[1]",
            minKVInfo[k]->Size[1]);
        oif->WriteValue<double> (geomsect.str(), "projection_plane_origin[0]",
            minKVInfo[k]->Origin[0]);
        oif->WriteValue<double> (geomsect.str(), "projection_plane_origin[1]",
            minKVInfo[k]->Origin[1]);
        oif->WriteValue<double> (geomsect.str(), "projection_plane_origin[2]",
            minKVInfo[k]->Origin[2]);
        oif->WriteValue<double> (geomsect.str(),
            "projection_plane_row_vector[0]", eprow[0]);
        oif->WriteValue<double> (geomsect.str(),
            "projection_plane_row_vector[1]", eprow[1]);
        oif->WriteValue<double> (geomsect.str(),
            "projection_plane_row_vector[2]", eprow[2]);
        oif->WriteValue<double> (geomsect.str(),
            "projection_plane_col_vector[0]", epcol[0]);
        oif->WriteValue<double> (geomsect.str(),
            "projection_plane_col_vector[1]", epcol[1]);
        oif->WriteValue<double> (geomsect.str(),
            "projection_plane_col_vector[2]", epcol[2]);

        if (produceConfigs && availableCTs.size() > 0)
        {
          for (unsigned int n = 0; n < availableCTs.size(); n++)
          {
            std::string tmp = itksys::SystemTools::GetFilenameName(
                configTemplate);
            std::string ext = itksys::SystemTools::GetFilenameExtension(tmp);
            std::ostringstream currConfig;
            currConfig << tmp.substr(0, tmp.length() - ext.length());
            currConfig << "_" << availableCTs[n] << "_" << viewName
                << "_setup-" << (k + 1) << "_hits-" << minKVInfo[k]->NoHits
                << ".ini";
            std::string currFile = scriptsfolder + currConfig.str();
            // first copy the template:
            if (itksys::SystemTools::CopyAFile(configTemplate.c_str(),
                currFile.c_str()))
            {
              IniAccess currScript(currFile);

              // OK, now override the missing (templated) parameters in config:
              std::string sep = DSEP;
              currScript.WriteString("volume", "file", availableCTs[n] + sep
                  + "Volume.mhd");

              std::ostringstream drrout;
              drrout << "DRRs" << sep;
              drrout << availableCTs[n] << "_" << viewName << "_setup-" << (k
                  + 1) << "_hits-" << minKVInfo[k]->NoHits << sep;
              currScript.WriteString("output", "output_base_file", drrout.str()
                  + "DRR___${PAR1}_${PAR2}_${PAR3}_${PAR4}_${PAR5}_${PAR6}.mhd");
              currScript.WriteString("output", "summary_file", drrout.str()
                  + "summary.txt");

              currScript.WriteString("projection-geometry", "advanced_config",
                  "ON");
              currScript.WriteValue<double> ("projection-geometry",
                  "focal_point[0]", efs[0]);
              currScript.WriteValue<double> ("projection-geometry",
                  "focal_point[1]", efs[1]);
              currScript.WriteValue<double> ("projection-geometry",
                  "focal_point[2]", efs[2]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_spacing[0]", minKVInfo[k]->Spacing[0]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_spacing[1]", minKVInfo[k]->Spacing[1]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_size[0]", minKVInfo[k]->Size[0]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_size[1]", minKVInfo[k]->Size[1]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_plane_origin[0]", minKVInfo[k]->Origin[0]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_plane_origin[1]", minKVInfo[k]->Origin[1]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_plane_origin[2]", minKVInfo[k]->Origin[2]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_plane_row_vector[0]", eprow[0]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_plane_row_vector[1]", eprow[1]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_plane_row_vector[2]", eprow[2]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_plane_col_vector[0]", epcol[0]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_plane_col_vector[1]", epcol[1]);
              currScript.WriteValue<double> ("projection-geometry",
                  "projection_plane_col_vector[2]", epcol[2]);

              // in addition, prepare the initial_parameters for the
              // transformation according to logged summary (commented):
              if (minKVInfo[k]->KVList && minKVInfo[k]->KVList->size() > 0)
              {
                for (unsigned int z = 0; z < minKVInfo[k]->KVList->size(); z++)
                {
                  std::string thiskv = (*minKVInfo[k]->KVList)[z];
                  // -> search in summary entries for this kV
                  for (unsigned int y = 0; y < summaryEntries.size(); y++)
                  {
                    std::string sument = summaryEntries[y];
                    if (sument.find(thiskv) != std::string::npos)
                    {
                      std::size_t pppos = sument.find("||");
                      if (pppos != std::string::npos)
                      {
                        sument = sument.substr(0, pppos);
                        // -> add (commented) entry:
                        std::string parkey = "// ";
                        parkey += thiskv + ":    initial_parameters";
                        currScript.WriteString("transformation", parkey, sument);
                      }
                      break;
                    }
                  }
                }
              }

              currScript.Update();
              producedConfigs.push_back(itksys::SystemTools::GetFilenameName(
                  currScript.GetFileName()));
            }
          }
        }

        if (minKVInfo[k]->KVList)
          delete minKVInfo[k]->KVList;
        delete minKVInfo[k];
      }
    }
    minKVInfo.clear();
    oif->Update();
    delete oif;
  }
  rticonv = NULL;

  // finally prepare a bash / batch script for running the scripts:
  if (produceConfigs && producedConfigs.size() > 0)
  {
    // LINUX

    std::string bashFile = "";
    bool fexists = false;
    int c = 1;
    do
    {
      bashFile = baseoutfolder + "Generate_ALL_";
      if (c == 1)
      {
        bashFile += viewName + ".sh";
      }
      else
      {
        std::ostringstream oss;
        oss.str("");
        oss << viewName << "_" << c << ".sh";
        bashFile += oss.str();
      }
      fexists = itksys::SystemTools::FileExists(bashFile.c_str(), true);
      c++;
    } while (fexists);

    std::ofstream bash(bashFile.c_str());

    bash << "#!/bin/bash\n\n\n";
    bash << "# auto-generated by ORAIF 'TMR Image Conversion' module\n\n\n";
    bash << "# $1 ... path-to-GLSL-ray-caster-binary\n\n\n";
    for (unsigned int i = 0; i < producedConfigs.size(); i++)
    {
      bash << "$1 " << producedConfigs[i] << "\n";
    }
    bash << "\n";

    bash.close();

    // WINDOWS

    std::string batchFile = "";
    fexists = false;
    c = 1;
    do
    {
      batchFile = baseoutfolder + "Generate_ALL_";
      if (c == 1)
      {
        batchFile += viewName + ".bat";
      }
      else
      {
        std::ostringstream oss;
        oss.str("");
        oss << viewName << "_" << c << ".bat";
        batchFile += oss.str();
      }
      fexists = itksys::SystemTools::FileExists(batchFile.c_str(), true);
      c++;
    } while (fexists);

    std::ofstream batch(batchFile.c_str());

    batch
        << "\nREM auto-generated by ORAIF 'TMR Image Conversion' module\n\n\n";
    batch << "REM %1 ... path-to-GLSL-ray-caster-binary\n\n\n";
    for (unsigned int i = 0; i < producedConfigs.size(); i++)
    {
      batch << "%1 " << producedConfigs[i] << "\n";
    }
    batch << "\n";

    batch.close();
  }

  if (progress)
    progress->SetProgress(1.0);

  VeryDetailedDebugMacro(<< "Finish conversion of kVs in '" << viewFolder <<
      "' --> SUCCESS = " << success << ")")

  return success;
}

template<typename TPixelType>
typename ImageOrienter<TPixelType>::ImageType *
ImageOrienter<TPixelType>::ConvertPlanarImageObject(std::string imageFile)
{
  if (!itksys::SystemTools::FileExists(imageFile.c_str()))
  {
    SimpleErrorMacro(<< "Input planar image does not exist: " << imageFile)
    return NULL;
  }

  bool useITKReader = true;
  if (ToLowerCaseF(itksys::SystemTools::GetFilenameExtension(imageFile)) == ".rti" ||
      ToLowerCaseF(itksys::SystemTools::GetFilenameExtension(imageFile)) == ".rti.org")
    useITKReader = false;

  ImageType *dataSet = NULL;
  if (!useITKReader) // RTI converter
  {
    typedef RTIToMHDConverter<PixelType> ConverterType; // FIXME: pix type
    typename ConverterType::Pointer rticonv = ConverterType::New();
    typename ConverterType::StrLstType sliceList;
    typedef typename ConverterType::ImageType ITKImageType;
    typedef typename ConverterType::ImagePointer ITKImagePointer;
    typedef typename ConverterType::MetaInfoPointer MetaInfoPointer;
    sliceList.push_back(imageFile);
    rticonv->SetSliceList(sliceList);
    if (!rticonv->BuildImageFromSlices())
    {
      SimpleErrorMacro(<< "ERROR building planar image: " << imageFile)
      return NULL;
    }
    // now analyze the image and generate origin and orientation:
    ITKImagePointer itkImage = rticonv->GetImage();
    MetaInfoPointer mi = rticonv->GetMetaInfo();
    if (!itkImage || !mi)
    {
      SimpleErrorMacro(<< "ERROR extracting image and meta-info: " << imageFile)
      return NULL;
    }
    // NOTE: We have to flip the image raw data along its vertical axis, but
    // have to keep the geometrical settings!
    PixelType *pixels = itkImage->GetPixelContainer()->GetBufferPointer();
    const int w = itkImage->GetLargestPossibleRegion().GetSize()[0]; // img width
    const int wb = w * sizeof(PixelType); // width in bytes
    const int h = itkImage->GetLargestPossibleRegion().GetSize()[1]; // img height
    const int hl = h / 2; // half lines
    PixelType *line = new PixelType[w]; // temp buffer
    void *pline = (void *) line;
    void *pbuff = 0;
    void *pbuff2 = 0;
    int n = w * h - w; // last line
    for (int k = 0, y = 0; y < hl; k += w, y++, n -= w)
    {
      pbuff = (void *) (pixels + n);
      memcpy(pline, pbuff, wb); // store for swap
      pbuff2 = (void *) (pixels + k);
      memcpy(pbuff, pbuff2, wb); // swap
      memcpy(pbuff2, pline, wb);
    }
    delete [] line;

     // Create Image ITK/VTK image
    itk::ImageIOBase::IOComponentType ct = ITKVTKImage::GetComponentTypeFromTemplate<TPixelType>();
    if (ct == itk::ImageIOBase::UNKNOWNCOMPONENTTYPE)
    {
      SimpleErrorMacro(<< "Error during ConvertPlanarImageObject (UNKNOWN COMPONENT TYPE): " <<
          imageFile);
      return false;
    }
    dataSet = new ITKVTKImage(itk::ImageIOBase::SCALAR, ct);
    // Set the image data (up-cast to base-class)
    ITKVTKImage::ITKImagePointer ip = static_cast<ITKVTKImage::ITKImageType *>(itkImage.GetPointer());
    dataSet->SetITKImage(ip, false); // set image: (ITK -> VTK if necessary)
    ip = NULL;
    dataSet->SetMetaInfo(mi);
  }
  else // -> ITK reader
  {
    if (ToLowerCaseF(itksys::SystemTools::GetFilenameExtension(imageFile)) == ".ora.xml")
    {
    	// Allow ora.xml files as input and ignore invalid MD5 checksums
      dataSet = ImageType::LoadFromORAMetaImage(imageFile, true);
    }
    else
    {
      dataSet = ImageType::LoadImageFromFile(imageFile, ITKVTKImage::ITKIO);
    }
    // assume that the pixels are correctly aligned!
    typedef itk::Image<TPixelType, ITKVTKImage::Dimensions> InputImageType;
    typedef typename InputImageType::Pointer InputImageTypePointer;
    InputImageTypePointer itkImage = NULL;
    if (dataSet)
    {
      // Get ITK image
      ImageType::ITKImagePointer itkBaseImage;
      TEMPLATE_CALL_COMP(dataSet->GetComponentType(), itkBaseImage = dataSet->GetAsITKImage, )
      // Cast to original ITK image type
      itkImage = static_cast<InputImageType * >(itkBaseImage.GetPointer());
      itkBaseImage = NULL;
    }
    if (!itkImage || itkImage->GetLargestPossibleRegion().GetSize()[2] != 1)
    {
      SimpleErrorMacro(<< "Image appears to be non-planar: " << imageFile)
      delete dataSet;
      return NULL;
    }
  }

  return dataSet;
}

template<typename TPixelType>
bool ImageOrienter<TPixelType>::ConvertPlanarImage(std::string sourceRTIFile,
    std::string outputFile, bool useCompression, bool writeOraXml,
    bool oraXmlAnonymized)
{
  VeryDetailedDebugMacro(<< "Start conversion of planar image '" <<
      sourceRTIFile << "' -> '" << outputFile << "'.")

  if (!itksys::SystemTools::FileExists(sourceRTIFile.c_str()))
  {
    SimpleErrorMacro(<< "Input planar image does not exist: " << sourceRTIFile)
    return false;
  }
  if (outputFile.length() == 0)
  {
    SimpleErrorMacro(<< "Output image file name required")
    return false;
  }

  typedef RTIToMHDConverter<PixelType> ConverterType;
  typename ConverterType::Pointer rticonv = ConverterType::New();
  typename ConverterType::StrLstType sliceList;
  typedef typename ConverterType::ImageType ImageType;
  typedef typename ConverterType::ImagePointer ImagePointer;
  typedef typename ConverterType::MetaInfoPointer MetaInfoPointer;

  sliceList.push_back(sourceRTIFile);
  rticonv->SetSliceList(sliceList);
  if (!rticonv->BuildImageFromSlices())
  {
    SimpleErrorMacro(<< "ERROR building planar image: " << sourceRTIFile)
    return false;
  }

  // now analyze the image and generate origin and orientation:
  ImagePointer itkImage = rticonv->GetImage();
  MetaInfoPointer mi = rticonv->GetMetaInfo();
  if (!itkImage || !mi)
  {
    SimpleErrorMacro(<< "ERROR extracting image and meta-info: " << sourceRTIFile)
    return false;
  }

  // NOTE: We have to flip the image raw data along its vertical axis, but
  // have to keep the geometrical settings!
  PixelType *pixels = itkImage->GetPixelContainer()->GetBufferPointer();
  const int w = itkImage->GetLargestPossibleRegion().GetSize()[0]; // img width
  const int wb = w * sizeof(PixelType); // width in bytes
  const int h = itkImage->GetLargestPossibleRegion().GetSize()[1]; // img height
  const int hl = h / 2; // half lines
  PixelType *line = new PixelType[w]; // temp buffer
  void *pline = (void *) line;
  void *pbuff = 0;
  void *pbuff2 = 0;
  int n = w * h - w; // last line
  for (int k = 0, y = 0; y < hl; k += w, y++, n -= w)
  {
    pbuff = (void *) (pixels + n);
    memcpy(pline, pbuff, wb); // store for swap
    pbuff2 = (void *) (pixels + k);
    memcpy(pbuff, pbuff2, wb); // swap
    memcpy(pbuff2, pline, wb);
  }
  delete [] line;

  // generate oriented ITK-image:
  typedef itk::Image<unsigned short, 3> USHORTImageType;
  typedef itk::CastImageFilter<ImageType, USHORTImageType> CasterType;
  typename CasterType::Pointer caster = CasterType::New();
  caster->SetInput(itkImage);
  caster->Update();

  typedef itk::ImageFileWriter<USHORTImageType> WriterType;
  WriterType::Pointer wr = WriterType::New();

  if (itksys::SystemTools::GetFilenameLastExtension(outputFile) == ".xmhd")
  {
    typedef ora::XRawImageIO<USHORTImageType::PixelType,
        USHORTImageType::ImageDimension> XIOType;
    typename XIOType::Pointer xio = XIOType::New();
    xio->SetAutoExtensionMode(true);
    wr->SetImageIO(xio); // set the extended IO (with AutoExtensionMode)
    useCompression = true;
  }

  wr->SetInput(caster->GetOutput());
  wr->SetFileName(outputFile.c_str());
  wr->SetUseCompression(useCompression);
  try
  {
    wr->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    SimpleErrorMacro(<< "ERROR: Could not write planar image: " << e)
    return false;
  }

  if (writeOraXml)
  {
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(outputFile);
    std::string oraXmlFile = outputFile.substr(0, outputFile.length()
        - ext.length()) + ".ora.xml";
    if (oraXmlAnonymized) // anonymize the ora.xml-file:
      mi->Anonymize();
    if (!mi->WriteToXMLFile(oraXmlFile))
    {
      SimpleErrorMacro(<< "ERROR: Could not write ORA-XML file!")
      return false;
    }
  }

  // extract planar image's source position:
  // FIXME:


  VeryDetailedDebugMacro(<< "Finish conversion of planar image '" <<
      sourceRTIFile << "' -> '" << outputFile << "'.")

  return true;
}

}

#endif /* ORAIMAGEORIENTER_TXX_ */
