//
#include "BasicUnitTestIncludes.hxx"

// TODO: include further test-specific headers

// TODO: define further test-specific global variables (like ImageOutput)
int CustomOption = -1;

// TODO: try to structure the code a bit by implementing sub-tasks in methods
// (if a method is likely to be quite generic and likely to be useful for other
// tests too, extend BasicUnitTestIncludes.hxx! - BUT: please do not add code
// to BasicUnitTestIncludes.hxx that relies on a third-party library such as
// ITK - generate a new include file for such methods)

/**
 * Tests base functionality of:
 *
 *   TODO (fill-in the class name(s)).
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see TODO (fill-in referenced class(es))
 *
 * @author TODO (fill-in authorship/co-authorship)
 * @version TODO (fill-in test version)
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  // TODO: add as many help lines to the usage-information as needed (supported)
  helpLines.push_back("  -co or --custom-option ... custom option: <int> (a custom integer)");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL, helpLines);
    return EXIT_SUCCESS;
  }
  // advanced arguments:
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-co" || std::string(argv[i]) == "--custom-option")
    {
      i++;
      CustomOption = atoi(argv[i]);
      continue;
    }
    // TODO: add as many argument checks as necessary
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  // TODO: insert an appropriate caption
  VERBOSE(<< "\nTesting TODO.\n")

  // TODO: rename the test sub-section as needed
  VERBOSE(<< "  * A sub-test ... ")
  lok = true; // initialize sub-section's success state

  // TODO: implement test code and update lok

  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  // TODO: rename the test sub-section as needed
  VERBOSE(<< "  * Another sub-test ... ")
  lok = true; // initialize sub-section's success state

  // TODO: implement test code and update lok

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
