//
#ifndef ORAMULTIRESOLUTIONNWAY2D3DREGISTRATIONFRAMEWORK_TXX_
#define ORAMULTIRESOLUTIONNWAY2D3DREGISTRATIONFRAMEWORK_TXX_

#include "oraMultiResolutionNWay2D3DRegistrationFramework.h"
//internal
#include "oraMetricEvents.hxx"
//ITK
#include <itkImageRegionIterator.h>

namespace ora
{

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::MultiResolutionNWay2D3DRegistrationFramework() :
  Superclass()
{
  this->m_Moving3DVolume = NULL;
  this->m_FixedImages.clear();
  this->m_FixedImageRegions.clear();
  this->m_DRREngine = NULL;
  this->m_Metric = MetricType::New();
  this->m_Optimizer = NULL;
  this->m_Transform = NULL;
  this->m_InitialTransformParameters.SetSize(0);
  this->m_InternalTransforms.clear();
  this->m_FixedPyramids.clear();
  this->m_ProjectionGeometries.clear();
  this->m_DimensionReducers.clear();
  this->m_MaskCasters.clear();
  this->m_InformationChangers.clear();
  this->m_Interpolators.clear();
  this->m_FixedRegionPyramids.clear();
  this->m_MaskPyramids.clear();
  this->m_Masks.clear();
  this->m_MetricMasks.clear();
  this->m_MovingPyramid = MovingPyramidType::New();
  this->m_FixedSchedule.SetSize(1, 2);
  this->m_FixedSchedule.Fill(1); // 1 level without resolution change
  this->m_MovingSchedule.SetSize(1, 3);
  this->m_MovingSchedule.Fill(1); // 1 level without resolution change
  this->m_VolumeCaster = VolumeCasterType::New();
  this->m_VolumeBackCaster = VolumeBackCasterType::New();
  this->m_MetricFixedImageMappings.clear();
  this->m_CurrentLevel = 0;
  this->m_UseMovingPyramidForFinalLevel = true;
  this->m_UseMovingPyramidForUnshrinkedLevels = true;
  this->m_UseFixedPyramidForFinalLevel = true;
  this->m_UseFixedPyramidForUnshrinkedLevels = true;
  this->m_EvaluationCommand = ReceptorType::New();
  this->m_EvaluationCommand->SetCallbackFunction(this,
      &Self::OnMetricEvaluationEvent);
  this->m_BeforeEvaluationObserverTag = INVALID_OBSERVER_TAG;
  this->m_AfterEvaluationObserverTag = INVALID_OBSERVER_TAG;
  this->m_TransformsCommand = MemberType::New();
  this->m_TransformsCommand->SetCallbackFunction(this, &Self::OnTransformsEvent);
  this->m_LastMetricParameters.SetSize(0);
  this->m_LastMetricValue = 0;
  this->m_LastMetricDerivative.SetSize(0);
  this->m_UseAutoProjectionPropsAdjustment = true;
  this->m_NumberOfMetricEvaluationsAtLevel = 0;
  this->m_Stop = false;
  TransformOutputPointer transformDecorator =
      static_cast<TransformOutputType *> (this->MakeOutput(0).GetPointer());
  this->ProcessObject::SetNthOutput(0, transformDecorator.GetPointer());
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::~MultiResolutionNWay2D3DRegistrationFramework()
{
  this->m_Moving3DVolume = NULL;
  this->RemoveAllFixedImagesAndProps();
  this->m_DRREngine = NULL;
  this->m_Metric = NULL;
  this->m_Transform = NULL;
  this->m_MovingPyramid = NULL;
  this->m_VolumeCaster = NULL;
  this->m_VolumeBackCaster = NULL;
  this->m_EvaluationCommand = NULL;
  this->m_TransformsCommand = NULL;
  this->m_Optimizer = NULL;
  this->RemoveAllMetricFixedImageMappings();
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::PrintSelf(std::ostream& os,
    itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Moving 3D Volume: " << this->m_Moving3DVolume.GetPointer()
      << "\n";
  os << indent << "Fixed Images: N=" << this->m_FixedImages.size() << "\n";
  os << indent << "Fixed Image Regions: N=" << this->m_FixedImageRegions.size()
      << "\n";
  os << indent << "Projection Geometries: N=" <<   this->m_ProjectionGeometries.size()
      << "\n";
  os << indent << "DRR Engine: " << this->m_DRREngine.GetPointer() << "\n";
  os << indent << "Metric: " << this->m_Metric.GetPointer() << "\n";
  os << indent << "Transform: " << this->m_Transform.GetPointer() << "\n";
  os << indent << "Initial Transform Parameters: "
      << this->m_InitialTransformParameters << "\n";
  os << indent << "Internal Transforms: N="
      << this->m_InternalTransforms.size() << "\n";
  os << indent << "Fixed Pyramids: N=" << this->m_FixedPyramids.size() << "\n";
  os << indent << "Fixed Region Pyramids: N="
      << this->m_FixedRegionPyramids.size() << "\n";
  os << indent << "DRR Masks: N=" << this->m_Masks.size() << "\n";
  os << indent << "Dimension Reducers: N=" << this->m_DimensionReducers.size()
      << "\n";
  os << indent << "Information Changers: N="
      << this->m_InformationChangers.size() << "\n";
  os << indent << "Interpolators: N=" << this->m_Interpolators.size() << "\n";
  os << indent << "Fixed Schedule: " << this->m_FixedSchedule << "\n";
  os << indent << "Use Fixed Pyramid For Unshrinked Levels: "
      << this->m_UseFixedPyramidForUnshrinkedLevels << "\n";
  os << indent << "Use Fixed Pyramid For Final Level: "
      << this->m_UseFixedPyramidForFinalLevel << "\n";
  os << indent << "Moving Pyramid: " << this->m_MovingPyramid.GetPointer()
      << "\n";
  os << indent << "Moving Schedule: " << this->m_MovingSchedule << "\n";
  os << indent << "Use Moving Pyramid For Unshrinked Levels: "
      << this->m_UseMovingPyramidForUnshrinkedLevels << "\n";
  os << indent << "Use Moving Pyramid For Final Level: "
      << this->m_UseMovingPyramidForFinalLevel << "\n";
  os << indent << "Volume Caster: " << this->m_VolumeCaster.GetPointer()
      << "\n";
  os << indent << "Volume Back Caster: "
      << this->m_VolumeBackCaster.GetPointer() << "\n";
  os << indent << "Metric Fixed Image Mappings: N="
      << this->m_MetricFixedImageMappings.size() << "\n";
  os << indent << "Current Level: " << this->m_CurrentLevel << "\n";
  os << indent << "Before Evaluation Observer Tag: "
      << this->m_BeforeEvaluationObserverTag << "\n";
  os << indent << "After Evaluation Observer Tag: "
      << this->m_AfterEvaluationObserverTag << "\n";
  os << indent << "Evaluation Command: "
      << this->m_EvaluationCommand.GetPointer() << "\n";
  os << indent << "Transforms Command: "
      << this->m_TransformsCommand.GetPointer() << "\n";
  os << indent << "Optimizer: " << this->m_Optimizer.GetPointer() << "\n";
  os << indent << "Last Metric Parameters: " << this->m_LastMetricParameters
      << "\n";
  os << indent << "Last Metric Value: " << this->m_LastMetricValue << "\n";
  os << indent << "Last Metric Derivative: " << this->m_LastMetricDerivative
      << "\n";
  os << indent << "Use Auto Projection Props Adjustment: "
      << this->m_UseAutoProjectionPropsAdjustment << "\n";
  os << indent << "Number Of Metric Evaluations At Level: "
      << this->m_NumberOfMetricEvaluationsAtLevel << "\n";
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::SetMoving3DVolume(
    Moving3DImagePointer image)
{
  if (image != this->m_Moving3DVolume)
  {
    this->m_Moving3DVolume = image;
    this->Modified();
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::SetDRREngine(DRREnginePointer drrEngine)
{
  // only set Filter if it was not set before
  if(this->m_DRREngine)
    return;
  this->m_DRREngine = drrEngine;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
std::size_t MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::GetNumberOfFixedImages() const
{
  return this->m_FixedImages.size();
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
bool MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
	TMoving2DImage, TMoving3DImage>::AddFixedImageAndProps(
    FixedImagePointer fixedImage, FixedImageRegionType fixedImageRegion,
    ProjectionPointer projGeom, MaskImagePointer maskImage)
{
  typedef itk::ChangeInformationImageFilter<FixedImageType> ChangerType;
  typedef typename ChangerType::Pointer ChangerPointer;

  if (!fixedImage || !projGeom)
    return false;

  // prepare mask-related vectors:
  this->m_Masks.push_back(NULL);
  this->m_MaskPyramids.push_back(NULL);
  this->m_MaskCasters.push_back(NULL);
  this->m_MetricMasks.push_back(NULL);

  // check whether this image has not already been added:
  for (std::size_t i = 0; i < this->GetNumberOfFixedImages(); i++)
  {
    if (this->m_FixedImages[i] == fixedImage)
      return false; // do not add twice!
  }

  bool regionOK = true;
  FixedImageRegionType largest = fixedImage->GetLargestPossibleRegion();
  for (unsigned int d = 0; d < 2; d++)
  {
    if (fixedImageRegion.GetIndex()[d] < largest.GetIndex()[d]
        || fixedImageRegion.GetIndex()[d] >= (int) (largest.GetIndex()[d]
            + largest.GetSize()[d]))
    {
      regionOK = false;
    }
    if ((fixedImageRegion.GetIndex()[d] + fixedImageRegion.GetSize()[d])
        > (largest.GetIndex()[d] + largest.GetSize()[d]))
    {
      regionOK = false;
    }
  }
  if (!regionOK)
    return false;

  // mask region must equal fixed image region, spacing must be OK:
  if (maskImage)
  {
  	MaskRegionType maskRegion = maskImage->GetLargestPossibleRegion();
  	if (maskRegion != largest) // must equal
  		return false;
  	MaskSpacingType maskSpacing = maskImage->GetSpacing();
  	if (maskSpacing != fixedImage->GetSpacing())
  		return false;
  }

  // -> extract fixed image region from fixed image and redefine the
  // fixed image region:
  typedef itk::ExtractImageFilter<FixedImageType, FixedImageType>
      Extractor2DType;
  typedef typename Extractor2DType::Pointer Extractor2DPointer;

  FixedImageRegionType originalFixedImageRegion = fixedImageRegion; // store

  Extractor2DPointer extract2D = Extractor2DType::New();
  extract2D->SetExtractionRegion(fixedImageRegion);
  extract2D->SetInput(fixedImage);
  FixedImagePointer image = extract2D->GetOutput();
  extract2D->Update();
  image->DisconnectPipeline();
  // reset fixed image region
  fixedImageRegion.SetIndex(0, 0);
  fixedImageRegion.SetIndex(1, 0);
  image->SetLargestPossibleRegion(fixedImageRegion);
  image->SetRequestedRegion(fixedImageRegion);
  image->SetBufferedRegion(fixedImageRegion);
  this->m_FixedImages.push_back(image);

  // use the fixed image index in the drr engine to avoid confusion
  int idx = static_cast<int>(this->m_FixedImages.size());
  if(this->m_DRREngine->GetNumberOfIndependentOutputs()<idx)
	  this->m_DRREngine->SetNumberOfIndependentOutputs(idx);
  this->m_ProjectionGeometries.push_back(projGeom);
  this->m_FixedImageRegions.push_back(fixedImageRegion);
  // add an exclusive internal transform for each fixed image:
  IdentityTransformPointer internalTransform = IdentityTransformType::New();
  // (connected 3D transform is set later ...)
  this->m_InternalTransforms.push_back(internalTransform.GetPointer());
  // add a fixed pyramid for the new fixed image
  FixedPyramidPointer pyramid = FixedPyramidType::New();
  pyramid->SetNumberOfLevels(1); // some default -> will be corrected later
  pyramid->SetInput(image); // connect the fixed image to the pyramid
  this->m_FixedPyramids.push_back(pyramid);
  // add a fixed image region pyramid for the new image
  FixedImageRegionPyramidType firp;
  firp.push_back(fixedImageRegion); // original region @ level 0
  this->m_FixedRegionPyramids.push_back(firp);
  // add a dimension reducer for the associated moving image later
  ExtractorPointer extractor = ExtractorType::New();
  Region3DType eregion;
  Index3DType eidx;
  eidx[0] = 0; // DRR-output is always 0-indexed (as the geometry is adjusted)
  eidx[1] = 0;
  eidx[2] = 0; // first (and only) slice
  eregion.SetIndex(eidx);
  Size3DType esz;
  esz[0] = fixedImageRegion.GetSize()[0];
  esz[1] = fixedImageRegion.GetSize()[1];
  esz[2] = 0; // no 3rd dimension anymore
  eregion.SetSize(esz);
  extractor->SetExtractionRegion(eregion); // preliminary extract region
  this->m_DimensionReducers.push_back(extractor);
  // add an information changer for the moving 2D image later which sets
  // the image origin constantly to 0,0:
  ChangeInfoPointer changer = ChangeInfoType::New();
  changer->ChangeNone();
  changer->ChangeOriginOn();
  typename FixedImageType::PointType origin;
  origin.Fill(0);
  changer->SetOutputOrigin(origin);
  changer->ChangeDirectionOn();
  typename FixedImageType::DirectionType direction;
  direction.SetIdentity();
  changer->SetOutputDirection(direction);
  this->m_InformationChangers.push_back(changer);
  // add an interpolator for each moving 2D image (for each sub-metric)
  InterpolatorPointer interpolator = InterpolatorType::New();
  this->m_Interpolators.push_back(interpolator);

  // optional mask image:
  if (maskImage)
  {
  	int fi = this->m_FixedImages.size() - 1;

    // -> extract fixed image region from mask image (if necessary):
    typedef itk::ExtractImageFilter<MaskImageType, MaskImageType>
        MaskExtractor2DType;
    typedef typename MaskExtractor2DType::Pointer MaskExtractor2DPointer;
    MaskExtractor2DPointer maskExtract2D = MaskExtractor2DType::New();
    maskExtract2D->SetExtractionRegion(originalFixedImageRegion);
    maskExtract2D->SetInput(maskImage);
    MaskImagePointer mask = maskExtract2D->GetOutput();
    maskExtract2D->Update();
    mask->DisconnectPipeline();
    // reset mask image region
    MaskRegionType maskRegion = mask->GetLargestPossibleRegion();
    maskRegion.SetIndex(0, 0);
    maskRegion.SetIndex(1, 0);
    mask->SetLargestPossibleRegion(maskRegion);
    mask->SetRequestedRegion(maskRegion);
    mask->SetBufferedRegion(maskRegion);
    // set origin / direction to default values:
    MaskPointType maskOrig;
    maskOrig.Fill(0);
    MaskDirectionType maskDir;
    maskDir.SetIdentity();
    mask->SetOrigin(maskOrig);
    mask->SetDirection(maskDir);
    this->m_Masks[fi] = mask; // set pointer

    // prepare mask pyramid:
    MaskPyramidPointer pyramid = MaskPyramidType::New();
    pyramid->SetUseShrinkImageFilter(true); // shrink-based
    pyramid->SetInput(mask);
    pyramid->SetNumberOfLevels(this->GetNumberOfLevels());
    this->m_MaskPyramids[fi] = pyramid;

    // prepare spatial mask objects:
    MetricMaskPointer metricMask = MetricMaskType::New();
		this->m_MetricMasks[fi] = metricMask;

		// prepare 2D -> 3D mask caster:
		MaskCasterPointer maskCaster = MaskCasterType::New();
		this->m_MaskCasters[fi] = maskCaster;
  }

  this->Modified();

  return true;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
bool MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::AddFixedImageAndProps(
    typename DRREngineType::OutputImagePointer fixedImage,
    typename DRREngineType::OutputImageType::RegionType fixedImageRegion,
    ProjectionPointer projGeom,
    typename DRREngineType::MaskImagePointer maskImage)
{
  if (!fixedImage || fixedImageRegion.GetIndex()[2] != 0
      || fixedImageRegion.GetSize()[2] != 1)
    return false;

  if (maskImage)
  {
    if (maskImage->GetLargestPossibleRegion().GetIndex()[2] != 0 ||
    		maskImage->GetLargestPossibleRegion().GetSize()[2] != 1)
      return false;
  }

  bool regionOK = true;
  typename DRREngineType::OutputImageType::RegionType largest;
  largest = fixedImage->GetLargestPossibleRegion();
  for (unsigned int d = 0; d < 2; d++)
  {
    if (fixedImageRegion.GetIndex()[d] < largest.GetIndex()[d]
        || fixedImageRegion.GetIndex()[d] >= (int) (largest.GetIndex()[d]
            + largest.GetSize()[d]))
    {
      regionOK = false;
    }
    if ((fixedImageRegion.GetIndex()[d] + fixedImageRegion.GetSize()[d])
        > (largest.GetIndex()[d] + largest.GetSize()[d]))
    {
      regionOK = false;
    }
  }
  if (!regionOK)
    return false;

  typedef typename DRREngineType::OutputImageType::RegionType Region3DType;
  typedef typename DRREngineType::OutputImageType::IndexType Index3DType;
  typedef typename DRREngineType::OutputImageType::SizeType Size3DType;

  typename DRREngineType::OutputImageType::DirectionType storedDir =
      fixedImage->GetDirection();
  typename DRREngineType::OutputImageType::DirectionType identityDir;
  identityDir.SetIdentity();
  fixedImage->SetDirection(identityDir); // identity for casting
  ExtractorPointer extractFilter = ExtractorType::New();
  Region3DType region3D = fixedImage->GetLargestPossibleRegion();
  Index3DType idx = fixedImageRegion.GetIndex(); // extract fixed image region
  Size3DType sz = fixedImageRegion.GetSize();
  idx[2] = 0; // first and only slice
  sz[2] = 0; // no 3rd dimension
  Region3DType extractRegion;
  extractRegion.SetIndex(idx);
  extractRegion.SetSize(sz);
  extractFilter->SetExtractionRegion(extractRegion);
  extractFilter->SetInput(fixedImage);
  FixedImageRegionType fixedRegion2D;
  FixedImagePointer fixed2DImage = NULL;
  try
  {
    extractFilter->Update();
    fixed2DImage = extractFilter->GetOutput();
    fixed2DImage->DisconnectPipeline();
    extractFilter = NULL;
    // extracted region of interest, therefore take the largest possible now
    fixedRegion2D = fixed2DImage->GetLargestPossibleRegion();
    fixedRegion2D.SetIndex(0, 0);
    fixedRegion2D.SetIndex(1, 0);
    fixed2DImage->SetLargestPossibleRegion(fixedRegion2D);
    fixed2DImage->SetBufferedRegion(fixedRegion2D);
    fixed2DImage->SetRequestedRegionToLargestPossibleRegion();
  }
  catch (itk::ExceptionObject &e)
  {
    fixed2DImage = NULL;
  }
  fixedImage->SetDirection(storedDir); // restore after casting

  MaskImagePointer mask2DImage = NULL;
  if (maskImage)
  {
  	typename DRREngineType::MaskImageType::DirectionType storedMaskDir =
        maskImage->GetDirection();
  	typename DRREngineType::MaskImageType::DirectionType identityMaskDir;
    identityMaskDir.SetIdentity();
    maskImage->SetDirection(identityMaskDir); // identity for casting
    MaskExtractorPointer extractMaskFilter = MaskExtractorType::New();
    extractMaskFilter->SetExtractionRegion(extractRegion);
    extractMaskFilter->SetInput(maskImage);
    try
    {
      extractMaskFilter->Update();
      mask2DImage = extractMaskFilter->GetOutput();
      mask2DImage->DisconnectPipeline();
      extractMaskFilter = NULL;
      // extracted region of interest, therefore take the largest possible now
      MaskRegionType maskRegion2D = mask2DImage->GetLargestPossibleRegion();
      maskRegion2D.SetIndex(0, 0);
      maskRegion2D.SetIndex(1, 0);
      mask2DImage->SetLargestPossibleRegion(maskRegion2D);
      mask2DImage->SetBufferedRegion(maskRegion2D);
      mask2DImage->SetRequestedRegionToLargestPossibleRegion();
    }
    catch (itk::ExceptionObject &e)
    {
      mask2DImage = NULL;
    }
    maskImage->SetDirection(storedMaskDir); // restore after casting
  }

  return AddFixedImageAndProps(fixed2DImage, fixedRegion2D, projGeom, mask2DImage);
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
typename MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::FixedImagePointer MultiResolutionNWay2D3DRegistrationFramework<
    TFixed2DImage, TMoving2DImage, TMoving3DImage>::GetIthFixedImage(
    std::size_t i)
{
  if (i >= this->GetNumberOfFixedImages())
    return NULL;
  return this->m_FixedImages[i];
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
typename MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::ProjectionPointer MultiResolutionNWay2D3DRegistrationFramework<
    TFixed2DImage, TMoving2DImage, TMoving3DImage>::GetIthProjectionProps(
    std::size_t i)
{
  if (i >= this->GetNumberOfFixedImages())
    return NULL;
  return this->m_ProjectionGeometries[i];
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
typename MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::FixedImageRegionType MultiResolutionNWay2D3DRegistrationFramework<
    TFixed2DImage, TMoving2DImage, TMoving3DImage>::GetIthFixedImageRegion(
    std::size_t i)
{
  if (i >= this->GetNumberOfFixedImages())
  {
    FixedImageRegionType dummy;
    typename FixedImageRegionType::IndexType idx;
    idx.Fill(0);
    typename FixedImageRegionType::SizeType sz;
    sz.Fill(0);
    dummy.SetIndex(idx);
    dummy.SetSize(sz);
    return dummy;
  }
  return this->m_FixedImageRegions[i];
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
typename MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
TMoving2DImage, TMoving3DImage>::MaskImagePointer
MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::GetIthDRRMask(std::size_t i)
{
  if (i >= this->GetNumberOfFixedImages())
    return NULL;
  return this->m_Masks[i];
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
bool MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::RemoveIthFixedImageAndProps(std::size_t i)
{
  if (i >= this->GetNumberOfFixedImages())
    return false;

  this->m_FixedImages[i] = NULL;
  this->m_FixedImages.erase(this->m_FixedImages.begin() + i);
  this->m_DRREngine->RemoveDRROutput(i);
  this->m_ProjectionGeometries[i] = NULL;
  this->m_DimensionReducers[i] = NULL;
  this->m_DimensionReducers.erase(this->m_DimensionReducers.begin()+i);
  this->m_ProjectionGeometries.erase(this->m_ProjectionGeometries.begin() + i);
  this->m_InternalTransforms[i] = NULL;
  this->m_InternalTransforms.erase(this->m_InternalTransforms.begin() + i);
  this->m_FixedPyramids[i] = NULL;
  this->m_FixedPyramids.erase(this->m_FixedPyramids.begin() + i);
  this->m_MaskCasters[i] = NULL;
  this->m_MaskCasters.erase(this->m_MaskCasters.begin() + i);
  this->m_MetricMasks[i] = NULL;
  this->m_MetricMasks.erase(this->m_MetricMasks.begin() + i);
  this->m_InformationChangers[i] = NULL;
  this->m_InformationChangers.erase(this->m_InformationChangers.begin() + i);
  this->m_Interpolators[i] = NULL;
  this->m_Masks[i] = NULL;
  this->m_Masks.erase(this->m_Masks.begin() + i);
  this->m_MaskPyramids[i] = NULL;
  this->m_MaskPyramids.erase(this->m_MaskPyramids.begin() + i);
  this->m_Interpolators.erase(this->m_Interpolators.begin() + i);
  this->m_FixedImageRegions.erase(this->m_FixedImageRegions.begin() + i);
  this->m_FixedRegionPyramids.erase(this->m_FixedRegionPyramids.begin() + i);

  return true;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::RemoveAllFixedImagesAndProps()
{
  for (std::size_t i = 0; i < this->GetNumberOfFixedImages(); i++)
  {
    this->m_FixedImages[i] = NULL;
    this->m_FixedImages[i] = NULL;
    this->m_InternalTransforms[i] = NULL;
    this->m_FixedPyramids[i] = NULL;
    this->m_DimensionReducers[i] = NULL;
    this->m_MaskCasters[i] = NULL;
    this->m_MetricMasks[i] = NULL;
    this->m_InformationChangers[i] = NULL;
    this->m_Interpolators[i] = NULL;
    this->m_ProjectionGeometries[i]= NULL;
    this->m_Masks[i] = NULL;
    this->m_MaskPyramids[i] = NULL;
  }
  this->m_MetricMasks.clear();
  this->m_FixedImages.clear();
  this->m_InternalTransforms.clear();
  this->m_FixedPyramids.clear();
  this->m_DimensionReducers.clear();
  this->m_InformationChangers.clear();
  this->m_Interpolators.clear();
  this->m_FixedImageRegions.clear();
  this->m_FixedRegionPyramids.clear();
  this->m_ProjectionGeometries.clear();
  this->m_Masks.clear();
  this->m_MaskPyramids.clear();
  this->m_MaskCasters.clear();
  this->m_MetricMasks.clear();
  this->m_DRREngine->SetNumberOfIndependentOutputs(1);
  this->m_DRREngine->SetProjectionGeometry(0, NULL);
  this->m_DRREngine->SetDRRMask(0, NULL);
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
const typename MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::TransformOutputType *
MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::GetOutput() const
{
  return static_cast<const TransformOutputType *> (this->itk::ProcessObject::GetOutput(
      0));
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
itk::DataObject::Pointer MultiResolutionNWay2D3DRegistrationFramework<
    TFixed2DImage, TMoving2DImage, TMoving3DImage>::MakeOutput(
    unsigned int idx)
{
  if (idx == 0)
    return static_cast<itk::DataObject*> (TransformOutputType::New().GetPointer());

  return NULL;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::ApplyMovingMultiResolutionSettings()
{
  this->m_MovingPyramid->SetNumberOfLevels(this->GetNumberOfLevels());
  this->m_MovingPyramid->SetSchedule(this->m_MovingSchedule);
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::ApplyFixedMultiResolutionSettings()
    throw (itk::ExceptionObject)
{
  unsigned int numMetrics = this->m_Metric->GetNumberOfMetricInputs();

  for (unsigned int i = 0; i < numMetrics; i++)
  {
    BaseMetricPointer subMetric = this->m_Metric->GetIthMetricInput(i);
    if (!subMetric)
      itkExceptionMacro(<< "Invalid metric sub-component detected.")

    int fi = this->GetMappedFixedImageIndex(subMetric);
    if (fi >= 0)
    {
      // set masks pyramid schedule (NOTE: mask schedule = fixed schedule)
      if(this->m_Masks[fi])
      {
        this->m_MaskPyramids[fi]->SetNumberOfLevels(this->GetNumberOfLevels());
        this->m_MaskPyramids[fi]->SetSchedule(this->m_FixedSchedule);
        this->m_MaskPyramids[fi]->UpdateLargestPossibleRegion();
      }
      // image data
      this->m_FixedPyramids[fi]->SetNumberOfLevels(this->GetNumberOfLevels());
      this->m_FixedPyramids[fi]->SetSchedule(this->m_FixedSchedule);
      this->m_FixedPyramids[fi]->UpdateLargestPossibleRegion();
      // image region data
      this->m_FixedRegionPyramids[fi].reserve(this->GetNumberOfLevels());
      this->m_FixedRegionPyramids[fi].resize(this->GetNumberOfLevels());
      // use the algorithm of shrink image filter to adapt the fixed image
      // regions as well:
      typedef typename FixedImageRegionType::SizeType SizeType;
      typedef typename FixedImageRegionType::IndexType IndexType;
      SizeType inputSize = this->m_FixedImageRegions[fi].GetSize();
      IndexType inputStart = this->m_FixedImageRegions[fi].GetIndex();
      for (unsigned int level = 0; level < this->GetNumberOfLevels(); level++)
      {
        SizeType size;
        IndexType start;
        for (unsigned int dim = 0; dim < 2; dim++)
        {
          const float scaleFactor =
              static_cast<float> (this->m_FixedSchedule[level][dim]);

          size[dim] = static_cast<typename SizeType::SizeValueType> (vcl_floor(
              static_cast<float> (inputSize[dim]) / scaleFactor));
          if (size[dim] < 1)
          {
            size[dim] = 1;
          }

          start[dim]
              = static_cast<typename IndexType::IndexValueType> (vcl_ceil(
                  static_cast<float> (inputStart[dim]) / scaleFactor));
        }
        this->m_FixedRegionPyramids[fi][level].SetSize(size);
        this->m_FixedRegionPyramids[fi][level].SetIndex(start);
      }
    }
    else
    {
      itkExceptionMacro(<< "Missing mapping between metric and fixed " <<
          "image (index: " << i << ").")
    }
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::UpdateMovingImageMultiResolutionConnections(
    unsigned int level)
{
  if (level >= this->GetNumberOfLevels())
    return;

  bool useOriginalVolume = false;
  if (!this->m_UseMovingPyramidForFinalLevel && this->GetNumberOfLevels()
      == (level + 1))
    useOriginalVolume = true;
  if (!this->m_UseMovingPyramidForUnshrinkedLevels && fabs(
      this->m_MovingSchedule[level][0] - 1.0) < 1e-6 && fabs(
      this->m_MovingSchedule[level][1] - 1.0) < 1e-6 && fabs(
      this->m_MovingSchedule[level][2] - 1.0) < 1e-6)
    useOriginalVolume = true;

  if (useOriginalVolume) // use original image
  {
    this->m_DRREngine->SetInput(this->m_Moving3DVolume); // direct conn.
  }
  else // use pyramid
  {
    // NOTE: internally the multi-resolution pyramid image filter makes use of
    // DiscreteGaussianImageFilter which can only handle signed image types;
    // therefore we need two intermediate casters which cast the volume to
    // a signed type and than back-converts it to the original type:

    // connect respective resolution level
    this->m_VolumeCaster->SetInput(this->m_Moving3DVolume);
    this->m_MovingPyramid->SetInput(this->m_VolumeCaster->GetOutput());
    this->m_VolumeBackCaster->SetInput(this->m_MovingPyramid->GetOutput(level));

    // NOTE: Although the image origins computed by the pyramid filter seem to
    // be shifted, they are correct. The rationale for recomputing the image
    // origin is that it is the overall goal to keep the center of mass of any
    // objects within the image unchanged in physical coordinates.

    this->m_VolumeBackCaster->UpdateLargestPossibleRegion();

    // indirect connection: pyramid filter
    this->m_DRREngine->SetInput(this->m_VolumeBackCaster->GetOutput());
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::UpdateFixedImageMultiResolutionConnections(
    unsigned int level)
{
  if (level >= this->GetNumberOfLevels())
    return;

  bool useOriginalImages = false;
  if (!this->m_UseFixedPyramidForFinalLevel && this->GetNumberOfLevels()
      == (level + 1))
    useOriginalImages = true;
  if (!this->m_UseFixedPyramidForUnshrinkedLevels && fabs(
      this->m_FixedSchedule[level][0] - 1.0) < 1e-6 && fabs(
      this->m_FixedSchedule[level][1] - 1.0) < 1e-6 && fabs(
      this->m_FixedSchedule[level][2] - 1.0) < 1e-6)
    useOriginalImages = true;

  unsigned int numMetrics = this->m_Metric->GetNumberOfMetricInputs();
  for (unsigned int i = 0; i < numMetrics; i++)
  {
    BaseMetricPointer subMetric = this->m_Metric->GetIthMetricInput(i);
    if (!subMetric)
      continue;

    int fi = this->GetMappedFixedImageIndex(subMetric);
    if (fi >= 0)
    {
      // NOTE: The rationale for recomputing the image origin at each multi-
      // resolution level is to keep the center of mass of any object within
      // the image unchanged in physical space.
      // However, the fixed image will be translated to an arbitrary origin
      // (here 0,0 in 2D-space) and the moving 2D image (DRR) is expected to
      // cover the fixed image perfectly (also origin 0,0 but considers the
      // pyramid-induced offset in parametrization stage).
      // -> constantly set the fixed image origin to 0,0:
      ChangeInfoPointer changer = ChangeInfoType::New();
      changer->ChangeNone();
      changer->ChangeOriginOn();
      typename FixedImageType::PointType origin;
      origin.Fill(0);
      changer->SetOutputOrigin(origin);

      if (useOriginalImages) // use original image
        changer->SetInput(this->m_FixedImages[fi]);
      else
        // use pyramid
        changer->SetInput(this->m_FixedPyramids[fi]->GetOutput(level));

      changer->Update();
      subMetric->SetFixedImage(changer->GetOutput());
      changer = NULL;

      // update the extraction region for 3D -> 2D dimension reduction of the
      // internal 3D DRR as well:
      Region3DType eregion;
      Index3DType eidx;
      eidx[0] = 0; // (already adapted to fixed image region!)
      eidx[1] = 0; // (already adapted to fixed image region!)
      eidx[2] = 0; // first (and only) slice
      eregion.SetIndex(eidx);
      Size3DType esz;
      esz[0] = this->m_FixedRegionPyramids[fi][level].GetSize()[0];
      esz[1] = this->m_FixedRegionPyramids[fi][level].GetSize()[1];
      esz[2] = 0; // no 3rd dimension anymore
      eregion.SetSize(esz);
      this->m_DimensionReducers[fi]->SetExtractionRegion(eregion);

      // update fixed image region of sub-metrics:
      subMetric->SetFixedImageRegion(this->m_FixedRegionPyramids[fi][level]);

      // update masking pipeline if set:
      if (this->m_Masks[fi])
      {
      	if (level != (this->GetNumberOfLevels() - 1))
      	{
      		this->m_MetricMasks[fi]->SetImage(
      				this->m_MaskPyramids[fi]->GetOutput(level));
      	}
      	else
      	{
      		this->m_MetricMasks[fi]->SetImage(this->m_Masks[fi]);
      	}
				this->m_MetricMasks[fi]->Update();
				subMetric->SetFixedImageMask(this->m_MetricMasks[fi]);
      }
      else
      {
      	MetricMaskPointer zero = NULL;
      	subMetric->SetFixedImageMask(zero);
      }
    }
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::InitializeDRREngineAndOutputs()
    throw (itk::ExceptionObject)
{
  unsigned int numMetrics = this->m_Metric->GetNumberOfMetricInputs();

  if (numMetrics <= 0)
    itkExceptionMacro(<< "No sub-metrics defined.")

  // set transform:
  if(!this->m_DRREngine)
  {
    itkExceptionMacro(<< "ERROR: " << this->GetNameOfClass()
        << "No DRR Engine set!")
  }
  this->m_DRREngine->SetTransform(this->GetTransform());


  for (unsigned int i = 0; i < numMetrics; i++)
  {
    BaseMetricPointer subMetric = this->m_Metric->GetIthMetricInput(i);
    if (!subMetric)
      itkExceptionMacro(<< "Invalid metric sub-component detected.")

    int fi = this->GetMappedFixedImageIndex(subMetric);
    if (fi >= 0)
    {
      // 3D -> 2D reduction (w.r.t. fixed image region which is adjusted in
      // UpdateFixedImageMultiResolutionConnections())
      this->m_DimensionReducers[fi]->SetInput(this->m_DRREngine->GetOutput(fi));
      this->m_InformationChangers[fi]->SetInput(
          this->m_DimensionReducers[fi]->GetOutput());
      subMetric->SetMovingImage(this->m_InformationChangers[fi]->GetOutput());
    }
    else
    {
      itkExceptionMacro(<< "Missing mapping between metric and fixed " <<
          "image (index: " << i << ").")
    }
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::InitializeTransformComponents(
    bool applyInitialTransformParameters) throw (itk::ExceptionObject)
{
  if (!this->m_Transform)
    itkExceptionMacro(<< "No transform set!")

  // connect the user-specified transform to the internal transforms in order
  // to establish pushing pipelines where a internal transform will
  // constantly output an 2D-identity-transform and the user-specified
  // transform will receive the set parameters:
  for (std::size_t i = 0; i < this->GetNumberOfFixedImages(); i++)
    this->m_InternalTransforms[i]->SetConnected3DTransform(this->m_Transform);
  // ensure that the number of initial parameters matches
  if (this->m_Transform->GetNumberOfParameters()
      != this->m_InitialTransformParameters.Size())
  {
    this->m_InitialTransformParameters.SetSize(
        this->m_Transform->GetNumberOfParameters());
    this->m_InitialTransformParameters.Fill(0);
  }

  if (applyInitialTransformParameters) // -> set init pars and synchronize

  {
    for (std::size_t i = 0; i < this->GetNumberOfFixedImages(); i++)
      this->m_InternalTransforms[i]->SetParameters(
          this->m_InitialTransformParameters);
  } // else: no synchronization
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::InitializeSubmetricComponents(
    bool callInitialize) throw (itk::ExceptionObject)
{
  unsigned int numMetrics = this->m_Metric->GetNumberOfMetricInputs();

  for (unsigned int i = 0; i < numMetrics; i++)
  {
    BaseMetricPointer subMetric = this->m_Metric->GetIthMetricInput(i);
    if (!subMetric)
      itkExceptionMacro(<< "Invalid metric sub-component detected.")

    // connect the sub-metrics with the internal identity transforms since the
    // fixed and moving 2D images are set up to perfectly align:
    subMetric->SetTransform(this->m_InternalTransforms[i]);

    // connect the 1:1 interpolator components:
    int fi = this->GetMappedFixedImageIndex(subMetric);
    if (fi >= 0)
    {
      subMetric->SetInterpolator(this->m_Interpolators[fi]);
    }
    else
    {
      itkExceptionMacro(<< "Missing mapping between metric and " <<
          "fixed image (index: " << i << ").")
    }
  }

  // more or less fake parameters for BeforeEvaluationEvent():
  this->m_Metric->SetCurrentParameters(this->m_Optimizer->GetInitialPosition());

  // force moving image update of ALL DRR-engine-outputs!
  this->m_Transform->SetParameters(this->m_Metric->GetCurrentParameters());
  for (unsigned int i = 0; i < numMetrics; i++)
    OnTransformsEvent(this->m_InternalTransforms[i], TransformChanged());

  if (callInitialize) // initialize each sub-metric on demand

  {
    for (unsigned int i = 0; i < numMetrics; i++)
    {
      BaseMetricPointer subMetric = this->m_Metric->GetIthMetricInput(i);
      subMetric->Initialize();
    }
  }

  // composite metric needs a transform as well; however, this transform won't
  // be really used, so just apply some 'dummy':
  IdentityTransformPointer identityTransform = IdentityTransformType::New();
  this->m_Metric->SetTransform(identityTransform);
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::InitializeOptimizer(
    bool setInitialPosition) throw (itk::ExceptionObject)
{
  if (!this->m_Optimizer)
    itkExceptionMacro(<< "No optimization component set.")
  this->m_Optimizer->SetCostFunction(this->m_Metric);
  if (setInitialPosition)
  {
    this->m_Optimizer->SetInitialPosition(this->m_InitialTransformParameters);
    // -> adjust transform parameters immediately to optimizers initial position
    // in order to avoid confusions until the first metric is evaluated:
    this->m_Transform->SetParameters(this->m_Optimizer->GetInitialPosition());
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::AddInternalObservers()
{
  this->RemoveInternalObservers(); // delete old ones

  // add metric evaluation event observers
  this->m_BeforeEvaluationObserverTag = this->m_Metric->AddObserver(
      ora::BeforeEvaluationEvent(), this->m_EvaluationCommand);
  this->m_AfterEvaluationObserverTag = this->m_Metric->AddObserver(
      ora::AfterEvaluationEvent(), this->m_EvaluationCommand);
  for (std::size_t i = 0; i < this->GetNumberOfFixedImages(); i++)
  {
    this->m_InternalTransforms[i]->AddObserver(ora::BeforeParametersSet(),
        this->m_TransformsCommand);
    this->m_InternalTransforms[i]->AddObserver(ora::TransformChanged(),
        this->m_TransformsCommand);
    this->m_InternalTransforms[i]->AddObserver(ora::AfterParametersSet(),
        this->m_TransformsCommand);
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::RemoveInternalObservers()
{
  if (this->m_BeforeEvaluationObserverTag != INVALID_OBSERVER_TAG)
  {
    this->m_Metric->RemoveObserver(this->m_BeforeEvaluationObserverTag);
    this->m_BeforeEvaluationObserverTag = INVALID_OBSERVER_TAG;
  }
  if (this->m_AfterEvaluationObserverTag != INVALID_OBSERVER_TAG)
  {
    this->m_Metric->RemoveObserver(this->m_AfterEvaluationObserverTag);
    this->m_AfterEvaluationObserverTag = INVALID_OBSERVER_TAG;
  }
  for (std::size_t i = 0; i < this->GetNumberOfFixedImages(); i++)
    this->m_InternalTransforms[i]->RemoveAllObservers();

}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
bool MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::Initialize() throw (itk::ExceptionObject)
{
  if (!this->m_Moving3DVolume)
    itkExceptionMacro(<< "No moving 3D volume set.")

  if (this->GetNumberOfFixedImages() <= 0)
    itkExceptionMacro(<< "No fixed images set.")

  if (this->m_Metric->GetNumberOfMetricInputs() <= 0)
    itkExceptionMacro(<< "Composite metric appears to have no inputs.")
  if (this->m_Metric->GetValueCompositeRule().length() <= 0)
    itkExceptionMacro(<< "Composite metric does not have a value rule.")

  if (!this->m_Transform)
    itkExceptionMacro(<< "No 3D/3D volume transformation component set.")

  if (!this->m_Optimizer)
    itkExceptionMacro(<< "No optimization component set.")

  // initialize the multi-resolution pyramids and connect them with sub-metrics:
  // moving 3D volume:
  this->ApplyMovingMultiResolutionSettings();
  this->UpdateMovingImageMultiResolutionConnections(this->m_CurrentLevel);
  // fixed 2D images:
  this->ApplyFixedMultiResolutionSettings();
  this->UpdateFixedImageMultiResolutionConnections(this->m_CurrentLevel);

  // initialize DRR engine and connect its outputs with the sub-metrics' inputs:
  this->InitializeDRREngineAndOutputs();

  // initialize and connect transforms:
  this->InitializeTransformComponents(true);

  // initialize optimizer-metric-connection:
  // - some optimizers (e.g. Amoeba) might require the cost function to return
  //   a valid number of transform parameters, so set a dummy transform with
  //   the right number of transform parameters:
  IdentityTransformPointer dummyTransform = IdentityTransformType::New();
  dummyTransform->SetNumberOfConnectedTransformParameters(
      this->m_InitialTransformParameters.GetSize());
  this->m_Metric->SetTransform(dummyTransform);
  this->InitializeOptimizer(true);

  // be sure that props are applied:
  ApplyAllProjectionPropsToDRREngine(0);

  // initialize submetrics and connect them with internal identity transforms:
  this->InitializeSubmetricComponents(true);

  // connect the transformation to the output (result):
  TransformOutputType
      *transformOutput =
          static_cast<TransformOutputType *> (this->itk::ProcessObject::GetOutput(
              0));
  transformOutput->Set(this->m_Transform.GetPointer());

  return true;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::StopRegistration()
{
  if (!this->m_Stop)
  {
    this->m_Stop = true;
    // the following event can be used to stop the optimizer as optimizers
    // implement the Stop()-method usually at the final classes (unfortunately):
    this->InvokeEvent(ora::StopRequestedEvent());
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::GenerateData()
    throw (itk::ExceptionObject)
{
  // StartEvent() is invoked by Superclass

  try
  {
    this->m_Stop = false; // set back stop-flag
    this->m_CurrentLevel = 0;

    this->AddInternalObservers(); // add observers

    this->Initialize(); // expected to throw an exception instead of returning 0

    for (this->m_CurrentLevel = 0; this->m_CurrentLevel
        < this->GetNumberOfLevels(); this->m_CurrentLevel++)
    {
      if (this->m_Stop) // stop requested
        break;

      if (this->m_CurrentLevel > 0) // because Initialize() also does it!
      {
        ApplyAllProjectionPropsToDRREngine(this->m_CurrentLevel);
      }

      // start new level event:
      this->InvokeEvent(ora::StartMultiResolutionLevelEvent());

      this->m_NumberOfMetricEvaluationsAtLevel = 0; // set back after event

      if (this->m_CurrentLevel > 0)
      {
        // moving 3D volume:
        this->ApplyMovingMultiResolutionSettings();
        this->UpdateMovingImageMultiResolutionConnections(this->m_CurrentLevel);
        // fixed 2D images:
        this->ApplyFixedMultiResolutionSettings();
        this->UpdateFixedImageMultiResolutionConnections(this->m_CurrentLevel);

        if (!this->m_Metric->GetReInitializeMetricsBeforeEvaluating())
        {
          // -> need to re-initialize sub-metric components in order to adapt
          // to the new fixed image region:
          this->InitializeSubmetricComponents(true);
        }
      }

      // start new optimization cycle event:
      this->InvokeEvent(ora::StartOptimizationEvent());

      this->m_Optimizer->StartOptimization(); // do the registration

      // final result (of level):
      this->m_Transform->SetParameters(this->m_Optimizer->GetCurrentPosition());
      // -> take over as initial position for next level:
      this->m_Optimizer->SetInitialPosition(
          this->m_Optimizer->GetCurrentPosition());
    }
  }
  catch (itk::ExceptionObject &initExc)
  {
    std::ostringstream os;
    if (this->GetNumberOfFixedImages() > 0)
      os << this->GetNumberOfFixedImages() << "-way";
    else
      os << "n-way";
    itkExceptionMacro(<< "Initialization of " << os.str() <<
        " 2D/3D-registration failed: " << initExc)
  }

  this->m_CurrentLevel = 0; // set back for next call to Initialize()
  this->RemoveInternalObservers(); // remove observers

  // EndEvent() is invoked by Superclass
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::SetNumberOfLevels(unsigned int levels)
{
  if (levels <= 0) // invalid
    return;

  this->m_FixedSchedule.SetSize(levels, 2);
  this->m_MovingSchedule.SetSize(levels, 3);

  unsigned int d, l;
  for (l = 0; l < levels; l++)
  {
    double sch = pow(2, levels - 1.0 - l);
    for (d = 0; d < 2; d++)
    {
      this->m_FixedSchedule[l][d] = static_cast<unsigned int> (sch);
      this->m_MovingSchedule[l][d] = static_cast<unsigned int> (sch);
    }
    this->m_MovingSchedule[l][2] = static_cast<unsigned int> (sch);
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
unsigned int MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::GetNumberOfLevels()
{
  return this->m_FixedSchedule.rows(); // = this->m_MovingSchedule.rows()
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
bool MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::SetSchedules(FixedScheduleType fschedule,
    MovingScheduleType mschedule)
{
  if (fschedule.rows() != mschedule.rows())
    return false;
  if (fschedule.cols() != 2 || mschedule.cols() != 3)
    return false;

  this->m_FixedSchedule = fschedule; // copy
  this->m_MovingSchedule = mschedule;

  return true;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
typename MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::FixedScheduleType MultiResolutionNWay2D3DRegistrationFramework<
    TFixed2DImage, TMoving2DImage, TMoving3DImage>::GetFixedSchedule()
{
  return this->m_FixedSchedule;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
typename MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::MovingScheduleType MultiResolutionNWay2D3DRegistrationFramework<
    TFixed2DImage, TMoving2DImage, TMoving3DImage>::GetMovingSchedule()
{
  return this->m_MovingSchedule;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
bool MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::AddMetricFixedImageMapping(
    BaseMetricPointer metric, unsigned int fixedImage)
{
  if (fixedImage >= this->GetNumberOfFixedImages() || !metric)
    return false;

  // search for mapping already related to the specified metric
  if (GetMappedFixedImageIndex(metric) != -1)
  {
    this->m_MetricFixedImageMappings[metric] = fixedImage;
    return true;
  }

  this->m_MetricFixedImageMappings.insert(MetricFixedImagePairType(metric,
      fixedImage));

  return true;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
int MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::GetMappedFixedImageIndex(
    BaseMetricPointer metric)
{
  if (!metric)
    return -1;

  typename MetricFixedImageMapType::iterator it =
      this->m_MetricFixedImageMappings.find(metric);
  int ret = -1;
  if (it != this->m_MetricFixedImageMappings.end())
    ret = this->m_MetricFixedImageMappings[metric];

  return ret;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
std::vector<typename MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::BaseMetricPointer> MultiResolutionNWay2D3DRegistrationFramework<
    TFixed2DImage, TMoving2DImage, TMoving3DImage>::GetMappedMetrics(
    unsigned int fixedImage)
{
  std::vector<BaseMetricPointer> result;

  typename MetricFixedImageMapType::iterator it;
  for (it = this->m_MetricFixedImageMappings.begin(); it
      != this->m_MetricFixedImageMappings.end(); ++it)
  {
    if ((*it).second == fixedImage)
    {
      result.push_back((*it).first);
    }
  }

  return result;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
bool MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::RemoveMetricFixedImageMapping(
    BaseMetricPointer metric)
{
  typename MetricFixedImageMapType::iterator it;
  for (it = this->m_MetricFixedImageMappings.begin(); it
      != this->m_MetricFixedImageMappings.end(); ++it)
  {
    if ((*it).first == metric)
    {
      this->m_MetricFixedImageMappings.erase(it);
      return true;
    }
  }

  return false;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
unsigned int MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::RemoveMetricFixedImageMapping(
    unsigned int fixedImage)
{
  unsigned int numdel = 0;

  typename MetricFixedImageMapType::iterator it;
  bool deleted = true;
  while (deleted)
  {
    deleted = false;
    for (it = this->m_MetricFixedImageMappings.begin(); it
        != this->m_MetricFixedImageMappings.end(); ++it)
    {
      if ((*it).second == fixedImage)
      {
        this->m_MetricFixedImageMappings.erase(it);
        deleted = true;
        numdel++;
        break;
      }
    }
  }

  return numdel;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::RemoveAllMetricFixedImageMappings()
{
  this->m_MetricFixedImageMappings.clear();
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::ApplyAllProjectionPropsToDRREngine(int level)
{
  for(unsigned int i = 0; i < this->GetNumberOfFixedImages(); i++)
		this->ApplyProjectionPropsToDRREngine(this->GetIthProjectionProps(i), level, i);
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::ApplyProjectionPropsToDRREngine(ProjectionPointer geom,
        int level, int fixedImageIndex)
{
  if (level >= (int) this->GetNumberOfLevels() || (level >= 0
      && (fixedImageIndex < 0 || fixedImageIndex
          >= (int) this->GetNumberOfFixedImages())))
    return;

  // auto-adjust props to multi-resolution level
  if (this->m_UseAutoProjectionPropsAdjustment)
  {
    // assume that 'default projection' directly corresponds to the fixed
    // image region at the finest level!

    // new projection to set after calculation
    ProjectionPointer newProjection = ProjectionType::New();

    // -> take over fixed image's spacing:
    double spacing[2];
    spacing[0] = this->m_FixedPyramids[fixedImageIndex]->
    		GetOutput(level)->GetSpacing()[0];
    spacing[1] = this->m_FixedPyramids[fixedImageIndex]->
    		GetOutput(level)->GetSpacing()[1];
    newProjection->SetDetectorPixelSpacing(spacing);

    // -> take over fixed image region's size:
    FixedImageRegionPyramidType rpyr =
        this->m_FixedRegionPyramids[fixedImageIndex];
    FixedImageRegionType rreg = rpyr[level];
    int dsz[2];
    dsz[0] = rreg.GetSize()[0];
    dsz[1] = rreg.GetSize()[1];
    newProjection->SetDetectorSize(dsz);

    // -> compute image offset according to pyramid image filter's method:
    // mass of objects remains at the same physical position ...
    const double *row;
    row = geom->GetDetectorRowOrientation();
    const double *column;
    column = geom->GetDetectorColumnOrientation();
    newProjection->SetDetectorOrientation(row, column);
    double normal[3];
    normal[0] = (row[1] * column[2]) - (row[2] * column[1]);
    normal[1] = (row[2] * column[0]) - (row[0] * column[2]);
    normal[2] = (row[0] * column[1]) - (row[1] * column[0]);
    // mass of object remains at the same physical position:
    typename Moving3DImageType::SpacingType ospac;
    ospac[0] = newProjection->GetDetectorPixelSpacing()[0];
    ospac[1] = newProjection->GetDetectorPixelSpacing()[1];
    ospac[2] = 1; // ignore
    typename Moving3DImageType::SpacingType ispac;
    ispac[0] = geom->GetDetectorPixelSpacing()[0];
    ispac[1] = geom->GetDetectorPixelSpacing()[1];
    ispac[2] = 1; // ignore
    typedef itk::Matrix<double, 3, 3> OrientationType;
    OrientationType dir;
    // -> convert to column-based representation for computation
    for (int i = 0; i < 3; i++)
    {
      dir[0][i] = row[i];
      dir[1][i] = column[i];
      dir[2][i] = normal[i];
    }
    itk::Vector<double, 3> offset = (dir * (ospac - ispac)) / 2.;
    double oorigin[3];
    for (int d = 0; d < 3; d++)
      oorigin[d] = geom->GetDetectorOrigin()[d] + offset[d];
    newProjection->SetDetectorOrigin(oorigin);
    newProjection->SetSourcePosition(geom->GetSourcePosition());
    this->m_DRREngine->SetProjectionGeometry(fixedImageIndex, newProjection);
  }
  else
  {
    this->m_DRREngine->SetProjectionGeometry(fixedImageIndex, geom);
  }

  // also apply mask to DRR-engine if configured:
  if (this->m_Masks[fixedImageIndex])
  {
		// NOTE: Once again, the physical mask props (origin, orientation) are
		// completely regardless - only the pixels matter!
		if (level != (int)(this->GetNumberOfLevels() - 1)) // MR pyramid required
		{
			this->m_MaskCasters[fixedImageIndex]->SetInput(
					this->m_MaskPyramids[fixedImageIndex]->GetOutput(level));
			this->m_MaskCasters[fixedImageIndex]->UpdateLargestPossibleRegion();
			this->m_DRREngine->SetDRRMask(fixedImageIndex,
					this->m_MaskCasters[fixedImageIndex]->GetOutput());
		}
		else // last level -> original resolution -> original mask
		{
			this->m_MaskCasters[fixedImageIndex]->SetInput(
					this->m_Masks[fixedImageIndex]);
			this->m_MaskCasters[fixedImageIndex]->UpdateLargestPossibleRegion();
			this->m_DRREngine->SetDRRMask(fixedImageIndex,
					this->m_MaskCasters[fixedImageIndex]->GetOutput());
		}
  }
  else
  {
  	this->m_DRREngine->SetDRRMask(fixedImageIndex, NULL);
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
typename MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::DRR3DImagePointer MultiResolutionNWay2D3DRegistrationFramework<
    TFixed2DImage, TMoving2DImage, TMoving3DImage>::Compute3DTestProjection(
    unsigned int propsIndex, unsigned int level) throw (itk::ExceptionObject)
{
  if (!this->m_Moving3DVolume || level >= this->GetNumberOfLevels()
      || propsIndex >= this->GetNumberOfFixedImages())
    return NULL;

  this->UpdateMovingImageMultiResolutionConnections(level);

  this->m_DRREngine->SetTransform(this->GetTransform());
  this->ApplyProjectionPropsToDRREngine(this->GetIthProjectionProps(propsIndex), level, propsIndex);
  DRR3DImagePointer output;
  try
  {
    this->m_DRREngine->SetCurrentDRROutputIndex(propsIndex);
    this->m_DRREngine->Update();
    output = this->m_DRREngine->GetOutput(propsIndex);
  }
  catch (...) // error
  {
    // reset values
    return NULL;
  }

  return output;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
typename MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage,
    TMoving2DImage, TMoving3DImage>::Moving2DImagePointer MultiResolutionNWay2D3DRegistrationFramework<
    TFixed2DImage, TMoving2DImage, TMoving3DImage>::Compute2DTestProjection(
    unsigned int propsIndex, unsigned int level) throw (itk::ExceptionObject)
{
  DRR3DImagePointer drr = this->Compute3DTestProjection(propsIndex, level);

  if (drr)
  {
    Region3DType reg = drr->GetLargestPossibleRegion();
    ExtractorPointer extractor = ExtractorType::New();
    Region3DType eregion;
    Index3DType eidx;
    eidx[0] = 0; // 0-indexed DRR outputs
    eidx[1] = 0;
    eidx[2] = 0; // first (and only) slice
    eregion.SetIndex(eidx);
    Size3DType esz;
    esz[0] = reg.GetSize()[0];
    esz[1] = reg.GetSize()[1];
    esz[2] = 0; // no 3rd dimension anymore
    eregion.SetSize(esz);
    extractor->SetExtractionRegion(eregion); // preliminary extract region
    extractor->SetInput(drr);
    extractor->Update();

    return extractor->GetOutput();
  }

  return NULL;
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::OnMetricEvaluationEvent(
    const itk::EventObject &eventObject)
{
  if (std::string(eventObject.GetEventName()) == "BeforeEvaluationEvent")
  {
    this->m_NumberOfMetricEvaluationsAtLevel++;
    // update transform with parameters dictated by optimizer and applied
    // as argument to composite metric:
    this->m_Transform->SetParameters(this->m_Metric->GetCurrentParameters());
    this->m_LastMetricParameters = this->m_Transform->GetParameters();

    // NOTE: No further action here (re-engineering in version 1.7).
  }
  else if (std::string(eventObject.GetEventName()) == "AfterEvaluationEvent")
  {
    this->m_LastMetricValue = this->m_Metric->GetLastValue();
    this->m_LastMetricDerivative = this->m_Metric->GetLastDerivative();
  }
}

template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
void MultiResolutionNWay2D3DRegistrationFramework<TFixed2DImage, TMoving2DImage,
    TMoving3DImage>::OnTransformsEvent(itk::Object *obj,
    const itk::EventObject &eventObject)
{
  if (std::string(eventObject.GetEventName()) == "BeforeParametersSet")
  {
    // concurrent connected 3D transform access protection: ENTER
    this->m_InternalTransformsMutex.Lock();
  }
  else if (std::string(eventObject.GetEventName()) == "TransformChanged")
  {
    // transformation has *really* changed -> update respective DRR output:
    // - search for the causative component:
    int idx = -1;
    IdentityTransformPointer transform = itk::SmartPointer<
        IdentityTransformType>(reinterpret_cast<IdentityTransformType *> (obj));
    for (std::size_t i = 0; i < this->GetNumberOfFixedImages(); i++)
    {
      if (this->m_InternalTransforms[i] == transform)
      {
        idx = static_cast<int> (i);
        break;
      }
    }
    // - update the according DRR-output and related filters:
    if (idx > -1)
    {
      // update i-th output
      this->m_DRREngine->SetCurrentDRROutputIndex(idx);
      this->m_DRREngine->Update();
      this->m_DimensionReducers[idx]->UpdateLargestPossibleRegion();
      this->m_InformationChangers[idx]->UpdateLargestPossibleRegion();
      BaseMetricPointer subMetric = this->m_Metric->GetIthMetricInput(idx);
      if (subMetric && subMetric->GetMovingImage())
        subMetric->GetMovingImage()->GetSource()->Update();
    }
  }
  else if (std::string(eventObject.GetEventName()) == "AfterParametersSet")
  {
    // concurrent connected 3D transform access protection: LEAVE
    this->m_InternalTransformsMutex.Unlock();
  }
}
}

#endif /* ORAMultiResolutionNWay2D3DRegistrationFramework_TXX_ */
