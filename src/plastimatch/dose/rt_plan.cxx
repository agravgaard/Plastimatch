/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmdose_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "aperture.h"
#include "dose_volume_functions.h"
#include "plm_image.h"
#include "plm_exception.h"
#include "plm_timer.h"
#include "print_and_exit.h"
#include "proj_matrix.h"
#include "proj_volume.h"
#include "ray_data.h"
#include "rpl_volume.h"
#include "rt_beam.h"
#include "rt_depth_dose.h"
#include "rt_dose.h"
#include "rt_lut.h"
#include "rt_parms.h"
#include "rt_plan.h"
#include "rt_sigma.h"
#include "rt_mebs.h"
#include "rt_study.h"
#include "volume.h"
#include "volume_macros.h"

static void display_progress (float is, float of);

class Rt_plan_private {

public:
    bool debug;
    float rdp[3];
    bool have_rdp;
    bool have_dose_norm;
    float normalization_dose; // dose prescription
    char non_norm_dose;
    float depth_dose_max;

    /* Filenames for input and output images */
    std::string patient_fn;
    std::string target_fn;
    std::string output_dose_fn;
    std::string output_proj_img_fn;

    /* Patient (ct_image) , target, output dose volume */
    Plm_image::Pointer patient;
    Plm_image::Pointer target;
    Plm_image::Pointer dose;

    Rt_parms::Pointer rt_parms;
    Rt_study* rt_study;

    /* Storage of beams */
    std::vector<Rt_beam*> beam_storage;

public: 
    Rt_plan_private ()
    {
        debug = false;
        this->rdp[0] = 0.f;
        this->rdp[1] = 0.f;
        this->rdp[2] = 0.f;
        this->have_rdp = false;
        this->have_dose_norm = false;
        this->normalization_dose = 100.f;
        this->non_norm_dose = 'n';
        this->depth_dose_max = 1.f;
        
        patient = Plm_image::New();
        target = Plm_image::New();
        dose = Plm_image::New();
        rt_parms = Rt_parms::New ();
    }

    ~Rt_plan_private ()
    {
    }
};

Rt_plan::Rt_plan ()
{
    this->d_ptr = new Rt_plan_private;
}

Rt_plan::~Rt_plan ()
{
    delete d_ptr;
}

Plm_return_code
Rt_plan::parse_args (int argc, char* argv[])
{
    d_ptr->rt_parms->set_rt_plan (this);
    return d_ptr->rt_parms->parse_args (argc, argv);
}

void
Rt_plan::set_patient (const std::string& patient_fn)
{
    d_ptr->patient_fn = patient_fn;
}

void
Rt_plan::set_patient (Plm_image::Pointer& ct_vol)
{
    d_ptr->patient = ct_vol;
}

void
Rt_plan::set_patient (ShortImageType::Pointer& ct_vol)
{
    d_ptr->patient->set_itk (ct_vol);

    /* compute_segdepth_volume assumes float */
    d_ptr->patient->convert (PLM_IMG_TYPE_GPUIT_FLOAT);
}

void
Rt_plan::set_patient (FloatImageType::Pointer& ct_vol)
{
    d_ptr->patient->set_itk (ct_vol);
}

void
Rt_plan::set_patient (Volume* ct_vol)
{
    d_ptr->patient->set_volume (ct_vol);
}

Volume::Pointer
Rt_plan::get_patient_volume ()
{
    return d_ptr->patient->get_volume_float ();
}

Plm_image *
Rt_plan::get_patient ()
{
    return d_ptr->patient.get();
}

void
Rt_plan::set_target (const std::string& target_fn)
{
    d_ptr->target_fn = target_fn;
    d_ptr->target = Plm_image::New (new Plm_image (target_fn));

    /* Need float, because compute_segdepth_volume assumes float */
    d_ptr->target->convert (PLM_IMG_TYPE_GPUIT_FLOAT);

    this->propagate_target_to_beams ();
}

void
Rt_plan::set_target (UCharImageType::Pointer& target_vol)
{
    d_ptr->target->set_itk (target_vol);

    /* compute_segdepth_volume assumes float */
    d_ptr->target->convert (PLM_IMG_TYPE_GPUIT_FLOAT);

    this->propagate_target_to_beams ();
}

void
Rt_plan::set_target (FloatImageType::Pointer& target_vol)
{
    d_ptr->target->set_itk (target_vol);

    this->propagate_target_to_beams ();
}

Plm_image::Pointer&
Rt_plan::get_target ()
{
    return d_ptr->target;
}

void 
Rt_plan::set_rt_study(Rt_study* rt_study) 
{
    d_ptr->rt_study = rt_study;
}

Rt_study*
Rt_plan::get_rt_study()
{
    return d_ptr->rt_study;
}

Rt_beam*
Rt_plan::append_beam ()
{
    Rt_beam* last_beam = get_last_rt_beam ();
    Rt_beam* new_beam;
    if (last_beam) {
        new_beam = new Rt_beam (last_beam);
    } else {
        new_beam = new Rt_beam;
    }
    d_ptr->beam_storage.push_back (new_beam);
    new_beam->set_target (d_ptr->target);
    return new_beam;
}

Rt_beam*
Rt_plan::get_last_rt_beam ()
{
    if (d_ptr->beam_storage.empty()) {
        return 0;
    }
    return d_ptr->beam_storage.back();
}

bool
Rt_plan::get_debug (void) const
{
    return d_ptr->debug;
}

void
Rt_plan::set_debug (bool debug)
{
    d_ptr->debug = debug;
}

void
Rt_plan::set_threading (Threading threading)
{
    /* Not used yet */
}

void
Rt_plan::set_normalization_dose(float normalization_dose)
{
    d_ptr->normalization_dose = normalization_dose;
}

float
Rt_plan::get_normalization_dose()
{
    return d_ptr->normalization_dose;
}

const float*
Rt_plan::get_ref_dose_point () const
{
    return d_ptr->rdp;
}

float
Rt_plan::get_ref_dose_point (int dim) const
{
    return d_ptr->rdp[dim];
}

void
Rt_plan::set_ref_dose_point (const float* rdp)
{
    for (int d = 0; d < 3; d++) {
        d_ptr->rdp[d] = rdp[d];
    }
}

void
Rt_plan::set_ref_dose_point (const double* rdp)
{
    for (int d = 0; d < 3; d++) {
        d_ptr->rdp[d] = rdp[d];
    }
}

void 
Rt_plan::set_have_ref_dose_point(bool have_rdp)
{
	d_ptr->have_rdp = have_rdp;
}

bool
Rt_plan::get_have_ref_dose_point()
{
	return d_ptr->have_rdp;
}

void 
Rt_plan::set_have_dose_norm(bool have_dose_norm)
{
	d_ptr->have_dose_norm = have_dose_norm;
}

bool
Rt_plan::get_have_dose_norm()
{
	return d_ptr->have_dose_norm;
}

char 
Rt_plan::get_non_norm_dose () const
{
    return d_ptr->non_norm_dose;
}
    
void 
Rt_plan::set_non_norm_dose (char non_norm_dose)
{
	d_ptr->non_norm_dose = non_norm_dose;
}

void
Rt_plan::propagate_target_to_beams ()
{
    /* Loop through beams, and reset target on them */
    for (size_t i = 0; i < d_ptr->beam_storage.size(); i++) {
        d_ptr->beam_storage[i]->set_target(d_ptr->target);
    }
}

bool
Rt_plan::prepare_beam_for_calc (Rt_beam *beam)
{
    if (!beam) return false;
    if (!this->get_patient()) return false;

    if (beam->get_aperture()->get_distance() > beam->get_source_distance ())
    {
        throw Plm_exception ("Source distance must be greater than aperture distance");
    }
    
    if (!beam->rpl_vol) {beam->rpl_vol = new Rpl_volume;}
    beam->rpl_vol->set_geometry (
        beam->get_source_position(),
        beam->get_isocenter_position(),
        beam->get_aperture()->vup,
        beam->get_aperture()->get_distance(),
        beam->get_aperture()->get_dim(),
        beam->get_aperture()->get_center(),
        beam->get_aperture()->get_spacing(),
        beam->get_step_length());
    if (!beam->rpl_vol) return false;
    /* building the ct_density_vol */
    beam->rpl_ct_vol_HU = new Rpl_volume;
    beam->rpl_ct_vol_HU->set_geometry (
        beam->get_source_position(),
        beam->get_isocenter_position(),
        beam->get_aperture()->vup,
        beam->get_aperture()->get_distance(),
        beam->get_aperture()->get_dim(),
        beam->get_aperture()->get_center(),
        beam->get_aperture()->get_spacing(),
        beam->get_step_length());
    if (!beam->rpl_ct_vol_HU) return false;
    if (beam->get_flavor() == 'f'|| beam->get_flavor() == 'g' || beam->get_flavor() == 'h')
    {
        /* building the sigma_vol */
        beam->sigma_vol = new Rpl_volume;
        beam->sigma_vol->set_geometry (
            beam->get_source_position(),
            beam->get_isocenter_position(),
            beam->get_aperture()->vup,
            beam->get_aperture()->get_distance(),
            beam->get_aperture()->get_dim(),
            beam->get_aperture()->get_center(),
            beam->get_aperture()->get_spacing(),
            beam->get_step_length());
        
        if (!beam->sigma_vol) return false;
    }

    /* Copy aperture from scene into rpl volume */
    beam->rpl_ct_vol_HU->set_aperture (beam->get_aperture());

    beam->rpl_vol->set_aperture (beam->get_aperture());

    if (beam->get_flavor() == 'f' || beam->get_flavor() == 'g' || beam->get_flavor() == 'h')
    {
        Aperture::Pointer ap_sigma = Aperture::New(beam->get_aperture());
        beam->sigma_vol->set_aperture (ap_sigma);
        beam->sigma_vol->set_aperture (beam->get_aperture());
    }

    /* Scan through aperture to fill in rpl_volume */
    beam->rpl_vol->set_ct_volume (d_ptr->patient);

    if(beam->rpl_vol->get_ct() && beam->rpl_vol->get_ct_limit())
    {
        /* We don't do everything again, we just copy the ct & ct_limits as all the volumes geometrically equal*/
        beam->rpl_ct_vol_HU->set_ct (beam->rpl_vol->get_ct());
        beam->rpl_ct_vol_HU->set_ct_limit(beam->rpl_vol->get_ct_limit());
        
        if (beam->get_flavor() == 'f' || beam->get_flavor() == 'g' || beam->get_flavor() == 'h')
        {
            beam->sigma_vol->set_ct(beam->rpl_vol->get_ct());
            beam->sigma_vol->set_ct_limit(beam->rpl_vol->get_ct_limit());
        }
    }
    else
    {
        printf("ray_data or clipping planes to be copied from rpl volume don't exist\n");
    }

    /*Now we can compute the rpl_volume*/
    beam->rpl_vol->compute_rpl_PrSTRP_no_rgc ();
    /* and the others */
    if(beam->rpl_vol->get_Ray_data() && beam->rpl_vol->get_front_clipping_plane() && beam->rpl_vol->get_back_clipping_plane())
    {
        beam->rpl_ct_vol_HU->set_ray(beam->rpl_vol->get_Ray_data());
        beam->rpl_ct_vol_HU->set_front_clipping_plane(beam->rpl_vol->get_front_clipping_plane());
        beam->rpl_ct_vol_HU->set_back_clipping_plane(beam->rpl_vol->get_back_clipping_plane());
        beam->rpl_ct_vol_HU->compute_rpl_HU();

        if (beam->get_flavor() == 'f' || beam->get_flavor() == 'g' || beam->get_flavor() == 'h')
        {
            /* We don't do everything again, we just copy the ray_data & clipping planes as all the volumes geometrically equal*/
        
            beam->sigma_vol->set_ray(beam->rpl_vol->get_Ray_data());
            beam->sigma_vol->set_front_clipping_plane(beam->rpl_vol->get_front_clipping_plane());
            beam->sigma_vol->set_back_clipping_plane(beam->rpl_vol->get_back_clipping_plane());
        }
    }
    else
    {
        printf("ct or ct_limits to be copied from rpl_vol don't exist\n");
    }
    return true;
}

    void
        Rt_plan::compute_dose (Rt_beam *beam)
    {
        printf ("-- compute_dose entry --\n");
        Volume::Pointer ct_vol = this->get_patient_volume ();
        Volume::Pointer dose_vol = ct_vol->clone_empty ();
        float* dose_img = (float*) dose_vol->img;

        Volume* dose_volume_tmp = new Volume;
        float* dose_img_tmp = (float*) dose_volume_tmp->img;

        UNUSED_VARIABLE (dose_img_tmp);

        float margin = 0;
        int margins[2] = {0,0};
        double range = 0;
        int new_dim[2]={0,0};
        double new_center[2]={0,0};
        double biggest_sigma_ever = 0;
        Plm_timer timer;
        double time_sigma_conv = 0.0;
        double time_dose_calc = 0.0;
        double time_dose_misc = 0.0;
        double time_dose_reformat = 0.0;

        printf ("Computing rpl_ct\n");
        beam->rpl_ct_vol_HU->compute_rpl_HU ();

        if (beam->get_flavor() == 'f' || beam->get_flavor() == 'g' || beam->get_flavor() == 'h')
        {
            float sigmaMax = 0;
            float *sigma_max =&sigmaMax; // used to find the max sigma in the volume and add extra margins during the dose creation volume

            printf ("Computing_void_rpl\n");

            beam->sigma_vol->compute_rpl_PrSTRP_no_rgc(); // we compute the rglength in the sigma_volume, without the range compensator as it will be added by a different process
            Rpl_volume* rpl_vol = beam->rpl_vol;
            Rpl_volume* sigma_vol = beam->sigma_vol;

            float* sigma_img = (float*) sigma_vol->get_vol()->img;
            UNUSED_VARIABLE (sigma_img);


            /* building the sigma_dose_vol */
            if (beam->get_flavor() == 'g') {
                beam->rpl_dose_vol = new Rpl_volume;
            }

            if (beam->get_flavor() == 'h') {
                beam->rpl_vol_lg = new Rpl_volume;
                beam->rpl_ct_vol_HU_lg = new Rpl_volume;
                beam->sigma_vol_lg = new Rpl_volume;
            }

            printf ("More setup\n");

            std::vector<Rt_depth_dose*> depth_dose = beam->get_mebs()->get_depth_dose();

            for (int i = 0; i < depth_dose.size(); i++) {
                const Rt_depth_dose *ppp = beam->get_mebs()->get_depth_dose()[i];
                printf("Building dose matrix for %lg MeV beamlets - \n", ppp->E0);
                timer.start ();

                compute_sigmas (this, beam, ppp->E0, sigma_max, "small", margins);
                time_sigma_conv += timer.report ();

                if (beam->get_flavor() == 'f') // Desplanques' algorithm
                {
                    range = 10 * get_proton_range(ppp->E0); // range in mm
                    dose_volume_create(dose_volume_tmp, sigma_max, beam->rpl_vol, range);
                    compute_dose_ray_desplanques(dose_volume_tmp, ct_vol, beam, dose_vol, i);
                }
                else if (beam->get_flavor() == 'g') // Sharp's algorithm
                {
                    timer.start ();

                    if (*sigma_max > biggest_sigma_ever)
                    {
                        biggest_sigma_ever = *sigma_max;
                        /* Calculating the pixel-margins of the aperture to take into account the scattering*/
                        margin = (float) 3 * (*sigma_max)/(beam->get_aperture()->get_distance() + beam->rpl_vol->get_front_clipping_plane()) * beam->get_aperture()->get_distance()+1;
                        margins[0] = ceil (margin/vec3_len(beam->rpl_vol->get_proj_volume()->get_incr_c()));
                        margins[1] = ceil (margin/vec3_len(beam->rpl_vol->get_proj_volume()->get_incr_r()));
                        new_dim[0] = beam->rpl_vol->get_aperture()->get_dim(0) + 2 * margins[0];
                        new_dim[1] = beam->rpl_vol->get_aperture()->get_dim(1) + 2 * margins[1];
                        new_center[0] = beam->rpl_vol->get_aperture()->get_center(0) + margins[0];
                        new_center[1] = beam->rpl_vol->get_aperture()->get_center(1) + margins[1];

                        beam->rpl_dose_vol->get_aperture()->set_center(new_center);
                        beam->rpl_dose_vol->get_aperture()->set_dim(new_dim);
                        beam->rpl_dose_vol->get_aperture()->set_distance(beam->rpl_vol->get_aperture()->get_distance());
                        beam->rpl_dose_vol->get_aperture()->set_spacing(beam->rpl_vol->get_aperture()->get_spacing());

                        beam->rpl_dose_vol->set_geometry (
                            beam->get_source_position(),
                            beam->get_isocenter_position(),
                            beam->get_aperture()->vup,
                            beam->get_aperture()->get_distance(),
                            beam->rpl_dose_vol->get_aperture()->get_dim(),
                            beam->rpl_dose_vol->get_aperture()->get_center(),
                            beam->get_aperture()->get_spacing(),
                            beam->get_step_length());

                        beam->rpl_dose_vol->set_ct(beam->rpl_vol->get_ct());
                        beam->rpl_dose_vol->set_ct_limit(beam->rpl_vol->get_ct_limit());
                        beam->rpl_dose_vol->compute_ray_data();
                        beam->rpl_dose_vol->set_front_clipping_plane(beam->rpl_vol->get_front_clipping_plane());
                        beam->rpl_dose_vol->set_back_clipping_plane(beam->rpl_vol->get_back_clipping_plane());
                    }

                    /* update the dose_vol with the CT values before to calculate the dose */
                    beam->rpl_dose_vol->compute_rpl_void();
                    time_dose_misc += timer.report ();

                    /* dose calculation in the rpl_dose_volume */
                    timer.start ();
                    compute_dose_ray_sharp (ct_vol, beam, beam->rpl_dose_vol, i, margins);
                    time_dose_calc += timer.report ();
                    timer.start ();
                    dose_volume_reconstruction(beam->rpl_dose_vol, dose_vol);
                    time_dose_reformat += timer.report ();
                }

                if (beam->get_flavor() == 'h') // Shackleford's algorithm
                {
                    /* Calculating the pixel-margins of the aperture to take into account the scattering*/
                    margin = (float) 3 * (*sigma_max)/(beam->get_aperture()->get_distance()+beam->rpl_vol->get_front_clipping_plane()) * beam->get_aperture()->get_distance()+1;
                    margins[0] = ceil (margin/vec3_len(beam->rpl_vol->get_proj_volume()->get_incr_c()));
                    margins[1] = ceil (margin/vec3_len(beam->rpl_vol->get_proj_volume()->get_incr_r()));
                    new_dim[0] = beam->rpl_vol->get_aperture()->get_dim(0) + 2 * margins[0];
                    new_dim[1] = beam->rpl_vol->get_aperture()->get_dim(1) + 2 * margins[1];
                    new_center[0] = beam->rpl_vol->get_aperture()->get_center(0) + margins[0];
                    new_center[1] = beam->rpl_vol->get_aperture()->get_center(1) + margins[1];

                    int radius_sample = 4;
                    int theta_sample = 8;
                    std::vector<double> xy_grid (2*(radius_sample * theta_sample),0); // contains the xy coordinates of the sectors in the plane; the central pixel is not included in this vector. 
                    std::vector<double> area (radius_sample, 0); // contains the areas of the sectors

                    beam->rpl_vol_lg->get_aperture()->set_center(new_center);
                    beam->rpl_vol_lg->get_aperture()->set_dim(new_dim);
                    beam->rpl_vol_lg->get_aperture()->set_distance(beam->rpl_vol->get_aperture()->get_distance());
                    beam->rpl_vol_lg->get_aperture()->set_spacing(beam->rpl_vol->get_aperture()->get_spacing());
                    beam->rpl_vol_lg->set_geometry (beam->get_source_position(), beam->get_isocenter_position(), beam->get_aperture()->vup, beam->get_aperture()->get_distance(), beam->rpl_vol_lg->get_aperture()->get_dim(), beam->rpl_vol_lg->get_aperture()->get_center(), beam->get_aperture()->get_spacing(), beam->get_step_length());
                    beam->rpl_vol_lg->set_ct(beam->rpl_vol->get_ct());
                    beam->rpl_vol_lg->set_ct_limit(beam->rpl_vol->get_ct_limit());
                    beam->rpl_vol_lg->compute_ray_data();
                    beam->rpl_vol_lg->compute_rpl_PrSTRP_no_rgc();

                    beam->rpl_ct_vol_HU_lg->get_aperture()->set_center(new_center);
                    beam->rpl_ct_vol_HU_lg->get_aperture()->set_dim(new_dim);
                    beam->rpl_ct_vol_HU_lg->get_aperture()->set_distance(beam->rpl_vol->get_aperture()->get_distance());
                    beam->rpl_ct_vol_HU_lg->get_aperture()->set_spacing(beam->rpl_vol->get_aperture()->get_spacing());
                    beam->rpl_ct_vol_HU_lg->set_geometry (beam->get_source_position(), beam->get_isocenter_position(), beam->get_aperture()->vup, beam->get_aperture()->get_distance(), beam->rpl_vol_lg->get_aperture()->get_dim(), beam->rpl_vol_lg->get_aperture()->get_center(), beam->get_aperture()->get_spacing(), beam->get_step_length());
                    beam->rpl_ct_vol_HU_lg->set_ct(beam->rpl_vol->get_ct());
                    beam->rpl_ct_vol_HU_lg->set_ct_limit(beam->rpl_vol->get_ct_limit());
                    beam->rpl_ct_vol_HU_lg->compute_ray_data();
                    beam->rpl_ct_vol_HU_lg->set_front_clipping_plane(beam->rpl_vol_lg->get_front_clipping_plane());
                    beam->rpl_ct_vol_HU_lg->set_back_clipping_plane(beam->rpl_vol_lg->get_back_clipping_plane());
                    beam->rpl_ct_vol_HU_lg->compute_rpl_HU();

                    beam->sigma_vol_lg->get_aperture()->set_center(new_center);
                    beam->sigma_vol_lg->get_aperture()->set_dim(new_dim);	
                    beam->sigma_vol_lg->get_aperture()->set_distance(beam->rpl_vol->get_aperture()->get_distance());
                    beam->sigma_vol_lg->get_aperture()->set_spacing(beam->rpl_vol->get_aperture()->get_spacing());
                    beam->sigma_vol_lg->set_geometry (beam->get_source_position(), beam->get_isocenter_position(), beam->get_aperture()->vup, beam->get_aperture()->get_distance(), beam->rpl_vol_lg->get_aperture()->get_dim(), beam->rpl_vol_lg->get_aperture()->get_center(), beam->get_aperture()->get_spacing(), beam->get_step_length());
                    beam->sigma_vol_lg->set_ct(beam->rpl_vol->get_ct());
                    beam->sigma_vol_lg->set_ct_limit(beam->rpl_vol->get_ct_limit());
                    beam->sigma_vol_lg->compute_ray_data();
                    beam->sigma_vol_lg->set_front_clipping_plane(beam->rpl_vol_lg->get_front_clipping_plane());
                    beam->sigma_vol_lg->set_back_clipping_plane(beam->rpl_vol_lg->get_back_clipping_plane());
                    beam->sigma_vol_lg->compute_rpl_PrSTRP_no_rgc();

                    compute_sigmas (this, beam, ppp->E0, sigma_max, "large", margins);				
                    build_hong_grid(&area, &xy_grid, radius_sample, theta_sample);
                    compute_dose_ray_shackleford (
                        dose_vol, this, beam,
                        i, &area, &xy_grid,
                        radius_sample, theta_sample);
                }
                printf("dose computed\n");
            }
        }
        if (beam->get_flavor() == 'a') // pull algorithm
        {    
            /* Dose D(POI) = Dose(z_POI) but z_POI =  rg_comp + depth in CT, if there is a range compensator */
            if (beam->rpl_vol->get_aperture()->have_range_compensator_image())
            {
                add_rcomp_length_to_rpl_volume(beam);
            }

            /* scan through patient CT Volume */
            plm_long ct_ijk[3];
            double ct_xyz[4];
            plm_long idx = 0;
            double idx_ap[2] = {0,0};
            int idx_ap_int[2] = {0,0};
            double rest[2] = {0,0};
            unsigned char* ap_img = (unsigned char*) beam->get_aperture()->get_aperture_volume()->img;
            double particle_number = 0;
            float WER = 0;
            float rgdepth = 0;

            for (ct_ijk[2] = 0; ct_ijk[2] < ct_vol->dim[2]; ct_ijk[2]++) {
                for (ct_ijk[1] = 0; ct_ijk[1] < ct_vol->dim[1]; ct_ijk[1]++) {
                    for (ct_ijk[0] = 0; ct_ijk[0] < ct_vol->dim[0]; ct_ijk[0]++) {
                        double dose = 0.0;
                        bool voxel_debug = false;

                        /* Transform vol index into space coords */
                        ct_xyz[0] = (double) (ct_vol->origin[0] + ct_ijk[0] * ct_vol->spacing[0]);
                        ct_xyz[1] = (double) (ct_vol->origin[1] + ct_ijk[1] * ct_vol->spacing[1]);
                        ct_xyz[2] = (double) (ct_vol->origin[2] + ct_ijk[2] * ct_vol->spacing[2]);
                        ct_xyz[3] = (double) 1.0;

                        if (beam->get_intersection_with_aperture(idx_ap, idx_ap_int, rest, ct_xyz) == false)
                        {
                            continue;
                        }

                        /* Check that the ray cross the aperture */
                        if (idx_ap[0] < 0 || idx_ap[0] > (double) beam->rpl_ct_vol_HU->get_proj_volume()->get_image_dim(0)-1
                            || idx_ap[1] < 0 || idx_ap[1] > (double) beam->rpl_ct_vol_HU->get_proj_volume()->get_image_dim(1)-1)
                        {
                            continue;
                        }

                        /* Check that the ray cross the active part of the aperture */
                        if (beam->get_aperture()->have_aperture_image() && beam->is_ray_in_the_aperture(idx_ap_int, ap_img) == false)
                        {
                            continue;
                        }

                        switch (beam->get_flavor()) {
                        case 'a':
                            dose = 0;
                            rgdepth = beam->rpl_vol->get_rgdepth (ct_xyz);
                            WER =  compute_PrWER_from_HU(beam->rpl_ct_vol_HU->get_rgdepth(ct_xyz));

                            for (int beam_idx = 0; beam_idx < beam->get_mebs()->get_depth_dose().size(); beam_idx++)
                            {
                                particle_number = beam->get_mebs()->get_particle_number_xyz(idx_ap_int, rest, beam_idx, beam->get_aperture()->get_dim());
                                if (particle_number != 0 && rgdepth >=0 && rgdepth < beam->get_mebs()->get_depth_dose()[beam_idx]->dend) 
                                {
                                    dose += particle_number * WER * energy_direct (rgdepth, beam, beam_idx);
                                }
                            }
                            break;
                        }

                        /* Insert the dose into the dose volume */
                        idx = volume_index (dose_vol->dim, ct_ijk);
                        dose_img[idx] = dose;
                    }
                }
            }
            display_progress ((float)idx, (float)ct_vol->npix);
        }

	/* Dose normalization process*/
	if (this->get_non_norm_dose() != 'y')
	{
            if (this->get_have_ref_dose_point()) // case 1: ref dose point defined
            {
                float rdp_ijk[3] = {0,0,0};
                float rdp[3] = {this->get_ref_dose_point(0), this->get_ref_dose_point(1), this->get_ref_dose_point(2)};
                rdp_ijk[0] = (rdp[0] - dose_vol->origin[0]) / dose_vol->spacing[0];
                rdp_ijk[1] = (rdp[1] - dose_vol->origin[1]) / dose_vol->spacing[1];
                rdp_ijk[2] = (rdp[2] - dose_vol->origin[2]) / dose_vol->spacing[2];
			
                if (rdp_ijk[0] >=0 && rdp_ijk[1] >=0 && rdp_ijk[2] >=0 && rdp_ijk[0] < dose_vol->dim[0] && rdp_ijk[1] < dose_vol->dim[1] && rdp_ijk[2] < dose_vol->dim[2])
                {
                    printf("Dose normalized to the dose reference point.\n");
                    dose_normalization_to_dose_and_point(dose_vol, beam->get_beam_weight() * this->get_normalization_dose(), rdp_ijk, rdp, beam); // if no normalization dose, norm_dose = 1 by default
                    if (this->get_have_dose_norm())
                    {
                        printf("%lg x %lg Gy.\n", beam->get_beam_weight(), this->get_normalization_dose());
                    }
                    else
                    {
                        printf("%lg x 100%%.\n", beam->get_beam_weight());
                    }
                    printf("Primary PB num. x, y: %d, %d, primary PB res. x, y: %lg PB/mm, %lg PB/mm\n", beam->get_aperture()->get_dim(0), beam->get_aperture()->get_dim(1), 1.0 / (double) beam->get_aperture()->get_spacing(0), 1.0 / (double) beam->get_aperture()->get_spacing(1));
                }
                else
                {
                    printf("***WARNING***\nThe reference dose point is not in the image volume.\n");
                    dose_normalization_to_dose(dose_vol, beam->get_beam_weight() * this->get_normalization_dose(), beam);
                    if (this->get_have_dose_norm())
                    {
                        printf("%lg x %lg Gy.\n", beam->get_beam_weight(), this->get_normalization_dose());
                    }
                    else
                    {
                        printf("%lg x 100%%.\n", beam->get_beam_weight());
                    }
                    printf("Primary PB num. x, y: %d, %d, primary PB res. x, y: %lg PB/mm, %lg PB/mm\n", beam->get_aperture()->get_dim(0), beam->get_aperture()->get_dim(1), 1.0 / (double) beam->get_aperture()->get_spacing(0), 1.0 / (double) beam->get_aperture()->get_spacing(1));
                }
            }
            else // case 2: no red dose point defined
            {				
                dose_normalization_to_dose(dose_vol, beam->get_beam_weight() * this->get_normalization_dose(), beam); // normalization_dose = 1 if no dose_prescription is set
                if (this->get_have_dose_norm())
                {
                    printf("%lg x %lg Gy.\n", beam->get_beam_weight(), this->get_normalization_dose());
                }
                else
                {
                    printf("%lg x 100%%.\n", beam->get_beam_weight());
                }
                printf("Primary PB num. x, y: %d, %d, primary PB res. x, y: %lg PB/mm, %lg PB/mm\n", beam->get_aperture()->get_dim(0), beam->get_aperture()->get_dim(1), 1.0 / (double) beam->get_aperture()->get_spacing(0), 1.0 / (double) beam->get_aperture()->get_spacing(1));
            }
	}
	else // raw dose, dose not normalized
	{
            for (int i = 0; i < dose_vol->dim[0] * dose_vol->dim[1] * dose_vol->dim[2]; i++)
            {
                dose_img[i] *= beam->get_beam_weight();
            }
	}

        Plm_image::Pointer dose = Plm_image::New();
        dose->set_volume (dose_vol);
        beam->set_dose(dose);

        printf ("Sigma conversion: %f seconds\n", time_sigma_conv);
        printf ("Dose calculation: %f seconds\n", time_dose_calc);
        printf ("Dose reformat: %f seconds\n", time_dose_reformat);
        printf ("Dose overhead: %f seconds\n", time_dose_misc); fflush(stdout);
    }

    Plm_return_code
        Rt_plan::compute_plan ()
    {
        if (!d_ptr->rt_parms) {
            print_and_exit ("Error: cannot compute_plan without an Rt_parms\n");
        }

        if (d_ptr->output_dose_fn == "") {
            print_and_exit ("Error: Output dose filename "
                "not specified in configuration file!\n");
        }
        if (d_ptr->patient_fn == "") {
            print_and_exit ("Error: Patient image "
                "not specified in configuration file!\n");
        }

        /* Load the patient CT image and save into the plan */
        Plm_image::Pointer ct = Plm_image::New (d_ptr->patient_fn,
            PLM_IMG_TYPE_ITK_FLOAT);
        if (!ct) {
            print_and_exit ("Error: Unable to load patient volume.\n");
        }
        this->set_patient (ct);
        this->print_verif ();

        Volume::Pointer ct_vol = this->get_patient_volume ();
        Volume::Pointer dose_vol = ct_vol->clone_empty ();
        plm_long dim[3] = {dose_vol->dim[0], dose_vol->dim[1], dose_vol->dim[2]};
        float* total_dose_img = (float*) dose_vol->img;

        for (size_t i = 0; i < d_ptr->beam_storage.size(); i++)
        {
            printf ("\nStart dose calculation Beam %d\n", (int) i + 1);
            Rt_beam *beam = d_ptr->beam_storage[i];

            /* try to generate plan with the provided parameters */
            if (!this->prepare_beam_for_calc (beam)) {
                print_and_exit ("ERROR: Unable to initilize plan.\n");
            }
            /* Compute beam modifiers, SOBP etc. according to the teatment strategy */
            beam->compute_prerequisites_beam_tools(this->get_target());
            /*
              if (beam->get_beam_line_type() == "passive")
              {
              /* handle auto-generated beam modifiers
              if (d_ptr->target_fn != "") {
              printf ("Target fn = %s\n", d_ptr->target_fn.c_str());
              this->set_target (d_ptr->target_fn);
              beam->compute_beam_modifiers(this->get_target()->get_vol());
              }
	
              /* generate depth dose curve, might be manual peaks or 
              optimized based on prescription, or automatic based on target

              if ((beam->get_mebs()->get_have_copied_peaks() == false && beam->get_mebs()->get_have_prescription() == false && d_ptr->target_fn == "")||(beam->get_mebs()->get_have_manual_peaks() == true)) {
		
              /* Manually specified, so do not optimize
              if (!beam->get_mebs()->generate ()) {
              return PLM_ERROR;
              }
              } 
              else if (d_ptr->target_fn != "" && !beam->get_mebs()->get_have_prescription()) {
              /* Optimize based on target volume
              Rpl_volume *rpl_vol = beam->rpl_vol;
              beam->get_mebs()->set_prescription(rpl_vol->get_min_wed() - beam->get_mebs()->get_proximal_margin(), rpl_vol->get_max_wed() + beam->get_mebs()->get_distal_margin());
              beam->get_mebs()->optimize_sobp ();
              } else {
              /* Optimize based on manually specified range and modulation
              beam->get_mebs()->optimize_sobp ();
              }
			
              /* compute the pencil beam spot matrix for passive beams
              beam->get_mebs()->initialize_and_compute_particle_number_matrix_passive(beam->get_aperture());
              }
              else // active
              {
              // to be computed

              /* Compute the aperture and wed matrices
              if (beam->get_mebs()->get_have_particle_number_map() == false)
              {
              /* we extract the max and min energies to cover the target/prescription
              beam->compute_beam_modifiers(
              beam->get_mebs()->compute_particle_number_matrix_from_target_active(beam->rpl_vol, beam->get_target(), beam->get_aperture());
              }
              else // spot map exists as a txt file
              {
              beam->get_mebs()->initialize_and_read_particle_number_matrix_active(beam->get_aperture());
              }
              } */

            /* Generate dose */
            this->set_debug (true);
            this->compute_dose (beam);

            /* Save beam modifiers */
            if (beam->get_aperture_out() != "") {
                Rpl_volume *rpl_vol = beam->rpl_vol;
                Plm_image::Pointer& ap = rpl_vol->get_aperture()->get_aperture_image();
                ap->save_image (beam->get_aperture_out().c_str());
            }

            if (beam->get_range_compensator_out() != "" && beam->get_beam_line_type() == "passive") {
                Rpl_volume *rpl_vol = beam->rpl_vol;
                Plm_image::Pointer& rc = rpl_vol->get_aperture()->get_range_compensator_image();
                rc->save_image (beam->get_range_compensator_out().c_str());
            }

            /* Save projected density volume */
            if (d_ptr->output_proj_img_fn != "") {
                Rpl_volume* proj_img = beam->rpl_ct_vol_HU;
                if (proj_img) {
                    proj_img->save (beam->get_proj_img_out());
                }
            }

            /* Save projected dose volume */
            if (beam->get_proj_dose_out() != "") {
                Rpl_volume* proj_dose = beam->rpl_dose_vol;
                if (proj_dose) {
                    proj_dose->save (beam->get_proj_dose_out());
                }
            }

            /* Save sigma volume */
            if (beam->get_sigma_out() != "") {
                Rpl_volume* sigma_img = beam->sigma_vol;
                if (sigma_img) {
                    sigma_img->save (beam->get_sigma_out());
                }
            }

            /* Save wed volume */
            if (beam->get_wed_out() != "") {
                Rpl_volume* rpl_vol = beam->rpl_vol;
                if (rpl_vol) {
                    rpl_vol->save (beam->get_wed_out());
                }
            }

            /* Save the spot map */
            if (beam->get_mebs()->get_particle_number_out() != "") {
                beam->get_mebs()->export_spot_map_as_txt(beam->get_aperture());
            }

            float* beam_dose_img = (float*) d_ptr->beam_storage[i]->get_dose()->get_volume()->img;

            /* Dose cumulation to the plan dose volume */
            for (int j = 0; j < dim[0] * dim[1] * dim[2]; j++)
            {
                total_dose_img[j] += beam_dose_img[j];
            }
        }

        /* Save dose output */
        Plm_image::Pointer dose = Plm_image::New();
        dose->set_volume (dose_vol);
        this->set_dose(dose);
        this->get_dose()->save_image (d_ptr->output_dose_fn.c_str());

        printf ("done.  \n\n");
        return PLM_SUCCESS;
    }

    Plm_image::Pointer
        Rt_plan::get_dose ()
    {
        return d_ptr->dose;
    }

    FloatImageType::Pointer
        Rt_plan::get_dose_itk ()
    {
        return d_ptr->dose->itk_float();
    }

    void 
        Rt_plan::set_output_dose (const std::string& output_dose_fn)
    {
        d_ptr->output_dose_fn = output_dose_fn;
    }

    void 
        Rt_plan::set_dose(Plm_image::Pointer& dose)
    {
        d_ptr->dose = dose;
    }

    static inline void
        display_progress (
            float is,
            float of
        ) 
    {
#if defined (PROGRESS)
        printf (" [%3i%%]\b\b\b\b\b\b\b",
            (int)floorf((is/of)*100.0f));
        fflush (stdout);
#endif
    }

    void
        Rt_plan::print_verif ()
    {
        printf("\n [PLAN]");
        printf("\n patient : %s", d_ptr->patient_fn.c_str());
        printf("\n target : %s", d_ptr->target_fn.c_str());
        printf("\n dose_out : %s", d_ptr->output_dose_fn.c_str());
        printf("\n debug : %d", this->get_debug());
        printf("\n dose norm : %lg", this->get_normalization_dose());

        printf("\n \n [SETTINGS]");
        int num_beams = d_ptr->beam_storage.size();
        printf("\n flavor: "); for (int i = 0; i < num_beams; i++) {printf("%c ** ", d_ptr->beam_storage[i]->get_flavor());}
        printf("\n homo_approx: "); for (int i = 0; i < num_beams; i++) {printf("%c ** ", d_ptr->beam_storage[i]->get_homo_approx());}
        printf("\n ray_step: "); for (int i = 0; i < num_beams; i++) {printf("%lg ** ", d_ptr->beam_storage[i]->get_step_length());}
        printf("\n aperture_out: "); for (int i = 0; i < num_beams; i++) {printf("%s ** ", d_ptr->beam_storage[i]->get_aperture_out().c_str());}
        printf("\n proj_dose_out: "); for (int i = 0; i < num_beams; i++) {printf("%s ** ", d_ptr->beam_storage[i]->get_proj_dose_out().c_str());}
        printf("\n proj_img_out: "); for (int i = 0; i < num_beams; i++) {printf("%s ** ", d_ptr->beam_storage[i]->get_proj_img_out().c_str());}
        printf("\n range_comp_out: "); for (int i = 0; i < num_beams; i++) {printf("%s ** ", d_ptr->beam_storage[i]->get_range_compensator_out().c_str());}
        printf("\n sigma_out: "); for (int i = 0; i < num_beams; i++) {printf("%s ** ", d_ptr->beam_storage[i]->get_sigma_out().c_str());}
        printf("\n wed_out: "); for (int i = 0; i < num_beams; i++) {printf("%s ** ", d_ptr->beam_storage[i]->get_wed_out().c_str());}
        printf("\n beam_weight: "); for (int i = 0; i < num_beams; i++) {printf("%g ** ", d_ptr->beam_storage[i]->get_beam_weight());}

        printf("\n \n [GEOMETRY & APERTURE]");
        printf("\n source: "); for (int i = 0; i < num_beams; i++) {printf("%lg %lg %lg ** ", d_ptr->beam_storage[i]->get_source_position()[0], d_ptr->beam_storage[i]->get_source_position()[1], d_ptr->beam_storage[i]->get_source_position()[2]);}
        printf("\n isocenter: "); for (int i = 0; i < num_beams; i++) {printf("%lg %lg %lg ** ", d_ptr->beam_storage[i]->get_isocenter_position()[0], d_ptr->beam_storage[i]->get_isocenter_position()[1], d_ptr->beam_storage[i]->get_isocenter_position()[2]);}
        printf("\n vup: "); for (int i = 0; i < num_beams; i++) {printf("%lg %lg %lg ** ", d_ptr->beam_storage[i]->get_aperture()->vup[0], d_ptr->beam_storage[i]->get_aperture()->vup[1], d_ptr->beam_storage[i]->get_aperture()->vup[2]);}
        printf("\n offset: "); for (int i = 0; i < num_beams; i++) {printf("%lg ** ", d_ptr->beam_storage[i]->get_aperture()->get_distance());}
        printf("\n ap_center in pixels: "); for (int i = 0; i < num_beams; i++) {printf("%lg %lg ** ", d_ptr->beam_storage[i]->get_aperture()->get_center()[0], d_ptr->beam_storage[i]->get_aperture()->get_center()[1]);}
        printf("\n i_res: "); for (int i = 0; i < num_beams; i++) {printf("%d %d ** ", d_ptr->beam_storage[i]->get_aperture()->get_dim()[0], d_ptr->beam_storage[i]->get_aperture()->get_dim()[1]);}
        printf("\n spacing: "); for (int i = 0; i < num_beams; i++) {printf("%lg %lg ** ", d_ptr->beam_storage[i]->get_aperture()->get_spacing()[0], d_ptr->beam_storage[i]->get_aperture()->get_spacing()[1]);}
        printf("\n source_size: "); for (int i = 0; i < num_beams; i++) {printf("%lg ** ", d_ptr->beam_storage[i]->get_source_size());}
        printf("\n ap_file_in: "); for (int i = 0; i < num_beams; i++) {printf("%s ** ", d_ptr->beam_storage[i]->get_aperture_in().c_str());}
        printf("\n rc_file_in: "); for (int i = 0; i < num_beams; i++) {printf("%s ** ", d_ptr->beam_storage[i]->get_range_compensator_in().c_str());}
        printf("\n smearing: "); for (int i = 0; i < num_beams; i++) {printf("%lg ** ", d_ptr->beam_storage[i]->get_smearing());}

        printf("\n \n [PEAK]");
        printf("\n E0: "); for (int i = 0; i < num_beams; i++) { printf("P%d ",i); for (int j = 0; j < d_ptr->beam_storage[i]->get_mebs()->get_depth_dose().size(); j++) {printf("%lg ** ", d_ptr->beam_storage[i]->get_mebs()->get_depth_dose()[j]->E0);}}
        printf("\n spread: "); for (int i = 0; i < num_beams; i++) { printf("P%d ",i); for (int j = 0; j < d_ptr->beam_storage[i]->get_mebs()->get_depth_dose().size(); j++) { printf("%lg ** ", d_ptr->beam_storage[i]->get_mebs()->get_depth_dose()[j]->spread);}}
        printf("\n weight: "); for (int i = 0; i < num_beams; i++) { printf("P%d ",i); for (int j = 0; j < d_ptr->beam_storage[i]->get_mebs()->get_depth_dose().size(); j++) { printf("%lg ** ", d_ptr->beam_storage[i]->get_mebs()->get_weight()[j]);}}
    }
