//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#define _USE_MATH_DEFINES  // For MSVC
#include <math.h>
#include <time.h>

#include <vtkSmartPointer.h>
#include <vtkTupleInterpolator.h>
#include <vtkCardinalSpline.h>
#include <vtkKochanekSpline.h>

#include "BasicUnitTestIncludes.hxx"

// CSV output flag
bool CSVOutput = false;

/** Runge function. **/
double FRunge(double x)
{
  return (1.0 / (1.0 + x * x));
}

/** Sine function. **/
double FSin(double x)
{
  return (sin(x));
}

/**
 * Test a specific tuple interpolation method with pre-initialized tuple
 * interpolator.
 **/
bool TestInterpolation(vtkSmartPointer<vtkTupleInterpolator> tupInt,
    double tolerance, const char *csvFileName, double(*mathFunc)(double),
    double *x, double *y, double N, double symmetricIntervalWidth)
{
  bool ok = true;
  std::ofstream csvint;
  if (CSVOutput)
  {
    csvint.open(csvFileName, std::ios::out);
    if (csvint.is_open())
      csvint << "x;y;e\n";
    else
      ok = false;
  }
  double yt[1];
  for (int i = 0; i < N; i++) // re-define supporting points
  {
    yt[0] = y[i];
    tupInt->AddTuple(x[i], yt);
  }
  // draw random samples from interval and test against mathFunc:
  for (int i = 0; i < 1000; i++)
  {
    double f = (double) (rand() % 100001) / 100000.;
    double xi = (f * 2.0 - 1.0) * symmetricIntervalWidth;
    double yi[1];
    tupInt->InterpolateTuple(xi, yi);
    double ei = fabs(yi[0] - mathFunc(xi));
    // we have a tolerance within [-PI;+PI] - empirical!
    if (xi >= -symmetricIntervalWidth && xi <= symmetricIntervalWidth && ei
        > tolerance)
      ok = false;
    if (CSVOutput && csvint.is_open())
      csvint << xi << ";" << yi[0] << ";" << ei << "\n";
  }
  if (CSVOutput && csvint.is_open())
    csvint.close();
  return ok;
}

/**
 * Tests base functionality of:
 *
 *   VTK tuple interpolation methods (1D).
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see vtkTupleInterpolator
 * @see vtkCardinalSpline
 * @see vtkKochanekSpline
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.2
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  helpLines.push_back(
      "  -co or --csv-output ... CSV (comma separated values) file outputs");
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, false, false);
    return EXIT_SUCCESS;
  }
  // advanced arguments:
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-co" || std::string(argv[i]) == "--csv-output")
    {
      CSVOutput = true;
      continue;
    }
  }

  VERBOSE(<< "\nTesting some VTK-based 1-tuple interpolation capabilities.\n")
  bool ok = true;

  VERBOSE(<< "  * Basic interpolation check: are 2 points enough? ... ")
  bool lok = true;
  // look whether 2 supporting points are enough
  double *xt = new double[2];
  double *yt = new double[2];
  xt[0] = -0.5;
  yt[0] = -2.3;
  xt[1] = +0.1;
  yt[1] = 3.54;
  vtkSmartPointer<vtkTupleInterpolator> ti = vtkSmartPointer<
      vtkTupleInterpolator>::New();
  // - LINEAR -
  ti->Initialize();
  ti->SetInterpolationTypeToLinear();
  ti->SetNumberOfComponents(1);
  ti->AddTuple(xt[0], yt + 0);
  ti->AddTuple(xt[1], yt + 1);
  if (ti->GetNumberOfComponents() != 1 || ti->GetNumberOfTuples() != 2)
    lok = false;
  double yy = 0;
  ti->InterpolateTuple(0, &yy);
  if (yy == 0)
    lok = false;
  // sample the piece:
  std::ofstream csv;
  if (CSVOutput)
  {
    csv.open("linear_2point_piece.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  int steps = 100;
  for (int i = 0; i < steps; i++)
  {
    double xx = xt[0] + (xt[1] - xt[0]) * (double) i / (double) (steps - 1);
    csv << xx << ";";
    ti->InterpolateTuple(xx, &yy);
    csv << yy << "\n";
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // - CARDINAL SPLINE -
  ti->SetInterpolationTypeToSpline();
  vtkSmartPointer<vtkCardinalSpline> cs =
      vtkSmartPointer<vtkCardinalSpline>::New();
  ti->SetInterpolatingSpline(cs);
  ti->SetNumberOfComponents(1);
  ti->AddTuple(xt[0], yt + 0);
  ti->AddTuple(xt[1], yt + 1);
  if (ti->GetNumberOfComponents() != 1 || ti->GetNumberOfTuples() != 2)
    lok = false;
  yy = 0;
  ti->InterpolateTuple(0, &yy);
  if (CSVOutput)
  {
    csv.open("cardinal_spline_2point_piece.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  steps = 100;
  for (int i = 0; i < steps; i++)
  {
    double xx = xt[0] + (xt[1] - xt[0]) * (double) i / (double) (steps - 1);
    csv << xx << ";";
    ti->InterpolateTuple(xx, &yy);
    csv << yy << "\n";
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // - KOCHANEK SPLINE -
  ti->Initialize();
  ti->SetInterpolationTypeToSpline();
  vtkSmartPointer<vtkKochanekSpline> ks =
      vtkSmartPointer<vtkKochanekSpline>::New();
  ti->SetInterpolatingSpline(ks);
  ks->SetDefaultBias(0.2);
  ks->SetDefaultContinuity(-0.3);
  ks->SetDefaultTension(0.4);
  ti->SetNumberOfComponents(1);
  ti->AddTuple(xt[0], yt + 0);
  ti->AddTuple(xt[1], yt + 1);
  if (ti->GetNumberOfComponents() != 1 || ti->GetNumberOfTuples() != 2)
    lok = false;
  yy = 0;
  ti->InterpolateTuple(0, &yy);
  if (CSVOutput)
  {
    csv.open("kochanek_spline_2point_piece.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  steps = 100;
  for (int i = 0; i < steps; i++)
  {
    double xx = xt[0] + (xt[1] - xt[0]) * (double) i / (double) (steps - 1);
    csv << xx << ";";
    ti->InterpolateTuple(xx, &yy);
    csv << yy << "\n";
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Basic interpolation check: does t-order matter? ... ")
  lok = true;
  delete[] xt;
  delete[] yt;
  xt = new double[5];
  yt = new double[5];
  xt[0] = -23.3;
  yt[0] = 3.43; // ordered
  xt[1] = -12.3;
  yt[1] = 0.2;
  xt[2] = -2.12;
  yt[2] = -13.2;
  xt[3] = 10.23;
  yt[3] = 12.1;
  xt[4] = 12.3;
  yt[4] = -2.3;
  double *ref = new double[100];
  // - LINEAR -
  ti->Initialize();
  ti->SetInterpolationTypeToLinear();
  ti->SetNumberOfComponents(1);
  for (int i = 0; i < 5; i++)
    ti->AddTuple(xt[i], yt + i);
  if (ti->GetNumberOfComponents() != 1 || ti->GetNumberOfTuples() != 5)
    lok = false;
  // sample the piece:
  if (CSVOutput)
  {
    csv.open("linear_order_ref.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  steps = 100;
  for (int i = 0; i < steps; i++)
  {
    double xx = -23.3 + (12.3 + 23.3) * (double) i / (double) (steps - 1);
    csv << xx << ";";
    ti->InterpolateTuple(xx, &yy);
    csv << yy << "\n";
    ref[i] = yy;
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // randomize around order
  xt[0] = -2.12;
  yt[0] = -13.2;
  xt[1] = 12.3;
  yt[1] = -2.3;
  xt[2] = -12.3;
  yt[2] = 0.2;
  xt[3] = 10.23;
  yt[3] = 12.1;
  xt[4] = -23.3;
  yt[4] = 3.43; // ordered
  ti->Initialize();
  ti->SetInterpolationTypeToLinear();
  ti->SetNumberOfComponents(1);
  for (int i = 0; i < 5; i++)
    ti->AddTuple(xt[i], yt + i);
  if (ti->GetNumberOfComponents() != 1 || ti->GetNumberOfTuples() != 5)
    lok = false;
  // sample the piece:
  if (CSVOutput)
  {
    csv.open("linear_order_test.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  steps = 100;
  for (int i = 0; i < steps; i++)
  {
    double xx = -23.3 + (12.3 + 23.3) * (double) i / (double) (steps - 1);
    csv << xx << ";";
    ti->InterpolateTuple(xx, &yy);
    csv << yy << "\n";
    if (fabs(ref[i] - yy) > 1e-6)
      lok = false;
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // - CARDINAL SPLINE -
  ti->Initialize();
  ti->SetInterpolationTypeToSpline();
  ti->SetInterpolatingSpline(cs);
  ti->SetNumberOfComponents(1);
  xt[0] = -23.3;
  yt[0] = 3.43; // ordered
  xt[1] = -12.3;
  yt[1] = 0.2;
  xt[2] = -2.12;
  yt[2] = -13.2;
  xt[3] = 10.23;
  yt[3] = 12.1;
  xt[4] = 12.3;
  yt[4] = -2.3;
  for (int i = 0; i < 5; i++)
    ti->AddTuple(xt[i], yt + i);
  if (ti->GetNumberOfComponents() != 1 || ti->GetNumberOfTuples() != 5)
    lok = false;
  // sample the piece:
  if (CSVOutput)
  {
    csv.open("cardinal_spline_order_ref.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  steps = 100;
  for (int i = 0; i < steps; i++)
  {
    double xx = -23.3 + (12.3 + 23.3) * (double) i / (double) (steps - 1);
    csv << xx << ";";
    ti->InterpolateTuple(xx, &yy);
    csv << yy << "\n";
    ref[i] = yy;
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // randomize around order
  xt[0] = -2.12;
  yt[0] = -13.2;
  xt[1] = 12.3;
  yt[1] = -2.3;
  xt[2] = -12.3;
  yt[2] = 0.2;
  xt[3] = 10.23;
  yt[3] = 12.1;
  xt[4] = -23.3;
  yt[4] = 3.43; // ordered
  ti->Initialize();
  ti->SetInterpolationTypeToSpline();
  ti->SetInterpolatingSpline(cs);
  ti->SetNumberOfComponents(1);
  for (int i = 0; i < 5; i++)
    ti->AddTuple(xt[i], yt + i);
  if (ti->GetNumberOfComponents() != 1 || ti->GetNumberOfTuples() != 5)
    lok = false;
  // sample the piece:
  if (CSVOutput)
  {
    csv.open("cardinal_spline_order_test.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  steps = 100;
  for (int i = 0; i < steps; i++)
  {
    double xx = -23.3 + (12.3 + 23.3) * (double) i / (double) (steps - 1);
    csv << xx << ";";
    ti->InterpolateTuple(xx, &yy);
    csv << yy << "\n";
    if (fabs(ref[i] - yy) > 1e-6)
      lok = false;
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // - KOCHANEK SPLINE -
  ti->Initialize();
  ti->SetInterpolationTypeToSpline();
  ti->SetInterpolatingSpline(ks);
  ti->SetNumberOfComponents(1);
  xt[0] = -23.3;
  yt[0] = 3.43; // ordered
  xt[1] = -12.3;
  yt[1] = 0.2;
  xt[2] = -2.12;
  yt[2] = -13.2;
  xt[3] = 10.23;
  yt[3] = 12.1;
  xt[4] = 12.3;
  yt[4] = -2.3;
  for (int i = 0; i < 5; i++)
    ti->AddTuple(xt[i], yt + i);
  if (ti->GetNumberOfComponents() != 1 || ti->GetNumberOfTuples() != 5)
    lok = false;
  // sample the piece:
  if (CSVOutput)
  {
    csv.open("kochanek_spline_order_ref.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  steps = 100;
  for (int i = 0; i < steps; i++)
  {
    double xx = -23.3 + (12.3 + 23.3) * (double) i / (double) (steps - 1);
    csv << xx << ";";
    ti->InterpolateTuple(xx, &yy);
    csv << yy << "\n";
    ref[i] = yy;
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // randomize around order
  xt[0] = -2.12;
  yt[0] = -13.2;
  xt[1] = 12.3;
  yt[1] = -2.3;
  xt[2] = -12.3;
  yt[2] = 0.2;
  xt[3] = 10.23;
  yt[3] = 12.1;
  xt[4] = -23.3;
  yt[4] = 3.43; // ordered
  ti->Initialize();
  ti->SetInterpolationTypeToSpline();
  ti->SetInterpolatingSpline(ks);
  ti->SetNumberOfComponents(1);
  for (int i = 0; i < 5; i++)
    ti->AddTuple(xt[i], yt + i);
  if (ti->GetNumberOfComponents() != 1 || ti->GetNumberOfTuples() != 5)
    lok = false;
  // sample the piece:
  if (CSVOutput)
  {
    csv.open("kochanek_spline_order_test.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  steps = 100;
  for (int i = 0; i < steps; i++)
  {
    double xx = -23.3 + (12.3 + 23.3) * (double) i / (double) (steps - 1);
    csv << xx << ";";
    ti->InterpolateTuple(xx, &yy);
    csv << yy << "\n";
    if (fabs(ref[i] - yy) > 1e-6)
      lok = false;
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  cs = NULL;
  ks = NULL;
  ti = NULL;
  delete[] xt;
  delete[] yt;
  delete[] ref;
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Generate sine & Runge supporting points ... ")
  lok = true; // local OK
  // - SINE -
  const int N1 = 13;
  double *xs = new double[N1];
  double *ys = new double[N1];
  if (CSVOutput)
  {
    csv.open("sine.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  // prepare the supporting points (sine):
  for (int i = 0; i < N1; i++)
  {
    xs[i] = -M_PI + (double) i / (double) (N1 - 1) * (2. * M_PI);
    ys[i] = FSin(xs[i]);

    if (CSVOutput && csv.is_open())
      csv << xs[i] << ";" << ys[i] << "\n";
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  // - RUNGE -
  const int N2 = 10;
  double *xr = new double[N2];
  double *yr = new double[N2];
  if (CSVOutput)
  {
    csv.open("runge.csv", std::ios::out);
    if (csv.is_open())
      csv << "x;y\n";
    else
      lok = false;
  }
  // prepare the supporting points (sine):
  for (int i = 0; i < N2; i++)
  {
    xr[i] = -5.0 + (double) i / (double) (N2 - 1) * (2. * 5.0);
    yr[i] = FRunge(xr[i]);

    if (CSVOutput && csv.is_open())
      csv << xr[i] << ";" << yr[i] << "\n";
  }
  if (CSVOutput && csv.is_open())
    csv.close();
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using linear method ... ")
  // initialize the tuple interpolator (1D):
  vtkSmartPointer<vtkTupleInterpolator> tupInt = vtkSmartPointer<
      vtkTupleInterpolator>::New();
  tupInt->SetInterpolationTypeToLinear(); // linear
  tupInt->SetNumberOfComponents(1); // 1D (NOTE: set #components AFTER interpolation-type!)
  lok = TestInterpolation(tupInt, 0.04, "sin_linear_int.csv", FSin, xs, ys, N1,
      M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using cardinal spline method (open interval) ... ")
  // initialize the tuple interpolator (1D):
  tupInt->SetInterpolationTypeToSpline(); // spline (implicit reset!)
  vtkSmartPointer<vtkCardinalSpline> cardSpline = vtkSmartPointer<
      vtkCardinalSpline>::New();
  cardSpline->SetClosed(false);
  tupInt->SetInterpolatingSpline(cardSpline);
  tupInt->SetNumberOfComponents(1); // 1D (NOTE: set #components AFTER interpolation-type!)
  lok = TestInterpolation(tupInt, 0.1, "sin_card_spline_open_int.csv", FSin,
      xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using cardinal spline method (closed interval) ... ")
  // initialize the tuple interpolator (1D):
  tupInt->Initialize(); // reset
  tupInt->SetInterpolationTypeToSpline(); // spline
  cardSpline->SetClosed(true);
  tupInt->SetInterpolatingSpline(cardSpline);
  tupInt->SetNumberOfComponents(1); // 1D (NOTE: set #components AFTER interpolation-type!)
  lok = TestInterpolation(tupInt, 0.05, "sin_card_spline_closed_int.csv", FSin,
      xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using Kochanek spline (default setup) method (open interval) ... ")
  // initialize the tuple interpolator (1D):
  tupInt->Initialize(); // reset
  tupInt->SetInterpolationTypeToSpline(); // spline
  vtkSmartPointer<vtkKochanekSpline> kochSpline = vtkSmartPointer<
      vtkKochanekSpline>::New();
  kochSpline->SetClosed(false);
  tupInt->SetInterpolatingSpline(kochSpline);
  tupInt->SetNumberOfComponents(1); // 1D (NOTE: set #components AFTER interpolation-type!)
  lok = TestInterpolation(tupInt, 0.1, "sin_koch_spline_open_int.csv", FSin,
      xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using Kochanek spline (default setup) method (closed interval) ... ")
  // initialize the tuple interpolator (1D):
  tupInt->Initialize(); // reset
  tupInt->SetInterpolationTypeToSpline(); // spline
  kochSpline->SetClosed(true);
  tupInt->SetInterpolatingSpline(kochSpline);
  tupInt->SetNumberOfComponents(1); // 1D (NOTE: set #components AFTER interpolation-type!)
  lok = TestInterpolation(tupInt, 0.06, "sin_koch_spline_closed_int.csv", FSin,
      xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of Runge using linear method ... ")
  // initialize the tuple interpolator (1D):
  tupInt->Initialize();
  tupInt->SetInterpolationTypeToLinear(); // linear
  tupInt->SetNumberOfComponents(1); // 1D (NOTE: set #components AFTER interpolation-type!)
  lok = TestInterpolation(tupInt, 0.25 /* around 0! */, "runge_linear_int.csv",
      FRunge, xr, yr, N2, 5.0);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of Runge using cardinal spline method ... ")
  // initialize the tuple interpolator (1D):
  tupInt->SetInterpolationTypeToSpline(); // spline (implicit reset!)
  cardSpline->SetClosed(false);
  tupInt->SetInterpolatingSpline(cardSpline);
  tupInt->SetNumberOfComponents(1); // 1D (NOTE: set #components AFTER interpolation-type!)
  lok = TestInterpolation(tupInt, 0.15 /* around 0! */,
      "runge_card_spline_int.csv", FRunge, xr, yr, N2, 5.0);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of Runge using Kochanek spline method ... ")
  // initialize the tuple interpolator (1D):
  tupInt->Initialize(); // reset
  tupInt->SetInterpolationTypeToSpline(); // spline (implicit reset!)
  kochSpline->SetClosed(false);
  tupInt->SetInterpolatingSpline(kochSpline);
  tupInt->SetNumberOfComponents(1); // 1D (NOTE: set #components AFTER interpolation-type!)
  lok = TestInterpolation(tupInt, 0.18 /* around 0! */,
      "runge_koch_spline_int.csv", FRunge, xr, yr, N2, 5.0);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  delete[] xs;
  delete[] ys;
  delete[] xr;
  delete[] yr;
  cardSpline = NULL;
  kochSpline = NULL;
  tupInt = NULL;

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
