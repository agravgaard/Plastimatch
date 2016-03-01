//
#include "BasicUnitTestIncludes.hxx"

#include <sstream>

#include "vtkORAStructureReader.h"
#include "vtkContoursToSurfaceFilter.h"

#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTimerLog.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkCamera.h>
#include <vtkDataSetWriter.h>
#include <vtkPointData.h>
#include <vtkCellData.h>


// inter-active flag
bool InterActive = false;
// poly-data output
bool PolyDataOutput = false;

/**
 * Display the specified poly data in a minimal VTK-window which blocks
 * execution until the user closes the window. Works only if inter-active mode
 * is ON.
 */
void DisplayPolyData(vtkPolyData *pd, bool scalarVisibility,
    bool whiteBackground, std::string title, bool wireFrame = false)
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
    vtkActor *a = vtkActor::New();
    vtkPolyDataMapper *m = vtkPolyDataMapper::New();
    a->SetMapper(m);
    m->SetInput(pd);
    a->SetVisibility(true);
    if (wireFrame)
      a->GetProperty()->SetRepresentationToWireframe();
    else
      a->GetProperty()->SetRepresentationToSurface();
    if (!scalarVisibility)
    {
      m->SetScalarVisibility(false);
    }
    else
    {
      m->SetScalarVisibility(true);
      m->SetScalarModeToUsePointData();
    }
    ren->AddActor(a);
    rwi->Initialize();
    ren->ResetCamera();
    if (!whiteBackground)
      ren->SetBackground(0, 0, 0);
    else
      ren->SetBackground(1, 1, 1);
    rwi->Start();
    m->Delete();
    a->Delete();
    ren->Delete();
    rwi->Delete();
    renWin->Delete();
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

/** Check data set - return TRUE. **/
bool CheckPolyData(vtkPolyData *pd, bool hasColor, bool hasPointNormals,
    bool hasCellNormals, bool hasLines, bool hasPolys, bool hasPoints,
    int numPolys, int numLines, int numPoints, int maxNumPolys)
{
  if (!pd)
    return false;

  if (hasColor)
  {
    if (!pd->GetPointData()->GetArray("color"))
      return false;
  }
  else
  {
    if (pd->GetPointData()->GetArray("color"))
      return false;
  }
  if (hasPointNormals)
  {
    if (!pd->GetPointData()->GetNormals())
      return false;
  }
  else
  {
    if (pd->GetPointData()->GetNormals())
      return false;
  }
  if (hasCellNormals)
  {
    if (!pd->GetCellData()->GetNormals())
      return false;
  }
  else
  {
    if (pd->GetCellData()->GetNormals())
      return false;
  }
  if (hasLines)
  {
    if (pd->GetNumberOfLines() <= 0)
      return false;
  }
  else
  {
    if (pd->GetNumberOfLines() > 0)
      return false;
  }
  if (hasPolys)
  {
    if (pd->GetNumberOfPolys() <= 0)
      return false;
  }
  else
  {
    if (pd->GetNumberOfPolys() > 0)
      return false;
  }
  if (hasPoints)
  {
    if (pd->GetNumberOfPoints() <= 0)
      return false;
  }
  else
  {
    if (pd->GetNumberOfPoints() > 0)
      return false;
  }
  if (numLines != 0) // ignored if 0
  {
    if (pd->GetNumberOfLines() != numLines)
      return false;
  }
  if (numPolys != 0) // ignored if 0
  {
    if (pd->GetNumberOfPolys() != numPolys)
      return false;
  }
  if (numPoints != 0) // ignored if 0
  {
    if (pd->GetNumberOfPoints() != numPoints)
      return false;
  }
  if (maxNumPolys != 0) // ignored if 0
  {
    if (pd->GetNumberOfPolys() > maxNumPolys)
      return false;
  }

  return true;
}

/**
 * Tests base functionality of:
 *
 *   vtkORAStructureReader
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see vtkORAStructureReader
 * @see vtkContoursToSurfaceFilter
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
      "  -ia or --inter-active ... flag indicating that a minimal VTK-based GUI is displayed - requires the user to close GUI!");
  helpLines.push_back(
      "  -pdo or --poly-data-output ... flag indicating that test poly data should be written out (VTK-format)");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, false, true);
    return EXIT_SUCCESS;
  }
  // advanced arguments:
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-ia" || std::string(argv[i])
        == "--inter-active")
    {
      InterActive = true;
      continue;
    }
    if (std::string(argv[i]) == "-pdo" || std::string(argv[i])
        == "--poly-data-output")
    {
      PolyDataOutput = true;
      continue;
    }
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting VTK-ORA Structure Reader.\n")

  VERBOSE(<< "  * Check test data availability ... ")
  lok = true; // initialize sub-section's success state
  std::string struct3DFile = DataPath + "3DStructureInfo.inf";
  if (!vtksys::SystemTools::FileExists(struct3DFile.c_str()))
    lok = false;
  std::string struct2DFile = DataPath + "2DStructureInfo.inf";
  if (!vtksys::SystemTools::FileExists(struct2DFile.c_str()))
    lok = false;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  if (ok) // we need data for these tests!
  {
    VERBOSE(<< "  * Read a sample 3D ORA structure file ... ")
    lok = true; // initialize sub-section's success state
    vtkSmartPointer<vtkORAStructureReader> sr = vtkSmartPointer<
        vtkORAStructureReader>::New();
    std::ostringstream os;
    sr->Print(os);
    if (os.str().length() <= 0)
      lok = false;
    if (!sr->CanReadFile(struct3DFile.c_str()))
      lok = false;
    sr = NULL;
    sr = vtkSmartPointer<vtkORAStructureReader>::New();
    sr->SetGenerateSurfaceNormals(false); // no normals
    sr->SetReadColorsAsWell(false); // no color
    sr->SetFileName(struct3DFile.c_str());
    sr->Update();
    if (!CheckPolyData(sr->GetOutput(), false, false, false, false, true, true,
        0, 0, 0, 0))
      lok = false;
    DisplayPolyData(sr->GetOutput(), false, false, "No color, no normals");
    WritePolyData(sr->GetOutput(), "struct_nocol_nonormals.vtk");
    sr->SetReadColorsAsWell(true); // color, too
    sr->Update();
    if (!CheckPolyData(sr->GetOutput(), true, false, false, false, true, true,
        0, 0, 0, 0))
      lok = false;
    DisplayPolyData(sr->GetOutput(), true, true, "Color, no normals");
    WritePolyData(sr->GetOutput(), "struct_col_nonormals.vtk");
    sr->SetGenerateSurfaceNormals(true); // normals generation
    sr->SetSurfaceNormalsFeatureAngle(135);
    sr->Update();
    if (!CheckPolyData(sr->GetOutput(), true, true, true, false, true, true,
        0, 0, 0, 0))
      lok = false;
    DisplayPolyData(sr->GetOutput(), true, true, "Color, normals 135 deg");
    WritePolyData(sr->GetOutput(), "struct_col_normals_135.vtk");
    sr->SetGenerateSurfaceNormals(true); // normals generation
    sr->SetSurfaceNormalsFeatureAngle(30);
    sr->Update();
    if (!CheckPolyData(sr->GetOutput(), true, true, true, false, true, true,
        0, 0, 0, 0))
      lok = false;
    DisplayPolyData(sr->GetOutput(), true, true, "Color, normals 30 deg");
    WritePolyData(sr->GetOutput(), "struct_col_normals_30.vtk");
    sr = NULL;
    ok = ok && lok; // update OK-flag for test-scope
    VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

    VERBOSE(<< "  * Read a sample 2D ORA structure file ... ")
    lok = true; // initialize sub-section's success state
    sr = vtkSmartPointer<vtkORAStructureReader>::New();
    if (!sr->CanReadFile(struct2DFile.c_str()))
      lok = false;
    sr = NULL;
    sr = vtkSmartPointer<vtkORAStructureReader>::New();
    sr->SetGenerateSurfaceNormals(false); // no normals
    sr->SetReadColorsAsWell(false); // no color
    sr->SetFileName(struct2DFile.c_str());
    sr->SetGenerate3DSurfaceFrom2DContoursIfApplicable(false);
    sr->Modified();
    sr->Update();
    if (!CheckPolyData(sr->GetOutput(), false, false, false, true, false, true,
        0, 0, 0, 0))
      lok = false;
    DisplayPolyData(sr->GetOutput(), false, false, "Lines only", true);
    WritePolyData(sr->GetOutput(), "struct2d_lines.vtk");
    sr->SetGenerate3DSurfaceFrom2DContoursIfApplicable(true);
    sr->Modified();
    sr->Update();
    if (!CheckPolyData(sr->GetOutput(), false, false, false, false, true, true,
        0, 0, 0, 0))
      lok = false;
    DisplayPolyData(sr->GetOutput(), false, false, "3D Reconstruction", true);
    WritePolyData(sr->GetOutput(), "struct2d_3dreconstruction.vtk");
    sr->SetGenerateSurfaceNormals(true);
    sr->Modified();
    sr->Update();
    if (!CheckPolyData(sr->GetOutput(), false, true, true, false, true, true,
        0, 0, 0, 0))
      lok = false;
    int maxNumPolys = 1;
    if (sr->GetOutput())
      maxNumPolys = sr->GetOutput()->GetNumberOfPolys();
    DisplayPolyData(sr->GetOutput(), false, false,
        "3D Reconstruction + normals", true);
    WritePolyData(sr->GetOutput(), "struct2d_3dreconstruction_normals.vtk");
    sr->GetContoursToSurfaceFilter()->Print(os);
    if (os.str().length() <= 0)
      lok = false;
    sr->GetContoursToSurfaceFilter()->SetMeshDecimationModeToPro();
    sr->GetContoursToSurfaceFilter()->SetMeshReductionRatio(0.9);
    sr->Modified();
    sr->Update();
    if (!CheckPolyData(sr->GetOutput(), false, true, true, false, true, true,
        0, 0, 0, maxNumPolys * 0.2 /* some tolerance */))
      lok = false;
    DisplayPolyData(sr->GetOutput(), false, false,
        "3D Reconstruction + normals + decimate pro", true);
    WritePolyData(sr->GetOutput(),
        "struct2d_3dreconstruction_normals_decpro.vtk");
    sr->GetContoursToSurfaceFilter()->SetMeshDecimationModeToQuadric();
    sr->GetContoursToSurfaceFilter()->SetMeshReductionRatio(0.9);
    sr->Modified();
    sr->Update();
    if (!CheckPolyData(sr->GetOutput(), false, true, true, false, true, true,
        0, 0, 0, maxNumPolys * 0.2 /* some tolerance */))
      lok = false;
    DisplayPolyData(sr->GetOutput(), false, false,
        "3D Reconstruction + normals + quadric", true);
    WritePolyData(sr->GetOutput(),
        "struct2d_3dreconstruction_normals_quadric.vtk");
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
