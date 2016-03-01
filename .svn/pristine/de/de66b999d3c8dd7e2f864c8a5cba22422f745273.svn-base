

#ifndef ITKRTIIMAGEIO_H
#define ITKRTIIMAGEIO_H

#include <cstddef> /* Workaround bug in ITK 3.20 */

#include <itkImageIOBase.h>

#include "oraITKVTKImageMetaInformation.h"


namespace itk
{


/**
 * Open radART typical *.rti file header (version >= 1.4).
 * I decided to implemented it as a (dirty) class instead of a
 * struct to preserve inheritance features.
 * @author phil 
 * @author Markus 
 * @version 1.8.1
 */
class _RTI_HEADER
{
protected:
  // simply all data (this avoids the problem with double word borders on
  // different architectures):
  char m_Data[128];
public:
  // versions (4 bytes)
  unsigned char GetBmpTypeMajor()
  {
    return (unsigned char)m_Data[0];
  }
  void SetBmpTypeMajor(unsigned char v)
  {
    m_Data[0] = v;
  }
  unsigned char GetBmpTypeMinor()
  {
    return (unsigned char)m_Data[1];
  }
  void SetBmpTypeMinor(unsigned char v)
  {
    m_Data[1] = v;
  }
  unsigned char GetBmpTypeRevision()
  {
    return (unsigned char)m_Data[2];
  }
  void SetBmpTypeRevision(unsigned char v)
  {
    m_Data[2] = v;
  }
  unsigned char GetBmpTypeReserved()
  {
    return (unsigned char)m_Data[3];
  }
  void SetBmpTypeReserved(unsigned char v)
  {
    m_Data[3] = v;
  }
  // size (4 bytes)
  unsigned short GetCols()
  {
    return *(static_cast<unsigned short *>((void *)&m_Data[4]));
  }
  void SetCols(unsigned short v)
  {
    *(static_cast<unsigned short *>((void *)&m_Data[4])) = v;
  }
  unsigned short GetRows()
  {
    return *(static_cast<unsigned short *>((void *)&m_Data[6]));
  }
  void SetRows(unsigned short v)
  {
    *(static_cast<unsigned short *>((void *)&m_Data[6])) = v;
  }
  // bits info (4 bytes)
  unsigned short GetBitsAlloc()
  {
    int major = (int)GetBmpTypeMajor();
    int minor = (int)GetBmpTypeMinor();
    if (major <= 1 && minor < 4) // up to version 1.3 the high byte is not trustful!
      m_Data[9] = 0; // clear high byte
    return *(static_cast<unsigned short *>((void *)&m_Data[8]));
  }
  void SetBitsAlloc(unsigned short v)
  {
    *(static_cast<unsigned short *>((void *)&m_Data[8])) = v;
  }
  unsigned short GetBitsStored()
  {
    int major = (int)GetBmpTypeMajor();
    int minor = (int)GetBmpTypeMinor();
    if (major <= 1 && minor < 4) // up to version 1.3 the high byte is not trustful!
      m_Data[11] = 0; // clear high byte
    return *(static_cast<unsigned short *>((void *)&m_Data[10]));
  }
  void SetBitsStored(unsigned short v)
  {
    *(static_cast<unsigned short *>((void *)&m_Data[10])) = v;
  }
  // windowing (16 bytes)
  long GetCenter64()
  {
    return *(static_cast<long *>((void *)&m_Data[12]));
  }
  void SetCenter64(long v)
  {
    *(static_cast<long *>((void *)&m_Data[12])) = v;
  }
  long GetWidth64()
  {
    return *(static_cast<long *>((void *)&m_Data[20]));
  }
  void SetWidth64(long v)
  {
    *(static_cast<long *>((void *)&m_Data[20])) = v;
  }
  // rescaling (49 bytes)
  float GetRescaleIntercept()
  {
    return *(static_cast<float *>((void *)&m_Data[28]));
  }
  void SetRescaleIntercept(float v)
  {
    *(static_cast<float *>((void *)&m_Data[28])) = v;
  }
  float GetRescaleSlope()
  {
    return *(static_cast<float *>((void *)&m_Data[32]));
  }
  void SetRescaleSlope(float v)
  {
    *(static_cast<float *>((void *)&m_Data[32])) = v;
  }
  std::string GetRescaleUnit()
  {
    std::string s(20, ' ');

    for (unsigned int i = 0; i < s.length(); ++i)
      s[i] = m_Data[36 + i];

    return s;
  }
  void SetRescaleUnit(std::string s)
  {
    for (unsigned int i = 0; i < 20; ++i)
    {
      if (i < s.length())
        m_Data[36 + i] = s[i];
      else
        m_Data[36 + i] = ' ';
    }
  }
  std::string GetRescaleDescriptor()
  {
    std::string s(20, ' ');

    for (unsigned int i = 0; i < s.length(); ++i)
      s[i] = m_Data[56 + i];

    return s;
  }
  void SetRescaleDescriptor(std::string s)
  {
    for (unsigned int i = 0; i < 20; ++i)
    {
      if (i < s.length())
        m_Data[56 + i] = s[i];
      else
        m_Data[56 + i] = ' ';
    }
  }
  unsigned char GetRescaleDigits()
  {
    return (unsigned char)m_Data[76];
  }
  void SetRescaleDigits(unsigned char v)
  {
    m_Data[76] = v;
  }
  float GetGammaSingle()
  {
    return *(static_cast<float *>((void *)&m_Data[77]));
  }
  void SetGammaSingle(float v)
  {
    *(static_cast<float *>((void *)&m_Data[77])) = v;
  }
};


/**
 * ITK-compliant Image IO for reading open radART *.rti images.
 * @author phil 
 * @version 1.4
 */
class RTIImageIO
  : public ImageIOBase
{
public:
  /** Standard class typedefs. */
  typedef RTIImageIO Self;
  typedef ImageIOBase Superclass;
  typedef SmartPointer<Self> Pointer;

  /** Important typedefs **/
  typedef ora::ITKVTKImageMetaInformation MetaInfoType;
  typedef MetaInfoType::Pointer MetaInfoPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(RTIImageIO, ImageIOBase);

  /** object information streaming **/
  void PrintSelf(std::ostream& os, Indent indent) const;


  // READ STUFF:

  /** Determine the file type. Returns true if this ImageIO can read the
    * file specified. */
  virtual bool CanReadFile(const char*);

  /** Set the spacing and dimension information for the set filename. */
  virtual void ReadImageInformation();

  /** Reads the data from disk into the memory buffer provided. */
  virtual void Read(void* buffer);

  // WRITE STUFF:

  /** Determine the file type. Returns true if this ImageIO can write the
   * file specified. */
  virtual bool CanWriteFile(const char*);

  /** Writes the spacing and dimentions of the image.
   * Assumes SetFileName has been called with a valid file name. */
  virtual void WriteImageInformation();

  /** Writes the data to disk from the memory buffer provided. Make sure
   * that the IORegion has been set properly. */
  virtual void Write(const void* buffer);

  /** override in order to auto-detect the complementary meta file name **/
  virtual void SetFileName(const char *fileName)
  {
    Superclass::SetFileName(fileName);
    if (std::string(fileName).length() > 4)
      m_ComplementaryMetaFile = std::string(fileName).
        substr(0, std::string(fileName).length() - 3) + "inf";
    else
      m_ComplementaryMetaFile = "";
  };

  /**
   * Set an (optional) image meta information object which contains loaded
   * information about the available images (image list) and the available
   * frames of references (FOR collection).
   **/
  void SetMetaInfo(MetaInfoPointer metaInfo)
  {
    m_MetaInfo = metaInfo;
  }
  /** Get the image meta information. **/
  MetaInfoPointer GetMetaInfo()
  {
    return m_MetaInfo;
  }

  /**
   * Get sequencer help flag. If this flag is TRUE it indicates that
   * the first of a series of images is processed. It is automatically reset
   * after reading one image.
   */
  bool GetSequencerHelpFlag()
  {
    return m_SequencerHelpFlag;
  }
  /**
   * Set sequencer help flag. If this flag is TRUE it indicates that
   * the first of a series of images is processed. It is automatically reset
   * after reading one image.
   */
  void SetSequencerHelpFlag(bool helpFlag)
  {
    m_SequencerHelpFlag = helpFlag;
  }

  /**
   * Set flag indicating that this IO is used as IO for an image series reader;
   * setting it to TRUE will lead to specific behavior for that purpose
   * (meta information consistency)
   **/
  void SetImageSeriesReaderMode(bool value)
  {
    m_ImageSeriesReaderHelpFlag = false; // set back
    m_ImageSeriesReaderMode = value;
  }
  /**
   * Get flag indicating that this IO is used as IO for an image series reader;
   * setting it to TRUE will lead to specific behavior for that purpose
   * (meta information consistency)
   **/
  bool GetImageSeriesReaderMode()
  {
    return m_ImageSeriesReaderMode;
  }

protected:
  /** open radART *.rti's header information (pre-buffered in CanReadFile()) **/
  _RTI_HEADER *m_Header;
  /** typical open radART *.inf file containing additional meta image information **/
  std::string m_ComplementaryMetaFile;
  /** reference to the image meta information object **/
  MetaInfoPointer m_MetaInfo;
  /**
   * Help flag for series image reading - if this flag is TRUE it indicates that
   * the first of a series of images is processed. It is automatically reset
   * after reading one image.
   **/
  bool m_SequencerHelpFlag;
  /**
   * current image number within sequence (0-based); works with
   * m_SequencerHelpFlag
   **/
  int m_SequenceNumber;
  /**
   * flag indicating that this IO is used as IO for an image series reader;
   * setting it to TRUE will lead to specific behavior for that purpose
   * (meta information consistency); this flag must newly be set for each
   * new image series loading process
   **/
  bool m_ImageSeriesReaderMode;
  /** internal flag for image series reader behavior **/
  bool m_ImageSeriesReaderHelpFlag;

  /** internal default constructor **/
  RTIImageIO();

  /** internal destructor **/
  ~RTIImageIO();

private:
  /** purposely not implemented **/
  RTIImageIO(const Self&);

  /** purposely not implemented **/
  void operator=(const Self&);

};


}


#endif /* ITKRTIIMAGEIO_H */
