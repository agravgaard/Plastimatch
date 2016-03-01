//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <time.h>
#include <math.h>
#include <cstddef> /* Workaround bug in ITK 3.20 */

#include <itkImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageFileWriter.h>
#include <itkEuler3DTransform.h>

#include <vtkPlane.h>
#include <vtkMath.h>

#include "oraITKVTKDRRFilter.h"
#include "oraProjectionProperties.h"

#include "BasicUnitTestIncludes.hxx"

// extended output (error measures)
bool ExtendedOutput = false;
// failure write output
bool FailureWriteOutput = false;

typedef unsigned short VolumePixelType;
typedef itk::Image<VolumePixelType, 3> VolumeImageType;
typedef itk::ImageRegionIteratorWithIndex<VolumeImageType> VolumeIteratorType;
typedef itk::ImageFileWriter<VolumeImageType> VolumeWriterType;
typedef float DRRPixelType;
typedef ora::ITKVTKDRRFilter<VolumePixelType, DRRPixelType> DRRFilterType;
typedef DRRFilterType::InputImageType VolumeImageType;
typedef DRRFilterType::OutputImageType DRRImageType;
typedef itk::ImageRegionIteratorWithIndex<DRRImageType> DRRIteratorType;
typedef ora::ProjectionProperties<DRRPixelType> DRRPropsType;
typedef vtkSmartPointer<vtkColorTransferFunction> ITFPointer;
typedef itk::Euler3DTransform<double> TransformType;
typedef itk::ImageFileWriter<DRRImageType> DRRWriterType;

/**
 * @param mode different orientation / positioning options for the phantom <br>
 * 0 ... identity-orientation, centered around WCS-zero-point <br>
 * 1 ... identity-orientation, offset around WCS-zero-point <br>
 * 2 ... non-identity-orientation-0, centered around WCS-zero-point <br>
 * 3 ... non-identity-orientation-1, centered around WCS-zero-point <br>
 * 4 ... non-identity-orientation-2, centered around WCS-zero-point <br>
 * 5 ... non-identity-orientation-3, centered around WCS-zero-point <br>
 * 6 ... non-identity-orientation-0, offset around WCS-zero-point <br>
 * 7 ... non-identity-orientation-1, offset around WCS-zero-point <br>
 * 8 ... non-identity-orientation-2, offset around WCS-zero-point <br>
 * 9 ... non-identity-orientation-3, offset around WCS-zero-point <br>
 * @return a phantom volume suitable for DRR engine geometry verification
 **/
VolumeImageType::Pointer GeneratePhantom(int mode)
{
  if (mode < 0 || mode > 9)
    return NULL;

  VolumeImageType::SizeType isize;
  isize[0] = 101;
  isize[1] = 91;
  isize[2] = 81;
  VolumeImageType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  iindex[2] = 0;
  VolumeImageType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  VolumeImageType::SpacingType ispacing;
  ispacing[0] = 0.5;
  ispacing[1] = 0.5;
  ispacing[2] = 0.5;
  VolumeImageType::DirectionType idirection;
  idirection.SetIdentity();
  if (mode == 2 || mode == 6)
  {
    // 1 0 0 0 0 1 0 -1 0
    idirection[0][0] = 1;
    idirection[1][0] = 0;
    idirection[2][0] = 0;
    idirection[0][1] = 0;
    idirection[1][1] = 0;
    idirection[2][1] = 1;
    idirection[0][2] = 0;
    idirection[1][2] = -1;
    idirection[2][2] = 0;
  }
  else if (mode == 3 || mode == 7)
  {
    // -1 0 0 0 0 -1 0 -1 0
    idirection[0][0] = -1;
    idirection[1][0] = 0;
    idirection[2][0] = 0;
    idirection[0][1] = 0;
    idirection[1][1] = 0;
    idirection[2][1] = -1;
    idirection[0][2] = 0;
    idirection[1][2] = -1;
    idirection[2][2] = 0;
  }
  else if (mode == 4 || mode == 8)
  {
    // non-axis-aligned
    TransformType::Pointer axisTransform = TransformType::New();
    TransformType::ParametersType apars(6);
    apars.Fill(0);
    apars[0] = 0.112;
    apars[1] = -0.023;
    apars[2] = -0.1973;
    axisTransform->SetParameters(apars);
    TransformType::OutputVectorType ax;
    ax[0] = -1;
    ax[1] = 0;
    ax[2] = 0;
    ax = axisTransform->TransformVector(ax);
    idirection[0][0] = ax[0];
    idirection[1][0] = ax[1];
    idirection[2][0] = ax[2];
    ax[0] = 0;
    ax[1] = 0;
    ax[2] = -1;
    ax = axisTransform->TransformVector(ax);
    idirection[0][1] = ax[0];
    idirection[1][1] = ax[1];
    idirection[2][1] = ax[2];
    ax[0] = 0;
    ax[1] = -1;
    ax[2] = 0;
    ax = axisTransform->TransformVector(ax);
    idirection[0][2] = ax[0];
    idirection[1][2] = ax[1];
    idirection[2][2] = ax[2];
  }
  else if (mode == 5 || mode == 9)
  {
    // non-axis-aligned
    TransformType::Pointer axisTransform = TransformType::New();
    TransformType::ParametersType apars(6);
    apars.Fill(0);
    apars[0] = -0.1342;
    apars[1] = 0.158;
    apars[2] = 0.0823;
    axisTransform->SetParameters(apars);
    TransformType::OutputVectorType ax;
    ax[0] = 1;
    ax[1] = 0;
    ax[2] = 0;
    ax = axisTransform->TransformVector(ax);
    idirection[0][0] = ax[0];
    idirection[1][0] = ax[1];
    idirection[2][0] = ax[2];
    ax[0] = 0;
    ax[1] = 0;
    ax[2] = 1;
    ax = axisTransform->TransformVector(ax);
    idirection[0][1] = ax[0];
    idirection[1][1] = ax[1];
    idirection[2][1] = ax[2];
    ax[0] = 0;
    ax[1] = -1;
    ax[2] = 0;
    ax = axisTransform->TransformVector(ax);
    idirection[0][2] = ax[0];
    idirection[1][2] = ax[1];
    idirection[2][2] = ax[2];
  }
  VolumeImageType::PointType iorigin;
  iorigin[0] = 0;
  iorigin[1] = 0;
  iorigin[2] = 0;
  double vec[3];
  for (int d = 0; d < 3; d++) // primarily centered
  {
    vec[0] = idirection[0][d];
    vec[1] = idirection[1][d];
    vec[2] = idirection[2][d];
    vtkMath::Normalize(vec);
    iorigin[0] += -vec[0] * (double) isize[d] * ispacing[d] / 2.;
    iorigin[1] += -vec[1] * (double) isize[d] * ispacing[d] / 2.;
    iorigin[2] += -vec[2] * (double) isize[d] * ispacing[d] / 2.;
  }
  if (mode == 1 || (mode >= 6 && mode <= 9)) // offset
  {
    srand(time(NULL));
    iorigin[0] += ((double) (rand() % 101 - 50)) / 5.0;
    iorigin[1] += ((double) (rand() % 101 - 50)) / 5.0;
    iorigin[2] += ((double) (rand() % 101 - 50)) / 5.0;
  }
  VolumeImageType::Pointer volume = VolumeImageType::New();
  volume->SetSpacing(ispacing);
  volume->SetOrigin(iorigin);
  volume->SetDirection(idirection);
  volume->SetRegions(iregion);
  volume->Allocate();
  volume->FillBuffer(0);
  VolumeIteratorType it(volume, iregion);
  VolumePixelType v = 5000;
  VolumeImageType::IndexType p;
  p[0] = 11;
  p[1] = 0;
  p[2] = 0;
  it.SetIndex(p);
  it.Set(v);
  p[0] = 3;
  p[1] = 73;
  p[2] = 6;
  it.SetIndex(p);
  it.Set(v);
  p[0] = 95;
  p[1] = 87;
  p[2] = 4;
  it.SetIndex(p);
  it.Set(v);
  p[0] = 80;
  p[1] = 6;
  p[2] = 11;
  it.SetIndex(p);
  it.Set(v);
  p[0] = 0;
  p[1] = 0;
  p[2] = 71;
  it.SetIndex(p);
  it.Set(v);
  p[0] = 0;
  p[1] = 90;
  p[2] = 80;
  it.SetIndex(p);
  it.Set(v);
  p[0] = 100;
  p[1] = 90;
  p[2] = 75;
  it.SetIndex(p);
  it.Set(v);
  p[0] = 100;
  p[1] = 0;
  p[2] = 69;
  it.SetIndex(p);
  it.Set(v);
  p[0] = 50;
  p[1] = 45;
  p[2] = 40;
  it.SetIndex(p);
  it.Set(v);
  if (ImageOutput)
  {
    VolumeWriterType::Pointer w = VolumeWriterType::New();
    w->SetInput(volume);
    char buff[200];
    sprintf(buff, "volume_mode%d.mhd", mode);
    w->SetFileName(buff);
    try
    {
      w->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      volume = NULL;
    }
    w = NULL;
  }

  return volume;
}

/**
 * Compute DRR w.r.t. current projection properties and verify it with
 * analytically calculated phantom projection data.
 * @param drrFilter DRR filter
 * @param props current projection properties
 * @param volume phantom volume
 * @param fname file name for DRR output (if image output is activated)
 */
bool ComputeAndVerifyProjection(DRRFilterType::Pointer drrFilter,
    DRRPropsType::Pointer props, VolumeImageType::Pointer volume,
    const char *fname)
{
  if (ExtendedOutput)
    VERBOSE(<< "\n    " << fname << "\n")
  DRRPropsType::SpacingType sp = props->GetProjectionSpacing();
  double maxSp = sp[0];
  if (sp[1] > maxSp)
    maxSp = sp[1];
  // 3rd component uninteresting ...
  // MAXIMUM TOLERATED ERROR: (DRR-spacing-dependent; times 1.5 because weighted
  // centroid estimation is not perfect)
  const double MAX_ALLOWED_ERROR = maxSp * 2.5;
  const double MAX_ALLOWED_SINGLE_ERROR = maxSp * 10.;

  bool succ = true;
  // apply projection settings
  ITFPointer iITF = drrFilter->GetInternalIntensityTransferFunction();
  iITF->ShallowCopy(props->GetITF());
  drrFilter->SetDRRPlaneOrientation(props->GetProjectionPlaneOrientation());
  drrFilter->SetDRRPlaneOrigin(props->GetProjectionPlaneOrigin());
  drrFilter->SetDRRSize(props->GetProjectionSize());
  drrFilter->SetDRRSpacing(props->GetProjectionSpacing());
  drrFilter->SetSampleDistance(props->GetSamplingDistance());
  drrFilter->SetSourceFocalSpotPosition(props->GetSourceFocalSpotPosition());
  drrFilter->SetInput(volume);
  DRRImageType::Pointer drr = NULL;
  try
  {
    drrFilter->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    succ = false;
  }
  if (drrFilter->GetOutput())
  {
    drr = drrFilter->GetOutput();
    if (ImageOutput)
    {
      DRRWriterType::Pointer w = DRRWriterType::New();
      w->SetFileName(fname);
      w->SetInput(drr);
      try
      {
        w->Update();
      }
      catch (itk::ExceptionObject &e)
      {
        succ = false;
      }
      w = NULL;
    }
    double maxError = -1;
    double minError = 1e7;
    double meanError = 0;
    std::vector<double> errors;
    VolumeIteratorType it(volume, volume->GetLargestPossibleRegion());
    VolumeImageType::IndexType idx;
    VolumeImageType::PointType point;
    DRRImageType::IndexType computedIdx;
    DRRImageType::PointType dp;
    int numMarkers = 0;
    errors.clear();
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      if (it.Get() > 0)
      {
        numMarkers++;
        idx = it.GetIndex();
        volume->TransformIndexToPhysicalPoint(idx, point);
        point = drrFilter->GetTransform()->TransformPoint(point);

        double fs[3];
        fs[0] = props->GetSourceFocalSpotPosition()[0];
        fs[1] = props->GetSourceFocalSpotPosition()[1];
        fs[2] = props->GetSourceFocalSpotPosition()[2];
        double p[3];
        p[0] = point[0];
        p[1] = point[1];
        p[2] = point[2];
        double n[3];
        n[0] = drrFilter->GetDRRPlaneOrientation()[2][0];
        n[1] = drrFilter->GetDRRPlaneOrientation()[2][1];
        n[2] = drrFilter->GetDRRPlaneOrientation()[2][2];
        double po[3];
        po[0] = props->GetProjectionPlaneOrigin()[0];
        po[1] = props->GetProjectionPlaneOrigin()[1];
        po[2] = props->GetProjectionPlaneOrigin()[2];
        double pc;
        double x[3];
        vtkPlane::IntersectWithLine(fs, p, n, po, pc, x);
        dp[0] = x[0];
        dp[1] = x[1];
        dp[2] = x[2];
        drr->TransformPhysicalPointToIndex(dp, computedIdx);
        // search for nearest first fiducial pixel in generated projection:
        DRRIteratorType dit(drr, drr->GetLargestPossibleRegion());
        DRRImageType::IndexType didx;
        bool seedFound = false;
        int radius = -1;
        int c;
        int neighborhood = 0;
        int *nidxX = NULL;
        int *nidxY = NULL;
        didx.Fill(0); // 3rd comp. of DRR index is assumed to be constantly 0
        // NOTE: maximum search radius is half of DRR width/height
        int maxradius = (int) drr->GetLargestPossibleRegion().GetSize()[0] / 2;
        if ((int) drr->GetLargestPossibleRegion().GetSize()[1] / 2 > maxradius)
          maxradius = (int) drr->GetLargestPossibleRegion().GetSize()[1] / 2;
        do
        {
          radius++;
          if (radius > maxradius)
            break;
          // generate neighborhood info:
          c = 0;
          if (radius > 0)
          {
            neighborhood = radius * 8;
            nidxX = new int[radius * 8];
            nidxY = new int[radius * 8];
            for (int y = -radius; y <= +radius; y++)
            {
              if (y == -radius || y == +radius)
              {
                for (int x = -radius; x <= +radius; x++)
                {
                  nidxX[c] = x;
                  nidxY[c] = y;
                  c++;
                }
              }
              else
              {
                nidxX[c] = -radius;
                nidxY[c] = y;
                c++;
                nidxX[c] = +radius;
                nidxY[c] = y;
                c++;
              }
            }
          }
          else
          {
            neighborhood = 1;
            nidxX = new int[1];
            nidxY = new int[1];
            nidxX[0] = 0;
            nidxY[0] = 0;
          }
          // search for seed:
          for (int u = 0; u < neighborhood; u++)
          {
            didx[0] = computedIdx[0] + nidxX[u];
            didx[1] = computedIdx[1] + nidxY[u];
            if (didx[0] < dit.GetRegion().GetIndex()[0] || didx[1]
                < dit.GetRegion().GetIndex()[1]
                || didx[0]
                    > static_cast<DRRImageType::IndexType::IndexValueType> (dit.GetRegion().GetIndex()[0]
                        + dit.GetRegion().GetSize()[0])
                || didx[1]
                    > static_cast<DRRImageType::IndexType::IndexValueType> (dit.GetRegion().GetIndex()[1]
                        + dit.GetRegion().GetSize()[1]))
              continue;

            dit.SetIndex(didx);
            if (dit.Get() > 0)
            {
              seedFound = true; // found (contained in didx)
              break;
            }
          }
          delete[] nidxX;
          delete[] nidxY;
        }
        while (!seedFound);

        if (seedFound)
        {
          // outgoing from seed we locate all connected pixels and compute the
          // weighted centroid:
          double centroid[3];
          double wsum;
          bool pixelFound;
          DRRImageType::PixelType dv;
          DRRImageType::PointType dpoint;
          DRRImageType::IndexType seed;
          seed[0] = didx[0];
          seed[1] = didx[1];
          seed[2] = didx[2];
          radius = -1;
          wsum = 0;
          centroid[0] = 0;
          centroid[1] = 0;
          centroid[2] = 0;
          do
          {
            pixelFound = false;
            radius++;
            // generate neighborhood info:
            c = 0;
            if (radius > 0)
            {
              neighborhood = radius * 8;
              nidxX = new int[radius * 8];
              nidxY = new int[radius * 8];
              for (int y = -radius; y <= +radius; y++)
              {
                if (y == -radius || y == +radius)
                {
                  for (int x = -radius; x <= +radius; x++)
                  {
                    nidxX[c] = x;
                    nidxY[c] = y;
                    c++;
                  }
                }
                else
                {
                  nidxX[c] = -radius;
                  nidxY[c] = y;
                  c++;
                  nidxX[c] = +radius;
                  nidxY[c] = y;
                  c++;
                }
              }
            }
            else
            {
              neighborhood = 1;
              nidxX = new int[1];
              nidxY = new int[1];
              nidxX[0] = 0;
              nidxY[0] = 0;
            }

            // search for pixels:
            for (int u = 0; u < neighborhood; u++)
            {
              didx[0] = seed[0] + nidxX[u];
              didx[1] = seed[1] + nidxY[u];
              didx[2] = seed[2];
              dit.SetIndex(didx);
              dv = dit.Get();
              if (dv > 0)
              {
                pixelFound = true; // found (contained in didx)
                wsum += (double) dv;
                drr->TransformIndexToPhysicalPoint(didx, dpoint);
                centroid[0] += (double) dpoint[0] * (double) dv;
                centroid[1] += (double) dpoint[1] * (double) dv;
                centroid[2] += (double) dpoint[2] * (double) dv;
                // break; ... BUG!
              }
            }
            delete[] nidxX;
            delete[] nidxY;
          }
          while (pixelFound);

          if (wsum > 0)
          {
            centroid[0] /= wsum;
            centroid[1] /= wsum;
            centroid[2] /= wsum;
          }
          double dpd[3];
          dpd[0] = dp[0];
          dpd[1] = dp[1];
          dpd[2] = dp[2];
          double errDist = vtkMath::Distance2BetweenPoints(centroid, dpd);
          errDist = sqrt(errDist);
          if (errDist > maxError)
            maxError = errDist;
          if (errDist < minError)
            minError = errDist;
          meanError += errDist;
          errors.push_back(errDist);
        }
        else // NO SEED FOUND (broke loop due to zero-image)
        {
          double errDist = 9e6; // 1m!
          if (errDist > maxError)
            maxError = errDist;
          if (errDist < minError)
            minError = errDist;
          meanError += errDist;
          errors.push_back(errDist);
        }
      }
    }
    if (succ && maxError > MAX_ALLOWED_ERROR)
    {
      // however, 1 or 2 markers are allowed to exceed the limit (due to
      // limitations of the detection algorithm); it is a common problem - if
      // a pair of markers is close together - that the wrong markers are
      // mutually detected (therefore up to 2 outliers allowed):
      int cc = 0;
      double theError0 = -1;
      double theError1 = -1;
      for (unsigned int i = 0; i < errors.size(); i++)
      {
        if (errors[i] > MAX_ALLOWED_ERROR)
        {
          if (cc == 0)
            theError0 = errors[i];
          else
            theError1 = errors[i];
          cc++;
        }
      }
      if (cc <= 2)
      {
        if (theError0 > MAX_ALLOWED_SINGLE_ERROR || theError1
            > MAX_ALLOWED_SINGLE_ERROR)
          succ = false;
        else
          // tolerated!
          succ = true;
      }
      else // >2 outliers
      {
        succ = false;
      }
    }
    if (!ImageOutput && FailureWriteOutput && !succ)
    {
      DRRWriterType::Pointer w = DRRWriterType::New();
      w->SetFileName(fname);
      w->SetInput(drr);
      try
      {
        w->Update();
      }
      catch (itk::ExceptionObject &e)
      {
        ;
      }
      w = NULL;
    }
    meanError /= (double) numMarkers;
    double errorSTD = 0;
    for (unsigned int i = 0; i < errors.size(); i++)
    {
      errorSTD += (errors[i] - meanError) * (errors[i] - meanError);
    }
    if (errors.size() > 1)
      errorSTD /= (double) (errors.size() - 1);
    errorSTD = sqrt(errorSTD);
    if (ExtendedOutput)
    {
      VERBOSE(<< "     - verification: " << (succ ? "OK" : "FAILURE") << "\n")
      VERBOSE(<< "     - TRANSFORM: " << drrFilter->GetTransform()->GetParameters() << "\n")
      VERBOSE(<< "     - tolerance: " << (MAX_ALLOWED_ERROR * 1000) << " um\n")
      if (maxError < 9e6)
      {
        VERBOSE(<< "     - number of markers: " << numMarkers << "\n")
        VERBOSE(<< "     - max. error: " << (maxError * 1000) << " um\n")
        VERBOSE(<< "     - min. error: " << (minError * 1000) << " um\n")
        VERBOSE(<< "     - mean: " << (meanError * 1000) << " um\n")
        VERBOSE(<< "     - STD: " << (errorSTD * 1000) << " um\n")
      }
      else
      {
        VERBOSE(<< "     - OUTPUT DRR APPEARS TO HAVE SOLELY ZERO PIXELS\n")
      }
    }
  }
  else
  {
    succ = false;
  }

  return succ;
}

/**
 * Tests base functionality of:
 *
 *   ora::ITKVTKDRRFilter,
 *   ora::ProjectionProperties
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::ITKVTKDRRFilter
 * @see ora::ProjectionProperties
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.1
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  helpLines.push_back(
      "  -xo or --extended-output ... write out additional error measures");
  helpLines.push_back(
      "  -fwo or --failure-write-output ... write output projections if a failure occurs");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, true, false);
    return EXIT_SUCCESS;
  }
  // advanced arguments:
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-fwo" || std::string(argv[i])
        == "--failure-write-output")
    {
      FailureWriteOutput = true;
      continue;
    }
    if (std::string(argv[i]) == "-xo" || std::string(argv[i])
        == "--extended-output")
    {
      ExtendedOutput = true;
      continue;
    }
  }

  VERBOSE(<< "\nTesting projection geometry of DRR engine.\n")
  bool ok = true;

  DRRFilterType::Pointer drrFilter = DRRFilterType::New();
  drrFilter->BuildRenderPipeline(); // must be called externally
  drrFilter->SetContextTitle("");
  drrFilter->WeakMTimeBehaviorOff();

  VERBOSE(<< "  * Generating test phantom data ... ")
  VolumeImageType::Pointer volume = GeneratePhantom(4);
  if (!volume)
    ok = false;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Setting up DRR engine ... ")
  DRRPropsType::Pointer props = DRRPropsType::New();
  // ITF:  0,0.0 2500,0.0 2501,1.0 6000,1.0
  ITFPointer itf = ITFPointer::New();
  itf->AddRGBPoint(0, 0, 0, 0);
  itf->AddRGBPoint(2500, 0.0, 0.0, 0.0);
  itf->AddRGBPoint(2501, 1.0, 1.0, 1.0);
  itf->AddRGBPoint(6000, 1.0, 1.0, 1.0);
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
  props->SetSamplingDistance(0.05);
  DRRPropsType::PointType drrFocalSpot;
  drrFocalSpot[0] = 0;
  drrFocalSpot[1] = 0;
  drrFocalSpot[2] = 1000;
  props->SetSourceFocalSpotPosition(drrFocalSpot);
  TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();
  drrFilter->SetTransform(transform);
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Computing and verifying projections ... ")
  const int NUM_OF_TRANSFORMS = 5;
  for (int mode = 0; mode <= 9; mode++)
  {
    volume = GeneratePhantom(mode); // new phantom mode
    if (!volume)
    {
      VERBOSE(<< "\n    CANNOT GENERATE VOLUME (mode=" << mode << ")\n")
      ok = false;
      continue;
    }
    drrOrientation[0][0] = 1;
    drrOrientation[0][1] = 0;
    drrOrientation[0][2] = 0;
    drrOrientation[1][0] = 0;
    drrOrientation[1][1] = 1;
    drrOrientation[1][2] = 0;
    drrOrientation[2][0] = 0;
    drrOrientation[2][1] = 0;
    drrOrientation[2][2] = 1;
    props->SetProjectionPlaneOrientation(drrOrientation);
    drrOrigin[0] = -170;
    drrOrigin[1] = -121.25;
    drrOrigin[2] = -500;
    props->SetProjectionPlaneOrigin(drrOrigin);
    drrSize[0] = 2100;
    drrSize[1] = 2100;
    props->SetProjectionSize(drrSize);
    drrSpacing[0] = 0.125;
    drrSpacing[1] = 0.125;
    props->SetProjectionSpacing(drrSpacing);
    char buff[100];
    srand(time(NULL));
    for (int i = 0; i < NUM_OF_TRANSFORMS; i++)
    {
      TransformType::ParametersType pars(6);
      pars.Fill(0);
      if (i > 0)
      {
        pars[0] += ((double) (rand() % 101 - 50)) / 300.;
        pars[1] += ((double) (rand() % 101 - 50)) / 300.;
        pars[2] += ((double) (rand() % 101 - 50)) / 300.;
        pars[3] += ((double) (rand() % 101 - 50)) / 10.;
        pars[4] += ((double) (rand() % 101 - 50)) / 10.;
        pars[5] += ((double) (rand() % 101 - 50)) / 5.;
      }
      transform->SetParameters(pars);
      sprintf(buff, "drr_A_mode%d_trans%d.mhd", mode, (i + 1));
      if (!ComputeAndVerifyProjection(drrFilter, props, volume, buff))
        ok = false;
    }
    drrFocalSpot[0] = 50;
    drrFocalSpot[1] = -20;
    drrFocalSpot[2] = 1000;
    props->SetSourceFocalSpotPosition(drrFocalSpot);
    drrOrigin[0] = -172;
    drrOrigin[1] = -118;
    drrOrigin[2] = -500;
    props->SetProjectionPlaneOrigin(drrOrigin);
    for (int i = 0; i < NUM_OF_TRANSFORMS; i++)
    {
      TransformType::ParametersType pars(6);
      pars.Fill(0);
      if (i > 0)
      {
        pars[0] += ((double) (rand() % 101 - 50)) / 400.;
        pars[1] += ((double) (rand() % 101 - 50)) / 400.;
        pars[2] += ((double) (rand() % 101 - 50)) / 400.;
        pars[3] += ((double) (rand() % 101 - 50)) / 30.;
        pars[4] += ((double) (rand() % 101 - 50)) / 30.;
        pars[5] += ((double) (rand() % 101 - 50)) / 20.;
      }
      transform->SetParameters(pars);
      sprintf(buff, "drr_B_mode%d_trans%d.mhd", mode, (i + 1));
      if (!ComputeAndVerifyProjection(drrFilter, props, volume, buff))
        ok = false;
    }

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
    for (int i = 0; i < NUM_OF_TRANSFORMS; i++)
    {
      TransformType::ParametersType pars(6);
      pars.Fill(0);
      if (i > 0)
      {
        pars[0] += ((double) (rand() % 101 - 50)) / 400.;
        pars[1] += ((double) (rand() % 101 - 50)) / 400.;
        pars[2] += ((double) (rand() % 101 - 50)) / 400.;
        pars[3] += ((double) (rand() % 101 - 50)) / 30.;
        pars[4] += ((double) (rand() % 101 - 50)) / 30.;
        pars[5] += ((double) (rand() % 101 - 50)) / 20.;
      }
      transform->SetParameters(pars);
      sprintf(buff, "drr_C_mode%d_trans%d.mhd", mode, (i + 1));
      if (!ComputeAndVerifyProjection(drrFilter, props, volume, buff))
        ok = false;
    }
  }
  props = NULL;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

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
