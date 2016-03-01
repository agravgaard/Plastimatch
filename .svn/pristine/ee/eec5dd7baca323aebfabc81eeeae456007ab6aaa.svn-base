//
#include <time.h>
#include <cstddef> /* Workaround bug in ITK 3.20 */
//
#include "BasicUnitTestIncludes.hxx"

#include "oraMultiResolutionPyramidMaskFilter.h"
//ITK
#include <itkImageRegionIterator.h>
#include <itkImageFileWriter.h>
#include <itkImage.h>
#include <itkEuler3DTransform.h>


// dot product of vector x and direction-matrix y (i-th column is considered)
#define DOT_dir(x, y, i) \
	(x[0] * y[0][i] + x[1] * y[1][i] + x[2] * y[2][i])

/**
 * Tests base functionality of:
 *
 *   ora::MultiResolutionPyramidMaskFilter
 *
 * Test application result is 0 (EXIT_SUCCESS) if SUCCESSFUL.
 *
 * Arguments: run test with -h or --help option to explore them!
 *
 * @see ora::MultiResolutionPyramidMaskFilter
 *
 * @author jeanluc
 * @author phil
 * @version 1.0
 *
 */
int main(int argc, char *argv[])
{
  typedef itk::Image<unsigned char, 3> MaskImageType;
  typedef ora::MultiResolutionPyramidMaskFilter<MaskImageType> PyramidMaskFilterType;
  typedef PyramidMaskFilterType::Pointer PyramidMaskFilterPointer;
  typedef itk::ImageRegionIterator<MaskImageType> MaskIteratorType;
  typedef itk::Euler3DTransform<double> TransformType;

  // basic command line pre-processing:
  std::string progName = "";
  std::vector<std::string> helpLines;
  if (!CheckBasicTestCommandLineArguments(argc, argv, progName))
  {
    PrintBasicTestUsage(progName.length() > 0 ? progName.c_str() : NULL, helpLines);
    return EXIT_SUCCESS;
  }

  bool ok = true; // OK-flag for whole test
  bool lok = true; // local OK-flag for a test sub-section

  VERBOSE(<< "\nTesting Functionality of multi-resolution pyramid filter for masks.\n")

  VERBOSE(<< "  * Generating a mask  ")
  lok = true;
  const unsigned char MASK_OUT = 0;
  const unsigned char MASK_IN = 255;
	MaskImageType::Pointer mask1 = MaskImageType::New();
	MaskImageType::RegionType maskRegion;
	MaskImageType::SizeType size;
	size[0] = 410;
	size[1] = 410;
	size[2] = 1; //single slice
	MaskImageType::IndexType index;
	index.Fill(0);
	maskRegion.SetIndex(index);
	maskRegion.SetSize(size);
	MaskImageType::SpacingType spacing;
	spacing[0] = 0.4;
	spacing[1] = 0.9;
	spacing[2] = 1.0;
	MaskImageType::PointType origin;
	origin[0] = -20;
	origin[1] = 33;
	origin[2] = 16;
	MaskImageType::DirectionType direction;
	TransformType::Pointer euler = TransformType::New();
	euler->SetRotation(0.02, -0.04, -0.01);
	TransformType::InputVectorType vd;
	vd[0] = 1; vd[1] = 0; vd[2] = 0; // row-dir
	vd = euler->TransformVector(vd);
	direction[0][0] = vd[0]; direction[1][0] = vd[1]; direction[2][0] = vd[2]; // (col-based)
	vd[0] = 0; vd[1] = 1; vd[2] = 0; // col-dir
	vd = euler->TransformVector(vd);
	direction[0][1] = vd[0]; direction[1][1] = vd[1]; direction[2][1] = vd[2]; // (col-based)
	vd[0] = 0; vd[1] = 0; vd[2] = 1; // slice-dir
	vd = euler->TransformVector(vd);
	direction[0][2] = vd[0]; direction[1][2] = vd[1]; direction[2][2] = vd[2]; // (col-based)
	mask1->SetSpacing(spacing);
	mask1->SetRegions(maskRegion);
	mask1->SetOrigin(origin);
	mask1->SetDirection(direction);
	mask1->Allocate();
	MaskIteratorType mit(mask1, mask1->GetLargestPossibleRegion());
	for (mit.GoToBegin(); !mit.IsAtEnd(); ++mit)
	{
	 index = mit.GetIndex();
	 if (index[0] < 30 || index[0] > 380 || index[1] < 30 || index[1] > 380 )
		 mit.Set(MASK_OUT);
	 else
		 mit.Set(MASK_IN);
	}
	if (ImageOutput)
	{
		typedef itk::ImageFileWriter<MaskImageType> MaskWriterType;
		MaskWriterType::Pointer dw = MaskWriterType::New();
		char sbuff[100];
		sprintf(sbuff, "mrpyr-mask1.mhd");
		dw->SetFileName(sbuff);
		dw->SetInput(mask1);
		try
		{
		 dw->Update();
		}
		catch (itk::ExceptionObject &e)
		{
		 lok = false;
		}
  }
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  VERBOSE(<< "  * Initializing pyramid filter  ")
  lok = true; // initialize sub-section's success state
  const int levels = 3;
  PyramidMaskFilterPointer pyramidMaskFilter = PyramidMaskFilterType::New();
  if (!pyramidMaskFilter)
    lok = false;
  pyramidMaskFilter->SetNumberOfLevels(levels);
  pyramidMaskFilter->SetInput(mask1);
  // Set schedule
  PyramidMaskFilterType::ScheduleType schedule;
  schedule.SetSize(levels, 3);
  unsigned int d, l;
  for (l = 0; l < (unsigned int) levels; l++)
  {
    double sch = pow(2, levels - 1.0 - l);
    for (d = 0; d < 2; d++)
      schedule[l][d] = static_cast<unsigned int>(sch);
  }
  ok = ok && lok; // update OK-flag for test-scope
  VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

  std::string identifier = "";
  for (int k = 0; k < 2; k++)
  {
  	if (k == 0)
  	{
  		pyramidMaskFilter->SetUseShrinkImageFilter(true); // shrink filter
  		identifier = " [SHRINK-FILTER] ... ";
  	}
  	else
  	{
  		pyramidMaskFilter->SetUseShrinkImageFilter(false); // resample filter
  		identifier = " [RESAMPLE-FILTER] ... ";
  	}
  	try
  	{
  		pyramidMaskFilter->Update();
  	}
  	catch (itk::ExceptionObject &e)
  	{
  		ok = false;
  	}

		VERBOSE(<< "  * Generating scaled masks" << identifier)
		lok = true; // initialize sub-section's success state
		MaskImageType::Pointer masks[levels];
		pyramidMaskFilter->SetSchedule(schedule);
		for (unsigned int i = 0; i < (unsigned int)levels; i++)
		{
			masks[i] = pyramidMaskFilter->GetOutput(i);
			if (!masks[i])
				lok = false;
			if (ImageOutput)
			{
				typedef itk::ImageFileWriter<MaskImageType> MaskWriterType;
				MaskWriterType::Pointer dw = MaskWriterType::New();
				char sbuff[100];
				if (k == 0)
					sprintf(sbuff, "mrpyr-mask_shrinked_level%d.mhd", i);
				else
					sprintf(sbuff, "mrpyr-mask_resampled_level%d.mhd", i);
				dw->SetFileName(sbuff);
				dw->SetInput(masks[i]);
				try
				{
					dw->Update();
				}
				catch (itk::ExceptionObject &e)
				{
					lok = false;
				}
			}
		}
		ok = ok && lok; // update OK-flag for test-scope
		VERBOSE(<< (lok ? "OK" : "FAILURE") << "\n")

		VERBOSE(<< "  * Checking mask image properties" << identifier)
		lok = true; // initialize sub-section's success state
		for (int i = 0; i < levels; i++)
		{
			if (!masks[i])
			{
				lok = false;
				continue;
			}
			// check whether there are interpolated values (not allowed!):
			{
				MaskIteratorType mi(masks[i], masks[i]->GetLargestPossibleRegion());
				for (mi.GoToBegin(); !mi.IsAtEnd(); ++mi)
				{
					if (mi.Get() != MASK_IN && mi.Get() != MASK_OUT)
					{
						lok = false;
						break;
					}
				}
			}
			// check mask size
			MaskImageType::RegionType::SizeType sampledSize =
					masks[i]->GetLargestPossibleRegion().GetSize();
			if (fabs(size[0] / pow(2.0, levels - i - 1.0) - sampledSize[0]) >= 1 ||
					fabs(size[1] / pow(2.0, levels - i - 1.0) - sampledSize[1]) >= 1)
				lok = false;
			// check spacing
			MaskImageType::SpacingType maskSpacing = masks[i]->GetSpacing();
			if (fabs(spacing[0] * pow(2.0, levels - i - 1.0) - maskSpacing[0]) >= 1e-6 ||
					fabs(spacing[1] * pow(2.0, levels - i - 1.0) - maskSpacing[1]) >= 1e-6)
				lok = false;
			// check whether direction was preserved
			MaskImageType::DirectionType maskDirection = masks[i]->GetDirection();
			for (int r = 0; r < 3; r++)
				for (int c = 0; c < 3; c++)
					if (fabs(direction[r][c] - maskDirection[r][c]) > 1e-6)
						lok = false;
			// origin should not be off-set more than 1 pixel w.r.t. direction
			MaskImageType::PointType maskOrigin = masks[i]->GetOrigin();
			for (int d = 0; d < 3; d++)
			{
				double dmask = DOT_dir(maskOrigin, maskDirection, d);
				double dref = DOT_dir(origin, maskDirection, d);
				if (fabs(dmask - dref) >= maskSpacing[d])
					lok = false;
			}
			// all but the outer pixels should have a corresponding "partner" in the
			// source mask image in terms of physical coordinates
			{
				// - get border pixel indices
				MaskImageType::IndexValueType xmin = size[0];
				MaskImageType::IndexValueType xmax = 0;
				MaskImageType::IndexValueType ymin = size[1];
				MaskImageType::IndexValueType ymax = 0;
				MaskIteratorType mi(masks[i], masks[i]->GetLargestPossibleRegion());
				for (mi.GoToBegin(); !mi.IsAtEnd(); ++mi)
				{
					if (mi.Get() == MASK_IN)
					{
						if (mi.GetIndex()[0] > xmax)
							xmax = mi.GetIndex()[0];
						if (mi.GetIndex()[0] < xmin)
							xmin = mi.GetIndex()[0];
						if (mi.GetIndex()[1] > ymax)
							ymax = mi.GetIndex()[1];
						if (mi.GetIndex()[1] < ymin)
							ymin = mi.GetIndex()[1];
					}
				}
				// - now search for corresponding pixels in the original mask (do not
				//   consider the border mask pixels)
				MaskIteratorType refi(mask1, mask1->GetLargestPossibleRegion());
				for (mi.GoToBegin(); !mi.IsAtEnd(); ++mi)
				{
					if (mi.Get() == MASK_IN)
					{
						if (mi.GetIndex()[0] > xmin &&
								mi.GetIndex()[0] < xmax &&
								mi.GetIndex()[1] > ymin &&
								mi.GetIndex()[1] < ymax)
						{
							MaskImageType::PointType phys;
							masks[i]->TransformIndexToPhysicalPoint(mi.GetIndex(), phys);
							MaskImageType::IndexType refIdx;
							mask1->TransformPhysicalPointToIndex(phys, refIdx);
							refi.SetIndex(refIdx);
							if (refi.Get() != MASK_IN)
								lok = false;
						}
					}
				}
			}
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
