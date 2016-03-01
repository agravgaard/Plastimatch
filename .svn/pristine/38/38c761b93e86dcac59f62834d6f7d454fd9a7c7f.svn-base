

#include "oraFileTools.h"

#include <iostream>

#include "oraStringTools.h"
#include "oraIniAccess.h"

#include <itksys/SystemTools.hxx>


namespace ora
{


// define the static members (after declaration):
UnixUNCConverter *UnixUNCConverter::m_Instance = 0;

std::string
UnixUNCConverter
::DiscardWinFileDriveInformation(std::string path)
{
  path = ora::TrimF(path);
  if (path.length() >= 2 &&
      path.substr(1, 1) == ":") /* <drive>:\ */
  {
    if (path.length() > 3)
      return path.substr(3);
    else
      return "";
  }
  else if (path.length() >= 2 &&
           path.substr(0, 2) == "\\\\") /* \\server\share\ */
  {
    /* -> search for 3rd \\ */
    std::size_t p = path.find("\\", 3);
    if (p != std::string::npos)
    {
      p = path.find("\\", p + 1);
      if (p != std::string::npos)
        return path.substr(p + 1);
      else // no further components (share only)
        return "";
    }
    else // invalid UNC path
      return "";
  }
  else // invalid or (maybe) a relative path
    return path;
}

UnixUNCConverter
::UnixUNCConverter()
{
  m_UNCToUnixTable.clear();
  m_UnixToUNCTable.clear();
  m_UNCvsUNIXSectionName = "UNCvsUNIX"; // default
}

UnixUNCConverter
::~UnixUNCConverter()
{
  if (m_Instance)
    delete m_Instance;
}

UnixUNCConverter *
UnixUNCConverter
::GetInstance()
{
  if (!m_Instance)
    m_Instance = new UnixUNCConverter();

  return m_Instance;
}

void
UnixUNCConverter
::AddTranslationPair(std::string unc, std::string unx)
{
  // from both directions for better access:
  m_UNCToUnixTable.insert(std::make_pair(ToLowerCaseF(unc), unx));
  m_UnixToUNCTable.insert(std::make_pair(unx, unc));
}

void
UnixUNCConverter
::ClearTranslationPairs()
{
  m_UNCToUnixTable.clear();
  m_UnixToUNCTable.clear();
}

void
UnixUNCConverter
::ListTranslationPairs(std::ostream &os)
{
  int c = 1;

  os << "Pairs (UNC -> UNIX): " << std::endl;
  for(ConversionTableType::iterator it = m_UNCToUnixTable.begin();
    it != m_UNCToUnixTable.end(); ++it, ++c)
    os << c << ": " << it->first << " -> " << it->second << std::endl;
}

bool
UnixUNCConverter
::LoadTranslationPairsFromFile(std::string fileName)
{
  IniAccess ini(fileName);

  // [UNCvsUNIX] section must exist!
  if (!ini.IsSectionExisting(m_UNCvsUNIXSectionName))
    return false;

  std::string unc = ""; // UNC share, e.g. "\\192.168.123.16\Server01"
  std::string unx = ""; // UNIX mount, e.g. "/server01"
  int i = 0;

  do
  {
    ++i;
    unc = ini.ReadString(m_UNCvsUNIXSectionName, "unc" +
        StreamConvert<std::string, int>(i), "");
    unx = ini.ReadString(m_UNCvsUNIXSectionName, "unix" +
        StreamConvert<std::string, int>(i), "");

    if (unc.length() > 0 && unx.length() > 0)
      AddTranslationPair(unc, unx);
  }
  while (unc.length() > 0 && unx.length() > 0);

  return true;
}

std::string
UnixUNCConverter
::EnsureUNIXCompatibility(std::string path)
{
  // look if path starts with known UNC-fragment (case-insensitive)
  std::string lpath = ToLowerCaseF(path);

  ConversionTableType::iterator it;
  for (it = m_UNCToUnixTable.begin(); it != m_UNCToUnixTable.end(); ++it)
  {
    if (lpath.find(it->first) == 0) // starts with UNC-fragment
    {
      std::string tmp = it->second + path.substr(it->first.length());

      itksys::SystemTools::ConvertToUnixSlashes(tmp);
      tmp = itksys::SystemTools::ConvertToUnixOutputPath(tmp.c_str());

      return tmp;
    }
  }

  // in variable 'path' case is not changed!
  itksys::SystemTools::ConvertToUnixSlashes(path);

  return itksys::SystemTools::ConvertToUnixOutputPath(path.c_str());
}

std::string
UnixUNCConverter
::EnsureUNCCompatibility(std::string path)
{
  // look if path starts with known UNIX-fragment

  ConversionTableType::iterator it;
  for (it = m_UnixToUNCTable.begin(); it != m_UnixToUNCTable.end(); ++it)
  {
    if (path.find(it->first) == 0) // starts with UNIX-fragment
    {
      std::string tmp = it->second + path.substr(it->first.length());

      tmp = itksys::SystemTools::ConvertToWindowsOutputPath(tmp.c_str());

      if (tmp.length() >= 2 && tmp[0] == '\"' && tmp[tmp.length() - 1] == '\"')
        tmp = tmp.substr(1, tmp.length() - 2);

      return tmp;
    }
  }

  // in variable 'path' case is not changed!
  std::string tmp = itksys::SystemTools::ConvertToWindowsOutputPath(
      path.c_str());
  if (tmp.length() >= 2 && tmp[0] == '\"' && tmp[tmp.length() - 1] == '\"')
    tmp = tmp.substr(1, tmp.length() - 2);

  return tmp;
}

std::string
UnixUNCConverter
::EnsureOSCompatibility(std::string path)
{
#if (defined(_WIN32) || defined(_WIN64)) && !defined(__CYGWIN__)
  return EnsureUNCCompatibility(path);
#else
  return EnsureUNIXCompatibility(path);
#endif
}


}
