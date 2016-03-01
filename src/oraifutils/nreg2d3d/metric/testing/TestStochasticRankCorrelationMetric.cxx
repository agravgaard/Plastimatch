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
#include <itkTranslationTransform.h>

#include "oraStochasticRankCorrelationImageToImageMetric.h"

#define VERBOSE(x) \
{ \
  if (Verbose) \
  {\
    std::cout x; \
    std::cout.flush(); \
  }\
}

// verbose flag
bool Verbose = false;
// CSV output flag
bool CSVOutput = false;
// image output
bool ImageOutput = false;
// extended output
bool ExtendedOutput = false;

/**
 * Print test usage information.
 **/
void PrintUsage(char *binname)
{
  std::string progname = "<test-binary-name>";

  if (binname)
    progname = std::string(binname);

  std::cout << "\n";
  std::cout << "   *** T E S T   U S A G E ***\n";
  std::cout << "\n";
  std::cout << progname << " [options]\n";
  std::cout << "\n";
  std::cout << "  -h or --help ... print this short help\n";
  std::cout << "  -v or --verbose ... verbose messages to std::cout\n";
  std::cout
      << "  -co or --csv-output ... CSV (comma separated values) file outputs\n";
  std::cout << "  -io or --image-output ... image output\n";
  std::cout << "  -xo or --extended-output ... extended message output\n";
  std::cout << "\n";
  std::cout << "  NOTE: optional arguments are case-sensitive!\n";
  std::cout << "\n";
  std::cout << "  Author: Philipp Steininger\n";
  std::cout
      << "  Affiliation: Institute for Research and Development on Advanced Radiation Technologies (radART)\n";
  std::cout
      << "               Paracelsus Medical University (PMU), Salzburg, AUSTRIA\n";
  std::cout << "\n";
}

const unsigned int Dimension2D = 2;
const unsigned int Dimension3D = 3;
typedef float PixelType;
typedef unsigned int RankPixelType; // NOTE: type must cover no. of pixels!
typedef itk::Image<PixelType, Dimension2D> Image2DType;
typedef itk::Image<PixelType, Dimension3D> Image3DType;
typedef itk::Image<RankPixelType, Dimension2D> RankImage2DType;
typedef itk::Image<RankPixelType, Dimension3D> RankImage3DType;
typedef itk::ImageRegionIterator<Image2DType> Iterator2DType;
typedef itk::ImageRegionIterator<Image3DType> Iterator3DType;
typedef ora::StochasticRankCorrelationImageToImageMetric<Image2DType,
    Image2DType, RankImage2DType> SRCMetricType;
typedef ora::StochasticRankCorrelationImageToImageMetric<Image3DType,
    Image3DType, RankImage3DType> SRCMetric3DType;
typedef itk::LinearInterpolateImageFunction<Image2DType, double>
    InterpolatorType;
typedef itk::LinearInterpolateImageFunction<Image3DType, double>
    Interpolator3DType;
typedef itk::Rigid2DTransform<double> TransformType;
typedef itk::TranslationTransform<double, 3> Transform3DType;
typedef TransformType::ParametersType ParametersType;
typedef Transform3DType::ParametersType Parameters3DType;
typedef itk::CStyleCommand CommandType;
typedef itk::ImageMaskSpatialObject<2> MaskSpatialObjectType;

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

  if (ImageOutput)
  {
    typedef itk::ImageFileWriter<Image2DType> WriterType;
    WriterType::Pointer w = WriterType::New();
    w->SetFileName(fname);
    w->SetInput(image);
    w->Update();
  }

  return image;
}

/** @return a test volume for some tests **/
Image3DType::Pointer GenerateTestVolume(const char *fname)
{
  Image3DType::SizeType isize;
  isize[0] = 101;
  isize[1] = 101;
  isize[2] = 81;
  Image3DType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  iindex[2] = 0;
  Image3DType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  Image3DType::SpacingType ispacing;
  ispacing[0] = 1.0;
  ispacing[1] = 1.0;
  ispacing[2] = 1.0;
  Image3DType::PointType iorigin;
  iorigin[0] = -(isize[0] * ispacing[0]) / 2.; // centered
  iorigin[1] = -(isize[1] * ispacing[1]) / 2.;
  iorigin[2] = -(isize[2] * ispacing[2]) / 2.;
  Image3DType::DirectionType idirection;
  idirection.SetIdentity();
  Image3DType::Pointer volume = Image3DType::New();
  volume->SetSpacing(ispacing);
  volume->SetOrigin(iorigin);
  volume->SetDirection(idirection);
  volume->SetRegions(iregion);
  volume->Allocate();
  Iterator3DType it(volume, iregion);
  Image3DType::PixelType v;
  Image3DType::IndexType p;
  int mx = 1, mx2 = 20; // margins
  int my = 1, my2 = 30;
  int mz = 1, mz2 = 25;
  int zold = 0;
  for (it.GoToBegin(); !it.IsAtEnd(); ++it) // frustum of a pyramid
  {
    p = it.GetIndex();
    if ((int) p[2] >= mz && (int) p[2] < (int) (isize[2] - mz))
    {
      if ((int) p[0] >= mx && (int) p[0] < (int) (isize[0] - mx) && (int) p[1]
          >= my && (int) p[1] < (int) (isize[1] - my))
      {
        if ((int) p[0] >= mx2 && (int) p[0] < (int) (isize[0] - mx2)
            && (int) p[1] >= my2 && (int) p[1] < (int) (isize[1] - my2)
            && (int) p[2] >= mz2 && (int) p[2] < (int) (isize[2] - mz2))
        {
          v = rand() % 151 + 1800; // some bony tissue
        }
        else
        {
          v = rand() % 201 + 1300; // some soft tissue
        }
      }
      else
      {
        v = static_cast<Image3DType::PixelType> (0);
      }
    }
    else
    {
      v = static_cast<Image3DType::PixelType> (0);
    }
    it.Set(v);
    if (zold != p[2]) // slice change
    {
      if (p[2] % 2 == 0)
        mx++;
      if (p[2] % 3 == 0)
        my++;
      zold = p[2];
    }
  }
  if (ImageOutput)
  {
    typedef itk::ImageFileWriter<Image3DType> VolumeWriterType;
    VolumeWriterType::Pointer w = VolumeWriterType::New();
    w->SetInput(volume);
    w->SetFileName(fname);
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

  return volume;
}

/** Current mask to receive copy of mask during event. **/
SRCMetricType::MaskImagePointer CurrentCopyMask = NULL;
/** File name for copy mask (if should be written) **/
std::string CurrentCopyMaskFileName = "";

/** Metric (SRC) event command. **/
void SRCMetricEvent(itk::Object *obj, const itk::EventObject &ev, void *cd)
{
  SRCMetricType *metric = (SRCMetricType *) cd;

  if (std::string(ev.GetEventName()) == "AfterMaskCreation")
  {
    SRCMetricType::MaskImageConstPointer mask = metric->GetStochasticMask();
    if (CurrentCopyMask && mask)
    {
      CurrentCopyMask->SetRegions(mask->GetLargestPossibleRegion());
      CurrentCopyMask->SetOrigin(mask->GetOrigin());
      CurrentCopyMask->SetSpacing(mask->GetSpacing());
      CurrentCopyMask->SetDirection(mask->GetDirection());
      CurrentCopyMask->Allocate();
      typedef itk::ImageRegionIterator<SRCMetricType::MaskImageType>
          MaskIteratorType;
      typedef itk::ImageRegionConstIterator<SRCMetricType::MaskImageType>
          MaskConstIteratorType;
      MaskConstIteratorType inmit(mask, mask->GetLargestPossibleRegion());
      MaskIteratorType outmit(CurrentCopyMask,
          CurrentCopyMask->GetLargestPossibleRegion());
      while (!inmit.IsAtEnd()) // copy mask
      {
        outmit.Set(inmit.Get());
        ++inmit;
        ++outmit;
      }
      if (ImageOutput && CurrentCopyMaskFileName.length() > 0) // write out
      {
        typedef itk::ImageFileWriter<SRCMetricType::MaskImageType> WriterType;
        WriterType::Pointer w = WriterType::New();
        w->SetInput(CurrentCopyMask);
        w->SetFileName(CurrentCopyMaskFileName);
        try
        {
          w->Update();
        }
        catch (itk::ExceptionObject &e)
        {
          std::cerr << "Mask write error: " << e << "\n";
        }
      }
    }
  }
}

/** \brief Tests functionality of stochastic rank correlation metric.
 * Tests functionality of stochastic rank correlation metric.
 *
 * Test application result is 0 if SUCCESSFUL.
 *
 * Arguments: <br>
 * -h or --help ... print short help <br>
 * -v or --verbose ... message output (verbose) <br>
 * -co or --csv-output ... CSV (comma separated values) file outputs <br>
 * -io or --image-output ... image output <br>
 * -xo or --extended-output ... extended message output<br>
 *
 * @see ora::StochasticRankCorrelationImageToImageMetric
 * @see ora::AverageRanksImageToImageFilter
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.2
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // arguments check
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-v" || std::string(argv[i]) == "--verbose")
      Verbose = true;
    if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
    {
      if (argc > 0)
        PrintUsage(argv[0]);
      else
        PrintUsage(NULL);
      return EXIT_FAILURE;
    }
    if (std::string(argv[i]) == "-co" || std::string(argv[i]) == "--csv-output")
      CSVOutput = true;
    if (std::string(argv[i]) == "-io"
        || std::string(argv[i]) == "--image-output")
      ImageOutput = true;
    if (std::string(argv[i]) == "-xo"
        || std::string(argv[i]) == "--extended-output")
      ExtendedOutput = true;
  }

  VERBOSE(<< "\nTesting stochastic rank correlation image to image metric.\n")
  bool ok = true;

  VERBOSE(<< "  * Various sample coverages with simple value / derivative computation ... ")
  bool lok = true;
  srand(time(NULL));
  // the 2 images should be slightly different (rand())
  Image2DType::Pointer image1 = GenerateTestImage("image_1.mhd");
  Image2DType::Pointer image2 = GenerateTestImage("image_2.mhd");
  TransformType::Pointer transform = TransformType::New();
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  SRCMetricType::Pointer src = SRCMetricType::New();
  src->SetFixedImage(image1);
  src->SetFixedImageRegion(image1->GetLargestPossibleRegion());
  src->SetMovingImage(image2);
  src->SetTransform(transform);
  src->SetInterpolator(interpolator);
  SRCMetricType::SeedsType seeds;
  seeds.SetSize(2);
  seeds[0] = 1252343;
  seeds[1] = 434728;
  src->SetRandomSeeds(seeds);
  ParametersType pars(transform->GetNumberOfParameters());
  pars.fill(0);
  SRCMetricType::ScalesType dscales;
  dscales.SetSize(transform->GetNumberOfParameters());
  dscales[0] = 0.01745329251994329577; // 1 degree
  dscales[1] = 1; // mm
  dscales[2] = 1; // mm
  try
  {
    src->SetDerivativeScales(dscales);
    src->SetFixedNumberOfHistogramBins(256);
    src->SetFixedHistogramMinIntensity(0);
    src->SetFixedHistogramMaxIntensity(255);
    src->SetMovingNumberOfHistogramBins(256);
    src->SetMovingHistogramMinIntensity(0);
    src->SetMovingHistogramMaxIntensity(255);
    double minvalue = 1000000;
    int minIdx = -1;
    for (int i = 0; i <= /*2*/0; i++)
    {
      src->SetSampleCoverage((double) i * 5.0); // whole image
      src->Initialize();
      if (ImageOutput) // fixed rank image output
      {
        typedef itk::ImageFileWriter<RankImage2DType> WriterType;
        char buff[100];
        sprintf(buff, "fixed_rank_image_cov_%f.mhd", src->GetSampleCoverage());
        WriterType::Pointer w = WriterType::New();
        w->SetFileName(buff);
        w->SetInput(src->GetFixedRankImage());
        w->Update();
      }
      if (i == 0) // metric evaluation makes no sense
        continue;
      SRCMetricType::MeasureType value;
      SRCMetricType::DerivativeType derivative;
      if (!ExtendedOutput)
        VERBOSE(<< "\n    coverage = " << src->GetSampleCoverage() << " %")
      for (int j = 0; j < 5; j++)
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
          pars[0] += 0.12;
          pars[1] -= 1.9;
          pars[2] += 1.45;
        }
        // evaluate!
        src->GetValueAndDerivative(pars, value, derivative);
        if (value < minvalue)
        {
          minvalue = value;
          minIdx = j;
        }
        if (ExtendedOutput)
          VERBOSE(<< "coverage = " << src->GetSampleCoverage()
              << " %\n parameters = " << pars << "\n value = " << value
              << "\n derivative = " << derivative << "\n")
      }
      if (minIdx == 0) // 0-parameters
      {
        VERBOSE(<< " ... OK")
      }
      else
      {
        lok = false;
        VERBOSE(<< " ... FAILURE")
      }
      if (ExtendedOutput)
        VERBOSE(<< "\n")
    }
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "Error error: " << e << std::endl;
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Test deterministic/non-deterministic behavior ... ")
  lok = true;
  pars.Fill(0);
  pars[0] = 0.05;
  pars[1] = -2.34;
  pars[2] = 1.443;
  seeds[0] = 0;
  seeds[1] = 0;
  src->SetRandomSeeds(seeds); // non-deterministic
  src->SetSampleCoverage(50);
  // - on value basis -
  src->Initialize();
  SRCMetricType::MeasureType value1;
  SRCMetricType::MeasureType value2;
  value1 = src->GetValue(pars);
  src->Initialize();
  value2 = src->GetValue(pars);
  if (value1 == value2) // non-deterministic behavior expected
  {
    if (ExtendedOutput)
      VERBOSE(<< "\n    non-deterministic value mismatch: " << value1 << "\n")
    lok = false;
  }
  else
  {
    if (ExtendedOutput)
      VERBOSE(<< "\n    non-deterministic value match: " << value1 << " vs. " <<
          value2 << "\n")
  }
  seeds[0] = 12312;
  seeds[1] = 4244242;
  src->SetRandomSeeds(seeds); // deterministic
  src->Initialize();
  value1 = src->GetValue(pars);
  src->Initialize();
  value2 = src->GetValue(pars);
  if (value1 != value2) // deterministic behavior expected
  {
    if (ExtendedOutput)
      VERBOSE(<< "\n    deterministic value mismatch: " << value1 << " vs. "
          << value2 << "\n")
    lok = false;
  }
  else
  {
    if (ExtendedOutput)
      VERBOSE(<< "\n    deterministic value match: " << value1 << "\n")
  }
  // - on mask basis -
  CommandType::Pointer cmd1 = CommandType::New();
  cmd1->SetClientData(src);
  cmd1->SetCallback(SRCMetricEvent);
  src->AddObserver(ora::AfterMaskCreation(), cmd1);
  SRCMetricType::MaskImagePointer m1 = SRCMetricType::MaskImageType::New();
  SRCMetricType::MaskImagePointer m2 = SRCMetricType::MaskImageType::New();
  CurrentCopyMaskFileName = "deterministic_mask1.mhd";
  CurrentCopyMask = m1;
  src->Initialize();
  CurrentCopyMaskFileName = "deterministic_mask2.mhd";
  CurrentCopyMask = m2;
  src->Initialize();
  typedef itk::ImageRegionIterator<SRCMetricType::MaskImageType>
      MaskIteratorType;
  MaskIteratorType *m1it = new MaskIteratorType(m1,
      m1->GetLargestPossibleRegion());
  MaskIteratorType *m2it = new MaskIteratorType(m2,
      m2->GetLargestPossibleRegion());
  if (m1->GetLargestPossibleRegion().GetNumberOfPixels()
      == m2->GetLargestPossibleRegion().GetNumberOfPixels())
  {
    bool equal = true;
    while (!m1it->IsAtEnd()) // equal masks expected (deterministic)
    {
      if (m1it->Get() != m2it->Get())
      {
        equal = false;
        break;
      }
      ++(*m1it);
      ++(*m2it);
    }
    if (!equal)
    {
      if (ExtendedOutput)
        VERBOSE(<< "\n    deterministic mask mismatch\n")
      lok = false;
    }
    else
    {
      if (ExtendedOutput)
        VERBOSE(<< "\n    deterministic mask match\n")
    }
  }
  else
  {
    if (ExtendedOutput)
      VERBOSE(<< "\n    deterministic mask mismatch (regions)\n")
    lok = false;
  }
  seeds[0] = 0;
  seeds[1] = 0;
  src->SetRandomSeeds(seeds); // non-deterministic
  CurrentCopyMaskFileName = "non-deterministic_mask1.mhd";
  CurrentCopyMask = m1;
  src->Initialize();
  CurrentCopyMaskFileName = "non-deterministic_mask2.mhd";
  CurrentCopyMask = m2;
  src->Initialize();
  delete m1it;
  delete m2it;
  m1it = new MaskIteratorType(m1, m1->GetLargestPossibleRegion());
  m2it = new MaskIteratorType(m2, m2->GetLargestPossibleRegion());
  if (m1->GetLargestPossibleRegion().GetNumberOfPixels()
      == m2->GetLargestPossibleRegion().GetNumberOfPixels())
  {
    bool equal = true;
    while (!m1it->IsAtEnd()) // unequal masks expected (non-deterministic)
    {
      if (m1it->Get() != m2it->Get())
      {
        equal = false;
        break;
      }
      ++(*m1it);
      ++(*m2it);
    }
    if (equal)
    {
      if (ExtendedOutput)
        VERBOSE(<< "\n    non-deterministic mask mismatch\n")
      lok = false;
    }
    else
    {
      if (ExtendedOutput)
        VERBOSE(<< "\n    non-deterministic mask match\n")
    }
  }
  else
  {
    if (ExtendedOutput)
      VERBOSE(<< "\n    non-deterministic mask mismatch (regions)\n")
    lok = false;
  }
  CurrentCopyMask = NULL; // turn off copying
  delete m1it;
  delete m2it;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Check no-overlap behavior ... ")
  lok = true;
  SRCMetricType::MeasureType value;
  SRCMetricType::DerivativeType derivative;
  pars.Fill(0);
  pars[2] = 3000; // no overlap!
  src->SetNoOverlapReactionMode(0); // exception
  src->SetSampleCoverage(10);
  try
  {
    value = src->GetValue(pars);
    lok = false; // exception awaited!
  }
  catch (itk::ExceptionObject &e)
  {
    ; // expected
  }
  src->SetNoOverlapReactionMode(0); // exception
  try
  {
    src->GetDerivative(pars, derivative);
    lok = false; // exception awaited!
  }
  catch (itk::ExceptionObject &e)
  {
    ; // expected
  }
  try
  {
    src->GetValueAndDerivative(pars, value, derivative);
    lok = false; // exception awaited!
  }
  catch (itk::ExceptionObject &e)
  {
    ; // expected
  }
  src->SetNoOverlapReactionMode(1); // specified measure value!
  src->SetNoOverlapMetricValue(100.5);
  try
  {
    value = src->GetValue(pars);
    if (value != 100.5)
      lok = false;
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Test fixed image region and fixed image mask ... ")
  lok = true;
  Image2DType::ConstPointer fimage = src->GetFixedImage();
  Image2DType::RegionType freg = fimage->GetLargestPossibleRegion();
  if (ExtendedOutput)
    VERBOSE(<< "\n    old fixed image region: " << freg)
  Image2DType::SizeType fregsz = freg.GetSize();
  fregsz[0] = static_cast<Image2DType::SizeValueType> (fregsz[0] * 0.5);
  fregsz[1] = static_cast<Image2DType::SizeValueType> (fregsz[1] * 0.3);
  Image2DType::IndexType fregidx = freg.GetIndex();
  fregidx[0] = static_cast<Image2DType::IndexValueType> (fregsz[0] * 0.1);
  fregidx[1] = static_cast<Image2DType::IndexValueType> (fregsz[1] * 0.4);
  freg.SetIndex(fregidx);
  freg.SetSize(fregsz);
  src->SetFixedImageRegion(freg);
  if (ExtendedOutput)
    VERBOSE(<< "    new fixed image region: " << freg)
  src->SetFixedImageMask((SRCMetricType::FixedImageMaskPointer) NULL);
  for (int k = 0; k <= 1; k++)
  {
    SRCMetricType::MaskImagePointer fim = NULL;
    if (k == 1) // apply additional fixed image mask
    {
      fim = SRCMetricType::MaskImageType::New();
      fim->SetRegions(fimage->GetLargestPossibleRegion());
      fim->Allocate();
      fim->SetOrigin(fimage->GetOrigin());
      fim->SetSpacing(fimage->GetSpacing());
      fim->SetDirection(fimage->GetDirection());
      fim->FillBuffer(0);
      SRCMetricType::MaskImageType::SizeType fimsz =
          fim->GetLargestPossibleRegion().GetSize();
      int radius2 = (fimsz[0] / 3) * (fimsz[0] / 3);
      MaskIteratorType fimit(fim, fim->GetLargestPossibleRegion());
      while (!fimit.IsAtEnd())
      {
        SRCMetricType::MaskImageType::IndexType idx = fimit.GetIndex();
        int xx = idx[0] - fimsz[0] / 2;
        int yy = idx[1] - fimsz[1] / 2;
        if ((xx * xx + yy * yy) <= radius2)
          fimit.Set(1);
        else
          fimit.Set(0);
        ++fimit;
      }
      if (ImageOutput)
      {
        typedef itk::ImageFileWriter<SRCMetricType::MaskImageType>
            MaskWriterType;
        MaskWriterType::Pointer w = MaskWriterType::New();
        w->SetInput(fim);
        w->SetFileName("fixed_image_mask.mhd");
        try
        {
          w->Update();
        }
        catch (itk::ExceptionObject &e)
        {
          std::cerr << "Fixed image mask write error: " << e << "\n";
          lok = false;
        }
      }
      // -> set as fixed image mask:
      MaskSpatialObjectType::Pointer fspatial = MaskSpatialObjectType::New();
      fspatial->SetImage(fim);
      fspatial->Update();
      src->SetFixedImageMask(fspatial);
    }
    for (int x = 0; x < 10; x++)
    {
      double currCov =
          static_cast<double> (15. + (double) (rand() % 701) / 10.);
      if (x == 9)
        currCov = 100; // important case!
      src->SetStochasticUserMask(NULL);
      src->SetSampleCoverage(currCov);
      m1 = NULL;
      m1 = SRCMetricType::MaskImageType::New();
      CurrentCopyMask = m1;
      std::ostringstream os;
      os.str("");
      os << "mask_sc" << currCov << "_fir_fim" << k << ".mhd";
      CurrentCopyMaskFileName = os.str();
      src->Initialize();
      if (m1->GetLargestPossibleRegion().GetNumberOfPixels() > 0)
      {
        // - check whether only pixels within fixed image region are >0 and whether
        //   number of unmasked pixels is sample coverage -
        int numUnmasked = 0;
        m1it = new MaskIteratorType(m1, m1->GetLargestPossibleRegion());
        while (!m1it->IsAtEnd()) // total number of unmasked pixels
        {
          if (m1it->Get() > 0)
            numUnmasked++;
          ++(*m1it);
        }
        delete m1it;
        m1it = new MaskIteratorType(m1, freg);
        int numUnmaskedRegion = 0;
        int totalPixels = 0;
        if (!src->GetFixedImageMask())
        {
          while (!m1it->IsAtEnd()) // unmasked pixels within fixed region
          {
            totalPixels++; // all pixels of fixed image region
            if (m1it->Get() > 0)
              numUnmaskedRegion++;
            ++(*m1it);
          }
        }
        else // + fixed image mask
        {
          MaskIteratorType fimit(fim, fim->GetLargestPossibleRegion());
          while (!m1it->IsAtEnd()) // unmasked pixels within fixed region & mask
          {
            fimit.SetIndex(m1it->GetIndex());
            if (fimit.Get() > 0) // only unmasked pixels considered
            {
              totalPixels++; // all pixels of fixed image region within mask
              if (m1it->Get() > 0)
                numUnmaskedRegion++;
            }
            ++(*m1it);
          }
        }
        delete m1it;
        if (numUnmaskedRegion == numUnmasked)
        {
          // - check whether sample coverage is OK -
          double realCov = (double) (numUnmasked) / (double) totalPixels * 100.;
          if (fabs(realCov - currCov) > 0.5) // 0.5 % resolution requested
            lok = false;
        }
        else // only pixels within fixed image region allowed!
        {
          lok = false;
        }
      }
      else // no mask copied
      {
        lok = false;
      }
      CurrentCopyMask = NULL;
      // test value/derivative:
      pars.Fill(-0.1);
      src->GetValueAndDerivative(pars, value, derivative);
      if (ExtendedOutput)
        VERBOSE(<< "\n    coverage=" << src->GetSampleCoverage() << ": " <<
            value << " (" << derivative << ")\n")
    }
    // user stochastic masks
    for (int x = 0; x < 2; x++)
    {
      SRCMetricType::MaskImagePointer usm = SRCMetricType::MaskImageType::New();
      usm->SetRegions(fimage->GetLargestPossibleRegion());
      usm->Allocate();
      usm->SetOrigin(fimage->GetOrigin());
      usm->SetSpacing(fimage->GetSpacing());
      usm->SetDirection(fimage->GetDirection());
      usm->FillBuffer(0);
      MaskIteratorType usmit(usm, usm->GetLargestPossibleRegion());
      if (x == 0) // each 3rd pixel user stochastic mask
      {
        int cc = 0;
        while (!usmit.IsAtEnd())
        {
          if (cc % 3 == 0)
            usmit.Set(1);
          else
            usmit.Set(0);
          cc++;
          ++usmit;
        }
      }
      else // random user stochastic mask (~1/5)
      {
        while (!usmit.IsAtEnd())
        {
          if (rand() % 5 == 0)
            usmit.Set(1);
          else
            usmit.Set(0);
          ++usmit;
        }
      }
      if (ImageOutput)
      {
        typedef itk::ImageFileWriter<SRCMetricType::MaskImageType> WriterType;
        WriterType::Pointer w = WriterType::New();
        w->SetInput(usm);
        std::ostringstream oss;
        oss << "usm_" << x << ".mhd";
        w->SetFileName(oss.str().c_str());
        try
        {
          w->Update();
        }
        catch (itk::ExceptionObject &e)
        {
          std::cerr << "Mask write error: " << e << "\n";
        }
      }
      src->SetStochasticUserMask(usm);
      m1 = NULL;
      m1 = SRCMetricType::MaskImageType::New();
      CurrentCopyMask = m1;
      std::ostringstream os;
      os.str("");
      os << "mask_usm" << x << "_fir_fim" << k << ".mhd";
      CurrentCopyMaskFileName = os.str();
      src->Initialize();
      // - check whether only pixels within fixed image region are >0 and
      //   whether unmasked pixels correlate with original mask -
      int numUnmasked = 0;
      m1it = new MaskIteratorType(m1, m1->GetLargestPossibleRegion());
      while (!m1it->IsAtEnd()) // total number of unmasked pixels
      {
        if (m1it->Get() > 0)
          numUnmasked++;
        ++(*m1it);
      }
      delete m1it;
      m1it = new MaskIteratorType(m1, freg);
      int numUnmaskedRegion = 0;
      int totalPixels = 0;
      if (!src->GetFixedImageMask())
      {
        while (!m1it->IsAtEnd()) // unmasked pixels within fixed region
        {
          totalPixels++; // all pixels of fixed image region
          if (m1it->Get() > 0)
            numUnmaskedRegion++;
          ++(*m1it);
        }
      }
      else // + fixed image mask
      {
        MaskIteratorType fimit(fim, fim->GetLargestPossibleRegion());
        while (!m1it->IsAtEnd()) // unmasked pixels within fixed region & mask
        {
          fimit.SetIndex(m1it->GetIndex());
          if (fimit.Get() > 0) // only unmasked pixels considered
          {
            totalPixels++; // all pixels of fixed image region within mask
            if (m1it->Get() > 0)
              numUnmaskedRegion++;
          }
          ++(*m1it);
        }
      }
      delete m1it;
      if (numUnmaskedRegion == numUnmasked)
      {
        // - check whether unmasked pixels correlate with original pixels -
        m1it = new MaskIteratorType(m1, m1->GetLargestPossibleRegion());
        MaskIteratorType usmit2(usm, usm->GetLargestPossibleRegion());
        bool match = true;
        while (!m1it->IsAtEnd()) // total number of unmasked pixels
        {
          if (m1it->Get() > 0)
          {
            usmit2.SetIndex(m1it->GetIndex());
            if (usmit2.Get() <= 0)
            {
              match = false;
              break;
            }
          }
          ++(*m1it);
        }
        delete m1it;
        if (!match)
          lok = false;
      }
      else // only pixels within fixed image region allowed!
      {
        lok = false;
      }
      CurrentCopyMask = NULL;
      // test value/derivative:
      pars.Fill(0.1);
      src->GetValueAndDerivative(pars, value, derivative);
      if (ExtendedOutput)
        VERBOSE(<< "\n    user-mask (" << x << "): " <<
            value << " (" << derivative << ")\n")
    }
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Fixed + moving image mask, fixed image region test ... ")
  lok = true;
  if (src->GetFixedImageMask()) // must be set from last test-section!
  {
    src->Initialize();
    unsigned long pixCounts[5];
    ParametersType testPars[5];
    for (int t = 0; t < 5; t++)
    {
      testPars[t].SetSize(transform->GetNumberOfParameters());
      testPars[t][0] = 0.1 - (double) (rand() % 10001) / 50000.;
      testPars[t][1] = 2. - (double) (rand() % 10001) / 5000.;
      testPars[t][2] = 3.333 - (double) (rand() % 10001) / 3000.;
      value = src->GetValue(testPars[t]);
      pixCounts[t] = src->GetNumberOfPixelsCounted();
      if (ExtendedOutput)
        VERBOSE(<< "\n    [no moving image mask] pars=" << testPars[t] <<
            ": value=" << value << " (pixels counted: " << pixCounts[t] <<
            ")\n")
    }
    // - generate moving image mask that is smaller than fixed mask -
    SRCMetricType::MaskImagePointer mim = SRCMetricType::MaskImageType::New();
    mim->SetRegions(fimage->GetLargestPossibleRegion());
    mim->Allocate();
    mim->SetOrigin(fimage->GetOrigin());
    mim->SetSpacing(fimage->GetSpacing());
    mim->SetDirection(fimage->GetDirection());
    mim->FillBuffer(0);
    SRCMetricType::MaskImageType::SizeType mimsz =
        mim->GetLargestPossibleRegion().GetSize();
    int radius2 = (mimsz[0] / 3 - 5) * (mimsz[0] / 3 - 5);
    MaskIteratorType mimit(mim, mim->GetLargestPossibleRegion());
    while (!mimit.IsAtEnd())
    {
      SRCMetricType::MaskImageType::IndexType idx = mimit.GetIndex();
      int xx = idx[0] - mimsz[0] / 2;
      int yy = idx[1] - mimsz[1] / 2;
      if ((xx * xx + yy * yy) <= radius2)
        mimit.Set(1);
      else
        mimit.Set(0);
      ++mimit;
    }
    if (ImageOutput)
    {
      typedef itk::ImageFileWriter<SRCMetricType::MaskImageType> MaskWriterType;
      MaskWriterType::Pointer w = MaskWriterType::New();
      w->SetInput(mim);
      w->SetFileName("moving_image_mask.mhd");
      try
      {
        w->Update();
      }
      catch (itk::ExceptionObject &e)
      {
        std::cerr << "Moving image mask write error: " << e << "\n";
        lok = false;
      }
    }
    // -> set as fixed image mask:
    MaskSpatialObjectType::Pointer mspatial = MaskSpatialObjectType::New();
    mspatial->SetImage(mim);
    mspatial->Update();
    src->SetMovingImageMask(mspatial);
    src->Initialize();
    // - now test whether pix counts <= old counts (due to smaller moving mask)-
    for (int t = 0; t < 5; t++)
    {
      value = src->GetValue(testPars[t]);
      unsigned long currCount = src->GetNumberOfPixelsCounted();
      if (currCount > pixCounts[t])
        lok = false;
      if (ExtendedOutput)
      {
        VERBOSE(<< "\n    [WITH moving image mask] pars=" << testPars[t] <<
            ": value=" << value << " (pixels counted: " << currCount <<
            " vs. " << pixCounts[t] << ") - result: " <<
            (currCount <= pixCounts[t]) << "\n")
      }
    }
  }
  else
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Stress test: sampling around optimum ... ")
  lok = true;
  src->SetFixedImage(image1);
  src->SetMovingImage(image2);
  // region: 55,60-340,540
  SRCMetricType::FixedImageType::RegionType stressRegion;
  stressRegion.SetIndex(0, 55);
  stressRegion.SetIndex(1, 60);
  stressRegion.SetSize(0, 286);
  stressRegion.SetSize(1, 481);
  src->SetFixedImageRegion(stressRegion);
  src->SetFixedImageMask((SRCMetricType::FixedImageMaskPointer) NULL);
  src->SetMovingImageMask((SRCMetricType::MovingImageMaskPointer) NULL);
  src->SetSampleCoverage(30); // 30 %
  src->SetStochasticUserMask(NULL);
  src->SetNoOverlapReactionMode(1);
  src->SetNoOverlapMetricValue(1000.);
  seeds.fill(0);
  src->SetRandomSeeds(seeds);
  CurrentCopyMask = m1;
  CurrentCopyMaskFileName = "stress_test_mask.mhd";
  src->Initialize();
  CurrentCopyMask = NULL;
  std::ofstream csv;
  if (CSVOutput)
  {
    csv.open("stress_test.csv", std::ios::out);
    if (csv.is_open())
      csv << "iteration;rot;xtransl;ytransl;measure\n";
  }
  SRCMetricType::MeasureType minvalue = 1e10;
  SRCMetricType::ParametersType minpars;
  for (int i = 1; i <= 200; i++)
  {
    if (i > 1)
    {
      pars[0] = 0.7 - (double) (rand() % 10001) / 7150.;
      pars[1] = 10 - (double) (rand() % 10001) / 500.;
      pars[2] = 10 - (double) (rand() % 10001) / 500.;
    }
    else
    {
      pars.Fill(0); // optimum
    }
    value = src->GetValue(pars);
    if (value < minvalue)
    {
      minvalue = value;
      minpars = pars;
    }
    if (CSVOutput && csv.is_open())
      csv << i << ";" << pars[0] << ";" << pars[1] << ";" << pars[2] << ";"
          << value << "\n";
    if ((i == 1 || i % 50 == 0) && ExtendedOutput)
      VERBOSE(<< "\n    " << i << ";" << pars[0] << ";" << pars[1] << ";" <<
          pars[2] << ";" << value << "\n")
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // - integrity check -
  if (fabs(minpars[0]) > 0.05 || fabs(minpars[1]) > 2 || fabs(minpars[2]) > 2)
    lok = false;
  if (ExtendedOutput)
    VERBOSE(<< "\n    min. parameters: " << minpars << ", min. value: " <<
        minvalue << "\n")
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")


  VERBOSE(<< "  * Stress test (with Horn-correction): sampling around optimum ... ")
  lok = true;
  src->SetUseHornTiedRanksCorrection(true); // Use Horn-correction (tied ranks)
  CurrentCopyMask = m1;
  CurrentCopyMaskFileName = "stress_test_mask_horn.mhd";
  src->Initialize();
  CurrentCopyMask = NULL;
  if (CSVOutput)
  {
    csv.open("stress_test_horn.csv", std::ios::out);
    if (csv.is_open())
      csv << "iteration;rot;xtransl;ytransl;measure\n";
  }
  minvalue = 1e10;
  for (int i = 1; i <= 500; i++)
  {
    if (i > 1)
    {
      pars[0] = 0.7 - (double) (rand() % 10001) / 7150.;
      pars[0] = 0; // FIXME:
      pars[1] = 10 - (double) (rand() % 10001) / 500.;
      pars[2] = 10 - (double) (rand() % 10001) / 500.;
    }
    else
    {
      pars.Fill(0); // optimum
    }
    value = src->GetValue(pars);
    if (value < minvalue)
    {
      minvalue = value;
      minpars = pars;
    }
    if (CSVOutput && csv.is_open())
      csv << i << ";" << pars[0] << ";" << pars[1] << ";" << pars[2] << ";"
          << value << "\n";
    if ((i == 1 || i % 50 == 0) && ExtendedOutput)
      VERBOSE(<< "\n    " << i << ";" << pars[0] << ";" << pars[1] << ";" <<
          pars[2] << ";" << value << "\n")
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // - integrity check -
  if (fabs(minpars[0]) > 0.05 || fabs(minpars[1]) > 2 || fabs(minpars[2]) > 2)
    lok = false;
  if (ExtendedOutput)
    VERBOSE(<< "\n    min. parameters: " << minpars << ", min. value: " <<
        minvalue << "\n")
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * 3D stress test: (translational) sampling around optimum ...  ")
  lok = true;
  // - in theory the 2D test show that metric works, however do at least one
  //   3D test as well -
  Image3DType::Pointer volume1 = GenerateTestVolume("volume1.mhd");
  Image3DType::Pointer volume2 = GenerateTestVolume("volume2.mhd");
  SRCMetric3DType::Pointer src3D = SRCMetric3DType::New();
  src3D->SetFixedImage(volume1);
  src3D->SetMovingImage(volume2);
  src3D->SetFixedImageRegion(volume1->GetLargestPossibleRegion());
  src3D->SetSampleCoverage(30);
  src3D->SetNoOverlapReactionMode(1);
  src3D->SetNoOverlapMetricValue(1000);
  src3D->SetFixedNumberOfHistogramBins(200);
  src3D->SetFixedHistogramMinIntensity(0);
  src3D->SetFixedHistogramMaxIntensity(1950);
  src3D->SetMovingNumberOfHistogramBins(200);
  src3D->SetMovingHistogramMinIntensity(0);
  src3D->SetMovingHistogramMaxIntensity(1950);
  Interpolator3DType::Pointer interp3D = Interpolator3DType::New();
  src3D->SetInterpolator(interp3D);
  Transform3DType::Pointer transform3D = Transform3DType::New();
  src3D->SetTransform(transform3D);
  src3D->Initialize();
  if (CSVOutput)
  {
    csv.open("stress_test_3d.csv", std::ios::out);
    if (csv.is_open())
      csv << "iteration;xtransl;ytransl;ztransl;measure\n";
  }
  SRCMetric3DType::MeasureType minvalue3D = 1e10;
  SRCMetric3DType::MeasureType value3D;
  SRCMetric3DType::ParametersType minpars3D;
  SRCMetric3DType::ParametersType pars3D;
  pars3D.SetSize(transform3D->GetNumberOfParameters());
  for (int i = 1; i <= 200; i++)
  {
    if (i > 1)
    {
      pars3D[0] = 10 - (double) (rand() % 10001) / 500.;
      pars3D[1] = 10 - (double) (rand() % 10001) / 500.;
      pars3D[2] = 10 - (double) (rand() % 10001) / 500.;
    }
    else
    {
      pars3D.Fill(0); // optimum
    }
    value3D = src3D->GetValue(pars3D);
    if (value3D < minvalue3D)
    {
      minvalue3D = value3D;
      minpars3D = pars3D;
    }
    if (CSVOutput && csv.is_open())
      csv << i << ";" << pars3D[0] << ";" << pars3D[1] << ";" << pars3D[2]
          << ";" << value3D << "\n";
    if ((i == 1 || i % 50 == 0) && ExtendedOutput)
      VERBOSE(<< "\n    " << i << ";" << pars3D[0] << ";" << pars3D[1] << ";"
          << pars3D[2] << ";" << value3D << "\n")
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // - integrity check -
  if (fabs(minpars3D[0]) > 2 || fabs(minpars3D[1]) > 2 || fabs(minpars3D[2])
      > 2)
    lok = false;
  if (ExtendedOutput)
    VERBOSE(<< "\n    min. parameters: " << minpars3D << ", min. value: " <<
        minvalue3D << "\n")
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * 3D stress test: (translational, with Horn-correction) sampling around optimum ...  ")
  lok = true;
  src3D->SetUseHornTiedRanksCorrection(true);
  src3D->Initialize();
  if (CSVOutput)
  {
    csv.open("stress_test_3d_horn.csv", std::ios::out);
    if (csv.is_open())
      csv << "iteration;xtransl;ytransl;ztransl;measure\n";
  }
  minvalue3D = 1e10;
  pars3D.SetSize(transform3D->GetNumberOfParameters());
  for (int i = 1; i <= 200; i++)
  {
    if (i > 1)
    {
      pars3D[0] = 10 - (double) (rand() % 10001) / 500.;
      pars3D[1] = 10 - (double) (rand() % 10001) / 500.;
      pars3D[2] = 10 - (double) (rand() % 10001) / 500.;
    }
    else
    {
      pars3D.Fill(0); // optimum
    }
    value3D = src3D->GetValue(pars3D);
    if (value3D < minvalue3D)
    {
      minvalue3D = value3D;
      minpars3D = pars3D;
    }
    if (CSVOutput && csv.is_open())
      csv << i << ";" << pars3D[0] << ";" << pars3D[1] << ";" << pars3D[2]
          << ";" << value3D << "\n";
    if ((i == 1 || i % 50 == 0) && ExtendedOutput)
      VERBOSE(<< "\n    " << i << ";" << pars3D[0] << ";" << pars3D[1] << ";"
          << pars3D[2] << ";" << value3D << "\n")
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // - integrity check -
  if (fabs(minpars3D[0]) > 2 || fabs(minpars3D[1]) > 2 || fabs(minpars3D[2])
      > 2)
    lok = false;
  if (ExtendedOutput)
    VERBOSE(<< "\n    min. parameters: " << minpars3D << ", min. value: " <<
        minvalue3D << "\n")
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  src->RemoveAllObservers();
  src3D->RemoveAllObservers();
  if (src->GetReferenceCount() == 1 && src3D->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  src = NULL; // reference counter must be zero!
  src3D = NULL;
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
