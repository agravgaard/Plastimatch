//
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <time.h>

#include <itkOnePlusOneEvolutionaryOptimizer.h>
#include <itkAmoebaOptimizer.h>
#include <itkNormalVariateGenerator.h>
#include <itkEuler3DTransform.h>
#include <itkExtractImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkImageMaskSpatialObject.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkCommand.h>
#include <itkNormalizedMutualInformationHistogramImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkMeanReciprocalSquareDifferenceImageToImageMetric.h>
#include <itkMeanSquaresImageToImageMetric.h>

#include "oraCPUSiddonDRRFilter.h"
#include "oraImageBasedProjectionGeometry.h"
#include "oraMultiResolutionNWay2D3DRegistrationFramework.h"
#include "oraStochasticRankCorrelationImageToImageMetric.h"
#include "oraGradientDifferenceImageToImageMetric.h"
#include "oraIntensityTransferFunction.h"

#include "CommonToolsNew.hxx"

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
// update modulo (for visualization only)
int Modulo = 1;
// iteration counter (for visualization only)
int Iteration = 0;

/**
 * Print test usage information.
 **/
void PrintUsage(char *binname)
{
  std::string progname = "<application-binary-name>";

  if (binname)
    progname = std::string(binname);

  //FIXME: add options to allow changing the drr engine trough parameters
  std::cout << "\n";
  std::cout << "   *** N R E G 2 D 3 D   U S A G E ***\n";
  std::cout << "\n";
  std::cout << progname
      << " [options] <volume-file> <fixed-image1> <fixed-image2> ...\n";
  std::cout << "\n";
  std::cout << "  -h or --help ... print this short help\n";
  std::cout
      << "  -v or --verbose ... verbose messages to std::cout [optional]\n";
  std::cout
      << "  -pp{i} or --projection-props{i} ... i-th (zero-based) projection properties (in mm): <x-off> <y-off> <x-size> <y-size> <source-pos-x> <source-pos-y> <source-pos-z> [must be specified]\n";
  std::cout
      << "  -srcc{i} or --src-config{i} ... i-th (zero-based) metric configuration (SRC, stochastic rank correlation): <fhist-min> <fhist-max> <fbins> <mhist-min> <mhist-max> <mbins> <coverage> <rule-prefix> <rule-postfix> [1 metric config per fixed image must be specified; prefix and postfix are ( and ) in their simpliest form]\n";
  std::cout
      << "  -nmic{i} or --nmi-config{i} ... i-th (zero-based) metric configuration (NMI, norm. mutual info.): <fhist-min> <fhist-max> <fbins> <mhist-min> <mhist-max> <mbins> <pad-value> <rule-prefix> <rule-postfix> [1 metric config per fixed image must be specified; prefix and postfix are ( and ) in their simpliest form]\n";
  std::cout
      << "  -ncc{i} or --nc-config{i} ... i-th (zero-based) metric configuration (NC, norm. correlation): <subtract-mean> <rule-prefix> <rule-postfix> [1 metric config per fixed image must be specified; prefix and postfix are ( and ) in their simpliest form]\n";
  std::cout
      << "  -mrsdc{i} or --mrsd-config{i} ... i-th (zero-based) metric configuration (MRSD, mean reciprocal square diff.): <lambda> <rule-prefix> <rule-postfix> [1 metric config per fixed image must be specified; prefix and postfix are ( and ) in their simpliest form]\n";
  std::cout
      << "  -msc{i} or --ms-config{i} ... i-th (zero-based) metric configuration (MS, mean squares): <rule-prefix> <rule-postfix> [1 metric config per fixed image must be specified; prefix and postfix are ( and ) in their simpliest form]\n";
  std::cout
      << "  -gdc{i} or --gd-config{i} ... i-th (zero-based) metric configuration (GD, gradient difference): <rule-prefix> <rule-postfix> [1 metric config per fixed image must be specified; prefix and postfix are ( and ) in their simpliest form]\n";
  std::cout
      << "  -itf or --intensity-transfer-function{i} ... intensity transfer function: <itf-num-pairs> <itf-in1> <itf-out1> <itf-in2> <itf-out2> ... [must be specified]\n";
  std::cout
      << "  -itfff or --itf-from-file{i} ... intensity transfer function specification by file (same format as -itf-option): <file-name> [alternative to -itf-option]\n";
  std::cout
      << "  -ma{i} or --mask{i} ... i-th optional mask image specification: <mask-image-file> [same size as fixed image; same pixel spacing as fixed image; UCHAR; pixels>0 contribute]\n";
  std::cout
      << "  -cma{i} or --circular-mask{i} ... i-th optional circular mask specification: <mask-radius> <c-x> <c-y>\n";
  std::cout
      << "  -rma{i} or --rectangular-mask{i} ... i-th optional rectangular mask specification: <off-x> <off-y> <length> <height>\n";
  std::cout
      << "  -it or --initial-transform ... initial transform parameters (3 rotations in rad, 3 translations in mm) [default: 0 0 0 0 0 0]\n";
  std::cout
      << "  -l or --levels ... number of multi-resolution levels [default: 1]\n";
  std::cout
      << "  -os or --optimizer-scales ... optimizer scales (6 scales) [default: 57.3 57.3 57.3 1 1 1]\n";
  std::cout
      << "  -io or --image-output ... generate images that document the registration [optional]\n";
  std::cout
      << "  -evolc or --evol-config ... evolutionary optimizer configuration: max. iterations, initial radius, growth factor, shrink factor, epsilon (min Frobenius norm), seed [if seed < 0 -> random (non-deterministic); must be specified] \n";
  std::cout
      << "  -amoebac or --amoeba-config ... amoeba (Nelder-Mead) optimizer configuration: max. iterations, parameter tolerance, function value tolerance, initial-simplex-delta-0, ..., initial-simplex-delta-5 [if at least one delta <= 0 -> automatic initial simplex is applied; must be specified] \n";
  std::cout
      << "-or or --optimized-registration ... optimized registration (threaded composite metric evaluation if n>1) is switched ON; WARNING: This may be a rather experimental option!\n";
  std::cout << "\n";
  std::cout << "  NOTE: optional arguments are case-sensitive!\n";
  std::cout << "\n";
  std::cout << "  Authors: Philipp Steininger\n";
  std::cout << "           Jean-Luc Spielmann\n";
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
typedef double GradientPixelType;
typedef unsigned int RankPixelType;
typedef ora::CPUSiddonDRRFilter<VolumePixelType, DRRPixelType> DRRFilterType;
typedef DRRFilterType::Pointer DRRFilterPointer;
typedef DRRFilterType::OutputImageType DRRImageType;
typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
typedef itk::Image<RankPixelType, 2> Rank2DImageType;
typedef itk::Euler3DTransform<double> Transform3DType;
typedef ora::ImageBasedProjectionGeometry<DRRPixelType> ImageBasedDRRGeometryType;
typedef ora::ProjectionGeometry DRRGeometryType;
typedef itk::ImageFileWriter<DRRImageType> DRRWriterType;
typedef itk::ImageFileWriter<DRR2DImageType> DRR2DWriterType;
typedef itk::ExtractImageFilter<DRRImageType, DRR2DImageType> ExtractorType;
typedef ora::MultiResolutionNWay2D3DRegistrationFramework<DRR2DImageType,
    DRR2DImageType, VolumeImageType> RegistrationType;
typedef RegistrationType::MetricType RegMetricType;
typedef DRRImageType::RegionType FixedImageRegionType;
typedef itk::SingleValuedNonLinearOptimizer BaseOptimizerType;
typedef itk::OnePlusOneEvolutionaryOptimizer EvolOptimizerType;
typedef itk::AmoebaOptimizer AmoebaOptimizerType;
typedef itk::ImageToImageMetric<DRR2DImageType, DRR2DImageType> BaseMetricType;
typedef ora::StochasticRankCorrelationImageToImageMetric<DRR2DImageType,
    DRR2DImageType, Rank2DImageType> SRCMetricType;
typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<
    DRR2DImageType, DRR2DImageType> NMIMetricType;
typedef itk::NormalizedCorrelationImageToImageMetric<DRR2DImageType,
    DRR2DImageType> NCMetricType;
typedef itk::MeanReciprocalSquareDifferenceImageToImageMetric<DRR2DImageType,
    DRR2DImageType> MRSDMetricType;
typedef itk::MeanSquaresImageToImageMetric<DRR2DImageType, DRR2DImageType>
    MSMetricType;
typedef ora::GradientDifferenceImageToImageMetric<DRR2DImageType,
    DRR2DImageType, GradientPixelType> GDMetricType;
typedef RegistrationType::MaskImageType MaskImageType;
typedef itk::ImageRegionIterator<MaskImageType> MaskIteratorType;
typedef itk::ImageFileWriter<MaskImageType> MaskWriterType;
typedef itk::CStyleCommand CommandType;

/** Helper struct for generic metric config. **/
typedef struct MetricConfigStruct
{
  std::string id;
  std::string ruleprefix;
  std::string rulepostfix;

  MetricConfigStruct()
  {
    id = "MetricConfig";
    ruleprefix = "";
    rulepostfix = "";
  }
} MetricConfig;

/** Helper struct for SRC metric config. **/
typedef struct SRCMetricConfigStruct :
    MetricConfig
{
  double fhistmin;
  double fhistmax;
  int fhistbins;
  double mhistmin;
  double mhistmax;
  int mhistbins;
  double samplecoverage;

  SRCMetricConfigStruct() :
    MetricConfig()
  {
    id = "SRCMetricConfig";
  }
} SRCMetricConfig;

/** Helper struct for NMI metric config. **/
typedef struct NMIMetricConfigStruct :
    MetricConfig
{
  double fhistmin;
  double fhistmax;
  int fhistbins;
  double mhistmin;
  double mhistmax;
  int mhistbins;
  double padvalue;

  NMIMetricConfigStruct() :
    MetricConfig()
  {
    id = "NMIMetricConfig";
  }
} NMIMetricConfig;

/** Helper struct for NC metric config. **/
typedef struct NCMetricConfigStruct :
    MetricConfig
{
  bool subtractmean;

  NCMetricConfigStruct() :
    MetricConfig()
  {
    id = "NCMetricConfig";
  }
} NCMetricConfig;

/** Helper struct for MRSD metric config. **/
typedef struct MRSDMetricConfigStruct :
    MetricConfig
{
  double lambda;

  MRSDMetricConfigStruct() :
    MetricConfig()
  {
    id = "MRSDMetricConfig";
  }
} MRSDMetricConfig;

/** Helper struct for MS metric config. **/
typedef struct MSMetricConfigStruct :
    MetricConfig
{
  MSMetricConfigStruct() :
    MetricConfig()
  {
    id = "MSMetricConfig";
  }
} MSMetricConfig;

/** Helper struct for GD metric config. **/
typedef struct GDMetricConfigStruct :
    MetricConfig
{
  GDMetricConfigStruct() :
    MetricConfig()
  {
    id = "GDMetricConfig";
  }
} GDMetricConfig;

/** Helper struct for generic optimizer config. **/
typedef struct OptimizerConfigStruct
{
  std::string id;

  OptimizerConfigStruct()
  {
    id = "OptimizerConfig";
  }
} OptimizerConfig;

/** Helper struct for EVOL optimizer config. **/
typedef struct EVOLOptimizerConfigStruct :
    OptimizerConfig
{
  int seed;
  int maxIter;
  double radius;
  double gfact;
  double sfact;
  double epsilon;

  EVOLOptimizerConfigStruct() :
    OptimizerConfig()
  {
    id = "EVOLOptimizerConfig";
    seed = time(NULL);
    maxIter = 200;
    radius = 1.01;
    gfact = 1.05;
    sfact = 0.98;
    epsilon = 0.1;
  }
} EVOLOptimizerConfig;

/** Helper struct for AMOEBA optimizer config. **/
typedef struct AMOEBAOptimizerConfigStruct :
    OptimizerConfig
{
  int maxiter;
  double partol;
  double functol;
  double deltas[6];
  AMOEBAOptimizerConfigStruct() :
    OptimizerConfig()
  {
    id = "AMOEBAOptimizerConfig";
    maxiter = 200;
    partol = 1e-6;
    functol = 1e-4;
    for (int i = 0; i < 6; i++)
      deltas[i] = 0; // <= 0 means auto simplex initialization!
  }
} AMOEBAOptimizerConfig;

/** Central 2D/3D registration framework. **/
RegistrationType::Pointer NReg = NULL;
std::vector<int> MetricIterationsVec;
std::vector<double> MetricValuesVec;

/** Convert transform parameters to user-readable string. **/
std::string MakeParametersHumanReadable(Transform3DType::ParametersType rpars,
    bool twoLines = false)
{
  std::ostringstream os;
  std::string sep = ", ";
  if (twoLines)
    sep = "\n";
  double rad2deg = 180. / M_PI;
  os << (rpars[0] * rad2deg) << " deg, " << (rpars[1] * rad2deg) << " deg, "
      << (rpars[2] * rad2deg) << " deg" << sep << (rpars[3]) << " mm, "
      << (rpars[4]) << " mm, " << (rpars[5]) << " mm";
  return os.str();
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
    int i = reg->GetDRREngine()->GetCurrentDRROutputIndex();
    VERBOSE(<< "     [DRR-engine origin: " <<
        reg->GetDRREngine()->GetProjectionGeometry(i)->GetDetectorOrigin() << "]\n")
    VERBOSE(<< "     [DRR-engine spacing: " <<
        reg->GetDRREngine()->GetProjectionGeometry(i)->GetDetectorPixelSpacing() << "]\n")
    VERBOSE(<< "     [DRR-engine size: " <<
        reg->GetDRREngine()->GetProjectionGeometry(i)->GetDetectorSize() << "]\n")
    VERBOSE(<< "     [Initial Parameters: " <<
		reg->GetInitialTransformParameters() << "]\n")
    Iteration = 0; // set back
  }
}

/** Optimizer iteration observer. **/
void OptimizerEvent(itk::Object *obj, const itk::EventObject &ev, void *cd)
{
  if (std::string(ev.GetEventName()) == "IterationEvent"
      || std::string(ev.GetEventName()) == "FunctionEvaluationIterationEvent")
  {
    OptimizerConfig *optConfig = (OptimizerConfig *) cd;

    Iteration++;

    unsigned int currIt = 0;
    double optvalue = 0;

    if (optConfig->id == "EVOLOptimizerConfig")
    {
      EvolOptimizerType *opt = (EvolOptimizerType *)NReg->GetOptimizer();
      currIt = opt->GetCurrentIteration();
      optvalue = opt->GetValue();
      double frob = opt->GetFrobeniusNorm();
      VERBOSE(<< "      " << currIt << "\t" << NReg->GetLastMetricValue() << " ["
        << optvalue << "/" << frob << "]\t" << NReg->GetLastMetricParameters() <<
        "\n")
    }
    else if (optConfig->id == "AMOEBAOptimizerConfig")
    {
      AmoebaOptimizerType *opt = (AmoebaOptimizerType *)NReg->GetOptimizer();
      currIt = Iteration;
      optvalue = opt->GetCachedValue(); // no re-computation!
      VERBOSE(<< "      " << currIt << "\t" << NReg->GetLastMetricValue() << " ["
        << optvalue << "]\t" << NReg->GetLastMetricParameters() << "\n")
    }
  }
}

/**
 * A simple NReg2D3D example application using various metrics and optimizers.
 *
 * Run the application with -h or --help option to get information on command
 * line arguments.
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @author jeanluc
 * @version 1.4
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
  BaseOptimizerType::ScalesType oscales(6);
  oscales[0] = 57.3;
  oscales[1] = 57.3;
  oscales[2] = 57.3;
  oscales[3] = 1.0;
  oscales[4] = 1.0;
  oscales[5] = 1.0;
  initialParameters.Fill(0);
  int levels = 1;
  OptimizerConfig *optConfig = NULL;
  bool optreg = false;
  bool doNotTerminate = false;

  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-v" || std::string(argv[i]) == "--verbose")
    {
      Verbose = true;
      last = i;
      continue;
    }
    if (std::string(argv[i]) == "--stay")
    {
      doNotTerminate = true;
      last = i;
      continue;
    }
    if (std::string(argv[i]) == "-or" || std::string(argv[i])
        == "--optimized-registration")
    {
      optreg = true;
      last = i;
      continue;
    }
    if (std::string(argv[i]) == "-io" || std::string(argv[i])
        == "--image-output")
    {
      ImageOutput = true;
      last = i;
      continue;
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
      last = i + 7;
      i += (last - i);
      continue;
    }
    // metric-configs are really processed below, this just a pre-processing:
    if (std::string(argv[i]).substr(0, 5) == "-srcc"
        || std::string(argv[i]).substr(0, 12) == "--src-config")
    {
      last = i + 9;
      i += (last - i);
      continue;
    }
    if (std::string(argv[i]).substr(0, 5) == "-nmic"
        || std::string(argv[i]).substr(0, 12) == "--nmi-config")
    {
      last = i + 9;
      i += (last - i);
      continue;
    }
    if (std::string(argv[i]).substr(0, 4) == "-ncc"
        || std::string(argv[i]).substr(0, 11) == "--nc-config")
    {
      last = i + 3;
      i += (last - i);
      continue;
    }
    if (std::string(argv[i]).substr(0, 6) == "-mrsdc"
        || std::string(argv[i]).substr(0, 13) == "--mrsd-config")
    {
      last = i + 3;
      i += (last - i);
      continue;
    }
    if (std::string(argv[i]).substr(0, 4) == "-msc"
        || std::string(argv[i]).substr(0, 11) == "--ms-config")
    {
      last = i + 2;
      i += (last - i);
      continue;
    }
    if (std::string(argv[i]).substr(0, 4) == "-gdc"
        || std::string(argv[i]).substr(0, 11) == "--gd-config")
    {
      last = i + 2;
      i += (last - i);
      continue;
    }
    // -itf{i} or --intensity-transfer-function{i} are really processed below, this just a
    // pre-processing for setting last:
    if (std::string(argv[i]).substr(0, 3) == "-itf"
        || std::string(argv[i]).substr(0, 29)
            == "--intensity-transfer-function")
    {
      i++;
      int numPairs = atoi(argv[i]);
      last = i + numPairs;
      i += numPairs;
      continue;
    }
    // -itfff{i} or --itf-from-file{i} are really processed below, this just a
    // pre-processing for setting last:
    if (std::string(argv[i]).substr(0, 5) == "-itfff"
        || std::string(argv[i]).substr(0, 15) == "--itf-from-file")
    {
      last = i + 1;
      i++;
      continue;
    }
    // -ma{i} or --mask{i} are really processed below, this just a
    // pre-processing for setting last:
    if (std::string(argv[i]).substr(0, 3) == "-ma"
        || std::string(argv[i]).substr(0, 6) == "--mask")
    {
      last = i + 1;
      i++;
      continue;
    }
    // -cma{i} or --circular-mask{i} are really processed below, this just a
    // pre-processing for setting last:
    if (std::string(argv[i]).substr(0, 4) == "-cma"
        || std::string(argv[i]).substr(0, 15) == "--circular-mask")
    {
      last = i + 3;
      i += 3;
      continue;
    }
    // -rma{i} or --rectangular-mask{i} are really processed below, this just a
    // pre-processing for setting last:
    if (std::string(argv[i]).substr(0, 4) == "-rma"
        || std::string(argv[i]).substr(0, 18) == "--rectangular-mask")
    {
      last = i + 4;
      i += 4;
      continue;
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
      i--;
      continue;
    }
    if (std::string(argv[i]) == "-l" || std::string(argv[i]) == "--levels")
    {
      i++;
      levels = atoi(argv[i]);
      last = i;
      continue;
    }
    if (std::string(argv[i]) == "-m" || std::string(argv[i]) == "--modulo")
    {
      i++;
      Modulo = atoi(argv[i]);
      last = i;
      continue;
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
      i--;
      continue;
    }
    if (std::string(argv[i]) == "-evolc" || std::string(argv[i])
        == "--evol-config")
    {
      EVOLOptimizerConfig *evol = new EVOLOptimizerConfig();
      optConfig = evol;
      last = i + 6;
      i++;
      evol->maxIter = atoi(argv[i]);
      i++;
      evol->radius = atof(argv[i]);
      i++;
      evol->gfact = atof(argv[i]);
      i++;
      evol->sfact = atof(argv[i]);
      i++;
      evol->epsilon = atof(argv[i]);
      i++;
      evol->seed = atoi(argv[i]);
      if (evol->seed < 0)
        evol->seed = time(NULL);
      continue;
    }
    if (std::string(argv[i]) == "-amoebac" || std::string(argv[i])
        == "--amoeba-config")
    {
      AMOEBAOptimizerConfig *amoeba = new AMOEBAOptimizerConfig();
      optConfig = amoeba;
      last = i + 9;
      i++;
      amoeba->maxiter = atoi(argv[i]);
      i++;
      amoeba->partol = atof(argv[i]);
      i++;
      amoeba->functol = atof(argv[i]);
      for (int d = 0; d < 6; d++)
      {
        i++;
        amoeba->deltas[d] = atof(argv[i]);
      }
      continue;
    }
  }
  if ((last + 3) > argc)
  {
    std::cout << "Obviously command line arguments are invalid.\n";
    std::cout << "Need volume and fixed image(s) as last arguments!\n";
    return EXIT_FAILURE;
  }
  if (!optConfig)
  {
    std::cout << "No optimizer was configured!\n";
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
  std::vector<ImageBasedDRRGeometryType::Pointer> projProps;
  std::vector<MetricConfig *> metricConfigs;
  std::vector<MaskConfigItem> maskConfigs;
  std::vector<RegistrationType::DRREngineType::MaskImagePointer> maskPointers;
  ITFPointer itf = NULL;
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
        ImageBasedDRRGeometryType::Pointer props = ImageBasedDRRGeometryType::New();
        double fs[3];
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
        props->SetSourcePosition(fs);
        projProps.push_back(props);
        j = j + 3;
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

    // search for according metric configuration:
    found = false;
    os.str("");
    os << fi;
    // format: <fhist-min> <fhist-max> <fbins> <mhist-min> <mhist-max>
    //   <mbins> <coverage>
    for (int j = 0; j < argc; j++)
    {
      if (std::string(argv[j]) == ("-srcc" + os.str()) || std::string(argv[j])
          == ("--src-config" + os.str()))
      {
        SRCMetricConfig *srcc = new SRCMetricConfig();
        j++;
        srcc->fhistmin = atof(argv[j]);
        j++;
        srcc->fhistmax = atof(argv[j]);
        j++;
        srcc->fhistbins = atoi(argv[j]);
        j++;
        srcc->mhistmin = atof(argv[j]);
        j++;
        srcc->mhistmax = atof(argv[j]);
        j++;
        srcc->mhistbins = atoi(argv[j]);
        j++;
        srcc->samplecoverage = atof(argv[j]);
        j++;
        srcc->ruleprefix = std::string(argv[j]);
        j++;
        srcc->rulepostfix = std::string(argv[j]);
        j++;
        metricConfigs.push_back(srcc);
        found = true;
        break;
      }
      else if (std::string(argv[j]) == ("-nmic" + os.str()) || std::string(
          argv[j]) == ("--nmi-config" + os.str()))
      {
        NMIMetricConfig *nmic = new NMIMetricConfig();
        j++;
        nmic->fhistmin = atof(argv[j]);
        j++;
        nmic->fhistmax = atof(argv[j]);
        j++;
        nmic->fhistbins = atoi(argv[j]);
        j++;
        nmic->mhistmin = atof(argv[j]);
        j++;
        nmic->mhistmax = atof(argv[j]);
        j++;
        nmic->mhistbins = atoi(argv[j]);
        j++;
        nmic->padvalue = atof(argv[j]);
        j++;
        nmic->ruleprefix = std::string(argv[j]);
        j++;
        nmic->rulepostfix = std::string(argv[j]);
        j++;
        metricConfigs.push_back(nmic);
        found = true;
        break;
      }
      else if (std::string(argv[j]) == ("-ncc" + os.str()) || std::string(
          argv[j]) == ("--nc-config" + os.str()))
      {
        NCMetricConfig *ncc = new NCMetricConfig();
        j++;
        ncc->subtractmean = atoi(argv[j]);
        j++;
        ncc->ruleprefix = std::string(argv[j]);
        j++;
        ncc->rulepostfix = std::string(argv[j]);
        j++;
        metricConfigs.push_back(ncc);
        found = true;
        break;
      }
      else if (std::string(argv[j]) == ("-mrsdc" + os.str()) || std::string(
          argv[j]) == ("--mrsd-config" + os.str()))
      {
        MRSDMetricConfig *mc = new MRSDMetricConfig();
        j++;
        mc->lambda = atof(argv[j]);
        j++;
        mc->ruleprefix = std::string(argv[j]);
        j++;
        mc->rulepostfix = std::string(argv[j]);
        j++;
        metricConfigs.push_back(mc);
        found = true;
        break;
      }
      else if (std::string(argv[j]) == ("-msc" + os.str()) || std::string(
          argv[j]) == ("--ms-config" + os.str()))
      {
        MSMetricConfig *mc = new MSMetricConfig();
        j++;
        mc->ruleprefix = std::string(argv[j]);
        j++;
        mc->rulepostfix = std::string(argv[j]);
        j++;
        metricConfigs.push_back(mc);
        found = true;
        break;
      }
      else if (std::string(argv[j]) == ("-gdc" + os.str()) || std::string(
          argv[j]) == ("--gd-config" + os.str()))
      {
        GDMetricConfig *gc = new GDMetricConfig();
        j++;
        gc->ruleprefix = std::string(argv[j]);
        j++;
        gc->rulepostfix = std::string(argv[j]);
        j++;
        metricConfigs.push_back(gc);
        found = true;
        break;
      }
    }
    if (!found)
    {
      std::cout << "Could not find metric configuration for " << fi
          << "-th fixed image!\n";
      return EXIT_FAILURE;
    }

    // search for according mask configuration (optional):
    found = false;
    os.str("");
    os << fi;
    // -cma, -rma or -ma
    for (int j = 0; j < argc; j++)
    {
      if (std::string(argv[j]) == ("-cma" + os.str()) || std::string(argv[j])
          == ("--circular-mask" + os.str()))
      {
        if (argc > (j + 3))
        {
          MaskConfigItem mc;
          mc.type = 1; // circular
          j++;
          mc.p1 = atof(argv[j]);
          j++;
          mc.x = atof(argv[j]);
          j++;
          mc.y = atof(argv[j]);
          maskConfigs.push_back(mc);
          found = true;
          break;
        }
        else
        {
          std::cout << "Circular mask configuration for fixed image " << fi
              << " is invalid (number of parameters)!\n";
          return EXIT_FAILURE;
        }
      }
      if (std::string(argv[j]) == ("-rma" + os.str()) || std::string(argv[j])
          == ("--rectangular-mask" + os.str()))
      {
        if (argc > (j + 4))
        {
          MaskConfigItem mc;
          mc.type = 2; // rect
          j++;
          mc.x = atof(argv[j]);
          j++;
          mc.y = atof(argv[j]);
          j++;
          mc.p1 = atof(argv[j]);
          j++;
          mc.p2 = atof(argv[j]);
          maskConfigs.push_back(mc);
          found = true;
          break;
        }
        else
        {
          std::cout << "Rectangular mask configuration for fixed image " << fi
              << " is invalid (number of parameters)!\n";
          return EXIT_FAILURE;
        }
      }
      if (std::string(argv[j]) == ("-ma" + os.str()) || std::string(argv[j])
          == ("--mask" + os.str()))
      {
        if (argc > (j + 1))
        {
          MaskConfigItem mc;
          mc.type = 3; // file
          j++;
          mc.file = std::string(argv[j]);
          maskConfigs.push_back(mc);
          found = true;
          break;
        }
        else
        {
          std::cout << "Mask (file) configuration for fixed image " << fi
              << " is invalid (number of parameters)!\n";
          return EXIT_FAILURE;
        }
      }
    }
    if (!found)
    {
      MaskConfigItem mc; // type=0
      maskConfigs.push_back(mc); // mark this index as not masked
    }

    fi++; // next fixed image!
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

  // -itf, -itfff
  for (int j = 0; j < argc; j++)
  {
		if (std::string(argv[j]) == "-itf" || std::string(argv[j]) == "--intensity-transfer-function")
		{
      std::ostringstream itfos;
      j++;
      int numPairs = atoi(argv[j]);
      if (numPairs >= 2)
      {
        itfos << numPairs;
        int c = 0;
        int ec = numPairs * 2;
        while (c < ec && j < (argc - 1))
        {
          j++;
          itfos << " " << std::string(argv[j]);
          c++;
        }
      }
      else
      {
        std::cout << fi << "-th number of ITF-pairs is wrong!\n";
        return EXIT_FAILURE;
      }
      itf = ITFType::New();
      if (!ITFStringToITF(itfos.str(), itf))
      {
	    std::cout << fi
          << "-th ITF configuration appears to be invalid - check it!\n";
        return EXIT_FAILURE;
      }
      break;
    }
    if (std::string(argv[j]) == ("-itfff") || std::string(argv[j]) == ("--itf-from-file"))
    {
      j++;
      std::ifstream ifs(argv[j]);
      std::string itfstr((std::istreambuf_iterator<char>(ifs)),
      std::istreambuf_iterator<char>());
      itf = ITFType::New();
      if (!ITFStringToITF(itfstr, itf))
      {
        std::cout << fi
          << "-th ITF configuration appears to be invalid - check it!\n";
        return EXIT_FAILURE;
      }
      break;
    }
  }

  // set up registration
  VERBOSE(<< " > Configure registration framework\n")
  NReg = RegistrationType::New();
  DRRFilterPointer drrFilter = DRRFilterType::New();
  NReg->SetDRREngine(drrFilter.GetPointer());
  NReg->RemoveAllMetricFixedImageMappings();
  NReg->SetMoving3DVolume(movingImage);
  for (std::size_t fi = 0; fi < fixedImages.size(); fi++)
  {
    FixedImageRegionType fir;
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
      FixedImageRegionType::IndexType idx;
      idx.Fill(0);
      idx[0]
          = static_cast<FixedImageRegionType::IndexValueType> (fixedImageRegions[fi][0]
              / spac[0]);
      idx[1]
          = static_cast<FixedImageRegionType::IndexValueType> (fixedImageRegions[fi][1]
              / spac[1]);
      fir.SetIndex(idx);
      FixedImageRegionType::SizeType sz;
      sz.Fill(1);
      sz[0]
          = static_cast<FixedImageRegionType::SizeValueType> (fixedImageRegions[fi][2]
              / spac[0]);
      sz[1]
          = static_cast<FixedImageRegionType::SizeValueType> (fixedImageRegions[fi][3]
              / spac[1]);
      fir.SetSize(sz);
    }
    delete[] fixedImageRegions[fi];

    projProps[fi]->ExtractDetectorGeometryFromImageAndRegion(fixedImages[fi], fir);

    if (!projProps[fi]->IsGeometryValid())
    {
      std::cout << "ERROR: Projection properties appear to be invalid!\n";
      return EXIT_FAILURE;
    }

    // optional fixed image mask
    if (maskConfigs[fi].type >= 1 && maskConfigs[fi].type <= 3 )
    {
//    	template<typename MaskImageType, typename FixedImageType,
//    	    typename MetricFixedImageType>
//    	typename MaskImageType::Pointer GenerateGeometricMask(
//    	    MaskConfigItem &maskConfig, typename FixedImageType::Pointer fixedImage,
//    	    typename MetricFixedImageType::RegionType &freg)

    	RegistrationType::DRREngineType::MaskImagePointer fmask = NULL;
    	DRRImageType::RegionType freg = fixedImages[fi]->GetLargestPossibleRegion();
      fmask = GenerateGeometricMask<RegistrationType::DRREngineType::MaskImageType,
      		DRRImageType>(maskConfigs[fi], fixedImages[fi], freg);
      if (fmask)
      {
      	maskPointers.push_back(fmask);

        char buff[100];
        sprintf(buff, "MASK_%d.mhd", (int)fi);
        WriteImage<RegistrationType::DRREngineType::MaskImageType> (std::string(buff), fmask);

        if (!NReg->AddFixedImageAndProps(fixedImages[fi], fir, projProps[fi].GetPointer(), fmask))
				{
					std::cout << "ERROR adding " << fi << "-th fixed image and props!\n";
					return EXIT_FAILURE;
				}
      }
      else
      {
      	if (!NReg->AddFixedImageAndProps(fixedImages[fi], fir, projProps[fi].GetPointer()))
				{
					std::cout << "ERROR adding " << fi << "-th fixed image and props!\n";
					return EXIT_FAILURE;
				}
      	maskPointers.push_back(NULL);
        VERBOSE(<< "WARNING: Skipped " << fi << "-th mask - invalid config.\n")
      }
    }
    else // no mask
    {
      if (!NReg->AddFixedImageAndProps(fixedImages[fi], fir, projProps[fi].GetPointer()))
      {
        std::cout << "ERROR adding " << fi << "-th fixed image and props!\n";
        return EXIT_FAILURE;
      }
    }
  }
  if (NReg->GetNumberOfFixedImages() <= 0)
  {
    std::cout << "ERROR: No fixed image defined!\n";
    return EXIT_FAILURE;
  }

  Transform3DType::Pointer transform = Transform3DType::New();
  NReg->SetTransform(transform);
  NReg->SetInitialTransformParameters(initialParameters);

  NReg->SetNumberOfLevels(levels);
  NReg->SetUseAutoProjectionPropsAdjustment(true);
  NReg->SetUseMovingPyramidForFinalLevel(false);
  NReg->SetUseMovingPyramidForUnshrinkedLevels(false);
  NReg->SetUseFixedPyramidForFinalLevel(false);
  NReg->SetUseFixedPyramidForUnshrinkedLevels(false);

  RegMetricType::Pointer cm = NReg->GetMetric();
  std::string rule = ""; // -> minimization regardless of metric-type expected
  for (std::size_t i = 0; i < fixedImages.size(); i++)
  {
    BaseMetricType::Pointer metric = NULL;

    if (metricConfigs[i]->id == "SRCMetricConfig") // orig: -> MIN
    {
      SRCMetricType::Pointer m = SRCMetricType::New();
      metric = m;

      // forget about seeds - non-deterministic!
      SRCMetricType::SeedsType mrseeds;
      mrseeds.SetSize(SRCMetricType::ImageDimension);
      for (unsigned int d = 0; d < SRCMetricType::ImageDimension; d++)
        mrseeds[d] = 0;
      m->SetRandomSeeds(mrseeds);

      // clip always
      m->SetFixedHistogramClipAtEnds(true);
      m->SetMovingHistogramClipAtEnds(true);

      // configurable metric part:
      SRCMetricConfig *scfg =
          reinterpret_cast<SRCMetricConfig *> (metricConfigs[i]);
      m->SetFixedHistogramMinIntensity(scfg->fhistmin);
      m->SetFixedHistogramMaxIntensity(scfg->fhistmax);
      m->SetFixedNumberOfHistogramBins(scfg->fhistbins);
      m->SetMovingHistogramMinIntensity(scfg->mhistmin);
      m->SetMovingHistogramMaxIntensity(scfg->mhistmax);
      m->SetMovingNumberOfHistogramBins(scfg->mhistbins);
      m->SetSampleCoverage(scfg->samplecoverage);

      // default "no-overlap" behavior
      m->SetNoOverlapReactionMode(1);
      m->SetNoOverlapMetricValue(1000);

      // forget optimizations at the moment:
      m->SetUseHornTiedRanksCorrection(false);
      m->SetMovingZeroRanksContributeToMeasure(false);

      // no debugging
      m->SetExtractSampleDistribution(false);
    }
    else if (metricConfigs[i]->id == "NMIMetricConfig") // orig: -> MAX
    {
      NMIMetricType::Pointer m = NMIMetricType::New();
      metric = m;

      // configurable metric part:
      NMIMetricConfig *ncfg =
          reinterpret_cast<NMIMetricConfig *> (metricConfigs[i]);
      NMIMetricType::MeasurementVectorType lb;
      lb[0] = ncfg->fhistmin;
      lb[1] = ncfg->mhistmin;
      m->SetLowerBound(lb);
      NMIMetricType::MeasurementVectorType ub;
      ub[0] = ncfg->fhistmax;
      ub[1] = ncfg->mhistmax;
      m->SetUpperBound(ub);
      NMIMetricType::HistogramSizeType hsz;
      hsz[0] = ncfg->fhistbins;
      hsz[1] = ncfg->mhistbins;
      m->SetHistogramSize(hsz);
      m->SetPaddingValue(ncfg->padvalue);
      m->SetComputeGradient(false);
    }
    else if (metricConfigs[i]->id == "NCMetricConfig") // orig: -> MIN
    {
      NCMetricType::Pointer m = NCMetricType::New();
      metric = m;

      // configurable metric part:
      NCMetricConfig *ncfg =
          reinterpret_cast<NCMetricConfig *> (metricConfigs[i]);
      m->SetSubtractMean(ncfg->subtractmean);
      m->SetComputeGradient(false);
    }
    else if (metricConfigs[i]->id == "MRSDMetricConfig") // orig: -> MAX
    {
      MRSDMetricType::Pointer m = MRSDMetricType::New();
      metric = m;

      // configurable metric part:
      MRSDMetricConfig *mcfg =
          reinterpret_cast<MRSDMetricConfig *> (metricConfigs[i]);
      m->SetLambda(mcfg->lambda);
      m->SetComputeGradient(false);
    }
    else if (metricConfigs[i]->id == "MSMetricConfig") // orig: -> MIN
    {
      MSMetricType::Pointer m = MSMetricType::New();
      metric = m;

      // configurable metric part:
      /* MSMetricConfig *mcfg =
          reinterpret_cast<MSMetricConfig *> (metricConfigs[i]); */
      m->SetComputeGradient(false);
    }
    else if (metricConfigs[i]->id == "GDMetricConfig") // orig: -> MAX
    {
      GDMetricType::Pointer m = GDMetricType::New();
      metric = m;

      // configurable metric part:
      /* GDMetricConfig *gcfg =
          reinterpret_cast<GDMetricConfig *> (metricConfigs[i]); */
    }

    // finally add the metric
    std::ostringstream os;
    std::ostringstream os2;
    os.str("");
    os << "m" << i;
    os2.str("");
    os2 << "d" << i;
    cm->AddMetricInput(metric, os.str(), os2.str());
    NReg->AddMetricFixedImageMapping(metric, i);
    if (i == 0)
    {
      rule = metricConfigs[i]->ruleprefix + os.str()
          + metricConfigs[i]->rulepostfix;
    }
    else
    {
      rule += "+" + metricConfigs[i]->ruleprefix;
      rule += os.str() + metricConfigs[i]->rulepostfix;
    }
  }
  cm->SetValueCompositeRule(rule);
  VERBOSE(<< "METRIC VALUE COMPOSITION RULE: " << rule)

  CommandType::Pointer optcscmd = CommandType::New();
  optcscmd->SetClientData(optConfig);
  optcscmd->SetCallback(OptimizerEvent);

  BaseOptimizerType::Pointer opt = NULL;
  if (optConfig->id == "EVOLOptimizerConfig")
  {
    EVOLOptimizerConfig *ecfg = (EVOLOptimizerConfig *) optConfig;
    EvolOptimizerType::Pointer evol = EvolOptimizerType::New();
    typedef itk::Statistics::NormalVariateGenerator NVGeneratorType;
    NVGeneratorType::Pointer gen = NVGeneratorType::New();
    gen->Initialize(ecfg->seed);
    evol->SetNormalVariateGenerator(gen);
    evol->SetMaximumIteration(ecfg->maxIter);
    evol->Initialize(ecfg->radius, ecfg->gfact, ecfg->sfact);
    evol->SetEpsilon(ecfg->epsilon);
    evol->SetMaximize(false); // we always minimize!
    evol->AddObserver(itk::IterationEvent(), optcscmd);
    opt = evol;
  }
  else if (optConfig->id == "AMOEBAOptimizerConfig")
  {
    AMOEBAOptimizerConfig *acfg = (AMOEBAOptimizerConfig *) optConfig;
    AmoebaOptimizerType::Pointer amoeba = AmoebaOptimizerType::New();
    bool autoSimplex = false;
    AmoebaOptimizerType::ParametersType initialSimplexDeltas;
    initialSimplexDeltas.SetSize(6);
    for (int d = 0; d < 6; d++)
    {
      initialSimplexDeltas[d] = acfg->deltas[d];
      if (acfg->deltas[d] <= 0)
        autoSimplex = true;
    }
    if (autoSimplex)
    {
      amoeba->SetAutomaticInitialSimplex(true);
    }
    else
    {
      amoeba->SetAutomaticInitialSimplex(false);
      amoeba->SetInitialSimplexDelta(initialSimplexDeltas);
    }
    amoeba->SetMaximumNumberOfIterations(acfg->maxiter);
    amoeba->SetParametersConvergenceTolerance(acfg->partol);
    amoeba->SetFunctionConvergenceTolerance(acfg->functol);
    amoeba->SetMaximize(false); // we always minimize!
    amoeba->AddObserver(itk::FunctionEvaluationIterationEvent(), optcscmd);
    opt = amoeba;
  }
  opt->SetScales(oscales);
  NReg->SetOptimizer(opt);
  cm->SetUseOptimizedValueComputation(optreg);

  // add observers:
  CommandType::Pointer cscmd = CommandType::New();
  cscmd->SetClientData(NReg);
  cscmd->SetCallback(RegistrationEvent);
  NReg->AddObserver(itk::StartEvent(), cscmd);
  NReg->AddObserver(ora::StartMultiResolutionLevelEvent(), cscmd);
  NReg->AddObserver(ora::StartOptimizationEvent(), cscmd);
  NReg->AddObserver(itk::EndEvent(), cscmd);

  // do registration:
  try
  {
    NReg->Initialize();

    NReg->GetTransform()->SetParameters(NReg->GetInitialTransformParameters());
    for (unsigned int i = 0; i < NReg->GetNumberOfFixedImages(); i++)
    {
      std::ostringstream os;
      os.str("");
      os << "UNREGISTERED_PROJECTION_" << i << ".mhd";
      RegistrationType::DRR3DImagePointer drr3D =
          NReg->Compute3DTestProjection(i, NReg->GetNumberOfLevels() - 1);
      WriteImage<DRRImageType> (os.str(), drr3D);

      if (maskPointers.size() > i && maskPointers[i])
      {
      	os.str("");
      	os << "UNREGISTERED_PROJECTION_MASKED_" << i << ".mhd";
      	WriteMaskedImage<DRRImageType, RegistrationType::DRREngineType::MaskImageType>(os.str(),
      			drr3D, maskPointers[i]);
      }
    }

    vtkSmartPointer<vtkTimerLog> clock = vtkSmartPointer<vtkTimerLog>::New();
    clock->StartTimer();
    NReg->Update();
    clock->StopTimer();
    VERBOSE(<< "\n    REGISTRATION-TIME: [" << clock->GetElapsedTime() << "] s.\n")

    for (unsigned int i = 0; i < NReg->GetNumberOfFixedImages(); i++)
    {
      std::ostringstream os;
      os.str("");
      os << "REGISTERED_PROJECTION_" << i << ".mhd";
      RegistrationType::DRR3DImagePointer drr3D =
          NReg->Compute3DTestProjection(i, NReg->GetNumberOfLevels() - 1);
      WriteImage<DRRImageType> (os.str(), drr3D);

      if (maskPointers.size() > i && maskPointers[i])
      {
      	os.str("");
      	os << "REGISTERED_PROJECTION_MASKED_" << i << ".mhd";
      	WriteMaskedImage<DRRImageType, DRRFilterType::MaskImageType>(os.str(),
      			drr3D, maskPointers[i]);
      }
    }

    RegistrationType::TransformOutputConstPointer result = NReg->GetOutput();
    Transform3DType::ParametersType rpars = result->Get()->GetParameters();
    VERBOSE(<< "\n    RESULT-TRANSFORMATION: " << MakeParametersHumanReadable(rpars) << "\n\n")
  }
  catch (itk::ExceptionObject &e)
  {
    std::cout << "ERROR during registration: " << e << "\n";
  }

  if (maskPointers.size() > 0)
  {
  	for (std::size_t i = 0; i < maskPointers.size(); i++)
  		if (maskPointers[i])
  		{
  			//FIXME: Unregistering before setting to null causes crash
  			//maskPointers[i]->UnRegister();
  			maskPointers[i] = NULL;
  		}
  	maskPointers.clear();
  }

  // FIXME: add other clean-ups!!!

  NReg = NULL;
  drrFilter = NULL;
  itf = NULL;

  return EXIT_SUCCESS;
}
