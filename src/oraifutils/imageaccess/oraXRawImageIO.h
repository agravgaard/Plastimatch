//
#ifndef ORAXRAWIMAGEIO_H_
#define ORAXRAWIMAGEIO_H_

#include <itkRawImageIO.h>

namespace ora
{

/** \class XRawImageIO
 * \brief Extended RAW image IO for image reading and writing of RAW images.
 *
 * Extended RAW image IO compatible with ITK for image reading and writing.
 * The 'X' (eXtension) relates to its implemented zlib-compression capabilities.
 * This means that this class is capable of reading z-compressed RAW images
 * and writing z-compressed RAW images.
 *
 * Compression can be switched ON/OFF using the UseCompression property.
 *
 * Furthermore there is a AutoExtensionMode for both reading and writing images.
 * That is the type of image (raw or metaimage) and the compression are
 * automatically determined from the specified extension of the file name.
 * More precisely (case-insensitive):
 * Extension ".raw": a normal raw image is written or read <br>
 * Extension ".zraw": a compressed raw image is written or read <br>
 * Extension ".xraw": a fast-compressed raw image is written or read <br>
 * Extension ".mhd": a metaimage [.mhd] is read (uncompressed or compressed) and
 * a metaimage (metadata [.mhd] + uncompressed raw file [.raw]) is generated  <br>
 * Extension ".zmhd": a metaimage [.mhd] is read (uncompressed or compressed) and
 * a compressed metaimage (metadata [.mhd] + compressed raw file [.zraw]) is
 * generated  <br>
 * Extension ".xmhd": a metaimage [.mhd] is read (uncompressed or compressed) and
 * a fast-compressed metaimage (metadata [.mhd] + fast-compressed raw file
 * [.xraw]) is generated  <br>  <br>
 * However, if AutoExtensionMode is false, the IO relies on the predefined
 * settings!<br>
 * NOTE: the generated/read metadata files (.mhd) are really minimal supporting
 * the following entries: <br>
 * - ObjectType (always Image) <br>
 * - NDims <br>
 * - BinaryData (only Binary supported during import) <br>
 * - BinaryDataByteOrderMSB <br>
 * - CompressedData <br>
 * - CompressedDataSize (written, ignored during import) <br>
 * - TransformMatrix <br>
 * - Offset <br>
 * - CenterOfRotation (always 0 0 0; written, ignored during import) <br>
 * - AnatomicalOrientation (written, ignored during import) <br>
 * - ElementSpacing <br>
 * - DimSize <br>
 * - ElementType <br>
 * - ElementDataFile <br>
 *
 * NOTE: the compression capabilities are only available for BINARY FileType!
 *
 * \ingroup IOFilters
 *
 * @author phil 
 * @version 1.0
 */
template<class TPixel, unsigned int VImageDimension = 2>
class XRawImageIO: public itk::RawImageIO<TPixel, VImageDimension>
{
public:
  /** Standard class typedefs. */
  typedef XRawImageIO Self;
  typedef itk::RawImageIO<TPixel, VImageDimension> Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(XRawImageIO, RawImageIO);

  /**
   * Writes the data to disk from the memory buffer provided. NOTE: this method
   * supports z-compression.
   * @see UseCompression property
   **/
  virtual void Write(const void* buffer);

  /**
   * Reads the data from disk into the memory buffer provided. NOTE: this method
   * supports z-compression.
   * @see UseCompression property
   **/
  virtual void Read(void* buffer);

  /**
   * This is relevant if AutoExtensionMode is true as the image information for
   * metaimage data can be extracted, certainly not for raw images.
   */
  virtual void ReadImageInformation();

  /** Set use fast, but less effective compression. **/
  itkSetMacro(UseFastCompression, bool)
  /** Get use fast, but less effective compression. **/
  itkGetMacro(UseFastCompression, bool)
  itkBooleanMacro(UseFastCompression)

  /** Set auto-extension mode (see class-description). **/
  itkSetMacro(AutoExtensionMode, bool)
  /** Get auto-extension mode (see class-description). **/
  itkGetMacro(AutoExtensionMode, bool)
  itkBooleanMacro(AutoExtensionMode)

protected:
  /** Use fast, but less effective compression. **/
  bool m_UseFastCompression;
  /** Size of compressed image (last compression activity). **/
  unsigned long m_CompressedSize;
  /** Auto-extension mode (see class-description). **/
  bool m_AutoExtensionMode;
  /** Helper for metaimage data reading. **/
  std::string m_DataFile;

  /** Default constructor. **/
  XRawImageIO();
  /** Destructor. **/
  virtual ~XRawImageIO();
  /** Standard object information. **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Compress the specified source buffer and write it to file dest until EOF on
   * source buffer.
   * @param buffer source buffer
   * @param buffSizeBytes buffer size in bytes
   * @param dest a prepared destination file
   * @param level compression level; values: Z_NO_COMPRESSION (0),
   * Z_BEST_SPEED (1), Z_BEST_COMPRESSION (9), Z_DEFAULT_COMPRESSION (-1)
   * @return Z_OK on success, Z_MEM_ERROR if memory could not be
   * allocated for processing, Z_STREAM_ERROR if an invalid compression
   * level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   * version of the library linked do not match, or Z_ERRNO if there is
   * an error reading or writing the files.
   **/
  virtual int ZCompress(char *buffer, unsigned long buffSizeBytes, FILE *dest,
      int level);

  /*
   * Decompress from file source to buffer until stream ends or EOF.
   * @param source the source file to be decompressed
   * @param buffer buffer to receive the decompressed bytes
   * @param buffSizeInBytes buffer size in bytes
   * @return Z_OK on success, Z_MEM_ERROR if memory could not be
   * allocated for processing, Z_DATA_ERROR if the deflate data is
   * invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   * the version of the library linked do not match, or Z_ERRNO if there
   * is an error reading or writing the files.
   **/
  virtual int ZDecompress(FILE *source, char *buffer,
      unsigned long buffSizeInBytes);

  /**
   * Write a meta data file (metaimage format) supporting the entries mentioned
   * in class description.
   * @param metaDataFileName name of the metadata file to be generated (usually
   * the extension .mhd)
   * @param dataFileName name of the according data file (NOTE: this is just
   * for referencing in the metadata file, so please remove absolute paths
   * from this file specification!)
   * @return true if successful
   */
  virtual bool WriteMetaData(const char *metaDataFileName,
      const char *dataFileName);

  /** Helper method for trimming a string. **/
  void ITrim(std::string &s);
  /** Helper method for tokenizing a delimited string. **/
  void ITokenize(const std::string &str, std::vector<std::string> &tokens,
      const std::string &delimiters);

private:
  /** Purposely not implemented. **/
  XRawImageIO(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraXRawImageIO.txx"

#endif /* ORAXRAWIMAGEIO_H_ */
