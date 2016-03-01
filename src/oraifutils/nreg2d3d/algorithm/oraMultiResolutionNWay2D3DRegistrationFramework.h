//
#ifndef ORAMULTIRESOLUTIONNWAY2D3DREGISTRATIONFRAMEWORK_H_
#define ORAMULTIRESOLUTIONNWAY2D3DREGISTRATIONFRAMEWORK_H_
//ITK
#include <itkObjectFactory.h>
#include <itkConceptChecking.h>
#include <itkProcessObject.h>
#include <itkImageToImageMetric.h>
#include <itkTransform.h>
#include <itkSingleValuedNonLinearOptimizer.h>
#include <itkDataObjectDecorator.h>
#include <itkMultiResolutionPyramidImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkCommand.h>
#include <itkEventObject.h>
#include <itkImageMaskSpatialObject.h>
#include <itkSimpleFastMutexLock.h>
//internal
#include "oraOneToOneInterpolateImageFunction.h"
#include "oraProjectionGeometry.h"
#include "oraDRRFilter.h"
#include "oraCompositeImageToImageMetric.h"
#include "oraParametrizableIdentityTransform.h"
#include "oraMultiResolutionPyramidMaskFilter.h"

#include <vector>
#include <map>

namespace ora
{

/** Custom events. **/
itkEventMacro(StartMultiResolutionLevelEvent, itk::AnyEvent)
itkEventMacro(StartOptimizationEvent, itk::AnyEvent)
itkEventMacro(StopRequestedEvent, itk::AnyEvent)

/** \class MultiResolutionNWay2D3DRegistrationMethod
 * \brief N-way multi-resolution 2D/3D registration framework.
 *
 * An N-way multi-resolution 2D/3D registration framework for flexible
 * intensity-based 2D/3D image registration using the DRR Filter interface
 *
 * Using this framework a flexible 2D/3D registration scenario may be set up.
 * One can involve one or more (!) fixed reference images with a specified
 * projection geometry. A specified volume (moving 3D volume) is then aligned
 * w.r.t. the reference images and the respective projection geometries by
 * optimizing the fitness of the internally generated projection images and the
 * reference images. The user can specify and configure the metric and optimizer
 * components as usual. The values of the internal metrics are
 * composed by a central composite metric component which can be configured
 * appropriately. Moreover multi-resolution strategies can be configured
 * where the fixed images and the moving 3D volume can have independent pyramid
 * schedules. In addition this class provides support for fixed image regions
 * (rectangular regions of interest, rROIs) in order to restrict alignment to
 * specified sub-regions of the reference images. Furthermore, this class
 * supports (optional) DRR masks that define which DRR pixels should be
 * computed.
 *
 * The class provides several events which can be tracked by command patterns:
 * <br>
 * - StartEvent() ... fired whenever a 2D/3D-registration starts <br>
 * - EndEvent() ... fired whenever a 2D/3D-registration terminates <br>
 * - StartMultiResolutionLevelEvent() ... fired whenever a new multi-resolution
 * level is started <br>
 * - StartOptimizationEvent() ... fired right before the optimization process
 * of a multi-resolution level is started <br>
 *
 * Before a registration is used maked sure to set a DRREngine that implements
 * the DRR Engine interface. The DRREngine cannot be changed once it is set.
 *
 * For more information regarding the usage and configuration of this class and
 * the necessary components, please refer to the listed tests below.
 *
 * <b>WARNING</b> This class does NOT SUPPORT optimizers that use the metric's
 * DERIVATIVE explicitly. This is due to internal restrictions and a broad
 * variety of the metrics' internal realization of derivative estimation.
 * Usually it is anyway more efficient to use optimization strategies that
 * estimate the cost function gradient based on a set of composite values (e.g.
 * SPSA).
 *
 * <b>WARNING</b> At the moment this framework does not support optimized
 * (parallelized) composite value computation as the internal pipeline has not
 * been adapted and tested for this purpose. There are some problems with the
 * DRR-metric-connections and the common transform component.
 *
 * This class is templated over the fixed 2D image type (which is internally
 * used for similarity measurement), the moving 2D image type (which is the
 * output of the DRR engine for similarity measurement), the moving 3D image
 * type (which is the input volume type), and the mask pixel type (which is the
 * pixel type of DRR masks - must be SIGNED!).
 *
 * <b>Tests</b>:<br>
 * TestMultiResolutionNWay2D3DRegistrationFramework.cxx</br>
 *
 * @author jeanluc
 * @version 1.0
 *
 * \ingroup RegistrationAlgorithms
 */
template<class TFixed2DImage, class TMoving2DImage, class TMoving3DImage>
class MultiResolutionNWay2D3DRegistrationFramework:
    public itk::ProcessObject
{
public:
  /** Standard class typedefs. **/
  typedef MultiResolutionNWay2D3DRegistrationFramework Self;
  typedef itk::ProcessObject Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. **/
  itkNewMacro(Self)

  /** Run-time type information (and related methods). **/
  itkTypeMacro(MultiResolutionNWay2D3DRegistrationFramework, itk::ProcessObject)

  /** Type of the Fixed image. **/
  typedef TFixed2DImage FixedImageType;
  typedef typename FixedImageType::PixelType FixedPixelType;
  typedef typename FixedImageType::Pointer FixedImagePointer;
  typedef typename FixedImageType::RegionType FixedImageRegionType;

  /** Type of projection geometry. **/
  typedef ProjectionGeometry ProjectionType;
  typedef ProjectionType::Pointer ProjectionPointer;

  /** Type of the Moving image (2D), DRR image. **/
  typedef TMoving2DImage Moving2DImageType;
  typedef typename Moving2DImageType::PixelType Moving2DPixelType;
  typedef typename Moving2DImageType::Pointer Moving2DImagePointer;

  /** Type of the Moving image (3D), volume. **/
  typedef TMoving3DImage Moving3DImageType;
  typedef typename Moving3DImageType::PixelType Moving3DPixelType;
  typedef typename Moving3DImageType::Pointer Moving3DImagePointer;
  typedef float SignedVolumePixelType;
  typedef itk::Image<SignedVolumePixelType, 3> SignedVolumeImageType;

  /**  Types of the metrics. */
  typedef ora::CompositeImageToImageMetric<FixedImageType, Moving2DImageType>
      MetricType;
  typedef typename MetricType::Pointer MetricPointer;
  typedef typename MetricType::Superclass BaseMetricType;
  typedef BaseMetricType* BaseMetricPointer;
  typedef typename MetricType::MeasureType MetricMeasureType;
  typedef typename MetricType::DerivativeType MetricDerivativeType;

  /** Type of 3D transform that transforms the volume. **/
  typedef itk::MatrixOffsetTransformBase<double, 3, 3> TransformType;
  typedef TransformType::Pointer TransformPointer;
  typedef TransformType::ParametersType ParametersType;

  /** Type of filter output (decorated transformation) **/
  typedef itk::DataObjectDecorator<TransformType> TransformOutputType;
  typedef typename TransformOutputType::Pointer TransformOutputPointer;
  typedef typename TransformOutputType::ConstPointer
      TransformOutputConstPointer;

  /** Type of the optimizer. **/
  typedef itk::SingleValuedNonLinearOptimizer OptimizerType;
  typedef OptimizerType::Pointer OptimizerPointer;

  /** Type of DRR engine. **/
  typedef DRRFilter<Moving3DPixelType, Moving2DPixelType> DRREngineType;
  typedef typename DRREngineType::Pointer DRREnginePointer;
  typedef typename DRREngineType::OutputImageType DRR3DImageType;
  typedef typename DRR3DImageType::Pointer DRR3DImagePointer;

  /** Mask type. **/
  typedef typename DRREngineType::MaskPixelType MaskPixelType;
  typedef itk::Image<MaskPixelType, 2> MaskImageType;
  typedef typename MaskImageType::Pointer MaskImagePointer;

  /** Types of the multi-resolution pyramid filters **/
  typedef itk::MultiResolutionPyramidImageFilter<FixedImageType, FixedImageType>
      FixedPyramidType;
  typedef typename FixedPyramidType::Pointer FixedPyramidPointer;
  typedef typename FixedPyramidType::ScheduleType FixedScheduleType;
  typedef itk::MultiResolutionPyramidImageFilter<SignedVolumeImageType,
      SignedVolumeImageType> MovingPyramidType;
  typedef typename MovingPyramidType::Pointer MovingPyramidPointer;
  typedef typename MovingPyramidType::ScheduleType MovingScheduleType;
  typedef ora::MultiResolutionPyramidMaskFilter<MaskImageType> MaskPyramidType;
  typedef typename MaskPyramidType::Pointer MaskPyramidPointer;
  typedef typename MaskPyramidType::ScheduleType MaskScheduleType;

  /** ImageDimension constants */
  itkStaticConstMacro(FixedImageDimension, unsigned int,
      FixedImageType::ImageDimension);
  itkStaticConstMacro(Moving2DImageDimension, unsigned int,
      Moving2DImageType::ImageDimension);
  itkStaticConstMacro(Moving3DImageDimension, unsigned int,
      Moving3DImageType::ImageDimension);

  /** Concepts (image-type-related) **/
  itkConceptMacro(FixedImageHasNumericTraitsCheck,
      (itk::Concept::HasNumericTraits<FixedPixelType>));
    itkConceptMacro(FixedImageHasRightDimensionCheck,
        (itk::Concept::SameDimension<itkGetStaticConstMacro(FixedImageDimension),
            2>));
  itkConceptMacro(Moving2DImageHasNumericTraitsCheck,
      (itk::Concept::HasNumericTraits<Moving2DPixelType>));
    itkConceptMacro(Moving2DImageHasRightDimensionCheck,
        (itk::Concept::SameDimension<itkGetStaticConstMacro(Moving2DImageDimension),
            2>));
  itkConceptMacro(Moving3DImageHasNumericTraitsCheck,
      (itk::Concept::HasNumericTraits<Moving3DPixelType>));
    itkConceptMacro(Moving3DImageHasRightDimensionCheck,
        (itk::Concept::SameDimension<itkGetStaticConstMacro(Moving3DImageDimension),
            3>));

  itkGetObjectMacro(Moving3DVolume, Moving3DImageType)
  virtual void SetMoving3DVolume(Moving3DImagePointer image);

  /**
   * Get number of fixed images (the 'n' of n-way). NOTE: the number of
   * projection properties equals the number of fixed images!
   **/
  std::size_t GetNumberOfFixedImages() const;
  /**
   * Add a fixed image to the registration scenario with associated projection
   * properties (geometry, ray-casting properties). The specified fixed image
   * region is set to the metric components; for maximum efficiency projProps
   * should also consider the fixed image region in order to compute DRRs which
   * are as small as possible.
   * @param fixedImage fixed (reference) image in 2D representation
   * @param fixedImageRegion fixed image region which determines the part of the
   * image that is expected being considered during registration (rectangular
   * region of interest)
   * @param projProps projection properties which define the projection
   * geometry and ray-casting properties for the associated moving DRRs during
   * registration;
   * @param maskImage optional mask image which is required to have the same
   * size as the fixed input image; values !=0 are considered; in order to fit
   * the multi-resolution approach, this mask is internally adapted according
   * to the fixed image schedule!
   * <br>NOTE: The properties should be fully defined!
   * <br>NOTE: This method is NOT protected against adding a fixed 2D image
   * multiple times as the image is internally cropped to fit specified region!
   * <br>NOTE: If the fixed image region is different from the largest possible
   * image region, the fixed image will internally be cropped adequately and
   * set as cropped 2D metric input. The same is true for the mask. Be aware of
   * that!
   * @return TRUE if the fixed image and properties could successfully be
   * added to the registration scenario
   * @see ora::ProjectionGeometry
   **/
  virtual bool AddFixedImageAndProps(FixedImagePointer fixedImage,
      FixedImageRegionType fixedImageRegion, ProjectionPointer projGeom,
      MaskImagePointer maskImage = NULL);
  /**
   * Overloaded version of AddFixedImageAndProps() which requires the fixed
   * image (and optionally the mask) to be represented in 3D. The fixed image
   * (as well as the mask) is internally casted to 2D.
   * <br>NOTE: the third component of the fixed image region is expected to
   * have a start-index of zero and a size of one and is expected to lie within
   * the image's largest possible region.
   * <br>NOTE: Temporarily the 3D fixed/mask image's direction must be set to
   * identity in order to enable casting; this is undone at the end of the
   * procedure.
   * <br>NOTE: Furthermore the pipeline between the internal 2D and the original
   * 3D fixed image is disconnected!
   * @see AddFixedImageAndProps(FixedImagePointer, FixedImageRegionType, ProjectionPropsPointer) **/
  virtual bool AddFixedImageAndProps(
      typename DRREngineType::OutputImagePointer fixedImage,
      typename DRREngineType::OutputImageType::RegionType fixedImageRegion,
      ProjectionPointer projGeom,
      typename DRREngineType::MaskImagePointer maskImage = NULL);

  /** Set the DRREngine used to generate the DRRs. The engine can only be set
   * once.  The engine needs to implement the ora::DRRFilter interface. Not
   * setting the DRREngine will lead to a crash, no default is given. Moreover,
   * note that the intensity transfer function (ITF) must be set directly
   * on the drrEngine, and is not managed by this framework! **/
  virtual void SetDRREngine(DRREnginePointer drrEngine);

  /** Get i-th (out of n) fixed image. NULL if not available. **/
  FixedImagePointer GetIthFixedImage(std::size_t i);
  /** Get i-th (out of n) projection props. NULL if not available. **/
  ProjectionPointer GetIthProjectionProps(std::size_t i);
  /** Get i-th (out of n) fixed image region. zero-size region at failure. **/
  FixedImageRegionType GetIthFixedImageRegion(std::size_t i);
  /** Get i-th (out of n) DRR mask. NULL if not available. **/
  MaskImagePointer GetIthDRRMask(std::size_t i);
  /**
   * Remove the i-th (out of n) fixed image and its associated props.
   * @return TRUE if successfully removed
   **/
  bool RemoveIthFixedImageAndProps(std::size_t i);
  /** Remove all n fixed images and their associated props. **/
  void RemoveAllFixedImagesAndProps();

  /** Access the internal GLSL-based DRR engine. **/
  itkGetObjectMacro(DRREngine, DRREngineType)
  /**
   * Access the internal composite metric component.
   * <br>NOTE: this metric and all sub-components will internally be connected
   * to a common 1:1 interpolator and a parametrizable identity transform before
   * registration starts! So do not care about these props.
   **/
  itkGetObjectMacro(Metric, MetricType)

  /**
   * Add a mapping between a metric-component which is an input of the central
   * composite metric and a specified fixed image by its index. This is
   * required in order to inform the registration method which pyramid filter
   * outputs and input metric components need to be connected in a multi-
   * resolution scenario. <br> NOTE: one metric sub-component can be connected
   * to exactly one fixed image, but one fixed image can be mapped to various
   * sub-metric components!
   * @param metric the metric sub-component which needs to be connected to the
   * specified fixed image
   * @param fixedImage the index of the fixed image which needs to be
   * connected to the specified metric sub-component (the index must exist and
   * is certainly zero-based)
   * @return TRUE if the mapping could be successfully established
   */
  virtual bool AddMetricFixedImageMapping(BaseMetricPointer metric,
      unsigned int fixedImage);
  /**
   * @return the index of the fixed image which is actually mapped to the
   * specified metric sub-component; -1 if there is no valid mapping
   */
  int GetMappedFixedImageIndex(BaseMetricPointer metric);
  /**
   * @return the vector of metric sub-components that are actually mapped to the
   * specified fixed image; as one fixed image can be mapped to different metric
   * sub-components the returned vector may contain more than one metric
   * pointers
   */
  std::vector<BaseMetricPointer> GetMappedMetrics(unsigned int fixedImage);
  /**
   * Remove the mapping between the specified metric and its fixed image if
   * there is a mapping.
   * @param metric the metric sub-component
   * @return TRUE if a mapping was removed
   */
  bool RemoveMetricFixedImageMapping(BaseMetricPointer metric);
  /**
   * Remove the mapping(s) between the specified fixed image and its connected
   * metric sub-components.
   * @param fixedImage the index of the considered fixed image
   * @return the number of removed mappings
   */
  unsigned int RemoveMetricFixedImageMapping(unsigned int fixedImage);
  /**
   * Remove all actual mappings between metric sub-components and fixed images.
   */
  void RemoveAllMetricFixedImageMappings();

  itkGetObjectMacro(Transform, TransformType)
  itkSetObjectMacro(Transform, TransformType)

  /** @return the transform resulting from the registration process **/
  const TransformOutputType *GetOutput() const;

  /**
   * Initialize the registration. This method sets up internal sub-components as
   * well as user-specified components and ties them together for subsequent
   * registration.
   * @return TRUE if the initialization was successful (all requested components
   * found and successfully tied together)
   **/
  virtual bool Initialize() throw (itk::ExceptionObject);

  /**
   * Set the number of multi-resolution levels. This implies that the number
   * resolution changes by factor 2 at each level keeping the original
   * resolution at the final level. This is applied to both the 2D fixed image
   * and the 3D moving volume. For example levels=3 results in the following
   * fixed image schedule: <br>
   * <code>
   * 4 4 <br>
   * 2 2 <br>
   * 1 1</code>
   * @param levels the number of levels to be applied (> 0)
   * @see m_FixedSchedule
   * @see m_MovingSchedule
   */
  virtual void SetNumberOfLevels(unsigned int levels);
  /** @return the number of configured multi-resolution levels **/
  virtual unsigned int GetNumberOfLevels();
  /**
   * Set the schedules for both the fixed and the moving image pyramids. The
   * number of levels (rows) in both schedules must match!
   * @param fschedule 2D fixed image pyramid schedule (is also used as mask
   * image schedule!)
   * @param mschedule 3D moving volume pyramid schedule
   * @param maskSchedule mask pyramid schedule
   * @return TRUE if the schedules could be taken over
   * @see m_FixedSchedule
   * @see m_MovingSchedule
   */
  virtual bool SetSchedules(FixedScheduleType fschedule,
      MovingScheduleType mschedule);
  /**
   * @return a COPY of the actual 2D fixed image schedule (thus manipulating
   * the returned schedule will not show any effect during registration)
   */
  virtual FixedScheduleType GetFixedSchedule();
  /**
   * @return a COPY of the actual 3D moving volume schedule (thus manipulating
   * the returned schedule will not show any effect during registration)
   */
  virtual MovingScheduleType GetMovingSchedule();
  /**
   * Set flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for levels where all schedule factors are equal to
   * 1.0. NOTE: the pyramid filter additionally applies gaussian smoothing which
   * may be undesired for 'original'-resolution images in some registration
   * scenarios.
   */
  itkSetMacro(UseFixedPyramidForUnshrinkedLevels, bool)
  /**
   * Get flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for levels where all schedule factors are equal to
   * 1.0.
   */
  itkGetMacro(UseFixedPyramidForUnshrinkedLevels, bool)
  /**
   * Set flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for the final (finest) level. If set to FALSE the last
   * schedule factors will be ignored and the original image is passed (without
   * any kind of smoothing).
   */
  itkSetMacro(UseFixedPyramidForFinalLevel, bool)
  /**
   * Get flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for the final (finest) level.
   */
  itkGetMacro(UseFixedPyramidForFinalLevel, bool)

  /**
   * Set flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for levels where all schedule factors are equal to
   * 1.0. NOTE: the pyramid filter additionally applies gaussian smoothing which
   * may be undesired for 'original'-resolution images in some registration
   * scenarios.
   */
  itkSetMacro(UseMovingPyramidForUnshrinkedLevels, bool)
  /**
   * Get flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for levels where all schedule factors are equal to
   * 1.0.
   */
  itkGetMacro(UseMovingPyramidForUnshrinkedLevels, bool)
  /**
   * Set flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for the final (finest) level. If set to FALSE the last
   * schedule factors will be ignored and the original image is passed (without
   * any kind of smoothing).
   */
  itkSetMacro(UseMovingPyramidForFinalLevel, bool)
  /**
   * Get flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for the final (finest) level.
   */
  itkGetMacro(UseMovingPyramidForFinalLevel, bool)

  /** Get the current multi-resolution level number (0-based). **/
  itkGetMacro(CurrentLevel, unsigned int)

  /**
   * Compute a test projection of the current volume in 3D representation w.r.t.
   * to a specific property set which corresponds to a specified fixed image
   * (projection geometry). Furthermore the computation refers to a specific
   * multi-resolution level. Computing a test projection may be useful for
   * initializing a metric (e.g. scalar range) before registration starts or
   * during level changes.
   * NOTE: Registration must be initialized before this method is called!
   * Otherwise the multi-resolution settings do not work!
   * NOTE: After calling this method the projectionIndex and level is NOT reset.
   * @param propsIndex index of the projection property set
   * @param level level number (zero-based) which must match one of the pre-
   * configured levels
   * @return the computed 3D test projection if successful, NULL otherwise
   * @see GetNumberOfFixedImages()
   * @see GetIthProjectionProps()
   * @see GetNumberOfLevels()
   * @see Initialize()
   */
  DRR3DImagePointer Compute3DTestProjection(unsigned int projectionIndex,
      unsigned int level) throw (itk::ExceptionObject);
  /**
   * Compute a test projection of the current volume in 2D representation w.r.t.
   * to a specific property set which corresponds to a specified fixed image
   * (projection geometry). Furthermore the computation refers to a specific
   * multi-resolution level. Computing a test projection may be useful for
   * initializing a metric (e.g. scalar range) before registration starts or
   * during level changes. NOTE: using this method pose-information of the
   * image in 3D space is not reset to the previous settings.
   * NOTE: Registration must be initialized before this method is called!
   * Otherwise the multi-resolution settings do not work!
   * @param propsIndex index of the projection property set
   * @param level level number (zero-based) which must match one of the pre-
   * configured levels
   * @return the computed 2D test projection if successful, NULL otherwise
   * @see GetNumberOfFixedImages()
   * @see GetIthProjectionProps()
   * @see GetNumberOfLevels()
   * @see Initialize()
   */
  Moving2DImagePointer Compute2DTestProjection(unsigned int projectionIndex,
      unsigned int level) throw (itk::ExceptionObject);

  /**
   * Initialize the DRR engine by setting the number of outputs and connecting
   * them to the configured sub-metrics.
   * NOTE: Whenever the number or constellation of sub-metrics is changed during
   * an already initialized registration, this routine must be called in order
   * to adjust the internal pipelines.
   */
  virtual void InitializeDRREngineAndOutputs() throw (itk::ExceptionObject);

  /**
   * Initialize and connect the internal transformation components.
   * NOTE: Whenever the user-specified transformation component changes during
   * an already initialized registration, this routine must be called in order
   * to adjust the internal pipelines.
   * @param applyInitialTransformParameters if TRUE the specified initial
   * transform parameters are applied to the transformation component
   */
  virtual void InitializeTransformComponents(
      bool applyInitialTransformParameters) throw (itk::ExceptionObject);

  /**
   * Set initial transform parameters for registration initialization.
   * NOTE: The number of initial parameters must match the number of parameters
   * required by set 3D transform. Otherwise a zero vector will be used for
   * initialization.
   **/
  itkSetMacro(InitialTransformParameters, ParametersType)
  /** Get initial transform parameters used for registration initialization. **/
  itkGetMacro(InitialTransformParameters, ParametersType)

  /**
   * Initialize sub-metric components. Connect them with the internal identity
   * transform and 1:1 interpolators.
   * NOTE: Whenever the number or constellation of sub-metrics is changed during
   * an already initialized registration, this routine must be called in order
   * to adjust the internal pipelines.
   * @param callInitialize if TRUE the Initialize()-method of each sub-metric
   * is called
   */
  virtual void InitializeSubmetricComponents(bool callInitialize)
      throw (itk::ExceptionObject);

  /**
   * Set user-specified optimizer component which aims at optimizing the
   * implicitly specified composite cost function (by value and/or gradient
   * information).
   */
  itkSetObjectMacro(Optimizer, OptimizerType)
  /**
   * Get user-specified optimizer component which aims at optimizing the
   * implicitly specified composite cost function (by value and/or gradient
   * information).
   */
  itkGetObjectMacro(Optimizer, OptimizerType)

  /**
   * Initialize the optimization component. Connect it with the composite
   * metric component.
   * NOTE: Whenever the optimizer component is exchanged, this method should
   * be called in order to update the internal optimizer-metric-connection.
   * @param setInitialPosition if TRUE the initial position (= initial
   * transform) will be set
   */
  virtual void InitializeOptimizer(bool setInitialPosition)
      throw (itk::ExceptionObject);

  /** Get last explicitly evaluated composite metric parameters. **/
  itkGetMacro(LastMetricParameters, ParametersType)
  /** Get last explicitly evaluated composite metric value. **/
  itkGetMacro(LastMetricValue, MetricMeasureType)
  /** Get last explicitly evaluated composite metric derivative. **/
  itkGetMacro(LastMetricDerivative, MetricDerivativeType)

  /**
   * Set whether or not the projection geometry (DRR origin, DRR
   * spacing, DRR size) should automatically be adjusted according to current
   * multi-resolution level and current fixed image region. **/
  itkSetMacro(UseAutoProjectionPropsAdjustment, bool)
  /**
   * Get whether or not the projection geometry (DRR origin, DRR
   * spacing, DRR size) should automatically be adjusted according to current
   * multi-resolution level and current fixed image region. **/
  itkGetMacro(UseAutoProjectionPropsAdjustment, bool)

  /** Get number of effective composite metric evaluations at current level. **/
  itkGetMacro(NumberOfMetricEvaluationsAtLevel, unsigned int)

  /**
   * Set a flag indicating that the registration should be stopped as soon as
   * possible. The registration can only be stopped during multi-resolution
   * changes as the generic optimizer component has no general 'stop'-interface.
   * In addition the StopRequestedEvent() is invoked which may be used to call
   * the individual 'stop'-method of the used optimizer component.
   **/
  virtual void StopRegistration();


protected:
  /** Type of internal transform (for metric). **/
  typedef typename MetricType::TransformType InternalTransformType;
  typedef typename InternalTransformType::Pointer InternalTransformPointer;

  /** Type of internal interpolator (for metric). **/
  typedef OneToOneInterpolateImageFunction<Moving2DImageType, double>
      InterpolatorType;
  typedef typename InterpolatorType::Pointer InterpolatorPointer;

  /** Smart Pointer type to a DataObject. **/
  typedef typename itk::DataObject::Pointer DataObjectPointer;

  /** Type of metric-fixed-image mapping. **/
  typedef std::map<BaseMetricType *, unsigned int> MetricFixedImageMapType;
  typedef std::pair<BaseMetricType *, unsigned int> MetricFixedImagePairType;

  /** Types of intermediate image casters. **/
  typedef itk::CastImageFilter<Moving3DImageType, SignedVolumeImageType>
      VolumeCasterType;
  typedef typename VolumeCasterType::Pointer VolumeCasterPointer;
  typedef itk::CastImageFilter<SignedVolumeImageType, Moving3DImageType>
      VolumeBackCasterType;
  typedef typename VolumeBackCasterType::Pointer VolumeBackCasterPointer;

  /** Type of fixed image region pyramid. **/
  typedef std::vector<FixedImageRegionType> FixedImageRegionPyramidType;

  /** Type of internal image information changer. **/
  typedef itk::ChangeInformationImageFilter<FixedImageType> ChangeInfoType;
  typedef typename ChangeInfoType::Pointer ChangeInfoPointer;

  /** Type of internal dimension reducers (3D -> 2D). **/
  typedef itk::ExtractImageFilter<typename DRREngineType::OutputImageType,
      FixedImageType> ExtractorType;
  typedef typename ExtractorType::Pointer ExtractorPointer;
  typedef typename DRREngineType::OutputImageType::RegionType Region3DType;
  typedef typename DRREngineType::OutputImageType::IndexType Index3DType;
  typedef typename DRREngineType::OutputImageType::SizeType Size3DType;

  /** Metric mask type. **/
  typedef itk::ImageMaskSpatialObject<MetricType::FixedImageDimension> MetricMaskType;
  typedef typename MetricMaskType::Pointer MetricMaskPointer;

  /** Type of internal mask dimension reducers/casters (3D -> 2D). **/
  typedef itk::ExtractImageFilter<typename DRREngineType::MaskImageType,
  		MaskImageType> MaskExtractorType;
  typedef typename MaskExtractorType::Pointer MaskExtractorPointer;
  typedef typename MaskImageType::RegionType MaskRegionType;
  typedef typename MaskImageType::IndexType MaskIndexType;
  typedef typename MaskImageType::SizeType MaskSizeType;
  typedef typename MaskImageType::SpacingType MaskSpacingType;
  typedef typename MaskImageType::PointType MaskPointType;
  typedef typename MaskImageType::DirectionType MaskDirectionType;
  typedef itk::CastImageFilter<MaskImageType, typename DRREngineType::MaskImageType>
      MaskCasterType;
  typedef typename MaskCasterType::Pointer MaskCasterPointer;

  /** Command type for receiving events. **/
  typedef itk::ReceptorMemberCommand<Self> ReceptorType;
  typedef typename ReceptorType::Pointer ReceptorPointer;
  typedef itk::MemberCommand<Self> MemberType;
  typedef typename MemberType::Pointer MemberPointer;

  /** Internal transform type (parametrizable identity). **/
  typedef ParametrizableIdentityTransform<double, 2> IdentityTransformType;
  typedef IdentityTransformType::Pointer IdentityTransformPointer;

  /** Thread access synchronization type. **/
  typedef itk::SimpleFastMutexLock MutexType;

  /** Constant representing an 'invalid' observer tag (not set). **/
  static const unsigned long INVALID_OBSERVER_TAG = 0x99999999;

  /** Moving 3D volume (DRRs are computed from this moving volume). **/
  Moving3DImagePointer m_Moving3DVolume;
  /** Vector of n fixed 2D images (reference images). **/
  std::vector<FixedImagePointer> m_FixedImages;
  /** Vector of n 2D fixed image regions (reference image regions). **/
  std::vector<FixedImageRegionType> m_FixedImageRegions;
  /** Vector of masks for the different fixed images. **/
  std::vector<MaskImagePointer> m_Masks;
  /** Vector for 2D spatial mask objects for the different fixed images. **/
  std::vector<MetricMaskPointer> m_MetricMasks;
  /**
   * Internal DRR engine that generates the moving 2D images outgoing from the
   * 3D input volume. This filter is GPU-based to avoid the 'DRR-bottleneck'.
   **/
  DRREnginePointer m_DRREngine;
  /**
   * Composite metric component that takes multiple metric inputs and maps them
   * to a single output.
   */
  MetricPointer m_Metric;
  /** User-specified 3D/3D transformation component. **/
  TransformPointer m_Transform;
  /** Initial transform parameters for registration initialization. **/
  ParametersType m_InitialTransformParameters;
  /**
   * Vector of parameterizable identity transforms (each sub-metric receives an
   * exclusive transform - this is for distinction and synchronization later).
   * The parametrization of such a transform will modify the user-set transform
   * component (the one that is connected to the central DRR engine at the end).
   **/
  std::vector<IdentityTransformPointer> m_InternalTransforms;
  /** Only 1 transform at a time is allowed to modify the connected 3D transform **/
  MutexType m_InternalTransformsMutex;
  /** Vector of 2D fixed image pyramids for multi-resolution approaches **/
  std::vector<FixedPyramidPointer> m_FixedPyramids;
  /** Vector of 2D fixed image region pyramids for multi-resolution appr. **/
  std::vector<FixedImageRegionPyramidType> m_FixedRegionPyramids;
  /** Vector of masks pyramids for multiresolution approach. **/
  std::vector<MaskPyramidPointer> m_MaskPyramids;
  /** Vector of dimension reducers 3D->2D for moving images (DRRs) **/
  std::vector<ExtractorPointer> m_DimensionReducers;
  /** Vector of dimension caster 2D->3D for masks **/
  std::vector<MaskCasterPointer> m_MaskCasters;
  /** Vector of moving 2D image information changers (for DRRs) **/
  std::vector<ChangeInfoPointer> m_InformationChangers;
  /** Vector of moving 2D image interpolators **/
  std::vector<InterpolatorPointer> m_Interpolators;
  /** Vector of n projection properties (one for each fixed image). **/
  std::vector<ProjectionPointer> m_ProjectionGeometries;
  /**
   * Schedule for 2D fixed image pyramid for multi-resolution approaches: <br>
   * each row specifies a multi-resolution level consisting of 2 columns each
   * containing the respective dimension-shrink-factor, e.g.: <br>
   * <code>
   * 8 4 <br>
   * 4 2 <br>
   * 2 1 </code><br>
   * is a 3-level pyramid which shrinks the image by factor 8 along the first
   * dimension and by factor 4 along the second dimension ... in the third
   * level it shrinks the image by factor 2 along the first dimension and keeps
   * the original resolution along the second dimension
   * @see SetNumberOfLevels()
   * @see SetSchedules()
   **/
  FixedScheduleType m_FixedSchedule;
  /**
   * Flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for levels where all schedule factors are equal to
   * 1.0. NOTE: the pyramid filter additionally applies gaussian smoothing which
   * may be undesired for 'original'-resolution images in some registration
   * scenarios.
   */
  bool m_UseFixedPyramidForUnshrinkedLevels;
  /**
   * Flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for the final (finest) level. If set to TRUE the last
   * schedule factors will be ignored and the original image is passed (without
   * any kind of smoothing).
   */
  bool m_UseFixedPyramidForFinalLevel;
  /** 3D moving volume pyramid for multi-resolution approaches **/
  MovingPyramidPointer m_MovingPyramid;
  /**
   * Schedule for 3D moving volume pyramid for multi-resolution approaches:<br>
   * each row specifies a multi-resolution level consisting of 3 columns each
   * containing the respective dimension-shrink-factor, e.g.: <br>
   * <code>
   * 8 4 4 <br>
   * 2 1 1 </code><br>
   * is a 2-level pyramid which shrinks the image by factor 8 along the first
   * dimension and by factor 4 along the second and third dimensions ... in the
   * second level it shrinks the image by factor 2 along the first dimension and
   * keeps the original resolution along the second and third dimensions
   * @see SetNumberOfLevels()
   * @see SetSchedules()
   **/
  FixedScheduleType m_MovingSchedule;
  /**
   * Flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for levels where all schedule factors are equal to
   * 1.0. NOTE: the pyramid filter additionally applies gaussian smoothing which
   * may be undesired for 'original'-resolution images in some registration
   * scenarios.
   */
  bool m_UseMovingPyramidForUnshrinkedLevels;
  /**
   * Flag determining whether or not to use the internal multi-resolution
   * pyramid image filter for the final (finest) level. If set to TRUE the last
   * schedule factors will be ignored and the original image is passed (without
   * any kind of smoothing).
   */
  bool m_UseMovingPyramidForFinalLevel;
  /** Intermediate volume caster (possibly unsigned to signed pixel type) **/
  VolumeCasterPointer m_VolumeCaster;
  /** Intermediate volume back caster (signed to unsigned pixel type) **/
  VolumeBackCasterPointer m_VolumeBackCaster;
  /** Mappings between metrics and fixed images (by indices) **/
  MetricFixedImageMapType m_MetricFixedImageMappings;
  /** Holds the current multi-resolution level number (0-based). **/
  unsigned int m_CurrentLevel;
  /** Tag of BeforeEvaluationEvent() observer. **/
  unsigned long m_BeforeEvaluationObserverTag;
  /** Tag of AfterEvaluationEvent() observer. **/
  unsigned long m_AfterEvaluationObserverTag;
  /**
   * Command receiving composite metric's before/after evaluation events.
   * @see ora::BeforeEvaluationEvent
   * @see ora::AfterEvaluationEvent
   **/
  ReceptorPointer m_EvaluationCommand;
  /**
   * Command receiving events of the internal (identity) transforms.
   * @see ora::BeforeParametersSet
   * @see ora::TransformChanged
   * @see ora::AfterParametersSet
   **/
  MemberPointer m_TransformsCommand;
  /**
   * User-specified optimizer component which aims at optimizing the implicitly
   * specified composite cost function (by value and/or gradient information).
   */
  OptimizerPointer m_Optimizer;
  /** Last explicitly evaluated composite metric parameters. **/
  ParametersType m_LastMetricParameters;
  /** Last explicitly evaluated composite metric value. **/
  MetricMeasureType m_LastMetricValue;
  /** Last explicitly evaluated composite metric derivative. **/
  MetricDerivativeType m_LastMetricDerivative;
  /**
   * Flag indicating whether or not the projection geometry (DRR origin, DRR
   * spacing, DRR size) should automatically be adjusted according to current
   * multi-resolution level and current fixed image region. **/
  bool m_UseAutoProjectionPropsAdjustment;

  /** Counts number of effective composite metric evaluations at current level. **/
  unsigned int m_NumberOfMetricEvaluationsAtLevel;
  /** Flag indicating that registration should be stopped as soon as possible **/
  bool m_Stop;

  /** Default constructor. **/
  MultiResolutionNWay2D3DRegistrationFramework();
  /** Destructor. **/
  virtual ~MultiResolutionNWay2D3DRegistrationFramework();
  /** Standard object output. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Make a DataObject of the correct type to be used as the specified
   * output.
   */
  virtual DataObjectPointer MakeOutput(unsigned int idx);

  /**
   * Method invoked by the pipeline in order to trigger the computation of
   * the filter which is n-way 2D/3D-registration in this case.
   */
  virtual void GenerateData() throw (itk::ExceptionObject);

  /**
   * Apply multi-resolution settings (schedule, number of levels) of the moving
   * image pyramid to all involved registration components.
   */
  virtual void ApplyMovingMultiResolutionSettings();
  /**
   * Apply multi-resolution settings (schedule, number of levels) of the fixed
   * image pyramid to all involved registration components.
   */
  virtual void ApplyFixedMultiResolutionSettings() throw (itk::ExceptionObject);

  /**
   * Update the moving image multi-resolution pipeline connections w.r.t. the
   * specified level and current configuration. This means that the right
   * multi-resolution output is connected to the DRR engine's input.
   * @param level reference level
   * @see m_UseMovingPyramidForFinalLevel
   * @see m_UseMovingPyramidForUnshrinkedLevels
   * @see m_MovingSchedule
   */
  virtual void UpdateMovingImageMultiResolutionConnections(unsigned int level);
  /**
   * Update the fixed image multi-resolution pipeline connections w.r.t. the
   * specified level and current configuration. This means that the right
   * multi-resolution output is connected to the respective metric sub-
   * component's input.
   * @param level reference level
   * @see m_UseFixedPyramidForFinalLevel
   * @see m_UseFixedPyramidForUnshrinkedLevels
   * @see m_FixedSchedule
   */
  virtual void UpdateFixedImageMultiResolutionConnections(unsigned int level);

  /**
   * Apply specified projection properties to DRR engine. Optionally the
   * properties are auto-adjusted to the current multi-resolution level by
   * taking the current fixed image region into account. Additionally, the right
   * mask (if there is one configured) is set to the DRR engine.
   * @param geometry pointer to the DRR projection geometry object
   * @param level if <0 (default) the specified projection properties are
   * applied to the DRR engine 1:1; if level>=0 the projection properties are
   * auto-adjusted to the specified multi-resolution level w.r.t. to the fixed
   * image region of the according multi-resolution level (impacts: image
   * origin, image size, image spacing only; the rest of the properties e.g.
   * sampling distance must be manually changed by the user who can listen to
   * the registration method's IterationEvent() and composite metric's
   * BeforeEvaluationEvent() to realize that). NOTE: The DRR mask (if set) is
   * NOT adjusted to the level as this is dependent on the purpose of use!
   * @param fixedImageIndex index of the fixed image that is referenced
   * (required if level>=0 to work properly) and used for auto-adjustment
   * NOTE: Auto-adjustment requires the projection properties to define the DRR
   * geometry at the FINEST LEVEL!!!
   **/
  virtual void ApplyProjectionPropsToDRREngine(ProjectionPointer geometry,
      int level, int fixedImageIndex);

  /** This method is called during initialization to initialize the DRREngine
   * outputs for the specified level. */
  virtual void ApplyAllProjectionPropsToDRREngine(int level);

  /**
   * Add internal observers for monitoring the composite metric.
   * @see RemoveInternalObservers()
   **/
  virtual void AddInternalObservers();
  /**
   * Remove internal observers for monitoring the composite metric.
   * @see AddInternalObservers()
   **/
  virtual void RemoveInternalObservers();
  /**
   * Entry point for composite metric's before/after evaluation event.
   * @param eventObject event specification
   * @see ora::BeforeEvaluationEvent
   * @see ora::AfterEvaluationEvent
   **/
  virtual void OnMetricEvaluationEvent(const itk::EventObject &eventObject);
  /**
   * Entry point for parametrizable identity transforms' before/after parameters
   * set and transform changed events.
   * @param eventObject event specification
   * @see ora::BeforeParametersSet
   * @see ora::TransformChanged
   * @see ora::AfterParametersSet
   **/
  virtual void OnTransformsEvent(itk::Object * obj,
      const itk::EventObject &eventObject);

private:
  /** Purposely not implemented. **/
  MultiResolutionNWay2D3DRegistrationFramework(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraMultiResolutionNWay2D3DRegistrationFramework.txx"

#endif /* ORAMULTIRESOLUTIONNWAY2D3DREGISTRATIONFRAMEWORK_H_ */
