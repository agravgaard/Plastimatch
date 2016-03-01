
#include "oraCompactRTISliceMetaInfo.h"

#include <vector>

// ORAIFTools
#include "oraFileTools.h"

// Forward declarations
#include "oraIniAccess.h"

namespace ora 
{


CompactRTISliceMetaInfo
::CompactRTISliceMetaInfo()
{
  FileName = "";
  FORUID = "";
  AcquisitionType = "";
  SliceSize.Fill(0.);
  SliceOrigin.Fill(0.);
  SlicingDirectionNorm.Fill(0.);
  SlicingDirectionNorm[2] = 1.; // default right-handed x/y/z
  SlicingDirectionNorm.Normalize();
  RowDirectionNorm.Fill(0.);
  RowDirectionNorm[0] = 1.; // default right-handed x/y/z
  RowDirectionNorm.Normalize();
  m_ComputedSlicePositionValid = false;
  AvoidTheseMetaVectorsForInsertion.clear();
  m_CleanFileName = "";
}

CompactRTISliceMetaInfo
::~CompactRTISliceMetaInfo()
{
  AvoidTheseMetaVectorsForInsertion.clear();
}

double
CompactRTISliceMetaInfo
::ComputeSlicePositionIfNecessary(bool force)
{
  if (!m_ComputedSlicePositionValid || force)
  {
    m_ComputedSlicePosition = SlicingDirectionNorm *
        SliceOrigin.GetVectorFromOrigin();
    m_ComputedSlicePositionValid = true;
  }

  return m_ComputedSlicePosition;
}

std::string
CompactRTISliceMetaInfo
::GenerateCleanFileNameIfNecessary(bool force)
{
  if (m_CleanFileName.length() <= 0 || force)
  {
    // RTI file name convention: (especially for stacks)
    // [<enumerator>-]<some-structured-descriptor><+ or -><decimal-position>.rti
    // => make clean: [<enumerator>-]<some-structured-descriptor>
    std::string sign = "+-";
    std::string::size_type pos = FileName.find_last_of(sign);
    if (pos != std::string::npos)
      m_CleanFileName = FileName.substr(0, pos);
    else // simply take over file name
      m_CleanFileName = FileName;
  }
  return m_CleanFileName;
}

CompactRTISliceMetaInfo *
CompactRTISliceMetaInfo
::CreateFromCachedInformation(IniAccess *cachedMetaFile,
    ImageList::Pointer imageList)
{
  if (!cachedMetaFile || !imageList)
    return NULL;

  CompactRTISliceMetaInfo *result = new CompactRTISliceMetaInfo();

  // FileName: auto-rename the *.inf file into *.rti
  std::string fn = cachedMetaFile->GetFileName();
  std::string::size_type p = fn.find_last_of(".");
  if (p != std::string::npos)
  {
    result->FileName = fn.substr(0, p + 1);
    if (result->FileName.length() >= 5 &&
        ToLowerCaseF(result->FileName.substr(result->FileName.length() - 5, 5)) == ".rti.")
      result->FileName += "org";
    else
      result->FileName += "rti";
  }
  else
    result->FileName = ""; // invalid
  result->FileName = UnixUNCConverter::GetInstance()->EnsureOSCompatibility(
      result->FileName);

  // Referenced Frame of Reference:
  ImageListEntry::Pointer imgEntry = imageList->FindEntry(result->FileName);
  if (imgEntry && imgEntry->GetReferencedFOR())
    result->FORUID = imgEntry->GetReferencedFOR()->GetUID();
  // else: if all slices do not reference a FOR, it is also 'OK'

  // Slice Acquisition Type:
  if (imgEntry)
    result->AcquisitionType = imgEntry->GetAcquisitionType();
  else // maybe available via meta info file
    result->AcquisitionType = cachedMetaFile->ReadString("Info", "AcquTypeID",
        "");

  // Slice Size:
  double le = cachedMetaFile->ReadValue<double>("Metrics", "Left", 0.);
  double ri = cachedMetaFile->ReadValue<double>("Metrics", "Right", 0.);
  double to = cachedMetaFile->ReadValue<double>("Metrics", "Top", 0.);
  double bo = cachedMetaFile->ReadValue<double>("Metrics", "Bottom", 0.);
  result->SliceSize[0] = (ri - le) * 10.; // cm -> mm !!!
  result->SliceSize[1] = (to - bo) * 10.; // cm -> mm !!!

  // Slice Origin:
  std::string originStr = cachedMetaFile->ReadString("Metrics",
      "ImagePosition", "");
  std::vector<std::string> origin;
  ora::TokenizeIncludingEmptySpaces(originStr, origin, ",");
  if (origin.size() >= 3)
  {
    result->SliceOrigin[0] = atof(origin[0].c_str()) * 10.; // cm -> mm
    result->SliceOrigin[1] = atof(origin[1].c_str()) * 10.; // cm -> mm
    result->SliceOrigin[2] = atof(origin[2].c_str()) * 10.; // cm -> mm
  }

  // Slicing Direction and Row Direction: the information in the meta info file
  // should be more reliable ...
  // "R11,R12,R13,R21,R22,R23"
  std::vector<std::string> direction;
  std::string directionStr = cachedMetaFile->ReadString("Metrics",
      "ImageOrientation", "");
  ora::TokenizeIncludingEmptySpaces(directionStr, direction, ",");

  if (direction.size() >= 6)
  {
    VectorType d1;
    for (int i = 0; i < 3; i++)
    {
      result->RowDirectionNorm[i] = atof(direction[i].c_str());
      d1[i] = atof(direction[3 + i].c_str());
    }
    result->SlicingDirectionNorm = itk::CrossProduct(
        result->RowDirectionNorm, d1);
    result->SlicingDirectionNorm.Normalize(); // normalization required!
    result->RowDirectionNorm.Normalize();
  }

  return result;
}

bool
CompactRTISliceMetaInfo
::CreateFromCachedInformation(ImageList::Pointer imageList,
    MetaVectorType &metaSliceList)
{
  if (!imageList)
    return false;

  metaSliceList.clear();

  typedef ImageList::ImageListMapType MapType;
  typedef MapType::iterator IteratorType;

  MapType *imgMap = imageList->GetDirectImageListMap();
  if (!imgMap)
    return false;
  // iterate through all available image list entries in image list and add the
  // information to the metaSliceList (this is different from the approach
  // realized in CreateFromCachedInformation(IniAccess *, ImageList::Pointer)):
  IteratorType it;
  ImageListEntry::Pointer imgEntry = NULL;
  std::string rtiFileName = "";
  for (it = imgMap->begin(); it != imgMap->end(); ++it)
  {
    rtiFileName = UnixUNCConverter::GetInstance()->EnsureOSCompatibility(
        it->first);
    imgEntry = it->second;
    if (rtiFileName.length() > 0 && imgEntry)
    {
      CompactRTISliceMetaInfo *metaSlice = new CompactRTISliceMetaInfo();

      // RTI File Name:
      metaSlice->FileName = rtiFileName;

      // Referenced Frame of Reference:
      if (imgEntry->GetReferencedFOR())
        metaSlice->FORUID = imgEntry->GetReferencedFOR()->GetUID();
      // else: if all slices do not reference a FOR, it is also 'OK'

      // Slice Acquisition Type:
      metaSlice->AcquisitionType = imgEntry->GetAcquisitionType();

      // Slice Size:
      // NOT AVAILABLE IN THIS MODE!

      // Slice Origin:
      metaSlice->SliceOrigin = imgEntry->GetImagePosition();

      // Slicing Direction and Row Direction:
      MatrixType orientation = imgEntry->GetImageOrientation();
      VectorType d1;
      for (int i = 0; i < 3; i++)
      {
        metaSlice->RowDirectionNorm[i] = orientation[0][i];
        metaSlice->SlicingDirectionNorm[i] = orientation[2][i];
      }
      metaSlice->SlicingDirectionNorm.Normalize(); // normalization required!
      metaSlice->RowDirectionNorm.Normalize();

      metaSliceList.push_back(metaSlice);
    }
  }

  if (metaSliceList.size() > 0)
    return true;
  else
    return false;
}


}
