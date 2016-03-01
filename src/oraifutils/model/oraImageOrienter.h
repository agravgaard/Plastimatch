

#ifndef ORAIMAGEORIENTER_H_
#define ORAIMAGEORIENTER_H_

// ORAIFTools
#include "SimpleDebugger.h"
#include "oraSimpleMacros.h"

// Forward declarations
namespace ora
{
class QProgressEmitter;
class ITKVTKImage;
}


namespace ora 
{


/** \class ImageOrienter
 * \brief Tool class for orienting ORA.XML images in world coordinate system.
 *
 * A tool class for transforming ORA.XML images from their native representation
 * (volume + frame of reference + beam isocenter) into an absolute position
 * and oriention in the world coordinate system.
 *
 * FIXME: the whole class must be adapted to the new pixel-independence strategy!
 *
 * @author phil 
 * @author Markus 
 * @version 1.1.1
 */
template <typename TPixelType>
class ImageOrienter : public SimpleDebugger
{
public:
  /** Standard typedefs **/
  typedef TPixelType PixelType;
  typedef ITKVTKImage ImageType;

  /** Default constructor. **/
  ImageOrienter();
  /** Default constructor. **/
  ~ImageOrienter();

  SimpleSetter(FramesInfo, std::string)
  SimpleGetter(FramesInfo, std::string)

  SimpleSetter(SetupErrorInfo, std::string)
  SimpleGetter(SetupErrorInfo, std::string)

  /**
   * Convert an ORA.XML volume into an absolute oriented volume in world
   * coordinate system. This method requires a set valid FramesInfo!
   * @param volumeFile input ORA.XML volume file
   * @param beamInfoFile BeamInfo specifying the isocenter where the generated
   * volume should relate to (this argument is allowed to be empty if and only
   * if the isocenter argument is defined)
   * @param outputFile the oriented volume that should be generated (e.g. a
   * MHD file - not an ORA.XML; NOTE: you can also use ".xmhd" extension for
   * extended and faster compression)
   * @param outputInfoFile optional file specification; if specified an info
   * file containing beam and isocenter information is created
   * @param progress an optional progress emitter which potentially emits
   * progress signals (scaled from 0.0 to 1.0)
   * @param isocenter optional pointer to a 3D-isocenter-array (same order of
   * coordinates as in beamInfoFile); if specified, the beamInfoFile argument
   * will be ignored and not outputInfoFile will be generated
   * @return TRUE if successful
   */
  bool ConvertVolume(std::string volumeFile, std::string beamInfoFile,
      std::string outputFile, std::string outputInfoFile = "",
      QProgressEmitter *progress = NULL, double *isocenter = NULL);

  /**
   * Convert an arbitrary image (ITK or ORA.XML) and orient it according to the
   * specified isocenter. NOTE: the set FOR info (m_FramesInfo) matters for
   * ORA.XML images!
   * @param volumeFile image file name
   * @param isocenter mandatory iso-center (typically that of an associated plan)
   * @return the image pointer in case of success, NULL otherwise
   */
  ImageType *ConvertVolumeObject(std::string volumeFile, double *isocenter);

  /**
   * Convert a set of kV images located in a typical ORA view folder, correct
   * the orientation and origin and store it to a specified output folder. In
   * addition a summary file (for TMR) is generated that explains the setup
   * errors of each individual files. This method requires a set valid
   * setup-errors file! NOTE: the summary file will contain the parameters for
   * a Yaw-Pitch-Roll (YPR-mode in TMR-script) and will assume a rotation center
   * of 0,0,0!!!
   * @param viewFolder folder that contains kV images (kV*.rti files); note that
   * the correction of the origin and orientation is solely based on the SAD,
   * SFD and metric properties of the corresponding *.inf file and not on the
   * image list (as these props are obviously buggy or relate to another
   * coordinate system)
   * @param outputFolder the directory where the converted kV images
   * (*.mhd files) should be stored to; in addition the summary (TMR) file will
   * be stored in this folder
   * @param outputInfoFile optional file specification; if specified an info
   * file will be generated that contains view information and an overview
   * of the image origins and orientations
   * @param progress an optional progress emitter which potentially emits
   * progress signals (scaled from 0.0 to 1.0)
   * @param configTemplate an optional specification of a TMR configuration
   * template; if specified some case-dependent variants of this configuration
   * will be concretized and produced in the base output folder (this might be
   * very useful!); NOTE: This method will perfectly work if you fulfill the
   * following prerequisites: <br>
   * (base-folder denotes a basic output folder for both CTs and kVs)<br>
   * 1. CTs have previously exported to sub-folders of basic output folder, e.g.
   * base-folder/CT_sparse and base-folder/CT_fine <br>
   * 2. the CT-images in these folders are named Volume.mhd <br>
   * 3. the kVs are generated in a second-order folder of base-folder, e.g.
   * base-folder/kVs/144 <br>
   * 4. (the generated TMR configuration files will be stored directly in
   * base-folder with speaking names)
   * @return TRUE if successful
   */
  bool ConvertKVImages(std::string viewFolder, std::string outputFolder,
      std::string outputInfoFile = "", QProgressEmitter *progress = NULL,
      std::string configTemplate = "");

  /**
   * Convert a single planar image from RTI format to ITK-format (e.g. *.xmhd).
   * @param sourceRTIFile input file name of RTI image
   * @param outputFile file name of image in ITK-format
   * @param useCompression compress the image raw data
   * @param writeOraXml additionally generated ORA-XML file
   * @param oraXmlAnonymized anonymize ORA-XML file
   * @return true if successfully converted
   */
  bool ConvertPlanarImage(std::string sourceRTIFile, std::string outputFile,
      bool useCompression, bool writeOraXml = false,
      bool oraXmlAnonymized = false);

  /**
   * Convert a single planar image from RTI format to ITK-format or directly
   * reads it from ITK format. Alternatively an ORA-XML file can be provided.
   * @param imageFile input file name of RTI or ITK or ORA-XML image
   * @return the image pointer in case of success, NULL otherwise
   */
  ImageType *ConvertPlanarImageObject(std::string imageFile);

protected:

  /** Helper structure for coping with minimal kV image information. **/
  typedef struct
  {
    double Origin[3];
    double Size[2];
    double Spacing[2];
    double SAD;
    double SFD;
    double Gantry;
    int NoHits;
    std::vector<std::string> *KVList;
  } MinmalKVGeometryType;

  /** Frames.inf reference **/
  std::string m_FramesInfo;
  /** SetupErrors.inf reference **/
  std::string m_SetupErrorInfo;

};


} 

#include "oraImageOrienter.txx"

#endif /* ORAIMAGEORIENTER_H_ */
