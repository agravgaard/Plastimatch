//
#include "reg23info.h"

// ORAIFTools
#include <oraStringTools.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>

/**
 * A simple application that updates the REG23 RC (win32 resource) file
 * with the actual REG23 global information. A variety of keys are
 * supported (see source code).
 *
 * Expects the file path and name of the resource file as first parameter.
 *
 * @author phil
 * @author Markus
 * @version 1.1
 */
int main(int argc, char *argv[])
{
  if (argc < 2)
    return EXIT_SUCCESS;

  std::ifstream ifile(argv[1]);
  std::string line, s, s2;
  std::ostringstream os, os2;
  int c = 0;
  while(std::getline(ifile, line))
  {
    s = ora::ToLowerCaseF(line);
    ora::Trim(s);
    if (s.substr(0, 11) == "fileversion")
    {
      s2 = ora::REG23GlobalInformation::ProductVersion;
      ora::ReplaceString(s2, ".", ",");
      os2.str(""); os2 << "FILEVERSION " << s2;
      if (ora::ToLowerCaseF(os2.str()) != ora::ToLowerCaseF(line))
        c++;
      os << os2.str() << "\n";
    }
    else if (s.substr(0, 14) == "productversion")
    {
      s2 = ora::REG23GlobalInformation::ProductVersion;
      ora::ReplaceString(s2, ".", ",");
      os2.str(""); os2 << "PRODUCTVERSION " << s2;
      if (ora::ToLowerCaseF(os2.str()) != ora::ToLowerCaseF(line))
        c++;
      os << os2.str() << "\n";
    }
    else if (s.substr(0, 19) == "value \"companyname\"")
    {
      s2 = ora::REG23GlobalInformation::GetCombinedCompanyName();
      std::size_t p = ora::ToLowerCaseF(line).find("value");
      std::string spaces(p, ' ');
      os2.str(""); os2 << spaces << "VALUE \"CompanyName\", \"" << s2 << "\"";
      if (ora::ToLowerCaseF(os2.str()) != ora::ToLowerCaseF(line))
        c++;
      os << os2.str() << "\n";
    }
    else if (s.substr(0, 23) == "value \"filedescription\"")
    {
      s2 = ora::REG23GlobalInformation::GetCombinedApplicationName();
      std::size_t p = ora::ToLowerCaseF(line).find("value");
      std::string spaces(p, ' ');
      os2.str(""); os2 << spaces << "VALUE \"FileDescription\", \"" << s2 << "\"";
      if (ora::ToLowerCaseF(os2.str()) != ora::ToLowerCaseF(line))
        c++;
      os << os2.str() << "\n";
    }
    else if (s.substr(0, 19) == "value \"fileversion\"")
    {
      s2 = ora::REG23GlobalInformation::ProductVersion;
      std::size_t p = ora::ToLowerCaseF(line).find("value");
      std::string spaces(p, ' ');
      os2.str(""); os2 << spaces << "VALUE \"FileVersion\", \"" << s2 << "\"";
      if (ora::ToLowerCaseF(os2.str()) != ora::ToLowerCaseF(line))
        c++;
      os << os2.str() << "\n";
    }
    else if (s.substr(0, 20) == "value \"internalname\"")
    {
      s2 = ora::REG23GlobalInformation::ApplicationShortName;
      std::size_t p = ora::ToLowerCaseF(line).find("value");
      std::string spaces(p, ' ');
      os2.str(""); os2 << spaces << "VALUE \"InternalName\", \"" << s2 << "\"";
      if (ora::ToLowerCaseF(os2.str()) != ora::ToLowerCaseF(line))
        c++;
      os << os2.str() << "\n";
    }
    else if (s.substr(0, 22) == "value \"legalcopyright\"")
    {
      s2 = ora::REG23GlobalInformation::Copyright;
      std::size_t p = ora::ToLowerCaseF(line).find("value");
      std::string spaces(p, ' ');
      os2.str(""); os2 << spaces << "VALUE \"LegalCopyright\", \"" << s2 << "\"";
      if (ora::ToLowerCaseF(os2.str()) != ora::ToLowerCaseF(line))
        c++;
      os << os2.str() << "\n";
    }
    else if (s.substr(0, 24) == "value \"originalfilename\"")
    {
      s2 = ora::REG23GlobalInformation::GetOriginalApplicationBinaryName();
      std::size_t p = ora::ToLowerCaseF(line).find("value");
      std::string spaces(p, ' ');
      os2.str(""); os2 << spaces << "VALUE \"OriginalFilename\", \"" << s2 << "\"";
      if (ora::ToLowerCaseF(os2.str()) != ora::ToLowerCaseF(line))
        c++;
      os << os2.str() << "\n";
    }
    else if (s.substr(0, 19) == "value \"productname\"")
    {
      s2 = ora::REG23GlobalInformation::ApplicationShortName;
      std::size_t p = ora::ToLowerCaseF(line).find("value");
      std::string spaces(p, ' ');
      os2.str(""); os2 << spaces << "VALUE \"ProductName\", \"" << s2 << "\"";
      if (ora::ToLowerCaseF(os2.str()) != ora::ToLowerCaseF(line))
        c++;
      os << os2.str() << "\n";
    }
    else if (s.substr(0, 22) == "value \"productversion\"")
    {
      s2 = ora::REG23GlobalInformation::GetCombinedVersionString();
      std::size_t p = ora::ToLowerCaseF(line).find("value");
      std::string spaces(p, ' ');
      os2.str(""); os2 << spaces << "VALUE \"ProductVersion\", \"" << s2 << "\"";
      if (ora::ToLowerCaseF(os2.str()) != ora::ToLowerCaseF(line))
        c++;
      os << os2.str() << "\n";
    }
    else
    {
      os << line << "\n";
    }
  }
  ifile.close();

  // only update on demand, otherwise the source file would always be changed!
  if (c > 0)
  {
    std::ofstream ofile;
    ofile.open(argv[1], std::ios::out);
    ofile << os.str();
    ofile.close();
    std::cout << "*** reg23-rc-updater updated " << c << " keys in resource file ***\n";
  }
  else
  {
    std::cout << "*** reg23-rc-updater had nothing to change ***\n";
  }

  return EXIT_SUCCESS;
}
