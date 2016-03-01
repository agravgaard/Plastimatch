//C
#include <math.h>
#include <cstddef> /* Workaround bug in ITK 3.20 */
//
#include "BasicUnitTestIncludes.hxx"

#include "oraImageBasedProjectionGeometry.h"
//ITK
#include <vnl/vnl_matrix.h>
#include <itkMatrix.h>
#include <itkImage.h>
#include <itkImageFileWriter.h>


#define EPSILON 1e-6

/**
 * Tests base functionality of:
 *
 *   ora::ImageBasedProjectionGeometry.
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::ImageBasedProjectionGeometry
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
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL, helpLines, true);
    return EXIT_SUCCESS;
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting image-based projection geometry interface.\n")

  VERBOSE(<< "  * Without source computation ... ")
  lok = true; // initialize sub-section's success state
  typedef float XrayPixelType;
  typedef itk::Image<XrayPixelType, 3> ImageType;
  typedef ImageType::PointType PointType;
  typedef ImageType::SpacingType SpacingType;
  typedef ImageType::SizeType SizeType;
  typedef ImageType::DirectionType DirectionType;
  typedef ImageType::RegionType RegionType;
  typedef ImageType::IndexType IndexType;

  // create an image:
  ImageType::Pointer image = ImageType::New();
  RegionType ireg;
  IndexType idx;
  idx.Fill(0);
  SizeType sz;
  sz[0] = 120;
  sz[1] = 200;
  sz[2] = 1;
  ireg.SetIndex(idx);
  ireg.SetSize(sz);
  image->SetRegions(ireg);
  SpacingType spac;
  spac[0] = 0.5;
  spac[1] = 2.0;
  spac[2] = 1.0;
  image->SetSpacing(spac);
  DirectionType dir;
  dir[0][0] = 0; // row direction
  dir[1][0] = 0;
  dir[2][0] = -1;
  dir[0][1] = 0; // column direction
  dir[1][1] = 1;
  dir[2][1] = 0;
  double r[3];
  r[0] = dir[0][0];
  r[1] = dir[1][0];
  r[2] = dir[2][0];
  double c[3];
  c[0] = dir[0][1];
  c[1] = dir[1][1];
  c[2] = dir[2][1];
  dir[0][2] = r[1] * c[2] - r[2] * c[1]; // slicing direction
  dir[1][2] = r[2] * c[0] - r[0] * c[2];
  dir[2][2] = r[0] * c[1] - r[1] * c[0];
  image->SetDirection(dir);
  PointType orig;
  orig[0] = -400;
  orig[1] = -100;
  orig[2] = -50;
  image->SetOrigin(orig);
  image->Allocate(); // generate
  image->FillBuffer(100);

  if (ImageOutput) // write out image
  {
    typedef itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer w = WriterType::New();
    w->SetFileName("orientation_image.mhd");
    w->SetInput(image);
    try
    {
      w->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      lok = false;
    }
  }

  typedef ora::ImageBasedProjectionGeometry<XrayPixelType> GeometryType;
  GeometryType::Pointer geom = GeometryType::New();
  std::ostringstream os;
  geom->Print(os, 0);
  if (os.str().length() <= 0)
    lok = false;
  if (geom->IsGeometryValid())
    lok = false;

  if (!geom->ExtractDetectorGeometryFromImage(image))
    lok = false;
  const double *gorig = geom->GetDetectorOrigin();
  if (gorig[0] != orig[0] || gorig[1] != orig[1] || gorig[2] != orig[2])
    lok = false;
  const double *gs = geom->GetDetectorPixelSpacing();
  if (gs[0] != spac[0] || gs[1] != spac[1])
    lok = false;
  const int *gsz = geom->GetDetectorSize();
  if (gsz[0] != (int)sz[0] || gsz[1] != (int)sz[1])
    lok = false;
  const double *gr = geom->GetDetectorRowOrientation();
  if (gr[0] != r[0] || gr[1] != r[1] || gr[2] != r[2])
    lok = false;
  const double *gc = geom->GetDetectorColumnOrientation();
  if (gc[0] != c[0] || gc[1] != c[1] || gc[2] != c[2])
    lok = false;
  double S[3]; // set source position
  S[0] = 500;
  S[1] = 300;
  S[2] = -5;
  geom->SetSourcePosition(S);
  if (!geom->IsGeometryValid()) // should be OK now!
    lok = false;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Without source computation, but with image region ... ")
  lok = true; // initialize sub-section's success state
  GeometryType::Pointer geom2 = GeometryType::New();
  ImageType::RegionType reg = image->GetLargestPossibleRegion();
  reg.SetIndex(0, 30);
  reg.SetIndex(1, 25);
  reg.SetIndex(2, 1); // invalid index
  reg.SetSize(0, 70);
  reg.SetSize(1, 110);
  reg.SetSize(2, 1);
  if (geom2->ExtractDetectorGeometryFromImageAndRegion(image, reg))
    lok = false;
  reg.SetIndex(0, 30);
  reg.SetIndex(1, 25);
  reg.SetIndex(2, 0);
  reg.SetSize(0, 70);
  reg.SetSize(1, 110);
  reg.SetSize(2, 0); // invalid size
  if (geom2->ExtractDetectorGeometryFromImageAndRegion(image, reg))
    lok = false;
  reg.SetIndex(0, 30);
  reg.SetIndex(1, 25);
  reg.SetIndex(2, 0);
  reg.SetSize(0, 70);
  reg.SetSize(1, 110);
  reg.SetSize(2, 1);
  if (!geom2->ExtractDetectorGeometryFromImageAndRegion(image, reg))
    lok = false;
  const double *gs2 = geom2->GetDetectorPixelSpacing();
  if (gs2[0] != spac[0] || gs2[1] != spac[1])
    lok = false;
  const int *gsz2 = geom2->GetDetectorSize();
  sz = reg.GetSize();
  if (gsz2[0] != (int)sz[0] || gsz2[1] != (int)sz[1])
    lok = false;
  const double *gr2 = geom2->GetDetectorRowOrientation();
  if (gr2[0] != r[0] || gr2[1] != r[1] || gr2[2] != r[2])
    lok = false;
  const double *gc2 = geom2->GetDetectorColumnOrientation();
  if (gc2[0] != c[0] || gc2[1] != c[1] || gc2[2] != c[2])
    lok = false;
  PointType orig2;
  for (int d = 0; d < 3; d++)
  	orig2[d] = orig[d] +
  		static_cast<double>(reg.GetIndex()[0]) * gs2[0] * gr2[d] +
  		static_cast<double>(reg.GetIndex()[1]) * gs2[1] * gc2[d];
  const double *gorig2 = geom2->GetDetectorOrigin();
  if (gorig2[0] != orig2[0] || gorig2[1] != orig2[1] || gorig2[2] != orig2[2])
    lok = false;
  geom2->SetSourcePosition(S);
  if (!geom->IsGeometryValid()) // should be OK now!
    lok = false;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * With source computation ... ")
  lok = true; // initialize sub-section's success state
  dir[0][0] = 1; // row direction
  dir[1][0] = 0;
  dir[2][0] = 0;
  dir[0][1] = 0; // column direction
  dir[1][1] = 1;
  dir[2][1] = 0;
  dir[0][2] = 0; // slicing direction
  dir[1][2] = 0;
  dir[2][2] = 1;
  image->SetDirection(dir);
  orig[0] = -50;
  orig[1] = -70;
  orig[2] = -200;
  image->SetOrigin(orig);

  if (ImageOutput) // write out image
  {
    typedef itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer w = WriterType::New();
    w->SetFileName("orientation_image2.mhd");
    w->SetInput(image);
    try
    {
      w->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      lok = false;
    }
  }

  if (!geom->ExtractDetectorGeometryFromImageAndAssumeSourcePosition(image,
        50, 70, 1000))
    lok = false;
  const double *Sn = geom->GetSourcePosition();
  if (fabs(Sn[0]) > EPSILON || fabs(Sn[1]) > EPSILON || fabs(Sn[2] - 800) > EPSILON)
    lok = false;
  if (!geom->IsGeometryValid()) // should be OK
    lok = false;

  if (!geom->ExtractDetectorGeometryFromImageAndAssumeSourcePosition(image,
        40, 90, 1000, true))
    lok = false;
  const double *Sn2 = geom->GetSourcePosition();
  if (fabs(Sn2[0] + 10) > EPSILON || fabs(Sn2[1] - 20) > EPSILON || fabs(Sn2[2] + 1200) > EPSILON)
    lok = false;
  if (!geom->IsGeometryValid()) // should be OK
    lok = false;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * With source computation and region ... ")
  lok = true; // initialize sub-section's success state
  GeometryType::Pointer geom3 = GeometryType::New();
  if (!geom3->ExtractDetectorGeometryFromImageAndRegionAndAssumeSourcePosition(
  			image, reg, 50, 70, 1000))
    lok = false;
  Sn = geom3->GetSourcePosition();
  if (fabs(Sn[0]) > EPSILON || fabs(Sn[1]) > EPSILON || fabs(Sn[2] - 800) > EPSILON)
    lok = false;
  if (!geom3->IsGeometryValid()) // should be OK
    lok = false;

  if (!geom3->ExtractDetectorGeometryFromImageAndRegionAndAssumeSourcePosition(
  			image, reg, 40, 90, 1000, true))
    lok = false;
  Sn2 = geom3->GetSourcePosition();
  if (fabs(Sn2[0] + 10) > EPSILON || fabs(Sn2[1] - 20) > EPSILON || fabs(Sn2[2] + 1200) > EPSILON)
    lok = false;
  if (!geom3->IsGeometryValid()) // should be OK
    lok = false;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final check ... ")
  lok = true; // initialize sub-section's success state
  if (!geom || geom->GetReferenceCount() != 1)
    lok = false;
  geom = NULL;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

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
