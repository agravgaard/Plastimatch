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
#include <itkCommand.h>
#include <itkMath.h>

#include "oraAverageRanksImageToImageFilter.h"

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
// extended message output
bool ExtendedOutput = false;
// CSV output flag
bool CSVOutput = false;
// image output
bool ImageOutput = false;

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
typedef unsigned char PixelType;
typedef unsigned int RankPixelType; // NOTE: type must cover no. of pixels!
typedef itk::Image<PixelType, Dimension2D> Image2DType;
typedef itk::Image<RankPixelType, Dimension2D> RankImage2DType;
typedef ora::AverageRanksImageToImageFilter<Image2DType, RankImage2DType>
    Filter2DType;
typedef itk::ImageRegionIterator<Image2DType> Iterator2DType;
typedef itk::CStyleCommand CommandType;

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

Filter2DType::MaskImagePointer GenerateTestMask(const char *fname,
    int &numPixels)
{
  numPixels = 0;
  typedef Filter2DType::MaskImageType MaskType;
  MaskType::SizeType isize;
  isize[0] = 400;
  isize[1] = 600;
  MaskType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  MaskType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  MaskType::SpacingType ispacing;
  ispacing[0] = .5;
  ispacing[1] = .5;
  MaskType::PointType iorigin;
  iorigin[0] = .0;
  iorigin[1] = .0;
  MaskType::DirectionType idirection;
  idirection.SetIdentity();
  MaskType::Pointer image = Image2DType::New();
  image->SetSpacing(ispacing);
  image->SetOrigin(iorigin);
  image->SetDirection(idirection);
  image->SetRegions(iregion);
  image->Allocate();
  image->FillBuffer(static_cast<MaskType::PixelType> (0));

  typedef itk::ImageRegionIterator<MaskType> MaskIteratorType;

  MaskIteratorType it(image, image->GetLargestPossibleRegion());
  int radius2 = (isize[0] / 3) * (isize[0] / 3);
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    MaskType::IndexType i = it.GetIndex();
    int xx = i[0] - isize[0] / 2;
    int yy = i[1] - isize[1] / 2;
    if ((xx * xx + yy * yy) <= radius2)
    {
      if (rand() % 10 == 0) // approximately 10 % within circle mask!
      {
        it.Set(static_cast<MaskType::PixelType> (1));
        numPixels++;
      }
    }
  }

  if (ImageOutput)
  {
    typedef itk::ImageFileWriter<MaskType> WriterType;
    WriterType::Pointer w = WriterType::New();
    w->SetFileName(fname);
    w->SetInput(image);
    w->Update();
  }

  return image;
}

// helpers
bool AfterHistogramExtractionEventFired = false;
int HistogramSizeCheck = 0;
int HistogramTotalFrequencyCheck = 0;
int TransformedHistogramTotalFrequencyCheck = 0;
bool HistogramChecksOK = false;
bool AfterHistogramTransformationEventFired = false;
bool AfterHistogramRankGenerationEventFired = false;
std::string HistogramFileName = "";
std::string TransformedHistogramFileName = "";
std::string RankHistogramFileName = "";

/** Write histogram as CSV-file. **/
void WriteHistogramFile(Filter2DType::HistogramPointer histogram,
    std::string fname)
{
  if (!histogram || !CSVOutput)
    return;
  std::ofstream csv;
  csv.open(fname.c_str(), std::ios::out);
  if (csv.is_open())
  {
    csv << "bin;frequency\n";
    for (unsigned int i = 0; i < histogram->GetSize()[0]; i++)
      csv << i << ";" << histogram->GetFrequency(i, 0) << "\n";
    csv.close();
  }
}

/** Average ranks filter event command. **/
void FilterEvent(itk::Object *obj, const itk::EventObject &ev, void *cd)
{
  Filter2DType *filter = (Filter2DType *) cd;

  if (std::string(ev.GetEventName()) == "AfterHistogramExtraction")
  {
    AfterHistogramExtractionEventFired = true;
    if (HistogramSizeCheck > 0 || HistogramTotalFrequencyCheck > 0)
    {
      HistogramChecksOK = true;
      Filter2DType::HistogramPointer hist = filter->GetAveragedRankHistogram();
      if (HistogramSizeCheck > 0)
      {
        if ((int) hist->GetSize()[0] != HistogramSizeCheck)
          HistogramChecksOK = false;
      }
      if (HistogramTotalFrequencyCheck > 0)
      {
        // only legitimate if ClipAtEnds==false!
        int f = itk::Math::Round<int, double>(hist->GetTotalFrequency());
        if (f != HistogramTotalFrequencyCheck)
          HistogramChecksOK = false;
      }
    }
    if (HistogramFileName.length() > 0)
      WriteHistogramFile(filter->GetAveragedRankHistogram(), HistogramFileName);
  }
  else if (std::string(ev.GetEventName()) == "AfterHistogramTransformation")
  {
    AfterHistogramTransformationEventFired = true;
    if (TransformedHistogramFileName.length() > 0)
      WriteHistogramFile(filter->GetAveragedRankHistogram(),
          TransformedHistogramFileName);
    if (TransformedHistogramTotalFrequencyCheck > 0)
    {
      if (HistogramSizeCheck <= 0 && HistogramTotalFrequencyCheck <= 0)
        HistogramChecksOK = true; // not yet initialized
      Filter2DType::HistogramPointer hist = filter->GetAveragedRankHistogram();
      // only legitimate if ClipAtEnds==false!
      int f = itk::Math::Round<int, double>(hist->GetTotalFrequency());
      if (f != TransformedHistogramTotalFrequencyCheck)
        HistogramChecksOK = false;
    }
  }
  else if (std::string(ev.GetEventName()) == "AfterHistogramRankGeneration")
  {
    AfterHistogramRankGenerationEventFired = true;
    if (RankHistogramFileName.length() > 0)
      WriteHistogramFile(filter->GetAveragedRankHistogram(),
          RankHistogramFileName);
  }
}

/** \brief Tests functionality of average ranks image filter.
 * Tests functionality of average ranks image filter.
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
    if (std::string(argv[i]) == "-io" || std::string(argv[i])
        == "--image-output")
      ImageOutput = true;
    if (std::string(argv[i]) == "-xo" || std::string(argv[i])
        == "--extended-output")
      ExtendedOutput = true;
  }

  VERBOSE(<< "\nTesting average ranks image to image filter.\n")
  bool ok = true;

  VERBOSE(<< "  * Unmasked histogram extraction test ... ")
  bool lok = true;
  typedef itk::ImageFileWriter<RankImage2DType> RankWriter2D;
  Filter2DType::Pointer rankFilter = Filter2DType::New();
  srand(time(NULL));
  Image2DType::Pointer image1 = GenerateTestImage("test_image_1.mhd");
  rankFilter->SetUseHistogramTransformation(false);
  rankFilter->SetDoNotGenerateOutput(true); // rank map only
  rankFilter->SetMaskImage(NULL); // whole image
  rankFilter->SetInput(image1);
  rankFilter->SetNumberOfHistogramBins(256);
  rankFilter->SetHistogramMinIntensity(0);
  rankFilter->SetHistogramMaxIntensity(255);
  rankFilter->SetHistogramClipAtEnds(false);
  // add an observer
  CommandType::Pointer cmd1 = CommandType::New();
  cmd1->SetClientData(rankFilter);
  cmd1->SetCallback(FilterEvent);
  rankFilter->AddObserver(ora::AfterHistogramExtraction(), cmd1);
  rankFilter->AddObserver(ora::AfterHistogramTransformation(), cmd1);
  rankFilter->AddObserver(ora::AfterHistogramRankGeneration(), cmd1);
  try
  {
    itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
    double ts = clock->GetTimeStamp();
    int endcount = 100;
    for (int i = 0; i < endcount; ++i)
    {
      AfterHistogramExtractionEventFired = false; // set back
      AfterHistogramTransformationEventFired = false;
      AfterHistogramRankGenerationEventFired = false;

      HistogramSizeCheck = 256; // bins
      HistogramTotalFrequencyCheck
          = image1->GetLargestPossibleRegion().GetNumberOfPixels();
      TransformedHistogramTotalFrequencyCheck = 0;

      TransformedHistogramFileName = ""; // no transformation here
      if (i == 0)
      {
        HistogramFileName = "unmasked_histogram.csv";
        RankHistogramFileName = "unmasked_rank_map.csv";
      }
      else
      {
        HistogramFileName = "";
        RankHistogramFileName = "";
      }

      rankFilter->Modified();
      rankFilter->Update();

      if (!AfterHistogramExtractionEventFired
          || AfterHistogramTransformationEventFired
          || !AfterHistogramRankGenerationEventFired || !HistogramChecksOK)
        lok = false;

      Filter2DType::HistogramPointer rankMap =
          rankFilter-> GetAveragedRankHistogram();
      // make sure that the number of bins matches, and the last rank is <=
      // number of pixels of the image!
      double lastFrequency = 0;
      int b = rankMap->GetSize()[0] - 1;
      while (b >= 0)
      {
        if (rankMap->GetFrequency(b, 0) > 0)
        {
          lastFrequency = rankMap->GetFrequency(b, 0);
          break;
        }
        b--;
      }
      if (rankMap->GetSize()[0] != 256 || itk::Math::Round<unsigned int, double>(lastFrequency)
          > image1->GetLargestPossibleRegion().GetNumberOfPixels())
        lok = false;
    }
    ts = clock->GetTimeStamp() - ts;
    if (ExtendedOutput)
      VERBOSE(<< "\n    ~rank-map-extraction-time: "
          << ((ts / (double)endcount) * 1000.) << " ms\n")
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Unmasked rank image test ... ")
  lok = true;
  rankFilter->SetDoNotGenerateOutput(false); // generate output image as well
  try
  {
    itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
    double ts = clock->GetTimeStamp();
    int endcount = 100;
    HistogramSizeCheck = -1; // forget here
    HistogramTotalFrequencyCheck = -1;
    TransformedHistogramTotalFrequencyCheck = -1;
    HistogramFileName = "";
    TransformedHistogramFileName = "";
    RankHistogramFileName = "";
    for (int i = 0; i < endcount; ++i)
    {
      AfterHistogramExtractionEventFired = false; // set back
      AfterHistogramTransformationEventFired = false;
      AfterHistogramRankGenerationEventFired = false;

      rankFilter->Modified();
      rankFilter->Update();

      if (!AfterHistogramExtractionEventFired
          || AfterHistogramTransformationEventFired
          || !AfterHistogramRankGenerationEventFired)
        lok = false;

      Filter2DType::OutputImagePointer outputImage = rankFilter-> GetOutput();
      if (outputImage)
      {
        if (i == 0 && ImageOutput)
        {
          typedef itk::ImageFileWriter<Filter2DType::OutputImageType> WType;
          WType::Pointer w = WType::New();
          w->SetFileName("unmasked_output_image.mhd");
          w->SetInput(outputImage);
          w->Update();
        }
        if (outputImage->GetLargestPossibleRegion()
            != rankFilter->GetInput()->GetLargestPossibleRegion())
          lok = false;
      }
      else
      {
        lok = false;
      }
    }
    ts = clock->GetTimeStamp() - ts;
    if (ExtendedOutput)
      VERBOSE(<< "\n    ~rank-image-generation-time: "
          << ((ts / (double)endcount) * 1000.) << " ms\n")
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Masked histogram extraction test ... ")
  lok = true;
  Image2DType::Pointer image2 = GenerateTestImage("test_image_2.mhd");
  rankFilter->SetUseHistogramTransformation(false);
  rankFilter->SetDoNotGenerateOutput(true); // rank map only
  int numPixels = 0;
  Filter2DType::MaskImagePointer mask = GenerateTestMask("test_mask_2.mhd",
      numPixels);
  rankFilter->SetMaskImage(mask); // masked!
  rankFilter->SetInput(image2);
  rankFilter->SetNumberOfHistogramBins(200);
  rankFilter->SetHistogramMinIntensity(0);
  rankFilter->SetHistogramMaxIntensity(225);
  rankFilter->SetHistogramClipAtEnds(false);
  try
  {
    itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
    double ts = clock->GetTimeStamp();
    int endcount = 100;
    for (int i = 0; i < endcount; ++i)
    {
      AfterHistogramExtractionEventFired = false; // set back
      AfterHistogramTransformationEventFired = false;
      AfterHistogramRankGenerationEventFired = false;

      HistogramSizeCheck = 200; // bins
      HistogramTotalFrequencyCheck = numPixels;
      TransformedHistogramTotalFrequencyCheck = 0;

      TransformedHistogramFileName = ""; // no transformation here
      if (i == 0)
      {
        HistogramFileName = "masked_histogram.csv";
        RankHistogramFileName = "masked_rank_map.csv";
      }
      else
      {
        HistogramFileName = "";
        RankHistogramFileName = "";
      }

      rankFilter->Modified();
      rankFilter->Update();

      if (!AfterHistogramExtractionEventFired
          || AfterHistogramTransformationEventFired
          || !AfterHistogramRankGenerationEventFired || !HistogramChecksOK)
        lok = false;

      Filter2DType::HistogramPointer rankMap =
          rankFilter-> GetAveragedRankHistogram();
      // make sure that the number of bins matches, and the last rank is <=
      // number of pixels of the image!
      double lastFrequency = 0;
      int b = rankMap->GetSize()[0] - 1;
      while (b >= 0)
      {
        if (rankMap->GetFrequency(b, 0) > 0)
        {
          lastFrequency = rankMap->GetFrequency(b, 0);
          break;
        }
        b--;
      }
      if (rankMap->GetSize()[0] != 200 || itk::Math::Round<unsigned int, double>(lastFrequency) > numPixels)
        lok = false;
    }

    // check zero-mask:
    mask->FillBuffer(0);
    rankFilter->SetMaskImage(mask);
    try
    {
      rankFilter->Modified();
      rankFilter->Update();
      Filter2DType::HistogramPointer rankMap =
          rankFilter->GetAveragedRankHistogram();
      // make sure that the number of bins matches, and all ranks are 0!
      int b = rankMap->GetSize()[0] - 1;
      while (b >= 0)
      {
        if (rankMap->GetFrequency(b, 0) != 0)
        {
          lok = false;
          break;
        }
        b--;
      }
      if (rankMap->GetSize()[0] != 200)
        lok = false;
    }
    catch (itk::ExceptionObject &e)
    {
      lok = false;
    }
    ts = clock->GetTimeStamp() - ts;
    if (ExtendedOutput)
      VERBOSE(<< "\n    ~rank-map-extraction-time: "
          << ((ts / (double)endcount) * 1000.) << " ms\n")
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Masked rank image test ... ")
  lok = true;
  rankFilter->SetDoNotGenerateOutput(false); // generate output image as well
  try
  {
    itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
    double ts = clock->GetTimeStamp();
    int endcount = 50;
    HistogramSizeCheck = -1; // forget here
    HistogramTotalFrequencyCheck = -1;
    TransformedHistogramTotalFrequencyCheck = -1;
    HistogramFileName = "";
    TransformedHistogramFileName = "";
    RankHistogramFileName = "";
    rankFilter->SetGenerateOutputForMaskedPixelsOnly(false); // whole image
    for (int i = 0; i < endcount; ++i)
    {
      AfterHistogramExtractionEventFired = false; // set back
      AfterHistogramTransformationEventFired = false;
      AfterHistogramRankGenerationEventFired = false;

      rankFilter->Modified();
      rankFilter->Update();

      if (!AfterHistogramExtractionEventFired
          || AfterHistogramTransformationEventFired
          || !AfterHistogramRankGenerationEventFired)
        lok = false;

      Filter2DType::OutputImagePointer outputImage = rankFilter-> GetOutput();
      if (outputImage)
      {
        if (i == 0 && ImageOutput)
        {
          typedef itk::ImageFileWriter<Filter2DType::OutputImageType> WType;
          WType::Pointer w = WType::New();
          w->SetFileName("masked_output_image.mhd");
          w->SetInput(outputImage);
          w->Update();
        }
        if (outputImage->GetLargestPossibleRegion()
            != rankFilter->GetInput()->GetLargestPossibleRegion())
          lok = false;
      }
      else
      {
        lok = false;
      }
    }
    ts = clock->GetTimeStamp() - ts;
    if (ExtendedOutput)
      VERBOSE(<< "\n    ~rank-image-generation-time: "
          << ((ts / (double)endcount) * 1000.) << " ms\n")
    ts = clock->GetTimeStamp();
    rankFilter->SetGenerateOutputForMaskedPixelsOnly(true); // at mask pos.!
    for (int i = 0; i < endcount; ++i)
    {
      AfterHistogramExtractionEventFired = false; // set back
      AfterHistogramTransformationEventFired = false;
      AfterHistogramRankGenerationEventFired = false;

      rankFilter->Modified();
      rankFilter->Update();

      if (!AfterHistogramExtractionEventFired
          || AfterHistogramTransformationEventFired
          || !AfterHistogramRankGenerationEventFired)
        lok = false;

      Filter2DType::OutputImagePointer outputImage = rankFilter-> GetOutput();
      if (outputImage)
      {
        if (i == 0 && ImageOutput)
        {
          typedef itk::ImageFileWriter<Filter2DType::OutputImageType> WType;
          WType::Pointer w = WType::New();
          w->SetFileName("masked_output_image_at_unmasked_positions.mhd");
          w->SetInput(outputImage);
          w->Update();
        }
        if (outputImage->GetLargestPossibleRegion()
            != rankFilter->GetInput()->GetLargestPossibleRegion())
          lok = false;
      }
      else
      {
        lok = false;
      }
    }
    ts = clock->GetTimeStamp() - ts;
    if (ExtendedOutput)
      VERBOSE(<< "\n    ~rank-image-generation-time (unmasked positions): "
          << ((ts / (double)endcount) * 1000.) << " ms\n")
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Masked, transformed histogram extraction test ... ")
  lok = true;
  rankFilter->SetUseHistogramTransformation(true); // to 'original' range!
  rankFilter->SetDoNotGenerateOutput(true); // rank map only
  mask = GenerateTestMask("test_mask_2b.mhd", numPixels);
  rankFilter->SetMaskImage(mask); // masked!
  rankFilter->SetInput(image2);
  rankFilter->SetNumberOfHistogramBins(200);
  rankFilter->SetHistogramMinIntensity(0);
  rankFilter->SetHistogramMaxIntensity(225);
  rankFilter->SetHistogramClipAtEnds(false);
  try
  {
    itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
    double ts = clock->GetTimeStamp();
    int endcount = 100;
    for (int i = 0; i < endcount; ++i)
    {
      AfterHistogramExtractionEventFired = false; // set back
      AfterHistogramTransformationEventFired = false;
      AfterHistogramRankGenerationEventFired = false;

      HistogramSizeCheck = 200; // bins
      HistogramTotalFrequencyCheck = numPixels; // yet untransformed!!!
      TransformedHistogramTotalFrequencyCheck
          = image2->GetLargestPossibleRegion().GetNumberOfPixels(); // transformed!

      if (i == 0)
      {
        HistogramFileName = "masked2_histogram.csv";
        TransformedHistogramFileName = "masked2_transformed_histogram.csv";
        RankHistogramFileName = "masked2_rank_map.csv";
      }
      else
      {
        HistogramFileName = "";
        TransformedHistogramFileName = "";
        RankHistogramFileName = "";
      }

      rankFilter->Modified();
      rankFilter->Update();

      if (!AfterHistogramExtractionEventFired
          || !AfterHistogramTransformationEventFired
          || !AfterHistogramRankGenerationEventFired || !HistogramChecksOK)
        lok = false;

      Filter2DType::HistogramPointer rankMap =
          rankFilter-> GetAveragedRankHistogram();
      // make sure that the number of bins matches, and the last rank is <=
      // number of pixels of the image!
      double lastFrequency = 0;
      int b = rankMap->GetSize()[0] - 1;
      while (b >= 0)
      {
        if (rankMap->GetFrequency(b, 0) > 0)
        {
          lastFrequency = rankMap->GetFrequency(b, 0);
          break;
        }
        b--;
      }
      if (rankMap->GetSize()[0] != 200 || itk::Math::Round<unsigned int, double>(lastFrequency)
          > image2->GetLargestPossibleRegion().GetNumberOfPixels())
        lok = false;
    }
    ts = clock->GetTimeStamp() - ts;
    if (ExtendedOutput)
      VERBOSE(<< "\n    ~rank-map-extraction-time: "
          << ((ts / (double)endcount) * 1000.) << " ms\n")
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Masked, transformed rank image test ... ")
  lok = true;
  rankFilter->SetDoNotGenerateOutput(false); // generate output image as well
  rankFilter->SetUseHistogramTransformation(true);
  try
  {
    rankFilter->SetGenerateOutputForMaskedPixelsOnly(false); // whole image
    itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
    double ts = clock->GetTimeStamp();
    int endcount = 50;
    HistogramSizeCheck = -1; // forget here
    HistogramTotalFrequencyCheck = -1;
    TransformedHistogramTotalFrequencyCheck = -1;
    HistogramFileName = "";
    TransformedHistogramFileName = "";
    RankHistogramFileName = "";
    for (int i = 0; i < endcount; ++i)
    {
      AfterHistogramExtractionEventFired = false; // set back
      AfterHistogramTransformationEventFired = false;
      AfterHistogramRankGenerationEventFired = false;

      rankFilter->Modified();
      rankFilter->Update();

      if (!AfterHistogramExtractionEventFired
          || !AfterHistogramTransformationEventFired
          || !AfterHistogramRankGenerationEventFired)
        lok = false;

      Filter2DType::OutputImagePointer outputImage = rankFilter-> GetOutput();
      if (outputImage)
      {
        if (i == 0 && ImageOutput)
        {
          typedef itk::ImageFileWriter<Filter2DType::OutputImageType> WType;
          WType::Pointer w = WType::New();
          w->SetFileName("masked_transformed_output_image.mhd");
          w->SetInput(outputImage);
          w->Update();
        }
        if (outputImage->GetLargestPossibleRegion()
            != rankFilter->GetInput()->GetLargestPossibleRegion())
          lok = false;
      }
      else
      {
        lok = false;
      }
    }
    ts = clock->GetTimeStamp() - ts;
    if (ExtendedOutput)
      VERBOSE(<< "\n    ~rank-image-generation-time: "
          << ((ts / (double)endcount) * 1000.) << " ms\n")

    rankFilter->SetGenerateOutputForMaskedPixelsOnly(true); // masked pos.!
    ts = clock->GetTimeStamp();
    for (int i = 0; i < endcount; ++i)
    {
      AfterHistogramExtractionEventFired = false; // set back
      AfterHistogramTransformationEventFired = false;
      AfterHistogramRankGenerationEventFired = false;

      rankFilter->Modified();
      rankFilter->Update();

      if (!AfterHistogramExtractionEventFired
          || !AfterHistogramTransformationEventFired
          || !AfterHistogramRankGenerationEventFired)
        lok = false;

      Filter2DType::OutputImagePointer outputImage = rankFilter-> GetOutput();
      if (outputImage)
      {
        if (i == 0 && ImageOutput)
        {
          typedef itk::ImageFileWriter<Filter2DType::OutputImageType> WType;
          WType::Pointer w = WType::New();
          w->SetFileName("masked_transformed_output_image_at_unmasked_positions.mhd");
          w->SetInput(outputImage);
          w->Update();
        }
        if (outputImage->GetLargestPossibleRegion()
            != rankFilter->GetInput()->GetLargestPossibleRegion())
          lok = false;
      }
      else
      {
        lok = false;
      }
    }
    ts = clock->GetTimeStamp() - ts;
    if (ExtendedOutput)
      VERBOSE(<< "\n    ~rank-image-generation-time (unmasked positions): "
          << ((ts / (double)endcount) * 1000.) << " ms\n")
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  rankFilter->RemoveAllObservers();
  VERBOSE(<< "  * Final reference count check ... ")
  if (rankFilter->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  rankFilter = NULL; // reference counter must be zero!
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
