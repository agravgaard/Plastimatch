//
#ifndef COMMONREGISTRATIONTOOLFUNCTIONS_HXX_
#define COMMONREGISTRATIONTOOLFUNCTIONS_HXX_

/**
 * Some common tool variables, macros and functions that are used by the n-way
 * 2D/3D-registration tests.
 *
 * @see TestMultiResolutionNWay2D3DRegistrationMethod.cxx
 * @see TestMultiResolutionNWay2D3DRegistrationMethod2.cxx
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 *
 * \ingroup Tests
 */

#include <stdlib.h>
#include <iostream>
#include <string>
#include <time.h>
#include <cstddef> /* Workaround bug in ITK 3.20 */

#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkEuler3DTransform.h>
#include <itkExtractImageFilter.h>
#include <itkNormalizedMutualInformationHistogramImageToImageMetric.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkPowellOptimizer.h>
#include <itkGradientDifferenceImageToImageMetric.h>
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>

#include <vtkColorTransferFunction.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTimerLog.h>

#include "oraMultiResolutionNWay2D3DRegistrationMethod.h"
#include "oraITKVTKDRRFilter.h"
#include "oraProjectionProperties.h"

#define VERBOSE(x) \
{ \
  if (Verbose) \
  {\
    std::cout x; \
    std::cout.flush(); \
  }\
}

// extended output
bool ExtendedOutput = false;

typedef unsigned short VolumePixelType;
typedef itk::Image<VolumePixelType, 3> VolumeImageType;
typedef itk::ImageRegionIteratorWithIndex<VolumeImageType> VolumeIteratorType;
typedef itk::ImageFileWriter<VolumeImageType> VolumeWriterType;
typedef float DRRPixelType;
typedef ora::ITKVTKDRRFilter<VolumePixelType, DRRPixelType> DRRFilterType;
typedef DRRFilterType::InputImageType VolumeImageType;
typedef DRRFilterType::OutputImageType DRRImageType;
typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
typedef itk::Euler3DTransform<double> Transform3DType;
typedef ora::ProjectionProperties<DRRPixelType> DRRPropsType;
typedef vtkSmartPointer<vtkColorTransferFunction> ITFPointer;
typedef itk::ImageFileWriter<DRRImageType> DRRWriterType;
typedef itk::ImageFileWriter<DRR2DImageType> DRR2DWriterType;
typedef itk::ExtractImageFilter<DRRImageType, DRR2DImageType> ExtractorType;
typedef ora::MultiResolutionNWay2D3DRegistrationMethod<DRR2DImageType,
    DRR2DImageType, VolumeImageType> RegistrationType;
typedef RegistrationType::MetricType RegMetricType;
typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<
    DRR2DImageType, DRR2DImageType> NMIMetricType;
typedef itk::MeanSquaresImageToImageMetric<DRR2DImageType, DRR2DImageType>
    MSMetricType;
typedef itk::PowellOptimizer PowellOptimizerType;
typedef itk::GradientDifferenceImageToImageMetric<DRR2DImageType,
    DRR2DImageType> GDMetricType;
typedef RegistrationType::MaskImageType MaskImageType;
typedef itk::ImageRegionIterator<MaskImageType> MaskIteratorType;
typedef itk::ImageFileWriter<MaskImageType> MaskWriterType;

std::string CurrentRegistrationPrefix = ""; // just a helper variable

/** Write a 2D DRR into a file. **/
void Write2DDRR(DRR2DImageType::ConstPointer drr, std::string fileName) {
  DRR2DWriterType::Pointer w = DRR2DWriterType::New();
  w->SetInput(drr);
  w->SetFileName(fileName.c_str());
  w->Update();
}

/** Write a 3D DRR into a file. **/
void Write3DDRR(DRRImageType::Pointer drr, std::string fileName) {
  DRRWriterType::Pointer w = DRRWriterType::New();
  w->SetInput(drr);
  w->SetFileName(fileName.c_str());
  w->Update();
}

/** Multi-resolution event command. **/
void MultiResolutionEvent(itk::Object *obj, const itk::EventObject &ev,
    void *cd) {
  RegistrationType *reg = (RegistrationType *) cd;

  if (std::string(ev.GetEventName()) == "StartEvent") {
    if (ExtendedOutput)
      VERBOSE(<< "\n    STARTING registration ...\n")
  } else if (std::string(ev.GetEventName()) == "EndEvent") {
    if (ExtendedOutput) {
      VERBOSE(<< "      - FINAL OPTIMUM: " <<
          reg->GetOptimizer()->GetCurrentPosition())
      VERBOSE(<< "        (after " << reg->GetNumberOfMetricEvaluationsAtLevel() <<
          " composite metric evaluations)\n")
      VERBOSE(<< "    ... FINISHING registration\n")
    }
  } else if (std::string(ev.GetEventName()) == "StartMultiResolutionLevelEvent") {
    if (ExtendedOutput) {
      if (reg->GetCurrentLevel() > 0) {
        VERBOSE(<< "      - OPTIMUM (L" << reg->GetCurrentLevel() << "): " <<
            reg->GetOptimizer()->GetCurrentPosition())
        VERBOSE(<< "        (after " << reg->GetNumberOfMetricEvaluationsAtLevel() <<
            " composite metric evaluations)\n")
      }
      VERBOSE(<< "    > LEVEL " << (reg->GetCurrentLevel() + 1) << " of " <<
          reg->GetNumberOfLevels() << "\n")
    }
  } else if (std::string(ev.GetEventName()) == "StartOptimizationEvent") {
    if (ImageOutput) {
      for (unsigned int i = 0; i < reg->GetMetric()->GetNumberOfMetricInputs(); i++) {
        RegistrationType::BaseMetricPointer subMetric =
            reg->GetMetric()->GetIthMetricInput(i);
        std::ostringstream os;
        os << CurrentRegistrationPrefix << "_FIXED_IMAGE_L"
            << (reg->GetCurrentLevel() + 1) << "_NO" << i << ".mhd";
        Write2DDRR(subMetric->GetFixedImage(), os.str());
        os.str("");
        os << CurrentRegistrationPrefix << "_INITIAL_MOVING_IMAGE_L"
            << (reg->GetCurrentLevel() + 1) << "_NO" << i << ".mhd";
        Write2DDRR(subMetric->GetMovingImage(), os.str());
      }
    }
    if (ExtendedOutput) {
      VERBOSE(<< "     [DRR-engine origin: " <<
          reg->GetDRREngine()->GetDRRPlaneOrigin() << "]\n")
      VERBOSE(<< "     [DRR-engine spacing: " <<
          reg->GetDRREngine()->GetDRRSpacing() << "]\n")
      VERBOSE(<< "     [DRR-engine size: " <<
          reg->GetDRREngine()->GetDRRSize() << "]\n")
      VERBOSE(<< "     [DRR-engine sampling distance: " <<
          reg->GetDRREngine()->GetSampleDistance() << "]\n")
    }

    if (CurrentRegistrationPrefix == "reg_4-way" || CurrentRegistrationPrefix
        == "reg_3-way") {
      // adjust optimizer settings:
      PowellOptimizerType *popt = (PowellOptimizerType *) reg->GetOptimizer();
      if (reg->GetCurrentLevel() == 0) {
        popt->SetStepLength(3.0);
        popt->SetStepTolerance(1.5);
      } else if (reg->GetCurrentLevel() == 1) {
        popt->SetStepLength(2.0);
        popt->SetStepTolerance(1.0);
      } else if (reg->GetCurrentLevel() == 2) {
        popt->SetStepLength(1.0);
        popt->SetStepTolerance(0.75);
      }
      if (ExtendedOutput) {
        VERBOSE(<< "     *** (OPTIMIZER): steplen=" << popt->GetStepLength() <<
            ", steptol=" << popt->GetStepTolerance() << "\n")
      }
    }
  }
}

/** Optimizer iteration event. **/
void OptimizerEvent(itk::Object *obj, const itk::EventObject &ev, void *cd) {
  RegistrationType *reg = (RegistrationType *) cd;

  if (std::string(ev.GetEventName()) == "IterationEvent") {
    if (ExtendedOutput) {
      PowellOptimizerType *popt = (PowellOptimizerType *) reg->GetOptimizer();

      unsigned int currIt = popt->GetCurrentIteration();
      unsigned int currLIt = popt->GetCurrentLineIteration();
      VERBOSE(<< "      " << currIt << " (" << currLIt << ")\t"
          << reg->GetLastMetricValue()
          << "\t" << reg->GetLastMetricParameters() << "\n")
    }

    if (CurrentRegistrationPrefix == "reg_1-way-stop") {
      PowellOptimizerType *popt = (PowellOptimizerType *) reg->GetOptimizer();

      if (popt->GetCurrentIteration() == 2) {
        reg->StopRegistration(); // -> catch StopRequestedEvent() as well!
      }
    }
  }
}

int RecognizedStopIteration = -1;

/** Registration stop request event. **/
void StopRegistrationEvent(itk::Object *obj, const itk::EventObject &ev,
    void *cd) {
  RegistrationType *reg = (RegistrationType *) cd;
  PowellOptimizerType *popt = (PowellOptimizerType *) reg->GetOptimizer();
  RecognizedStopIteration = popt->GetCurrentIteration(); // store
  popt->StopOptimization(); // does not have any effect on Powell optimizer
}

/** Compute 3D->2D extract region for extract image filter. **/
void Compute3D2DExtractRegion(DRRImageType::Pointer image3D,
    DRRImageType::RegionType &extractRegion) {
  if (!image3D)
    return;

  DRRImageType::RegionType region3D = image3D->GetLargestPossibleRegion();
  DRRImageType::IndexType start = region3D.GetIndex();
  DRRImageType::SizeType size = region3D.GetSize();
  size[2] = 0; // no 3rd dimension
  start[2] = 0; // 1st and only slice
  extractRegion.SetIndex(start);
  extractRegion.SetSize(size);
}

/** @return a test moving volume for testing n-way 2D/3D-registration **/
VolumeImageType::Pointer GenerateTestVolume() {
  VolumeImageType::SizeType isize;
  isize[0] = 101;
  isize[1] = 101;
  isize[2] = 81;
  VolumeImageType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  iindex[2] = 0;
  VolumeImageType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  VolumeImageType::SpacingType ispacing;
  ispacing[0] = 1.0;
  ispacing[1] = 1.0;
  ispacing[2] = 1.0;
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
  VolumePixelType v;
  VolumeImageType::IndexType p;
  srand(time(NULL));
  int mx = 1, mx2 = 20; // margins
  int my = 1, my2 = 30;
  int mz = 1, mz2 = 25;
  int zold = 0;
  for (it.GoToBegin(); !it.IsAtEnd(); ++it) // frustum of a pyramid
  {
    p = it.GetIndex();
    if ((int) p[2] >= mz && (int) p[2] < (int) (isize[2] - mz)) {
      if ((int) p[0] >= mx && (int) p[0] < (int) (isize[0] - mx) && (int) p[1]
          >= my && (int) p[1] < (int) (isize[1] - my)) {
        if ((int) p[0] >= mx2 && (int) p[0] < (int) (isize[0] - mx2)
            && (int) p[1] >= my2 && (int) p[1] < (int) (isize[1] - my2)
            && (int) p[2] >= mz2 && (int) p[2] < (int) (isize[2] - mz2)) {
          v = rand() % 151 + 1800; // some bony tissue
        } else {
          v = rand() % 201 + 1300; // some soft tissue
        }
      } else {
        v = static_cast<VolumePixelType> (0);
      }
    } else {
      v = static_cast<VolumePixelType> (0);
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
  if (ImageOutput) {
    VolumeWriterType::Pointer w = VolumeWriterType::New();
    w->SetInput(volume);
    w->SetFileName("volume.mhd");
    try {
      w->Update();
    }
    catch (itk::ExceptionObject &e) {
      volume = NULL;
    }
    w = NULL;
  }

  return volume;
}

/**
 * @param volume the volume to be projected
 * @param gantryAngle angle of the MV gantry (kV source is 90 degrees advanced);
 * the angle is expected to be in the range [-180;+180.00]
 * @param transform transformation to be applied to volume for projection
 * @param fname file name of the projection for image output
 * @param focalSpot returned focal spot position (must be of size 3!)
 * @param imageSizeID 0 ... original XVI image size, 1 ... 300x320 pixels,
 * 2 ... 250x280 pixels
 * @return a DRR (in single-sliced 3D representation) which simulates a kV
 * projection image acquired from an ELEKTA Linac's XVI (90 degrees out of
 * phase w.r.t. MV gantry); some fixed projection size is assumed
 **/
DRRImageType::Pointer GenerateLinacKVProjectionImage(
    VolumeImageType::Pointer volume, double gantryAngle,
    Transform3DType::Pointer transform, const char *fname, double focalSpot[3],
    int imageSizeID) {
  if (gantryAngle < -180 || gantryAngle > 180)
    return NULL;
  if (!volume)
    return NULL;
  if (!transform)
    return NULL;

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

  // gantry angle correction transformation:
  vtkSmartPointer<vtkTransform> thelp = vtkSmartPointer<vtkTransform>::New();
  thelp->Identity();
  thelp->RotateY(gantryAngle);

  DRRPropsType::MatrixType drrOrientation;
  // for MV gantry angle 0 degrees
  drrOrientation[0][0] = 0;
  drrOrientation[0][1] = 1;
  drrOrientation[0][2] = 0;
  drrOrientation[1][0] = 0;
  drrOrientation[1][1] = 0;
  drrOrientation[1][2] = 1;
  drrOrientation[2][0] = 1;
  drrOrientation[2][1] = 0;
  drrOrientation[2][2] = 0;
  // correction
  double *vec;
  int d;
  vec = thelp->TransformDoubleVector(drrOrientation[0]);
  for (d = 0; d < 3; d++)
    drrOrientation[0][d] = vec[d];
  vec = thelp->TransformDoubleVector(drrOrientation[1]);
  for (d = 0; d < 3; d++)
    drrOrientation[1][d] = vec[d];
  vec = thelp->TransformDoubleVector(drrOrientation[2]);
  for (d = 0; d < 3; d++)
    drrOrientation[2][d] = vec[d];
  props->SetProjectionPlaneOrientation(drrOrientation);

  DRRPropsType::PointType drrOrigin;
  // for MV gantry angle 0 degrees
  double o[3];
  o[0] = -540;
  if (imageSizeID == 1) // 300x320
  {
    o[1] = -150;
    o[2] = -160;
  } else if (imageSizeID == 2) // 250x280
  {
    o[1] = -135;
    o[2] = -140;
  } else // if (imageSizeID == 0) // 410x410
  {
    o[1] = -205;
    o[2] = -205;
  }
  // correction
  double *to = thelp->TransformDoublePoint(o);
  drrOrigin[0] = to[0];
  drrOrigin[1] = to[1];
  drrOrigin[2] = to[2];
  props->SetProjectionPlaneOrigin(drrOrigin);

  DRRPropsType::SizeType drrSize;
  if (imageSizeID == 1) // 300x320
  {
    drrSize[0] = 300;
    drrSize[1] = 320;
  } else if (imageSizeID == 2) // 250x280
  {
    drrSize[0] = 250;
    drrSize[1] = 280;
  } else // if (imageSizeID == 0) // 410x410
  {
    drrSize[0] = 410;
    drrSize[1] = 410;
  }
  props->SetProjectionSize(drrSize);
  DRRPropsType::SpacingType drrSpacing;
  drrSpacing[0] = 1.0;
  drrSpacing[1] = 1.0;
  props->SetProjectionSpacing(drrSpacing);

  props->SetSamplingDistance(0.5);

  DRRPropsType::PointType drrFocalSpot;
  // for MV gantry angle 0 degrees
  double fs[3];
  fs[0] = 1000;
  fs[1] = 0;
  fs[2] = 0;
  // correction
  double *tfs = thelp->TransformDoublePoint(fs);
  drrFocalSpot[0] = tfs[0];
  drrFocalSpot[1] = tfs[1];
  drrFocalSpot[2] = tfs[2];
  focalSpot[0] = tfs[0];
  focalSpot[1] = tfs[1];
  focalSpot[2] = tfs[2];
  props->SetSourceFocalSpotPosition(drrFocalSpot);

  // apply props to DRR filter:
  DRRFilterType::Pointer drrFilter = DRRFilterType::New();
  drrFilter->BuildRenderPipeline(); // must be called externally
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
  drrFilter->SetTransform(NULL); // disconnect transform
  props = NULL;

  DRRImageType::Pointer drr = drrFilter->GetOutput();
  try {
    drrFilter->Update();
  }
  catch (itk::ExceptionObject &e) {
    drrFilter = NULL;
    return NULL;
  }

  if (ImageOutput && fname) {
    DRRWriterType::Pointer w = DRRWriterType::New();
    w->SetInput(drr);
    w->SetFileName(fname);
    try {
      w->Update();
    }
    catch (itk::ExceptionObject &e) {
      std::cerr << "ERROR during writing: " << e << std::endl;
    }
    w = NULL;
  }

  drrFilter = NULL;
  return drr;
}

/** Compute min / max intensity values from the specified DRR. **/
void ComputeMinMaxIntensities(DRRImageType::Pointer drr, double &minv,
    double &maxv) {
  minv = 0;
  maxv = 0;
  if (!drr)
    return;

  typedef itk::ImageRegionIteratorWithIndex<DRRImageType> DRRIteratorType;

  DRRIteratorType dit(drr, drr->GetLargestPossibleRegion());
  DRRImageType::PixelType v;
  minv = +9e99;
  maxv = -9e99;
  for (dit.GoToBegin(); !dit.IsAtEnd(); ++dit) {
    v = dit.Get();
    if (v > maxv)
      maxv = v;
    if (v < minv)
      minv = v;
  }
}

/**
 * @param strictly if TRUE: strict checking (small tolerance); if FALSE: only
 * check if at least 5 parameters are closer to result than initial parameters
 * @return TRUE if the registration result (EULER3D-transform) is within
 * tolerance; expected value: 0,0,0,0,0,0
 **/
bool VerifyRegistrationResult(bool strictly,
    Transform3DType::ParametersType respars,
    Transform3DType::ParametersType initialpars)
{
  if (strictly)
  {
    for (unsigned int i = 0; i < 3; i++) // rotations: 0.1 rad tolerance
    {
      if (fabs(respars[i]) > 0.1)
        return false;
    }
    for (unsigned int i = 3; i < 5; i++) // x/y translations: 0.5 mm tolerance
    {
      if (fabs(respars[i]) > 0.5)
        return false;
    }
    // be a bit more tolerant w.r.t. z translation (depth-info): 2 mm tolerance
    if (fabs(respars[5]) > 2.0)
      return false;
  }
  else // more tolerant ...
  {
    int failcount = 0;
    for (unsigned int i = 0; i < 6; i++)
    {
      if (fabs(respars[i]) > fabs(initialpars[i]))
        failcount++;
    }
    if (failcount > 1) // 1 (e.g. z-parameter ...) allowed
      return false;
  }

  return true;
}

/**
 * @return a mask image with a centered circular "1"-region of specified radius;
 * the geometric constraints are derived from templateDRR
 */
MaskImageType::Pointer GenerateCircularMask(DRRImageType::Pointer templateDRR,
    double radius, std::string fname) {
  if (!templateDRR || radius < 0.)
    return NULL;

  MaskImageType::Pointer mask = MaskImageType::New();
  mask->SetRegions(templateDRR->GetLargestPossibleRegion());
  mask->SetOrigin(templateDRR->GetOrigin());
  mask->SetDirection(templateDRR->GetDirection());
  mask->SetSpacing(templateDRR->GetSpacing());
  mask->Allocate();

  MaskIteratorType mit(mask, mask->GetLargestPossibleRegion());
  MaskImageType::IndexType i;
  MaskImageType::SpacingType s = mask->GetSpacing();
  double hw = mask->GetLargestPossibleRegion().GetSize()[0] * s[0] / 2.;
  double hh = mask->GetLargestPossibleRegion().GetSize()[1] * s[1] / 2.;
  double x, y;
  double radius2 = radius * radius;
  for (mit.GoToBegin(); !mit.IsAtEnd(); ++mit) {
    i = mit.GetIndex();
    x = (double) i[0] * s[0] - hw;
    y = (double) i[1] * s[1] - hh;
    if ((x * x + y * y) <= radius2)
      mit.Set(1);
    else
      mit.Set(0);
  }

  if (ImageOutput) {
    MaskWriterType::Pointer w = MaskWriterType::New();
    w->SetFileName(fname.c_str());
    w->SetInput(mask);
    try {
      w->Update();
    }
    catch (itk::ExceptionObject &e) {
      std::cerr << "ERROR during mask writing: " << e << std::endl;
    }
    w = NULL;
  }

  return mask;
}

#endif /* COMMONREGISTRATIONTOOLFUNCTIONS_HXX_ */
