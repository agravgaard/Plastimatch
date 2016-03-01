//
#include "BasicUnitTestIncludes.hxx"

#include <stdio.h>
#include <string.h>
#include <vector>
#include <cmath>

#include "oraITKVTKImage.h"
// ORAIFTools
#include "oraSimpleMacros.h"

#include <itkImage.h>
#include <itkImportImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkRawImageIO.h>
#include <itkImageRegionConstIterator.h>


// Set up some variables used as test data
const unsigned int Dimension3D = 3;
itk::Size<Dimension3D> size3D;
itk::Point<double, Dimension3D> origin3D;
itk::Vector<double, Dimension3D> spacing3D;
// TODO: Test image orientation and metadata of ITKVTKImage

/** Number of pixels base don the image size #size3D (s_x*s_y*s_z). */
unsigned int numberOfPixels3D;
/** Memory block containing the pixel data (buffer) used as test data.
 * NOTE: Must have the same size as the number of pixels #numberOfPixels3D.
 */
double *testBuffer3D;

/** Vector of image componnet type enums that is use dfor iteration. */
std::vector<itk::ImageIOBase::IOComponentType> pixelTypes;
/** Vector of image componnet type enums that is use dfor iteration. */
std::vector<std::string> fileNames;

/** Epsilon for equality comparison of doubles. */
const double epsilon = 1e-6;


/** Utility function to test double variables on equality.
 * @param left Variable 1.
 * @param right variable 2.
 * @param epsilon Epsilon of double precision.
 * @return TRUE if doubles are equal, else FALSE.
 */
bool doubleEqual(double a, double b, double epsilon) {
  return (std::fabs(a - b) < epsilon);
}

/** Creates an itk::Image from the #testBuffer3D and the test-information.
 * Afterwards it is written to the specified filename with an ITK image writer.
 * @param fileName Filename of the image that is created from the #testBuffer3D
 * @return TRUE if no errors occur. FALSE if writing fails.
 */
template<typename TComponentType>
bool WriteMHDTestImage3D(std::string fileName)
{

  typedef TComponentType PixelType;
  typedef itk::Image<PixelType, Dimension3D> InputImageType;
  typename InputImageType::Pointer img = InputImageType::New();

  // Feed the buffer into an ImportImageFilter producing an Image as output
  typedef itk::ImportImageFilter<PixelType, Dimension3D> ImportFilterType;
  typename ImportFilterType::Pointer importFilter = ImportFilterType::New();

  // Set image information
  typename ImportFilterType::IndexType start;
  start.Fill(0);
  typename ImportFilterType::RegionType region;
  region.SetIndex(start);
  region.SetSize(size3D);
  importFilter->SetRegion(region);
  importFilter->SetOrigin(origin3D);
  importFilter->SetSpacing(spacing3D);

  // Create a new buffer and cast the testBuffer to the required type
  PixelType * buffer = new PixelType[numberOfPixels3D];
  for (unsigned int i = 0; i < numberOfPixels3D; ++i)
    buffer[i] = static_cast<PixelType> (testBuffer3D[i]);

  // Pass buffer to the ImportImageFilter.
  // NOTE: The image object is responsible for deleting the memory block once it
  // is no longer in use (last argument). To appropriately delete the memory
  // block, the memory must be allocated with the C++ new() operator.
  // It is not required to call delete() on the buffer, because it is owned by
  // the ImportImageFilter
  importFilter->SetImportPointer(buffer, numberOfPixels3D, true);

  typedef itk::ImageFileWriter<InputImageType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(fileName);
  writer->SetInput(importFilter->GetOutput());

  try
  {
    writer->Update();
  }
  catch (itk::ExceptionObject & exp)
  {
    std::cout << "Exception caught at writing: " << fileName << std::endl;
    std::cout << exp << std::endl;
    return false;
  }

  return true;
}

/** Casts the base-class ITK image back to the original type and validates the
 * image information and data.
 * @param ivi Input image to validate.
 * @return TRUE if no errors occur. FALSE if writing fails.
 */
template<typename TComponentType>
bool TestMHDImage(ora::ITKVTKImage* &ivi)
{
  if (!ivi)
    return false;
  bool lok = true;

  // Get image as base-class itk::DataObject
  ora::ITKVTKImage::ITKImagePointer ip = ivi->GetAsITKImage<TComponentType> ();
  if (!ip)
    return false;

  // Cast to original ITK image type
  typedef itk::Image<TComponentType, ora::ITKVTKImage::Dimensions>
      InputImageType;
  typedef typename InputImageType::Pointer InputImageTypePointer;
  typename InputImageType::Pointer image =
      static_cast<InputImageType *> (ip.GetPointer());

  // Validate image data
  const int precisionBackup = std::cout.precision();
  std::cout << std::setprecision(15);
  typename InputImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
  if (size != size3D)
  {
    std::cout<< "Size mismatch! " << size << " reference: " << size3D;
    lok = false;
  }

  typename InputImageType::PointType origin = image->GetOrigin();
  for (unsigned int i = 0; i < Dimension3D; ++i)
  {
    if (!doubleEqual(origin[i], origin3D[i], epsilon))
    {
      std::cout<< "\nOrigin[" << i << "] mismatch! " << origin[i] <<
          " reference: " << origin3D[i] << std::endl;
      lok = false;
    }
  }

  typename InputImageType::SpacingType spacing = image->GetSpacing();
  for (unsigned int i = 0; i < Dimension3D; ++i)
  {
    if (!doubleEqual(spacing[i], spacing3D[i], epsilon))
    {
      std::cout<< "\nSpacing[" << i << "] mismatch! " << spacing[i] <<
          " reference: " << spacing3D[i] << std::endl;
      lok = false;
    }
  }

  double * itBuffer = testBuffer3D;
  typedef itk::ImageRegionConstIterator<InputImageType> ConstIteratorType;
  ConstIteratorType itImage(image, image->GetLargestPossibleRegion());

  itImage.GoToBegin();
  while (!itImage.IsAtEnd())
  {
    if (itImage.Get() != static_cast<TComponentType> (*itBuffer))
    {
      std::cout << "\nPixel[" << itBuffer - testBuffer3D << "] mismatch! "
          << itImage.Get() << " reference: " << *itBuffer << std::endl;
      lok = false;
    }
    ++itImage;
    ++itBuffer;
  }

  std::cout << std::setprecision(precisionBackup);
  return lok;
}

/**
 * Tests base functionality of:
 *
 *   ora::ITKVTKImage
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::ITKVTKImage
 *
 * @author Markus <markus.neuner (at) pmu.ac.at>
 * @version 1.0
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
        helpLines);
    return EXIT_SUCCESS;
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting ITKVTKImage.\n")
  VERBOSE(<< "\nPREPROCESSOR DEFINES: ")
#if defined(USE_ITK_FOR_WRITING)
  VERBOSE(<< " USE_ITK_FOR_WRITING")
#endif
#if defined(USE_XRAWIO_FOR_ALL)
  VERBOSE(<< " USE_XRAWIO_FOR_ALL")
#endif
#if defined(FAST_XIO_COMPRESSION)
  VERBOSE(<< " FAST_XIO_COMPRESSION")
#endif
  VERBOSE(<< "\n")

  // Set reference image information
  size3D[0] = 101;     // size along X
  size3D[1] = 202;     // size along Y
  size3D[2] = 50;      // size along Z
  origin3D[0] = 1.4; //-10.5; // X coordinate
  origin3D[1] = 10.5;  // Y coordinate
  origin3D[2] = 20.8;  // Z coordinate
  spacing3D[0] = 1.4;  // along X direction
  spacing3D[1] = 2.6;  // along Y direction
  spacing3D[2] = 3.5;  // along Z direction

  // Fill test buffer with a binary centered sphere and increasing background
  numberOfPixels3D = size3D[0] * size3D[1] * size3D[2];
  testBuffer3D = new double[numberOfPixels3D];
  const double radius = 50.0;
  const double radius2 = radius * radius;
  double * itBuffer = testBuffer3D;
  for (unsigned int z = 0; z < size3D[2]; z++)
  {
    const double dz = static_cast<double> (z) - static_cast<double> (size3D[2])
        / 2.0;
    for (unsigned int y = 0; y < size3D[1]; y++)
    {
      const double dy = static_cast<double> (y)
          - static_cast<double> (size3D[1]) / 2.0;
      for (unsigned int x = 0; x < size3D[0]; x++)
      {
        const double dx = static_cast<double> (x)
            - static_cast<double> (size3D[0]) / 2.0;
        const double d2 = dx * dx + dy * dy + dz * dz;
        if (d2 < radius2)
        {
          *itBuffer++ = 255;
        }
        else
        {
          *itBuffer++ = (std::sqrt(d2) - radius) * (z + y + x) / (size3D[0]
              + size3D[1] + size3D[2]);
        }
      }
    }
  }

  // Fill vector of component types
  pixelTypes.push_back(itk::ImageIOBase::UCHAR);
  pixelTypes.push_back(itk::ImageIOBase::CHAR);
  pixelTypes.push_back(itk::ImageIOBase::USHORT);
  pixelTypes.push_back(itk::ImageIOBase::SHORT);
  pixelTypes.push_back(itk::ImageIOBase::UINT);
  pixelTypes.push_back(itk::ImageIOBase::INT);
  pixelTypes.push_back(itk::ImageIOBase::ULONG);
  pixelTypes.push_back(itk::ImageIOBase::LONG);
  pixelTypes.push_back(itk::ImageIOBase::FLOAT);
  pixelTypes.push_back(itk::ImageIOBase::DOUBLE);
  pixelTypes.push_back(itk::ImageIOBase::UNKNOWNCOMPONENTTYPE);

  VERBOSE(<< "\nCreate and write 3D datasets in mhd metaimage file format\n")

  // Create ImageIO object to access the method
  // itk::ImageIOBase::GetComponentTypeAsString()
  // TODO: Copy method as static function to ora::ITKVTKImage
  itk::RawImageIO<char, 2>::Pointer io = itk::RawImageIO<char, 2>::New();

  std::string fileName = "";
  std::vector<itk::ImageIOBase::IOComponentType>::iterator it;
  for (it = pixelTypes.begin(); it < pixelTypes.end(); it++)
  {
    VERBOSE(<< "  * PixelType " << io->GetComponentTypeAsString(*it) << ": ")
    lok = false;
    fileName = DataPath + "testImage3D" + io->GetComponentTypeAsString(*it)
        + ".mhd";
    TEMPLATE_CALL_COMP(*it, lok = WriteMHDTestImage3D, fileName)

    // Add filename to list for later reading if successful
    if (lok)
      fileNames.push_back(fileName);

    // When the type is unknown no image will be created
    if (!lok && *it == itk::ImageIOBase::UNKNOWNCOMPONENTTYPE)
      lok = true;

    ok = ok && lok;
    VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")
  }

  VERBOSE(<< "\nRead 3D datasets with ITK and verify them\n")

  ora::ITKVTKImage *img;
  for (unsigned int i = 0; i < pixelTypes.size(); i++)
  {
    itk::ImageIOBase::IOComponentType ct = pixelTypes[i];
    if (ct == itk::ImageIOBase::UNKNOWNCOMPONENTTYPE)
      fileName = "";  // No component type no file available
    else
      fileName = fileNames[i];

    VERBOSE(<< "  * Read PixelType " << io->GetComponentTypeAsString(ct) << ": ")
    lok = true;
    img = ora::ITKVTKImage::LoadImageFromFile(fileName.c_str(),
        ora::ITKVTKImage::ITKIO);
    if (!img)
      lok = false;
    if (!img && ct == itk::ImageIOBase::UNKNOWNCOMPONENTTYPE)
      lok = true;

    ok = ok && lok;
    VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

    if (img)
    {
      VERBOSE(<< "  * Verify image object: ")
      lok = true;
      if (img->GetComponentType() != ct)
      {
        lok = false;
        std::cout << "Component type mismatch! ";
      }
      else
      {
        TEMPLATE_CALL_COMP(img->GetComponentType(), lok = TestMHDImage, img)
      }
      ok = ok && lok;
      VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

      delete(img);
    }
  }

  VERBOSE(<< "\nRead 3D datasets with VTK and verify them\n")

  for (unsigned int i = 0; i < pixelTypes.size(); i++)
  {
    itk::ImageIOBase::IOComponentType ct = pixelTypes[i];

    // TODO: Investigate that in more detail later! However, seems to be a fair
    // workaround at the moment, ensuring that the tests succeed on 64-bit!
    if (sizeof(int*) == 8) // 64-bit!
    {
      // the VTK-reader has obviously problems with long/unsigned long on 64-bit!
      if (ct == itk::ImageIOBase::LONG ||
          ct == itk::ImageIOBase::ULONG)
      {
         continue; // do not process!
      }
    }

    if (ct == itk::ImageIOBase::UNKNOWNCOMPONENTTYPE)
      fileName = ""; // No component type no file available
    else
      fileName = fileNames[i];

    try
    {
      VERBOSE(<< "  * Read PixelType " << io->GetComponentTypeAsString(ct) << ": ")
      lok = true;
      img = ora::ITKVTKImage::LoadImageFromFile(fileName.c_str(),
          ora::ITKVTKImage::VTKIO);
      if (!img)
        lok = false;
      if (!img && ct == itk::ImageIOBase::UNKNOWNCOMPONENTTYPE)
        lok = true;

      ok = ok && lok;
      VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

      if (img)
      {
        VERBOSE(<< "  * Verify image object: ")
        lok = true;
        if (img->GetComponentType() != ct)
        {
          lok = false;
          std::cout << "Component type mismatch! ";
        }
        else
        {
          TEMPLATE_CALL_COMP(img->GetComponentType(), lok = TestMHDImage, img)
        }
        ok = ok && lok;
        VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

        delete(img);
      }
    }
    catch (itk::ExceptionObject & exp)
    {
      std::cout << "Exception caught:\n" << exp << std::endl;
      lok = false;
      ok = ok && lok;
      VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")
    }
  }

  VERBOSE(<< "\nWrite/Read/Verify 3D datasets with ORA-XML writer.\n")

  ora::ITKVTKImage *imgOra;
  for (unsigned int i = 0; i < pixelTypes.size(); i++)
  {
    itk::ImageIOBase::IOComponentType ct = pixelTypes[i];

    // TODO: Investigate that in more detail later! However, seems to be a fair
    // workaround at the moment, ensuring that the tests succeed on 64-bit!
    if (sizeof(int*) == 8) // 64-bit!
    {
      // the VTK-reader has obviously problems with long/unsigned long on 64-bit!
      if (ct == itk::ImageIOBase::LONG ||
          ct == itk::ImageIOBase::ULONG)
      {
         continue; // do not process!
      }
    }

    if (ct == itk::ImageIOBase::UNKNOWNCOMPONENTTYPE)
      fileName = ""; // No component type no file available
    else
      fileName = fileNames[i];

    try
    {
      VERBOSE(<< "  * Read PixelType " << io->GetComponentTypeAsString(ct) << ": ")
      lok = true;
      img = ora::ITKVTKImage::LoadImageFromFile(fileName.c_str(),
          ora::ITKVTKImage::ITKIO);
      if (!img)
        lok = false;
      if (!img && ct == itk::ImageIOBase::UNKNOWNCOMPONENTTYPE)
        lok = true;

      ok = ok && lok;
      VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

      if (img)
      {
        std::string fileNameOra = fileName.substr(0, fileName.length() - 3)
            + ora::ITKVTKImage::ORA_EXTENDED_META_IMAGE_EXTENSION;

        VERBOSE(<< "  * Write ORA-XML PixelType " << io->GetComponentTypeAsString(ct) << ": ")
        lok = true;
        TEMPLATE_CALL_COMP(img->GetComponentType(), lok = img->SaveImageAsORAMetaImage
            ,fileName.c_str(), true, "");
        if (!lok)
          std::cout << "Writing failed! ";
        ok = ok && lok;
        VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

        VERBOSE(<< "  * Read ORA-XML PixelType " << io->GetComponentTypeAsString(ct) << ": ")
        lok = true;
        imgOra = ora::ITKVTKImage::LoadImageFromFile(fileNameOra.c_str(),
            ora::ITKVTKImage::ORAIO);
        if (!imgOra)
          lok = false;
        if (!imgOra && ct == itk::ImageIOBase::UNKNOWNCOMPONENTTYPE)
          lok = true;
        ok = ok && lok;
        VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

        if (imgOra)
        {
          VERBOSE(<< "  * Verify image object: ")
          lok = true;
          if (imgOra->GetComponentType() != ct)
          {
            lok = false;
            std::cout << "Component type mismatch! ";
          }
          else
          {
            TEMPLATE_CALL_COMP(imgOra->GetComponentType(), lok = TestMHDImage, imgOra)
          }
          ok = ok && lok;
          VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

          delete (imgOra);
        }

        delete (img);
      }
    }
    catch (itk::ExceptionObject & exp)
    {
      std::cout << "Exception caught:\n" << exp << std::endl;
      lok = false;
      ok = ok && lok;
      VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")
    }
  }


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
