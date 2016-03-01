

#include "oraComplementaryMetaFileCache.h"

#include <itksys/SystemTools.hxx>

// ORAIFTools
#include "oraFileTools.h"

// Forward declarations
#include "oraIniAccess.h"

namespace ora 
{


ComplementaryMetaFileCache
::ComplementaryMetaFileCache()
{
  Clear();
}

ComplementaryMetaFileCache
::~ComplementaryMetaFileCache()
{
  Clear();
}

void
ComplementaryMetaFileCache
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Meta File Cache (n=" << m_Map.size() << "):" <<
    std::endl;
  if (m_Map.size() > 0)
  {
    MapType::const_iterator it;
    for (it = m_Map.begin(); it != m_Map.end(); ++it)
      os << indent.GetNextIndent() << it->first << " (" <<
        it->second << ")" << std::endl;
  }
  else
    os << indent.GetNextIndent() << "<no entries>" << std::endl;
}

ComplementaryMetaFileCache::Pointer
ComplementaryMetaFileCache
::CreateFromRTIList(std::vector<std::string> rtiList)
{
  std::vector<std::string>::iterator it;
  Pointer ret = Self::New();

  for (it = rtiList.begin(); it != rtiList.end(); ++it)
  {
    std::string fn = *it;

    if (fn.length() > 4)
    {
      fn = fn.substr(0, fn.length() - 3) + "inf";
      if (itksys::SystemTools::FileExists(fn.c_str()))
        ret->AddEntry(new IniAccess(fn));
    }
  }

  if (ret->GetCachedMetaFileEntries().size() <= 0)
    ret = NULL;

  return ret;
}

IniAccess *
ComplementaryMetaFileCache
::FindEntry(const std::string fileName)
{
  return m_Map[UnixUNCConverter::GetInstance()->EnsureOSCompatibility(
      fileName)];
}

bool
ComplementaryMetaFileCache
::AddEntry(IniAccess *cachedMetaFile)
{
  if (cachedMetaFile)
  {
    std::pair<MapType::iterator, bool>
      ret = m_Map.insert(std::make_pair(
          UnixUNCConverter::GetInstance()->EnsureOSCompatibility(
          cachedMetaFile->GetFileName()), cachedMetaFile));

    return ret.second;
  }

  return false;
}

bool
ComplementaryMetaFileCache
::RemoveEntry(IniAccess *cachedMetaFile)
{
  if (cachedMetaFile)
    return this->RemoveEntry(cachedMetaFile->GetFileName());

  return false;
}

bool
ComplementaryMetaFileCache
::RemoveEntry(std::string fileName)
{
  fileName = UnixUNCConverter::GetInstance()->EnsureOSCompatibility(fileName);

  if (fileName.length() > 0)
  {
    MapType::iterator it;

    for (it = m_Map.begin(); it != m_Map.end(); ++it)
    {
      if (it->first == fileName)
      {
        m_Map.erase(it);//, it);
        return true;
      }
    }
  }

  return false;
}

void
ComplementaryMetaFileCache
::Clear()
{
  MapType::iterator it;

  for (it = m_Map.begin(); it != m_Map.end(); ++it)
  {
    if (it->second)
      delete it->second;
  }
  m_Map.clear();
}

std::vector<std::string>
ComplementaryMetaFileCache
::GetCachedMetaFileEntries()
{
  std::vector<std::string> ret;
  MapType::iterator it;

  for (it = m_Map.begin(); it != m_Map.end(); ++it)
    ret.push_back(it->first);

  return ret;
}

std::vector<IniAccess *>
ComplementaryMetaFileCache
::GetCachedMetaFileEntryPointers()
{
  std::vector<IniAccess *> ret;
  MapType::iterator it;

  for (it = m_Map.begin(); it != m_Map.end(); ++it)
    ret.push_back(it->second);

  return ret;
}


}

