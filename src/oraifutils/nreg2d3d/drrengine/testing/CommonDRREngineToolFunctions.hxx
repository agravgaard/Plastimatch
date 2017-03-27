//
#ifndef COMMONDRRENGINETOOLFUNCTIONS_HXX_
#define COMMONDRRENGINETOOLFUNCTIONS_HXX_
//
#include <math.h>
#include <cstddef> /* Workaround bug in ITK 3.20 */
//ITK
#include <itkImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageFileWriter.h>
#include <itkEuler3DTransform.h>
//ORAIFUTILS
#include "oraDRRFilter.h"
#include "oraProjectionGeometry.h"
#include "oraIntensityTransferFunction.h"

// Implements some tool functions supporting the regression tests independent
// of VTK library.
// @author phil
// @version 1.0

namespace ora
{


// typedefs
typedef unsigned short PhantomVolumePixelType;
typedef itk::Image<PhantomVolumePixelType, 3> PhantomVolumeImageType;
typedef itk::ImageRegionIteratorWithIndex<PhantomVolumeImageType> PhantomVolumeIteratorType;
typedef itk::ImageFileWriter<PhantomVolumeImageType> PhantomVolumeWriterType;
typedef itk::Euler3DTransform<double> TransformType;
typedef float DRRPixelType;
typedef ora::DRRFilter<ora::PhantomVolumePixelType, DRRPixelType> GenericDRRFilterType;
typedef GenericDRRFilterType::OutputImageType GenericDRRImageType;
typedef itk::ImageFileWriter<GenericDRRImageType> GenericDRRWriterType;
typedef itk::ImageRegionIteratorWithIndex<GenericDRRImageType> GenericDRRIteratorType;
typedef ora::ProjectionGeometry GeometryType;
typedef ora::IntensityTransferFunction ITFType;

/** Compute the norm of vector v. **/
double Norm(const double v[3])
{
  return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

/** Normalize vector v, and simultaneously return the v's norm. **/
double NormalizeVector(double v[3])
{
  double den;
  if ((den = Norm(v)) != 0.0)
  {
    for (int i=0; i < 3; i++)
      v[i] /= den;
  }
  return den;
}

/** Compute dot product between vectors x and y. **/
double ComputeDot(const double x[3], const double y[3])
{
  return (x[0] * y[0] + x[1] * y[1] + x[2] * y[2]);
}

/** Compute cross product of vectors x and y. **/
void ComputeCross(const double x[3], const double y[3], double z[3])
{
  z[0] = x[1] * y[2] - x[2] * y[1];
  z[1] = x[2] * y[0] - x[0] * y[2];
  z[2] = x[0] * y[1] - x[1] * y[0];
}

/** Compute the distance between two 3D points. **/
double ComputeDistanceBetween3DPoints(const double x[3], const double y[3])
{
  double dist = sqrt((x[0] - y[0]) * (x[0] - y[0])
         + (x[1] - y[1]) * (x[1] - y[1])
         + (x[2] - y[2]) * (x[2] - y[2]));
  return dist;
}

/** Given a line defined by the two 3D points p1 and p2, and a plane defined by
 * the normal n and plane origin p0, compute an intersection. The coordinates of
 * the intersection are returned in x. FALSE is returned if the plane and line
 * do not intersect. **/
bool IntersectLineWithPlane(const double p1[3], const double p2[3],
    const double p0[3], const double n[3], double x[3])
{
  double num, den, t, p21[3];
  double fabsden, fabstolerance;
  // Compute line vector
  p21[0] = p2[0] - p1[0];
  p21[1] = p2[1] - p1[1];
  p21[2] = p2[2] - p1[2];
  // Compute denominator. If ~0, line and plane are parallel.
  num = ComputeDot(n, p0) - (n[0] * p1[0] + n[1] * p1[1] + n[2] * p1[2]);
  den = n[0] * p21[0] + n[1] * p21[1] + n[2] * p21[2];
  // If denominator with respect to numerator is "zero", then the line and
  // plane are considered parallel.
  // trying to avoid an expensive call to fabs()
  if (den < 0.0)
    fabsden = -den;
  else
    fabsden = den;
  if (num < 0.0)
    fabstolerance = -num * 1.0e-06;
  else
    fabstolerance = num * 1.0e-06;
  if (fabsden <= fabstolerance)
    return false;
  // valid intersection
  t = num / den;
  x[0] = p1[0] + t * p21[0];
  x[1] = p1[1] + t * p21[1];
  x[2] = p1[2] + t * p21[2];
  return true;
}

/**
 * Generate a test phantom containing 9 single-voxeled fiducials.
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
PhantomVolumeImageType::Pointer GenerateFiducialPhantom(int mode, bool imageOutput)
{
  if (mode < 0 || mode > 9)
    return NULL;

  PhantomVolumeImageType::SizeType isize;
  isize[0] = 101;
  isize[1] = 91;
  isize[2] = 81;
  PhantomVolumeImageType::IndexType iindex;
  iindex[0] = 0;
  iindex[1] = 0;
  iindex[2] = 0;
  PhantomVolumeImageType::RegionType iregion;
  iregion.SetIndex(iindex);
  iregion.SetSize(isize);
  PhantomVolumeImageType::SpacingType ispacing;
  ispacing[0] = 0.5;
  ispacing[1] = 1.0;
  ispacing[2] = 0.7;
  PhantomVolumeImageType::DirectionType idirection;
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
  PhantomVolumeImageType::PointType iorigin;
  iorigin[0] = 0;
  iorigin[1] = 0;
  iorigin[2] = 0;
  double vec[3];
  for (int d = 0; d < 3; d++) // primarily centered
  {
    vec[0] = idirection[0][d];
    vec[1] = idirection[1][d];
    vec[2] = idirection[2][d];
    ora::NormalizeVector(vec);
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
  PhantomVolumeImageType::Pointer volume = PhantomVolumeImageType::New();
  volume->SetSpacing(ispacing);
  volume->SetOrigin(iorigin);
  volume->SetDirection(idirection);
  volume->SetRegions(iregion);
  volume->Allocate();
  volume->FillBuffer(0);
  PhantomVolumeIteratorType it(volume, iregion);
  PhantomVolumePixelType v = 5000;
  PhantomVolumeImageType::IndexType p;
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
  if (imageOutput)
  {
    PhantomVolumeWriterType::Pointer w = PhantomVolumeWriterType::New();
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

/** Compute DRR w.r.t. current projection properties of specified DRR filter and
 * verify it with analytically calculated phantom projection data.
 * @param drrFilter DRR filter (should have valid projection geometry on output
 * index 0)
 * @param volume phantom volume
 * @param fname file name for DRR output (if image output is activated)
 * @param imageOutput write out images flag
 * @param extendedOutput more output (verbose) flag
 * @param failureWriteOutput write failure output info flag
 */
bool ComputeAndVerifyProjection(ora::GenericDRRFilterType *drrFilter,
    ora::PhantomVolumeImageType::Pointer volume, const char *fname,
    bool imageOutput, bool extendedOutput, bool failureWriteOutput,
    const double maxAllowedErrorFactor = 2.5,
    const double maxAllowedSingleError = 10.)
{
  if (extendedOutput)
    std::cout << "\n    " << fname << "\n";
  if (!drrFilter || !volume)
    return false;

  ora::GeometryType::Pointer geom = drrFilter->GetProjectionGeometry(0);
  if (!geom || !geom->IsGeometryValid())
  {
    std::cout << "\n    invalid geometry!\n";
    return false;
  }
  const double *sp = geom->GetDetectorPixelSpacing();
  double maxSp = sp[0];
  if (sp[1] > maxSp)
    maxSp = sp[1];
  // MAXIMUM TOLERATED ERROR: (DRR-spacing-dependent; times 1.5 because weighted
  // centroid estimation is not perfect)
  double MAX_ALLOWED_ERROR = maxSp * maxAllowedErrorFactor;
  double MAX_ALLOWED_SINGLE_ERROR = maxSp * maxAllowedSingleError;

  bool succ = true;
  drrFilter->SetInput(volume); // be sure it is set
  drrFilter->SetCurrentDRROutputIndex(0);
  ora::GenericDRRImageType::Pointer drr = NULL;
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
    if (imageOutput)
    {
      ora::GenericDRRWriterType::Pointer w = ora::GenericDRRWriterType::New();
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
    ora::PhantomVolumeIteratorType it(volume, volume->GetLargestPossibleRegion());
    ora::PhantomVolumeImageType::IndexType idx;
    ora::PhantomVolumeImageType::PointType point;
    ora::GenericDRRImageType::IndexType computedIdx;
    ora::GenericDRRImageType::PointType dp;
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

        const double *fs = geom->GetSourcePosition();
        const double *rdir = geom->GetDetectorRowOrientation();
        const double *cdir = geom->GetDetectorColumnOrientation();
        const double *po = geom->GetDetectorOrigin();
        double n[3];
        ora::ComputeCross(rdir, cdir, n);
        double p[3];
        p[0] = point[0];
        p[1] = point[1];
        p[2] = point[2];
        double x[3];
        ora::IntersectLineWithPlane(fs, p, po, n, x);
        dp[0] = x[0];
        dp[1] = x[1];
        dp[2] = x[2];
        drr->TransformPhysicalPointToIndex(dp, computedIdx);
        // search for nearest first fiducial pixel in generated projection:
        ora::GenericDRRIteratorType dit(drr, drr->GetLargestPossibleRegion());
        ora::GenericDRRImageType::IndexType didx;
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
                    > static_cast<ora::GenericDRRImageType::IndexType::IndexValueType> (dit.GetRegion().GetIndex()[0]
                        + dit.GetRegion().GetSize()[0])
                || didx[1]
                    > static_cast<ora::GenericDRRImageType::IndexType::IndexValueType> (dit.GetRegion().GetIndex()[1]
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
          ora::GenericDRRImageType::PixelType dv;
          ora::GenericDRRImageType::PointType dpoint;
          ora::GenericDRRImageType::IndexType seed;
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
          double errDist = ora::ComputeDistanceBetween3DPoints(centroid, dpd);
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
    if (!imageOutput && failureWriteOutput && !succ)
    {
      ora::GenericDRRWriterType::Pointer w = ora::GenericDRRWriterType::New();
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
    if (extendedOutput)
    {
      std::cout << "     - verification: " << (succ ? "OK" : "FAILURE") << "\n";
      std::cout << "     - TRANSFORM: " << drrFilter->GetTransform()->GetParameters() << "\n";
      std::cout << "     - tolerance: " << (MAX_ALLOWED_ERROR * 1000) << " um\n";
      if (maxError < 9e6)
      {
        std::cout << "     - number of markers: " << numMarkers << "\n";
        std::cout << "     - max. error: " << (maxError * 1000) << " um\n";
        std::cout << "     - min. error: " << (minError * 1000) << " um\n";
        std::cout << "     - mean: " << (meanError * 1000) << " um\n";
        std::cout << "     - STD: " << (errorSTD * 1000) << " um\n";
      }
      else
      {
        std::cout << "     - OUTPUT DRR APPEARS TO HAVE SOLELY ZERO PIXELS\n";
      }
    }
  }
  else
  {
    succ = false;
  }

  return succ;
}

}

#endif /* COMMONDRRENGINETOOLFUNCTIONS_HXX_ */


