//
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <time.h>
#include <math.h>

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

#include <itksys/SystemTools.hxx>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkGradientDifferenceImageToImageMetric.h>
#include <itkPowellOptimizer.h>
#include <itkOnePlusOneEvolutionaryOptimizer.h>
#include <itkNormalVariateGenerator.h>
#include <itkEuler3DTransform.h>
#include <itkExtractImageFilter.h>
#include <itkCommand.h>

#include "oraITKVTKDRRFilter.h"
#include "oraProjectionProperties.h"
#include "oraMultiResolutionNWay2D3DRegistrationMethod.h"

#include <vtkTimerLog.h>

#define VERBOSE(x) \
{ \
  if (Verbose) \
  {\
    std::cout x; \
    std::cout.flush(); \
  }\
}

// verbose flag
bool Verbose = false;
// image output
bool ImageOutput = false;

/**
 * Print test usage information.
 **/
void PrintUsage(char *binname)
{
  std::string progname = "<application-binary-name>";

  if (binname)
    progname = std::string(binname);

  std::cout << "\n";
  std::cout << "   *** N R E G 2 D 3 D _ 1   U S A G E ***\n";
  std::cout << "\n";
  std::cout << progname
      << " [options] <volume-file> <fixed-image1> <fixed-image2> ...\n";
  std::cout << "\n";
  std::cout << "  -h or --help ... print this short help\n";
  std::cout
      << "  -v or --verbose ... verbose messages to std::cout [optional]\n";
  std::cout
      << "  -pp{i} or --projection-props{i} ... i-th (zero-based) projection properties (in mm): <x-off> <y-off> <x-size> <y-size> <source-pos-x> <source-pos-y> <source-pos-z> <step-size-mode> <itf-num-pairs> <itf-in1> <itf-out1> <itf-in2> <itf-out2> ... [must be specified]\n";
  std::cout
      << "  -it or --initial-transform ... initial transform parameters (3 rotations in rad, 3 translations in mm) [default: 0 0 0 0 0 0]\n";
  std::cout
      << "  -l or --levels ... number of multi-resolution levels [default: 1]\n";
  std::cout
      << "  -os or --optimizer-scales ... optimizer scales (6 scales) [default: 57.3 57.3 57.3 1 1 1]\n";
  std::cout
      << "  -io or --image-output ... generate images that document the registration [optional]\n";
  std::cout << "  -ec or --evolutionary-config ... evolutionary optimizer configuration: max. iterations, initial radius, growth factor, shrink factor, epsilon (min Frobenius norm) [default: 200 1.01 1.05 0.98 0.1] \n";
  std::cout << "  -es or --evolutionary-seed ... seed for deterministic behavior [optional]\n";
  std::cout << "\n";
  std::cout << "  NOTE: optional arguments are case-sensitive!\n";
  std::cout << "\n";
  std::cout << "  Author: Philipp Steininger\n";
  std::cout
      << "  Affiliation: Institute for Research and Development on Advanced Radiation Technologies (radART)\n";
  std::cout
      << "               Paracelsus Medical University (PMU), Salzburg, AUSTRIA\n";
  std::cout << "\n";
}

/** Essential typedefs. **/
typedef unsigned short VolumePixelType;
typedef itk::Image<VolumePixelType, 3> VolumeImageType;
typedef itk::ImageFileWriter<VolumeImageType> VolumeWriterType;
typedef float DRRPixelType;
typedef ora::ITKVTKDRRFilter<VolumePixelType, DRRPixelType> DRRFilterType;
typedef itk::Image<DRRPixelType, 3> DRRImageType;
typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
typedef itk::Euler3DTransform<double> Transform3DType;
typedef ora::ProjectionProperties<DRRPixelType> DRRPropsType;
typedef vtkSmartPointer<vtkColorTransferFunction> ITFPointer;
typedef itk::ImageFileWriter<DRRImageType> DRRWriterType;
typedef itk::ImageFileWriter<DRR2DImageType> DRR2DWriterType;
typedef itk::ExtractImageFilter<DRRImageType, DRR2DImageType> ExtractorType;
typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
    DRR2DImageType, VolumeImageType> RegistrationType;
typedef RegistrationType::MetricType RegMetricType;
//typedef itk::PowellOptimizer OptimizerType;
typedef itk::OnePlusOneEvolutionaryOptimizer OptimizerType;
typedef itk::GradientDifferenceImageToImageMetric<DRR2DImageType,
    DRR2DImageType> GDMetricType;
typedef RegistrationType::MaskImageType MaskImageType;
typedef itk::ImageRegionIterator<MaskImageType> MaskIteratorType;
typedef itk::ImageFileWriter<MaskImageType> MaskWriterType;

/** Read a specified image file. **/
template<typename T>
typename T::Pointer ReadImage(std::string fileName)
{
  typename T::Pointer image = NULL;

  typedef itk::ImageFileReader<T> ReaderType;

  typename ReaderType::Pointer r = ReaderType::New();
  r->SetFileName(fileName.c_str());
  try
  {
    image = r->GetOutput();
    r->Update();
    image->DisconnectPipeline();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "ERROR: Reading file '" << fileName << "'.\n";
    image = NULL;
  }

  return image;
}

/** Write a 3D DRR into a file. **/
void Write3DDRR(DRRImageType::Pointer drr, std::string fileName)
{
  DRRWriterType::Pointer w = DRRWriterType::New();
  w->SetInput(drr);
  w->SetFileName(fileName.c_str());
  w->Update();
}

/** Registration observers. **/
void RegistrationEvent(itk::Object *obj, const itk::EventObject &ev, void *cd)
{
  RegistrationType *reg = (RegistrationType *) cd;

  if (std::string(ev.GetEventName()) == "StartEvent")
  {
    VERBOSE(<< "\n    STARTING registration ...\n")
  }
  else if (std::string(ev.GetEventName()) == "EndEvent")
  {
    VERBOSE(<< "      - FINAL OPTIMUM: " <<
        reg->GetOptimizer()->GetCurrentPosition())
    VERBOSE(<< "        (after " << reg->GetNumberOfMetricEvaluationsAtLevel() <<
        " composite metric evaluations)\n")
    VERBOSE(<< "    ... FINISHING registration\n")
  }
  else if (std::string(ev.GetEventName()) == "StartMultiResolutionLevelEvent")
  {
    if (reg->GetCurrentLevel() > 0)
    {
      VERBOSE(<< "      - OPTIMUM (L" << reg->GetCurrentLevel() << "): " <<
          reg->GetOptimizer()->GetCurrentPosition())
      VERBOSE(<< "        (after " << reg->GetNumberOfMetricEvaluationsAtLevel() <<
          " composite metric evaluations)\n")
    }
    VERBOSE(<< "    > LEVEL " << (reg->GetCurrentLevel() + 1) << " of " <<
        reg->GetNumberOfLevels() << "\n")
  }
  else if (std::string(ev.GetEventName()) == "StartOptimizationEvent")
  {
    VERBOSE(<< "     [DRR-engine origin: " <<
        reg->GetDRREngine()->GetDRRPlaneOrigin() << "]\n")
    VERBOSE(<< "     [DRR-engine spacing: " <<
        reg->GetDRREngine()->GetDRRSpacing() << "]\n")
    VERBOSE(<< "     [DRR-engine size: " <<
        reg->GetDRREngine()->GetDRRSize() << "]\n")
    VERBOSE(<< "     [DRR-engine sampling distance: " <<
        reg->GetDRREngine()->GetSampleDistance() << "]\n")
    VERBOSE(<< "     [Initial Parameters: " <<
        reg->GetInitialTransformParameters() << "]\n")
  }
}

/** Optimizer iteration observer. **/
void OptimizerEvent(itk::Object *obj, const itk::EventObject &ev, void *cd)
{
  if (std::string(ev.GetEventName()) == "IterationEvent")
  {
    RegistrationType *reg = (RegistrationType *) cd;
    OptimizerType *opt = (OptimizerType *) reg->GetOptimizer();

    unsigned int currIt = opt->GetCurrentIteration();
//    unsigned int currLIt = opt->GetCurrentLineIteration();
//    VERBOSE(<< "      " << currIt << " (" << currLIt << ")\t"
//        << reg->GetLastMetricValue()
//        << "\t" << reg->GetLastMetricParameters() << "\n")
    double bestVal = opt->GetValue();
    double frob = opt->GetFrobeniusNorm();
    VERBOSE(<< "      " << currIt << "\t" << reg->GetLastMetricValue() << " ["
        << bestVal << "/" << frob << "]\t" << reg->GetLastMetricParameters() <<
        "\n")
  }
}

/**
 * A simple NReg2D3D example application using <br>
 * - Gradient Difference metric for similarity measurement, <br>
 * - 1+1 Evolutionary Optimizer for cost function optimization. <br>
 *
 * Run the application with -h or --help option to get information on command
 * line arguments.
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.1
 *
 * \ingroup NReg2D3DApplications
 */
int main(int argc, char *argv[])
{
  // arguments check
  if (argc < 3)
  {
    if (argc > 0)
      PrintUsage(argv[0]);
    else
      PrintUsage(NULL);
    return EXIT_FAILURE;
  }
  int last = 0;
  Transform3DType::ParametersType initialParameters(6);
  OptimizerType::ScalesType oscales(6);
  oscales[0] = 57.3;
  oscales[1] = 57.3;
  oscales[2] = 57.3;
  oscales[3] = 1.0;
  oscales[4] = 1.0;
  oscales[5] = 1.0;
  initialParameters.Fill(0);
  int levels = 1;
  int oseed = time(NULL);
  int maxIter = 200;
  double oradius = 1.01;
  double gfact = 1.05;
  double sfact = 0.98;
  double epsilon = 0.1;
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-v" || std::string(argv[i]) == "--verbose")
    {
      Verbose = true;
      last = i;
    }
    if (std::string(argv[i]) == "-io" || std::string(argv[i])
        == "--image-output")
    {
      ImageOutput = true;
      last = i;
    }
    if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
    {
      if (argc > 0)
        PrintUsage(argv[0]);
      else
        PrintUsage(NULL);
      last = i;
      return EXIT_FAILURE;
    }
    // -pp{i} or --projection-props{i} are really processed below, this is just
    // a pre-processing for setting last:
    if (std::string(argv[i]).substr(0, 3) == "-pp"
        || std::string(argv[i]).substr(0, 18) == "--projection-props")
    {
      last = i + 9;
      if (last < argc)
        last += atoi(argv[last]) * 2; // because we have ITF-PAIRS!
    }
    if (std::string(argv[i]) == "-it" || std::string(argv[i])
        == "--initial-transform")
    {
      last = i + 6;
      i++;
      int c = 0;
      while (i <= last)
      {
        initialParameters[c] = atof(argv[i]);
        c++;
        i++;
      }
    }
    if (std::string(argv[i]) == "-l" || std::string(argv[i]) == "--levels")
    {
      i++;
      levels = atoi(argv[i]);
      last = i;
    }
    if (std::string(argv[i]) == "-os" || std::string(argv[i])
        == "--optimizer-scales")
    {
      last = i + 6;
      i++;
      int c = 0;
      while (i <= last)
      {
        oscales[c] = atof(argv[i]);
        c++;
        i++;
      }
    }
    if (std::string(argv[i]) == "-ec" || std::string(argv[i])
        == "--evolutionary-config")
    {
      last = i + 5;
      i++;
      maxIter = atoi(argv[i]);
      i++;
      oradius = atof(argv[i]);
      i++;
      gfact = atof(argv[i]);
      i++;
      sfact = atof(argv[i]);
      i++;
      epsilon = atof(argv[i]);
      i++;
    }
    if (std::string(argv[i]) == "-es" || std::string(argv[i])
        == "--evolutionary-seed")
    {
      last = i + 1;
      i++;
      oseed = atoi(argv[i]);
      i++;
    }
  }
  if ((last + 3) > argc)
  {
    std::cout << "Obviously command line arguments are invalid.\n";
    std::cout << "Need volume and fixed image(s) as last arguments!\n";
    return EXIT_FAILURE;
  }

  // get image files and projection props:
  VERBOSE(<< " > Read input images and projection properties\n")
  std::string movingFile = std::string(argv[++last]);
  if (!itksys::SystemTools::FileExists(movingFile.c_str(), true))
  {
    std::cout << "ERROR: Moving image file '" << movingFile
        << "' does not exist!\n";
    return EXIT_FAILURE;
  }
  VolumeImageType::Pointer movingImage =
      ReadImage<VolumeImageType> (movingFile);
  if (!movingImage)
  {
    std::cout << "Could not read moving image!\n";
    return EXIT_FAILURE;
  }
  std::vector<std::string> fixedFiles;
  std::vector<double *> fixedImageRegions;
  std::vector<DRRPropsType::Pointer> projProps;
  std::vector<int> rayStepSizes;
  bool found;
  int fi = 0;
  for (int i = ++last; i < argc; i++)
  {
    // image:
    if (!itksys::SystemTools::FileExists(argv[i], true))
    {
      std::cout << "WARNING: Skip fixed image file '" << argv[i]
          << "' since it does not exist!\n";
      fi++;
      continue;
    }
    fixedFiles.push_back(std::string(argv[i]));
    // search for according projection properties:
    found = false;
    std::ostringstream os;
    os.str("");
    os << fi;
    // format: <x-off> <y-off> <x-size> <y-size> <source-pos-x> <source-pos-y>
    //   <source-pos-z> <itf-num-pairs> <itf-in1> <itf-out1> <itf-in2> <itf-out2> ...
    for (int j = 0; j < argc; j++)
    {
      if (std::string(argv[j]) == ("-pp" + os.str()) || std::string(argv[j])
          == ("--projection-props" + os.str()))
      {
        double *fir = new double[4];
        j++;
        // fixed image region: x-off, y-off, x-size, y-size
        int c = 0;
        while ((c + j) < argc && c < 4)
        {
          fir[c] = atof(argv[c + j]);
          c++;
        }
        if (c != 4)
        {
          std::cout << "Projection properties for fixed image " << fi
              << " are invalid (fixed image region)!\n";
          return EXIT_FAILURE;
        }
        fixedImageRegions.push_back(fir);
        j = j + 4; // set
        // source position: x, y, z
        c = 0;
        DRRPropsType::Pointer props = DRRPropsType::New();
        DRRPropsType::PointType fs;
        while ((c + j) < argc && c < 3)
        {
          fs[c] = atof(argv[c + j]);
          c++;
        }
        if (c != 3)
        {
          std::cout << "Projection properties for fixed image " << fi
              << " are invalid (source position)!\n";
          return EXIT_FAILURE;
        }
        props->SetSourceFocalSpotPosition(fs);
        projProps.push_back(props);
        j = j + 3;
        // ray step size computation mode:
        if (j < argc)
        {
          rayStepSizes.push_back(atoi(argv[j]));
          j++;
        }
        else
        {
          std::cout << "Projection properties for fixed image " << fi
              << " are invalid (ray step size computation mode)!\n";
          return EXIT_FAILURE;
        }
        // itf: num-pairs, pairs ...
        int numPairs = 0;
        if (j < argc && atoi(argv[j]) >= 2)
        {
          numPairs = atoi(argv[j]);
          ITFPointer itf = ITFPointer::New();
          j++;
          c = 0;
          while ((j + c + 1) < argc && c < (numPairs * 2))
          {
            itf->AddRGBPoint(atof(argv[j + c]), atof(argv[j + c + 1]), atof(
                argv[j + c + 1]), atof(argv[j + c + 1]));
            c += 2;
          }
          props->SetITF(itf);
        }
        else
        {
          std::cout << "Projection properties for fixed image " << fi
              << " are invalid (number of ITF-pairs)!\n";
          return EXIT_FAILURE;
        }
        if (c != (numPairs * 2))
        {
          std::cout << "Projection properties for fixed image " << fi
              << " are invalid (ITF-pairs)!\n";
          return EXIT_FAILURE;
        }
        found = true;
        break;
      }
    }
    if (!found)
    {
      std::cout << "Could not find projection properties for " << fi
          << "-th fixed image!\n";
      return EXIT_FAILURE;
    }
    fi++;
  }
  if (fixedFiles.size() < 1)
  {
    std::cout << "ERROR: No valid fixed image files configured!\n";
    return EXIT_FAILURE;
  }
  std::vector<DRRImageType::Pointer> fixedImages;
  for (std::size_t i = 0; i < fixedFiles.size(); i++)
  {
    DRRImageType::Pointer fixedImage = ReadImage<DRRImageType> (fixedFiles[i]);
    if (!fixedImage)
    {
      std::cout << "Could not read fixed image!\n";
      return EXIT_FAILURE;
    }
    fixedImages.push_back(fixedImage);
  }

  // set up registration
  VERBOSE(<< " > Configure registration framework\n")
  RegistrationType::Pointer nreg = RegistrationType::New();

  nreg->RemoveAllMetricFixedImageMappings();
  nreg->SetMoving3DVolume(movingImage);
  for (std::size_t fi = 0; fi < fixedImages.size(); fi++)
  {
    DRRPropsType::FixedImageRegionType fir;
    if (fixedImageRegions[fi][2] < 0 || fixedImageRegions[fi][3] < 0)
    {
      // -> largest possible region
      fir = fixedImages[fi]->GetLargestPossibleRegion();
    }
    else
    {
      if (fixedImageRegions[fi][0] < 0 || fixedImageRegions[fi][1] < 0
          || fixedImageRegions[fi][2] < 0 || fixedImageRegions[fi][3] < 0)
      {
        std::cout << "ERROR: fixed image region offset (or 1 size) < 0!\n";
        return EXIT_FAILURE;
      }
      DRRImageType::SpacingType spac = fixedImages[fi]->GetSpacing();
      DRRPropsType::FixedImageRegionType::IndexType idx;
      idx.Fill(0);
      idx[0]
          = static_cast<DRRPropsType::FixedImageRegionType::IndexValueType> (fixedImageRegions[fi][0]
              / spac[0]);
      idx[1]
          = static_cast<DRRPropsType::FixedImageRegionType::IndexValueType> (fixedImageRegions[fi][1]
              / spac[1]);
      fir.SetIndex(idx);
      DRRPropsType::FixedImageRegionType::SizeType sz;
      sz.Fill(1);
      sz[0]
          = static_cast<DRRPropsType::FixedImageRegionType::SizeValueType> (fixedImageRegions[fi][2]
              / spac[0]);
      sz[1]
          = static_cast<DRRPropsType::FixedImageRegionType::SizeValueType> (fixedImageRegions[fi][3]
              / spac[1]);
      fir.SetSize(sz);
    }
    delete[] fixedImageRegions[fi];
    projProps[fi]->SetGeometryFromFixedImage(fixedImages[fi], fir);
    projProps[fi]->ComputeAndSetSamplingDistanceFromVolume(
        movingImage->GetSpacing(), rayStepSizes[fi]);
    if (!projProps[fi]->AreAllPropertiesValid())
    {
      std::cout << "ERROR: Projection properties appear to be invalid!\n";
      return EXIT_FAILURE;
    }
    if (!nreg->AddFixedImageAndProps(fixedImages[fi], fir, projProps[fi]))
    {
      std::cout << "ERROR adding " << fi << "-th fixed image and props!\n";
      return EXIT_FAILURE;
    }
  }
  if (nreg->GetNumberOfFixedImages() <= 0)
  {
    std::cout << "ERROR: No fixed image defined!\n";
    return EXIT_FAILURE;
  }

  Transform3DType::Pointer transform = Transform3DType::New();
  nreg->SetTransform(transform);
  nreg->SetInitialTransformParameters(initialParameters);

  nreg->SetNumberOfLevels(levels);
  nreg->SetUseAutoProjectionPropsAdjustment(true);
  nreg->SetAutoSamplingDistanceAdjustmentMode(rayStepSizes[0]);
  nreg->SetUseMovingPyramidForFinalLevel(false);
  nreg->SetUseMovingPyramidForUnshrinkedLevels(false);
  nreg->SetUseFixedPyramidForFinalLevel(false);
  nreg->SetUseFixedPyramidForUnshrinkedLevels(false);

  RegMetricType::Pointer cm = nreg->GetMetric();
  std::string rule = "";
  for (std::size_t i = 0; i < fixedImages.size(); i++)
  {
    GDMetricType::Pointer m = GDMetricType::New();
    std::ostringstream os;
    std::ostringstream os2;
    os.str("");
    os << "m" << i;
    os2.str("");
    os2 << "d" << i;
    cm->AddMetricInput(m, os.str(), os2.str());
    nreg->AddMetricFixedImageMapping(m, i);
    if (i == 0)
      rule = os.str();
    else
      rule += "+" + os.str();
  }
  cm->SetValueCompositeRule(rule);

  OptimizerType::Pointer opt = OptimizerType::New();
  opt->SetScales(oscales);
  opt->SetMaximize(true);
  typedef itk::Statistics::NormalVariateGenerator NVGeneratorType;
  NVGeneratorType::Pointer gen = NVGeneratorType::New();
  gen->Initialize(oseed);
  opt->SetNormalVariateGenerator(gen);
  opt->SetMaximumIteration(maxIter);
  opt->Initialize(oradius, gfact, sfact);
  opt->SetEpsilon(epsilon);
  nreg->SetOptimizer(opt);
  cm->SetUseOptimizedValueComputation(false);

  // add observers:
  typedef itk::CStyleCommand CommandType;
  CommandType::Pointer cscmd = CommandType::New();
  cscmd->SetClientData(nreg);
  cscmd->SetCallback(RegistrationEvent);
  nreg->AddObserver(itk::StartEvent(), cscmd);
  nreg->AddObserver(ora::StartMultiResolutionLevelEvent(), cscmd);
  nreg->AddObserver(ora::StartOptimizationEvent(), cscmd);
  nreg->AddObserver(itk::EndEvent(), cscmd);
  CommandType::Pointer optcscmd = CommandType::New();
  optcscmd->SetClientData(nreg);
  optcscmd->SetCallback(OptimizerEvent);
  opt->AddObserver(itk::IterationEvent(), optcscmd);

  // do registration:
  try
  {
    nreg->Initialize();

    transform->SetParameters(nreg->GetInitialTransformParameters());
    for (std::size_t i = 0; i < fixedImages.size(); i++)
    {
      std::ostringstream os;
      os.str("");
      os << "UNREGISTERED_PROJECTION_" << i << ".mhd";
      RegistrationType::DRR3DImagePointer drr3D =
          nreg->Compute3DTestProjection(i, nreg->GetNumberOfLevels() - 1);
      Write3DDRR(drr3D, os.str());
    }

    vtkSmartPointer<vtkTimerLog> clock = vtkSmartPointer<vtkTimerLog>::New();
    clock->StartTimer();
    nreg->Update();
    clock->StopTimer();
    VERBOSE(<< "\n    REGISTRATION-TIME: " << clock->GetElapsedTime() << " s.\n")

    for (std::size_t i = 0; i < fixedImages.size(); i++)
    {
      std::ostringstream os;
      os.str("");
      os << "REGISTERED_PROJECTION_" << i << ".mhd";
      RegistrationType::DRR3DImagePointer drr3D =
          nreg->Compute3DTestProjection(i, nreg->GetNumberOfLevels() - 1);
      Write3DDRR(drr3D, os.str());
    }

    RegistrationType::TransformOutputConstPointer result = nreg->GetOutput();
    Transform3DType::ParametersType rpars = result->Get()->GetParameters();
    double rad2deg = 180. / M_PI;
    VERBOSE(<< "\n    RESULT-TRANSFORMATION: " << (rpars[0] * rad2deg) << " deg, "
         << (rpars[1] * rad2deg) << " deg, " << (rpars[2] * rad2deg) << " deg, "
         << (rpars[3]) << " mm, " << (rpars[4]) << " mm, " << (rpars[5]) << " mm\n\n")
  }
  catch (itk::ExceptionObject &e)
  {
    std::cout << "ERROR during registration: " << e << "\n";
    return EXIT_FAILURE;
  }

  nreg = NULL;

  return EXIT_SUCCESS;
}

