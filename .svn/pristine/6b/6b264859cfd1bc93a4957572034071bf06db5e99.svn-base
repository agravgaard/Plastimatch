//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <time.h>
#include <cstddef> /* Workaround bug in ITK 3.20 */

#include <itkImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itkImageFileWriter.h>
#include <itkEuler3DTransform.h>
#include <itkExtractImageFilter.h>
#include <itkImageToImageFilterDetail.h>
#include <itkMath.h>

#include "BasicUnitTestIncludes.hxx"

#include "oraGLSLRayCastingDRRFilter.h"
#include "oraDRRFilter.h"
#include "oraProjectionGeometry.h"
#include "oraIntensityTransferFunction.h"

// quite large-minded tolerance for DRR pixel intensity outcome comparison
// (on-the-fly vs. off-the-fly ITF mapping); "large-minded" comes from the fact
// that DRR intensities are essentially a sum and internally we use FLOAT for
// persistent ITF mapping (vs. DOUBLE precision)
#define ACCURACY 100.f

/**
 * Tests base functionality of:
 *
 *   ora::GLSLRayCastingDRRFilter
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see oraGLSLRayCastingDRRFilter
 *
 * @author phil
 * @version 1.0
 *
 * \ingroup Tests
 */

int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  helpLines.push_back(
      "  -xo or --extended-output ... extended message output (time measurement)");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, true, false);
    return EXIT_SUCCESS;
  }
  bool ok = true; // OK-flag for whole test

  typedef unsigned short VolumePixelType;
  typedef float DRRPixelType;
  typedef ora::GLSLRayCastingDRRFilter<VolumePixelType, DRRPixelType> DRRFilterType;
  typedef DRRFilterType::InputImageType VolumeImageType;
  typedef DRRFilterType::OutputImageType DRRImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef itk::ExtractImageFilter<DRRImageType, DRR2DImageType> ExtractorType;
  typedef itk::ImageRegionIteratorWithIndex<VolumeImageType> VolumeIteratorType;
  typedef itk::ImageFileWriter<VolumeImageType> VolumeWriterType;
  typedef itk::ImageFileWriter<DRRImageType> DRRWriterType;
  typedef itk::ImageFileWriter<DRR2DImageType> DRR2DWriterType;
  typedef itk::Euler3DTransform<double> TransformType;
  typedef itk::ImageRegionIterator<DRRImageType> IntensityIteratorType;
  typedef itk::ImageRegionIterator<DRRFilterType::MaskImageType>
       MaskIntensityIteratorType;
  typedef ora::IntensityTransferFunction::Pointer ITFPointer;
  typedef ora::ProjectionGeometry::Pointer ProjectionGeometryPointer;
  bool lok = true; // local OK-flag for a test sub-section

  lok = true; // initialize sub-section's success state
  DRRFilterType::Pointer drrFilter = DRRFilterType::New();
  drrFilter->SetNumberOfThreads(1);

  VERBOSE(<< "\nTesting GLSL ray-casting DRR filter functionality.\n")

  VERBOSE(<< "  * Generating input data ... ")
  VolumeImageType::SizeType isize;
  isize[0] = 80;
  isize[1] = 70;
  isize[2] = 30;
  VolumeImageType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  iindex[2] = 0;
  VolumeImageType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  VolumeImageType::SpacingType ispacing;
  ispacing[0] = 0.7;
  ispacing[1] = 0.8;
  ispacing[2] = 1.8;
  VolumeImageType::PointType iorigin;
  iorigin[0] = -(isize[0] * ispacing[0]) / 2.; // centered
  iorigin[1] = -(isize[1] * ispacing[1]) / 2.;
  iorigin[2] = -(isize[2] * ispacing[2]) / 2.;
  VolumeImageType::DirectionType idirection;
  idirection.SetIdentity();
  VolumeImageType::Pointer volume = VolumeImageType::New();
  volume->SetSpacing(ispacing);
  volume->SetOrigin(iorigin);
  volume->SetDirection(idirection);
  volume->SetRegions(iregion);
  volume->Allocate();
  VolumeIteratorType it(volume, iregion);
  VolumePixelType v = 0;
  VolumeImageType::IndexType idx;
  srand(time(NULL));
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    idx = it.GetIndex();
    if (idx[0] > 10 && idx[0] < 70 && idx[1] > 5 && idx[1] < 65 && idx[2] > 2
        && idx[2] < 28)
    {
      v = rand() % 1000 + 1000; // tissue
    }
    else
    {
      v = 0; // air
    }
    it.Set(v);
  }
  if (ImageOutput)
  {
    VolumeWriterType::Pointer w = VolumeWriterType::New();
    w->SetInput(volume);
    w->SetFileName("volume.mhd");
    try
    {
      w->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      lok = false;
    }
    w = NULL;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Generating and applying DRR properties ... ")
  lok = true;
  //TODO
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Computing DRRs (3D and 2D casting) ... ")
  lok = true;
  //TODO
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Independent DRR output checks ... ")
  lok = true;
  //TODO
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing single DRR mask functionality ... ")
  lok = true;
  //TODO
	ok = ok && lok;
	VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

	VERBOSE(<< "  * Testing multiple DRR mask functionality ... ")
	lok = true;
	//TODO
	ok = ok && lok;
	VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

	VERBOSE(<< "  * Testing ITF functionality... ")
	lok = true;
	//TODO
	ok = ok && lok;
	VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  if (drrFilter->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  drrFilter = NULL; // reference counter must be zero!

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
