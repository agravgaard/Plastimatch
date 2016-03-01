//
#ifndef ORAREG23MODEL_TXX_
#define ORAREG23MODEL_TXX_

#include "oraREG23Model.h"

// ORAIFTools
#include <oraIniAccess.h>
#include <oraStringTools.h>
#include <oraFileTools.h>
// ORAIFModel
#include <oraImageConsumer.h>
// ORAIFImageTools
#include <oraVTKUnsharpMaskingImageFilter.h>
// ORAIFStructureAccess
#include <oraVTKStructure.h>
// ORAIFNReg2D3DAlgorithm
#include <oraMultiResolutionNWay2D3DRegistrationMethod.h>
#include <itkAmoebaOptimizerEx.h>
#include <itkPowellOptimizerEx.h>
// ORAIFNReg2D3DDRREngine
#include <oraProjectionProperties.h>
// ORAIFNReg2D3DMetrics
#include <oraStochasticRankCorrelationImageToImageMetric.h>
#include <oraGradientDifferenceImageToImageMetric.h>

#include "itkImageKernelOperator.h"  // NOTE: Copied into source tree from ITK/Review
//ITK
#include <itkCastImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkEuler3DTransform.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkIntensityWindowingImageFilter.h>
#include <itkNormalizedMutualInformationHistogramImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkMeanReciprocalSquareDifferenceImageToImageMetric.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkImageMaskSpatialObject.h>
#include <itkSingleValuedNonLinearOptimizer.h>
#include <itkOnePlusOneEvolutionaryOptimizer.h>
#include <itkNormalVariateGenerator.h>
#include <itkImageRegionConstIterator.h>
#include <itkRealTimeClock.h>
#include <itkImageFileWriter.h>
#include <itkNormalizedCorrelationImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkTranslationTransform.h>
#include <itkIdentityTransform.h>
#include <itkMeanImageFilter.h>
#include <itkFlipImageFilter.h>
#include <itkThresholdImageFilter.h>
//VTK
#include <vtkColorTransferFunction.h>
#include <vtkTable.h>
#include <vtkMath.h>
#include <vtkFunctionParser.h>
#include <vtkImageCast.h>
#include <vtkPlane.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkPointData.h>
#include <vtkImageStencilData.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMatrix4x4.h>
#include <vtkTriangleFilter.h>
#include <vtkLinearSubdivisionFilter.h>

#include <fstream>
#include <sstream>

namespace ora
{

template<typename TVolumeComponentType>
bool REG23Model::InstantiateMetrics(GenericRegistrationPointer nreg)
{
  if (!nreg)
    return false;

  typedef TVolumeComponentType VolumePixelType;
  typedef unsigned RankPixelType;
  typedef double GradientPixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef itk::Image<RankPixelType, 2> Rank2DImageType;
  typedef itk::Image<MaskPixelType, 3> MaskImage3DType;
  typedef MaskImage3DType::Pointer MaskImage3DPointer;
  typedef itk::Image<MaskPixelType, 2> MaskImage2DType;
  typedef MaskImage2DType::Pointer MaskImage2DPointer;
  typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
      DRR2DImageType, VolumeImageType> RegistrationType;
  typedef typename RegistrationType::MetricType CompositeMetricType;
  typedef typename CompositeMetricType::Pointer CompositeMetricPointer;
  typedef itk::ImageToImageMetric<DRR2DImageType, DRR2DImageType> BaseMetricType;
  typedef typename BaseMetricType::Pointer BaseMetricPointer;
  typedef StochasticRankCorrelationImageToImageMetric<DRR2DImageType,
    DRR2DImageType, Rank2DImageType> SRCMetricType;
  typedef typename SRCMetricType::Pointer SRCMetricPointer;
  typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<
    DRR2DImageType, DRR2DImageType> NMIMetricType;
  typedef typename NMIMetricType::Pointer NMIMetricPointer;
  typedef itk::NormalizedCorrelationImageToImageMetric<DRR2DImageType,
    DRR2DImageType> NCMetricType;
  typedef typename NCMetricType::Pointer NCMetricPointer;
  typedef itk::MeanReciprocalSquareDifferenceImageToImageMetric<DRR2DImageType,
    DRR2DImageType> MRSDMetricType;
  typedef typename MRSDMetricType::Pointer MRSDMetricPointer;
  typedef itk::MeanSquaresImageToImageMetric<DRR2DImageType, DRR2DImageType>
      MSMetricType;
  typedef typename MSMetricType::Pointer MSMetricPointer;
  typedef GradientDifferenceImageToImageMetric<DRR2DImageType,
      DRR2DImageType, GradientPixelType> GDMetricType;
  typedef typename GDMetricType::Pointer GDMetricPointer;
  typedef itk::ExtractImageFilter<MaskImage3DType, MaskImage2DType> ExtractorType;
  typedef ExtractorType::Pointer ExtractorPointer;
  typedef itk::ChangeInformationImageFilter<MaskImage2DType> ChangerType;
  typedef ChangerType::Pointer ChangerPointer;
  typedef itk::ImageMaskSpatialObject<2> MaskSpatialObjectType;
  typedef MaskSpatialObjectType::Pointer MaskSpatialObjectPointer;

  typename RegistrationType::Pointer cnreg =
      static_cast<RegistrationType *>(nreg.GetPointer());
  CompositeMetricPointer cm = cnreg->GetMetric();
  std::string rule = ""; // -> minimization regardless of metric-type expected
  std::string s = "", errorMessage = "", prefix, postfix;
  std::vector<std::string> margs;
  for (std::size_t i = 0; i < m_PPFixedImages.size(); i++)
  {
    BaseMetricPointer metric = NULL;
    margs.clear();
    if (!ParseMetricEntry(m_Config->ReadString("Registration", "MetricType" + StreamConvert(i + 1), ""),
        m_Config->ReadString("Registration", "MetricConfig" + StreamConvert(i + 1), ""),
        m_Config->ReadString("Registration", "MetricRulePrefix" + StreamConvert(i + 1), ""),
        m_Config->ReadString("Registration", "MetricRulePostfix" + StreamConvert(i + 1), ""),
        margs, errorMessage))
      return false;

    prefix = "";
    postfix = "";
    if (margs[0] == "SRC") // orig: -> MIN
    {
      SRCMetricPointer m = SRCMetricType::New();
      metric = m;
    }
    else if (margs[0] == "NMI") // orig: -> MAX
    {
      NMIMetricPointer m = NMIMetricType::New();
      metric = m;
    }
    else if (margs[0] == "NCC") // orig: -> MIN
    {
      NCMetricPointer m = NCMetricType::New();
      metric = m;
    }
    else if (margs[0] == "MRSD") // orig: -> MAX
    {
      MRSDMetricPointer m = MRSDMetricType::New();
      metric = m;
    }
    else if (margs[0] == "MS") // orig: -> MIN
    {
      MSMetricPointer m = MSMetricType::New();
      metric = m;
    }
    else if (margs[0] == "GD") // orig: -> MAX
    {
      GDMetricPointer m = GDMetricType::New();
      metric = m;
    }
    else
    {
      return false;
    }

    // configure the metric component
    ConfigureMetricComponent<TVolumeComponentType>(metric.GetPointer(), margs,
        prefix, postfix);

    // optional fixed image mask
    if (m_MaskImages[i]->ProduceImage())
    {
      ITKVTKImage *prodImage = m_MaskImages[i]->ProduceImage(); // for old GCC
      MaskImage3DPointer fmask = static_cast<MaskImage3DType*>(
          prodImage->GetAsITKImage<MaskPixelType>().GetPointer());
      if (fmask)
      {
        ExtractorPointer extractor = ExtractorType::New();
        MaskImage3DType::RegionType ereg;
        MaskImage3DType::IndexType eidx;
        eidx.Fill(0);
        MaskImage3DType::SizeType esz = fmask->GetLargestPossibleRegion().GetSize();
        esz[2] = 0; // -> 2D
        ereg.SetIndex(eidx);
        ereg.SetSize(esz);
        extractor->SetExtractionRegion(ereg);
        extractor->SetInput(fmask);
        try
        {
          extractor->Update();
        }
        catch (itk::ExceptionObject &e)
        {
          return false;
        }
        // -> mask should have 0,0 origin and [1 0;0 1] direction:
        ChangerPointer changer = ChangerType::New();
        changer->ChangeNone();
        changer->ChangeDirectionOn();
        changer->ChangeOriginOn();
        MaskImage2DType::DirectionType mdir;
        mdir.SetIdentity();
        changer->SetOutputDirection(mdir);
        MaskImage2DType::PointType morig;
        morig.Fill(0);
        changer->SetOutputOrigin(morig);
        changer->SetInput(extractor->GetOutput());
        try
        {
          changer->Update();
        }
        catch (itk::ExceptionObject &e)
        {
          return false;
        }
        MaskSpatialObjectType::Pointer fmspat = MaskSpatialObjectType::New();
        fmspat->SetImage(changer->GetOutput());
        try
        {
          fmspat->Update();
        }
        catch (itk::ExceptionObject &e)
        {
          return false;
        }
        // set the fixed image mask to the metric:
        metric->SetFixedImageMask(fmspat);

        // accelerate DRR computation a bit by explicitly marking the pixels
        // to be computed:
        if (m_Config->ReadValue<bool>("Auto-Masking", "UseMaskOptimization", false))
        {
          // FIXME: eventually, we could add a special section for SRC (where
          // we could the pixels more efficiently ...

          // finally set the DRR pixel mask!
          std::vector<MaskImage3DPointer> masks;
          masks.push_back(fmask); // just one level (no multi-resolution, yet)
          cnreg->SetIthDRRMasks(i, masks);
        }
      }
    }
    // finally add the metric to the composite metric and build rule part:
    std::ostringstream os;
    std::ostringstream os2;
    os.str("");
    os << "m" << i;
    os2.str("");
    os2 << "d" << i;
    cm->AddMetricInput(metric, os.str(), os2.str());
    cnreg->AddMetricFixedImageMapping(metric, i);
    if (i == 0)
    {
      rule = prefix + os.str() + postfix;
    }
    else
    {
      rule += "+" + prefix;
      rule += os.str() + postfix;
    }
  }
  cm->SetValueCompositeRule(rule);

  return true;
}

template<typename TVolumeComponentType>
void REG23Model::ConfigureMetricComponent(itk::Object::Pointer metric,
    std::vector<std::string> &margs, std::string &prefix, std::string &postfix)
{
  typedef TVolumeComponentType VolumePixelType;
  typedef unsigned RankPixelType;
  typedef double GradientPixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef itk::Image<RankPixelType, 2> Rank2DImageType;
  typedef StochasticRankCorrelationImageToImageMetric<DRR2DImageType,
    DRR2DImageType, Rank2DImageType> SRCMetricType;
  typedef typename SRCMetricType::Pointer SRCMetricPointer;
  typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<
    DRR2DImageType, DRR2DImageType> NMIMetricType;
  typedef typename NMIMetricType::Pointer NMIMetricPointer;
  typedef itk::NormalizedCorrelationImageToImageMetric<DRR2DImageType,
    DRR2DImageType> NCMetricType;
  typedef typename NCMetricType::Pointer NCMetricPointer;
  typedef itk::MeanReciprocalSquareDifferenceImageToImageMetric<DRR2DImageType,
    DRR2DImageType> MRSDMetricType;
  typedef typename MRSDMetricType::Pointer MRSDMetricPointer;
  typedef itk::MeanSquaresImageToImageMetric<DRR2DImageType, DRR2DImageType>
      MSMetricType;
  typedef typename MSMetricType::Pointer MSMetricPointer;
  typedef GradientDifferenceImageToImageMetric<DRR2DImageType,
      DRR2DImageType, GradientPixelType> GDMetricType;
  typedef typename GDMetricType::Pointer GDMetricPointer;

  prefix = "";
  postfix = "";
  if (margs.size() <= 0 || !metric)
    return;
  if (margs[0] == "SRC") // orig: -> MIN
  {
    SRCMetricType *m = dynamic_cast<SRCMetricType*>(metric.GetPointer());
    // forget about seeds -> deterministic in order to do not confuse users by
    // producing varying registration results when performing the same
    // registration multiple times!
    SRCMetricType::SeedsType mrseeds;
    mrseeds.SetSize(SRCMetricType::ImageDimension);
    for (unsigned int d = 0; d < SRCMetricType::ImageDimension; d++)
      mrseeds[d] = 1239484 + d * 327;
    m->SetRandomSeeds(mrseeds);
    // clip always
    m->SetFixedHistogramClipAtEnds(true);
    m->SetMovingHistogramClipAtEnds(true);
    // configurable metric part:
    m->SetFixedHistogramMinIntensity(atof(margs[1].c_str()));
    m->SetFixedHistogramMaxIntensity(atof(margs[2].c_str()));
    m->SetFixedNumberOfHistogramBins(atoi(margs[3].c_str()));
    m->SetMovingHistogramMinIntensity(atof(margs[4].c_str()));
    m->SetMovingHistogramMaxIntensity(atof(margs[5].c_str()));
    m->SetMovingNumberOfHistogramBins(atoi(margs[6].c_str()));
    m->SetSampleCoverage(atof(margs[7].c_str()));
    // default "no-overlap" behavior
    m->SetNoOverlapReactionMode(1);
    m->SetNoOverlapMetricValue(1000);
    // forget optimizations at the moment:
    m->SetUseHornTiedRanksCorrection(false);
    m->SetMovingZeroRanksContributeToMeasure(false);
    // no debugging
    m->SetExtractSampleDistribution(false);
    if (margs.size() > 8)
      prefix = margs[8];
    if (margs.size() > 9)
      postfix = margs[9];
  }
  else if (margs[0] == "NMI") // orig: -> MAX
  {
    NMIMetricType *m = dynamic_cast<NMIMetricType*>(metric.GetPointer());
    // configurable metric part:
    typename NMIMetricType::MeasurementVectorType lb;
    lb[0] = atof(margs[1].c_str());
    lb[1] = atof(margs[4].c_str());
    m->SetLowerBound(lb);
    typename NMIMetricType::MeasurementVectorType ub;
    ub[0] = atof(margs[2].c_str());
    ub[1] = atof(margs[5].c_str());
    m->SetUpperBound(ub);
    typename NMIMetricType::HistogramSizeType hsz;
    hsz[0] = atoi(margs[3].c_str());
    hsz[1] = atoi(margs[6].c_str());
    m->SetHistogramSize(hsz);
    m->SetPaddingValue(atof(margs[7].c_str()));
    m->SetComputeGradient(false);
    if (margs.size() > 8)
      prefix = margs[8];
    if (margs.size() > 9)
      postfix = margs[9];
  }
  else if (margs[0] == "NCC") // orig: -> MIN
  {
    NCMetricType *m = dynamic_cast<NCMetricType*>(metric.GetPointer());
    // configurable metric part:
    m->SetSubtractMean(atoi(margs[1].c_str()));
    m->SetComputeGradient(false);
    if (margs.size() > 2)
      prefix = margs[2];
    if (margs.size() > 3)
      postfix = margs[3];
  }
  else if (margs[0] == "MRSD") // orig: -> MAX
  {
    MRSDMetricType *m = dynamic_cast<MRSDMetricType*>(metric.GetPointer());
    // configurable metric part:
    m->SetLambda(atof(margs[1].c_str()));
    m->SetComputeGradient(false);
    if (margs.size() > 2)
      prefix = margs[2];
    if (margs.size() > 3)
      postfix = margs[3];
  }
  else if (margs[0] == "MS") // orig: -> MIN
  {
    MSMetricType *m = dynamic_cast<MSMetricType*>(metric.GetPointer());
    m->SetComputeGradient(false);
    if (margs.size() > 1)
      prefix = margs[1];
    if (margs.size() > 2)
      postfix = margs[2];
  }
  else if (margs[0] == "GD") // orig: -> MAX
  {
    //GDMetricType *m = dynamic_cast<GDMetricType*>(metric.GetPointer());
    if (margs.size() > 1)
      prefix = margs[1];
    if (margs.size() > 2)
      postfix = margs[2];
  }
}

template<typename TVolumeComponentType>
bool REG23Model::InstantiateOptimizer(GenericRegistrationPointer nreg)
{
  if (!nreg)
    return false;

  typedef TVolumeComponentType VolumePixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
      DRR2DImageType, VolumeImageType> RegistrationType;
  typedef itk::SingleValuedNonLinearOptimizer BaseOptimizerType;
  typedef BaseOptimizerType::Pointer BaseOptimizerPointer;
  typedef itk::OnePlusOneEvolutionaryOptimizer EvolOptimizerType;
  typedef EvolOptimizerType::Pointer EvolOptimizerPointer;
  typedef itk::AmoebaOptimizerEx AmoebaOptimizerType;
  typedef AmoebaOptimizerType::Pointer AmoebaOptimizerPointer;
  typedef itk::PowellOptimizerEx PowellOptimizerType;
  typedef PowellOptimizerType::Pointer PowellOptimizerPointer;
  typedef itk::Statistics::NormalVariateGenerator NVGeneratorType;
  typedef NVGeneratorType::Pointer NVGeneratorPointer;
  typedef typename RegistrationType::MetricType CompositeMetricType;
  typedef typename CompositeMetricType::Pointer CompositeMetricPointer;

  typename RegistrationType::Pointer cnreg =
      static_cast<RegistrationType *>(nreg.GetPointer());

  std::vector<std::string> oargs;
  std::string errorMessage;
  if (!ParseOptimizerEntry(m_Config->ReadString("Registration", "OptimizerType", ""),
      m_Config->ReadString("Registration", "OptimizerConfig", ""),
      m_Config->ReadString("Registration", "OptimizerScales", ""),
      oargs, errorMessage))
    return false;

  BaseOptimizerPointer opt = NULL;
  if (oargs[0] == "1+1EVOL")
  {
    EvolOptimizerPointer evol = EvolOptimizerType::New();
    NVGeneratorPointer gen = NVGeneratorType::New();
    gen->Initialize(atoi(oargs[6].c_str()));
    evol->SetNormalVariateGenerator(gen);
    evol->SetMaximumIteration(atoi(oargs[1].c_str()));
    m_NumberOfIterations = atoi(oargs[1].c_str());
    evol->Initialize(atof(oargs[2].c_str()), atof(oargs[3].c_str()), atof(oargs[4].c_str()));
    evol->SetEpsilon(atof(oargs[5].c_str()));
    evol->SetMaximize(false); // we always minimize!
    evol->AddObserver(itk::IterationEvent(), m_OptimizerCommand);
    // (SetCallbackFunction() must be called externally from a task)
    opt = evol;
  }
  else if (oargs[0] == "AMOEBA")
  {
    AmoebaOptimizerPointer amoeba = AmoebaOptimizerType::New();
    bool autoSimplex = false;
    AmoebaOptimizerType::ParametersType initialSimplexDeltas;
    initialSimplexDeltas.SetSize(6);
    for (int d = 0; d < 6; d++)
    {
      initialSimplexDeltas[d] = atof(oargs[d + 4].c_str());
      if (initialSimplexDeltas[d] <= 0) // one <0 --> auto!
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
    amoeba->SetMaximumNumberOfIterations(atoi(oargs[1].c_str()));
    m_NumberOfIterations = atoi(oargs[1].c_str());
    amoeba->SetParametersConvergenceTolerance(atof(oargs[2].c_str()));
    amoeba->SetFunctionConvergenceTolerance(atof(oargs[3].c_str()));
    amoeba->SetMaximize(false); // we always minimize!
    amoeba->AddObserver(itk::FunctionEvaluationIterationEvent(), m_OptimizerCommand);
    // (SetCallbackFunction() must be called externally from a task)
    opt = amoeba;
  }
  else if (oargs[0] == "POWELL")
  {
    PowellOptimizerPointer powell = PowellOptimizerType::New();
    powell->SetMaximumIteration(atoi(oargs[1].c_str()));
    powell->SetMaximumLineIteration(atoi(oargs[2].c_str()));
    m_NumberOfIterations = powell->GetMaximumIteration() *
        powell->GetMaximumLineIteration() * 6;
    powell->SetStepLength(atof(oargs[3].c_str()));
    powell->SetStepTolerance(atof(oargs[4].c_str()));
    powell->SetValueTolerance(atof(oargs[5].c_str()));
    powell->SetMaximize(false); // we always minimize!
    powell->AddObserver(itk::IterationEvent(), m_OptimizerCommand);
    // (SetCallbackFunction() must be called externally from a task)
    opt = powell;
  }
  BaseOptimizerType::ScalesType scales;
  scales.SetSize(6);
  for (std::size_t i = 0, j = oargs.size() - 6; i < 6; i++, j++)
    scales[i] = atof(oargs[j].c_str());
  opt->SetScales(scales);

  cnreg->SetOptimizer(opt);
  CompositeMetricPointer cm = cnreg->GetMetric();
  cm->SetUseOptimizedValueComputation(m_Config->ReadValue<bool>("Registration",
      "UseOptimizedValueComputation", false));

  return true;
}

template<typename TVolumeComponentType>
bool REG23Model::InitializeInternal()
{
  if (!m_DRRToolRenderWindow) // required!
    return false;

  typedef TVolumeComponentType VolumePixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef itk::Image<DRRPixelType, 3> DRRImageType;
  typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
      DRR2DImageType, VolumeImageType> RegistrationType;
  typedef ora::ProjectionProperties<DRRPixelType> DRRPropsType;

  if (m_NReg)
  {
    RegistrationType *cnreg = dynamic_cast<RegistrationType *>(
        m_NReg.GetPointer());
    if (cnreg)
    {
      typename RegistrationType::DRREngineType *de = cnreg->GetDRREngine();
      de->GetEventObject()->RemoveAllObservers(); // delete all observers!
    }
  }
  m_NReg = NULL; // init

  typename RegistrationType::Pointer nreg = RegistrationType::New();
  nreg->RemoveAllMetricFixedImageMappings();
  // configure images&co:
  ITKVTKImage *prodImage = m_PPVolume->ProduceImage(); // for old GCC
  typename VolumeImageType::Pointer movingImage = static_cast<VolumeImageType * >(
      prodImage->GetAsITKImage<VolumePixelType>().GetPointer());
  nreg->SetMoving3DVolume(movingImage);
  std::string s, s2;
  for (std::size_t fi = 0; fi < m_PPFixedImages.size(); fi++)
  {
    ITKVTKImage *fprodImage = GetFixedImage(fi, true);  //m_PPFixedImages[fi]->ProduceImage();
    typename DRRImageType::Pointer fixedImage = static_cast<DRRImageType * >(
        fprodImage->GetAsITKImage<DRRPixelType>().
        GetPointer());
    DRRPropsType::Pointer props = DRRPropsType::New(); // proj-props
    // (the region can implicitly be resized by the pre-processing operations)
    s = m_Config->ReadString("DRR", "ITF" + StreamConvert(fi + 1), "");
    props->SetITF(ParseITF(s, s2));
    DRRPropsType::FixedImageRegionType fir = fixedImage->
        GetLargestPossibleRegion();
    props->SetGeometryFromFixedImage(fixedImage, fir);
    s = ToUpperCaseF(TrimF(m_Config->ReadString("DRR",
        "CastStepSize" + StreamConvert(fi + 1), "")));
    if (IsNumeric(s))
    {
      props->SetSamplingDistance(atof(s.c_str()));
    }
    else
    {
      int mode = -1;
      if (s == "AUTO-0")
        mode = 0;
      else if (s == "AUTO-1")
        mode = 1;
      else if (s == "AUTO-2")
        mode = 2;
      else if (s == "AUTO-3")
        mode = 3;
      if (props->ComputeAndSetSamplingDistanceFromVolume(movingImage->
          GetSpacing(), mode) <= 0)
        return false;
    }
    DRRPropsType::PointType fs;
    fs[0] = m_SourcePositions[fi][0];
    fs[1] = m_SourcePositions[fi][1];
    fs[2] = m_SourcePositions[fi][2];
    props->SetSourceFocalSpotPosition(fs);
    if (!props->AreAllPropertiesValid())
      return false;
    if (!nreg->AddFixedImageAndProps(fixedImage, fir, props))
      return false;
  }

  nreg->SetTransform(m_Transform);
  m_Transform->SetCenter(m_CenterOfRotation); // apply COR
  // -> apply initial parameters (from config) to current parameters:
  for (unsigned int d = 0; d < 6; d++)
    m_CurrentParameters[d] = m_InitialParameters[d];
  nreg->SetInitialTransformParameters(m_CurrentParameters);

  // FIXME: at the moment, we do not support multi-resolution levels:
  nreg->SetNumberOfLevels(1);
  nreg->SetUseAutoProjectionPropsAdjustment(true);
  nreg->SetAutoSamplingDistanceAdjustmentMode(1); // FIXME: later
  nreg->SetUseMovingPyramidForFinalLevel(false);
  nreg->SetUseMovingPyramidForUnshrinkedLevels(false);
  nreg->SetUseFixedPyramidForFinalLevel(false);
  nreg->SetUseFixedPyramidForUnshrinkedLevels(false);

  // instantiate the metrics:
  if (!InstantiateMetrics<TVolumeComponentType>(nreg.GetPointer()))
    return false;
  // instantiate the optimizer:
  if (!InstantiateOptimizer<TVolumeComponentType>(nreg.GetPointer()))
    return false;

  // connect a custom render window to internal DRR engine (hidden window):
  nreg->SetExternalRenderWindow(m_DRRToolRenderWindow);

  // add observers:
  nreg->AddObserver(itk::StartEvent(), m_RegistrationCommand);
  nreg->AddObserver(ora::StartMultiResolutionLevelEvent(),m_RegistrationCommand);
  nreg->AddObserver(ora::StartOptimizationEvent(), m_RegistrationCommand);
  nreg->AddObserver(itk::EndEvent(), m_RegistrationCommand);
  // (SetCallbackFunction() must be called externally from a task)

  // add event object observer of DRR engine:
  typename RegistrationType::DRREngineType *de = nreg->GetDRREngine();
  if (de && de->GetEventObject())
  {
    vtkSmartPointer<vtkCallbackCommand> cb =
        vtkSmartPointer<vtkCallbackCommand>::New();
    cb->SetCallback(REG23Model::GlobalRenderCoordinationCB);
    de->GetEventObject()->AddObserver(vtkCommand::StartEvent, cb);
    de->GetEventObject()->AddObserver(vtkCommand::EndEvent, cb);
    de->SetFireStartEndEvents(true);
    de->SetDoNotTryToResizeNonInteractiveWindows(true);
  }

  // in addition, we initialize the registration framework (which is done later
  // again) and compute the initial DRRs which we can display:
  try
  {
    if (!nreg->Initialize())
      return false;

    // generate initial DRRs and offer them to the viewers:
    m_CurrentMovingImageMutex.lock();
    // make current moving images available for observers in the Update-event:
    for (std::size_t i = 0; i < nreg->GetNumberOfFixedImages(); i++)
    {
      ITKVTKImage *ivi = new ITKVTKImage(itk::ImageIOBase::SCALAR,
          itk::ImageIOBase::FLOAT);
      // for 1st level:
      DRRImageType::Pointer drr = nreg->Compute3DTestProjection(i, 0);
      if (!drr)
      {
        for (std::size_t j = 0; j < m_CurrentMovingImages.size(); j++)
        {
          if (m_CurrentMovingImages[j])
            delete m_CurrentMovingImages[j];
        }
        m_CurrentMovingImages.clear();
        break;
      }
      ivi->SetITKImage(drr.GetPointer(), true); // convert to VTK-format
      m_CurrentMovingImages.push_back(ivi); // must delete externally!
    }
    m_CurrentMovingImageMutex.unlock();

    // let the external observer access (e.g. copy) the images if desired:
    if (m_CurrentMovingImages.size() > 0)
      this->Notify(UPDATE_INITIAL_MOVING_IMAGES);

    m_CurrentMovingImageMutex.lock();
    // delete:
    for (std::size_t j = 0; j < m_CurrentMovingImages.size(); j++)
    {
      if (m_CurrentMovingImages[j])
        delete m_CurrentMovingImages[j];
    }
    m_CurrentMovingImages.clear();
    m_CurrentMovingImageMutex.unlock();
  }
  catch (itk::ExceptionObject &e)
  {
    return false;
  }

  m_NReg = nreg; // take over

  this->Notify(UPDATE_PARAMETERS); // signal (because initial ones are set)

  return true;
}

template<typename TVolumeComponentType>
bool REG23Model::DeinitializeInternal()
{
  // FIXME: remove observers, too!!!
  return true;
}

template<typename TComponentType>
ITKVTKImage *
REG23Model::CastImage(ITKVTKImage *image, std::vector<std::string> &args)
{
  if (!image || args.size() != 2)
    return NULL;
  ITKVTKImage::ITKImagePointer ibase = image->GetAsITKImage<TComponentType>();
  ITKVTKIMAGE_DOWNCAST(TComponentType, ibase, icast)
  ITKVTKImage *resultImage = NULL;

  // START: helper macro for reducing the lines of code below
#define CAST_CODE_MACRO(TOutputImageType, OutputComponentType) \
  typedef itk::CastImageFilter<OriginalImageType, TOutputImageType> CastFilterType; \
  typename CastFilterType::Pointer caster = CastFilterType::New(); \
  caster->SetInput(icast); \
  try \
  { \
    caster->Update(); \
    TOutputImageType::Pointer outImage = caster->GetOutput(); \
    outImage->DisconnectPipeline(); \
    resultImage = new ITKVTKImage(itk::ImageIOBase::SCALAR, OutputComponentType); \
    resultImage->SetITKImage(static_cast<ITKVTKImage::ITKImageType *>(outImage.GetPointer()), false); \
  } \
  catch (itk::ExceptionObject &e) \
  { \
    delete resultImage; \
    return NULL; \
  }
  // END: helper macro for reducing the lines of code below

  if (args[1] == "CHAR")
  {
    typedef itk::Image<char, ITKVTKImage::Dimensions> OutputImageType;
    CAST_CODE_MACRO(OutputImageType, itk::ImageIOBase::CHAR)
  }
  else if (args[1] == "UCHAR")
  {
    typedef itk::Image<unsigned char, ITKVTKImage::Dimensions> OutputImageType;
    CAST_CODE_MACRO(OutputImageType, itk::ImageIOBase::UCHAR)
  }
  else if (args[1] == "SHORT")
  {
    typedef itk::Image<short, ITKVTKImage::Dimensions> OutputImageType;
    CAST_CODE_MACRO(OutputImageType, itk::ImageIOBase::SHORT)
  }
  else if (args[1] == "USHORT")
  {
    typedef itk::Image<unsigned short, ITKVTKImage::Dimensions> OutputImageType;
    CAST_CODE_MACRO(OutputImageType, itk::ImageIOBase::USHORT)
  }
  else if (args[1] == "INT")
  {
    typedef itk::Image<int, ITKVTKImage::Dimensions> OutputImageType;
    CAST_CODE_MACRO(OutputImageType, itk::ImageIOBase::INT)
  }
  else if (args[1] == "UINT")
  {
    typedef itk::Image<unsigned int, ITKVTKImage::Dimensions> OutputImageType;
    CAST_CODE_MACRO(OutputImageType, itk::ImageIOBase::UINT)
  }
  else if (args[1] == "LONG")
  {
    typedef itk::Image<long, ITKVTKImage::Dimensions> OutputImageType;
    CAST_CODE_MACRO(OutputImageType, itk::ImageIOBase::LONG)
  }
  else if (args[1] == "ULONG")
  {
    typedef itk::Image<unsigned long, ITKVTKImage::Dimensions> OutputImageType;
    CAST_CODE_MACRO(OutputImageType, itk::ImageIOBase::ULONG)
  }
  else if (args[1] == "FLOAT")
  {
    typedef itk::Image<float, ITKVTKImage::Dimensions> OutputImageType;
    CAST_CODE_MACRO(OutputImageType, itk::ImageIOBase::FLOAT)
  }
  else if (args[1] == "DOUBLE")
  {
    typedef itk::Image<double, ITKVTKImage::Dimensions> OutputImageType;
    CAST_CODE_MACRO(OutputImageType, itk::ImageIOBase::DOUBLE)
  }

  return resultImage;
}

template<typename TComponentType>
ITKVTKImage *
REG23Model::CropImage(ITKVTKImage *image, std::vector<std::string> &args)
{
  if (!image || args.size() != 7)
    return NULL;
  ITKVTKImage::ITKImagePointer ibase = image->GetAsITKImage<TComponentType>();
  ITKVTKIMAGE_DOWNCAST(TComponentType, ibase, icast)
  ITKVTKImage *resultImage = NULL;

  typedef itk::ExtractImageFilter<OriginalImageType, OriginalImageType>
  ExtractionFilterType;
  typename ExtractionFilterType::Pointer extractor = ExtractionFilterType::New();
  extractor->SetInput(icast);
  typename OriginalImageType::RegionType ereg;
  typename OriginalImageType::IndexType idx;
  for (int d = 0; d < 3; d++)
    idx[d] = atoi(args[d + 1].c_str());
  ereg.SetIndex(idx);
  typename OriginalImageType::SizeType sz;
  for (int d = 0; d < 3; d++)
    sz[d] = atoi(args[d + 4].c_str());
  ereg.SetSize(sz);
  extractor->SetExtractionRegion(ereg);
  try
  {
    extractor->Update();
    OriginalImagePointer outImage = extractor->GetOutput();
    outImage->DisconnectPipeline();
    typename OriginalImageType::RegionType ireg =
      outImage->GetLargestPossibleRegion();
    typename OriginalImageType::IndexType zidx;
    zidx.Fill(0);
    ireg.SetIndex(zidx);
    outImage->SetRegions(ireg); // set back start index
    // adapt origin:
    typename OriginalImageType::DirectionType dir = outImage->GetDirection();
    typename OriginalImageType::PointType orig = outImage->GetOrigin();
    typename OriginalImageType::SpacingType spac = outImage->GetSpacing();
    for (int d = 0; d < 3; d++)
    {
      double f = (double)idx[d] * spac[d];
      for (int d2 = 0; d2 < 3; d2++)
        orig[d2] += dir[d2][d] * f;
    }
    outImage->SetOrigin(orig);
    resultImage = new ITKVTKImage(itk::ImageIOBase::SCALAR,
                                  image->GetComponentType());
    resultImage->SetITKImage(static_cast<ITKVTKImage::ITKImageType *>(
                               outImage.GetPointer()), false);
  }
  catch (itk::ExceptionObject &e)
  {
    return NULL;
  }

  return resultImage;
}

template<typename TComponentType>
ITKVTKImage *
REG23Model::UnsharpMaskImage(ITKVTKImage *image,
    std::vector<std::string> &args)
{
  if (!image || args.size() < 1)
    return NULL;
  ITKVTKImage::ITKImagePointer ibase = image->GetAsITKImage<TComponentType>();
  ITKVTKIMAGE_DOWNCAST(TComponentType, ibase, icast)
  ITKVTKImage *resultImage = NULL;

  int radius = 0; // 0 ... heuristic radius determination is required
  if (args.size() > 1) // radius value explicitly specified
  {
    radius = atoi(args[1].c_str());
  }

  // (unsharp masking is implemented as VTK algorithm)
  vtkImageData *ivcast = image->GetAsVTKImage<TComponentType>();
  VTKUnsharpMaskingImageFilter *umf = VTKUnsharpMaskingImageFilter::New();
  if (radius > 0)
  {
    umf->SetAutoRadius(false);
    umf->SetRadius(radius);
  }
  else
  {
    umf->SetAutoRadius(true);
  }
  umf->SetInput(ivcast);
  umf->Update();
  if (umf->GetOutput())
  {
    resultImage = new ITKVTKImage(itk::ImageIOBase::SCALAR,
                                  image->GetComponentType());
    resultImage->SetVTKImage(umf->GetOutput(), true);
    typename OriginalImageType::Pointer newCastImage =
        static_cast<OriginalImageType * >(resultImage->GetAsITKImage<TComponentType>().
            GetPointer());
    newCastImage->SetDirection(icast->GetDirection()); // VTK has no direction
    umf->Delete();
    umf = NULL;
  }
  else
  {
    umf->Delete();
    umf = NULL;
    return NULL;
  }

  return resultImage;
}

template<typename TComponentType>
ITKVTKImage *
REG23Model::ResampleImage(ITKVTKImage *image, std::vector<std::string> &args)
{
  if (!image || args.size() < 5)
    return NULL;
  ITKVTKImage::ITKImagePointer ibase = image->GetAsITKImage<TComponentType>();
  ITKVTKIMAGE_DOWNCAST(TComponentType, ibase, icast)
  ITKVTKImage *resultImage = NULL;

  typedef itk::InterpolateImageFunction<OriginalImageType, double> InterpolationType;
  typedef itk::LinearInterpolateImageFunction<OriginalImageType, double>
  LinearInterpolationType;
  typedef itk::BSplineInterpolateImageFunction<OriginalImageType, double>
  BSplineInterpolationType;
  typedef itk::ResampleImageFilter<OriginalImageType, OriginalImageType, double>
  ResampleFilterType;
  typedef itk::Euler3DTransform<double> TransformType;

  typename InterpolationType::Pointer interpolator = NULL;
  if (std::string(args[4]) == std::string("LINEAR"))
  {
    typename LinearInterpolationType::Pointer interp = LinearInterpolationType::New();
    interpolator = interp;
  }
  else // args[4] == "BSPLINE-3"
  {
    typename BSplineInterpolationType::Pointer interp = BSplineInterpolationType::New();
    interp->SetSplineOrder(3);
    interpolator = interp;
  }
  TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();
  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  typename ResampleFilterType::SpacingType spac;
  spac[0] = atof(args[1].c_str());
  spac[1] = atof(args[2].c_str());
  spac[2] = atof(args[3].c_str());
  resampler->SetTransform(transform);
  resampler->SetInterpolator(interpolator);
  resampler->SetOutputSpacing(spac);
  resampler->SetOutputOrigin(icast->GetOrigin());
  resampler->SetOutputDirection(icast->GetDirection());
  typename OriginalImageType::IndexType sidx;
  sidx.Fill(0);
  resampler->SetOutputStartIndex(sidx);
  typename OriginalImageType::SizeType newSize;
  for (int d = 0; d < 3; d++)
  {
    newSize[d] = static_cast<typename OriginalImageType::SizeValueType> (std::floor(
                   icast->GetLargestPossibleRegion().GetSize()[d]
                   * icast->GetSpacing()[d] / spac[d]));
    // Ensure that size is at least 1
    if (newSize[d] <= 0)
      newSize[d] = 1;
  }
  resampler->SetSize(newSize);

  if (args.size() == 6) // optional smoothing-factor
  {
    int r = 0;
    if (args[5] == "ANTIALIASING")
    {
      typename OriginalImageType::SpacingType origSpac = icast->GetSpacing();
      double maxDiff = -1e6, diff = 0;
      double minSpac = 1e6;
      for (int d = 0; d < 3; d++)
      {
        diff = spac[d] - origSpac[d];
        if (diff > maxDiff)
          maxDiff = diff;
        if (origSpac[d] < minSpac)
          minSpac = origSpac[d];
      }
      if (maxDiff > 0) // only if new spacing is bigger than old spacing
        r = ceil(maxDiff / minSpac);
    }
    else
    {
      r = atoi(args[5].c_str());
    }
    if (r > 0)
    {
      typedef itk::MeanImageFilter<OriginalImageType, OriginalImageType>
        GaussianBlurType;
      typename GaussianBlurType::Pointer blurrer = GaussianBlurType::New();
      typename OriginalImageType::SizeType radius;
      radius.Fill(r);
      blurrer->SetRadius(radius);
      blurrer->SetInput(icast);
      try
      {
        blurrer->Update();
        icast = blurrer->GetOutput();
        icast->DisconnectPipeline();
      }
      catch (itk::ExceptionObject &e)
      {
        return NULL;
      }
    }
  }

  resampler->SetInput(icast);
  try
  {
    resampler->Update();
    OriginalImagePointer outImage = resampler->GetOutput();
    outImage->DisconnectPipeline();
    resultImage = new ITKVTKImage(itk::ImageIOBase::SCALAR,
                                  image->GetComponentType());
    resultImage->SetITKImage(static_cast<ITKVTKImage::ITKImageType *>(
                               outImage.GetPointer()), false);
  }
  catch (itk::ExceptionObject &e)
  {
    return NULL;
  }

  return resultImage;
}

template<typename TComponentType>
ITKVTKImage *
REG23Model::RescaleImage(ITKVTKImage *image, std::vector<std::string> &args,
                         int rescaleMode, ITKVTKImage *imageMask)
{
  if (!image)
    return NULL;
  if (rescaleMode < 0 || rescaleMode > 3)
    return NULL;
  if ((rescaleMode == 0 && args.size() != 3) ||
      (rescaleMode == 1 && args.size() != 3) ||
      (rescaleMode == 2 && args.size() != 5) ||
      (rescaleMode == 3 && (args.size() != 3 || !imageMask)))
    return NULL;
  ITKVTKImage::ITKImagePointer ibase = image->GetAsITKImage<TComponentType>();
  ITKVTKIMAGE_DOWNCAST(TComponentType, ibase, icast)
  ITKVTKImage *resultImage = NULL;

  typedef itk::RescaleIntensityImageFilter<OriginalImageType, OriginalImageType>
  RescaleFilterType1;
  typedef itk::ShiftScaleImageFilter<OriginalImageType, OriginalImageType>
  RescaleFilterType2;
  typedef itk::IntensityWindowingImageFilter<OriginalImageType, OriginalImageType>
  RescaleFilterType3;
  typedef itk::ImageToImageFilter<OriginalImageType, OriginalImageType> FilterType;
  typename FilterType::Pointer filter = NULL;
  if (rescaleMode == 0)
  {
    typename RescaleFilterType1::Pointer r = RescaleFilterType1::New();
    r->SetOutputMinimum(atof(args[1].c_str()));
    r->SetOutputMaximum(atof(args[2].c_str()));
    filter = r;
  }
  else if (rescaleMode == 1)
  {
    typename RescaleFilterType2::Pointer r = RescaleFilterType2::New();
    r->SetShift(atof(args[1].c_str()));
    r->SetScale(atof(args[2].c_str()));
    filter = r;
  }
  else if (rescaleMode == 2)
  {
    typename RescaleFilterType3::Pointer r = RescaleFilterType3::New();
    r->SetWindowMinimum(atof(args[1].c_str()));
    r->SetWindowMaximum(atof(args[2].c_str()));
    r->SetOutputMinimum(atof(args[3].c_str()));
    r->SetOutputMaximum(atof(args[4].c_str()));
    filter = r;
  }
  else // if (rescaleMode == 3)
  {
    // The mask is cast to INT regardless of its original type
    typedef itk::Image<int, ITKVTKImage::Dimensions> MaskImageType;
    typedef typename MaskImageType::Pointer MaskImagePointer;

    ITKVTKImage *imageMaskCastTEMP = NULL;
    std::vector<std::string> castArgs;
    castArgs.push_back("CAST");
    castArgs.push_back("INT");
    TEMPLATE_CALL_COMP(imageMask->GetComponentType(),
        imageMaskCastTEMP = CastImage, imageMask, castArgs)
    MaskImagePointer imageMaskCast = static_cast<MaskImageType *>
        (imageMaskCastTEMP->GetAsITKImage<int> ().GetPointer());
    delete (imageMaskCastTEMP);

    // Check if mask size matches image size
    MaskImageType::SizeType maskSize =
        imageMaskCast->GetLargestPossibleRegion().GetSize();
    typename OriginalImageType::SizeType imageSize =
        icast->GetLargestPossibleRegion().GetSize();
    if (maskSize != imageSize)
    {
      // Wrong mask size. TODO: Warning/Logging
      return NULL;
    }

    // Get min/max intenities inside mask area
    typedef itk::ImageRegionConstIterator<MaskImageType> MaskIteratorType;
    typedef itk::ImageRegionConstIterator<OriginalImageType> ImageIteratorType;
    MaskIteratorType itMask(imageMaskCast,
        imageMaskCast->GetLargestPossibleRegion());
    ImageIteratorType itImage(icast, icast->GetLargestPossibleRegion());
    double maskMin = std::numeric_limits<double>::max();
    double maskMax = -std::numeric_limits<double>::max();

    itImage = itImage.Begin();
    unsigned int counter = 0;
    for (itMask = itMask.Begin(); !itMask.IsAtEnd(); ++itMask, ++itImage)
    {
      if (itMask.Value())
      {
        maskMin = std::min(maskMin, (double) itImage.Value());
        maskMax = std::max(maskMax, (double) itImage.Value());
        ++counter;
      }
    }

    if (counter == 0)
    {
      // Mask image is empty: Makes no sense to rescale
      return NULL;
    }

    // Rescale
    typename RescaleFilterType3::Pointer r = RescaleFilterType3::New();
    r->SetWindowMinimum(maskMin);
    r->SetWindowMaximum(maskMax);
    r->SetOutputMinimum(atof(args[1].c_str()));
    r->SetOutputMaximum(atof(args[2].c_str()));
    filter = r;
  }
  filter->SetInput(icast);
  try
  {
    filter->Update();
    OriginalImagePointer outImage = filter->GetOutput();
    outImage->DisconnectPipeline();
    resultImage = new ITKVTKImage(itk::ImageIOBase::SCALAR,
                                  image->GetComponentType());
    resultImage->SetITKImage(static_cast<ITKVTKImage::ITKImageType *>(
                               outImage.GetPointer()), false);
  }
  catch (itk::ExceptionObject &e)
  {
    return NULL;
  }

  return resultImage;
}

template<typename TComponentType>
ITKVTKImage *
REG23Model::ThresholdImageData(ITKVTKImage *image, std::vector<std::string> &args)
{
  if (!image || args.size() < 3)
    return NULL;
  ITKVTKImage::ITKImagePointer ibase = image->GetAsITKImage<TComponentType>();
  ITKVTKIMAGE_DOWNCAST(TComponentType, ibase, icast)
  ITKVTKImage *resultImage = NULL;

  typedef itk::ThresholdImageFilter<OriginalImageType> ThreshImageFilterType;
  typename ThreshImageFilterType::Pointer threshFilter = ThreshImageFilterType::New ();
  threshFilter->SetInput(icast);

  if (args[0] == "THRESHOLDABOVE")
  {
    if (args.size() != 3)
      return NULL;
    threshFilter->ThresholdAbove(atof(args[1].c_str()));
    threshFilter->SetOutsideValue(atof(args[2].c_str()));
  }
  else if (args[0] == "THRESHOLDBELOW")
  {
    if (args.size() != 3)
      return NULL;
    threshFilter->ThresholdBelow(atof(args[1].c_str()));
    threshFilter->SetOutsideValue(atof(args[2].c_str()));
  }
  else if (args[0] == "THRESHOLDOUTSIDE")
  {
    if (args.size() != 4)
      return NULL;
    threshFilter->ThresholdOutside(atof(args[1].c_str()), atof(args[2].c_str()));
    threshFilter->SetOutsideValue(atof(args[3].c_str()));
  }
  else
  {
    return NULL;
  }

  try
  {
    threshFilter->Update();
    OriginalImagePointer outImage = threshFilter->GetOutput();
    outImage->DisconnectPipeline();
    resultImage = new ITKVTKImage(itk::ImageIOBase::SCALAR,
                                  image->GetComponentType());
    resultImage->SetITKImage(static_cast<ITKVTKImage::ITKImageType *>(
                               outImage.GetPointer()), false);
  }
  catch (itk::ExceptionObject &e)
  {
    return NULL;
  }

  return resultImage;
}

template<typename TComponentType>
ITKVTKImage *
REG23Model::FlipImageData(ITKVTKImage *image, std::vector<std::string> &args)
{
  if (!image || args.size() < 2)
    return NULL;
  ITKVTKImage::ITKImagePointer ibase = image->GetAsITKImage<TComponentType>();
  ITKVTKIMAGE_DOWNCAST(TComponentType, ibase, icast)
  ITKVTKImage *resultImage = NULL;


  typedef itk::FlipImageFilter<OriginalImageType> FlipImageFilterType;
  typename FlipImageFilterType::Pointer flipFilter = FlipImageFilterType::New ();
  flipFilter->SetInput(icast);

  typename FlipImageFilterType::FlipAxesArrayType flipAxes;
  flipAxes.Fill(false);
  if (args[1] == "X")
    flipAxes[0] = true;
  if (args[1] == "Y")
    flipAxes[1] = true;

  flipFilter->SetFlipAxes(flipAxes);

  typedef itk::ChangeInformationImageFilter<OriginalImageType> ChangeFilterType;
  typename ChangeFilterType::Pointer changeFilter = ChangeFilterType::New();
  changeFilter->ChangeNone();
  changeFilter->ChangeDirectionOn();
  changeFilter->ChangeOriginOn();
  changeFilter->SetOutputDirection(icast->GetDirection());
  changeFilter->SetOutputOrigin(icast->GetOrigin());
  changeFilter->SetInput(flipFilter->GetOutput());

  try
  {
    changeFilter->Update();
    OriginalImagePointer outImage = changeFilter->GetOutput();
    outImage->DisconnectPipeline();
    resultImage = new ITKVTKImage(itk::ImageIOBase::SCALAR,
                                  image->GetComponentType());
    resultImage->SetITKImage(static_cast<ITKVTKImage::ITKImageType *>(
                               outImage.GetPointer()), false);
  }
  catch (itk::ExceptionObject &e)
  {
    return NULL;
  }

  return resultImage;
}

template<typename TComponentType>
ITKVTKImage *
REG23Model::PreProcessImage(ITKVTKImage::ITKImagePointer image,
                            std::string configBaseKey, bool &error,
                            ITKVTKImageMetaInformation::Pointer metaInfo)
{
// convenience macro for updating mean, variance, min, max of current image:
#define UPDATE_IMAGE_STATISTICS(itkvtkImage) \
  { \
    vtkImageData *newImage = NULL; \
    TEMPLATE_CALL_COMP(itkvtkImage->GetComponentType(), \
        newImage = itkvtkImage->GetAsVTKImage) \
    vtkSmartPointer<vtkImageCast> cast = vtkSmartPointer<vtkImageCast>::New(); \
    cast->SetOutputScalarTypeToFloat(); \
    cast->SetInput(newImage); \
    vtkImageData *newCastImage = cast->GetOutput(); \
    cast->Update(); \
    ExtractMeanAndVarianceFromImage(newCastImage, prevMean, prevSTD); \
    prevSTD = sqrt(prevSTD); \
    double sr[2]; \
    newImage->GetScalarRange(sr); \
    prevMin = sr[0]; \
    prevMax = sr[1]; \
  }

  if (!m_Config || !m_ValidConfiguration || !image || !metaInfo)
  {
    error = true;
    return NULL;
  }
  error = false;
  bool isFixedImage = (configBaseKey.substr(0, 10) == "FixedImage");
  ITKVTKImage *lastImage = new ITKVTKImage(itk::ImageIOBase::SCALAR,
                           ITKVTKImage::GetComponentTypeFromTemplate<TComponentType>());
  lastImage->SetITKImage(image, false);
  lastImage->SetMetaInfo(metaInfo);
  int i = 1;
  std::string s, key, emsg;
  std::vector<std::string> args;
  ITKVTKImage *pimage = NULL;
  double prevMean = 0, prevSTD = 0, prevMin = 0, prevMax = 0;
  double *pwl = NULL;
  if (isFixedImage)
  {
    int idx = atoi(configBaseKey.substr(10, configBaseKey.length()).c_str()) - 1;
    if (m_ExplicitInitialWLs[idx]) // have explicit window/level for this image!
    {
      pwl = m_ExplicitInitialWLs[idx];

      UPDATE_IMAGE_STATISTICS(lastImage)
    }
  }
  do
  {
    key = configBaseKey + ".PreProcess" + StreamConvert(i);
    s = TrimF(m_Config->ReadString("Images", key, ""));
    if (s.length() == 0) // all pre-processing steps done
    {
      if (isFixedImage &&
          lastImage && lastImage->GetComponentType() != itk::ImageIOBase::FLOAT)
      {
        s = "Cast(FLOAT)"; // -> virtual float cast for fixed images
      }
    }
    if (s.length() > 0)
    {
      if (ParseImageProcessingEntry(s, emsg, args, CAST | CROP | RESAMPLE |
          RESCALEMINMAX | RESCALESHIFTSCALE | RESCALEWINDOWING | STORE |
          UNSHARPMASKING | FLIP | THRESHOLD_ABOVE | THRESHOLD_BELOW |
          THRESHOLD_OUTSIDE) && args.size() > 0)
      {
        pimage = NULL;
        if (args[0] == "CAST")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = CastImage, lastImage, args)
        }
        else if (args[0] == "CROP")
        {
          // automatic W/L correction is in general not possible!
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = CropImage, lastImage, args)

           if (pwl) // -> adapt to mean / variance
           {
             // extract old min/max WL positions relative to mean and var:
             double fmi = (prevMean - pwl[0]) / prevSTD;
             double fma = (pwl[1] - prevMean) / prevSTD;

             UPDATE_IMAGE_STATISTICS(pimage)

             // adapt WL to new mean and var:
             pwl[0] = prevMean - fmi * prevSTD;
             pwl[1] = prevMean + fma * prevSTD;
           }
        }
        else if (args[0] == "RESAMPLE")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = ResampleImage, lastImage, args)
          if (pwl) // -> adapt to mean / variance
          {
            // extract old min/max WL positions relative to mean and var:
            double fmi = (prevMean - pwl[0]) / prevSTD;
            double fma = (pwl[1] - prevMean) / prevSTD;

            UPDATE_IMAGE_STATISTICS(pimage)

            // adapt WL to new mean and var:
            pwl[0] = prevMean - fmi * prevSTD;
            pwl[1] = prevMean + fma * prevSTD;
          }
        }
        else if (args[0] == "FLIP")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = FlipImageData, lastImage, args)
        }
        else if (args[0] == "THRESHOLDABOVE" ||
            args[0] == "THRESHOLDBELOW" ||
            args[0] == "THRESHOLDOUTSIDE")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = ThresholdImageData, lastImage, args)
        }
        else if (args[0] == "RESCALEMINMAX")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = RescaleImage, lastImage, args, 0)
          if (pwl)
          {
            // adapt WL to new intensity range (linearly):
            double fmi = (pwl[0] - prevMin) / (prevMax - prevMin);
            double fma = (pwl[1] - prevMin) / (prevMax - prevMin);
            double mi = atof(args[1].c_str());
            double ma = atof(args[2].c_str());
            pwl[0] = mi + fmi * (ma - mi);
            pwl[1] = mi + fma * (ma - mi);

            UPDATE_IMAGE_STATISTICS(pimage)
          }
        }
        else if (args[0] == "RESCALESHIFTSCALE")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = RescaleImage, lastImage, args, 1)
          if (pwl)
          {
            // adapt WL to new intensity range (linearly):
            double sh = atof(args[1].c_str());
            double sc = atof(args[2].c_str());
            pwl[0] = (sh + pwl[0]) * sc;
            pwl[1] = (sh + pwl[1]) * sc;

            UPDATE_IMAGE_STATISTICS(pimage)
          }
        }
        else if (args[0] == "RESCALEWINDOWING")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = RescaleImage, lastImage, args, 2)

          if (pwl)
          {
            UPDATE_IMAGE_STATISTICS(pimage)

            // -> span across whole intensity-range as windowing already
            // window-levels!!!
            pwl[0] = prevMin;
            pwl[1] = prevMax;
          }
        }
        else if (args[0] == "UNSHARPMASKING")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = UnsharpMaskImage, lastImage, args)

           if (pwl)
           {
             UPDATE_IMAGE_STATISTICS(pimage)

             // unsharp masking changes the histogram, but the intensity range
             // does usually not change dramatically; so simply keep the
             // previous window/level settings which may be more attractive than
             // spanning to the whole intensity range:
           }
        }
        else if (args[0] == "STORE")
        {
          // lastImage should have metaInfo set!
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
              lastImage->SaveImageAsORAMetaImage, args[1], false, "")
        }

        if (args[0] != "STORE")
        {
          if (lastImage)
            delete lastImage;
          lastImage = NULL;
          if (!pimage)
          {
            error = true;
            return NULL;
          }
          lastImage = pimage;
          lastImage->SetMetaInfo(metaInfo);
        }
      }
      else
      {
        error = true;
        return NULL;
      }
    }
    i++;
  }
  while (s.length() > 0);

  return lastImage;
}

template<typename TComponentType>
ITKVTKImage *
REG23Model::PostProcessImage(ITKVTKImage::ITKImagePointer image,
                            std::string configBaseKey, bool &error,
                            ITKVTKImage *imageMask,
                            ITKVTKImageMetaInformation::Pointer metaInfo)
{
  if (!m_Config || !m_ValidConfiguration || !image || !metaInfo)
  {
    error = true;
    return NULL;
  }
  error = false;
  ITKVTKImage *lastImage = new ITKVTKImage(itk::ImageIOBase::SCALAR,
                           ITKVTKImage::GetComponentTypeFromTemplate<TComponentType>());
  lastImage->SetITKImage(image, false);
  lastImage->SetMetaInfo(metaInfo);
  int i = 1;
  std::string s, key, emsg;
  std::vector<std::string> args;
  ITKVTKImage *pimage = NULL;
  do
  {
    key = configBaseKey + ".PostProcess" + StreamConvert(i);
    s = TrimF(m_Config->ReadString("Images", key, ""));
    if (s.length() == 0) // all post-processing steps done
    {
      if (configBaseKey.substr(0, 10) == "FixedImage" &&
          lastImage && lastImage->GetComponentType() != itk::ImageIOBase::FLOAT)
      {
        s = "Cast(FLOAT)"; // -> virtual float cast for fixed images
      }
    }
    if (s.length() > 0)
    {
      if (ParseImageProcessingEntry(s, emsg, args, CAST | RESCALEMINMAX |
          RESCALESHIFTSCALE | RESCALEWINDOWING | STORE | RESCALEMINMAXMASK |
          UNSHARPMASKING) && args.size() > 0)
      {
        pimage = NULL;
        if (args[0] == "CAST")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = CastImage, lastImage, args)
        }
        else if (args[0] == "RESCALEMINMAX")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                pimage = RescaleImage, lastImage, args, 0)
        }
        else if (args[0] == "RESCALEMINMAXMASK")
        {
          if (!imageMask)
          {
            // No mask: same as RESCALEMINMAX
            TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                pimage = RescaleImage, lastImage, args, 0)
          }
          else
          {
            TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                pimage = RescaleImage, lastImage, args, 3, imageMask)
          }
        }
        else if (args[0] == "RESCALESHIFTSCALE")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = RescaleImage, lastImage, args, 1)
        }
        else if (args[0] == "RESCALEWINDOWING")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = RescaleImage, lastImage, args, 2)
        }
        else if (args[0] == "UNSHARPMASKING")
        {
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
                             pimage = UnsharpMaskImage, lastImage, args)
        }
        else if (args[0] == "STORE")
        {
          // lastImage should have metaInfo set!
          TEMPLATE_CALL_COMP(lastImage->GetComponentType(),
              lastImage->SaveImageAsORAMetaImage, args[1], false, "")
        }

        if (args[0] != "STORE")
        {
          if (lastImage)
            delete lastImage;
          lastImage = NULL;
          if (!pimage)
          {
            error = true;
            return NULL;
          }
          lastImage = pimage;
          lastImage->SetMetaInfo(metaInfo);
        }
      }
      else
      {
        error = true;
        return NULL;
      }
    }
    i++;
  }
  while (s.length() > 0);

  return lastImage;
}

template<typename TVolumeComponentType>
bool REG23Model::ExecuteRegistration()
{
  m_CurrentIteration = 0;
  m_BestMeasure = 1e10; // very big value!
  if (!IsReadyForAutoRegistration())
    return false;

  typedef TVolumeComponentType VolumePixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
      DRR2DImageType, VolumeImageType> RegistrationType;

  typename RegistrationType::Pointer cnreg =
      static_cast<RegistrationType *>(m_NReg.GetPointer());

  bool succ = true;
  try
  {
    m_RegistrationIsRunning = true;
    InitFunctionTable();
    cnreg->SetInitialTransformParameters(m_CurrentParameters);
    cnreg->Modified(); // be sure!
    succ &= cnreg->Initialize(); // do initialization again ...
    if (succ)
    {
      // re-configure the metric components with properties from the initial
      // DRRs if necessary:
      InitializeDynamicMetricProperties<TVolumeComponentType>();
      // do the registration:
      cnreg->Update();
    }
  }
  catch (itk::ExceptionObject &e)
  {
    succ = false;
  }
  m_RegistrationIsRunning = false;

  return succ;
}

template<typename TVolumeComponentType>
void REG23Model::InitializeDynamicMetricProperties()
{
  typedef TVolumeComponentType VolumePixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
      DRR2DImageType, VolumeImageType> RegistrationType;
  typedef typename RegistrationType::DRR3DImageType DRR3DImageType;
  typedef itk::ImageRegionConstIterator<DRR3DImageType> DRR3DIteratorType;

  typename RegistrationType::Pointer cnreg =
      static_cast<RegistrationType *>(m_NReg.GetPointer());

  for (std::size_t i = 0; i < cnreg->GetNumberOfFixedImages(); i++)
  {
    // check whether or not we need to extract the initial DRR:
    std::string key = "MetricConfig" + StreamConvert(i + 1);
    std::string s = ToLowerCaseF(m_Config->ReadString("Registration", key, ""));
    bool analyzeImage = ParseEntryHasVariable(s);
    if (!analyzeImage)
      continue;

    typename DRR3DImageType::Pointer drr = cnreg->Compute3DTestProjection(i, 0);
    DRR3DIteratorType it(drr, drr->GetLargestPossibleRegion());
    double minIntensity = 1e16;
    double maxIntensity = -1e16;
    double meanIntensity = 0;
    double intensityVariance = 0;
    int c = 0;
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      c++;
      DRRPixelType v = it.Get();
      if (v < minIntensity)
        minIntensity = v;
      if (v > maxIntensity)
        maxIntensity = v;
      meanIntensity += static_cast<double>(v);
    }
    meanIntensity /= (double)c;
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      double v = static_cast<double>(it.Get());
      intensityVariance += ((v - meanIntensity) * (v - meanIntensity));
    }
    intensityVariance /= (double)(c - 1);

    // Masked variables
    double minIntensityMask = std::numeric_limits<double>::max();
    double maxIntensityMask = -std::numeric_limits<double>::max();
    double meanIntensityMask = 0;
    double intensityVarianceMask = 0;

    ITKVTKImage *imageMask = m_MaskImages[i]->ProduceImage();
    bool useSameAsUnmasked = false;
    if (imageMask)
    {
      // The mask is cast to INT regardless of its original type
      typedef itk::Image<int, ITKVTKImage::Dimensions> MaskImageType;
      typedef typename MaskImageType::Pointer MaskImagePointer;

      ITKVTKImage *imageMaskCastTEMP = NULL;
      std::vector<std::string> castArgs;
      castArgs.push_back("CAST");
      castArgs.push_back("INT");
      TEMPLATE_CALL_COMP(imageMask->GetComponentType(),
          imageMaskCastTEMP = CastImage, imageMask, castArgs)
      MaskImagePointer imageMaskCast = static_cast<MaskImageType *>
          (imageMaskCastTEMP->GetAsITKImage<int> ().GetPointer());
      delete (imageMaskCastTEMP);

      // Check if mask size matches image size
      MaskImageType::SizeType maskSize =
          imageMaskCast->GetLargestPossibleRegion().GetSize();
      typename DRR3DImageType::SizeType imageSize =
          drr->GetLargestPossibleRegion().GetSize();
      if (maskSize != imageSize)
      {
        // Wrong mask size. TODO: Warning/Logging
        useSameAsUnmasked = true;
      }

      // Get min/max intenities inside mask area
      typedef itk::ImageRegionConstIterator<MaskImageType> MaskIteratorType;
      MaskIteratorType itMask(imageMaskCast,
          imageMaskCast->GetLargestPossibleRegion());

      it= it.Begin();
      int counter = 0;
      for (itMask = itMask.Begin(); !itMask.IsAtEnd(); ++itMask, ++it)
      {
        if (itMask.Value())
        {
          ++counter;
          minIntensityMask = std::min(minIntensityMask, (double) it.Value());
          maxIntensityMask = std::max(maxIntensityMask, (double) it.Value());
          meanIntensityMask += static_cast<double>(it.Value());
        }
      }
      if (counter <= 1)
      {
        // Mask is empty (no or only one pixelvalue != 0) and prevent division by zero
        useSameAsUnmasked = true;
      }
      else
      {
        meanIntensityMask /= (double) counter;
        it = it.Begin();
        for (itMask = itMask.Begin(); !itMask.IsAtEnd(); ++itMask, ++it)
        {
          if (itMask.Value())
          {
            double v = static_cast<double> (it.Value());
            intensityVarianceMask += ((v - meanIntensityMask) * (v
                - meanIntensityMask));
          }
        }
        intensityVarianceMask /= (double) (counter - 1);
      }
    }

    if (!imageMask || useSameAsUnmasked)
    {
      // No mask set or an error occured at masking
      // -> Use same as unmasked
      minIntensityMask = minIntensity;
      maxIntensityMask = maxIntensity;
      meanIntensityMask = meanIntensity;
      intensityVarianceMask = intensityVariance;
    }

    vtkSmartPointer<vtkFunctionParser> parser =
        vtkSmartPointer<vtkFunctionParser>::New();
    parser->RemoveAllVariables();
    parser->SetScalarVariableValue("drrmin", minIntensity);
    parser->SetScalarVariableValue("drrmax", maxIntensity);
    parser->SetScalarVariableValue("drrmean", meanIntensity);
    parser->SetScalarVariableValue("drrstd", intensityVariance);
    parser->SetScalarVariableValue("drrminmask", minIntensityMask);
    parser->SetScalarVariableValue("drrmaxmask", maxIntensityMask);
    parser->SetScalarVariableValue("drrmeanmask", meanIntensityMask);
    parser->SetScalarVariableValue("drrstdmask", intensityVarianceMask);

    std::vector<std::string> v;
    Tokenize(s, v, " ");
    s = "";
    for (std::size_t j = 0; j < v.size(); j++)
    {
      if (ParseEntryHasVariable(v[j]))
      {
        parser->SetFunction(v[j].c_str());
        s += " " + StreamConvert(parser->GetScalarResult());
      }
      else
      {
        s += " " + v[j];
      }
    }
    Trim(s);

    v.clear();
    key = "MetricType" + StreamConvert(i + 1);
    v.push_back(ToUpperCaseF(m_Config->ReadString("Registration", key, "")));
    Tokenize(s, v, " ");
    std::string prefix = "", postfix = "";
    ConfigureMetricComponent<TVolumeComponentType>(cnreg->GetMetric()->
        GetIthMetricInput(i).GetPointer(), v, prefix, postfix);
    cnreg->GetMetric()->GetIthMetricInput(i)->Initialize(); // initialize again!
  }

}

template<typename TVolumeComponentType>
void REG23Model::UpdateCurrentRegistrationParameters(bool stopRegistration,
    bool forceImagesUpdate)
{
  if (!m_RegistrationIsRunning)
    return;

  typedef TVolumeComponentType VolumePixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef itk::Image<DRRPixelType, 3> DRR3DImageType;
  typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
      DRR2DImageType, VolumeImageType> RegistrationType;
  typedef itk::OnePlusOneEvolutionaryOptimizer EvolOptimizerType;
  typedef itk::AmoebaOptimizerEx AmoebaOptimizerType;
  typedef itk::PowellOptimizerEx PowellOptimizerType;
  typedef itk::CastImageFilter<DRR2DImageType, DRR3DImageType> CastFilterType;

  typename RegistrationType::Pointer cnreg =
      static_cast<RegistrationType *>(m_NReg.GetPointer());

  m_CurrentParameters = m_Transform->GetParameters();

  if (stopRegistration)
  {
    cnreg->StopRegistration(); // has effect on multi-resolution only
    // -> optimizer-specific stopping, unfortunately, see below
  }
  if (m_CurrentOptimizerTypeID == "1+1EVOL")
  {
    EvolOptimizerType *opt = dynamic_cast<EvolOptimizerType *>(cnreg->GetOptimizer());
    m_CurrentOptimizationMeasure = opt->GetValue();
    m_CurrentOptimizationParameters = opt->GetCurrentPosition();
    if (stopRegistration)
      opt->StopOptimization();
  }
  else if (m_CurrentOptimizerTypeID == "AMOEBA")
  {
    AmoebaOptimizerType *opt = dynamic_cast<AmoebaOptimizerType *>(cnreg->GetOptimizer());
    m_CurrentOptimizationMeasure = opt->GetCachedValue();
    m_CurrentOptimizationParameters = opt->GetCachedCurrentPosition();
    if (stopRegistration)
     opt->StopOptimization();
  }
  else if (m_CurrentOptimizerTypeID == "POWELL")
  {
    PowellOptimizerType *opt = dynamic_cast<PowellOptimizerType *>(cnreg->GetOptimizer());
    m_CurrentOptimizationMeasure = opt->GetValue();
    m_CurrentOptimizationParameters = opt->GetCurrentPosition();
    if (stopRegistration)
     opt->StopOptimization();
  }


  m_LastMetricMeasure = cnreg->GetLastMetricValue();

  this->Notify(UPDATE_PARAMETERS); // notify registered views!

  // for cost function evolution tracking:
  if (m_RenderCostFunction && m_CurrentIteration >= 1)
  {
    m_FunctionValuesMutex.lock();
    QPointF mp;
    // cost function:
    mp.setX(m_CurrentIteration);
    mp.setY(m_LastMetricMeasure);
    m_FunctionValues[0].push_back(mp);
    // transformation parameters:
    const double r2d = 1. / vtkMath::Pi() * 180.;
    for (unsigned int xx = 1; xx <= m_CurrentParameters.GetSize(); xx++)
    {
      if (xx <= 3) // rotation -> in degrees!
        mp.setY(m_CurrentParameters[xx - 1] * r2d);
      else // translation -> in cm
        mp.setY(m_CurrentParameters[xx - 1] / 10.);
      m_FunctionValues[xx].push_back(mp);
    }
    m_FunctionValuesMutex.unlock();

    this->Notify(UPDATE_COST_FUNCTION); // notify registered views!
  }

  // for overlay / DRR visualization:
  bool doUpdate = false;
  if (!m_UpdateImagesAsSoonAsBetter)
  {
    int modulo = (m_UpdateImagesModulo < 1) ? 1 : m_UpdateImagesModulo;
    doUpdate = (m_CurrentIteration == 1 || m_CurrentIteration % modulo == 0);
  }
  else if (m_CurrentIteration >= 1) // && m_UpdateImagesAsSoonAsBetter
  {
    // -> only if value becomes better (we always minimize):
    if (m_CurrentOptimizationMeasure < m_BestMeasure)
    {
      doUpdate = true;
      m_BestMeasure = m_CurrentOptimizationMeasure; // store
    }
  }
  if ((m_UpdateImagesPending || doUpdate) && m_UpdateImagesMaxFrameRate > 0.)
  {
    itk::RealTimeClock::Pointer timer = itk::RealTimeClock::New();
    double diff = timer->GetTimeStamp() - m_LastUpdateImagesTime;
    if (diff <= 0)
      diff = 1e-6;
    if (1 / diff <= m_UpdateImagesMaxFrameRate)
    {
      if (m_UpdateImagesPending)
        doUpdate = true;
      m_UpdateImagesPending = false; // set back, now allowed ...
      m_LastUpdateImagesTime = timer->GetTimeStamp(); // ... and update time!
    }
    else
    {
      m_UpdateImagesPending = true; // signal that we are waiting ...
      doUpdate = false; // ..., but prohibit update now!
    }
  }
  if (forceImagesUpdate)
    doUpdate = true;
  if (doUpdate)
  {
    m_CurrentMovingImageMutex.lock();
    // make current moving images available for observers in the Update-event:
    typename CastFilterType::Pointer caster = CastFilterType::New();
    for (std::size_t i = 0; i < cnreg->GetNumberOfFixedImages(); i++)
    {
      ITKVTKImage *ivi = new ITKVTKImage(itk::ImageIOBase::SCALAR,
          itk::ImageIOBase::FLOAT);
      caster->SetInput(cnreg->GetMetric()->GetIthMetricInput(i)->GetMovingImage());
      try
      {
        caster->UpdateLargestPossibleRegion();
      }
      catch (itk::ExceptionObject &e)
      {
        for (std::size_t j = 0; j < m_CurrentMovingImages.size(); j++)
        {
          if (m_CurrentMovingImages[j])
            delete m_CurrentMovingImages[j];
        }
        m_CurrentMovingImages.clear();
        break;
      }
      ivi->SetITKImage(caster->GetOutput(), true); // convert to VTK-format
      m_CurrentMovingImages.push_back(ivi); // must delete externally!
    }
    m_CurrentMovingImageMutex.unlock();

    // let the external observer access (e.g. copy) the images if desired:
    if (m_CurrentMovingImages.size() > 0)
    {
      this->Notify(UPDATE_MOVING_IMAGES);
    }

    m_CurrentMovingImageMutex.lock();
    // delete:
    for (std::size_t j = 0; j < m_CurrentMovingImages.size(); j++)
    {
      if (m_CurrentMovingImages[j])
        delete m_CurrentMovingImages[j];
    }
    m_CurrentMovingImages.clear();
    m_CurrentMovingImageMutex.unlock();
  }
}

template<typename TVolumeComponentType>
void REG23Model::ComputeCurrentMovingImages()
{
  if (!IsReadyForManualRegistration())
    return;

  typedef TVolumeComponentType VolumePixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef itk::Image<DRRPixelType, 3> DRRImageType;
  typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
      DRR2DImageType, VolumeImageType> RegistrationType;

  typename RegistrationType::Pointer cnreg =
      static_cast<RegistrationType *>(m_NReg.GetPointer());

  // generate current DRRs and offer them to the viewers:
  m_CurrentMovingImageMutex.lock();
  // make current moving images available for observers in the Update-event:
  for (std::size_t i = 0; i < cnreg->GetNumberOfFixedImages(); i++)
  {
    ITKVTKImage *ivi = new ITKVTKImage(itk::ImageIOBase::SCALAR,
        itk::ImageIOBase::FLOAT);
    // for 1st level:
    DRRImageType::Pointer drr = cnreg->Compute3DTestProjection(i, 0);
    if (!drr)
    {
      for (std::size_t j = 0; j < m_CurrentMovingImages.size(); j++)
      {
        if (m_CurrentMovingImages[j])
          delete m_CurrentMovingImages[j];
      }
      m_CurrentMovingImages.clear();
      break;
    }
    ivi->SetITKImage(drr.GetPointer(), true); // convert to VTK-format
    m_CurrentMovingImages.push_back(ivi); // must delete externally!
  }
  m_CurrentMovingImageMutex.unlock();

  // let the external observer access (e.g. copy) the images if desired:
  if (m_CurrentMovingImages.size() > 0)
    this->Notify(UPDATE_CURRENT_MOVING_IMAGES);

  m_CurrentMovingImageMutex.lock();
  // delete:
  for (std::size_t j = 0; j < m_CurrentMovingImages.size(); j++)
  {
    if (m_CurrentMovingImages[j])
      delete m_CurrentMovingImages[j];
  }
  m_CurrentMovingImages.clear();
  m_CurrentMovingImageMutex.unlock();
}

template<typename TVolumeComponentType>
bool REG23Model::GenerateAndSaveDRRs(std::string outputDir,
    std::string drrPattern, int outputMode)
{
  if (!itksys::SystemTools::FileExists(outputDir.c_str()) ||
      drrPattern.length() <= 0 || outputMode < 0 || outputMode > 3
      || !IsReadyForManualRegistration())
    return false;

  typedef TVolumeComponentType VolumePixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef itk::Image<DRRPixelType, 3> DRRImageType;
  typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
      DRR2DImageType, VolumeImageType> RegistrationType;

  typename RegistrationType::Pointer nreg =
      static_cast<RegistrationType *>(m_NReg.GetPointer());

  std::string baseFN = outputDir;
  EnsureStringEndsWith(baseFN, DSEP);
  baseFN += drrPattern;
  char fbuff[2048];

  if (outputMode == 0) // 3D FLOAT
  {
    typedef itk::ImageFileWriter<DRRImageType> WriterType;

    WriterType::Pointer w = WriterType::New();
    DRRImageType::Pointer drr = NULL;
    for (std::size_t fi = 0; fi < m_PPFixedImages.size(); fi++)
    {
      drr = nreg->Compute3DTestProjection(fi, 0);
      if (!drr)
        return false;
      sprintf(fbuff, baseFN.c_str(), (fi + 1)); // insert view index (%d)
      w->SetFileName(fbuff);
      w->SetInput(drr);
      try
      {
        w->Update();
      }
      catch (itk::ExceptionObject &e)
      {
        std::cerr << "DRR storage error: " << e << std::endl;
        return false;
      }
    }
    drr = NULL;
    w = NULL;
  }
  else // 2D
  {
    typedef itk::Image<unsigned short, 2> USHORTDRR2DImageType;
    typedef itk::Image<unsigned char, 2> UCHARDRR2DImageType;
    typedef itk::ImageFileWriter<DRR2DImageType> FLOATWriterType;
    typedef itk::ImageFileWriter<USHORTDRR2DImageType> USHORTWriterType;
    typedef itk::ImageFileWriter<UCHARDRR2DImageType> UCHARWriterType;
    typedef itk::CastImageFilter<DRR2DImageType, DRR2DImageType> FLOATCastType;
    typedef itk::CastImageFilter<DRR2DImageType, USHORTDRR2DImageType> USHORTCastType;
    typedef itk::CastImageFilter<DRR2DImageType, UCHARDRR2DImageType> UCHARCastType;

    FLOATWriterType::Pointer wf = FLOATWriterType::New();
    USHORTWriterType::Pointer wus = USHORTWriterType::New();
    UCHARWriterType::Pointer wuc = UCHARWriterType::New();
    DRR2DImageType::Pointer drr = NULL;
    FLOATCastType::Pointer cf = FLOATCastType::New();
    USHORTCastType::Pointer cus = USHORTCastType::New();
    UCHARCastType::Pointer cuc = UCHARCastType::New();
    wf->SetInput(cf->GetOutput());
    wus->SetInput(cus->GetOutput());
    wuc->SetInput(cuc->GetOutput());
    for (std::size_t fi = 0; fi < m_PPFixedImages.size(); fi++)
    {
      drr = nreg->Compute2DTestProjection(fi, 0);
      if (!drr)
        return false;
      sprintf(fbuff, baseFN.c_str(), (fi + 1)); // insert view index (%d)
      wf->SetFileName(fbuff);
      wus->SetFileName(fbuff);
      wuc->SetFileName(fbuff);
      cf->SetInput(drr);
      cus->SetInput(drr);
      cuc->SetInput(drr);
      try
      {
        if (outputMode == 1) // USHORT
        {
          cus->Update();
          wus->Update();
        }
        else if (outputMode == 2) // FLOAT
        {
          cf->Update();
          wf->Update();
        }
        else if (outputMode == 3) // UCHAR
        {
          cuc->Update();
          wuc->Update();
        }
      }
      catch (itk::ExceptionObject &e)
      {
        std::cerr << "DRR storage error: " << e << std::endl;
        return false;
      }
    }
    drr = NULL;
    cf = NULL; cus = NULL; cuc = NULL;
    wf = NULL; wus = NULL; wuc = NULL;
  }

  return true;
}

template<typename TVolumeComponentType>
bool REG23Model::GenerateITFOptimizerConfiguration(std::string outputDir)
{
  if (outputDir.length() <= 0)
    return false;

  std::string optimizerEXE = "ITFOptimizer";
  if (m_ITFOptimizerConfigStrings.size() > 0)
    optimizerEXE = m_ITFOptimizerConfigStrings[0];

  if (!itksys::SystemTools::MakeDirectory(outputDir.c_str()))
    return false;
  if (!itksys::SystemTools::FileExists(outputDir.c_str()))
    return false;

  typedef itk::Image<TVolumeComponentType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
      DRR2DImageType, VolumeImageType> RegistrationType;
  typedef typename RegistrationType::ProjectionPropsPointer ProjectionPropsPointer;

  typename RegistrationType::Pointer nreg =
      static_cast<RegistrationType *>(m_NReg.GetPointer());
  if (!nreg)
    return false;

  // store volume (already oriented and transformed according to frame of
  // reference and iso-center:
  std::string vbasename = outputDir;
  EnsureStringEndsWith(vbasename, DSEP);
  vbasename += "volume.mhd";
  ITKVTKImage *volumeImage = GetVolumeImage();
  if (!volumeImage)
    return false;
  bool ok = volumeImage->SaveImageAsORAMetaImage<TVolumeComponentType>(
      vbasename, false, "");
  if (!ok)
    return false;

  std::string fbasename = outputDir;
  EnsureStringEndsWith(fbasename, DSEP);
  std::string mbasename = fbasename;
  std::string bbasename = fbasename;
  fbasename += "fixed-image-%d.mhd";
  mbasename += "mask-image-%d.mhd";
  ITKVTKImage *fixedImage;
  ITKVTKImage *maskImage;
  char fbuff[2048];
  char mbuff[2048];
  char bbuff[2048];
  std::ofstream *f = NULL;
  std::ostringstream os;
  double *sp;
  std::string s;
  int i;
  double itfv[6];
  for (std::size_t fi = 0; fi < m_PPFixedImages.size(); fi++)
  {
    // post-processed (!) fixed image if available (pre-processed otherwise)
    // 3D FLOAT
    fixedImage = this->GetFixedImage(fi, true);
    if (!fixedImage)
      return false;
    sprintf(fbuff, fbasename.c_str(), (fi + 1));
    TEMPLATE_CALL_COMP(fixedImage->GetComponentType(),
        ok = fixedImage->SaveImageAsORAMetaImage, fbuff, false, "")
    if (!ok)
      return false;

    // 3D UCHAR
    maskImage = GetMaskImage(fi);
    if (!maskImage)
      return false;
    sprintf(mbuff, mbasename.c_str(), (fi + 1));
    TEMPLATE_CALL_COMP(maskImage->GetComponentType(),
        ok = maskImage->SaveImageAsORAMetaImage, mbuff, false, "")
    if (!ok)
      return false;

    // generate batch-file:
    os.str("");
    os << "\"" << optimizerEXE << "\" ";
    os << "--verbose ";
    sprintf(bbuff, std::string(bbasename + "final-ITF-%d.txt").c_str(), (fi + 1));
    os << "--final-itf-out \"" << std::string(bbuff) << "\" "; // final ITF
    sp = GetSourcePosition(fi);
    if (!sp)
      return false;
    s = ToUpperCaseF(TrimF(m_Config->ReadString("", "", "")));
    if (s == "AUTO-0")
      i = 0;
    else if (s == "AUTO-2")
      i = 2;
    else if (s == "AUTO-3")
      i = 3;
    else // default (also if we have an explicitly defined step size!)
      i = 1;
    os << "--projection-props 0 0 -1 -1 " << sp[0] << " " << sp[1] << " " <<
        sp[2] << " " << i << " "; // projection props
    // ITF: <itf-num-pairs> <itf-in1> <itf-out1> ... take the internal converted
    // ITF!!
    ProjectionPropsPointer props = nreg->GetIthProjectionProps(fi);
    if (!props || !props->GetITF())
      return false;
    os << "--intensity-transfer-function " << props->GetITF()->GetSize() << " ";
    for (int j = 0; j < props->GetITF()->GetSize(); j++)
    {
      props->GetITF()->GetNodeValue(j, itfv);
      os << itfv[0] << " " << itfv[1] << " ";
    }

    // FIXME:
    os << "--itf-window 100 0 300 300 "; // ITF visualization window
    os << "--stay "; // do not end after optimization
    os << "--drr-window 0 350 500 500 "; // DRR visualization window
    os << "--no-wrap "; // do not wrap bounds (limit them instead)
    os << "--best "; // update whenever metric gets better
    os << "--transform "; // transform
    for (unsigned int j = 0; j < m_CurrentParameters.GetSize(); j++)
      os << m_CurrentParameters[j] << " ";
    os << "--mask \"" << std::string(mbuff) << "\" "; // mask
    // volume and fixed image last
    os << "\"" << vbasename << "\" \"" << std::string(fbuff) << "\"";
    sprintf(bbuff, std::string(bbasename + "optimize-view-%d.bat").c_str(), (fi + 1));
    f = new std::ofstream(bbuff);
    (*f) << os.str() << std::endl;
    f->close();
    delete f;
  }

  return true;
}

template<typename TVolumeComponentType>
bool
REG23Model::ComputeCrossCorrelationInitialTransform(const unsigned int &index,
    double &translationX, double &translationY, const bool &doMovingUnsharpMasking,
    const bool &fastSearch, const bool &radiusSearch,
    std::vector<double> &args, const std::string &structureUID, itk::Command *itkCommand)
{
  // TODO: cleanup debug output and create a separate unit test
  const bool writeDebugData = false;
  // Timing measurement
  itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
  typedef itk::RealTimeClock::TimeStampType TimeStampType;
  TimeStampType timeStart = clock->GetTimeStamp();

  if (index >= m_FixedImages.size() || !this->IsReadyForManualRegistration())
    return false;

  m_RegistrationIsRunning = true; // this is also sort of registration

  // Define input arguments
  if (args.empty())
  {
    // Default: {x-, x+, y-, y+, xoff, yoff, w, h}
    args.push_back(-30);
    args.push_back(30);
    args.push_back(-30);
    args.push_back(30);
    args.push_back(-30);
    args.push_back(-30);
    args.push_back(61);
    args.push_back(61);
  }
  if (args.size() != 8)
  {
    m_RegistrationIsRunning = false;
    return false;
  }
  // list that stores the arguments in [pixels] (is computed later)
  std::vector<int> argsPixels;

  // Get all required input data
  typedef float FloatPixeltType;
  typedef itk::Image<FloatPixeltType, ITKVTKImage::Dimensions> FloatImageType;
  typedef itk::Image<FloatPixeltType, 2> FloatImageType2D;

  // Fixed image
  ITKVTKImage *fixedIVI = this->GetFixedImage(index, true);
  if (!fixedIVI)
  {
    m_RegistrationIsRunning = false;
    return false;
  }
  ITKVTKImage *fixedIVIFloat = NULL;
  std::vector<std::string> castArgs;
  castArgs.push_back("CAST");
  castArgs.push_back("FLOAT");
  TEMPLATE_CALL_COMP(fixedIVI->GetComponentType(),
      fixedIVIFloat = CastImage, fixedIVI, castArgs)

  // Moving image
  typedef TVolumeComponentType VolumePixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef itk::Image<DRRPixelType, 3> DRRImageType;
  typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
      DRR2DImageType, VolumeImageType> RegistrationType;
  typename RegistrationType::Pointer nreg =
      static_cast<RegistrationType *> (m_NReg.GetPointer());
  if (!nreg)
  {
    m_RegistrationIsRunning = false;
    return false;
  }
  typename DRRImageType::Pointer drr = nreg->Compute3DTestProjection(index, 0);
  if (!drr)
  {
    m_RegistrationIsRunning = false;
    return false;
  }
  ITKVTKImage *movingIVI = new ITKVTKImage(itk::ImageIOBase::SCALAR,
      itk::ImageIOBase::FLOAT);
  movingIVI->SetITKImage(drr.GetPointer(), false);

  if (doMovingUnsharpMasking)
  {
    // Unsharp masking of moving image
    std::vector<std::string> unsharpArgs;
    unsharpArgs.push_back("UNSHARPMASKING");
    ITKVTKImage *movingIVITemp = UnsharpMaskImage<float> (movingIVI,
        unsharpArgs);
    delete movingIVI;
    movingIVI = movingIVITemp;
  }

  // Cast to ITK image types
  ITKVTKImage::ITKImagePointer ibase = fixedIVIFloat->GetAsITKImage<float> ();
  FloatImageType::Pointer fixedImage =
      static_cast<FloatImageType *> (ibase.GetPointer());
  ibase = movingIVI->GetAsITKImage<float> ();
  FloatImageType::Pointer movingImage =
      static_cast<FloatImageType *> (ibase.GetPointer());

  // Mask image is cast to INT regardless of its original type
  typedef itk::Image<int, ITKVTKImage::Dimensions> MaskImageType;
  typedef typename MaskImageType::Pointer MaskImagePointer;
  ITKVTKImage *maskIVI = this->GetMaskImage(index);
  MaskImagePointer maskImage = NULL;
  if (maskIVI)
  {
    ITKVTKImage *imageMaskCastTEMP = NULL;
    std::vector<std::string> castArgs;
    castArgs.push_back("CAST");
    castArgs.push_back("INT");
    TEMPLATE_CALL_COMP(maskIVI->GetComponentType(),
        imageMaskCastTEMP = CastImage, maskIVI, castArgs)
    maskImage = static_cast<MaskImageType *> (imageMaskCastTEMP->GetAsITKImage<int>().GetPointer());
    delete (imageMaskCastTEMP);
  }

  // Volume image
  ibase = this->GetVolumeImage()->GetAsITKImage<TVolumeComponentType>();
  typename VolumeImageType::Pointer volumeImage = static_cast<VolumeImageType *> (ibase.GetPointer());

  // Fast option re-samples image to a spacing of 2 mm
  if (fastSearch)
  {
    // Resampler
    typedef itk::ResampleImageFilter<FloatImageType, FloatImageType>
        ResampleFilterType;
    ResampleFilterType::Pointer resampleFilter = ResampleFilterType::New();
    resampleFilter->SetInput(fixedImage);

    // Use the IdentityTransform because resampling is performed in the same
    // physical extent of the input image
    typedef itk::IdentityTransform<double, FloatImageType::ImageDimension> TransformType;
    TransformType::Pointer transform = TransformType::New();
    transform->SetIdentity();
    resampleFilter->SetTransform(transform);

    // Use linear interpolation (good run-time vs. quality)
    typedef itk::LinearInterpolateImageFunction<FloatImageType, double>
        InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    resampleFilter->SetInterpolator(interpolator);
    resampleFilter->SetDefaultPixelValue(0); // value for regions without source

    // Spacing of output
    FloatImageType::SpacingType spacing;
    const double spacingX = 2.0;
    const double spacingY = 2.0;
    spacing[0] = spacingX;
    spacing[1] = spacingY;
    spacing[2] = fixedImage->GetSpacing()[2];
    resampleFilter->SetOutputSpacing(spacing);

    // Preserve origin and direction of the input image
    resampleFilter->SetOutputOrigin(fixedImage->GetOrigin());
    resampleFilter->SetOutputDirection(fixedImage->GetDirection());

    // Compute size of output based on desired spacing and input size
    const FloatImageType::SizeType inputSize =
        fixedImage->GetLargestPossibleRegion().GetSize();
    typedef FloatImageType::SizeType::SizeValueType SizeValueType;
    FloatImageType::SizeType fixedSize;

    const FloatImageType::SpacingType &inputSpacing =
        fixedImage->GetSpacing();
    double factorX = spacingX / inputSpacing[0];
    double factorY = spacingY / inputSpacing[1];
    fixedSize[0] = static_cast<SizeValueType> (inputSize[0] / factorX);
    fixedSize[1] = static_cast<SizeValueType> (inputSize[1] / factorY);
    fixedSize[2] = inputSize[2];
    resampleFilter->SetSize(fixedSize);

    try
    {
      resampleFilter->Update();
      fixedImage = resampleFilter->GetOutput();
      fixedImage->DisconnectPipeline();
      resampleFilter->SetInput(movingImage);
      resampleFilter->Update();
      movingImage = resampleFilter->GetOutput();
      movingImage->DisconnectPipeline();
    }
    catch (itk::ExceptionObject & err)
    {
      std::cerr << "ExceptionObject caught at resampling!" << std::endl;
      std::cerr << err << std::endl;
      return false;
    }
  }

  // Extract 2D fixed image region (X-ray) based on mask image bounding box
  typedef itk::ExtractImageFilter<FloatImageType, FloatImageType2D>
      ExtractFilterType3D2D;
  ExtractFilterType3D2D::Pointer extractFilterFixed =
      ExtractFilterType3D2D::New();

  // Get the region to extract a slice
  FloatImageType::RegionType inputRegionFixed =
      fixedImage->GetLargestPossibleRegion();
  FloatImageType::IndexType inputStartFixed = inputRegionFixed.GetIndex();
  FloatImageType::SizeType inputSizeFixed = inputRegionFixed.GetSize();
  FloatImageType::RegionType inputRegionMoving =
      movingImage->GetLargestPossibleRegion();
  FloatImageType::IndexType inputStartMoving = inputRegionMoving.GetIndex();
  FloatImageType::SizeType inputSizeMoving = inputRegionMoving.GetSize();

  // Fixed and moving image must have the same size, spacing and direction
  if (inputRegionFixed.GetSize() != inputRegionMoving.GetSize())
  {
    m_RegistrationIsRunning = false;
    return false;
  }
  if (fixedImage->GetSpacing() != movingImage->GetSpacing())
  {
    m_RegistrationIsRunning = false;
    return false;
  }
  if (fixedImage->GetDirection() != movingImage->GetDirection())
  {
    m_RegistrationIsRunning = false;
    return false;
  }

  // Convert all arguments from [mm] to [pixels]
  FloatImageType::SpacingType spacingFixed = fixedImage->GetSpacing();
  // Increase patch size because of decreased resolution
  if (fastSearch)
  {
    argsPixels.push_back(vnl_math_rnd((args[0] - 20) / spacingFixed[0]));  // x-
    argsPixels.push_back(vnl_math_rnd((args[1] + 20) / spacingFixed[0]));  // x+
    argsPixels.push_back(vnl_math_rnd((args[2] - 20) / spacingFixed[1]));  // y-
    argsPixels.push_back(vnl_math_rnd((args[3] + 20) / spacingFixed[1]));  // y+
    argsPixels.push_back(vnl_math_rnd((args[4] - 20) / spacingFixed[0]));  // xoff
    argsPixels.push_back(vnl_math_rnd((args[5] - 20) / spacingFixed[1]));  // yoff
    argsPixels.push_back(vnl_math_rnd((args[6] + 40) / spacingFixed[0]));  // w
    argsPixels.push_back(vnl_math_rnd((args[7] + 40) / spacingFixed[1]));  // h
  }
  else
  {
    argsPixels.push_back(vnl_math_rnd(args[0] / spacingFixed[0]));  // x-
    argsPixels.push_back(vnl_math_rnd(args[1] / spacingFixed[0]));  // x+
    argsPixels.push_back(vnl_math_rnd(args[2] / spacingFixed[1]));  // y-
    argsPixels.push_back(vnl_math_rnd(args[3] / spacingFixed[1]));  // y+
    argsPixels.push_back(vnl_math_rnd(args[4] / spacingFixed[0]));  // xoff
    argsPixels.push_back(vnl_math_rnd(args[5] / spacingFixed[1]));  // yoff
    argsPixels.push_back(vnl_math_rnd(args[6] / spacingFixed[0]));  // w
    argsPixels.push_back(vnl_math_rnd(args[7] / spacingFixed[1]));  // h
  }

  // Extract 2D moving image patch based on projected volume center
  ExtractFilterType3D2D::Pointer extractFilterMoving =
      ExtractFilterType3D2D::New();

  // Extract patch region from moving image
  int widthPatch = vnl_math_abs(argsPixels[6]);
  if (widthPatch % 2 == 0)  // even -> make odd (required for kernel operator)
    widthPatch = widthPatch - 1;
  int heightPatch = vnl_math_abs(argsPixels[7]);
  if (heightPatch % 2 == 0)  // even -> make odd (required for kernel operator)
    heightPatch = heightPatch - 1;

  // Get position of projected volume center
  // The default is a moving image centered patch (if projection not possible)
  int centerX = inputStartMoving[0] + (inputSizeMoving[0] - 1) / 2;
  int centerY = inputStartMoving[1] + (inputSizeMoving[1] - 1) / 2;

  // Determine position of projected volume center
  FloatImageType::PointType originFixed = fixedImage->GetOrigin();
  FloatImageType::DirectionType directionFixed = fixedImage->GetDirection();
  double *sourcePosition  = this->GetSourcePosition(index);
  if (sourcePosition)
  {
    vtkSmartPointer<vtkPlane> fixedImagePlane = vtkSmartPointer<vtkPlane>::New();
    double origin[3];
    origin[0] = originFixed[0];
    origin[1] = originFixed[1];
    origin[2] = originFixed[2];
    fixedImagePlane->SetOrigin(origin);
    double n[3];
    n[0] = directionFixed[0][2];
    n[1] = directionFixed[1][2];
    n[2] = directionFixed[2][2];
    fixedImagePlane->SetNormal(n);

    // Volume center point
    typename VolumeImageType::SizeType volumeSize = volumeImage->GetLargestPossibleRegion().GetSize();
    typedef itk::ContinuousIndex<double, 3> ContinuousIndexType;
    ContinuousIndexType indexVolCenter;
    for (unsigned int i = 0; i < volumeSize.GetSizeDimension(); ++i)
    {
      indexVolCenter[i] = (volumeSize[i] - 1) * 0.5;
    }
    typename VolumeImageType::PointType volumeCenterPoint;
    volumeImage->TransformContinuousIndexToPhysicalPoint(indexVolCenter, volumeCenterPoint);

    // Use structure center if available
    VTKStructure *structure = NULL;
    std::vector<std::string> toks;
    toks.clear();
    if (structureUID.length() >= 7 && /** at least: (0|0|0) **/
        structureUID.substr(0, 1) == "(" &&
        structureUID.substr(structureUID.length() - 1, 1) == ")")
    {
      std::string tmp = TrimF(structureUID.substr(1, structureUID.length() - 2));
      Tokenize(tmp, toks, "|");
    }

    if (toks.size() != 3) // obviously not an explicit coordinate, is it a structure UID?
    {
      if (m_Structures)
        structure = m_Structures->FindStructure(structureUID);
      if (structure)
      {
        vtkPolyData *pd = structure->GetStructurePolyData();
        if (!pd)
        {
          m_RegistrationIsRunning = false;
          return false;
        }
        double bounds[6];
        pd->GetBounds(bounds);
        volumeCenterPoint[0] = bounds[0] + (bounds[1] - bounds[0]) * 0.5;
        volumeCenterPoint[1] = bounds[2] + (bounds[3] - bounds[2]) * 0.5;
        volumeCenterPoint[2] = bounds[4] + (bounds[5] - bounds[4]) * 0.5;
      } // else: maybe the VOLUMECENTER
    }
    else if (toks.size() == 3) // obviously explicit coordinate (x,y,z)
    {
      // -> simply take over the explicit coordinate
      volumeCenterPoint[0] = atof(toks[0].c_str());
      volumeCenterPoint[1] = atof(toks[1].c_str());
      volumeCenterPoint[2] = atof(toks[2].c_str());
    }

    volumeCenterPoint = m_Transform->TransformPoint(volumeCenterPoint);
    double volCenterPoint[3];
    volCenterPoint[0] = volumeCenterPoint[0];
    volCenterPoint[1] = volumeCenterPoint[1];
    volCenterPoint[2] = volumeCenterPoint[2];

    // Project volume center point onto image plane
    double t = 0;
    double volumeCenterPointProj[3];
    fixedImagePlane->IntersectWithLine(sourcePosition,
        volCenterPoint, t, volumeCenterPointProj);
    FloatImageType::IndexType centerIndex;
    FloatImageType::PointType centerPoint = volumeCenterPointProj;
    fixedImage->TransformPhysicalPointToIndex(centerPoint, centerIndex);

    if (t > 0 && t < VTK_DOUBLE_MAX)
    {
      centerX = centerIndex[0];
      centerY = centerIndex[1];
    }
  }

  FloatImageType::IndexType startMoving;
  startMoving[0] = centerX + argsPixels[4];
  startMoving[1] = centerY + argsPixels[5];
  startMoving[2] = 0;
  FloatImageType::SizeType sizeMoving;
  sizeMoving[0] = widthPatch;
  sizeMoving[1] = heightPatch;
  sizeMoving[2] = 0;

  // Limit region to largest possible region of moving image
  if (startMoving[0] < inputStartMoving[0])
  {
    startMoving[0] = inputStartMoving[0];
    sizeMoving[0] = sizeMoving[0] - (inputStartMoving[0] - startMoving[0]);
  }
  if (startMoving[1] < inputStartMoving[1])
  {
    startMoving[1] = inputStartMoving[1];
    sizeMoving[1] = sizeMoving[1] - (inputStartMoving[1] - startMoving[1]);
  }
  if (startMoving[0] + sizeMoving[0] > inputStartMoving[0] + inputSizeMoving[0])
  {
    sizeMoving[0] = inputStartMoving[0] + inputSizeMoving[0] - startMoving[0];
  }
  if (startMoving[1] + sizeMoving[1] > inputStartMoving[1] + inputSizeMoving[1])
  {
    sizeMoving[1] = inputStartMoving[1] + inputSizeMoving[1] - startMoving[1];
  }
  // Make sure that sizes are valid for kernel operator
  if (sizeMoving[0] < 3 || sizeMoving[1] < 3)
  {
    m_RegistrationIsRunning = false;
    return false;
  }
  if (sizeMoving[0] % 2 == 0)
    sizeMoving[0] = sizeMoving[0] - 1;
  if (sizeMoving[1] % 2 == 0)
    sizeMoving[1] = sizeMoving[1] - 1;

  FloatImageType::RegionType desiredRegionMoving;
  desiredRegionMoving.SetSize(sizeMoving);
  desiredRegionMoving.SetIndex(startMoving);
  extractFilterMoving->SetExtractionRegion(desiredRegionMoving);
  extractFilterMoving->SetInput(movingImage);



  // Determine fixed image region to search for the moving image patch
  typedef typename MaskImageType::IndexType::IndexValueType MaskIndexValueType;
  MaskIndexValueType maskMinX = std::numeric_limits<MaskIndexValueType>::max();
  MaskIndexValueType maskMinY = std::numeric_limits<MaskIndexValueType>::max();
  MaskIndexValueType maskMaxX = -std::numeric_limits<MaskIndexValueType>::max();
  MaskIndexValueType maskMaxY = -std::numeric_limits<MaskIndexValueType>::max();
  if (maskImage)
  {
    // Mask available: Use bounding box of mask area
    typedef itk::ImageRegionConstIteratorWithIndex<MaskImageType>
        MaskImageIteratorType;
    MaskImageIteratorType itImageMask(maskImage,
        maskImage->GetLargestPossibleRegion());
    for (itImageMask = itImageMask.Begin(); !itImageMask.IsAtEnd(); ++itImageMask)
    {
      if (itImageMask.Value())
      {
        MaskImageType::IndexType idx = itImageMask.GetIndex();
        maskMinX = std::min(maskMinX, idx[0]);
        maskMinY = std::min(maskMinY, idx[1]);
        maskMaxX = std::max(maskMaxX, idx[0]);
        maskMaxY = std::max(maskMaxY, idx[1]);
      }
    }
  }


  // Check if mask bounding box has valid values, else use full fixed image
  // or region around the projected volume or structure center
  FloatImageType::IndexType startFixed;
  FloatImageType::SizeType sizeFixed;
  if (radiusSearch)
  {
    startFixed[0] = centerX;
    startFixed[1] = centerY;
    startFixed[2] = 0;
    sizeFixed[0] = 1;
    sizeFixed[1] = 1;
    sizeFixed[2] = 0;
  }
  else if (maskMinX >= 0 && maskMinY >= 0 && maskMaxX < (MaskIndexValueType) inputSizeFixed[0] && maskMaxY
      < (MaskIndexValueType) inputSizeFixed[1] && maskMaxX - maskMinX > 0 && maskMaxY - maskMinY > 0)
  {
    // Extract the region from the masked moving image that is not zero
    startFixed[0] = maskMinX;
    startFixed[1] = maskMinY;
    startFixed[2] = 0;
    sizeFixed[0] = maskMaxX - maskMinX;
    sizeFixed[1] = maskMaxY - maskMinY;
    sizeFixed[2] = 0;
  }
  else
  {
    // No Mask available use the full fixed image
    startFixed[0] = inputStartFixed[0];
    startFixed[1] = inputStartFixed[1];
    startFixed[2] = 0;
    sizeFixed[0] = inputSizeFixed[0];
    sizeFixed[1] = inputSizeFixed[1];
    sizeFixed[2] = 0;
  }

  // Apply additional margins
  startFixed[0] = startFixed[0] + argsPixels[0];
  startFixed[1] = startFixed[1] + argsPixels[2];
  startFixed[2] = 0;
  sizeFixed[0] = sizeFixed[0] + argsPixels[1] - argsPixels[0];
  sizeFixed[1] = sizeFixed[1] + argsPixels[3] - argsPixels[2];
  sizeFixed[2] = 0;

  // Limit region to largest possible region of fixed image
  if (startFixed[0] < inputStartFixed[0])
  {
    startFixed[0] = inputStartFixed[0];
    sizeFixed[0] = sizeFixed[0] - (inputStartFixed[0] - startFixed[0]);
  }
  if (startFixed[1] < inputStartFixed[1])
  {
    startFixed[1] = inputStartFixed[1];
    sizeFixed[1] = sizeFixed[1] - (inputStartFixed[1] - startFixed[1]);
  }
  if (startFixed[0] + sizeFixed[0] > inputStartFixed[0] + inputSizeFixed[0])
  {
    sizeFixed[0] = inputStartFixed[0] + inputSizeFixed[0] - startFixed[0];
  }
  if (startFixed[1] + sizeFixed[1] > inputStartFixed[1] + inputSizeFixed[1])
  {
    sizeFixed[1] = inputStartFixed[1] + inputSizeFixed[1] - startFixed[1];
  }

  FloatImageType::RegionType desiredRegionFixed;
  desiredRegionFixed.SetSize(sizeFixed);
  desiredRegionFixed.SetIndex(startFixed);
  extractFilterFixed->SetExtractionRegion(desiredRegionFixed);
  extractFilterFixed->SetInput(fixedImage);

  try
  {
    extractFilterFixed->Update();
    extractFilterMoving->Update();
  }
  catch (itk::ExceptionObject & err)
  {
    std::cerr << "ExceptionObject caught at 2D image extraction." << __FILE__
        << ":" << __LINE__ << std::endl;
    std::cerr << err << std::endl;
    std::cerr << "File: " << err.GetFile() << ", Line: " << err.GetLine()
        << ", Location: " << err.GetLocation() << std::endl;
    std::cerr << "Parameters: " << "index: " << index
        << ", doMovingUnsharpMasking: " << doMovingUnsharpMasking << ", args: ";
    for (size_t i = 0; i < args.size(); ++i)
      std::cerr << args[i] << ", ";
    std::cerr << "structureUID: " << structureUID << ", itkCommand"
        << &itkCommand << std::endl;
    std::cerr << "argsPixels: ";
    for (size_t i = 0; i < argsPixels.size(); ++i)
      std::cerr << argsPixels[i] << ", ";
    std::cerr << std::endl;
    std::cerr << "State of extractFilterFixed: " << std::endl
        << extractFilterFixed << std::endl;
    std::cerr << "State of extractFilterMoving: " << std::endl
        << extractFilterFixed << std::endl;
    m_RegistrationIsRunning = false;
    return false;
  }

  // Reset orientation and origin of the images to coincide and for convenience
  typedef itk::ChangeInformationImageFilter<FloatImageType2D> ChangeFilterType;
  FloatImageType2D::DirectionType directionMatrix =
      extractFilterFixed->GetOutput()->GetDirection();
  directionMatrix.SetIdentity();
  ChangeFilterType::Pointer changerFixed = ChangeFilterType::New();
  changerFixed->SetOutputDirection(directionMatrix);
  changerFixed->ChangeDirectionOn();
  changerFixed->SetInput(extractFilterFixed->GetOutput());
  ChangeFilterType::Pointer changerMoving = ChangeFilterType::New();
  changerMoving->SetOutputDirection(directionMatrix);
  changerMoving->ChangeDirectionOn();
  changerMoving->SetInput(extractFilterMoving->GetOutput());
  try
  {
    changerFixed->Update();
    changerMoving->Update();
  }
  catch (itk::ExceptionObject & err)
  {
    std::cerr << "ExceptionObject caught at 2D image information change." << std::endl;
    std::cerr << err << std::endl;
    m_RegistrationIsRunning = false;
    return false;
  }

  FloatImageType2D::Pointer fixedImage2D = changerFixed->GetOutput();
  fixedImage2D->DisconnectPipeline();
  FloatImageType2D::Pointer movingImage2D = changerMoving->GetOutput();
  movingImage2D->DisconnectPipeline();

  // Perform normalized correlation
  // <input type, mask type (not used), output type>
  typedef itk::NormalizedCorrelationImageFilter<FloatImageType2D,
      FloatImageType2D, FloatImageType2D> CorrelationFilterType;
  CorrelationFilterType::Pointer correlationFilter =
      CorrelationFilterType::New();
  correlationFilter->SetInput(fixedImage2D);
  // Use the image kernel operator from ITK/REVIEW
  itk::ImageKernelOperator<FloatPixeltType, 2> kernelOperator;
  kernelOperator.SetImageKernel(movingImage2D);
  FloatImageType2D::SizeType radiusKernel;
  radiusKernel[0] = (sizeMoving[0] - 1) / 2;
  radiusKernel[1] = (sizeMoving[1] - 1) / 2;
  kernelOperator.CreateToRadius(radiusKernel);
  correlationFilter->SetTemplate(kernelOperator);

  try
  {
    unsigned long observerTag = 0;
    if (itkCommand)
      observerTag = correlationFilter->AddObserver(itk::ProgressEvent(), itkCommand);
    correlationFilter->Update();
    if (itkCommand)
      correlationFilter->RemoveObserver(observerTag);
  }
  catch (itk::ExceptionObject & excp)
  {
    correlationFilter->RemoveAllObservers();
    std::cerr << "Error while computing cross correlation." << std::endl;
    std::cerr << excp << std::endl;
    m_RegistrationIsRunning = false;
    return false;
  }

  // Find maximum in correlation image
  typedef itk::MinimumMaximumImageCalculator<FloatImageType2D>
      MinimumMaximumImageCalculatorType;
  MinimumMaximumImageCalculatorType::Pointer
      minimumMaximumImageCalculatorFilter =
          MinimumMaximumImageCalculatorType::New();
  minimumMaximumImageCalculatorFilter->SetImage(correlationFilter->GetOutput());
  minimumMaximumImageCalculatorFilter->Compute();

  // Calculate transformation (in [mm])
  FloatImageType2D::IndexType maxIdx =
      minimumMaximumImageCalculatorFilter->GetIndexOfMaximum();
  FloatImageType2D::PointType maxPt;
  correlationFilter->GetOutput()->TransformIndexToPhysicalPoint(maxIdx, maxPt);
  FloatImageType2D::IndexType currentIdx;
  currentIdx[0] = startMoving[0] + (sizeMoving[0] - 1) / 2;
  currentIdx[1] = startMoving[1] + (sizeMoving[1] - 1) / 2;
  FloatImageType2D::PointType currentPt;
  movingImage2D->TransformIndexToPhysicalPoint(currentIdx, currentPt);
  translationX = currentPt[0] - maxPt[0];
  translationY = currentPt[1] - maxPt[1];

  if (writeDebugData)
  {
    TimeStampType timeStop = clock->GetTimeStamp();
    std::cerr << " Time (sec)   = " << (timeStop - timeStart) << std::endl;
    std::cerr << " FixedImageRegion" << desiredRegionFixed << std::endl;
    std::cerr << " MovingImageRegion" << desiredRegionMoving << std::endl;
    std::cerr << " Patch center = " << currentIdx << std::endl;
    std::cerr << " Max. corr.   = " << maxIdx << std::endl;
    int txIdx = currentIdx[0] - maxIdx[0];
    int tyIdx = currentIdx[1] - maxIdx[1];
    std::cerr << " Translation X/Y [pixels] = " << txIdx << ", " << tyIdx
        << std::endl;
    std::cerr << " Translation X/Y [mm]     = " << translationX << ", "
        << translationY << std::endl;

    try
    {
      typedef itk::Image<unsigned char, 2> UCharImageType;
      std::stringstream ss;

      typedef itk::ImageFileWriter<FloatImageType2D> FloatWriterType;
      FloatWriterType::Pointer writerFloat = FloatWriterType::New();
      ss.str("");
      ss << "view_" << index << "_extracted_region.mhd";
      writerFloat->SetFileName(ss.str());
      writerFloat->SetInput(movingImage2D);
      writerFloat->Update();
      typedef itk::RescaleIntensityImageFilter<FloatImageType2D, UCharImageType>
          RescaleFilterType;
      RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
      rescaleFilter->SetOutputMinimum(0);
      rescaleFilter->SetOutputMaximum(255);
      rescaleFilter->SetInput(correlationFilter->GetOutput());

      typedef itk::ImageFileWriter<UCharImageType> WriterType;
      WriterType::Pointer writer = WriterType::New();
      ss.str("");
      ss << "view_" << index << "_correlation.png";
      writer->SetFileName(ss.str());
      writer->SetInput(rescaleFilter->GetOutput());
      writer->Update();

      rescaleFilter->SetInput(fixedImage2D);
      writer->SetInput(rescaleFilter->GetOutput());
      ss.str("");
      ss << "view_" << index << "_fixed.png";
      writer->SetFileName(ss.str());
      writer->Update();

      rescaleFilter->SetInput(movingImage2D);
      ss.str("");
      ss << "view_" << index << "_moving.png";
      writer->SetFileName(ss.str());
      writer->Update();

      typedef itk::ResampleImageFilter<FloatImageType2D, FloatImageType2D>
          ResampleFilterType;
      ResampleFilterType::Pointer resample = ResampleFilterType::New();
      typedef itk::TranslationTransform<double, 2> TransformType;
      TransformType::Pointer transform = TransformType::New();
      transform->SetIdentity();
      TransformType::ParametersType pars = transform->GetParameters();
      pars[0] = translationX;
      pars[1] = translationY;
      transform->SetParameters(pars);
      resample->SetTransform(transform);
      resample->SetOutputStartIndex(
          fixedImage2D->GetLargestPossibleRegion().GetIndex());
      resample->SetSize(fixedImage2D->GetLargestPossibleRegion().GetSize());
      resample->SetOutputOrigin(fixedImage2D->GetOrigin());
      resample->SetOutputSpacing(fixedImage2D->GetSpacing());
      resample->SetOutputDirection(fixedImage2D->GetDirection());
      resample->SetDefaultPixelValue(0);
      resample->SetInput(movingImage2D);
      rescaleFilter->SetInput(resample->GetOutput());
      ss.str("");
      ss << "view_" << index << "_resampled.png";
      writer->SetFileName(ss.str());
      writer->Update();

      typedef itk::SubtractImageFilter<FloatImageType2D, FloatImageType2D,
          FloatImageType2D> DifferenceFilterType;
      DifferenceFilterType::Pointer difference = DifferenceFilterType::New();
      difference->SetInput1(fixedImage2D);
      difference->SetInput2(resample->GetOutput());

      // Difference image between the fixed and moving image after registration
      rescaleFilter->SetInput(difference->GetOutput());
      ss.str("");
      ss << "view_" << index << "_diff_after.png";
      writer->SetFileName(ss.str());
      writer->Update();

      // Difference image between the fixed and moving image before registration
      transform->SetIdentity();
      resample->SetTransform(transform);
      resample->Modified();
      ss.str("");
      ss << "view_" << index << "_diff_before.png";
      writer->SetFileName(ss.str());
      writer->Update();
    }
    catch (itk::ExceptionObject & excp)
    {
      std::cerr << "Error while writing debug images." << std::endl;
      std::cerr << excp << std::endl;
    }
  }

  // Cleanup
  fixedIVI = NULL;
  delete fixedIVIFloat;
  nreg = NULL;
  drr = NULL;
  delete movingIVI;
  ibase = NULL;
  fixedImage = NULL;
  movingImage = NULL;
  maskImage = NULL;

  m_RegistrationIsRunning = false;

  return true;
}

template<typename TVolumeComponentType>
itk::Image<unsigned char, 3>::Pointer
REG23Model::CreateVolumeMask(ITKVTKImage *volume,
    std::vector<vtkSmartPointer<vtkPolyData> > &maskstructures)
{
  if (!volume)
    return NULL;
  if (maskstructures.size() < 1)
    return NULL;

  // Cast to ITK image types
  typedef TVolumeComponentType VolumePixelType;
  typedef itk::Image<VolumePixelType, 3> VolumeImageType;
  ITKVTKImage::ITKImagePointer ibase = this->GetVolumeImage()->GetAsITKImage<VolumePixelType>();
  typename VolumeImageType::Pointer volumeImageITK = static_cast<VolumeImageType *> (ibase.GetPointer());
  vtkSmartPointer<vtkImageData> volumeImageVTK = volume->GetAsVTKImage<VolumePixelType>();

  // Volume properties
  typename VolumeImageType::PointType originVol = volumeImageITK->GetOrigin();
  typename VolumeImageType::DirectionType directionVol = volumeImageITK->GetDirection();

  // Mask inside and outside values
  // NOTE: mask is of type unsigned char
  unsigned char maskOutsideValue = 0;
  unsigned char maskInsideValue = 255;

  // Create template image (required as input for the image stencil operation)
  vtkSmartPointer<vtkImageData> templateImageAllInside = vtkSmartPointer<vtkImageData>::New();
  templateImageAllInside->SetOrigin(volumeImageVTK->GetOrigin());
  templateImageAllInside->SetSpacing(volumeImageVTK->GetSpacing());
  templateImageAllInside->SetDimensions(volumeImageVTK->GetDimensions());
  templateImageAllInside->SetExtent(volumeImageVTK->GetWholeExtent());
  templateImageAllInside->SetScalarTypeToUnsignedChar();
  templateImageAllInside->AllocateScalars();
  // Fill the template image with the inside value
  vtkSmartPointer<vtkDataArray> scalars = templateImageAllInside->GetPointData()->GetScalars();
  for(vtkIdType i = 0; i < scalars->GetNumberOfTuples(); i++)
  {
    scalars->SetTuple1(i, maskInsideValue);
  }

  // Create a result image (initial all voxels are inside)
  typedef unsigned char MaskVolumePixelType;
  typedef itk::Image<MaskPixelType, 3> MaskVolumeImageType;
  MaskVolumeImageType::Pointer resultMaskImage = MaskVolumeImageType::New();
  resultMaskImage->SetRegions( volumeImageITK->GetLargestPossibleRegion());
  resultMaskImage->Allocate();
  resultMaskImage->FillBuffer(maskInsideValue);
  resultMaskImage->SetSpacing(volumeImageITK->GetSpacing());
  resultMaskImage->SetOrigin(volumeImageITK->GetOrigin());
  resultMaskImage->SetDirection(volumeImageITK->GetDirection());

  typedef itk::ImageRegionIterator<MaskVolumeImageType> MaskIteratorType;
  MaskIteratorType itMask(resultMaskImage, resultMaskImage->GetLargestPossibleRegion());
  itMask.GoToBegin();

  // Convert directional cosine matrix of ITK to VTK transform matrix to transform
  // structures to volume image space for mask generation
  vtkSmartPointer<vtkMatrix4x4> dc = vtkSmartPointer<vtkMatrix4x4>::New();
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

  // Iterate over provided structures and generate mask images with vtkImageStencil
  for (unsigned int i = 0; i < maskstructures.size(); ++i)
  {
    // Get polydata
    vtkPolyData *structure = maskstructures[i];

    // Transform polydata to volume image space (VTK has no direction)
    vtkSmartPointer<vtkTransform> volumeTransform = vtkSmartPointer<vtkTransform>::New();
    volumeTransform->Identity();
    volumeTransform->PostMultiply();
    volumeTransform->Translate(-originVol[0], -originVol[1], -originVol[2]);
    volumeTransform->Concatenate(dc);
    volumeTransform->Translate(originVol[0], originVol[1], originVol[2]);
    volumeTransform->Update();

    vtkSmartPointer<vtkTransformPolyDataFilter> transformToVolumeSpace =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformToVolumeSpace->SetTransform(volumeTransform);
    transformToVolumeSpace->SetInput(structure);
    transformToVolumeSpace->Update();

    // Subdivide for more stability
    vtkSmartPointer<vtkTriangleFilter> triangleFilter =
       vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInput(transformToVolumeSpace->GetOutput());
    triangleFilter->Update();

    vtkSmartPointer<vtkLinearSubdivisionFilter> subdivisionFilter =
        vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
    subdivisionFilter->SetInput(triangleFilter->GetOutput());
    subdivisionFilter->SetNumberOfSubdivisions(1);
    subdivisionFilter->Update();

    // Convert polygonal data to image stencil
    vtkSmartPointer<vtkPolyDataToImageStencil> polydataToStencil =
      vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    // Add additional margin to account for voxels on the edge
    polydataToStencil->SetOutputOrigin(templateImageAllInside->GetOrigin());
    polydataToStencil->SetOutputSpacing(templateImageAllInside->GetSpacing());
    polydataToStencil->SetOutputWholeExtent(templateImageAllInside->GetWholeExtent());
    polydataToStencil->SetInput(subdivisionFilter->GetOutput());
    try
    {
      polydataToStencil->Update();
    }
    catch (...)
    {
      std::cerr << "ERROR at generation of volume stencil (polydata id " << i << ")." << std::endl;
      return NULL;
    }

    // Create a volume image mask (voxels inside the polydata get the inside
    // value)
    vtkSmartPointer<vtkImageStencil> stencilToVolume =
      vtkSmartPointer<vtkImageStencil>::New();
    stencilToVolume->SetStencil(polydataToStencil->GetOutput());
    stencilToVolume->ReverseStencilOff();
    stencilToVolume->SetInput(templateImageAllInside);
    stencilToVolume->SetBackgroundValue(maskOutsideValue);
    try
    {
      stencilToVolume->Update();
    }
    catch (...)
    {
      std::cerr << "ERROR at generation of volume mask (polydata id " << i << ")." << std::endl;
      return NULL;
    }
    // Combine current structure mask with accumulated result
    itMask.GoToBegin();
    vtkSmartPointer<vtkDataArray> scalarsStructure = stencilToVolume->GetOutput()->GetPointData()->GetScalars();
    for(vtkIdType j = 0; j < scalarsStructure->GetNumberOfTuples(); j++)
    {
      // Remove voxels from mask that are outside of the structure and preserve
      // previous removed voxels
      if (!(static_cast<int>(scalarsStructure->GetTuple1(j)) == maskOutsideValue &&
          static_cast<int>(itMask.Value()) == maskInsideValue))
      {
        itMask.Value() = maskOutsideValue;
      }
      ++itMask;
    }
  }

return resultMaskImage;
}


}

#endif /* ORAREG23MODEL_TXX_ */
