//
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>

#include "oraPortalImagingDeviceProjectionProperties.h"
#include "oraTranslationScaleFlexMapCorrection.h"

#include <vtkMath.h>

#include "BasicUnitTestIncludes.hxx"

// CSV (comma-separated file) output
bool CSVOutput = false;

typedef unsigned short PixelType;
typedef ora::PortalImagingDeviceProjectionProperties<PixelType> PropsType;
typedef ora::TranslationScaleFlexMapCorrection<PixelType> FlexType;

/** Write a flexmap CSV file. **/
void WriteFlexMapCurveCSV(std::ofstream &csv, std::string fileName,
    const double *x, const double *y, const int N)
{
  if (!CSVOutput)
    return;
  csv.open(fileName.c_str(), std::ios::out);
  if (csv.is_open())
  {
    csv << "angle;correction\n";
    for (int i = 0; i < N; i++)
      csv << x[i] << ";" << y[i] << "\n";
    csv.close();
  }
}

/**
 * Helper for extracting interpolated flex map value.
 * @param typeID 0=x-cw,1=x-ccw,2=y-cw,3=y-ccw,4=z-cw,5=z-ccw
 **/
double GetInterpolatedFlexMapValue(double x, int typeID,
    FlexType::Pointer flexMap)
{
  if (typeID >= 0 && typeID < 6 && flexMap)
  {
    if (typeID == 0)
      return flexMap->GetInterpolatedXTranslationCWYAtX(x);
    else if (typeID == 1)
      return flexMap->GetInterpolatedXTranslationCCWYAtX(x);
    else if (typeID == 2)
      return flexMap->GetInterpolatedYTranslationCWYAtX(x);
    else if (typeID == 3)
      return flexMap->GetInterpolatedYTranslationCCWYAtX(x);
    else if (typeID == 4)
      return flexMap->GetInterpolatedZTranslationCWYAtX(x);
    else if (typeID == 5)
      return flexMap->GetInterpolatedZTranslationCCWYAtX(x);
    return 0.;
  }
  else
    return 0.;
}

/**
 * Helper for extracting orientation of interest.
 * @param typeID 0=x-cw,1=x-ccw,2=y-cw,3=y-ccw,4=z-cw,5=z-ccw
 **/
double *ExtractOrientationDirection(PropsType::MatrixType &orient, int typeID)
{
  if (typeID >= 0 && typeID < 6)
  {
    if (typeID == 0 || typeID == 1)
      return orient[0];
    else if (typeID == 2 || typeID == 3)
      return orient[1];
    else if (typeID == 4 || typeID == 5)
      return orient[2];
    else
      return NULL;
  }
  else
    return NULL;
}

/**
 * Validate a corrected curve (origin only) against a reference curve.
 * @param typeID 0=x-cw,1=x-ccw,2=y-cw,3=y-ccw,4=z-cw,5=z-ccw
 **/
bool ValidateCorrectedCurve(std::vector<PropsType::PointType> &origin,
    std::vector<PropsType::MatrixType> &orient, std::vector<
        PropsType::PointType> &focus,
    std::vector<PropsType::PointType> &refOrigin, std::vector<
        PropsType::MatrixType> &refOrient,
    std::vector<PropsType::PointType> &refFocus, int typeID,
    FlexType::Pointer flexMap, std::vector<double> refAngles)
{
  if (typeID >= 0 && typeID < 6 && flexMap)
  {
    if (origin.size() == refOrigin.size() && orient.size() == refOrient.size()
        && focus.size() == refFocus.size() && refAngles.size()
        == refOrigin.size())
    {
      bool ok = true;
      // expected: unmodified orientation
      for (unsigned int i = 0; i < orient.size(); i++)
      {
        for (int d1 = 0; d1 < 3; d1++)
          for (int d2 = 0; d2 < 3; d2++)
            if (fabs(orient[i][d1][d2] - refOrient[i][d1][d2]) > 1e-6)
              ok = false;
      }
      // expected: unmodified focal spot position
      for (unsigned int i = 0; i < focus.size(); i++)
      {
        for (int d1 = 0; d1 < 3; d1++)
          if (fabs(focus[i][d1] - refFocus[i][d1]) > 1e-6)
            ok = false;
      }
      // expected: partial correction according to interpolated flex map value
      std::ofstream csv;
      if (CSVOutput)
      {
        std::string fn = "flex_validation";
        if (typeID == 0)
          fn += "_cw_x";
        else if (typeID == 1)
          fn += "_ccw_x";
        else if (typeID == 2)
          fn += "_cw_y";
        else if (typeID == 3)
          fn += "_ccw_y";
        else if (typeID == 4)
          fn += "_cw_z";
        else if (typeID == 5)
          fn += "_ccw_z";
        if (flexMap->GetUseSplineInterpolation())
          fn += "_spline.csv";
        else
          fn += "_linear.csv";
        csv.open(fn.c_str(), std::ios::out);
        if (csv.is_open())
          csv << "angle;ref-proj;corr-proj;corr-value;error\n";
      }
      double *v = NULL;
      for (unsigned int i = 0; i < origin.size(); i++)
      {
        v = ExtractOrientationDirection(orient[i], typeID);
        if (v) // NOTE: v is normalized!
        {
          double co[3];
          co[0] = origin[i][0];
          co[1] = origin[i][1];
          co[2] = origin[i][2];
          double tcorr = vtkMath::Dot(co, v);
          double ro[3];
          ro[0] = refOrigin[i][0];
          ro[1] = refOrigin[i][1];
          ro[2] = refOrigin[i][2];
          double tref = vtkMath::Dot(ro, v);
          double tflex = GetInterpolatedFlexMapValue(refAngles[i], typeID,
              flexMap);
          if (fabs(tcorr - tref - tflex) > 1e-3)
          {
            ok = false;
          }
          if (CSVOutput && csv.is_open())
            csv << refAngles[i] << ";" << tref << ";" << tcorr << ";" << tflex
                << ";" << fabs(tcorr - tref - tflex) << "\n";
        }
        else
          ok = false;
      }
      if (CSVOutput && csv.is_open())
        csv.close();

      return ok;
    }
    else
      return false;
  }
  else
    return false;
}

/**
 * Tests base functionality of:
 *
 *   ora::PortalImagingDeviceProjectionProperties
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::PortalImagingDeviceProjectionProperties
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.4
 *
 * \ingroup Tests
 */
int main(int argc, char *argv[])
{
  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  helpLines.push_back(
      "  -co or --csv-output ... extended CSV (comma-separated file) output");
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

  VERBOSE(<< "\nTesting portal imaging device (MV imaging) projection properties.\n")
  bool ok = true;
  PropsType::Pointer props = PropsType::New();

  VERBOSE(<< "  * Basic requirements check ... ")
  if (props->IsGeometryValid())
    ok = false;
  PropsType::SizeType psz;
  psz[0] = 1024;
  psz[1] = 410;
  props->SetProjectionSize(psz);
  if (props->IsGeometryValid())
    ok = false;
  PropsType::SpacingType psp;
  psp[0] = 0.4;
  psp[1] = 1.0;
  props->SetProjectionSpacing(psp);
  if (props->IsGeometryValid())
    ok = false;
  props->SetSourceAxisDistance(1000.);
  if (props->IsGeometryValid())
    ok = false;
  props->SetSourceFilmDistance(1500.);
  if (props->IsGeometryValid())
    ok = false;
  props->SetLinacGantryDirection(0); // CW
  if (props->IsGeometryValid())
    ok = false;
  props->SetLinacGantryAngle(0);
  if (props->IsGeometryValid())
    ok = false;
  if (!props->Update())
    ok = false;
  if (!props->IsGeometryValid())
    ok = false;
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Stress test ... ")
  std::vector<PropsType::PointType> cwRefOrigins;
  std::vector<PropsType::PointType> ccwRefOrigins;
  std::vector<PropsType::MatrixType> cwRefOrients;
  std::vector<PropsType::MatrixType> ccwRefOrients;
  std::vector<PropsType::PointType> cwRefFocus;
  std::vector<PropsType::PointType> ccwRefFocus;
  std::vector<double> refAngles;
  if (ok)
  {
    int endcount = 10000;
    double diff = (180. - (-180)) / (double) endcount;
    std::ofstream csv;
    for (int j = 0; j <= 1; j++)
    {
      if (CSVOutput)
      {
        if (j == 0)
          csv.open("orig_cw.csv", std::ios::out);
        else
          csv.open("orig_ccw.csv", std::ios::out);
        if (csv.is_open())
          csv << "angle;orig-x;orig-y;orig-z\n";
        else
          ok = false;
      }
      for (int i = 1; i <= endcount; i++)
      {
        double a = -180. + diff * (double) i;
        a = a / 180. * M_PI;
        props->SetLinacGantryAngle(a);
        props->SetLinacGantryDirection(j);
        if (!props->Update() || !props->IsGeometryValid())
        {
          ok = false;
          break;
        }
        try
        {
          // just to test accessibility
          PropsType::MatrixType orient = props->GetProjectionPlaneOrientation();
          PropsType::PointType origin = props->GetProjectionPlaneOrigin();
          PropsType::PointType focus = props->GetSourceFocalSpotPosition();
          if (CSVOutput && csv.is_open())
            csv << FlexType::ProjectAngleInto0To2PIRange(a) << ";" << origin[0]
                << ";" << origin[1] << ";" << origin[2] << "\n";
          if (i % 25 == 0) // store each 25th plane origin as reference
          {
            if (j == 0)
            {
              refAngles.push_back(FlexType::ProjectAngleInto0To2PIRange(a));
              cwRefOrigins.push_back(origin);
              cwRefOrients.push_back(orient);
              cwRefFocus.push_back(focus);
            }
            else
            {
              ccwRefOrigins.push_back(origin);
              ccwRefOrients.push_back(orient);
              ccwRefFocus.push_back(focus);
            }
          }
        }
        catch (...)
        {
          ok = false;
          break;
        }
      }
      if (CSVOutput && csv.is_open())
        csv.close();
      if (!ok)
        break;
    }
  }
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Flex map integration ... ")
  std::vector<PropsType::PointType> cwFlexOriginsLin;
  std::vector<PropsType::PointType> cwFlexOriginsQuad;
  std::vector<PropsType::PointType> ccwFlexOriginsLin;
  std::vector<PropsType::PointType> ccwFlexOriginsQuad;
  std::vector<PropsType::PointType> cwFlexFocusLin;
  std::vector<PropsType::PointType> cwFlexFocusQuad;
  std::vector<PropsType::PointType> ccwFlexFocusLin;
  std::vector<PropsType::PointType> ccwFlexFocusQuad;
  std::vector<PropsType::MatrixType> cwFlexOrientsLin;
  std::vector<PropsType::MatrixType> cwFlexOrientsQuad;
  std::vector<PropsType::MatrixType> ccwFlexOrientsLin;
  std::vector<PropsType::MatrixType> ccwFlexOrientsQuad;
  FlexType::Pointer flexMap = FlexType::New();
  if (ok)
  {
    flexMap->SetEssentialsOnly(true);
    if (flexMap->Correct()) // no linac projection props set!
      ok = false;
    flexMap->SetEssentialsOnly(false);
    if (flexMap->Correct()) // no linac projection props set!
      ok = false;
    flexMap->SetLinacProjProps(props);
    flexMap->SetEssentialsOnly(true);
    if (!flexMap->Correct()) // must succeed
      ok = false;
    flexMap->SetEssentialsOnly(false);
    if (!flexMap->Correct()) // must succeed
      ok = false;
    // set correction factors by typical ORA strings (this tests both ORA string
    // setting and direct setting as internally the direct methods are called)
    std::string
        xCWCorrStr =
            "0,20.26,10,20.23,20,20.20,30,20.20,40,20.15,50,20.10,60,20.05,70,20.05,80,20.05,90,19.99,100,20.05,110,20.05,120,20.05,130,20.05,140,20.05,150,20.05,160,20.10,170,20.15,180,20.15,190,20.20,200,20.25,210,20.25,220,20.35,230,20.40,240,20.45,250,20.45,260,20.50,270,20.50,275,20.50,280,20.50,290,20.50,300,20.45,310,20.45,320,20.40,330,20.40,340,20.35,350,20.30,360,20.26";
    std::string
        xCCWCorrStr =
            "0,20.20,10,20.18,20,20.05,30,20.10,40,20.05,50,20.05,60,20.00,70,20.05,80,20.05,90,20.05,100,20.05,110,20.05,120,20.05,130,20.05,140,20.10,150,20.10,160,20.15,170,20.15,175,20.15,180,20.20,190,20.25,200,20.30,210,20.30,220,20.35,230,20.40,240,20.45,250,20.50,260,20.45,265,20.45,271,20.50,280,20.45,290,20.40,300,20.40,310,20.40,320,20.40,330,20.35,340,20.30,350,20.25,360,20.20";
    std::string
        yCWCorrStr =
            "0,20.37,10,20.35,20,20.35,30,20.34,40,20.40,50,20.40,60,20.42,70,20.40,80,20.40,90,20.45,100,20.50,110,20.50,120,20.55,130,20.55,140,20.55,150,20.60,160,20.60,170,20.60,180,20.60,190,20.60,200,20.60,210,20.60,220,20.60,230,20.55,240,20.55,250,20.52,260,20.50,270,20.50,275,20.45,280,20.45,290,20.40,300,20.40,310,20.35,320,20.40,330,20.35,340,20.40,350,20.40,360,20.37";
    std::string
        yCCWCorrStr =
            "0,20.35,10,20.35,20,20.40,30,20.35,40,20.40,50,20.40,60,20.40,70,20.40,80,20.45,90,20.45,100,20.45,110,20.50,120,20.55,130,20.60,140,20.60,150,20.60,160,20.65,170,20.65,175,20.65,180,20.65,190,20.60,200,20.60,210,20.60,220,20.60,230,20.60,240,20.55,250,20.55,260,20.50,265,20.50,271,20.50,280,20.40,290,20.45,300,20.40,310,20.40,320,20.40,330,20.35,340,20.35,350,20.35,360,20.35";
    std::string
        zCWCorrStr =
            "0,154.00,10,154.00,20,154.00,30,154.00,40,154.00,50,154.00,60,154.00,70,154.00,80,154.00,90,153.50,100,153.50,110,153.50,120,153.00,130,153.00,140,153.00,150,153.00,160,153.00,170,153.00,180,153.00,190,153.00,200,153.00,210,153.00,220,153.00,230,153.00,240,153.00,250,153.00,260,153.00,270,153.00,280,153.25,290,153.50,300,153.50,310,153.50,320,153.50,330,154.00,340,154.00,350,154.00,360,154.00";
    std::string
        zCCWCorrStr =
            "0,154.30,10,154.00,20,154.10,30,154.00,40,153.00,50,154.00,60,154.00,70,154.50,80,154.00,90,153.88,100,153.50,110,153.50,120,153.00,130,153.00,140,153.00,150,153.00,160,153.00,170,153.00,180,153.00,190,153.00,200,153.00,210,153.00,220,153.00,230,153.00,240,153.00,250,153.00,260,153.00,270,153.00,280,151.25,290,153.50,300,153.50,310,153.50,320,155.50,330,151.00,340,154.00,350,152.00,360,154.00";
    flexMap->SetXTranslationCWByORAString(xCWCorrStr, 20.5, -0.065);
    flexMap->SetXTranslationCCWByORAString(xCCWCorrStr, 20.5, 0.12);
    flexMap->SetYTranslationCWByORAString(yCWCorrStr, 20.5, -0.091);
    flexMap->SetYTranslationCCWByORAString(yCCWCorrStr, 20.5, -0.101);
    flexMap->SetZTranslationCWByORAString(zCWCorrStr, 150.0, 0.096);
    flexMap->SetZTranslationCCWByORAString(zCCWCorrStr, 150.0, -0.07);
    std::ofstream csv;
    WriteFlexMapCurveCSV(csv, "flex_map_x_cw.csv",
        flexMap->GetXTranslationCWX(), flexMap->GetXTranslationCWY(),
        flexMap->GetXTranslationCWN());
    WriteFlexMapCurveCSV(csv, "flex_map_x_ccw.csv",
        flexMap->GetXTranslationCCWX(), flexMap->GetXTranslationCCWY(),
        flexMap->GetXTranslationCCWN());
    WriteFlexMapCurveCSV(csv, "flex_map_y_cw.csv",
        flexMap->GetYTranslationCWX(), flexMap->GetYTranslationCWY(),
        flexMap->GetYTranslationCWN());
    WriteFlexMapCurveCSV(csv, "flex_map_y_ccw.csv",
        flexMap->GetYTranslationCCWX(), flexMap->GetYTranslationCCWY(),
        flexMap->GetYTranslationCCWN());
    WriteFlexMapCurveCSV(csv, "flex_map_z_cw.csv",
        flexMap->GetZTranslationCWX(), flexMap->GetZTranslationCWY(),
        flexMap->GetZTranslationCWN());
    WriteFlexMapCurveCSV(csv, "flex_map_z_ccw.csv",
        flexMap->GetZTranslationCCWX(), flexMap->GetZTranslationCCWY(),
        flexMap->GetZTranslationCCWN());
    flexMap->SetLinacProjProps(NULL); // set back (-> test auto-referencing)
    props->SetFlexMap(flexMap);
    if (flexMap->GetLinacProjProps() != props) // auto-referencing!
      ok = false;
    if (ok) // test flex map application
    {
      int endcount = 10000;
      double diff = (180. - (-180)) / (double) endcount;
      std::ofstream csv;
      for (int k = 0; k <= 1; k++)
      {
        if (k == 0)
          flexMap->UseSplineInterpolationOff();
        else
          flexMap->UseSplineInterpolationOn();
        for (int j = 0; j <= 1; j++)
        {
          if (CSVOutput)
          {
            std::string dirs = "_cw";
            std::string modes = "_linear";
            if (j == 1)
              dirs = "_ccw";
            if (k == 1)
              modes = "_quadratic";
            std::string fn = "corr" + dirs + modes;
            fn += ".csv";
            csv.open(fn.c_str(), std::ios::out);
            if (csv.is_open())
              csv << "angle;corr-x;corr-y;corr-z\n";
            else
              ok = false;
          }
          for (int i = 1; i <= endcount; i++)
          {
            double a = -180. + diff * (double) i;
            a = a / 180. * M_PI;
            props->SetLinacGantryAngle(a);
            props->SetLinacGantryDirection(j);
            if (!props->Update() || !props->IsGeometryValid())
            {
              ok = false;
              break;
            }
            try
            {
              // just to test accessibility
              PropsType::MatrixType orient =
                  props->GetProjectionPlaneOrientation();
              PropsType::PointType origin = props->GetProjectionPlaneOrigin();
              PropsType::PointType focus = props->GetSourceFocalSpotPosition();
              if (CSVOutput && csv.is_open())
                csv << FlexType::ProjectAngleInto0To2PIRange(a) << ";"
                    << origin[0] << ";" << origin[1] << ";" << origin[2]
                    << "\n";
              if (i % 25 == 0)
              {
                if (j == 0)
                {
                  if (k == 0)
                  {
                    cwFlexOriginsLin.push_back(origin);
                    cwFlexFocusLin.push_back(focus);
                    cwFlexOrientsLin.push_back(orient);
                  }
                  else
                  {
                    cwFlexOriginsQuad.push_back(origin);
                    cwFlexFocusQuad.push_back(focus);
                    cwFlexOrientsQuad.push_back(orient);
                  }
                }
                else
                {
                  if (k == 0)
                  {
                    ccwFlexOriginsLin.push_back(origin);
                    ccwFlexFocusLin.push_back(focus);
                    ccwFlexOrientsLin.push_back(orient);
                  }
                  else
                  {
                    ccwFlexOriginsQuad.push_back(origin);
                    ccwFlexFocusQuad.push_back(focus);
                    ccwFlexOrientsQuad.push_back(orient);
                  }
                }
              }
            }
            catch (...)
            {
              ok = false;
              break;
            }
          }
          if (CSVOutput && csv.is_open())
            csv.close();
          if (!ok)
            break;
        }
      }
    }

  }
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Check flex map correction ... ")
  if (ok)
  {
    flexMap->SetUseSplineInterpolation(false); // LINEAR
    // CW
    ok = ok && ValidateCorrectedCurve(cwFlexOriginsLin, cwFlexOrientsLin,
        cwFlexFocusLin, cwRefOrigins, cwRefOrients, cwRefFocus, 0, flexMap,
        refAngles);
    ok = ok && ValidateCorrectedCurve(cwFlexOriginsLin, cwFlexOrientsLin,
        cwFlexFocusLin, cwRefOrigins, cwRefOrients, cwRefFocus, 2, flexMap,
        refAngles);
    ok = ok && ValidateCorrectedCurve(cwFlexOriginsLin, cwFlexOrientsLin,
        cwFlexFocusLin, cwRefOrigins, cwRefOrients, cwRefFocus, 4, flexMap,
        refAngles);
    // CCW
    ok = ok && ValidateCorrectedCurve(ccwFlexOriginsLin, ccwFlexOrientsLin,
        ccwFlexFocusLin, ccwRefOrigins, ccwRefOrients, ccwRefFocus, 1, flexMap,
        refAngles);
    ok = ok && ValidateCorrectedCurve(ccwFlexOriginsLin, ccwFlexOrientsLin,
        ccwFlexFocusLin, ccwRefOrigins, ccwRefOrients, ccwRefFocus, 3, flexMap,
        refAngles);
    ok = ok && ValidateCorrectedCurve(ccwFlexOriginsLin, ccwFlexOrientsLin,
        ccwFlexFocusLin, ccwRefOrigins, ccwRefOrients, ccwRefFocus, 5, flexMap,
        refAngles);
    flexMap->SetUseSplineInterpolation(true); // QUADRATIC
    // CW
    ok = ok && ValidateCorrectedCurve(cwFlexOriginsQuad, cwFlexOrientsQuad,
        cwFlexFocusQuad, cwRefOrigins, cwRefOrients, cwRefFocus, 0, flexMap,
        refAngles);
    ok = ok && ValidateCorrectedCurve(cwFlexOriginsQuad, cwFlexOrientsQuad,
        cwFlexFocusQuad, cwRefOrigins, cwRefOrients, cwRefFocus, 2, flexMap,
        refAngles);
    ok = ok && ValidateCorrectedCurve(cwFlexOriginsQuad, cwFlexOrientsQuad,
        cwFlexFocusQuad, cwRefOrigins, cwRefOrients, cwRefFocus, 4, flexMap,
        refAngles);
    // CCW
    ok = ok && ValidateCorrectedCurve(ccwFlexOriginsQuad, ccwFlexOrientsQuad,
        ccwFlexFocusQuad, ccwRefOrigins, ccwRefOrients, ccwRefFocus, 1,
        flexMap, refAngles);
    ok = ok && ValidateCorrectedCurve(ccwFlexOriginsQuad, ccwFlexOrientsQuad,
        ccwFlexFocusQuad, ccwRefOrigins, ccwRefOrients, ccwRefFocus, 3,
        flexMap, refAngles);
    ok = ok && ValidateCorrectedCurve(ccwFlexOriginsQuad, ccwFlexOrientsQuad,
        ccwFlexFocusQuad, ccwRefOrigins, ccwRefOrients, ccwRefFocus, 5,
        flexMap, refAngles);
  }
  VERBOSE(<< (ok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Final reference count check ... ")
  if (props && props->GetReferenceCount() == 2 && flexMap
      && flexMap->GetReferenceCount() == 1 && !(flexMap = NULL)
      && props->GetReferenceCount() == 1)
  {
    VERBOSE(<< "OK\n")
  }
  else
  {
    VERBOSE(<< "FAILURE\n")
    ok = false;
  }
  props = NULL; // reference counter must be zero!


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
