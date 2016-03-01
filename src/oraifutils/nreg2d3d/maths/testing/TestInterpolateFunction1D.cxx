//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#define _USE_MATH_DEFINES  // For MSVC
#include <math.h>
#include <vector>

#include "oraInterpolateFunction1D.h"

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
  return sin(x);
}

typedef ora::InterpolateFunction1D InterpolateType;

/**
 * Test a specific tuple interpolation method with pre-initialized interpolator
 * function object.
 **/
bool TestInterpolation(InterpolateType *interp, double tolerance,
    const char *csvFileName, double(*mathFunc)(double), double *x, double *y,
    double N, double symmetricIntervalWidth)
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
  interp->SetSupportingPoints(x, y, N);
  if (interp->GetN() != N)
    return false;
  // draw random samples from interval and test against mathFunc:
  double maxei = 0;
  for (int i = 0; i < 1000; i++)
  {
    double f = (double) (rand() % 100001) / 100000.;
    double xi = (f * 2.0 - 1.0) * symmetricIntervalWidth;
    double yi = interp->Interpolate(xi);
    double ei = fabs(yi - mathFunc(xi));
    if (ei > maxei)
      maxei = ei;
    // we have a tolerance within [-PI;+PI] - empirical!
    if (xi >= -symmetricIntervalWidth && xi <= symmetricIntervalWidth && ei
        > tolerance)
      ok = false;
    if (CSVOutput && csvint.is_open())
      csvint << xi << ";" << yi << ";" << ei << "\n";
  }
  if (CSVOutput && csvint.is_open())
    csvint.close();
  if (!ok)
    VERBOSE(<< " [" << maxei << " > " << tolerance << "] ")
  return ok;
}

/**
 * Tests base functionality of:
 *
 *   ora::InterpolateFunction1D
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::InterpolateFunction1D
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

  VERBOSE(<< "\nTesting interpolate function 1D.\n")
  bool ok = true;

  VERBOSE(<< "  * Testing basic properties of interpolator ... ")
  bool lok = true; // local OK
  InterpolateType *interp = new InterpolateType();
  interp->SetHybridCriticalEdgeWidth(0);
  interp->SetInterpolationMode(InterpolateType::LINEAR);
  interp->SetUseClosedSplineInterpolation(false);
  double *x = new double[1];
  x[0] = 0.0;
  double *y = new double[1];
  y[0] = 2.3;
  double N = 1;
  interp->SetSupportingPoints(x, y, N); // not enough -> do not take over!
  if (interp->GetN() != 0 || interp->GetX() || interp->GetY())
    lok = false;
  delete[] x;
  delete[] y;
  x = new double[2];
  x[0] = -1.2;
  x[1] = 3.3;
  y = new double[2];
  y[0] = 4.3;
  y[1] = -1.2;
  N = 2;
  interp->SetSupportingPoints(x, y, N); // take over!
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY())
    lok = false;
  // what about supporting point definition in constructor?
  if (interp)
    delete interp;
  interp = new InterpolateType(x, y, N, InterpolateType::LINEAR);
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY()
      || interp->GetInterpolationMode() != InterpolateType::LINEAR)
    lok = false;
  if (interp)
    delete interp;
  interp = new InterpolateType(x, y, N, InterpolateType::CARDINAL_SPLINE);
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY()
      || interp->GetInterpolationMode() != InterpolateType::CARDINAL_SPLINE)
    lok = false;
  if (interp)
    delete interp;
  interp = new InterpolateType(x, y, N, InterpolateType::KOCHANEK_SPLINE);
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY()
      || interp->GetInterpolationMode() != InterpolateType::KOCHANEK_SPLINE)
    lok = false;
  if (interp)
    delete interp;
  interp
      = new InterpolateType(x, y, N, InterpolateType::CARDINAL_SPLINE_HYBRID);
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY()
      || interp->GetInterpolationMode()
          != InterpolateType::CARDINAL_SPLINE_HYBRID)
    lok = false;
  if (interp)
    delete interp;
  interp
      = new InterpolateType(x, y, N, InterpolateType::KOCHANEK_SPLINE_HYBRID);
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY()
      || interp->GetInterpolationMode()
          != InterpolateType::KOCHANEK_SPLINE_HYBRID)
    lok = false;
  // changing of attributes should preserve supporting points and interpolation!
  interp->SetUseClosedSplineInterpolation(true);
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY()
      || interp->GetInterpolationMode()
          != InterpolateType::KOCHANEK_SPLINE_HYBRID)
    lok = false;
  interp->SetHybridCriticalEdgeWidth(0.2);
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY()
      || interp->GetInterpolationMode()
          != InterpolateType::KOCHANEK_SPLINE_HYBRID)
    lok = false;
  interp->SetKochanekBias(-0.5);
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY()
      || interp->GetInterpolationMode()
          != InterpolateType::KOCHANEK_SPLINE_HYBRID)
    lok = false;
  interp->SetKochanekContinuity(0.1);
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY()
      || interp->GetInterpolationMode()
          != InterpolateType::KOCHANEK_SPLINE_HYBRID)
    lok = false;
  interp->SetKochanekTension(0.3);
  if (interp->GetN() != 2 || !interp->GetX() || !interp->GetY()
      || interp->GetInterpolationMode()
          != InterpolateType::KOCHANEK_SPLINE_HYBRID)
    lok = false;
  interp->SetSupportingPoints(NULL, NULL, 0); // delete supporting points
  if (interp->GetN() != 0 || interp->GetX() || interp->GetY())
    lok = false;
  if (interp)
    delete interp;
  interp = NULL;
  delete[] x;
  delete[] y;
  ok = ok && lok;
  VERBOSE(<< (lok ? "  OK" : "  FAILURE") << "\n")

  VERBOSE(<< "  * Generate sine & Runge supporting points ... ")
  lok = true; // local OK
  std::ofstream csv;
  // - SINE -
  const int N1 = 17;
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
  const int N2 = 20;
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
  interp = new InterpolateType();
  interp->SetInterpolationMode(InterpolateType::LINEAR);
  lok = TestInterpolation(interp, 0.02, "sin_linear_int.csv", FSin, xs, ys, N1,
      M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using cardinal spline method ... ")
  interp->SetInterpolationMode(InterpolateType::CARDINAL_SPLINE);
  interp->SetUseClosedSplineInterpolation(false);
  lok = TestInterpolation(interp, 0.07, "sin_cardinal_spline_int.csv", FSin,
      xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using cardinal spline method (closed interval) ... ")
  interp->SetInterpolationMode(InterpolateType::CARDINAL_SPLINE);
  interp->SetUseClosedSplineInterpolation(true);
  lok = TestInterpolation(interp, 0.04, "sin_cardinal_spline_closed_int.csv",
      FSin, xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using hybrid cardinal spline method ... ")
  interp->SetInterpolationMode(InterpolateType::CARDINAL_SPLINE_HYBRID);
  interp->SetUseClosedSplineInterpolation(false);
  interp->SetHybridCriticalEdgeWidth(0.5);
  lok = TestInterpolation(interp, 0.03, "sin_cardinal_spline_hybrid_int.csv",
      FSin, xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using Kochanek spline (setup 1) method ... ")
  interp->SetInterpolationMode(InterpolateType::KOCHANEK_SPLINE);
  interp->SetUseClosedSplineInterpolation(false);
  lok = TestInterpolation(interp, 0.07, "sin_kochanek_spline_s1_int.csv", FSin,
      xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using Kochanek spline (setup 1) method (closed interval) ... ")
  interp->SetInterpolationMode(InterpolateType::KOCHANEK_SPLINE);
  interp->SetUseClosedSplineInterpolation(true);
  lok = TestInterpolation(interp, 0.05,
      "sin_kochanek_spline_s1_closed_int.csv", FSin, xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using hybrid Kochanek spline (setup 1) method ... ")
  interp->SetInterpolationMode(InterpolateType::KOCHANEK_SPLINE_HYBRID);
  interp->SetUseClosedSplineInterpolation(false);
  interp->SetHybridCriticalEdgeWidth(0.5);
  lok = TestInterpolation(interp, 0.015,
      "sin_kochanek_spline_s1_hybrid_int.csv", FSin, xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using Kochanek spline (setup 2) method ... ")
  interp->SetInterpolationMode(InterpolateType::KOCHANEK_SPLINE);
  interp->SetUseClosedSplineInterpolation(false);
  interp->SetKochanekBias(0.5);
  interp->SetKochanekTension(0.5);
  lok = TestInterpolation(interp, 0.07, "sin_kochanek_spline_s2_int.csv", FSin,
      xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using Kochanek spline (setup 2) method (closed interval) ... ")
  interp->SetInterpolationMode(InterpolateType::KOCHANEK_SPLINE);
  interp->SetUseClosedSplineInterpolation(true);
  lok = TestInterpolation(interp, 0.05,
      "sin_kochanek_spline_s2_closed_int.csv", FSin, xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of sine using hybrid Kochanek spline (setup 2) method ... ")
  interp->SetInterpolationMode(InterpolateType::KOCHANEK_SPLINE_HYBRID);
  interp->SetUseClosedSplineInterpolation(false);
  interp->SetHybridCriticalEdgeWidth(0.5);
  lok = TestInterpolation(interp, 0.025,
      "sin_kochanek_spline_s2_hybrid_int.csv", FSin, xs, ys, N1, M_PI);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of Runge using linear method ... ")
  interp = new InterpolateType();
  interp->SetInterpolationMode(InterpolateType::LINEAR);
  lok = TestInterpolation(interp, 0.07, "runge_linear_int.csv", FRunge, xr, yr,
      N2, 5.0);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of Runge using cardinal spline method ... ")
  interp->SetInterpolationMode(InterpolateType::CARDINAL_SPLINE);
  interp->SetUseClosedSplineInterpolation(false);
  lok = TestInterpolation(interp, 0.015, "runge_cardinal_spline_int.csv",
      FRunge, xr, yr, N2, 5.0);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of Runge using hybrid cardinal spline method ... ")
  interp->SetInterpolationMode(InterpolateType::CARDINAL_SPLINE_HYBRID);
  interp->SetUseClosedSplineInterpolation(false);
  interp->SetHybridCriticalEdgeWidth(0.5);
  lok = TestInterpolation(interp, 0.015, "runge_cardinal_spline_hybrid_int.csv",
      FRunge, xr, yr, N2, 5.0);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of Runge using Kochanek spline (setup 1) method ... ")
  interp->SetInterpolationMode(InterpolateType::KOCHANEK_SPLINE);
  interp->SetUseClosedSplineInterpolation(false);
  interp->SetKochanekBias(0);
  interp->SetKochanekContinuity(0);
  interp->SetKochanekTension(0);
  lok = TestInterpolation(interp, 0.035, "runge_kochanek_spline_s1_int.csv",
      FRunge, xr, yr, N2, 5.0);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of Runge using hybrid Kochanek spline (setup 1) method ... ")
  interp->SetInterpolationMode(InterpolateType::KOCHANEK_SPLINE_HYBRID);
  interp->SetUseClosedSplineInterpolation(false);
  interp->SetHybridCriticalEdgeWidth(0.5);
  lok = TestInterpolation(interp, 0.035, "runge_kochanek_spline_s1_hybrid_int.csv",
      FRunge, xr, yr, N2, 5.0);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of Runge using Kochanek spline (setup 2) method ... ")
  interp->SetInterpolationMode(InterpolateType::KOCHANEK_SPLINE);
  interp->SetUseClosedSplineInterpolation(false);
  interp->SetKochanekBias(0);
  interp->SetKochanekContinuity(0);
  interp->SetKochanekTension(-0.3);
  lok = TestInterpolation(interp, 0.015, "runge_kochanek_spline_s2_int.csv",
      FRunge, xr, yr, N2, 5.0);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Interpolation of Runge using hybrid Kochanek spline (setup 2) method ... ")
  interp->SetInterpolationMode(InterpolateType::KOCHANEK_SPLINE_HYBRID);
  interp->SetUseClosedSplineInterpolation(false);
  interp->SetHybridCriticalEdgeWidth(0.5);
  lok = TestInterpolation(interp, 0.015, "runge_kochanek_spline_s2_hybrid_int.csv",
      FRunge, xr, yr, N2, 5.0);
  ok = ok && lok;
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

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
