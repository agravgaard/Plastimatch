//
#include "BasicUnitTestIncludes.hxx"

#include "oraIntensityTransferFunction.h"

/**
 * Tests base functionality of:
 *
 *   ora::IntensityTransferFunction.cxx
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::IntensityTransferFunction
 *
 * @author jeanluc
 * @version 1.0
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL, helpLines);
    return EXIT_SUCCESS;
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting intensity transfer function interface.\n")

  VERBOSE(<< "  * Basic configuration \n")
  lok = true; // initialize sub-section's success state

  ora::IntensityTransferFunction::Pointer itf = ora::IntensityTransferFunction::New();
  //adding support points
  itf->AddSupportingPoint(0.0, 0.0);
  itf->AddSupportingPoint(100.0, 100.0);
  //checking if output value correct for input 50
  if (itf->MapInValue(50.0) != 50.0)
    lok = false;
  //adding another supporting point
  itf->AddSupportingPoint(50.0, 25.0);
  //checking of input is mapped to right values
  if (itf->MapInValue(12.5) != 6.25)
    lok = false;
  if (itf->MapInValue(75.0) != 62.5)
	lok = false;
  //checking if it returns the right number of supporting points
  if (itf->GetNumberOfSupportingPoints() != 3)
    lok = false;
  //try to retrieve all supporting numbers and check
//  if output is sorted (as it should be after calling a map function
  const std::vector<ora::SupportingPointStruct> *supportingPointValues;
  supportingPointValues = itf->GetSupportingPoints();
  if((*supportingPointValues)[0].inValue != 0.0 ||
		  (*supportingPointValues)[0].outValue != 0.0 ||
		  (*supportingPointValues)[1].inValue != 50.0 ||
		  (*supportingPointValues)[1].outValue != 25.0||
		  (*supportingPointValues)[2].inValue != 100.0||
		  (*supportingPointValues)[2].outValue != 100.0)
    lok = false;
  // removing support point and check if updated correctly
  itf->RemoveSupportingPoint(1);
  supportingPointValues = itf->GetSupportingPoints();
  if((*supportingPointValues)[0].inValue != 0.0 ||
		  (*supportingPointValues)[1].inValue != 100.0 ||
		  (*supportingPointValues)[0].outValue != 0.0 ||
		  (*supportingPointValues)[1].outValue != 100.0)
    lok = false;
  //add new supporting point and check mapping
  itf->AddSupportingPoint(70.0, 50.0);
  double temp = 0;
  itf->MapInValue(85.0, temp);
  if(temp != 75.0)
    lok = false;
  //removing all points and check if it worked
  itf->RemoveAllSupportingPoints();
  if(itf->GetNumberOfSupportingPoints() != 0)
	lok = false;
  //adding range of supporting point and map in array of values
  itf->AddSupportingPoint(0.0, 0.0);
  itf->AddSupportingPoint(100.0, 100.0);
  itf->AddSupportingPoint(250.0, 200.0);
  itf->AddSupportingPoint(200.0, 150.0);
  double ins[10] = {-10.0, 22.0, 50.0, 200.0, 100.10, -30.0, 132.2, 160.9, 226.11, 400.0};
  double out[10];
  itf->MapInValues(10, ins, out);
  //check clamping
  for(int i = 0; i < 10; i++)
  {
	  if((i == 0 || i == 5 || i == 9))
	  {
		  if(out[i] != 0)
			  lok = false;
	  }
	  else if(out[i] == 0)
	  {
		  lok = false;
	  }
  }
  //check output
  std::ostringstream os;
  itf->Print(os, 0);
  if (os.str().length() <= 0)
		lok = false;
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
