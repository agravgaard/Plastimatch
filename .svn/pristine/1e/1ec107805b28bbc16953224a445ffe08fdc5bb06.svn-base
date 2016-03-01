//
#include "BasicUnitTestIncludes.hxx"

#include "oraVTKMeanImageFilter.h"

#include <vtksys/SystemTools.hxx>
#include <vtkSmartPointer.h>
#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkImageData.h>
#include <vtkTimerLog.h>


/** Check whether pixel intensities of the specified images match w.r.t. the
 * specified tolerance. ALL COMPONENTS ARE CHECKED! **/
bool CheckAgainstReferenceImage(vtkImageData *img1, vtkImageData *img2,
    double tolerance = 1e-6)
{
  if (!img1 || !img2)
    return false;

  int dims1[3];
  img1->GetDimensions(dims1);
  int dims2[3];
  img2->GetDimensions(dims2);
  if (dims1[0] != dims2[0] || dims1[1] != dims2[1] || dims1[2] != dims2[2])
    return false;
  if (img1->GetNumberOfScalarComponents() != img2->GetNumberOfScalarComponents())
    return false;

  // slow approach:
  double v1, v2;
  for (int c = 0; c < img1->GetNumberOfScalarComponents(); c++)
  {
    for (int x = 0; x < dims1[0]; x++)
    {
      for (int y = 0; y < dims1[1]; y++)
      {
        for (int z = 0; z < dims1[2]; z++)
        {
          v1 = img1->GetScalarComponentAsDouble(x, y, z, c);
          v2 = img2->GetScalarComponentAsDouble(x, y, z, c);
          if (fabs(v2 - v1) > tolerance)
            return false;
        }
      }
    }
  }

  return true;
}

/**
 * Tests base functionality of:
 *
 *   ora::VTKMeanImageFilter.
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::VTKMeanImageFilter
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
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL,
        helpLines, true, true);
    return EXIT_SUCCESS;
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting VTKMeanImageFilter.\n")

  VERBOSE(<< "  * Check test data availability ... ")
  lok = true; // initialize sub-section's success state
  std::string file2D = DataPath + "2dimage.mhd";
  if (!vtksys::SystemTools::FileExists(file2D.c_str()))
    lok = false;
  std::string file2DF = DataPath + "2dimage_float.mhd";
  if (!vtksys::SystemTools::FileExists(file2DF.c_str()))
    lok = false;
  vtkSmartPointer<vtkMetaImageReader> reader = vtkSmartPointer<vtkMetaImageReader>::New();
  reader->SetFileName(file2D.c_str());
  reader->Update();
  vtkSmartPointer<vtkImageData> image2D = vtkSmartPointer<vtkImageData>::New();
  image2D->DeepCopy(reader->GetOutput());
  vtkSmartPointer<vtkMetaImageReader> reader2 = vtkSmartPointer<vtkMetaImageReader>::New();
  reader2->SetFileName(file2DF.c_str());
  reader2->Update();
  vtkSmartPointer<vtkImageData> image2DF = vtkSmartPointer<vtkImageData>::New();
  image2DF->DeepCopy(reader2->GetOutput());
  reader = NULL;
  reader2 = NULL;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Test 2D image (USHORT) filtering ... ")
  lok = true; // initialize sub-section's success state
  vtkSmartPointer<vtkMetaImageWriter> w = vtkSmartPointer<vtkMetaImageWriter>::New();
  vtkSmartPointer<ora::VTKMeanImageFilter> f = vtkSmartPointer<ora::VTKMeanImageFilter>::New();
  vtkSmartPointer<vtkTimerLog> tl = vtkSmartPointer<vtkTimerLog>::New();
  vtkSmartPointer<vtkImageData> refImage;
  int check;
  std::string s, spre, spost;
  int nativeTh = f->GetNumberOfThreads();
  double mean = 0;
  for (int r = 1; r <= 20; r++)
  {
    refImage = NULL;
    for (int th = 1; th <= 4; th++)
    {
      f->SetInput(image2D);
      f->SetDimensionalityTo2();
      f->SetRadius(r);
      f->SetNumberOfThreads(th);

      tl->StartTimer();
      for (int c = 0; c < 10; c++)
      {
        f->Modified();
        f->Update();
      }
      tl->StopTimer();
      if (r == 1 && th == 1)
        mean = f->GetMeanIntensityOfOriginalImage(); // reference
      if (th == 1)
      {
        check = 2; // is reference round
        refImage = NULL;
        refImage = vtkSmartPointer<vtkImageData>::New();
        refImage->ShallowCopy(f->GetOutput()); // copy as reference
      }
      else
      {
        if (CheckAgainstReferenceImage(f->GetOutput(), refImage))
        {
          if (fabs(f->GetMeanIntensityOfOriginalImage() - mean) > 1e-6)
          {
            check = 0; // failure
            lok = false;
          }
          else
          {
            check = 1; // ok
          }
        }
        else
        {
          check = 0; // failure
          lok = false;
        }
      }
      if (check == 1)
        s = "ok";
      else if (check == 0)
        s = "failure";
      else
        s = "not necessary";
      if (th == nativeTh)
      {
        spre = " *** (SYSTEM-THREAD-COUNT) ";
        spost = " *** ";
      }
      else
      {
        spre = "";
        spost = "";
      }
      VERBOSE(<< "\n " << spre << " radius = " << r << " , #threads = " << th << ", avg = " << f->GetMeanIntensityOfOriginalImage() << " (check = " << s << "): " << (tl->GetElapsedTime() / 10. * 1000.) << " ms" << spost << "\n")

      if (ImageOutput)
      {
        char buff[200];
        sprintf(buff, "2dimage_smoothed_r%d_th%d.mhd", r, th);
        w->SetInput(f->GetOutput());
        w->SetCompression(false);
        w->SetFileName(buff);
        w->Write();
      }
    }
  }
  refImage = NULL;
  f = NULL;
  w = NULL;
  tl = NULL;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Test 2D image (FLOAT) filtering ... ")
  lok = true; // initialize sub-section's success state
  w = vtkSmartPointer<vtkMetaImageWriter>::New();
  f = vtkSmartPointer<ora::VTKMeanImageFilter>::New();
  tl = vtkSmartPointer<vtkTimerLog>::New();
  nativeTh = f->GetNumberOfThreads();
  mean = 0;
  for (int r = 1; r <= 20; r++)
  {
    refImage = NULL;
    for (int th = 1; th <= 4; th++)
    {
      f->SetInput(image2DF);
      f->SetDimensionalityTo2();
      f->SetRadius(r);
      f->SetNumberOfThreads(th);

      tl->StartTimer();
      for (int c = 0; c < 10; c++)
      {
        f->Modified();
        f->Update();
      }
      tl->StopTimer();
      if (r == 1 && th == 1)
        mean = f->GetMeanIntensityOfOriginalImage(); // reference
      if (th == 1)
      {
        check = 2; // is reference round
        refImage = NULL;
        refImage = vtkSmartPointer<vtkImageData>::New();
        refImage->ShallowCopy(f->GetOutput()); // copy as reference
      }
      else
      {
        if (CheckAgainstReferenceImage(f->GetOutput(), refImage))
        {
          if (fabs(f->GetMeanIntensityOfOriginalImage() - mean) > 1e-6)
          {
            check = 0; // failure
            lok = false;
          }
          else
          {
            check = 1; // ok
          }
        }
        else
        {
          check = 0; // failure
          lok = false;
        }
      }
      if (check == 1)
        s = "ok";
      else if (check == 0)
        s = "failure";
      else
        s = "not necessary";
      if (th == nativeTh)
      {
        spre = " *** (SYSTEM-THREAD-COUNT) ";
        spost = " *** ";
      }
      else
      {
        spre = "";
        spost = "";
      }
      VERBOSE(<< "\n " << spre << " radius = " << r << " , #threads = " << th << ", avg = " << f->GetMeanIntensityOfOriginalImage() << " (check = " << s << "): " << (tl->GetElapsedTime() / 10. * 1000.) << " ms" << spost << "\n")

      if (ImageOutput)
      {
        char buff[200];
        sprintf(buff, "2dimage_float_smoothed_r%d_th%d.mhd", r, th);
        w->SetInput(f->GetOutput());
        w->SetCompression(false);
        w->SetFileName(buff);
        w->Write();
      }
    }
  }
  refImage = NULL;
  f = NULL;
  w = NULL;
  tl = NULL;
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Test 3D image (USHORT) filtering ... ")
  lok = true;
  VERBOSE(<< " -- FIXME (implement 3D) -- ") // FIXME
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Test 3D image (FLOAT) filtering ... ")
  lok = true;
  VERBOSE(<< " -- FIXME (implement 3D) -- ") // FIXME
  ok = ok && lok; // update OK-flag for test-scope
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
