//
#include <stdlib.h>
#include <iostream>
#include <string>
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

#include "oraCompositeImageToImageMetric.h"
#include "oraOneToOneInterpolateImageFunction.h"
#include "oraParametrizableIdentityTransform.h"

#include "BasicUnitTestIncludes.hxx"

/**
 * Tests base functionality of:
 *
 *   ora::CompositeImageToImageMetric
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::CompositeImageToImageMetric
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @author Markus <markus.neuner (at) pmu.ac.at>
 * @version 1.1.1
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

  VERBOSE(<< "\nTesting composite image to image metric.\n")

  const unsigned int Dimension = 2;
  typedef float PixelType;
  typedef itk::Image<PixelType, Dimension> ImageType;

  typedef ora::CompositeImageToImageMetric<ImageType, ImageType> MetricType;
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

  bool ok = true;

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

  MetricType::Pointer cmetric = MetricType::New();

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

  VERBOSE(<< "  * Input metric tests (incl. variable naming) ... ")
  if (ok)
  {
    cmetric->RemoveAllMetricInputsAndVariables();
    cmetric->AddMetricInput(NULL, "v", "g"); // invalid
    if (cmetric->GetNumberOfMetricInputs() > 0)
      ok = false;
    cmetric->AddMetricInput(msqm1, "", "g"); // invalid
    if (cmetric->GetNumberOfMetricInputs() > 0)
      ok = false;
    cmetric->AddMetricInput(msqm1, "", ""); // invalid
    if (cmetric->GetNumberOfMetricInputs() > 0)
      ok = false;
    cmetric->AddMetricInput(msqm1, "v", "g"); // invalid
    if (cmetric->GetNumberOfMetricInputs() != 1)
      ok = false;
    cmetric->RemoveAllMetricInputsAndVariables();
    if (cmetric->GetNumberOfMetricInputs() > 0)
      ok = false;
    // finally really set input metrics:
    cmetric->AddMetricInput(msqm1, "m_1", "grad1");
    cmetric->AddMetricInput(msqm2, "m_2", "grad2");
    cmetric->AddMetricInput(msqm3, "m_3", "grad3");
    if (cmetric->GetNumberOfMetricInputs() != 3)
      ok = false;
  }
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Basic composite rule checks ... ")
  if (ok)
  {
    cmetric->SetValueCompositeRule(""); // invalid
    cmetric->SetDerivativeCompositeRule("");
    std::vector<std::size_t> indices =
        cmetric-> ExtractReferencedVariableIndices(true);
    if (indices.size() != 0)
      ok = false;
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 0)
      ok = false;

    // value rule tests:
    cmetric->SetValueCompositeRule("m_1");
    indices = cmetric->ExtractReferencedVariableIndices(true);
    if (indices.size() != 1 || indices[0] != 0)
      ok = false;
    cmetric->SetValueCompositeRule("m_2");
    indices = cmetric->ExtractReferencedVariableIndices(true);
    if (indices.size() != 1 || indices[0] != 1)
      ok = false;
    cmetric->SetValueCompositeRule("m_3");
    indices = cmetric->ExtractReferencedVariableIndices(true);
    if (indices.size() != 1 || indices[0] != 2)
      ok = false;
    cmetric->SetValueCompositeRule("m_3 * 2.0 + m_12"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(true);
    if (indices.size() != 1 || indices[0] != 2)
      ok = false;
    cmetric->SetValueCompositeRule("m_3 * 2.0 + sin(m_1) * m_12"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(true);
    if (indices.size() != 2 || indices[0] != 0 || indices[1] != 2)
      ok = false;
    cmetric->SetValueCompositeRule("m_3 * 2.0 + sin(5.0) * m_12 + m_1"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(true);
    if (indices.size() != 2 || indices[0] != 0 || indices[1] != 2)
      ok = false;
    cmetric->SetValueCompositeRule("bm_3 * 2.0 + sin(m_2) * m_12"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(true);
    if (indices.size() != 1 || indices[0] != 1)
      ok = false;
    cmetric->SetValueCompositeRule(
        "if(m_1>m_2, abs(sin(m_3))*0.2+3.4 * 2.0 + ceil(m_1)*sin(4.3) * ln(32*m_12),if(m_2=m_3,3.4*3,23.0))"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(true);
    if (indices.size() != 3 || indices[0] != 0 || indices[1] != 1 || indices[2]
        != 2)
      ok = false;

    // derivative rule tests:
    cmetric->SetDerivativeCompositeRule("grad1[x]");
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 1 || indices[0] != 0)
      ok = false;
    cmetric->SetDerivativeCompositeRule("grad1[0]###grad1[1]###grad1[2]");
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 1 || indices[0] != 0)
      ok = false;
    cmetric->SetDerivativeCompositeRule("grad2[x]");
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 1 || indices[0] != 1)
      ok = false;
    cmetric->SetDerivativeCompositeRule("grad2[0]###grad2[1]###grad2[2]");
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 1 || indices[0] != 1)
      ok = false;
    cmetric->SetDerivativeCompositeRule("grad3[x]");
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 1 || indices[0] != 2)
      ok = false;
    cmetric->SetDerivativeCompositeRule("grad3[0]###grad3[1]###grad3[2]");
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 1 || indices[0] != 2)
      ok = false;
    cmetric->SetDerivativeCompositeRule("grad3[x] * 2.0 + grad12[x]"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 1 || indices[0] != 2)
      ok = false;
    cmetric->SetDerivativeCompositeRule(
        "grad3[0] * 2.0 + grad12[0]###grad3[1] / 5.0 * 2.0 + grad12[1]###grad3[2] * 2.0 + sin(grad12[2])"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 1 || indices[0] != 2)
      ok = false;
    cmetric->SetDerivativeCompositeRule(
        "grad3[x] * 2.0 + sin(grad1[x]) * grad12[x]"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 2 || indices[0] != 0 || indices[1] != 2)
      ok = false;
    cmetric->SetDerivativeCompositeRule(
        "grad3[0] * 2.0 + sin(grad1[0]) * grad12[0]###grad3[1] / 4.5 * 2.0 + sin(grad1[1]) * grad12[1]###log(grad3[2]) * 2.0 + sin(grad1[2]) * grad12[2]"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 2 || indices[0] != 0 || indices[1] != 2)
      ok = false;
    cmetric->SetDerivativeCompositeRule(
        "grad3[x] * 2.0 + sin(5.0) * grad12[x] + grad1[x]"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 2 || indices[0] != 0 || indices[1] != 2)
      ok = false;
    cmetric->SetDerivativeCompositeRule(
        "grad3[0] * 2.0 + sin(5.0) * grad12[0] + grad1[0]###grad3[1] * 2.0 + sin(5.0) * grad12[1] + grad1[1]###grad3[2] * 2.0 + sin(5.0) * grad12[2] + grad1[2]"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 2 || indices[0] != 0 || indices[1] != 2)
      ok = false;
    cmetric->SetDerivativeCompositeRule(
        "bgrad3[x] * 2.0 + sin(grad2[x]) * grad12[x]"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 1 || indices[0] != 1)
      ok = false;
    cmetric->SetDerivativeCompositeRule(
        "bgrad3[0] * 2.0 + sin(grad2[0]) * grad12[0]###bgrad3[1] * 2.0 + sin(grad2[1]) * grad12[1]###bgrad3[2] * 2.0 + sin(grad2[2]) * grad12[2]"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 1 || indices[0] != 1)
      ok = false;
    cmetric->SetDerivativeCompositeRule(
        "if(grad1[x]>grad2[x], abs(sin(grad3[x]))*0.2+3.4 * 2.0 + ceil(grad1[x])*sin(4.3) * ln(32*grad12[x]),if(grad2[x]=grad3[x],3.4*3,23.0))"); // test!
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 3 || indices[0] != 0 || indices[1] != 1 || indices[2]
        != 2)
      ok = false;

    // finally set real composition rules:
    cmetric->SetValueCompositeRule("1.0 * m_1 + 2.0 * ln(m_2 + 1) + 3.0 * m_3");
    indices = cmetric->ExtractReferencedVariableIndices(true);
    if (indices.size() != 3 || indices[0] != 0 || indices[1] != 1 || indices[2]
        != 2)
      ok = false;
    cmetric->SetDerivativeCompositeRule("grad1[x] + grad2[x] + grad3[x]");
    indices = cmetric->ExtractReferencedVariableIndices(false);
    if (indices.size() != 3 || indices[0] != 0 || indices[1] != 1 || indices[2]
        != 2)
      ok = false;
  }
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Non-optimized composite evaluation ... ")
  Transform3DType::Pointer transform3D = Transform3DType::New(); // 3D transf.
  itransform->SetConnected3DTransform(transform3D);
  itransform->SetStealJacobianFromConnected3DTransform(true);
  cmetric->SetUseOptimizedValueComputation(false); // no optimization
  tpars.Fill(0);
  transform->SetParameters(tpars);
  resample->Update(); // update connected moving images of the metrics
  pars.Fill(0);
  // value:
  itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
  double ts = clock->GetTimeStamp();
  MetricType::MeasureType analyticval = 1.0 * msqm1->GetValue(pars) + 2.0
      * log(msqm2->GetValue(pars) + 1) + 3.0 * msqm3->GetValue(pars);
  for (int u = 0; u < 1000; u++)
  {
    MetricType::MeasureType cval = cmetric->GetValue(pars); // formula above!
    if (itk::Math::Round<int, double>(cval * 100000) != itk::Math::Round<int, double>(analyticval * 100000)) // 5 decimals check
      ok = false;
  }
  ts = (clock->GetTimeStamp() - ts) * 1000;
  VERBOSE(<< " val.-time: [" << ts << " ms] ")
  // gradient:
  cmetric->SetUseOptimizedDerivativeComputation(false); // no optimization
  pars[0] = 0.12;
  pars[1] = -0.25;
  pars[2] = -0.42;
  pars[3] = 40;
  pars[4] = 60;
  pars[5] = -30;
  MetricType::DerivativeType deriv1;
  msqm1->GetDerivative(pars, deriv1);
  MetricType::DerivativeType deriv2;
  msqm2->GetDerivative(pars, deriv2);
  MetricType::DerivativeType deriv3;
  msqm3->GetDerivative(pars, deriv3);
  MetricType::DerivativeType analyticderiv(deriv1.Size());
  for (unsigned int v = 0; v < deriv1.Size(); v++)
    analyticderiv[v] = deriv1[v] + deriv2[v] + deriv3[v];
  ts = clock->GetTimeStamp();
  for (int u = 0; u < 1000; u++)
  {
    MetricType::DerivativeType cderivative;
    cmetric->GetDerivative(pars, cderivative);
    for (unsigned int v = 0; v < deriv1.Size(); v++)
    {
      if (itk::Math::Round<int, double>(analyticderiv[v] * 100000) != itk::Math::Round<int, double>(cderivative[v] * 100000))
        ok = false;
    }
  }
  ts = (clock->GetTimeStamp() - ts) * 1000;
  cmetric->SetDerivativeCompositeRule(
      "grad1[0] + grad2[0] + grad3[0]###grad1[1] + grad2[1] + grad3[1]###grad1[2] + grad2[2] + grad3[2]###grad1[3] + grad2[3] + grad3[3]###grad1[4] + grad2[4] + grad3[4]###grad1[5] + grad2[5] + grad3[5]");
  double ts2 = clock->GetTimeStamp();
  for (int u = 0; u < 1000; u++)
  {
    MetricType::DerivativeType cderivative;
    cmetric->GetDerivative(pars, cderivative);
    for (unsigned int v = 0; v < deriv1.Size(); v++)
    {
      if (itk::Math::Round<int, double>(analyticderiv[v] * 100000) != itk::Math::Round<int, double>(cderivative[v] * 100000))
        ok = false;
    }
  }
  ts2 = (clock->GetTimeStamp() - ts2) * 1000;
  VERBOSE(<< " ~deriv.-time: [" << ((ts + ts2) / 2) << " ms] ")
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Optimized composite evaluation ... ")
  cmetric->SetUseOptimizedValueComputation(true); // optimization
  tpars.Fill(0);
  transform->SetParameters(tpars);
  resample->Update(); // update connected moving images of the metrics
  pars.Fill(0);
  ts = clock->GetTimeStamp();
  analyticval = 1.0 * msqm1->GetValue(pars) + 2.0 * log(msqm2->GetValue(pars)
      + 1) + 3.0 * msqm3->GetValue(pars);
  for (int u = 0; u < 1000; u++)
  {
    MetricType::MeasureType cval = cmetric->GetValue(pars); // formula above!
    if (itk::Math::Round<int, double>(cval * 100000) != itk::Math::Round<int, double>(analyticval * 100000)) // 5 decimals check
      ok = false;
  }
  ts = (clock->GetTimeStamp() - ts) * 1000;
  VERBOSE(<< " val.-time: [" << ts << " ms] ")
  // gradient
  cmetric->SetUseOptimizedDerivativeComputation(true); // optimization
  // NOTE:
  // problem: mean-squares metric gradient evaluation is not thread-safe as it
  // uses the transform's Jacobian internally! Therefore we test multi-threaded
  // evaluation with one single thread (override #CPUs):
  cmetric->SetOverrideNumberOfAvailableCPUs(1);
  pars[0] = 0.12;
  pars[1] = -0.25;
  pars[2] = -0.42;
  pars[3] = 40;
  pars[4] = 60;
  pars[5] = -30;
  msqm1->GetDerivative(pars, deriv1);
  msqm2->GetDerivative(pars, deriv2);
  msqm3->GetDerivative(pars, deriv3);
  for (unsigned int v = 0; v < deriv1.Size(); v++)
    analyticderiv[v] = deriv1[v] + deriv2[v] + deriv3[v];
  ts = clock->GetTimeStamp();
  for (int u = 0; u < 1000; u++)
  {
    MetricType::DerivativeType cderivative;
    cmetric->GetDerivative(pars, cderivative);
    for (unsigned int v = 0; v < deriv1.Size(); v++)
    {
      if (itk::Math::Round<int, double>(analyticderiv[v] * 100000) != itk::Math::Round<int, double>(cderivative[v] * 100000))
        ok = false;
    }
  }
  ts = (clock->GetTimeStamp() - ts) * 1000;
  cmetric->SetDerivativeCompositeRule(
      "grad1[0] + grad2[0] + grad3[0]###grad1[1] + grad2[1] + grad3[1]###grad1[2] + grad2[2] + grad3[2]###grad1[3] + grad2[3] + grad3[3]###grad1[4] + grad2[4] + grad3[4]###grad1[5] + grad2[5] + grad3[5]");
  ts2 = clock->GetTimeStamp();
  for (int u = 0; u < 1000; u++)
  {
    MetricType::DerivativeType cderivative;
    cmetric->GetDerivative(pars, cderivative);
    for (unsigned int v = 0; v < deriv1.Size(); v++)
    {
      if (itk::Math::Round<int, double>(analyticderiv[v] * 100000) != itk::Math::Round<int, double>(cderivative[v] * 100000))
        ok = false;
    }
  }
  ts2 = (clock->GetTimeStamp() - ts2) * 1000;
  VERBOSE(<< " ~deriv.-time: [" << ((ts + ts2) / 2) << " ms] ")
  cmetric->SetOverrideNumberOfAvailableCPUs(0); // set back
  transform3D = NULL;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  if (cmetric->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  cmetric = NULL; // reference counter must be zero!

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

