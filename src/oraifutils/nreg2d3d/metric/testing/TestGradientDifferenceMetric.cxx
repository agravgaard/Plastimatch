//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include <time.h>
#include <cstddef> /* Workaround bug in ITK 3.20 */

#include <itkImage.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <itkRealTimeClock.h>
#include <itkCommand.h>
#include <itkRigid2DTransform.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>

#include "oraGradientDifferenceImageToImageMetric.h"

#include "BasicUnitTestIncludes.hxx"

const unsigned int Dimension2D = 2;
typedef float PixelType;
typedef float GradientPixelType;
typedef unsigned char MaskPixelType;
typedef itk::Image<PixelType, Dimension2D> Image2DType;
typedef itk::Image<MaskPixelType, Dimension2D> MaskImage2DType;
typedef itk::ImageRegionIterator<Image2DType> Iterator2DType;
typedef ora::GradientDifferenceImageToImageMetric<Image2DType, Image2DType,
    GradientPixelType> GDMetricType;
typedef itk::LinearInterpolateImageFunction<Image2DType, double>
    InterpolatorType;
typedef itk::Rigid2DTransform<double> TransformType;
typedef TransformType::ParametersType ParametersType;
typedef itk::CStyleCommand CommandType;
typedef itk::ImageMaskSpatialObject<2> MaskSpatialObjectType;

// extended output
bool ExtendedOutput = false;

/** Generate a specified test image for some tests. **/
Image2DType::Pointer GenerateTestImage(const char *fname)
{
  Image2DType::SizeType isize;
  isize[0] = 400;
  isize[1] = 600;
  Image2DType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  Image2DType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  Image2DType::SpacingType ispacing;
  ispacing[0] = .5;
  ispacing[1] = .5;
  Image2DType::PointType iorigin;
  iorigin[0] = .0;
  iorigin[1] = .0;
  Image2DType::DirectionType idirection;
  idirection.SetIdentity();
  Image2DType::Pointer image = Image2DType::New();
  image->SetSpacing(ispacing);
  image->SetOrigin(iorigin);
  image->SetDirection(idirection);
  image->SetRegions(iregion);
  image->Allocate();

  Iterator2DType it(image, image->GetLargestPossibleRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    Image2DType::IndexType i = it.GetIndex();
    if (i[0] > 50 && i[1] > 50 && i[0] < 350 && i[1] < 550)
    {
      if (i[0] < 100 && i[1] < 250)
      {
        it.Set(150);
      }
      else
      {
        if (i[0] < 200 && i[1] < 350)
        {
          it.Set(125);
        }
        else
        {
          if (i[0] > 200 && i[1] > 350)
            it.Set(235 + static_cast<PixelType> (rand() % 41) - 20);
          else
            it.Set(100 + static_cast<PixelType> (rand() % 21) - 10);
        }
      }
    }
    else
    {
      it.Set(0);
    }
  }

  if (ImageOutput && fname)
  {
    typedef itk::ImageFileWriter<Image2DType> WriterType;
    WriterType::Pointer w = WriterType::New();
    w->SetFileName(fname);
    w->SetInput(image);
    w->Update();
  }

  return image;
}

/** Generate a circular mask image which can later be translated into a spatial object. **/
MaskImage2DType::Pointer GenerateTestMask(const char *fname, int cx, int cy,
    int r, int &numPixels)
{
  MaskImage2DType::Pointer mask = MaskImage2DType::New();
  Image2DType::Pointer ref = GenerateTestImage(NULL);

  mask->SetRegions(ref->GetLargestPossibleRegion());
  mask->Allocate();
  mask->SetOrigin(ref->GetOrigin());
  mask->SetSpacing(ref->GetSpacing());
  mask->SetDirection(ref->GetDirection());
  mask->FillBuffer(0);

  int radius2 = r * r;

  numPixels = 0;
  typedef itk::ImageRegionIterator<MaskImage2DType> MaskIteratorType;
  MaskIteratorType mit(mask, mask->GetLargestPossibleRegion());
  while (!mit.IsAtEnd())
  {
    MaskImage2DType::IndexType idx = mit.GetIndex();
    int xx = idx[0] - cx;
    int yy = idx[1] - cy;
    if ((xx * xx + yy * yy) <= radius2)
    {
      mit.Set(1);
      numPixels++;
    }
    else
    {
      mit.Set(0);
    }
    ++mit;
  }

  if (ImageOutput && fname)
  {
    typedef itk::ImageFileWriter<MaskImage2DType> WriterType;
    WriterType::Pointer w = WriterType::New();
    w->SetFileName(fname);
    w->SetInput(mask);
    w->Update();
  }

  return mask;
}

/**
 * Tests base functionality of:
 *
 *   ora::GradientDifferenceImageToImageMetric
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::GradientDifferenceImageToImageMetric
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  helpLines.push_back("  -xo or --extended-output ... extended message output");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL, helpLines, true, false);
    return EXIT_SUCCESS;
  }
  // advanced arguments:
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-xo" || std::string(argv[i]) == "--extended-output")
    {
      ExtendedOutput = true;
      continue;
    }
  }

  VERBOSE(<< "\nTesting gradient difference image to image metric.\n")
  bool ok = true;

  GDMetricType::Pointer gd = GDMetricType::New();

  VERBOSE(<< "  * String stream test ... ")
  bool lok = true;
  try
  {
    std::ostringstream os;
    os << gd;
    if (os.str().length() <= 0)
      lok = false;
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")


  VERBOSE(<< "  * Simple value / derivative computation ... ")
  lok = true;
  srand(time(NULL));
  // the 2 images should be slightly different (rand())
  Image2DType::Pointer image1 = GenerateTestImage("image_1.mhd");
  Image2DType::Pointer image2 = GenerateTestImage("image_2.mhd");
  TransformType::Pointer transform = TransformType::New();
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  gd->SetFixedImage(image1);
  gd->SetFixedImageRegion(image1->GetLargestPossibleRegion());
  gd->SetMovingImage(image2);
  gd->SetTransform(transform);
  gd->SetInterpolator(interpolator);
  ParametersType pars(transform->GetNumberOfParameters());
  pars.fill(0);
  GDMetricType::ScalesType dscales;
  dscales.SetSize(transform->GetNumberOfParameters());
  dscales[0] = 0.01745329251994329577; // 1 degree
  dscales[1] = 1; // mm
  dscales[2] = 1; // mm
  try
  {
    gd->SetDerivativeScales(dscales);
    double maxvalue = -1e20;
    int maxIdx = -1;
    gd->Initialize();
    GDMetricType::MeasureType value;
    GDMetricType::DerivativeType derivative;
    for (int j = 0; j < 10; j++)
    {
      if (j == 0)
      {
        pars.Fill(0.0);
      }
      else if (j == 1)
      {
        pars[0] = -0.2;
        pars[1] = 5.5;
        pars[2] = -3.3;
      }
      else
      {
        pars[0] += 0.06;
        pars[1] -= 0.9;
        pars[2] += 0.85;
      }
      // evaluate!
      gd->GetValueAndDerivative(pars, value, derivative);
      if (value > maxvalue)
      {
        maxvalue = value;
        maxIdx = j;
      }
      if (ExtendedOutput)
        VERBOSE(<< "\n  parameters = " << pars << "\n   value = " << value
            << "\n   derivative = " << derivative
            << "\n   #pixels = " << gd->GetNumberOfPixelsCounted())
    }
    if (maxIdx != 0) // 0-parameters
      lok = false;
    if (ExtendedOutput)
      VERBOSE(<< "\n")
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "Error error: " << e << std::endl;
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Check no-overlap behavior ... ")
  lok = true;
  GDMetricType::MeasureType value;
  GDMetricType::DerivativeType derivative;
  pars.Fill(0);
  pars[2] = 3000; // no overlap!
  gd->SetNoOverlapReactionMode(0); // exception
  try
  {
    value = gd->GetValue(pars);
    lok = false; // exception awaited!
  }
  catch (itk::ExceptionObject &e)
  {
    ; // expected
  }
  gd->SetNoOverlapReactionMode(0); // exception
  try
  {
    gd->GetDerivative(pars, derivative);
    lok = false; // exception awaited!
  }
  catch (itk::ExceptionObject &e)
  {
    ; // expected
  }
  try
  {
    gd->GetValueAndDerivative(pars, value, derivative);
    lok = false; // exception awaited!
  }
  catch (itk::ExceptionObject &e)
  {
    ; // expected
  }
  gd->SetNoOverlapReactionMode(1); // specified measure value!
  const double nov = 21234.75;
  gd->SetNoOverlapMetricValue(21234.75);
  try
  {
    value = gd->GetValue(pars);
    if (value != nov)
    {
      lok = false;
    }
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Fixed + moving image mask, fixed image region test ... ")
  lok = true;
  Image2DType::RegionType freg;
  Image2DType::SizeType fregsz = image1->GetLargestPossibleRegion().GetSize();
  fregsz[0] = static_cast<Image2DType::SizeValueType> (fregsz[0] * 0.9);
  fregsz[1] = static_cast<Image2DType::SizeValueType> (fregsz[1] * 0.85);
  Image2DType::IndexType fregidx = image1->GetLargestPossibleRegion().GetIndex();
  fregidx[0] = static_cast<Image2DType::IndexValueType> (fregsz[0] * 0.07);
  fregidx[1] = static_cast<Image2DType::IndexValueType> (fregsz[1] * 0.03);
  freg.SetIndex(fregidx);
  freg.SetSize(fregsz);
  gd->SetFixedImageRegion(freg);
  int mpix1 = 0;
  MaskImage2DType::Pointer fmask = GenerateTestMask("mask1.mhd", 200, 300, 150, mpix1);
  MaskSpatialObjectType::Pointer fspatial = MaskSpatialObjectType::New();
  fspatial->SetImage(fmask);
  fspatial->Update();
  gd->SetFixedImageMask(fspatial);
  int mpix2 = 0;
  MaskImage2DType::Pointer mmask = GenerateTestMask("mask2.mhd", 100, 200, 100, mpix2);
  MaskSpatialObjectType::Pointer mspatial = MaskSpatialObjectType::New();
  mspatial->SetImage(mmask);
  mspatial->Update();
  gd->SetMovingImageMask(mspatial);
  try
  {
    double maxvalue = -1e20;
    int maxIdx = -1;
    gd->Initialize();
    GDMetricType::MeasureType value;
    GDMetricType::DerivativeType derivative;
    for (int j = 0; j < 10; j++)
    {
      if (j == 0)
      {
        pars.Fill(0.0);
      }
      else if (j == 1)
      {
        pars[0] = -0.2;
        pars[1] = 5.5;
        pars[2] = -3.3;
      }
      else
      {
        pars[0] += 0.06;
        pars[1] -= 0.9;
        pars[2] += 0.85;
      }
      // evaluate!
      gd->GetValueAndDerivative(pars, value, derivative);
      if (value > maxvalue)
      {
        maxvalue = value;
        maxIdx = j;
      }
      if (ExtendedOutput)
        VERBOSE(<< "\n  parameters = " << pars << "\n   value = " << value
            << "\n   derivative = " << derivative
            << "\n   #pixels = " << gd->GetNumberOfPixelsCounted())
    }
    if (maxIdx != 0) // 0-parameters
      lok = false;
    if (ExtendedOutput)
      VERBOSE(<< "\n")
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "Error error: " << e << std::endl;
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  if (gd->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  gd = NULL; // reference counter must be zero!
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

  return EXIT_SUCCESS;
}
