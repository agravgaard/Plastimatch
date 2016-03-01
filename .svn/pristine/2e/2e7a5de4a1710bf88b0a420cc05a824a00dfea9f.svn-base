#ifndef COMMONTOOLS_HXX_
#define COMMONTOOLS_HXX_

/**
 * Some tool function for the examples.
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 */

#include <math.h>
#include <string>
#include <vector>

#include <itksys/SystemTools.hxx>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionIteratorWithIndex.h>

#include <vtkSmartPointer.h>
#include <vtkColorTransferFunction.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// global types
typedef vtkSmartPointer<vtkColorTransferFunction> ITFPointer;

// global flag indicating whether "image output" is switched ON/OFF
bool ImageOutput = false;

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
template<typename T>
bool WriteImage(std::string fileName, T *image)
{
  if (!ImageOutput)
    return true;

  typedef itk::ImageFileWriter<T> WriterType;
  typename WriterType::Pointer w = WriterType::New();
  w->SetFileName(fileName.c_str());
  w->SetInput(image);
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

/** Convert ITF-string (number of pairs followed by pairs, space-sep.) to ITF **/
bool ITFStringToITF(std::string itfstring, ITFPointer itf)
{
  std::vector<std::string> tokens;
  std::string::size_type lastPos = itfstring.find_first_not_of(" ", 0);
  std::string::size_type pos = itfstring.find_first_of(" ", lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
  {
    tokens.push_back(itfstring.substr(lastPos, pos - lastPos));
    lastPos = itfstring.find_first_not_of(" ", pos);
    pos = itfstring.find_first_of(" ", lastPos);
  }

  if (tokens.size() < 5) // at least 2 supporting points required
    return false;

  int numPairs = atoi(tokens[0].c_str());
  if ((numPairs * 2) != (int) (tokens.size() - 1))
    return false;

  double maxVal = -1;
  for (std::size_t i = 1; i < tokens.size(); i += 2)
  {
    if (atof(tokens[i].c_str()) > maxVal) // search max. intensity (x-axis)
      maxVal = atof(tokens[i].c_str());
  }
  for (std::size_t i = 2; i < tokens.size(); i += 2)
  {
    double v = atof(tokens[i].c_str()) / maxVal; // normalize to [0;1]
    itf->AddRGBPoint(atof(tokens[i - 1].c_str()), v, v, v);
  }

  return true;
}

/** Mask configuration. **/
typedef struct MaskConfigItemStruct
{
  int type; // 1=circular,2=rectangular,3=file
  std::string file;
  double x;
  double y;
  double p1;
  double p2;

  MaskConfigItemStruct()
  {
    type = 0;
    x = y = p1 = p2 = 0;
    file = "";
  }
} MaskConfigItem;

/** Generate a circular/rectangular mask with specified properties. **/
template<typename MaskImageType, typename FixedImageType,
    typename MetricFixedImageType>
typename MaskImageType::Pointer GenerateGeometricMask(
    MaskConfigItem &maskConfig, typename FixedImageType::Pointer fixedImage,
    typename MetricFixedImageType::RegionType &freg)
{
  if (maskConfig.type < 1 || maskConfig.type > 3)
    return NULL;

  typename MaskImageType::Pointer fmask = MaskImageType::New();
  typename MaskImageType::PointType fmorig;
  fmorig.Fill(0);
  fmask->SetOrigin(fmorig);
  typename FixedImageType::SpacingType fspac = fixedImage->GetSpacing();
  typename MaskImageType::SpacingType fmspac;
  fmspac[0] = fspac[0];
  fmspac[1] = fspac[1];
  fmask->SetSpacing(fmspac);
  typename MetricFixedImageType::DirectionType fmdir;
  fmdir.SetIdentity();
  fmask->SetDirection(fmdir);
  typename MaskImageType::RegionType fmreg;
  typename MaskImageType::IndexType fmidx;
  fmidx.Fill(0);
  typename MaskImageType::SizeType fmsz;
  fmsz[0] = freg.GetSize()[0];
  fmsz[1] = freg.GetSize()[1];
  fmreg.SetIndex(fmidx);
  fmreg.SetSize(fmsz);
  fmask->SetRegions(fmreg);
  fmask->Allocate();
  fmask->FillBuffer(0); // background

  typedef itk::ImageRegionIteratorWithIndex<MaskImageType> MaskIteratorType;
  MaskIteratorType fimit(fmask, fmreg);
  if (maskConfig.type == 1) // circular
  {
    double r2 = maskConfig.p1 * maskConfig.p1; // sqr
    double c[2];
    c[0] = maskConfig.x;
    c[1] = maskConfig.y;
    while (!fimit.IsAtEnd())
    {
      typename MaskImageType::IndexType idx = fimit.GetIndex();
      double posX = (double) idx[0] * fmspac[0];
      double posY = (double) idx[1] * fmspac[1];

      double xx = posX - c[0];
      double yy = posY - c[1];
      if ((xx * xx + yy * yy) <= r2)
        fimit.Set(1); // set pixel inside circular mask

      ++fimit;
    }
  }
  else if (maskConfig.type == 2) // rectangular
  {
    double xmin = maskConfig.x;
    double xmax = maskConfig.x + maskConfig.p1;
    double ymin = maskConfig.y;
    double ymax = maskConfig.y + maskConfig.p2;

    while (!fimit.IsAtEnd())
    {
      typename MaskImageType::IndexType idx = fimit.GetIndex();
      double posX = (double) idx[0] * fmspac[0];
      double posY = (double) idx[1] * fmspac[1];
      if (posX >= xmin && posX <= xmax && posY >= ymin && posY <= ymax)
        fimit.Set(1); // set pixel inside rectangular mask
      ++fimit;
    }
  }
  else if (maskConfig.type == 3) // from file
  {
    fmask = ReadImage<MaskImageType> (maskConfig.file);
  }

  return fmask;
}

#endif /* COMMONTOOLS_HXX_ */
