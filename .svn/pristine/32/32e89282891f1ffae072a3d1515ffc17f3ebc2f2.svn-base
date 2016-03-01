//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <time.h>
#include <cstddef> /* Workaround bug in ITK 3.20 */

#include <itkImage.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkRigid2DTransform.h>
#include <itkVersorRigid3DTransform.h>
#include <itkResampleImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkImageFileWriter.h>
#include <itkRealTimeClock.h>
#include <itkMath.h>

#include "oraMultiImageToImageMetric.h"
#include "oraOneToOneInterpolateImageFunction.h"
#include "oraParametrizableIdentityTransform.h"

#include "BasicUnitTestIncludes.hxx"

/**
 * Tests base functionality of:
 *
 *   ora::MultiImageToImageMetric
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::MultiImageToImageMetric
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @author Markus <markus.neuner (at) pmu.ac.at>
 * @version 1.0.1
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
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, true, false);
    return EXIT_SUCCESS;
  }

  bool ok = true;

  VERBOSE(<< "\nTesting multi image to image metric.\n")
  const unsigned int Dimension = 2;
  typedef float PixelType;
  typedef itk::Image<PixelType, Dimension> ImageType;
  typedef ora::MultiImageToImageMetric<ImageType, ImageType> MetricType;
  typedef MetricType::ParametersType ParametersType;
  typedef itk::MeanSquaresImageToImageMetric<ImageType, ImageType>
      MSQMetricType;
  typedef itk::ImageRegionIterator<ImageType> IteratorType;
  typedef itk::Rigid2DTransform<double> TransformType;
  typedef ora::ParametrizableIdentityTransform<double, 2> ITransformType;
  typedef itk::VersorRigid3DTransform<double> Transform3DType;
  typedef ora::OneToOneInterpolateImageFunction<ImageType, double>
      InterpolatorType;
  typedef itk::LinearInterpolateImageFunction<ImageType, double>
      LInterpolatorType;
  typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleType;
  typedef itk::ImageFileWriter<ImageType> WriterType;

  // Generate reference images and moving image with specified pixel values:
  ImageType::SizeType isize;
  isize[0] = 40;
  isize[1] = 60;
  ImageType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  ImageType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  ImageType::SpacingType ispacing;
  ispacing[0] = 0.5;
  ispacing[1] = 0.5;
  ImageType::PointType iorigin;
  iorigin[0] = -10.0;
  iorigin[1] = 5.0;
  ImageType::DirectionType idirection;
  idirection.SetIdentity();
  ImageType::Pointer refImage = ImageType::New();
  refImage->SetSpacing(ispacing);
  refImage->SetOrigin(iorigin);
  refImage->SetDirection(idirection);
  refImage->SetRegions(iregion);
  refImage->Allocate();
  IteratorType it(refImage, iregion);
  PixelType v = 0;
  int x = 0;
  int y = 0;
  int c = 0;
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    x = c % isize[0];
    y = c / isize[0];
    if (x > 10 && x < 30 && y > 10 && y < 50)
      v = 1;
    else
      v = 0;
    it.Set(v);
    c++;
  }
  ImageType::Pointer refImage2 = ImageType::New();
  refImage2->SetSpacing(ispacing);
  refImage2->SetOrigin(iorigin);
  refImage2->SetDirection(idirection);
  refImage2->SetRegions(iregion);
  refImage2->Allocate();
  IteratorType it2(refImage2, iregion);
  c = 0;
  for (it2.GoToBegin(); !it2.IsAtEnd(); ++it2)
  {
    x = c % isize[0];
    y = c / isize[0];
    if (x > 15 && x < 25 && y > 20 && y < 40)
      v = 0;
    else
      v = 1;
    it2.Set(v);
    c++;
  }
  ImageType::Pointer refImage3 = ImageType::New();
  refImage3->SetSpacing(ispacing);
  refImage3->SetOrigin(iorigin);
  refImage3->SetDirection(idirection);
  refImage3->SetRegions(iregion);
  refImage3->Allocate();
  IteratorType it3(refImage3, iregion);
  c = 0;
  for (it3.GoToBegin(); !it3.IsAtEnd(); ++it3)
  {
    x = c % isize[0];
    y = c / isize[0];
    if (x > 15 && x < 25 && y > 20 && y < 40)
      v = 1;
    else
      v = 0;
    it3.Set(v);
    c++;
  }
  ImageType::Pointer movImage = ImageType::New();
  movImage->SetSpacing(ispacing);
  movImage->SetOrigin(iorigin);
  movImage->SetDirection(idirection);
  movImage->SetRegions(iregion);
  movImage->Allocate();
  IteratorType itm(movImage, iregion);
  c = 0;
  for (itm.GoToBegin(); !itm.IsAtEnd(); ++itm)
  {
    x = c % isize[0];
    y = c / isize[0];
    if (x > 10 && x < 30 && y > 10 && y < 50)
      v = 1;
    else
      v = 0;
    itm.Set(v);
    c++;
  }

  ResampleType::Pointer resample = ResampleType::New(); // moving image transf.
  TransformType::Pointer transform = TransformType::New();
  LInterpolatorType::Pointer linterpolator = LInterpolatorType::New();
  resample->SetInterpolator(linterpolator);
  resample->SetTransform(transform);
  resample->SetInput(movImage);
  resample->SetOutputParametersFromImage(movImage);

  ITransformType::Pointer itransform = ITransformType::New();
  ParametersType pars(itransform->GetNumberOfParameters());
  ParametersType tpars(transform->GetNumberOfParameters());
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  MSQMetricType::Pointer msqm1 = MSQMetricType::New();
  msqm1->SetFixedImage(refImage);
  msqm1->SetMovingImage(resample->GetOutput());
  msqm1->SetTransform(itransform);
  msqm1->SetInterpolator(interpolator);
  msqm1->SetFixedImageRegion(refImage->GetLargestPossibleRegion());
  msqm1->Initialize();
  MSQMetricType::Pointer msqm2 = MSQMetricType::New();
  msqm2->SetFixedImage(refImage2);
  msqm2->SetMovingImage(resample->GetOutput());
  msqm2->SetTransform(itransform);
  msqm2->SetInterpolator(interpolator);
  msqm2->SetFixedImageRegion(refImage2->GetLargestPossibleRegion());
  msqm2->Initialize();
  MSQMetricType::Pointer msqm3 = MSQMetricType::New();
  msqm3->SetFixedImage(refImage3);
  msqm3->SetMovingImage(resample->GetOutput());
  msqm3->SetTransform(itransform);
  msqm3->SetInterpolator(interpolator);
  msqm3->SetFixedImageRegion(refImage3->GetLargestPossibleRegion());
  msqm3->Initialize();

  WriterType::Pointer w = WriterType::New();
  w->SetInput(refImage);
  w->SetFileName("fixedImage.mhd");
  try
  {
    if (ImageOutput)
      w->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    ok = false;
  }
  w->SetInput(refImage2);
  w->SetFileName("fixedImage2.mhd");
  try
  {
    if (ImageOutput)
      w->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    ok = false;
  }
  w->SetInput(refImage3);
  w->SetFileName("fixedImage3.mhd");
  try
  {
    if (ImageOutput)
      w->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    ok = false;
  }
  w->SetInput(resample->GetOutput());
  tpars.Fill(0);
  transform->SetParameters(tpars);
  w->SetFileName("movingImage1.mhd");
  try
  {
    if (ImageOutput)
      w->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    ok = false;
  }
  tpars[0] = 0.3;
  tpars[1] = 5;
  tpars[2] = 10;
  transform->SetParameters(tpars);
  w->SetFileName("movingImage2.mhd");
  try
  {
    if (ImageOutput)
      w->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    ok = false;
  }

  MetricType::Pointer mmetric = MetricType::New();

  VERBOSE(<< "  * Basic checks ... ")
  std::ostringstream os;
  mmetric->Print(os, 0);
  if (os.str().length() <= 0)
    ok = false;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  // test whether or not the metric components are working:
  VERBOSE(<< "  * Metric components check ... \n")
  try
  {
    tpars.Fill(0);
    pars.Fill(0);
    // the metrics have a simple identity transformation set; as in 2D/3D-
    // registration we have to apply the real transformation directly to the
    // moving image:
    transform->SetParameters(tpars);
    VERBOSE(<< "    VALUE1 (" << pars << "): " << msqm1->GetValue(pars) << "\n")
    VERBOSE(<< "    VALUE2 (" << pars << "): " << msqm2->GetValue(pars) << "\n")
    VERBOSE(<< "    VALUE3 (" << pars << "): " << msqm3->GetValue(pars) << "\n")
    tpars[0] = 0.3;
    tpars[1] = 5;
    tpars[2] = 10;
    pars[0] = 0.3;
    pars[1] = 5;
    pars[2] = 10;
    pars[3] = -4.3;
    pars[4] = 1.0033;
    pars[5] = -0.482;
    // the metrics have a simple identity transformation set; as in 2D/3D-
    // registration we have to apply the real transformation directly to the
    // moving image:
    transform->SetParameters(tpars);
    // internal metric interpolator is connected to moving image and needs an
    // update (this could also be achieved calling Initialize() which will be
    // slower in general); NOTE: there are further things which have to be
    // re-computed eventually ... (this will depend on the metric type)!
    interpolator->GetInputImage()->GetSource()->Update();
    // resample->Update(); ... same effect
    VERBOSE(<< "    VALUE1 (" << pars << "): " << msqm1->GetValue(pars) << "\n")
    VERBOSE(<< "    VALUE2 (" << pars << "): " << msqm2->GetValue(pars) << "\n")
    VERBOSE(<< "    VALUE3 (" << pars << "): " << msqm3->GetValue(pars) << "\n")
  }
  catch (itk::ExceptionObject &e)
  {
    ok = false;
  }
  VERBOSE(<< (ok ? "    ... OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Input metric tests ... ")
  if (ok)
  {
    mmetric->RemoveAllMetricInputs();
    mmetric->AddMetricInput(NULL); // invalid
    if (mmetric->GetNumberOfMetricInputs() > 0)
      ok = false;
    mmetric->AddMetricInput(msqm1); // valid
    if (mmetric->GetNumberOfMetricInputs() != 1)
      ok = false;
    mmetric->AddMetricInput(msqm1); // valid
    if (mmetric->GetNumberOfMetricInputs() != 2)
      ok = false;
    mmetric->AddMetricInput(msqm2); // invalid
    if (mmetric->GetNumberOfMetricInputs() != 3)
      ok = false;
    mmetric->RemoveAllMetricInputs();
    if (mmetric->GetNumberOfMetricInputs() > 0)
      ok = false;
    mmetric->AddMetricInput(msqm1);
    mmetric->AddMetricInput(msqm2);
    mmetric->AddMetricInput(msqm3);
    if (mmetric->GetNumberOfMetricInputs() != 3)
      ok = false;
    mmetric->RemoveIthMetricInput(1);
    if (mmetric->GetNumberOfMetricInputs() != 2)
      ok = false;
    mmetric->RemoveIthMetricInput(0);
    if (mmetric->GetNumberOfMetricInputs() != 1)
      ok = false;
    mmetric->RemoveIthMetricInput(0);
    if (mmetric->GetNumberOfMetricInputs() != 0)
      ok = false;
  }
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Test multiple value evaluation ... ")
  Transform3DType::Pointer transform3D = Transform3DType::New(); // 3D transf.
  itransform->SetConnected3DTransform(transform3D);
  itransform->SetStealJacobianFromConnected3DTransform(true);
  mmetric->SetReInitializeMetricsBeforeEvaluating(false);
  tpars.Fill(0);
  transform->SetParameters(tpars);
  resample->Update(); // update connected moving images of the metrics
  pars.Fill(0);
  mmetric->AddMetricInput(msqm1);
  mmetric->AddMetricInput(msqm2);
  mmetric->AddMetricInput(msqm3);
  // values:
  itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
  double ts = clock->GetTimeStamp();
  MetricType::MeasureType analyticval(mmetric->GetNumberOfMetricInputs());
  srand(time(NULL));
  for (int u = 0; u < 1000; u++)
  {
    transform->SetAngle(((double) (rand() % 101 - 50)) / 200.);
    resample->Update(); // update connected moving images of the metrics
    MetricType::MeasureType mval = mmetric->GetValue(pars);
    analyticval[0] = msqm1->GetValue(pars);
    analyticval[1] = msqm2->GetValue(pars);
    analyticval[2] = msqm3->GetValue(pars);
    if (mval.Size() == mmetric->GetNumberOfMetricInputs())
    {
      for (unsigned int j = 0; j < mmetric->GetNumberOfMetricInputs(); j++)
      {
        // 5 decimals check of the individual mapped outputs:
        if (itk::Math::Round<int, double>(mval[j] * 100000) != itk::Math::Round<int, double>(analyticval[j] * 100000))
          ok = false;
      }
    }
    else
    {
      ok = false;
    }
  }
  ts = (clock->GetTimeStamp() - ts) * 1000;
  VERBOSE(<< " val.-time: [" << ts << " ms] ")
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Test multiple derivative evaluation ... ")
  tpars.Fill(0);
  transform->SetParameters(tpars);
  resample->Update(); // update connected moving images of the metrics
  pars.Fill(0);
  // derivatives:
  ts = clock->GetTimeStamp();
  itk::Array<double> analyticder1(mmetric->GetNumberOfParameters());
  itk::Array<double> analyticder2(mmetric->GetNumberOfParameters());
  itk::Array<double> analyticder3(mmetric->GetNumberOfParameters());
  srand(time(NULL));
  for (int u = 0; u < 1000; u++)
  {
    transform->SetAngle(((double) (rand() % 101 - 50)) / 200.);
    ITransformType::OutputVectorType transl;
    transl[0] = ((double) (rand() % 101 - 50)) / 10.;
    transl[1] = ((double) (rand() % 101 - 50)) / 10.;
    transform->SetTranslation(transl);
    resample->Update(); // update connected moving images of the metrics
    MetricType::DerivativeType mder;
    Transform3DType::AxisType ax;
    ax[0] = 0;
    ax[1] = 1;
    ax[2] = 0;
    transform3D->SetRotation(ax, ((double) (rand() % 101 - 50)) / 200.);
    pars = transform3D->GetParameters(); // simply to vary the parameters
    mmetric->GetDerivative(pars, mder);
    msqm1->GetDerivative(pars, analyticder1);
    msqm2->GetDerivative(pars, analyticder2);
    msqm3->GetDerivative(pars, analyticder3);
    if (mder.rows() == mmetric->GetNumberOfMetricInputs() && mder.cols()
        == mmetric->GetNumberOfParameters())
    {
      // 5 decimals check of the individual mapped outputs:
      for (unsigned int j = 0; j < mmetric->GetNumberOfParameters(); j++)
      {
        if (itk::Math::Round<int, double>(mder[0][j] * 100000) != itk::Math::Round<int, double>(analyticder1[j] * 100000))
          ok = false;
      }
      for (unsigned int j = 0; j < mmetric->GetNumberOfParameters(); j++)
      {
        if (itk::Math::Round<int, double>(mder[1][j] * 100000) != itk::Math::Round<int, double>(analyticder2[j] * 100000))
          ok = false;
      }
      for (unsigned int j = 0; j < mmetric->GetNumberOfParameters(); j++)
      {
        if (itk::Math::Round<int, double>(mder[2][j] * 100000) != itk::Math::Round<int, double>(analyticder3[j] * 100000))
          ok = false;
      }
    }
    else
    {
      ok = false;
    }
  }
  ts = (clock->GetTimeStamp() - ts) * 1000;
  VERBOSE(<< " val.-time: [" << ts << " ms] ")
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  if (mmetric->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  mmetric = NULL; // reference counter must be zero!

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
