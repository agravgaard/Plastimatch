//
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>

// ORAIFTools
#include <oraIniAccess.h>
#include <oraQFileTools.h>
#include <oraStringTools.h>

#include <itksys/SystemTools.hxx>

/**
 * Collects the result-output-files of REG23 found in a specified folder
 * (results-folder) and in its sub-folders. This tool generates a comma-
 * separated-values (CVS) file (output-csv-file) from the found data. It
 * contains:<br>
 * - the results-file descriptor (folder/file names outgoing from specified
 *   results-folder<br>
 * - the registration error w.r.t. to reference transform and initial transform
 * <br>
 * - final registration parameters<br><br>
 *
 * The <comma-as-decimal-sign-flag> enables EXCEL-compatibility!
 *
 * @author phil
 * @version 1.0
 */
int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " <results-folder> <output-csv-file> [<comma-as-decimal-sign-flag>]\n";
    return EXIT_FAILURE;
  }

  if (!itksys::SystemTools::FileExists(argv[1]))
  {
    std::cerr << "<results-folder> (" << argv[1] << ") does not exist!\n";
    return EXIT_FAILURE;
  }

  if (ora::TrimF(std::string(argv[2])).length() <= 0)
  {
    std::cerr << "<output-csv-file> (" << argv[2] << ") is invalid!\n";
    return EXIT_FAILURE;
  }

  bool commaAsDecimal = false;
  if (argc > 3)
    commaAsDecimal = atoi(argv[3]);

  std::vector<std::string> inis = ora::GeneralFileTools::FindFilesInDirectory(
      std::string(argv[1]), "*.ini", true, "*");
  std::string inifile;
  ora::IniAccess *ini = NULL;
  std::ofstream csv(argv[2]);
  int off = std::string(argv[1]).length();
  std::string s = "", s2 = "";
  int k;
  // header:
  csv << "ID;ErrorToReference;ErrorToInitial;result-rx;result-ry;result-rz;result-tx;result-ty;result-tz;reg-time-sec;reg-iterations\n";

  for (std::size_t i = 0; i < inis.size(); i++)
  {
    if (i > 0)
      csv << "\n";

    inifile = inis[i];
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
    ora::ReplaceString(inifile, "/", "\\"); // back-slashes in windows
#endif

    ini = new ora::IniAccess(inifile);

    // image identifier (extracted from sub-folders relative to results-folder)
    s = inifile.substr(off, inifile.length());
    csv << s << ";";

    // real result-info
    s = ini->ReadString("Registration-Error", "FinalToReferenceVectorNorm", "");
    if (commaAsDecimal)
      ora::ReplaceString(s, ".", ",");
    csv << s << ";";
    s = ini->ReadString("Registration-Error", "FinalToInitialVectorNorm", "");
    if (commaAsDecimal)
      ora::ReplaceString(s, ".", ",");
    csv << s << ";";
    s = ini->ReadString("Result-Transform", "Nominal_Parameters_rot_in_deg_transl_in_mm", "");
    ora::ReplaceString(s, ",", ";");
    if (commaAsDecimal)
      ora::ReplaceString(s, ".", ",");
    csv << s << ";";

    // extract registration time:
    k = 0;
    do
    {
      k++;
      s = ini->ReadString("User-Tracking", "log" + ora::StreamConvert(k), "");
      ora::Trim(s);
      if (s.length() > 0)
      {
        if (s.substr(0, 26) == "EXECUTE: Auto-registration")
        {
          std::size_t p = s.find("registration time: ");
          if (p != std::string::npos)
          {
            std::size_t p2 = s.find(" s", p + 1);
            s = s.substr(p + std::string("registration time: ").length(),
                p2 - p - std::string("registration time: ").length());
            if (commaAsDecimal)
              ora::ReplaceString(s, ".", ",");
            break;
          }
        }
      }
    } while (s.length() > 0);
    csv << s << ";";

    // extract registration iterations:
    k = 0;
    do
    {
      k++;
      s = ini->ReadString("User-Tracking", "log" + ora::StreamConvert(k), "");
      ora::Trim(s);
      if (s.length() > 0)
      {
        if (s.substr(0, 26) == "EXECUTE: Auto-registration")
        {
          std::size_t p = s.find("iterations: ");
          if (p != std::string::npos)
          {
            std::size_t p2 = s.find(", ", p + 1);
            s = s.substr(p + std::string("iterations: ").length(),
                p2 - p - std::string("iterations: ").length());
            break;
          }
        }
      }
    } while (s.length() > 0);
    csv << s;

    delete ini;
  }
  csv.close();

  return EXIT_SUCCESS;
}
