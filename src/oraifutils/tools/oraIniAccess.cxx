

#include "oraIniAccess.h"

#include <iostream>
#include <fstream>
#include <math.h>

#include "oraFileTools.h"

#include <itksys/SystemTools.hxx>




namespace ora
{


IniAccessSectionEntry
::IniAccessSectionEntry(std::string sectionName)
{
  m_SectionName = sectionName;
  m_Indents = new IndentMapType;
}

IniAccessSectionEntry
::~IniAccessSectionEntry()
{
  m_Indents->clear();
  delete m_Indents;
}

void
IniAccessSectionEntry
::AddIndent(std::string indent, std::string value)
{
  (*m_Indents)[indent] = value;
}

void
IniAccessSectionEntry
::DeleteIndent(std::string indent)
{
  IndentMapType::iterator cursor = m_Indents->find(indent);
  m_Indents->erase(cursor);
}

IniAccessSectionEntry
::IndentMapType *IniAccessSectionEntry::GetIndents()
{
  return m_Indents;
}

std::string
IniAccessSectionEntry
::GetSectionName()
{
  return m_SectionName;
}




IniAccess
::IniAccess()
{
  Init();
}

IniAccess
::IniAccess(std::string fileName)
{
  Init();
  m_Filename = fileName;
  LoadValues();
}

IniAccess
::~IniAccess()
{
  DiscardChanges();
  for(unsigned int i = 0; i < m_Sections->size(); ++i)
    delete (*m_Sections)[i];
  m_Sections->clear();
  delete m_Sections;
  m_PreHeader->clear();
  delete m_PreHeader;
}

void
IniAccess
::AddPreHeader(const std::string newHeaderEntry)
{
  m_PreHeader->push_back(newHeaderEntry);
  m_ContentChanged = true;
}

bool
IniAccess
::DeleteComments(const std::string section)
{
  bool result = false;

  if (section.length() > 0)
  {
    for (unsigned int i = 0; i < m_Sections->size(); ++i)
    {
      IniAccessSectionEntry *sect = (*m_Sections)[i];
      if (sect->GetSectionName() == section)
      {
        IniAccessSectionEntry::IndentMapType *indents = sect->GetIndents();
        IniAccessSectionEntry::IndentMapType::iterator it;
        for (it = indents->begin(); it != indents->end(); ++it)
        {
          if (it->first[0] == '[')
          {
            indents->erase(it);
            result = true;
          }
        }
        break;
      }
    }
  }
  if (result)
    m_ContentChanged = true;

  return result;
}

bool
IniAccess
::DeleteIndent(const std::string section, const std::string indent)
{
  bool result = false;
  bool isAComment = false;

  if ((indent.length() >= 2 && indent[0] == '/' && indent[1] == '/') ||
      (indent.length() >= 1 && indent[0] == ';'))
    isAComment = true;

  if (section.length() > 0 && !isAComment)
  {
    for (unsigned int i = 0; i < m_Sections->size(); ++i)
    {
      IniAccessSectionEntry *sect = (*m_Sections)[i];
      if (sect->GetSectionName() == section)
      {
        IniAccessSectionEntry::IndentMapType::iterator it = sect->GetIndents()->find(indent);
        if (it != sect->GetIndents()->end())
        {
          sect->GetIndents()->erase(it);
          result = true;
        }

        break;
      }
    }
  }
  if (result)
    m_ContentChanged = true;

  return result;
}

bool
IniAccess
::DeletePreHeader()
{
  if (m_PreHeader->size() > 0)
  {
    m_PreHeader->clear();
    m_ContentChanged = true;

    return true;
  }
  else
    return false;
}

bool
IniAccess
::DeleteSection(const std::string section)
{
  bool result = false;

  if (section.length() > 0)
  {
    for (unsigned int i = 0; i < m_Sections->size(); ++i)
    {
      IniAccessSectionEntry *sect = (*m_Sections)[i];
      if (sect->GetSectionName() == section)
      {
        m_Sections->erase(m_Sections->begin() + i);
        result = true;
        break;
      }
    }
  }
  if (result)
    m_ContentChanged = true;

  return result;
}

void
IniAccess
::DiscardChanges()
{
  m_ContentChanged = false;
}

std::string IniAccess::GetFileName()
{
  return m_Filename;
}

std::vector<std::string> *
IniAccess
::GetComments(const std::string section)
{
  std::vector<std::string> *result = new std::vector<std::string>;

  if (section.length() > 0)
  {
    for (unsigned int i = 0; i < m_Sections->size(); ++i)
    {
      IniAccessSectionEntry *sect = (*m_Sections)[i];
      if (sect->GetSectionName() == section)
      {
        IniAccessSectionEntry::IndentMapType *indents = sect->GetIndents();
        IniAccessSectionEntry::IndentMapType::iterator it;
        for (it = indents->begin(); it != indents->end(); ++it)
        {
          if (it->first[0] == '[')
            result->push_back(it->first.substr(1, it->first.length() - 1));
        }

        break;
      }
    }
  }

  return result;
}

std::vector<std::string> *
IniAccess
::GetIndents(const std::string section)
{
  std::vector<std::string> *result = new std::vector<std::string>;

  if (section.length() > 0)
  {
    for (unsigned int i = 0; i < m_Sections->size(); ++i)
    {
      IniAccessSectionEntry *sect = (*m_Sections)[i];
      if (sect->GetSectionName() == section)
      {
        IniAccessSectionEntry::IndentMapType *indents = sect->GetIndents();
        IniAccessSectionEntry::IndentMapType::iterator it;
        for (it = indents->begin(); it != indents->end(); ++it)
        {
          if (it->first.length() > 0 && it->first[0] != '[')
            result->push_back(it->first);
        }

        break;
      }
    }
  }

  return result;
}

std::vector<std::string> *
IniAccess
::GetPreHeader()
{
  return new std::vector<std::string>(*m_PreHeader);
}

std::vector<std::string> *
IniAccess
::GetSections()
{
  std::vector<std::string> *result = new std::vector<std::string>;

  for (unsigned int i = 0; i < m_Sections->size(); ++i)
  {
    IniAccessSectionEntry *sect = (*m_Sections)[i];
    result->push_back(sect->GetSectionName());
  }

  return result;
}

std::vector<IniAccessSectionEntry *> *
IniAccess
::GetSectionMaps()
{
  return m_Sections;
}

bool
IniAccess
::IsSectionExisting(std::string section)
{
  for (unsigned int i = 0; i < m_Sections->size(); ++i)
  {
    IniAccessSectionEntry *sect = (*m_Sections)[i];

    if (sect->GetSectionName() == section)
      return true;
  }

  return false;
}

bool
IniAccess
::IsIndentExisting(std::string section, std::string indent)
{
  if (section.length() > 0)
  {
    for (unsigned int i = 0; i < m_Sections->size(); ++i)
    {
      IniAccessSectionEntry *sect = (*m_Sections)[i];
      if (sect->GetSectionName() == section)
      {
        IniAccessSectionEntry::IndentMapType::iterator it =
            sect->GetIndents()->find(indent);
        if (it != sect->GetIndents()->end())
          return true;
        else
          return false;
      }
    }

    return false; // obviously not found
  }
  else
    return false;
}

std::string
IniAccess
::ReadString(const std::string section,
  const std::string indent, const std::string defaultValue,
  const bool uncUNIXConversion)
{
  std::string result = defaultValue;
  bool isAComment = false;

  if ((indent.length() >= 2 && indent[0] == '/' && indent[1] == '/') ||
      (indent.length() >= 1 && indent[0] == ';'))
    isAComment = true;

  if (section.length() > 0 && !isAComment)
  {
    for (unsigned int i = 0; i < m_Sections->size(); ++i)
    {
      IniAccessSectionEntry *sect = (*m_Sections)[i];
      if (sect->GetSectionName() == section)
      {
        IniAccessSectionEntry::IndentMapType::iterator it =
          sect->GetIndents()->find(indent);
        if (it != sect->GetIndents()->end())
          result = it->second;

        break;
      }
    }
  }

  if (( uncUNIXConversion || m_InvokeUNCvsUNIXOnRead) && result.length() > 0)
    result = UnixUNCConverter::GetInstance()->EnsureOSCompatibility(result);

  return result;
}

void
IniAccess
::SetFileName(std::string fileName)
{
  if (m_Filename != fileName)
  {
    m_Filename = fileName;
    m_ContentChanged = true;
  }
}

bool
IniAccess
::Update()
{
  if (m_ContentChanged)
  {
    if (!itksys::SystemTools::FileExists(m_Filename.c_str()))
    {
      // make sure that folder exists:
      itksys_stl::string s = itksys::SystemTools::GetFilenamePath(m_Filename.c_str());
      if (s.length() > 0)
      {
        if (!itksys::SystemTools::MakeDirectory(s.c_str()))
          return false;
      } // else: eventually in current application-folder
    }

    // generate check sum on demand!
    GenerateORACheckSum();

    std::ofstream ofile(m_Filename.c_str());
    PrintOut(ofile);
    ofile.close();
    m_ContentChanged = false; // set back

    return true;
  }
  else
    return false;
}

bool
IniAccess
::WriteString(const std::string section, const std::string indent,
  const std::string value)
{
  bool result = false;
  IniAccessSectionEntry *sect = NULL;

  if (section.length() > 0)
  {
    for (unsigned int i = 0; i < m_Sections->size(); ++i)
    {
      sect = (*m_Sections)[i];
      if (sect->GetSectionName() == section)
      {
        if (indent.length() > 0)
          sect->AddIndent(indent, value);
        result = true;

        break;
      }
    }
    if (!result) // section does not exist yet
    {
      sect = new IniAccessSectionEntry(section);
      m_Sections->push_back(sect);
      if (indent.length() > 0)
        sect->AddIndent(indent, value);
      result = true;
    }
  }

  if (result)
    m_ContentChanged = true;

  return result;
}

void
IniAccess
::LoadValues()
{
  std::string s;
  IniAccessSectionEntry *sect = NULL;

  if (m_Filename == "")
    return;
  if (!itksys::SystemTools::FileExists(m_Filename.c_str())) // seems to be a new file
    return;
  m_Sections->clear();
  std::fstream ifile(m_Filename.c_str(), std::ios::in);
  for (std::string line; std::getline(ifile, line); )
  {
    Trim(line);

    // NOTE: traditionally the following line-breaks are common:
    // * UNIX-derivates: 0xA ("\n" = line feed)
    // * MAC: 0xD ("\r" = carriage return)
    // * MSWINDOWS: 0xD0xA ("\r\n" = carriage return line feed)
    if (line.length() > 0 && line[line.length() - 1] == '\xA') // cut line feed
      line = line.substr(0, line.length() - 1);
    if (line.length() > 0 && line[line.length() - 1] == '\xD') // cut carriage return
      line = line.substr(0, line.length() - 1);

    if (line.length() > 0 &&
        line[0] == '[' &&
        line[line.length() - 1] == ']') // section
    {
      s = line.substr(1, line.length() - 2);
      sect = new IniAccessSectionEntry(s);
      m_Sections->push_back(sect);
    }
    else // indent or comment
    {
      if (sect == NULL)
      {
        if (line.length() > 0)
          m_PreHeader->push_back(line);

        continue;
      }

      if (line[0] != ';' &&
          line.substr(0, 2) != "//" &&
          line.find("=", 0) > 0 &&
          line.find("=", 0) < line.length()) // indent [with value]
      {
        sect->AddIndent(line.substr(0, line.find("=", 0)),
                        line.substr(line.find("=", 0) + 1, line.length()));
      }
      else if (line.length() > 0) // comment
      {
        sect->AddIndent("[" + line, "");
      }
    }
  }

  ifile.close();
}

void
IniAccess
::PrintOut(std::ostream &os)
{
  os << std::endl;
  for (unsigned int i = 0; i < m_PreHeader->size(); ++i)
    os << (*m_PreHeader)[i] << std::endl;

  for (unsigned int i = 0; i < m_Sections->size(); ++i)
  {
    IniAccessSectionEntry *section = (*m_Sections)[i];
    os << std::endl << "[" << section->GetSectionName() << "]" << std::endl;
    IniAccessSectionEntry::IndentMapType *indents = section->GetIndents();
    IniAccessSectionEntry::IndentMapType::iterator it;

    for (it = indents->begin(); it != indents->end(); ++it)
    {
      if (it->first.length() <= 0)
        continue;

      if (it->first[0] != '[') // no comment
        os << it->first << "=" << it->second << std::endl;
      else // comment
        os << it->first.substr(1, it->first.length() - 1) << std::endl;
    }
  }

  os << std::endl;
}

void
IniAccess
::Init()
{
  m_Filename = "";
  m_ContentChanged = false;
  m_Sections = new std::vector<IniAccessSectionEntry *>;
  m_PreHeader = new std::vector<std::string>;
  m_InvokeUNCvsUNIXOnRead = false; // deactivate this feature by default
  m_AddORACheckSum = false; // no CS by default
}

void
IniAccess
::GenerateORACheckSum()
{
  if (!m_AddORACheckSum)
    return;

  // ensure that we have a full file path
  std::string fn = m_Filename;
  if (!itksys::SystemTools::FileIsFullPath(fn.c_str()))
  {
    std::string cwd = itksys::SystemTools::GetCurrentWorkingDirectory();
    fn = itksys::SystemTools::CollapseFullPath(fn.c_str(), cwd.c_str());
  }
  // get the file name without drive information
  fn = UnixUNCConverter::GetInstance()->EnsureUNCCompatibility(
      fn); // ensure that we have a UNX-path!
  fn = UnixUNCConverter::DiscardWinFileDriveInformation(fn);
  ora::ToUpperCase(fn);

  if (fn.length() <= 0)
    return;

  // be sure that the check sum section is correctly initialized
  this->WriteValue<int>("File", "CheckSum", 0);

  // however, there is a strange open radART static key:
  std::string key = "donaudampfschifffahrtsgesellschaft";

  // print current content to string stream and analyse the single lines
  std::ostringstream os;
  this->PrintOut(os);
  std::size_t p1 = 0;
  std::size_t p2 = 0;
  std::string content = os.str();
  std::string line;
  std::vector<std::string> encStrings;
  do
  {
    p1 = p2;
    p2 = content.find("\n", p2 + 1); // search LF
    if (p2 > p1 && p2 != std::string::npos)
    {
      line = content.substr(p1 + 1, p2 - p1 - 1);
      if (line.length() > 0 && line[line.length()] == '\r') // windows (CR)
        line = line.substr(0, line.length() - 1);
      // line must not be empty nor a comment
      if (line.length() > 0 && line[0] != ';') // NOTE: / are considered!
        encStrings.push_back(line);
    }
  } while (p2 > p1 && p2 != std::string::npos);
  encStrings.push_back(fn); // additionally consider the cleaned file name

  // encode the lines
  long int cs = 0;
  int kl = key.length();
  const int tttpot = (int)pow(2.0, 30.0); // two to the power of thirty
  for (unsigned int i = 0; i < encStrings.size(); ++i)
  {
    line = encStrings[i];
    for (unsigned int j = 0; j < line.length(); ++j)
      cs += (unsigned char)key[j % kl] ^ (unsigned char)line[j];

    // do not calculate the modulo in inner loop ('d be too often); just
    // call it in outer loop - there is enough "space" between 2^30 and
    // 2^31-1 (=upper long-limit)!
    cs = cs % tttpot;
  }

  // update actual check sum
  this->WriteValue<int>("File", "CheckSum", cs);
}


}
