//
#include <stdlib.h>
#include <iostream>
#include <string>

#include <itkImage.h>
#include <itkRigid2DTransform.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionIteratorWithIndex.h>

#include "oraOneToOneInterpolateImageFunction.h"

#include "BasicUnitTestIncludes.hxx"

/**
 * Tests base functionality of:
 *
 *   ora::OneToOneInterpolateImageFunction
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::OneToOneInterpolateImageFunction
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.2
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
        helpLines, false, false);
    return EXIT_SUCCESS;
  }

  VERBOSE(<< "\nTesting 1:1 interpolate image function.\n")

  const unsigned int Dimension = 2;
  typedef float PixelType;
  typedef itk::Image<PixelType, Dimension> ImageType;
  typedef ora::OneToOneInterpolateImageFunction<ImageType, double>
      InterpolatorType;
  typedef itk::Rigid2DTransform<double> TransformType;
  typedef itk::ImageRegionIterator<ImageType> IteratorType;
  typedef itk::ImageRegionIteratorWithIndex<ImageType> IteratorWIType;
  typedef itk::ContinuousIndex<double, Dimension> ContIndexType;

  // Generate reference image with specified pixel values:
  ImageType::SizeType isize;
  isize[0] = 123;
  isize[1] = 259;
  ImageType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  ImageType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  ImageType::SpacingType ispacing;
  ispacing[0] = 1.11;
  ispacing[1] = 0.23;
  ImageType::PointType iorigin;
  iorigin[0] = -13.4;
  iorigin[1] = 5.75;
  ImageType::DirectionType idirection;
  TransformType::Pointer t = TransformType::New();
  t->SetRotation(-0.26179938779914943654); // -15°
  idirection = t->GetMatrix();
  ImageType::Pointer refImage = ImageType::New();
  refImage->SetSpacing(ispacing);
  refImage->SetOrigin(iorigin);
  refImage->SetDirection(idirection);
  refImage->SetRegions(iregion);
  refImage->Allocate();
  IteratorType it(refImage, iregion);
  PixelType v = 0;
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    it.Set(v);
    v += 2;
    if (v > 22)
      v = 0;
  }

  // Generate test image that geometrically equals the reference image:
  ImageType::Pointer testImage = ImageType::New();
  iregion.SetSize(isize);
  testImage->SetSpacing(ispacing);
  testImage->SetOrigin(iorigin);
  testImage->SetDirection(idirection);
  testImage->SetRegions(iregion);
  testImage->Allocate();

  // Now test 1:1 interpolator functionality with the test image:
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  interpolator->SetInputImage(refImage);

  VERBOSE(<< "  * Interpolator with index ... ")
  IteratorWIType twit(testImage, iregion);
  PixelType vt;
  bool ok = true;
  for (twit.GoToBegin(), it.GoToBegin(); !twit.IsAtEnd(); ++twit, ++it)
  {
    vt = interpolator->EvaluateAtIndex(twit.GetIndex());
    v = it.Get();
    if (v != vt)
    {
      ok = false;
      break;
    }
  }
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  ImageType::PointType p;
  if (ok)
  {
    VERBOSE(<< "  * Interpolator with physical points ... ")
    for (twit.GoToBegin(), it.GoToBegin(); !twit.IsAtEnd(); ++twit, ++it)
    {
      refImage->TransformIndexToPhysicalPoint(twit.GetIndex(), p);
      vt = interpolator->Evaluate(p);
      v = it.Get();
      if (v != vt)
      {
        ok = false;
        break;
      }
    }
    VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")
  }

  if (ok)
  {
    // NOTE: in reality Evaluate() already uses continuous index evalution
    // internally; however, test it explicitly here
    VERBOSE(<< "  * Interpolator with continuous index ... ")
    ContIndexType ci;
    for (twit.GoToBegin(), it.GoToBegin(); !twit.IsAtEnd(); ++twit, ++it)
    {
      refImage->TransformIndexToPhysicalPoint(twit.GetIndex(), p);
      refImage->TransformPhysicalPointToContinuousIndex(p, ci);
      vt = interpolator->EvaluateAtContinuousIndex(ci);
      v = it.Get();
      if (v != vt)
      {
        ok = false;
        break;
      }
    }
    VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")
  }

  VERBOSE(<< "  * Final reference count check ... ")
  if (interpolator->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  interpolator = NULL; // reference counter must be zero!

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

