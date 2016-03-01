//
#include "BasicUnitTestIncludes.hxx"

#include <sstream>
#include <time.h>

#include "vtkORAStructureReader.h"
#include "vtkSurfaceToPerspectiveProjectionContourFilter.h"

#include <vtksys/SystemTools.hxx>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkMatrix3x3.h>
#include <vtkMetaImageWriter.h>
#include <vtkDataSetWriter.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkDataSetMapper.h>
#include <vtkAxesActor.h>
#include <vtkSuperquadricSource.h>


// poly-data output
bool PolyDataOutput = false;
// inter-active flag
bool InterActive = false;

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

/** Write out poly data to file (VTK-format) if flag is set. **/
void WritePolyData(vtkPolyData *pd, std::string fileName)
{
  if (PolyDataOutput)
  {
    vtkDataSetWriter *sw = vtkDataSetWriter::New();
    sw->SetFileName(fileName.c_str());
    sw->SetInput(pd);
    sw->Write();
    sw->Delete();
  }
}

/**
 * Display the specified image and poly data in a minimal VTK-window which blocks
 * execution until the user closes the window. Works only if inter-active mode
 * is ON.
 */
void DisplayImageAndPolyData(vtkPolyData *pd, vtkImageData *i,
    vtkMatrix3x3 *iorient, vtkPolyData *obj1, vtkPolyData *obj2,
    std::string title)
{
  if (InterActive)
  {
    vtkRenderWindow *renWin = vtkRenderWindow::New();
    renWin->SetSize(500, 500);
    vtkRenderer *ren = vtkRenderer::New();
    vtkRenderWindowInteractor *rwi = vtkRenderWindowInteractor::New();
    renWin->AddRenderer(ren);
    renWin->SetInteractor(rwi);
    renWin->SetWindowName(title.c_str());

    // poly data (contour):
    vtkActor *a = vtkActor::New();
    vtkPolyDataMapper *m = vtkPolyDataMapper::New();
    a->SetMapper(m);
    m->SetInput(pd);
    a->SetVisibility(true);
    a->GetProperty()->SetRepresentationToWireframe();
    a->GetProperty()->SetColor(1, 0, 0); // red contours
    m->SetScalarVisibility(false);
    ren->AddActor(a);

    // image data (mask image):
    vtkActor *a2 = vtkActor::New();
    vtkDataSetMapper *m2 = vtkDataSetMapper::New();
    a2->SetMapper(m2);
    m2->SetInput(i);
    a2->SetVisibility(true);
    m2->SetScalarVisibility(true);
    // -> account for image plane orientation:
    vtkSmartPointer<vtkTransform> ipot = vtkSmartPointer<vtkTransform>::New();
    vtkSmartPointer<vtkMatrix4x4> ipom = vtkSmartPointer<vtkMatrix4x4>::New();
    ipom->Identity();
    for (int d1 = 0; d1 < 3; d1++)
      for (int d2 = 0; d2 < 3; d2++)
      ipom->SetElement(d1, d2, iorient->GetElement(d1, d2));
    ipot->PostMultiply();
    ipot->Translate(-i->GetOrigin()[0], -i->GetOrigin()[1], -i->GetOrigin()[2]);
    ipom->Invert();
    ipot->Concatenate(ipom);
    ipot->Translate(i->GetOrigin()[0], i->GetOrigin()[1], i->GetOrigin()[2]);
    a2->SetUserTransform(ipot);
    ren->AddActor(a2);

    // poly data (object 1):
    if (obj1)
    {
      vtkActor *ao = vtkActor::New();
      vtkPolyDataMapper *mo = vtkPolyDataMapper::New();
      ao->SetMapper(mo);
      mo->SetInput(obj1);
      ao->SetVisibility(true);
      ao->GetProperty()->SetRepresentationToSurface();
      ao->GetProperty()->SetColor(0, 0, 1);
      mo->SetScalarVisibility(false);
      ren->AddActor(ao);
      ao->Delete();
      mo->Delete();
    }
    // poly data (object 2):
    if (obj2)
    {
      vtkActor *ao = vtkActor::New();
      vtkPolyDataMapper *mo = vtkPolyDataMapper::New();
      ao->SetMapper(mo);
      mo->SetInput(obj2);
      ao->SetVisibility(true);
      ao->GetProperty()->SetRepresentationToSurface();
      ao->GetProperty()->SetColor(0, 0, 1);
      mo->SetScalarVisibility(false);
      ren->AddActor(ao);
      ao->Delete();
      mo->Delete();
    }

    // axes (WCS):
    vtkAxesActor *ax = vtkAxesActor::New();
    ax->SetTotalLength(100, 100, 100);
    ren->AddActor(ax);

    // axes (obj):
    vtkAxesActor *ax2 = vtkAxesActor::New();
    ax2->SetTotalLength(50, 50, 50);
    ax2->SetAxisLabels(false);
    // -> account for image plane orientation:
    vtkSmartPointer<vtkTransform> ipot2 = vtkSmartPointer<vtkTransform>::New();
    ipot2->Translate(i->GetOrigin()[0], i->GetOrigin()[1], i->GetOrigin()[2]);
    ax2->SetUserTransform(ipot2);
    ren->AddActor(ax2);

    rwi->Initialize();
    ren->ResetCamera();
    renWin->Render();
    ren->SetBackground(0.5, 0.5, 0.5);
    rwi->Start();
    m->Delete();
    a->Delete();
    m2->Delete();
    a2->Delete();
    ax->Delete();
    ax2->Delete();
    ren->Delete();
    rwi->Delete();
    renWin->Delete();
  }
}

/**
 * Tests base functionality of:
 *
 *   vtkSurfaceToPerspectiveProjectionContourFilter
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see vtkORAStructureReader
 * @see vtkSurfaceToPerspectiveProjectionContourFilter
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
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
      "  -pdo or --poly-data-output ... flag indicating that test poly data should be written out (VTK-format)");
  helpLines.push_back(
      "  -ia or --inter-active ... flag indicating that a minimal VTK-based GUI is displayed - requires the user to close GUI!");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, true, true);
    return EXIT_SUCCESS;
  }
  // advanced arguments:
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-pdo" || std::string(argv[i])
        == "--poly-data-output")
    {
      PolyDataOutput = true;
      continue;
    }
    if (std::string(argv[i]) == "-ia" || std::string(argv[i])
        == "--inter-active")
    {
      InterActive = true;
      continue;
    }
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting Surface to Perspective Projection Contour Filter.\n")

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
    vtkSmartPointer<vtkSurfaceToPerspectiveProjectionContourFilter> contourer =
        vtkSmartPointer<vtkSurfaceToPerspectiveProjectionContourFilter>::New();
    if (!contourer->ConnectToRenderWindow(renWin))
      lok = false;
    contourer->AddInput(sr->GetOutput());
    if (contourer->GetNumberOfInputs() <= 0)
      lok = false;
    if (contourer->IsProjectionGeometryValid())
      lok = false;
    double srcPos[3];
    srcPos[0] = 0;
    srcPos[1] = 0;
    srcPos[2] = 1000;
    contourer->SetSourcePosition(srcPos);
    if (contourer->IsProjectionGeometryValid())
      lok = false;
    double plnOrig[3];
    plnOrig[0] = -205;
    plnOrig[1] = -205;
    plnOrig[2] = -500;
    contourer->SetPlaneOrigin(plnOrig);
    if (contourer->IsProjectionGeometryValid())
      lok = false;
    int plnSz[2];
    plnSz[0] = 1024;
    plnSz[1] = 1024;
    contourer->SetPlaneSizePixels(plnSz);
    if (contourer->IsProjectionGeometryValid())
      lok = false;
    double plnSp[2];
    plnSp[0] = 0.400390625;
    plnSp[1] = 0.400390625;
    contourer->SetPlaneSpacing(plnSp);
    if (contourer->IsProjectionGeometryValid())
      lok = false;
    vtkSmartPointer<vtkMatrix3x3> plnOrient =
        vtkSmartPointer<vtkMatrix3x3>::New();
    plnOrient->Identity();
    contourer->SetPlaneOrientation(plnOrient);
    if (!contourer->IsProjectionGeometryValid())
      lok = false;
    contourer->Update();
    if (!contourer->GetOutput())
      lok = false;
    vtkImageData *maskImage = contourer->GetOutput();
    if (!contourer->GetOutputPolyData())
      lok = false;
    vtkPolyData *contours = contourer->GetOutputPolyData();
    DisplayImageAndPolyData(contours, maskImage, plnOrient, sr->GetOutput(), NULL, "Simple geometry (ventral) - 1 component");
    WritePolyData(contours, "contour_1comp_contours.vtk");
    WriteImageData(maskImage, "contour_1comp_mask_image.mhd");
    vtkSmartPointer<vtkTransformPolyDataFilter> tf = vtkSmartPointer<
        vtkTransformPolyDataFilter>::New();
    tf->SetInput(sr->GetOutput());
    vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
    t->Translate(-10, 20, 30);
    t->RotateZ(5);
    tf->SetTransform(t);
    tf->Update();
    contourer->AddInput(tf->GetOutput());
    contourer->Update();
    if (!contourer->GetOutput())
      lok = false;
    maskImage = contourer->GetOutput();
    contours = contourer->GetOutputPolyData();
    DisplayImageAndPolyData(contours, maskImage, plnOrient, sr->GetOutput(), tf->GetOutput(), "Simple geometry (ventral) - 2 components");
    WritePolyData(contours, "contour_2comp_contours.vtk");
    WriteImageData(maskImage, "contour_2comp_mask_image.mhd");
    std::ostringstream os;
    contourer->Print(os);
    if (os.str().length() <= 0)
      lok = false;
    ok = ok && lok; // update OK-flag for test-scope
    VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

    VERBOSE(<< "  * Advanced functional tests ... ")
    lok = true; // initialize sub-section's success state
    contourer->RemoveAllInputs();
    vtkSmartPointer<vtkSuperquadricSource> sq = vtkSmartPointer<vtkSuperquadricSource>::New();
    sq->SetCenter(0, 0, 0);
    sq->SetThetaResolution(100);
    sq->SetPhiResolution(100);
    sq->SetThetaRoundness(1);
    sq->SetPhiRoundness(1);
    sq->SetToroidal(true);
    sq->SetScale(1, 1, 1);
    sq->SetSize(70);
    sq->Update();
    vtkSmartPointer<vtkTransformPolyDataFilter> tff = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    vtkSmartPointer<vtkTransform> tfft = vtkSmartPointer<vtkTransform>::New();
    tff->SetTransform(tfft);
    tfft->RotateZ(90);
    tfft->Translate(50, 50, 20);
    tff->SetInput(sq->GetOutput());
    tff->Update();
    contourer->AddInput(tff->GetOutput());
    vtkSmartPointer<vtkSuperquadricSource> sq2 = vtkSmartPointer<vtkSuperquadricSource>::New();
    sq2->SetCenter(0, 0, 0);
    sq2->SetThetaResolution(100);
    sq2->SetPhiResolution(100);
    sq2->SetThetaRoundness(1);
    sq2->SetPhiRoundness(1);
    sq2->SetToroidal(true);
    sq2->SetScale(1, 1, 1);
    sq2->SetSize(70);
    sq2->Update();
    vtkSmartPointer<vtkTransformPolyDataFilter> tff2 = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    vtkSmartPointer<vtkTransform> tfft2 = vtkSmartPointer<vtkTransform>::New();
    tff2->SetTransform(tfft2);
    tfft2->RotateZ(50);
    tfft2->Translate(-45, -35, -15);
    tff2->SetInput(sq->GetOutput());
    tff2->Update();
    contourer->AddInput(tff2->GetOutput());
    srcPos[0] = 1000;
    srcPos[1] = 0;
    srcPos[2] = 0;
    contourer->SetSourcePosition(srcPos);
    plnOrig[0] = -520;
    plnOrig[1] = -195;
    plnOrig[2] = 215;
    contourer->SetPlaneOrigin(plnOrig);
    plnSz[0] = 1024;
    plnSz[1] = 1024;
    contourer->SetPlaneSizePixels(plnSz);
    plnSp[0] = 0.400390625;
    plnSp[1] = 0.400390625;
    contourer->SetPlaneSpacing(plnSp);
    vtkSmartPointer<vtkTransform> tt = vtkSmartPointer<vtkTransform>::New();
    tt->Identity();
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
    contourer->SetPlaneOrientation(plnOrient);
    if (!contourer->IsProjectionGeometryValid())
      lok = false;
    contourer->Update();
    if (!contourer->GetOutput())
      lok = false;
    maskImage = contourer->GetOutput();
    contours = contourer->GetOutputPolyData();
    DisplayImageAndPolyData(contours, maskImage, plnOrient, tff->GetOutput(), tff2->GetOutput(), "Advanced geometry (lateral) 1");
    WritePolyData(contours, "contour_adv1_contours.vtk");
    WriteImageData(maskImage, "contour_adv1_mask_image.mhd");

    tt->Identity();
    srand(time(NULL));
    double r = (5.0 - (double) (rand() % 100001) / 4000);
    tt->RotateX(r);
    r = (5.0 - (double) (rand() % 100001) / 4000);
    tt->RotateY(r);
    r = (5.0 - (double) (rand() % 100001) / 4000);
    tt->RotateZ(r);
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
    contourer->SetPlaneOrientation(plnOrient);
    if (!contourer->IsProjectionGeometryValid())
      lok = false;
    contourer->Modified();
    contourer->Update();
    if (!contourer->GetOutput())
      lok = false;
    maskImage = contourer->GetOutput();
    contours = contourer->GetOutputPolyData();
    DisplayImageAndPolyData(contours, maskImage, plnOrient, tff->GetOutput(), tff2->GetOutput(), "Advanced geometry (lateral) 2");
    WritePolyData(contours, "contour_adv2_contours.vtk");
    WriteImageData(maskImage, "contour_adv2_mask_image.mhd");
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
