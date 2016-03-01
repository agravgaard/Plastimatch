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

#include <vtkColorTransferFunction.h>
#include <vtkSmartPointer.h>

#include "oraITKVTKDRRFilter.h"
#include "oraProjectionProperties.h"

#include "BasicUnitTestIncludes.hxx"

#define VERBOSE_TIMES(drrFilter, id) \
{ \
  if (ExtendedOutput) \
  { \
    double volumeTransfer = 0.; \
    double maskTransfer = 0.; \
    double drrComputation = 0.; \
    double preProcessing = 0.; \
    double rayCasting = 0.; \
    double postProcessing = 0.; \
    drrFilter->GetTimeMeasuresOfLastComputation(volumeTransfer, \
      maskTransfer, drrComputation, preProcessing, rayCasting,  \
      postProcessing); \
    std::cout << "Times in [ms] (" << id << "):\n"; \
    std::cout << " vol-transf:  " << volumeTransfer << "\n"; \
    std::cout << " mask-transf: " << maskTransfer << "\n"; \
    std::cout << " DRR-comp:    " << drrComputation << "\n"; \
    std::cout << " pre-prop:    " << preProcessing << "\n"; \
    std::cout << " ray-cast:    " << rayCasting << "\n"; \
    std::cout << " post-prop:   " << postProcessing << "\n"; \
    std::cout.flush(); \
  } \
}

// extended output (time measurements)
bool ExtendedOutput = false;

/**
 * Tests base functionality of:
 *
 *   ora::ITKVTKDRRFilter
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::ITKVTKDRRFilter
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @author Markus <markus.neuner (at) pmu.ac.at>
 * @version 1.2.1
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
  // advanced arguments:
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-xo" || std::string(argv[i])
        == "--extended-output")
    {
      ExtendedOutput = true;
      continue;
    }
  }

  VERBOSE(<< "\nTesting DRR engine.\n")
  bool ok = true;

  typedef unsigned short VolumePixelType;
  typedef float DRRPixelType;
  typedef ora::ITKVTKDRRFilter<VolumePixelType, DRRPixelType> DRRFilterType;
  typedef DRRFilterType::InputImageType VolumeImageType;
  typedef DRRFilterType::OutputImageType DRRImageType;
  typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
  typedef itk::ExtractImageFilter<DRRImageType, DRR2DImageType> ExtractorType;
  typedef itk::ImageRegionIteratorWithIndex<VolumeImageType> VolumeIteratorType;
  typedef itk::ImageFileWriter<VolumeImageType> VolumeWriterType;
  typedef itk::ImageFileWriter<DRRImageType> DRRWriterType;
  typedef itk::ImageFileWriter<DRR2DImageType> DRR2DWriterType;
  typedef ora::ProjectionProperties<DRRPixelType> DRRPropsType;
  typedef vtkSmartPointer<vtkColorTransferFunction> ITFPointer;
  typedef itk::Euler3DTransform<double> TransformType;
  typedef itk::ImageRegionIterator<DRRImageType> IntensityIteratorType;
  typedef itk::ImageRegionIterator<DRRFilterType::MaskImageType>
      MaskIntensityIteratorType;

  DRRFilterType::Pointer drrFilter = DRRFilterType::New();
  drrFilter->BuildRenderPipeline(); // must be called externally
  drrFilter->SetContextTitle("");
  drrFilter->WeakMTimeBehaviorOff();

  VERBOSE(<< "  * Generating input data ... ")
  bool lok = true; // local OK
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
  DRRPropsType::Pointer props = DRRPropsType::New();
  // ITF:  0,0.0 500,0.05 1001,0.2 1200,0.3 1201,0.3 2500,1.0 3000,1.0
  ITFPointer itf = ITFPointer::New();
  itf->AddRGBPoint(0, 0, 0, 0);
  itf->AddRGBPoint(500, 0.05, 0.05, 0.05);
  itf->AddRGBPoint(1001, 0.2, 0.2, 0.2);
  itf->AddRGBPoint(1200, 0.3, 0.3, 0.3);
  itf->AddRGBPoint(1201, 0.3, 0.3, 0.3);
  itf->AddRGBPoint(2500, 1.0, 1.0, 1.0);
  itf->AddRGBPoint(3000, 1.0, 1.0, 1.01);
  props->SetITF(itf);
  DRRPropsType::MatrixType drrOrientation;
  drrOrientation[0][0] = 1;
  drrOrientation[0][1] = 0;
  drrOrientation[0][2] = 0;
  drrOrientation[1][0] = 0;
  drrOrientation[1][1] = 1;
  drrOrientation[1][2] = 0;
  drrOrientation[2][0] = 0;
  drrOrientation[2][1] = 0;
  drrOrientation[2][2] = 1;
  TransformType::Pointer thelp = TransformType::New();
  TransformType::ParametersType parameters(6);
  parameters.Fill(0);
  parameters[0] = 0.05;
  parameters[1] = 0.03;
  parameters[2] = -0.035;
  thelp->SetParameters(parameters);
  TransformType::InputPointType p;
  p[0] = drrOrientation[0][0];
  p[1] = drrOrientation[0][1];
  p[2] = drrOrientation[0][2];
  p = thelp->TransformPoint(p);
  drrOrientation[0][0] = p[0];
  drrOrientation[0][1] = p[1];
  drrOrientation[0][2] = p[2];
  p[0] = drrOrientation[1][0];
  p[1] = drrOrientation[1][1];
  p[2] = drrOrientation[1][2];
  p = thelp->TransformPoint(p);
  drrOrientation[1][0] = p[0];
  drrOrientation[1][1] = p[1];
  drrOrientation[1][2] = p[2];
  p[0] = drrOrientation[2][0];
  p[1] = drrOrientation[2][1];
  p[2] = drrOrientation[2][2];
  p = thelp->TransformPoint(p);
  drrOrientation[2][0] = p[0];
  drrOrientation[2][1] = p[1];
  drrOrientation[2][2] = p[2];
  thelp = 0;
  props->SetProjectionPlaneOrientation(drrOrientation);
  DRRPropsType::PointType drrOrigin;
  drrOrigin[0] = -100;
  drrOrigin[1] = -80;
  drrOrigin[2] = -150;
  props->SetProjectionPlaneOrigin(drrOrigin);
  DRRPropsType::SizeType drrSize;
  drrSize[0] = 200;
  drrSize[1] = 160;
  props->SetProjectionSize(drrSize);
  DRRPropsType::SpacingType drrSpacing;
  drrSpacing[0] = 1.0;
  drrSpacing[1] = 1.0;
  props->SetProjectionSpacing(drrSpacing);
  props->SetSamplingDistance(0.5);
  DRRPropsType::PointType drrFocalSpot;
  drrFocalSpot[0] = 100;
  drrFocalSpot[1] = 10;
  drrFocalSpot[2] = 800;
  props->SetSourceFocalSpotPosition(drrFocalSpot);
  TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();
  // apply props
  drrFilter->SetNumberOfIndependentOutputs(1);
  ITFPointer iITF = drrFilter->GetInternalIntensityTransferFunction();
  iITF->ShallowCopy(itf);
  drrFilter->SetDRRPlaneOrientation(props->GetProjectionPlaneOrientation());
  drrFilter->SetDRRPlaneOrigin(props->GetProjectionPlaneOrigin());
  drrFilter->SetDRRSize(props->GetProjectionSize());
  drrFilter->SetDRRSpacing(props->GetProjectionSpacing());
  drrFilter->SetSampleDistance(props->GetSamplingDistance());
  drrFilter->SetSourceFocalSpotPosition(props->GetSourceFocalSpotPosition());
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
    if (i > 0) // vary transform parameters randomly
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
          sprintf(buff, "drr3D_%d.mhd", i);
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
            sprintf(buff, "drr2D_%d.mhd", i);
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
  extractFilter = NULL;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * DRR rescaling tests ... ")
  lok = true;
  const double MAX_INTENSITY_TOLERANCE = 0.5; // in %
  srand(time(NULL));
  // reference DRR (slope=1,intercept=0)
  DRRImageType::Pointer refDRR = drrFilter->GetOutput();
  DRRImageType::PixelType *refDRRArray = new DRRImageType::PixelType[refDRR->GetLargestPossibleRegion().GetNumberOfPixels()];
  IntensityIteratorType refIt(refDRR, refDRR->GetLargestPossibleRegion());
  int c = 0;
  DRRImageType::PixelType maxRefVal = -9e9;
  for (refIt.GoToBegin(); !refIt.IsAtEnd(); ++refIt)
  {
    refDRRArray[c] = refIt.Get();
    if (refDRRArray[c] > maxRefVal)
      maxRefVal = refDRRArray[c];
    c++;
  }
  if (maxRefVal == 0)
    maxRefVal = 1; // ... to avoid /0. !!!
  if (ImageOutput)
  {
    DRRWriterType::Pointer w = DRRWriterType::New();
    w->SetFileName("slope_intercept_reference_drr.mhd");
    w->SetInput(refDRR);
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
  for (int i = 0; i < 5; i++)
  {
    double slope = ((double) (rand() % 10000) - 5000.) / 1555.;
    double intercept = ((double) (rand() % 50000) - 25000.) / 1555.;
    drrFilter->SetRescaleSlope(slope);
    drrFilter->SetRescaleIntercept(intercept);
    if (ExtendedOutput)
    {
      VERBOSE(<< "\n")
      VERBOSE(<< "   Slope/Intercept " << (i + 1) << " of 5\n")
      VERBOSE(<< "   slope = " << slope << "\n")
      VERBOSE(<< "   intercept = " << intercept << "\n")
    }
    DRRImageType::Pointer currDRR = NULL;
    try
    {

      drrFilter->Update();

      currDRR = drrFilter->GetOutput();

      IntensityIteratorType drrIt(currDRR, currDRR->GetLargestPossibleRegion());
      int cc = 0;
      double maxError = -1;
      for (drrIt.GoToBegin(); !drrIt.IsAtEnd(); ++drrIt)
      {
        DRRImageType::PixelType refVal = refDRRArray[cc++];
        refVal = refVal * slope + intercept;
        DRRImageType::PixelType drrVal = drrIt.Get();
        // check
        double currError = fabs(refVal - drrVal) / maxRefVal * 100.;
        if (currError > maxError)
          maxError = currError;
      }
      if (ExtendedOutput)
        VERBOSE(<< "   max. error = " << maxError << " % (allowed: " <<
            MAX_INTENSITY_TOLERANCE << " %)\n")
      if (maxError > MAX_INTENSITY_TOLERANCE)
        lok = false;

      if (ImageOutput)
      {
        DRRWriterType::Pointer w = DRRWriterType::New();
        char buff[100];
        sprintf(buff, "slope_intercept_drr_%d.mhd", i);
        w->SetFileName(buff);
        w->SetInput(currDRR);
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
      if (ExtendedOutput)
      {
        if (lok)
          VERBOSE(<< "   ... ok\n")
        else
        VERBOSE(<< "   ... fail\n")
      }
    }
    catch (itk::ExceptionObject &e)
    {
      lok = false;
      if (ExtendedOutput)
        VERBOSE(<< "   ... fail\n")
    }
  }
  delete [] refDRRArray;
  drrFilter->SetRescaleSlope(1.0); // set back to original state
  drrFilter->SetRescaleIntercept(0.0);
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
  c = 0;
  for (dit1.GoToBegin(); !dit1.IsAtEnd(); ++dit1)
    refArr1[c++] = dit1.Get();
  // change some DRR-props:
  drrFilter->SetCurrentDRROutputIndex(1); // compute second input
  drrOrigin[0] -= 10;
  drrOrigin[1] += 15;
  drrOrigin[2] -= 20;
  props->SetProjectionPlaneOrigin(drrOrigin);
  drrFilter->SetDRRPlaneOrigin(props->GetProjectionPlaneOrigin());
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
      if (itk::Math::Round<int, double>(refArr1[c] * 100000.f) != itk::Math::Round<int, double>(refArr2[c]
          * 100000.f))
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
      if (itk::Math::Round<int, double>(refArr1[c] * 100000.f) != itk::Math::Round<int, double>(refArr2[c]
          * 100000.f))
        numDiffPixels++;
    }
    if (numDiffPixels == 0)
      lok = false;
  }
  else // changed, but not in size expected!
  {
    lok = false;
  }
  // change some DRR-props:
  drrFilter->SetCurrentDRROutputIndex(2); // compute third input
  ITFPointer itf2 = ITFPointer::New();
  itf2->AddRGBPoint(0, 0.05, 0.05, 0.05);
  itf2->AddRGBPoint(500, 0.07, 0.07, 0.07);
  itf2->AddRGBPoint(1001, 0.12, 0.12, 0.12);
  itf2->AddRGBPoint(1200, 0.16, 0.16, 0.16);
  itf2->AddRGBPoint(1201, 0.16, 0.16, 0.16);
  itf2->AddRGBPoint(2500, 0.3, 0.3, 0.3);
  itf2->AddRGBPoint(3000, 0.3, 0.3, 0.3);
  iITF = drrFilter->GetInternalIntensityTransferFunction();
  iITF->ShallowCopy(itf2);
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
      if (itk::Math::Round<int, double>(refArr1[c] * 100000.f) != itk::Math::Round<int, double>(refArr3[c]
          * 100000.f))
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
      if (itk::Math::Round<int, double>(refArr2[c] * 100000.f) != itk::Math::Round<int, double>(refArr3[c]
          * 100000.f))
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
      if (itk::Math::Round<int, double>(refArr1[c] * 100000.f) != itk::Math::Round<int, double>(refArr2[c]
          * 100000.f) && itk::Math::Round<int, double>(refArr1[c] * 100000.f) != itk::Math::Round<int, double>(
          refArr3[c] * 100000.f))
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
  props = NULL;
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
  DRRFilterType::SizeType currSz = drrFilter->GetDRRSize();
  currSz[0] *= 1;
  currSz[1] *= 1;
  drrFilter->SetDRRSize(currSz);
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
  for (int k = 0; k < 9; k++)
  {
    DRRFilterType::PointType currOrig = drrFilter->GetDRRPlaneOrigin();
    if (k == 3 || k == 6)
    {
      currOrig[0] -= 30;
      currOrig[1] -= 30;
      drrFilter->SetDRRPlaneOrigin(currOrig);
    }
    if (k >= 3 && k < 6)
    {
      double slope = ((double) (rand() % 10000) - 5000.) / 1555.;
      double intercept = ((double) (rand() % 50000) - 25000.) / 1555.;
      drrFilter->SetRescaleSlope(slope);
      drrFilter->SetRescaleIntercept(intercept);
      if (ExtendedOutput && k == 3)
        VERBOSE(<< "\n +++++ masked rescale/slope tests: START +++++\n")
    }
    else if (k == 6)
    {
      if (ExtendedOutput)
        VERBOSE(<< "\n +++++ masked rescale/slope tests: END +++++\n")
      drrFilter->SetRescaleSlope(1); // set back
      drrFilter->SetRescaleIntercept(0);
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

    drrFilter->SetDRRMask(mask);

    currOrig[0] += 5;
    currOrig[1] += 5;
    drrFilter->SetDRRPlaneOrigin(currOrig);
    try
    {
      drrFilter->Update();
      VERBOSE_TIMES(drrFilter, k)
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
      double ri = drrFilter->GetRescaleIntercept();
      if (ExtendedOutput)
        VERBOSE(<< " intercept=" << ri << "\n")
      for (mit2.GoToBegin(); !mit2.IsAtEnd(); ++mit2)
      {
        if (mit2.Get() == 0)
        {
          DRRFilterType::MaskImageType::IndexType idx = mit2.GetIndex();
          DRRImageType::PixelType vv = currDRR->GetPixel(idx);
          double divf = fabs(ri);
          if (divf == 0)
            divf = 1.; // avoid division by zero
          double e = fabs(vv - ri) / divf * 100.;
          if (e > me)
            me = e;
          if (e > MAX_INTENSITY_TOLERANCE)
          {
            ec++;
            lok = false; // must not have intensity different from 0!!!
          }
        }
      }
      if (!lok && ExtendedOutput)
        VERBOSE(<< "   max. error (count=" << ec << ") = " << me <<
            " % (allowed: " << MAX_INTENSITY_TOLERANCE << " %)\n")
    }
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Testing multiple DRR mask functionality ... ")
  lok = true;
  drrFilter->SetNumberOfIndependentOutputs(3);
  DRRFilterType::PointType currOrig = drrFilter->GetDRRPlaneOrigin();
  currOrig[0] -= 30;
  currOrig[1] -= 30;
  currOrig[2] -= 100;
  drrFilter->SetDRRPlaneOrigin(currOrig);
  // NOTE:
  // in runs i=0..2 the masks will be transferred to GPU, BUT
  // in runs i=3..5 the masks will simply be re-binded, not newly transferred!
  for (int i = 0; i < 6; i++)
  {
    drrFilter->SetCurrentDRROutputIndex(i % 3);

    currOrig = drrFilter->GetDRRPlaneOrigin();
    currOrig[0] += 5;
    currOrig[1] += 5;
    drrFilter->SetDRRPlaneOrigin(currOrig);

    if (i % 3 == 0)
      mask = mask1;
    else if (i % 3 == 1)
      mask = mask2;
    else if (i % 3 == 2)
      mask = mask3;
    drrFilter->SetDRRMask(mask);

    try
    {
      drrFilter->Update();
      VERBOSE_TIMES(drrFilter, i)
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
      currOrig[0] -= 30;
      currOrig[1] -= 30;
      drrFilter->SetDRRPlaneOrigin(currOrig);
    }
  }
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
