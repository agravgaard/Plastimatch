//
#include "BasicUnitTestIncludes.hxx"
#include "CommonRegistrationToolFunctions.hxx"

/**
 * Tests base functionality of:
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
 * @version 1.6
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

  VERBOSE(<< "\nTesting n-way multi-resolution 2D/3D-registration.\n")
  bool ok = true;

  VERBOSE(<< "  * Streaming tests ... ")
  bool lok = true; // local OK
  RegistrationType::Pointer registration = RegistrationType::New();
  std::ostringstream teststream;
  registration->Print(teststream, 0);
  if (teststream.str().length() <= 0)
    lok = false;
  teststream.str("");
  itk::Indent indent(5);
  registration->Print(teststream, 0);
  if (teststream.str().length() <= 0)
    lok = false;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Generating registration data sets ... ")
  lok = true;
  Transform3DType::Pointer initVolTransform = Transform3DType::New();
  initVolTransform->SetIdentity();
  VolumeImageType::Pointer volume = GenerateTestVolume();
  if (!volume)
    lok = false;
  double fs1[3];
  DRRImageType::Pointer fixed3DImage1 = GenerateLinacKVProjectionImage(volume,
      -90.0, initVolTransform, "fixed1_-90.mhd", fs1, 0);
  if (!fixed3DImage1)
    lok = false;
  double fs2[3];
  DRRImageType::Pointer fixed3DImage2 = GenerateLinacKVProjectionImage(volume,
      0.0, initVolTransform, "fixed2_0.mhd", fs2, 0);
  if (!fixed3DImage2)
    lok = false;
  DRRImageType::Pointer fixed3DImage21 = GenerateLinacKVProjectionImage(volume,
      0.0, initVolTransform, "fixed21_0.mhd", fs2, 1);
  if (!fixed3DImage21)
    lok = false;
  double fs3[3];
  DRRImageType::Pointer fixed3DImage3 = GenerateLinacKVProjectionImage(volume,
      35.5, initVolTransform, "fixed3_+35.5.mhd", fs3, 0);
  if (!fixed3DImage3)
    lok = false;
  double fs4[3];
  DRRImageType::Pointer fixed3DImage4 = GenerateLinacKVProjectionImage(volume,
      75.5, initVolTransform, "fixed4_+75.5.mhd", fs4, 0);
  if (!fixed3DImage4)
    lok = false;
  DRRImageType::Pointer fixed3DImage42 = GenerateLinacKVProjectionImage(volume,
      75.5, initVolTransform, "fixed42_+75.5.mhd", fs4, 2);
  if (!fixed3DImage42)
    lok = false;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Basic registration method tests ... ")
  lok = true;
  registration->SetMoving3DVolume(volume);
  registration->SetMoving3DVolume(NULL);
  registration->SetMoving3DVolume(volume);
  // USE 2D FIXED IMAGE REPRESENTATION
  DRRPropsType::Pointer dummyProps = DRRPropsType::New();
  ExtractorType::Pointer extractFilter = ExtractorType::New();
  DRRImageType::RegionType eregion;
  Compute3D2DExtractRegion(fixed3DImage1, eregion);
  extractFilter->SetExtractionRegion(eregion);
  extractFilter->SetInput(fixed3DImage1);
  extractFilter->Update();
  RegistrationType::FixedImagePointer f1 = extractFilter->GetOutput();
  f1->DisconnectPipeline();
  DRRImageType::DirectionType storedDir = fixed3DImage2->GetDirection();
  DRRImageType::DirectionType identityDir;
  identityDir.SetIdentity();
  fixed3DImage2->SetDirection(identityDir);
  Compute3D2DExtractRegion(fixed3DImage2, eregion);
  extractFilter->SetExtractionRegion(eregion);
  extractFilter->SetInput(fixed3DImage2);
  extractFilter->Update();
  RegistrationType::FixedImagePointer f2 = extractFilter->GetOutput();
  f2->DisconnectPipeline();
  fixed3DImage2->SetDirection(storedDir);
  storedDir = fixed3DImage3->GetDirection();
  fixed3DImage3->SetDirection(identityDir);
  Compute3D2DExtractRegion(fixed3DImage3, eregion);
  extractFilter->SetExtractionRegion(eregion);
  extractFilter->SetInput(fixed3DImage3);
  extractFilter->Update();
  RegistrationType::FixedImagePointer f3 = extractFilter->GetOutput();
  f3->DisconnectPipeline();
  fixed3DImage3->SetDirection(storedDir);
  registration->AddFixedImageAndProps(f1, f1->GetLargestPossibleRegion(),
      dummyProps);
  if (registration->GetNumberOfFixedImages() != 1)
    lok = false;
  registration->RemoveIthFixedImageAndProps(0);
  if (registration->GetNumberOfFixedImages() != 0)
    lok = false;
  registration->AddFixedImageAndProps(f1, f1->GetLargestPossibleRegion(),
      dummyProps);
  if (registration->GetNumberOfFixedImages() != 1)
    lok = false;
  registration->RemoveAllFixedImagesAndProps();
  if (registration->GetNumberOfFixedImages() != 0)
    lok = false;
  registration->AddFixedImageAndProps(f1, f1->GetLargestPossibleRegion(),
      dummyProps);
  if (registration->GetNumberOfFixedImages() != 1)
    lok = false;
  registration->AddFixedImageAndProps(f2, f2->GetLargestPossibleRegion(),
      dummyProps);
  if (registration->GetNumberOfFixedImages() != 2)
    lok = false;
  registration->AddFixedImageAndProps(f3, f3->GetLargestPossibleRegion(),
      dummyProps);
  if (registration->GetNumberOfFixedImages() != 3)
    lok = false;
  registration->AddFixedImageAndProps(f3, f3->GetLargestPossibleRegion(),
      dummyProps); // twice - cannot protect
  if (registration->GetNumberOfFixedImages() != 4)
    lok = false;
  registration->RemoveIthFixedImageAndProps(3);
  registration->RemoveIthFixedImageAndProps(1);
  if (registration->GetNumberOfFixedImages() != 2)
    lok = false;
  registration->RemoveIthFixedImageAndProps(1);
  if (registration->GetNumberOfFixedImages() != 1)
    lok = false;
  registration->RemoveIthFixedImageAndProps(0);
  if (registration->GetNumberOfFixedImages() != 0)
    lok = false;
  registration->AddFixedImageAndProps(f1, f1->GetLargestPossibleRegion(),
      dummyProps);
  if (registration->GetNumberOfFixedImages() != 1)
    lok = false;
  registration->AddFixedImageAndProps(f2, f2->GetLargestPossibleRegion(),
      dummyProps);
  if (registration->GetNumberOfFixedImages() != 2)
    lok = false;
  registration->AddFixedImageAndProps(f3, f3->GetLargestPossibleRegion(),
      dummyProps);
  if (registration->GetNumberOfFixedImages() != 3)
    lok = false;
  registration->RemoveAllFixedImagesAndProps();
  if (registration->GetNumberOfFixedImages() != 0)
    lok = false;
  f1 = NULL;
  f2 = NULL;
  f3 = NULL;
  // USE 3D FIXED IMAGE REPRESENTATION
  registration->AddFixedImageAndProps(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion(), dummyProps);
  if (registration->GetNumberOfFixedImages() != 1)
    lok = false;
  registration->RemoveIthFixedImageAndProps(0);
  if (registration->GetNumberOfFixedImages() != 0)
    lok = false;
  registration->AddFixedImageAndProps(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion(), dummyProps);
  if (registration->GetNumberOfFixedImages() != 1)
    lok = false;
  registration->RemoveAllFixedImagesAndProps();
  if (registration->GetNumberOfFixedImages() != 0)
    lok = false;
  registration->AddFixedImageAndProps(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion(), dummyProps);
  if (registration->GetNumberOfFixedImages() != 1)
    lok = false;
  registration->AddFixedImageAndProps(fixed3DImage2,
      fixed3DImage2->GetLargestPossibleRegion(), dummyProps);
  if (registration->GetNumberOfFixedImages() != 2)
    lok = false;
  registration->AddFixedImageAndProps(fixed3DImage3,
      fixed3DImage3->GetLargestPossibleRegion(), dummyProps);
  if (registration->GetNumberOfFixedImages() != 3)
    lok = false;
  // *** metric-fixed-image mapping tests:
  NMIMetricType::Pointer testNMI1 = NMIMetricType::New();
  NMIMetricType::Pointer testNMI2 = NMIMetricType::New();
  MSMetricType::Pointer testMS1 = MSMetricType::New();
  MSMetricType::Pointer testMS2 = MSMetricType::New();
  if (registration->GetMappedFixedImageIndex(testNMI1) != -1
      || registration->GetMappedFixedImageIndex(testNMI2) != -1
      || registration->GetMappedFixedImageIndex(testMS1) != -1
      || registration->GetMappedFixedImageIndex(testMS2) != -1)
    lok = false;
  if (registration->GetMappedMetrics(0).size() > 0
      || registration->GetMappedMetrics(1).size() > 0
      || registration->GetMappedMetrics(2).size() > 0)
    lok = false;
  if (registration->AddMetricFixedImageMapping(testNMI1, 3)) // invalid add
    lok = false;
  if (!registration->AddMetricFixedImageMapping(testNMI1, 1)) // add
    lok = false;
  if (registration->GetMappedFixedImageIndex(testNMI1) != 1)
    lok = false;
  if (registration->GetMappedMetrics(1).size() != 1
      || registration->GetMappedMetrics(1)[0] != testNMI1)
    lok = false;
  if (!registration->AddMetricFixedImageMapping(testNMI1, 0)) // update
    lok = false;
  if (registration->GetMappedFixedImageIndex(testNMI1) != 0)
    lok = false;
  if (registration->GetMappedMetrics(0).size() != 1
      || registration->GetMappedMetrics(1).size() != 0
      || registration->GetMappedMetrics(0)[0] != testNMI1)
    lok = false;
  if (!registration->RemoveMetricFixedImageMapping(testNMI1)) // remove
    lok = false;
  if (registration->GetMappedFixedImageIndex(testNMI1) != -1)
    lok = false;
  if (!registration->AddMetricFixedImageMapping(testNMI1, 1)) // add
    lok = false;
  if (registration->RemoveMetricFixedImageMapping(1) != 1) // remove
    lok = false;
  if (registration->GetMappedFixedImageIndex(testNMI1) != -1)
    lok = false;
  if (!registration->AddMetricFixedImageMapping(testNMI1, 1)) // add
    lok = false;
  registration->RemoveAllMetricFixedImageMappings();
  if (registration->GetMappedFixedImageIndex(testNMI1) != -1)
    lok = false;
  registration->AddMetricFixedImageMapping(testNMI1, 0); // add
  registration->AddMetricFixedImageMapping(testMS1, 1);
  registration->AddMetricFixedImageMapping(testNMI2, 1);
  registration->AddMetricFixedImageMapping(testMS2, 2);
  if (registration->GetMappedFixedImageIndex(testNMI1) != 0
      || registration->GetMappedFixedImageIndex(testNMI2) != 1
      || registration->GetMappedFixedImageIndex(testMS1) != 1
      || registration->GetMappedFixedImageIndex(testMS2) != 2
      || registration->GetMappedMetrics(0).size() != 1
      || registration->GetMappedMetrics(0)[0] != testNMI1
      || registration->GetMappedMetrics(1).size() != 2
      || (registration->GetMappedMetrics(1)[0] != testNMI2
          && registration->GetMappedMetrics(1)[0] != testMS1)
      || (registration->GetMappedMetrics(1)[1] != testNMI2
          && registration->GetMappedMetrics(1)[1] != testMS1)
      || registration->GetMappedMetrics(2).size() != 1
      || registration->GetMappedMetrics(2)[0] != testMS2)
    lok = false;
  registration->RemoveMetricFixedImageMapping(testNMI2); // remove
  if (registration->GetMappedFixedImageIndex(testNMI1) != 0
      || registration->GetMappedFixedImageIndex(testNMI2) != -1
      || registration->GetMappedFixedImageIndex(testMS1) != 1
      || registration->GetMappedFixedImageIndex(testMS2) != 2
      || registration->GetMappedMetrics(0).size() != 1
      || registration->GetMappedMetrics(0)[0] != testNMI1
      || registration->GetMappedMetrics(1).size() != 1
      || registration->GetMappedMetrics(1)[0] != testMS1
      || registration->GetMappedMetrics(2).size() != 1
      || registration->GetMappedMetrics(2)[0] != testMS2)
    lok = false;
  registration->RemoveAllMetricFixedImageMappings();
  if (registration->GetMappedFixedImageIndex(testNMI1) != -1
      || registration->GetMappedFixedImageIndex(testNMI2) != -1
      || registration->GetMappedFixedImageIndex(testMS1) != -1
      || registration->GetMappedFixedImageIndex(testMS2) != -1
      || registration->GetMappedMetrics(0).size() != 0
      || registration->GetMappedMetrics(1).size() != 0
      || registration->GetMappedMetrics(2).size() != 0)
    lok = false;
  testNMI1 = NULL;
  testNMI2 = NULL;
  testMS1 = NULL;
  testMS2 = NULL;
  // *** / metric-fixed-image mapping tests /
  registration->AddFixedImageAndProps(fixed3DImage3,
      fixed3DImage3->GetLargestPossibleRegion(), dummyProps); // twice (3D)!
  if (registration->GetNumberOfFixedImages() != 4)
    lok = false;
  registration->RemoveIthFixedImageAndProps(1);
  if (registration->GetNumberOfFixedImages() != 3)
    lok = false;
  registration->RemoveIthFixedImageAndProps(1);
  if (registration->GetNumberOfFixedImages() != 2)
    lok = false;
  registration->RemoveIthFixedImageAndProps(0);
  if (registration->GetNumberOfFixedImages() != 1)
    lok = false;
  registration->RemoveIthFixedImageAndProps(0);
  if (registration->GetNumberOfFixedImages() != 0)
    lok = false;
  registration->AddFixedImageAndProps(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion(), dummyProps);
  if (registration->GetNumberOfFixedImages() != 1)
    lok = false;
  registration->AddFixedImageAndProps(fixed3DImage2,
      fixed3DImage2->GetLargestPossibleRegion(), dummyProps);
  if (registration->GetNumberOfFixedImages() != 2)
    lok = false;
  registration->AddFixedImageAndProps(fixed3DImage3,
      fixed3DImage3->GetLargestPossibleRegion(), dummyProps);
  if (registration->GetNumberOfFixedImages() != 3)
    lok = false;
  registration->RemoveAllFixedImagesAndProps();
  if (registration->GetNumberOfFixedImages() != 0)
    lok = false;
  // multi-resolution tests:
  registration->SetNumberOfLevels(0); // invalid
  if (registration->GetNumberOfLevels() != 1)
    lok = false;
  registration->SetNumberOfLevels(4);
  if (registration->GetNumberOfLevels() != 4)
    lok = false;
  RegistrationType::FixedScheduleType fsch = registration->GetFixedSchedule();
  if (fsch[0][1] != 8 || fsch[1][0] != 4 || fsch[2][1] != 2 || fsch[3][0] != 1)
    lok = false;
  RegistrationType::MovingScheduleType msch = registration->GetMovingSchedule();
  if (msch[0][1] != 8 || msch[1][2] != 4 || msch[2][0] != 2 || msch[3][1] != 1)
    lok = false;
  registration->SetNumberOfLevels(1);
  if (registration->GetNumberOfLevels() != 1)
    lok = false;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing 1-way registration ... ")
  lok = true;
  vtkSmartPointer<vtkTimerLog> clock = vtkSmartPointer<vtkTimerLog>::New();
  double ts;
  CurrentRegistrationPrefix = "reg_1-way";
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
  // prepare transformation
  Transform3DType::Pointer transform = Transform3DType::New();
  transform->SetIdentity();
  registration->SetTransform(transform);
  Transform3DType::ParametersType initPars = transform->GetParameters();
  initPars.Fill(0);
  initPars[0] = 0.5;
  initPars[1] = -0.3;
  initPars[2] = -0.5;
  initPars[3] = 10;
  initPars[4] = -10;
  initPars[5] = -15;
  registration->SetInitialTransformParameters(initPars);
  // setup multi-resolution
  registration->SetNumberOfLevels(3);
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
  // prepare metric
  GDMetricType::Pointer gd1 = GDMetricType::New();
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
  cm->SetValueCompositeRule("gd1");
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
  DRR2DImageType::ConstPointer
      drr2D =
          static_cast<const DRR2DImageType *> (registration->Compute2DTestProjection(
              0, 2).GetPointer());
  if (ImageOutput)
    Write2DDRR(drr2D, CurrentRegistrationPrefix + "_test_proj2D_L2.mhd");
  drr2D = registration->Compute2DTestProjection(0, 1);
  if (ImageOutput)
    Write2DDRR(drr2D, CurrentRegistrationPrefix + "_test_proj2D_L1.mhd");
  drr2D = registration->Compute2DTestProjection(0, 0);
  if (ImageOutput)
    Write2DDRR(drr2D, CurrentRegistrationPrefix + "_test_proj2D_L0.mhd");
  RegistrationType::DRR3DImagePointer drr3D =
      registration->Compute3DTestProjection(0, 0);
  if (ImageOutput)
    Write3DDRR(drr3D, CurrentRegistrationPrefix + "_test_proj3D_L0.mhd");
  // registration
  registration->AddObserver(itk::StartEvent(), cscmd);
  registration->AddObserver(ora::StartMultiResolutionLevelEvent(), cscmd);
  registration->AddObserver(ora::StartOptimizationEvent(), cscmd);
  registration->AddObserver(itk::EndEvent(), cscmd);
  popt->AddObserver(itk::IterationEvent(), optcscmd);
  try
  {
    transform->SetParameters(registration->GetInitialTransformParameters());
    drr3D = registration->Compute3DTestProjection(0,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION.mhd");

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
          + "_REGISTERED_PROJECTION.mhd");
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
  if (ExtendedOutput)
    VERBOSE(<< (lok ? "    ... OK" : "FAILURE") << "\n")
  else
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing simple re-usage of 1-way registration ... ")
  lok = true;
  CurrentRegistrationPrefix = "reg_1-way-re-usage";
  try
  {
    transform->SetParameters(registration->GetInitialTransformParameters());
    drr3D = registration->Compute3DTestProjection(0,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION.mhd");

    clock->StartTimer();
    registration->Modified(); // must modify filter before!
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
          + "_REGISTERED_PROJECTION.mhd");
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
  if (ExtendedOutput)
    VERBOSE(<< (lok ? "    ... OK" : "FAILURE") << "\n")
  else
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing stopping of 1-way registration ... ")
  lok = true;
  CurrentRegistrationPrefix = "reg_1-way-stop";
  CommandType::Pointer stopcmd = CommandType::New();
  stopcmd->SetClientData(registration);
  stopcmd->SetCallback(StopRegistrationEvent);
  registration->AddObserver(ora::StopRequestedEvent(), stopcmd);
  try
  {
    transform->SetParameters(registration->GetInitialTransformParameters());

    clock->StartTimer();
    registration->Modified(); // must modify filter before!
    registration->Update(); // registration
    clock->StopTimer();
    ts = clock->GetElapsedTime();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    REGISTRATION-TIME (STOPPED): " << ts << " s.\n")
    }

  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
    std::cerr << "Registration ERROR: " << e << std::endl;
  }
  if (lok)
  {
    if (RecognizedStopIteration != 2) // stop @ iteration 2 !!!
    {
      lok = false;
    }
  }
  ok = ok && lok;
  if (ExtendedOutput)
    VERBOSE(<< (lok ? "    ... OK" : "FAILURE") << "\n")
  else
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing advanced re-usage of 1-way registration ... ")
  lok = true;
  CurrentRegistrationPrefix = "reg_1-way-advanced-re-usage";
  // exchange fixed image:
  registration->RemoveAllFixedImagesAndProps();
  registration->RemoveAllMetricFixedImageMappings();
  cm->RemoveAllMetricInputsAndVariables();
  props1 = NULL;
  props1 = DRRPropsType::New();
  props1->SetGeometryFromFixedImage(fixed3DImage3,
      fixed3DImage1->GetLargestPossibleRegion());
  fs[0] = fs3[0];
  fs[1] = fs3[1];
  fs[2] = fs3[2];
  props1->SetSourceFocalSpotPosition(fs);
  if (!props1->IsGeometryValid())
    lok = false;
  props1->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  // ITF:  0,0.0 500,0.05 1001,0.2 1200,0.3 1201,0.3 2500,1.0 3000,1.0
  itf = NULL;
  itf = ITFPointer::New();
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
  initPars[0] = 0.2;
  initPars[1] = -0.2;
  initPars[2] = -0.2;
  initPars[3] = 5;
  initPars[4] = -5;
  initPars[5] = -5;
  registration->SetInitialTransformParameters(initPars);
  // setup multi-resolution
  registration->SetNumberOfLevels(1);
  registration->AddFixedImageAndProps(fixed3DImage3,
      fixed3DImage3->GetLargestPossibleRegion(), props1);
  MSMetricType::Pointer ms1 = MSMetricType::New();
  cm->AddMetricInput(ms1, "ms1", "dms1");
  registration->AddMetricFixedImageMapping(ms1, 0);
  cm->SetValueCompositeRule("ms1");
  // optimizer
  popt->SetMaximize(false); // -> minimize ms
  popt->SetMaximumIteration(20);
  popt->SetMaximumLineIteration(10);
  popt->SetStepLength(1.5);
  popt->SetStepTolerance(0.05);
  try
  {
    transform->SetParameters(registration->GetInitialTransformParameters());
    drr3D = registration->Compute3DTestProjection(0,
        registration->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION.mhd");

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
          + "_REGISTERED_PROJECTION.mhd");
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
  registration->RemoveAllObservers();
  ok = ok && lok;
  if (ExtendedOutput)
    VERBOSE(<< (lok ? "    ... OK" : "FAILURE") << "\n")
  else
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing 2-way registration ... ")
  lok = true;
  CurrentRegistrationPrefix = "reg_2-way";
  RegistrationType::Pointer nreg = RegistrationType::New();
  // prepare ray-casting properties:
  props1 = NULL;
  props1 = DRRPropsType::New();
  props1->SetGeometryFromFixedImage(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion());
  fs[0] = fs1[0];
  fs[1] = fs1[1];
  fs[2] = fs1[2];
  props1->SetSourceFocalSpotPosition(fs);
  if (!props1->IsGeometryValid())
    lok = false;
  props1->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  // ITF:  0,0.0 500,0.05 1001,0.2 1200,0.3 1201,0.3 2500,1.0 3000,1.0
  itf = NULL;
  itf = ITFPointer::New();
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
  props2->SetGeometryFromFixedImage(fixed3DImage3,
      fixed3DImage3->GetLargestPossibleRegion());
  fs[0] = fs3[0];
  fs[1] = fs3[1];
  fs[2] = fs3[2];
  props2->SetSourceFocalSpotPosition(fs);
  if (!props2->IsGeometryValid())
    lok = false;
  props2->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  // ITF:  0,0.0 500,0.05 1001,0.2 1200,0.3 1201,0.3 2500,1.0 3000,1.0
  ITFPointer itf2 = ITFPointer::New();
  itf2->AddRGBPoint(0, 0, 0, 0);
  itf2->AddRGBPoint(500, 0.05, 0.05, 0.05);
  itf2->AddRGBPoint(1001, 0.2, 0.2, 0.2);
  itf2->AddRGBPoint(1200, 0.3, 0.3, 0.3);
  itf2->AddRGBPoint(1201, 0.3, 0.3, 0.3);
  itf2->AddRGBPoint(2500, 1.0, 1.0, 1.0);
  itf2->AddRGBPoint(3000, 1.0, 1.0, 1.01);
  props2->SetITF(itf2);
  if (!props2->AreAllPropertiesValid())
    lok = false;
  // prepare transformation
  transform->SetIdentity();
  nreg->SetTransform(transform);
  initPars = transform->GetParameters();
  initPars.Fill(0);
  initPars[0] = 0.5;
  initPars[1] = -0.3;
  initPars[2] = -0.5;
  initPars[3] = 10;
  initPars[4] = -10;
  initPars[5] = -15;
  nreg->SetInitialTransformParameters(initPars);
  // setup multi-resolution
  nreg->SetNumberOfLevels(3);
  nreg->SetUseAutoProjectionPropsAdjustment(true);
  nreg->SetAutoSamplingDistanceAdjustmentMode(0);
  nreg->RemoveAllMetricFixedImageMappings();
  nreg->SetUseMovingPyramidForFinalLevel(false);
  nreg->SetUseMovingPyramidForUnshrinkedLevels(false);
  nreg->SetMoving3DVolume(volume);
  nreg->SetUseFixedPyramidForFinalLevel(false);
  nreg->SetUseFixedPyramidForUnshrinkedLevels(false);
  nreg->AddFixedImageAndProps(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion(), props1);
  nreg->AddFixedImageAndProps(fixed3DImage3,
      fixed3DImage3->GetLargestPossibleRegion(), props2);
  // prepare metrics
  gd1 = GDMetricType::New();
  GDMetricType::Pointer gd2 = GDMetricType::New();
  // optimizer
  popt = PowellOptimizerType::New();
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
  cm = nreg->GetMetric();
  cm->AddMetricInput(gd1, "gd1", "dgd1");
  nreg->AddMetricFixedImageMapping(gd1, 0);
  cm->AddMetricInput(gd2, "gd2", "dgd2");
  nreg->AddMetricFixedImageMapping(gd2, 1);
  cm->SetValueCompositeRule("gd1 + gd2");
  // do not use derivatives here ...
  nreg->SetOptimizer(popt);
  cm->SetUseOptimizedValueComputation(false);
  //  cm->SetUseOptimizedValueComputation(true); // FIXME:
  // registration
  cscmd->SetClientData(nreg);
  nreg->AddObserver(itk::StartEvent(), cscmd);
  nreg->AddObserver(ora::StartMultiResolutionLevelEvent(), cscmd);
  nreg->AddObserver(ora::StartOptimizationEvent(), cscmd);
  nreg->AddObserver(itk::EndEvent(), cscmd);
  optcscmd = NULL;
  optcscmd = CommandType::New();
  optcscmd->SetClientData(nreg);
  optcscmd->SetCallback(OptimizerEvent);
  popt->AddObserver(itk::IterationEvent(), optcscmd);
  try
  {
    nreg->Initialize(); // test-DRRs require preceding Initialize()

    transform->SetParameters(nreg->GetInitialTransformParameters());
    drr3D = nreg->Compute3DTestProjection(0, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_1.mhd");
    drr3D = nreg->Compute3DTestProjection(1, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_2.mhd");

    clock->StartTimer();
    nreg->Update(); // registration
    clock->StopTimer();
    ts = clock->GetElapsedTime();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    REGISTRATION-TIME: " << ts << " s.\n")
    }

    drr3D = nreg->Compute3DTestProjection(0, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_1.mhd");
    drr3D = nreg->Compute3DTestProjection(1, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_2.mhd");
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
    std::cerr << "Registration ERROR: " << e << std::endl;
  }
  if (lok)
  {
    RegistrationType::TransformOutputConstPointer result = nreg->GetOutput();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    RESULT-TRANSFORMATION:\n")
      itk::Indent ind(4);
      result->Get()->Print(std::cout, ind);
    }
    if (!noResultsCheck)
      lok
          = VerifyRegistrationResult(strictResultsCheck,
              result->Get()->GetParameters(),
              nreg->GetInitialTransformParameters());
  }
  ok = ok && lok;
  if (ExtendedOutput)
    VERBOSE(<< (lok ? "    ... OK" : "FAILURE") << "\n")
  else
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing 4-way registration ... ")
  lok = true;
  CurrentRegistrationPrefix = "reg_4-way";
  nreg->RemoveAllFixedImagesAndProps();
  nreg->RemoveAllMetricFixedImageMappings();
  cm = nreg->GetMetric();
  cm->RemoveAllMetricInputsAndVariables();
  // prepare ray-casting properties:
  props1 = NULL;
  props1 = DRRPropsType::New();
  props1->SetGeometryFromFixedImage(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion());
  fs[0] = fs1[0];
  fs[1] = fs1[1];
  fs[2] = fs1[2];
  props1->SetSourceFocalSpotPosition(fs);
  if (!props1->IsGeometryValid())
    lok = false;
  props1->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  // ITF:  0,0.0 500,0.05 1001,0.2 1200,0.3 1201,0.3 2500,1.0 3000,1.0
  itf = NULL;
  itf = ITFPointer::New();
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
  props2 = DRRPropsType::New();
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
  DRRPropsType::Pointer props3 = DRRPropsType::New();
  props3->SetGeometryFromFixedImage(fixed3DImage3,
      fixed3DImage3->GetLargestPossibleRegion());
  fs[0] = fs3[0];
  fs[1] = fs3[1];
  fs[2] = fs3[2];
  props3->SetSourceFocalSpotPosition(fs);
  if (!props3->IsGeometryValid())
    lok = false;
  props3->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  props3->SetITF(itf);
  if (!props3->AreAllPropertiesValid())
    lok = false;
  DRRPropsType::Pointer props4 = DRRPropsType::New();
  props4->SetGeometryFromFixedImage(fixed3DImage4,
      fixed3DImage4->GetLargestPossibleRegion());
  fs[0] = fs4[0];
  fs[1] = fs4[1];
  fs[2] = fs4[2];
  props4->SetSourceFocalSpotPosition(fs);
  if (!props4->IsGeometryValid())
    lok = false;
  props4->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  props4->SetITF(itf);
  if (!props4->AreAllPropertiesValid())
    lok = false;
  // prepare transformation
  transform->SetIdentity();
  nreg->SetTransform(transform);
  initPars = transform->GetParameters();
  initPars.Fill(0);
  initPars[0] = 0.5;
  initPars[1] = -0.3;
  initPars[2] = -0.5;
  initPars[3] = 10;
  initPars[4] = -10;
  initPars[5] = -15;
  nreg->SetInitialTransformParameters(initPars);
  // setup multi-resolution
  nreg->SetNumberOfLevels(3);
  nreg->SetUseAutoProjectionPropsAdjustment(true);
  nreg->SetAutoSamplingDistanceAdjustmentMode(0);
  nreg->RemoveAllMetricFixedImageMappings();
  nreg->SetUseMovingPyramidForFinalLevel(false);
  nreg->SetUseMovingPyramidForUnshrinkedLevels(false);
  nreg->SetMoving3DVolume(volume);
  nreg->SetUseFixedPyramidForFinalLevel(false);
  nreg->SetUseFixedPyramidForUnshrinkedLevels(false);
  nreg->AddFixedImageAndProps(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion(), props1);
  nreg->AddFixedImageAndProps(fixed3DImage2,
      fixed3DImage2->GetLargestPossibleRegion(), props2);
  nreg->AddFixedImageAndProps(fixed3DImage3,
      fixed3DImage3->GetLargestPossibleRegion(), props3);
  nreg->AddFixedImageAndProps(fixed3DImage4,
      fixed3DImage4->GetLargestPossibleRegion(), props4);
  // prepare metrics
  gd1 = GDMetricType::New();
  gd2 = GDMetricType::New();
  ms1 = MSMetricType::New();
  MSMetricType::Pointer ms2 = MSMetricType::New();
  // optimizer
  pscales.SetSize(6);
  pscales[0] = 180 / 3.1415;
  pscales[1] = 180 / 3.1415;
  pscales[2] = 180 / 3.1415;
  pscales[3] = 1;
  pscales[4] = 1;
  pscales[5] = 1;
  popt->SetScales(pscales);
  popt->SetMaximize(false);
  popt->SetMaximumIteration(10);
  popt->SetMaximumLineIteration(5);
  popt->SetStepLength(3.0);
  popt->SetStepTolerance(1.5);
  // setup registration
  cm = nreg->GetMetric();
  cm->AddMetricInput(gd1, "gd1", "dgd1");
  nreg->AddMetricFixedImageMapping(gd1, 0);
  cm->AddMetricInput(gd2, "gd2", "dgd2");
  nreg->AddMetricFixedImageMapping(gd2, 1);
  cm->AddMetricInput(ms1, "ms1", "dms1");
  nreg->AddMetricFixedImageMapping(ms1, 2);
  cm->AddMetricInput(ms2, "ms2", "dms2");
  nreg->AddMetricFixedImageMapping(ms2, 3);
  cm->SetValueCompositeRule("ms1 + ms2 - gd1 - gd2");
  // do not use derivatives here ...
  nreg->SetOptimizer(popt);
  cm->SetUseOptimizedValueComputation(false);
  try
  {
    nreg->Initialize(); // test-DRRs require preceding Initialize()

    transform->SetParameters(nreg->GetInitialTransformParameters());
    drr3D = nreg->Compute3DTestProjection(0, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_1.mhd");
    drr3D = nreg->Compute3DTestProjection(1, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_2.mhd");
    drr3D = nreg->Compute3DTestProjection(2, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_3.mhd");
    drr3D = nreg->Compute3DTestProjection(3, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_4.mhd");

    clock->StartTimer();
    nreg->Update(); // registration
    clock->StopTimer();
    ts = clock->GetElapsedTime();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    REGISTRATION-TIME: " << ts << " s.\n")
    }

    drr3D = nreg->Compute3DTestProjection(0, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_1.mhd");
    drr3D = nreg->Compute3DTestProjection(1, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_2.mhd");
    drr3D = nreg->Compute3DTestProjection(2, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_3.mhd");
    drr3D = nreg->Compute3DTestProjection(3, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_4.mhd");
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
    std::cerr << "Registration ERROR: " << e << std::endl;
  }
  if (lok)
  {
    RegistrationType::TransformOutputConstPointer result = nreg->GetOutput();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    RESULT-TRANSFORMATION:\n")
      itk::Indent ind(4);
      result->Get()->Print(std::cout, ind);
    }
    if (!noResultsCheck)
      lok
          = VerifyRegistrationResult(strictResultsCheck,
              result->Get()->GetParameters(),
              nreg->GetInitialTransformParameters());
  }
  ok = ok && lok;
  if (ExtendedOutput)
    VERBOSE(<< (lok ? "    ... OK" : "FAILURE") << "\n")
  else
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing 3-way registration with different sizes ... ")
  lok = true;
  CurrentRegistrationPrefix = "reg_3-way";
  nreg->RemoveAllFixedImagesAndProps();
  nreg->RemoveAllMetricFixedImageMappings();
  cm = nreg->GetMetric();
  cm->RemoveAllMetricInputsAndVariables();
  // prepare ray-casting properties:
  props1 = NULL;
  props1 = DRRPropsType::New();
  props1->SetGeometryFromFixedImage(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion());
  fs[0] = fs1[0];
  fs[1] = fs1[1];
  fs[2] = fs1[2];
  props1->SetSourceFocalSpotPosition(fs);
  if (!props1->IsGeometryValid())
    lok = false;
  props1->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  // ITF:  0,0.0 500,0.05 1001,0.2 1200,0.3 1201,0.3 2500,1.0 3000,1.0
  itf = NULL;
  itf = ITFPointer::New();
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
  props2 = DRRPropsType::New();
  props2->SetGeometryFromFixedImage(fixed3DImage21,
      fixed3DImage21->GetLargestPossibleRegion());
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
  props4 = DRRPropsType::New();
  props4->SetGeometryFromFixedImage(fixed3DImage42,
      fixed3DImage42->GetLargestPossibleRegion());
  fs[0] = fs4[0];
  fs[1] = fs4[1];
  fs[2] = fs4[2];
  props4->SetSourceFocalSpotPosition(fs);
  if (!props4->IsGeometryValid())
    lok = false;
  props4->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  props4->SetITF(itf);
  if (!props4->AreAllPropertiesValid())
    lok = false;
  // prepare transformation
  transform->SetIdentity();
  nreg->SetTransform(transform);
  initPars = transform->GetParameters();
  initPars.Fill(0);
  initPars[0] = 0.5;
  initPars[1] = -0.3;
  initPars[2] = -0.5;
  initPars[3] = 10;
  initPars[4] = -10;
  initPars[5] = -15;
  nreg->SetInitialTransformParameters(initPars);
  // setup multi-resolution
  nreg->SetNumberOfLevels(3);
  nreg->SetUseAutoProjectionPropsAdjustment(true);
  nreg->SetAutoSamplingDistanceAdjustmentMode(0);
  nreg->RemoveAllMetricFixedImageMappings();
  nreg->SetUseMovingPyramidForFinalLevel(false);
  nreg->SetUseMovingPyramidForUnshrinkedLevels(false);
  nreg->SetMoving3DVolume(volume);
  nreg->SetUseFixedPyramidForFinalLevel(false);
  nreg->SetUseFixedPyramidForUnshrinkedLevels(false);
  nreg->AddFixedImageAndProps(fixed3DImage1,
      fixed3DImage1->GetLargestPossibleRegion(), props1);
  nreg->AddFixedImageAndProps(fixed3DImage21,
      fixed3DImage21->GetLargestPossibleRegion(), props2);
  nreg->AddFixedImageAndProps(fixed3DImage42,
      fixed3DImage42->GetLargestPossibleRegion(), props4);
  // prepare metrics
  gd1 = GDMetricType::New();
  gd2 = GDMetricType::New();
  ms1 = MSMetricType::New();
  // optimizer
  pscales.SetSize(6);
  pscales[0] = 180 / 3.1415;
  pscales[1] = 180 / 3.1415;
  pscales[2] = 180 / 3.1415;
  pscales[3] = 1;
  pscales[4] = 1;
  pscales[5] = 1;
  popt->SetScales(pscales);
  popt->SetMaximize(false);
  popt->SetMaximumIteration(10);
  popt->SetMaximumLineIteration(5);
  popt->SetStepLength(3.0);
  popt->SetStepTolerance(1.5);
  // setup registration
  cm = nreg->GetMetric();
  cm->AddMetricInput(gd1, "gd1", "dgd1");
  nreg->AddMetricFixedImageMapping(gd1, 0);
  cm->AddMetricInput(gd2, "gd2", "dgd2");
  nreg->AddMetricFixedImageMapping(gd2, 1);
  cm->AddMetricInput(ms1, "ms1", "dms1");
  nreg->AddMetricFixedImageMapping(ms1, 2);
  cm->SetValueCompositeRule("ms1 - gd1 - gd2");
  // do not use derivatives here ...
  nreg->SetOptimizer(popt);
  cm->SetUseOptimizedValueComputation(false);
  try
  {
    nreg->Initialize(); // test-DRRs require preceding Initialize()

    transform->SetParameters(nreg->GetInitialTransformParameters());
    drr3D = nreg->Compute3DTestProjection(0, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_1.mhd");
    drr3D = nreg->Compute3DTestProjection(1, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_2.mhd");
    drr3D = nreg->Compute3DTestProjection(2, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_3.mhd");

    clock->StartTimer();
    nreg->Update(); // registration
    clock->StopTimer();
    ts = clock->GetElapsedTime();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    REGISTRATION-TIME: " << ts << " s.\n")
    }

    drr3D = nreg->Compute3DTestProjection(0, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_1.mhd");
    drr3D = nreg->Compute3DTestProjection(1, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_2.mhd");
    drr3D = nreg->Compute3DTestProjection(2, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_3.mhd");
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
    std::cerr << "Registration ERROR: " << e << std::endl;
  }
  if (lok)
  {
    RegistrationType::TransformOutputConstPointer result = nreg->GetOutput();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    RESULT-TRANSFORMATION:\n")
      itk::Indent ind(4);
      result->Get()->Print(std::cout, ind);
    }
    if (!noResultsCheck)
      lok
          = VerifyRegistrationResult(strictResultsCheck,
              result->Get()->GetParameters(),
              nreg->GetInitialTransformParameters());
  }
  ok = ok && lok;
  if (ExtendedOutput)
    VERBOSE(<< (lok ? "    ... OK" : "FAILURE") << "\n")
  else
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing 2-way smaller-region-registration ... ")
  lok = true;
  CurrentRegistrationPrefix = "reg_2-way_smaller-region";
  nreg->RemoveAllFixedImagesAndProps();
  nreg->RemoveAllMetricFixedImageMappings();
  cm = nreg->GetMetric();
  cm->RemoveAllMetricInputsAndVariables();
  // prepare ray-casting properties:
  props1 = NULL;
  props1 = DRRPropsType::New();
  DRRImageType::RegionType reg1;
  reg1.SetIndex(0, 90);//150);
  reg1.SetIndex(1, 110);//170);
  reg1.SetIndex(2, 0);
  reg1.SetSize(0, 110);//100);
  reg1.SetSize(1, 170);//80);
  reg1.SetSize(2, 1);
  props1->SetGeometryFromFixedImage(fixed3DImage1, reg1);
  fs[0] = fs1[0];
  fs[1] = fs1[1];
  fs[2] = fs1[2];
  props1->SetSourceFocalSpotPosition(fs);
  if (!props1->IsGeometryValid())
    lok = false;
  props1->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  // ITF:  0,0.0 500,0.05 1001,0.2 1200,0.3 1201,0.3 2500,1.0 3000,1.0
  itf = NULL;
  itf = ITFPointer::New();
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
  props3 = DRRPropsType::New();
  DRRImageType::RegionType reg3;
  reg3.SetIndex(0, 150);
  reg3.SetIndex(1, 160);
  reg3.SetIndex(2, 0);
  reg3.SetSize(0, 110);
  reg3.SetSize(1, 90);
  reg3.SetSize(2, 1);
  props3->SetGeometryFromFixedImage(fixed3DImage3, reg3);
  fs[0] = fs3[0];
  fs[1] = fs3[1];
  fs[2] = fs3[2];
  props3->SetSourceFocalSpotPosition(fs);
  if (!props3->IsGeometryValid())
    lok = false;
  props3->ComputeAndSetSamplingDistanceFromVolume(volume->GetSpacing(), 0);
  props3->SetITF(itf);
  if (!props3->AreAllPropertiesValid())
    lok = false;
  // prepare transformation
  transform->SetIdentity();
  nreg->SetTransform(transform);
  initPars = transform->GetParameters();
  initPars.Fill(0);
  initPars[0] = 0.3;
  initPars[1] = -0.25;
  initPars[2] = -0.3;
  initPars[3] = 10;
  initPars[4] = -10;
  initPars[5] = -15;
  nreg->SetInitialTransformParameters(initPars);
  // setup multi-resolution
  nreg->SetNumberOfLevels(3);
  nreg->SetUseAutoProjectionPropsAdjustment(true);
  nreg->SetAutoSamplingDistanceAdjustmentMode(0);
  nreg->RemoveAllMetricFixedImageMappings();
  nreg->SetUseMovingPyramidForFinalLevel(false);
  nreg->SetUseMovingPyramidForUnshrinkedLevels(false);
  nreg->SetMoving3DVolume(volume);
  nreg->SetUseFixedPyramidForFinalLevel(false);
  nreg->SetUseFixedPyramidForUnshrinkedLevels(false);
  nreg->AddFixedImageAndProps(fixed3DImage1, reg1, props1);
  nreg->AddFixedImageAndProps(fixed3DImage3, reg3, props3);
  // prepare metrics
  gd1 = GDMetricType::New();
  gd2 = GDMetricType::New();
  // optimizer
  pscales.SetSize(6);
  pscales[0] = 180 / 3.1415;
  pscales[1] = 180 / 3.1415;
  pscales[2] = 180 / 3.1415;
  pscales[3] = 1;
  pscales[4] = 1;
  pscales[5] = 1;
  popt->SetScales(pscales);
  popt->SetMaximize(false);
  popt->SetMaximumIteration(10);
  popt->SetMaximumLineIteration(5);
  popt->SetStepLength(1.0);
  popt->SetStepTolerance(0.5);
  // setup registration
  cm = nreg->GetMetric();
  cm->AddMetricInput(gd1, "gd1", "dgd1");
  nreg->AddMetricFixedImageMapping(gd1, 0);
  cm->AddMetricInput(gd2, "gd2", "dgd2");
  nreg->AddMetricFixedImageMapping(gd2, 1);
  cm->SetValueCompositeRule("-gd1 - gd2");
  // do not use derivatives here ...
  nreg->SetOptimizer(popt);
  cm->SetUseOptimizedValueComputation(false);
  try
  {
    nreg->Initialize(); // test-DRRs require preceding Initialize()

    transform->SetParameters(nreg->GetInitialTransformParameters());
    drr3D = nreg->Compute3DTestProjection(0, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_1.mhd");
    drr3D = nreg->Compute3DTestProjection(1, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_UNREGISTERED_PROJECTION_2.mhd");

    clock->StartTimer();
    nreg->Update(); // registration
    clock->StopTimer();
    ts = clock->GetElapsedTime();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    REGISTRATION-TIME: " << ts << " s.\n")
    }

    drr3D = nreg->Compute3DTestProjection(0, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_1.mhd");
    drr3D = nreg->Compute3DTestProjection(1, nreg->GetNumberOfLevels() - 1);
    if (ImageOutput)
      Write3DDRR(drr3D, CurrentRegistrationPrefix
          + "_REGISTERED_PROJECTION_2.mhd");
    drr3D = nreg->Compute3DTestProjection(2, nreg->GetNumberOfLevels() - 1);
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
    std::cerr << "Registration ERROR: " << e << std::endl;
  }
  if (lok)
  {
    RegistrationType::TransformOutputConstPointer result = nreg->GetOutput();
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n    RESULT-TRANSFORMATION:\n")
      itk::Indent ind(4);
      result->Get()->Print(std::cout, ind);
    }
    if (!noResultsCheck)
      lok
          = VerifyRegistrationResult(strictResultsCheck,
              result->Get()->GetParameters(),
              nreg->GetInitialTransformParameters());
  }
  ok = ok && lok;
  if (ExtendedOutput)
    VERBOSE(<< (lok ? "    ... OK" : "FAILURE") << "\n")
  else
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  if (registration->GetReferenceCount() == 1 && nreg->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  registration = NULL; // reference counter must be zero!
  nreg = NULL;

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

