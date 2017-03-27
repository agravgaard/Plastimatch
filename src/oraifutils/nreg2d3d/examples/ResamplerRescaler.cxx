//
#include <iostream>
#include <stdlib.h>
#include <string>

#include <itksys/SystemTools.hxx>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkEuler3DTransform.h>
#include <itkExtractImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkIntensityWindowingImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>

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

/**
 * Print test usage information.
 **/
void PrintUsage(char *binname)
{
  std::string progname = "<application-binary-name>";

  if (binname)
    progname = std::string(binname);

  std::cout << "\n";
  std::cout << "   *** R E S A M P L E R R E S C A L E R   U S A G E ***\n";
  std::cout << "\n";
  std::cout << progname << " [options] <input-image> <output-image>\n";
  std::cout << "\n";
  std::cout << "  -h or --help ... print this short help\n";
  std::cout
      << "  -v or --verbose ... verbose messages to std::cout [optional]\n";
  std::cout
      << "  -c or --crop ... crop the image (3D -> 3D): idx[0], idx[1], idx[2], size[0], size[1], size[2]\n";
  std::cout
      << "  -s or --resample ... resample the image (3D -> 3D): spacing[0], spacing[1], spacing[2], interpolation-mode (linear or b-spline-5)\n";
  std::cout
      << "  -r1 or --rescale1 ... min/max rescale the image (3D -> 3D): intensity-min, intensity-max\n";
  std::cout
      << "  -r2 or --rescale2 ... shift / scale the image (3D -> 3D): shift, scale\n";
  std::cout
      << "  -r3 or --rescale3 ... intensity-window the image (3D -> 3D): window-min, window-max, output-min, output-max\n";
  std::cout
      << "  -om or --output-mode ... optionally cast the image output pixel type: r (for real) or c (for unsigned char) or s (for unsigned short) [default: r]\n";
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

typedef float RealPixelType;
typedef unsigned char UCHARPixelType;
typedef unsigned short USHORTPixelType;
typedef itk::Image<RealPixelType, 3> RealImageType;
typedef itk::Image<UCHARPixelType, 3> UCHARImageType;
typedef itk::Image<USHORTPixelType, 3> USHORTImageType;
typedef itk::ExtractImageFilter<RealImageType, RealImageType>
    ExtractionFilterType;
typedef itk::InterpolateImageFunction<RealImageType, double> InterpolationType;
typedef itk::LinearInterpolateImageFunction<RealImageType, double>
    LinearInterpolationType;
typedef itk::BSplineInterpolateImageFunction<RealImageType, double>
    BSplineInterpolationType;
typedef itk::ResampleImageFilter<RealImageType, RealImageType, double>
    ResampleFilterType;
typedef itk::Euler3DTransform<double> TransformType;
typedef itk::RescaleIntensityImageFilter<RealImageType, RealImageType>
    RescaleFilterType1;
typedef itk::ShiftScaleImageFilter<RealImageType, RealImageType>
    RescaleFilterType2;
typedef itk::IntensityWindowingImageFilter<RealImageType, RealImageType>
    RescaleFilterType3;

/** Extraction config. **/
typedef struct ExtractionConfigStruct
{
  RealImageType::IndexType Index;
  RealImageType::SizeType Size;

  ExtractionConfigStruct()
  {
    Index.Fill(0);
    Size.Fill(0);
  }
} ExtractionConfig;

/** Resampling config. **/
typedef struct ResampleConfigStruct
{
  RealImageType::SpacingType Spacing;
  std::string InterpolationMode;

  ResampleConfigStruct()
  {
    Spacing.Fill(1);
    InterpolationMode = "linear"; // b-spline-5
  }
} ResampleConfig;

/** Rescale config. **/
typedef struct RescaleConfigStruct
{
  double min1;
  double max1;
  double min2;
  double max2;
  int mode;

  RescaleConfigStruct()
  {
    mode = 0; // rescale
    min1 = min2 = max1 = max2 = 0;
  }
} RescaleConfig;

/** Cast config. **/
typedef struct CastConfigStruct
{
  std::string mode;

  CastConfigStruct()
  {
    mode = "r";
  }
} CastConfig;

/** Read a specified image file. **/
template<typename T>
typename T::Pointer ReadImage(std::string fileName)
{
  typename T::Pointer image = NULL;

  typedef itk::ImageFileReader<T> ReaderType;

  typename ReaderType::Pointer r = ReaderType::New();
  r->SetFileName(fileName.c_str());
  try
  {
    image = r->GetOutput();
    r->Update();
    image->DisconnectPipeline();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "ERROR: Reading file '" << fileName << "'.\n";
    image = NULL;
  }

  return image;
}

/** Write a specified image file. **/
template<typename T, typename T2>
bool CastAndWriteImage(std::string fileName, typename T::Pointer image)
{
  typedef itk::CastImageFilter<T, T2> CastFilterType;
  typename CastFilterType::Pointer c = CastFilterType::New();
  c->SetInput(image);
  typename T2::Pointer cimage = c->GetOutput();
  try
  {
    c->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "ERROR: Casting image:" << e << ".\n";
    return false;
  }

  typedef itk::ImageFileWriter<T2> WriterType;
  typename WriterType::Pointer w = WriterType::New();
  w->SetFileName(fileName.c_str());
  w->SetInput(cimage);
  try
  {
    w->Update();
    return true;
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << "ERROR: Writing file '" << fileName << "'.\n";
    return false;
  }
}

/**
 * Tool application that resamples, rescales, crops and casts 3D images. It can
 * for example be used for resampling and rescaling X-ray images.
 *
 * Each sub-operation is optional. The following order is applied:<br>
 * - extract region<br>
 * - resample image<br>
 * - rescale image<br>
 * - cast image<br>
 *
 * Fundamentally, each image is treated (read) with real pixel type (which is
 * also the default for output).
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 */
int main(int argc, char *argv[])
{
  // arguments check
  if (argc < 3)
  {
    if (argc > 0)
      PrintUsage(argv[0]);
    else
      PrintUsage(NULL);
    return EXIT_FAILURE;
  }

  int last = 0;
  ExtractionConfig *extractConfig = NULL;
  ResampleConfig *resampleConfig = NULL;
  RescaleConfig *rescaleConfig = NULL;
  CastConfig *castConfig = new CastConfig();

  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-v" || std::string(argv[i]) == "--verbose")
    {
      Verbose = true;
      last = i;
      continue;
    }
    if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
    {
      if (argc > 0)
        PrintUsage(argv[0]);
      else
        PrintUsage(NULL);
      last = i;
      return EXIT_FAILURE;
    }
    if (std::string(argv[i]) == "-c" || std::string(argv[i]) == "--crop")
    {
      last = i + 6;
      extractConfig = new ExtractionConfig();
      i++;
      extractConfig->Index[0] = atof(argv[i]);
      i++;
      extractConfig->Index[1] = atof(argv[i]);
      i++;
      extractConfig->Index[2] = atof(argv[i]);
      i++;
      extractConfig->Size[0] = atof(argv[i]);
      i++;
      extractConfig->Size[1] = atof(argv[i]);
      i++;
      extractConfig->Size[2] = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-s" || std::string(argv[i]) == "--resample")
    {
      last = i + 4;
      resampleConfig = new ResampleConfig();
      i++;
      resampleConfig->Spacing[0] = atof(argv[i]);
      i++;
      resampleConfig->Spacing[1] = atof(argv[i]);
      i++;
      resampleConfig->Spacing[2] = atof(argv[i]);
      i++;
      resampleConfig->InterpolationMode = std::string(argv[i]);
      if (resampleConfig->InterpolationMode != "linear"
          && resampleConfig->InterpolationMode != "b-spline-5")
      {
        std::cerr << "Unknown interpolation mode: '"
            << resampleConfig->InterpolationMode << "'\n";
        return EXIT_FAILURE;
      }
      continue;
    }
    if (std::string(argv[i]) == "-r1" || std::string(argv[i]) == "--rescale1")
    {
      rescaleConfig = new RescaleConfig();
      last = i + 2;
      rescaleConfig->mode = 1;
      i++;
      rescaleConfig->min1 = atof(argv[i]);
      i++;
      rescaleConfig->max1 = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-r2" || std::string(argv[i]) == "--rescale2")
    {
      rescaleConfig = new RescaleConfig();
      last = i + 2;
      rescaleConfig->mode = 2;
      i++;
      rescaleConfig->min1 = atof(argv[i]);
      i++;
      rescaleConfig->max1 = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-r3" || std::string(argv[i]) == "--rescale3")
    {
      rescaleConfig = new RescaleConfig();
      last = i + 4;
      rescaleConfig->mode = 3;
      i++;
      rescaleConfig->min1 = atof(argv[i]);
      i++;
      rescaleConfig->max1 = atof(argv[i]);
      i++;
      rescaleConfig->min2 = atof(argv[i]);
      i++;
      rescaleConfig->max2 = atof(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-om" || std::string(argv[i])
        == "--output-mode")
    {
      last = i + 1;
      i++;
      castConfig->mode = std::string(argv[i]);
      if (castConfig->mode != "r" && castConfig->mode != "c"
          && castConfig->mode != "s")
      {
        std::cerr << "Unknown output mode: '" << castConfig->mode << "'\n";
        return EXIT_FAILURE;
      }
      continue;
    }
  }
  if ((last + 3) > argc)
  {
    std::cout << "Obviously command line arguments are invalid.\n";
    std::cout << "Need input and output image as last arguments!\n";
    return EXIT_FAILURE;
  }

  // get image files and projection props:
  VERBOSE(<< " > Read input image\n")
  std::string inputFile = std::string(argv[++last]);
  if (!itksys::SystemTools::FileExists(inputFile.c_str(), true))
  {
    std::cout << "ERROR: Input image file '" << inputFile
        << "' does not exist!\n";
    return EXIT_FAILURE;
  }
  RealImageType::Pointer inputImage = ReadImage<RealImageType> (inputFile);
  if (!inputImage)
  {
    std::cout << "Could not read input image!\n";
    return EXIT_FAILURE;
  }
  std::string outputFile = std::string(argv[++last]);

  // optional cropping:
  if (extractConfig)
  {
    VERBOSE(<< " > Crop image\n")
    ExtractionFilterType::Pointer extractor = ExtractionFilterType::New();
    extractor->SetInput(inputImage);
    RealImageType::RegionType ereg;
    ereg.SetIndex(extractConfig->Index);
    ereg.SetSize(extractConfig->Size);
    extractor->SetExtractionRegion(ereg);
    try
    {
      extractor->Update();
      inputImage = extractor->GetOutput();
      inputImage->DisconnectPipeline();
      RealImageType::RegionType ireg = inputImage->GetLargestPossibleRegion();
      RealImageType::IndexType zidx;
      zidx.Fill(0);
      ireg.SetIndex(zidx);
      inputImage->SetRegions(ireg); // set back start index
      // adapt origin:
      RealImageType::DirectionType dir = inputImage->GetDirection();
      RealImageType::PointType orig = inputImage->GetOrigin();
      for (int d = 0; d < 3; d++)
      {
        double f = (double)ereg.GetIndex()[d] * inputImage->GetSpacing()[d];
        for (int d2 = 0; d2 < 3; d2++)
          orig[d2] += dir[d2][d] * f;
      }
      inputImage->SetOrigin(orig);
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << "ERROR during cropping: " << e << "\n";
      return EXIT_FAILURE;
    }
  }

  // optional resampling:
  if (resampleConfig)
  {
    InterpolationType::Pointer interpolator = NULL;
    if (resampleConfig->InterpolationMode == "linear")
    {
      LinearInterpolationType::Pointer interp = LinearInterpolationType::New();
      interpolator = interp;
    }
    else if (resampleConfig->InterpolationMode == "b-spline-5")
    {
      BSplineInterpolationType::Pointer interp =
          BSplineInterpolationType::New();
      interp->SetSplineOrder(5);
      interpolator = interp;
    }
    TransformType::Pointer transform = TransformType::New();
    transform->SetIdentity();
    ResampleFilterType::Pointer resampler = ResampleFilterType::New();
    resampler->SetTransform(transform);
    resampler->SetInterpolator(interpolator);
    resampler->SetOutputSpacing(resampleConfig->Spacing);
    resampler->SetOutputOrigin(inputImage->GetOrigin());
    resampler->SetOutputDirection(inputImage->GetDirection());
    RealImageType::IndexType sidx;
    sidx.Fill(0);
    resampler->SetOutputStartIndex(sidx);
    RealImageType::SizeType newSize;
    for (int d = 0; d < 3; d++)
    {
      newSize[d] = static_cast<RealImageType::SizeValueType> (std::floor(
          inputImage->GetLargestPossibleRegion().GetSize()[d]
              * inputImage->GetSpacing()[d] / resampleConfig->Spacing[d]));
    }
    resampler->SetSize(newSize);
    resampler->SetInput(inputImage);
    try
    {
      resampler->Update();
      inputImage = resampler->GetOutput();
      inputImage->DisconnectPipeline();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << "ERROR during resampling: " << e << "\n";
      return EXIT_FAILURE;
    }
  }

  // optional intensity rescaling (3 different modes)
  if (rescaleConfig)
  {
    typedef itk::ImageToImageFilter<RealImageType, RealImageType> FilterType;
    FilterType::Pointer filter = NULL;
    if (rescaleConfig->mode == 1)
    {
      RescaleFilterType1::Pointer r = RescaleFilterType1::New();
      r->SetOutputMinimum(rescaleConfig->min1);
      r->SetOutputMaximum(rescaleConfig->max1);
      filter = r;
    }
    else if (rescaleConfig->mode == 2)
    {
      RescaleFilterType2::Pointer r = RescaleFilterType2::New();
      r->SetShift(rescaleConfig->min1);
      r->SetScale(rescaleConfig->max1);
      filter = r;
    }
    else if (rescaleConfig->mode == 3)
    {
      RescaleFilterType3::Pointer r = RescaleFilterType3::New();
      r->SetWindowMinimum(rescaleConfig->min1);
      r->SetWindowMaximum(rescaleConfig->max1);
      r->SetOutputMinimum(rescaleConfig->min2);
      r->SetOutputMaximum(rescaleConfig->max2);
      filter = r;
    }
    else
    {
      std::cerr << "Unknown rescale mode." << std::endl;
      return EXIT_FAILURE;
    }
    filter->SetInput(inputImage);
    try
    {
      filter->Update();
      inputImage = filter->GetOutput();
      inputImage->DisconnectPipeline();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << "ERROR during rescaling: " << e << "\n";
      return EXIT_FAILURE;
    }
  }

  // finally write out image (including optional casting to other pixel mode):
  VERBOSE(<< " > Write (and cast) output image\n")
  if (castConfig->mode == "r")
    CastAndWriteImage<RealImageType, RealImageType> (outputFile, inputImage);
  else if (castConfig->mode == "c")
    CastAndWriteImage<RealImageType, UCHARImageType> (outputFile, inputImage);
  else if (castConfig->mode == "s")
    CastAndWriteImage<RealImageType, USHORTImageType> (outputFile, inputImage);

  return EXIT_SUCCESS;
}
