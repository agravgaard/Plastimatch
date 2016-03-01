/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include <iostream>
#include <fstream>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkDisplacementFieldJacobianDeterminantFilter.h>

#include "vf_jacobian.h"

typedef itk::DisplacementFieldJacobianDeterminantFilter<DeformationFieldType, float> JacobianFilterType;

Jacobian::Jacobian ()
{
    vf = 0;
    vfjacstats_fn = " ";
    jacobian_min = 0;
    jacobian_max = 0;
}

void 
Jacobian::set_output_vfstats_name (const std::string& vfjacstats)
{
    this->vfjacstats_fn = vfjacstats;
}

void 
Jacobian::set_input_vf(DeformationFieldType::Pointer vf)
{
    this->vf = vf;
}

void 
Jacobian::write_output_statistics(Jacobian_stats *JacoStats)
{
    FILE *fid;
  
    fid=fopen(JacoStats->outputstats_fn.c_str(),"w");
  
    if (fid != NULL)
    {
        fprintf(fid,"Min Jacobian: %.6f\n",JacoStats->min);
        fprintf(fid,"Max Jacobian: %.6f\n",JacoStats->max);
        fclose(fid);
    }
}

FloatImageType::Pointer
Jacobian::make_jacobian ()
{
    DeformationFieldType::Pointer deffield; 
    deffield= this->vf;
    
    JacobianFilterType::Pointer jacobianFilter = JacobianFilterType::New();
    jacobianFilter->SetInput( deffield );
    jacobianFilter->SetUseImageSpacing( true );
    jacobianFilter->Update();
    
    typedef itk::MinimumMaximumImageCalculator<FloatImageType> MinMaxFilterType;
    MinMaxFilterType::Pointer minmaxfilter = MinMaxFilterType::New();
    
    FloatImageType::Pointer outimg =jacobianFilter->GetOutput();
    
    try
    {
	minmaxfilter->SetImage(jacobianFilter->GetOutput());
    }
    catch( itk::ExceptionObject& err )
    {
        std::cout << "Unexpected error." << std::endl;
        std::cout << err << std::endl;
        exit( EXIT_FAILURE );
    }
    minmaxfilter->Compute();
    
    std::cout<<"Minimum of the determinant of the Jacobian of the warp: " <<minmaxfilter->GetMinimum()<<std::endl;
    std::cout<<"Maximum of the determinant of the Jacobian of the warp: " <<minmaxfilter->GetMaximum()<<std::endl;
    
    this->jacobian_min = minmaxfilter->GetMinimum();
    this->jacobian_max = minmaxfilter->GetMaximum();

    Jacobian_stats JacoStats;
    JacoStats.min = minmaxfilter->GetMinimum();
    JacoStats.max = minmaxfilter->GetMaximum();
    JacoStats.outputstats_fn = this->vfjacstats_fn;
    if (this->vfjacstats_fn != "") {
        this->write_output_statistics(&JacoStats);
    }
    return outimg;
}
