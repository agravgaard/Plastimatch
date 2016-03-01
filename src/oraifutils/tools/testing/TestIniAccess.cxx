//
#include "BasicUnitTestIncludes.hxx"

#include "oraIniAccess.h"

#include <vector>
#include <sstream>

#include <itksys/SystemTools.hxx>

// keep INI files after test on demand:
bool DoNotDeleteInisAfterTest = false;

/**
 * Tests base functionality of:
 *
 *   ora::IniAccess.
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::IniAccess
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  helpLines.push_back("  -ki or --keep-inis ... flag indicating that INI files should be kept after finishing the test");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL, helpLines);
    return EXIT_SUCCESS;
  }
  // advanced arguments:
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-ki" || std::string(argv[i]) == "--keep-inis")
    {
      DoNotDeleteInisAfterTest = true;
      continue;
    }
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting ora::IniAccess.\n")

  VERBOSE(<< "  * Basic read/write/override/clear capability ... ")
  lok = true; // initialize sub-section's success state

  if (itksys::SystemTools::FileExists("test1.ini"))
  {
    if (!itksys::SystemTools::RemoveFile("test1.ini"))
      lok = false;
  }
  std::vector<std::string> v;
  std::string s;
  ora::IniAccess *ini1 = new ora::IniAccess("test1.ini");
  s = "this is test string 1";
  v.push_back(s);
  ini1->WriteString("Strings", "string_1", s);
  s = "this is Test string 2";
  v.push_back(s);
  ini1->WriteString("Strings", "string_2", s);
  s = "this is teSt string 3";
  v.push_back(s);
  ini1->WriteString("Strings", "string_3", s);
  s = "this is test sTRing 4";
  v.push_back(s);
  ini1->WriteString("Strings", "string_4", s);
  for (std::size_t i = 1; i <= v.size(); i++)
  {
    if (ini1->ReadString("Strings", "string_" + ora::StreamConvert(i), "") !=
        v[i - 1])
      lok = false;
  }
  if (ini1->ReadString("Strings", "string_5", "") != "")
    lok = false;
  delete ini1; // should not output a file!
  if (itksys::SystemTools::FileExists("test1.ini"))
    lok = false;
  ini1 = new ora::IniAccess("test1.ini");
  ini1->WriteString("Strings", "string_1", s);
  ini1->WriteString("Strings1", "string_2", s);
  ini1->WriteString("Strings", "string_aasdf", s);
  ini1->WriteString("Strings3", "sadf", s);
  ini1->WriteString("Strings", "asdgasdf", s);
  if (!ini1->Update()) // should output file!
    lok = false;
  if (!itksys::SystemTools::FileExists("test1.ini"))
    lok = false;
  if (!DoNotDeleteInisAfterTest)
  {
    if (!itksys::SystemTools::RemoveFile("test1.ini"))
      lok = false;
  }
  delete ini1;

  if (itksys::SystemTools::FileExists("test2.ini"))
  {
    if (!itksys::SystemTools::RemoveFile("test2.ini"))
      lok = false;
  }
  ini1 = new ora::IniAccess();
  ini1->SetFileName("test2.ini");
  double d = -1.279485;
  int x = 1.4;
  unsigned short us = 3249;
  char c = 'd';
  float f = 1.434f;
  // FIXME: test ReadValue/WriteValue-templates
  if (!ini1->Update())
    lok = false;

  if (!DoNotDeleteInisAfterTest)
  {
    if (!itksys::SystemTools::RemoveFile("test2.ini"))
      lok = false;
  }
  delete ini1;

  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Comments ... ")
  lok = true; // initialize sub-section's success state

  // FIXME: test comments

  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Direct access ... ")
  lok = true; // initialize sub-section's success state

  // FIXME: test direct access to internal arrays

  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * UNIX/UNC-conversion ... ")
  lok = true; // initialize sub-section's success state

  // FIXME: test integrated UNIX/UNC-conversion capabilities on linux

  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * ORA check-sum ... ")
  lok = true; // initialize sub-section's success state

  if (itksys::SystemTools::FileExists("TeSt-cs.iNi"))
  {
    if (!itksys::SystemTools::RemoveFile("TeSt-cs.iNi"))
      lok = false;
  }

  ora::IniAccess *csINI = new ora::IniAccess("TeSt-cs.iNi");
  csINI->SetAddORACheckSum(true); // check-sum
  csINI->WriteString("Sect1", "key...1", "adkfla451341:_dfjh+#§$%&");
  csINI->WriteString("Sect1", "key...132", "adkfla451341:_dfjh+#§$%&");
  csINI->WriteString("Sect1", "key...193", "adkfla451341:_dfjh+#§$%&");
  csINI->WriteString("Sect1", "kadf.1", "adkfla451341:_dfjh+#§$%&");
  csINI->WriteString("Sect7348", "asdfjhkadf.1", "adkfla451341:_dfjh+#§$%&äöÜß");
  csINI->WriteValue<double>("DouBLE", "dhasdgwe1i3", 3.242);
  csINI->WriteValue<double>("DouBLE", "dhaasdfsdgwe1i3", 6434.4234);
  csINI->WriteValue<double>("DouBLE", "sdf3", -4788923.3774);
  csINI->WriteValue<double>("DouBLE", "dhasdgwhfsde1i3", 0.000012);
  csINI->WriteValue<double>("DouBLE", "dhasdgweasdf1i3", -1243.434);
  if (!csINI->Update())
    lok = false;
  if (!DoNotDeleteInisAfterTest)
  {
    if (!itksys::SystemTools::RemoveFile("TeSt-cs.iNi"))
      lok = false;
  }
  delete csINI;

  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "Test result: ")
  if (ok)
  {
    VERBOSE(<< "OK\n\n")
    return EXIT_SUCCESS;
  }
  else
  {
    VERBOSE(<< "FAILURE\n\n")
    return EXIT_FAILURE;
  }
}
