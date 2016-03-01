//
#ifndef COMMONREGISTRATIONTOOLFUNCTIONSNEW_HXX_
#define COMMONREGISTRATIONTOOLFUNCTIONSNEW_HXX_

/**
 * Some common tool variables, macros and functions that are used by the n-way
 * 2D/3D-registration tests.
 *
 * @see TestMultiResolutionNWay2D3DRegistrationMethod.cxx
 * @see TestMultiResolutionNWay2D3DRegistrationMethod2.cxx
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @author jeanluc
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
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>

#include <vtkColorTransferFunction.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTimerLog.h>

#include "oraMultiResolutionNWay2D3DRegistrationFramework.h"
#include "oraDRRFilter.h"
#include "oraImageBasedProjectionGeometry.h"
#include "oraCPUSiddonDRRFilter.h"
#include "oraGradientDifferenceImageToImageMetric.h"

#define VERBOSE(x) \
{ \
  if (Verbose) \
  {\
    std::cout x; \
    std::cout.flush(); \
  }\
}

// global constants describing a specified DRR engine implementation:
typedef enum
{
  DRR_ENGINE_CPU_SIDDON = 1 // ora::CPUSiddonDRRFilter
} DRREngineTypeEnum;

// extended output
bool ExtendedOutput = false;

typedef unsigned short VolumePixelType;
typedef itk::Image<VolumePixelType, 3> VolumeImageType;
typedef itk::ImageRegionIteratorWithIndex<VolumeImageType> VolumeIteratorType;
typedef itk::ImageFileWriter<VolumeImageType> VolumeWriterType;
typedef float DRRPixelType;
typedef ora::DRRFilter<VolumePixelType, DRRPixelType> DRRFilterType;
typedef DRRFilterType::Pointer DRRFilterPointer;
typedef DRRFilterType::InputImageType VolumeImageType;
typedef DRRFilterType::OutputImageType DRRImageType;
typedef itk::Image<DRRPixelType, 2> DRR2DImageType;
typedef itk::Euler3DTransform<double> Transform3DType;
typedef ora::ImageBasedProjectionGeometry<DRRPixelType> DRRGeometryType;
typedef ora::IntensityTransferFunction ITFType;
typedef ITFType::Pointer ITFPointer;
typedef itk::ImageFileWriter<DRRImageType> DRRWriterType;
typedef itk::ImageFileWriter<DRR2DImageType> DRR2DWriterType;
typedef itk::ExtractImageFilter<DRRImageType, DRR2DImageType> ExtractorType;
typedef ora::MultiResolutionNWay2D3DRegistrationFramework<DRR2DImageType,
    DRR2DImageType, VolumeImageType> RegistrationType;
typedef RegistrationType::MetricType RegMetricType;
typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<
    DRR2DImageType, DRR2DImageType> NMIMetricType;
typedef itk::MeanSquaresImageToImageMetric<DRR2DImageType, DRR2DImageType>
    MSMetricType;
typedef itk::PowellOptimizer PowellOptimizerType;
typedef ora::GradientDifferenceImageToImageMetric<DRR2DImageType,
		DRR2DImageType, double> GDMetricType;
typedef RegistrationType::DRREngineType:: MaskImageType MaskImageType;
typedef itk::ImageRegionIterator<MaskImageType> MaskIteratorType;
typedef itk::ImageFileWriter<MaskImageType> MaskWriterType;
typedef ora::CPUSiddonDRRFilter<VolumePixelType, DRRPixelType> SiddonDRRFilterType;
typedef SiddonDRRFilterType::Pointer SiddonDRRFilterPointer;


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
    	int numDRRs = reg->GetNumberOfMetricEvaluationsAtLevel() *
    			reg->GetNumberOfFixedImages();
      VERBOSE(<< "      - FINAL OPTIMUM: " <<
          reg->GetOptimizer()->GetCurrentPosition())
      VERBOSE(<< "        (after " << reg->GetNumberOfMetricEvaluationsAtLevel() <<
          " composite metric evaluations [= " << numDRRs << " DRR computations])\n")
      VERBOSE(<< "    ... FINISHING registration\n")
    }
  } else if (std::string(ev.GetEventName()) == "StartMultiResolutionLevelEvent") {
    if (ExtendedOutput) {
      if (reg->GetCurrentLevel() > 0) {
      	int numDRRs = reg->GetNumberOfMetricEvaluationsAtLevel() *
      			reg->GetNumberOfFixedImages();
        VERBOSE(<< "      - OPTIMUM (L" << reg->GetCurrentLevel() << "): " <<
            reg->GetOptimizer()->GetCurrentPosition())
				VERBOSE(<< "        (after " << reg->GetNumberOfMetricEvaluationsAtLevel() <<
						" composite metric evaluations [= " << numDRRs << " DRR computations])\n")
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
    	int outputIndex = reg->GetDRREngine()->GetCurrentDRROutputIndex();
      VERBOSE(<< "     [DRR-engine origin: " <<
          reg->GetDRREngine()->GetProjectionGeometry(outputIndex)->GetDetectorOrigin() << "]\n")
      VERBOSE(<< "     [DRR-engine spacing: " <<
          reg->GetDRREngine()->GetProjectionGeometry(outputIndex)->GetDetectorPixelSpacing() << "]\n")
      VERBOSE(<< "     [DRR-engine size: " <<
          reg->GetDRREngine()->GetProjectionGeometry(outputIndex)->GetDetectorSize() << "]\n")
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
  srand(537348); // fixed!
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

/** LINAC imaging geometry (relationship between MV gantry G (0 deg) and kV
 * imaging system (source S, detector origin R, detector row direction r,
 * detector column direction c, isocenter I of the roational imaging system).
 * WCS defines the directions of the world coordinate system.
 *
 *     /|        G = 0 deg
 *    / |        |                          z   y
 *  c/  |        |                          |  /    (WCS)
 *  /   |        |I                         | /
 * R  --+--------o-------------S            |---- x
 * |   /         |
 * |r /          |
 * | /           |
 * |/            |
 *
 * @param drrEngineType specifies the concrete type of DRR engine which should
 * be used for DRR computation
 * @param volume the volume to be projected
 * @param gantryAngle angle of the MV gantry (kV source is 90 degrees advanced);
 * the angle is expected to be in the range [-180;+180.00]
 * @param transform transformation to be applied to volume for projection
 * @param fname file name of the projection for image output
 * @param focalSpot returned focal spot position (must be of size 3!)
 * @param imageSizeID 0 ... original XVI image size, 1 ... 300x320 pixels,
 * 2 ... 250x280 pixels (images centered around cAx)
 * @return a DRR (in single-sliced 3D representation) which simulates a kV
 * projection image acquired from an ELEKTA Linac's XVI (+90 degrees out of
 * phase w.r.t. MV gantry); some fixed projection size is assumed
 **/
DRRImageType::Pointer GenerateLinacKVProjectionImage(
		DRREngineTypeEnum drrEngineType,
    VolumeImageType::Pointer volume, double gantryAngle,
    Transform3DType::Pointer transform, const char *fname, double focalSpot[3],
    int imageSizeID) {
  if (gantryAngle < -180 || gantryAngle > 180)
    return NULL;
  if (!volume)
    return NULL;
  if (!transform)
    return NULL;

  DRRFilterType *drrFilter = NULL;
  if (drrEngineType == DRR_ENGINE_CPU_SIDDON)
  {
    SiddonDRRFilterPointer siddonFilter = SiddonDRRFilterType::New();
    siddonFilter->Register();
    drrFilter = siddonFilter.GetPointer();
  }

  DRRGeometryType::Pointer geom = DRRGeometryType::New();
  // ITF:  0,0.0 500,0.05 1001,0.2 1200,0.3 1201,0.3 2500,1.0 3000,1.0
  ITFPointer itf = ITFType::New();
  itf->AddSupportingPoint(0, 0);
  itf->AddSupportingPoint(500, 0.05);
  itf->AddSupportingPoint(1001, 0.2);
  itf->AddSupportingPoint(1200, 0.3);
  itf->AddSupportingPoint(1201, 0.3);
  itf->AddSupportingPoint(2500, 1.0);
  itf->AddSupportingPoint(3000, 1.0);

  // gantry angle correction transformation:
  vtkSmartPointer<vtkTransform> thelp = vtkSmartPointer<vtkTransform>::New();
  thelp->Identity();
  thelp->RotateY(gantryAngle);

  // nominal row/column direction @ MV gantry 0 deg:
  double row[3] = {0, 0, -1};
  double column[3] = {0, 1, 0};
  thelp->TransformVector(row, row);
  thelp->TransformVector(column, column);
  geom->SetDetectorOrientation(row, column);

  // nominal image origin @ MV gantry 0 deg:
  double drrOrigin[3];
  drrOrigin[0] = -536;
  if (imageSizeID == 1) // 300x320
  {
    drrOrigin[1] = -160; // column
    drrOrigin[2] = 150; // row
  }
  else if (imageSizeID == 2) // 250x280
  {
    drrOrigin[1] = -140; // column
	  drrOrigin[2] = 125; // row
  }
  else // if (imageSizeID == 0) // 410x410
  {
    drrOrigin[1] = -205; // column
    drrOrigin[2] = 205; // row
  }
  thelp->TransformPoint(drrOrigin, drrOrigin);
  geom->SetDetectorOrigin(drrOrigin);

  int drrSize[2];
  if (imageSizeID == 1) // 300x320
  {
    drrSize[0] = 300;
    drrSize[1] = 320;
  }
  else if (imageSizeID == 2) // 250x280
  {
    drrSize[0] = 250;
    drrSize[1] = 280;
  }
  else // if (imageSizeID == 0) // 410x410
  {
    drrSize[0] = 410;
    drrSize[1] = 410;
  }
  geom->SetDetectorSize(drrSize);

  double drrSpacing[2];
  drrSpacing[0] = 1.0;
  drrSpacing[1] = 1.0;
  geom->SetDetectorPixelSpacing(drrSpacing);

  // for MV gantry angle 0 degrees
  focalSpot[0] = 1000;
  focalSpot[1] = 0;
  focalSpot[2] = 0;
  thelp->TransformPoint(focalSpot, focalSpot);
  geom->SetSourcePosition(focalSpot);

  // apply geom to DRR filter:
  drrFilter->SetInput(volume);
  drrFilter->SetITF(itf);
  drrFilter->SetProjectionGeometry(0, geom.GetPointer());
  drrFilter->SetTransform(transform);

  DRRImageType::Pointer drr = drrFilter->GetOutput();
  try
  {
    drrFilter->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    drrFilter->UnRegister();
    drrFilter = NULL;
    return NULL;
  }

  if (ImageOutput && fname)
  {
    DRRWriterType::Pointer w = DRRWriterType::New();
    w->SetInput(drr);
    w->SetFileName(fname);
    try
    {
      w->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << "ERROR during writing: " << e << std::endl;
    }
    w = NULL;
  }
  drrFilter->SetTransform(NULL); // disconnect transform
  drrFilter->UnRegister();
  drrFilter = NULL;
  geom = NULL;
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
  bool strict = true;
  bool notStrict = true;
  // strict test
  for (unsigned int i = 0; i < 3; i++) // rotations: 0.1 rad tolerance
  {
    if (fabs(respars[i]) > 0.1)
      strict = false;
  }
  for (unsigned int i = 3; i < 5; i++) // x/y translations: 0.5 mm tolerance
  {
    if (fabs(respars[i]) > 0.5)
  	  strict = false;
  }
  // be a bit more tolerant w.r.t. z translation (depth-info): 2 mm tolerance
  if (fabs(respars[5]) > 2.0)
  	strict = false;
  int failcount = 0;
  for (unsigned int i = 0; i < 6; i++)
  {
  	if (initialpars[i] > 0)
  	{
  		if (respars[i] >= initialpars[i])
  			failcount++;
  	}
  	else
  	{
  		if (respars[i] <= initialpars[i])
  			failcount++;
  	}
  }
  if (failcount > 1) // 1 (e.g. z-parameter ...) allowed
  	notStrict = false;
  // however, ...
  if (!notStrict && strict)
  	notStrict = true; //!!!
  if (ExtendedOutput) {
    VERBOSE(<< "\n Strict test passed	: " << strict
    		<< "\n Tolerant test passed : " << notStrict << "\n")
  }
  if(strictly)
	  return strict;
  return notStrict;
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
  MaskImageType::RegionType maskReg;
  MaskImageType::IndexType maskIdx;
  maskIdx.Fill(0);
  maskReg.SetIndex(maskIdx);
  MaskImageType::SizeType maskSz;
  maskSz[0] = templateDRR->GetLargestPossibleRegion().GetSize()[0];
  maskSz[1] = templateDRR->GetLargestPossibleRegion().GetSize()[1];
  maskSz[2] = 1;
  maskReg.SetSize(maskSz);
  mask->SetRegions(maskReg);
  MaskImageType::PointType maskOrig;
  maskOrig.Fill(0);
  mask->SetOrigin(maskOrig);
  MaskImageType::DirectionType maskDir;
  maskDir.SetIdentity();
  mask->SetDirection(maskDir);
  MaskImageType::SpacingType maskSpac;
  maskSpac[0] = templateDRR->GetSpacing()[0];
  maskSpac[1] = templateDRR->GetSpacing()[1];
  maskSpac[2] = 1.0;
  mask->SetSpacing(maskSpac);
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

  if (ImageOutput)
  {
    MaskWriterType::Pointer w = MaskWriterType::New();
    w->SetFileName(fname.c_str());
    w->SetInput(mask);
    try
    {
      w->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << "ERROR during mask writing: " << e << std::endl;
    }
    w = NULL;
  }

  return mask;
}


#endif /* COMMONREGISTRATIONTOOLFUNCTIONSNEW_HXX_ */
