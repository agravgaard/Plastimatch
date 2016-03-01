//
#ifndef ORAREG23MODEL_H_
#define ORAREG23MODEL_H_

#include <QMutexLocker>
#include <QMutex>
#include <QVector>
#include <QPointF>

#include <oraModel.h>  // ORAIFMVC
#include <oraTaskManager.h>  // ORAIFModel
#include <oraTask.h>  // ORAIFModel
#include <oraITKVTKImage.h>  // ORAIFImageAccess

#include <itkProcessObject.h>
#include <itkImage.h>
#include <itkEuler3DTransform.h>
#include <itkCommand.h>

#include <vector>

// forward declarations:
namespace ora
{
class IniAccess;
class ImageConsumer;
class ImageImporterTask;
class REG23RegistrationInitializationTask;
class REG23SparseRegistrationExecutionTask;
class REG23RegistrationExecutionTask;
class VTKStructureSet;
class VTKStructure;
class SimpleTransformUndoRedoManager;
class AbstractTransformTask;
}
class vtkRenderWindow;
class vtkColorTransferFunction;
class vtkImageData;
class vtkPolyData;
class vtkTimerLog;

namespace ora
{

/**
 * FIXME:
 *
 * @author phil 
 * @author Markus 
 * @version 1.3
 */
class REG23Model :
      public Model
{
public:
  /** Generic public types (for pixel-independent handling later) **/
  typedef itk::ProcessObject::Pointer GenericRegistrationPointer;
  typedef float DRRPixelType; // DRR pixel type is statically float!
  typedef unsigned char MaskPixelType; // Mask pixel type is statically uchar!
  typedef itk::Euler3DTransform<double> TransformType;
  typedef TransformType::Pointer TransformPointer;
  typedef TransformType::ParametersType ParametersType;
  // NOTE: Must include "oraTask.h" for Visual Studio, otherwise callbacks fail
  typedef itk::MemberCommand<ora::Task> CommandType;
  
  typedef CommandType::Pointer CommandPointer;

  /** MVC-notification UID for cost function re-rendering (update). **/
  static const int UPDATE_COST_FUNCTION = 1001;
  /** MVC-notification UID for moving image update. NOTE: The images are only
   * available during this event - the images must be copied! **/
  static const int UPDATE_MOVING_IMAGES = 1002;
  /** MVC-notification UID for registration parameters change (update). **/
  static const int UPDATE_PARAMETERS = 1003;
  /** MVC-notification UID for "input images complete" state. **/
  static const int UPDATE_INPUT_IMAGES = 1004;
  /** MVC-notification UID for "mask images complete" state. **/
  static const int UPDATE_MASK_IMAGES = 1005;
  /** MVC-notification UID for initial moving image update. NOTE: The images are
   * only available during this event - the images must be copied! **/
  static const int UPDATE_INITIAL_MOVING_IMAGES = 1006;
  /** MVC-notification UID for current moving image update. NOTE: The images are
   * only available during this event - the images must be copied! **/
  static const int UPDATE_CURRENT_MOVING_IMAGES = 1007;

  // Note: Not the best solution to represent flags but sufficient
  // Better would be QFlags or a FlagSet class that internally uses a bitset
  /** Possible pre- and post-processing operations on images. */
  typedef enum
  {
    /** Cast image. */
    CAST = 1 << 1,
    /** Crop image. */
    CROP = 1 << 2,
    /** Resample image (spacing). */
    RESAMPLE = 1 << 3,
    /** Rescale image intensities to the range [min,max]. */
    RESCALEMINMAX = 1 << 4,
    /** Rescale image intensities by multiplying with \a scale and then
     * adding \a shift. */
    RESCALESHIFTSCALE = 1 << 5,
    /** Rescale image intensities by windowing ([win-min,win-max] is mapped
     * to [out-min;out-max] and clamped). */
    RESCALEWINDOWING = 1 << 6,
    /** Store image into a file. */
    STORE = 1 << 7,
    /** Rescale image intensities to the range [min,max] within the specified
     * mask region. */
    RESCALEMINMAXMASK = 1 << 8,
    /** Apply unsharp masking to the image. **/
    UNSHARPMASKING = 1 << 9,
    /** Apply flipping to the image data. **/
    FLIP = 1 << 10,
    /** Apply thresholding (above-mode) to the image data. **/
    THRESHOLD_ABOVE = 1 << 11,
    /** Apply thresholding (below-mode) to the image data. **/
    THRESHOLD_BELOW = 1 << 12,
    /** Apply thresholding (outside-mode) to the image data. **/
    THRESHOLD_OUTSIDE = 1 << 13,
    /** Represents all operations. */
    ALL = CAST | CROP | RESAMPLE | RESCALEMINMAX | RESCALESHIFTSCALE |
          RESCALEWINDOWING | STORE | RESCALEMINMAXMASK | UNSHARPMASKING | FLIP |
          THRESHOLD_ABOVE | THRESHOLD_BELOW | THRESHOLD_OUTSIDE
  } ProcessOperations;

  /** Default constructor. **/
  REG23Model();
  /** Destructor. **/
  ~REG23Model();

  /** Get current config file name **/
  std::string GetConfigFile()
  {
    return m_ConfigFile;
  }
  /** Set current config file name **/
  void SetConfigFile(std::string file)
  {
    m_ConfigFile = file;
  }

  /**
   * Load an REG23 configuration file (INI-like) which contains information
   * on the registration setup, and - optionally - on the images to be
   * registered. <br>
   * NOTE: There is an exemplary config file in the source directory
   * (reg23-config.ini)! The internal member m_ConfigFile contains the name.
   * @param errorSection return the section where an error occured
   * @param errorKey return the key where an error occured
   * @param errorMessage return the error message
   * @return TRUE if the configuration is sufficient
   */
  bool LoadConfiguration(std::string &errorSection, std::string &errorKey,
                         std::string &errorMessage);

  /** @return TRUE if the previously loaded configuration is valid **/
  bool HaveValidConfiguration();

  /** @return TRUE if the previously loaded configuration contains ORA connectivity **/
  bool HaveORAConnectivity();

  /** Set optional application command line arguments (to support
   * "FROM-COMMAND-LINE" entries in image file specifications).
   **/
  void SetCommandLineArguments(std::vector<std::string> args)
  {
    m_CommandLineArguments = args;
  }
  /** Get optional application command line arguments (to support
   * "FROM-COMMAND-LINE" entries in image file specifications).
   **/
  std::vector<std::string> GetCommandLineArguments()
  {
    return m_CommandLineArguments;
  }

  /** Set index of the last option entry in the command line arguments (in order
   * to be able to parse the file names after the options).
   */
  void SetLastCommandLineOptionIndex(int index)
  {
    m_LastCommandLineOptionIndex = index;
  }
  /** Get index of the last option entry in the command line arguments (in order
   * to be able to parse the file names after the options).
   */
  int GetLastCommandLineOptionIndex()
  {
    return m_LastCommandLineOptionIndex;
  }

  /** Get volume file name retrieved from config file / command line **/
  std::string GetVolumeImageFileName()
  {
    return m_VolumeImageFileName;
  }
  /** Get fixed image file names retrieved from config file / command line **/
  std::vector<std::string> GetFixedImageFileNames()
  {
    return m_FixedImageFileNames;
  }
  /** @return TRUE if currently set image file names are OK **/
  bool HaveValidImageFileNames();
  /** Get view names according to fixed image file names (the configuration
   * loader tries to retrieve them via the ViewInfo that must be placed
   * in the image folder - works only for RTI input images!) **/
  std::vector<std::string> GetViewNames()
  {
    return m_ViewNames;
  }

  /** Parses an entry for DRR variables (drrmin, drrmax, drrmean, drrstd,
   * drrminmask, drrmaxmask, drrmeanmask, drrstdmask).
   * @param entry Entry to parse. Internally it is converted to lower case.
   * @return TRUE if a variable name is found in the \a entry string, else FALSE.
   */
  bool ParseEntryHasVariable(const std::string entry) const;

  /**
   * Generate an image importer task for the volume and return it. This task
   * is NOT added to any task manager. It returns NULL if an error occurred.
   **/
  ImageImporterTask *GenerateVolumeImporterTask();
  /**
   * Generate an image importer task for each fixed image and return them. The
   * tasks are NOT added to any task manager. It returns an empty vector if an
   * error occurred.
   **/
  std::vector<ImageImporterTask *> GenerateFixedImageImporterTasks();
  /** Generate the task that is capable of initializing/deinitalizing this
   * model (registration components including pre-processing). If the task
   * could successfully be generated it is returned, otherwise NULL is returned.
   */
  REG23RegistrationInitializationTask *GenerateInitializationTask();
  /** Generate the task that executes N-way 2D/3D registration. If the task
   * could successfully be generated it is returned, otherwise NULL is returned.
   */
  REG23RegistrationExecutionTask *GenerateExecutionTask();
  /** Generate the task that executes sparse 2D/3D registration. If the task
   * could successfully be generated it is returned. If the task generation is
   * NOT REQUESTED (m_ExecuteSparsePreRegistration == FALSE) or the task
   * generation failed, NULL is returned.
   */
  REG23SparseRegistrationExecutionTask *GenerateSparseExecutionTask();

  /** Set primary task manager (for easier access) **/
  void SetTaskManager(TaskManager *taskman);
  /** Get primary task manager (for easier access) **/
  TaskManager *GetTaskManager();

  /** Connect a render window to global rendering lock (GL synchronization). **/
  void ConnectRenderWindowToGlobalLock(vtkRenderWindow *renWin);
  /** Disconnect a render window from global rendering lock (GL synchronization). **/
  void DisconnectRenderWindowFromGlobalLock(vtkRenderWindow *renWin);

  /** Set an external tool render window for internal rendering tasks (helper) **/
  void SetToolRenderWindow(vtkRenderWindow *renWin);
  /** Get an external tool render window for internal rendering tasks (helper) **/
  vtkRenderWindow *GetToolRenderWindow()
  {
    return m_ToolRenderWindow;
  }
  /** Set an external DRR tool render window for internal rendering **/
  void SetDRRToolRenderWindow(vtkRenderWindow *renWin);
  /** Get external DRR tool render window for internal rendering **/
  vtkRenderWindow *GetDRRToolRenderWindow()
  {
    return m_DRRToolRenderWindow;
  }

  /** Access command that is attached to the optimizer IterationEvent **/
  CommandType *GetOptimizerCommand()
  {
    return m_OptimizerCommand;
  }
  /**
   * Access command that is attached to the registration events (StartEvent,
   * StartMultiResolutionLevelEvent, StartOptimizationEvent, EndEvent)
   **/
  CommandType *GetRegistrationCommand()
  {
    return m_RegistrationCommand;
  }

  /**
   * @return TRUE if all data and component initializations which are necessary
   * for automatic N-way 2D/3D registration are available
   **/
  bool IsReadyForAutoRegistration();
  /**
   * @return TRUE if all data and component initializations which are necessary
   * for manual N-way 2D/3D registration are available
   **/
  bool IsReadyForManualRegistration();

  /** Get flag indicating whether registration should be started automatically **/
  bool GetStartRegistrationAutomatically()
  {
    return m_StartRegistrationAutomatically;
  }
  /** Set flag indicating whether registration should be started automatically **/
  void SetStartRegistrationAutomatically(bool flag)
  {
    m_StartRegistrationAutomatically = flag;
  }

  /** Get buffered flag indicating whether cost function should be rendered **/
  bool GetRenderCostFunction()
  {
    return m_RenderCostFunction;
  }

  /** Get pointer to table containing current function values (basically
   * valid if GetRenderCostFunction() returns TRUE. Thread-safe. **/
  const QVector<QVector<QPointF> > &GetFunctionValues();
  /** Get pointer to table containing current function values without
   * built-in thread-safety. Use the the lock acquiring/releasing methods.
   * @see AcquireFunctionLock()
   * @see ReleaseFunctionLock()
   **/
  const QVector<QVector<QPointF> > &GetFunctionValuesThreadUnsafe()
  {
    return m_FunctionValues;
  }

  /** Get the i-th (index) current moving image. NOTE: A moving image will be
   * basically only available during UPDATE_MOVING_IMAGES (MVC-event). If the
   * image is not available, NULL is returned. Thread-safe.
   */
  ITKVTKImage *GetCurrentMovingImage(std::size_t index);

  /** Get the i-th (index) fixed image. NOTE: A fixed image is basically
   * guaranteed to be available during UPDATE_INPUT_IMAGES (MVC-event). If the
   * image is not available, NULL is returned. Thread-safe.
   * @param index The fixed image index.
   * @param getPostProcessed If \c TRUE the post-processed fixed-image is returned.
   *    Note: If the post-processed fixed image is not available (NULL) the
   *    pre-processed fixed image is returned.
   * @return The pre-processed fixed image if available, else NULL. If \a
   *    getPostProcessed is TRUE the post-processed fixed image if available.
   */
  ITKVTKImage *GetFixedImage(const std::size_t index, const bool getPostProcessed = false);
  /** Get the volume image. NOTE: The volume is basically
   * guaranteed to be available during UPDATE_INPUT_IMAGES (MVC-event). If the
   * image is not available, NULL is returned. Thread-safe.
   */
  ITKVTKImage *GetVolumeImage();

  /** Get the i-th (index) mask image. NOTE: A mask image is basically
   * guaranteed to be available during UPDATE_MASK_IMAGES (MVC-event). If the
   * image is not available, NULL is returned. Thread-safe.
   */
  ITKVTKImage *GetMaskImage(std::size_t index);

  /** Get the i-th (index) mask image contour. NOTE: A mask image is basically
   * guaranteed to be available during UPDATE_MASK_IMAGES (MVC-event). If the
   * image is not available, NULL is returned. Thread-safe.
   */
  ITKVTKImage *GetMaskImageContour(std::size_t index);

  /** Get the i-th (index) imaging source position. If the source position is
   * not available, NULL is returned - otherwise the 3-tuple is returned.
   */
  double *GetSourcePosition(std::size_t index);

  /** Get the center of rotation where rotations relate to. **/
  void GetCenterOfRotation(double centerOfRotation[3]);

  /** Send the initial parameters by throwing an UPDATE_PARAMETERS
   * update and making the initial parameters TEMPORARILY available in the
   * current parameters member. **/
  void SendInitialParametersNotification();

  /**
   * Get current transform parameters (from manual or automatic
   * registration). NOTE: These parameters do not necessarily equal the
   * current optimization parameters!
   * @see UpdateCurrentRegistrationParameters()
   **/
  ParametersType GetCurrentParameters()
  {
    return m_CurrentParameters;
  }
  /** Get reference transform parameters (scientific mode). **/
  ParametersType GetReferenceParameters()
  {
    return m_ReferenceParameters;
  }
  /**
   * Override and apply the specified current transformation parameters (mainly
   * for manual registration).
   */
  void OverrideAndApplyCurrentParameters(ParametersType parameters);
  /**
   * Get current optimization parameters (from automatic registration). NOTE: These
   * parameters represent the current optimization position which may depend
   * on the optimization strategy - not necessarily equal to transform pars!
   * @see UpdateCurrentRegistrationParameters()
   **/
  ParametersType GetCurrentOptimizationParameters()
  {
    return m_CurrentOptimizationParameters;
  }
  /** Get stored number of total (planned) optimization iterations **/
  int GetNumberOfIterations()
  {
    return m_NumberOfIterations;
  }
  /**
   * Get current optimization iteration. This measure must be manually updated from
   * the external optimization callback. Use IncrementIteration().
   * @see IncrementIteration()
   **/
  int GetCurrentIteration()
  {
    return m_CurrentIteration;
  }
  /**
   * Get current optimization measure value. This measure represents the current
   * optimizer value, and not necessarily the last metric value.
   * @see UpdateCurrentRegistrationParameters()
   **/
  double GetCurrentOptimizationMeasure()
  {
    return m_CurrentOptimizationMeasure;
  }
  /**
   * Get last composite metric value (not necessarily the last optimization measure).
   * @see UpdateCurrentRegistrationParameters()
   */
  double GetLastMetricMeasure()
  {
    return m_LastMetricMeasure;
  }

  /** Acquire the function lock and keep the model from modifying it.
   * Please release it as fast as possible!
   * @see ReleaseFunctionLock()
   **/
  void AcquireFunctionLock()
  {
    m_FunctionValuesMutex.lock();
  }
  /** Release the function lock after acquiring it.
   * @see AcquireFunctionLock()
   **/
  void ReleaseFunctionLock()
  {
    m_FunctionValuesMutex.unlock();
  }

  /** Send the MVC-notification "UPDATE_INPUT_IMAGES" telling the model
   * observers that the input images are now complete. **/
  void SendInputImagesCompleteNotification();

  /** Send the MVC-notification "UPDATE_MASK_IMAGES" telling the model
   * observers that the mask images are now complete. BUT: the notification is
   * only sent if there is at least one mask image! **/
  void SendMaskImagesCompleteNotification();

  /**
   * Save the current transform matrix (corresponding to current parameters) to
   * a file if m_OutputTransformFile is set.
   * In addition, the user tracking Info from the transform undo/redo framework
   * is stored to the file in the section [User-Tracking]
   * @return TRUE if either m_OutputTransformFile was set and the file was
   * successfully generated or m_OutputTransformFile was not set; return FALSE
   * is some error occured
   * @see m_OutputTransformFile
   **/
  bool SaveTransformToFile();

  /** Get optional configured graphics style (Qt widgets and windows) **/
  std::string GetConfiguredGraphicsStyle()
  {
    return m_ConfiguredGraphicsStyle;
  }

  /** Global render protection callback. **/
  static void GlobalRenderCoordinationCB(vtkObject *caller, unsigned long eid,
      void *clientdata, void *calldata);

  /** Get the window and level related min/max intensity-values for the
   * specified fixed image according to the configuration. By default - if there
   * is no config entry - the min and max values found in the image are returned.
   * @param fixedImage for some config entries the fixed image may be necessary
   * @param index fixed image index
   * @param unsharpMaskImage if true, the window/level min/max are returned for
   * the unsharp mask representation of the fixed image
   * @return intensity minimum followed by maximum if successful; if an error
   * occurs, NULL is returned **/
  double *GetWindowLevelMinMaxForFixedImage(vtkImageData *fixedImage,
      std::size_t index, bool unsharpMaskImage = false);

  /** Get flag indicating whether or not the transformation help widgets are
   * initially visible. **/
  bool GetTransformationWidgetsVisible()
  {
    return m_TransformationWidgetsVisible;
  }

  /** Get flag indicating whether or not the nature of transformation is
   * automatically determined based on the initial left mouse click. **/
  bool GetRegionSensitiveTransformNature()
  {
    return m_RegionSensitiveTransformNature;
  }

  /** Concatenate the current internal transformation with the specified
   * axis/angle rotation (relative to the current center of rotation). The axis
   * should be normalized, and the angle is represented in radians. **/
  void ConcatenateAndApplyRelativeAxisAngleRotation(double *axis, double angle);

  /**
   * Compute the current moving images according to the current transformation.
   * The UPDATE_CURRENT_MOVING_IMAGES event is generated if successful - the
   * images are only available during this notification.
   **/
  template<typename TVolumeComponentType>
  void ComputeCurrentMovingImages();

  /** Get flag indicating whether or not the "display masks" option is initially
   * set. **/
  bool GetDisplayMasks()
  {
    return m_DisplayMasks;
  }
  /** Get color of the displayed mask contours in HSV color space. **/
  void GetMasksColorHSV(double color[3])
  {
    color[0] = m_MasksColorHSV[0];
    color[1] = m_MasksColorHSV[1];
    color[2] = m_MasksColorHSV[2];
  }

  /** Get pointer to undo/redo manager for transforms. Basically, not
   * thread-safe! **/
  SimpleTransformUndoRedoManager *GetUndoRedoManager()
  {
    return m_UndoRedoManager;
  }

  /** Get flag indicating whether or not a safety question dialog is shown if the
   * user presses the cancel ("decline") button instead of the OK ("accept")
   * button. **/
  bool GetShowWarningOnCancel()
  {
    return m_ShowWarningOnCancel;
  }
  /** Get flag indicating whether or not a safety warning dialog is shown if the
   * fixed images have acquisition date/times that diverge more than
   * m_MaxFixedImageAcquisitionDivergingTimeMin. **/
  bool GetShowWarningOnDivergingFixedImageAcquisitionTimes()
  {
    return m_ShowWarningOnDivergingFixedImageAcquisitionTimes;
  }
  /** Get the time value (in fractions of a minute) that defines the maximum
   * fixed image acquisition time difference over all fixed images. **/
  double GetMaxFixedImageAcquisitionDivergingTimeMin()
  {
    return m_MaxFixedImageAcquisitionDivergingTimeMin;
  }

  /** Compute the overall fixed image acquisition time divergence in fractions
   * of a minute. If m_ShowWarningOnDivergingFixedImageAcquisitionTimes==false
   * or there is no ORA-connectivity, 0 will be returned. **/
  double ComputeFixedImagesAcquisitionTimeDivergence();

  /** Checks whether or not the hardware (GPU, OpenGL extensions) is adequate
   * (by generating a GLSL dummy ray-caster).
   * @param nonSupportReasons returned string that contains one or more newline-
   * separated strings describing the reasons for missing hardware-support **/
  bool IsHardwareAdequate(std::string &nonSupportReasons);

  /** Get scientific mode flag **/
  bool IsScientificMode()
  {
    return m_ScientificMode;
  }
  /** Set scientific mode flag **/
  void SetScientificMode(bool active)
  {
    m_ScientificMode = active;
  }
  /** Set intelligent mask mode flag (load/store masks for faster recovery) **/
  void SetIntelligentMaskMode(bool active)
  {
    m_IntelligentMaskMode = active;
  }
  /** Get intelligent mask mode flag (load/store masks for faster recovery) **/
  bool IsIntelligentMaskMode()
  {
    return m_IntelligentMaskMode;
  }

  /** Get flag indicating that real-time-adaptive windowing should be
   * activated. **/
  bool GetRealTimeAdaptiveWindowing()
  {
    return m_RealTimeAdaptiveWindowing;
  }

  /** Get factor determining windowing mouse sensitivity (<1.0 less sensitive,
   * >1.0 more sensitive). **/
  double GetWindowingSensitivity()
  {
    return m_WindowingSensitivity;
  }

  /** @return optimizer and metric type strings (abbreviations from config file);
   * is valid AFTER initialization, otherwise empty vector is returned; 1st
   * component is optimizer type, further components are the metric types for
   * each fixed image **/
  std::vector<std::string> GetOptimizerAndMetricTypeStrings();

  /** @return whether unsharp masking is enabled for the specified fixed image
   * (1-based index). **/
  bool IsUnsharpMaskingEnabled(std::size_t i);
  /** @return whether unsharp masking is activated for the specified fixed image
   * (1-based index). **/
  bool IsUnsharpMaskingActivated(std::size_t i);

  /** Get mode flag indicating whether or not the current window/level settings
   * should be stored in the respective fixed images' source folder (in a file
   * named reg23-wl.inf). 0..do not store, 1..store **/
  int GetWindowLevelStorageStrategy()
  {
    return m_WindowLevelStorageStrategy;
  }
  /** Set mode flag indicating whether or not the current window/level settings
   * should be stored in the respective fixed images' source folder (in a file
   * named reg23-wl.inf).
   * @param strategy 0..do not store, 1..store **/
  void SetWindowLevelStorageStrategy(int strategy)
  {
    m_WindowLevelStorageStrategy = strategy;
  }
  /** Get mode flag indicating whether or not the window/level settings
   * should be recovered from the respective fixed images' source folder (from a
   * file named reg23-wl.inf). 0..do not recover (apply window/level as
   * defined in config file), 1..recover from file (if existing and valid,
   * otherwise window/level applied as defined in config file) **/
  int GetWindowLevelRecoveryStrategy()
  {
    return m_WindowLevelRecoveryStrategy;
  }
  /** Set mode flag indicating whether or not the window/level settings
   * should be recovered from the respective fixed images' source folder (from a
   * file named reg23-wl.inf).
   * @param strategy 0..do not recover (apply window/level as
   * defined in config file), 1..recover from file (if existing and valid,
   * otherwise window/level applied as defined in config file) **/
  void SetWindowLevelRecoveryStrategy(int strategy)
  {
    m_WindowLevelRecoveryStrategy = strategy;
  }

  /** Store window/level to reg23-wl.inf file in respective fixed image
   * source-folder if storage strategy is accordingly set (otherwise do
   * nothing). This method is only executed if the registration framework has
   * been correctly initialized (ready for auto-registration).
   * @param index 0-based fixed image index
   * @param fixedImageWL window and level of the fixed image to be stored
   * @param unsharpMaskWL window and level of the unsharp mask image to be
   * stored**/
  void StoreWindowLevelToFile(std::size_t index, double fixedImageWL[2],
      double unsharpMaskWL[2]);

  /** Generate DRRs for all configured views with current transformation and
   * store them to disk.
   * @param outputDir directory where the generated DRRs should be stored
   * @param drrPattern file name patter which determines the generated DRR file
   * names (a "%d" must be included which will be replaced by the respective
   * view index starting with 1)
   * @param outputMode the image output mode (0..3D FLOAT, 1..2D USHORT,
   * 2..2D FLOAT, 3..2D UCHAR) - NOTE: the output mode must be compatible with
   * the image type (extension); furthermore, no rescaling is performed!
   * @return TRUE if the DRRs could be generated and saved
   **/
  template<typename TVolumeComponentType>
  bool GenerateAndSaveDRRs(std::string outputDir, std::string drrPattern,
      int outputMode);

  /**
   * Generate an ITFOptimizer (an example tool of the nreg2d3d framework)
   * configuration outgoing from the current transformation and registration
   * components configuration. NOTE: A configuration for each view is generated!
   * The process relates to the post-processed fixed images!
   * @param outputDir the directory where the input images and configuration
   * fragments should be stored
   * @return TRUE if successful
   */
  template<typename TVolumeComponentType>
  bool GenerateITFOptimizerConfiguration(std::string outputDir);

  /** Computes an initial transformation of the moving image (DRR) to the fixed
   * image (X-ray) by "brute-force" cross correlation (CC) of a "small" region
   * (patch) from the moving image with a region of the fixed image.
   * A fixed image region is extracted, which is based on the bounding box of
   * the generated mask (Auto-Masking!). If no mask is available the whole fixed
   * image region is used.
   * A rectangular patch is extracted from the current DRR around the projection
   * of the respective 3D volume center. If the volume center cannot be projected
   * the DRR center is used to extract the patch.
   * Normalized cross correlation (CC) is computed between the fixed image
   * region and the patch. The maximum CC-value is used as the target position
   * for the transformation. The translations are: t = p_moving - p_fixed, where
   * p_moving is the center of the patch and p_fixed is the location of the
   * maximum correlation.
   * @param index Index of the images to use (fixed, moving).
   * @param [out] translationX Computed intial x-translation in [mm].
   *    NOTE: This is a pure in-plane 2D translation along the pixel direction,
   *    which does not consider the image orientation.
   * @param [out] translationY Computed intial y-translation [mm].
   *    NOTE: This is a pure in-plane 2D translation along the pixel direction,
   *    which does not consider the image orientation.
   * @param doMovingUnsharpMasking If TRUE the moving image is pre-processed with
   *    unsharp masking.
   * @param fastSearch If TRUE a fast pre-registration sheme based on
   *    downsampling the original images (X-ray, DRR) to a spacing of 2mm.
   *    Furthermore the configured patch region is extended by 40 mm.
   * @param radiusSearch If TRUE a region based search around the projected volume
   *     center or structure center is used. The parameters <x->,<x+>,<y->,<y+>
   *     (see below) are then used as search radius around the configured patch region.
   *     This mode is mask independent, hence the mask is not used to define the search radius.
   * @param args Contains the arguments to set the position and size of the
   *    used fixed and moving image regions. NOTE: Must have 8 values.
   *    If no args are provided the default values are used:
   *    Default: {x-, x+, y-, y+, xoff, yoff, w, h} = {-30, 30, -30, 30, -30, -30, 61, 61}
   *    - The first 4 values define additional margins of the fixed image
   *      region, which is extracted from the mask image (if available).
   *      The {x-, x+, y-, y+} margin offsets can be specified in [mm].
   *    - The 5th to 8th value define the x and y offset and width/height
   *      of the rectangular moving image region relative to the projected
   *      volume center in [mm]. Note: the width and height are rounded down to
   *      the next odd number.
   *    NOTE: if the resultant regions are outside the largest possible DRR
   *    image region, the region is auto-cropped in the respective direction(s)!
   * @param structureUID If a structure-uid is provided the center of the
   *     structure is projected onto the image plane and used to place the DRR
   *     patch. If no valid UID is provided the extracted patch is placed
   *     relative to the projected CT volume center. Moreover, structureUID can
   *     be an explicit WCS point specification using the following string-
   *     convention: "(<x>|<y>|<z>)" where <x>,<y>,<z> are the absolute point
   *     coordinates in WCS.
   * @param itkCommand If set this command will be called when an itk::ProgressEvent()
   *    occurs during cross correlation computation.
   * @return TRUE if transformation computation was successful and FALSE if an
   *    error occurred.
   */
  template<typename TVolumeComponentType>
  bool ComputeCrossCorrelationInitialTransform(const unsigned int &index,
      double &translationX, double &translationY, const bool &doMovingUnsharpMasking,
      const bool &fastSearch, const bool &radiusSearch,
      std::vector<double> &args, const std::string &structureUID = "",
      itk::Command *itkCommand = NULL);

  /** Compute the resultant 3D transformation parameters from a specified
   * in-plane translation (in the fixed image coordinate system). The
   * translation is specified in terms of pixels and back-projected to the
   * center of rotation.
   * @param index zero-based index of the according fixed image
   * @param dx translation along the 1st image dimension in PIXELS
   * @param dy translation along the 2nd image dimension in PIXELS
   * @return the resultant 3D transformation parameters (certainly added to the
   * current parameters) if successful; in case of failure a zero-length
   * parameter vector is returned; NOTE: This method DOES NOT APPLY the computed
   * parameters - this must be achieved using the
   * OverrideAndApplyCurrentParameters() later!
   * @see OverrideAndApplyCurrentParameters()
   *  **/
  ParametersType ComputeTransformationFromPixelInPlaneTranslation(
      unsigned int index, int dx, int dy);
  /** Compute the resultant 3D transformation parameters from a specified
   * in-plane translation (in the fixed image coordinate system). The
   * translation is specified in terms of physical units (mm) and back-projected
   * to the center of rotation.
   * @param index zero-based index of the according fixed image
   * @param dx translation along the 1st image dimension in PHYSICAL UNITS (MM)
   * @param dy translation along the 2nd image dimension in PHYSICAL UNITS (MM)
   * @return the resultant 3D transformation parameters (certainly added to the
   * current parameters) if successful; in case of failure a zero-length
   * parameter vector is returned; NOTE: This method DOES NOT APPLY the computed
   * parameters - this must be achieved using the
   * OverrideAndApplyCurrentParameters() later!
   * @see OverrideAndApplyCurrentParameters()
   *  **/
  ParametersType ComputeTransformationFromPhysicalInPlaneTranslation(
      unsigned int index, double dx, double dy);

  /** Set flag indicating whether sparse pre-registration should be executed the
   * next time. **/
  void SetExecuteSparsePreRegistration(bool value)
  {
    m_ExecuteSparsePreRegistration = value;
  }
  /** Get flag indicating whether sparse pre-registration should be executed the
   * next time. **/
  bool GetExecuteSparsePreRegistration()
  {
    return m_ExecuteSparsePreRegistration;
  }

  /** @return whether auto-registration is actually running **/
  bool GetRegistrationIsRunning()
  {
    return m_RegistrationIsRunning;
  }

  /** If the "intelligent mask mode" is ON, this method tries to load previously
   * generated and stored 2D masks instead of re-generating them (faster!).
   * However, this is only possible if certain criteria are met! For example,
   * the image geometry must fit.
   * @return TRUE if ALL masks could be loaded and do not need to be
   * re-generated **/
  bool LoadIntelligentMasks();

  /** Write the specified (generated) mask and the according info to the fixed
   * image folder of the indexed fixed image if "intelligent mask mode" is ON.
   * @param index 1-based fixed image index
   * @param finalMask mask to be written (not NULL)
   * @return true if successful
   */
  bool WriteIntelligentMaskAndInfo(std::size_t index, ITKVTKImage *finalMask);

protected:
  /** Intensity transfer function definition type. **/
  typedef vtkSmartPointer<vtkColorTransferFunction> ITFPointer;

  /** Give some tasks exclusive access to all components of this model.
   * @see REG23RegistrationInitializationTask
   * @see REG23RegistrationExecutionTask
   * @see REG23SparseRegistrationExecutionTask
   **/
  friend class REG23RegistrationInitializationTask;
  friend class REG23RegistrationExecutionTask;
  friend class REG23SparseRegistrationExecutionTask;

  /** Flag indicating whether last config-load-attempt was successful **/
  bool m_ValidConfiguration;
  /** Config file name **/
  std::string m_ConfigFile;
  /** Last loaded configuration **/
  IniAccess *m_Config;
  /** Buffered SiteInfo file object **/
  IniAccess *m_SiteInfo;
  /** Optional ITF pool file **/
  IniAccess *m_ITFPool;
  /** Buffered ORA patient UID **/
  std::string m_PatientUID;
  /** Buffered iso-center position **/
  double *m_IsoCenter;
  /** The core registration framework: NReg2D/3D. **/
  GenericRegistrationPointer m_NReg;
  /** Wrapped volume image (CT image) **/
  ImageConsumer *m_Volume;
  /** Wrapped pre-processed volume image **/
  ImageConsumer *m_PPVolume;
  /** Wrapped fixed images (X-ray images) **/
  std::vector<ImageConsumer *> m_FixedImages;
  /** Wrapped pre-processed fixed images **/
  std::vector<ImageConsumer *> m_PPFixedImages;
  /** Wrapped mask images (size equals that of m_FixedImages) **/
  std::vector<ImageConsumer *> m_MaskImages;
  /** Wrapped post-processed fixed images (size equals that of m_FixedImages,
   * but can contain NULL when no post-processing is applied to the
   * pre-processed image) **/
  std::vector<ImageConsumer *> m_PPFixedImagesPost;
  /** Source positions **/
  std::vector<double *> m_SourcePositions;
  /** Source positions gantry offset angles in degrees **/
  std::vector<double> m_GantryOffsetAngle;
  /** Explicit initial window/level pairs (optional, NULL if not specified) **/
  std::vector<double *> m_ExplicitInitialWLs;
  /** A synthetic structure set that holds the ORA structures **/
  VTKStructureSet *m_Structures;
  /** Optional application command line arguments (to support "FROM-COMMAND-LINE"
   * entries in image file specifications).
   **/
  std::vector<std::string> m_CommandLineArguments;
  /** Index of the last option entry in the command line arguments (in order to
   * be able to parse the file names after the options).
   */
  int m_LastCommandLineOptionIndex;
  /** Volume file name retrieved from config file / command line **/
  std::string m_VolumeImageFileName;
  /** Fixed image file names retrieved from config file / command line **/
  std::vector<std::string> m_FixedImageFileNames;
  /** View names according to fixed image file names **/
  std::vector<std::string> m_ViewNames;
  /** Primary task manager (for easier access) **/
  TaskManager *m_TaskManager;
  /** An external tool render window for internal rendering tasks (helper) **/
  vtkRenderWindow *m_ToolRenderWindow;
  /** An external DRR tool render window for internal rendering **/
  vtkRenderWindow *m_DRRToolRenderWindow;
  /**
   * Current transform parameters (from manual or automatic
   * registration). NOTE: These parameters do not necessarily equal the
   * current optimization parameters!
   * @see UpdateCurrentRegistrationParameters()
   **/
  ParametersType m_CurrentParameters;
  /** Reference transform parameters (scientific mode). These is a reference
   * transform where the final transform is compared to. **/
  ParametersType m_ReferenceParameters;
  /** Initial transform parameters (from config file). **/
  ParametersType m_InitialParameters;
  /**
   * Current optimization parameters (from automatic registration). NOTE: These
   * parameters represent the current optimization position which may depend
   * on the optimization strategy - not necessarily equal to transform pars!
   * @see UpdateCurrentRegistrationParameters()
   **/
  ParametersType m_CurrentOptimizationParameters;
  /** Stored number of total (planned) optimization iterations **/
  int m_NumberOfIterations;
  /**
   * Current optimization iteration. This measure must be manually updated from
   * the external optimization callback. Use IncrementIteration().
   * @see IncrementIteration()
   **/
  int m_CurrentIteration;
  /**
   * Current optimization measure value. This measure represents the current
   * optimizer value, and not necessarily the last metric value.
   * @see UpdateCurrentRegistrationParameters()
   **/
  double m_CurrentOptimizationMeasure;
  /** Internal helper for storing the best optimization measure **/
  double m_BestMeasure;
  /**
   * Last composite metric value (not necessarily the last optimization measure).
   * @see UpdateCurrentRegistrationParameters()
   */
  double m_LastMetricMeasure;
  /** Internal 3D/3D transform (EULER) **/
  TransformPointer m_Transform;
  /** Command that is attached to the optimizer IterationEvent **/
  CommandPointer m_OptimizerCommand;
  /**
   * Command that is attached to the registration events (StartEvent,
   * StartMultiResolutionLevelEvent, StartOptimizationEvent, EndEvent)
   **/
  CommandPointer m_RegistrationCommand;
  /** Flag indicating whether registration should be started automatically **/
  bool m_StartRegistrationAutomatically;
  /** Flag indicating that registration is running **/
  bool m_RegistrationIsRunning;
  /** Type ID of current optimizer **/
  std::string m_CurrentOptimizerTypeID;
  /** Buffered flag indicating whether cost function should be rendered **/
  bool m_RenderCostFunction;
  /** Vectors containing current cost function and parameter values as points
   * (basically valid if m_RenderCostFunction==TRUE). The first vector contains
   * the cost function x/y-pairs, further vectors contain the x/y-pairs of
   * the transformation parameters. **/
  QVector<QVector<QPointF> > m_FunctionValues;
  /** Cost function protector. **/
  QMutex m_FunctionValuesMutex;
  /** Flag indicating that the DRRs should be updates as soon as the cost
   * function value gets better (smaller) **/
  bool m_UpdateImagesAsSoonAsBetter;
  /** Modulo value for DRR update (if <=1, each DRR is updated) **/
  int m_UpdateImagesModulo;
  /** Maximum frame rate for image update (if <=0, no limit) **/
  double m_UpdateImagesMaxFrameRate;
  /** Time stamp of last overlay image rendering **/
  double m_LastUpdateImagesTime;
  /** An image update is pending (due to max. frame rate) **/
  bool m_UpdateImagesPending;
  /** Moving image locker. **/
  QMutex m_CurrentMovingImageMutex;
  /** Current moving images (DRRs) in VTK-format. **/
  std::vector<ITKVTKImage *> m_CurrentMovingImages;
  /** Fixed image locker. **/
  QMutex m_FixedImageMutex;
  /** Volume image locker. **/
  QMutex m_VolumeImageMutex;
  /** Mask image locker. **/
  QMutex m_MaskImageMutex;
  /** Optional output (result) transformation file name. If specified, the
   * SaveTransformToFile()-method will write the desired transform file.
   * @see SaveTransformToFile()
   **/
  std::string m_OutputTransformFile;
  /** Optional configured graphics style (Qt widgets and windows) **/
  std::string m_ConfiguredGraphicsStyle;
  /** Transform center of rotation for Euler 3D transform **/
  TransformType::InputPointType m_CenterOfRotation;
  /** Global rendering context coordinator **/
  static QMutex m_RenderCoordinator;
  /** Internal data member for window/level min and max**/
  double m_CurrentWindowLevelMinMax[2];
  /** Flag indicating whether or not the transformation help widgets are
   * initially visible **/
  bool m_TransformationWidgetsVisible;
  /** Flag indicating whether or not the nature of transformation is
   * automatically determined based on the initial left mouse click **/
  bool m_RegionSensitiveTransformNature;
  /** Flag indicating whether or not the "display masks" option is initially
   * set **/
  bool m_DisplayMasks;
  /** Color of the displayed mask contours in HSV color space **/
  double m_MasksColorHSV[3];
  /** Time stamp measuring application start time (more or less) **/
  std::string m_ModelCreationTimeStamp;
  /** Undo/redo manager for transforms **/
  SimpleTransformUndoRedoManager *m_UndoRedoManager;
  /** Scientific mode flag **/
  bool m_ScientificMode;
  /** Intelligent mask mode flag (load/store masks for faster recovery) **/
  bool m_IntelligentMaskMode;
  /** Flag indicating that real-time-adaptive windowing should be activated. **/
  bool m_RealTimeAdaptiveWindowing;
  /** Factor determining windowing mouse sensitivity (<1.0 less sensitive,>1.0
   * more sensitive). **/
  double m_WindowingSensitivity;
  /** Flag indicating whether or not a safety question dialog is shown if the
   * user presses the cancel ("decline") button instead of the OK ("accept")
   * button. **/
  bool m_ShowWarningOnCancel;
  /** Flag indicating whether or not a safety warning dialog is shown if the
   * fixed images have acquisition date/times that diverge more than
   * m_MaxFixedImageAcquisitionDivergingTimeMin. **/
  bool m_ShowWarningOnDivergingFixedImageAcquisitionTimes;
  /** A time value (in fractions of a minute) that defines the maximum fixed image
   * acquisition time difference over all fixed images **/
  double m_MaxFixedImageAcquisitionDivergingTimeMin;
  /** Vector containing unsharp masking enabled state. **/
  std::vector<bool> m_UnsharpMaskingEnabled;
  /** Vector containing unsharp masking activation state. **/
  std::vector<bool> m_UnsharpMaskingActivated;
  /** Mode flag indicating whether or not the current window/level settings
   * should be stored in the respective fixed images' source folder (in a file
   * named reg23-wl.inf). 0..do not store, 1..store **/
  int m_WindowLevelStorageStrategy;
  /** Mode flag indicating whether or not the window/level settings
   * should be recovered from the respective fixed images' source folder (from a
   * file named reg23-wl.inf). 0..do not recover (apply window/level as
   * defined in config file), 1..recover from file (if existing and valid,
   * otherwise window/level applied as defined in config file) **/
  int m_WindowLevelRecoveryStrategy;
  /** (Science mode) ITF optimizer config strings **/
  std::vector<std::string> m_ITFOptimizerConfigStrings;
  /** Flag indicating whether sparse pre-registration should be executed the
   * next time. **/
  bool m_ExecuteSparsePreRegistration;
  /** Internal ID of selected sparse pre-registration type **/
  std::string m_SparsePreRegistrationType;

  /**
   * Pre-process a specified ITK-based image:<br>
   * - optionally crop it <br>
   * - optionally resample it <br>
   * - optionally rescale it <br>
   * - optionally cast it to another component type<br>
   * - optionally flip its image data
   * - optionally threshold its image data.<br>
   * This pre-processing directly relates to the loaded configuration specified
   * by configBaseKey parameter.
   * The dimension is statically presumed being 3.
   * <br>NOTE: This method is templated over the image's component type!
   * @param image the image to be processed (casted ITK data object pointer)
   * @param configBaseKey the primary key in the loaded configuration file (in
   * "Images" section, e.g. "Volume" for the volume image or "FixedImage3" for
   * the third fixed image)
   * @param error returned flag that indicates whether an error occurred
   * @param metaInfo image meta information - this information object is passed
   * through during pre-processing, but NOTE: the information is not updated! it
   * is likely that the metric-information (spacing ...) is not up-to-date after
   * pre-processing!!!
   * @return the pre-processed image as new ITK/VTK image instance if the
   * operation was successful, return NULL if it failed (error==true) or no
   * pre-processing was configured (error==false)
   **/
  template<typename TComponentType>
  ITKVTKImage *PreProcessImage(ITKVTKImage::ITKImagePointer image,
      std::string configBaseKey, bool &error,
      ITKVTKImageMetaInformation::Pointer metaInfo);

  /**
   * Post-process a specified ITK-based image:<br>
   * - optionally crop it <br>
   * - optionally cast it to another component type.<br>
   * This post-processing directly relates to the loaded configuration specified
   * by configBaseKey parameter.
   * The dimension is statically presumed being 3.
   * <br>NOTE: This method is templated over the image's component type!
   * @param image the image to be processed (casted ITK data object pointer)
   * @param imageMask the (optional) mask-image used for processing.
   *    NOTE: Used at RESCALEMINMAXMASK (if not NULL).
   *    Must have the same size as the \a image. If the mask is empty
   *    (no pixel != 0) NULL is returned at RESCALEMINMAXMASK.
   * @param configBaseKey the primary key in the loaded configuration file (in
   * "Images" section, e.g. "Volume" for the volume image or "FixedImage3" for
   * the third fixed image)
   * @param error returned flag that indicates whether an error occurred
   * @param metaInfo image meta information - this information object is passed
   * through during post-processing, but NOTE: the information is not updated! it
   * is likely that the metric-information (spacing ...) is not up-to-date after
   * post-processing!!!
   * @return the pre-processed image as new ITK/VTK image instance if the
   * operation was successful, return NULL if it failed (error==true) or no
   * post-processing was configured (error==false)
   **/
  template<typename TComponentType>
  ITKVTKImage *PostProcessImage(ITKVTKImage::ITKImagePointer image,
      std::string configBaseKey, bool &error, ITKVTKImage *imageMask,
      ITKVTKImageMetaInformation::Pointer metaInfo);

  /**
   * Cast the specified image according to the arguments from config file (trust
   * these parsed arguments!).
   * @return a new ITKVTKImage instance containing the pointer to the result
   * image if successful, NULL otherwise
   * @see PreProcessImage()
   */
  template<typename TComponentType>
  ITKVTKImage *CastImage(ITKVTKImage *image, std::vector<std::string> &args);

  /**
   * Crop the specified image according to the arguments from config file (trust
   * these parsed arguments!).
   * @return a new ITKVTKImage instance containing the pointer to the result
   * image if successful, NULL otherwise
   * @see PreProcessImage()
   */
  template<typename TComponentType>
  ITKVTKImage *CropImage(ITKVTKImage *image, std::vector<std::string> &args);

  /**
   * Resample the specified image according to the arguments from config file
   * (trust these parsed arguments!).
   * NOTE: The size is limited to be greater than zero (>= 1).
   * @return a new ITKVTKImage instance containing the pointer to the result
   * image if successful, NULL otherwise
   * @see PreProcessImage()
   */
  template<typename TComponentType>
  ITKVTKImage *ResampleImage(ITKVTKImage *image, std::vector<std::string> &args);

  /**
   * Rescale the specified image according to the arguments from config file
   * (trust these parsed arguments!).
   * @param rescaleMode 0 ... min/max rescaling, 1 ... shift/scale rescaling,
   * 2 ... windowing rescaling, 3 ... min/max rescaling with masking
   * @param imageMask Required and used for \a rescaleMode 3.
   * @return a new ITKVTKImage instance containing the pointer to the result
   * image if successful, NULL otherwise.
   * @see PreProcessImage()
   * @see PostProcessImage()
   */
  template<typename TComponentType>
  ITKVTKImage *RescaleImage(ITKVTKImage *image, std::vector<std::string> &args,
      int rescaleMode, ITKVTKImage *imageMask = NULL);

  /** Apply a flip to the image data. The image geometry is preserved (origin,
   * direction).
   * @param image The input image to flip.
   * @param args Argument vector with at least 2 elements. The second element
   * determines the flip direction ("X", "Y").
   * @return a new ITKVTKImage instance containing the pointer to the result
   * image if successful, NULL otherwise
   * @see PreProcessImage()
   */
  template<typename TComponentType>
  ITKVTKImage *FlipImageData(ITKVTKImage *image, std::vector<std::string> &args);

  /** Threshold the specified image data. Three types of thresholding are
   * available: below, above, outside.
   * @param image The input image to flip.
   * @param args Argument vector with 2 or 3 elements
   * @return a new ITKVTKImage instance containing the pointer to the result
   * image if successful, NULL otherwise
   * @see PreProcessImage()
   */
  template<typename TComponentType>
  ITKVTKImage *ThresholdImageData(ITKVTKImage *image, std::vector<std::string> &args);

  /**
   * Apply unsharp masking to the specified image according to the arguments
   * from config file (trust these parsed arguments!).
   * @return a new ITKVTKImage instance containing the pointer to the result
   * image if successful, NULL otherwise.
   * @see PreProcessImage()
   * @see PostProcessImage()
   */
  template<typename TComponentType>
  ITKVTKImage *UnsharpMaskImage(ITKVTKImage *image,
      std::vector<std::string> &args);

  /** Generates a volume mask based on a provided list of (closed) polydata-objects.
   * Voxels inside the polydata-objects are "outside" (value of zero) of the mask,
   * and voxels outside any polydata-object are "inside" (value 255).
   * NOTE: The returned image is of type unsigned char.
   * NOTE: If more than 4 bounds of a polydata object are outside of the
   * volume bounds a segfault occurs in vtkImageStencil (NULL is returned)!
   * @param volume A dataset to generate a volumetric mask image.
   * @param maskstructures A list of (closed) polydata-objects that define
   * which values of the mask are "outside" (inside of any polydata-object).
   * @return An mask-volume as ITKVTKImage of type unsigned char with the same
   * characteristics as the volume (direction, origin, spacing, size).
   */
  template<typename TVolumeComponentType>
  itk::Image<unsigned char, 3>::Pointer CreateVolumeMask(ITKVTKImage *volume,
      std::vector<vtkSmartPointer<vtkPolyData> > &maskstructures);

  /**
   * Load the i-th prepared structure (dummy instance) from file. NOTE: In order
   * to a account for the iso-center, an additional transform is applied.
   * @param i index within m_Structures
   * @return TRUE if all prepared structures could be loaded successfully
   * @see m_Structures
   */
  bool LoadStructure(unsigned int i);

  /**
   * Look if the current source positions are fully defined. Determine the rest
   * of the source positions automatically if required.
   * @return TRUE if all source positions are now OK
   */
  bool FixAutoSourcePositions();

  /**
   * Generate an automatic binary mask image for the specified image index
   * and structure.
   * @param i index within m_PPFixedImages
   * @param structure structure to be projected
   * @return the generated mask (if successful), NULL otherwise
   * @see m_PPFixedImages
   */
  ITKVTKImage *GeneratePerspectiveProjectionMask(unsigned int i,
      VTKStructure *structure);

  /**
   * Generate a copy of the input mask image.
   * @param src the image to be copied (ITK image is copied and a dummy meta
   * information object is added)
   * @return the copy of the source image if specified, NULL otherwise
   */
  ITKVTKImage *CopyMaskImage(ITKVTKImage *src);
  /**
   * Generate an inverted copy of the input mask image.
   * @param src the image to be inverted
   * @return the inverted copy of the source image if specified, NULL otherwise
   */
  ITKVTKImage *InvertMaskImage(ITKVTKImage *src);
  /**
   * Generate a new mask by logically connecting the two input mask images with
   * an OR connection.
   * @param src1 the first image to be OR-connected
   * @param src2 the second image to be OR-connected
   * @return the result mask of both source images if specified, NULL otherwise
   */
  ITKVTKImage *ApplyORToMaskImages(ITKVTKImage *src1, ITKVTKImage *src2);
  /**
   * Generate a new mask by logically connecting the two input mask images with
   * an AND connection.
   * @param src1 the first image to be AND-connected
   * @param src2 the second image to be AND-connected
   * @return the result mask of both source images if specified, NULL otherwise
   */
  ITKVTKImage *ApplyANDToMaskImages(ITKVTKImage *src1, ITKVTKImage *src2);
  /**
   * Generate a new mask by logically connecting the two input mask images with
   * an XOR connection.
   * @param src1 the first image to be XOR-connected
   * @param src2 the second image to be XOR-connected
   * @return the result mask of both source images if specified, NULL otherwise
   */
  ITKVTKImage *ApplyXORToMaskImages(ITKVTKImage *src1, ITKVTKImage *src2);

  /** Fills in holes and cavities by applying an iterative voting operation
   * until no pixels are being changed or until it reaches the maximum number
   * of iterations.
   * It fills in holes of medium size (tens of pixels in radius). The number of
   * iterations is related to the size of the holes to be filled in. The larger
   * the holes, the more iterations must be run in order to fill in the full hole.
   * The size of the neighborhood is related to the curvature of the hole borders
   * and therefore the hole size.
   * Note: It may also fill in cavities in the external side of binary images.
   * @param src The input image for hole filling.
   * @param radiusX Radius in x-dimension of the neighborhood used to compute the median.
   *    The value on each dimension is used as the semi-size of a rectangular
   *    box (e.g. in 2D a size of (1,2) will result in a 3x5 neighborhood.
   * @param radiusY Radius in y-dimension of the neighborhood used to compute the median.
   * @param invert Since a binary image is expected as input, the levels that
   *    are going to be considered background and foreground are defined as
   *    0 and 255. If \a invert is TRUE these values are swapped and the effect
   *    is an island removal of the foreground.
   * @param majorityThreshold Defines the number of pixels over 50% that will
   *    decide whether an OFF pixel will become ON or not (e.g. a pixel has a
   *    neighborhood of 100 pixels (excluding itself), the 50% will be 50;
   *    a majority threshold of 5 means that 67 or more neighbor pixels are
   *    required to be ON in order to switch the current OFF pixel to ON).
   *    Summed up a background pixel will be converted into a foreground pixel
   *    if the number of foreground neighbors surpass the number of background
   *    neighbors by the majority value: (width * height - 1)/2 + majority.
   * @param numberOfIterations Maximum number of iterations to perform. It is
   *    executed iteratively as long as at least one pixel has changed in a
   *    previous iteration, or until the specified number of iterations has been reached.
   *    The number of iterations will determine the maximum size of holes and
   *    cavities that this filter will be able to fill-in. The more iterations,
   *    the larger the cavities that will be filled in.
   * @return The hole filled/island removed input image if no error occurs, else NULL.
   * @see itk::VotingBinaryIterativeHoleFillingImageFilter
   */
  ITKVTKImage *HoleFillMaskImage(ITKVTKImage *src,
      const unsigned int radiusX, const unsigned int radiusY, const bool invert,
      const unsigned int majorityThreshold, const unsigned int numberOfIterations) const;

  /** Adds a centered rectangular mask to the input mask \a src with the
   * specified \a width and \a height in [mm].
   * @param src  The input image to add a rectangular mask.
   * @param maskWidth The width of the mask in [mm].
   * @param maskHeight The height of the mask in [mm].
   * @return The rectangular masked input image if no error occurs, else NULL.
   */
  ITKVTKImage *DrawCenteredRectangleToMaskImage(ITKVTKImage *src,
      const double maskWidth, const double maskHeight);

  /**
   * Initialize internal components. Apply configuration, instantiate the
   * registration components and tie them together. After successfully calling
   * this method, the NReg2D/3D core should be ready to run.
   * <br>NOTE: This method is templated over the volume's component type!
   * @return true if the initialization was successful
   **/
  template<typename TVolumeComponentType>
  bool InitializeInternal();

  /**
   * Instantiate the configured metrics and apply also the masks to them.
   * @param nreg this method rather accesses nreg than m_NReg
   * @return true if successful
   **/
  template<typename TVolumeComponentType>
  bool InstantiateMetrics(GenericRegistrationPointer nreg);

  /**
   * Instantiate the configured optimizer and configure it.
   * @param nreg this method rather accesses nreg than m_NReg
   * @return true if successful
   **/
  template<typename TVolumeComponentType>
  bool InstantiateOptimizer(GenericRegistrationPointer nreg);

  /**
   * Deinitialize internal registration core.
   * <br>NOTE: This method is templated over the volume's component type!
   * @return true if the deinitialization was successful
   **/
  template<typename TVolumeComponentType>
  bool DeinitializeInternal();

  /**
   * Parse a specified image pre/post-processing entry of the configuration and
   * return the parsed arguments as string-vector. The supported entry-types
   * and associated notation are documented in the exemplary config file in
   * the source folder!
   * @param entry the config entry to be parsed
   * @param errorMessage return the error message (if FALSE is returned)
   * @param result parsed arguments in string-representation (first element is
   * the operation ID, the following ones are the arguments)
   * @param operationFlags Indicates what operations should be parsed.
   * @return TRUE if the configuration entry is OK
   */
  bool ParseImageProcessingEntry(std::string entry,
      std::string &errorMessage, std::vector<std::string> &result,
      const int operationFlags) const;

  /**
   * Parse a specified ITF entry from configuration file and return the
   * converted ITF in appropriate representation.
   * @param entry the config entry to be parsed
   * @param errorMessage return the error message (if NULL is returned)
   * @return ITF if successful, NULL otherwise
   */
  ITFPointer ParseITF(std::string entry, std::string &errorMessage);

  /**
   * Parse a specified registration metric entry set. Analyze the entries and
   * return them.
   * @param typeEntry metric type entry to be parsed
   * @param configEntry metric config entry to be parsed
   * @param prefixEntry metric rule prefix entry to be parsed
   * @param postfixEntry metric rule postfix entry to be parsed
   * @param args returned analyzed parts if successful (first item is the
   * metric type ID; the last two items represent optional pre- and postfix)
   * @param errorMessage return the error message (if FALSE is returned)
   * @return TRUE if successful
   */
  bool ParseMetricEntry(std::string typeEntry, std::string configEntry,
      std::string prefixEntry, std::string postfixEntry,
      std::vector<std::string> &args, std::string &errorMessage);

  /**
   * Parse a specified registration optimizer entry set. Analyze the entries
   * and return them.
   * @param typeEntry optimizer type entry to be parsed
   * @param configEntry optimizer config entry to be parsed
   * @param scalesEntry optimizer scales entry to be parsed
   * @param args returned analyzed parts if successful (first item is the
   * optimizer type ID; the last 6 items represent optional scales)
   * @param errorMessage return the error message (if FALSE is returned)
   * @return TRUE if successful
   */
  bool ParseOptimizerEntry(std::string typeEntry, std::string configEntry,
      std::string scalesEntry, std::vector<std::string> &args,
      std::string &errorMessage);

  /**
   * Execute N-way 2D/3D registration. Certainly, the framework
   * must exist and be ready for usage.
   * @return TRUE if successful
   **/
  template<typename TVolumeComponentType>
  bool ExecuteRegistration();

  /**
   * Updates some of the current registration parameters during automatic
   * registration:
   * m_CurrentParameters, m_CurrentOptimizationParameters, m_CurrentMeasure.
   * This method should be called from the registration and the optimization
   * callbacks!
   * @param stopRegistration if this flag is set to TRUE, the registration
   * will stop as soon as possible
   * @param forceImagesUpdate force image updates (ignore modulo / best value)
   **/
  template<typename TVolumeComponentType>
  void UpdateCurrentRegistrationParameters(bool stopRegistration,
      bool forceImagesUpdate = false);

  /** Increment the current iteration number from an external callback. **/
  void IncrementIteration();

  /** Initialize function table. **/
  void InitFunctionTable();

  /** Extract mean and variance from the specified image (works for single-
   * channel images).
   * @param image the image to be analyzed (FLOAT pixel type expected)
   * @param mean returned mean of intensities
   * @param variance returned variance of intensities
   * @return TRUE if mean and variance could successfully be derived **/
  bool ExtractMeanAndVarianceFromImage(vtkImageData *image, double &mean,
      double &variance);

  /**
   * Analyze the specified metrics' configuration items, extract the initial
   * moving images and properties if necessary, and re-configure the metrics.
   **/
  template<typename TVolumeComponentType>
  void InitializeDynamicMetricProperties();

  /**
   * Configure the specified metric with the properties specified in the
   * arguments vector.
   * @param metric metric to be configured
   * @param margs the arguments in string-representation - 1st item is type!
   * @param prefix returned optional prefix
   * @param postfix returned optional postfix
   */
  template<typename TVolumeComponentType>
  void ConfigureMetricComponent(itk::Object::Pointer metric,
      std::vector<std::string> &margs, std::string &prefix, std::string &postfix);

  /**
   * Parses an entry for generic variable expressions and returns the found
   * occurrences (structure UIDs and attributes) in vector form. A generic
   * variable is of the form "v{<struct-uid>.<attrib>}" where <struct-uid> is
   * the structure UID (must exist!) or "VOLUME" and <attrib> is one of the
   * following:
   * xmin,xmax,ymin,ymax,zmin,zmax,w,h,d,cells,points.
   * @param expr the expression to be parsed
   * @param structuids the structure UIDs or "VOLUME" of the found generic variables
   * @param attribus the attributes of the found generic variables
   * @param currentStructureUID optionally provide the structure UID of the
   * current structure which is currently processed but not added to m_Structures
   * @return TRUE if expr is valid and the returned vectors can be believed
   */
  bool ParseGenericVariables(std::string expr,
      std::vector<std::string> &structuids,
      std::vector<std::string> &attribs, std::string currentStructUID = "");
  /**
   * Get the value of a specified generic variable. This function should only
   * be called if it is sure that structuid.attrib exists!
   * @param structuid the structure UID or "VOLUME"
   * @param attrib the attribute descriptor
   * @param currentStructureUID optionally provide the structure UID of the
   * current structure which is currently processed but not added to m_Structures
   * @param currentPD optionally provide the poly data according to
   * currentStructureUID
   * @return the variable value
   */
  double GetGenericVariableValue(std::string structuid, std::string attrib,
      std::string currentStructUID = "", vtkPolyData *currentPD = NULL);

  /**
   * Parse and optionally process a specified structure entry set. Analyze the
   * entries and return the result of the analysis.
   * @param index 1-based (continuous) integer index of structure entry
   * @param simulation flag indicating whether the entry should be processed
   * (i.e. loaded and processed) or just simulated (checked); moreover, if
   * simulation==TRUE, the parsed structures are added as dummy-entries to
   * m_Structures in order to support the FindStructure()-method!
   * @param errorKey return the error key (if FALSE is returned)
   * @param errorMessage return the error message (if FALSE is returned)
   * @return TRUE if successful
   */
  bool ParseAndProcessStructureEntry(int index, bool simulation,
      std::string &errorKey, std::string &errorMessage);

  /**
   * Parse and optionally process a specified mask entry set. Analyze the
   * entries and return the result of the analysis.
   * @param index 1-based (continuous) integer index of mask entry
   * @param simulation flag indicating whether the entry should be processed
   * (i.e. really processed) or just simulated (checked);
   * @param errorKey return the error key (if FALSE is returned)
   * @param errorMessage return the error message (if FALSE is returned)
   * @param finalMask returned result mask (if simulation==TRUE, otherwise
   * NULL will be returned)
   * @return TRUE if successful
   */
  bool ParseAndProcessMaskEntry(int index, bool simulation,
      std::string &errorKey, std::string &errorMessage, ITKVTKImage *&finalMask);

  /**
   * Parse and store and ORA setup error entry.
   * @param entries a list of ORA (Study)SetupError-entries
   * @param errorMessage return the error message (if FALSE is returned)
   * @return TRUE if the setup error could be successfully extracted and stored
   */
  bool ParseORASetupErrors(std::vector<std::string> &entries,
      std::string &errorMessage);

  /** Parse the sparse pre-registration configuration entries and check their
   * validity. */
  bool ParseSparsePreRegistrationEntries(std::string section,
      std::string &errorKey, std::string &errorMessage);

  /** Parse the initial transform settings from rotation/translation-based
   * initialization and store them in the model. **/
  bool ParseInitialRotationTranslationTransform(std::string section,
      std::string &errorKey, std::string &errorMessage);

};

}

#include "oraREG23Model.txx"

#endif /* ORAREG23MODEL_H_ */
