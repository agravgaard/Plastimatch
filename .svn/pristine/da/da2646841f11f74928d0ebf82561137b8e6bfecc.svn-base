

#include "oraImageList.h"

#include <vector>

// ORAIFTools
#include "oraStringTools.h"
#include "oraFileTools.h"
#include "oraIniAccess.h"
#include "SimpleDebugger.h"


namespace ora 
{


ImageListEntry
::ImageListEntry()
{
  m_ImageFileName = "";
  m_AcquisitionType = "";
  m_AcquisitionDate = "";
  m_PatientPosition = "";
  m_ReferencedFOR = NULL;
  m_ImagePosition.Fill(0.);
  m_ImageOrientation.SetIdentity();
  m_Geometry = UNDEFINED;
  m_FocalPoint.Fill(0.);
  m_EyeLine.Fill(0.);
}

ImageListEntry
::~ImageListEntry()
{

}

void
ImageListEntry
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Image File Name:" << std::endl;
  os << indent.GetNextIndent() << "'" << m_ImageFileName << "'" << std::endl;

  os << indent << "Acquisition Type:" << std::endl;
  os << indent.GetNextIndent() << m_AcquisitionType << std::endl;

  os << indent << "Acquisition Date:" << std::endl;
  os << indent.GetNextIndent() << m_AcquisitionDate << std::endl;

  os << indent << "Patient Position:" << std::endl;
  os << indent.GetNextIndent() << m_PatientPosition << std::endl;

  os << indent << "Referenced FOR:" << std::endl;
  if (m_ReferencedFOR)
    m_ReferencedFOR->Print(os, indent.GetNextIndent());
  else
    os << indent.GetNextIndent() << "not set" << std::endl;

  os << indent << "Image Position:" << std::endl;
  os << indent.GetNextIndent() << m_ImagePosition << std::endl;

  os << indent << "Image Orientation:" << std::endl;
  for (int i = 0; i < 3; i++)
    os << indent.GetNextIndent() << m_ImageOrientation[i][0] <<
      " " << m_ImageOrientation[i][1] << " " << m_ImageOrientation[i][2] <<
      std::endl;

  os << indent << "Image Geometry:" << std::endl;
  if (m_Geometry == UNDEFINED)
    os << indent.GetNextIndent() << "UNDEFINED" << std::endl;
  else if (m_Geometry == PERSPECTIVE)
    os << indent.GetNextIndent() << "PERSPECTIVE" << std::endl;
  else if (m_Geometry == ORTHOGRAPHIC)
    os << indent.GetNextIndent() << "ORTHOGRAPHIC" << std::endl;
  else if (m_Geometry == TOPOGRAM)
    os << indent.GetNextIndent() << "TOPOGRAM" << std::endl;
  else if (m_Geometry == SLICE)
    os << indent.GetNextIndent() << "SLICE" << std::endl;

  os << indent << "Focal Point:" << std::endl;
  os << indent.GetNextIndent() << m_FocalPoint << std::endl;

  os << indent << "Eye Line:" << std::endl;
  os << indent.GetNextIndent() << m_EyeLine << std::endl;
}

bool
ImageListEntry
::LoadFromORAString(std::string oraString,
    FORCollPointer forColl)
{

  // FORMAT:
  // <file-name>=
  // <acquisition-type>|<acquisition-date>|<patient-position>|
  // <FOR-UID>|<img-pos-x>,<img-pos-y>,<img-pos-z>|
  // <r11>,<r12>,<r13>,<r21>,<r22>,<r23>|
  // <geometry>|<fp-x>,<fp-y>,<fp-z>|<eyeline-x>,<eyeline-y>,<eyeline-z>

  std::vector<std::string> tok0;

  TokenizeIncludingEmptySpaces(oraString, tok0, "=");

  if (tok0.size() < 2)
    return false;

  tok0[0] = UnixUNCConverter::GetInstance()->EnsureOSCompatibility(
    tok0[0]); // UNC-compatibility!
  this->m_ImageFileName = tok0[0];

  std::vector<std::string> tok1;

  TokenizeIncludingEmptySpaces(tok0[1], tok1, "|");
  if (tok1.size() < 9)
    return false;

  // acquisition:
  m_AcquisitionType = TrimF(tok1[0]);
  m_AcquisitionDate = TrimF(tok1[1]);

  // coordinate system:
  m_PatientPosition = TrimF(tok1[2]);
  ToUpperCase(m_PatientPosition);
  Trim(tok1[3]);
  if (forColl)
    m_ReferencedFOR = forColl->FindFOR(tok1[3]);
  else
    m_ReferencedFOR = NULL;

  // image geometry:
  std::vector<std::string> tok2;

  tok2.clear();
  TokenizeIncludingEmptySpaces(tok1[4], tok2, ",");
  if (tok2.size() < 3)
    return false;
  for (int i = 0; i < 3; ++i)
    this->m_ImagePosition[i] = atof(tok2[i].c_str());

  tok2.clear();
  TokenizeIncludingEmptySpaces(tok1[5], tok2, ",");
  if (tok2.size() < 6)
    return false;
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 3; ++j)
      this->m_ImageOrientation[i][j] = atof(tok2[i * 3 + j].c_str());
  // -> calculate the 3rd cross-vector (right-handed orthogonal system):
  VectorType d0;
  for (int i = 0; i < 3; ++i)
    d0[i] = this->m_ImageOrientation[0][i];
  VectorType d1;
  for (int i = 0; i < 3; ++i)
    d1[i] = this->m_ImageOrientation[1][i];
  VectorType d2 = itk::CrossProduct(d0, d1);
  for (int i = 0; i < 3; ++i)
    this->m_ImageOrientation[2][i] = d2[i];

  // geometry:
  int geometry = -1;

  if (TrimF(tok1[6]).length() > 0)
    geometry = atoi(TrimF(tok1[6]).c_str());
  if (geometry == 0)
    m_Geometry = SLICE;
  else if (geometry == 1)
    m_Geometry = PERSPECTIVE;
  else if (geometry == 2)
    m_Geometry = TOPOGRAM;
  else if (geometry == 3)
    m_Geometry = ORTHOGRAPHIC;
  else
    m_Geometry = UNDEFINED;

  // focal point:
  tok2.clear();
  TokenizeIncludingEmptySpaces(tok1[7], tok2, ",");
  if (tok2.size() < 3)
    return false;
  for (int i = 0; i < 3; ++i)
    this->m_FocalPoint[i] = atof(tok2[i].c_str());

  // eye line:
  tok2.clear();
  TokenizeIncludingEmptySpaces(tok1[7], tok2, ",");

  if (tok2.size() < 3)
    return false;
  for (int i = 0; i < 3; ++i)
    this->m_EyeLine[i] = atof(tok2[i].c_str());

  return true;
}


ImageList
::ImageList()
{
  m_ImageListMap.clear();
}

ImageList
::~ImageList()
{
  m_ImageListMap.clear();
}

void
ImageList
::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  //Superclass::Print(os, indent);

  os << indent << "Image List Entries (n=" << m_ImageListMap.size() << "):" <<
    std::endl;
  if (m_ImageListMap.size() > 0)
  {
    ImageListMapType::const_iterator it;
    for (it = m_ImageListMap.begin(); it != m_ImageListMap.end(); ++it)
      os << indent.GetNextIndent() << it->first << " (" <<
        it->second.GetPointer() << ")" << std::endl;
  }
  else
    os << indent.GetNextIndent() << "<no entries>" << std::endl;
}

ImageList::Pointer
ImageList
::CreateFromFile(std::string imageListFile,
    FrameOfReferenceCollection::Pointer forColl)
{
  imageListFile = UnixUNCConverter::GetInstance()->EnsureOSCompatibility(
    imageListFile); // UNC-compatibility!

  IniAccess ini(imageListFile);

  if (!ini.IsSectionExisting("Images"))
    return NULL;

  std::vector<std::string> *indents = ini.GetIndents("Images");
  std::vector<std::string>::iterator it;

  ImageList::Pointer list = ImageList::New();
  for (it = indents->begin(); it != indents->end(); ++it)
  {
    ImageListEntry::Pointer ile = ImageListEntry::New();

    if (ile->LoadFromORAString(*it + "=" +
        ini.ReadString("Images", *it, "", false), forColl))
      list->AddEntry(ile);
  }
  delete indents;

  if (list->GetImageListEntries().size() <= 0) // no valid entries found
    list = NULL;

  return list;
}

ImageListEntry::Pointer
ImageList
::FindEntry(const std::string fileName)
{
  ImageListEntry::Pointer ret = NULL;
  ImageListMapType::iterator it;
  std::string searchStr = UnixUNCConverter::GetInstance()->
    EnsureOSCompatibility(fileName);
  ora::ToLowerCase(searchStr);

  for (it = m_ImageListMap.begin(); it != m_ImageListMap.end(); ++it)
  {
    if (ora::ToLowerCaseF(it->first) == searchStr)
    {
      ret = it->second;
      if (ret) // only exit if the entry is NOT NULL!
        break;
    }
  }
  return ret;
}

bool
ImageList
::AddEntry(ImageListEntry::Pointer imageListEntry)
{
  if (imageListEntry)
  {
    std::pair<ImageListMapType::iterator, bool>
      ret = m_ImageListMap.insert(std::make_pair(
          UnixUNCConverter::GetInstance()->EnsureOSCompatibility(
          imageListEntry->GetImageFileName()), imageListEntry));

    return ret.second;
  }

  return false;
}

bool
ImageList
::RemoveEntry(ImageListEntry::Pointer imageListEntry)
{
  if (imageListEntry)
    return this->RemoveEntry(imageListEntry->GetImageFileName());

  return false;
}

bool
ImageList
::RemoveEntry(std::string fileName)
{
  fileName = UnixUNCConverter::GetInstance()->EnsureOSCompatibility(fileName);

  if (fileName.length() > 0)
  {
    ImageListMapType::iterator it;

    for (it = m_ImageListMap.begin(); it != m_ImageListMap.end(); ++it)
    {
      if (it->first == fileName)
      {
        m_ImageListMap.erase(it);//, it);
        return true;
      }
    }
  }

  return false;
}

void
ImageList
::Clear()
{
  m_ImageListMap.clear();
}

std::vector<std::string>
ImageList
::GetImageListEntries()
{
  std::vector<std::string> ret;
  ImageListMapType::iterator it;

  for (it = m_ImageListMap.begin(); it != m_ImageListMap.end(); ++it)
  {
    ret.push_back(it->first);
  }

  return ret;
}


}

