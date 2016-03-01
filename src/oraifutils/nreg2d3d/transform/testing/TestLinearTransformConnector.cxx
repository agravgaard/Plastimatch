//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <time.h>

#include "oraITKVTKLinearTransformConnector.h"

#include <vtkSmartPointer.h>
#include <vtkTransform.h>

#include <itkVersorRigid3DTransform.h>

#include "BasicUnitTestIncludes.hxx"

typedef ora::ITKVTKLinearTransformConnector ConnectorType;
typedef vtkSmartPointer<vtkTransform> VTKTransformPointer;
typedef vtkSmartPointer<vtkMatrix4x4> VTKMatrixPointer;
typedef itk::VersorRigid3DTransform<double> ITKTransformType;

/** Copy (implicit) ITK matrix (3x3-matrix + offset) to VTK 4x4-matrix. **/
void CopyITKToVTKMatrix(ITKTransformType::MatrixType imatrix,
    ITKTransformType::OutputVectorType ioffset,
    ConnectorType::RelativeMatrixPointer vmatrix)
{
  int i, j;
  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 3; j++)
    {
      vmatrix->SetElement(i, j, imatrix[i][j]);
    }
    vmatrix->SetElement(i, 3, ioffset[i]);
  }
}

/** @return TRUE if the VTK 4x4-matrices match **/
bool CheckVTKMatrices(ConnectorType::RelativeMatrixPointer vmatrix,
    ConnectorType::RelativeMatrixPointer vmatrix2)
{
  bool ok = true;
  int i, j;

  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 4; j++)
    {
      if (vmatrix->GetElement(i, j) != vmatrix2->GetElement(i, j))
      {
        ok = false;
        break;
      }
    }
    if (!ok)
      break;
  }

  return ok;
}

/**
 * @return TRUE if the (implicit) ITK-matrix (3x3-matrix + offset) and the VTK
 * 4x4-matrix are equal
 */
bool CheckMatrices(ConnectorType::RelativeMatrixPointer vmatrix,
    ITKTransformType::MatrixType imatrix,
    ITKTransformType::OutputVectorType ioffset)
{
  bool ok = true;
  int i, j;

  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 3; j++)
    {
      if (vmatrix->GetElement(i, j) != imatrix[i][j])
      {
        ok = false;
        break;
      }
    }
    if (vmatrix->GetElement(i, 3) != ioffset[i])
    {
      ok = false;
    }
    if (!ok)
      break;
  }

  return ok;
}

/**
 * Tests base functionality of:
 *
 *   ora::ITKVTKLinearTransformConnector
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::ITKVTKLinearTransformConnector
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
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, false, false);
    return EXIT_SUCCESS;
  }

  VERBOSE(<< "\nTesting linear ITK/VTK transformation connector.\n")
  bool ok = true;

  ConnectorType::Pointer connector = ConnectorType::New();

  VERBOSE(<< "  * Checking basic ITK -> VTK functionality ... ")
  VTKTransformPointer vtransform = VTKTransformPointer::New();
  connector->SetVTKTransform(vtransform);
  ITKTransformType::Pointer itransform = ITKTransformType::New();
  connector->SetITKTransform(itransform); // -> VTK ('start'-behavior)
  // check
  if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
      itransform->GetOffset()))
    ok = false;
  connector->SetITKTransform(NULL); // set back
  ITKTransformType::OutputVectorType itransl; // modify transform
  itransl[0] = -12.23;
  itransl[1] = 4.03;
  itransl[2] = 293.99;
  itransform->SetTranslation(itransl);
  connector->SetITKTransform(itransform); // -> VTK ('start'-behavior)
  if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
      itransform->GetOffset()))
    ok = false;
  connector->SetITKTransform(NULL); // set back
  ITKTransformType::InputPointType icenter;
  icenter[0] = -12;
  icenter[1] = -5.6;
  icenter[2] = +120;
  itransform->SetCenter(icenter);
  ITKTransformType::AxisType iaxis;
  iaxis[0] = 84.34;
  iaxis[1] = -34.78;
  iaxis[2] = 124.54;
  itransform->SetRotation(iaxis, 1.439);
  connector->SetITKTransform(itransform); // -> VTK ('start'-behavior)
  if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
      itransform->GetOffset()))
    ok = false;
  // continuous behavior
  srand(time(NULL));
  const int MAX_NUM_TRANSFORMS = 10000;
  for (int i = 0; i < MAX_NUM_TRANSFORMS; i++)
  {
    icenter[0] = ((double) (rand() % 10001 - 5000)) / 77.234;
    icenter[1] = ((double) (rand() % 10001 - 5000)) / 77.234;
    icenter[2] = ((double) (rand() % 10001 - 5000)) / 77.234;
    itransform->SetCenter(icenter);
    if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
        itransform->GetOffset()))
      ok = false;
    iaxis[0] = ((double) (rand() % 10001 - 5000)) / 777.;
    iaxis[1] = ((double) (rand() % 10001 - 5000)) / 777.;
    iaxis[2] = ((double) (rand() % 10001 - 5000)) / 777.;
    itransform->SetRotation(iaxis, ((double) (rand() % 10001 - 5000))
        / 1591.5494);
    itransform->Modified(); // set rotation does not call Modified()!!!
    if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
        itransform->GetOffset()))
      ok = false;
    itransl[0] = ((double) (rand() % 10001 - 5000)) / 23.8372;
    itransl[1] = ((double) (rand() % 10001 - 5000)) / 23.8372;
    itransl[2] = ((double) (rand() % 10001 - 5000)) / 23.8372;
    itransform->SetTranslation(itransl);
    if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
        itransform->GetOffset()))
      ok = false;
  }
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Checking basic VTK -> ITK functionality ... ")
  connector->SetITKTransform(NULL);
  connector->SetVTKTransform(NULL);
  connector->SetITKTransform(itransform);
  vtransform->Identity();
  connector->SetVTKTransform(vtransform); // -> ITK ('start'-behavior)
  // check
  if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
      itransform->GetOffset()))
    ok = false;
  connector->SetVTKTransform(NULL); // set back
  vtransform->Translate(-12.342, 0.0852, 1234.8589);
  connector->SetVTKTransform(vtransform); // -> ITK ('start'-behavior)
  // check
  if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
      itransform->GetOffset()))
    ok = false;
  connector->SetVTKTransform(NULL); // set back
  vtransform->Translate(-1.23, 23.52, -38.89);
  vtransform->RotateWXYZ(-58, 12.003, 48.38, -484.2);
  connector->SetVTKTransform(vtransform); // -> ITK ('start'-behavior)
  // check
  if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
      itransform->GetOffset()))
    ok = false;
  for (int i = 0; i < MAX_NUM_TRANSFORMS; i++)
  {
    vtransform->RotateX(((double) (rand() % 10001 - 5000)) / 55.6);
    vtransform->Modified(); // must explicitly be invoked!
    if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
        itransform->GetOffset()))
      ok = false;
    vtransform->RotateY(((double) (rand() % 10001 - 5000)) / 55.6);
    vtransform->Modified(); // must explicitly be invoked!
    if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
        itransform->GetOffset()))
      ok = false;
    vtransform->RotateZ(((double) (rand() % 10001 - 5000)) / 55.6);
    vtransform->Modified(); // must explicitly be invoked!
    if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
        itransform->GetOffset()))
      ok = false;
    vtransform->Translate(((double) (rand() % 10001 - 5000)) / 3.32943,
        ((double) (rand() % 10001 - 5000)) / 5.533, ((double) (rand() % 10001
            - 5000)) / 8.403);
    vtransform->Modified(); // must explicitly be invoked!
    if (!CheckMatrices(vtransform->GetMatrix(), itransform->GetMatrix(),
        itransform->GetOffset()))
      ok = false;
  }
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Checking relative ITK -> VTK functionality ... ")
  VTKTransformPointer relITKVTKTransform = VTKTransformPointer::New();
  relITKVTKTransform->RotateY(-2.8332);
  relITKVTKTransform->RotateZ(32.0023);
  VTKMatrixPointer relITKVTKMatrix = relITKVTKTransform->GetMatrix();
  connector->SetRelativeITKVTKMatrix(relITKVTKMatrix);
  VTKMatrixPointer testMatrix = VTKMatrixPointer::New();
  CopyITKToVTKMatrix(itransform->GetMatrix(), itransform->GetOffset(),
      testMatrix);
  vtkMatrix4x4::Multiply4x4(testMatrix, relITKVTKMatrix, testMatrix);
  if (!CheckVTKMatrices(testMatrix, vtransform->GetMatrix()))
    ok = false;
  for (int i = 0; i < MAX_NUM_TRANSFORMS; i++)
  {
    icenter[0] = ((double) (rand() % 10001 - 5000)) / 77.234;
    icenter[1] = ((double) (rand() % 10001 - 5000)) / 77.234;
    icenter[2] = ((double) (rand() % 10001 - 5000)) / 77.234;
    itransform->SetCenter(icenter);
    iaxis[0] = ((double) (rand() % 10001 - 5000)) / 777.;
    iaxis[1] = ((double) (rand() % 10001 - 5000)) / 777.;
    iaxis[2] = ((double) (rand() % 10001 - 5000)) / 777.;
    itransform->SetRotation(iaxis, ((double) (rand() % 10001 - 5000))
        / 1591.5494);
    itransl[0] = ((double) (rand() % 10001 - 5000)) / 23.8372;
    itransl[1] = ((double) (rand() % 10001 - 5000)) / 23.8372;
    itransl[2] = ((double) (rand() % 10001 - 5000)) / 23.8372;
    itransform->SetTranslation(itransl);
    CopyITKToVTKMatrix(itransform->GetMatrix(), itransform->GetOffset(),
        testMatrix);
    vtkMatrix4x4::Multiply4x4(testMatrix, relITKVTKMatrix, testMatrix);
    if (!CheckVTKMatrices(testMatrix, vtransform->GetMatrix()))
      ok = false;

    relITKVTKTransform->RotateX(((double) (rand() % 10001 - 5000)) / 55.6);
    relITKVTKTransform->RotateY(((double) (rand() % 10001 - 5000)) / 55.6);
    relITKVTKTransform->RotateZ(((double) (rand() % 10001 - 5000)) / 55.6);
    relITKVTKMatrix = relITKVTKTransform->GetMatrix();
    connector->SetRelativeITKVTKMatrix(relITKVTKMatrix);
    itransform->Modified(); // force update
    CopyITKToVTKMatrix(itransform->GetMatrix(), itransform->GetOffset(),
        testMatrix);
    vtkMatrix4x4::Multiply4x4(testMatrix, relITKVTKMatrix, testMatrix);
    if (!CheckVTKMatrices(testMatrix, vtransform->GetMatrix()))
      ok = false;
  }
  relITKVTKMatrix = NULL;
  relITKVTKTransform = NULL;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Checking relative VTK -> ITK functionality ... ")
  VTKTransformPointer relVTKITKTransform = VTKTransformPointer::New();
  relVTKITKTransform->RotateY(2.8332);
  relVTKITKTransform->RotateX(-32.0023);
  VTKMatrixPointer relVTKITKMatrix = relVTKITKTransform->GetMatrix();
  connector->SetRelativeVTKITKMatrix(relVTKITKMatrix);
  vtkMatrix4x4::Multiply4x4(vtransform->GetMatrix(), relVTKITKMatrix,
      testMatrix);
  VTKMatrixPointer itkMatrix = VTKMatrixPointer::New();
  CopyITKToVTKMatrix(itransform->GetMatrix(), itransform->GetOffset(),
      itkMatrix);
  if (!CheckVTKMatrices(testMatrix, itkMatrix))
    ok = false;

  for (int i = 0; i < MAX_NUM_TRANSFORMS; i++)
  {
    vtransform->RotateX(((double) (rand() % 10001 - 5000)) / 55.6);
    vtransform->RotateY(((double) (rand() % 10001 - 5000)) / 55.6);
    vtransform->RotateZ(((double) (rand() % 10001 - 5000)) / 55.6);
    vtransform->Translate(((double) (rand() % 10001 - 5000)) / 3.32943,
        ((double) (rand() % 10001 - 5000)) / 5.533, ((double) (rand() % 10001
            - 5000)) / 8.403);
    relVTKITKTransform->RotateX(((double) (rand() % 10001 - 5000)) / 55.6);
    relVTKITKTransform->RotateY(((double) (rand() % 10001 - 5000)) / 55.6);
    relVTKITKTransform->RotateZ(((double) (rand() % 10001 - 5000)) / 55.6);
    relVTKITKMatrix = relVTKITKTransform->GetMatrix();
    connector->SetRelativeVTKITKMatrix(relVTKITKMatrix);
    vtransform->Modified(); // force update
    vtkMatrix4x4::Multiply4x4(vtransform->GetMatrix(), relVTKITKMatrix,
        testMatrix);
    CopyITKToVTKMatrix(itransform->GetMatrix(), itransform->GetOffset(),
        itkMatrix);
    if (!CheckVTKMatrices(testMatrix, itkMatrix))
      ok = false;
  }
  itkMatrix = NULL;
  relVTKITKTransform = NULL;
  relVTKITKMatrix = NULL;
  testMatrix = NULL;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  vtransform = NULL;
  itransform = NULL;
  VERBOSE(<< "  * Final reference count check ... ")
  if (connector->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  connector = NULL; // reference counter must be zero!

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

