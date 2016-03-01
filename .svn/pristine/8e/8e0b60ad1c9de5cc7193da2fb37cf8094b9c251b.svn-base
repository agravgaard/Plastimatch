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
#include <itkMultiThreader.h>
#include <itkSimpleFastMutexLock.h>
#include <itkNormalizedMutualInformationHistogramImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkMeanReciprocalSquareDifferenceImageToImageMetric.h>
#include <itkMeanSquaresImageToImageMetric.h>

#include "oraITKVTKDRRFilter.h"
#include "oraProjectionProperties.h"
#include "oraMultiResolutionNWay2D3DRegistrationMethod.h"
#include "oraStochasticRankCorrelationImageToImageMetric.h"
#include "oraGradientDifferenceImageToImageMetric.h"

#include "VTKWindowTools.hxx"
#include "CommonTools.hxx"

#include <vtkTimerLog.h>
#include <vtkRenderWindowCollection.h>
#include <vtkDoubleArray.h>

#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
#include <X11/Xlib.h>
#endif

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
// mask optimization
bool MaskOptimization = false;
// extreme mask optimization
bool XMaskOptimization = false;
// update modulo (for visualization only)
int Modulo = 1;
// iteration counter (for visualization only)
int Iteration = 0;
// best value visualization flag
bool BestValueVisualization = false;
// best value
double BestValue = 0;

/**
 * Print test usage information.
 **/
void PrintUsage(char *binname)
{
  std::string progname = "<application-binary-name>";

  if (binname)
    progname = std::string(binname);

  std::cout << "\n";
  std::cout << "   *** N R E G 2 D 3 D _ 2   U S A G E ***\n";
  std::cout << "\n";
  std::cout << progname
      << " [options] <volume-file> <fixed-image1> <fixed-image2> ...\n";
  std::cout << "\n";
  std::cout << "  -h or --help ... print this short help\n";
  std::cout
      << "  -v or --verbose ... verbose messages to std::cout [optional]\n";
  std::cout
      << "  -pp{i} or --projection-props{i} ... i-th (zero-based) projection properties (in mm): <x-off> <y-off> <x-size> <y-size> <source-pos-x> <source-pos-y> <source-pos-z> <step-size-mode>[must be specified]\n";
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
      << "  -itf{i} or --intensity-transfer-function{i} ... i-th intensity transfer function: <itf-num-pairs> <itf-in1> <itf-out1> <itf-in2> <itf-out2> ... [must be specified]\n";
  std::cout
      << "  -itfff{i} or --itf-from-file{i} ... i-th intensity transfer function specification by file (same format as -itf-option): <file-name> [alternative to -itf-option]\n";
  std::cout
      << "  -ma{i} or --mask{i} ... i-th optional mask image specification: <mask-image-file> [same size as fixed image; UCHAR; pixels>0 contribute]\n";
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
      << "  -mo or --mask-optimization ... if specified, DRR-masks according to fixed image masks are applied!\n";
  std::cout
      << "  -xmo or --extreme-mask-optimization ... if specified, DRR-masks according to fixed image masks and SRC configs are applied!\n";
  std::cout
      << "-or or --optimized-registration ... optimized registration (threaded composite metric evaluation if n>1) is switched ON\n";
  std::cout
      << "--stay ... do not quit application after registration, keep graph window until user closes (thus requires -gw defined!)\n";
  std::cout
      << "-rw{i} or --registration-window{i} ... i-th (zero-based) registration window (red-green-yellow overlay image) is displayed (ATTENTION: will slow down registration!!, if level==window -> no windowing): posX, posY, width, height, level1, window1, level2, window2 [default: not displayed]\n";
  std::cout
      << "-gw or --graph-window ... graph window (metric evoluation) is displayed(ATTENTION: will slow down registration!!): posX, posY, width, height [default: not displayed]\n";
  std::cout
      << "-m or --modulo ... update registration-windows each m-th iteration [default: 1]\n";
  std::cout
      << "-b or --best ... update registration-windows when metric value becomes better (invalidates modulo) [default: not set]\n";
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
typedef double GradientPixelType;
typedef unsigned int RankPixelType;
typedef ora::ITKVTKDRRFilter<VolumePixelType, DRRPixelType> DRRFilterType;
typedef itk::Image<DRRPixelType, 3> DRRImageType;
typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
typedef itk::Image<RankPixelType, 2> Rank2DImageType;
typedef itk::Euler3DTransform<double> Transform3DType;
typedef ora::ProjectionProperties<DRRPixelType> DRRPropsType;
typedef itk::ImageFileWriter<DRRImageType> DRRWriterType;
typedef itk::ImageFileWriter<DRR2DImageType> DRR2DWriterType;
typedef itk::ExtractImageFilter<DRRImageType, DRR2DImageType> ExtractorType;
typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
    DRR2DImageType, VolumeImageType> RegistrationType;
typedef RegistrationType::MetricType RegMetricType;
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
typedef itk::MultiThreader ThreaderType;
typedef ThreaderType::Pointer ThreaderPointer;

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

/** Info structure with some data for thread. **/
typedef struct
{
  RegistrationType *nreg;
} TInfoStruct;

/** Central 2D/3D registration framework. **/
RegistrationType::Pointer NReg = NULL;
/** RegWins information objects **/
std::vector<OverlayViewerObjects> RegWins;
/** Graph window (if configured). **/
vtkSmartPointer<vtkRenderWindow> GraphWin = NULL;
/** Tool tip for metric evolution **/
vtkSmartPointer<vtkTooltipItem> ParToolTip = NULL;
/** Data table for metric evolution. **/
vtkSmartPointer<vtkTable> MetricTable = NULL;
std::vector<int> MetricIterationsVec;
std::vector<double> MetricValuesVec;
/** Main mutex for rendering processes in main thread (communication tool). **/
itk::SimpleFastMutexLock MainMutex;

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

    Iteration = 0; // set back

    // extract the fixed images for current level:
    // (the fixed images are already resampled at this point)
    for (std::size_t i = 0; i < RegWins.size(); i++)
    {
      if (RegWins[i].renWin)
      {
        MainMutex.Lock();
        RegistrationType::BaseMetricPointer subMetric =
            reg->GetMetric()->GetIthMetricInput(i);
        typedef RegistrationType::BaseMetricType::FixedImageType FType;
        FType::ConstPointer fimage = subMetric->GetFixedImage();
        if (RegWins[i].colorMapper1->GetInput())
        {
          RegWins[i].colorMapper1->GetInput()->Delete();
        }
        vtkImageData *vfimage = ConnectVTKImageToITKImage<FType> (fimage);
        RegWins[i].colorMapper1->SetInput(vfimage);
        double srange[2];
        vfimage->GetScalarRange(srange);
        double vmin, vmax;
        if (RegWins[i].level1 == RegWins[i].window1) // no windowing
        {
          vmin = srange[0];
          vmax = srange[1];
        }
        else
        {
          vmin = RegWins[i].level1 - RegWins[i].window1 / 2.;
          vmax = RegWins[i].level1 + RegWins[i].window1 / 2.;
        }
        RegWins[i].colorMap1->SetTableRange(vmin, vmax);
        MainMutex.Unlock();
      }
    }
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
    BaseOptimizerType::ParametersType optpars(0);

    if (optConfig->id == "EVOLOptimizerConfig")
    {
      EvolOptimizerType *opt = (EvolOptimizerType *)NReg->GetOptimizer();
      currIt = opt->GetCurrentIteration();
      optvalue = opt->GetValue();
      double frob = opt->GetFrobeniusNorm();
      VERBOSE(<< "      " << currIt << "\t" << NReg->GetLastMetricValue() << " ["
        << optvalue << "/" << frob << "]\t" << NReg->GetLastMetricParameters() <<
        "\n")
      optpars = opt->GetCurrentPosition();
    }
    else if (optConfig->id == "AMOEBAOptimizerConfig")
    {
      AmoebaOptimizerType *opt = (AmoebaOptimizerType *)NReg->GetOptimizer();
      currIt = Iteration;
      optvalue = opt->GetCachedValue(); // no re-computation!
      VERBOSE(<< "      " << currIt << "\t" << NReg->GetLastMetricValue() << " ["
        << optvalue << "]\t" << NReg->GetLastMetricParameters() << "\n")
      optpars = opt->GetCachedCurrentPosition();
    }
    else
    {
      return;
    }

    // updated metric evolution graph if demanded
    if (GraphWin && MetricTable)
    {
      MetricIterationsVec.push_back(currIt);
      MetricValuesVec.push_back(optvalue);
      MetricTable->SetNumberOfRows(MetricIterationsVec.size());
      for (vtkIdType i = 0; i < MetricTable->GetNumberOfRows(); i++)
      {
        MetricTable->SetValue(i, 0, MetricIterationsVec[i]);
        MetricTable->SetValue(i, 1, MetricValuesVec[i]);
      }
      MetricTable->Modified();

      std::string resstring = MakeParametersHumanReadable(optpars, true);
      ParToolTip->SetText(resstring.c_str());
      ParToolTip->SetPosition(70, ParToolTip->GetScene()->GetSceneHeight() / 2.);

      if (MetricTable->GetNumberOfRows() >= 2)
      {
        MainMutex.Lock();
        GraphWin->MakeCurrent();
        GraphWin->Render();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
      wglMakeCurrent((HDC__*) GraphWin->GetGenericDisplayId(), NULL);
#else
        glXMakeCurrent((Display*) GraphWin->GetGenericDisplayId(), None, NULL);
#endif
        MainMutex.Unlock();
      }
    }

    // update existing registration windows:
    bool doUpdate = false;
    if (!BestValueVisualization)
    {
      doUpdate = (Iteration == 1 || Iteration % Modulo == 0);
    }
    else
    {
      // -> only if value becomes better (we always minimize):
      if (optvalue < BestValue)
      {
        doUpdate = true;
        BestValue = optvalue; // store
      }
    }
    vtkRenderWindow *rwin = NULL;
    if (doUpdate)
    {
      for (std::size_t i = 0; i < RegWins.size(); i++)
      {
        rwin = RegWins[i].renWin;
        if (rwin)
        {
          MainMutex.Lock();

          // extract current moving images:
          RegistrationType::BaseMetricPointer subMetric =
              NReg->GetMetric()->GetIthMetricInput(i);
          typedef RegistrationType::BaseMetricType::MovingImageType MType;
          MType::ConstPointer mimage = subMetric->GetMovingImage();
          if (RegWins[i].colorMapper2->GetInput())
          {
            RegWins[i].colorMapper2->GetInput()->Delete();
          }
          vtkImageData *vmimage = ConnectVTKImageToITKImage<MType> (mimage);
          RegWins[i].colorMapper2->SetInput(vmimage);
          double srange[2];
          vmimage->GetScalarRange(srange);
          double vmin, vmax;
          if (RegWins[i].level2 == RegWins[i].window2) // no windowing
          {
            vmin = srange[0];
            vmax = srange[1];
          }
          else
          {
            vmin = RegWins[i].level2 - RegWins[i].window2 / 2.;
            vmax = RegWins[i].level2 + RegWins[i].window2 / 2.;
          }
          RegWins[i].colorMap2->SetTableRange(vmin, vmax);

          // render
          rwin->MakeCurrent();
          rwin->Render();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
          wglMakeCurrent((HDC__*) rwin->GetGenericDisplayId(), NULL);
#else
          glXMakeCurrent((Display*) rwin->GetGenericDisplayId(), None, NULL);
#endif

          MainMutex.Unlock();
        }
      }
    }
  }
}

/** After SRC mask extraction event. **/
void SRCMaskEvent(itk::Object *obj, const itk::EventObject &ev, void *cd)
{
  if (std::string(ev.GetEventName()) == "AfterMaskCreation")
  {
    RegistrationType *reg = (RegistrationType *) cd;
    SRCMetricType *m = (SRCMetricType *) obj;
    if (m && reg)
    {
      // identify the firing metric:
      int idx = -1;
      for (std::size_t i = 0; i < reg->GetMetric()->GetNumberOfMetricInputs(); i++)
      {
        if (reg->GetMetric()->GetIthMetricInput(i) == m)
        {
          idx = i;
          break;
        }
      }

      // Mask optimization: the DRR pixels to be computed are explicitly marked
      // in order to reduce the number of GPU-operations.
      if (XMaskOptimization)
      {
        // modify DRR mask in order to generate only the pixels which we really
        // need (be fast!):
        SRCMetricType::MaskImageConstPointer stochasticMask =
            m->GetStochasticMask();
        if (stochasticMask)
        {
          DRRPropsType::Pointer props = reg->GetIthProjectionProps(idx);
          if (props)
          {
            // -> add a virtual 3rd dimension to 2D mask:
            typedef itk::CastImageFilter<SRCMetricType::MaskImageType,
                DRRPropsType::MaskImageType> MaskCastType;
            MaskCastType::Pointer caster = MaskCastType::New();
            caster->SetInput(stochasticMask);
            try
            {
              caster->Update();
              DRRPropsType::MaskImagePointer drrMask = caster->GetOutput();
              drrMask->DisconnectPipeline();

              // finally set the DRR pixel mask!
              std::vector<DRRPropsType::MaskImagePointer> masks;
              masks.push_back(drrMask);
              NReg->SetIthDRRMasks(idx, masks);
            }
            catch (itk::ExceptionObject &e)
            {
              std::cerr << "ERROR during mask casting: " << e << "\n";
            }
          }
        }
      }
    }
  }
}

/** Call of registration in a thread. **/
ITK_THREAD_RETURN_TYPE ThreadedRegistration(void *arg)
{
  itk::MultiThreader::ThreadInfoStruct *tis =
      reinterpret_cast<itk::MultiThreader::ThreadInfoStruct *> (arg);
  TInfoStruct *info = reinterpret_cast<TInfoStruct *> (tis->UserData);
  if (info && info->nreg)
  {
    RegistrationType *nreg = info->nreg;

    // do registration:
    try
    {
      nreg->Initialize();

      nreg->GetTransform()->SetParameters(nreg->GetInitialTransformParameters());
      vtkRenderWindow *rwin = NULL;
      for (unsigned int i = 0; i < nreg->GetNumberOfFixedImages(); i++)
      {
        std::ostringstream os;
        os.str("");
        os << "UNREGISTERED_PROJECTION_" << i << ".mhd";
        RegistrationType::DRR3DImagePointer drr3D =
            nreg->Compute3DTestProjection(i, nreg->GetNumberOfLevels() - 1);
        WriteImage<DRRImageType> (os.str(), drr3D);
      }

      vtkSmartPointer<vtkTimerLog> clock = vtkSmartPointer<vtkTimerLog>::New();
      clock->StartTimer();
      nreg->Update();
      clock->StopTimer();
      VERBOSE(<< "\n    REGISTRATION-TIME: [" << clock->GetElapsedTime() << "] s.\n")

      for (unsigned int i = 0; i < nreg->GetNumberOfFixedImages(); i++)
      {
        std::ostringstream os;
        os.str("");
        os << "REGISTERED_PROJECTION_" << i << ".mhd";
        RegistrationType::DRR3DImagePointer drr3D =
            nreg->Compute3DTestProjection(i, nreg->GetNumberOfLevels() - 1);
        WriteImage<DRRImageType> (os.str(), drr3D);

        rwin = RegWins[i].renWin;
        if (rwin)
        {
          MainMutex.Lock();

          // extract current moving images:
          typedef RegistrationType::DRR3DImageType DType;
          if (RegWins[i].colorMapper2->GetInput())
          {
            RegWins[i].colorMapper2->GetInput()->Delete();
          }
          vtkImageData *vmimage = ConnectVTKImageToITKImage<DType> (drr3D);
          RegWins[i].colorMapper2->SetInput(vmimage);
          double srange[2];
          vmimage->GetScalarRange(srange);
          double vmin, vmax;
          if (RegWins[i].level2 == RegWins[i].window2) // no windowing
          {
            vmin = srange[0];
            vmax = srange[1];
          }
          else
          {
            vmin = RegWins[i].level2 - RegWins[i].window2 / 2.;
            vmax = RegWins[i].level2 + RegWins[i].window2 / 2.;
          }
          RegWins[i].colorMap2->SetTableRange(vmin, vmax);

          // render
          rwin->MakeCurrent();
          rwin->Render();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
          wglMakeCurrent((HDC__*) rwin->GetGenericDisplayId(), NULL);
#else
          glXMakeCurrent((Display*) rwin->GetGenericDisplayId(), None, NULL);
#endif

          MainMutex.Unlock();
        }
      }

      RegistrationType::TransformOutputConstPointer result = nreg->GetOutput();
      Transform3DType::ParametersType rpars = result->Get()->GetParameters();
      VERBOSE(<< "\n    RESULT-TRANSFORMATION: " << MakeParametersHumanReadable(rpars) << "\n\n")
    }
    catch (itk::ExceptionObject &e)
    {
      std::cout << "ERROR during registration: " << e << "\n";
    }
  }

  return ITK_THREAD_RETURN_VALUE;
}

/**
 * A simple NReg2D3D example application using <br>
 * - Stochastic Rank Correlation metric for similarity measurement, <br>
 * - 1+1 Evolutionary Optimizer for cost function optimization. <br>
 *
 * Run the application with -h or --help option to get information on command
 * line arguments.
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.3
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
  std::vector<WindowGeometry *> rwGeoms;
  WindowGeometry gwGeom;
  bool doNotTerminate = false;

  RegWins.clear();

#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
  // initialize the Xlib module support for concurrent threads
  // NOTE: must be the first Xlib-call of an application!
  XInitThreads();
#endif

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
    if (std::string(argv[i]) == "-mo" || std::string(argv[i])
        == "--mask-optimization")
    {
      MaskOptimization = true;
      last = i;
      continue;
    }
    if (std::string(argv[i]) == "-xmo" || std::string(argv[i])
        == "--extreme-mask-optimization")
    {
      XMaskOptimization = true;
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
      last = i + 8;
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
    // -rw{i} or --registration-window{i} are really processed below, this is
    // just a pre-processing for setting last:
    if (std::string(argv[i]).substr(0, 3) == "-rw"
        || std::string(argv[i]).substr(0, 21) == "--registration-window")
    {
      last = i + 4;
      i += (last - i);
      continue;
    }
    if (std::string(argv[i]) == "-gw" || std::string(argv[i])
        == "--graph-window")
    {
      last = i + 4;
      i++;
      gwGeom.posX = atoi(argv[i]);
      i++;
      gwGeom.posY = atof(argv[i]);
      i++;
      gwGeom.width = atof(argv[i]);
      i++;
      gwGeom.height = atof(argv[i]);
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
    if (std::string(argv[i]) == "-b" || std::string(argv[i]) == "--best")
    {
      BestValueVisualization = true;
      BestValue = 100000; // SRC: init with big value
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
  std::vector<DRRPropsType::Pointer> projProps;
  std::vector<int> rayStepSizes;
  std::vector<MetricConfig *> metricConfigs;
  std::vector<MaskConfigItem> maskConfigs;
  std::vector<ITFPointer> itfs;
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

    // search for according ITF configuration:
    found = false;
    os.str("");
    os << fi;
    // -itf, -itfff
    for (int j = 0; j < argc; j++)
    {
      if (std::string(argv[j]) == ("-itf" + os.str()) || std::string(argv[j])
          == ("--intensity-transfer-function" + os.str()))
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
        ITFPointer itf = ITFPointer::New();
        if (!ITFStringToITF(itfos.str(), itf))
        {
          std::cout << fi
              << "-th ITF configuration appears to be invalid - check it!\n";
          return EXIT_FAILURE;
        }
        itfs.push_back(itf);
        found = true;
        break;
      }
      if (std::string(argv[j]) == ("-itfff" + os.str()) || std::string(argv[j])
          == ("--itf-from-file" + os.str()))
      {
        j++;
        std::ifstream ifs(argv[j]);
        std::string itfstr((std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>());
        ITFPointer itf = ITFPointer::New();
        if (!ITFStringToITF(itfstr, itf))
        {
          std::cout << fi
              << "-th ITF configuration appears to be invalid - check it!\n";
          return EXIT_FAILURE;
        }
        itfs.push_back(itf);
        found = true;
        break;
      }
    }
    if (!found)
    {
      std::cout << "Could not find intensity transfer function for " << fi
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

    // search for according registration window configuration (optional):
    found = false;
    os.str("");
    os << fi;
    // format: <posX> <posY> <width> <height>
    for (int j = 0; j < argc; j++)
    {
      if (std::string(argv[j]) == ("-rw" + os.str()) || std::string(argv[j])
          == ("--registration-window" + os.str()))
      {
        if (argc > (j + 4))
        {
          WindowGeometry *geom = new WindowGeometry;
          j++;
          geom->posX = atoi(argv[j]);
          j++;
          geom->posY = atoi(argv[j]);
          j++;
          geom->width = atoi(argv[j]);
          j++;
          geom->height = atoi(argv[j]);
          j++;
          geom->level1 = atof(argv[j]);
          j++;
          geom->window1 = atof(argv[j]);
          j++;
          geom->level2 = atof(argv[j]);
          j++;
          geom->window2 = atof(argv[j]);
          j++;
          rwGeoms.push_back(geom);
          found = true;
          break;
        }
        else
        {
          std::cout << "Registration window configuration for fixed image "
              << fi << " is invalid (number of parameters)!\n";
          return EXIT_FAILURE;
        }
      }
    }
    if (!found)
      rwGeoms.push_back(NULL); // mark this index as "not visualized"

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
  NReg = RegistrationType::New();

  NReg->RemoveAllMetricFixedImageMappings();
  NReg->SetMoving3DVolume(movingImage);
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
    projProps[fi]->SetITF(itfs[fi]);
    projProps[fi]->SetGeometryFromFixedImage(fixedImages[fi], fir);
    projProps[fi]->ComputeAndSetSamplingDistanceFromVolume(
        movingImage->GetSpacing(), rayStepSizes[fi]);
    if (!projProps[fi]->AreAllPropertiesValid())
    {
      std::cout << "ERROR: Projection properties appear to be invalid!\n";
      return EXIT_FAILURE;
    }
    if (!NReg->AddFixedImageAndProps(fixedImages[fi], fir, projProps[fi]))
    {
      std::cout << "ERROR adding " << fi << "-th fixed image and props!\n";
      return EXIT_FAILURE;
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
  NReg->SetAutoSamplingDistanceAdjustmentMode(rayStepSizes[0]);
  NReg->SetUseMovingPyramidForFinalLevel(false);
  NReg->SetUseMovingPyramidForUnshrinkedLevels(false);
  NReg->SetUseFixedPyramidForFinalLevel(false);
  NReg->SetUseFixedPyramidForUnshrinkedLevels(false);

  CommandType::Pointer metcscmd = CommandType::New();
  metcscmd->SetClientData(NReg);
  metcscmd->SetCallback(SRCMaskEvent);
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

      // add a mask extraction observer
      // (NOTE: Circular fixed image mask is also specified and generated in this
      // event handler!)
      m->AddObserver(ora::AfterMaskCreation(), metcscmd);
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

    // optional fixed image mask
    // FIXME: at the moment this will only work for a 1-resolution-level-registration!
    if (maskConfigs[i].type >= 1 && maskConfigs[i].type <= 3 && levels == 1)
    {
      SRCMetricType::MaskImagePointer fmask = NULL;
      RegistrationType::FixedImageRegionType freg =
          NReg->GetIthFixedImageRegion(i);
      fmask = GenerateGeometricMask<SRCMetricType::MaskImageType, DRRImageType,
          SRCMetricType::FixedImageType> (maskConfigs[i], fixedImages[i], freg);

      if (fmask)
      {
        char buff[100];
        sprintf(buff, "MASK_%d.mhd", (int) i);
        WriteImage<SRCMetricType::MaskImageType> (std::string(buff), fmask);

        typedef itk::ImageMaskSpatialObject<SRCMetricType::FixedImageDimension>
            MaskSpatialObjectType;
        MaskSpatialObjectType::Pointer fmspat = MaskSpatialObjectType::New();
        fmspat->SetImage(fmask);
        try
        {
          fmspat->Update();
        }
        catch (itk::ExceptionObject &e)
        {
          std::cerr << "ERROR during spatial object creation: " << e << "\n";
        }

        // set the fixed image mask to the metric:
        metric->SetFixedImageMask(fmspat);

        // accelerate DRR computation a bit by explicitly marking the pixels
        // to be computed:
        if (MaskOptimization)
        {
          // -> add a virtual 3rd dimension to 2D mask:
          typedef itk::CastImageFilter<SRCMetricType::MaskImageType,
              DRRPropsType::MaskImageType> MaskCastType;
          MaskCastType::Pointer caster = MaskCastType::New();
          caster->SetInput(fmask);
          try
          {
            caster->Update();
            DRRPropsType::MaskImagePointer drrMask = caster->GetOutput();
            drrMask->DisconnectPipeline();

            // finally set the DRR pixel mask!
            std::vector<DRRPropsType::MaskImagePointer> masks;
            masks.push_back(drrMask);
            NReg->SetIthDRRMasks(i, masks);
          }
          catch (itk::ExceptionObject &e)
          {
            std::cerr << "ERROR during mask casting: " << e << "\n";
          }
        }
      }
      else
      {
        VERBOSE(<< "WARNING: Skipped " << i << "-th mask - invalid config.\n")
      }
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

  // prepare / initialize registration windows:
  for (std::size_t j = 0; j < rwGeoms.size(); j++)
  {
    if (rwGeoms[j])
    {
      std::ostringstream snum;
      snum << j;
      OverlayViewerObjects regWinInfo = CreateOverlayViewer("View "
          + snum.str(), rwGeoms[j]->posX, rwGeoms[j]->posY, rwGeoms[j]->width,
          rwGeoms[j]->height);
      regWinInfo.level1 = rwGeoms[j]->level1; // store
      regWinInfo.window1 = rwGeoms[j]->window1;
      regWinInfo.level2 = rwGeoms[j]->level2;
      regWinInfo.window2 = rwGeoms[j]->window2;
      regWinInfo.renWin->Start();
      delete rwGeoms[j];
      RegWins.push_back(regWinInfo);
    }
    else
    {
      OverlayViewerObjects dummy;
      RegWins.push_back(dummy); // null-item
    }
  }

  // prepare / initialize graph window (metric evolution window):
  if (gwGeom.width > 0)
  {
    MetricTable = vtkSmartPointer<vtkTable>::New();
    vtkSmartPointer<vtkDoubleArray> iterations =
        vtkSmartPointer<vtkDoubleArray>::New();
    iterations->SetName("iterations");
    MetricTable->AddColumn(iterations);
    vtkSmartPointer<vtkDoubleArray> measures =
        vtkSmartPointer<vtkDoubleArray>::New();
    measures->SetName("metric");
    MetricTable->AddColumn(measures);
    MetricIterationsVec.clear();
    MetricValuesVec.clear();
    MetricTable->SetNumberOfRows(0);
    ParToolTip = vtkSmartPointer<vtkTooltipItem>::New();
    ParToolTip->SetText("");
    GraphWin = SetupVTKGraphWindow(MetricTable, gwGeom.posX, gwGeom.posY,
        gwGeom.width, gwGeom.height, 1, 0, 0, 0, 255, "  Iterations  ",
        "  Metric  ", ParToolTip);
    GraphWin->SetWindowName("Metric Evolution");
  }

  // start the registration thread:
  ThreaderPointer threader = ThreaderType::New();
  threader->SetNumberOfThreads(1);
  TInfoStruct info;
  info.nreg = NReg;
  threader->SetSingleMethod(ThreadedRegistration, &info);
  threader->SingleMethodExecute();

  if (doNotTerminate && GraphWin)
  {
    GraphWin->GetInteractor()->Start();
  }

  // clean-up (render windows are no smart-pointers here):
  vtkRenderWindow *rwin = NULL;
  for (std::size_t i = 0; i < RegWins.size(); i++)
  {
    rwin = RegWins[i].renWin;
    if (rwin)
      rwin->Delete();
  }
  // FIXME: add other clean-ups!!!

  NReg = NULL;

  return EXIT_SUCCESS;
}

