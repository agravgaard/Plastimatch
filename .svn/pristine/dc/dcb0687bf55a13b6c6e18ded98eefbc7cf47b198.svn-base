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

#include "oraCPUSiddonDRRFilter.h"
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
 *   ora::CPUSiddonDRRFilter
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see oraCPUSiddonDRRFilter.h
 * @see	oraCPUSiddonDRRFilter.txx
 *
 * @author jeanluc
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
  typedef ora::CPUSiddonDRRFilter<VolumePixelType, DRRPixelType> DRRFilterType;
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

  VERBOSE(<< "\nTesting Siddon CPU DRR Filter Functionality.\n")

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
  drrFilter->SetNumberOfIndependentOutputs(2);
  // ITF:  0,0.0 500,0.05 1001,0.2 1200,0.3 1201,0.3 2500,1.0 3000,1.0
  ITFPointer itf = ora::IntensityTransferFunction::New();
  itf->AddSupportingPoint(0, 0);
  itf->AddSupportingPoint(500, 0.05);
  itf->AddSupportingPoint(1001, 0.2);
  itf->AddSupportingPoint(1200, 0.3);
  itf->AddSupportingPoint(1201, 0.3);
  itf->AddSupportingPoint(2500, 1.0);
  itf->AddSupportingPoint(3000, 1.0);
  drrFilter->SetITF(itf);
  // Generate Projection Geometry
  ProjectionGeometryPointer geom = ora::ProjectionGeometry::New();
  // set detector orientation
  double detectorRowOrientation[3];
  detectorRowOrientation[0] = 1;
  detectorRowOrientation[1] = 0;
  detectorRowOrientation[2] = 0;
  double detectorColumnOrientation[3];
  detectorColumnOrientation[0] = 0;
  detectorColumnOrientation[1] = 1;
  detectorColumnOrientation[2] = 0;
  TransformType::Pointer thelp = TransformType::New();
  TransformType::ParametersType parameters(6);
  parameters.Fill(0);
  parameters[0] = 0.05;
  parameters[1] = 0.03;
  parameters[2] = -0.035;
  thelp->SetParameters(parameters);
  TransformType::InputPointType p;
  p[0] = detectorRowOrientation[0];
  p[1] = detectorRowOrientation[1];
  p[2] = detectorRowOrientation[2];
  p = thelp->TransformPoint(p);
  detectorRowOrientation[0] = p[0];
  detectorRowOrientation[1] = p[1];
  detectorRowOrientation[2] = p[2];
  p[0] = detectorColumnOrientation[0];
  p[1] = detectorColumnOrientation[1];
  p[2] = detectorColumnOrientation[2];
  p = thelp->TransformPoint(p);
  detectorColumnOrientation[0] = p[0];
  detectorColumnOrientation[1] = p[1];
  detectorColumnOrientation[2] = p[2];
  thelp = 0;
  geom->SetDetectorRowOrientation(detectorRowOrientation);
  geom->SetDetectorColumnOrientation(detectorColumnOrientation);
  // set drr Origin
  double drrOrigin[3];
  drrOrigin[0] = -100;
  drrOrigin[1] = -80;
  drrOrigin[2] = -150;
  geom->SetDetectorOrigin(drrOrigin);
  // set drr Detector size
  int drrSize[2];
  drrSize[0] = 200;
  drrSize[1] = 160;
  geom->SetDetectorSize(drrSize);
  // set drr spacing
  double drrSpacing[2];
  drrSpacing[0] = 1.0;
  drrSpacing[1] = 1.0;
  geom->SetDetectorPixelSpacing(drrSpacing);
  // set source position
  double drrFocalSpot[3];
  drrFocalSpot[0] = 100;
  drrFocalSpot[1] = 10;
  drrFocalSpot[2] = 800;
  geom->SetSourcePosition(drrFocalSpot);
  // set Projection Geometry
  drrFilter->SetCurrentDRROutputIndex(0);
  drrFilter->SetProjectionGeometry(0, geom);
  // set Transform
  TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();
  drrFilter->SetTransform(transform);
  drrFilter->SetInput(volume);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Computing DRRs (3D and 2D casting) ... ")
  lok = true;
  ExtractorType::Pointer extractFilter = ExtractorType::New();
  extractFilter->SetInput(drrFilter->GetOutput());
  for (unsigned int i = 0; i < 10; i++)
  {
	if(i > 0)
	{
      TransformType::ParametersType pars(6);
      pars.Fill(0);
      pars[0] += ((double) (rand() % 101 - 50)) / 400.;
      pars[1] += ((double) (rand() % 101 - 50)) / 400.;
      pars[2] += ((double) (rand() % 101 - 50)) / 400.;
      pars[3] += ((double) (rand() % 101 - 50)) / 10.;
      pars[4] += ((double) (rand() % 101 - 50)) / 10.;
      pars[5] += ((double) (rand() % 101 - 50)) / 5.;
      transform->SetParameters(pars);
	}
    try
    {
      drrFilter->Update();
      if (drrFilter->GetOutput())
      {
        if (ImageOutput)
        {
          DRRWriterType::Pointer w = DRRWriterType::New();
          char buff[100];
          sprintf(buff, "drr3D_Siddon_%d.mhd", i);
          w->SetFileName(buff);
          w->SetInput(drrFilter->GetOutput());
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
        // test casting to 2D
        try
        {
          DRRImageType::RegionType drrRegion =
            drrFilter->GetOutput()->GetLargestPossibleRegion();
          DRRImageType::SizeType esize = drrRegion.GetSize();
          esize[2] = 0; // 3D->2D
          DRRImageType::IndexType eindex = drrRegion.GetIndex();
          eindex[2] = 0; // 1st and only slice
          DRRImageType::RegionType extractRegion;
          extractRegion.SetIndex(eindex);
          extractRegion.SetSize(esize);
          extractFilter->SetExtractionRegion(extractRegion);
          extractFilter->Update();
          if (ImageOutput)
          {
            DRR2DWriterType::Pointer w = DRR2DWriterType::New();
            char buff[100];
            sprintf(buff, "drr2D_Siddon%d.mhd", i);
            w->SetFileName(buff);
            w->SetInput(extractFilter->GetOutput());
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
        }
        catch (itk::ExceptionObject &e)
        {
          lok = false;
        }
      }
      else
      {
        lok = false;
      }
    }
    catch (itk::ExceptionObject &e)
    {
      lok = false;
    }
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Independent DRR output checks ... ")
  lok = true;
  drrFilter->SetNumberOfIndependentOutputs(3); // independent outputs > 1
  drrFilter->SetCurrentDRROutputIndex(0);
  if (!drrFilter->GetOutput(0) || !drrFilter->GetOutput(1)
      || !drrFilter->GetOutput(2))
    lok = false;
  // (largest possible region of the un-computed outputs must be 0!)
  DRRImageType::RegionType tregion;
  tregion = drrFilter->GetOutput(0)->GetLargestPossibleRegion();
  if (tregion.GetNumberOfPixels() <= 0)
    lok = false;
  tregion = drrFilter->GetOutput(1)->GetLargestPossibleRegion();
  if (tregion.GetNumberOfPixels() != 0)
    lok = false;
  tregion = drrFilter->GetOutput(2)->GetLargestPossibleRegion();
  if (tregion.GetNumberOfPixels() != 0)
    lok = false;
  drrFilter->SetCurrentDRROutputIndex(1); // compute another output
  drrFilter->SetProjectionGeometry(1, geom);
  try
  {
    drrFilter->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  if (!drrFilter->GetOutput(0) || !drrFilter->GetOutput(1)
      || !drrFilter->GetOutput(2))
    lok = false;
  tregion = drrFilter->GetOutput(0)->GetLargestPossibleRegion();
  if (tregion.GetNumberOfPixels() <= 0)
    lok = false;
  tregion = drrFilter->GetOutput(1)->GetLargestPossibleRegion();
  if (tregion.GetNumberOfPixels() <= 0)
    lok = false;
  tregion = drrFilter->GetOutput(2)->GetLargestPossibleRegion();
  if (tregion.GetNumberOfPixels() != 0)
    lok = false;
  drrFilter->SetCurrentDRROutputIndex(2); // compute another output
  drrFilter->SetProjectionGeometry(2, geom);
  try
  {
    drrFilter->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  if (!drrFilter->GetOutput(0) || !drrFilter->GetOutput(1)
      || !drrFilter->GetOutput(2))
    lok = false;
  tregion = drrFilter->GetOutput(0)->GetLargestPossibleRegion();
  if (tregion.GetNumberOfPixels() <= 0)
    lok = false;
  tregion = drrFilter->GetOutput(1)->GetLargestPossibleRegion();
  if (tregion.GetNumberOfPixels() <= 0)
    lok = false;
  tregion = drrFilter->GetOutput(2)->GetLargestPossibleRegion();
  if (tregion.GetNumberOfPixels() <= 0)
    lok = false;
  drrFilter->SetNumberOfIndependentOutputs(1); // 1 output again
  itk::Object::SetGlobalWarningDisplay(false); // query throws warnings!
  if (!drrFilter->GetOutput(0) || drrFilter->GetOutput(1)
      || drrFilter->GetOutput(2))
    lok = false;
  itk::Object::SetGlobalWarningDisplay(true);
  drrFilter->SetCurrentDRROutputIndex(2); // invalid
  if (drrFilter->GetCurrentDRROutputIndex() != 0)
    lok = false;
  // now check whether the outputs are REALLY independent from each other:
  drrFilter->SetNumberOfIndependentOutputs(3); // 3 independent outputs again
  drrFilter->SetCurrentDRROutputIndex(0); // compute first input (again)
  drrFilter->SetProjectionGeometry(1, geom); //resetting projection geometries
  drrFilter->SetProjectionGeometry(2, geom);
  try
  {
    drrFilter->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  // store image intensities in a reference array:
  tregion = drrFilter->GetOutput(0)->GetLargestPossibleRegion();
  int refArr1Size = tregion.GetNumberOfPixels();
  DRRPixelType *refArr1 = new DRRPixelType[refArr1Size];
  IntensityIteratorType dit1(drrFilter->GetOutput(0), tregion);
  int c = 0;
  for (dit1.GoToBegin(); !dit1.IsAtEnd(); ++dit1)
    refArr1[c++] = dit1.Get();
  // change some DRR-props:
  drrFilter->SetCurrentDRROutputIndex(1); // compute second input
  drrOrigin[0] -= 10;
  drrOrigin[1] += 15;
  drrOrigin[2] -= 20;
  geom->SetDetectorOrigin(drrOrigin);
  drrFilter->SetProjectionGeometry(1, geom);
  try
  {
    drrFilter->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  // first check whether 1st output changed (expected: UNCHANGED):
  drrFilter->GetOutput(0)->Update(); // additionally provoke update!!!
  tregion = drrFilter->GetOutput(0)->GetLargestPossibleRegion();
  int refArr2Size = tregion.GetNumberOfPixels();
  DRRPixelType *refArr2 = new DRRPixelType[refArr2Size];
  IntensityIteratorType dit1a(drrFilter->GetOutput(0), tregion);
  c = 0;
  for (dit1a.GoToBegin(); !dit1a.IsAtEnd(); ++dit1a)
    refArr2[c++] = dit1a.Get();
  if (refArr1Size == refArr2Size)
  {
    for (c = 0; c < refArr1Size; c++)
    {
      if (itk::Math::Round<int, double>(refArr1[c] * ACCURACY) !=
      		itk::Math::Round<int, double>(refArr2[c] * ACCURACY))
      {
        lok = false;
        break;
      }
    }
  }
  else
  {
    lok = false;
  }
  // now check whether 2nd output is different from 1st (expected: CHANGED):
  delete[] refArr2;
  tregion = drrFilter->GetOutput(1)->GetLargestPossibleRegion();
  refArr2Size = tregion.GetNumberOfPixels();
  refArr2 = new DRRPixelType[refArr2Size];
  IntensityIteratorType dit2(drrFilter->GetOutput(1), tregion);
  c = 0;
  for (dit2.GoToBegin(); !dit2.IsAtEnd(); ++dit2)
    refArr2[c++] = dit2.Get();
  if (refArr1Size == refArr2Size)
  {
    int numDiffPixels = 0;
    for (c = 0; c < refArr1Size; c++)
    {
      if (itk::Math::Round<int, double>(refArr1[c] * ACCURACY) !=
      		itk::Math::Round<int, double>(refArr2[c] * ACCURACY))
        numDiffPixels++;
    }
    if (numDiffPixels == 0)
      lok = false;
  }
  else // changed, but not in size expected!
  {
    lok = false;
  }
  // change some DRR-settings:
  drrFilter->SetCurrentDRROutputIndex(2); // compute third input
  ITFPointer itf2 = ora::IntensityTransferFunction::New();
  itf2->AddSupportingPoint(0, 0.05);
  itf2->AddSupportingPoint(500, 0.07);
  itf2->AddSupportingPoint(1001, 0.12);
  itf2->AddSupportingPoint(1200, 0.16);
  itf2->AddSupportingPoint(1201, 0.16);
  itf2->AddSupportingPoint(2500, 0.3);
  itf2->AddSupportingPoint(3000, 0.3);
  drrFilter->SetITF(itf2);
  try
  {
    drrFilter->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    lok = false;
  }
  // first check whether 1st output changed (expected: UNCHANGED):
  drrFilter->GetOutput(0)->Update(); // additionally provoke update!!!
  tregion = drrFilter->GetOutput(0)->GetLargestPossibleRegion();
  int refArr3Size = tregion.GetNumberOfPixels();
  DRRPixelType *refArr3 = new DRRPixelType[refArr3Size];
  IntensityIteratorType dit1b(drrFilter->GetOutput(0), tregion);
  c = 0;
  for (dit1b.GoToBegin(); !dit1b.IsAtEnd(); ++dit1b)
    refArr3[c++] = dit1b.Get();
  if (refArr1Size == refArr3Size)
  {
    for (c = 0; c < refArr1Size; c++)
    {
      if (itk::Math::Round<int, double>(refArr1[c] * ACCURACY) !=
      		itk::Math::Round<int, double>(refArr3[c] * ACCURACY))
      {
        lok = false;
        break;
      }
    }
  }
  else
  {
    lok = false;
  }
  // check whether 2nd output changed (expected: UNCHANGED):
  drrFilter->GetOutput(1)->Update(); // additionally provoke update!!!
  tregion = drrFilter->GetOutput(1)->GetLargestPossibleRegion();
  refArr3Size = tregion.GetNumberOfPixels();
  delete[] refArr3;
  refArr3 = new DRRPixelType[refArr3Size];
  IntensityIteratorType dit1c(drrFilter->GetOutput(1), tregion);
  c = 0;
  for (dit1c.GoToBegin(); !dit1c.IsAtEnd(); ++dit1c)
    refArr3[c++] = dit1c.Get();
  if (refArr2Size == refArr3Size)
  {
    for (c = 0; c < refArr2Size; c++)
    {
      if (itk::Math::Round<int, double>(refArr2[c] * ACCURACY) !=
      		itk::Math::Round<int, double>(refArr3[c] * ACCURACY))
      {
        lok = false;
        break;
      }
    }
  }
  else
  {
    lok = false;
  }
  // now check whether 3rd output is different from 1st/2nd (expected: CHANGED):
  delete[] refArr3;
  tregion = drrFilter->GetOutput(2)->GetLargestPossibleRegion();
  refArr3Size = tregion.GetNumberOfPixels();
  refArr3 = new DRRPixelType[refArr3Size];
  IntensityIteratorType dit3(drrFilter->GetOutput(2), tregion);
  c = 0;
  for (dit3.GoToBegin(); !dit3.IsAtEnd(); ++dit3)
    refArr3[c++] = dit3.Get();
  if (refArr1Size == refArr3Size && refArr2Size == refArr3Size)
  {
    int numDiffPixels = 0;
    for (c = 0; c < refArr3Size; c++)
    {
      if (itk::Math::Round<int, double>(refArr1[c] * ACCURACY) !=
      		itk::Math::Round<int, double>(refArr2[c] * ACCURACY) &&
      		itk::Math::Round<int, double>(refArr1[c] * ACCURACY) !=
      				itk::Math::Round<int, double>(refArr3[c] * ACCURACY))
        numDiffPixels++;
    }
    if (numDiffPixels == 0)
      lok = false;
  }
  else // changed, but not in size expected!
  {
    lok = false;
  }
  delete[] refArr1;
  delete[] refArr2;
  delete[] refArr3;
  itf2 = NULL;
  geom = NULL;
  if (ImageOutput)
  {
    DRRWriterType::Pointer dw = DRRWriterType::New();
    dw->SetFileName("indep_drr_0.mhd");
    dw->SetInput(drrFilter->GetOutput(0));
    try
    {
      dw->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      lok = false;
    }
    dw->SetFileName("indep_drr_1.mhd");
    dw->SetInput(drrFilter->GetOutput(1));
    try
    {
      dw->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      lok = false;
    }
    dw->SetFileName("indep_drr_2.mhd");
    dw->SetInput(drrFilter->GetOutput(2));
    try
    {
      dw->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      lok = false;
    }
    dw = NULL;
  }
  drrFilter->SetNumberOfIndependentOutputs(1);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing single DRR mask functionality ... ")
    lok = true;
    // prepare a few DRR masks:
    geom = drrFilter->GetProjectionGeometry(drrFilter->GetCurrentDRROutputIndex());
    DRRFilterType::SizeType currSz;
    currSz[0] = drrSize[0];
    currSz[1] = drrSize[1];
    drrSize[0] = currSz[0] * 1;
    drrSize[1] = currSz[1] * 1;
    geom->SetDetectorSize(drrSize);
    DRRFilterType::MaskImageType::RegionType maskReg;
    DRRFilterType::MaskImageType::SizeType maskSz;
    maskSz[0] = currSz[0];
    maskSz[1] = currSz[1];
    maskSz[2] = 1;
    DRRFilterType::MaskImageType::SpacingType maskSpac;
    maskSpac.Fill(1.0);
    DRRFilterType::MaskImageType::IndexType maskIdx;
    maskIdx.Fill(0);
    maskReg.SetSize(maskSz);
    maskReg.SetIndex(maskIdx);
    DRRFilterType::MaskImagePointer mask1 = DRRFilterType::MaskImageType::New();
    DRRFilterType::MaskImagePointer mask2 = DRRFilterType::MaskImageType::New();
    DRRFilterType::MaskImagePointer mask3 = DRRFilterType::MaskImageType::New();
    DRRFilterType::MaskImagePointer mask = NULL;
    for (int k = 0; k < 3; ++k)
    {
      if (k == 0)
        mask = mask1;
      else if (k == 1)
        mask = mask2;
      else if (k == 2)
        mask = mask3;

      mask->SetRegions(maskReg);
      mask->SetSpacing(maskSpac);
      mask->Allocate();

      MaskIntensityIteratorType mit(mask, mask->GetLargestPossibleRegion());
      srand(time(NULL));
      int radius2 = static_cast<int> (currSz[1] * 0.4);
      if (currSz[0] < currSz[1])
        radius2 = static_cast<int> (currSz[0] * 0.4);
      radius2 = radius2 * radius2;
      for (mit.GoToBegin(); !mit.IsAtEnd(); ++mit)
      {
        if (k == 0)
        {
          if (rand() % 101 < 50)
            mit.Set(0);
          else
            mit.Set(100);
        }
        else if (k == 1)
        {
          if (rand() % 101 < 70)
            mit.Set(0);
          else
            mit.Set(100);
        }
        else if (k == 2)
        {
          DRRFilterType::MaskImageType::IndexType idx = mit.GetIndex();
          int x = idx[0] - currSz[0] / 2;
          int y = idx[1] - currSz[1] / 2;
          if ((x * x + y * y) <= radius2)
            mit.Set(255);
          else
            mit.Set(0);
        }
      }
      if (ImageOutput)
      {
        typedef itk::ImageFileWriter<DRRFilterType::MaskImageType> MaskWriterType;
        MaskWriterType::Pointer dw = MaskWriterType::New();
        char sbuff[100];
        sprintf(sbuff, "mask%d.mhd", k);
        dw->SetFileName(sbuff);
        dw->SetInput(mask);
        try
        {
          dw->Update();
        }
        catch (itk::ExceptionObject &e)
        {
          lok = false;
        }
      }
    }
    // now simply generate DRRs with alternating masks:
    drrFilter->SetNumberOfIndependentOutputs(1);
    const double MAX_INTENSITY_TOLERANCE = 0.5; // in %
    for (int k = 0; k < 6; k++)
    {
      const double *currOrig = drrFilter->GetProjectionGeometry(0)->GetDetectorOrigin();
      drrOrigin[0] = currOrig[0];
      drrOrigin[1] = currOrig[1];
      if (k == 3)
      {
        drrOrigin[0] -= 30;
        drrOrigin[1] -= 30;
        drrFilter->GetProjectionGeometry(0)->SetDetectorOrigin(drrOrigin);
      }

      drrFilter->SetCurrentDRROutputIndex(0);
      if (k % 3 == 0)
        mask = mask1;
      else if (k % 3 == 1)
        mask = mask2;
      else if (k % 3 == 2)
        mask = mask3;
      if (k >= 6)
        mask = NULL;

      drrFilter->SetDRRMask(0,mask);

      drrOrigin[0] += 5;
      drrOrigin[1] += 5;
      drrFilter->GetProjectionGeometry(0)->SetDetectorOrigin(drrOrigin);

      try
      {
        drrFilter->Update();
      }
      catch (itk::ExceptionObject &e)
      {
        lok = false;
      }
      if (ImageOutput)
      {
        DRRWriterType::Pointer dw = DRRWriterType::New();
        char sbuff[100];
        sprintf(sbuff, "indep_masked_drr_single_%d.mhd", k);
        dw->SetFileName(sbuff);
        dw->SetInput(drrFilter->GetOutput(0));
        try
        {
          dw->Update();
        }
        catch (itk::ExceptionObject &e)
        {
          lok = false;
        }
      }
      if (mask)
      {
        // check whether the masked region is really intensityless
        MaskIntensityIteratorType mit2(mask, mask->GetLargestPossibleRegion());
        DRRImageType::Pointer currDRR = drrFilter->GetOutput(0);
        double me = -1;
        int ec = 0;
        for (mit2.GoToBegin(); !mit2.IsAtEnd(); ++mit2)
        {
          if (mit2.Get() == 0)
          {
            DRRFilterType::MaskImageType::IndexType idx = mit2.GetIndex();
            DRRImageType::PixelType vv = currDRR->GetPixel(idx);
            double e = fabs(vv)  * 100.;
            if (e > me)
              me = e;
            if (e > MAX_INTENSITY_TOLERANCE)
            {
              ec++;
              lok = false; // must not have intensity different from 0!!!
            }
          }
        }
      }
    }
    ok = ok && lok;
    VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

    VERBOSE(<< "  * Testing multiple DRR mask functionality ... ")
    lok = true;
    const double *currOrig = drrFilter->GetProjectionGeometry(0)->GetDetectorOrigin();
    drrOrigin[0] = currOrig[0];
    drrOrigin[1] = currOrig[1];
    drrOrigin[2] = currOrig[2];
    drrOrigin[0] -= 30;
    drrOrigin[1] -= 30;
    drrOrigin[2] -= 100;
    drrFilter->GetProjectionGeometry(0)->SetDetectorOrigin(drrOrigin);
    drrFilter->SetNumberOfIndependentOutputs(3);
    drrFilter->SetProjectionGeometry(1, geom);
    drrFilter->SetProjectionGeometry(2, geom);
    for (int i = 0; i < 6; i++)
    {
      drrFilter->SetCurrentDRROutputIndex(i % 3);
      drrOrigin[0] += 5;
      drrOrigin[1] += 5;
      drrFilter->GetProjectionGeometry(i % 3)->SetDetectorOrigin(drrOrigin);
      if (i % 3 == 0)
        mask = mask1;
      else if (i % 3 == 1)
        mask = mask2;
      else if (i % 3 == 2)
        mask = mask3;
      drrFilter->SetDRRMask(i% 3, mask);

      try
      {
        drrFilter->Update();
      }
      catch (itk::ExceptionObject &e)
      {
        lok = false;
      }
      if (ImageOutput)
      {
        DRRWriterType::Pointer dw = DRRWriterType::New();
        char sbuff[100];
        sprintf(sbuff, "indep_masked_drr_multi_%d.mhd", i);
        dw->SetFileName(sbuff);
        dw->SetInput(drrFilter->GetOutput(i % 3));
        try
        {
          dw->Update();
        }
        catch (itk::ExceptionObject &e)
        {
          lok = false;
        }
      }
      // check whether the masked region is really intensityless
      MaskIntensityIteratorType mit2(mask, mask->GetLargestPossibleRegion());
      DRRImageType::Pointer currDRR = drrFilter->GetOutput(i % 3);
      for (mit2.GoToBegin(); !mit2.IsAtEnd(); ++mit2)
      {
        if (mit2.Get() == 0)
        {
          DRRFilterType::MaskImageType::IndexType idx = mit2.GetIndex();
          if (currDRR->GetPixel(idx) > 1e-6 || currDRR->GetPixel(idx) < -1e-6)
          {
            lok = false; // must not have intensity different from 0!!!
            break;
          }
        }
      }
      if (i == 2) // another position after first runs
      {
        drrOrigin[0] -= 30;
        drrOrigin[1] -= 30;
        geom->SetDetectorOrigin(drrOrigin);
      }
    }
    drrFilter->SetCurrentDRROutputIndex(0);
    drrFilter->SetNumberOfIndependentOutputs(1);
    drrFilter->SetDRRMask(0, NULL);
    ok = ok && lok;
    VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")


    VERBOSE(<< "  * Testing ITF functionality... ")
    lok = true;
    drrFilter->SetITF(NULL);
    ITFPointer itf3 = ora::IntensityTransferFunction::New();
    itf3->AddSupportingPoint(0, 0.05);
    itf3->AddSupportingPoint(500, 0.07);
    itf3->AddSupportingPoint(1001, 0.12);
    itf3->AddSupportingPoint(1200, 0.16);
    itf3->AddSupportingPoint(1201, 0.16);
    itf3->AddSupportingPoint(2500, 0.3);
    itf3->AddSupportingPoint(3000, 0.3);
    drrFilter->SetITF(itf3);
    drrFilter->Update();
    if (ImageOutput)
    {
      DRRWriterType::Pointer w = DRRWriterType::New();
      w->SetFileName("off_the_fly_reference.mhd");
      w->SetInput(drrFilter->GetOutput());
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

    // store image intensities in a reference array:
    tregion = drrFilter->GetOutput(0)->GetLargestPossibleRegion();
    refArr1Size = tregion.GetNumberOfPixels();
    refArr1 = new DRRPixelType[refArr1Size];
    IntensityIteratorType dit4(drrFilter->GetOutput(0), tregion);
    c = 0;
    for (dit4.GoToBegin(); !dit4.IsAtEnd(); ++dit4)
      refArr1[c++] = dit4.Get();

    //enable global ITF Calculation and check if changed
    drrFilter->SetOffTheFlyITFMapping(true);
    drrFilter->Update();
    if (ImageOutput)
    {
      DRRWriterType::Pointer w = DRRWriterType::New();
      w->SetFileName("off_the_fly_image.mhd");
      w->SetInput(drrFilter->GetOutput());
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
    tregion = drrFilter->GetOutput(0)->GetLargestPossibleRegion();
    refArr2Size = tregion.GetNumberOfPixels();
    refArr2 = new DRRPixelType[refArr2Size];
    IntensityIteratorType dit5(drrFilter->GetOutput(0), tregion);
    c = 0;
    for (dit5.GoToBegin(); !dit5.IsAtEnd(); ++dit5)
      refArr2[c++] = dit5.Get();
    //compare output with previous output (no change expected)
    if (refArr1Size == refArr2Size)
    {
      for (c = 0; c < refArr1Size; c++)
      {
        if (itk::Math::Round<int, double>(refArr1[c] * ACCURACY) !=
        		itk::Math::Round<int, double>(refArr2[c] * ACCURACY))
        {
        	VERBOSE(<< " [test 1: " << refArr1[c] << " vs. " << refArr2[c] << "] ")
          lok = false;
          break;
        }
      }
    }
    else
    {
      lok = false;
    }

    //Generate new volume
    VolumeImageType::Pointer volume2 = VolumeImageType::New();
    volume2->SetSpacing(ispacing);
    volume2->SetOrigin(iorigin);
    volume2->SetDirection(idirection);
    VolumeImageType::RegionType iregion2;
    iregion2.SetIndex(iindex);
    iregion2.SetSize(isize);
    volume2->SetRegions(iregion2);
    volume2->Allocate();
    VolumeIteratorType it2(volume2, iregion2);
    VolumeImageType::IndexType idx1;
    srand(time(NULL));
    for (it2.GoToBegin(); !it2.IsAtEnd(); ++it2)
    {
      idx1 = it2.GetIndex();
      if (idx1[0] > 10 && idx1[0] < 70 && idx1[1] > 5 && idx1[1] < 65 && idx1[2] > 2
          && idx1[2] < 28)
      {
    	  v = 0; // air
      }
      else
      {
    	v = rand() % 1000 + 1000;// tissue
      }
      it2.Set(v);
    }
    drrFilter->SetInput(volume2);
    drrFilter->Update();
    if (ImageOutput)
    {
      DRRWriterType::Pointer w = DRRWriterType::New();
      w->SetFileName("off_the_fly_reference2.mhd");
      w->SetInput(drrFilter->GetOutput());
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
    // store image intensities in a reference array:
    tregion = drrFilter->GetOutput(0)->GetLargestPossibleRegion();
    refArr3Size = tregion.GetNumberOfPixels();
    refArr3 = new DRRPixelType[refArr3Size];
    IntensityIteratorType dit6(drrFilter->GetOutput(0), tregion);
    c = 0;
    for (dit6.GoToBegin(); !dit6.IsAtEnd(); ++dit6)
      refArr3[c++] = dit6.Get();
    // compare to previous volume  (changed true)
    if (refArr3Size == refArr1Size)
    {
    	int numDiffPixels = 0;
      for(c = 0; c < refArr3Size; c++)
      {
        if(itk::Math::Round<int, double>(refArr3[c]) !=
        		itk::Math::Round<int, double>(refArr1[c]))
        {
          numDiffPixels++;
          break;
        }
      }
      if (numDiffPixels == 0)
      {
      	VERBOSE(<< " [test 2] ")
    		lok = false;
      }
    }
    else //size not changed
    {
      lok = false;
    }

    //disable global ITF Calculation and check if changed
    drrFilter->SetOffTheFlyITFMapping(false);
    drrFilter->Update();
    if (ImageOutput)
    {
      DRRWriterType::Pointer w = DRRWriterType::New();
      w->SetFileName("off_the_fly_image2.mhd");
      w->SetInput(drrFilter->GetOutput());
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
    tregion = drrFilter->GetOutput(0)->GetLargestPossibleRegion();
    refArr2Size = tregion.GetNumberOfPixels();
    delete[] refArr2;
    refArr2 = new DRRPixelType[refArr2Size];
    IntensityIteratorType dit7(drrFilter->GetOutput(0), tregion);
    c = 0;
    for (dit7.GoToBegin(); !dit7.IsAtEnd(); ++dit7)
      refArr2[c++] = dit7.Get();
    if (refArr2Size == refArr3Size)
    {
      for (c = 0; c < refArr2Size; c++)
      {
        if (itk::Math::Round<int, double>(refArr2[c] * ACCURACY) !=
        		itk::Math::Round<int, double>(refArr3[c] * ACCURACY))
        {
          lok = false;
          VERBOSE(<< " [test 3: " << refArr2[c] << " vs. " << refArr3[c] << "] ")
          break;
        }
      }
    }
    else
    {
      lok = false;
    }
    delete[] refArr1;
    delete[] refArr2;
    delete[] refArr3;
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
