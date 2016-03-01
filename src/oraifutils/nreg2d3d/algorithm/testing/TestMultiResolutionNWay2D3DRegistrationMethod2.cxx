//
#include "BasicUnitTestIncludes.hxx"
#include "CommonRegistrationToolFunctions.hxx"

/**
 * Tests extended functionality of:
 *
 *   ora::MultiResolutionNWay2D3DRegistrationMethod
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::MultiResolutionNWay2D3DRegistrationMethod
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  bool strictResultsCheck = false;
  bool noResultsCheck = false;

  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  helpLines.push_back(
      "  -xo or --extended-output ... write out extended output");
  helpLines.push_back(
      "  -sc or --strict-check ... strict registration result check (accurate - can easily fail!); otherwise only check if result is closer to truth than initial parameters!");
  helpLines.push_back(
      "  -nc or --no-check ... forget about the registration results, they do not contribute to test result");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, true, false);
    return EXIT_SUCCESS;
  }
  // advanced arguments:
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-xo" || std::string(argv[i])
        == "--extended-output")
    {
      ExtendedOutput = true;
      continue;
    }
    if (std::string(argv[i]) == "-sc" || std::string(argv[i])
        == "--strict-check")
    {
      strictResultsCheck = true;
      continue;
    }
    if (std::string(argv[i]) == "-nc" || std::string(argv[i]) == "--no-check")
    {
      noResultsCheck = true;
      continue;
    }
  }

  VERBOSE(<< "\nTesting n-way multi-resolution 2D/3D-registration (2).\n")
  bool ok = true;
  bool lok; // local OK

  VERBOSE(<< "  * Testing oriented volume 2-way registration ... ")
  RegistrationType::Pointer registration = RegistrationType::New();
  lok = true;
  VolumeImageType::Pointer volume = GenerateTestVolume();
  if (!volume)
    lok = false;
  // apply a modified non-axis-aligned orientation to that volume:
  VolumeImageType::DirectionType volDir = volume->GetDirection();
  Transform3DType::Pointer orientTransform = Transform3DType::New();
  srand(time(NULL));
  double angleX = (double)(rand() % 100001) / 100000. / 4. - 0.25 / 2.;
  double angleY = (double)(rand() % 100001) / 100000. / 4. - 0.25 / 2.;
  double angleZ = (double)(rand() % 100001) / 100000. / 4. - 0.25 / 2.;
  orientTransform->SetRotation(angleX, angleY, angleZ);
  Transform3DType::OutputVectorType dir;
  // row
  dir[0] = volDir[0][0]; dir[1] = volDir[1][0]; dir[2] = volDir[2][0];
  dir = orientTransform->TransformVector(dir);
  volDir[0][0] = dir[0]; volDir[1][0] = dir[1]; volDir[2][0] = dir[2];
  // column
  dir[0] = volDir[0][1]; dir[1] = volDir[1][1]; dir[2] = volDir[2][1];
  dir = orientTransform->TransformVector(dir);
  volDir[0][1] = dir[0]; volDir[1][1] = dir[1]; volDir[2][1] = dir[2];
  // slicing
  dir[0] = volDir[0][2]; dir[1] = volDir[1][2]; dir[2] = volDir[2][2];
  dir = orientTransform->TransformVector(dir);
  volDir[0][2] = dir[0]; volDir[1][2] = dir[1]; volDir[2][2] = dir[2];
  // change
  volume->SetDirection(volDir);
  if (ImageOutput) // overwrite volume.mhd
  {
    VolumeWriterType::Pointer w = VolumeWriterType::New();
    w->SetInput(volume);
    w->SetFileName("volume.mhd");
    try
    {
      w->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      volume = NULL;
    }
    w = NULL;
  }
  Transform3DType::Pointer initVolTransform = Transform3DType::New();
  initVolTransform->SetIdentity();
  double fs1[3];
  DRRImageType::Pointer fixed3DImage1 = GenerateLinacKVProjectionImage(volume,
      -60.6, initVolTransform, "fixedb1_-60.6.mhd", fs1, 0);
  if (!fixed3DImage1)
    lok = false;
  double fs2[3];
  DRRImageType::Pointer fixed3DImage2 = GenerateLinacKVProjectionImage(volume,
      5.2, initVolTransform, "fixedb2_+5.2.mhd", fs2, 0);
  if (!fixed3DImage2)
    lok = false;
  double fs3[3];
  DRRImageType::Pointer fixed3DImage3 = GenerateLinacKVProjectionImage(volume,
      40.9, initVolTransform, "fixedb3_+40.9.mhd", fs3, 0);
  if (!fixed3DImage3)
    lok = false;

  // setup a nice 2-way registration scenario:
  CurrentRegistrationPrefix = "reg_2-way_oriented";
  typedef itk::CStyleCommand CommandType;
  CommandType::Pointer cscmd = CommandType::New();
  cscmd->SetClientData(registration);
  cscmd->SetCallback(MultiResolutionEvent);
  CommandType::Pointer optcscmd = CommandType::New();
  optcscmd->SetClientData(registration);
  optcscmd->SetCallback(OptimizerEvent);
  // prepare ray-casting properties:
  DRRPropsType::Pointer props1 = DRRPropsType::New();
  props1->SetGeometryFromFixedImage(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion());
  DRRPropsType::PointType fs;
  fs[0] = fs1[0];
  fs[1] = fs1[1];
  fs[2] = fs1[2];
  props1->SetSourceFocalSpotPosition(fs);
  if (!props1->IsGeometryValid())
    lok = false;
  props1->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  // ITF:  0,0.0 500,0.05 1001,0.2 1200,0.3 1201,0.3 2500,1.0 3000,1.0
  ITFPointer itf = ITFPointer::New();
  itf->AddRGBPoint(0, 0, 0, 0);
  itf->AddRGBPoint(500, 0.05, 0.05, 0.05);
  itf->AddRGBPoint(1001, 0.2, 0.2, 0.2);
  itf->AddRGBPoint(1200, 0.3, 0.3, 0.3);
  itf->AddRGBPoint(1201, 0.3, 0.3, 0.3);
  itf->AddRGBPoint(2500, 1.0, 1.0, 1.0);
  itf->AddRGBPoint(3000, 1.0, 1.0, 1.01);
  props1->SetITF(itf);
  if (!props1->AreAllPropertiesValid())
    lok = false;
  DRRPropsType::Pointer props2 = DRRPropsType::New();
  props2->SetGeometryFromFixedImage(fixed3DImage2,
      fixed3DImage2->GetLargestPossibleRegion());
  fs[0] = fs2[0];
  fs[1] = fs2[1];
  fs[2] = fs2[2];
  props2->SetSourceFocalSpotPosition(fs);
  if (!props2->IsGeometryValid())
    lok = false;
  props2->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  props2->SetITF(itf);
  if (!props2->AreAllPropertiesValid())
    lok = false;
  // prepare transformation
  Transform3DType::Pointer transform = Transform3DType::New();
  transform->SetIdentity();
  registration->SetTransform(transform);
  Transform3DType::ParametersType initPars = transform->GetParameters();
  initPars.Fill(0);
  initPars[0] = 0.2;
  initPars[1] = -0.1;
  initPars[2] = -0.21;
  initPars[3] = 5;
  initPars[4] = -4;
  initPars[5] = -8;
  registration->SetInitialTransformParameters(initPars);
  // setup multi-resolution
  registration->SetNumberOfLevels(1);
  registration->SetUseAutoProjectionPropsAdjustment(true);
  registration->SetAutoSamplingDistanceAdjustmentMode(0);
  registration->RemoveAllMetricFixedImageMappings();
  registration->SetUseMovingPyramidForFinalLevel(false);
  registration->SetUseMovingPyramidForUnshrinkedLevels(false);
  registration->SetMoving3DVolume(volume);
  registration->SetUseFixedPyramidForFinalLevel(false);
  registration->SetUseFixedPyramidForUnshrinkedLevels(false);
  registration->AddFixedImageAndProps(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion(), props1);
  registration->AddFixedImageAndProps(fixed3DImage2,
      fixed3DImage2->GetLargestPossibleRegion(), props2);
  // prepare metric
  GDMetricType::Pointer gd1 = GDMetricType::New();
  GDMetricType::Pointer gd2 = GDMetricType::New();
  // optimizer
  PowellOptimizerType::Pointer popt = PowellOptimizerType::New();
  PowellOptimizerType::ScalesType pscales;
  pscales.SetSize(6);
  pscales[0] = 180 / 3.1415;
  pscales[1] = 180 / 3.1415;
  pscales[2] = 180 / 3.1415;
  pscales[3] = 1;
  pscales[4] = 1;
  pscales[5] = 1;
  popt->SetScales(pscales);
  popt->SetMaximize(true);
  popt->SetMaximumIteration(10);
  popt->SetMaximumLineIteration(5);
  popt->SetStepLength(1.0);
  popt->SetStepTolerance(0.5);
  // setup registration
  RegMetricType::Pointer cm = registration->GetMetric();
  cm->AddMetricInput(gd1, "gd1", "dgd1");
  registration->AddMetricFixedImageMapping(gd1, 0);
  cm->AddMetricInput(gd2, "gd2", "dgd2");
  registration->AddMetricFixedImageMapping(gd2, 1);
  cm->SetValueCompositeRule("gd1+gd2");
  // do not use derivatives here ...
  registration->SetOptimizer(popt);
  cm->SetUseOptimizedValueComputation(false);
  // compute test-DRRs
  try
  {
    registration->Initialize();
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
    std::cerr << "Initialize ERROR: " << e << std::endl;
  }
  // registration
  registration->AddObserver(itk::StartEvent(), cscmd);
  registration->AddObserver(ora::StartMultiResolutionLevelEvent(), cscmd);
  registration->AddObserver(ora::StartOptimizationEvent(), cscmd);
  registration->AddObserver(itk::EndEvent(), cscmd);
  popt->AddObserver(itk::IterationEvent(), optcscmd);
  vtkSmartPointer<vtkTimerLog> clock = vtkSmartPointer<vtkTimerLog>::New();
  double ts;
  RegistrationType::DRR3DImagePointer drr3D;
  try
  {
    transform->SetParameters(registration->GetInitialTransformParameters());
    drr3D = registration->Compute3DTestProjection(0,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_NO0.mhd");
    drr3D = registration->Compute3DTestProjection(1,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_NO1.mhd");

    clock->StartTimer();
    registration->Update(); // registration
    clock->StopTimer();
    ts = clock->GetElapsedTime();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    REGISTRATION-TIME: " << ts << " s.\n")
    }

    drr3D = registration->Compute3DTestProjection(0,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_NO0.mhd");
    drr3D = registration->Compute3DTestProjection(1,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_NO1.mhd");
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
    std::cerr << "Registration ERROR: " << e << std::endl;
  }
  if (lok)
  {
    RegistrationType::TransformOutputConstPointer result =
        registration->GetOutput();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    RESULT-TRANSFORMATION:\n")
      itk::Indent ind(4);
      result->Get()->Print(std::cout, ind);
    }
    if (!noResultsCheck)
      lok = VerifyRegistrationResult(strictResultsCheck,
          result->Get()->GetParameters(),
          registration->GetInitialTransformParameters());
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing masked 2-way registration ... ")
  lok = true;
  CurrentRegistrationPrefix = "reg_2-way_masked";
  registration->SetNumberOfLevels(3);
  // compute test-DRRs of all levels -> derive circular DRR masks and set em
  try
  {
    registration->Initialize();
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
    std::cerr << "Initialize ERROR: " << e << std::endl;
  }
  std::vector<MaskImageType::Pointer> masks0;
  std::vector<MaskImageType::Pointer> masks1;
  try
  {
    transform->SetParameters(registration->GetInitialTransformParameters());
    for (int j = 0; j < (int)registration->GetNumberOfLevels(); j++)
    {
      char sbuff[200];
      std::string fn = CurrentRegistrationPrefix + "_mask_NO0_L%d.mhd";
      sprintf(sbuff, fn.c_str(), j);
      drr3D = registration->Compute3DTestProjection(0, j);
      masks0.push_back(GenerateCircularMask(drr3D, 60.0, std::string(sbuff)));
      fn = CurrentRegistrationPrefix + "_mask_NO1_L%d.mhd";
      sprintf(sbuff, fn.c_str(), j);
      drr3D = registration->Compute3DTestProjection(1, j);
      masks1.push_back(GenerateCircularMask(drr3D, 50.0, std::string(sbuff)));
    }
    registration->SetIthDRRMasks(0, masks0);
    registration->SetIthDRRMasks(1, masks1);
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
    std::cerr << "Mask construction ERROR: " << e << std::endl;
  }
  try
  {
    transform->SetParameters(registration->GetInitialTransformParameters());
    drr3D = registration->Compute3DTestProjection(0,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_NO0.mhd");
    drr3D = registration->Compute3DTestProjection(1,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_NO1.mhd");

    clock->StartTimer();
    registration->Update(); // registration
    clock->StopTimer();
    ts = clock->GetElapsedTime();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    REGISTRATION-TIME: " << ts << " s.\n")
    }

    drr3D = registration->Compute3DTestProjection(0,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_NO0.mhd");
    drr3D = registration->Compute3DTestProjection(1,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_NO1.mhd");
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
    std::cerr << "Registration ERROR: " << e << std::endl;
  }
  if (lok)
  {
    RegistrationType::TransformOutputConstPointer result =
        registration->GetOutput();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    RESULT-TRANSFORMATION:\n")
      itk::Indent ind(4);
      result->Get()->Print(std::cout, ind);
    }
    if (!noResultsCheck)
      lok = VerifyRegistrationResult(strictResultsCheck,
          result->Get()->GetParameters(),
          registration->GetInitialTransformParameters());
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing masked 2-way registration (2) ... ")
  lok = true;
  registration->RemoveAllFixedImagesAndProps();
  registration->RemoveAllMetricFixedImageMappings();
  // FIXME: convert masks to spatial objects, modify level-observer and set
  // spatial objects for MS-metric-components (as moving or fixed image mask)
  // FIXME:
  ok = ok && lok;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing multi-valued 1-way registration ... ")
  lok = true;
  VERBOSE(<< "  \n*** FIXME: integrate multi-metric ***\n")
  // FIXME: implement a mechanism for making multi-output metrics for
  // multi-input optimization possible (new metric type)!
  ok = ok && lok;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing multi-valued 3-way registration ... ")
  lok = true;
  VERBOSE(<< "  \n*** FIXME: integrate multi-metric ***\n")
  // FIXME: implement a mechanism for making multi-output metrics for
  // multi-input optimization possible (new metric type)!
  ok = ok && lok;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  if (registration->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  registration = NULL; // reference counter must be zero!
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
