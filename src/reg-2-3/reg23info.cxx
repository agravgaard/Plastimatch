//
#include "reg23info.h"

namespace ora
{

// static member initialization:
const std::string REG23GlobalInformation::ProductVersion = "1.2.5.4";
const std::string REG23GlobalInformation::NaturalProductVersion = "3-Fingers";
const std::string REG23GlobalInformation::Copyright = "Copyright (c) 2010-2011 radART";
const std::string REG23GlobalInformation::CompanyShortName = "radART";
const std::string REG23GlobalInformation::CompanyLongName = "Institute for Research and Development on Advanced Radiation Technologies";
const std::string REG23GlobalInformation::ApplicationShortName = "REG23";
const std::string REG23GlobalInformation::ApplicationLongName = "N-way 2D/3D Registration";
const std::string REG23GlobalInformation::Authors = "Philipp Steininger*Registration framework, GUI, scripting, ORA-integration*-|" \
  "Markus Neuner*Mask-optimization, GUI, scripting, deployment and maintenance*-|" \
  "Heinz Deutschmann*ORA-integration*-";


std::string REG23GlobalInformation::GetCombinedVersionString()
{
  if (NaturalProductVersion.length() > 0)
  {
    std::string s = "'" + NaturalProductVersion;
    s += "' (" + ProductVersion;
    s += ")";
    return s;
  }
  else
  {
    return ProductVersion;
  }
}

std::string REG23GlobalInformation::GetCombinedCompanyName()
{
  if (CompanyShortName.length() > 0)
  {
    if (CompanyLongName.length() > 0)
    {
      std::string s = CompanyLongName;
      s += " (" + CompanyShortName;
      s += ")";
      return s;
    }
    else
    {
      return CompanyShortName;
    }
  }
  else
  {
    return CompanyLongName;
  }
}

std::string REG23GlobalInformation::GetCombinedApplicationName()
{
  if (ApplicationShortName.length() > 0)
  {
    if (ApplicationLongName.length() > 0)
    {
      std::string s = ApplicationLongName;
      s += " (" + ApplicationShortName;
      s += ")";
      return s;
    }
    else
    {
      return ApplicationShortName;
    }
  }
  else
  {
    return ApplicationLongName;
  }
}

std::string REG23GlobalInformation::GetOriginalApplicationBinaryName()
{
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
  return "reg23.exe";
#else
  return "reg23";
#endif
}

std::vector<REG23GlobalInformation::AuthorInformation> REG23GlobalInformation::GetAuthorsInformation()
{
  std::vector<AuthorInformation> authors;
  std::vector<std::string> authorstrings;
  std::vector<std::string> authordetails;
  AuthorInformation ai;
  Tokenize(Authors, authorstrings, "|");
  for (std::size_t i = 0; i < authorstrings.size(); i++)
  {
    authordetails.clear();
    Tokenize(authorstrings[i], authordetails, "*");
    if (authordetails.size() == 3)
    {
      ai.Name = TrimF(authordetails[0]);
      ai.Contribution = TrimF(authordetails[1]);
      ai.Mail = TrimF(authordetails[2]);
      authors.push_back(ai);
    }
  }
  return authors;
}

}
