//
#ifndef ORAXRAWIMAGEIO_TXX_
#define ORAXRAWIMAGEIO_TXX_

#include "oraXRawImageIO.h"

#include <itkByteSwapper.h>
#include <itkzlib/zlib.h>
#include <itkSpatialOrientationAdapter.h>


namespace ora
{

template<class TPixel, unsigned int VImageDimension>
XRawImageIO<TPixel, VImageDimension>::XRawImageIO() :
  Superclass()
{
  this->m_UseFastCompression = false;
  this->m_AutoExtensionMode = true;
  this->m_CompressedSize = 0;
  this->m_DataFile = "";
  this->m_ByteOrder = itk::ImageIOBase::LittleEndian;
}

template<class TPixel, unsigned int VImageDimension>
XRawImageIO<TPixel, VImageDimension>::~XRawImageIO()
{

}

template<class TPixel, unsigned int VImageDimension>
void XRawImageIO<TPixel, VImageDimension>::PrintSelf(std::ostream& os,
    itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Use Fast Compression: " << m_UseFastCompression << "" << std::endl;
  os << indent << "Auto Extension Mode: " << m_AutoExtensionMode << "" << std::endl;
  os << indent << "Compressed Size: " << m_CompressedSize << "" << std::endl;
  os << indent << "Data File: " << m_DataFile << "" << std::endl;
}

template<class TPixel, unsigned int VImageDimension>
bool XRawImageIO<TPixel, VImageDimension>::WriteMetaData(
    const char *metaDataFileName, const char *dataFileName)
{
  if (std::string(metaDataFileName).length() <= 0)
    return false;

  std::ofstream mf;

  mf.open(metaDataFileName, std::ios::out);
  if (!mf.is_open())
    return false;

  // write the minimum set of metaimage format entries:
  mf << "ObjectType = Image" << std::endl;
  mf << "NDims = " << this->GetNumberOfDimensions() << "" << std::endl;
  mf << "BinaryData = " << (this->m_FileType == itk::ImageIOBase::Binary ?
      "True" : "False") << "" << std::endl;
  mf << "BinaryDataByteOrderMSB = " <<
      (this->m_ByteOrder == itk::ImageIOBase::LittleEndian ? "False" :
      "True") << "" << std::endl;
  mf << "CompressedData = " << (this->m_UseCompression ? "True" : "False") <<
      "" << std::endl;
  mf << "CompressedDataSize = " << this->m_CompressedSize << "" << std::endl;
  std::vector<double> dir0 = this->GetDirection(0);
  std::vector<double> dir1 = this->GetDirection(1);
  std::vector<double> dir2 = this->GetDirection(2);
  mf << "TransformMatrix = " << dir0[0] << " " << dir0[1] << " " <<
      dir0[2] << " " << dir1[0] << " " << dir1[1] << " " << dir1[2] << " " <<
      dir2[0] << " " << dir2[1] << " " << dir2[2] << "" << std::endl;
  mf << "Offset = " << this->GetOrigin(0) << " " << this->GetOrigin(1) << " " <<
      this->GetOrigin(2) << "" << std::endl;
  mf << "CenterOfRotation = 0 0 0" << std::endl;
  // extract anatomical information from direction set:
  // -> compute anatomical descriptions of the main directions (DICOM LPS)
  std::string orientation = "";
  itk::SpatialOrientationAdapter::DirectionType dir;
  for(unsigned i = 0; i < 3; i++)
  {
    dir[i][0] = dir0[i];
    dir[i][1] = dir1[i];
    dir[i][2] = dir2[i];
  }
  itk::SpatialOrientation::ValidCoordinateOrientationFlags coordOrient;
  // -> extract
  coordOrient = itk::SpatialOrientationAdapter().FromDirectionCosines(dir);
  // -> convert to string:
  // 1st direction:
  switch(coordOrient)
  {
    default:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSP:
    {
      orientation += "R";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LAI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LAS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LIA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LIP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LSA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LSP:
    {
      orientation += "L";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ALI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ALS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASR:
    {
      orientation += "A";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PLI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PLS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PRI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PRS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PIL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PIR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PSL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PSR:
    {
      orientation += "P";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IPL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IPR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IAL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IAR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ILA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ILP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IRA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IRP:
    {
      orientation += "I";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SPL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SPR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SAL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SAR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SLA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SLP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SRA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SRP:
    {
      orientation += "S";
      break;
    }
  }
  // 2nd direction:
  switch(coordOrient)
  {
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PRI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PRS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IRA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IRP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SRA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SRP:
    {
      orientation += "R";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PLI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PLS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ALI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ALS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ILA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ILP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SLA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SLP:
    {
      orientation += "L";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LAI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LAS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IAL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IAR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SAL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SAR:
    {
      orientation += "A";
      break;
    }
    default:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IPL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IPR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SPL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SPR:
    {
      orientation += "P";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PIL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PIR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LIA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LIP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP:
    {
      orientation += "I";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PSL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PSR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LSA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LSP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSP:
    {
      orientation += "S";
      break;
    }
  }
  // 3rd direction:
  switch(coordOrient)
  {
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PIR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PSR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IAR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IPR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SAR:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SPR:
    {
      orientation += "R";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PIL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PSL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IAL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IPL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SAL:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SPL:
    {
      orientation += "L";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LIA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LSA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ILA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IRA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SLA:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SRA:
    {
      orientation += "A";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LIP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LSP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ILP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IRP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SLP:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SRP:
    {
      orientation += "P";
      break;
    }
    default:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PLI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PRI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ALI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LAI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPI:
    {
      orientation += "I";
      break;
    }
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PLS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PRS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ALS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LAS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAS:
    case itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPS:
    {
      orientation += "S";
      break;
    }
  }
  mf << "AnatomicalOrientation = " << orientation << "" << std::endl;
  mf << "ElementSpacing = " << this->GetSpacing(0) << " " <<
      this->GetSpacing(1) << " " << this->GetSpacing(2) << "" << std::endl;
  mf << "DimSize = " << this->GetDimensions(0) << " " <<
      this->GetDimensions(1) << " " << this->GetDimensions(2) << "" << std::endl;
  std::string cts = "";
  if (this->GetComponentType() == itk::ImageIOBase::UCHAR)
    cts = "MET_UCHAR";
  else if (this->GetComponentType() == itk::ImageIOBase::CHAR)
    cts = "MET_CHAR";
  else if (this->GetComponentType() == itk::ImageIOBase::USHORT)
    cts = "MET_USHORT";
  else if (this->GetComponentType() == itk::ImageIOBase::SHORT)
    cts = "MET_SHORT";
  else if (this->GetComponentType() == itk::ImageIOBase::UINT)
    cts = "MET_UINT";
  else if (this->GetComponentType() == itk::ImageIOBase::INT)
    cts = "MET_INT";
  else if (this->GetComponentType() == itk::ImageIOBase::ULONG)
    cts = "MET_ULONG";
  else if (this->GetComponentType() == itk::ImageIOBase::LONG)
    cts = "MET_LONG";
  else if (this->GetComponentType() == itk::ImageIOBase::FLOAT)
    cts = "MET_FLOAT";
  else if (this->GetComponentType() == itk::ImageIOBase::DOUBLE)
    cts = "MET_DOUBLE";
  else
    cts = "MET_OTHER";
  mf << "ElementType = " << cts << "" << std::endl;
  mf << "ElementDataFile = " << dataFileName << "" << std::endl;

  mf.close();

  return true;
}

template<class TPixel, unsigned int VImageDimension>
void XRawImageIO<TPixel, VImageDimension>::Write(const void* buffer)
{
  if (this->m_FileType != itk::ImageIOBase::Binary)
  {
    itkExceptionMacro(<< "This IO only supports BINARY file type!");
  }

  std::string fn = this->m_FileName;
  bool writeMetaFile = false;;
  if (this->m_AutoExtensionMode) // check extensions and re-configure
  {
    std::string ext = itksys::SystemTools::GetFilenameExtension(fn);
    ext = itksys::SystemTools::LowerCase(ext);
    if (ext == ".zraw")
    {
      // leave fn
      this->m_UseCompression = true;
      this->m_UseFastCompression = false;
      writeMetaFile = false;
    }
    else if (ext == ".xraw")
    {
      // leave fn
      this->m_UseCompression = true;
      this->m_UseFastCompression = true;
      writeMetaFile = false;
    }
    else if (ext == ".mhd")
    {
      fn = fn.substr(0, fn.length() - ext.length()) + ".raw";
      this->m_UseCompression = false;
      writeMetaFile = true;
    }
    else if (ext == ".zmhd")
    {
      fn = fn.substr(0, fn.length() - ext.length()) + ".zraw";
      this->m_UseCompression = true;
      this->m_UseFastCompression = false;
      writeMetaFile = true;
    }
    else if (ext == ".xmhd")
    {
      fn = fn.substr(0, fn.length() - ext.length()) + ".xraw";
      this->m_UseCompression = true;
      this->m_UseFastCompression = true;
      writeMetaFile = true;
    }
    else //if (ext == ".raw")
    {
      // leave fn
      this->m_UseCompression = false;
      writeMetaFile = false;
    }
  }

  FILE *file = fopen(fn.c_str(), "wb");
  if (!file)
  {
    itkExceptionMacro(<< "File could not be opened: " << fn);
  }

  this->ComputeStrides();

  const unsigned long numberOfBytes = this->GetImageSizeInBytes();
  const unsigned long numberOfComponents = this->GetImageSizeInComponents();

  long ret;
  char *tempBuffer = (char *)buffer;
  bool newbuff = false;

  // convenience macro (type-dependent if-clause + body):
#define oraWriteCompressedRawBytesAfterSwappingMacro(StrongType, WeakType) \
  ( this->GetComponentType() == WeakType ) \
  { \
    typedef itk::ByteSwapper<StrongType> InternalByteSwapperType; \
    /* It would be crazy to allocate a second buffer if we do not need to ... */ \
    if ((!InternalByteSwapperType::SystemIsBigEndian() && \
        this->m_ByteOrder == itk::ImageIOBase::BigEndian) || \
        (!InternalByteSwapperType::SystemIsLittleEndian() && \
        this->m_ByteOrder == itk::ImageIOBase::LittleEndian)) \
    { \
      tempBuffer = new char[numberOfBytes]; \
      memcpy(tempBuffer, buffer , numberOfBytes); \
      if (this->m_ByteOrder == itk::ImageIOBase::LittleEndian) \
        InternalByteSwapperType::SwapRangeFromSystemToLittleEndian( \
            (StrongType *)tempBuffer, numberOfComponents); \
      else if (this->m_ByteOrder == itk::ImageIOBase::BigEndian) \
        InternalByteSwapperType::SwapRangeFromSystemToBigEndian( \
              (StrongType *)tempBuffer, numberOfComponents); \
      \
      newbuff = true; \
    } \
  }

  // swap bytes if necessary, do the compression and write the file:
  if oraWriteCompressedRawBytesAfterSwappingMacro(unsigned short,
      itk::ImageIOBase::USHORT)
  else if oraWriteCompressedRawBytesAfterSwappingMacro(short,
      itk::ImageIOBase::SHORT)
  else if oraWriteCompressedRawBytesAfterSwappingMacro(char,
      itk::ImageIOBase::CHAR)
  else if oraWriteCompressedRawBytesAfterSwappingMacro(unsigned char,
      itk::ImageIOBase::UCHAR)
  else if oraWriteCompressedRawBytesAfterSwappingMacro(unsigned int,
      itk::ImageIOBase::UINT)
  else if oraWriteCompressedRawBytesAfterSwappingMacro(int,
      itk::ImageIOBase::INT)
  else if oraWriteCompressedRawBytesAfterSwappingMacro(long,
      itk::ImageIOBase::LONG)
  else if oraWriteCompressedRawBytesAfterSwappingMacro(unsigned long,
      itk::ImageIOBase::ULONG)
  else if oraWriteCompressedRawBytesAfterSwappingMacro(float,
      itk::ImageIOBase::FLOAT)
  else if oraWriteCompressedRawBytesAfterSwappingMacro(double,
      itk::ImageIOBase::DOUBLE)

  if (this->m_UseCompression)
  {
    /* Now compress and write out file ... */
    if (this->m_UseFastCompression) // fast, but less effective
      ret = ZCompress(tempBuffer, numberOfBytes, file, Z_BEST_SPEED);
    else // good speed, good compression
      ret = ZCompress(tempBuffer, numberOfBytes, file, Z_DEFAULT_COMPRESSION);

    if (ret != Z_OK)
    {
      itkExceptionMacro(<< "Error during compression of image data: " << fn);
    }
  }
  else // no compression
  {
    this->m_CompressedSize = numberOfBytes;
    ret = fwrite(tempBuffer, 1, numberOfBytes, file);
    if ((unsigned long)ret != numberOfBytes)
    {
      itkExceptionMacro(<< "Error during writing image data: " << fn);
    }
  }

  if (newbuff)
    delete[] tempBuffer;

  // ok, done
  fclose(file);

  // write meta data:
  if (writeMetaFile)
  {
    int extlen = itksys::SystemTools::GetFilenameExtension(fn).length();
    std::string metadataFN = fn.substr(0, fn.length() - extlen) + ".mhd";
    std::string dataFN = itksys::SystemTools::GetFilenameName(fn);
    if (!WriteMetaData(metadataFN.c_str(), dataFN.c_str()))
    {
      itkExceptionMacro(<< "Error during writing meta data: " << metadataFN);
    }
  }
}

template<class TPixel, unsigned int VImageDimension>
void XRawImageIO<TPixel, VImageDimension>::ITrim(std::string &s)
{
  std::string::size_type pos = s.find_last_not_of(' ');
  if(pos != std::string::npos)
  {
    s.erase(pos + 1);
    pos = s.find_first_not_of(' ');
    if(pos != std::string::npos)
      s.erase(0, pos);
  }
  else
  {
    s.erase(s.begin(), s.end());
  }
}

template<class TPixel, unsigned int VImageDimension>
void XRawImageIO<TPixel, VImageDimension>::ITokenize(const std::string &str,
    std::vector<std::string> &tokens, const std::string &delimiters)
{
  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  std::string::size_type pos = str.find_first_of(delimiters, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
  {
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    lastPos = str.find_first_not_of(delimiters, pos);
    pos = str.find_first_of(delimiters, lastPos);
  }
}

template<class TPixel, unsigned int VImageDimension>
void XRawImageIO<TPixel, VImageDimension>::ReadImageInformation()
{
  this->m_DataFile = ""; // reset
  if (this->m_AutoExtensionMode) // re-configure dependent on extension
  {
    std::string fn = this->m_FileName;
    std::string ext = itksys::SystemTools::GetFilenameExtension(fn);
    ext = itksys::SystemTools::LowerCase(ext);
    if (ext == ".mhd" || ext == ".zmhd" || ext == ".xmhd")
    {
      this->SetHeaderSize(0); // do not have to consider a header!

      // read metadata file for more information:
      std::ifstream mf;
      mf.open(fn.c_str(), std::ios::in);
      if (!mf.is_open())
        return;

      const int MAX_LINE = 2047;
      char line[MAX_LINE];
      do
      {
        mf.getline(line, MAX_LINE);
        std::string ln = std::string(line);
        std::size_t p = ln.find('=', 0);
        if (p != std::string::npos)
        {
          std::string key = ln.substr(0, p);
          ITrim(key);
          key = itksys::SystemTools::LowerCase(key);
          std::string value = ln.substr(p + 1, ln.length());
          ITrim(value);
          // supported entries:
          if (key == "objecttype")
          {
            value = itksys::SystemTools::LowerCase(value);
            if (value != "image") // no other supported
              itkExceptionMacro(<< "METADATA ERROR: Only 'Image' (ObjectType) supported!");
          }
          else if (key == "binarydata")
          {
            value = itksys::SystemTools::LowerCase(value);
            if (value != "true") // no other supported
              itkExceptionMacro(<< "METADATA ERROR: Only 'True' (BinaryData) supported!");
          }
          else if (key == "elementdatafile")
          {
            if (value.length() <= 0) // need it!
              itkExceptionMacro(<< "METADATA ERROR: No data file reference (ElementDataFile) found!");
            std::string tfn = "";
            if (itksys::SystemTools::FileIsFullPath(value.c_str())) // absolute
            {
              tfn = value;
            }
            else // relative path (relative to metadata file's path!)
            {
              std::string name = itksys::SystemTools::GetFilenameName(fn);
              tfn = fn.substr(0, fn.length() - name.length()); // + separator
              tfn += value;
              if (tfn[0] == '\\' || tfn[0] == '/')
                tfn = "." + tfn;
            }
            if (!itksys::SystemTools::FileExists(tfn.c_str(), true))
              itkExceptionMacro(<< "METADATA ERROR: No data file reference (ElementDataFile) found!");
            this->m_DataFile = tfn; // store for later
          }
          else if (key == "binarydatabyteordermsb")
          {
            value = itksys::SystemTools::LowerCase(value);
            if (value == "true") // big endian
              this->SetByteOrderToBigEndian();
            else // little endian
              this->SetByteOrderToLittleEndian();
          }
          else if (key == "compresseddata")
          {
            value = itksys::SystemTools::LowerCase(value);
            if (value == "true") // compressed (Z-lib assumed)
              this->SetUseCompression(true);
            else // uncompressed
              this->SetUseCompression(false);
          }
          else if (key == "elementtype")
          {
            value = itksys::SystemTools::UpperCase(value);
            if (value == "MET_UCHAR")
              this->SetComponentType(itk::ImageIOBase::UCHAR);
            else if (value == "MET_CHAR")
              this->SetComponentType(itk::ImageIOBase::CHAR);
            else if (value == "MET_USHORT")
              this->SetComponentType(itk::ImageIOBase::USHORT);
            else if (value == "MET_SHORT")
              this->SetComponentType(itk::ImageIOBase::SHORT);
            else if (value == "MET_UINT")
              this->SetComponentType(itk::ImageIOBase::UINT);
            else if (value == "MET_INT")
              this->SetComponentType(itk::ImageIOBase::INT);
            else if (value == "MET_ULONG")
              this->SetComponentType(itk::ImageIOBase::ULONG);
            else if (value == "MET_LONG")
              this->SetComponentType(itk::ImageIOBase::LONG);
            else if (value == "MET_FLOAT")
              this->SetComponentType(itk::ImageIOBase::FLOAT);
            else if (value == "MET_DOUBLE")
              this->SetComponentType(itk::ImageIOBase::DOUBLE);
            else
              this->SetComponentType(itk::ImageIOBase::UNKNOWNCOMPONENTTYPE);
          }
          else if (key == "ndims")
          {
            int ndims = atoi(value.c_str());
            if (ndims > 0)
              this->SetNumberOfDimensions((unsigned int)ndims);
          }
          else if (key == "dimsize")
          {
            std::vector<unsigned int> dims;
            std::vector<std::string> toks;
            ITokenize(value, toks, " ");
            for (std::size_t i = 0; i < toks.size(); i++)
            {
              int x = atoi(toks[i].c_str());
              if (x > 0)
                dims.push_back((unsigned int)x);
            }
            // assume that we have ndims:
            if (dims.size() != this->GetNumberOfDimensions())
              break;
            for (std::size_t i = 0; i < dims.size(); i++)
              this->SetDimensions(i, dims[i]);
          }
          else if (key == "offset")
          {
            std::vector<double> origin;
            std::vector<std::string> toks;
            ITokenize(value, toks, " ");
            for (std::size_t i = 0; i < toks.size(); i++)
            {
              double d = atof(toks[i].c_str());
              origin.push_back(d);
            }
            // assume that we have ndims:
            if (origin.size() != this->GetNumberOfDimensions())
              break;
            for (std::size_t i = 0; i < origin.size(); i++)
              this->SetOrigin(i, origin[i]);
          }
          else if (key == "elementspacing")
          {
            std::vector<double> spacing;
            std::vector<std::string> toks;
            ITokenize(value, toks, " ");
            for (std::size_t i = 0; i < toks.size(); i++)
            {
              double d = atof(toks[i].c_str());
              spacing.push_back(d);
            }
            // assume that we have ndims:
            if (spacing.size() != this->GetNumberOfDimensions())
              break;
            for (std::size_t i = 0; i < spacing.size(); i++)
              this->SetSpacing(i, spacing[i]);
          }
          else if (key == "transformmatrix")
          {
            std::vector<double> dirs;
            std::vector<std::string> toks;
            ITokenize(value, toks, " ");
            for (std::size_t i = 0; i < toks.size(); i++)
            {
              double d = atof(toks[i].c_str());
              dirs.push_back(d);
            }
            // assume that we have ndims:
            if (dirs.size() !=
                (this->GetNumberOfDimensions() * this->GetNumberOfDimensions()))
              break;
            unsigned int i = 0;
            std::vector<double> dirX;
            for (unsigned int j = 0; j < this->GetNumberOfDimensions(); j++)
            {
              dirX.clear();
              for (unsigned int k = 0; k < this->GetNumberOfDimensions(); k++)
                dirX.push_back(dirs[i++]);
              this->SetDirection(j, dirX);
            }
          }
        }
      }
      while (!mf.eof() && !mf.fail() && !mf.bad());

      mf.close();
    }
  }
}

template<class TPixel, unsigned int VImageDimension>
void XRawImageIO<TPixel, VImageDimension>::Read(void* buffer)
{
  if (this->m_FileType != itk::ImageIOBase::Binary)
  {
    itkExceptionMacro(<< "This IO only supports BINARY file type!");
  }

  std::string fn = this->m_FileName;
  if (this->m_DataFile.length() > 0) // obviously a metaimage pair!
    fn = this->m_DataFile;
  std::string mode = "rb";
  if (this->m_FileType != itk::ImageIOBase::Binary)
    mode = "r";
  FILE *file = fopen(fn.c_str(), mode.c_str());
  if (!file)
  {
    itkExceptionMacro(<< "File could not be opened: " << fn);
  }

  this->ComputeStrides();

  // offset into file
  unsigned long streamStart = this->GetHeaderSize();
  if (fseek(file, (long)streamStart, SEEK_SET) != 0)
  {
    itkExceptionMacro(<< "Error during file seeking: " << fn);
  }

  const unsigned long numberOfBytesToBeRead = static_cast<unsigned long>(
      this->GetImageSizeInBytes());
  long ret;

  if (this->m_UseCompression)
  {
    ret = ZDecompress(file, (char *)buffer, numberOfBytesToBeRead);
    if (ret != Z_OK)
    {
      itkExceptionMacro(<< "Error during decompression of image data: " << fn);
    }
  }
  else // no compression
  {
    ret = fread(buffer, 1, numberOfBytesToBeRead, file);
    if ((unsigned long)ret != numberOfBytesToBeRead)
    {
      itkExceptionMacro(<< "Error during reading image data: " << fn);
    }
  }

  // convenience macro (type-dependent if-clause + body):
#define oraReadDecompressedRawBytesAfterSwappingMacro(StrongType, WeakType) \
    (this->GetComponentType() == WeakType) \
    { \
      typedef itk::ByteSwapper< StrongType > InternalByteSwapperType; \
      if (this->m_ByteOrder == itk::ImageIOBase::LittleEndian && \
          !InternalByteSwapperType::SystemIsLittleEndian()) \
      { \
        InternalByteSwapperType::SwapRangeFromSystemToLittleEndian( \
            (StrongType *)buffer, this->GetImageSizeInComponents()); \
      } \
      else if (this->m_ByteOrder == itk::ImageIOBase::BigEndian && \
               !InternalByteSwapperType::SystemIsBigEndian()) \
      { \
        InternalByteSwapperType::SwapRangeFromSystemToBigEndian( \
            (StrongType*)buffer, this->GetImageSizeInComponents()); \
      } \
    }

  // Swap bytes if necessary
  if oraReadDecompressedRawBytesAfterSwappingMacro(unsigned short,
      itk::ImageIOBase::USHORT)
  else if oraReadDecompressedRawBytesAfterSwappingMacro(short,
      itk::ImageIOBase::SHORT)
  else if oraReadDecompressedRawBytesAfterSwappingMacro(char,
      itk::ImageIOBase::CHAR)
  else if oraReadDecompressedRawBytesAfterSwappingMacro(unsigned char,
      itk::ImageIOBase::UCHAR)
  else if oraReadDecompressedRawBytesAfterSwappingMacro(unsigned int,
      itk::ImageIOBase::UINT)
  else if oraReadDecompressedRawBytesAfterSwappingMacro(int,
      itk::ImageIOBase::INT)
  else if oraReadDecompressedRawBytesAfterSwappingMacro(long,
      itk::ImageIOBase::LONG)
  else if oraReadDecompressedRawBytesAfterSwappingMacro(unsigned long,
      itk::ImageIOBase::ULONG)
  else if oraReadDecompressedRawBytesAfterSwappingMacro(float,
      itk::ImageIOBase::FLOAT)
  else if oraReadDecompressedRawBytesAfterSwappingMacro(double,
      itk::ImageIOBase::DOUBLE)
}

template<class TPixel, unsigned int VImageDimension>
int XRawImageIO<TPixel, VImageDimension>::ZCompress(char *buffer,
    unsigned long buffSizeBytes, FILE *dest, int level)
{
  // chunk size: 512K
  const unsigned int CHUNK = 524288;
  int ret, flush;
  unsigned have;
  z_stream strm;
  unsigned char *in = new unsigned char[CHUNK];
  unsigned char out[CHUNK];
  unsigned long count = 0;

  // allocate deflate state
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit(&strm, level);
  if (ret != Z_OK)
    return ret;

  // compress until end of file
  this->m_CompressedSize = 0;
  do
  {
    in = (unsigned char *)(buffer + count);
    if ((count + CHUNK) <= buffSizeBytes) // within stream
    {
      strm.avail_in = CHUNK;
      count += CHUNK;
    }
    else // at end
    {
      strm.avail_in = buffSizeBytes - count;
      count = buffSizeBytes;
    }
    flush = buffSizeBytes <= count ? Z_FINISH : Z_NO_FLUSH;
    strm.next_in = in;

    // run deflate() on input until output buffer not full, finish compression
    // if all of source has been read in
    do
    {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = deflate(&strm, flush); // no bad return value
      have = CHUNK - strm.avail_out;
      this->m_CompressedSize += have;
      if (fwrite(out, 1, have, dest) != have || ferror(dest))
      {
        (void)deflateEnd(&strm);
        return Z_ERRNO;
      }
    }
    while (strm.avail_out == 0);
  }
  while (flush != Z_FINISH); // done when last data in file processed

  // clean up and return
  (void)deflateEnd(&strm);

  return Z_OK;
}

template<class TPixel, unsigned int VImageDimension>
int XRawImageIO<TPixel, VImageDimension>::ZDecompress(FILE *source,
    char *buffer, unsigned long buffSizeInBytes)
{
  // chunk size: 512K
  const unsigned int CHUNK = 524288;
  int ret;
  unsigned have;
  z_stream strm;
  unsigned char in[CHUNK];
  unsigned long count = 0;

  // allocate inflate state
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  ret = inflateInit(&strm);
  if (ret != Z_OK)
    return ret;

  // decompress until deflate stream ends or end of file
  do
  {
    strm.avail_in = fread(in, 1, CHUNK, source);
    if (ferror(source))
    {
      (void)inflateEnd(&strm);
      return Z_ERRNO;
    }
    if (strm.avail_in == 0)
      break;
    strm.next_in = in;

    /* run inflate() on input until output buffer not full */
    do
    {
      strm.next_out = (unsigned char *)(buffer + count);
      if ((count + CHUNK) <= buffSizeInBytes) // within stream
      {
        strm.avail_out = CHUNK;
      }
      else
      {
        strm.avail_out = buffSizeInBytes - count;
      }

      ret = inflate(&strm, Z_NO_FLUSH);
      switch (ret)
      {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR; // and fall through
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        return ret;
      }
      have = CHUNK - strm.avail_out;
      count += have;
    }
    while (strm.avail_out == 0 && count < buffSizeInBytes);
  }
  while (ret != Z_STREAM_END && count < buffSizeInBytes); // done

  // clean up and return
  (void)inflateEnd(&strm);

  return (count >= buffSizeInBytes) ? Z_OK : Z_DATA_ERROR;
}

}

#endif /* ORAXRAWIMAGEIO_TXX_ */
