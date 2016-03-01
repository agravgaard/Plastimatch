//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <time.h>
#include <math.h>
//ORAIFUTILS
#include "CommonDRREngineToolFunctions.hxx"
#include "BasicUnitTestIncludes.hxx"
#include "oraCPUSiddonDRRFilter.h"

// extended output (error measures)
bool ExtendedOutput = false;
// failure write output
bool FailureWriteOutput = false;

typedef float DRRPixelType;
typedef ora::CPUSiddonDRRFilter<ora::PhantomVolumePixelType, DRRPixelType> SiddonFilterType;

/**
 * Tests geometry and common functionality of:
 *
 *   ora::CPUSiddonDRRFilter,
 *   ora::ProjectionGeometry
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::CPUSiddonDRRFilter
 * @see ora::ProjectionGeometry
 *
 * @author phil
 * @version 1.2
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  helpLines.push_back(
      "  -xo or --extended-output ... write out additional error measures");
  helpLines.push_back(
      "  -fwo or --failure-write-output ... write output projections if a failure occurs");
  helpLines.push_back(
      "  -j <n> or --jobs <n> ... number of threads to use (if unspecified or <=0, auto-adjusted)");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, true, false);
    return EXIT_SUCCESS;
  }
  // advanced arguments:
  int njobs = 0;
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-fwo" || std::string(argv[i])
        == "--failure-write-output")
    {
      FailureWriteOutput = true;
      continue;
    }
    if (std::string(argv[i]) == "-xo" || std::string(argv[i])
        == "--extended-output")
    {
      ExtendedOutput = true;
      continue;
    }
    if (std::string(argv[i]) == "-j" || std::string(argv[i])
        == "--jobs")
    {
      i++;
      njobs = atoi(argv[i]);
      continue;
    }
  }

  VERBOSE(<< "\nTesting projection geometry of DRR engine.\n")
  bool ok = true;

  SiddonFilterType::Pointer drrFilter = SiddonFilterType::New();

  VERBOSE(<< "  * Generating test phantom data ... ")
  bool lok = true; // local OK
  ora::PhantomVolumeImageType::Pointer volume = ora::GenerateFiducialPhantom(4, ImageOutput);
  if (!volume)
    lok = false;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Setting up DRR engine ... ")
  lok = true; // local OK
  ora::GeometryType::Pointer geom = ora::GeometryType::New();
  drrFilter->SetProjectionGeometry(0, geom);
  // ITF:  0,0.0 2500,0.0 2501,1.0 6000,1.0
  ora::ITFType::Pointer itf = ora::ITFType::New();
  itf->AddSupportingPoint(0, 0);
  itf->AddSupportingPoint(2500, 0);
  itf->AddSupportingPoint(2501, 1);
  itf->AddSupportingPoint(6000, 1);
  drrFilter->SetITF(itf);
  double row[3] = {1, 0, 0};
  double column[3] = {0, 1, 0};
  geom->SetDetectorOrientation(row, column);
  double drrOrigin[3] = {-100, -80, -150};
  geom->SetDetectorOrigin(drrOrigin);
  int drrSize[2] = {200, 160};
  geom->SetDetectorSize(drrSize);
  double drrSpacing[2] = {1.0, 1.0};
  geom->SetDetectorPixelSpacing(drrSpacing);
  double drrFocalSpot[3] = {0, 0, 1000};
  geom->SetSourcePosition(drrFocalSpot);
  if (!geom->IsGeometryValid())
    lok = false;
  ora::TransformType::Pointer transform = ora::TransformType::New();
  transform->SetIdentity();
  drrFilter->SetTransform(transform);
  if (njobs > 0)
  	drrFilter->SetNumberOfThreads(njobs);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Computing and verifying projections ... ")
  lok = true;
  const int NUM_OF_TRANSFORMS = 5;
  for (int mode = 0; mode <= 9; mode++)
  {
    volume = ora::GenerateFiducialPhantom(mode, ImageOutput); // new phantom mode
    if (!volume)
    {
      VERBOSE(<< "\n    CANNOT GENERATE VOLUME (mode=" << mode << ")\n")
      lok = false;
      continue;
    }
    row[0] = 1;
    row[1] = 0;
    row[2] = 0;
    column[0] = 0;
    column[1] = 1;
    column[2] = 0;
    geom->SetDetectorOrientation(row, column);
    drrOrigin[0] = -170;
    drrOrigin[1] = -121.25;
    drrOrigin[2] = -500;
    geom->SetDetectorOrigin(drrOrigin);
    drrSize[0] = 650;
    drrSize[1] = 525;
    geom->SetDetectorSize(drrSize);
    drrSpacing[0] = 0.4;
    drrSpacing[1] = 0.5;
    geom->SetDetectorPixelSpacing(drrSpacing);
    char buff[100];
    srand(time(NULL));
    for (int i = 0; i < NUM_OF_TRANSFORMS; i++)
    {
      ora::TransformType::ParametersType pars(6);
      pars.Fill(0);
      if (i > 0)
      {
        pars[0] += ((double) (rand() % 101 - 50)) / 300.;
        pars[1] += ((double) (rand() % 101 - 50)) / 300.;
        pars[2] += ((double) (rand() % 101 - 50)) / 300.;
        pars[3] += ((double) (rand() % 101 - 50)) / 10.;
        pars[4] += ((double) (rand() % 101 - 50)) / 10.;
        pars[5] += ((double) (rand() % 101 - 50)) / 5.;
      }
      transform->SetParameters(pars);
      sprintf(buff, "drr_A_mode%d_trans%d.mhd", mode, (i + 1));
      if (!ora::ComputeAndVerifyProjection(drrFilter, volume, buff,
          ImageOutput, ExtendedOutput, FailureWriteOutput, 1.0))
        lok = false;
    }
    drrFocalSpot[0] = 50;
    drrFocalSpot[1] = -20;
    drrFocalSpot[2] = 1000;
    geom->SetSourcePosition(drrFocalSpot);
    drrOrigin[0] = -172;
    drrOrigin[1] = -118;
    drrOrigin[2] = -500;
    geom->SetDetectorOrigin(drrOrigin);
    for (int i = 0; i < NUM_OF_TRANSFORMS; i++)
    {
      ora::TransformType::ParametersType pars(6);
      pars.Fill(0);
      if (i > 0)
      {
        pars[0] += ((double) (rand() % 101 - 50)) / 400.;
        pars[1] += ((double) (rand() % 101 - 50)) / 400.;
        pars[2] += ((double) (rand() % 101 - 50)) / 400.;
        pars[3] += ((double) (rand() % 101 - 50)) / 30.;
        pars[4] += ((double) (rand() % 101 - 50)) / 30.;
        pars[5] += ((double) (rand() % 101 - 50)) / 20.;
      }
      transform->SetParameters(pars);
      sprintf(buff, "drr_B_mode%d_trans%d.mhd", mode, (i + 1));
      if (!ora::ComputeAndVerifyProjection(drrFilter, volume, buff,
          ImageOutput, ExtendedOutput, FailureWriteOutput, 1.0))
        lok = false;
    }

    ora::TransformType::Pointer thelp = ora::TransformType::New();
    ora::TransformType::ParametersType parameters(6);
    parameters.Fill(0);
    parameters[0] = 0.05;
    parameters[1] = 0.03;
    parameters[2] = -0.035;
    thelp->SetParameters(parameters);
    ora::TransformType::InputVectorType v;
    v[0] = row[0];
    v[1] = row[1];
    v[2] = row[2];
    v = thelp->TransformVector(v);
    row[0] = v[0];
    row[1] = v[1];
    row[2] = v[2];
    v[0] = column[0];
    v[1] = column[1];
    v[2] = column[2];
    v = thelp->TransformVector(v);
    column[0] = v[0];
    column[1] = v[1];
    column[2] = v[2];
    thelp = NULL;
    geom->SetDetectorOrientation(row, column);
    for (int i = 0; i < NUM_OF_TRANSFORMS; i++)
    {
      ora::TransformType::ParametersType pars(6);
      pars.Fill(0);
      if (i > 0)
      {
        pars[0] += ((double) (rand() % 101 - 50)) / 400.;
        pars[1] += ((double) (rand() % 101 - 50)) / 400.;
        pars[2] += ((double) (rand() % 101 - 50)) / 400.;
        pars[3] += ((double) (rand() % 101 - 50)) / 30.;
        pars[4] += ((double) (rand() % 101 - 50)) / 30.;
        pars[5] += ((double) (rand() % 101 - 50)) / 20.;
      }
      transform->SetParameters(pars);
      sprintf(buff, "drr_C_mode%d_trans%d.mhd", mode, (i + 1));
      if (!ora::ComputeAndVerifyProjection(drrFilter, volume, buff,
          ImageOutput, ExtendedOutput, FailureWriteOutput, 1.0))
        lok = false;
    }
  }
  geom = NULL;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  if (drrFilter->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  drrFilter = NULL; // reference counter must be zero!

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
