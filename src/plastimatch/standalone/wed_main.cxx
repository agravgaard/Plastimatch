/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "aperture.h"
#include "plm_image.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "proj_volume.h"
#include "ray_trace_probe.h"
#include "rpl_volume.h"
#include "rt_beam.h"
#include "rt_plan.h"
#include "volume.h"
#include "volume_limit.h"
#include "wed_parms.h"

Volume* 
create_wed_volume (Wed_Parms* parms, Rpl_volume* rpl_vol)
{

   /* water equivalent depth volume has the same x,y dimensions as the rpl
     * volume. Note: this means the wed x,y dimensions are equal to the
     * aperture dimensions and the z-dimension is equal to the sampling
     * resolution chosen for the rpl */
    plm_long wed_dims[3];

    Volume *vol = rpl_vol->get_vol ();
    wed_dims[0] = vol->dim[0];
    wed_dims[1] = vol->dim[1];
    wed_dims[2] = vol->dim[2];

    /////////////////////////////
    //Should be insenstive to aperture rotation?
    /*
    Proj_volume *proj_vol = d_ptr->proj_vol;
    double iso_src_vec[3];   //vector from isocenter to source
    proj_vol->get_proj_matrix()->get_nrm(iso_src_vec);
    */

    float xoff = -(vol->dim[0] - parms->ic[0]);
    float yoff = -(vol->dim[1] - parms->ic[1]);

    float wed_off[3] = {xoff, yoff, 0.0f};
    float wed_ps[3] = {1.0f, 1.0f, 1.0f};

    return new Volume (wed_dims, wed_off, wed_ps, NULL, PT_FLOAT, 1);
}

static Volume*
create_dew_volume (Wed_Parms* parms, const Volume::Pointer& patient_vol)
{
    float dew_off[3];
    dew_off[0] = patient_vol->origin[0];
    dew_off[1] = patient_vol->origin[1];
    dew_off[2] = patient_vol->origin[2];

    float dew_ps[3];
    dew_ps[0] = patient_vol->spacing[0];
    dew_ps[1] = patient_vol->spacing[1];
    dew_ps[2] = patient_vol->spacing[2];

    plm_long dew_dims[3];
    dew_dims[0] = patient_vol->dim[0];
    dew_dims[1] = patient_vol->dim[1];
    dew_dims[2] = patient_vol->dim[2];

    //If output volume dimensions were set in .cfg file, use these.
    if (parms->dew_dim[0]!=-999.) {dew_dims[0]=parms->dew_dim[0];}
    if (parms->dew_dim[1]!=-999.) {dew_dims[1]=parms->dew_dim[1];}
    if (parms->dew_dim[2]!=-999.) {dew_dims[2]=parms->dew_dim[2];}

    if (parms->dew_origin[0]!=-999.) {dew_off[0]=parms->dew_origin[0];}
    if (parms->dew_origin[1]!=-999.) {dew_off[1]=parms->dew_origin[1];}
    if (parms->dew_origin[2]!=-999.) {dew_off[2]=parms->dew_origin[2];}

    if (parms->dew_spacing[0]!=-999.) {dew_ps[0]=parms->dew_spacing[0];}
    if (parms->dew_spacing[1]!=-999.) {dew_ps[1]=parms->dew_spacing[1];}
    if (parms->dew_spacing[2]!=-999.) {dew_ps[2]=parms->dew_spacing[2];}

    return new Volume (dew_dims, dew_off, dew_ps, NULL, PT_FLOAT, 1);
}

Volume* create_proj_wed_volume (Rpl_volume* rpl_vol)
{
    float proj_wed_off[3] = {0.0f, 0.0f, 0.0f};
    float proj_wed_ps[3] = {1.0f, 1.0f, 1.0f};
    plm_long proj_wed_dims[3];

    Volume *vol = rpl_vol->get_vol ();
    proj_wed_dims[0] = vol->dim[0];
    proj_wed_dims[1] = vol->dim[1];
    proj_wed_dims[2] = 1;

    return new Volume (proj_wed_dims, proj_wed_off, proj_wed_ps, NULL, PT_FLOAT, 1);
}

Volume* 
create_proj_sinogram_volume (Wed_Parms* parms, Volume *proj_wed_vol)
{
    float proj_wed_off[3];
    proj_wed_off[0] = proj_wed_vol->origin[0];
    proj_wed_off[1] = proj_wed_vol->origin[1];
    proj_wed_off[2] = proj_wed_vol->origin[2];

    float proj_wed_ps[3];
    proj_wed_ps[0] = proj_wed_vol->spacing[0];
    proj_wed_ps[1] = proj_wed_vol->spacing[1];
    proj_wed_ps[2] = proj_wed_vol->spacing[2];

    plm_long proj_wed_dims[3];
    proj_wed_dims[0] = proj_wed_vol->dim[0];
    proj_wed_dims[1] = proj_wed_vol->dim[1];
    proj_wed_dims[2] = parms->sinogram_res;

    return new Volume (proj_wed_dims, proj_wed_off, proj_wed_ps, NULL, PT_FLOAT, 1);
}

static int
skin_ct (Volume* ct_volume, Volume* skin_volume, float background)
{
  float *ct_img = (float*) ct_volume->img;
  float *skin_img = (float*) skin_volume->img;

  const plm_long *ct_dim = ct_volume->dim; 
  const plm_long *skin_dim = skin_volume->dim;

  if ((ct_dim[0]!=skin_dim[0])||(ct_dim[1]!=skin_dim[1])||(ct_dim[2]!=skin_dim[2]))  {
        fprintf (stderr, "\n** ERROR: CT dimensions do not match skin dimensions.\n");
	return -1;
  }

  plm_long n_voxels = ct_dim[0]*ct_dim[1]*ct_dim[2];
  
  for (plm_long i=0; i!=n_voxels; ++i)  {

    if (skin_img[i] == 0)  {ct_img[i] = background;}
    else if (skin_img[i] == 1)  {continue;}
    else {
      fprintf (stderr, "\n** ERROR: Value other than '0' or '1' in skin input.\n");
      return -1;
    }
  }

  return 0;
}

#if defined (commentout)
void
wed_ct_compute_mode_2 (
    const std::string& out_fn,
    Wed_Parms* parms,
    Plm_image::Pointer& ct_vol,  // This is not always ct, 
                                 //  sometimes it is dose or 
                                 //  sometimes it is target mask.
    Rt_plan *scene,
    Rt_beam *beam,
    float background
)
{
    Rpl_volume* rpl_vol = beam->rpl_vol;

    /* Compute the aperture and range compensator */
    rpl_vol->compute_beam_modifiers_passive_scattering (ct_vol->get_volume_float().get());

    /* Save files as output */
    Plm_image::Pointer& ap 
        = rpl_vol->get_aperture()->get_aperture_image();
    Plm_image::Pointer& rc 
        = rpl_vol->get_aperture()->get_range_compensator_image();

#if defined (commentout)
    ap->save_image (parms->output_ap_fn.c_str());
    rc->save_image (out_fn);
#endif
}

void
wed_ct_compute_mode_3 (
    const std::string& out_fn,
    Wed_Parms* parms,
    Plm_image::Pointer& ct_vol,  // This is not always ct, 
                                 //  sometimes it is dose or 
                                 //  sometimes it is target mask.
    Rt_plan *scene,
    Rt_beam *beam,
    float background
)
{
    Rpl_volume* rpl_vol = beam->rpl_vol;

    Volume* proj_wed_vol;
    Volume* sinogram_vol;
	
    proj_wed_vol = create_proj_wed_volume(rpl_vol);

    if (parms->sinogram!=0)  {
        sinogram_vol = create_proj_sinogram_volume(parms, proj_wed_vol);

        float *sin_img = (float*) sinogram_vol->img;
        float *proj_img = (float*) proj_wed_vol->img;
        plm_long n_voxels_sin = sinogram_vol->dim[0]*sinogram_vol->dim[1]*sinogram_vol->dim[2];
        plm_long n_voxels_proj = proj_wed_vol->dim[0]*proj_wed_vol->dim[1]*proj_wed_vol->dim[2];
        float *sin_array = new float[n_voxels_sin];
        float *proj_array = new float[n_voxels_proj];

        //Loop over angles determined by the resolution, and calcaulate a projective
        //volume for each.  Then fill the sinogram volume with each slice.
        int angles = parms->sinogram_res;

        float src[3] = {parms->src[0], parms->src[1], parms->src[2]};
        float iso[3] = {parms->isocenter[0], parms->isocenter[1], parms->isocenter[2]};

        float src2[3] = {0, 0, parms->src[2]};
        float radius[2] = {src[0]-iso[0], src[1]-iso[1]};
        float radius_len = sqrt( (src[0]-iso[0])*(src[0]-iso[0]) + (src[1]-iso[1])*(src[1]-iso[1]));

        float init_angle = atan2(radius[1],radius[0]);
        float angle = 0;

        for (int i=0; i!=angles; ++i)  {
            angle = init_angle + ( i / (float) parms->sinogram_res)*2.*M_PI;
            src2[0] = cos(angle)*radius_len + iso[0];
            src2[1] = sin(angle)*radius_len + iso[1];

            beam->set_source_position (src2);
            scene->prepare_beam_for_calc (beam);
            rpl_vol = beam->rpl_vol;
            rpl_vol->compute_proj_wed_volume (proj_wed_vol, background);

            //Fill proj array with voxel values.
            for (plm_long zz=0; zz!=n_voxels_proj; ++zz)  {
                proj_array[zz] = proj_img[zz];
            }
	
            for (int j=0; j!=proj_wed_vol->dim[0]; ++j)  {
                for (int k=0; k!=proj_wed_vol->dim[1]; ++k)  {
                    sin_array[sinogram_vol->index(j,k,i)]
                        = proj_array[proj_wed_vol->index(j,k,0)];
                }
            }
        }
        //Fill sinogram image with voxel values from assembled array.
        for (plm_long zz=0; zz!=n_voxels_sin; ++zz)  {
            sin_img[zz] = sin_array[zz];
        }

        Plm_image(sinogram_vol).save_image(out_fn);

        delete[] sin_array;
        delete[] proj_array;
    }

    else {
        rpl_vol->compute_proj_wed_volume (proj_wed_vol, background);
        Plm_image(proj_wed_vol).save_image(out_fn);
    }       
}
#endif

void
do_wed (Wed_Parms *parms)
{
    float background[4];

    //Background value for wed ct output
    background[0] = -1000.;
    //Background value for wed dose output
    background[1] = 0.;
    //Background value for radiation length output
    background[2] = 0.;
    //Background value for projection of wed
    background[3] = 0.;

    /* load the input ct */
    Plm_image::Pointer ct_vol = Plm_image::New (
        parms->input_ct_fn, PLM_IMG_TYPE_ITK_FLOAT);
    if (!ct_vol) {
        print_and_exit ("** ERROR: Unable to load patient volume.\n");
    }

    /* Load the input dose */
    Plm_image::Pointer dose_vol;
    if (parms->input_dose_fn != "") {
        printf("Loading input dose: %s\n",parms->input_dose_fn.c_str());
        dose_vol = plm_image_load (parms->input_dose_fn.c_str(), 
            PLM_IMG_TYPE_ITK_FLOAT);
    }

    /* Load the input proj_wed */
    Rpl_volume::Pointer proj_wed = Rpl_volume::New ();
    if (parms->input_proj_wed_fn != "") {
        proj_wed->load (parms->input_proj_wed_fn);
    }

    if (parms->input_skin_fn != "") {
        printf ("Skin file defined.  Modifying input ct...\n");
 
        Volume* ct_volume = ct_vol->get_volume_float().get();
        Plm_image::Pointer skin_vol = plm_image_load (parms->input_skin_fn, 
            PLM_IMG_TYPE_ITK_FLOAT);
        if (!skin_vol) {
            print_and_exit ("\n** ERROR: Unable to load skin input.\n");
        }
        Volume* skin_volume = skin_vol->get_volume_float().get();
    
        if (skin_ct(ct_volume, skin_volume, background[0])) {
            print_and_exit ("\n** ERROR: Unable to apply skin input to ct input.\n");
        }
    }
  
    Aperture::Pointer aperture = Aperture::New();
    aperture->set_distance (parms->ap_offset);
    aperture->set_spacing (parms->ap_spacing);
    if (parms->have_ires) {
        aperture->set_dim (parms->ires);
    }
    if (parms->have_ic) {
        aperture->set_center (parms->ic);
    }

    double src[3], isocenter[3];
    for (int i = 0; i < 3; i++) {
        src[i] = parms->src[i];
        isocenter[i] = parms->isocenter[i];
    }
    Rpl_volume rpl;
    rpl.set_ct_volume (ct_vol);
    rpl.set_aperture (aperture);
    rpl.set_geometry (
        src,
        isocenter,
        aperture->vup,
        aperture->get_distance(),
        aperture->get_dim(),
        aperture->get_center(),
        aperture->get_spacing(),
        parms->ray_step);

    if (proj_wed) {
        proj_wed->set_ct_volume (ct_vol);
        proj_wed->set_aperture (aperture);
        proj_wed->set_geometry (
            src,
            isocenter,
            aperture->vup,
            aperture->get_distance(),
            aperture->get_dim(),
            aperture->get_center(),
            aperture->get_spacing(),
            parms->ray_step);
    }

    /* Compute the rpl volume */
    rpl.compute_rpl_PrSTRP_no_rgc ();

#if defined (commentout)
    //Note: Set dimensions first, THEN center, as set_dim() also changes center
    int ap_res[2];
    float ap_center[2];

    //Aperture dimensions
    if (parms->have_ires) {
        beam.get_aperture()->set_dim (parms->ires);
	//If dew, pad each by one for interpolations
	if (parms->mode==1)  {
            ap_res[0] = (int) (parms->ires[0]+2);
            ap_res[1] = (int) (parms->ires[1]+2);
            beam.get_aperture()->set_dim (ap_res);
            parms->ires[0]=ap_res[0];
            parms->ires[1]=ap_res[1];
	}
    }
    //If dew option, and not specified in .cfg files, then we guess
    //at some scene dimensions set by input wed image.

    else if (parms->mode==1) {
        Volume *wed_vol = dose_vol->get_volume_float().get();
        //Grab aperture dimensions from input wed.
        //We also pad each dimension by 1, for the later trilinear 
        //interpolations.
        ap_res[0] = (int) (wed_vol->dim[0]+2);
        ap_res[1] = (int) (wed_vol->dim[1]+2);
  
        beam.get_aperture()->set_dim (ap_res);
        parms->ires[0]=ap_res[0];
        parms->ires[1]=ap_res[1];
    }

    //Aperture Center
    //Note - Center MUST be reset here if set in the config file, as set_dim()
    //will reset the center.
    if (parms->have_ic) {

        if (parms->mode==1) {

            //If center is not defined in config file (in general,
            //it shouldn't be), then default values should be reset
            if ((parms->ic[0]==-99.5)&&(parms->ic[1]==-99.5))  {
                Volume *wed_vol = dose_vol->get_volume_float().get();
                parms->ic[0] = wed_vol->origin[0] + wed_vol->dim[0];
                parms->ic[1] = wed_vol->origin[1] + wed_vol->dim[1];
            }

            //else set it at the center
            else {
                ap_center[0] = parms->ic[0]+1.*parms->ap_spacing[0];
                ap_center[1] = parms->ic[1]+1.*parms->ap_spacing[1];
                beam.get_aperture()->set_center (ap_center);
            }
        }

        else {
            ap_center[0] = parms->ic[0];
            ap_center[1] = parms->ic[1];
            beam.get_aperture()->set_center (ap_center);
        }

    }
    //And again, guess if not specified.
    //Note - add the dew option below if not specified.
    else if (parms->mode==1)  {
        //Set center as half the resolutions.
        ap_center[0] = (float) ap_res[0]/2.;
        ap_center[1] = (float) ap_res[1]/2.;
        beam.get_aperture()->set_center (ap_center);
        parms->ic[0]=ap_center[0];
        parms->ic[1]=ap_center[1];
    } 
    beam.set_step_length(parms->ray_step);

    /* Try to setup the scene with the provided parameters.
       This function computes the rpl volume. */
    if (!scene.prepare_beam_for_calc (&beam)) {
        print_and_exit ("ERROR: Unable to initilize scene.\n");
    }

    /* Save rpl volume if requested */
    Rpl_volume* rpl_vol = beam.rpl_vol;
    if (parms->output_proj_wed_fn != "") {
        rpl_vol->save (parms->output_proj_wed_fn);
    }
#endif

    if (parms->output_proj_wed_fn != "") {
        rpl.save (parms->output_proj_wed_fn);
    }

    if (parms->output_dew_ct_fn != "") {
        Volume* dew_vol = create_dew_volume (parms, ct_vol->get_volume_float());
        rpl.compute_dew_volume (ct_vol->get_volume_float().get(), 
            dew_vol, -1000);
        Plm_image(dew_vol).save_image(parms->output_dew_ct_fn);
    }

    if (parms->output_wed_ct_fn != "") {
        printf ("Computing patient wed volume...\n");
	Volume *wed_vol = create_wed_volume (parms, &rpl);
        rpl.compute_wed_volume (wed_vol, ct_vol->get_volume_float().get(), 
            background[0]);
        Plm_image(wed_vol).save_image(parms->output_wed_ct_fn);
        printf ("done.\n");
    }

    /* Compute the proj_ct volume */
    if (parms->output_proj_ct_fn != "") {
        rpl.compute_rpl_HU ();
        rpl.save (parms->output_proj_ct_fn);
    }
}

int
main (int argc, char* argv[])
{
    Wed_Parms *parms = new Wed_Parms();
  
    //sets parms if input with .cfg file, skips with group option
    if (!parms->parse_args (argc, argv)) {
        exit (0); 
    }

#if defined (commentout)  
    if (parms->group)  {
        int wed_iter = 0;
    
        while(wed_iter!=parms->group)  {
            if (parms->group) {
                parms->parse_group(argc, argv, wed_iter);
                do_wed (parms);
                wed_iter++;
            }
      
        }
    }
    else {
        //Compute wed without loop
        do_wed (parms);
    }
#endif

    do_wed (parms);

    return 0;
}
