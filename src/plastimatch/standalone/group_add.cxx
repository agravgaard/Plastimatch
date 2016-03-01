/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "aperture.h"
#include "plm_math.h"
#include "proj_volume.h"
#include "ray_trace_probe.h"
#include "rpl_volume.h"
#include "rt_beam.h"
#include "rt_plan.h"
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


class group_add_parms  {

public:
  group_add_parms(){
    file="";
    for (int i=0;i!=3;++i)  {offset[i]=0.;}
    weight=0.;
  }
  

  std::string file;
  float offset[3];
  double weight;


};

int
get_group_lines(char* groupfile)
{
  std::string line;
  std::ifstream text(groupfile);
  int numlines = 0;
  if (text.is_open())  {
    while (text.good()) {
      getline(text,line);	    
      if ( (!line.empty()) && (line.compare(0,1,"#")) )  {
	numlines++;
      }
    }
  }
  return numlines;

}

void
parse_group(int argc, char** argv, int linenumber, std::vector<group_add_parms> *parms_vec)
{

  group_add_parms parms;

  int linecounter = 0;
  
  std::string line;
  std::ifstream text(argv[1]);
  if (text.is_open()) {
    while (text.good()) {
      getline(text,line);
      if ( (!line.empty()) && (line.compare(0,1,"#")) )  {
	
	if (linecounter == linenumber)  {
	  
	  std::string pvol_file;
	  std::string dose_file;
	  std::string dose_wed_file;
	  float offset[3];
	  double weight;
	  
	  std::stringstream linestream(line);
	  
	  linestream >> pvol_file >> dose_file >> dose_wed_file >> offset[0] >> offset[1] >> offset[2] >> weight;
	  
	  if (dose_wed_file.size()>=4)  {
	    if (dose_wed_file.compare(dose_wed_file.size()-4,4,".mha"))  {
	      print_and_exit ("%s is not in <name>.mha format.\n", dose_wed_file.c_str());
	      return;
	    }
	  }
	  else {print_and_exit ("%s is not in <name>.mha format.\n", dose_wed_file.c_str());}
	  

	  parms.file = dose_wed_file;
	  for (int i=0;i!=3;++i)  {parms.offset[i] = offset[i];}
	  parms.weight = weight;
	  parms_vec->push_back(parms);
	  
	}
	linecounter++;
	
      }
    }
  }
}

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

  std::string Output_FN = "Added_output.mha";

  typedef itk::ImageRegionIterator< FloatImageType > FloatIteratorType;
 
  std::vector<group_add_parms> *parms_vec = new std::vector<group_add_parms>();
  int numlines = get_group_lines(argv[1]);

  Plm_image_header image_header;
  plm_long image_dim[3];
  float spacing[3];
  float origin[3];

  for (int i=0;i!=numlines;++i)  {
    parse_group(argc, argv, i, parms_vec);
  }



  Plm_image *input_image = new Plm_image();
 
  std::cout<<"Number of files in list to add is "<<parms_vec->size()<<":"<<std::endl;

  //Create, initialize variables we'll need to construct the added image
  float added_dim[3];
  float added_spacing[3];
  float added_origin[3];
  float aobuffer;

  for (int i=0;i!=3;++i)  {
    added_dim[i]=0;
    added_spacing[i]=0.;
    added_origin[i]=0.;
  }
  
  std::vector< std::vector< std::vector<float> > > added_vect, input_vect;

  for (std::vector<group_add_parms>::iterator it = parms_vec->begin(); it != parms_vec->end();++it)  {

    input_image->load_native(it->file);
    image_header.set_from_plm_image(input_image);

    image_header.get_dim(image_dim);
    image_header.get_spacing(spacing);
    image_header.get_origin(origin);

    //Here we calculate the new dimensions, spacing, and origin of the added image

    if (it == parms_vec->begin())  {
      for (int i=0;i!=3;++i)  {
	added_spacing[i]=spacing[i];
	added_origin[i]=origin[i] + (it->offset[i]);
	added_dim[i]=image_dim[i];
      }
    }

    else {
      for (int i=0;i!=3;++i)  {	

	if ( added_spacing[i] > spacing[i] ) {added_spacing[i]=spacing[i];} //Set added resolution to highest res. member
	if ( added_origin[i] > (origin[i] + it->offset[i]) ) {
	  aobuffer = added_origin[i];
	  added_origin[i]=origin[i] + (it->offset[i]);
	  added_dim[i] += aobuffer - added_origin[i];
	}
	if ( (added_dim[i]+added_origin[i]) < (image_dim[i]+origin[i]+it->offset[i]) ) {added_dim[i]=image_dim[i]+(origin[i]-added_origin[i]+(it->offset[i]));}
      }
    }
  }

  //Synth the new, added image with default value 0
  
  Synthetic_mha_main_parms added_parms;
  for (int i=0;i!=3;++i)  {
    added_parms.sm_parms.dim[i] = (plm_long) added_dim[i];
    added_parms.sm_parms.spacing[i] = added_spacing[i];
    added_parms.sm_parms.origin[i] = added_origin[i];
    added_parms.sm_parms.background = 0.; //set background to 0 for wed doses
  }
  added_parms.output_fn = Output_FN;
  do_synthetic_mha(&added_parms);
  std::cout<<"Empty file \""<<Output_FN<<"\" written to disk successfully"<<std::endl;
  std::cout<<"Attempting to assign added values..."<<std::endl;
  

  plm_long added_length[3];
  for (int i=0;i!=3;++i)  {added_length[i]= (plm_long) added_dim[i];}

  resize_3d_vect(added_vect,added_length);

  //Added the voxels from each image into the added image
  for (std::vector<group_add_parms>::iterator it = parms_vec->begin(); it != parms_vec->end();++it)  {
    input_image->load_native(it->file);
    image_header.set_from_plm_image(input_image);

    image_header.get_dim(image_dim);
    image_header.get_spacing(spacing);
    image_header.get_origin(origin);
   
    plm_long n_voxels = image_dim[0]*image_dim[1]*image_dim[2];

    Volume::Pointer& input_volume = input_image->get_volume_float();

    float *in_img = (float*) input_volume->img;
    resize_3d_vect(input_vect,image_dim);

    plm_long ijk[3];
    for (plm_long zz=0; zz!=n_voxels; ++zz)  {
      COORDS_FROM_INDEX(ijk,zz,image_dim);
      input_vect[ ijk[0] ][ ijk[1] ][ ijk[2] ] = in_img[zz];
    }

    float x_low,x_high,y_low,y_high,z_low,z_high;
    int x_low2,x_high2,y_low2,y_high2,z_low2,z_high2;


    for (int i=0; i!=image_dim[0]; ++i)  {
      for (int j=0; j!=image_dim[1]; ++j)  {
	for (int k=0; k!=image_dim[2]; ++k)  {

	  x_low = origin[0] + it->offset[0] + (i-.5)*spacing[0];
	  x_high = x_low+spacing[0];
	  y_low = origin[1] + it->offset[1] + (j-.5)*spacing[1];
	  y_high = y_low+spacing[1];
	  z_low = origin[2] + it->offset[2] + (k-.5)*spacing[2];
	  z_high = z_low+spacing[2];

	  //The extra "if's" account for some fuzziness with the maximum values not being cut off -
	  //chopping off everything above the added max value.
	  x_low2 = (int) floor((x_low-added_origin[0])/added_spacing[0]+.5);
	  x_high2 = (int) floor((x_high-added_origin[0])/added_spacing[0]+.5);
	  if (x_high2 > added_length[0]-1)  {x_high2 = added_length[0]-1;}
	  y_low2 = (int) floor((y_low-added_origin[1])/added_spacing[1]+.5);
	  y_high2 = (int) floor((y_high-added_origin[1])/added_spacing[1]+.5);
	  if (y_high2 > added_length[1]-1)  {y_high2 = added_length[1]-1;}
	  z_low2 = (int) floor((z_low-added_origin[2])/added_spacing[2]+.5);
	  z_high2 = (int) floor((z_high-added_origin[2])/added_spacing[2]+.5);
	  if (z_high2 > added_length[2]-1)  {z_high2 = added_length[2]-1;}

	  for (int ii=x_low2; ii<=x_high2; ++ii)  {
	    for (int jj=y_low2; jj<=y_high2; ++jj)  {
	      for (int kk=z_low2; kk<=z_high2; ++kk)  {

		float unit = 1.;
		
		//fraction of the input voxel in each added cell:
		if (ii==x_low2) {unit *= (1 - ((x_low-added_origin[0])/added_spacing[0]+.5 - x_low2));}
		if (ii==x_high2) {unit *= ((x_high-added_origin[0])/added_spacing[0]+.5 - x_high2);}
		if (jj==y_low2) {unit *= (1 - ((y_low-added_origin[1])/added_spacing[1]+.5 - y_low2));}
		if (jj==y_high2) {unit *= ((y_high-added_origin[1])/added_spacing[1]+.5 - y_high2);}
		if (kk==z_low2) {unit *= (1 - ((z_low-added_origin[2])/added_spacing[2]+.5 - z_low2));}
		if (kk==z_high2) {unit *= ((z_high-added_origin[2])/added_spacing[2]+.5 - z_high2);}

		if ((x_low2<0)||(y_low2<0)||(z_low2<0)||(x_high2>=added_dim[0])||(y_high2>=added_dim[1])||(z_high2>=added_dim[2]))  {continue;}

		added_vect[ii][jj][kk] += unit*input_vect[i][j][k]*it->weight;
	      }
	    }
	  }

	}
      }
    }

  }

  Plm_image *output_image = new Plm_image();
  output_image->load_native("Added_output.mha");

  FloatImageType::Pointer img_out = output_image->m_itk_float;
  FloatImageType::RegionType rg_out = img_out->GetLargestPossibleRegion ();
  FloatIteratorType image_out_it (img_out, rg_out);

  plm_long ijk[3];
  int zz = 0;
  for (image_out_it.GoToBegin(); !image_out_it.IsAtEnd(); ++image_out_it) {

    COORDS_FROM_INDEX(ijk,zz,added_length);
    image_out_it.Set( added_vect[ ijk[0] ][ ijk[1] ][ ijk[2] ] );

    zz++;
  }

  itk_image_save_float (img_out, "Added_output.mha");

  delete output_image;
  delete parms_vec;
  delete input_image;

  return 0;
}
