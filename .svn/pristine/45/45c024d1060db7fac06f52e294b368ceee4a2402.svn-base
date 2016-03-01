//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <time.h>

#include <itkVersorRigid3DTransform.h>
#include <itkCommand.h>

#include "oraParametrizableIdentityTransform.h"

#include "BasicUnitTestIncludes.hxx"

// helper
int TransformChangedCounter = 0;
int BeforeParametersSetCounter = 0;
int AfterParametersSetCounter = 0;

/** Transform-events observer. **/
void TransformEvent(itk::Object *obj, const itk::EventObject &ev, void *cd)
{
  if (std::string(ev.GetEventName()) == "TransformChanged")
    TransformChangedCounter++; // simply count
  else if (std::string(ev.GetEventName()) == "BeforeParametersSet")
    BeforeParametersSetCounter++; // simply count
  else if (std::string(ev.GetEventName()) == "AfterParametersSet")
    AfterParametersSetCounter++; // simply count
}

/**
 * Tests base functionality of:
 *
 *   ora::ParametrizableIdentityTransform
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::ParametrizableIdentityTransform
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.5
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

  VERBOSE(<< "\nTesting parametrizable identity transformation.\n")
  bool ok = true;

  typedef ora::ParametrizableIdentityTransform<double, 2> TransformType;
  typedef TransformType::ParametersType ParametersType;
  typedef TransformType::MatrixType MatrixType;
  typedef TransformType::OutputVectorType OutputVectorType;
  typedef TransformType::InputPointType InputPointType;
  typedef TransformType::OutputPointType OutputPointType;
  typedef TransformType::InverseMatrixType InverseMatrixType;
  typedef TransformType::InverseTransformBasePointer
      InverseTransformBasePointer;
  typedef TransformType::OutputVnlVectorType OutputVnlVectorType;
  typedef TransformType::OutputCovariantVectorType OutputCovariantVectorType;
  typedef TransformType::JacobianType JacobianType;
  typedef itk::VersorRigid3DTransform<double> Transform3DType;

  TransformType::Pointer transform = TransformType::New();

  VERBOSE(<< "  * Checking identity nature ... ")
  bool lok = true;
  // transform must always have identity and no other attribs!
  MatrixType matrix = transform->GetMatrix();
  if (matrix[0][0] != 1 || matrix[0][1] != 0 || matrix[1][0] != 0
      || matrix[1][1] != 1)
    lok = false;
  MatrixType m;
  m[0][0] = 5.1;
  m[0][1] = 1.1;
  m[1][0] = 3.1;
  m[1][1] = 2.1;
  transform->SetMatrix(m);
  matrix = transform->GetMatrix();
  if (matrix[0][0] != 1 || matrix[0][1] != 0 || matrix[1][0] != 0
      || matrix[1][1] != 1)
    lok = false;
  OutputVectorType offset = transform->GetOffset();
  if (offset[0] != 0 || offset[1] != 0)
    lok = false;
  OutputVectorType o;
  o[0] = 1.3;
  o[1] = -23;
  transform->SetOffset(o);
  offset = transform->GetOffset();
  if (offset[0] != 0 || offset[1] != 0)
    lok = false;
  InputPointType center = transform->GetCenter();
  if (center[0] != 0 || center[1] != 0)
    lok = false;
  InputPointType c;
  c[0] = 13;
  c[1] = -232;
  transform->SetCenter(c);
  center = transform->GetCenter();
  if (center[0] != 0 || center[1] != 0)
    lok = false;
  OutputVectorType transl = transform->GetTranslation();
  if (transl[0] != 0 || transl[1] != 0)
    lok = false;
  OutputVectorType t;
  t[0] = 1334;
  t[1] = -333;
  transform->SetTranslation(t);
  transl = transform->GetTranslation();
  if (transl[0] != 0 || transl[1] != 0)
    lok = false;
  InverseMatrixType invMatrix = transform->GetInverseMatrix();
  if (invMatrix[0][0] != 1 || invMatrix[0][1] != 0 || invMatrix[1][0] != 0
      || invMatrix[1][1] != 1)
    lok = false;
  InverseTransformBasePointer invTransform = transform->GetInverseTransform();
  if (invTransform)
  {
    TransformType::Pointer invt = itk::SmartPointer<TransformType>(
        static_cast<TransformType *> (invTransform.GetPointer()));
    matrix = invt->GetMatrix();
    if (matrix[0][0] != 1 || matrix[0][1] != 0 || matrix[1][0] != 0
        || matrix[1][1] != 1)
      lok = false;
    invt = NULL;
  }
  else
  {
    lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Checking parameters ... ")
  lok = true;
  // transform must simply reflect set parameters:
  ParametersType paras = transform->GetParameters();
  unsigned int npars = (TransformType::SpaceDimension
      * (TransformType::SpaceDimension + 1));
  if (paras.Size() == npars)
  {
    for (unsigned int i = 0; i < npars; i++)
    {
      if (paras[i] != 0)
        lok = false;
    }
  }
  else
  {
    lok = false;
  }
  ParametersType p;
  p.SetSize(6); // =npars
  p[0] = 10;
  p[1] = -5;
  p[2] = 6.4;
  p[3] = 23.2;
  p[4] = 0.0003;
  p[5] = 66.3;
  transform->SetParameters(p);
  paras = transform->GetParameters();
  if (paras.Size() == 6)
  {
    for (int i = 0; i < 6; i++)
    {
      if (paras[i] != p[i])
        lok = false;
    }
  }
  else
  {
    lok = false;
  }
  // must not have other effects on transformation:
  matrix = transform->GetMatrix();
  if (matrix[0][0] != 1 || matrix[0][1] != 0 || matrix[1][0] != 0
      || matrix[1][1] != 1)
    lok = false;
  offset = transform->GetOffset();
  if (offset[0] != 0 || offset[1] != 0)
    lok = false;
  offset = transform->GetOffset();
  if (offset[0] != 0 || offset[1] != 0)
    lok = false;
  center = transform->GetCenter();
  if (center[0] != 0 || center[1] != 0)
    lok = false;
  transl = transform->GetTranslation();
  if (transl[0] != 0 || transl[1] != 0)
    lok = false;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Checking transforms ... ")
  lok = true;
  // identity transforms:
  InputPointType po;
  po[0] = 23.43;
  po[1] = 84.002;
  OutputPointType po2 = transform->TransformPoint(po);
  if (po[0] != po2[0] || po[1] != po2[1])
    lok = false;
  OutputVectorType vec;
  vec[0] = 12.3;
  vec[1] = 0.003;
  OutputVectorType vec2 = transform->TransformVector(vec);
  if (vec[0] != vec2[0] || vec[1] != vec2[1])
    lok = false;
  OutputVnlVectorType vvec;
  vvec[0] = -1.203;
  vvec[1] = -312.2;
  OutputVnlVectorType vvec2 = transform->TransformVector(vvec);
  if (vvec[0] != vvec2[0] || vvec[1] != vvec2[1])
    lok = false;
  OutputCovariantVectorType cvec;
  cvec[0] = -1.203;
  cvec[1] = -312.2;
  OutputCovariantVectorType cvec2 = transform->TransformCovariantVector(cvec);
  if (cvec[0] != cvec2[0] || cvec[1] != cvec2[1])
    lok = false;
  JacobianType jac = transform->GetJacobian(po); // constant awaited!
  for (int x = 0; x < 6; x++)
  {
    for (int y = 0; y < 2; y++)
    {
      if (jac[y][x] != 0)
        lok = false;
    }
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Parameter vector resizing behavior ... ")
  lok = true;
  transform->SetNumberOfConnectedTransformParameters(4);
  paras = transform->GetParameters();
  if (paras.size() == 4)
  {
    for (unsigned int d = 0; d < paras.size(); d++)
    {
      if (fabs(paras[d]) > 1e-6)
        lok = false;
    }
  }
  else
  {
    lok = false;
  }
  Transform3DType::Pointer tt = Transform3DType::New();
  transform->SetConnected3DTransform(tt);
  paras = transform->GetParameters();
  if (paras.size() == tt->GetNumberOfParameters())
  {
    for (unsigned int d = 0; d < paras.size(); d++)
    {
      if (fabs(paras[d]) > 1e-6)
        lok = false;
    }
    transform->SetNumberOfConnectedTransformParameters(3); // no effect!!!
    paras = transform->GetParameters();
    if (paras.size() != tt->GetNumberOfParameters())
      lok = false;
  }
  else
  {
    lok = false;
  }
  transform->SetConnected3DTransform(NULL); // dereference
  transform->SetNumberOfConnectedTransformParameters(3); // effect!!!
  paras = transform->GetParameters();
  if (paras.size() != 3)
    lok = false;
  tt = NULL;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Checking connected 3D transform behavior ... ")
  lok = true;
  Transform3DType::Pointer t3 = Transform3DType::New();
  Transform3DType::AxisType ax;
  ax[0] = 0.765;
  ax[1] = 3.2;
  ax[2] = -43;
  t3->SetRotation(ax, 2.3);
  Transform3DType::OutputVectorType transl3;
  transl3[0] = -3;
  transl3[1] = -12;
  transl3[2] = -33;
  t3->SetTranslation(transl3);
  transform->SetConnected3DTransform(t3);
  transform->SetStealJacobianFromConnected3DTransform(true);
  Transform3DType::ParametersType pars3 = t3->GetParameters();
  Transform3DType::ParametersType pars31(pars3.Size());
  for (unsigned int d = 0; d < pars3.Size(); d++)
    pars31[d] = pars3[d] + 0.23 * d;
  transform->SetParameters(pars31);
  // check whether or not the parameters were taken over:
  Transform3DType::ParametersType pars32 = t3->GetParameters();
  for (unsigned int d = 0; d < pars32.Size(); d++)
  {
    if (pars32[d] != pars31[d])
      lok = false;
  }
  TransformType::InputPointType jacp;
  jacp[0] = 12.;
  jacp[1] = -32.553;
  TransformType::JacobianType jac1 = transform->GetJacobian(jacp); // implicit
  Transform3DType::InputPointType jacp3;
  jacp3[0] = jacp[0];
  jacp3[1] = jacp[1];
  jacp3[2] = 0; // mapped to zero!
  Transform3DType::JacobianType jac3 = t3->GetJacobian(jacp3); // explicit
  if (jac1.columns() != jac3.columns() || jac1.rows() != jac3.rows())
    lok = false;
  if (lok)
  {
    for (unsigned int y = 0; y < jac1.rows(); y++)
    {
      for (unsigned int x = 0; x < jac1.columns(); x++)
      {
        if (jac1.get(y, x) != jac3.get(y, x))
          lok = false;
      }
    }
  }
  transform->SetStealJacobianFromConnected3DTransform(false); // no steal
  jac1 = transform->GetJacobian(jacp); // implicit
  for (unsigned int y = 0; y < jac1.rows(); y++) // constant awaited
  {
    for (unsigned int x = 0; x < jac1.columns(); x++)
    {
      if (jac1.get(y, x) != 0)
        lok = false;
    }
  }
  t3 = NULL;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Checking events ... ")
  lok = true;
  typedef itk::CStyleCommand CommandType;
  CommandType::Pointer cmd = CommandType::New();
  cmd->SetCallback(TransformEvent);
  cmd->SetClientData(transform);
  transform->AddObserver(ora::TransformChanged(), cmd);
  transform->AddObserver(ora::BeforeParametersSet(), cmd);
  transform->AddObserver(ora::AfterParametersSet(), cmd);
  TransformChangedCounter = BeforeParametersSetCounter
      = AfterParametersSetCounter = 0;
  transform->SetParameters(transform->GetParameters()); // not changed
  if (TransformChangedCounter != 0 || BeforeParametersSetCounter != 1
      || AfterParametersSetCounter != 1)
    lok = false;
  npars = transform->GetNumberOfConnectedTransformParameters();
  srand(time(NULL));
  Transform3DType::ParametersType tpars(npars);
  for (int i = 0; i < 100; i++)
  {
    if (i % 10 != 0)
    {
      for (unsigned int j = 0; j < npars; j++)
        tpars[j] = .5 - (double) (rand() % 100001) / 100000.;
    }
    bool parschanged = false;
    for (unsigned int j = 0; j < npars; j++)
    {
      if (tpars[j] != transform->GetParameters()[j])
        parschanged = true;
    }
    TransformChangedCounter = BeforeParametersSetCounter
        = AfterParametersSetCounter = 0;
    transform->SetParameters(tpars);
    if ((parschanged && TransformChangedCounter != 1) || (!parschanged
        && TransformChangedCounter != 0))
      lok = false;
    if (BeforeParametersSetCounter != 1 || AfterParametersSetCounter != 1)
      lok = false;
  }
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  if (transform->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  transform = NULL; // reference counter must be zero!

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

