//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>

#include <itkImage.h>
#include <itkVersorRigid3DTransform.h>

#include "oraProjectionProperties.h"

#include "BasicUnitTestIncludes.hxx"

/**
 * Tests base functionality of:
 *
 *   ora::ProjectionProperties
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::ProjectionProperties
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

  VERBOSE(<< "\nTesting projection properties.\n")
  bool ok = true;

  const unsigned int Dimension = 3;
  typedef float PixelType;
  typedef itk::Image<PixelType, Dimension> ImageType;
  typedef ora::ProjectionProperties<PixelType> ProjPropsType;
  typedef ProjPropsType::MatrixType OrientationType;
  typedef ProjPropsType::PointType PositionType;
  typedef ProjPropsType::SizeType SizeType;
  typedef ProjPropsType::SpacingType SpacingType;
  typedef ProjPropsType::TransferFunctionSpecificationType ITFSpecType;
  typedef ProjPropsType::TransferFunctionPointer ITFPointer;
  typedef itk::VersorRigid3DTransform<double> TransformType;
  typedef ProjPropsType::Spacing3DType Spacing3DType;
  typedef ProjPropsType::MaskImageType MaskImageType;

  ProjPropsType::Pointer props = ProjPropsType::New();

  VERBOSE(<< "  * Manual geometry setup ... ")
  if (!props->IsGeometryValid()) // must initially be invalid
  {
    OrientationType orient;
    orient[0][0] = 1.;
    orient[0][1] = 0.;
    orient[0][2] = 0.; // invalid
    orient[1][0] = 0.;
    orient[1][1] = 1.;
    orient[1][2] = 0.;
    orient[2][0] = -1.;
    orient[2][1] = 0.;
    orient[2][2] = 0.;
    props->SetProjectionPlaneOrientation(orient);
    PositionType origin;
    origin[0] = -100;
    origin[1] = -100;
    origin[2] = -500;
    props->SetProjectionPlaneOrigin(origin);
    SizeType size;
    size[0] = 0;
    size[1] = 800; // invalid
    props->SetProjectionSize(size);
    SpacingType spacing;
    spacing[0] = 0.5;
    spacing[1] = -1.0; // invalid
    props->SetProjectionSpacing(spacing);
    PositionType focalSpot;
    focalSpot[0] = 0;
    focalSpot[1] = 0;
    focalSpot[2] = -500; // invalid
    props->SetSourceFocalSpotPosition(focalSpot);

    // incremental adjustment ...
    if (props->IsGeometryValid()) // too many invalid settings
      ok = false;
    orient[2][0] = 0.;
    orient[2][1] = 0.;
    orient[2][2] = 1.; // valid
    props->SetProjectionPlaneOrientation(orient);
    if (props->IsGeometryValid()) // too many invalid settings
      ok = false;
    size[0] = 400; // valid
    props->SetProjectionSize(size);
    if (props->IsGeometryValid()) // too many invalid settings
      ok = false;
    spacing[1] = 0.25; // valid
    props->SetProjectionSpacing(spacing);
    if (props->IsGeometryValid()) // too many invalid settings
      ok = false;
    focalSpot[2] = 1500; // valid
    props->SetSourceFocalSpotPosition(focalSpot);
    if (!props->IsGeometryValid()) // geometry globally OK!
      ok = false;
    if (props->AreAllPropertiesValid()) // but not all props OK!
      ok = false;
  }
  else
    ok = false;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  if (ok)
  {
    VERBOSE(<< "  * Manual ray-casting props setup ... ")
    if (!props->AreRayCastingPropertiesValid()) // must initially be invalid!
    {
      float sdist = 0;
      props->SetSamplingDistance(sdist);
      ITFSpecType itfspec;
      itfspec.SetSize(2);
      itfspec[0] = -200;
      itfspec[1] = 0; // invalid
      props->SetIntensityTransferFunctionFromNodePairs(itfspec);
      if (props->AreRayCastingPropertiesValid()) // too many invalid settings
        ok = false;
      sdist = 0.5; // valid
      props->SetSamplingDistance(sdist);
      if (props->AreRayCastingPropertiesValid()) // too many invalid settings
        ok = false;
      itfspec.SetSize(3);
      itfspec[0] = -200;
      itfspec[1] = 0; // invalid
      itfspec[2] = 500;
      props->SetIntensityTransferFunctionFromNodePairs(itfspec);
      if (props->AreRayCastingPropertiesValid()) // too many invalid settings
        ok = false;
      itfspec.SetSize(4);
      itfspec[0] = -200;
      itfspec[1] = 0; // valid
      itfspec[2] = 500;
      itfspec[3] = 1.0;
      props->SetIntensityTransferFunctionFromNodePairs(itfspec);
      if (!props->AreRayCastingPropertiesValid()) // valid ray-cast-props
        ok = false;
      itfspec.SetSize(6);
      itfspec[0] = -200;
      itfspec[1] = -0.5; // valid (!)
      itfspec[2] = 500;
      itfspec[3] = 0.3;
      itfspec[4] = 2500;
      itfspec[5] = 0.9;
      props->SetIntensityTransferFunctionFromNodePairs(itfspec);
      if (!props->AreRayCastingPropertiesValid()) // valid ray-cast-props
        ok = false;
      itfspec[0] = -200;
      itfspec[1] = -20; // valid (!)
      itfspec[2] = 500;
      itfspec[3] = 25;
      itfspec[4] = 2500;
      itfspec[5] = 400;
      props->SetIntensityTransferFunctionFromNodePairs(itfspec);
      if (!props->AreRayCastingPropertiesValid()) // valid ray-cast-props
        ok = false;
      // check whether implicit rescaling / clamping worked:
      ITFPointer itf = props->GetITF();
      if (itf)
      {
        if (itf->GetSize() != 3)
          ok = false;
        for (int i = 0; i < 3; i++)
        {
          double x[6];
          itf->GetNodeValue(i, x);
          if (x[1] < 0. || x[1] > 1.)
          {
            ok = false;
            break;
          }
          if (i == 0 && fabs(x[1] - 0.0 / 400.0) > 1e-5)
            ok = false;
          else if (i == 1 && fabs(x[1] - 25.0 / 400.0) > 1e-5)
            ok = false;
          else if (i == 2 && fabs(x[1] - 400.0 / 400.0) > 1e-5)
            ok = false;
        }

        props->SetITF(NULL);
        if (props->AreRayCastingPropertiesValid()) // invalid ray-cast-props
          ok = false;
        itf = NULL;
        itf = ITFPointer::New();
        itf->RemoveAllPoints();
        itf->AddRGBPoint(-200, -20, 0, 0); // invalid (direct ITF must scale)
        itf->AddRGBPoint(500, 25, 0, 0);
        itf->AddRGBPoint(2500, 400, 0, 0);
        props->SetITF(itf);
        if (props->AreRayCastingPropertiesValid()) // invalid ray-cast-props
          ok = false;

        itf->RemoveAllPoints();
        itf->AddRGBPoint(-200, 0, 0, 0); // valid
        itf->AddRGBPoint(500, 0.12, 0, 0);
        itf->AddRGBPoint(2500, 0.9, 0, 0);
        props->SetITF(itf);
        if (!props->AreRayCastingPropertiesValid()) // valid ray-cast-props
          ok = false;
      }
      else
        ok = false;
    }
    else
      ok = false;
    VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")
  }

  if (ok)
  {
    VERBOSE(<< "  * Manual mask props ... ")
    if (!props->AreMaskPropertiesValid()) // no mask -> valid
      ok = false;
    MaskImageType::Pointer mask = MaskImageType::New();
    MaskImageType::RegionType maskReg;
    MaskImageType::IndexType maskIdx;
    maskIdx.Fill(0);
    MaskImageType::SizeType maskSz;
    maskSz[0] = 12; // invalid!
    maskSz[1] = 24;
    maskSz[2] = 1;
    maskReg.SetIndex(maskIdx);
    maskReg.SetSize(maskSz);
    mask->SetRegions(maskReg);
    mask->Allocate();
    props->SetDRRMask(mask); // INVALID MASK SIZE!
    if (props->AreMaskPropertiesValid() || props->AreAllPropertiesValid())
      ok = false;
    props->SetDRRMask(NULL);
    if (!props->AreMaskPropertiesValid()) // no mask, valid again
      ok = false;
    maskSz[0] = props->GetProjectionSize()[0]; // redefine with right size
    maskSz[1] = props->GetProjectionSize()[1];
    maskReg.SetSize(maskSz);
    mask->SetRegions(maskReg);
    mask->Allocate();
    props->SetDRRMask(mask);
    if (!props->AreMaskPropertiesValid()) // valid mask size!
      ok = false;
    VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")
  }

  if (ok)
  {
    VERBOSE(<< "  * Manual global props ... ")
    if (!props->AreAllPropertiesValid()) // valid global props
      ok = false;
    VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")
  }

  VERBOSE(<< "  * String conversion check ... ")
  std::ostringstream os;
  os.str("");
  try
  {
    os << props;
  }
  catch (...)
  {
    ok = false;
  }
  if (os.str().length() <= 0)
    ok = false;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  if (ok)
  {
    props = NULL;
    props = ProjPropsType::New();
    VERBOSE(<< "  * Automatic geometry setup ... ")
    if (!props->IsGeometryValid())
    {
      // fixed image that defines geometry (nearly completely)
      ImageType::SizeType isize;
      isize[0] = 123;
      isize[1] = 259;
      isize[2] = 1;
      ImageType::IndexType iindex;
      iindex[0] = 5;
      iindex[1] = 10;
      iindex[2] = 0;
      ImageType::RegionType iregion;
      iregion.SetIndex(iindex);
      iregion.SetSize(isize);
      ImageType::SpacingType ispacing;
      ispacing[0] = 1.11;
      ispacing[1] = 0.23;
      ispacing[2] = 1.0;
      ImageType::PointType iorigin;
      iorigin[0] = -100.4;
      iorigin[1] = -90.75;
      iorigin[2] = -500;
      ImageType::DirectionType idirection;
      TransformType::Pointer t = TransformType::New();
      TransformType::AxisType axis;
      axis[0] = 1;
      axis[1] = 1;
      axis[2] = 1;
      t->SetRotation(axis, -0.26179938779914943654); // -15°
      idirection = t->GetMatrix();
      ImageType::Pointer refImage = ImageType::New();
      refImage->SetSpacing(ispacing);
      refImage->SetOrigin(iorigin);
      refImage->SetDirection(idirection);
      refImage->SetRegions(iregion);
      refImage->Allocate();
      PositionType fs;
      fs[0] = 0;
      fs[1] = 0;
      fs[2] = 1500;
      props->SetSourceFocalSpotPosition(fs);

      props->SetGeometryFromFixedImage(refImage, iregion);
      if (!props->IsGeometryValid()) // complete
        ok = false;
    }
    else
      ok = false;
    VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")
  }

  if (ok)
  {
    VERBOSE(<< "  * Automatic ray-casting props setup ... ")
    ITFPointer itf = ITFPointer::New();
    itf->RemoveAllPoints();
    itf->AddRGBPoint(-200, 0, 0, 0); // some valid ITF
    itf->AddRGBPoint(500, 0.12, 0, 0);
    itf->AddRGBPoint(2500, 0.9, 0, 0);
    props->SetITF(itf);
    if (props->AreRayCastingPropertiesValid()) // invalid ray-cast-props
      ok = false;

    Spacing3DType volSpac;
    volSpac[0] = 1.0;
    volSpac[1] = 2.0;
    volSpac[2] = 0; // invalid
    props->ComputeAndSetSamplingDistanceFromVolume(volSpac, 0);
    if (props->AreRayCastingPropertiesValid()) // invalid ray-cast-props
      ok = false;
    volSpac[2] = 3.0; // valid spacing
    // 0 ... half of smallest spacing component (Shannon theorem)
    props->ComputeAndSetSamplingDistanceFromVolume(volSpac, 0);
    float sdist = props->GetSamplingDistance();
    if (fabs(sdist - 0.5f) > 1e-5)
      ok = false;
    // 1 ... smallest spacing component (empirical, but usually enough)
    props->ComputeAndSetSamplingDistanceFromVolume(volSpac, 1);
    sdist = props->GetSamplingDistance();
    if (fabs(sdist - 1.0f) > 1e-5)
      ok = false;
    // 2 ... largest spacing component (empirical, usually low quality)
    props->ComputeAndSetSamplingDistanceFromVolume(volSpac, 2);
    sdist = props->GetSamplingDistance();
    if (fabs(sdist - 3.0f) > 1e-5)
      ok = false;
    // 3 ... half of largest spacing component (sparse Shannon theorem)
    props->ComputeAndSetSamplingDistanceFromVolume(volSpac, 3);
    sdist = props->GetSamplingDistance();
    if (fabs(sdist - 1.5f) > 1e-5)
      ok = false;
    if (!props->AreRayCastingPropertiesValid()) // valid ray-cast-props
      ok = false;
    VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")
  }

  if (ok)
  {
    VERBOSE(<< "  * Automatic global props ... ")
    if (!props->AreAllPropertiesValid()) // valid global props
      ok = false;
    VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")
  }

  VERBOSE(<< "  * Final reference count check ... ")
  if (props && props->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  props = NULL; // reference counter must be zero!

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

