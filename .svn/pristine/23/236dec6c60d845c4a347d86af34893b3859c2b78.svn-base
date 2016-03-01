
#ifndef ORAITKVTKMETAINFORMATIONXMLFILE_H_
#define ORAITKVTKMETAINFORMATIONXMLFILE_H_

#include "oraITKVTKImageMetaInformation.h"

#include <itkXMLFile.h>


namespace ora 
{


/**
 * This class reads meta-information of open radART images from files
 * using an XML-based format.
 *
 * @author phil 
 * @version 1.2
 */
class ITKVTKMetaInformationXMLFileReader :
    public itk::XMLReader<ITKVTKImageMetaInformation>
{
public:
  /** standard typedefs */
  typedef ITKVTKMetaInformationXMLFileReader Self;
  typedef itk::XMLReader<ITKVTKImageMetaInformation> Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(ITKVTKMetaInformationXMLFileReader,
      itk::XMLReader<ITKVTKImageMetaInformation>);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /**
   * Set flag indicating whether invalid MD5-hashes in metainfo-files are
   * ignored (setting this flag is not recommended as it eventually
   * conveys corrupted files).
   **/
  itkSetMacro(IgnoreInvalidHash, bool);
  /**
   * Get flag indicating whether invalid MD5-hashes in metainfo-files are
   * ignored.
   **/
  itkGetMacro(IgnoreInvalidHash, bool);

  /**
   * Determine whether a file can be read.
   * NOTE: this method invokes MD5-based verification of the meta-file and,
   * therefore, guarantees high-level integrity. In addition, this
   * method pre-buffers the file-content and does not cause extra-loading
   * time (but it loads the complete XML-file into memory).
   */
  virtual int CanReadFile(const char* name);

  /** Provides a typical update-mechanism for the reader. **/
  virtual void Update();

protected:
  /** buffer for collecting the characters of the data handler **/
  std::ostringstream m_Data;
  /**
   * flag indicating whether invalid MD5-hashes in metainfo-files are
   * ignored (setting this flag is not recommended as it eventually
   * conveys corrupted files)
   **/
  bool m_IgnoreInvalidHash;
  /** temporary output object **/
  ITKVTKImageMetaInformation::Pointer m_TempOutput;
  /** temporary slice metainfo object **/
  SliceMetaInformation::Pointer m_TempSlice;
  /** original read file version **/
  std::string m_OriginalReadFileVersion;

  /** Default constructor **/
  ITKVTKMetaInformationXMLFileReader();
  /** Default destructor **/
  virtual ~ITKVTKMetaInformationXMLFileReader();

  /**
   * Callback function. Called from XML parser with start-of-element
   * information.
   * @see itk::XMLReaderBase::StartElement(const char *, char **)
   */
  virtual void StartElement(const char * name, const char **atts);

  /**
   * Callback function. Called from XML parser when ending tag encountered.
   * @see itk::XMLReaderBase::EndElement(const char *)
   */
  virtual void EndElement(const char *name);

  /**
   * Callback function. Called from XML parser with the character data for an
   * XML element.
   * @see itk::XMLReaderBase::CharacterDataHandler(const char *, int)
   */
  virtual void CharacterDataHandler(const char *inData, int inLength);

  /**
   * A new implementation of parse for pre-buffering the file content.
   * @see XMLReaderBase#parse()
   */
  void parse(void);

  /**
   * Convert an array of comma-separated elements a direction (matrix).
   * @param dir array of comma-separated elements where the columns are
   * marching fastest
   * @return the resultant direction matrix (zero-matrix if invalid)
   */
  VolumeMetaInformation::DirectionType ArrayToDirectionType(std::string dir);

  /**
   * Convert an array of comma-separated elements to a point.
   * @param point string of comma-separated point elements
   * @return point (zero-point if invalid)
   */
  VolumeMetaInformation::PointType ArrayToPointType(std::string point);
  /**
   * Convert an array of comma-separated elements to a fixed array.
   * @param farray string of comma-separated array elements
   * @return fixed array (zero-array if invalid)
   */
  VolumeMetaInformation::SpacingType ArrayToFixedArray(std::string farray);
  /**
   * Convert an array of comma-separated elements to a fixed array (int type).
   * @param farray string of comma-separated array elements
   * @return fixed array of int-type (zero-array if invalid)
   */
  VolumeMetaInformation::SizeType ArrayToFixedArrayIntType(std::string farray);
  /**
   * Convert an array of pipe-separated elements to a string array.
   * @param sarray string of pipe-separated array elements
   * @return string array (size=0 if invalid)
   */
  VolumeMetaInformation::StringArrayType ArrayToStringArrayType(
       std::string sarray);

private:
  // purposely not implemented
  ITKVTKMetaInformationXMLFileReader(const Self&);
  // purposely not implemented
  void operator=(const Self&);
};


/**
 * This class writes meta-information of open radART images into files
 * using an XML-based format.
 * @author phil 
 * @version 1.0
 */
class ITKVTKMetaInformationXMLFileWriter
  : public itk::XMLWriterBase<ITKVTKImageMetaInformation>
{
public:
  /** standard typedefs */
  typedef itk::XMLWriterBase<ITKVTKImageMetaInformation> Superclass;
  typedef ITKVTKMetaInformationXMLFileWriter Self;
  typedef itk::SmartPointer<Self> Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ITKVTKMetaInformationXMLFileWriter,
               itk::XMLWriterBase<ITKVTKImageMetaInformation>);

  /** Test whether a file is writable. */
  virtual int CanWriteFile(const char *name);

  /** Actually write out the file in question */
  virtual int WriteFile();

  /** Write out a start element tag (generic version for std::ostream). */
  void WriteStartElement(const char *const tag, std::ostream &os);

  /** Write an end element tag (generic version for std::ostream). */
  void WriteEndElement(const char *const tag, std::ostream &os);

  static std::string ComputeHashMD5(char *data, const unsigned int &datasize);

protected:
  /** Default constructor **/
  ITKVTKMetaInformationXMLFileWriter();
  /** Default destructor **/
  virtual ~ITKVTKMetaInformationXMLFileWriter();

  /**
   * Convert a direction (matrix) to an array of comma-separated elements.
   * @param dir direction matrix
   * @return string of comma-separated matrix elements where the columns are
   * marching fastest
   */
  std::string DirectionTypeToArray(VolumeMetaInformation::DirectionType dir);

  /**
   * Convert a point to an array of comma-separated elements.
   * @param point point
   * @return string of comma-separated point elements
   */
  std::string PointTypeToArray(VolumeMetaInformation::PointType point);
  /**
   * Convert a fixed array to an array of comma-separated elements.
   * @param array fixed array
   * @return string of comma-separated array elements
   */
  std::string FixedArrayToArray(VolumeMetaInformation::SpacingType array);
  /**
   * Convert a fixed array to an array of comma-separated elements.
   * @param array fixed array
   * @return string of comma-separated array elements
   */
  std::string FixedArrayToArray(VolumeMetaInformation::SizeType array);

  /**
   * Convert a string array to an array of pipe-separated elements.
   * @param array string array
   * @return string of pipe-separated array elements
   */
  std::string StringArrayTypeToArray(
      VolumeMetaInformation::StringArrayType array);

private:
  // purposely not implemented
  ITKVTKMetaInformationXMLFileWriter(const Self&);
  // purposely not implemented
  void operator=(const Self&);

};


}


#endif /* ORAITKVTKMETAINFORMATIONXMLFILE_H_ */
