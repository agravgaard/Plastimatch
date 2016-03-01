//
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <time.h>
#include <math.h>

#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
#include <X11/Xlib.h>
#endif

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkOnePlusOneEvolutionaryOptimizer.h>
#include <itkSingleValuedNonLinearOptimizer.h>
#include <itkImageToImageMetric.h>
#include <itkNormalVariateGenerator.h>
#include <itkEuler3DTransform.h>
#include <itkCommand.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkMultiThreader.h>
#include <itkNormalizedMutualInformationHistogramImageToImageMetric.h>
#include <itkExtractImageFilter.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkMeanReciprocalSquareDifferenceImageToImageMetric.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkCorrelationCoefficientHistogramImageToImageMetric.h>
#include <itkMeanSquaresHistogramImageToImageMetric.h>
#include <itkRegularStepGradientDescentOptimizer.h>
#include <itkPowellOptimizer.h>
#include <itkImageMaskSpatialObject.h>

#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>
#include <vtkColorTransferFunction.h>

#include "oraDRRITFLikelihoodCostFunction.h"
#include "oraITKVTKDRRFilter.h"
#include "oraProjectionProperties.h"
#include "oraParametrizableIdentityTransform.h"
#include "oraStochasticRankCorrelationImageToImageMetric.h"
#include "oraGradientDifferenceImageToImageMetric.h"

#include "VTKWindowTools.hxx"

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
  std::cout << "   *** I T F O P T I M I Z E R   U S A G E ***\n";
  std::cout << "\n";
  std::cout << progname << " [options] <volume-file> <reference-image>\n";
  std::cout << "\n";
  std::cout << "  -h or --help ... print this short help\n";
  std::cout
      << "  -v or --verbose ... verbose messages to std::cout [optional]\n";
  std::cout
      << "  -io or --image-output ... generate images that document the registration [optional]\n";
  std::cout
      << "  -fio or --final-itf-out ... file-output of final ITF <filename> [optional]\n";
  std::cout
      << "  -xfio or --extended-final-itf-out ... file-output of final ITF with a specified number of (interpolated) evenly distributed supporting points <filename> <num-points> <start-point-intensity> <end-point-intensity> [optional]\n";
  std::cout
      << "  -pp or --projection-props ... projection properties (in mm): <x-off> <y-off> <x-size> <y-size> <source-pos-x> <source-pos-y> <source-pos-z> <step-size-mode>\n";
  std::cout
      << "  -itf or --intensity-transfer-function ... initial intensity transfer function for optimization (NOTE: all points of that function are basically optimized!): <itf-num-pairs> <itf-in1> <itf-out1> <itf-in2> <itf-out2> ...\n";
  std::cout
      << "  -itfff or -itf-from-file ... initial intensity transfer function for optimization from a file that has the same format as '-itf'-argument\n";
  std::cout
      << "  -ec or --evolutionary-config ... Evolutionary-optimizer configuration: max. iterations, initial radius, growth factor, shrink factor, epsilon (min Frobenius norm), seed (if 0 then non-deterministic)\n";
  std::cout
      << "  -rsgdc or --rsgd-config ... Regular-step-gradient-descent configuration: max. iterations, min. step size, max. step size, relaxation factor\n";
  std::cout
      << "  -powc or --pow-config ... Powell-optimizer configuration: max. iterations, max. line iterations, step-length, step-tolerance, value-tolerance\n";
  std::cout
      << "  -dimc or --drr-itf-metric-config ... DRR-ITF-metric configuration: min. intensity, max. intensity, number of bins, map-outside-intensities-to-zero-flag\n";
  std::cout
      << "  -nmimc or --nmi-metric-config ... Normalized-mutual-information-metric configuration: m-bins, f-bins, m-lower-bounds, f-lower-bounds, m-upper-bounds, f-upper-bounds\n";
  std::cout
      << "  -gdmc or --gd-metric-config ... Gradient-difference-metric configuration (no parameters needed)\n";
  std::cout
      << "  -msmc or --ms-metric-config ... Mean-squares-metric configuration (no parameters needed)\n";
  std::cout
      << "  -mshmc or --msh-metric-config ... Mean-squares-histogram-metric configuration (no parameters needed)\n";
  std::cout
      << "  -mrsdmc or --mrsd-metric-config ... Mean-reciprocal-square-differences-metric configuration: lambda (capture radius in terms of intensity)\n";
  std::cout
      << "  -mmimc or --mmi-metric-config ... Mattes-mutual-information-metric configuration: number of pixel samples (<=0 -> use all pixels), number of bins\n";
  std::cout
      << "  -ncmc or --nc-metric-config ... Normalized-correlation-metric configuration (no parameters needed)\n";
  std::cout
      << "  -ccmc or --cc-metric-config ... Correlation-coefficient-metric configuration (no parameters needed)\n";
  std::cout
      << "  -srcmc or --src-metric-config ... Stochastic-rank-correlation-metric configuration: f-bins, f-min, f-max, m-bins, m-min, m-max, sample-coverage, horn-flag\n";
  std::cout
      << "  -m or --mask ... Image mask (unsigned char, same size as reference image, 2D) defining the region of interest (pixels > 0): file name\n";
  std::cout
      << "  -os or --optimizer-scales ... the optimizer weightings (one for each ITF supporting point - number must match!) [default: 100 100 ...]\n";
  std::cout
      << "  -eos or --easy-optimizer scales ... easy optimizer weightings configuration: <weight-of-first-point> <weight-of-last-point> <weight-of-other-points>\n";
  std::cout
      << "  -t or --transform ... the 6 Euler 3D transform parameters that describe the spatial 3D transform that maps the volume to the reference image position [default: 0 0 0 0 0 0]\n";
  std::cout
      << "  -dr or --drr-rescale ... rescale DRR to specified intensity range before evaluating metric: min. intensity, max. intensity [default: no rescaling]\n";
  std::cout
      << "-iw or --itf-window ... ITF window (transfer function) is displayed (ATTENTION: will slow down optimization!!): posX, posY, width, height [default: not displayed]\n";
  std::cout
      << "--stay ... do not quit application after registration, keep graph window until user closes (thus requires -gw defined!)\n";
  std::cout
      << "-dw or --drr-window ... current DRR window is displayed (ATTENTION: will slow down optimization!!): posX, posY, width, height [default: not displayed]\n";
  std::cout
      << "-nw or --no-wrap ... do not wrap relative ITF supporting point weights - limit them! [default: wrapping]\n";
  std::cout
      << "-b or --best ... update visual windows when metric value becomes better [default: not set]\n";
  std::cout << "\n";
  std::cout << "  NOTE: optional arguments are case-sensitive!\n";
  std::cout << "\n";
  std::cout << "  Authors: Philipp Steininger\n";
  std::cout << "           Markus Neuner\n";
  std::cout
      << "  Affiliation: Institute for Research and Development on Advanced Radiation Technologies (radART)\n";
  std::cout
      << "               Paracelsus Medical University (PMU), Salzburg, AUSTRIA\n";
  std::cout << "\n";
}

typedef float XrayPixelType;
typedef itk::Image<XrayPixelType, 3> XrayImageType;
typedef itk::Image<XrayPixelType, 2> XrayImageType2D;
typedef unsigned char MaskPixelType;
typedef itk::Image<MaskPixelType, 2> MaskImageType2D;
typedef itk::ImageMaskSpatialObject<2> SpatialMaskType2D;
typedef itk::ExtractImageFilter<XrayImageType, XrayImageType2D> ReducerType;
typedef unsigned short VolumePixelType;
typedef itk::Image<VolumePixelType, 3> VolumeImageType;
typedef ora::ITKVTKDRRFilter<VolumePixelType, XrayPixelType> DRRFilterType;
typedef ora::ProjectionProperties<XrayPixelType> DRRPropsType;
typedef vtkSmartPointer<vtkColorTransferFunction> ITFPointer;
typedef ora::ParametrizableIdentityTransform<double, 2> TransformType;
typedef itk::LinearInterpolateImageFunction<XrayImageType2D, double>
    InterpolatorType;
typedef ora::DRRITFLikelihoodCostFunction<XrayImageType2D, XrayImageType2D>
    DRRITFMetricType;
typedef itk::MeanSquaresImageToImageMetric<XrayImageType2D, XrayImageType2D>
    MSMetricType;
typedef itk::MeanSquaresHistogramImageToImageMetric<XrayImageType2D,
    XrayImageType2D> MSHMetricType;
typedef itk::CorrelationCoefficientHistogramImageToImageMetric<XrayImageType2D,
    XrayImageType2D> CCMetricType;
typedef itk::MeanReciprocalSquareDifferenceImageToImageMetric<XrayImageType2D,
    XrayImageType2D> MRSDMetricType;
typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<
    XrayImageType2D, XrayImageType2D> NMIMetricType;
typedef itk::MattesMutualInformationImageToImageMetric<XrayImageType2D,
    XrayImageType2D> MMIMetricType;
typedef itk::NormalizedCorrelationImageToImageMetric<XrayImageType2D,
    XrayImageType2D> NCMetricType;
typedef ora::GradientDifferenceImageToImageMetric<XrayImageType2D,
    XrayImageType2D, float> GDMetricType;
typedef itk::Image<float, 2> RankImageType;
typedef ora::StochasticRankCorrelationImageToImageMetric<XrayImageType2D,
    XrayImageType2D, RankImageType> SRCMetricType;
typedef itk::OnePlusOneEvolutionaryOptimizer EvolutionaryOptimizerType;
typedef itk::RegularStepGradientDescentOptimizer RSGDOptimizerType;
typedef itk::PowellOptimizer PowOptimizerType;
typedef itk::Statistics::NormalVariateGenerator NVGeneratorType;
typedef itk::SingleValuedNonLinearOptimizer BaseOptimizerType;
typedef itk::ImageToImageMetric<XrayImageType2D, XrayImageType2D>
    BaseMetricType;
typedef itk::Euler3DTransform<double> Transform3DType;
typedef itk::CStyleCommand CommandType;
typedef itk::RescaleIntensityImageFilter<XrayImageType> RescaleFilterType;
typedef itk::MultiThreader ThreaderType;

typedef struct EvolutionaryConfigStruct
{
  int oseed;
  int maxIter;
  double oradius;
  double gfact;
  double sfact;
  double epsilon;

  EvolutionaryConfigStruct()
  {
    oseed = time(NULL);
    maxIter = 200;
    oradius = 1.01;
    gfact = 1.05;
    sfact = 0.98;
    epsilon = 0.1;
  }
} EvolutionaryConfig;

typedef struct RSGDConfigStruct
{
  int maxIter;
  double minStep;
  double maxStep;
  double relax;
  double gradTol;

  RSGDConfigStruct()
  {
    maxIter = 200;
    minStep = 0.01;
    maxStep = 0.2;
    relax = 0.5;
    gradTol = 1e-15;
  }
} RSGDConfig;

typedef struct PowConfigStruct
{
  int maxIter;
  int maxLineIter;
  double stepLen;
  double stepTol;
  double valueTol;

  PowConfigStruct()
  {
    maxIter = 20;
    maxLineIter = 10;
    stepLen = 0.1;
    stepTol = 1e-3;
    valueTol = 1e-6;
  }
} PowConfig;

typedef struct DRRITFMetricConfigStruct
{
  XrayPixelType min;
  XrayPixelType max;
  int bins;
  bool mapOutToZero;

  DRRITFMetricConfigStruct()
  {
    min = 0;
    max = 255;
    bins = 256;
    mapOutToZero = true;
  }
} DRRITFMetricConfig;

typedef struct NMIMetricConfigStruct
{
  int fbins;
  int mbins;
  XrayPixelType flb;
  XrayPixelType mlb;
  XrayPixelType fub;
  XrayPixelType mub;

  NMIMetricConfigStruct()
  {
    fbins = 256;
    mbins = 256;
    flb = 0;
    fub = 255;
    mlb = 0;
    mub = 255;
  }
} NMIMetricConfig;

typedef struct GDMetricConfigStruct
{
} GDMetricConfig;

typedef struct MSMetricConfigStruct
{
} MSMetricConfig;

typedef struct MRSDMetricConfigStruct
{
  double lambda;

  MRSDMetricConfigStruct()
  {
    lambda = 1.0;
  }
} MRSDMetricConfig;

typedef struct MMIMetricConfigStruct
{
  int numberOfSamples;
  int numberOfBins;

  MMIMetricConfigStruct()
  {
    numberOfSamples = 0;
    numberOfBins = 50;
  }
} MMIMetricConfig;

typedef struct NCMetricConfigStruct
{
} NCMetricConfig;

typedef struct CCMetricConfigStruct
{
} CCMetricConfig;

typedef struct MSHMetricConfigStruct
{
} MSHMetricConfig;

typedef struct SRCMetricConfigStruct
{
  int fbins;
  XrayPixelType fmin;
  XrayPixelType fmax;
  int mbins;
  XrayPixelType mmin;
  XrayPixelType mmax;
  double sampleCoverage; // 0..100
  bool horn;

  SRCMetricConfigStruct()
  {
    fbins = 256;
    fmin = 0;
    fmax = 255;
    mbins = 256;
    mmin = 0;
    mmax = 255;
    sampleCoverage = 10;
    horn = false;
  }
} SRCMetricConfig;

// intensity transfer function that is optimized
ITFPointer ITF = NULL;
std::vector<double> ITFSupportingPoints;
// projection properties
DRRPropsType::Pointer PProps = NULL;
// DRR engine
DRRFilterType::Pointer DRRFilter = NULL;
// optional Rescale filter
RescaleFilterType::Pointer Rescaler = NULL;
// dimension reducer (3D -> 2D)
ReducerType::Pointer Reducer = NULL;
// iteration
int Iteration = 0;
/** ITF window (if configured). **/
vtkSmartPointer<vtkRenderWindow> ITFWin = NULL;
/** Data table for metric evolution. **/
vtkSmartPointer<vtkTable> ITFTable = NULL;
/** Main mutex for rendering processes in main thread (communication tool). **/
itk::SimpleFastMutexLock MainMutex;
// DRR window
vtkSmartPointer<vtkRenderWindow> DRRViewer = NULL;
// mapper object
vtkSmartPointer<vtkDataSetMapper> DRRMapper = NULL;
// actor object
vtkSmartPointer<vtkActor> DRRActor = NULL;
// DRR visualization lookup table
vtkSmartPointer<vtkImageMapToColors> DRRLU = NULL;
// ITF-point weights wrap mode
bool ITFWrap = true;
// flag for "best"-mode
bool BestMode = false;
double BestValue = 0;
bool BestMinimize = false;
// final ITF out file
std::string FinalITFFile = "";
// extended final ITF out file
std::string XFinalITFFile = "";
int XFinalITFNumPoints = 0;
double XFinalITFMin = 0;
double XFinalITFMax = 0;

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

/** Write a specified image file. **/
template<typename T>
bool WriteImage(std::string fileName, typename T::ConstPointer image)
{
  if (!ImageOutput)
    return true;

  typedef itk::ImageFileWriter<T> WriterType;
  typename WriterType::Pointer w = WriterType::New();
  w->SetFileName(fileName.c_str());
  w->SetInput(image);
  try
  {
    w->Update();
    return true;
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "ERROR: Writing file '" << fileName << "'.\n";
    return false;
  }
}

/** Convert current ITF to optimizer position. **/
void ITFToRelativePositions(BaseOptimizerType::ParametersType &position)
{
  position.SetSize(ITFSupportingPoints.size());
  for (unsigned int i = 0; i < position.GetSize(); i++)
    position[i] = ITF->GetRedValue(ITFSupportingPoints[i]);
}

/**
 * Convert current position to ITF. NOTE: the interval [0;1] is supported; if
 * a position coordinate is outside this interval, it will be mapped into it;
 * a virtual circular interval!
 **/
void RelativePositionsToITF(BaseOptimizerType::ParametersType position)
{
  if (ITFWrap) // wrap around borders
  {
    for (unsigned int i = 0; i < position.GetSize(); i++)
    {
      while (position[i] > 1.0)
      {
        position[i] -= 1.0;
      }
      while (position[i] < 0.0)
      {
        position[i] += 1.0;
      }
    }
  }
  else // cut at borders
  {
    for (unsigned int i = 0; i < position.GetSize(); i++)
    {
      if (position[i] > 1.0)
        position[i] = 1.0;
      if (position[i] < 0.0)
        position[i] = 0.0;
    }
  }
  // take over into ITF: the easiest way is to renew the RGB-points:
  ITF->RemoveAllPoints();
  for (unsigned int i = 0; i < ITFSupportingPoints.size(); i++)
    ITF->AddRGBPoint(ITFSupportingPoints[i], position[i], position[i],
        position[i]);
}

/** Convert current ITF into VTK table representation (line graph). **/
void CurrentITFToVTKTable()
{
  double maxVal = ITFSupportingPoints[ITFSupportingPoints.size() - 1];
  for (vtkIdType i = 0; i < ITFTable->GetNumberOfRows(); i++)
  {
    ITFTable->SetValue(i, 0, ITFSupportingPoints[i]);
    ITFTable->SetValue(i, 1, ITF->GetRedValue(ITFSupportingPoints[i]) * maxVal);
  }
}

/** Apply current projection properties to DRR engine. **/
void ApplyProjectionPropertiesToDRREngine()
{
  // ITF
  DRRPropsType::TransferFunctionPointer itf =
      DRRFilter->GetInternalIntensityTransferFunction();
  itf->ShallowCopy(PProps->GetITF());

  DRRFilter->SetDRRPlaneOrientation(PProps->GetProjectionPlaneOrientation());
  DRRFilter->SetDRRPlaneOrigin(PProps->GetProjectionPlaneOrigin());
  DRRFilter->SetDRRSize(PProps->GetProjectionSize());
  DRRFilter->SetDRRSpacing(PProps->GetProjectionSpacing());
  DRRFilter->SetSampleDistance(PProps->GetSamplingDistance());
  DRRFilter->SetDRRMask(PProps->GetDRRMask());
  DRRFilter->SetSourceFocalSpotPosition(PProps->GetSourceFocalSpotPosition());
  DRRFilter->SetRescaleSlope(PProps->GetRescaleSlope());
  DRRFilter->SetRescaleIntercept(PProps->GetRescaleIntercept());
}

/**
 * Transformation changed event: transform represents the ITF supporting point
 * weights.
 **/
void TransformationChangedEvent(itk::Object *obj, const itk::EventObject &ev,
    void *cd)
{
  TransformType *transform = static_cast<TransformType *> (obj);

  VERBOSE(<< "    > investigate: " << transform->GetParameters() << "\n")
  RelativePositionsToITF(transform->GetParameters());
  ApplyProjectionPropertiesToDRREngine();
  DRRFilter->Update();
  if (Rescaler)
    Rescaler->Update();
  Reducer->Update();
}
/** Optimizer iteration event. **/
void OptimizerIterationEvent(itk::Object *obj, const itk::EventObject &ev,
    void *cd)
{
  BaseOptimizerType *opt = static_cast<BaseOptimizerType *> (obj);
  EvolutionaryOptimizerType::Pointer evol = EvolutionaryOptimizerType::New();
  RSGDOptimizerType::Pointer rsgd = RSGDOptimizerType::New();
  PowOptimizerType::Pointer powell = PowOptimizerType::New();

  double value = 0;
  double convergence = 0; // convergence measure
  if (std::string(opt->GetNameOfClass()) == std::string(evol->GetNameOfClass()))
  {
    evol = reinterpret_cast<EvolutionaryOptimizerType *> (opt);
    value = evol->GetCurrentCost();
    convergence = evol->GetFrobeniusNorm();
  }
  else if (std::string(opt->GetNameOfClass()) == std::string(
      rsgd->GetNameOfClass()))
  {
    rsgd = reinterpret_cast<RSGDOptimizerType *> (opt);
    value = rsgd->GetValue();
    convergence = rsgd->GetCurrentStepLength();
  }
  else if (std::string(opt->GetNameOfClass()) == std::string(
      powell->GetNameOfClass()))
  {
    powell = reinterpret_cast<PowOptimizerType *> (opt);
    value = powell->GetValue();
    convergence = 0;
  }

  bool doUpdate = true;
  if (BestMode)
  {
    if (BestMinimize)
    {
      if (value < BestValue)
        BestValue = value;
      else
        doUpdate = false;
    }
    else
    {
      if (value > BestValue)
        BestValue = value;
      else
        doUpdate = false;
    }
  }

  // update ITF graph if demanded
  if (doUpdate && ITFWin && ITFTable)
  {
    CurrentITFToVTKTable();
    ITFTable->Modified();

    MainMutex.Lock();
    ITFWin->MakeCurrent();
    ITFWin->Render();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
    wglMakeCurrent((HDC__*)ITFWin->GetGenericDisplayId(), NULL);
#else
    glXMakeCurrent((Display*) ITFWin->GetGenericDisplayId(), None, NULL);
#endif
    MainMutex.Unlock();
  }

  if (doUpdate && DRRViewer)
  {
    MainMutex.Lock();
    vtkImageData *vmm = NULL;
    if (!Rescaler)
      vmm = Create2DVTKImageFromITKImage<const XrayImageType> (
          DRRFilter->GetOutput());
    else
      vmm = Create2DVTKImageFromITKImage<const XrayImageType> (
          Rescaler->GetOutput());
    DRRLU->SetInput(vmm);
    vmm->Delete();
    // render
    DRRViewer->MakeCurrent();
    DRRViewer->Render();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
    wglMakeCurrent((HDC__*)DRRViewer->GetGenericDisplayId(), NULL);
#else
    glXMakeCurrent((Display*) DRRViewer->GetGenericDisplayId(), None, NULL);
#endif
    MainMutex.Unlock();
  }

  VERBOSE(<< Iteration << "\t" << value << "\t" << opt->GetCurrentPosition()
      << "  <<" << convergence << ">>\n")
  Iteration++;
}

/** Call of optimization in a thread. **/
ITK_THREAD_RETURN_TYPE ThreadedOptimization(void *arg)
{
  itk::MultiThreader::ThreadInfoStruct *tis =
      reinterpret_cast<itk::MultiThreader::ThreadInfoStruct *> (arg);
  BaseOptimizerType *optimizer =
      reinterpret_cast<BaseOptimizerType *> (tis->UserData);

  if (!optimizer)
    return ITK_THREAD_RETURN_VALUE;

  // extract initial DRR:
  VERBOSE(<< "\nINITIAL POSITION: " << optimizer->GetInitialPosition() << "\n")
  RelativePositionsToITF(optimizer->GetInitialPosition());
  ApplyProjectionPropertiesToDRREngine();
  try
  {
    DRRFilter->Update();
    const BaseMetricType *mm =
        reinterpret_cast<const BaseMetricType *> (optimizer->GetCostFunction());
    mm->GetMovingImage()->GetSource()->Update();
    WriteImage<XrayImageType2D> ("initial_drr.mhd", mm->GetMovingImage());

    // update ITF graph if demanded
    if (ITFWin && ITFTable)
    {
      CurrentITFToVTKTable();
      ITFTable->Modified();

      MainMutex.Lock();
      ITFWin->MakeCurrent();
      ITFWin->Render();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
      wglMakeCurrent((HDC__*)ITFWin->GetGenericDisplayId(), NULL);
#else
      glXMakeCurrent((Display*) ITFWin->GetGenericDisplayId(), None, NULL);
#endif
      MainMutex.Unlock();
    }

    if (DRRViewer)
    {
      MainMutex.Lock();
      vtkImageData *vmm = Create2DVTKImageFromITKImage<const XrayImageType2D> (
          mm->GetMovingImage());
      DRRLU->SetInput(vmm);
      vmm->Delete();
      // render
      DRRViewer->MakeCurrent();
      DRRViewer->Render();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
      wglMakeCurrent((HDC__*)DRRViewer->GetGenericDisplayId(), NULL);
#else
      glXMakeCurrent((Display*) DRRViewer->GetGenericDisplayId(), None, NULL);
#endif
      MainMutex.Unlock();
    }

  }
  catch (itk::ExceptionObject &e)
  {
    std::cout << "ERROR: Final DRR computation/writing failed!\n";
    return ITK_THREAD_RETURN_VALUE;
  }

  try
  {
    optimizer->StartOptimization();
  }
  catch (itk::ExceptionObject)
  {
    std::cout << "ERROR: Optimizer failed!\n";
    return ITK_THREAD_RETURN_VALUE;
  }

  // updated ITF graph if demanded
  if (ITFWin && ITFTable)
  {
    RelativePositionsToITF(optimizer->GetCurrentPosition());
    CurrentITFToVTKTable();
    ITFTable->Modified();

    MainMutex.Lock();
    ITFWin->MakeCurrent();
    ITFWin->Render();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
    wglMakeCurrent((HDC__*)ITFWin->GetGenericDisplayId(), NULL);
#else
    glXMakeCurrent((Display*) ITFWin->GetGenericDisplayId(), None, NULL);
#endif
    MainMutex.Unlock();
  }

  // extract optimum:
  VERBOSE(<< "\nFINAL OPTIMUM: " << optimizer->GetCurrentPosition() << "\n")
  RelativePositionsToITF(optimizer->GetCurrentPosition());
  ApplyProjectionPropertiesToDRREngine();
  try
  {
    DRRFilter->Update();
    const BaseMetricType *mm =
        reinterpret_cast<const BaseMetricType *> (optimizer->GetCostFunction());
    mm->GetMovingImage()->GetSource()->Update();
    WriteImage<XrayImageType2D> ("optimized_drr.mhd", mm->GetMovingImage());

    if (DRRViewer)
    {
      MainMutex.Lock();
      vtkImageData *vmm = Create2DVTKImageFromITKImage<const XrayImageType2D> (
          mm->GetMovingImage());
      DRRLU->SetInput(vmm);
      vmm->Delete();
      // render
      DRRViewer->MakeCurrent();
      DRRViewer->Render();
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
      wglMakeCurrent((HDC__*)DRRViewer->GetGenericDisplayId(), NULL);
#else
      glXMakeCurrent((Display*) DRRViewer->GetGenericDisplayId(), None, NULL);
#endif
      MainMutex.Unlock();
    }
  }
  catch (itk::ExceptionObject &e)
  {
    std::cout << "ERROR: Final DRR computation/writing failed!\n";
    return ITK_THREAD_RETURN_VALUE;
  }

  if (FinalITFFile.length() > 0 && ITF && ITFSupportingPoints.size() > 0)
  {
    std::ofstream ofs;
    ofs.open(FinalITFFile.c_str(), std::ios::out);
    if (ofs.is_open())
    {
      double maxVal = ITFSupportingPoints[ITFSupportingPoints.size() - 1];
      ofs << ITFSupportingPoints.size();
      for (unsigned int i = 0; i < ITFSupportingPoints.size(); i++)
      {
        ofs << " " << ITFSupportingPoints[i];
        ofs << " " << (ITF->GetRedValue(ITFSupportingPoints[i]) * maxVal);
      }
      ofs.close();
    }
  }

  if (XFinalITFFile.length() > 0 && ITF && ITFSupportingPoints.size() > 1
      && XFinalITFNumPoints > 1 && XFinalITFMax > XFinalITFMin && XFinalITFMin
      >= ITFSupportingPoints[0] && XFinalITFMax
      <= ITFSupportingPoints[ITFSupportingPoints.size() - 1])
  {
    std::ofstream ofs;
    ofs.open(XFinalITFFile.c_str(), std::ios::out);
    if (ofs.is_open())
    {
      double maxVal = ITFSupportingPoints[ITFSupportingPoints.size() - 1];
      double iv = (XFinalITFMax - XFinalITFMin) / (double) (XFinalITFNumPoints
          - 1);
      ofs << XFinalITFNumPoints;
      double x = XFinalITFMin - iv;
      for (int i = 0; i < XFinalITFNumPoints; i++)
      {
        x += iv;
        ofs << " " << x;
        double f = (ITF->GetRedValue(x) * maxVal / XFinalITFMax);
        if (f < 0.0)
          f = 0;
        if (f > 1.0)
          f = 1.0;
        ofs << " " << (f * XFinalITFMax);
      }
      ofs.close();
    }
  }

  return ITK_THREAD_RETURN_VALUE;
}

/** Convert ITF-string (number of pairs followed by pairs, space-sep.) to ITF **/
bool ITFStringToITF(std::string itfstring)
{
  ITF = NULL;
  std::vector<std::string> tokens;
  std::string::size_type lastPos = itfstring.find_first_not_of(" ", 0);
  std::string::size_type pos = itfstring.find_first_of(" ", lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
  {
    tokens.push_back(itfstring.substr(lastPos, pos - lastPos));
    lastPos = itfstring.find_first_not_of(" ", pos);
    pos = itfstring.find_first_of(" ", lastPos);
  }

  if (tokens.size() < 5) // at least 2 supporting points required
    return false;

  int numPairs = atoi(tokens[0].c_str());
  if ((numPairs * 2) != (int) (tokens.size() - 1))
    return false;

  double maxVal = -1;
  for (std::size_t i = 1; i < tokens.size(); i += 2)
  {
    if (atof(tokens[i].c_str()) > maxVal) // search max. intensity (x-axis)
      maxVal = atof(tokens[i].c_str());
  }
  ITF = ITFPointer::New();
  for (std::size_t i = 2; i < tokens.size(); i += 2)
  {
    double v = atof(tokens[i].c_str()) / maxVal; // normalize to [0;1]
    ITF->AddRGBPoint(atof(tokens[i - 1].c_str()), v, v, v);
  }

  return true;
}

/**
 * An iterative optimization approach that aims at finding an optimal ITF
 * (intensity transfer function) for DRR (digitally reconstructed radiograph)
 * rendering by comparing a reference image (e.g. X-ray) to the DRRs of a
 * volume (e.g. CT).
 *
 * Run the application with -h or --help option to get information on command
 * line arguments.
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @author markus <markus.neuner e_T pmu.ac.at>
 * @version 1.0
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
  double *fir = NULL;
  int rayStepSizeMode = 0;
  EvolutionaryConfig *evolConfig = NULL;
  RSGDConfig *rsgdConfig = NULL;
  PowConfig *powConfig = NULL;
  DRRITFMetricConfig *drritfConfig = NULL;
  NMIMetricConfig *nmiConfig = NULL;
  GDMetricConfig *gdConfig = NULL;
  MSMetricConfig *msConfig = NULL;
  MRSDMetricConfig *mrsdConfig = NULL;
  MMIMetricConfig *mmiConfig = NULL;
  MSHMetricConfig *mshConfig = NULL;
  CCMetricConfig *ccConfig = NULL;
  NCMetricConfig *ncConfig = NULL;
  SRCMetricConfig *srcConfig = NULL;
  BaseOptimizerType::ScalesType scales;
  scales.SetSize(0); // undefined
  Transform3DType::ParametersType transformPars;
  transformPars.SetSize(6);
  transformPars.fill(0); // default
  XrayPixelType minrescale = 0;
  XrayPixelType maxrescale = 0;
  bool doNotTerminate = false;
  WindowGeometry iwGeom;
  WindowGeometry dwGeom;
  std::string maskFile = "";
  double easyScalesFirst = -1;
  double easyScalesLast = -1;
  double easyScalesOther = -1;

  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-v" || std::string(argv[i]) == "--verbose")
    {
      Verbose = true;
      last = i;
      continue;
    }
    if (std::string(argv[i]) == "-b" || std::string(argv[i]) == "--best")
    {
      BestMode = true;
      last = i;
      continue;
    }
    if (std::string(argv[i]) == "--stay")
    {
      doNotTerminate = true;
      last = i;
      continue;
    }
    if (std::string(argv[i]) == "-nw" || std::string(argv[i]) == "--no-wrap")
    {
      ITFWrap = false;
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
    if (std::string(argv[i]) == "-fio" || std::string(argv[i])
        == "--final-itf-out")
    {
      last = i + 1;
      i++;
      FinalITFFile = std::string(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-xfio" || std::string(argv[i])
        == "--extended-final-itf-out")
    {
      last = i + 4;
      i++;
      XFinalITFFile = std::string(argv[i]);
      i++;
      XFinalITFNumPoints = atoi(argv[i]);
      i++;
      XFinalITFMin = atof(argv[i]);
      i++;
      XFinalITFMax = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-pp" || std::string(argv[i])
        == "--projection-props")
    {
      last = i + 8;
      fir = new double[4];
      i++;
      // fixed image region: x-off, y-off, x-size, y-size
      int c = 0;
      while ((c + i) < argc && c < 4)
      {
        fir[c] = atof(argv[c + i]);
        c++;
      }
      if (c != 4)
      {
        std::cout
            << "Projection properties are invalid (fixed image region)!\n";
        return EXIT_FAILURE;
      }
      i = i + 4; // set
      // source position: x, y, z
      c = 0;
      PProps = DRRPropsType::New();
      DRRPropsType::PointType fs;
      while ((c + i) < argc && c < 3)
      {
        fs[c] = atof(argv[c + i]);
        c++;
      }
      if (c != 3)
      {
        std::cout << "Projection properties are invalid (source position)!\n";
        return EXIT_FAILURE;
      }
      PProps->SetSourceFocalSpotPosition(fs);
      i = i + 3;
      // ray step size computation mode:
      if (i < argc)
      {
        rayStepSizeMode = atoi(argv[i]);
      }
      else
      {
        std::cout
            << "Projection properties are invalid (ray step size computation mode)!\n";
        return EXIT_FAILURE;
      }
      continue;
    }
    if (std::string(argv[i]) == "-itf" || std::string(argv[i])
        == "--intensity-transfer-function")
    {
      std::ostringstream itfos;
      i++;
      int numPairs = atoi(argv[i]);
      if (numPairs >= 2)
      {
        itfos << numPairs;
        int c = 0;
        int ec = numPairs * 2;
        while (c < ec && i < (argc - 1))
        {
          i++;
          itfos << " " << std::string(argv[i]);
          c++;
        }
        last = i;
      }
      else
      {
        std::cout << "Number of ITF-pairs is wrong!\n";
        return EXIT_FAILURE;
      }
      if (!ITFStringToITF(itfos.str()))
      {
        std::cout << "ITF configuration appears to be invalid - check it!\n";
        return EXIT_FAILURE;
      }
      continue;
    }
    if (std::string(argv[i]) == "-itfff" || std::string(argv[i])
        == "--itf-from-file")
    {
      last = i + 1;
      i++;
      std::ifstream ifs(argv[i]);
      std::string itfstr((std::istreambuf_iterator<char>(ifs)),
          std::istreambuf_iterator<char>());
      if (!ITFStringToITF(itfstr))
      {
        std::cout << "ITF configuration appears to be invalid - check it!\n";
        return EXIT_FAILURE;
      }
      continue;
    }
    if (std::string(argv[i]) == "-m" || std::string(argv[i]) == "--mask")
    {
      last = i + 1;
      i++;
      maskFile = std::string(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-ec" || std::string(argv[i])
        == "--evolutionary-config")
    {
      last = i + 5;
      evolConfig = new EvolutionaryConfig();
      i++;
      evolConfig->maxIter = atoi(argv[i]);
      i++;
      evolConfig->oradius = atof(argv[i]);
      i++;
      evolConfig->gfact = atof(argv[i]);
      i++;
      evolConfig->sfact = atof(argv[i]);
      i++;
      evolConfig->epsilon = atof(argv[i]);
      i++;
      if (atoi(argv[i]) != 0)
        evolConfig->oseed = atoi(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-powc" || std::string(argv[i])
        == "--pow-config")
    {
      last = i + 5;
      powConfig = new PowConfig();
      i++;
      powConfig->maxIter = atoi(argv[i]);
      i++;
      powConfig->maxLineIter = atoi(argv[i]);
      i++;
      powConfig->stepLen = atof(argv[i]);
      i++;
      powConfig->stepTol = atof(argv[i]);
      i++;
      powConfig->valueTol = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-rsgdc" || std::string(argv[i])
        == "--rsgd-config")
    {
      last = i + 5;
      rsgdConfig = new RSGDConfig();
      i++;
      rsgdConfig->maxIter = atoi(argv[i]);
      i++;
      rsgdConfig->minStep = atof(argv[i]);
      i++;
      rsgdConfig->maxStep = atof(argv[i]);
      i++;
      rsgdConfig->relax = atof(argv[i]);
      i++;
      rsgdConfig->gradTol = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-dr" || std::string(argv[i])
        == "--drr-rescale")
    {
      last = i + 2;
      i++;
      minrescale = atof(argv[i]);
      i++;
      maxrescale = atof(argv[i]);
      Rescaler = RescaleFilterType::New();
      continue;
    }
    if (std::string(argv[i]) == "-dimc" || std::string(argv[i])
        == "--drr-itf-metric-config")
    {
      last = i + 4;
      drritfConfig = new DRRITFMetricConfig();
      i++;
      drritfConfig->min = atof(argv[i]);
      i++;
      drritfConfig->max = atof(argv[i]);
      i++;
      drritfConfig->bins = atoi(argv[i]);
      i++;
      drritfConfig->mapOutToZero = atoi(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-nmimc" || std::string(argv[i])
        == "--nmi-metric-config")
    {
      last = i + 6;
      nmiConfig = new NMIMetricConfig();
      i++;
      nmiConfig->mbins = atoi(argv[i]);
      i++;
      nmiConfig->fbins = atoi(argv[i]);
      i++;
      nmiConfig->mlb = atof(argv[i]);
      i++;
      nmiConfig->flb = atof(argv[i]);
      i++;
      nmiConfig->mub = atof(argv[i]);
      i++;
      nmiConfig->fub = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-gdmc" || std::string(argv[i])
        == "--gd-metric-config")
    {
      last = i;
      gdConfig = new GDMetricConfig();
      continue;
    }
    if (std::string(argv[i]) == "-msmc" || std::string(argv[i])
        == "--ms-metric-config")
    {
      last = i;
      msConfig = new MSMetricConfig();
      continue;
    }
    if (std::string(argv[i]) == "-mrsdmc" || std::string(argv[i])
        == "--mrsd-metric-config")
    {
      last = i + 1;
      mrsdConfig = new MRSDMetricConfig();
      i++;
      mrsdConfig->lambda = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-mmimc" || std::string(argv[i])
        == "--mmi-metric-config")
    {
      last = i + 2;
      mmiConfig = new MMIMetricConfig();
      i++;
      mmiConfig->numberOfSamples = atoi(argv[i]);
      i++;
      mmiConfig->numberOfBins = atoi(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-srcmc" || std::string(argv[i])
        == "--src-metric-config")
    {
      last = i + 8;
      srcConfig = new SRCMetricConfig();
      i++;
      srcConfig->fbins = atoi(argv[i]);
      i++;
      srcConfig->fmin = atof(argv[i]);
      i++;
      srcConfig->fmax = atof(argv[i]);
      i++;
      srcConfig->mbins = atoi(argv[i]);
      i++;
      srcConfig->mmin = atof(argv[i]);
      i++;
      srcConfig->mmax = atof(argv[i]);
      i++;
      srcConfig->sampleCoverage = atof(argv[i]);
      i++;
      srcConfig->horn = atoi(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-ncmc" || std::string(argv[i])
        == "--nc-metric-config")
    {
      last = i;
      ncConfig = new NCMetricConfig();
      continue;
    }
    if (std::string(argv[i]) == "-ccmc" || std::string(argv[i])
        == "--cc-metric-config")
    {
      last = i;
      ccConfig = new CCMetricConfig();
      continue;
    }
    if (std::string(argv[i]) == "-mshmc" || std::string(argv[i])
        == "--msh-metric-config")
    {
      last = i;
      mshConfig = new MSHMetricConfig();
      continue;
    }
    if (std::string(argv[i]) == "-os" || std::string(argv[i])
        == "--optimizer-scales")
    {
      int c = 0;
      int j = i;
      while (j < argc)
      {
        j++;
        std::string s = std::string(argv[j]);
        if (s[0] >= '0' && s[1] <= '9')
          c++;
        else
          break;
      }
      last = i + c;
      scales.SetSize(c);
      for (j = 0; j < c; j++)
      {
        i++;
        scales[j] = atof(argv[i]);
      }
      continue;
    }
    if (std::string(argv[i]) == "-eos" || std::string(argv[i])
        == "--easy-optimizer-scales")
    {
      last = i + 3;
      i++;
      easyScalesFirst = atof(argv[i]);
      i++;
      easyScalesLast = atof(argv[i]);
      i++;
      easyScalesOther = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-t" || std::string(argv[i]) == "--transform")
    {
      last = i + 6;
      int c = 0;
      while (c < 6 && i < argc)
      {
        i++;
        transformPars[c] = atof(argv[i]);
        c++;
      }
      continue;
    }
    if (std::string(argv[i]) == "-iw" || std::string(argv[i]) == "--itf-window")
    {
      last = i + 4;
      i++;
      iwGeom.posX = atoi(argv[i]);
      i++;
      iwGeom.posY = atof(argv[i]);
      i++;
      iwGeom.width = atof(argv[i]);
      i++;
      iwGeom.height = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-dw" || std::string(argv[i]) == "--drr-window")
    {
      last = i + 4;
      i++;
      dwGeom.posX = atoi(argv[i]);
      i++;
      dwGeom.posY = atof(argv[i]);
      i++;
      dwGeom.width = atof(argv[i]);
      i++;
      dwGeom.height = atof(argv[i]);
      continue;
    }
  }
  if (!PProps)
  {
    std::cout << "Obviously no projection properties were defined.\n";
    return EXIT_FAILURE;
  }
  if (!ITF)
  {
    std::cout << "Obviously no ITF was defined.\n";
    return EXIT_FAILURE;
  }
  if ((last + 3) > argc)
  {
    std::cout << "Obviously command line arguments are invalid.\n";
    std::cout << "Need volume and reference image as last arguments!\n";
    return EXIT_FAILURE;
  }
  if (!evolConfig && !rsgdConfig && !powConfig)
  {
    std::cout << "Obviously command line arguments are invalid.\n";
    std::cout << "Need an optimizer configuration!\n";
    return EXIT_FAILURE;
  }
  if (!drritfConfig && !nmiConfig && !gdConfig && !msConfig && !mrsdConfig
      && !mmiConfig && !ncConfig && !ccConfig && !mshConfig && !srcConfig)
  {
    std::cout << "Obviously command line arguments are invalid.\n";
    std::cout << "Need a metric configuration!\n";
    return EXIT_FAILURE;
  }

#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
  // initialize the Xlib module support for concurrent threads
  // NOTE: must be the first Xlib-call of an application!
  XInitThreads();
#endif

  // get image files and projection props:
  VERBOSE(<< " > Read input images\n")
  std::string volumeFile = std::string(argv[++last]);
  if (!itksys::SystemTools::FileExists(volumeFile.c_str(), true))
  {
    std::cout << "ERROR: Volume image file '" << volumeFile
        << "' does not exist!\n";
    return EXIT_FAILURE;
  }
  VolumeImageType::Pointer volume = ReadImage<VolumeImageType> (volumeFile);
  if (!volume)
  {
    std::cout << "Could not read volume image!\n";
    return EXIT_FAILURE;
  }
  std::string referenceFile = std::string(argv[++last]);
  if (!itksys::SystemTools::FileExists(referenceFile.c_str(), true))
  {
    std::cout << "ERROR: Reference image file '" << referenceFile
        << "' does not exist!\n";
    return EXIT_FAILURE;
  }
  XrayImageType::Pointer xray = ReadImage<XrayImageType> (referenceFile);
  if (!xray)
  {
    std::cout << "Could not read reference image!\n";
    return EXIT_FAILURE;
  }

  VERBOSE(<< " > Setup pipeline\n")
  DRRFilter = DRRFilterType::New();
  DRRFilter->BuildRenderPipeline(); // must be called externally
  DRRFilter->SetContextTitle("");
  DRRFilter->WeakMTimeBehaviorOff();
  DRRFilter->SetInput(volume);

  if (drritfConfig || nmiConfig || gdConfig || mrsdConfig || ccConfig)
  {
    BestMinimize = false;
    BestValue = -1e18;
  }
  else if (msConfig || mmiConfig || ncConfig || mshConfig || srcConfig)
  {
    BestMinimize = true;
    BestValue = 1e18;
  }
  BaseOptimizerType::Pointer optimizer = NULL;
  if (evolConfig)
  {
    NVGeneratorType::Pointer gen = NVGeneratorType::New();
    gen->Initialize(evolConfig->oseed);
    EvolutionaryOptimizerType::Pointer opt = EvolutionaryOptimizerType::New();
    opt->SetNormalVariateGenerator(gen);
    opt->SetMaximumIteration(evolConfig->maxIter);
    opt->SetEpsilon(evolConfig->epsilon);
    opt->Initialize(evolConfig->oradius, evolConfig->gfact, evolConfig->sfact);
    opt->SetMinimize(BestMinimize);
    optimizer = opt;
  }
  else if (rsgdConfig)
  {
    RSGDOptimizerType::Pointer opt = RSGDOptimizerType::New();
    opt->SetNumberOfIterations(rsgdConfig->maxIter);
    opt->SetMinimumStepLength(rsgdConfig->minStep);
    opt->SetMaximumStepLength(rsgdConfig->maxStep);
    opt->SetRelaxationFactor(rsgdConfig->relax);
    opt->SetGradientMagnitudeTolerance(rsgdConfig->gradTol);
    opt->SetMinimize(BestMinimize);
    optimizer = opt;
  }
  else if (powConfig)
  {
    PowOptimizerType::Pointer opt = PowOptimizerType::New();
    opt->SetMaximumIteration(powConfig->maxIter);
    opt->SetMaximumLineIteration(powConfig->maxLineIter);
    opt->SetStepLength(powConfig->stepLen);
    opt->SetStepTolerance(powConfig->stepTol);
    opt->SetValueTolerance(powConfig->valueTol);
    opt->SetMaximize(!BestMinimize);
    optimizer = opt;
  }

  // extract supporting point positions, compute initial (relative) optimizer
  // position:
  double *itfData = ITF->GetDataPointer();
  int itfSize = ITF->GetSize();
  ITFSupportingPoints.clear();
  for (int i = 0; i < itfSize; i++)
    ITFSupportingPoints.push_back(itfData[4 * i]); // x-positions
  BaseOptimizerType::ParametersType initialPosition;
  // optimizer initial position is initial ITF in [0;1] interval
  ITFToRelativePositions(initialPosition);
  optimizer->SetInitialPosition(initialPosition);

  if (scales.Size() == 0 && easyScalesFirst < 0)
  {
    scales.SetSize(initialPosition.Size());
    scales.Fill(100); // default
  }
  else if (easyScalesFirst >= 0 && easyScalesLast >= 0 && easyScalesOther >= 0)
  {
    scales.SetSize(initialPosition.Size());
    scales.Fill(easyScalesOther);
    scales[0] = easyScalesFirst;
    scales[scales.size() - 1] = easyScalesLast;
  }
  if (scales.Size() != initialPosition.Size())
  {
    std::cout << "Number of optimizer scales is invalid!\n";
    return EXIT_FAILURE;
  }
  optimizer->SetScales(scales);

  double dscalesvalue = 0.01; // for finite distance derivative computation
  BaseMetricType::Pointer metric = NULL;
  if (drritfConfig)
  {
    DRRITFMetricType::Pointer m = DRRITFMetricType::New();
    m->SetFixedHistogramClipAtEnds(true);
    m->SetFixedHistogramMinIntensity(drritfConfig->min);
    m->SetFixedHistogramMaxIntensity(drritfConfig->max);
    m->SetFixedNumberOfHistogramBins(drritfConfig->bins);
    m->SetMapOutsideIntensitiesToZeroProbability(drritfConfig->mapOutToZero);
    itk::Array<double> dscales;
    dscales.SetSize(scales.Size());
    dscales.Fill(dscalesvalue);
    m->SetDerivativeScales(dscales);
    metric = m;
  }
  else if (nmiConfig)
  {
    NMIMetricType::Pointer m = NMIMetricType::New();
    NMIMetricType::HistogramSizeType bins;
    bins[0] = nmiConfig->mbins;
    bins[1] = nmiConfig->fbins;
    m->SetHistogramSize(bins);
    NMIMetricType::MeasurementVectorType lb;
    lb[0] = nmiConfig->mlb;
    lb[1] = nmiConfig->flb;
    m->SetLowerBound(lb);
    NMIMetricType::MeasurementVectorType ub;
    ub[0] = nmiConfig->mub;
    ub[1] = nmiConfig->fub;
    m->SetUpperBound(ub);
    itk::Array<double> dscales;
    dscales.SetSize(scales.Size());
    dscales.Fill(dscalesvalue);
    m->SetDerivativeStepLength(1.0);
    m->SetDerivativeStepLengthScales(dscales);
    metric = m;
  }
  else if (gdConfig)
  {
    GDMetricType::Pointer m = GDMetricType::New();
    itk::Array<double> dscales;
    dscales.SetSize(scales.Size());
    dscales.Fill(dscalesvalue);
    m->SetDerivativeScales(dscales);
    metric = m;
  }
  else if (msConfig)
  {
    MSMetricType::Pointer m = MSMetricType::New();
    metric = m;
  }
  else if (mrsdConfig)
  {
    MRSDMetricType::Pointer m = MRSDMetricType::New();
    m->SetLambda(mrsdConfig->lambda);
    m->SetDelta(dscalesvalue);
    metric = m;
  }
  else if (mmiConfig)
  {
    MMIMetricType::Pointer m = MMIMetricType::New();
    m->SetUseAllPixels(false);
    if (mmiConfig->numberOfSamples > 0)
      m->SetNumberOfSpatialSamples(mmiConfig->numberOfSamples);
    else
      m->SetUseAllPixels(true);
    m->SetNumberOfHistogramBins(mmiConfig->numberOfBins);
    metric = m;
  }
  else if (ncConfig)
  {
    NCMetricType::Pointer m = NCMetricType::New();
    metric = m;
  }
  else if (ccConfig)
  {
    CCMetricType::Pointer m = CCMetricType::New();
    itk::Array<double> dscales;
    dscales.SetSize(scales.Size());
    dscales.Fill(dscalesvalue);
    m->SetDerivativeStepLengthScales(dscales);
    m->SetDerivativeStepLength(1.0);
    metric = m;
  }
  else if (mshConfig)
  {
    MSHMetricType::Pointer m = MSHMetricType::New();
    itk::Array<double> dscales;
    dscales.SetSize(scales.Size());
    dscales.Fill(dscalesvalue);
    m->SetDerivativeStepLengthScales(dscales);
    m->SetDerivativeStepLength(1.0);
    metric = m;
  }
  else if (srcConfig)
  {
    SRCMetricType::Pointer m = SRCMetricType::New();
    m->SetFixedHistogramClipAtEnds(true);
    m->SetFixedHistogramMinIntensity(srcConfig->fmin);
    m->SetFixedHistogramMaxIntensity(srcConfig->fmax);
    m->SetFixedNumberOfHistogramBins(srcConfig->fbins);
    m->SetMovingHistogramClipAtEnds(true);
    m->SetMovingHistogramMinIntensity(srcConfig->fmin);
    m->SetMovingHistogramMaxIntensity(srcConfig->fmax);
    m->SetMovingNumberOfHistogramBins(srcConfig->fbins);
    m->SetSampleCoverage(srcConfig->sampleCoverage);
    m->SetUseHornTiedRanksCorrection(srcConfig->horn);
    itk::Array<double> dscales;
    dscales.SetSize(scales.Size());
    dscales.Fill(dscalesvalue);
    m->SetDerivativeScales(dscales);
    metric = m;
  }

  // the rigid body transform here is static:
  Transform3DType::Pointer rigidTransform = Transform3DType::New();
  rigidTransform->SetParameters(transformPars);
  DRRFilter->SetTransform(rigidTransform);

  // the virtual transform is a tool for optimizing the supporting point weights:
  TransformType::Pointer transform = TransformType::New();
  transform->SetNumberOfConnectedTransformParameters(ITF->GetSize());
  CommandType::Pointer cmd = CommandType::New();
  cmd->SetCallback(TransformationChangedEvent);
  cmd->SetClientData(NULL);
  transform->AddObserver(ora::TransformChanged(), cmd);

  // complete projection properties and apply them to DRR engine:
  XrayImageType::RegionType freg;
  if (fir[2] < 0 || fir[3] < 0)
  {
    // -> largest possible region
    freg = xray->GetLargestPossibleRegion();
  }
  else
  {
    if (fir[0] < 0 || fir[1] < 0 || fir[2] < 0 || fir[3] < 0)
    {
      std::cout << "ERROR: fixed image region offset (or 1 size) < 0!\n";
      return EXIT_FAILURE;
    }
    XrayImageType::SpacingType spac = xray->GetSpacing();
    DRRPropsType::FixedImageRegionType::IndexType idx;
    idx.Fill(0);
    idx[0]
        = static_cast<DRRPropsType::FixedImageRegionType::IndexValueType> (fir[0]
            / spac[0]);
    idx[1]
        = static_cast<DRRPropsType::FixedImageRegionType::IndexValueType> (fir[1]
            / spac[1]);
    freg.SetIndex(idx);
    DRRPropsType::FixedImageRegionType::SizeType sz;
    sz.Fill(0);
    sz[0]
        = static_cast<DRRPropsType::FixedImageRegionType::SizeValueType> (fir[2]
            / spac[0]);
    sz[1]
        = static_cast<DRRPropsType::FixedImageRegionType::SizeValueType> (fir[3]
            / spac[1]);
    freg.SetSize(sz);
  }
  PProps->SetITF(ITF); // connect
  PProps->SetGeometryFromFixedImage(xray, freg);
  PProps->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(),
      rayStepSizeMode);
  if (!PProps->AreAllPropertiesValid())
  {
    std::cout << "ERROR: Projection properties appear to be invalid!\n";
    return EXIT_FAILURE;
  }
  ApplyProjectionPropertiesToDRREngine();

  Reducer = ReducerType::New();
  if (!Rescaler)
  {
    Reducer->SetInput(DRRFilter->GetOutput());
    metric->SetMovingImage(Reducer->GetOutput());
  }
  else
  {
    Rescaler->SetInput(DRRFilter->GetOutput());
    Rescaler->SetOutputMinimum(minrescale);
    Rescaler->SetOutputMaximum(maxrescale);
    Reducer->SetInput(Rescaler->GetOutput());
    metric->SetMovingImage(Reducer->GetOutput());
  }
  XrayImageType::RegionType extReg;
  extReg = xray->GetLargestPossibleRegion();
  extReg.SetSize(2, 0); // eliminate 3rd dimension
  Reducer->SetExtractionRegion(extReg);

  try
  {
    DRRFilter->Update();
    if (Rescaler)
      Rescaler->Update();
    Reducer->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cout << "ERROR: DRR could not be rendered!\n";
    return EXIT_FAILURE;
  }

  ReducerType::Pointer freducer = ReducerType::New();
  freducer->SetInput(xray);
  freg.SetSize(2, 0); // no 3rd dimension desired
  freducer->SetExtractionRegion(freg);
  freducer->Update();
  metric->SetFixedImage(freducer->GetOutput());
  XrayImageType2D::RegionType freg2D;
  XrayImageType2D::IndexType idx2D;
  idx2D[0] = freg.GetIndex(0);
  idx2D[1] = freg.GetIndex(1);
  freg2D.SetIndex(idx2D);
  XrayImageType2D::SizeType sz2D;
  sz2D[0] = freg.GetSize(0);
  sz2D[1] = freg.GetSize(1);
  freg2D.SetSize(sz2D);
  metric->SetFixedImageRegion(freg2D);
  if (maskFile.length() > 0)
  {
    MaskImageType2D::Pointer mi = ReadImage<MaskImageType2D> (maskFile);
    if (mi)
    {
      SpatialMaskType2D::Pointer sm = SpatialMaskType2D::New();
      sm->SetImage(mi);
      sm->Update();
      metric->SetFixedImageMask(sm);
    }
    else
    {
      VERBOSE(<< "WARNING: Mask could not be read - resume without it!\n")
    }
  }
  metric->SetTransform(transform); // fake transform
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  metric->SetInterpolator(interpolator);
  metric->Initialize();
  optimizer->SetCostFunction(metric);
  CommandType::Pointer cmd2 = CommandType::New();
  cmd2->SetCallback(OptimizerIterationEvent);
  cmd2->SetClientData(NULL);
  optimizer->AddObserver(itk::IterationEvent(), cmd2);

  // prepare / initialize DRR window:
  if (dwGeom.width > 0)
  {
    DRRViewer = CreateVTKWindow(true, "Current DRR", dwGeom.posX, dwGeom.posY,
        dwGeom.width, dwGeom.height, true, NULL, 0);
    DRRMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    DRRLU = vtkSmartPointer<vtkImageMapToColors>::New();
    vtkLookupTable *lut = vtkLookupTable::New();
    vtkImageData *vref = Create2DVTKImageFromITKImage<XrayImageType> (xray);
    // .. intensity range of reference image:
    lut->SetTableRange(vref->GetScalarRange()[0], vref->GetScalarRange()[1]);
    vref->Delete();
    lut->SetRampToLinear();
    lut->SetValueRange(0, 1);
    lut->SetSaturationRange(0, 0);
    lut->SetHueRange(0, 0);
    lut->SetAlphaRange(1, 1);
    lut->Build();
    DRRLU->SetLookupTable(lut);
    lut->Delete();
    DRRMapper->SetInput(DRRLU->GetOutput());
    DRRActor = vtkSmartPointer<vtkActor>::New();
    DRRActor->SetMapper(DRRMapper);
    DRRViewer->GetRenderers()->GetFirstRenderer()->AddActor(DRRActor);
  }

  // prepare / initialize ITF window (transfer function window):
  if (iwGeom.width > 0 && ITF && ITFSupportingPoints.size() > 0)
  {
    ITFTable = vtkSmartPointer<vtkTable>::New();
    vtkSmartPointer<vtkDoubleArray> inPoints =
        vtkSmartPointer<vtkDoubleArray>::New();
    inPoints->SetName("in");
    ITFTable->AddColumn(inPoints);
    vtkSmartPointer<vtkDoubleArray> outPoints =
        vtkSmartPointer<vtkDoubleArray>::New();
    outPoints->SetName("out");
    ITFTable->AddColumn(outPoints);
    ITFTable->SetNumberOfRows(ITF->GetSize());
    CurrentITFToVTKTable();
    ITFWin = SetupVTKGraphWindow(ITFTable, iwGeom.posX, iwGeom.posY,
        iwGeom.width, iwGeom.height, 2, 255, 0, 0, 255, "  in  ", "  out  ");
    ITFWin->SetWindowName("Intensity Transfer Function (ITF)");
  }

  // start the optimization thread:
  ThreaderType::Pointer threader = ThreaderType::New();
  threader->SetNumberOfThreads(1);
  threader->SetSingleMethod(ThreadedOptimization, optimizer);
  threader->SingleMethodExecute();

  if (doNotTerminate && ITFWin)
  {
    ITFWin->GetInteractor()->Start();
  }

  if (fir)
    delete[] fir;

  return EXIT_SUCCESS;
}
