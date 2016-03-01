//
#include "oraREG23Model.h"

// ORAIFTools
#include <oraIniAccess.h>
#include <oraStringTools.h>
#include <oraFileTools.h>
// ORAIFModel
#include <oraImageConsumer.h>
#include <oraImageImporterTask.h>
// ORAIFStructureAccess
#include <oraVTKStructure.h>
#include <vtkORAStructureReader.h>
#include <vtkSurfaceToPerspectiveProjectionImageFilter.h>
#include <vtkImplicitTriangulatedPolyData.h>
// ORAIFRegistration
#include <oraSetupError.h>
// ORAIFTransform
#include <oraYawPitchRoll3DTransform.h>
// ORAIFNReg2D3DDRREngine
#include <oraPortalImagingDeviceProjectionProperties.h>
// ORAIFNReg2D3DAlgorithm
#include <oraCompositeImageToImageMetric.h>

#include "oraREG23RegistrationInitializationTask.h"
#include "oraREG23RegistrationExecutionTask.h"
#include "oraREG23SparseRegistrationExecutionTask.h"
#include "oraSimpleTransformUndoRedo.h"

#include <itksys/SystemTools.hxx>
#include <itkImageDuplicator.h>
#include <itkInvertIntensityImageFilter.h>
#include <itkAndImageFilter.h>
#include <itkOrImageFilter.h>
#include <itkXorImageFilter.h>
#include <itkVotingBinaryIterativeHoleFillingImageFilter.h>
#include <itkVersorRigid3DTransform.h>
#ifdef ITK_USE_REVIEW
#include <itkBinaryContourImageFilter.h>
#else
#include <itkSimpleContourExtractorImageFilter.h>
#endif
#include <itkImageFileWriter.h>

#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkMatrix3x3.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkColorTransferFunction.h>
#include <vtkTable.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkCallbackCommand.h>
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkDataSetWriter.h>
#include <vtkPLYWriter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkDataSetReader.h>
#include <vtkPLYReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkWarpVector.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkDataSetAttributes.h>
#include <vtkDecimatePro.h>
#include <vtkQuadricDecimation.h>
#include <vtkClipPolyData.h>
#include <vtkSampleFunction.h>
#include <vtkContourFilter.h>
#include <vtkMetaImageWriter.h>
#include <vtkDataArray.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkFunctionParser.h>
#include <vtkTimerLog.h>

#include <sstream>
#include <fstream>
#include <algorithm>
#include <itkImageFileWriter.h>

namespace ora
{

// static vars:
QMutex REG23Model::m_RenderCoordinator;

REG23Model::REG23Model() :
    Model()
{
  m_NReg = NULL;
  m_Config = NULL;
  m_ValidConfiguration = false;
  m_SiteInfo = NULL;
  m_ITFPool = NULL;
  m_PatientUID = "";
  m_IsoCenter = NULL;
  m_Volume = new ImageConsumer(true); // delete @ end
  m_PPVolume = NULL;
  m_FixedImages.clear();
  m_PPFixedImages.clear();
  m_MaskImages.clear();
  m_PPFixedImagesPost.clear();
  m_SourcePositions.clear();
  m_ExplicitInitialWLs.clear();
  m_CommandLineArguments.clear();
  m_LastCommandLineOptionIndex = -1;
  m_VolumeImageFileName = "";
  m_FixedImageFileNames.clear();
  m_ViewNames.clear();
  m_Structures = new VTKStructureSet();
  m_TaskManager = NULL;
  m_ToolRenderWindow = NULL;
  m_DRRToolRenderWindow = NULL;
  m_Transform = TransformType::New();
  m_CurrentParameters.SetSize(m_Transform->GetNumberOfParameters()); // =6
  m_CurrentParameters.Fill(0);
  m_InitialParameters.SetSize(m_Transform->GetNumberOfParameters());
  m_InitialParameters.Fill(0);
  m_ReferenceParameters.SetSize(0); // invalid!
  m_CurrentOptimizationParameters.SetSize(m_Transform->GetNumberOfParameters());
  m_CurrentOptimizationParameters.Fill(0);
  m_NumberOfIterations = 0;
  m_CurrentIteration = 0;
  m_CurrentOptimizationMeasure = 0;
  m_LastMetricMeasure = 0;
  m_OptimizerCommand = CommandType::New();
  m_RegistrationCommand = CommandType::New();
  m_StartRegistrationAutomatically = false;
  m_RegistrationIsRunning = false;
  m_CurrentOptimizerTypeID = "";
  m_RenderCostFunction = false;
  m_FunctionValues.clear();
  m_UpdateImagesAsSoonAsBetter = false;
  m_UpdateImagesModulo = 1;
  m_UpdateImagesMaxFrameRate = 0;
  m_UpdateImagesPending = false;
  itk::RealTimeClock::Pointer timer = itk::RealTimeClock::New();
  m_LastUpdateImagesTime = timer->GetTimeStamp() - 1000.;
  m_CurrentMovingImages.clear();
  m_OutputTransformFile = "";
  m_ConfiguredGraphicsStyle = "";
  for (std::size_t k = 0; k < 3; k++) // default: isocenter
    m_CenterOfRotation[k] = 0;
  m_TransformationWidgetsVisible = false;
  m_RegionSensitiveTransformNature = false;
  m_DisplayMasks = false;
  m_MasksColorHSV[0] = 0;
  m_MasksColorHSV[1] = 0;
  m_MasksColorHSV[2] = 1;
  CurrentORADateTimeString(m_ModelCreationTimeStamp);
  m_UndoRedoManager = new SimpleTransformUndoRedoManager();
  m_UndoRedoManager->SetLogFlag(true);
  m_ShowWarningOnCancel = true;
  m_ShowWarningOnDivergingFixedImageAcquisitionTimes = true;
  m_MaxFixedImageAcquisitionDivergingTimeMin = 1.1;
  m_ScientificMode = false;
  m_IntelligentMaskMode = false;
  m_RealTimeAdaptiveWindowing = true;
  m_WindowingSensitivity = 1.0;
  m_UnsharpMaskingEnabled.clear();
  m_UnsharpMaskingActivated.clear();
  m_WindowLevelStorageStrategy = 0; // do not store
  m_WindowLevelRecoveryStrategy = 0; // do not recover
  m_ITFOptimizerConfigStrings.clear();
  m_ExecuteSparsePreRegistration = false;
  m_SparsePreRegistrationType = "";
}

REG23Model::~REG23Model()
{
  m_NReg = NULL;
  if (m_Config)
    delete m_Config;
  m_Config = NULL;
  if (m_SiteInfo)
    delete m_SiteInfo;
  m_SiteInfo = NULL;
  if (m_ITFPool)
    delete m_ITFPool;
  m_ITFPool = NULL;
  if (m_IsoCenter)
    delete [] m_IsoCenter;
  m_IsoCenter = NULL;
  delete m_Volume;
  if (m_PPVolume)
    delete m_PPVolume;
  for (std::size_t i = 0; i < m_FixedImages.size(); i++)
    delete m_FixedImages[i];
  for (std::size_t i = 0; i < m_PPFixedImages.size(); i++)
    delete m_PPFixedImages[i];
  for (std::size_t i = 0; i < m_MaskImages.size(); i++)
    delete m_MaskImages[i];
  for (std::size_t i = 0; i < m_PPFixedImagesPost.size(); i++)
    delete m_PPFixedImagesPost[i];
  for (std::size_t i = 0; i < m_SourcePositions.size(); i++)
    delete [] m_SourcePositions[i];
  for (std::size_t i = 0; i < m_ExplicitInitialWLs.size(); i++)
    if (m_ExplicitInitialWLs[i])
      delete m_ExplicitInitialWLs[i];
  m_FixedImages.clear();
  m_PPFixedImages.clear();
  m_MaskImages.clear();
  m_PPFixedImagesPost.clear();
  m_SourcePositions.clear();
  m_ExplicitInitialWLs.clear();
  m_FixedImageFileNames.clear();
  m_ViewNames.clear();
  std::vector<VTKStructure *> structs = m_Structures->GetStructures();
  for (std::size_t i = 0; i < structs.size(); i++)
    delete structs[i];
  m_Structures->Clear();
  delete m_Structures;
  SetToolRenderWindow(NULL);
  SetDRRToolRenderWindow(NULL);
  m_Transform = NULL;
  m_OptimizerCommand = NULL;
  m_RegistrationCommand = NULL;
  m_FunctionValues.clear();
  delete m_UndoRedoManager;
  m_UndoRedoManager = NULL;
}

void REG23Model::ConnectRenderWindowToGlobalLock(vtkRenderWindow *renWin)
{
  if (!renWin)
    return;
  vtkSmartPointer<vtkCallbackCommand> cb =
      vtkSmartPointer<vtkCallbackCommand>::New();
  cb->SetCallback(REG23Model::GlobalRenderCoordinationCB);
  renWin->AddObserver(vtkCommand::StartEvent, cb);
  renWin->AddObserver(vtkCommand::EndEvent, cb);
}

void REG23Model::DisconnectRenderWindowFromGlobalLock(vtkRenderWindow *renWin)
{
  if (!renWin)
    return;
  renWin->RemoveObservers(vtkCommand::StartEvent);
  renWin->RemoveObservers(vtkCommand::EndEvent);
}

void REG23Model::SetToolRenderWindow(vtkRenderWindow *renWin)
{
  DisconnectRenderWindowFromGlobalLock(m_ToolRenderWindow);
  m_ToolRenderWindow = renWin;
  ConnectRenderWindowToGlobalLock(m_ToolRenderWindow);
}

void REG23Model::SetDRRToolRenderWindow(vtkRenderWindow *renWin)
{
  // NOTE: Do not connect/disconnect tool render window here, rather connect
  // the start/end events of the DRR engine!

//  DisconnectRenderWindowFromGlobalLock(m_DRRToolRenderWindow);
  m_DRRToolRenderWindow = renWin;
//  ConnectRenderWindowToGlobalLock(m_DRRToolRenderWindow);
}

void REG23Model::GlobalRenderCoordinationCB(vtkObject *caller, unsigned long eid,
      void *clientdata, void *calldata)
{
  vtkRenderWindow *rwin = dynamic_cast<vtkRenderWindow*>(caller);
  if (eid == vtkCommand::StartEvent)
  {
    REG23Model::m_RenderCoordinator.lock();
    if (rwin)
    {
      rwin->MakeCurrent(); // be sure it is current
    }
  }
  else if (eid == vtkCommand::EndEvent)
  {
    REG23Model::m_RenderCoordinator.unlock();
  }
}

bool REG23Model::LoadConfiguration(std::string &errorSection,
                                   std::string &errorKey, std::string &errorMessage)
{
  m_ValidConfiguration = false;
  errorSection = "";
  errorKey = "";
  errorMessage = "";
  if (m_SiteInfo)
    delete m_SiteInfo;
  m_SiteInfo = NULL;
  if (m_ITFPool)
    delete m_ITFPool;
  m_ITFPool = NULL;
  m_PatientUID = "";
  m_VolumeImageFileName = "";
  m_FixedImageFileNames.clear();
  m_ViewNames.clear();
  std::vector<std::string> v;
  if (!itksys::SystemTools::FileExists(m_ConfigFile.c_str()))
  {
    errorMessage = "Config file (" + m_ConfigFile + ") does not exist.";
    return false;
  }
  m_Config = new IniAccess(m_ConfigFile);

  // ORA-connectivity
  UnixUNCConverter *uuc = UnixUNCConverter::GetInstance(); // UNIX-UNC-map
  errorSection = "ORA";
  int i = 1, j;
  std::string s = "", s2 = "";
  std::vector<std::string> sa;
  do
  {
    errorKey = "UnixUNC" + StreamConvert(i);
    s = m_Config->ReadString(errorSection, errorKey, "");
    if (s.length() > 0)
    {
      sa.clear();
      Tokenize(s, sa, ",");
      if (sa.size() != 2)
      {
        errorMessage = "UNIX-UNC-pair separated with \",\" expected.";
        return false;
      }
      // add pair to GLOBAL UNIX/UNC mapping table:
      uuc->AddTranslationPair(TrimF(sa[1]), TrimF(sa[0]));
    }
    i++;
  }
  while (s.length() > 0);
  errorKey = "SiteInfo";
  s = TrimF(m_Config->ReadString(errorSection, errorKey, "", true));
  if (s.length() > 0) // if specified, we expect a valid file!
  {
    if (!itksys::SystemTools::FileExists(s.c_str()))
    {
      errorMessage = "Specified site info file does not exist.";
      return false;
    }
    m_SiteInfo = new IniAccess(s); // pre-load
  }
  errorKey = "PatientUID";
  s = TrimF(m_Config->ReadString(errorSection, errorKey, "", false));
  if (s.length() > 0 && m_SiteInfo) // if specified, we expect an existent UID
  {
    std::string patsFld = m_SiteInfo->ReadString("Paths", "PatientsFolder", "",
                          true);
    if (!itksys::SystemTools::FileExists(patsFld.c_str()))
    {
      errorMessage = "Site's patient folder does not exist.";
      return false;
    }
    EnsureStringEndsWith(patsFld, DSEP);
    patsFld += s;
    if (!itksys::SystemTools::FileExists(patsFld.c_str()))
    {
      errorMessage = "Specified patient UID appears to be invalid.";
      return false;
    }
    m_PatientUID = s; // buffer it
  }

  // Images
  int last = m_LastCommandLineOptionIndex;
  // - moving
  errorSection = "Images";
  errorKey = "Volume";
  s = TrimF(m_Config->ReadString(errorSection, errorKey, "", true));
  if (ToUpperCaseF(s) == "FROM-COMMAND-LINE")
  {
    std::string volumeFile = "";
    if (m_LastCommandLineOptionIndex > 0)
    {
      last++;
      if (last < (int) m_CommandLineArguments.size())
      {
        volumeFile = m_CommandLineArguments[last];
        if (!itksys::SystemTools::FileExists(volumeFile.c_str()))
          volumeFile = "";
        std::string ext = itksys::SystemTools::GetFilenameExtension(volumeFile);
        if (ora::ToLowerCaseF(ext) == ".ora.xml" && !HaveORAConnectivity())
          volumeFile = "";
      }
    }
    if (volumeFile.length() <= 0)
    {
      errorMessage = "Volume file expected as command line argument. ";
      errorMessage += "But it appears to be invalid or not specified or to ";
      errorMessage += "require an existent ORA-connectivity!";
      return false;
    }
    m_VolumeImageFileName = volumeFile;
  }
  else
  {
    if (!itksys::SystemTools::FileExists(s.c_str()))
    {
      errorMessage = "Volume image file does not exist.";
      return false;
    }
    // -> check if it is an ora.xml
    std::string ext = TrimF(itksys::SystemTools::GetFilenameExtension(s));
    if (ToLowerCaseF(ext) == ".ora.xml" && !m_SiteInfo)
    {
      errorMessage
      = "ORA.XML file specified, but no ORA-connectivity configured.";
      return false; // because we require ORA-connectivity in this case!
    }
    m_VolumeImageFileName = s;
  }
  // - volume pre-processing entries
  i = 1;
  do
  {
    errorKey = "Volume.PreProcess" + StreamConvert(i);
    s = TrimF(m_Config->ReadString(errorSection, errorKey, ""));
    if (s.length() > 0 && !ParseImageProcessingEntry(s, errorMessage, v,
        CAST | CROP | RESAMPLE | RESCALEMINMAX | RESCALESHIFTSCALE |
        RESCALEWINDOWING | STORE)) // TODO: unsharp masking for volumes!
      return false; // errorMessage filled from parser!
    i++;
  } while (s.length() > 0);
  // - fixed
  for (std::size_t k = 0; k < m_FixedImages.size(); k++)
    delete m_FixedImages[k];
  m_FixedImages.clear();
  for (std::size_t k = 0; k < m_MaskImages.size(); k++)
    delete m_MaskImages[k];
  m_MaskImages.clear();
  for (std::size_t k = 0; k < m_PPFixedImagesPost.size(); k++)
    delete m_PPFixedImagesPost[k];
  m_PPFixedImagesPost.clear();
  bool expectFixedImagesFromCommandLine = false;
  i = 1;
  do
  {
    errorKey = "FixedImage" + StreamConvert(i);
    s = TrimF(m_Config->ReadString(errorSection, errorKey, "", true));
    if (ToUpperCaseF(s) == "FROM-COMMAND-LINE")
    {
      expectFixedImagesFromCommandLine = true;
      break; // rest is ignored
    }
    i++;
  }
  while (s.length() > 0);
  if (!expectFixedImagesFromCommandLine)
  {
    i = 1;
    do
    {
      errorKey = "FixedImage" + StreamConvert(i);
      s = TrimF(m_Config->ReadString(errorSection, errorKey, "", true));
      if (s.length() > 0)
      {
        if (!itksys::SystemTools::FileExists(s.c_str()))
        {
          errorMessage = "Fixed image file does not exist.";
          m_FixedImageFileNames.clear();
          m_ViewNames.clear();
          for (std::size_t k = 0; k < m_FixedImages.size(); k++)
            delete m_FixedImages[k];
          m_FixedImages.clear();
          return false;
        }
        // -> check if it is an rti
        std::string ext = TrimF(itksys::SystemTools::GetFilenameExtension(s));
        if ((ToLowerCaseF(ext) == ".rti" || ToLowerCaseF(ext) == ".rti.org") &&
            !m_SiteInfo)
        {
          errorMessage
          = "RTI file specified, but no ORA-connectivity configured.";
          m_FixedImageFileNames.clear();
          m_ViewNames.clear();
          for (std::size_t k = 0; k < m_FixedImages.size(); k++)
            delete m_FixedImages[k];
          m_FixedImages.clear();
          return false; // because we require ORA-connectivity in this case!
        }
        m_FixedImageFileNames.push_back(s);
        m_FixedImages.push_back(new ImageConsumer(true)); // delete @ end
        // check if we find a view info file (-> apply view name, otherwise "")
        bool haveViewInfo = false;
        if ((ToLowerCaseF(ext) == ".rti" || ToLowerCaseF(ext) == ".rti.org"))
        {
          std::string vf = itksys::SystemTools::GetFilenamePath(s);
          EnsureStringEndsWith(vf, DSEP);
          vf += m_SiteInfo->ReadString("Paths", "ViewInfoFile", "");
          if (itksys::SystemTools::FileExists(vf.c_str()))
          {
            IniAccess vif(vf);
            m_ViewNames.push_back(vif.ReadString("View", "ViName", ""));
            haveViewInfo = true;
          }
        }
        // specify / override fixed image's view name:
        s2 = "FixedImage" + StreamConvert(i);
        s2 += ".ViewName";
        s2 = TrimF(m_Config->ReadString("Images", s2, ""));
        if (s2.length() > 0)
        {
          if (haveViewInfo) // override
            m_ViewNames[i - 1] = s2;
          else // specify
            m_ViewNames.push_back(s2);
          haveViewInfo = true; // mark
        }
        if (!haveViewInfo) // no view info available
          m_ViewNames.push_back("");
      }
      i++;
    }
    while (s.length() > 0);
  }
  else
  {
    std::vector<std::string> fixedFiles;
    if (m_LastCommandLineOptionIndex > 0)
    {
      i = 1;
      while (++last < (int) m_CommandLineArguments.size())
      {
        errorKey = "FixedImage" + StreamConvert(i);
        std::string ffile = m_CommandLineArguments[last];
        if (!itksys::SystemTools::FileExists(ffile.c_str()))
        {
          fixedFiles.clear();
          break;
        }
        std::string ext = itksys::SystemTools::GetFilenameExtension(ffile);
        if ((ToLowerCaseF(ext) == ".rti" || ToLowerCaseF(ext) == ".rti.org") &&
            !HaveORAConnectivity())
        {
          fixedFiles.clear();
          break;
        }
        if (ffile.length() > 0)
          fixedFiles.push_back(ffile);
        i++;
      }
    }
    if (fixedFiles.size() <= 0)
    {
      errorMessage
      = "Fixed image file(s) expected as command line argument(s). ";
      errorMessage += "But they appear to be invalid or not specified or to ";
      errorMessage += "require an existent ORA-connectivity!";
      return false;
    }
    m_FixedImageFileNames = fixedFiles;
    for (std::size_t k = 0; k < m_FixedImageFileNames.size(); k++)
    {
      m_FixedImages.push_back(new ImageConsumer(true)); // delete @ end
      // check if we find a view info file (-> apply view name, otherwise "")
      bool haveViewInfo = false;
      std::string ext = TrimF(itksys::SystemTools::GetFilenameExtension(
          m_FixedImageFileNames[k]));
      if ((ToLowerCaseF(ext) == ".rti" || ToLowerCaseF(ext) == ".rti.org"))
      {
        std::string vf = itksys::SystemTools::GetFilenamePath(
            m_FixedImageFileNames[k]);
        EnsureStringEndsWith(vf, DSEP);
        vf += m_SiteInfo->ReadString("Paths", "ViewInfoFile", "");
        if (itksys::SystemTools::FileExists(vf.c_str()))
        {
          IniAccess vif(vf);
          m_ViewNames.push_back(vif.ReadString("View", "ViName", ""));
          haveViewInfo = true;
        }
      }
      if (!haveViewInfo)
        m_ViewNames.push_back("");
    }
  }
  for (std::size_t k = 0; k < m_FixedImages.size(); k++)
  {
    m_MaskImages.push_back(new ImageConsumer(true)); // prepare for later
    m_PPFixedImagesPost.push_back(NULL); // prepare for later
  }
  // - fixed pre-processing
  i = 1;
  do
  {
    errorKey = "FixedImage" + StreamConvert(i);
    s = TrimF(m_Config->ReadString(errorSection, errorKey, ""));
    if (s.length() > 0)
    {
      j = 1;
      do
      {
        errorKey = "FixedImage" + StreamConvert(i);
        errorKey += ".PreProcess" + StreamConvert(j);
        s2 = TrimF(m_Config->ReadString(errorSection, errorKey, ""));
        if (s2.length() > 0 && !ParseImageProcessingEntry(s2, errorMessage, v,
            CAST | CROP | RESAMPLE | RESCALEMINMAX | RESCALESHIFTSCALE |
            RESCALEWINDOWING | STORE | UNSHARPMASKING | FLIP | THRESHOLD_ABOVE |
            THRESHOLD_BELOW | THRESHOLD_OUTSIDE))
          return false; // errorMessage filled from parser!
        j++;
      } while (s2.length() > 0);
    }
    i++;
  } while (s.length() > 0);
  // - fixed post-processing
  i = 1;
  s2 = "";
  do
  {
    errorKey = "FixedImage" + StreamConvert(i);
    s = TrimF(m_Config->ReadString(errorSection, errorKey, ""));
    if (s.length() > 0)
    {
      j = 1;
      do
      {
        errorKey = "FixedImage" + StreamConvert(i);
        errorKey += ".PostProcess" + StreamConvert(j);
        s2 = TrimF(m_Config->ReadString(errorSection, errorKey, ""));
        if (s2.length() > 0
            && !ParseImageProcessingEntry(s2, errorMessage, v, CAST |
                RESCALEMINMAX | RESCALESHIFTSCALE | RESCALEWINDOWING | STORE |
                RESCALEMINMAXMASK | UNSHARPMASKING))
          return false; // errorMessage filled from parser!
        j++;
      } while (s2.length() > 0);
    }
    i++;
  } while (s.length() > 0);

  // Geometry
  for (std::size_t k = 0; k < m_SourcePositions.size(); k++)
    delete [] m_SourcePositions[k];
  m_SourcePositions.clear();
  errorSection = "Geometry";
  for (std::size_t k = 1; k <= m_FixedImages.size(); k++)
  {
    errorKey = "SourcePosition" + StreamConvert(k);
    s = ToUpperCaseF(TrimF(m_Config->ReadString(errorSection, errorKey, "")));
    while (ReplaceString(s, " ", ""));
    if (s.length() > 0)
    {
      if (s.substr(0, 11) != "FROM-IMAGE(")
      {
        double *p = NULL;
        int n = ParseCommaSeparatedNumericStringVector(s, p);
        if (n == 3)
        {
          m_SourcePositions.push_back(p); // add source position
        }
        else // (n != 3)
        {
          if (p)
            delete [] p;
          errorMessage = "The SourcePosition-entry should either contain 'FROM-IMAGE(ROTATIONAL)' or the 3D coordinate in the form x,y,z.";
          return false;
        }
      }
      else // FROM-IMAGE(ROTATIONAL) or FROM-IMAGE(ROTATIONAL,<offset>)
      {
        if (ToLowerCaseF(itksys::SystemTools::GetFilenameExtension(m_FixedImageFileNames[k - 1])) != ".rti" &&
            ToLowerCaseF(itksys::SystemTools::GetFilenameExtension(m_FixedImageFileNames[k - 1])) != ".rti.org" &&
            ToLowerCaseF(itksys::SystemTools::GetFilenameExtension(m_FixedImageFileNames[k - 1])) != ".ora.xml")
        {
          errorMessage = "The 'FROM-IMAGE()' keyword requires a set corresponding RTI-image or ORA.XML-image.";
          return false;
        }
        std::string sa = s.substr(11, s.length());
        sa = sa.substr(0, sa.length() - 1); // cut last ")"
        std::vector<std::string> toks;
        Tokenize(sa, toks, ",");
        if (toks.size() < 1 || toks.size() > 2)
        {
          errorMessage = "The 'FROM-IMAGE()' keyword needs exactly 1 or 2 arguments!\n";
          errorMessage += s;
          return false;
        }
        if (toks[0] != "ROTATIONAL")
        {
          errorMessage = "The 'FROM-IMAGE()' keyword first argument must be 'ROTATIONAL'!\n";
          errorMessage += s;
          return false;
        }
        if (toks.size() == 2 && !IsNumeric(toks[1]))
        {
          errorMessage = "The 'FROM-IMAGE()' keyword argument <offset> must be numeric!";
          errorMessage += s;
          return false;
        }
        if (toks.size() == 2)
        {
          m_GantryOffsetAngle.push_back(atof(toks[1].c_str()));
        }
        else
        {
          m_GantryOffsetAngle.push_back(0.0);
        }
        m_SourcePositions.push_back(NULL); // for auto-determination later
      }
    }
    else
    {
      errorMessage = "SourcePosition" + StreamConvert(k);
      errorMessage += "-entry not found! Must be defined!";
      for (std::size_t u = 0; u < m_SourcePositions.size(); u++)
        delete [] m_SourcePositions[u];
      m_SourcePositions.clear();
      return false;
    }
  }
  errorKey = "IsoCenter";
  s = TrimF(m_Config->ReadString(errorSection, errorKey, "", false));
  if (s.length() > 0) // if specified, we expect a valid iso-center
  {
    if (m_IsoCenter)
      delete [] m_IsoCenter;
    m_IsoCenter = NULL;
    if (itksys::SystemTools::FileExists(s.c_str())) // BeamInfo/ViewInfo ...
    {
      IniAccess info(s);
      // -> check whether it is a beam info:
      bool isBeamInfo = (info.ReadString("Beam", "BmUID", "").length() > 0);
      if (isBeamInfo)
      {
        m_IsoCenter = new double[3];
        m_IsoCenter[0] = info.ReadValue<double> ("Beam", "BmIsoCtrX", 0.0);
        m_IsoCenter[1] = info.ReadValue<double> ("Beam", "BmIsoCtrY", 0.0);
        m_IsoCenter[2] = info.ReadValue<double> ("Beam", "BmIsoCtrZ", 0.0);
      }
      else // -> view info
      {
        s = info.ReadString("View", "ViIsoCtrXYZ", "");
        std::vector<std::string> toks;
        Tokenize(s, toks, ",");
        if (toks.size() == 3)
        {
          bool num = true;
          for (std::size_t u = 0; u < toks.size(); u++)
            num &= IsNumeric(toks[u]);
          if (num)
          {
            m_IsoCenter = new double[3];
            m_IsoCenter[0] = atof(toks[0].c_str());
            m_IsoCenter[1] = atof(toks[2].c_str()); // swap!
            m_IsoCenter[2] = atof(toks[1].c_str()); // swap!
          }
        }
      }
    }
    else // -> values expected
    {
      if (ParseCommaSeparatedNumericStringVector(s, m_IsoCenter) != 3 &&
          m_IsoCenter)
      {
        delete [] m_IsoCenter;
        m_IsoCenter = NULL;
      }
    }
    if (!m_IsoCenter)
    {
      errorMessage
      = "The iso-center entry appears to be neither a beam / view info file, nor a valid iso-center specification!";
      return false;
    }
  }

  // Structures
  i = 1;
  errorSection = "Structures";
  std::vector<VTKStructure *> structs = m_Structures->GetStructures();
  for (std::size_t i = 0; i < structs.size(); i++)
    delete structs[i];
  m_Structures->Clear();
  do
  {
    errorKey = "Structure" + StreamConvert(i);
    s = TrimF(m_Config->ReadString(errorSection, errorKey, ""));
    if (s.length() > 0)
    {
      if (!ParseAndProcessStructureEntry(i, true, errorKey, errorMessage))
        return false; // error-message set!
    }
    i++;
  } while (s.length() > 0);

  // Auto-Masking
  errorSection = "Auto-Masking";
  ITKVTKImage *dummy = NULL;
  for (std::size_t k = 1; k <= m_FixedImages.size(); k++)
  {
    if (!ParseAndProcessMaskEntry(k, true, errorKey, errorMessage, dummy))
      return false; // error-message set!
  }
  errorKey = "UseMaskOptimization";
  if (!IsStrictlyNumeric(m_Config->ReadString(errorSection, errorKey, "")))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }

  // DRR
  errorSection = "DRR";
  errorKey = "ITF-POOL-File";
  s = TrimF(m_Config->ReadString(errorSection, errorKey, "", true));
  if (s.length() > 0)
  {
    if (!itksys::SystemTools::FileExists(s.c_str()))
    {
      errorMessage = "The specified ITF-pool '" + s;
      errorMessage += "' does not exist!";
      return false;
    }
    m_ITFPool = new IniAccess(s);
  }
  for (std::size_t k = 1; k <= m_FixedImages.size(); k++)
  {
    errorKey = "ITF" + StreamConvert(k);
    if (!ParseITF(m_Config->ReadString(errorSection, errorKey, ""), errorMessage))
      return false;
    errorKey = "CastStepSize" + StreamConvert(k);
    s = ToUpperCaseF(TrimF(m_Config->ReadString(errorSection, errorKey, "")));
    if (s.length() == 0)
    {
      errorMessage = "A CastStepSize entry for each fixed image is required.";
      return false;
    }
    if (s != "AUTO-0" && s != "AUTO-1" && s != "AUTO-2" && s != "AUTO-3")
    {
      if (!IsNumeric(s))
      {
        errorMessage = "The entry must be either a value in mm, or equal one of these constants: AUTO-0,AUTO-1,AUTO-2,AUTO-3";
        return false;
      }
    }
  }

  // Transform
  errorSection = "Transform";
  m_InitialParameters.SetSize(6);
  m_InitialParameters.Fill(0);
  sa.clear();
  errorKey = "InitialParameters";
  sa.push_back(TrimF(m_Config->ReadString(errorSection, errorKey, "")));
  errorKey = "InitialRotationMatrix";
  sa.push_back(TrimF(m_Config->ReadString(errorSection, errorKey, "")));
  errorKey = "InitialTranslation";
  sa.push_back(TrimF(m_Config->ReadString(errorSection, errorKey, "")));
  if (sa[0].length() > 0 && sa[1].length() > 0 && sa[2].length() > 0)
  {
    errorMessage = "The initial transform must be specified by EITHER specifying the InitialParameters-key OR by specifying both InitialRotationMatrix and InitialTranslation!";
    return false;
  }
  errorKey = "InitialParameters";
  s = sa[0];
  if (s.length() > 0)
  {
    double *pars = NULL;
    int n = ParseCommaSeparatedNumericStringVector(s, pars);
    if (n != 6)
    {
      if (pars)
        delete [] pars;
      errorMessage = "The initial transform parameters must be of the form: <rx>,<ry>,<rz>,<tx>,<ty>,<tz>.";
      return false;
    }
    m_InitialParameters.SetSize(6);
    for (int d = 0; d < 3; d++)
      m_InitialParameters[d] = pars[d] / 180. * vtkMath::Pi();
    for (int d = 3; d < 6; d++)
      m_InitialParameters[d] = pars[d];
    if (pars)
      delete [] pars;
  } // else: OK, take over initial parameters
  errorKey = "CenterOfRotation";
  s = TrimF(m_Config->ReadString(errorSection, errorKey, ""));
  if (s.length() > 0)
  {
    double *cor = NULL;
    int n = ParseCommaSeparatedNumericStringVector(s, cor);
    if (n != 3)
    {
      if (cor)
        delete [] cor;
      errorMessage = "The initial transform parameters must be of the form: <rx>,<ry>,<rz>,<tx>,<ty>,<tz>.";
      return false;
    }
    for (std::size_t k = 0; k < 3; k++)
    {
      // NOTE: correct center of rotation with iso-center because we translated
      // the volume in order to position it w.r.t. the iso-center:
      cor[k] = cor[k] - m_IsoCenter[k]; // both in cm here
      m_CenterOfRotation[k] = cor[k] * 10.; // cm -> mm
    }
    if (cor)
      delete [] cor;
  } // else: OK, take over initial parameters
  else
  {
    for (std::size_t k = 0; k < 3; k++) // default: isocenter
      m_CenterOfRotation[k] = 0;
  }
  if (sa[0].length() <= 0)
  {
    if (!ParseInitialRotationTranslationTransform("Transform", errorKey, errorMessage))
    {
      return false; // errorMessage filled!
    }
  }

  // Registration
  errorSection = "Registration";
  for (std::size_t k = 1; k <= m_FixedImages.size(); k++)
  {
    errorKey = "MetricType" + StreamConvert(k);
    v.clear();
    if (!ParseMetricEntry(m_Config->ReadString(errorSection, errorKey, ""),
        m_Config->ReadString(errorSection, "MetricConfig" + StreamConvert(k), ""),
        m_Config->ReadString(errorSection, "MetricRulePrefix" + StreamConvert(k), ""),
        m_Config->ReadString(errorSection, "MetricRulePostfix" + StreamConvert(k), ""),
        v, errorMessage))
    {
      return false;
    }
  }
  errorKey = "OptimizerType";
  v.clear();
  if (!ParseOptimizerEntry(m_Config->ReadString(errorSection, errorKey, ""),
      m_Config->ReadString(errorSection, "OptimizerConfig", ""),
      m_Config->ReadString(errorSection, "OptimizerScales", ""),
      v, errorMessage))
  {
    return false;
  }
  errorKey = "UseOptimizedValueComputation";
  if (!IsStrictlyNumeric(m_Config->ReadString(errorSection, errorKey, "")))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }
  errorKey = "StartRegistrationAutomatically";
  if (!IsStrictlyNumeric(m_Config->ReadString(errorSection, errorKey, "")))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }
  m_StartRegistrationAutomatically = m_Config->ReadValue<bool>(errorSection,
      errorKey, false);
  errorKey = "OutputTransformFile";
  m_OutputTransformFile = m_Config->ReadString(errorSection, errorKey, "");
  if (!ParseSparsePreRegistrationEntries(errorSection, errorKey, errorMessage))
  {
    return false; // error message already filled!
  }

  // Visualization
  errorSection = "Visualization";
  errorKey = "RenderCostFunction";
  if (!IsStrictlyNumeric(m_Config->ReadString(errorSection, errorKey, "")))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }
  m_RenderCostFunction = m_Config->ReadValue<bool>(errorSection, errorKey, false);
  InitFunctionTable();
  errorKey = "OverlayRenderingModulo";
  m_UpdateImagesModulo = m_Config->ReadValue<int>(errorSection, errorKey, 1);
  errorKey = "OverlayRenderingBetter";
  if (!IsStrictlyNumeric(m_Config->ReadString(errorSection, errorKey, "")))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }
  m_UpdateImagesAsSoonAsBetter = m_Config->ReadValue<bool>(errorSection, errorKey, false);
  errorKey = "OverlayRenderingMaxFrameRate";
  m_UpdateImagesMaxFrameRate = m_Config->ReadValue<double>(errorSection, errorKey, 0.0);
  errorKey = "GraphicsStyle";
  s = ToLowerCaseF(TrimF(m_Config->ReadString(errorSection, errorKey, "")));
  if (s.length() > 0 && s != "plastique" && s != "windows" && s != "motif" && s != "cleanlooks")
  {
    errorMessage = "The configured graphics style is invalid, must be one of those: plastique, windows, motif, cleanlooks.";
    return false;
  }
  m_ConfiguredGraphicsStyle = s;
  for (std::size_t i = 0; i < m_ExplicitInitialWLs.size(); i++)
    if (m_ExplicitInitialWLs[i])
      delete m_ExplicitInitialWLs[i];
  m_ExplicitInitialWLs.clear();
  for (std::size_t k = 1; k <= m_FixedImages.size(); k++)
  {
    for (std::size_t r = 1; r <= 2; r++)
    {
      if (r == 1)
        errorKey = "InitialFixedImageWindowLevel" + StreamConvert(k);
      else
        errorKey = "UnsharpMaskingWindowLevel" + StreamConvert(k);
      s = ToLowerCaseF(TrimF(m_Config->ReadString(errorSection, errorKey, "")));
      if (s.length() <= 0)
        continue; // OK
      v.clear();
      Tokenize(s, v, ",");
      if (v.size() != 2 && v.size() != 3)
      {
        errorMessage = "The initial ";
        if (r == 1)
          errorMessage += "fixed";
        else
          errorMessage += "unsharp mask";
        errorMessage += " image window/level entry is invalid. Either <w>,<l> or MEAN-STD,<f1>,<f2> are possible.";
        return false;
      }
      if (v.size() == 2)
      {
        for (std::size_t n = 0; n < v.size(); n++)
        {
          if (!IsNumeric(TrimF(v[n])))
          {
            errorMessage = "The ";
            if (r == 1)
              errorMessage += "fixed";
            else
              errorMessage += "unsharp mask";
            errorMessage += " image window/level entry (<w>,<l>) has at least one non-numeric term!";
            return false;
          }
        }
        if (r == 1) // fixed image
        {
          double *eiwl = new double[2];
          double cw = atof(v[0].c_str());
          double cl = atof(v[1].c_str());
          eiwl[0] = cl - cw / 2.;
          eiwl[1] = cl + cw / 2.;
          m_ExplicitInitialWLs.push_back(eiwl); // explicit!
        }
      }
      else // 3
      {
        if (TrimF(v[0]) != "mean-std" && TrimF(v[0]) != "min-max-rel")
        {
          errorMessage = "Did you mean a MEAN-STD,<f1>,<f2> or MIN-MAX-REL,<f1>,<f2> entry? Need static term MEAN-STD or MIN-MAX-REL as first element!";
          return false;
        }
        for (std::size_t n = 1; n < v.size(); n++)
        {
          if (!IsNumeric(TrimF(v[n])))
          {
            errorMessage = "The ";
            if (r == 1)
              errorMessage += "fixed";
            else
              errorMessage += "unsharp mask";
            errorMessage = " image window/level entry (MEAN-STD,<f1>,<f2> or MIN-MAX-REL,<f1>,<f2>) has at least one non-numeric factor-term!";
            return false;
          }
        }
        if (r == 1) // fixed image
          m_ExplicitInitialWLs.push_back(NULL); // not explicit!
      }
    }
  }
  errorKey = "ManualTransformWidgetsVisible";
  s = m_Config->ReadString(errorSection, errorKey, "");
  if (s.length() == 0)
    s = "0";
  if (!IsStrictlyNumeric(s))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }
  m_TransformationWidgetsVisible = m_Config->
      ReadValue<bool>(errorSection, errorKey, false);
  errorKey = "EnableRegionSensitiveTransformOnLeftMouseClick";
  s = m_Config->ReadString(errorSection, errorKey, "");
  if (s.length() == 0)
    s = "0";
  if (!IsStrictlyNumeric(s))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }
  m_RegionSensitiveTransformNature = m_Config->
      ReadValue<bool>(errorSection, errorKey, false);
  errorKey = "MasksVisible";
  s = m_Config->ReadString(errorSection, errorKey, "");
  if (s.length() == 0)
    s = "0";
  if (!IsStrictlyNumeric(s))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }
  m_DisplayMasks = m_Config->ReadValue<bool>(errorSection, errorKey, false);
  errorKey = "MasksContourColor";
  s = m_Config->ReadString(errorSection, errorKey, "0.0,0.0,1.0");
  {
    double *c = NULL;
    int n = ParseCommaSeparatedNumericStringVector(s, c);
    if (n == 3)
    {
      m_MasksColorHSV[0] = c[0];
      m_MasksColorHSV[1] = c[1];
      m_MasksColorHSV[2] = c[2];
      delete [] c;
    }
    else
    {
      if (c != NULL)
        delete [] c;
      errorMessage = "Expecting a comma-separated HSV-triplet for this entry!";
      return false;
    }
  }
  errorKey = "RealTimeAdaptiveWindowing";
  s = m_Config->ReadString(errorSection, errorKey, "");
  if (s.length() == 0)
    s = "0";
  if (!IsStrictlyNumeric(s))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }
  m_RealTimeAdaptiveWindowing = m_Config->ReadValue<bool>(errorSection, errorKey, true);
  errorKey = "WindowingMouseSensitivity";
  s = m_Config->ReadString(errorSection, errorKey, "1.0");
  if (!IsNumeric(s) || atof(s.c_str()) <= 0)
  {
    errorMessage = "Expecting a sensitivity factor >0.0 for this entry!";
    return false;
  }
  m_WindowingSensitivity = atof(s.c_str());
  m_UnsharpMaskingEnabled.clear();
  m_UnsharpMaskingActivated.clear();
  for (std::size_t k = 1; k <= m_FixedImages.size(); k++)
  {
    errorKey = "EnableUnsharpMasking" + StreamConvert(k);
    s = m_Config->ReadString(errorSection, errorKey, "0");
    if (!IsStrictlyNumeric(s))
    {
      errorMessage = "Expecting a flag value (0 or 1) for this entry!";
      return false;
    }
    m_UnsharpMaskingEnabled.push_back(static_cast<bool>(atoi(s.c_str())));

    errorKey = "UnsharpMasking" + StreamConvert(k);
    s = m_Config->ReadString(errorSection, errorKey, "0");
    if (!IsStrictlyNumeric(s))
    {
      errorMessage = "Expecting a flag value (0 or 1) for this entry!";
      return false;
    }
    m_UnsharpMaskingActivated.push_back(static_cast<bool>(atoi(s.c_str())));

    // (UnsharpMaskingWindowLevel{i} checked above!
  }

  // Quality-Assurance
  errorSection = "Quality-Assurance";
  errorKey = "ShowWarningOnCancel";
  s = m_Config->ReadString(errorSection, errorKey, "");
  if (s.length() == 0)
    s = "1";
  if (!IsStrictlyNumeric(s))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }
  m_ShowWarningOnCancel = atoi(s.c_str());
  errorKey = "ShowWarningOnDivergingFixedImageAcquisitionTimes";
  s = m_Config->ReadString(errorSection, errorKey, "");
  if (s.length() == 0)
    s = "1";
  if (!IsStrictlyNumeric(s))
  {
    errorMessage = "Expecting a flag value (0 or 1) for this entry.";
    return false;
  }
  m_ShowWarningOnDivergingFixedImageAcquisitionTimes = atoi(s.c_str());
  if (!HaveORAConnectivity()) // only possible with ORA images!
    m_ShowWarningOnDivergingFixedImageAcquisitionTimes = false;
  errorKey = "MaxFixedImageAcquisitionDivergingTimeMin";
  s = m_Config->ReadString(errorSection, errorKey, "");
  if (s.length() == 0)
    s = "1.1";
  if (!IsNumeric(s) || atof(s.c_str()) <= 0)
  {
    errorMessage = "Expecting a positive, numeric value for this entry.";
    return false;
  }
  m_MaxFixedImageAcquisitionDivergingTimeMin = atof(s.c_str());

  // Science
  errorSection = "Science";
  errorKey = "ScientificMode";
  s = TrimF(m_Config->ReadString(errorSection, errorKey, ""));
  if (s.length() > 0)
  {
    if (!IsStrictlyNumeric(s))
    {
      errorMessage = "Expecting a flag value (0 or 1) for this entry.";
      return false;
    }
    m_ScientificMode = atoi(s.c_str());
  }
  if (m_ScientificMode)
  {
    std::vector<std::string> ses;
    i = 0;
    do
    {
      i++;
      errorKey = "ReferenceTransformByORASetupError" + StreamConvert(i);
      s = TrimF(m_Config->ReadString(errorSection, errorKey, ""));
      if (s.length() > 0)
        ses.push_back(s);
    } while (s.length() > 0);
    if (!ParseORASetupErrors(ses, errorMessage))
      return false; // error message set

    m_ITFOptimizerConfigStrings.clear();
    errorKey = "ITFOptimizerEXE";
    s = TrimF(m_Config->ReadString(errorSection, errorKey, ""));
    if (s.length() > 0 && !itksys::SystemTools::FileExists(s.c_str()))
    {
      errorMessage = "The specified ITF-optimizer executable file could not be found!";
      return false;
    }
    m_ITFOptimizerConfigStrings.push_back(s);
  }

  errorSection = "";
  errorKey = "";
  errorMessage = "";
  m_ValidConfiguration = true;
  return m_ValidConfiguration;
}

bool REG23Model::ParseImageProcessingEntry(std::string entry,
    std::string &errorMessage, std::vector<std::string> &result,
    const int operationFlags) const
{
  result.clear();
  errorMessage = "";
  std::string origEntry = entry;

  // remove all spaces, case-insensitive:
  while (ReplaceString(entry, " ", ""));
  std::string normalCaseEntry = entry;
  ToUpperCase(entry);
  // check if the last ")" is present
  if (entry.length() <= 0)
  {
    errorMessage = "Entry does not contain any processing operation.";
    return false;
  }
  if (entry[entry.length() - 1] != ')')
  {
    errorMessage = "Processing operations must end with a ')'.";
    return false;
  }

  std::vector<std::string> toks;
  if (entry.substr(0, 5) == "CAST(" && (operationFlags & CAST) == CAST)
  {
    entry = entry.substr(5, entry.length() - 6);
    Tokenize(entry, toks, ",");
    if (toks.size() != 1)
    {
      errorMessage = "The Cast()-operator requires exactly one argument (<type>).";
      return false;
    }
    if (toks[0] != "CHAR" && toks[0] != "UCHAR" && toks[0] != "SHORT" &&
        toks[0] != "USHORT" && toks[0] != "INT" && toks[0] != "UINT" &&
        toks[0] != "LONG" && toks[0] != "ULONG" &&
        toks[0] != "FLOAT" && toks[0] != "DOUBLE")
    {
      errorMessage = "The Cast()-operator's <type> argument does not match any of these: ";
      errorMessage += "CHAR, UCHAR, SHORT, USHORT, INT, UINT, LONG, ULONG, FLOAT, DOUBLE.";
      return false;
    }
    result.push_back("CAST");
    result.push_back(toks[0]);
    return true;
  }
  else if (entry.substr(0, 5) == "CROP("
      && (operationFlags & CROP) == CROP)
  {
    entry = entry.substr(5, entry.length() - 6);
    Tokenize(entry, toks, ",");
    if (toks.size() != 6)
    {
      errorMessage = "The Crop()-operator requires exactly 6 arguments (<x1>,<y1>,<z1>,<w>,<h>,<d>).";
      return false;
    }
    result.push_back("CROP");
    for (std::size_t i = 0; i < 6; i++)
    {
      if (!IsNumeric(toks[i]))
      {
        errorMessage = "Argument " + StreamConvert(i + 1);
        errorMessage += " of the Crop()-operator must be a numeric expression.";
        result.clear();
        return false;
      }
      if (i > 2 && atoi(toks[i].c_str()) <= 0)
      {
        errorMessage = "Argument " + StreamConvert(i + 1);
        errorMessage += " of the Crop()-operator must not <=0.";
        result.clear();
        return false;
      }
      result.push_back(StreamConvert(atoi(toks[i].c_str()))); // int-conversion
    }
    return true;
  }
  else if (entry.substr(0, 9) == "RESAMPLE("
      && (operationFlags & RESAMPLE) == RESAMPLE)
  {
    entry = entry.substr(9, entry.length() - 10);
    Tokenize(entry, toks, ",");
    if (toks.size() != 4 && toks.size() != 5)
    {
      errorMessage = "The Resample()-operator requires either 4 arguments (<sx>,<sy>,<sz>,<interpolation>) or 5 arguments (<sx>,<sy>,<sz>,<interpolation>,<smoothing-radius>).";
      return false;
    }
    result.push_back("RESAMPLE");
    for (std::size_t i = 0; i < 3; i++)
    {
      if (!IsNumeric(toks[i]))
      {
        errorMessage = "Argument " + StreamConvert(i + 1);
        errorMessage += " of the Resample()-operator must be a numeric expression.";
        result.clear();
        return false;
      }
      if (atof(toks[i].c_str()) <= 0.)
      {
        errorMessage = "Argument " + StreamConvert(i + 1);
        errorMessage += " of the Resample()-operator must not <=0.0.";
        result.clear();
        return false;
      }
      result.push_back(StreamConvert(atof(toks[i].c_str()))); // double-conversion
    }
    if (toks[3] != "LINEAR" && toks[3] != "BSPLINE-3")
    {
      errorMessage = "The Resample()-operator's <interpolation> argument does not match any of these: ";
      errorMessage += "LINEAR, BSPLINE-3.";
      result.clear();
      return false;
    }
    result.push_back(toks[3]);
    if (toks.size() == 5)
    {
      if (ToUpperCaseF(toks[4]) == "ANTIALIASING")
      {
        result.push_back("ANTIALIASING"); // auto-smoothing
      }
      else
      {
        if (!IsNumeric(toks[4]) || atoi(toks[4].c_str()) <= 0)
        {
          errorMessage = "The Resample()-operator's <smoothing-radius> argument does neither match ANTIALIASING nor specify a positive pixel radius!";
          result.clear();
          return false;
        }
        result.push_back(toks[4]);
      }
    }
    return true;
  }
  else if (entry.substr(0, 5) == "FLIP(" && (operationFlags & FLIP) == FLIP)
  {
    entry = entry.substr(5, entry.length() - 6);
    Tokenize(entry, toks, ",");
    if (toks.size() != 1)
    {
      errorMessage = "The Flip()-operator requires exactly one argument (<direction>).";
      return false;
    }
    if (toks[0] != "X" && toks[0] != "Y")
    {
      errorMessage = "The Flip()-operator's <direction> argument does not match any of these: ";
      errorMessage += "X, Y.";
      return false;
    }
    result.push_back("FLIP");
    result.push_back(toks[0]);
    return true;
  }
  else if (entry.substr(0, 15) == "THRESHOLDABOVE(" && (operationFlags & THRESHOLD_ABOVE) == THRESHOLD_ABOVE)
  {
    entry = entry.substr(15, entry.length() - 16);
    Tokenize(entry, toks, ",");
    if (toks.size() != 2)
    {
      errorMessage = "The ThresholdAbove()-operator requires exactly two arguments (<thresh>,<out>).";
      return false;
    }
    result.push_back("THRESHOLDABOVE");
    result.push_back(toks[0]); // thresh
    result.push_back(toks[1]); // out
    return true;
  }
  else if (entry.substr(0, 15) == "THRESHOLDBELOW(" && (operationFlags & THRESHOLD_BELOW) == THRESHOLD_BELOW)
  {
    entry = entry.substr(15, entry.length() - 16);
    Tokenize(entry, toks, ",");
    if (toks.size() != 2)
    {
      errorMessage = "The ThresholdBelow()-operator requires exactly two arguments (<thresh>,<out>).";
      return false;
    }
    result.push_back("THRESHOLDBELOW");
    result.push_back(toks[0]); // thresh
    result.push_back(toks[1]); // out
    return true;
  }
  else if (entry.substr(0, 17) == "THRESHOLDOUTSIDE(" && (operationFlags & THRESHOLD_OUTSIDE) == THRESHOLD_OUTSIDE)
  {
    entry = entry.substr(17, entry.length() - 18);
    Tokenize(entry, toks, ",");
    if (toks.size() != 3)
    {
      errorMessage = "The ThresholdOutside()-operator requires exactly three arguments (<min>,<max>,<out>).";
      return false;
    }
    result.push_back("THRESHOLDOUTSIDE");
    result.push_back(toks[0]); // min
    result.push_back(toks[1]); // max
    result.push_back(toks[2]); // out
    return true;
  }
  else if (entry.substr(0, 14) == "RESCALEMINMAX("
      && (operationFlags & RESCALEMINMAX) == RESCALEMINMAX)
  {
    entry = entry.substr(14, entry.length() - 15);
    Tokenize(entry, toks, ",");
    if (toks.size() != 2)
    {
      errorMessage = "The RescaleMinMax()-operator requires exactly 2 arguments (<min>,<max>).";
      return false;
    }
    result.push_back("RESCALEMINMAX");
    for (std::size_t i = 0; i < 2; i++)
    {
      if (!IsNumeric(toks[i]))
      {
        errorMessage = "Argument " + StreamConvert(i + 1);
        errorMessage += " of the RescaleMinMax()-operator must be a numeric expression.";
        result.clear();
        return false;
      }
      result.push_back(StreamConvert(atof(toks[i].c_str()))); // double-conversion
    }
    if (atof(toks[0].c_str()) > atof(toks[1].c_str()))
    {
      errorMessage = "The <min> argument of the RescaleMinMax()-operator must not be greater than the <max> argument.";
      result.clear();
      return false;
    }
    return true;
  }
  else if (entry.substr(0, 18) == "RESCALEMINMAXMASK("
      && (operationFlags & RESCALEMINMAXMASK) == RESCALEMINMAXMASK)
  {
    entry = entry.substr(18, entry.length() - 19);
    Tokenize(entry, toks, ",");
    if (toks.size() != 2)
    {
      errorMessage = "The RescaleMinMaxMask()-operator requires exactly 2 arguments (<min>,<max>).";
      return false;
    }
    result.push_back("RESCALEMINMAXMASK");
    for (std::size_t i = 0; i < 2; i++)
    {
      if (!IsNumeric(toks[i]))
      {
        errorMessage = "Argument " + StreamConvert(i + 1);
        errorMessage += " of the RescaleMinMaxMask()-operator must be a numeric expression.";
        result.clear();
        return false;
      }
      result.push_back(StreamConvert(atof(toks[i].c_str()))); // double-conversion
    }
    if (atof(toks[0].c_str()) > atof(toks[1].c_str()))
    {
      errorMessage = "The <min> argument of the RescaleMinMaxMask()-operator must not be greater than the <max> argument.";
      result.clear();
      return false;
    }
    return true;
  }
  else if (entry.substr(0, 18) == "RESCALESHIFTSCALE("
      && (operationFlags & RESCALESHIFTSCALE) == RESCALESHIFTSCALE)
  {
    entry = entry.substr(18, entry.length() - 19);
    Tokenize(entry, toks, ",");
    if (toks.size() != 2)
    {
      errorMessage = "The RescaleShiftScale()-operator requires exactly 2 arguments (<shift>,<scale>).";
      return false;
    }
    result.push_back("RESCALESHIFTSCALE");
    for (std::size_t i = 0; i < 2; i++)
    {
      if (!IsNumeric(toks[i]))
      {
        errorMessage = "Argument " + StreamConvert(i + 1);
        errorMessage += " of the RescaleShiftScale()-operator must be a numeric expression.";
        result.clear();
        return false;
      }
      result.push_back(StreamConvert(atof(toks[i].c_str()))); // double-conversion
    }
    return true;
  }
  else if (entry.substr(0, 17) == "RESCALEWINDOWING("
      && (operationFlags & RESCALEWINDOWING) == RESCALEWINDOWING)
  {
    entry = entry.substr(17, entry.length() - 18);
    Tokenize(entry, toks, ",");
    if (toks.size() != 4)
    {
      errorMessage = "The RescaleWindowing()-operator requires exactly 4 arguments (<win-min>,<win-min>,<out-min>,<out-max>).";
      return false;
    }
    result.push_back("RESCALEWINDOWING");
    for (std::size_t i = 0; i < 4; i++)
    {
      if (!IsNumeric(toks[i]))
      {
        errorMessage = "Argument " + StreamConvert(i + 1);
        errorMessage += " of the RescaleWindowing()-operator must be a numeric expression.";
        result.clear();
        return false;
      }
      result.push_back(StreamConvert(atof(toks[i].c_str()))); // double-conversion
    }
    if (atof(toks[0].c_str()) > atof(toks[1].c_str()))
    {
      errorMessage = "The <win-min> argument of the RescaleWindowing()-operator must not be greater than the <win-max> argument.";
      result.clear();
      return false;
    }
    if (atof(toks[2].c_str()) > atof(toks[3].c_str()))
    {
      errorMessage = "The <out-min> argument of the RescaleWindowing()-operator must not be greater than the <out-max> argument.";
      result.clear();
      return false;
    }
    return true;
  }
  else if (entry.substr(0, 15) == "UNSHARPMASKING("
      && (operationFlags & UNSHARPMASKING) == UNSHARPMASKING)
  {
    entry = TrimF(entry.substr(15, entry.length() - 16));
    toks.clear();
    if (entry.length() > 0)
    {
      Tokenize(entry, toks, ",");
      if (toks.size() != 1)
      {
        errorMessage = "The UnsharpMasking()-operator requires no or 1 argument ([<blur-radius>]).";
        return false;
      }
    }
    result.push_back("UNSHARPMASKING");
    if (toks.size() > 0 && !IsStrictlyNumeric(toks[0]))
    {
      errorMessage = "The <blur-radius> argument of UnsharpMasking() must be numeric (integer).";
      return false;
    }
    if (toks.size() > 0) // heuristic
    {
      result.push_back(StreamConvert(atoi(toks[0].c_str())));
    } // else: heuristic radius
    return true;
  }
  else if (entry.substr(0, 6) == "STORE(" && (operationFlags & STORE) == STORE)
  {
    entry = normalCaseEntry.substr(6, normalCaseEntry.length() - 7);
    Tokenize(entry, toks, ",");
    if (toks.size() != 1)
    {
      errorMessage = "The Store()-operator requires exactly one argument (<file-name>).";
      return false;
    }
    result.push_back("STORE");
    result.push_back(toks[0]);
    return true;
  }

  errorMessage = "Unknown pre-processing entry: '" + origEntry;
  errorMessage += "'.";
  return false;
}

bool REG23Model::HaveValidConfiguration()
{
  return m_ValidConfiguration;
}

bool REG23Model::HaveORAConnectivity()
{
  return (m_SiteInfo != NULL);
}

bool REG23Model::HaveValidImageFileNames()
{
  bool ok = true;
  if (m_VolumeImageFileName.length() <= 0 || !itksys::SystemTools::FileExists(
        m_VolumeImageFileName.c_str()))
    ok = false;
  if (m_FixedImageFileNames.size() <= 0)
    ok = false;
  for (std::size_t i = 0; i < m_FixedImageFileNames.size(); i++)
  {
    if (m_FixedImageFileNames[i].length() <= 0
        || !itksys::SystemTools::FileExists(m_FixedImageFileNames[i].c_str()))
      ok = false;
  }
  return ok;
}

bool REG23Model::ParseEntryHasVariable(const std::string entry) const
{
  std::string s = ToLowerCaseF(entry);
  bool result = s.find("drrmin", 0) != std::string::npos ||
                s.find("drrmax", 0) != std::string::npos ||
                s.find("drrmean", 0) != std::string::npos ||
                s.find("drrstd", 0) != std::string::npos ||
                s.find("drrminmask", 0) != std::string::npos ||
                s.find("drrmaxmask", 0) != std::string::npos ||
                s.find("drrmeanmask", 0) != std::string::npos ||
                s.find("drrstdmask", 0) != std::string::npos;
  return result;
}

ImageImporterTask *REG23Model::GenerateVolumeImporterTask()
{
  if (m_VolumeImageFileName.length() <= 0 || !itksys::SystemTools::FileExists(
        m_VolumeImageFileName.c_str()))
    return NULL;

  ImageImporterTask *task = new ImageImporterTask();
  task->SetImportModeToVolume();
  task->SetImageFileName(m_VolumeImageFileName.c_str());
  task->SetSiteInfo(m_SiteInfo); // ORA-connectivity
  task->SetPatientUID(m_PatientUID.c_str());
  task->SetVolumeIsoCenter(m_IsoCenter); // in cm
  task->SetTargetImageConsumer(m_Volume);
  task->ForceUnexecutableState(false); // force it to be NON-UNEXECUTABLE!

  return task;
}

std::vector<ImageImporterTask *> REG23Model::GenerateFixedImageImporterTasks()
{
  std::vector<ImageImporterTask *> ftasks;
  bool ok = true;
  if (m_FixedImageFileNames.size() <= 0)
    ok = false;
  for (std::size_t i = 0; i < m_FixedImageFileNames.size(); i++)
  {
    if (m_FixedImageFileNames[i].length() <= 0
        || !itksys::SystemTools::FileExists(m_FixedImageFileNames[i].c_str()))
      ok = false;
  }
  if (ok)
  {
    for (std::size_t i = 0; i < m_FixedImageFileNames.size(); i++)
    {
      ImageImporterTask *task = new ImageImporterTask();
      task->SetImportModeToPlanarImage();
      task->SetImageFileName(m_FixedImageFileNames[i].c_str());
      task->SetSiteInfo(m_SiteInfo); // ORA-connectivity
      task->SetPatientUID(m_PatientUID.c_str());
      task->SetTargetImageConsumer(m_FixedImages[i]);
      task->ForceUnexecutableState(false); // force it to be NON-UNEXECUTABLE!
      ftasks.push_back(task);
    }
  }
  return ftasks;
}

REG23RegistrationInitializationTask *REG23Model::GenerateInitializationTask()
{
  if (!m_ValidConfiguration) // it's senseless ...
    return NULL;
  REG23RegistrationInitializationTask *task =
    new REG23RegistrationInitializationTask();
  task->SetTargetModel(this);
  // all other "input"-checks are directly done in the task!
  return task;
}

REG23RegistrationExecutionTask *REG23Model::GenerateExecutionTask()
{
  if (!IsReadyForAutoRegistration()) // senseless ...
    return NULL;
  REG23RegistrationExecutionTask *task =
      new REG23RegistrationExecutionTask();
  task->SetTargetModel(this);
  // all other "input"-checks are directly done in the task!
  return task;
}

REG23SparseRegistrationExecutionTask *REG23Model::GenerateSparseExecutionTask()
{
  if (!IsReadyForManualRegistration()) // senseless ...
    return NULL;
  if (!m_ExecuteSparsePreRegistration) // no requested!
    return NULL;
  if (m_SparsePreRegistrationType == "NONE") // no pre-registration
    return NULL;
  REG23SparseRegistrationExecutionTask *task =
      new REG23SparseRegistrationExecutionTask();
  task->SetTargetModel(this);
  // all other "input"-checks are directly done in the task!
  return task;
}

std::vector<std::string> REG23Model::GetOptimizerAndMetricTypeStrings()
{
  std::vector<std::string> v;
  if (IsReadyForAutoRegistration())
  {
    // optimizer type
    v.push_back(ToUpperCaseF(TrimF(m_Config->ReadString("Registration",
        "OptimizerType", ""))));
    int i = 0;
    std::string s = "";
    do
    {
      i++;
      s = ToUpperCaseF(TrimF(m_Config->ReadString("Registration",
              "MetricType" + StreamConvert(i), "")));
      if (s.length() > 0)
        v.push_back(s);
    } while (s.length() > 0);
  }
  return v;
}

void REG23Model::SetTaskManager(TaskManager *taskman)
{
  m_TaskManager = taskman;
}

TaskManager *REG23Model::GetTaskManager()
{
  return m_TaskManager;
}

bool REG23Model::FixAutoSourcePositions()
{
  if (m_SourcePositions.size() != m_FixedImages.size()) // must be prepared
    return false;
  double *pos = NULL;
  for (std::size_t i = 0; i < m_SourcePositions.size(); i++)
  {
    if (!m_SourcePositions[i]) // only prepared, not filled, yet!
    {
      // (has been check in load-config that the corresponding images are RTI)
      ITKVTKImage *image = m_FixedImages[i]->ProduceImage();
      ITKVTKImageMetaInformation::Pointer mi = image->GetMetaInfo();
      if (!mi)
        return false;
      if (!mi->GetVolumeMetaInfo()->GetGantryValid() ||
          !mi->GetVolumeMetaInfo()->GetSourceAxisDistanceValid() ||
          !mi->GetVolumeMetaInfo()->GetSourceFilmDistanceValid())
        return false;
      typedef ora::PortalImagingDeviceProjectionProperties<DRRPixelType> PropsType;
      PropsType::Pointer props = PropsType::New();
      props->SetSourceAxisDistance(mi->GetVolumeMetaInfo()->GetSourceAxisDistance());
      props->SetSourceFilmDistance(mi->GetVolumeMetaInfo()->GetSourceFilmDistance());
      PropsType::SizeType psz;
      psz[0] = 410;
      psz[1] = 410;
      props->SetProjectionSize(psz); // essentially unimportant here
      PropsType::SpacingType psp;
      psp[0] = 1.0;
      psp[1] = 1.0;
      props->SetProjectionSpacing(psp); // essentially unimportant here
      props->SetLinacGantryDirection(0); // CW
      double gantry = mi->GetVolumeMetaInfo()->GetGantry() + m_GantryOffsetAngle[i];
      props->SetLinacGantryAngle(gantry / 180. * vtkMath::Pi());
      if (!props->Update())
        return false;
      pos = new double[3];
      for (int d = 0; d < 3; d++)
        pos[d] = props->GetSourceFocalSpotPosition()[d];
      m_SourcePositions[i] = pos;
    }
  }
  return true;
}

bool REG23Model::LoadStructure(unsigned int i)
{
  std::vector<VTKStructure *> structs = m_Structures->GetStructures();
  if (i >= structs.size() || !m_IsoCenter)
    return false;
  VTKStructure *st = structs[i];
  if (st->GetFileName().length() > 0)
  {
    vtkSmartPointer<vtkORAStructureReader> r =
        vtkSmartPointer<vtkORAStructureReader>::New();
    r->SetFileName(st->GetFileName().c_str());
    r->SetGenerate3DSurfaceFrom2DContoursIfApplicable(true);
    r->SetGenerateSurfaceNormals(true);
    if (!r->CanReadFile(st->GetFileName().c_str()))
      return false;
    r->SetReadColorsAsWell(false);
    r->Update();
    if (!r->GetOutput())
      return false;
    vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
    t->Identity();
    t->Translate(-10 * m_IsoCenter[0], -10 * m_IsoCenter[1], -10 * m_IsoCenter[2]);
    vtkSmartPointer<vtkTransformPolyDataFilter> tf =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    tf->SetTransform(t);
    tf->SetInput(r->GetOutput());
    tf->Update();
    st->SetStructurePolyData(tf->GetOutput());
    IniAccess *stInfo = r->GetInfoFileAccess();
    vtkSmartPointer<vtkProperty> prop = vtkSmartPointer<vtkProperty>::New();
    double *c = NULL;
    std::string s = stInfo->ReadString("WireFrame", "ColorRGB", "");
    if (ParseCommaSeparatedNumericStringVector(s, c) < 3)
      return false;
    prop->SetColor(c[0], c[1], c[2]);
    delete [] c;
    s = stInfo->ReadString("WireFrame", "Width", "");
    if (ParseCommaSeparatedNumericStringVector(s, c) < 1)
      return false;
    prop->SetLineWidth(c[0]);
    delete [] c;
    st->SetContourProperty(prop);
  }
  return true;
}

ITKVTKImage *REG23Model::GeneratePerspectiveProjectionMask(unsigned int i,
      VTKStructure *structure)
{
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
  if (i >= m_FixedImages.size())
#else
  if (i >= m_FixedImages.size() || !m_ToolRenderWindow)
#endif
    return NULL;
  if (!structure)
    return NULL;
  ITKVTKImage::ITKImagePointer ibase = m_PPFixedImages[i]->ProduceImage()->
      GetAsITKImage<DRRPixelType>();
  typedef itk::Image<DRRPixelType, ITKVTKImage::Dimensions> OriginalImageType;
  typedef OriginalImageType::Pointer OriginalImagePointer;
  OriginalImageType::Pointer icast = static_cast<OriginalImageType * >(ibase.GetPointer());
  ITKVTKImage *resultImage = NULL;

  vtkSmartPointer<vtkSurfaceToPerspectiveProjectionImageFilter> imager =
      vtkSmartPointer<vtkSurfaceToPerspectiveProjectionImageFilter>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = NULL;
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
  renWin = vtkSmartPointer<vtkRenderWindow>::New(); // win32: create own (hidden)
#else
  renWin = m_ToolRenderWindow;
#endif
  if (!imager->ConnectToRenderWindow(renWin))
    return NULL;
  imager->SetUseThreadSafeRendering(true); // use thread-safe rendering
  imager->AddInput(structure->GetStructurePolyData());
  imager->SetSourcePosition(m_SourcePositions[i]);
  double plnOrig[3];
  for (int d = 0; d < 3; d++)
    plnOrig[d] = icast->GetOrigin()[d];
  imager->SetPlaneOrigin(plnOrig);
  int plnSz[2];
  plnSz[0] = icast->GetLargestPossibleRegion().GetSize(0);
  plnSz[1] = icast->GetLargestPossibleRegion().GetSize(1);
  imager->SetPlaneSizePixels(plnSz);
  double plnSp[2];
  plnSp[0] = icast->GetSpacing()[0];
  plnSp[1] = icast->GetSpacing()[1];
  imager->SetPlaneSpacing(plnSp);
  vtkSmartPointer<vtkMatrix3x3> plnOrient = vtkSmartPointer<vtkMatrix3x3>::New();
  plnOrient->Identity();
  OriginalImageType::DirectionType dir = icast->GetDirection();
  for (int d1 = 0; d1 < 3; d1++)
    for (int d2 = 0; d2 < 3; d2++)
      plnOrient->SetElement(d1, d2, dir[d1][d2]);
  plnOrient->Transpose(); // rows are columns in ITK image!
  imager->SetPlaneOrientation(plnOrient);
  if (!imager->IsProjectionGeometryValid())
    return NULL;


#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
  ConnectRenderWindowToGlobalLock(renWin);
#endif

  imager->Update();

#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
  DisconnectRenderWindowFromGlobalLock(renWin);
#endif

  if (!imager->GetOutput())
    return NULL;
  vtkImageData *maskImage = imager->GetOutput();
  resultImage = new ITKVTKImage(itk::ImageIOBase::SCALAR, itk::ImageIOBase::UCHAR);
  resultImage->SetVTKImage(maskImage, false);

  ITKVTKImage::ITKImagePointer baseMask = resultImage->
      GetAsITKImage<MaskPixelType>();
  typedef itk::Image<MaskPixelType, ITKVTKImage::Dimensions> ITKMaskImageType;
  ITKMaskImageType::Pointer castMask = static_cast<ITKMaskImageType * >(
      baseMask.GetPointer());
  if (!castMask)
  {
    delete resultImage;
    return NULL;
  }
  castMask->SetDirection(dir); // since VTK image have no direction
  ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
  resultImage->SetMetaInfo(mi); // dummy
  imager = NULL;

  return resultImage;
}

bool REG23Model::ParseAndProcessMaskEntry(int index, bool simulation,
      std::string &errorKey, std::string &errorMessage, ITKVTKImage *&finalMask)
{
  errorKey = "";
  errorMessage = "";
  if (!m_Config || index < 1)
    return false;
  errorKey = "Mask" + StreamConvert(index);
  std::string baseKey = errorKey + ".Step";
  if (TrimF(m_Config->ReadString("Auto-Masking", baseKey + "1", "")).length() <= 0)
    return true; // do not process completely empty entries in this method!
  int step = 1;
  bool commandFound = true, bracketMissing = false;
  std::string s = "";
  std::string term = "";
  std::string cmdType = "";
  std::map<std::string, ITKVTKImage *> tempmasks;
  finalMask = NULL;
  bool hadMaskCommand = false;
  do
  {
    errorKey = baseKey + StreamConvert(step);
    s = m_Config->ReadString("Auto-Masking", errorKey, "");
    // remove all spaces, case-insensitive:
    while (ReplaceString(s, " ", ""));
    if (s.length() <= 0)
      break;

    std::vector<std::string> v;
    std::vector<std::string> args; // command-specific arguments
    v.clear();
    args.clear();
    Tokenize(s, v, ",");
    commandFound = true;
    bracketMissing = false;
    term = "";
    if (v.size() == 7 || v.size() == 6)  // HoleFill()
    {
      if (ToUpperCaseF(v[0].substr(0, 9)) == "HOLEFILL(")
      {
        cmdType = "holefill";
        if (v[5].length() > 0 && v[5].substr(v[5].length() - 1, 1) == ")")
        {
          args.push_back(v[0].substr(9, v[0].length() - 9)); // mask UID
          args.push_back(v[1].substr(0, v[1].length())); // radiusX
          args.push_back(v[2].substr(0, v[2].length())); // radiusY
          args.push_back(v[3].substr(0, v[3].length())); // invert
          args.push_back(v[4].substr(0, v[4].length())); // majorityThreshold
          args.push_back(v[5].substr(0, v[5].length() - 1)); // numberOfIterations
          if (args[0] != "-")  // use sub-mask
            args.push_back(v[6].substr(0, v[6].length())); // new mask UID

          if (args[0] == "-" && !hadMaskCommand)
          {
            errorMessage = "The final mask cannot be stored at this point.";
            return false;
          }
          if (args[0] != "-" && tempmasks.find(args[0]) == tempmasks.end())
          {
            errorMessage = "The specified mask UID does not exist: " + args[0];
            errorMessage += ".";
            return false;
          }
          if (args[0] != "-" && tempmasks.find(v[6]) != tempmasks.end())
          {
            errorMessage = "The specified submask UID is already existing: "
                + v[6];
            errorMessage += ".";
            return false;
          }
          // check arguments
          int radiusX = atoi(args[1].c_str());
          int radiusY = atoi(args[2].c_str());
          int invert = atoi(args[3].c_str());
          int majorityThreshold = atoi(args[4].c_str());
          int numberOfIterations = atoi(args[5].c_str());
          if (radiusX < 0 || radiusY < 0)
          {
            errorMessage = "The specified hole fill radius must be >=0: "
                + radiusX;
            errorMessage += ", " + radiusY;
            errorMessage += ".";
            return false;
          }
          if (majorityThreshold < 0)
          {
            errorMessage = "The specified hole fill majority threshold must be >=0: "
                + majorityThreshold;
            errorMessage += ".";
            return false;
          }
          if (numberOfIterations <= 0)
          {
            errorMessage = "The specified hole fill number of iterations must be >0: "
                + numberOfIterations ;
            errorMessage += ".";
            return false;
          }

          // PROCESS: HOLEFILL
          if (!simulation)
          {
            ITKVTKImage *mask = NULL;
            if (args[0] == "-") // final mask
            {
              mask = HoleFillMaskImage(finalMask, radiusX, radiusY, invert,
                  majorityThreshold, numberOfIterations);
              if (!mask)
                return false;
              finalMask = mask;
            }
            else // submask
            {
              //mask = HoleFillMaskImage(tempmasks[maskuids[0]], atoi(args[1].c_str()),
              mask = HoleFillMaskImage(tempmasks[args[0]], radiusX, radiusY, invert,
                  majorityThreshold, numberOfIterations);
              if (!mask)
                return false;
              tempmasks[args[6]] = mask;
            }
          }
          else if (args[0] != "-") // SIMULATION: -> add submask dummy
          {
            tempmasks[args[6]] = NULL;
          }
        }
        else
        {
          bracketMissing = true;
        }
      }
    }
    else if (v.size() == 4)  // Rect()
    {
      if (ToUpperCaseF(v[0].substr(0, 5)) == "RECT(")
      {
        cmdType = "rect";
        if (v[2].length() > 0 && v[2].substr(v[2].length() - 1, 1) == ")")
        {
          args.push_back(v[0].substr(5, v[0].length() - 5)); // mask UID
          args.push_back(v[1].substr(0, v[1].length())); // width
          args.push_back(v[2].substr(0, v[2].length() - 1)); // height
          args.push_back(v[3].substr(0, v[3].length())); // new mask UID

          if (args[0] == "-")
          {
            errorMessage = "The final mask cannot be rectangular masked.";
            return false;
          }
          if (args[0] != "" && tempmasks.find(args[0]) == tempmasks.end())
          {
            errorMessage = "The specified mask UID does not exist: " + args[0];
            errorMessage += ".";
            return false;
          }
          if (tempmasks.find(v[3]) != tempmasks.end())
          {
            errorMessage = "The specified submask UID is already existing: "
                + v[3];
            errorMessage += ".";
            return false;
          }
          // check arguments
          double width = atof(args[1].c_str());
          double height = atof(args[2].c_str());
          if (width < 0. || height < 0.)
          {
            errorMessage = "The specified rectangular mask width and height must be >=0: "
                + StreamConvert(width);
            errorMessage += ", " + StreamConvert(height);
            errorMessage += ".";
            return false;
          }

          // PROCESS: RECT
          if (!simulation)
          {
            ITKVTKImage *mask = NULL;
            if (args[0] != "")
            {
              // on submask
              mask = DrawCenteredRectangleToMaskImage(tempmasks[args[0]], width, height);
            }
            else
            {
              // FIXME: Create new mask image from scratch (properties of fixed image)
              // mask = DrawCenteredRectangleToMaskImage(, width, height);
              errorMessage = "A temporary mask must exist.";
              return false;
            }
            if (!mask)
              return false;
            tempmasks[args[3]] = mask;
          }
          else // SIMULATION: -> add submask dummy
          {
            tempmasks[args[3]] = NULL;
          }
        }
        else
        {
          bracketMissing = true;
        }
      }
    }
    else if (v.size() == 2) // TempMask(), Store()
    {
      if (ToUpperCaseF(v[0].substr(0, 9)) == "TEMPMASK(")
      {
        cmdType = "tempmask";
        if (hadMaskCommand) // ignore if Mask()-command has occurred
        {
          step++;
          continue;
        }
        if (v[0].substr(v[0].length() - 1, 1) == ")")
        {
          term = v[0].substr(9, v[0].length() - 10); // process later
          if (tempmasks.find(v[1]) != tempmasks.end())
          {
            errorMessage = "The specified submask UID is already existing: " + v[1];
            errorMessage += ".";
            return false;
          }
          args.push_back(v[1]);
        }
        else
        {
          bracketMissing = true;
        }
      }
      else if (ToUpperCaseF(v[0].substr(0, 6)) == "STORE(")
      {
        cmdType = "store";
        if (v[1].length() > 0 && v[1].substr(v[1].length() - 1, 1) == ")")
        {
          args.push_back(v[0].substr(6, v[0].length())); // mask UID
          args.push_back(v[1].substr(0, v[1].length() - 1)); // file name
          if (args[0] != "-" &&
              tempmasks.find(args[0]) == tempmasks.end())
          {
            errorMessage = "The specified mask UID does not exist: " + args[0];
            errorMessage += ".";
            return false;
          }
          if (args[0] == "-" && !hadMaskCommand)
          {
            errorMessage = "The final mask cannot be stored at this point.";
            return false;
          }
          if (!simulation) // PROCESS: STORE
          {
            if (args[0] == "-") // final mask
            {
              if (!finalMask->SaveImageAsORAMetaImage<MaskPixelType>(args[1], false, ""))
                return false;
            }
            else // submask
            {
              ITKVTKImage *mask = tempmasks[args[0]];
              if (!mask ||
                  !mask->SaveImageAsORAMetaImage<MaskPixelType>(args[1], false, ""))
                return false;
            }
          }
        }
        else
        {
          bracketMissing = true;
        }
      }
      else
      {
        commandFound = false;
      }
    }
    else if (v.size() == 1) // Mask()
    {
      if (ToUpperCaseF(v[0].substr(0, 5)) == "MASK(")
      {
        cmdType = "mask";
        if (hadMaskCommand)
        {
          errorMessage = "The Mask()-command can only occur once per mask!";
          return false;
        }
        if (v[0].substr(v[0].length() - 1, 1) == ")")
        {
          term = v[0].substr(5, v[0].length() - 6); // process later
          hadMaskCommand = true; // mark!
        }
        else
        {
          bracketMissing = true;
        }
      }
      else
      {
        commandFound = false;
      }
    }
    else
    {
      commandFound = false;
    }
    if (!commandFound)
    {
      errorMessage = "Obviously an unknown command: " + s;
      errorMessage += ".";
      return false;
    }
    if (bracketMissing)
    {
      errorMessage = "Trailing ')' in command missing.";
      return false;
    }

    if (term.length() > 0) // TempMask(), Mask()
    {
      // analyze the term:
      bool inverted = false;
      bool isStructure = false;
      bool combination = false;
      std::string combinationType = "";
      std::vector<std::string> maskuids;
      std::string structuid = "";
      // case 1: no +/- -> could be structure or submask (copy), ! possible
      if (term.find("|", 0) == std::string::npos &&
          term.find("&", 0) == std::string::npos &&
          term.find("^", 0) == std::string::npos)
      {
        if (term.substr(0, 1) == "!")
        {
          inverted = true;
          term = term.substr(1, term.length());
        }
        if (m_Structures->FindStructure(term))
        {
          isStructure = true;
          structuid = term;
        }
        if (!isStructure && tempmasks.find(term) == tempmasks.end())
        {
          errorMessage = "The specified structure / submask UID does not exist: " + term;
          errorMessage += ".";
          return false;
        }
        if (!isStructure)
          maskuids.push_back(term);
      }
      else // case 2: +/- of mask uids
      {
        if (term.find("!", 0) != std::string::npos)
        {
          errorMessage = "The inversion operator (!) cannot be mixed with &/|/^ operators!";
          return false;
        }
        combination = true;
        if (term.find("|", 0) != std::string::npos)
        {
          combinationType = "|";
        }
        else if (term.find("&", 0) != std::string::npos)
        {
          combinationType = "&";
        }
        else if (term.find("^", 0) != std::string::npos)
        {
          combinationType = "^";
        }
        std::vector<std::string> v2;
        Tokenize(term, v2, combinationType);
        if (v2.size() != 2)
        {
          errorMessage = "Please note that +/- are binary operators, requiring 2 arguments!";
          return false;
        }
        if (tempmasks.find(v2[0]) == tempmasks.end())
        {
          errorMessage = "The specified submask UID does not exist: " + v2[0];
          errorMessage += ".";
          return false;
        }
        if (tempmasks.find(v2[1]) == tempmasks.end())
        {
          errorMessage = "The specified submask UID does not exist: " + v2[1];
          errorMessage += ".";
          return false;
        }
        maskuids.push_back(v2[0]);
        maskuids.push_back(v2[1]);
      }

      if (!simulation)
      {
        ITKVTKImage *mask = NULL;
        if (!combination && isStructure)
        {
          mask = GeneratePerspectiveProjectionMask(index - 1,
              m_Structures->FindStructure(structuid));
        }
        else if (!combination && !isStructure) // copy
        {
          mask = CopyMaskImage(tempmasks[maskuids[0]]);
        }
        else if (combination && combinationType == "|")
        {
          mask = ApplyORToMaskImages(tempmasks[maskuids[0]], tempmasks[maskuids[1]]);
        }
        else if (combination && combinationType == "&")
        {
          mask = ApplyANDToMaskImages(tempmasks[maskuids[0]], tempmasks[maskuids[1]]);
        }
        else if (combination && combinationType == "^")
        {
          mask = ApplyXORToMaskImages(tempmasks[maskuids[0]], tempmasks[maskuids[1]]);
        }

        if (mask && inverted)
        {
          mask = InvertMaskImage(mask);
        }

        if (!mask) // could obviously not be generated
        {
          return false;
        }

        if (cmdType == "mask") // PROCESS: MASK
        {
          finalMask = mask;
        }
        else if (cmdType == "tempmask") // PROCESS: TEMPMASK
        {
          tempmasks[args[0]] = mask;
        }
      }
      else if (cmdType == "tempmask") // SIMULATION: -> add submask dummies
      {
        tempmasks[args[0]] = NULL;
      }
    }

    step++;
  } while (true);

  // remove temporary masks from heap:
  std::map<std::string, ITKVTKImage *>::iterator it;
  for (it = tempmasks.begin(); it != tempmasks.end(); ++it)
  {
    if (it->second)
      delete it->second;
  }
  tempmasks.clear();

  if ((!simulation && !finalMask) || (simulation && !hadMaskCommand))
  {
    errorMessage = "No Mask()-command detected, but this is mandatory!";
    return false;
  }

  // write out intelligent mask if the mode is active
  if (!simulation && IsIntelligentMaskMode())
  {
    return WriteIntelligentMaskAndInfo(index, finalMask);
  }

  return true;
}

ITKVTKImage *REG23Model::CopyMaskImage(ITKVTKImage *src)
{
  if (!src || !src->GetAsITKImage<MaskPixelType>())
    return NULL;

  typedef itk::Image<MaskPixelType, ITKVTKImage::Dimensions> ITKMaskImageType;
  typedef itk::ImageDuplicator<ITKMaskImageType> DuplicatorType;

  ITKMaskImageType::Pointer mask = static_cast<ITKMaskImageType * >(
      src->GetAsITKImage<MaskPixelType>().GetPointer());

  DuplicatorType::Pointer dupl = DuplicatorType::New();
  dupl->SetInputImage(mask);
  dupl->Update();
  if (dupl->GetOutput())
  {
    ITKVTKImage *copy = new ITKVTKImage(itk::ImageIOBase::SCALAR,
        itk::ImageIOBase::UCHAR);
    copy->SetITKImage(dupl->GetOutput(), false);
    ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
    copy->SetMetaInfo(mi); // dummy
    return copy;
  }
  else
  {
    return NULL;
  }
}

ITKVTKImage *REG23Model::InvertMaskImage(ITKVTKImage *src)
{
  if (!src || !src->GetAsITKImage<MaskPixelType>())
    return NULL;

  typedef itk::Image<MaskPixelType, ITKVTKImage::Dimensions> ITKMaskImageType;
  typedef itk::ImageDuplicator<ITKMaskImageType> DuplicatorType;
  typedef itk::InvertIntensityImageFilter<ITKMaskImageType, ITKMaskImageType>
    InverterType;

  ITKMaskImageType::Pointer mask = static_cast<ITKMaskImageType * >(
      src->GetAsITKImage<MaskPixelType>().GetPointer());

  InverterType::Pointer inverter = InverterType::New();
  inverter->SetInput(mask);
  inverter->SetMaximum(255);
  try
  {
    inverter->Update();
    ITKVTKImage *inverted = new ITKVTKImage(itk::ImageIOBase::SCALAR,
        itk::ImageIOBase::UCHAR);
    inverted->SetITKImage(inverter->GetOutput(), false);
    ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
    inverted->SetMetaInfo(mi); // dummy
    return inverted;
  }
  catch (itk::ExceptionObject &e)
  {
    return NULL;
  }
}

ITKVTKImage *REG23Model::ApplyORToMaskImages(ITKVTKImage *src1, ITKVTKImage *src2)
{
  if (!src1 || !src1->GetAsITKImage<MaskPixelType>() ||
      !src2 || !src2->GetAsITKImage<MaskPixelType>())
    return NULL;

  typedef itk::Image<MaskPixelType, ITKVTKImage::Dimensions> ITKMaskImageType;
  typedef itk::ImageDuplicator<ITKMaskImageType> DuplicatorType;
  typedef itk::OrImageFilter<ITKMaskImageType> ORType;

  ITKMaskImageType::Pointer mask1 = static_cast<ITKMaskImageType * >(
      src1->GetAsITKImage<MaskPixelType>().GetPointer());
  ITKMaskImageType::Pointer mask2 = static_cast<ITKMaskImageType * >(
      src2->GetAsITKImage<MaskPixelType>().GetPointer());

  ORType::Pointer orFilter = ORType::New();
  orFilter->SetInput1(mask1);
  orFilter->SetInput2(mask2);
  try
  {
    orFilter->Update();
    ITKVTKImage *orMask = new ITKVTKImage(itk::ImageIOBase::SCALAR,
        itk::ImageIOBase::UCHAR);
    orMask->SetITKImage(orFilter->GetOutput(), false);
    ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
    orMask->SetMetaInfo(mi); // dummy
    return orMask;
  }
  catch (itk::ExceptionObject &e)
  {
    return NULL;
  }
}

ITKVTKImage *REG23Model::ApplyANDToMaskImages(ITKVTKImage *src1, ITKVTKImage *src2)
{
  if (!src1 || !src1->GetAsITKImage<MaskPixelType>() ||
      !src2 || !src2->GetAsITKImage<MaskPixelType>())
    return NULL;

  typedef itk::Image<MaskPixelType, ITKVTKImage::Dimensions> ITKMaskImageType;
  typedef itk::ImageDuplicator<ITKMaskImageType> DuplicatorType;
  typedef itk::AndImageFilter<ITKMaskImageType> ANDType;

  ITKMaskImageType::Pointer mask1 = static_cast<ITKMaskImageType * >(
      src1->GetAsITKImage<MaskPixelType>().GetPointer());
  ITKMaskImageType::Pointer mask2 = static_cast<ITKMaskImageType * >(
      src2->GetAsITKImage<MaskPixelType>().GetPointer());

  ANDType::Pointer andFilter = ANDType::New();
  andFilter->SetInput1(mask1);
  andFilter->SetInput2(mask2);
  try
  {
    andFilter->Update();
    ITKVTKImage *andMask = new ITKVTKImage(itk::ImageIOBase::SCALAR,
        itk::ImageIOBase::UCHAR);
    andMask->SetITKImage(andFilter->GetOutput(), false);
    ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
    andMask->SetMetaInfo(mi); // dummy
    return andMask;
  }
  catch (itk::ExceptionObject &e)
  {
    return NULL;
  }
}

ITKVTKImage *REG23Model::ApplyXORToMaskImages(ITKVTKImage *src1, ITKVTKImage *src2)
{
  if (!src1 || !src1->GetAsITKImage<MaskPixelType>() ||
      !src2 || !src2->GetAsITKImage<MaskPixelType>())
    return NULL;

  typedef itk::Image<MaskPixelType, ITKVTKImage::Dimensions> ITKMaskImageType;
  typedef itk::ImageDuplicator<ITKMaskImageType> DuplicatorType;
  typedef itk::XorImageFilter<ITKMaskImageType> XORType;

  ITKMaskImageType::Pointer mask1 = static_cast<ITKMaskImageType * >(
      src1->GetAsITKImage<MaskPixelType>().GetPointer());
  ITKMaskImageType::Pointer mask2 = static_cast<ITKMaskImageType * >(
      src2->GetAsITKImage<MaskPixelType>().GetPointer());

  XORType::Pointer xorFilter = XORType::New();
  xorFilter->SetInput1(mask1);
  xorFilter->SetInput2(mask2);
  try
  {
    xorFilter->Update();
    ITKVTKImage *xorMask = new ITKVTKImage(itk::ImageIOBase::SCALAR,
        itk::ImageIOBase::UCHAR);
    xorMask->SetITKImage(xorFilter->GetOutput(), false);
    ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
    xorMask->SetMetaInfo(mi); // dummy
    return xorMask;
  }
  catch (itk::ExceptionObject &e)
  {
    return NULL;
  }
}

ITKVTKImage *REG23Model::HoleFillMaskImage(ITKVTKImage *src,
    const unsigned int radiusX, const unsigned int radiusY, const bool invert,
    const unsigned int majorityThreshold, const unsigned int numberOfIterations) const
{
  if (!src || !src->GetAsITKImage<MaskPixelType>())
    return NULL;

  typedef itk::Image<MaskPixelType, ITKVTKImage::Dimensions> ITKMaskImageType;
  typedef itk::ImageDuplicator<ITKMaskImageType> DuplicatorType;
  typedef itk::VotingBinaryIterativeHoleFillingImageFilter<ITKMaskImageType>
    HoleFillerType;

  ITKMaskImageType::Pointer mask = static_cast<ITKMaskImageType * >(
      src->GetAsITKImage<MaskPixelType>().GetPointer());

  HoleFillerType::Pointer holefiller = HoleFillerType::New();
  holefiller->SetInput(mask);
  holefiller->SetNumberOfThreads(1);

  // Set the radius of the neighborhood used to compute the median
  ITKMaskImageType::SizeType indexRadius;
  indexRadius[0] = radiusX; // radius along x
  indexRadius[1] = radiusY; // radius along y
  indexRadius[2] = 0; // 2D Image (IMPORTANT to set to zero! otherwise uninitialized)
  holefiller->SetRadius(indexRadius);
  // Set Foreground (object) and the Background of the binary input image
  if (invert)
  {
    holefiller->SetBackgroundValue(255);
    holefiller->SetForegroundValue(0);
  }
  else
  {
    holefiller->SetBackgroundValue(0);
    holefiller->SetForegroundValue(255);
  }
  // Set majority threshold (decision criterion)
  holefiller->SetMajorityThreshold(majorityThreshold);
  // Set maximum number of iterations
  holefiller->SetMaximumNumberOfIterations( numberOfIterations );

  try
  {
    holefiller->Update();
    ITKVTKImage *holefilled = new ITKVTKImage(itk::ImageIOBase::SCALAR,
        itk::ImageIOBase::UCHAR);
    holefilled->SetITKImage(holefiller->GetOutput(), false);
    ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
    holefilled->SetMetaInfo(mi); // dummy
    return holefilled;
  }
  catch (itk::ExceptionObject &e)
  {
    std::cout << "Exception object caught at hole filling!" << e << std::endl;
    return NULL;
  }
}

ITKVTKImage *REG23Model::DrawCenteredRectangleToMaskImage(ITKVTKImage *src,
    const double maskWidth, const double maskHeight)
{
  if (!src || !src->GetAsITKImage<MaskPixelType>())
    return NULL;
  if (maskWidth <= 0 || maskHeight <= 0)
    return NULL;

  typedef itk::Image<MaskPixelType, ITKVTKImage::Dimensions> ITKMaskImageType;
  typedef itk::ImageDuplicator<ITKMaskImageType> DuplicatorType;
  typedef itk::ImageRegionIteratorWithIndex<ITKMaskImageType>
      MaskIteratorWIType;
  typedef itk::ContinuousIndex<
      ITKMaskImageType::PointType::ValueType,
      ITKMaskImageType::ImageDimension> MaskContinuousIndexType;

  // Copy image
  ITKVTKImage *srcCopy = CopyMaskImage(src);
  if (!srcCopy || !srcCopy->GetAsITKImage<MaskPixelType>())
    return NULL;

  ITKMaskImageType::Pointer mask = static_cast<ITKMaskImageType * >(
      srcCopy->GetAsITKImage<MaskPixelType>().GetPointer());

  ITKMaskImageType::PointType origin = mask->GetOrigin();
  ITKMaskImageType::SizeType size = mask->GetLargestPossibleRegion().GetSize();
  ITKMaskImageType::SpacingType spacing = mask->GetSpacing();

  // Get continuous index of center pixel
  MaskContinuousIndexType cindexCenter;
  cindexCenter[0] = (size[0] - 1) / 2.0;
  cindexCenter[1] = (size[1] - 1) / 2.0;
  cindexCenter[2] = 0;  // in-plane

  // Get minimum and maximum index
  ITKMaskImageType::IndexType minIdx;
  minIdx[0] = vnl_math_rnd(cindexCenter[0] - 0.5 * maskWidth / spacing[0]);
  minIdx[1] = vnl_math_rnd(cindexCenter[1] - 0.5 * maskHeight / spacing[1]);
  minIdx[2] = 0;  // in-plane
  ITKMaskImageType::IndexType maxIdx;
  maxIdx[0] = vnl_math_rnd(cindexCenter[0] + 0.5 * maskWidth / spacing[0]);
  maxIdx[1] = vnl_math_rnd(cindexCenter[1] + 0.5 * maskHeight / spacing[1]);
  maxIdx[2] = 0;  // in-plane

  ITKMaskImageType::IndexType index;
  MaskIteratorWIType itMask(mask, mask->GetLargestPossibleRegion());
  for (itMask.GoToBegin(); !itMask.IsAtEnd(); ++itMask)
  {
    index = itMask.GetIndex();
    if (index[0] < minIdx[0] || index[1] < minIdx[1] || index[0] > maxIdx[0]
        || index[1] > maxIdx[1])
      ; // itMask.Set(0); // outside: Do nothing with already existing mask
    else
      itMask.Set(255); // inside
  }

  ITKVTKImage *newmask = new ITKVTKImage(itk::ImageIOBase::SCALAR,
      itk::ImageIOBase::UCHAR);
  newmask->SetITKImage(mask.GetPointer(), false);
  ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
  newmask->SetMetaInfo(mi); // dummy
  return srcCopy;
}

REG23Model::ITFPointer REG23Model::ParseITF(std::string entry,
    std::string &errorMessage)
{
  errorMessage = "";
  if (entry.length() <= 0)
  {
    errorMessage = "No values defined.";
    return NULL;
  }

  Trim(entry);
  std::string itfstr;
  if (ToUpperCaseF(entry).substr(0, 5) != "FILE:" &&
      ToUpperCaseF(entry).substr(0, 9) != "ITF-POOL(") // direct list
  {
    itfstr = entry;
  }
  else if (ToUpperCaseF(entry).substr(0, 9) == "ITF-POOL(") // ITF-pool
  {
    if (!m_ITFPool)
    {
      errorMessage = "The ITF-POOL() statement requires a valid ITF-POOL-File specification!";
      return NULL;
    }
    if (entry.substr(entry.length() - 1, 1) != ")")
    {
      errorMessage = "Closing ')' in ITF-POOL statement missing.";
      return NULL;
    }
    std::string ssecval = entry.substr(9, entry.length() - 10);
    std::vector<std::string> secval;
    Tokenize(ssecval, secval, ",");
    if (secval.size() != 2)
    {
      errorMessage = "Check whether section and key in ITF-POOL statement are correctly specified.";
      return NULL;
    }
    itfstr = TrimF(m_ITFPool->ReadString(TrimF(secval[0]), TrimF(secval[1]), ""));
    if (itfstr.length() <= 0)
    {
      errorMessage = "The specified section and key in ITF-POOL reference a non-existent or empty value.";
      return NULL;
    }
  }
  else // from file
  {
    std::string itffn = TrimF(entry.substr(5, entry.length()));
    if (!itksys::SystemTools::FileExists(itffn.c_str()))
    {
      errorMessage = "The specified ITF file appears to be non-existent!";
      return NULL;
    }
    std::ifstream ifs(itffn.c_str());
    std::string itfstri((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    itfstr = itfstri;
  }

  Trim(itfstr);
  double slope = 1.0; // linear intensity rescaling!
  double intercept = 0.0;
  if (ToUpperCaseF(itfstr).substr(0, 6) == "SCALE|")
  {
    itfstr = itfstr.substr(6, itfstr.length());
    Trim(itfstr);
    ITKVTKImage *volume = NULL;
    if (m_Volume)
      volume = m_Volume->ProduceImage();
    if (volume && volume->GetMetaInfo() &&
        volume->GetMetaInfo()->GetVolumeMetaInfo())
    {
      ITKVTKImageMetaInformation::Pointer mi = volume->GetMetaInfo();
      slope = mi->GetVolumeMetaInfo()->GetRescaleSlope();
      intercept = mi->GetVolumeMetaInfo()->GetRescaleIntercept();
    }
  }
  if (fabs(slope) < 1e-6)
  {
    errorMessage = "Volume defines an invalid rescale slope value (0.0)!";
    return NULL;
  }
  std::vector<std::string> tokens;
  Tokenize(itfstr, tokens, " ");
  if (tokens.size() < 5) // at least 2 supporting points required
  {
    errorMessage = "The ITF requires at least 2 supporting points!";
    return NULL;
  }
  int numPairs = atoi(tokens[0].c_str());
  if ((numPairs * 2) != (int) (tokens.size() - 1))
  {
    errorMessage = "Obviously, the specified number of ITF-pairs is invalid!";
    return NULL;
  }
  double maxVal = atof(tokens[2].c_str());
  for (std::size_t i = 2; i < tokens.size(); i += 2)
  {
    if (atof(tokens[i].c_str()) > maxVal) // search max. intensity (x-axis)
      maxVal = atof(tokens[i].c_str());
    if (atof(tokens[i].c_str()) < 0.0)
    {
      errorMessage = "No negative ITF output values allowed!";
      return NULL;
    }
  }
  if (maxVal <= 1.0)
    maxVal = 1.0; // if max <= 1.0 --> no rescaling!

  ITFPointer itf = ITFPointer::New();
  double x, y;
  for (std::size_t i = 2; i < tokens.size(); i += 2)
  {
    x = (atof(tokens[i - 1].c_str()) - intercept) / slope;
    y = atof(tokens[i].c_str()) / maxVal; // normalize to [0;1]
    itf->AddRGBPoint(x, y, y, y);
  }

  return itf;
}

bool REG23Model::ParseMetricEntry(std::string typeEntry, std::string configEntry,
  std::string prefixEntry, std::string postfixEntry,
  std::vector<std::string> &args, std::string &errorMessage)
{
  typeEntry = ToUpperCaseF(TrimF(typeEntry));
  configEntry = TrimF(configEntry);
  std::vector<std::string> cfgs;
  Tokenize(configEntry, cfgs, " ");
  bool succ = true;
  if (typeEntry == "SRC")
  {
    if (cfgs.size() == 7)
    {
      args.push_back("SRC");
      for (std::size_t i = 0; i < cfgs.size(); i++)
      {
        bool hasVariable = ParseEntryHasVariable(cfgs[i]);
        if (!IsNumeric(cfgs[i]) && !hasVariable)
        {
          succ = false;
          errorMessage = "SRC configuration parameter " + StreamConvert(i + 1);
          errorMessage += " is not numeric.";
        }
        args.push_back(TrimF(cfgs[i]));
      }
      bool hasVariable = ParseEntryHasVariable(cfgs[6]);
      if (!hasVariable)
      {
        double coverage = atof(cfgs[6].c_str());
        if (coverage <= 0. || coverage > 100.)
        {
          succ = false;
          errorMessage = "SRC coverage configuration parameter is not in ]0;100]";
        }
      }
      if (!succ)
      {
        args.clear();
        return false;
      }
    }
    else
    {
      errorMessage = "Wrong number of SRC configuration parameters.";
      return false;
    }
  }
  else if (typeEntry == "NMI")
  {
    if (cfgs.size() == 7)
    {
      args.push_back("NMI");
      for (std::size_t i = 0; i < cfgs.size(); i++)
      {
        bool hasVariable = ParseEntryHasVariable(cfgs[i]);
        if (!IsNumeric(cfgs[i]) && !hasVariable)
        {
          succ = false;
          errorMessage = "NMI configuration parameter " + StreamConvert(i + 1);
          errorMessage += " is not numeric.";
        }
        args.push_back(TrimF(cfgs[i]));
      }
      if (!succ)
      {
        args.clear();
        return false;
      }
    }
    else
    {
      errorMessage = "Wrong number of NMI configuration parameters.";
      return false;
    }
  }
  else if (typeEntry == "NCC")
  {
    if (cfgs.size() == 1)
    {
      args.push_back("NCC");
      if (!IsStrictlyNumeric(cfgs[0]))
      {
        errorMessage = "NCC flag (0 or 1) expected.";
        succ = false;
      }
      if (!succ)
      {
        args.clear();
        return false;
      }
      args.push_back(TrimF(cfgs[0]));
    }
    else
    {
      errorMessage = "Wrong number of NCC configuration parameters.";
      return false;
    }
  }
  else if (typeEntry == "MRSD")
  {
    if (cfgs.size() == 1)
    {
      args.push_back("MRSD");
      bool hasVariable = ParseEntryHasVariable(cfgs[0]);
      if (!IsNumeric(cfgs[0]) && !hasVariable)
      {
        errorMessage = "MRSD numeric entry (lambda) expected.";
        succ = false;
      }
      if (!succ)
      {
        args.clear();
        return false;
      }
      args.push_back(TrimF(cfgs[0]));
    }
    else
    {
      errorMessage = "Wrong number of MRSD configuration parameters.";
      return false;
    }
  }
  else if (typeEntry == "MS")
  {
    if (cfgs.size() > 0)
    {
      errorMessage = "Wrong number of MS configuration parameters.";
      return false;
    }
    args.push_back("MS");
  }
  else if (typeEntry == "GD")
  {
    if (cfgs.size() > 0)
    {
      errorMessage = "Wrong number of GD configuration parameters.";
      return false;
    }
    args.push_back("GD");
  }
  else
  {
    errorMessage = "Unknown metric type.";
    return false;
  }

  // additional pre- and postfix:
  if (TrimF(prefixEntry).length() > 0)
  {
    args.push_back(TrimF(prefixEntry));
    if (TrimF(postfixEntry).length() > 0)
      args.push_back(TrimF(postfixEntry));
  }
  else
  {
    if (TrimF(postfixEntry).length() > 0)
    {
      errorMessage = "Specification of a postfix requires a prefix too, but a prefix without postfix is possible.";
      return false;
    }
  }

  return true;
}

bool REG23Model::ParseOptimizerEntry(std::string typeEntry, std::string configEntry,
    std::string scalesEntry, std::vector<std::string> &args,
    std::string &errorMessage)
{
  m_CurrentOptimizerTypeID = "";
  typeEntry = ToUpperCaseF(TrimF(typeEntry));
  configEntry = TrimF(configEntry);
  std::vector<std::string> cfgs;
  Tokenize(configEntry, cfgs, " ");
  std::vector<std::string> scales;
  Tokenize(scalesEntry, scales, " ");
  bool succ = true;
  if (typeEntry == "1+1EVOL")
  {
    if (cfgs.size() == 6)
    {
      args.push_back("1+1EVOL");
      for (std::size_t i = 0; i < cfgs.size(); i++)
      {
        if (!IsNumeric(cfgs[i]))
        {
          succ = false;
          errorMessage = "1+1EVOL configuration parameter " + StreamConvert(i + 1);
          errorMessage += " is not numeric.";
        }
        args.push_back(TrimF(cfgs[i]));
      }
      int maxiter = atoi(cfgs[0].c_str());
      if (maxiter <= 0)
      {
        succ = false;
        errorMessage = "1+1EVOL maximum iterations configuration parameter is not >0.";
      }
      double radius = atof(cfgs[1].c_str());
      if (radius <= 0.)
      {
        succ = false;
        errorMessage = "1+1EVOL initial radius configuration parameter is not >0.0.";
      }
      double epsilon = atof(cfgs[4].c_str());
      if (epsilon <= 0.)
      {
        succ = false;
        errorMessage = "1+1EVOL epsilon (Frobenius norm) configuration parameter is not >0.0.";
      }
      if (!succ)
      {
        args.clear();
        return false;
      }
    }
    else
    {
      errorMessage = "Wrong number of 1+1EVOL configuration parameters.";
      return false;
    }
  }
  else if (typeEntry == "AMOEBA")
  {
    if (cfgs.size() == 9)
    {
      args.push_back("AMOEBA");
      for (std::size_t i = 0; i < cfgs.size(); i++)
      {
        if (!IsNumeric(cfgs[i]))
        {
          succ = false;
          errorMessage = "AMOEBA configuration parameter " + StreamConvert(i + 1);
          errorMessage += " is not numeric.";
        }
        args.push_back(TrimF(cfgs[i]));
      }
      int maxiter = atoi(cfgs[0].c_str());
      if (maxiter <= 0)
      {
        succ = false;
        errorMessage = "AMOEBA maximum iterations configuration parameter is not >0.";
      }
      double val;
      for (std::size_t i = 0; i < 3; i++)
      {
        val = atof(cfgs[i].c_str());
        if (val <= 0.)
        {
          succ = false;
          errorMessage = "AMOEBA configuration parameter #" + StreamConvert(i + 1);
          errorMessage += " is not >0.0.";
        }
      }
      if (!succ)
      {
        args.clear();
        return false;
      }
    }
    else
    {
      errorMessage = "Wrong number of AMOEBA configuration parameters.";
      return false;
    }
  }
  else if (typeEntry == "POWELL")
  {
    if (cfgs.size() == 5)
    {
      args.push_back("POWELL");
      for (std::size_t i = 0; i < cfgs.size(); i++)
      {
        if (!IsNumeric(cfgs[i]))
        {
          succ = false;
          errorMessage = "POWELL configuration parameter " + StreamConvert(i + 1);
          errorMessage += " is not numeric.";
        }
        args.push_back(TrimF(cfgs[i]));
      }
      int maxiter = atoi(cfgs[0].c_str());
      if (maxiter <= 0)
      {
        succ = false;
        errorMessage = "POWELL maximum iterations configuration parameter is not >0.";
      }
      maxiter = atoi(cfgs[1].c_str());
      if (maxiter <= 0)
      {
        succ = false;
        errorMessage = "POWELL maximum line iterations configuration parameter is not >0.";
      }

      double val;
      for (std::size_t i = 2; i < cfgs.size(); i++)
      {
        val = atof(cfgs[i].c_str());
        if (val <= 0.)
        {
          succ = false;
          errorMessage = "POWELL configuration parameter #" + StreamConvert(i + 1);
          errorMessage += " is not >0.0.";
        }
      }
      if (!succ)
      {
        args.clear();
        return false;
      }
    }
    else
    {
      errorMessage = "Wrong number of POWELL configuration parameters.";
      return false;
    }
  }
  else
  {
    errorMessage = "Unknown optimizer type.";
    return false;
  }

  // optional scales:
  if (scales.size() == 0) // -> default
  {
    for (int d = 0; d < 3; d++)
      scales.push_back("57.3");
    for (int d = 0; d < 3; d++)
      scales.push_back("1.0");
  }
  if (scales.size() != 6)
  {
    errorMessage = "The number of scales is wrong (6 scales expected).";
    return false;
  }
  for (std::size_t i = 0; i < scales.size(); i++)
    args.push_back(TrimF(scales[i]));

  m_CurrentOptimizerTypeID = args[0]; // store

  return true;
}

bool REG23Model::IsReadyForAutoRegistration()
{
  // NOTE: m_NReg is only set if all the other sub-components (images, metrics,
  // optimizer ...) are present and correctly tied together!
  if (m_NReg && m_OptimizerCommand && m_RegistrationCommand && m_Config)
    return true;
  else
    return false;
}

bool REG23Model::IsReadyForManualRegistration()
{
  if (m_NReg && m_RegistrationCommand && !m_RegistrationIsRunning)
    return true;
  else
    return false;
}

void REG23Model::IncrementIteration()
{
  m_CurrentIteration++;
}

void REG23Model::InitFunctionTable()
{
  m_FunctionValuesMutex.lock();
  m_FunctionValues.clear();
  // -> add vectors for cost function and each transformation parameter:
  for (unsigned int i = 0; i < (m_CurrentParameters.GetSize() + 1); i++)
  {
    QVector<QPointF> v;
    m_FunctionValues.push_back(v);
  }
  m_FunctionValuesMutex.unlock();
}

const QVector<QVector<QPointF> > &REG23Model::GetFunctionValues()
{
  QMutexLocker lock(&m_FunctionValuesMutex);
  return m_FunctionValues;
}

ITKVTKImage *REG23Model::GetCurrentMovingImage(std::size_t index)
{
  QMutexLocker lock(&m_CurrentMovingImageMutex);
  if (index >= m_CurrentMovingImages.size())
    return NULL;
  return m_CurrentMovingImages[index];
}

ITKVTKImage *REG23Model::GetFixedImage(const std::size_t index, const bool getPostProcessed)
{
  QMutexLocker lock(&m_FixedImageMutex);
  if (index >= m_PPFixedImages.size())
    return NULL;
  if (getPostProcessed && m_PPFixedImagesPost[index] != NULL)
    return m_PPFixedImagesPost[index]->ProduceImage(); // the post-processed one!
  return m_PPFixedImages[index]->ProduceImage(); // the pre-processed one!
}

ITKVTKImage *REG23Model::GetMaskImage(std::size_t index)
{
  QMutexLocker lock(&m_MaskImageMutex);
  if (index >= m_MaskImages.size())
    return NULL;
  return m_MaskImages[index]->ProduceImage();
}

ITKVTKImage *REG23Model::GetMaskImageContour(std::size_t index)
{
  QMutexLocker lock(&m_MaskImageMutex);
  if (index >= m_MaskImages.size())
    return NULL;

  // Copy image (contouring is performed in-place)
  ITKVTKImage *src = CopyMaskImage(m_MaskImages[index]->ProduceImage());

  if (!src || !src->GetAsITKImage<MaskPixelType>())
    return NULL;

  typedef itk::Image<MaskPixelType, ITKVTKImage::Dimensions> ITKMaskImageType;

  ITKMaskImageType::Pointer mask = static_cast<ITKMaskImageType * >(
      src->GetAsITKImage<MaskPixelType>().GetPointer());

#ifdef ITK_USE_REVIEW
  typedef itk::BinaryContourImageFilter<ITKMaskImageType, ITKMaskImageType> ContourType;
#else
  typedef itk::SimpleContourExtractorImageFilter<ITKMaskImageType, ITKMaskImageType> ContourType;
#endif

  ContourType::Pointer contourer = ContourType::New();
  contourer->SetInput(mask);

#ifdef ITK_USE_REVIEW
  contourer->SetForegroundValue(255);
  contourer->SetBackgroundValue(0);
  contourer->FullyConnectedOn();
#else
  contourer->SetInputForegroundValue(255);
  contourer->SetInputBackgroundValue(0);
  ContourType::InputSizeType radius;
  radius.Fill(1);
  contourer->SetRadius(radius);
#endif

  try
  {
    contourer->Update();
    ITKVTKImage *contour = new ITKVTKImage(itk::ImageIOBase::SCALAR,
        itk::ImageIOBase::UCHAR);
    contour->SetITKImage(contourer->GetOutput(), false);
    ITKVTKImageMetaInformation::Pointer mi = ITKVTKImageMetaInformation::New();
    contour->SetMetaInfo(mi); // dummy

    delete src;
    return contour;
  }
  catch (itk::ExceptionObject &e)
  {
    std::cout<<"Exception at contouring! " << e << std::endl;
    delete src;
    return NULL;
  }
}

ITKVTKImage *REG23Model::GetVolumeImage()
{
  QMutexLocker lock(&m_VolumeImageMutex);
  if (m_PPVolume)
    return m_PPVolume->ProduceImage(); // the pre-processed one!
  else
    return NULL;
}

void REG23Model::SendInputImagesCompleteNotification()
{
  this->Notify(UPDATE_INPUT_IMAGES); // notify registered views!
}

void REG23Model::SendInitialParametersNotification()
{
  ParametersType storeCurrent = m_CurrentParameters; // store
  m_CurrentParameters = m_InitialParameters; // make initial pars available
  this->Notify(UPDATE_PARAMETERS); // notify registered views!
  m_CurrentParameters = storeCurrent; // restore
}

void REG23Model::SendMaskImagesCompleteNotification()
{
  bool atLeastOneMask = false;
  for (std::size_t i = 0; i < m_MaskImages.size(); i++)
  {
    if (m_MaskImages[i]->ProduceImage())
    {
      atLeastOneMask = true;
      break;
    }
  }
  if (atLeastOneMask)
    this->Notify(UPDATE_MASK_IMAGES); // notify registered views!
}

bool REG23Model::SaveTransformToFile()
{
  if (m_OutputTransformFile.length() > 0)
  {
    if (m_Transform && m_CurrentParameters.Size() == 6)
    {
      TransformType::Pointer temp = TransformType::New();
      temp->SetCenter(m_Transform->GetCenter());
      temp->SetParameters(m_CurrentParameters);
      TransformType::MatrixType m = temp->GetMatrix();
      TransformType::OutputVectorType o = temp->GetOffset();

      std::string srot = "";
      std::string stransl = "";
      int c = 0;
      for (int i = 0; i < 3; i++)
      {
        // matrix
        for (int j = 0; j < 3; j++)
        {
          if (c > 0)
            srot += ",";
          srot += StreamConvert(m[i][j]);
          c++;
        }
        // offset
        if (i > 0)
          stransl += ",";
        stransl += StreamConvert(o[i]);
      }
      std::string stpars = "";
      std::string stfpars = "";
      for (unsigned int i = 0; i < m_CurrentParameters.GetSize(); i++)
      {
        if (i > 0)
          stpars += ",";
        stpars += StreamConvert(m_CurrentParameters[i]);
      }
      for (unsigned int i = 0; i < m_Transform->GetFixedParameters().GetSize(); i++)
      {
        if (i > 0)
          stfpars += ",";
        stfpars += StreamConvert(m_Transform->GetFixedParameters()[i] + m_IsoCenter[i] * 10.);
      }
      QString pf = "";
      for (unsigned int i = 0; i < 3; i++) // rotations
      {
        if (i > 0)
          pf += ",";
        pf += QString::number(m_CurrentParameters[i] / vtkMath::Pi() * 180.,
            'f', 3);
      }
      for (unsigned int i = 3; i < 6; i++) // translations
      {
        pf += ",";
        pf += QString::number(m_CurrentParameters[i], 'f', 3);
      }

      IniAccess outfile(m_OutputTransformFile);
      outfile.SetAddORACheckSum(true);
      outfile.WriteString("Result-Transform", "RotationMatrix3x3", srot);
      outfile.WriteString("Result-Transform", "TranslationVector", stransl);
      outfile.WriteString("Result-Transform", "TransformParamters", stpars);
      outfile.WriteString("Result-Transform", "TransformFixedParameters", stfpars);
      outfile.WriteString("Result-Transform",
          "Nominal_Parameters_rot_in_deg_transl_in_mm", pf.toStdString());

      temp = NULL;
      temp = TransformType::New();
      temp->SetCenter(m_Transform->GetCenter());
      temp->SetParameters(m_InitialParameters);
      m = temp->GetMatrix();
      o = temp->GetOffset();
      srot = "";
      stransl = "";
      c = 0;
      for (int i = 0; i < 3; i++)
      {
        // matrix
        for (int j = 0; j < 3; j++)
        {
          if (c > 0)
            srot += ",";
          srot += StreamConvert(m[i][j]);
          c++;
        }
        // offset
        if (i > 0)
          stransl += ",";
        stransl += StreamConvert(o[i]);
      }
      stpars = "";
      stfpars = "";
      for (unsigned int i = 0; i < m_InitialParameters.GetSize(); i++)
      {
        if (i > 0)
          stpars += ",";
        stpars += StreamConvert(m_InitialParameters[i]);
      }
      for (unsigned int i = 0; i < m_Transform->GetFixedParameters().GetSize(); i++)
      {
        if (i > 0)
          stfpars += ",";
        stfpars += StreamConvert(m_Transform->GetFixedParameters()[i] + m_IsoCenter[i] * 10.);
      }
      pf = "";
      for (unsigned int i = 0; i < 3; i++) // rotations
      {
        if (i > 0)
          pf += ",";
        pf += QString::number(m_InitialParameters[i] / vtkMath::Pi() * 180.,
            'f', 3);
      }
      for (unsigned int i = 3; i < 6; i++) // translations
      {
        pf += ",";
        pf += QString::number(m_InitialParameters[i], 'f', 3);
      }
      outfile.WriteString("Initial-Transform", "RotationMatrix3x3", srot);
      outfile.WriteString("Initial-Transform", "TranslationVector", stransl);
      outfile.WriteString("Initial-Transform", "TransformParamters", stpars);
      outfile.WriteString("Initial-Transform", "TransformFixedParameters", stfpars);
      outfile.WriteString("Initial-Transform",
          "Nominal_Parameters_rot_in_deg_transl_in_mm", pf.toStdString());
      temp = NULL;

      if (m_ScientificMode) // science mode
      {
        // -> output the target registration error related to iso-center and
        // reference transform as well as related to iso-center and initial
        // transform:
        TransformType::OutputPointType p0;
        p0.Fill(0);
        temp = NULL;
        temp = TransformType::New();
        temp->SetCenter(m_Transform->GetCenter());
        temp->SetParameters(m_InitialParameters);
        TransformType::OutputPointType pinit = temp->TransformPoint(p0);
        temp = NULL;
        temp = TransformType::New();
        temp->SetCenter(m_Transform->GetCenter());
        temp->SetParameters(m_CurrentParameters);
        TransformType::OutputPointType pfinal = temp->TransformPoint(p0);
        TransformType::OutputVectorType initErr = pfinal - pinit;
        std::string svec = StreamConvert(initErr[0]) + ",";
        svec += StreamConvert(initErr[1]) + ",";
        svec += StreamConvert(initErr[2]);
        outfile.WriteString("Registration-Error", "FinalToInitialVector", svec);
        svec = StreamConvert(initErr.GetNorm());
        outfile.WriteString("Registration-Error", "FinalToInitialVectorNorm", svec);
        if (m_ReferenceParameters.Size() > 0)
        {
          temp = NULL;
          temp = TransformType::New();
          temp->SetCenter(m_Transform->GetCenter());
          temp->SetParameters(m_ReferenceParameters);
          m = temp->GetMatrix();
          o = temp->GetOffset();
          srot = "";
          stransl = "";
          c = 0;
          for (int i = 0; i < 3; i++)
          {
            // matrix
            for (int j = 0; j < 3; j++)
            {
              if (c > 0)
                srot += ",";
              srot += StreamConvert(m[i][j]);
              c++;
            }
            // offset
            if (i > 0)
              stransl += ",";
            stransl += StreamConvert(o[i]);
          }
          stpars = "";
          stfpars = "";
          for (unsigned int i = 0; i < m_ReferenceParameters.GetSize(); i++)
          {
            if (i > 0)
              stpars += ",";
            stpars += StreamConvert(m_ReferenceParameters[i]);
          }
          for (unsigned int i = 0; i < m_Transform->GetFixedParameters().GetSize(); i++)
          {
            if (i > 0)
              stfpars += ",";
            stfpars += StreamConvert(m_Transform->GetFixedParameters()[i] + m_IsoCenter[i] * 10.);
          }
          pf = "";
          for (unsigned int i = 0; i < 3; i++) // rotations
          {
            if (i > 0)
              pf += ",";
            pf += QString::number(m_ReferenceParameters[i] / vtkMath::Pi() * 180.,
                'f', 3);
          }
          for (unsigned int i = 3; i < 6; i++) // translations
          {
            pf += ",";
            pf += QString::number(m_ReferenceParameters[i], 'f', 3);
          }
          outfile.WriteString("Reference-Transform", "RotationMatrix3x3", srot);
          outfile.WriteString("Reference-Transform", "TranslationVector", stransl);
          outfile.WriteString("Reference-Transform", "TransformParamters", stpars);
          outfile.WriteString("Reference-Transform", "TransformFixedParameters", stfpars);
          outfile.WriteString("Reference-Transform",
              "Nominal_Parameters_rot_in_deg_transl_in_mm", pf.toStdString());

          TransformType::OutputPointType pref = temp->TransformPoint(p0);
          TransformType::OutputVectorType refErr = pfinal - pref;
          std::string svec = StreamConvert(refErr[0]) + ",";
          svec += StreamConvert(refErr[1]) + ",";
          svec += StreamConvert(refErr[2]);
          outfile.WriteString("Registration-Error", "FinalToReferenceVector", svec);
          svec = StreamConvert(refErr.GetNorm());
          outfile.WriteString("Registration-Error", "FinalToReferenceVectorNorm", svec);
        }
      }

      outfile.WriteString("File", "FileFormat", "1.0");
      outfile.WriteString("Info", "Config", m_ConfigFile);
      std::string ts;
      CurrentORADateTimeString(ts);
      outfile.WriteString("Info", "TimeStamp", ts);

      // user-tracking-info
      std::vector<std::string> trackingInfo;
      trackingInfo.clear();
      if (m_UndoRedoManager)
        trackingInfo = m_UndoRedoManager->GetLogList();
      for (std::size_t k = 0; k < trackingInfo.size(); k++)
        outfile.WriteString("User-Tracking", "log" + StreamConvert(k + 1),
            trackingInfo[k]);
      outfile.WriteString("User-Tracking", "ApplicationStartTimeStamp",
          m_ModelCreationTimeStamp);
      outfile.WriteString("User-Tracking", "ApplicationEndTimeStamp", ts);

      bool ret = outfile.Update();

      return ret;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return true; // not requested
  }
}

bool REG23Model::ExtractMeanAndVarianceFromImage(vtkImageData *image,
    double &mean, double &variance)
{
  if (!image)
    return false;
  float *it = (float*)image->GetScalarPointer();
  unsigned int N = image->GetDimensions()[0] * image->GetDimensions()[1];
  mean = 0;
  for (unsigned int i = 0; i < N; ++i)
   mean += it[i];
  mean = mean / (double)N;
  it = (float*)image->GetScalarPointer();
  variance = 0;
  for (unsigned int i = 0; i < N; ++i)
   variance += ((it[i] - mean) * (it[i] - mean));
  variance = variance / (double)(N - 1);
  return true;
}

double *REG23Model::GetWindowLevelMinMaxForFixedImage(vtkImageData *fixedImage,
    std::size_t index, bool unsharpMaskImage)
{
  if (!fixedImage || index >= m_PPFixedImages.size() || !m_Config)
    return NULL;
  // retrieve config entry (was checked by LoadConfig() - is OK!)
  std::string key;
  if (!unsharpMaskImage)
    key = "InitialFixedImageWindowLevel" + StreamConvert(index + 1);
  else
    key = "UnsharpMaskingWindowLevel" + StreamConvert(index + 1);
  std::string s = ToLowerCaseF(TrimF(m_Config->ReadString("Visualization",
      key, "")));
  std::vector<std::string> v;
  Tokenize(s, v, ",");
  if (v.size() == 2)
  {
    if (!unsharpMaskImage)
    {
      m_CurrentWindowLevelMinMax[0] = m_ExplicitInitialWLs[index][0];
      m_CurrentWindowLevelMinMax[1] = m_ExplicitInitialWLs[index][1];
    }
    else
    {
      double win = atof(v[0].c_str());
      double lev = atof(v[1].c_str());
      m_CurrentWindowLevelMinMax[0] = lev - win / 2.;
      m_CurrentWindowLevelMinMax[1] = lev + win / 2.;
    }
  }
  else if (v.size() == 3 && TrimF(v[0]) == "mean-std")
  {
    double f1 = atof(v[1].c_str()); // neg. factor
    double f2 = atof(v[2].c_str()); // pos. factor
    // -> extract mean and variance of image
    double mean, variance;
    if (!ExtractMeanAndVarianceFromImage(fixedImage, mean, variance))
      return NULL;
    m_CurrentWindowLevelMinMax[0] = mean - f1 * std::sqrt(variance);
    m_CurrentWindowLevelMinMax[1] = mean + f2 * std::sqrt(variance);
  }
  else if (v.size() == 3 && TrimF(v[0]) == "min-max-rel")
  {
    double f1 = atof(v[1].c_str()); // min-factor
    double f2 = atof(v[2].c_str()); // max-factor
    double sr[2];
    fixedImage->GetScalarRange(sr);
    m_CurrentWindowLevelMinMax[0] = sr[0] + f1 * (sr[1] - sr[0]);
    m_CurrentWindowLevelMinMax[1] = sr[1] - f2 * (sr[1] - sr[0]);
  }
  else // default: simply return min/max
  {
    double sr[2];
    fixedImage->GetScalarRange(sr);
    m_CurrentWindowLevelMinMax[0] = sr[0];
    m_CurrentWindowLevelMinMax[1] = sr[1];
  }

  // recovery of window/level from stored file if available and valid:
  if (m_WindowLevelRecoveryStrategy == 1)
  {
    std::string fn = itksys::SystemTools::GetFilenamePath(
        m_FixedImageFileNames[index]);
    EnsureStringEndsWith(fn, DSEP);
    fn += "reg23-wl.inf";
    if (itksys::SystemTools::FileExists(fn.c_str()))
    {
      IniAccess f(fn);
      std::string sw = "", sl = "";
      if (!unsharpMaskImage)
      {
        sw = f.ReadString("Window/Level", "FixedImageWindow", "");
        sl = f.ReadString("Window/Level", "FixedImageLevel", "");
      }
      else
      {
        sw = f.ReadString("Window/Level", "UnsharpMaskImageWindow", "");
        sl = f.ReadString("Window/Level", "UnsharpMaskImageLevel", "");
      }
      if (sw.length() > 0 && sl.length() > 0 && IsNumeric(sw) && IsNumeric(sl))
      {
        // window/level valid -> take these values!
        double win = atof(sw.c_str());
        double lev = atof(sl.c_str());
        m_CurrentWindowLevelMinMax[0] = lev - win / 2.;
        m_CurrentWindowLevelMinMax[1] = lev + win / 2.;
      }
    }
  }

  return &m_CurrentWindowLevelMinMax[0];
}

bool REG23Model::ParseGenericVariables(std::string expr,
    std::vector<std::string> &structuids, std::vector<std::string> &attribs,
    std::string currentStructUID)
{
  while (ReplaceString(expr, " ", "")); // remove spaces
  if (expr.length() <= 0)
    return false; // invalid
  std::string exprlow = ToLowerCaseF(expr);
  std::size_t p = 0, f, f2;
  while (p < expr.length())
  {
    f = exprlow.find("v{", p);
    if (f != std::string::npos)
    {
      f2 = exprlow.find("}", p + 1); // closing bracket!
      if (f2 != std::string::npos)
      {
        std::string s = expr.substr(f + 2, f2 - f - 2);
        std::vector<std::string> v;
        v.clear();
        Tokenize(s, v, ".");
        if (v.size() == 2)
        {
          if ((ToUpperCaseF(v[0]) != "VOLUME" &&
                !m_Structures->FindStructure(v[0]))) // invalid structure UID
          {
            if (currentStructUID != v[0]) // last opportunity!
              return false;
          }
          std::string attr = ToLowerCaseF(v[1]);
          if (attr != "xmin" && attr != "xmax" && attr != "ymin"
              && attr != "ymax" && attr != "zmin" && attr != "zmax"
              && attr != "w" && attr != "h" && attr != "d" && attr != "cells"
              && attr != "points") // invalid attribute
            return false;
          structuids.push_back(v[0]);
          attribs.push_back(attr);
          p = f2;
        }
        else // not "."-separated!
        {
          return false;
        }
      }
      else // no closing bracket!
      {
        return false;
      }
    }
    else
    {
      p = expr.length(); // terminate
    }
  }
  return true;
}

double REG23Model::GetGenericVariableValue(std::string structuid,
    std::string attrib, std::string currentStructUID, vtkPolyData *currentPD)
{
  vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
  VTKStructure *st = m_Structures->FindStructure(structuid);
  if (st)
  {
    pd = st->GetStructurePolyData();
  }
  else if (ToUpperCaseF(structuid) == "VOLUME")
  {
    ITKVTKImage *volume = NULL;
    if (m_Volume)
      volume = m_Volume->ProduceImage();
    if (!volume)
      return 0;

    // Cast to ITK image types
    // NOTE: USE DUMMY TYPE SINCE ONLY VOLUME PROPERTIES ARE NEEDED
    typedef int VolumePixelType;
    typedef itk::Image<VolumePixelType, 3> VolumeImageType;
    ITKVTKImage::ITKImagePointer ibase = this->GetVolumeImage()->GetAsITKImage<VolumePixelType>();
    VolumeImageType::Pointer volumeImageITK = static_cast<VolumeImageType *> (ibase.GetPointer());
    vtkSmartPointer<vtkImageData> volumeImageVTK = volume->GetAsVTKImage<VolumePixelType>();

    // Volume properties
    VolumeImageType::PointType originVol = volumeImageITK->GetOrigin();
    VolumeImageType::DirectionType directionVol = volumeImageITK->GetDirection();

    // Create a dummy structure that represents the volume
    vtkSmartPointer<vtkCubeSource> cs = vtkSmartPointer<vtkCubeSource>::New();
    cs->SetBounds(volumeImageVTK->GetBounds());
    cs->Update();

    // Convert directional cosine matrix of ITK to VTK transform matrix to transform
    // the volume cube to structure space
    vtkSmartPointer<vtkMatrix4x4> dc = vtkMatrix4x4::New();
    dc->Identity();
    // rows are columns in ITK image! -> Transpose
    dc->SetElement(0, 0, directionVol[0][0]);
    dc->SetElement(0, 1, directionVol[1][0]);
    dc->SetElement(0, 2, directionVol[2][0]);
    dc->SetElement(1, 0, directionVol[0][1]);
    dc->SetElement(1, 1, directionVol[1][1]);
    dc->SetElement(1, 2, directionVol[2][1]);
    dc->SetElement(2, 0, directionVol[0][2]);
    dc->SetElement(2, 1, directionVol[1][2]);
    dc->SetElement(2, 2, directionVol[2][2]);
    dc->Invert();

    // Transform volume polydata to structure space (VTK has no direction)
    vtkTransform* volumeTransform = vtkTransform::New();
    volumeTransform->Identity();
    volumeTransform->PostMultiply();
    volumeTransform->Translate(-originVol[0], -originVol[1], -originVol[2]);
    volumeTransform->Concatenate(dc);
    volumeTransform->Translate(originVol[0], originVol[1], originVol[2]);
    volumeTransform->Update();

    vtkTransformPolyDataFilter* transformToVolumeSpace =
    vtkTransformPolyDataFilter::New();
    transformToVolumeSpace->SetTransform(volumeTransform);
    transformToVolumeSpace->SetInput(cs->GetOutput());
    transformToVolumeSpace->Update();

    pd->ShallowCopy(transformToVolumeSpace->GetOutput());
  }
  else
  {
    pd = currentPD;
  }
  if (!pd)
    return 0;
  if (attrib == "cells")
  {
    return (double)pd->GetPolys()->GetNumberOfCells();
  }
  else if (attrib == "points")
  {
    return (double)pd->GetPoints()->GetNumberOfPoints();
  }
  else
  {
    double b[6];
    pd->GetBounds(b);
    if (attrib == "xmin")
      return b[0];
    else if (attrib == "xmax")
      return b[1];
    else if (attrib == "ymin")
      return b[2];
    else if (attrib == "ymax")
      return b[3];
    else if (attrib == "zmin")
      return b[4];
    else if (attrib == "zmax")
      return b[5];
    else if (attrib == "w")
      return (b[1] - b[0]);
    else if (attrib == "h")
      return (b[3] - b[2]);
    else if (attrib == "d")
      return (b[5] - b[4]);
    else
      return 0;
  }
}

bool REG23Model::ParseAndProcessStructureEntry(int index, bool simulation,
    std::string &errorKey, std::string &errorMessage)
{
  errorKey = "";
  errorMessage = "";
  if (!m_Config || index < 1)
    return false;
  errorKey = "Structure" + StreamConvert(index);
  std::string s = m_Config->ReadString("Structures", errorKey, "");
  // remove all spaces, case-insensitive:
  while (ReplaceString(s, " ", ""));
  if (s.length() <= 0)
    return false; // do not process empty entries in this method!

  std::vector<std::string> v;
  Tokenize(s, v, ",");
  std::string stuid = "", stsuid = ""; // ORA-entries to be checked!
  std::string cmdType = "";
  std::vector<std::string> args; // command-specific arguments
  std::vector<bool> relargs; // marking whether or not an arg is relative
  std::string newStuid = "";
  if (v.size() == 2 || v.size() == 3) // <structure-set-uid>,<structure-uid>[,<new-structure-uid>]
  {
    cmdType = "";
    if (v.size() == 3) // maybe "File"-type?
    {
      std::string tmp = ToUpperCaseF(TrimF(v[0]));
      if (tmp.length() > 5 && tmp.substr(0, 5) == "FILE(")
      {
        cmdType = "file";
        tmp = TrimF(v[0]);
        tmp = TrimF(tmp.substr(5, tmp.length()));
        if (!itksys::SystemTools::FileExists(tmp.c_str()))
        {
          errorMessage = "The file (" + tmp;
          errorMessage += ") specified in File()-command does not name an existing file!";
          return false;
        }
        args.push_back(tmp);
        tmp = TrimF(v[1].substr(0, v[1].length() - 1));
        tmp = ToUpperCaseF(TrimF(tmp));
        if (tmp != "DSW" && tmp != "XML" && tmp != "PLY")
        {
          errorMessage = "The format (" + tmp;
          errorMessage += ") specified in File()-command does not name a valid polygonal file format!";
          return false;
        }
        args.push_back(tmp);
        if (TrimF(v[1]).substr(v[1].length() - 1, 1) != ")")
        {
          errorMessage = "Closing '(' missing in File()-command!";
          return false;
        }
        tmp = TrimF(v[2]);
        if (tmp.length() == 0)
        {
          errorMessage = "There is no structure UID specified in File()-command!";
          return false;
        }
        args.push_back(tmp);
      }
    }
    if (cmdType.length() == 0)
    {
      cmdType = "ora";
      stsuid = v[0];
      stuid = v[1];
      if (v.size() == 3)
        newStuid = v[2];
    }
  }
  else if (v.size() == 5) // Sphere(<x>,<y>,<z>,<r>),<structure-uid>
  {
    cmdType = "sphere";
    if (ToUpperCaseF(v[0].substr(0, 7)) == "SPHERE(")
    {
      if (v[3].length() > 0 && v[3].substr(v[3].length() - 1, 1) == ")")
      {
        args.push_back(v[0].substr(7, v[0].length()));
        args.push_back(v[1]);
        args.push_back(v[2]);
        args.push_back(v[3].substr(0, v[3].length() - 1));
        for (std::size_t i = 0; i < args.size(); i++)
        {
          if (TrimF(args[i]).length() <= 0)
          {
            errorMessage = "Argument #" + StreamConvert(i + 1);
            errorMessage += " of the Sphere()-command is empty!";
            return false;
          }
        }
        if (v[4].length() <= 0)
        {
          errorMessage = "Structure UID of sphere is missing!";
          return false;
        }
        args.push_back(v[4]);
      }
      else
      {
        errorMessage = "Closing ')' in Sphere()-command missing.";
        return false;
      }
    }
    else
    {
      errorMessage = "Did you mean the Sphere()-command? Its correct format is: Sphere(<x>,<y>,<z>,<r>),<structure-uid>.";
      return false;
    }
  }
  else if (v.size() == 7) // Box(<x>,<y>,<z>,<w>,<h>,<d>),<structure-uid>
  {
    cmdType = "box";
    if (ToUpperCaseF(v[0].substr(0, 4)) == "BOX(")
    {
      if (v[5].length() > 0 && v[5].substr(v[5].length() - 1, 1) == ")")
      {
        args.push_back(v[0].substr(4, v[0].length()));
        args.push_back(v[1]);
        args.push_back(v[2]);
        args.push_back(v[3]);
        args.push_back(v[4]);
        args.push_back(v[5].substr(0, v[5].length() - 1));
        for (std::size_t i = 0; i < args.size(); i++)
        {
          if (TrimF(args[i]).length() <= 0)
          {
            errorMessage = "Argument #" + StreamConvert(i + 1);
            errorMessage += " of the Box()-command is empty!";
            return false;
          }
        }
        if (v[6].length() <= 0)
        {
          errorMessage = "Structure UID of box is missing!";
          return false;
        }
        args.push_back(v[6]);
      }
      else
      {
        errorMessage = "Closing ')' in Box()-command missing.";
        return false;
      }
    }
    else
    {
      errorMessage = "Did you mean the Box()-command? Its correct format is: Box(<x>,<y>,<z>,<w>,<h>,<d>),<structure-uid>.";
      return false;
    }
  }
  else
  {
    errorMessage = "Unknown entry. Check the available structure types and the formatting (number of parameters).";
    return false;
  }

  // check needed ORA-structures:
  std::string oraFileName = "";
  if (stsuid.length() > 0 || stuid.length() > 0)
  {
    if (stsuid != "-" && !HaveORAConnectivity())
    {
      errorMessage = "This command requires ORA-connectivity, but it appears being not established!";
      return false;
    }
    if (stsuid != "-")
    {
      std::string fld = m_SiteInfo->ReadString("Paths", "PatientsFolder", "", true);
      EnsureStringEndsWith(fld, DSEP);
      fld += m_PatientUID + DSEP;
      fld += m_SiteInfo->ReadString("Paths", "StructureSetsSubFolder", "", false);
      EnsureStringEndsWith(fld, DSEP);
      fld += stsuid + DSEP; // STS UID
      fld += m_SiteInfo->ReadString("Paths", "StructuresSubFolder", "", false);
      EnsureStringEndsWith(fld, DSEP);
      fld += stuid + DSEP; // ST UID
      fld += m_SiteInfo->ReadString("Paths", "StructureInfoFile", "", false);
      if (!itksys::SystemTools::FileExists(fld.c_str()))
      {
        errorMessage = "Specified ORA structure set UID and/or structure UID appear to be invalid.";
        return false;
      }
      oraFileName = fld;
    }
    else
    {
      if (!m_Structures->FindStructure(stuid))
      {
        errorMessage = "Invalid structure '" + stuid + "' specified!";
        return false;
      }
    }
  }

  // really process the entry (load or create the structure):
  vtkSmartPointer<vtkPolyData> pd = NULL;
  vtkSmartPointer<vtkProperty> prop = vtkSmartPointer<vtkProperty>::New();
  std::string structureUID = "";
  if (cmdType == "ora")
  {
    structureUID = stuid;
    if (newStuid.length() > 0)
      structureUID = newStuid;
  }
  else
  {
    structureUID = args[args.size() - 1];
  }
  if (!simulation)
  {
    // remove old (dummy) occurrences of this structure:
    VTKStructure *structur = m_Structures->FindStructure(structureUID);
    if (structur)
    {
      m_Structures->RemoveStructure(structur);
      delete structur;
    }
    if (cmdType == "file")
    {
      if (args[1] == "DSW")
      {
        vtkSmartPointer<vtkDataSetReader> r =
            vtkSmartPointer<vtkDataSetReader>::New();
        r->SetFileName(args[0].c_str());
        r->Update();
        pd = r->GetPolyDataOutput();
      }
      else if (args[1] == "PLY")
      {
        vtkSmartPointer<vtkPLYReader> r =
            vtkSmartPointer<vtkPLYReader>::New();
        r->SetFileName(args[0].c_str());
        r->Update();
        pd = r->GetOutput();
      }
      else if (args[1] == "XML")
      {
        vtkSmartPointer<vtkXMLPolyDataReader> r =
            vtkSmartPointer<vtkXMLPolyDataReader>::New();
        r->SetFileName(args[0].c_str());
        r->Update();
        pd = r->GetOutput();
      }
    }
    else if (cmdType == "sphere")
    {
      vtkSmartPointer<vtkSphereSource> ss =
          vtkSmartPointer<vtkSphereSource>::New();
      vtkSmartPointer<vtkFunctionParser> fp =
          vtkSmartPointer<vtkFunctionParser>::New();
      std::vector<std::string> fpstuids;
      std::vector<std::string> fpattribs;
      bool fpsucc = true;
      fpsucc = fpsucc && ParseGenericVariables(args[0], fpstuids, fpattribs, structureUID);
      fpsucc = fpsucc && ParseGenericVariables(args[1], fpstuids, fpattribs, structureUID);
      fpsucc = fpsucc && ParseGenericVariables(args[2], fpstuids, fpattribs, structureUID);
      fpsucc = fpsucc && ParseGenericVariables(args[3], fpstuids, fpattribs, structureUID);
      if (!fpsucc)
        return false;
      for (std::size_t u = 0; u < fpstuids.size(); u++)
      {
        std::string varname = "v{" + ToLowerCaseF(fpstuids[u] + "." + fpattribs[u]);
        varname += "}";
        fp->SetScalarVariableValue(varname.c_str(),
            GetGenericVariableValue(fpstuids[u], fpattribs[u], structureUID, pd));
      }
      std::string fpfunc = ToLowerCaseF(args[0]);
      ReplaceString(fpfunc, "#", ",");
      fp->SetFunction(fpfunc.c_str());
      double cx = fp->GetScalarResult();
      fpfunc = ToLowerCaseF(args[1]);
      ReplaceString(fpfunc, "#", ",");
      fp->SetFunction(fpfunc.c_str());
      double cy = fp->GetScalarResult();
      fpfunc = ToLowerCaseF(args[2]);
      ReplaceString(fpfunc, "#", ",");
      fp->SetFunction(fpfunc.c_str());
      double cz = fp->GetScalarResult();
      fpfunc = ToLowerCaseF(args[3]);
      ReplaceString(fpfunc, "#", ",");
      fp->SetFunction(fpfunc.c_str());
      double r = fp->GetScalarResult();
      ss->SetCenter(cx, cy, cz);
      ss->SetRadius(r);
      ss->SetPhiResolution(100);
      ss->SetThetaResolution(100);
      ss->Update();
      pd = ss->GetOutput();
    }
    else if (cmdType == "box")
    {
      vtkSmartPointer<vtkCubeSource> cs =
          vtkSmartPointer<vtkCubeSource>::New();
      vtkSmartPointer<vtkFunctionParser> fp =
          vtkSmartPointer<vtkFunctionParser>::New();
      std::vector<std::string> fpstuids;
      std::vector<std::string> fpattribs;
      bool fpsucc = true;
      fpsucc = fpsucc && ParseGenericVariables(args[0], fpstuids, fpattribs, structureUID);
      fpsucc = fpsucc && ParseGenericVariables(args[1], fpstuids, fpattribs, structureUID);
      fpsucc = fpsucc && ParseGenericVariables(args[2], fpstuids, fpattribs, structureUID);
      fpsucc = fpsucc && ParseGenericVariables(args[3], fpstuids, fpattribs, structureUID);
      fpsucc = fpsucc && ParseGenericVariables(args[4], fpstuids, fpattribs, structureUID);
      fpsucc = fpsucc && ParseGenericVariables(args[5], fpstuids, fpattribs, structureUID);
      if (!fpsucc)
        return false;
      for (std::size_t u = 0; u < fpstuids.size(); u++)
      {
        std::string varname = "v{" + ToLowerCaseF(fpstuids[u] + "." + fpattribs[u]);
        varname += "}";
        fp->SetScalarVariableValue(varname.c_str(),
            GetGenericVariableValue(fpstuids[u], fpattribs[u], structureUID, pd));
      }
      std::string fpfunc = ToLowerCaseF(args[0]);
      ReplaceString(fpfunc, "#", ",");
      fp->SetFunction(fpfunc.c_str());
      double ox = fp->GetScalarResult();
      fpfunc = ToLowerCaseF(args[1]);
      ReplaceString(fpfunc, "#", ",");
      fp->SetFunction(fpfunc.c_str());
      double oy = fp->GetScalarResult();
      fpfunc = ToLowerCaseF(args[2]);
      ReplaceString(fpfunc, "#", ",");
      fp->SetFunction(fpfunc.c_str());
      double oz = fp->GetScalarResult();
      fpfunc = ToLowerCaseF(args[3]);
      ReplaceString(fpfunc, "#", ",");
      fp->SetFunction(fpfunc.c_str());
      double ww = fp->GetScalarResult();
      fpfunc = ToLowerCaseF(args[4]);
      ReplaceString(fpfunc, "#", ",");
      fp->SetFunction(fpfunc.c_str());
      double hh = fp->GetScalarResult();
      fpfunc = ToLowerCaseF(args[5]);
      ReplaceString(fpfunc, "#", ",");
      fp->SetFunction(fpfunc.c_str());
      double dd = fp->GetScalarResult();

      double bounds[6];
      bounds[0] = ox;
      bounds[1] = ox + ww;
      bounds[2] = oy;
      bounds[3] = oy + hh;
      bounds[4] = oz;
      bounds[5] = oz + dd;

      double oxVolume = GetGenericVariableValue("VOLUME", "xmin");
      double oyVolume = GetGenericVariableValue("VOLUME", "ymin");
      double ozVolume = GetGenericVariableValue("VOLUME", "zmin");
      double wwVolume = GetGenericVariableValue("VOLUME", "w");
      double hhVolume = GetGenericVariableValue("VOLUME", "h");
      double ddVolume = GetGenericVariableValue("VOLUME", "d");
      double boundsVolume[6];
      boundsVolume[0] = oxVolume;
      boundsVolume[1] = oxVolume + wwVolume;
      boundsVolume[2] = oyVolume;
      boundsVolume[3] = oyVolume + hhVolume;
      boundsVolume[4] = ozVolume;
      boundsVolume[5] = ozVolume + ddVolume;

      // Limit bounds of cube to volume to avoid segfaults in Volumemask()-command
      bounds[0] = vnl_math_max(bounds[0], boundsVolume[0]);
      bounds[1] = vnl_math_min(bounds[1], boundsVolume[1]);
      // Bounds in y-direction are not limited to enable full volume coverage
      // because roundoff errors (large spacing) require greater box sizes as
      // the bounding box of the volume.
      //bounds[2] = vnl_math_max(bounds[2], boundsVolume[2]);
      //bounds[3] = vnl_math_min(bounds[3], boundsVolume[3]);
      bounds[4] = vnl_math_max(bounds[4], boundsVolume[4]);
      bounds[5] = vnl_math_min(bounds[5], boundsVolume[5]);

      cs->SetBounds(bounds);
      cs->Update();
      // need to clean and generate normals (for eventual warping later):
      vtkSmartPointer<vtkCleanPolyData> cleaner =
          vtkSmartPointer<vtkCleanPolyData>::New();
      cleaner->SetInput(cs->GetOutput());
      cleaner->SetPointMerging(true);
      cleaner->SetConvertLinesToPoints(true);
      cleaner->SetConvertPolysToLines(true);
      cleaner->SetConvertStripsToPolys(true);
      cleaner->SetPieceInvariant(true);
      cleaner->Update();
      vtkSmartPointer<vtkPolyDataNormals> normals =
          vtkSmartPointer<vtkPolyDataNormals>::New();
      normals->SetInput(cleaner->GetOutput());
      normals->SetComputePointNormals(true);
      normals->SetSplitting(false); //!
      normals->Update();
      pd = normals->GetOutput();
    }
    else // ora
    {
      if (oraFileName.length() > 0)
      {
        vtkSmartPointer<vtkORAStructureReader> r =
            vtkSmartPointer<vtkORAStructureReader>::New();
        r->SetFileName(oraFileName.c_str());
        r->SetGenerate3DSurfaceFrom2DContoursIfApplicable(true);
        r->SetGenerateSurfaceNormals(true);
        if (!r->CanReadFile(oraFileName.c_str()))
          return false;
        r->SetReadColorsAsWell(false);
        r->Update();
        if (!r->GetOutput())
          return false;
        double *c = NULL;
        IniAccess *stInfo = r->GetInfoFileAccess();
        std::string s = stInfo->ReadString("WireFrame", "ColorRGB", "");
        if (ParseCommaSeparatedNumericStringVector(s, c) < 3)
          return false;
        prop->SetColor(c[0], c[1], c[2]);
        delete [] c;
        s = stInfo->ReadString("WireFrame", "Width", "");
        if (ParseCommaSeparatedNumericStringVector(s, c) < 1)
          return false;
        prop->SetLineWidth(c[0]);
        delete [] c;
        vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
        t->Identity();
        t->Translate(-10 * m_IsoCenter[0], -10 * m_IsoCenter[1], -10 * m_IsoCenter[2]);
        vtkSmartPointer<vtkTransformPolyDataFilter> tf =
            vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        tf->SetTransform(t);
        tf->SetInput(r->GetOutput());
        tf->Update();
        pd = tf->GetOutput();
      }
      else
      {
        pd = m_Structures->FindStructure(stuid)->GetStructurePolyData();
      }
    }
    if (!pd)
    {
      return false;
    }
  }

  // check the pre-processing entries and apply them to the basic structure:
  std::size_t j = 1;
  std::string t = "";
  std::string baseKey = errorKey;
  do
  {
    errorKey = baseKey + ".PreProcess" + StreamConvert(j);
    t = m_Config->ReadString("Structures", errorKey, "");
    while (ReplaceString(t, " ", ""));
    if (t.length() > 0)
    {
      if (ToUpperCaseF(t.substr(0, 6)) == "STORE(") // Store(<file-name>)
      {
        if (t.substr(t.length() - 1, 1) == ")")
        {
          t = t.substr(6, t.length() - 7);
          std::vector<std::string> v2;
          Tokenize(t, v2, ",");
          if (v2.size() == 2)
          {
            std::string fn = TrimF(v2[0]);
            if (fn.length() <= 0)
            {
              errorMessage = "No file name in Store()-command detected.";
              return false;
            }
            std::string format = ToUpperCaseF(TrimF(v2[1]));
            if (format != "DSW" && format != "PLY" && format != "XML")
            {
              errorMessage = "Wrong format in Store()-command detected.";
              return false;
            }
            if (!simulation) // PROCESS: STORE
            {
              if (format == "DSW")
              {
                vtkSmartPointer<vtkDataSetWriter> w =
                    vtkSmartPointer<vtkDataSetWriter>::New();
                w->SetFileName(fn.c_str());
                w->SetInput(pd);
                if (!w->Write())
                  return false; // error
              }
              else if (format == "PLY")
              {
                vtkSmartPointer<vtkPLYWriter> w =
                    vtkSmartPointer<vtkPLYWriter>::New();
                w->SetFileTypeToASCII();
                w->SetFileName(fn.c_str());
                w->SetInput(pd);
                if (!w->Write())
                  return false; // error
              }
              else if (format == "XML")
              {
                vtkSmartPointer<vtkXMLPolyDataWriter> w =
                    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
                w->SetCompressorTypeToNone();
                w->SetFileName(fn.c_str());
                w->SetInput(pd);
                if (!w->Write())
                  return false; // error
              }
            }

          }
          else
          {
            errorMessage = "Wrong number of arguments for Store()-command detected (need file name and format).";
            return false;
          }
        }
        else
        {
          errorMessage = "Missing closing ')' in Store()-command detected.";
          return false;
        }
      }
      else if (ToUpperCaseF(t.substr(0, 11)) == "NORMALWARP(") // NormalWarp(<amount>)
      {
        if (t.substr(t.length() - 1, 1) == ")")
        {
          std::string as = t.substr(11, t.length() - 12);
          if (as.length() > 0)
          {
            if (!simulation) // PROCESS: NORMALWARP
            {
              vtkSmartPointer<vtkFunctionParser> fp =
                  vtkSmartPointer<vtkFunctionParser>::New();
              std::vector<std::string> fpstuids;
              std::vector<std::string> fpattribs;
              bool fpsucc = true;
              fpsucc = fpsucc && ParseGenericVariables(as, fpstuids, fpattribs, structureUID);
              if (!fpsucc)
                return false;
              for (std::size_t u = 0; u < fpstuids.size(); u++)
              {
                std::string varname = "v{" + ToLowerCaseF(fpstuids[u] + "." + fpattribs[u]);
                varname += "}";
                fp->SetScalarVariableValue(varname.c_str(),
                    GetGenericVariableValue(fpstuids[u], fpattribs[u], structureUID, pd));
              }
              std::string fpfunc = ToLowerCaseF(as);
              ReplaceString(fpfunc, "#", ",");
              fp->SetFunction(fpfunc.c_str());
              double sf = fp->GetScalarResult();
              vtkSmartPointer<vtkWarpVector> warp =
                  vtkSmartPointer<vtkWarpVector>::New();
              warp->SetInputConnection(pd->GetProducerPort());
              warp->SetScaleFactor(sf);
              warp->SetInputArrayToProcess(0, 0, 0,
                  vtkDataObject::FIELD_ASSOCIATION_POINTS,
                  vtkDataSetAttributes::NORMALS);
              warp->Update();
              vtkSmartPointer<vtkPolyDataNormals> normals =
                  vtkSmartPointer<vtkPolyDataNormals>::New();
              normals->SetInput(warp->GetOutput());
              normals->SetComputePointNormals(true);
              normals->SetSplitting(false); //!
              normals->Update();
              vtkSmartPointer<vtkPolyData> pd2 =
                  vtkSmartPointer<vtkPolyData>::New();
              pd2->ShallowCopy(normals->GetOutput());
              pd = pd2;
            }
          }
          else
          {
            errorMessage = "Empty amount in NormalWarp()-command detected.";
            return false;
          }
        }
        else
        {
          errorMessage = "Missing closing ')' in NormalWarp()-command detected.";
          return false;
        }
      }
      else if (ToUpperCaseF(t.substr(0, 9)) == "DECIMATE(") // NormalWarp(<amount>)
      {
        if (t.substr(t.length() - 1, 1) == ")")
        {
          std::vector<std::string> v2;
          Tokenize(t, v2, ",");
          if (v2.size() != 2)
          {
            errorMessage = "Wrong number of arguments for Decimate()-command.";
            return false;
          }
          std::string ts = ToUpperCaseF(v2[0].substr(9, v2[0].length()));
          if (ts != "PRO" && ts != "QUADRIC")
          {
            errorMessage = "Unknown decimation type in Decimate()-command detected.";
            return false;
          }
          std::string rrs = v2[1].substr(0, v2[1].length() - 1);
          if (rrs.length() > 0)
          {
            if (!simulation) // PROCESS: DECIMATION
            {
              vtkSmartPointer<vtkFunctionParser> fp =
                  vtkSmartPointer<vtkFunctionParser>::New();
              std::vector<std::string> fpstuids;
              std::vector<std::string> fpattribs;
              bool fpsucc = true;
              fpsucc = fpsucc && ParseGenericVariables(rrs, fpstuids, fpattribs, structureUID);
              if (!fpsucc)
                return false;
              for (std::size_t u = 0; u < fpstuids.size(); u++)
              {
                std::string varname = "v{" + ToLowerCaseF(fpstuids[u] + "." + fpattribs[u]);
                varname += "}";
                fp->SetScalarVariableValue(varname.c_str(),
                    GetGenericVariableValue(fpstuids[u], fpattribs[u], structureUID, pd));
              }
              std::string fpfunc = ToLowerCaseF(rrs);
              ReplaceString(fpfunc, "#", ",");
              fp->SetFunction(fpfunc.c_str());
              double ratio = fp->GetScalarResult();
              vtkSmartPointer<vtkPolyDataAlgorithm> filter = NULL;
              if (ts == "PRO")
              {
                vtkSmartPointer<vtkDecimatePro> f =
                    vtkSmartPointer<vtkDecimatePro>::New();
                f->SetTargetReduction(ratio / 100.);
                f->SetFeatureAngle(90.);
                filter = f;
              }
              else // if (type == "QUADRIC")
              {
                vtkSmartPointer<vtkQuadricDecimation> f = vtkSmartPointer<
                    vtkQuadricDecimation>::New();
                f->SetTargetReduction(ratio / 100.);
                f->AttributeErrorMetricOff();
                filter = f;
              }
              // apply decimation
              vtkSmartPointer<vtkPolyDataNormals> normals =
                  vtkSmartPointer<vtkPolyDataNormals>::New();
              normals->SetInput(pd);
              normals->SetComputePointNormals(true);
              normals->SetSplitting(false); //!
              normals->Update();
              filter->SetInput(normals->GetOutput());
              filter->Update();
              vtkSmartPointer<vtkPolyData> pd2 =
                  vtkSmartPointer<vtkPolyData>::New();
              if (ts == "PRO") // preserves normals
              {
                pd2->ShallowCopy(filter->GetOutput());
              }
              else
              {
                vtkSmartPointer<vtkPolyDataNormals> normals =
                    vtkSmartPointer<vtkPolyDataNormals>::New();
                normals->SetInput(filter->GetOutput());
                normals->SetComputePointNormals(true);
                normals->SetSplitting(false); //!
                normals->Update();
                pd2->ShallowCopy(normals->GetOutput());
              }
              pd = pd2;
            }
          }
          else
          {
            errorMessage = "Non-numeric or invalid (]0;100[) decimation factor in Decimate()-command detected.";
            return false;
          }
        }
        else
        {
          errorMessage = "Missing closing ')' in Decimate()-command detected.";
          return false;
        }
      }
      else if (ToUpperCaseF(t.substr(0, 5)) == "CLIP(") // Clip(<another-struct>)
      {
        if (t.substr(t.length() - 1, 1) == ")")
        {
          std::vector<std::string> v2;
          Tokenize(t, v2, ",");
          if (v2.size() != 2)
          {
            errorMessage = "Wrong number of arguments for Clip()-command.";
            return false;
          }
          std::string invstr = "";
          if (v2[1].length() > 0)
            invstr = v2[1].substr(0, v2[1].length() - 1);
          if (!IsStrictlyNumeric(invstr))
          {
            errorMessage = "Flag (0 or 1) expected for <inverse-flag> of Clip()-command.";
            return false;
          }
          std::string otherstruct = v2[0].substr(5, v2[0].length());
          if (otherstruct.length() > 0)
          {
            VTKStructure *otherVStruct = m_Structures->FindStructure(otherstruct);
            if (otherVStruct)
            {
              if (!simulation)
              {
                vtkPolyData *other = otherVStruct->GetStructurePolyData();
                vtkSmartPointer<vtkCleanPolyData> cleaner =
                    vtkSmartPointer<vtkCleanPolyData>::New();
                cleaner->SetInput(other);
                cleaner->SetPointMerging(true);
                cleaner->SetConvertLinesToPoints(true);
                cleaner->SetConvertPolysToLines(true);
                cleaner->SetConvertStripsToPolys(true);
                cleaner->SetPieceInvariant(true);
                cleaner->Update();
                vtkSmartPointer<vtkImplicitTriangulatedPolyData> impOther =
                    vtkSmartPointer<vtkImplicitTriangulatedPolyData>::New();
                impOther->SetInput(cleaner->GetOutput());
                impOther->SetUsePointNormals(false);

                vtkSmartPointer<vtkClipPolyData> clipper =
                    vtkSmartPointer<vtkClipPolyData>::New();
                clipper->SetInput(pd);
                clipper->SetClipFunction(impOther);
                clipper->SetInsideOut(atoi(invstr.c_str()));
                clipper->Update();
                vtkSmartPointer<vtkPolyData> pd2 =
                    vtkSmartPointer<vtkPolyData>::New();
                pd2->ShallowCopy(clipper->GetOutput());
                pd = pd2;
              }
            }
            else
            {
              errorMessage = "Invalid/unavailable structure UID in Clip()-command detected.";
              return false;
            }
          }
          else
          {
            errorMessage = "Empty structure UID in Clip()-command detected.";
            return false;
          }
        }
        else
        {
          errorMessage = "Missing closing ')' in Clip()-command detected.";
          return false;
        }
      }
      else if (ToUpperCaseF(t.substr(0, 11)) == "VOLUMEMASK(")
      {
        // Volumemask([<structure1>,...,<structureN>,]<filename>)
        if (t.substr(t.length() - 1, 1) == ")")
        {
          // Extract arguments (without command and closing bracket)
          std::vector<std::string> v2;
          Tokenize(t.substr(11, t.length() - 11 - 1), v2, ",");
          if (v2.size() < 1)
          {
            errorMessage = "Wrong number of arguments for Volumemask()-command.\n";
            errorMessage += "Requires at least one argument ([<structure-uid>,]<filename>).";
            return false;
          }

          // Validate and generate list of structures from arguments
          std::vector<vtkSmartPointer<vtkPolyData> > maskstructures;
          // Current structure
          maskstructures.push_back(pd);
          // Parse argument list
          bool error = false;
          for (std::size_t i = 0; i < v2.size() - 1; ++i)
          {
            std::string maskstrucureUID = v2[i];
            if (maskstrucureUID.length() > 0)
            {
              VTKStructure *maskVStructure = m_Structures->FindStructure(maskstrucureUID);
              if (maskVStructure)
              {
                maskstructures.push_back(maskVStructure->GetStructurePolyData());
              }
              else
              {
                errorMessage = "Invalid/unavailable structure UID in Volumemask()-command detected: ";
                errorMessage += structureUID;
                errorMessage += "\n";
                error = true;
              }
            }
            else
            {
              errorMessage = "Empty structure UID in Volumemask()-command detected at position";
              errorMessage += i;
              errorMessage += ".\n";
              error = true;
            }
          }
          if (error)
            return false;

          if (maskstructures.size() > 0)
          {
            if (!simulation)
            {
              ITKVTKImage *volume = NULL;
              if (m_Volume)
                volume = m_Volume->ProduceImage();
              if (volume)
              {
                // Create volume mask and write it to a file
                typedef unsigned char MaskVolumePixelType;
                typedef itk::Image<MaskPixelType, 3> MaskVolumeImageType;
                MaskVolumeImageType::Pointer volumeImageMask = NULL;
                TEMPLATE_CALL_COMP(volume->GetComponentType(),
                    volumeImageMask = CreateVolumeMask, volume, maskstructures)
                if (volumeImageMask)
                {
                  try
                  {
                    // Write ITK image to file
                    // NOTE: The image is oriented in the current frame of reference
                    // If the same orientation as the ora-xml mhd volume is desired
                    // use the ITKVTKImage code below
                    std::string filename = v2[v2.size() - 1];

                    // Export with ITK in current FOR orientation
                    //typedef itk::ImageFileWriter<MaskVolumeImageType> WriterType;
                    //writer->SetFileName(filename.c_str());
                    //WriterType::Pointer writer = WriterType::New();
                    //writer->SetInput(volumeImageMask);
                    //writer->Update();

                    // Export in ora xml format with reference to volume frame of reference
                    ITKVTKImage volumeImageMaskITKVTK(itk::ImageIOBase::SCALAR, itk::ImageIOBase::UCHAR);
                    volumeImageMaskITKVTK.SetITKImage(static_cast<ITKVTKImage::ITKImageType *>(volumeImageMask.GetPointer()), false);
                    volumeImageMaskITKVTK.SetMetaInfo(volume->GetMetaInfo());
                    volumeImageMaskITKVTK.SaveImageAsORAMetaImage<MaskVolumePixelType>(filename.c_str(), true);

                  }
                  catch (itk::ExceptionObject &e)
                  {
                    std::cerr << "Error at writing volume mask in Volumemask()-command.\n";
                    std::cerr << e.GetDescription() << std::endl;
                  }
                }
                else
                {
                  std::cerr << "Error at generation of volume mask in Volumemask()-command.\n";
                }
              }
            }
          }
          else
          {
            errorMessage = "Empty structure list in Volumemask()-command detected.";
            return false;
          }
        }
        else
        {
          errorMessage = "Missing closing ')' in Clip()-command detected.";
          return false;
        }
      }
      else
      {
        errorMessage = "Unknown structure pre-processing command (" + t;
        errorMessage += ") detected.";
        return false;
      }
    }
    j++;
  } while (t.length() > 0);

  if (simulation) // -> add dummy-entry in order to support FindStructure()!
  {
    VTKStructure *astruct = new VTKStructure();
    astruct->SetFileName(oraFileName); // only filled for ORA-structures!
    astruct->SetStructureUID(structureUID);
    m_Structures->AddStructure(astruct);
  }
  else
  {
    VTKStructure *astruct = new VTKStructure();
    astruct->SetFileName(oraFileName); // only filled for ORA-structures!
    astruct->SetStructureUID(structureUID);
    astruct->SetStructureSetUID(stsuid);
    astruct->SetStructurePolyData(pd);
    astruct->SetContourProperty(prop);
    m_Structures->AddStructure(astruct);
  }

  return true;
}

double *REG23Model::GetSourcePosition(std::size_t index)
{
  if (index >= m_SourcePositions.size())
    return NULL;
  return m_SourcePositions[index];
}

void REG23Model::GetCenterOfRotation(double centerOfRotation[3])
{
  if (centerOfRotation)
  {
    centerOfRotation[0] = m_CenterOfRotation[0];
    centerOfRotation[1] = m_CenterOfRotation[1];
    centerOfRotation[2] = m_CenterOfRotation[2];
  }
}

void REG23Model::OverrideAndApplyCurrentParameters(ParametersType parameters)
{
  if (!m_Transform)
    return;
  m_CurrentParameters = parameters;
  m_Transform->SetParameters(m_CurrentParameters);

  this->Notify(UPDATE_PARAMETERS); // signal
}

void REG23Model::ConcatenateAndApplyRelativeAxisAngleRotation(double *axis,
    double angle)
{
  if (!m_Transform || !axis)
    return;

  typedef itk::VersorRigid3DTransform<double> VersorTransformType;
  // temporary versor transform that makes up the axis/angle transform:
  VersorTransformType::VersorType::VectorType ax;
  ax[0] = axis[0];
  ax[1] = axis[1];
  ax[2] = axis[2];
  VersorTransformType::VersorType v;
  v.Set(ax, angle);
  VersorTransformType::Pointer transform = VersorTransformType::New();
  transform->SetCenter(m_Transform->GetCenter()); // current COR!
  transform->SetRotation(v); // axis/angle rotation to be concatenated
  // compose
  m_Transform->Compose(transform, false);
  // update parameters
  m_CurrentParameters = m_Transform->GetParameters();

  this->Notify(UPDATE_PARAMETERS); // signal
}

REG23Model::ParametersType REG23Model::ComputeTransformationFromPixelInPlaneTranslation(
    unsigned int index, int dx, int dy)
{
  ParametersType resultpars(0);
  if (index >= m_FixedImages.size())
    return resultpars;
  // extract fixed image spacing
  ITKVTKImage *fi = this->GetFixedImage(index, true);
  if (!fi)
    return resultpars;
  ITKVTKImage::ITKImagePointer ibase = fi->GetAsITKImage<DRRPixelType>();
  if (!ibase)
    return resultpars;
  typedef itk::Image<DRRPixelType, ITKVTKImage::Dimensions> OriginalImageType;
  OriginalImageType::Pointer fii = static_cast<OriginalImageType *>(ibase.GetPointer());
  OriginalImageType::SpacingType spac = fii->GetSpacing();
  double sdx = spac[0] * dx;
  double sdy = spac[1] * dy;
  // -> now we can compute in physical coordinates:
  return ComputeTransformationFromPhysicalInPlaneTranslation(index, sdx, sdy);
}

REG23Model::ParametersType REG23Model::ComputeTransformationFromPhysicalInPlaneTranslation(
    unsigned int index, double dx, double dy)
{
  ParametersType resultpars(0);
  if (index >= m_FixedImages.size())
    return resultpars;
  if (!IsReadyForManualRegistration())
    return resultpars;
  // extract fixed image position and direction
  ITKVTKImage *fi = this->GetFixedImage(index, true);
  if (!fi)
    return resultpars;
  ITKVTKImage::ITKImagePointer ibase = fi->GetAsITKImage<DRRPixelType>();
  if (!ibase)
    return resultpars;
  typedef itk::Image<DRRPixelType, ITKVTKImage::Dimensions> OriginalImageType;
  OriginalImageType::Pointer fii = static_cast<OriginalImageType *>(ibase.GetPointer());
  OriginalImageType::DirectionType dir = fii->GetDirection();
  double v1[3]; // NOTE: ITK-matrix is column-based!
  v1[0] = dir[0][0];
  v1[1] = dir[1][0];
  v1[2] = dir[2][0];
  double v2[3];
  v2[0] = dir[0][1];
  v2[1] = dir[1][1];
  v2[2] = dir[2][1];
  double n[3];
  n[0] = dir[0][2];
  n[1] = dir[1][2];
  n[2] = dir[2][2];
  OriginalImageType::PointType orig = fii->GetOrigin();
  double o[3];
  o[0] = orig[0];
  o[1] = orig[1];
  o[2] = orig[2];
  // extract position of projected center of rotation:
  double cor3D[3];
  cor3D[0] = m_CenterOfRotation[0];
  cor3D[1] = m_CenterOfRotation[1];
  cor3D[2] = m_CenterOfRotation[2];
  double cor2D[3];
  double t = 0;
  vtkPlane::IntersectWithLine(m_SourcePositions[index], cor3D, n, o, t, cor2D);
  // add in-plane translation
  double x[3];
  x[0] = cor2D[0] + v1[0] * dx + v2[0] * dy;
  x[1] = cor2D[1] + v1[1] * dx + v2[1] * dy;
  x[2] = cor2D[2] + v1[2] * dx + v2[2] * dy;
  // back-project the translated coordinate onto the plane that is perpendicular
  // to the direction of projection
  n[0] = m_SourcePositions[index][0] - cor3D[0];
  n[1] = m_SourcePositions[index][1] - cor3D[1];
  n[2] = m_SourcePositions[index][2] - cor3D[2];
  double x2[3];
  vtkPlane::IntersectWithLine(m_SourcePositions[index], x, n, cor3D, t, x2);
  // compute resultant 3D translation vector
  resultpars = m_CurrentParameters;
  resultpars[3] += x2[0] - cor3D[0];
  resultpars[4] += x2[1] - cor3D[1];
  resultpars[5] += x2[2] - cor3D[2];

  return resultpars;
}

double REG23Model::ComputeFixedImagesAcquisitionTimeDivergence()
{
  if (!m_ShowWarningOnDivergingFixedImageAcquisitionTimes ||
      !HaveORAConnectivity())
    return 0; // forget it

  std::vector<std::string> atimes;
  ITKVTKImage *fixedImage;
  for (std::size_t i = 0; i < m_FixedImageFileNames.size(); i++)
  {
    fixedImage = GetFixedImage(i, false);
    if (fixedImage)
    {
      if (fixedImage->GetMetaInfo())
      {
        ITKVTKImageMetaInformation::Pointer mi = fixedImage->GetMetaInfo();
        std::vector<SliceMetaInformation::Pointer> *smis = mi->
            GetSlicesMetaInformation();
        SliceMetaInformation::Pointer smi = NULL;
        if (smis && smis->size() > 0)
          smi = (*smis)[0];
        if (smi)
        {
          if (smi->GetORAAcquisitionDate().length() > 0)
            atimes.push_back(smi->GetORAAcquisitionDate());
        }
      }
    }
  }
  if (atimes.size() != m_FixedImageFileNames.size())
    return (m_MaxFixedImageAcquisitionDivergingTimeMin + 1); // could not retrieve all times!

  tm minTime;
  time_t minTime2;
  ORAStringToCDateTime(atimes[0], minTime); // at least 1 fixed image guaranteed
  minTime2 = mktime(&minTime);
  tm maxTime;
  time_t maxTime2;
  ORAStringToCDateTime(atimes[0], maxTime);
  maxTime2 = mktime(&maxTime);
  tm t;
  time_t t2;
  double d;
  for (std::size_t i = 1; i < atimes.size(); i++) // min/max times
  {
    ORAStringToCDateTime(atimes[i], t);
    t2 = mktime(&t);
    d = difftime(t2, minTime2);
    if (d < 0)
      minTime2 = t2;
    d = difftime(t2, maxTime2);
    if (d > 0)
      maxTime2 = t2;
  }
  // compute overall diff
  d = fabs(difftime(minTime2, maxTime2)) / 60.;
  return d;
}

bool REG23Model::IsHardwareAdequate(std::string &nonSupportReasons)
{
  if (!m_DRRToolRenderWindow)
    return false;
  GLSLDRRRayCastMapper *dummy = GLSLDRRRayCastMapper::New();
  dummy->SetRenderWindow(m_DRRToolRenderWindow);
  bool ok = dummy->IsDRRComputationSupported();
  if (!ok)
    nonSupportReasons = std::string(dummy->GetDRRComputationNotSupportedReasons());
  dummy->Delete();
  dummy = NULL;
  return ok;
}

bool REG23Model::ParseORASetupErrors(std::vector<std::string> &entries,
    std::string &errorMessage)
{
  m_ReferenceParameters.SetSize(0); // initialize!

  if (entries.size() <= 0)
    return true; // OK

  // transformation implied by initial parameters concatenated with setup
  // error transformation:
  TransformType::Pointer transform = TransformType::New();
  transform->SetCenter(m_CenterOfRotation);

  std::string sv = m_Config->ReadString("Science", "DoNotAddInitialTransform", "1");
  if (atoi(sv.c_str())) // DO NOT prepend initial parameters
  {
    TransformType::ParametersType zero(m_InitialParameters.Size());
    zero.fill(0);
    transform->SetParameters(zero);
  }
  else // prepend initial parameters
  {
    transform->SetParameters(m_InitialParameters);
  }

  SetupError::Pointer se = NULL;
  SetupError::TransformType *t = NULL;
  SetupError::TransformType::InputPointType cor;
  TransformType::Pointer tt = NULL;
  SetupError::TransformType::MatrixType m;
  SetupError::TransformType::OutputVectorType o;
  double isocMM[3];
  isocMM[0] = m_IsoCenter[0] * 10; // cm -> mm
  isocMM[1] = m_IsoCenter[1] * 10; // cm -> mm
  isocMM[2] = m_IsoCenter[2] * 10; // cm -> mm
  std::vector<std::string> toks;
  for (std::size_t i = 0; i < entries.size(); i++)
  {
    toks.clear();
    Tokenize(entries[i], toks, "*");

    if (toks.size() != 2)
    {
      errorMessage = "Expecting entries of form <mode>*<(study)-setup-error-entry>!";
      return false;
    }

    se = NULL;
    se = SetupError::New();

    if (ToLowerCaseF(TrimF(toks[0])) == "rotation")
    {
      se->SetAllowRotation(true);
    }
    else if (ToLowerCaseF(TrimF(toks[0])) == "no-rotation")
    {
      se->SetAllowRotation(false);
    }
    else
    {
      errorMessage = "Only ROTATION and NO-ROTATION mode flags are available!";
      return false;
    }

    if (!se->LoadFromORAString(entries[i]))
    {
      errorMessage = "Obviously the format of a specified ORA (Study)SetupError entry (index ";
      errorMessage += StreamConvert(i + i) + ") is invalid. Could not be parsed!";
      return false;
    }

    se->CorrectCenterOfRotation(isocMM); // correct because we've shifted the volume!

    // transformation implied by setup error:
    t = se->GetTransform();
    // -> convert into this transform representation:
    m = t->GetMatrix();
    o = t->GetOffset();
    tt = NULL;
    tt = TransformType::New();
    // NOTE: ORA rotation matrices appear to be sometimes very inaccurate -
    // would cause an exception to directly set it on Euler3D-transform! So,
    // use the method of matrix-offset-transform-base to overcome this problem!
    tt->TransformType::Superclass::Superclass::SetMatrix(m);
    tt->SetOffset(o);
    // concatenate with current transform:
    transform->Compose(tt);
  }

  // store reference transform parameters:
  m_ReferenceParameters = transform->GetParameters();

  return true;
}

bool REG23Model::IsUnsharpMaskingEnabled(std::size_t i)
{
  if (i > m_UnsharpMaskingEnabled.size() || i < 1)
    return false;
  return m_UnsharpMaskingEnabled[i - 1];
}

bool REG23Model::IsUnsharpMaskingActivated(std::size_t i)
{
  if (i > m_UnsharpMaskingActivated.size() || i < 1)
    return false;
  return m_UnsharpMaskingActivated[i - 1];
}

void REG23Model::StoreWindowLevelToFile(std::size_t index,
    double fixedImageWL[2], double unsharpMaskWL[2])
{
  if (m_WindowLevelStorageStrategy == 1 && IsReadyForAutoRegistration())
  {
    if (index >= m_FixedImageFileNames.size())
      return;
    if (itksys::SystemTools::FileExists(m_FixedImageFileNames[index].c_str()))
    {
      std::string fn = itksys::SystemTools::GetFilenamePath(
          m_FixedImageFileNames[index]);
      EnsureStringEndsWith(fn, DSEP);
      fn += "reg23-wl.inf";
      IniAccess f(fn);
      f.SetAddORACheckSum(true);
      f.WriteValue<double>("Window/Level", "FixedImageWindow", fixedImageWL[0]);
      f.WriteValue<double>("Window/Level", "FixedImageLevel", fixedImageWL[1]);
      if (IsUnsharpMaskingEnabled(index + 1))
      {
        f.WriteValue<double>("Window/Level", "UnsharpMaskImageWindow", unsharpMaskWL[0]);
        f.WriteValue<double>("Window/Level", "UnsharpMaskImageLevel", unsharpMaskWL[1]);
      }
      f.WriteString("File", "FileFormat", "1.0");
      std::string ts = "";
      CurrentORADateTimeString(ts);
      f.WriteString("File", "TimeStamp", ts);
      f.Update();
    }
  }
}

bool REG23Model::ParseSparsePreRegistrationEntries(std::string section,
    std::string &errorKey, std::string &errorMessage)
{
  errorMessage = "";
  m_SparsePreRegistrationType = "";
  if (!m_Config || section.length() <= 0)
    return false;

  errorKey = "SparsePreRegistration";
  std::string s = ToLowerCaseF(TrimF(m_Config->ReadString(section, errorKey, "")));
  while (ReplaceString(s, " ", ""));
  std::vector<std::string> toks;
  if (s.length() > 0)
  {
    if (s.substr(0, 17) == "crosscorrelation(")
    {
      s = s.substr(17, s.length());
      s = s.substr(0, s.length() - 1); // cut last ")"
      Tokenize(s, toks, ",");
      if (toks.size() < 2 || toks.size() > 4)
      {
        errorMessage = "The CrossCorrelation()-command needs exactly 2 to 4 arguments!";
        return false;
      }
      if (!IsStrictlyNumeric(toks[0]) || atoi(toks[0].c_str()) < 1)
      {
        errorMessage = "The number of iterations must equal at least 1!";
        return false;
      }
      if (!IsNumeric(toks[1]))
      {
        errorMessage = "The tolerance argument must be numeric!";
        return false;
      }
      for (unsigned int i = 2; i < toks.size(); ++i) {
        if (!(ToUpperCaseF(toks[i]) == "FAST" || ToUpperCaseF(toks[i]) == "RADIUS"))
        {
          errorMessage = "The " + i;
          errorMessage += "th argument must be 'fast' or 'radius'!";
          return false;
        }
      }
      // now check the view-specific configurations:
      bool haveOneView = false;
      bool structEntryValid;
      for (std::size_t i = 1; i <= m_FixedImageFileNames.size(); i++)
      {
        errorKey = "CrossCorrelationView" + StreamConvert(i);
        s = TrimF(m_Config->ReadString(section, errorKey, ""));
        if (s.length() > 0)
        {
          toks.clear();
          Tokenize(s, toks, ",");
          if (toks.size() == 10)
          {
            if (!IsStrictlyNumeric(toks[0]))
            {
              errorMessage = "The unsharp-masking flag must be 0 or 1!";
              return false;
            }
            for (std::size_t k = 1; k < 8; k++)
            {
              if (!IsNumeric(toks[k]))
              {
                errorMessage = "All region- and patch-related entries must be numeric!";
                return false;
              }
            }
            structEntryValid = false;
            if (TrimF(ToUpperCaseF(toks[9])) == "VOLUMECENTER") // by volume
              structEntryValid = true;
            if (m_Structures->FindStructure(toks[9])) // by structure
              structEntryValid = true;
            if (toks[9].length() >= 7 && /** at least: (0|0|0) **/
                toks[9].substr(0, 1) == "(" &&
                toks[9].substr(toks[9].length() - 1, 1) == ")") // explicit
            {
              std::vector<std::string> itoks;
              itoks.clear();
              std::string tmp = TrimF(toks[9].substr(1, toks[9].length() - 2));
              Tokenize(tmp, itoks, "|");
              if (itoks.size() == 3 && IsNumeric(itoks[0]) &&
                  IsNumeric(itoks[1]) && IsNumeric(itoks[2]))
                structEntryValid = true;
            }
            if (!structEntryValid)
            {
              errorMessage = "Invalid structure or reference point '" + toks[9] + "' specified!\n";
              errorMessage += "Must be 'VOLUMECENTER' or a valid structure-uid or an explicit coordinate of the form (<x>,<y>,<z>).";
              return false;
            }
          }
          else
          {
            errorMessage = "A CrossCorrelationView-entry needs exactly 10 arguments!";
            return false;
          }
          haveOneView = true;
        } // else: empty is OK
      }
      if (!haveOneView)
      {
        errorMessage = "The CrossCorrelation()-command needs at least one configured view!";
        return false;
      }
      m_SparsePreRegistrationType = "CROSSCORRELATION";
    }
    else if (s.substr(0, 5) == "none(")
    {
      m_SparsePreRegistrationType = "NONE";
    }
    else
    {
      errorMessage = "Unknown sparse pre-registration type!";
      return false;
    }
  }
  else // default: None()
  {
    m_SparsePreRegistrationType = "NONE";
  }
  return true;
}

bool REG23Model::ParseInitialRotationTranslationTransform(std::string section,
    std::string &errorKey, std::string &errorMessage)
{
  errorMessage = "";
  if (section.length() <= 0)
    return false;

  errorKey = "InitialRotationMatrix";
  std::string sr = TrimF(m_Config->ReadString(section, errorKey, ""));
  errorKey = "InitialTranslation";
  std::string st = TrimF(m_Config->ReadString(section, errorKey, ""));

  if (sr.length() <= 0 && st.length() <= 0)
    return true; // this is OK !

  if (sr.length() <= 0 || st.length() <= 0)
  {
    if (sr.length() <= 0)
      errorKey = "InitialRotationMatrix";
    errorMessage = "Both keys (InitialRotationMatrix and InitialTranslation) must be specified!";
    return false;
  }

  std::vector<std::string> toks;
  toks.clear();
  Tokenize(sr, toks, ",");
  if (toks.size() != 9)
  {
    errorMessage = "There must be 9 rotation matrix coefficients!";
    return false;
  }
  bool ok = true;
  int c = 0;
  TransformType::MatrixType matrix;
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      if (!IsNumeric(toks[c]))
      {
        ok = false;
      }
      else
      {
        matrix[i][j] = atof(toks[c].c_str());
      }
      c++;
    }
  }
  if (!ok)
  {
    errorMessage = "All rotation matrix coefficients must be numeric!";
    return false;
  }

  toks.clear();
  Tokenize(st, toks, ",");
  if (toks.size() != 3)
  {
    errorMessage = "There must be 3 translation coefficients!";
    return false;
  }
  ok = true;
  TransformType::OutputVectorType translation;
  for (int i = 0; i < 3; i++)
  {
    if (!IsNumeric(toks[i]))
    {
      ok = false;
    }
    else
    {
      translation[i] = atof(toks[i].c_str()) * 10.; // cm -> mm
    }
  }
  if (!ok)
  {
    errorMessage = "All translation coefficients must be numeric!";
    return false;
  }

  // -> convert into Euler angles (w.r.t. the specified center of rotation):
  TransformPointer tt = TransformType::New();
  tt->SetCenter(m_CenterOfRotation);
  // NOTE: ORA rotation matrices appear to be sometimes very inaccurate -
  // would cause an exception to directly set it on Euler3D-transform! So,
  // use the method of matrix-offset-transform-base to overcome this problem!
  tt->TransformType::Superclass::Superclass::SetMatrix(matrix);
  tt->SetOffset(translation);

  m_InitialParameters = tt->GetParameters();

  return true;
}

bool REG23Model::LoadIntelligentMasks()
{
  if (!this->IsIntelligentMaskMode() || m_FixedImageFileNames.size() <= 0)
    return false;

  const double MAX_SPACING_TOL = 1e-3; // mm
  const double MAX_PLANE_POINTS_TOL = 3.0; // mm
  const double MAX_FOCUS_TOL = 3.0; // mm

  // for each image check: whether there is a reg23-masks.inf file
  bool infoFileMissing = false;
  std::string pth = "", fn = "", s = "", s2 = "";
  std::vector<std::string> infoFileNames;
  for (std::size_t i = 0; i < m_FixedImageFileNames.size(); i++)
  {
    pth = itksys::SystemTools::GetFilenamePath(m_FixedImageFileNames[i]);
    ora::EnsureStringEndsWith(pth, ora::DSEP);
    fn = pth + "reg23-masks.inf";
    if (!itksys::SystemTools::FileExists(fn.c_str()))
    {
      infoFileMissing = true;
      break;
    }
    infoFileNames.push_back(fn);
  }
  if (infoFileMissing)
    return false;

  // check which candidate of each mask info file generally matches the current
  // fixed images:
  std::vector<std::string> candidates, toks;
  ora::IniAccess *maskInf = NULL;
  std::vector<std::string> *indents = NULL;
  double fspac[2], forig[3], fdir[9], ffocus[3], fplanePoints[4][3], fpp[3];
  int fsz[2];
  double refSpac[3], refOrig[3], refDir[9], refPlanePoints[4][3], refPP[3];
  int refSz[3];
  ITKVTKImage *ivi = NULL;
  bool ret = false;
  // current structure rules (reference):
  std::vector<std::string> refStructRules, currStructRules;
  indents = m_Config->GetIndents("Structures");
  if (indents)
  {
    for (std::size_t i = 0; i < indents->size(); i++)
    {
      if (ora::ToLowerCaseF((*indents)[i].substr(0, 9)) == "structure")
      {
        s = ora::ToLowerCaseF((*indents)[i]) + "=";
        s += ora::ToLowerCaseF(m_Config->ReadString("Structures", (*indents)[i], ""));
        ora::Trim(s);
        refStructRules.push_back(s);
      }
    }
    delete indents;
    std::sort(refStructRules.begin(), refStructRules.end()); // default sort
  }
  else
  {
    return false;
  }
  std::vector<std::string> refMaskingRules, currMaskingRules;
  std::ostringstream os;
  std::vector<VTKStructure *> structs;
  long int timeStamp, fileSize, refTimeStamp, refFileSize;
  std::vector<bool> maskOK;
  std::vector<std::string> maskImageFile;
  for (std::size_t i = 0; i < infoFileNames.size(); i++)
  {
    maskOK.push_back(false); // init
    maskImageFile.push_back(""); // init
    candidates.clear();
    maskInf = new ora::IniAccess(infoFileNames[i]);
    pth = itksys::SystemTools::GetFilenamePath(infoFileNames[i]);
    ora::EnsureStringEndsWith(pth, ora::DSEP);

    indents = maskInf->GetIndents("masks");
    if (indents)
    {
      for (std::size_t j = 0; j < indents->size(); j++)
      {
        if (!maskInf->ReadValue<bool>("masks", (*indents)[j], false))
          continue;
        candidates.push_back((*indents)[j]);
      }
      delete indents;
      if (candidates.size() <= 0)
      {
        delete maskInf;
        return false;
      }
    }
    else
    {
      delete maskInf;
      return false;
    }

    // -> extract current fixed image's geometry:
    ivi = m_PPFixedImages[i]->ProduceImage();
    if (!ivi)
      return false; // should not happen!
    TEMPLATE_CALL_COMP(ivi->GetComponentType(),
            ret = ivi->ExtractBasicImageGeometry, refSpac, refSz, refOrig, refDir)
    if (!ret)
      return false; // should not happen!
    // -> resultant image plane corner points:
    refPlanePoints[0][0] = refOrig[0]; // P0
    refPlanePoints[0][1] = refOrig[1];
    refPlanePoints[0][2] = refOrig[2];
    refPlanePoints[1][0] = refOrig[0] + refDir[0] * (double)refSz[0] * refSpac[0]; // P1
    refPlanePoints[1][1] = refOrig[1] + refDir[1] * (double)refSz[0] * refSpac[0];
    refPlanePoints[1][2] = refOrig[2] + refDir[2] * (double)refSz[0] * refSpac[0];
    refPlanePoints[2][0] = refOrig[0] + refDir[3] * (double)refSz[1] * refSpac[1]; // P2
    refPlanePoints[2][1] = refOrig[1] + refDir[4] * (double)refSz[1] * refSpac[1];
    refPlanePoints[2][2] = refOrig[2] + refDir[5] * (double)refSz[1] * refSpac[1];
    refPlanePoints[3][0] = refPlanePoints[1][0] + refDir[3] * (double)refSz[1] * refSpac[1]; // P3
    refPlanePoints[3][1] = refPlanePoints[1][1] + refDir[4] * (double)refSz[1] * refSpac[1];
    refPlanePoints[3][2] = refPlanePoints[1][2] + refDir[5] * (double)refSz[1] * refSpac[1];

    // -> ok, now check each of the candidates:
    for (int j = (int)(candidates.size() - 1); j >= 0; j--) // reverse (newer are usually last)
    {
      // check if mask image exists:
      fn = maskInf->ReadString("images", candidates[j], "");
      ret = true;
      if (fn.length() <= 0)
        ret = false; // no mask image specified
      fn = pth + fn;
      if (!itksys::SystemTools::FileExists(fn.c_str()))
        ret = false; // mask image not found on disk
      if (!ret)
      {
        // -> inactive this entry as it is no longer usable!
        maskInf->WriteValue<bool>("masks", candidates[j], false);
        maskInf->Update();
        continue; // no mask image defined!
      }
      maskImageFile[i] = fn; // store

      // extract reference fixed image geometry:
      // - spacing
      toks.clear();
      s = maskInf->ReadString(candidates[j] + "-fixed-image-geometry", "spacing", "");
      ora::Tokenize(s, toks, ",");
      if (toks.size() != 2)
        continue; // invalid spacing
      fspac[0] = atof(toks[0].c_str());
      fspac[1] = atof(toks[1].c_str());
      // - size
      toks.clear();
      s = maskInf->ReadString(candidates[j] + "-fixed-image-geometry", "size", "");
      ora::Tokenize(s, toks, ",");
      if (toks.size() != 2)
        continue; // invalid size
      fsz[0] = atoi(toks[0].c_str());
      fsz[1] = atoi(toks[1].c_str());
      // - origin
      toks.clear();
      s = maskInf->ReadString(candidates[j] + "-fixed-image-geometry", "origin", "");
      ora::Tokenize(s, toks, ",");
      if (toks.size() != 3)
        continue; // invalid origin
      forig[0] = atof(toks[0].c_str());
      forig[1] = atof(toks[1].c_str());
      forig[2] = atof(toks[2].c_str());
      // - direction
      toks.clear();
      s = maskInf->ReadString(candidates[j] + "-fixed-image-geometry", "orientation", "");
      ora::Tokenize(s, toks, ",");
      if (toks.size() != 9)
        continue; // invalid direction
      for (int d = 0; d < 9; d++)
        fdir[d] = atof(toks[d].c_str());
      // - focus
      toks.clear();
      s = maskInf->ReadString(candidates[j] + "-fixed-image-geometry", "focus", "");
      ora::Tokenize(s, toks, ",");
      if (toks.size() != 3)
        continue; // invalid direction
      ffocus[0] = atof(toks[0].c_str());
      ffocus[1] = atof(toks[1].c_str());
      ffocus[2] = atof(toks[2].c_str());
      // -> resultant image plane corner points:
      fplanePoints[0][0] = forig[0]; // P0
      fplanePoints[0][1] = forig[1];
      fplanePoints[0][2] = forig[2];
      fplanePoints[1][0] = forig[0] + fdir[0] * (double)fsz[0] * fspac[0]; // P1
      fplanePoints[1][1] = forig[1] + fdir[1] * (double)fsz[0] * fspac[0];
      fplanePoints[1][2] = forig[2] + fdir[2] * (double)fsz[0] * fspac[0];
      fplanePoints[2][0] = forig[0] + fdir[3] * (double)fsz[1] * fspac[1]; // P2
      fplanePoints[2][1] = forig[1] + fdir[4] * (double)fsz[1] * fspac[1];
      fplanePoints[2][2] = forig[2] + fdir[5] * (double)fsz[1] * fspac[1];
      fplanePoints[3][0] = fplanePoints[1][0] + fdir[3] * (double)fsz[1] * fspac[1]; // P3
      fplanePoints[3][1] = fplanePoints[1][1] + fdir[4] * (double)fsz[1] * fspac[1];
      fplanePoints[3][2] = fplanePoints[1][2] + fdir[5] * (double)fsz[1] * fspac[1];

      // -> compare with current fixed image geometry:
      if (fsz[0] != refSz[0] || fsz[1] != refSz[1])
        continue; // size does not match
      if (fabs(fspac[0] - refSpac[0]) > MAX_SPACING_TOL ||
          fabs(fspac[1] - refSpac[1]) > MAX_SPACING_TOL)
        continue; // spacing does not match
      if (sqrt(vtkMath::Distance2BetweenPoints(ffocus, m_SourcePositions[i])) > MAX_FOCUS_TOL)
        continue; // focus does not match
      // for origin/direction comparison, we compare the image plane corner points:
      fpp[0] = fplanePoints[0][0]; refPP[0] = refPlanePoints[0][0];
      fpp[1] = fplanePoints[0][1]; refPP[1] = refPlanePoints[0][1];
      fpp[2] = fplanePoints[0][2]; refPP[2] = refPlanePoints[0][2];
      if (sqrt(vtkMath::Distance2BetweenPoints(fpp, refPP)) > MAX_PLANE_POINTS_TOL)
        continue; // plane points do not match
      fpp[0] = fplanePoints[1][0]; refPP[0] = refPlanePoints[1][0];
      fpp[1] = fplanePoints[1][1]; refPP[1] = refPlanePoints[1][1];
      fpp[2] = fplanePoints[1][2]; refPP[2] = refPlanePoints[1][2];
      if (sqrt(vtkMath::Distance2BetweenPoints(fpp, refPP)) > MAX_PLANE_POINTS_TOL)
        continue; // plane points do not match
      fpp[0] = fplanePoints[2][0]; refPP[0] = refPlanePoints[2][0];
      fpp[1] = fplanePoints[2][1]; refPP[1] = refPlanePoints[2][1];
      fpp[2] = fplanePoints[2][2]; refPP[2] = refPlanePoints[2][2];
      if (sqrt(vtkMath::Distance2BetweenPoints(fpp, refPP)) > MAX_PLANE_POINTS_TOL)
        continue; // plane points do not match
      fpp[0] = fplanePoints[3][0]; refPP[0] = refPlanePoints[3][0];
      fpp[1] = fplanePoints[3][1]; refPP[1] = refPlanePoints[3][1];
      fpp[2] = fplanePoints[3][2]; refPP[2] = refPlanePoints[3][2];
      if (sqrt(vtkMath::Distance2BetweenPoints(fpp, refPP)) > MAX_PLANE_POINTS_TOL)
        continue; // plane points do not match
      // check whether the structure rules match:
      currStructRules.clear();
      indents = maskInf->GetIndents(candidates[j] + "-structure-rules");
      if (indents)
      {
        for (std::size_t k = 0; k < indents->size(); k++)
        {
          if (ora::ToLowerCaseF((*indents)[k].substr(0, 9)) == "structure")
          {
            s = ora::ToLowerCaseF((*indents)[k]) + "=";
            s += ora::ToLowerCaseF(maskInf->ReadString(candidates[j] + "-structure-rules",
                (*indents)[k], ""));
            ora::Trim(s);
            currStructRules.push_back(s);
          }
        }
        delete indents;
        std::sort(currStructRules.begin(), currStructRules.end()); // default sort
      }
      else
      {
        continue; // no structure rules defined
      }
      if (refStructRules.size() != currStructRules.size())
        continue; // size does not match (also 0 entries allowed!!!)
      ret = true;
      for (std::size_t k = 0; k < currStructRules.size(); k++)
      {
        if (currStructRules[k] != refStructRules[k])
        {
          ret = false;
          break;
        }
      }
      if (!ret)
        continue; // current structure rules do not match the reference rules!

      // check whether the mask generation rules match:
      // - extract appropriate reference rules:
      os.str(""); os << "mask" << (i + 1);
      refMaskingRules.clear();
      indents = m_Config->GetIndents("Auto-Masking");
      if (indents)
      {
        for (std::size_t k = 0; k < indents->size(); k++)
        {
          if (ora::ToLowerCaseF((*indents)[k].substr(0, os.str().length())) == os.str())
          {
            s = ora::ToLowerCaseF((*indents)[k].substr(os.str().length(), 100)) + "=";
            s += ora::ToLowerCaseF(m_Config->ReadString("Auto-Masking", (*indents)[k], ""));
            ora::Trim(s);
            refMaskingRules.push_back(s);
          }
        }
        delete indents;
        std::sort(refMaskingRules.begin(), refMaskingRules.end()); // default sort
      }
      else
      {
        continue; // no reference rules found
      }
      // - extract current rules:
      currMaskingRules.clear();
      indents = maskInf->GetIndents(candidates[j] + "-mask-rules");
      if (indents)
      {
        for (std::size_t k = 0; k < indents->size(); k++)
        {
          s = ora::ToLowerCaseF((*indents)[k]) + "=";
          s += ora::ToLowerCaseF(maskInf->ReadString(candidates[j] + "-mask-rules",
              (*indents)[k], ""));
          ora::Trim(s);
          currMaskingRules.push_back(s);
        }
        delete indents;
        std::sort(currMaskingRules.begin(), currMaskingRules.end()); // default sort
      }
      else
      {
        continue; // no masking rules defined
      }
      // - compare the rules:
      if (refMaskingRules.size() != currMaskingRules.size())
        continue; // size does not match (also 0 entries allowed!!!)
      ret = true;
      for (std::size_t k = 0; k < currMaskingRules.size(); k++)
      {
        if (currMaskingRules[k] != refMaskingRules[k])
        {
          ret = false;
          break;
        }
      }
      if (!ret)
        continue; // current masking rules do not match the reference rules!

      // check ORA structure date/time and/or size:
      structs = m_Structures->GetStructures();
      ret = true;
      for (std::size_t k = 0; k < structs.size(); k++)
      {
        if (structs[k]->GetFileName().length() > 0) // obviously ORA source
        {
          // -> entries must exist:
          s = structs[k]->GetFileName() + "|";
          timeStamp = maskInf->ReadValue<long int>(candidates[j] + "-structure-info", s + "timestamp", 0);
          fileSize = maskInf->ReadValue<long int>(candidates[j] + "-structure-info", s + "sizestamp", 0);
          if (timeStamp <= 0 || fileSize <= 0)
          {
            ret = false;
            break;
          }
          refTimeStamp = itksys::SystemTools::ModifiedTime(structs[k]->GetFileName().c_str());
          refFileSize = itksys::SystemTools::FileLength(structs[k]->GetFileName().c_str());
          if (refFileSize != fileSize) // file size comparison
          {
            ret = false;
            break;
          }
          if (refTimeStamp != timeStamp) // modified time comparison
          {
            ret = false;
            break;
          }
        }
      }
      if (!ret)
        continue; // time/size stamp not defined or mismatch

      // eventually, we need special stored structure properties:
      s = ToUpperCaseF(TrimF(m_Config->ReadString("Registration", "SparsePreRegistration", "")));
      if (s.substr(0, 17) == "CROSSCORRELATION(") // we have a cross-correlation specified
      {
        s2 = TrimF(m_Config->ReadString("Registration", "CrossCorrelationView" + StreamConvert(i + 1), ""));
        toks.clear();
        Tokenize(s2, toks, ",");
        if (toks.size() == 10)
        {
          std::string suid = TrimF(toks[9]);
          structs = m_Structures->GetStructures();
          ret = false;
          for (std::size_t n = 0; n < structs.size(); n++)
          {
            if (structs[n]->GetStructureUID() == suid) // identified!
            {
              ret = true;
              break;
            }
          }
          if (ret) // -> yes, there is a structure UID involved in the correlation command!
          {
            // -> in order to be able to execute the sparse pre-registration without loading
            // structures, the stored props of it MUST BE AVAILABLE!
            s = maskInf->ReadString(candidates[j] + "-stored-structure-props",
                "SparsePreRegistration.CrossCorrelation.StructureCenter." + suid, "");
            if (s.length() <= 0) // cross-correlation specific structure props not available!
              continue;

            // OK, the props seem to be available -> exchange the structure uid with
            // the store explicit coordinates:
            ReplaceString(s2, suid, s);
            m_Config->WriteString("Registration", "CrossCorrelationView" + StreamConvert(i + 1), s2);
          }
        }
      }

      // OK, we did it!!!
      maskOK[i] = true; // mark
      break; // proceed with next fixed image ...
    }

    delete maskInf;
  }

  if (maskOK.size() != m_FixedImageFileNames.size())
    return false; // should not happen!
  for (std::size_t k = 0; k < maskOK.size(); k++)
  {
    if (!maskOK[k]) // at least one mask could not be verified
      return false;
  }

  // -> load the masks directly from the associated files and set them:
  ITKVTKImage *imask = NULL;
  for (std::size_t i = 0; i < maskImageFile.size(); i++)
  {
    imask = ITKVTKImage::LoadImageFromFile(maskImageFile[i], ITKVTKImage::ITKIO);
    this->m_MaskImages[i]->ConsumeImage(imask); // NOTE: can be NULL(=no mask)
  }

  return true;
}

bool REG23Model::WriteIntelligentMaskAndInfo(std::size_t index, ITKVTKImage *finalMask)
{
  if (index < 1 || index > m_FixedImageFileNames.size())
    return false;
  if (!finalMask)
    return false;

  std::string pth = itksys::SystemTools::GetFilenamePath(
      m_FixedImageFileNames[index - 1]);
  ora::EnsureStringEndsWith(pth, ora::DSEP);
  std::string infoFileName = pth + "reg23-masks.inf";
  ora::IniAccess infFile(infoFileName);
  std::string section = "", s = "", imgName = "";
  std::ostringstream os;
  int k = 1;
  // find new mask prefix:
  do
  {
    os.str("");
    os << "reg23-mask" << k;
    if (infFile.ReadString("masks", os.str(), "").length() <= 0)
    {
      section = os.str();
      os.str("");
      os << "mask" << k << ".mhd";
      imgName = os.str();
    }
    k++;
  } while (section.length() <= 0);
  infFile.WriteString("masks", section, "1");
  // write creation time stamp:
  ora::CurrentORADateTimeString(s);
  infFile.WriteString("creation", section, s);
  // write image file name:
  infFile.WriteString("images", section, imgName);
  // write image geometry:
  double spac[3], orig[3], dir[9];
  int sz[3];
  bool ret = false;
  TEMPLATE_CALL_COMP(finalMask->GetComponentType(),
      ret = finalMask->ExtractBasicImageGeometry, spac, sz, orig, dir)
  if (!ret)
    return false;
  os.str("");
  os << spac[0] << "," << spac[1];
  infFile.WriteString(section + "-fixed-image-geometry", "spacing", os.str());
  os.str("");
  os << sz[0] << "," << sz[1];
  infFile.WriteString(section + "-fixed-image-geometry", "size", os.str());
  os.str("");
  os << orig[0] << "," << orig[1] << "," << orig[2];
  infFile.WriteString(section + "-fixed-image-geometry", "origin", os.str());
  os.str("");
  os << dir[0] << "," << dir[1] << "," << dir[2] << ",";
  os << dir[3] << "," << dir[4] << "," << dir[5] << ",";
  os << dir[6] << "," << dir[7] << "," << dir[8];
  infFile.WriteString(section + "-fixed-image-geometry", "orientation", os.str());
  os.str("");
  os << m_SourcePositions[index - 1][0] << "," << m_SourcePositions[index - 1][1] << "," << m_SourcePositions[index - 1][2];
  infFile.WriteString(section + "-fixed-image-geometry", "focus", os.str());
  // write structures info:
  infFile.DeleteSection(section + "-structure-info"); // delete before
  std::vector<VTKStructure *> structs = m_Structures->GetStructures();
  long int timeStamp, fileSize;
  std::string s1, s2;
  for (std::size_t k = 0; k < structs.size(); k++)
  {
    if (structs[k]->GetFileName().length() > 0) // obviously ORA source
    {
      s1 = infFile.ReadString(section + "-structure-info", structs[k]->GetFileName() + "|timestamp", "");
      s2 = infFile.ReadString(section + "-structure-info", structs[k]->GetFileName() + "|sizestamp", "");
      if (s1.length() > 0 && s2.length() > 0) // already added
        continue;
      timeStamp = itksys::SystemTools::ModifiedTime(structs[k]->GetFileName().c_str());
      fileSize = itksys::SystemTools::FileLength(structs[k]->GetFileName().c_str());
      os.str("");
      os << structs[k]->GetFileName() + "|";
      infFile.WriteValue<long int>(section + "-structure-info", os.str() + "timestamp", timeStamp);
      infFile.WriteValue<long int>(section + "-structure-info", os.str() + "sizestamp", fileSize);
    }
  }
  // write structure rules:
  std::vector<std::string> *indents = m_Config->GetIndents("Structures");
  for (std::size_t i = 0; i < indents->size(); i++)
  {
    if (ora::ToLowerCaseF((*indents)[i].substr(0, 9)) == "structure")
    {
      infFile.WriteString(section + "-structure-rules", ora::ToLowerCaseF((*indents)[i]),
          ora::ToLowerCaseF(m_Config->ReadString("Structures", (*indents)[i], "")));
    }
  }
  delete indents;
  // write masking rules:
  indents = m_Config->GetIndents("Auto-Masking");
  os.str("");
  os << "mask" << index;
  std::string m = os.str();
  for (std::size_t i = 0; i < indents->size(); i++)
  {
    if (ora::ToLowerCaseF((*indents)[i].substr(0, m.length())) == m)
    {
      infFile.WriteString(section + "-mask-rules", ora::ToLowerCaseF((*indents)[i].substr(m.length(), 100)),
          ora::ToLowerCaseF(m_Config->ReadString("Auto-Masking", (*indents)[i], "")));
    }
  }
  delete indents;
  // structure properties for some special cases:
  // - cross-correlation sparse pre-registration: if a structure is defined as
  //   reference point, the structure center is stored here (in order to be able
  //   to do the pre-registration in intelligement mask mode without structure
  //   loading later)
  s = ToUpperCaseF(TrimF(m_Config->ReadString("Registration", "SparsePreRegistration", "")));
  if (s.substr(0, 17) == "CROSSCORRELATION(")
  {
    s = TrimF(m_Config->ReadString("Registration", "CrossCorrelationView" + StreamConvert(index), ""));
    std::vector<std::string> toks;
    Tokenize(s, toks, ",");
    if (toks.size() == 10)
    {
      VTKStructure *structur = m_Structures->FindStructure(TrimF(toks[9]));
      if (structur && structur->GetStructurePolyData()) // obviously a structure
      {
        // -> extract structure center and store it!
        double bounds[6], center[3];
        structur->GetStructurePolyData()->GetBounds(bounds);
        center[0] = bounds[0] + (bounds[1] - bounds[0]) * 0.5;
        center[1] = bounds[2] + (bounds[3] - bounds[2]) * 0.5;
        center[2] = bounds[4] + (bounds[5] - bounds[4]) * 0.5;
        os.str("");
        os << "(" << center[0] << "|" << center[1] << "|" << center[2] << ")";
        infFile.WriteString(section + "-stored-structure-props",
            "SparsePreRegistration.CrossCorrelation.StructureCenter." + structur->GetStructureUID(),
            os.str());
      }
    }
  }

  // try to save mask image:
  ret = false;
  TEMPLATE_CALL_COMP(finalMask->GetComponentType(),
      ret = finalMask->SaveImageAsORAMetaImage, pth + imgName, false, "")
  if (!ret)
  {
    infFile.DiscardChanges();
    return false;
  }

  infFile.Update();
  return true;
}

}
