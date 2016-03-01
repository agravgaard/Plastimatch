//
#include "BasicUnitTestIncludes.hxx"

#include "oraStringTools.h"

#include <vector>

/**
 * Tests base functionality of:
 *
 *   oraStringTools
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see oraStringTools.h
 * @see oraStringTools.cxx
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @author Markus <markus.neuner (at) pmu.ac.at>
 * @version 1.1
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    std::vector<std::string> helpLines;
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL, helpLines, false);
    return EXIT_SUCCESS;
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting ora::StringTools.\n")

  VERBOSE(<< "  * Numeric string checks ... ")
  lok = true; // initialize sub-section's success state
  // loosely numeric:
  if (ora::IsNumeric(""))
    lok = false;
  if (ora::IsNumeric("ab"))
    lok = false;
  if (!ora::IsNumeric("0") || !ora::IsNumeric("38") || !ora::IsNumeric("-1009393"))
    lok = false;
  if (!ora::IsNumeric("1.0") || !ora::IsNumeric("+0.003") || !ora::IsNumeric(".3"))
    lok = false;
  if (!ora::IsNumeric("1e-3") || !ora::IsNumeric("0.3e+33"))
    lok = false;
  if (!ora::IsNumeric(" 1e-3  ") || !ora::IsNumeric("   0.3e+33   "))
    lok = false;
  if (ora::IsNumeric("-b.clk3.3"))
    lok = false;
  if (!ora::IsNumeric("1.a")) // NOTE: this is a limitation -> use strictly!
    lok = false;
  // strictly numeric:
  if (ora::IsStrictlyNumeric(""))
    lok = false;
  if (ora::IsStrictlyNumeric("32a3"))
    lok = false;
  if (ora::IsStrictlyNumeric("3.23") || ora::IsStrictlyNumeric(".323"))
    lok = false;
  if (ora::IsStrictlyNumeric("-32") || ora::IsStrictlyNumeric("+32"))
    lok = false;
  if (!ora::IsStrictlyNumeric("0") || !ora::IsStrictlyNumeric("0123456789"))
    lok = false;
  if (!ora::IsStrictlyNumeric(" 0   ") || !ora::IsStrictlyNumeric("      0123456789  "))
    lok = false;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Version parsing ... ")
  lok = true; // initialize sub-section's success state
  std::vector<int> version;
  ora::ParseVersionString("", version);
  if (version.size() > 0)
    lok = false;
  ora::ParseVersionString("  ", version);
  if (version.size() > 0)
    lok = false;
  ora::ParseVersionString("1.", version);
  if (version.size() > 0)
    lok = false;
  ora::ParseVersionString(" .32 ", version);
  if (version.size() > 0)
    lok = false;
  ora::ParseVersionString("32..77", version);
  if (version.size() > 0)
    lok = false;
  ora::ParseVersionString("1.2", version);
  if (version.size() != 2 || version[0] != 1 || version[1] != 2)
    lok = false;
  ora::ParseVersionString(" 0.22.37.5    ", version);
  if (version.size() != 4 || version[0] != 0 || version[1] != 22
      || version[2] != 37 || version[3] != 5)
    lok = false;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Conversions ... ")
  lok = true; // initialize sub-section's success state


  // ConvertTo:
  try
  {
    // No exception expected
    if (ora::StringConvertTo<int>("0") != 0 || ora::StringConvertTo<int>("+0")
        != 0 || ora::StringConvertTo<int>("-0") != 0 || ora::StringConvertTo<
        int>(" 0") != 0 || ora::StringConvertTo<int>(" 0 ") != 0
        || ora::StringConvertTo<int>("0 ") != 0 || ora::StringConvertTo<int>(
        "+38") != 38 || ora::StringConvertTo<int>("-38") != -38
        || ora::StringConvertTo<int>("38") != 38 || ora::StringConvertTo<int>(
        "-1009393") != -1009393)
      lok = false;
    if (ora::StringConvertTo<double>("1.0") != 1.0 || ora::StringConvertTo<
        double>("+0.003") != 0.003 || ora::StringConvertTo<double>(".3") != 0.3)
      lok = false;
    if (ora::StringConvertTo<double>("1e-3") != 0.001 || ora::StringConvertTo<
        double>("0.3e+3") != 300)
      lok = false;
    if (ora::StringConvertTo<double>(" 1e-3  ") != 0.001
        || ora::StringConvertTo<double>("   0.3e+3   ") != 300)
      lok = false;
    if (ora::StringConvertTo<double>("1.a") != 1.0)
      lok = false;
    try {
     ora::StringConvertTo<double>(" 1e-3", true);
    } catch (ora::BadConversion& e){lok = false;}
    // Exception expected
    try {
      ora::StringConvertTo<double>("", false);
      lok = false;
    } catch (ora::BadConversion& e){}
    try {
      ora::StringConvertTo<double>("ab", false);
      lok = false;
    } catch (ora::BadConversion& e){}
    try {
      ora::StringConvertTo<double>("-b.clk3.3", false);
      lok = false;
    } catch (ora::BadConversion& e){}
    try {
      ora::StringConvertTo<double>("1.a", true);
      lok = false;
    } catch (ora::BadConversion& e){}
    try {
      ora::StringConvertTo<double>(" 1e-3  ", true);
      lok = false;
    } catch (ora::BadConversion& e){}
    try {
      std::cout << ora::StringConvertTo<double>("a1", true) << std::endl;
      lok = false;
    } catch (ora::BadConversion& e){}
    try {
      std::cout << ora::StringConvertTo<double>("a1", false) << std::endl;
      lok = false;
    } catch (ora::BadConversion& e){}
  }
  catch (ora::BadConversion& e)
  {
    lok = false;
    VERBOSE(<< "  BadConversion exception: " << e.what());
  }
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")


  // FIXME: test the rest of string operations in lib!!!

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
