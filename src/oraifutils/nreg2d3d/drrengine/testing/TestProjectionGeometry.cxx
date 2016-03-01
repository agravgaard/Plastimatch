//
#include "BasicUnitTestIncludes.hxx"

#include "oraProjectionGeometry.h"
#include <vnl/vnl_matrix.h>
#include <itkMatrix.h>

/**
 * Tests base functionality of:
 *
 *   ora::ProjectionGeometry.
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::ProjectionGeometry
 *
 * @author phil
 * @author Markus
 * @version 1.1
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

  VERBOSE(<< "\nTesting projection geometry interface.\n")

  VERBOSE(<< "  * Basic configuration ... ")
  lok = true; // initialize sub-section's success state
  {
  	ora::ProjectionGeometry::Pointer geom = ora::ProjectionGeometry::New();
  	std::ostringstream os;
  	geom->Print(os, 0);
  	if (os.str().length() <= 0)
  		lok = false;
  	if (geom->IsGeometryValid())
  		lok = false;
  	double focus[3] = {0, 0, 1000};
  	geom->SetSourcePosition(focus);
  	if (geom->IsGeometryValid())
  		lok = false;
  	double dorig[3] = {-205, 205, -500};
  	geom->SetDetectorOrigin(dorig);
  	if (geom->IsGeometryValid())
  		lok = false;
  	double drow[3] = {1, 0, 0};
  	double dcol[3] = {0, -1, 0};
  	geom->SetDetectorOrientation(drow, dcol);
  	if (geom->IsGeometryValid())
  		lok = false;
  	double dspac[2] = {1, 0.5};
  	geom->SetDetectorPixelSpacing(dspac);
  	if (geom->IsGeometryValid())
  		lok = false;
  	int dsz[2] = {400, 800};
  	geom->SetDetectorSize(dsz);
  	if (!geom->IsGeometryValid())
  		lok = false;
  	if (!geom || geom->GetReferenceCount() != 1)
  		lok = false;
  	geom = NULL;
  }
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * In-plane-check ... ")
  lok = true; // initialize sub-section's success state
  {
  	ora::ProjectionGeometry::Pointer geom = ora::ProjectionGeometry::New();
  	std::ostringstream os;
  	geom->Print(os, 0);
  	if (os.str().length() <= 0)
  		lok = false;
  	if (geom->IsGeometryValid())
  		lok = false;
  	double focus[3] = {10, 40, -500};
  	geom->SetSourcePosition(focus);
  	if (geom->IsGeometryValid())
  		lok = false;
  	double dorig[3] = {-205, 205, -500};
  	geom->SetDetectorOrigin(dorig);
  	if (geom->IsGeometryValid())
  		lok = false;
  	double drow[3] = {1, 0, 0};
  	double dcol[3] = {0, -1, 0};
  	geom->SetDetectorOrientation(drow, dcol);
  	if (geom->IsGeometryValid())
  		lok = false;
  	double dspac[2] = {1, 0.5};
  	geom->SetDetectorPixelSpacing(dspac);
  	if (geom->IsGeometryValid())
  		lok = false;
  	int dsz[2] = {400, 800};
  	geom->SetDetectorSize(dsz);
  	if (geom->IsGeometryValid()) // in-plane
  		lok = false;
  	if (!geom || geom->GetReferenceCount() != 1)
  		lok = false;
  	geom = NULL;
  }
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Projection-matrix generation ... ")
  lok = true; // initialize sub-section's success state
  {
  	ora::ProjectionGeometry::Pointer geom = ora::ProjectionGeometry::New();

    double focus[3] = {0, 0, -1000};
    geom->SetSourcePosition(focus);
    double dorig[3] = {200, -100, 500};
    geom->SetDetectorOrigin(dorig);
    double drow[3] = {-1, 0, 0};
    double dcol[3] = {0, 1, 0};
    geom->SetDetectorOrientation(drow, dcol);
    double dspac[2] = {0.5, 0.25};
    geom->SetDetectorPixelSpacing(dspac);
    int dsz[2] = {400, 200};
    geom->SetDetectorSize(dsz);
    if (!geom->IsGeometryValid())
      lok = false;

    if (!geom->Compute3x4ProjectionMatrix())
    {
      lok = false;
    }
    else
    {
      /* FIXME: Add working test, the matrix below is obsolete and only a stub
      double *projMatrix = geom->Compute3x4ProjectionMatrix();
      double projmatrixReference[12] =
         {0.00710838, -1.21989e-013, -0.000971478, 0.971478,
         -1.18162e-013, -0.00355419, -0.000236946, 0.236946,
         -2.3486e-015, -4.53698e-016, -4.73892e-006, 0.00473892};

      for (int i = 0; i < 12; ++i)
      {
        if (vnl_math_abs(projMatrix[i] - projmatrixReference[i]) > ora::ProjectionGeometry::F_EPSILON)
          lok = false;
      }

      // TODO: More tests of projection matrices with rotated planes and known
      // point correspondences
      // Get homogeneous projection matrix
      vnl_matrix_fixed<double, 3, 4> Pv;
      Pv.copy_in(geom->Compute3x4ProjectionMatrix());
      itk::Matrix<double, 3, 4> P;
      P = Pv;
      */
    }

  	geom = NULL;
  }
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
