
#ifndef ORARTITOMHDCONVERTER_TXX
#define ORARTITOMHDCONVERTER_TXX


#include "oraRTIToMHDConverter.h"

#include "oraFrameOfReference.h"
#include "oraImageList.h"
#include "oraComplementaryMetaFileCache.h"
#include "oraITKVTKImage.h"
#include "itkRTIImageIO.h"

// ORAIFTools
#include "oraStringTools.h"
#include "oraIniAccess.h"
#include "oraQFileTools.h"

#include <itkImageIOBase.h>
#include <itkImageSeriesReader.h>
#include <itksys/SystemTools.hxx>
#include <itkPoint.h>
#include <itkVector.h>
#include <itkFixedArray.h>
#include <itkRealTimeClock.h>

// Forward declarations
#include "oraCompactRTISliceMetaInfo.h"


namespace ora 
{


template <typename TPixelType>
void
RTIToMHDConverter<TPixelType>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Actual Slice List (n=" <<
    m_SliceList.size() << "):" << std::endl;
  for (unsigned int i = 0; i < m_SliceList.size(); i++)
    os << indent.GetNextIndent() << i << ": " <<
      m_SliceList[i] << std::endl;

  os << indent << "Actual Image:" << std::endl;
  if (m_Image)
    m_Image->Print(os, indent.GetNextIndent());
  else
    os << indent.GetNextIndent() << "NULL" << std::endl;

  os << indent << "Actual Image Meta Information:" << std::endl;
  if (m_MetaInfo)
    m_MetaInfo->Print(os, indent.GetNextIndent());
  else
    os << indent.GetNextIndent() << "NULL" << std::endl;

  os << indent << "Available Slice Stacks: (n=" <<
    m_AvailableSliceStacks.size() << ")" << std::endl;
  if (m_AvailableSliceStacks.size() > 0)
  {
    for (unsigned int u = 0; u < m_AvailableSliceStacks.size(); u++)
    {
      os << indent << "Stack " << (u + 1) << ": (n=" <<
        m_AvailableSliceStacks[u]->size() << ")" << std::endl;
      for (unsigned int v = 0; v < m_AvailableSliceStacks[u]->size(); v++)
        os << indent.GetNextIndent() << (*m_AvailableSliceStacks[u])[v] <<
          std::endl;
    }
  }
  else
    os << indent.GetNextIndent() << "NONE" << std::endl;
}

template <typename TPixelType>
RTIToMHDConverter<TPixelType>
::RTIToMHDConverter()
  : SimpleDebugger()
{
  m_SliceList.clear();
  m_Image = NULL;
  m_MetaInfo = NULL;
  ClearAvailableSliceStacks();
  m_ImageListFile = "";
  m_FrameOfReferenceFile = "";
  m_StackableAcquisitionTypes.clear();
  m_AvailableSliceStacks.clear();
  m_UseImageListInformationOnlyForMetaSorting = true;
}

template <typename TPixelType>
RTIToMHDConverter<TPixelType>
::~RTIToMHDConverter()
{
  m_SliceList.clear();
  m_Image = NULL;
  m_MetaInfo = NULL;
  ClearAvailableSliceStacks();
  m_StackableAcquisitionTypes.clear();
  m_AvailableSliceStacks.clear();
}

template <typename TPixelType>
void
RTIToMHDConverter<TPixelType>
::SetSliceListFromImageListFile()
{
  IniAccess ini(m_ImageListFile);
  // simply read all entries from the image list and set them:
  StrLstType *sliceList = ini.GetIndents("Images");
  if (sliceList)
  {
    this->SetSliceList(*sliceList);
    delete sliceList;
  }
}

template <typename TPixelType>
void
RTIToMHDConverter<TPixelType>
::SetSliceListByBaseFolder(std::string baseFolder)
{
  // browse the base folder (typically the patient folder):
  StrLstType sliceList = ora::GeneralFileTools::FindFilesInDirectory(
      baseFolder, "*.rti", true, "*");
  StrLstType sliceListOrg = ora::GeneralFileTools::FindFilesInDirectory(
      baseFolder, "*.rti.org", true, "*");
  for (std::size_t i = 0; i < sliceListOrg.size(); i++)
    sliceList.push_back(sliceListOrg[i]);

  this->SetSliceList(sliceList);
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::ValidateSlices(bool checkMetaDataAsWell)
{
  itk::Indent indent;

  if (m_SliceList.size() > 0)
  {
    UnixUNCConverter *conv = UnixUNCConverter::GetInstance();
    StrLstType tmp;
    bool validFile;
    std::string infFile;
    std::string::size_type p = std::string::npos;

    // delete inaccessible files from list without destroying the sort order:
    tmp.assign(m_SliceList.begin(), m_SliceList.end()); // temporary copy
    m_SliceList.clear();
    StrLstType::iterator it;
    for(it = tmp.begin(); it != tmp.end(); ++it)
    {
      *it = conv->EnsureOSCompatibility(*it); // UNC-UNIX-OS-conversion
      p = it->find_last_of(".");
      if (p != std::string::npos)
        infFile = it->substr(0, p + 1) + "inf";
      else
        infFile = ""; // invalid

      validFile = true;
      validFile = validFile && itksys::SystemTools::FileExists(it->c_str());
      validFile = validFile && (!checkMetaDataAsWell ||
          itksys::SystemTools::FileExists(infFile.c_str()));

      if (validFile)
        m_SliceList.push_back(*it); // remove items that do not exist
    }
    tmp.clear();

    return (m_SliceList.size() > 0);
  }
  else
    return false;
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::RTIFileNameStringSmallerThanOperator(const std::string &fn1,
    const std::string &fn2)
{
  // search for last occurrence of ".": (extension-begin)
  std::string::size_type posThisExtension = fn1.find_last_of('.');
  std::string::size_type posOtherExtension = fn2.find_last_of('.');

  if (posThisExtension != std::string::npos &&
      posOtherExtension != std::string::npos)
  {
    // extract the numeric part of the file name:
    std::string searchstr = "0123456789.+-";

    std::string::size_type posBeforeThisNumericPart = fn1.find_last_not_of(
      searchstr, posThisExtension);
    std::string thisNumericPart = "";

    if (posBeforeThisNumericPart < posThisExtension)
      thisNumericPart = fn1.substr(static_cast<std::string::size_type>(
          posBeforeThisNumericPart + 1), static_cast<std::string::size_type>(
          posThisExtension - posBeforeThisNumericPart - 1));
    // no prefix:
    else if (posThisExtension > static_cast<std::string::size_type>(0))
      thisNumericPart = fn1.substr(static_cast<std::string::size_type>(0),
        static_cast<std::string::size_type>(posThisExtension - 1));

    std::string::size_type posBeforeOtherNumericPart = fn2.find_last_not_of(
      searchstr, posOtherExtension);
    std::string otherNumericPart = "";

    if (posBeforeOtherNumericPart < posOtherExtension)
      otherNumericPart = fn2.substr(static_cast<std::string::size_type>(
        posBeforeOtherNumericPart + 1), static_cast<std::string::size_type>(
        posOtherExtension - posBeforeOtherNumericPart - 1));
    // no prefix:
    else if (posOtherExtension > static_cast<std::string::size_type>(0))
      otherNumericPart = fn2.substr(static_cast<std::string::size_type>(0),
        static_cast<std::string::size_type>(posOtherExtension - 1));

    if (otherNumericPart.length() > 0 && thisNumericPart.length() > 0)
      return (atof(thisNumericPart.c_str()) < atof(otherNumericPart.c_str()));
    // files without numeric part are ranked after files with numeric parts:
    else if (thisNumericPart.length() > 0)
      return true;
    else
      return false;
  }
  // files without numeric part are ranked after files with numeric parts:
  else if (posThisExtension != std::string::npos)
    return true;
  else
    return false;
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::CompareRTIFileNamesAscending(const std::string &fileName1,
    const std::string &fileName2)
{
  return RTIFileNameStringSmallerThanOperator(fileName1, fileName2);
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::CompareRTIFileNamesDescending(const std::string &fileName1,
    const std::string &fileName2)
{
  return !RTIFileNameStringSmallerThanOperator(fileName1, fileName2);
}

template <typename TPixelType>
void
RTIToMHDConverter<TPixelType>
::SortSlicesByFileNames(bool ascending)
{
  if (ValidateSlices(false)) // meta-data needs not necessarily to be defined
  {
    // simply invoke the standard sort algorithm (weak ordering):
    if (ascending)
      std::sort(m_SliceList.begin(), m_SliceList.end(),
          CompareRTIFileNamesAscending);
    else
      std::sort(m_SliceList.begin(), m_SliceList.end(),
          CompareRTIFileNamesDescending);
  }
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::RTIFileMetaInfoSmallerThanOperator(CompactRTISliceMetaInfo *i1,
    CompactRTISliceMetaInfo *i2)
{

  if (i1 && i2)
  {
    // -> project the slice origin onto the slicing direction and look if it
    // is smaller:
    // (compute the slice position w.r.t. slicing direction on demand)
    return (i1->ComputeSlicePositionIfNecessary() <
            i2->ComputeSlicePositionIfNecessary());
  }

  return false;
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::CompareRTIFileMetaInfoAscending(CompactRTISliceMetaInfo *i1,
    CompactRTISliceMetaInfo *i2)
{
  return RTIFileMetaInfoSmallerThanOperator(i1, i2);
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::CompareRTIFileMetaInfoDescending(CompactRTISliceMetaInfo *i1,
    CompactRTISliceMetaInfo *i2)
{
  return !RTIFileMetaInfoSmallerThanOperator(i1, i2);
}

template <typename TPixelType>
void
RTIToMHDConverter<TPixelType>
::SortSlicesByMetaInformation(bool ascending)
{
  // meta-information must exist depending on meta information usage:
  if (ValidateSlices(!m_UseImageListInformationOnlyForMetaSorting))
  {
    // is necessary for meta information access!
    Initialize(!m_UseImageListInformationOnlyForMetaSorting);
    // generate a compact representation:
    MetaVectorType metaSliceList;
    if (m_UseImageListInformationOnlyForMetaSorting) // without meta info files
    {
      CompactRTISliceMetaInfo::CreateFromCachedInformation(
          m_MetaInfo->GetImageList(), metaSliceList);
    }
    else // with pre-cached meta info files
    {
      ComplementaryMetaFileCache::Pointer cache = m_MetaInfo->
        GetComplementaryMetaFileCache();
      CacheVectorType cacheList = cache->GetCachedMetaFileEntryPointers();
      CacheIteratorType cit;
      for (cit = cacheList.begin(); cit != cacheList.end(); ++cit)
        metaSliceList.push_back(CompactRTISliceMetaInfo::
            CreateFromCachedInformation(*cit, m_MetaInfo->GetImageList()));
    }

    // detect available slice stacks:
    GenerateSortedAvailableSliceStacks(metaSliceList, ascending);

    // apply resulting sort-order to central RTI slice file name list:
    MetaIteratorType it;
    m_SliceList.clear();
    for (it = metaSliceList.begin(); it != metaSliceList.end(); ++it)
      if (*it)
        m_SliceList.push_back((*it)->FileName);
  }
}

template <typename TPixelType>
void
RTIToMHDConverter<TPixelType>
::ReSortSliceListByMetaInformation(bool ascending)
{
  if (ValidateSlices(false)) // use image list only
  {
    // is necessary for meta information access!
    Initialize(false);
    // generate a compact representation:
    MetaVectorType metaSliceList;
    // without meta info files!
    CompactRTISliceMetaInfo::CreateFromCachedInformation(
        m_MetaInfo->GetImageList(), metaSliceList);
    // now clear the files of meta-slice-list which do not belong to current
    // set slice list:
    MetaVectorType validMetaSliceList;
    for (unsigned int i = 0; i < m_SliceList.size(); ++i)
    {
      for (unsigned int j = 0; j < metaSliceList.size(); ++j)
      {
        if (m_SliceList[i] == metaSliceList[j]->FileName)
        {
          validMetaSliceList.push_back(metaSliceList[j]);
          break;
        }
      }
    }

    // detect available slice stacks:
    GenerateSortedAvailableSliceStacks(validMetaSliceList, ascending);

     // apply resulting sort-order to central RTI slice file name list:
    m_SliceList.clear();
    // one stack expected (but more stacks possible due to some very weird
    // circumstances):
    if (m_AvailableSliceStacks.size() >= 1)
    {
      // -> take biggest slice stack:
      StrLstType *lst = m_AvailableSliceStacks[0];
      for (unsigned int i = 1; i < m_AvailableSliceStacks.size(); i++)
        if ((!lst && m_AvailableSliceStacks[i]) ||
            (m_AvailableSliceStacks[i] &&
             m_AvailableSliceStacks[i]->size() > lst->size()))
          lst = m_AvailableSliceStacks[i];
      // same size expected, but smaller size possible due to weird
      // circumstances:
      if (lst && lst->size() <= validMetaSliceList.size())
      {
        for (unsigned int i = 0; i < lst->size(); ++i)
          m_SliceList.push_back((*lst)[i]);
      }
    }
  }
}

template <typename TPixelType>
void
RTIToMHDConverter<TPixelType>
::Initialize(bool preCacheMetaInfoFiles)
{
  if (!m_MetaInfo) //if not already created since last fundamental object-change
  {
    m_MetaInfo = MetaInfoType::New(); // meta information encapsulation
    // available FORs:
    FrameOfReferenceCollection::Pointer coll =
      FrameOfReferenceCollection::CreateFromFile(m_FrameOfReferenceFile);
    m_MetaInfo->SetFORColl(coll);
    // available images:
    ImageList::Pointer imglst =
      ImageList::CreateFromFile(m_ImageListFile, coll);
    m_MetaInfo->SetImageList(imglst);
    // complementary meta file cache:
    ComplementaryMetaFileCache::Pointer metaFileCache = NULL;
    if (preCacheMetaInfoFiles)
      metaFileCache = ComplementaryMetaFileCache::CreateFromRTIList(m_SliceList);
    m_MetaInfo->SetComplementaryMetaFileCache(metaFileCache);
  }
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::SelectAvailableSliceStack(StrLstType *stack)
{
  if (!stack) // clear selection requested
  {
    m_SliceList.clear();
    return true;
  }

  if (m_AvailableSliceStacks.size() > 0)
  {
    // this loop is for verification (even if stack already points to the list):
    for (unsigned int i = 0; i < m_AvailableSliceStacks.size(); i++)
    {
      if (stack == m_AvailableSliceStacks[i])
      {
        m_SliceList.clear();
        for (unsigned int j = 0; j < stack->size(); j++)
          m_SliceList.push_back((*stack)[j]);
        return true;
      }
    }
    return false;
  }
  else
    return false;
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::SelectAvailableSliceStack(unsigned int index)
{
  if (m_AvailableSliceStacks.size() > 0 &&
      index < m_AvailableSliceStacks.size())
  {
    StrLstType *stack = m_AvailableSliceStacks[index];
    m_SliceList.clear();
    for (unsigned int j = 0; j < stack->size(); j++)
      m_SliceList.push_back((*stack)[j]);
    return true;
  }
  else
    return false;
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::BuildImageFromSlices(int expectedMinimumNumberOfSlices)
{
  bool result = false;

  m_Image = NULL; // set back the image in each case!

  if (ValidateSlices(true))
  {
    // Check whether the expected minimum number of slices is reached after all
    // the different validation steps (only if required, >0):
    if (expectedMinimumNumberOfSlices > 0)
    {
      if (m_SliceList.size() < (unsigned int)expectedMinimumNumberOfSlices)
        return false; // fail!
    }

    typedef itk::ImageSeriesReader<ImageType> ReaderType;
    typedef typename ReaderType::Pointer ReaderPointer;
    typedef itk::RTIImageIO IOType;
    typedef IOType::Pointer IOPointer;

    IOPointer io = IOType::New();
    ReaderPointer reader = ReaderType::New();

    reader->SetUseStreaming(true);
    // ensure that meta information object is up-to-date:
    Initialize(false); // meta information not already pre-cached will be
                       // automatically be loaded during image loading
    io->SetImageSeriesReaderMode(true); // specified behavior!
    io->SetMetaInfo(m_MetaInfo); // connect the meta-information object
    io->SetSequencerHelpFlag(true); // important
    reader->SetImageIO(io);
    reader->SetFileNames(m_SliceList);
    try
    {
      reader->Update();
      m_Image = reader->GetOutput();
      if (m_Image)
        m_Image->DisconnectPipeline();
    }
    catch (itk::ExceptionObject &e)
    {
      SimpleErrorMacro(<< "Error during RTI reading (series): " << e);
    }

    if (m_Image)
    {
      // some meta-post-processing is necessary
      VolumeMetaInformation::Pointer volMI = m_MetaInfo->GetVolumeMetaInfo();

      // -> take over resultant 3D-spacing
      typename ImageType::SpacingType spac = m_Image->GetSpacing();
      ora::VolumeMetaInformation::SpacingType spac2;
      for (int i = 0; i < 3; i++)
        spac2[i] = spac[i];
      volMI->SetSpacing(spac2);

      // -> take over resultant 3D-dimension
      typename ImageType::SizeType sz = m_Image->GetLargestPossibleRegion().
        GetSize();
      ora::VolumeMetaInformation::SizeType sz2;
      for (int i = 0; i < 3; i++)
        sz2[i] = sz[i];
      volMI->SetSize(sz2);

      // -> compute anatomical descriptions of the main directions (DICOM LPS)
      char *orient = NULL;
      ora::VolumeMetaInformation::StringArrayType orientation;
      for (int i = 0; i < 3; ++i)
      {
        double dir[3];
        for (int j = 0; j < 3; j++)
          dir[j] = m_Image->GetDirection()[i][j];
        orient = GetOrientationFromVector(dir, true);
        orientation[i] = std::string(orient);
        delete [] orient;
      }
      volMI->SetAnatomicalOrientation(orientation);

      result = true;
    }

    io = NULL;
  }
  else
    VeryDetailedDebugMacro(<< "Could not validate set slice list.");

  return result;
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::SaveImage(std::string fileName, bool compress,
    std::string intermediateDirectory)
{
  if (!m_Image || !m_MetaInfo)
    return false;

  EnsureStringEndsWith(fileName, ".mhd");
  Trim(intermediateDirectory);

  if (fileName.length() < 5 || !m_Image)
    return false;

  itk::ImageIOBase::IOComponentType ct = ITKVTKImage::GetComponentTypeFromTemplate<TPixelType>();
  if (ct == itk::ImageIOBase::UNKNOWNCOMPONENTTYPE)
  {
    SimpleErrorMacro(<< "Error during save image (UNKNOWN COMPONENT TYPE): " <<
        fileName << ", " << compress << ", " << intermediateDirectory);
    return false;
  }
  // Create Image
  ITKVTKImage *img = new ITKVTKImage(itk::ImageIOBase::SCALAR, ct);

  // Set the image data (up-cast to base-class)
  ITKVTKImage::ITKImagePointer ip = static_cast<ITKVTKImage::ITKImageType *>(m_Image.GetPointer());
  img->SetITKImage(ip, false); // set image: (ITK -> VTK if necessary)
  ip = NULL;
  img->SetMetaInfo(m_MetaInfo);

  // Save image
  bool ret = false;
  TEMPLATE_CALL_COMP(img->GetComponentType(), ret = img->SaveImageAsORAMetaImage,
      fileName, compress, intermediateDirectory)

  delete img;

  return ret;
}

template <typename TPixelType>
void
RTIToMHDConverter<TPixelType>
::CleanUp()
{
  if (m_MetaInfo && m_MetaInfo->GetReferenceCount() == 2 &&
      m_MetaInfo->GetVolumeMetaInfo())
    m_MetaInfo->GetVolumeMetaInfo()->SetParent(NULL);
  m_MetaInfo = NULL;
  m_Image = NULL;
}

template <typename TPixelType>
bool
RTIToMHDConverter<TPixelType>
::IsStackable(std::string acquisitionType)
{
  StrLstType::iterator it;

  for (it = m_StackableAcquisitionTypes.begin();
    it != m_StackableAcquisitionTypes.end(); ++it)
    if (*it == acquisitionType)
      return true;

  return false;
}

template <typename TPixelType>
void
RTIToMHDConverter<TPixelType>
::ClearAvailableSliceStacks()
{
  StrLstVecType::iterator it;

  for (it = m_AvailableSliceStacks.begin();
    it != m_AvailableSliceStacks.end(); ++it)
    if (*it)
      delete *it;

  for (unsigned int i = 0; i < m_AvailableSliceStacksMeta.size(); ++i)
  {
    if (m_AvailableSliceStacksMeta[i])
    {
      for (unsigned int j = 0; j < (*m_AvailableSliceStacksMeta[i]).size(); ++j)
      {
        if ((*m_AvailableSliceStacksMeta[i])[j])
          delete (*m_AvailableSliceStacksMeta[i])[j];
      }
      m_AvailableSliceStacksMeta[i]->clear();
      delete m_AvailableSliceStacksMeta[i];
    }
  }

  m_AvailableSliceStacks.clear();
  m_AvailableSliceStacksMeta.clear();
}

template <typename TPixelType>
void
RTIToMHDConverter<TPixelType>
::GenerateSortedAvailableSliceStacks(MetaVectorType &compactMetaVector,
    bool ascending)
{
  // a synchronized slice stack information vector to make comparisons easier
  // (just pointers!):
  std::vector<bool> stackModified;

  stackModified.clear();
  ClearAvailableSliceStacks();

  const double epsilon = 0.001; // for floating point comparisons
  const double onePlusEpsilon = 1. + epsilon;
  const double oneMinusEpsilon = 1. - epsilon;

  MetaIteratorType it;
  MetaVectorType *actualMetaVector = NULL;

  actualMetaVector = &compactMetaVector; // start with the parametrized vec

  do
  {
    for (it = actualMetaVector->begin(); it != actualMetaVector->end(); ++it)
    {
      if (*it)
      {
        CompactRTISliceMetaInfo *mi = *it;

        if (m_AvailableSliceStacksMeta.size() > 0)
        {
          bool inserted = false;

          if (IsStackable(mi->AcquisitionType)) // makes only sense if stackable!
          {
            for (unsigned int i = 0; i < m_AvailableSliceStacksMeta.size(); ++i)
            {
              bool avoidThisStack = false;

              // check whether the examined stack is in the 'avoid'-list:
              for (unsigned int j = 0;
                j < mi->AvoidTheseMetaVectorsForInsertion.size(); ++j)
              {
                if (mi->AvoidTheseMetaVectorsForInsertion[j] ==
                    m_AvailableSliceStacksMeta[i])
                {
                  avoidThisStack = true;
                  break;
                }
              }
              if (avoidThisStack)
                continue; // resume with the next available stack

              // ... it is clear that each m_AvailableSliceStacksMeta-entry has at
              // least one item, therefore element 0 can be accessed without
              // verification!
              CompactRTISliceMetaInfo *ref = (*m_AvailableSliceStacksMeta[i])[0];

              // must have the same Acquisition Type:
              if (ref->AcquisitionType != mi->AcquisitionType)
                continue;

              // must have the same Frame of Reference:
              if (ref->FORUID != mi->FORUID)
                continue;

              // must have the same Slicing Direction (and Row Direction):
              // the slicing direction is the same, if the scalar product equals
              // the ordinary product of the vector-norms (1 for normalized vecs):
              double par = ref->SlicingDirectionNorm * mi->SlicingDirectionNorm;
              if (par > onePlusEpsilon || par < oneMinusEpsilon)
                continue;
              par = ref->RowDirectionNorm * mi->RowDirectionNorm;
              if (par > onePlusEpsilon || par < oneMinusEpsilon)
                continue;

              // the Slice Origins must coincide w.r.t. the slicing direction:
              CompactRTISliceMetaInfo::VectorType dir = ref->SliceOrigin -
                mi->SliceOrigin;
              dir.Normalize();
              par = fabs(dir * mi->SlicingDirectionNorm);
              if (par > onePlusEpsilon || par < oneMinusEpsilon)
                continue;

              // the Slice Size must be equal:
              if (!m_UseImageListInformationOnlyForMetaSorting)
              {
                // cannot be checked if sorting is based on image list only!
                if (mi->SliceSize[0] != ref->SliceSize[0] ||
                    mi->SliceSize[1] != ref->SliceSize[1])
                  continue;
              }

              // only fitting slices arrive here:
              m_AvailableSliceStacksMeta[i]->push_back(mi);
              stackModified[i] = true; // this stack is now modified
              inserted = true;
              break;
            }
          }

          if (!inserted) // new volume stack
          {
            MetaVectorType *mv = new MetaVectorType();
            mv->push_back(mi);
            m_AvailableSliceStacksMeta.push_back(mv);
            stackModified.push_back(true); // new stacks are modified!
          }
        }
        else // empty - simply add the first item
        {
          MetaVectorType *mv = new MetaVectorType();
          mv->push_back(mi);
          m_AvailableSliceStacksMeta.push_back(mv);
          stackModified.push_back(true); // new stacks are modified!
        }
      }
    }

    // sort the slices in each potential stack:
    for (unsigned int i = 0; i < m_AvailableSliceStacksMeta.size(); ++i)
    {
      if (!stackModified[i]) // unmodified stacks need not to be (re-)sorted!
        continue;

      MetaVectorType *mv = m_AvailableSliceStacksMeta[i];

      if (mv->size() <= 1) // do not need sorting
        continue;

      if (ascending)
        std::sort(mv->begin(), mv->end(), CompareRTIFileMetaInfoAscending);
      else
        std::sort(mv->begin(), mv->end(), CompareRTIFileMetaInfoDescending);
    }

    // now check if  there are slices that share the same slice position and
    // remove such slices from the stacks:
    MetaVectorType *droppedSlices = NULL;
    std::string baseFN; // base file name for more detailed RTI-separation

    for (unsigned int i = 0; i < m_AvailableSliceStacksMeta.size(); ++i)
    {
      if (stackModified[i])
      {
        bool modified = false;
        MetaVectorType *mv = m_AvailableSliceStacksMeta[i];
        MetaIteratorType mi;
        MetaIteratorType nmi; // points to next position of mi
        baseFN = ""; // initialize
        for (nmi = mv->begin(), ++nmi, mi = mv->begin(); nmi != mv->end(); /**/)
        {
          double diff = fabs((*mi)->ComputeSlicePositionIfNecessary() -
                             (*nmi)->ComputeSlicePositionIfNecessary());
          if (diff > epsilon) // not same slice position!
          {
            ++mi;
            ++nmi;
          }
          else // same slice position - problem
          {
            if (!droppedSlices)
              droppedSlices = new MetaVectorType();

            if (baseFN.length() <= 0) // set it arbitrarily
              baseFN = (*mi)->GenerateCleanFileNameIfNecessary();

            // drop the next item if this item matches the base file name
            // pattern:
            if ((*mi)->GenerateCleanFileNameIfNecessary() == baseFN)
            {
              (*nmi)->AvoidTheseMetaVectorsForInsertion.push_back(
                  m_AvailableSliceStacksMeta[i]); // do not try to add this slice in
                                                // this stack again
              droppedSlices->push_back(*nmi); // add item to a separate slice list
              mv->erase(nmi); // remove the item from erroneous stack
            }
            else
            {
              (*mi)->AvoidTheseMetaVectorsForInsertion.push_back(
                  m_AvailableSliceStacksMeta[i]); // do not try to add this slice in
                                                // this stack again
              droppedSlices->push_back(*mi); // add item to a separate slice list
              mv->erase(mi); // remove the item from erroneous stack
            }

            modified = true; // mark modification
            // do not increment the iterators to enable further checks on position
          }
        }

        stackModified[i] = modified; // update modified-state for next iteration
      }
    }

    if (actualMetaVector != &compactMetaVector) // free previous drop lists
    {
      delete actualMetaVector;
      actualMetaVector = NULL;
    }
    else // do not free the list given in the argument
      actualMetaVector = NULL;

    actualMetaVector = droppedSlices; // eventually new list for next iteration
  } while (actualMetaVector);


  // take over the meta slice stacks in the form of file names (strings):
  for (unsigned int i = 0; i < m_AvailableSliceStacksMeta.size(); ++i)
  {
    MetaVectorType *mv = m_AvailableSliceStacksMeta[i];
    if (!mv)
      continue;

    StrLstType *strLst = new StrLstType();
    for (unsigned int j = 0; j < mv->size(); ++j)
    {
      strLst->push_back((*mv)[j]->FileName);
    }
    m_AvailableSliceStacks.push_back(strLst);
  }
}


}


#endif /* ORARTITOMHDCONVERTER_TXX */
