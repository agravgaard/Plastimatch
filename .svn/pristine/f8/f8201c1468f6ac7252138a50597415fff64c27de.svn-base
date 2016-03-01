

#ifndef BASICUNITTESTINCLUDES_HXX_
#define BASICUNITTESTINCLUDES_HXX_


// basic includes
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

// standard verbose macro
#define VERBOSE(x) \
{ \
  if (Verbose) \
  {\
    std::cout x; \
    std::cout.flush(); \
  }\
}

// global constants
#if ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ )
  const std::string _OS_SLASH = "\\"; // backslash as file separator
#else
  const std::string _OS_SLASH = "/"; // slash as file separator
#endif

// global flags
bool Verbose = false; // verbose
bool ImageOutput = false; // test image output
std::string DataPath = "data" + _OS_SLASH; // custom test data path (default: "data"-subfolder of current directory)

// Utility-method that ensures that the specified string ends with file separator
std::string EnsureStringEndsWithFileSeparator(std::string s)
{
  if (s.length() >= _OS_SLASH.length())
  {
    if (s.substr(s.length() - _OS_SLASH.length(), _OS_SLASH.length()) != _OS_SLASH)
      return s + _OS_SLASH;
    else
      return s;
  }
  else
  {
    return s + _OS_SLASH;
  }
}
// Utility-method that ensures that the specified string DOES NOT end with file separator
std::string EnsureStringEndsNotWithFileSeparator(std::string s)
{
  if (s.length() >= _OS_SLASH.length())
  {
    if (s.substr(s.length() - _OS_SLASH.length(), _OS_SLASH.length()) == _OS_SLASH)
      return s.substr(0, s.length() - _OS_SLASH.length());
    else
      return s;
  }
  else
  {
    return s;
  }
}

// Utility-method for printing information on test usage.
void PrintBasicTestUsage(const char *binname,
    std::vector<std::string> &additionalLines, bool supportImageOutput = false,
    bool supportDataPath = false)
{
  std::string progname = "<test-binary-name>";

  if (binname)
    progname = std::string(binname);

  std::cout << "\n";
  std::cout << "   *** T E S T   U S A G E   I N F O R M A T I O N ***\n";
  std::cout << "\n";
  std::cout << progname << " [options]\n";
  std::cout << "\n";
  std::cout << "  -h or --help ... print this short help\n";
  std::cout << "  -v or --verbose ... verbose messages to std::cout\n";
  if (supportImageOutput)
    std::cout << "  -io or --image-output ... write out images that are used during test\n";
  if (supportDataPath)
    std::cout << "  -dp or --data-path ... set a custom data path where the test data are located (by default a data-subfolder is expected): <data-path>\n";
  for (std::size_t i = 0; i < additionalLines.size(); i++)
    std::cout << additionalLines[i] << std::endl;
  std::cout << "\n";
  std::cout << "  NOTE: optional arguments are case-sensitive!\n";
  std::cout << "\n";
  std::cout
      << "  Institute for Research and Development on Advanced Radiation Technologies (radART)\n";
  std::cout
      << "  Paracelsus Medical University (PMU), Salzburg, AUSTRIA\n";
  std::cout << "\n";
}

// Utility-method for checking the basic command line arguments of a test.
// If this method returns false, the help should be displayed!
// In addition, it will return the program name in string representation.
bool CheckBasicTestCommandLineArguments(int argc, char *argv[],
    std::string &programName)
{
  std::string progname = "";

  if (argc > 0)
    progname = std::string(argv[0]);

  // basic arguments check
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-v" || std::string(argv[i]) == "--verbose")
      Verbose = true;
    if (std::string(argv[i]) == "-io" || std::string(argv[i]) == "--image-output")
      ImageOutput = true;
    if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
      return false;
    if (std::string(argv[i]) == "-dp" || std::string(argv[i]) == "--data-path")
    {
      i++;
      DataPath = std::string(argv[i]);
    }
  }

  DataPath = EnsureStringEndsWithFileSeparator(DataPath);

  return true;
}


#endif /* BASICUNITTESTINCLUDES_HXX_ */
