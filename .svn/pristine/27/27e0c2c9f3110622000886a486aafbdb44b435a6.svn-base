/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "aperture.h"
#include "file_util.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "proj_volume.h"
#include "ray_trace_probe.h"
#include "rpl_volume.h"
#include "volume.h"
#include "volume_limit.h"
#include "wed_parms.h"

//For sure
#include "print_and_exit.h"
#include <fstream>
#include "plm_image.h"
#include "plm_image_header.h"
#include "itkImageRegionIterator.h"

#include "pcmd_synth.h"
#include "pcmd_synth.cxx"
#include "synthetic_mha.h"


void
resize_3d_vect(std::vector< std::vector< std::vector<float> > > &input_vector, plm_long size[])
{

  input_vector.resize( size[0] );
  for (int i=0;i!=size[0];++i)  {
    input_vector[i].resize( size[1] );
    for (int j=0;j!= size[1];++j)  {
      input_vector[i][j].resize( size[2] );
      for (int k=0;k!= size[2];++k)  {
	input_vector[i][j][k]=0.;
      }
    }
  }


}


int
main (int argc, char* argv[])
{
    if (argc != 4) {
        printf ("Usage: filter input_fn reference_fn output_fn\n");
        exit (1);
    }

#if defined (commentout)    
    std::string Reference_FN = "s4mid.mha";
    std::string Input_FN = "Drift_added_fromraw.mha";
    std::string Output_FN = "Filtered_output.mha";
#endif

    std::string Reference_FN = argv[2];
    std::string Input_FN = argv[1];
    std::string Output_FN = argv[3];

    if (!file_exists (Reference_FN)) {
        print_and_exit ("Cannot open %s for read\n",
            Reference_FN.c_str());
    }
    if (!file_exists (Input_FN)) {
        print_and_exit ("Cannot open %s for read\n",
            Input_FN.c_str());
    }

    typedef itk::ImageRegionIterator< FloatImageType > FloatIteratorType;
 

    Plm_image_header image_header;
    plm_long image_dim[3];
    float spacing[3];
    float origin[3];


    Plm_image *input_image = new Plm_image();
    std::vector< std::vector< std::vector<float> > >  input_vect, base_input_vect, ct_input_vect;

    input_image->load_native(Input_FN);
    image_header.set_from_plm_image(input_image);
    image_header.get_dim(image_dim);
    image_header.get_spacing(spacing);
    image_header.get_origin(origin);

    Synthetic_mha_main_parms output_parms;
    for (int i=0;i!=3;++i)  {
        output_parms.sm_parms.dim[i] = image_dim[i];
        output_parms.sm_parms.spacing[i] = spacing[i];
        output_parms.sm_parms.origin[i] = origin[i];
        output_parms.sm_parms.background = 0.; //set background to 0 for wed doses
    }
    output_parms.output_fn = Output_FN;
    do_synthetic_mha(&output_parms);
    std::cout<<"Empty file \""<<Output_FN<<"\" written to disk successfully"<<std::endl;
    std::cout<<"Attempting to assign added values..."<<std::endl;
  
    //Initial dose image////////////////////////////
  
    //Image to floats
    FloatImageType::Pointer img = input_image->m_itk_float;
    FloatImageType::RegionType rg = img->GetLargestPossibleRegion ();
    FloatIteratorType image_it (img, rg);
  
    resize_3d_vect(input_vect,image_dim);
    resize_3d_vect(base_input_vect,image_dim);
    
    int zz = 0;
    plm_long ijk[3];
    for (image_it.GoToBegin(); !image_it.IsAtEnd(); ++image_it) {
    
        COORDS_FROM_INDEX(ijk,zz,image_dim);
        input_vect[ ijk[0] ][ ijk[1] ][ ijk[2] ] = image_it.Get();
        base_input_vect[ ijk[0] ][ ijk[1] ][ ijk[2] ] = image_it.Get();

        zz++;
    }
    ///////////////////////////////////////////////////////



    //CT Image////////////////////////////

    Plm_image *ct_image = new Plm_image();

    Plm_image_header ct_image_header;
    plm_long ct_image_dim[3];
    float ct_spacing[3];
    float ct_origin[3];

    ct_image->load_native(Reference_FN);
    ct_image_header.set_from_plm_image(ct_image);
    ct_image_header.get_dim(ct_image_dim);
    ct_image_header.get_spacing(ct_spacing);
    ct_image_header.get_origin(ct_origin);

    Volume::Pointer ct_vol = ct_image->get_volume_float();
    float* ct_img = (float*) ct_vol->img;





    //  std::cout<<"origin "<<ct_origin[0]<<" "<<ct_origin[1]<<" "<<ct_origin[2]<<std::endl;
    /*
    //Image to floats
    FloatImageType::Pointer ct_img = ct_image->m_itk_float;
    FloatImageType::RegionType ct_rg = ct_img->GetLargestPossibleRegion ();
    FloatIteratorType ct_image_it (ct_img, ct_rg);
    */

    resize_3d_vect(ct_input_vect,ct_image_dim);


    int ct_vol_max = ct_image_dim[0]*ct_image_dim[1]*ct_image_dim[2];
    for (int i=0; i!=ct_vol_max;++i)  {
        COORDS_FROM_INDEX(ijk,i,ct_image_dim);
        ct_input_vect[ ijk[0] ][ ijk[1] ][ ijk[2] ] = ct_img[i];

    }

  
    float x_low,x_high,y_low,y_high,z_low,z_high;
    int x_low2,x_high2,y_low2,y_high2,z_low2,z_high2;

    for (int i=0; i!=ct_image_dim[0]; ++i)  {
        for (int j=0; j!=ct_image_dim[1]; ++j)  {
            for (int k=0; k!=ct_image_dim[2]; ++k)  {
	
                x_low = ct_origin[0] + (i-.5)*ct_spacing[0];
                x_high = x_low+ct_spacing[0];
                y_low = ct_origin[1] + (j-.5)*ct_spacing[1];
                y_high = y_low+ct_spacing[1];
                z_low = ct_origin[2] + (k-.5)*ct_spacing[2];
                z_high = z_low+ct_spacing[2];
	
                x_low2 = (int) floor((x_low-origin[0])/spacing[0]+.5);
                x_high2 = (int) floor((x_high-origin[0])/spacing[0]+.5);
                y_low2 = (int) floor((y_low-origin[1])/spacing[1]+.5);
                y_high2 = (int) floor((y_high-origin[1])/spacing[1]+.5);
                z_low2 = (int) floor((z_low-origin[2])/spacing[2]+.5);
                z_high2 = (int) floor((z_high-origin[2])/spacing[2]+.5);
	
                for (int ii=x_low2; ii<=x_high2; ++ii)  {
                    for (int jj=y_low2; jj<=y_high2; ++jj)  {
                        for (int kk=z_low2; kk<=z_high2; ++kk)  {
	      
                            float unit = 1.;
	      
                            //fraction of the input voxel in each added cell:
                            //additions from group_add - now we have to allow for the possibility of the 
                            //input have smaller voxels.  In group add, the inputs were guaranteed to
                            //have the same same size or larger.
	      
                            if (x_low2==x_high2)  { 
                                unit *= ( (1 - ((x_low-origin[0])/spacing[0]+.5 - x_low2)) - 
                                    (x_high2 - (x_high-origin[0])/spacing[0]+.5) );
                            }
                            else  {
                                if (ii==x_low2) {unit *= (1 - ((x_low-origin[0])/spacing[0]+.5 - x_low2));}
                                if (ii==x_high2) {unit *= ((x_high-origin[0])/spacing[0]+.5 - x_high2);}
                            }

                            if (y_low2==y_high2)  { 
                                unit *= ( (1 - ((y_low-origin[1])/spacing[1]+.5 - y_low2)) - 
                                    (y_high2 - (y_high-origin[1])/spacing[1]+.5) );
                            }
                            else  {
                                if (jj==y_low2) {unit *= (1 - ((y_low-origin[1])/spacing[1]+.5 - y_low2));}
                                if (jj==y_high2) {unit *= ((y_high-origin[1])/spacing[1]+.5 - y_high2);}
                            }

                            if (z_low2==z_high2)  { 
                                unit *= ( (1 - ((z_low-origin[2])/spacing[2]+.5 - z_low2)) -
                                    (z_high2 - (z_high-origin[2])/spacing[2]+.5) );
                            }
                            else  {
                                if (kk==z_low2) {unit *= (1 - ((z_low-origin[2])/spacing[2]+.5 - z_low2));}
                                if (kk==z_high2) {unit *= ((z_high-origin[2])/spacing[2]+.5 - z_high2);}
                            }


                            //     if ((x_low2<0)||(y_low2<0)||(z_low2<0)||(x_high2>=image_dim[0])||(y_high2>=image_dim[1])||(z_high2>=image_dim[2]))  {continue;}
                            if ((ii<0)||(jj<0)||(kk<0)||(ii>=image_dim[0])||(jj>=image_dim[1])||(kk>=image_dim[2]))  {continue;}


	      
                            if (ct_input_vect[i][j][k]==-1000)  {input_vect[ii][jj][kk]-=base_input_vect[ii][jj][kk]*unit;}


	      
                        }
                    }
                }
	
            }
        }
    }
  

    //Option to include full dose on any voxel that is partially in air (no interpolation):
    for (unsigned i=0; i!=base_input_vect.size(); ++i)  {
        for (unsigned j=0; j!=base_input_vect[0].size(); ++j)  {
            for (unsigned k=0; k!=base_input_vect[0][0].size(); ++k)  {
                //If the "filtered" value is less than the original, but still greater than 0, keep the original value.
                if ((input_vect[i][j][k] < base_input_vect[i][j][k])&&(input_vect[i][j][k] > .001*base_input_vect[i][j][k])) {
                    //	  std::cout<<input_vect[i][j][k]<<" "<<base_input_vect[i][j][k]<<std::endl;
                    input_vect[i][j][k] = base_input_vect[i][j][k];
                }
            }	    
        }
    }


    Plm_image *output_image = new Plm_image();
    output_image->load_native(Output_FN);

    FloatImageType::Pointer img_out = output_image->m_itk_float;
    FloatImageType::RegionType rg_out = img_out->GetLargestPossibleRegion ();
    FloatIteratorType image_out_it (img_out, rg_out);

    zz = 0;
    for (image_out_it.GoToBegin(); !image_out_it.IsAtEnd(); ++image_out_it) {

        COORDS_FROM_INDEX(ijk,zz,image_dim);
        image_out_it.Set( input_vect[ ijk[0] ][ ijk[1] ][ ijk[2] ] );

        zz++;
    }

    itk_image_save_float (img_out, Output_FN.c_str());

    delete output_image;
    delete input_image;

    return 0;
}
