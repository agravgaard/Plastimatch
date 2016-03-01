//
#include "BasicUnitTestIncludes.hxx"

#include <sstream>
#include <time.h>
#include <math.h>

#include "vtkORAStructureReader.h"
#include "vtkSurfaceToPerspectiveProjectionImageFilter.h"

#include <vtksys/SystemTools.hxx>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkMatrix3x3.h>
#include <vtkMetaImageWriter.h>
#include <vtkImageData.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkSphereSource.h>
#include <vtkPlane.h>
#include <vtkMath.h>


// extended message output
bool ExtendedOutput = false;

/** Write out image data to file (MHD-format) if flag is set. **/
void WriteImageData(vtkImageData *i, std::string fileName)
{
  if (ImageOutput)
  {
    vtkSmartPointer<vtkMetaImageWriter> ww =
        vtkSmartPointer<vtkMetaImageWriter>::New();
    ww->SetInput(i);
    ww->SetFileName(fileName.c_str());
    ww->Write();
  }
}

/** Add a multi-component surface to imager for geometry checks.  **/
double PhantPos[9][3];
void AddPhantomSurface(vtkSurfaceToPerspectiveProjectionImageFilter *f,
    vtkTransform *t)
{
  if (!f)
    return;

  PhantPos[0][0] = 11;
  PhantPos[0][1] = 0;
  PhantPos[0][2] = 0;
  PhantPos[1][0] = 3;
  PhantPos[1][1] = 73;
  PhantPos[1][2] = 6;
  PhantPos[2][0] = 95;
  PhantPos[2][1] = 87;
  PhantPos[2][2] = 4;
  PhantPos[3][0] = 80;
  PhantPos[3][1] = 6;
  PhantPos[3][2] = 11;
  PhantPos[4][0] = 0;
  PhantPos[4][1] = 90;
  PhantPos[4][2] = 80;
  PhantPos[5][0] = 100;
  PhantPos[5][1] = 90;
  PhantPos[5][2] = 75;
  PhantPos[6][0] = 100;
  PhantPos[6][1] = 0;
  PhantPos[6][2] = 69;
  PhantPos[7][0] = 50;
  PhantPos[7][1] = 45;
  PhantPos[7][2] = 40;
  PhantPos[8][0] = 0;
  PhantPos[8][1] = 0;
  PhantPos[8][2] = 71;

  double c[3];
  c[0] = PhantPos[7][0] * .8;
  c[1] = PhantPos[7][1] * .9;
  c[2] = PhantPos[7][2] * 1.1;
  for (int i = 0; i < 9; i++)
  {
    vtkSmartPointer<vtkSphereSource> sphere =
        vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetRadius(1.0);
    sphere->SetPhiResolution(20);
    sphere->SetThetaResolution(20);
    PhantPos[i][0] -= c[0];
    PhantPos[i][1] -= c[1];
    PhantPos[i][2] -= c[2];
    t->TransformPoint(PhantPos[i], PhantPos[i]);
    sphere->SetCenter(PhantPos[i]);
    f->AddInput(sphere->GetOutput());
  }
}

/** Verify the projected phantom geometry in the image. **/
bool VerifyProjectionGeometry(vtkImageData *image, vtkMatrix3x3 *orient,
    double *sourcePos, char *context)
{
  bool succ = true;
  double maxSp = image->GetSpacing()[0];
  if (image->GetSpacing()[1] > maxSp)
    maxSp = image->GetSpacing()[1];
  const double MAX_ALLOWED_ERROR = maxSp * 2.5;
  const double MAX_ALLOWED_SINGLE_ERROR = maxSp * 10.;

  double n[3];
  n[0] = orient->GetElement(2, 0);
  n[1] = orient->GetElement(2, 1);
  n[2] = orient->GetElement(2, 2);
  double v1[3];
  v1[0] = orient->GetElement(0, 0);
  v1[1] = orient->GetElement(0, 1);
  v1[2] = orient->GetElement(0, 2);
  double v2[3];
  v2[0] = orient->GetElement(1, 0);
  v2[1] = orient->GetElement(1, 1);
  v2[2] = orient->GetElement(1, 2);
  double pc, rc, xc;
  double x[3];
  int px, py;
  double maxError = -1;
  double minError = 1e7;
  double meanError = 0;
  std::vector<double> errors;
  std::vector<std::string> errorPairs;

  for (int i = 0; i < 9; i++)
  {
    vtkPlane::IntersectWithLine(sourcePos, PhantPos[i], n, image->GetOrigin(),
        pc, x);
    // x pixel
    rc = vtkMath::Dot(image->GetOrigin(), v1); // ref-coordinate
    xc = vtkMath::Dot(x, v1); // ref-coordinate
    px = floor(fabs(xc - rc) / image->GetSpacing()[0]);
    // y pixel
    rc = vtkMath::Dot(image->GetOrigin(), v2); // ref-coordinate
    xc = vtkMath::Dot(x, v2); // ref-coordinate
    py = floor(fabs(xc - rc) / image->GetSpacing()[1]);

//    std::cout << i << ": " << x[0] << "," << x[1] << "," << x[2] << " -> " << px << "," << py << std::endl;

    // NOTE: maximum search radius is half of image plane width/height
    int maxradius = (image->GetWholeExtent()[1] + 1) / 2;
    if ((int) ((image->GetWholeExtent()[3] + 1) / 2) > maxradius)
      maxradius = (image->GetWholeExtent()[3] + 1) / 2;
    int radius = -1, c, neighborhood = 0, px2, py2;
    int *nidxX = NULL;
    int *nidxY = NULL;
    unsigned char *ia = NULL;
    bool seedFound = false;
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
        px2 = px + nidxX[u];
        py2 = py + nidxY[u];
        if (px2 < 0 || py2 < 0 || px2 > image->GetWholeExtent()[1] || py2
            > image->GetWholeExtent()[3])
          continue;
        ia = (unsigned char *) image->GetScalarPointer(px2, py2, 0);
        if (ia[0] > 0)
        {
//          std::cout << "  seed: " << px2 << "," << py2 << std::endl;
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
      double dpoint[3];
      int seed[3];
      seed[0] = px2;
      seed[1] = py2;
      seed[2] = 0;
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
          px2 = seed[0] + nidxX[u];
          py2 = seed[1] + nidxY[u];
          if (px2 < 0 || py2 < 0 || px2 > image->GetWholeExtent()[1] || py2
              > image->GetWholeExtent()[3])
            continue;
          ia = (unsigned char *) image->GetScalarPointer(px2, py2, 0);
          if (ia[0] > 0)
          {
            pixelFound = true; // found (contained in px,py)
            wsum += (double) ia[0];

            dpoint[0] = (double) px2 * image->GetSpacing()[0] * v1[0]
                + (double) py2 * image->GetSpacing()[1] * v2[0]
                + image->GetOrigin()[0];
            dpoint[1] = (double) px2 * image->GetSpacing()[0] * v1[1]
                + (double) py2 * image->GetSpacing()[1] * v2[1]
                + image->GetOrigin()[1];
            dpoint[2] = (double) px2 * image->GetSpacing()[0] * v1[2]
                + (double) py2 * image->GetSpacing()[1] * v2[2]
                + image->GetOrigin()[2];

            centroid[0] += (double) dpoint[0] * (double) ia[0];
            centroid[1] += (double) dpoint[1] * (double) ia[0];
            centroid[2] += (double) dpoint[2] * (double) ia[0];
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
      double errDist = vtkMath::Distance2BetweenPoints(centroid, x);
      errDist = sqrt(errDist);
      if (errDist > maxError)
        maxError = errDist;
      if (errDist < minError)
        minError = errDist;
      meanError += errDist;
      errors.push_back(errDist);
      char sb[300];
      sprintf(sb, "%f,%f,%f (detected) vs. %f,%f,%f (calculated)",
          centroid[0], centroid[1], centroid[2], x[0], x[1], x[2]);
      errorPairs.push_back(std::string(sb));
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
      char sb[300];
      sprintf(sb, "<not found> (detected) vs. %f,%f,%f (calculated)",
          x[0], x[1], x[2]);
      errorPairs.push_back(std::string(sb));
    }
  }
  if (maxError > MAX_ALLOWED_ERROR)
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
  meanError /= 9.0;
  double errorSTD = 0;
  for (unsigned int i = 0; i < errors.size(); i++)
  {
    errorSTD += (errors[i] - meanError) * (errors[i] - meanError);
  }
  if (errors.size() > 1)
    errorSTD /= (double) (errors.size() - 1);
  errorSTD = sqrt(errorSTD);
  if (!succ || ExtendedOutput)
  {
    bool store = Verbose;
    if (!succ)
    {
      Verbose = true;
      VERBOSE(<< "\n    *** ERROR(!!!) " << context << " ****\n")
    }
    else
    {
      VERBOSE(<< "\n    " << context << "\n")
    }
    VERBOSE(<< "     - verification: " << (succ ? "OK" : "FAILURE") << "\n")
    VERBOSE(<< "     - tolerance: " << (MAX_ALLOWED_ERROR * 1000) << " um\n")
    if (maxError < 9e6)
    {
      VERBOSE(<< "     - number of markers: " << 9 << "\n")
      VERBOSE(<< "     - max. error: " << (maxError * 1000) << " um\n")
      VERBOSE(<< "     - min. error: " << (minError * 1000) << " um\n")
      VERBOSE(<< "     - mean: " << (meanError * 1000) << " um\n")
      VERBOSE(<< "     - STD: " << (errorSTD * 1000) << " um\n")
    }
    else
    {
      VERBOSE(<< "     - OUTPUT IMAGE APPEARS TO HAVE SOLELY ZERO PIXELS\n")
    }
    if (!succ)
    {
      for (std::size_t k = 0; k < errors.size(); k++)
        VERBOSE(<< "     (marker " << (k + 1) << ") " << (errors[k] * 1000) << " um, pair: " << errorPairs[k] << "\n")
    }
    if (!succ)
      Verbose = store;
  }

  return succ;
}

/**
 * Tests base functionality of:
 *
 *   vtkSurfaceToPerspectiveProjectionImageFilter
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see vtkORAStructureReader
 * @see vtkSurfaceToPerspectiveProjectionImageFilter
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
      "  -xo or --extended-output ... extended (more detailed message output)");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, true, true);
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

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting Surface to Perspective Projection Image Filter.\n")

  VERBOSE(<< "  * Check test data availability ... ")
  lok = true; // initialize sub-section's success state
  std::string struct3DFile = DataPath + "3DStructureInfo.inf";
  if (!vtksys::SystemTools::FileExists(struct3DFile.c_str()))
    lok = false;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  if (ok) // we need data for these tests!
  {
    VERBOSE(<< "  * Basic functional test ... ")
    lok = true; // initialize sub-section's success state
    vtkSmartPointer<vtkORAStructureReader> sr = vtkSmartPointer<
        vtkORAStructureReader>::New();
    sr->SetGenerateSurfaceNormals(true);
    sr->SetReadColorsAsWell(false); // no color
    sr->SetFileName(struct3DFile.c_str());
    sr->Update();
    if (!sr->GetOutput())
      lok = false;
    vtkSmartPointer<vtkRenderWindow> renWin =
        vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkSurfaceToPerspectiveProjectionImageFilter> imager =
        vtkSmartPointer<vtkSurfaceToPerspectiveProjectionImageFilter>::New();
    if (!imager->ConnectToRenderWindow(renWin))
      lok = false;
    imager->AddInput(sr->GetOutput());
    if (imager->GetNumberOfInputs() <= 0)
      lok = false;
    if (imager->IsProjectionGeometryValid())
      lok = false;
    double srcPos[3];
    srcPos[0] = 0;
    srcPos[1] = 0;
    srcPos[2] = 1000;
    imager->SetSourcePosition(srcPos);
    if (imager->IsProjectionGeometryValid())
      lok = false;
    double plnOrig[3];
    plnOrig[0] = -205;
    plnOrig[1] = -205;
    plnOrig[2] = -500;
    imager->SetPlaneOrigin(plnOrig);
    if (imager->IsProjectionGeometryValid())
      lok = false;
    int plnSz[2];
    plnSz[0] = 1024;
    plnSz[1] = 1024;
    imager->SetPlaneSizePixels(plnSz);
    if (imager->IsProjectionGeometryValid())
      lok = false;
    double plnSp[2];
    plnSp[0] = 0.400390625;
    plnSp[1] = 0.400390625;
    imager->SetPlaneSpacing(plnSp);
    if (imager->IsProjectionGeometryValid())
      lok = false;
    vtkSmartPointer<vtkMatrix3x3> plnOrient =
        vtkSmartPointer<vtkMatrix3x3>::New();
    plnOrient->Identity();
    imager->SetPlaneOrientation(plnOrient);
    if (!imager->IsProjectionGeometryValid())
      lok = false;
    imager->Update();
    if (!imager->GetOutput())
      lok = false;
    vtkImageData *maskImage = imager->GetOutput();
    WriteImageData(maskImage, "mask_image_1comp.mhd");
    vtkSmartPointer<vtkTransformPolyDataFilter> tf = vtkSmartPointer<
        vtkTransformPolyDataFilter>::New();
    tf->SetInput(sr->GetOutput());
    vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
    t->Translate(-10, 20, 30);
    t->RotateZ(5);
    tf->SetTransform(t);
    tf->Update();
    imager->AddInput(tf->GetOutput());
    imager->Update();
    if (!imager->GetOutput())
      lok = false;
    maskImage = imager->GetOutput();
    WriteImageData(maskImage, "mask_image_2comp.mhd");
    std::ostringstream os;
    imager->Print(os);
    if (os.str().length() <= 0)
      lok = false;
    ok = ok && lok; // update OK-flag for test-scope
    VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

    VERBOSE(<< "  * Projection geometry checks ... ")
    lok = true; // initialize sub-section's success state
    vtkSmartPointer<vtkTransform> phantTrans =
        vtkSmartPointer<vtkTransform>::New();
    srand(time(NULL));
    // mode A: "ordinary" geometry
    srcPos[0] = 0;
    srcPos[1] = 0;
    srcPos[2] = 1000;
    imager->SetSourcePosition(srcPos);
    plnOrig[0] = -205;
    plnOrig[1] = -205;
    plnOrig[2] = -500;
    imager->SetPlaneOrigin(plnOrig);
    plnSz[0] = 1024;
    plnSz[1] = 1024;
    imager->SetPlaneSizePixels(plnSz);
    plnSp[0] = 0.400390625;
    plnSp[1] = 0.400390625;
    imager->SetPlaneSpacing(plnSp);
    plnOrient->Identity();
    imager->SetPlaneOrientation(plnOrient);
    if (!imager->IsProjectionGeometryValid())
      lok = false;
    for (int u = 0; u < 5; u++)
    {
      phantTrans->Identity();
      phantTrans->Translate((5.0 - (double) (rand() % 100001) / 7000), (5.0
          - (double) (rand() % 100001) / 7000), (5.0 - (double) (rand()
          % 100001) / 7000));
      phantTrans->RotateX((5.0 - (double) (rand() % 100001) / 7000));
      phantTrans->RotateY((5.0 - (double) (rand() % 100001) / 7000));
      phantTrans->RotateZ((5.0 - (double) (rand() % 100001) / 7000));
      imager->RemoveAllInputs();
      AddPhantomSurface(imager, phantTrans);
      imager->Update();
      if (!imager->GetOutput())
        lok = false;
      maskImage = imager->GetOutput();
      char buff[100];
      sprintf(buff, "mask_image_phantom_modeA_%d.mhd", u);
      if (!VerifyProjectionGeometry(maskImage, plnOrient, srcPos, buff))
        lok = false;
      WriteImageData(maskImage, buff); // after verify! (sets back image props!)
    }
    // mode B: off-axis projection (shifted source position)
    srcPos[0] = -20;
    srcPos[1] = 40;
    srcPos[2] = 800;
    imager->SetSourcePosition(srcPos);
    plnOrig[0] = -205;
    plnOrig[1] = -205;
    plnOrig[2] = -500;
    imager->SetPlaneOrigin(plnOrig);
    plnSz[0] = 1024;
    plnSz[1] = 1024;
    imager->SetPlaneSizePixels(plnSz);
    plnSp[0] = 0.400390625;
    plnSp[1] = 0.400390625;
    imager->SetPlaneSpacing(plnSp);
    plnOrient->Identity();
    imager->SetPlaneOrientation(plnOrient);
    if (!imager->IsProjectionGeometryValid())
      lok = false;
    for (int u = 0; u < 5; u++)
    {
      phantTrans->Identity();
      phantTrans->Translate((5.0 - (double) (rand() % 100001) / 7000), (5.0
          - (double) (rand() % 100001) / 7000), (5.0 - (double) (rand()
          % 100001) / 7000));
      phantTrans->RotateX((5.0 - (double) (rand() % 100001) / 7000));
      phantTrans->RotateY((5.0 - (double) (rand() % 100001) / 7000));
      phantTrans->RotateZ((5.0 - (double) (rand() % 100001) / 7000));
      imager->RemoveAllInputs();
      AddPhantomSurface(imager, phantTrans);
      imager->Update();
      if (!imager->GetOutput())
        lok = false;
      maskImage = imager->GetOutput();
      char buff[100];
      sprintf(buff, "mask_image_phantom_modeB_%d.mhd", u);
      if (!VerifyProjectionGeometry(maskImage, plnOrient, srcPos, buff))
        lok = false;
      WriteImageData(maskImage, buff); // after verify! (sets back image props!)
    }
    // mode C: shifted image plane in addition
    srcPos[0] = -20;
    srcPos[1] = 40;
    srcPos[2] = 800;
    imager->SetSourcePosition(srcPos);
    plnOrig[0] = -210;
    plnOrig[1] = -195;
    plnOrig[2] = -520;
    imager->SetPlaneOrigin(plnOrig);
    plnSz[0] = 1024;
    plnSz[1] = 1024;
    imager->SetPlaneSizePixels(plnSz);
    plnSp[0] = 0.400390625;
    plnSp[1] = 0.400390625;
    imager->SetPlaneSpacing(plnSp);
    plnOrient->Identity();
    imager->SetPlaneOrientation(plnOrient);
    if (!imager->IsProjectionGeometryValid())
      lok = false;
    for (int u = 0; u < 5; u++)
    {
      phantTrans->Identity();
      phantTrans->Translate((5.0 - (double) (rand() % 100001) / 7000), (5.0
          - (double) (rand() % 100001) / 7000), (5.0 - (double) (rand()
          % 100001) / 7000));
      phantTrans->RotateX((5.0 - (double) (rand() % 100001) / 7000));
      phantTrans->RotateY((5.0 - (double) (rand() % 100001) / 7000));
      phantTrans->RotateZ((5.0 - (double) (rand() % 100001) / 7000));
      imager->RemoveAllInputs();
      AddPhantomSurface(imager, phantTrans);
      imager->Update();
      if (!imager->GetOutput())
        lok = false;
      maskImage = imager->GetOutput();
      char buff[100];
      sprintf(buff, "mask_image_phantom_modeC_%d.mhd", u);
      if (!VerifyProjectionGeometry(maskImage, plnOrient, srcPos, buff))
        lok = false;
      WriteImageData(maskImage, buff); // after verify! (sets back image props!)
    }
    // mode D: non-quadratic image plane
    srcPos[0] = -20;
    srcPos[1] = 40;
    srcPos[2] = 800;
    imager->SetSourcePosition(srcPos);
    plnOrig[0] = -210;
    plnOrig[1] = -195;
    plnOrig[2] = -520;
    imager->SetPlaneOrigin(plnOrig);
    plnSz[0] = 950;
    plnSz[1] = 1089;
    imager->SetPlaneSizePixels(plnSz);
    plnSp[0] = 0.400390625;
    plnSp[1] = 0.400390625;
    imager->SetPlaneSpacing(plnSp);
    plnOrient->Identity();
    imager->SetPlaneOrientation(plnOrient);
    if (!imager->IsProjectionGeometryValid())
      lok = false;
    for (int u = 0; u < 5; u++)
    {
      phantTrans->Identity();
      phantTrans->Translate((5.0 - (double) (rand() % 100001) / 7000), (5.0
          - (double) (rand() % 100001) / 7000), (5.0 - (double) (rand()
          % 100001) / 7000));
      phantTrans->RotateX((5.0 - (double) (rand() % 100001) / 7000));
      phantTrans->RotateY((5.0 - (double) (rand() % 100001) / 7000));
      phantTrans->RotateZ((5.0 - (double) (rand() % 100001) / 7000));
      imager->RemoveAllInputs();
      AddPhantomSurface(imager, phantTrans);
      imager->Update();
      if (!imager->GetOutput())
        lok = false;
      maskImage = imager->GetOutput();
      char buff[100];
      sprintf(buff, "mask_image_phantom_modeD_%d.mhd", u);
      if (!VerifyProjectionGeometry(maskImage, plnOrient, srcPos, buff))
        lok = false;
      WriteImageData(maskImage, buff); // after verify! (sets back image props!)
    }
    // mode E: tilted image plane, "lateral" geometry
    srcPos[0] = 1000;
    srcPos[1] = 20;
    srcPos[2] = 10;
    imager->SetSourcePosition(srcPos);
    plnOrig[0] = -510;
    plnOrig[1] = -195;
    plnOrig[2] = 200;
    imager->SetPlaneOrigin(plnOrig);
    plnSz[0] = 980;
    plnSz[1] = 1024;
    imager->SetPlaneSizePixels(plnSz);
    plnSp[0] = 0.400390625;
    plnSp[1] = 0.400390625;
    imager->SetPlaneSpacing(plnSp);
    for (int u = 0; u < 5; u++)
    {
      phantTrans->Identity();
      phantTrans->Translate((5.0 - (double) (rand() % 100001) / 7000), (5.0
          - (double) (rand() % 100001) / 7000), (5.0 - (double) (rand()
          % 100001) / 7000));
      phantTrans->RotateX((5.0 - (double) (rand() % 100001) / 7000));
      phantTrans->RotateY((5.0 - (double) (rand() % 100001) / 7000));
      phantTrans->RotateZ((5.0 - (double) (rand() % 100001) / 7000));
      vtkSmartPointer<vtkTransform> tt = vtkSmartPointer<vtkTransform>::New();
      tt->Identity();
      double r = (5.0 - (double) (rand() % 100001) / 7000);
      tt->RotateX(r);
      r = (5.0 - (double) (rand() % 100001) / 7000);
      tt->RotateY(r);
      r = (5.0 - (double) (rand() % 100001) / 7000);
      tt->RotateZ(r);
      double h[3];
      h[0] = 0; h[1] = 0; h[2] = -1;
      plnOrient->SetElement(0, 0, tt->TransformVector(h)[0]);
      plnOrient->SetElement(0, 1, tt->TransformVector(h)[1]);
      plnOrient->SetElement(0, 2, tt->TransformVector(h)[2]);
      h[0] = 0; h[1] = 1; h[2] = 0;
      plnOrient->SetElement(1, 0, tt->TransformVector(h)[0]);
      plnOrient->SetElement(1, 1, tt->TransformVector(h)[1]);
      plnOrient->SetElement(1, 2, tt->TransformVector(h)[2]);
      h[0] = 1; h[1] = 0; h[2] = 0;
      plnOrient->SetElement(2, 0, tt->TransformVector(h)[0]);
      plnOrient->SetElement(2, 1, tt->TransformVector(h)[1]);
      plnOrient->SetElement(2, 2, tt->TransformVector(h)[2]);
      imager->SetPlaneOrientation(plnOrient);
      if (!imager->IsProjectionGeometryValid())
        lok = false;

      imager->RemoveAllInputs();
      AddPhantomSurface(imager, phantTrans);
      imager->Update();
      if (!imager->GetOutput())
        lok = false;
      maskImage = imager->GetOutput();
      char buff[100];
      sprintf(buff, "mask_image_phantom_modeE_%d.mhd", u);
      if (!VerifyProjectionGeometry(maskImage, plnOrient, srcPos, buff))
        lok = false;
      WriteImageData(maskImage, buff); // after verify! (sets back image props!)
    }
    ok = ok && lok; // update OK-flag for test-scope
    VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")
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
