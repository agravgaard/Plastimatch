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
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <itkRealTimeClock.h>
#include <itkRigid2DTransform.h>
#include <itkTranslationTransform.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkImageMaskSpatialObject.h>

#include "oraDRRITFLikelihoodCostFunction.h"

#include "BasicUnitTestIncludes.hxx"

// extended output
bool ExtendedOutput = false;

const unsigned int Dimension2D = 2;
const unsigned int Dimension3D = 3;
typedef unsigned char PixelType;
typedef float FloatPixelType;
typedef unsigned char MaskPixelType;
typedef itk::Image<PixelType, Dimension2D> Image2DType;
typedef itk::Image<MaskPixelType, Dimension2D> MaskImage2DType;
typedef itk::Image<FloatPixelType, Dimension3D> Image3DType;
typedef itk::Image<MaskPixelType, Dimension3D> MaskImage3DType;
typedef itk::ImageRegionIterator<Image2DType> Iterator2DType;
typedef itk::ImageRegionIterator<MaskImage2DType> MaskIterator2DType;
typedef itk::ImageRegionIterator<Image3DType> Iterator3DType;
typedef itk::ImageRegionIterator<MaskImage3DType> MaskIterator3DType;
typedef ora::DRRITFLikelihoodCostFunction<Image2DType, Image2DType> CostFunction2DType;
typedef ora::DRRITFLikelihoodCostFunction<Image3DType, Image3DType> CostFunction3DType;
typedef itk::Rigid2DTransform<double> Transform2DType;
typedef itk::TranslationTransform<double, 3> Transform3DType;
typedef itk::LinearInterpolateImageFunction<Image2DType, double> Interpolator2DType;
typedef itk::LinearInterpolateImageFunction<Image3DType, double> Interpolator3DType;
typedef itk::ImageMaskSpatialObject<2> Mask2DSpatialObjectType;
typedef itk::ImageMaskSpatialObject<3> Mask3DSpatialObjectType;

/** Generate a random 2D test image for some tests. **/
Image2DType::Pointer Generate2DTestImage(const char *fname)
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

/** Generate a random 2D mask image for some tests. **/
MaskImage2DType::Pointer Generate2DMaskImage(const char *fname)
{
  MaskImage2DType::SizeType isize;
  isize[0] = 400;
  isize[1] = 600;
  MaskImage2DType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  MaskImage2DType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  MaskImage2DType::SpacingType ispacing;
  ispacing[0] = .5;
  ispacing[1] = .5;
  MaskImage2DType::PointType iorigin;
  iorigin[0] = .0;
  iorigin[1] = .0;
  MaskImage2DType::DirectionType idirection;
  idirection.SetIdentity();
  MaskImage2DType::Pointer image = Image2DType::New();
  image->SetSpacing(ispacing);
  image->SetOrigin(iorigin);
  image->SetDirection(idirection);
  image->SetRegions(iregion);
  image->Allocate();

  MaskIterator2DType it(image, image->GetLargestPossibleRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    if (rand() % 5 == 0)
      it.Set(1);
    else
      it.Set(0);
  }

  if (ImageOutput && fname)
  {
    typedef itk::ImageFileWriter<MaskImage2DType> WriterType;
    WriterType::Pointer w = WriterType::New();
    w->SetFileName(fname);
    w->SetInput(image);
    w->Update();
  }

  return image;
}

/** Generate a random 3D test image for some tests. **/
Image3DType::Pointer Generate3DTestImage(const char *fname)
{
  Image3DType::SizeType isize;
  isize[0] = 40;
  isize[1] = 60;
  isize[2] = 50;
  Image3DType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  iindex[2] = 0;
  Image3DType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  Image3DType::SpacingType ispacing;
  ispacing[0] = .5;
  ispacing[1] = .5;
  ispacing[2] = 1.2;
  Image3DType::PointType iorigin;
  iorigin[0] = .0;
  iorigin[1] = .0;
  iorigin[2] = .0;
  Image3DType::DirectionType idirection;
  idirection.SetIdentity();
  Image3DType::Pointer image = Image3DType::New();
  image->SetSpacing(ispacing);
  image->SetOrigin(iorigin);
  image->SetDirection(idirection);
  image->SetRegions(iregion);
  image->Allocate();

  Iterator3DType it(image, image->GetLargestPossibleRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    Image3DType::IndexType i = it.GetIndex();
    if (i[0] > 5 && i[1] > 5 && i[0] < 35 && i[1] < 55)
    {
      if (i[2] > 6 && i[2] < 25)
      {
        if (i[0] < 10 && i[1] < 25)
        {
          it.Set(150);
        }
        else
        {
          if (i[0] < 20 && i[1] < 35)
          {
            it.Set(125);
          }
          else
          {
            if (i[0] > 20 && i[1] > 35)
              it.Set(235 + static_cast<PixelType> (rand() % 41) - 20);
            else
              it.Set(100 + static_cast<PixelType> (rand() % 21) - 10);
          }
        }
      }
      else if (i[2] >= 25 && i[2] < 44)
      {
        if (i[0] < 10 && i[1] < 25)
        {
          it.Set(250);
        }
        else
        {
          if (i[0] < 20 && i[1] < 35)
          {
            it.Set(225);
          }
          else
          {
            if (i[0] > 20 && i[1] > 35)
              it.Set(335 + static_cast<PixelType> (rand() % 41) - 20);
            else
              it.Set(200 + static_cast<PixelType> (rand() % 21) - 10);
          }
        }
      }
      else
      {
        it.Set(0);
      }
    }
    else
    {
      it.Set(0);
    }
  }

  if (ImageOutput && fname)
  {
    typedef itk::ImageFileWriter<Image3DType> WriterType;
    WriterType::Pointer w = WriterType::New();
    w->SetFileName(fname);
    w->SetInput(image);
    w->Update();
  }

  return image;
}

/** Generate a random 3D mask image for some tests. **/
MaskImage3DType::Pointer Generate3DMaskImage(const char *fname)
{
  MaskImage3DType::SizeType isize;
  isize[0] = 40;
  isize[1] = 60;
  isize[2] = 50;
  MaskImage3DType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  iindex[2] = 0;
  MaskImage3DType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  MaskImage3DType::SpacingType ispacing;
  ispacing[0] = .5;
  ispacing[1] = .5;
  ispacing[2] = 1.2;
  MaskImage3DType::PointType iorigin;
  iorigin[0] = .0;
  iorigin[1] = .0;
  iorigin[2] = .0;
  MaskImage3DType::DirectionType idirection;
  idirection.SetIdentity();
  MaskImage3DType::Pointer image = MaskImage3DType::New();
  image->SetSpacing(ispacing);
  image->SetOrigin(iorigin);
  image->SetDirection(idirection);
  image->SetRegions(iregion);
  image->Allocate();

  MaskIterator3DType it(image, image->GetLargestPossibleRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    if (rand() % 5 == 0)
      it.Set(1);
    else
      it.Set(0);
  }

  if (ImageOutput && fname)
  {
    typedef itk::ImageFileWriter<MaskImage3DType> WriterType;
    WriterType::Pointer w = WriterType::New();
    w->SetFileName(fname);
    w->SetInput(image);
    w->Update();
  }

  return image;
}

/**
 * Tests base functionality of:
 *
 *   ora::DRRITFLikelihoodCostFunction
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::DRRITFLikelihoodCostFunction
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

  VERBOSE(<< "\nTesting DRR ITF likelihood cost function.\n")
  bool ok = true;

  VERBOSE(<< "  * Generating test data sets ... ")
  bool lok = true;
  srand(time(NULL)); // initialize randomizer
  Image2DType::Pointer image2D1 = Generate2DTestImage("image_2D_1.mhd");
  Image2DType::Pointer image2D2 = Generate2DTestImage("image_2D_2.mhd");
  Image3DType::Pointer image3D1 = Generate3DTestImage("image_3D_1.mhd");
  Image3DType::Pointer image3D2 = Generate3DTestImage("image_3D_2.mhd");
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Unmasked 2D tests ... ")
  lok = true;
  CostFunction2DType::Pointer cf2D = CostFunction2DType::New();
  cf2D->SetFixedImage(image2D1);
  cf2D->SetMovingImage(image2D2);
  cf2D->SetFixedImageRegion(image2D1->GetLargestPossibleRegion());
  Transform2DType::Pointer transform2D = Transform2DType::New();
  transform2D->SetIdentity();
  cf2D->SetTransform(transform2D);
  Interpolator2DType::Pointer interp2D = Interpolator2DType::New();
  cf2D->SetInterpolator(interp2D);
  cf2D->SetFixedHistogramClipAtEnds(true);
  cf2D->SetFixedHistogramMinIntensity(0);
  cf2D->SetFixedHistogramMaxIntensity(255);
  cf2D->SetFixedNumberOfHistogramBins(100);
  try
  {
    cf2D->Initialize();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "Initialization-ERROR: " << e << "\n";
    lok = false;
  }
  // sample a few transformation positions around "optimum":
  CostFunction2DType::ParametersType pars2D;
  pars2D.SetSize(3);
  if (ExtendedOutput)
    VERBOSE(<< "\n")
  try
  {
    for (int i = 0; i < 80; i++)
    {
      if (i > 0)
      {
        pars2D[0] = 0.2 - (double) (rand() % 10001) / 25000.;
        pars2D[1] = 10. - (double) (rand() % 10001) / 500.;
        pars2D[2] = 10. - (double) (rand() % 10001) / 500.;
      }
      else
      {
        pars2D.fill(0);
      }
      double value = cf2D->GetValue(pars2D);
      if (ExtendedOutput)
        VERBOSE(<< "  " << i << ";" << value << ";" << pars2D[0] << ";" <<
            pars2D[1] << ";" << pars2D[2] << "\n")
    }
    // re-configure:
    cf2D->SetFixedHistogramMinIntensity(30);
    cf2D->SetFixedHistogramMaxIntensity(200);
    cf2D->SetFixedNumberOfHistogramBins(150);
    cf2D->Initialize();
    for (int i = 0; i < 80; i++)
    {
      if (i > 0)
      {
        pars2D[0] = 0.2 - (double) (rand() % 10001) / 25000.;
        pars2D[1] = 10. - (double) (rand() % 10001) / 500.;
        pars2D[2] = 10. - (double) (rand() % 10001) / 500.;
      }
      else
      {
        pars2D.fill(0);
      }
      cf2D->SetMapOutsideIntensitiesToZeroProbability(false);
      double value = cf2D->GetValue(pars2D);
      if (ExtendedOutput)
        VERBOSE(<< "  " << i << "a;" << value << ";" << pars2D[0] << ";" <<
            pars2D[1] << ";" << pars2D[2] << "\n")
      cf2D->SetMapOutsideIntensitiesToZeroProbability(true);
      value = cf2D->GetValue(pars2D);
      if (ExtendedOutput)
        VERBOSE(<< "  " << i << "b;" << value << ";" << pars2D[0] << ";" <<
            pars2D[1] << ";" << pars2D[2] << "\n")
    }
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "Value-ERROR: " << e << "\n";
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Masked 2D tests ... ")
  lok = true;
  cf2D->SetFixedHistogramClipAtEnds(true);
  cf2D->SetFixedHistogramMinIntensity(0);
  cf2D->SetFixedHistogramMaxIntensity(255);
  cf2D->SetFixedNumberOfHistogramBins(100);
  MaskImage2DType::Pointer fim2D = Generate2DMaskImage("mask_2D.mhd");
  Mask2DSpatialObjectType::Pointer fspatial2D = Mask2DSpatialObjectType::New();
  fspatial2D->SetImage(fim2D);
  fspatial2D->Update();
  cf2D->SetFixedImageMask(fspatial2D);
  try
  {
    cf2D->Initialize();
    if (ExtendedOutput)
      VERBOSE(<< "\n")
    for (int i = 0; i < 80; i++)
    {
      if (i > 0)
      {
        pars2D[0] = 0.2 - (double) (rand() % 10001) / 25000.;
        pars2D[1] = 10. - (double) (rand() % 10001) / 500.;
        pars2D[2] = 10. - (double) (rand() % 10001) / 500.;
      }
      else
      {
        pars2D.fill(0);
      }
      double value = cf2D->GetValue(pars2D);
      if (ExtendedOutput)
        VERBOSE(<< "  " << i << ";" << value << ";" << pars2D[0] << ";" <<
            pars2D[1] << ";" << pars2D[2] << "\n")
    }
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "Masked-ERROR: " << e << "\n";
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Unmasked 3D tests ... ")
  lok = true;
  CostFunction3DType::Pointer cf3D = CostFunction3DType::New();
  cf3D->SetFixedImage(image3D1);
  cf3D->SetMovingImage(image3D2);
  cf3D->SetFixedImageRegion(image3D1->GetLargestPossibleRegion());
  Transform3DType::Pointer transform3D = Transform3DType::New();
  transform3D->SetIdentity();
  cf3D->SetTransform(transform3D);
  Interpolator3DType::Pointer interp3D = Interpolator3DType::New();
  cf3D->SetInterpolator(interp3D);
  cf3D->SetFixedHistogramClipAtEnds(true);
  cf3D->SetFixedHistogramMinIntensity(0);
  cf3D->SetFixedHistogramMaxIntensity(355);
  cf3D->SetFixedNumberOfHistogramBins(180);
  try
  {
    cf3D->Initialize();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "Initialization-ERROR: " << e << "\n";
    lok = false;
  }
  // sample a few transformation positions around "optimum":
  CostFunction3DType::ParametersType pars3D;
  pars3D.SetSize(3);
  if (ExtendedOutput)
    VERBOSE(<< "\n")
  try
  {
    for (int i = 0; i < 80; i++)
    {
      if (i > 0)
      {
        pars3D[0] = 10. - (double) (rand() % 10001) / 500.;
        pars3D[1] = 10. - (double) (rand() % 10001) / 500.;
        pars3D[2] = 10. - (double) (rand() % 10001) / 500.;
      }
      else
      {
        pars3D.fill(0);
      }
      double value = cf3D->GetValue(pars3D);
      if (ExtendedOutput)
        VERBOSE(<< "  " << i << ";" << value << ";" << pars3D[0] << ";" <<
            pars3D[1] << ";" << pars3D[2] << "\n")
    }
    // re-configure:
    cf3D->SetFixedHistogramMinIntensity(50);
    cf3D->SetFixedHistogramMaxIntensity(300);
    cf3D->SetFixedNumberOfHistogramBins(240);
    cf3D->Initialize();
    for (int i = 0; i < 80; i++)
    {
      if (i > 0)
      {
        pars3D[0] = 10. - (double) (rand() % 10001) / 500.;
        pars3D[1] = 10. - (double) (rand() % 10001) / 500.;
        pars3D[2] = 10. - (double) (rand() % 10001) / 500.;
      }
      else
      {
        pars3D.fill(0);
      }
      cf3D->SetMapOutsideIntensitiesToZeroProbability(false);
      double value = cf3D->GetValue(pars3D);
      if (ExtendedOutput)
        VERBOSE(<< "  " << i << "a;" << value << ";" << pars3D[0] << ";" <<
            pars3D[1] << ";" << pars3D[2] << "\n")
      cf3D->SetMapOutsideIntensitiesToZeroProbability(true);
      value = cf3D->GetValue(pars3D);
      if (ExtendedOutput)
        VERBOSE(<< "  " << i << "b;" << value << ";" << pars3D[0] << ";" <<
            pars3D[1] << ";" << pars3D[2] << "\n")
    }
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "Value-ERROR: " << e << "\n";
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Masked 3D tests ... ")
  lok = true;
  cf3D->SetFixedHistogramClipAtEnds(true);
  cf3D->SetFixedHistogramMinIntensity(0);
  cf3D->SetFixedHistogramMaxIntensity(355);
  cf3D->SetFixedNumberOfHistogramBins(300);
  MaskImage3DType::Pointer fim3D = Generate3DMaskImage("mask_3D.mhd");
  Mask3DSpatialObjectType::Pointer fspatial3D = Mask3DSpatialObjectType::New();
  fspatial3D->SetImage(fim3D);
  fspatial3D->Update();
  cf3D->SetFixedImageMask(fspatial3D);
  try
  {
    cf3D->Initialize();
    if (ExtendedOutput)
      VERBOSE(<< "\n")
    for (int i = 0; i < 80; i++)
    {
      if (i > 0)
      {
        pars3D[0] = 10. - (double) (rand() % 10001) / 500.;
        pars3D[1] = 10. - (double) (rand() % 10001) / 500.;
        pars3D[2] = 10. - (double) (rand() % 10001) / 500.;
      }
      else
      {
        pars3D.fill(0);
      }
      double value = cf3D->GetValue(pars3D);
      if (ExtendedOutput)
        VERBOSE(<< "  " << i << ";" << value << ";" << pars3D[0] << ";" <<
            pars3D[1] << ";" << pars3D[2] << "\n")
    }
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "Masked-ERROR: " << e << "\n";
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  if (cf2D->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  cf2D = NULL; // reference counter must be zero!
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
