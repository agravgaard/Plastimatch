/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aperture.h"
#include "parameter_parser.h"
#include "plm_image.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "rpl_volume.h"
#include "rt_beam.h"
#include "rt_depth_dose.h"
#include "rt_parms.h"
#include "rt_plan.h"
#include "rt_mebs.h"
#include "string_util.h"

class Rt_parms_private {
public:

    /* [PEAK] */
    double E0;
    double spread;
    double weight;
    double max_depth;
    double depth_res;
    std::string bragg_curve;
    
    /* Other parameters not directly defined by config the config file but necessary for the beam creation */
    Rt_plan* rt_plan;
    int beam_number; /* contains the number of the beam in the vector<Rt_beam*> beam_storage */
    Rt_mebs::Pointer mebs;
    bool have_prescription;
    bool ap_have_origin;
    bool have_manual_peaks;

public:
    Rt_parms_private ()
    {
        /* [PEAK] */
        this->E0 = 100.;
        this->spread = 1.;
        this->weight = 1.;
        this->max_depth = 400.0f;
        this->depth_res = 0.01f;
        this->bragg_curve ="";

        /* Other parameters not directly defined by config the config 
           file but necessary for the beam creation */
        this->rt_plan = 0;
        this->beam_number = -1;
        this->mebs = Rt_mebs::New();
        this->have_prescription = false;
        this->ap_have_origin = false;
        this->have_manual_peaks = false;
    }
};

Rt_parms::Rt_parms ()
{
    d_ptr = new Rt_parms_private;
}

Rt_parms::Rt_parms (Rt_plan* rt_plan)
{
    d_ptr = new Rt_parms_private;
    d_ptr->rt_plan = rt_plan;
}

Rt_parms::~Rt_parms ()
{
    delete d_ptr;
}

static void
print_usage (void)
{
    printf (
        "Usage: proton_dose [options] config_file\n"
        "Options:\n"
        " --debug           Create various debug files\n"
    );
    exit (1);
}

class Rt_parms_parser : public Parameter_parser
{
public:
    Rt_parms *rp;
public:
    Rt_parms_parser (Rt_parms *rp)
    {
        this->rp = rp;
    }
public:
    virtual Plm_return_code begin_section (
        const std::string& section)
    {
        if (section == "GLOBAL") {
            return PLM_SUCCESS;
        }
        if (section == "COMMENT") {
            return PLM_SUCCESS;
        }
        if (section == "PLAN") {
            return PLM_SUCCESS;
        }
        if (section == "BEAM") {
            rp->append_beam ();
            return PLM_SUCCESS;
        }
        if (section == "PEAK") {
            return PLM_SUCCESS;
        }

        /* else, unknown section */
        return PLM_ERROR;
    }
    virtual Plm_return_code end_section (
        const std::string& section)
    {
        if (section == "PEAK") {
            rp->append_peak ();
            return PLM_SUCCESS;
        }
        return PLM_SUCCESS;
    }
    virtual Plm_return_code set_key_value (
        const std::string& section,
        const std::string& key,
        const std::string& val)
    {
        return this->rp->set_key_value (section, key, val);
    }
};

void 
Rt_parms::set_rt_plan (Rt_plan *rt_plan)
{
    d_ptr->rt_plan = rt_plan;
}

void 
Rt_parms::append_beam ()
{
    d_ptr->rt_plan->append_beam ();
}

void 
Rt_parms::append_peak ()
{
    Rt_beam *rt_beam = d_ptr->rt_plan->get_last_rt_beam ();
    if (!rt_beam) {
        return;
    }
    rt_beam->get_mebs()->set_have_manual_peaks(true);
    rt_beam->get_mebs()->add_peak (d_ptr->E0, d_ptr->spread, d_ptr->weight);
}

Plm_return_code
Rt_parms::set_key_value (
    const std::string& section,
    const std::string& key, 
    const std::string& val)
{
    if (section == "COMMENT" || section == "GLOBAL") {
        return PLM_SUCCESS;
    }

    /* **** PLAN **** */
    if (section == "PLAN") {
        if (key == "patient") {
            d_ptr->rt_plan->set_patient (val);
        }
        else if (key == "target") {
            d_ptr->rt_plan->set_target (val);
         }
        else if (key == "threading") {
            Threading threading = THREADING_CPU_OPENMP;
            if (val == "single") {
                threading = THREADING_CPU_SINGLE;
            }
            else if (val == "openmp") {
#if (OPENMP_FOUND)
                threading = THREADING_CPU_OPENMP;
#else
                threading = THREADING_CPU_SINGLE;
#endif
            }
            else if (val == "cuda") {
#if (CUDA_FOUND)
                threading = THREADING_CUDA;
#elif (OPENMP_FOUND)
                threading = THREADING_CPU_OPENMP;
#else
                threading = THREADING_CPU_SINGLE;
#endif
            }
            else {
                goto error_exit;
            }
            d_ptr->rt_plan->set_threading (threading);
        }
        else if (key == "dose_out") {
            d_ptr->rt_plan->set_output_dose (val);
        }
        else if (key == "debug") {
            d_ptr->rt_plan->set_debug (string_value_true (val));
        }
        else if (key == "dose_prescription") {
            float norm_dose;
            if (sscanf (val.c_str(), "%f", &norm_dose) != 1) {
                goto error_exit;
            }
			if (norm_dose <= 0) {
				goto error_exit;
			}
            d_ptr->rt_plan->set_normalization_dose (norm_dose);
			d_ptr->rt_plan->set_have_dose_norm(true);
        }
		else if (key == "ref_dose_point") {
            float rdp[3];
            int rc = sscanf (val.c_str(), "%f %f %f", 
                &rdp[0], &rdp[1], &rdp[2]);
            if (rc != 3) {
                goto error_exit;
            }
            d_ptr->rt_plan->set_ref_dose_point (rdp);
			d_ptr->rt_plan->set_have_ref_dose_point(true);
        }
		else if (key == "non_normalized_dose") {
            if (val.length() >= 1) {
                d_ptr->rt_plan->set_non_norm_dose (val[0]);
            } else {
                goto error_exit;
            } 
        }
        else {
            goto error_exit;
        }
        return PLM_SUCCESS;
    }

    /* **** BEAM **** */
    if (section == "BEAM") {
        Rt_beam *rt_beam = d_ptr->rt_plan->get_last_rt_beam ();

        if (key == "flavor") {
            if (val.length() >= 1) {
                rt_beam->set_flavor (val[0]);
            } else {
                goto error_exit;
            } 
        }
		else if (key == "beam_line") {
            rt_beam->set_beam_line_type (val);
        }
        else if (key == "homo_approx") {
            if (val.length() >= 1) {
                rt_beam->set_homo_approx (val[0]);
            } else {
                goto error_exit;
            } 
        }
        else if (key == "ray_step") {
            float step_length;
            if (sscanf (val.c_str(), "%f", &step_length) != 1) {
                goto error_exit;
            }
            rt_beam->set_step_length (step_length);
        }
        else if (key == "aperture_out") {
            rt_beam->set_aperture_out (val);
        }
        else if (key == "proj_dose_out") {
            rt_beam->set_proj_dose_out (val);
        }
        else if (key == "proj_img_out") {
            rt_beam->set_proj_img_out (val);
        }
        else if (key == "rc_out") {
            rt_beam->set_range_compensator_out (val);
        }
		else if (key == "particle_number_out") {
            rt_beam->get_mebs()->set_particle_number_out (val);
        }
        else if (key == "sigma_out") {
            rt_beam->set_sigma_out (val);
        }
        else if (key == "wed_out") {
            rt_beam->set_wed_out (val);
        }
        else if (key == "beam_type") {
            Particle_type part = particle_type_parse (val);
            if (part == PARTICLE_TYPE_UNKNOWN) {
                goto error_exit;
            }
            rt_beam->get_mebs()->set_particle_type (part);
        }
        else if (key == "beam_weight") {
            float beam_weight;
            if (sscanf (val.c_str(), "%f", &beam_weight) != 1) {
                goto error_exit;
            }
            rt_beam->set_beam_weight (beam_weight);
        }
        else if (key == "depth_dose_z_max") {
            if (sscanf (val.c_str(), "%lf", &(d_ptr->max_depth)) != 1) {
                goto error_exit;
            }
			rt_beam->get_mebs()->set_depth_end(d_ptr->max_depth);
        }
        else if (key == "depth_dose_z_res") {
            if (sscanf (val.c_str(), "%lf", &(d_ptr->depth_res)) != 1) {
                goto error_exit;
            }
			rt_beam->get_mebs()->set_depth_resolution(d_ptr->max_depth);
        }
        else if (key == "source") {
            float src[3];
            int rc = sscanf (val.c_str(), "%f %f %f", 
                &src[0], &src[1], &src[2]);
            if (rc != 3) {
                goto error_exit;
            }
            rt_beam->set_source_position (src);
        }
        else if (key == "isocenter") {
            float isocenter[3];
            int rc = sscanf (val.c_str(), "%f %f %f", &isocenter[0],
                &isocenter[1], &isocenter[2]);
            if (rc != 3) {
                goto error_exit;
            }
            rt_beam->set_isocenter_position (isocenter);
        }
        else if (key == "prescription_min_max") {
            float prescription_min;
			float prescription_max;
            int rc = sscanf (val.c_str(), "%f %f", &prescription_min, &prescription_max);
            if (rc != 2) {
                goto error_exit;
            }
            rt_beam->get_mebs()->set_prescription (prescription_min, prescription_max);
        }
        else if (key == "aperture_up") {
            float vup[3];
            int rc = sscanf (val.c_str(), "%f %f %f", 
                &vup[0], &vup[1], &vup[2]);
            if (rc != 3) {
                goto error_exit;
            }
            rt_beam->set_aperture_vup (vup);
        }
        else if (key == "aperture_offset") {
            float ap_distance;
            if (sscanf (val.c_str(), "%f", &ap_distance) != 1) {
                goto error_exit;
            }
            rt_beam->set_aperture_distance (ap_distance);
        }
        else if (key == "aperture_origin") {
            float ap_origin[2];
            int rc = sscanf (val.c_str(), "%f %f", 
                &ap_origin[0], &ap_origin[1]);
            if (rc != 2) {
                goto error_exit;
            }
            rt_beam->set_aperture_origin (ap_origin);
        }
        else if (key == "aperture_resolution") {
            int ap_dim[2];
            int rc = sscanf (val.c_str(), "%i %i", &ap_dim[0], &ap_dim[1]);
            if (rc != 2) {
                goto error_exit;
            }
            rt_beam->set_aperture_resolution (ap_dim);
        }
        else if (key == "aperture_spacing") {
            float ap_spacing[2];
            int rc = sscanf (val.c_str(), "%f %f", 
                &ap_spacing[0], &ap_spacing[1]);
            if (rc != 2) {
                goto error_exit;
            }
            rt_beam->set_aperture_spacing (ap_spacing);
        }
		else if (key == "range_comp_mc_model") {
            if (val.length() >= 1) {
                rt_beam->set_rc_MC_model (val[0]);
            } else {
                goto error_exit;
            } 
        }
        else if (key == "source_size") {
            float source_size;
            if (sscanf (val.c_str(), "%f", &source_size) != 1) {
                goto error_exit;
            }
            rt_beam->set_source_size (source_size);
        }
        else if (key == "aperture_file_in") {
            rt_beam->set_aperture_in (val);
        }
        else if (key == "range_compensator_file_in") {
            rt_beam->set_range_compensator_in (val);
        }
		else if (key == "particle_number_in") {
            rt_beam->get_mebs()->set_particle_number_in (val);
			rt_beam->get_mebs()->set_have_particle_number_map(true);
        }
        else if (key == "aperture_smearing") {
            float smearing;
            if (sscanf (val.c_str(), "%f", &smearing) != 1) {
                goto error_exit;
            }
            rt_beam->set_smearing (smearing);
        }
        else if (key == "proximal_margin") {
            float proximal_margin;
            if (sscanf (val.c_str(), "%f", &proximal_margin) != 1) {
                goto error_exit;
            }
            rt_beam->get_mebs()->set_proximal_margin (proximal_margin);
        }
        else if (key == "distal_margin") {
            float distal_margin;
            if (sscanf (val.c_str(), "%f", &distal_margin) != 1) {
                goto error_exit;
            }
            rt_beam->get_mebs()->set_distal_margin (distal_margin);
        }
		else if (key == "energy_resolution") {
            float eres;
            if (sscanf (val.c_str(), "%f", &eres) != 1) {
                goto error_exit;
            }
            rt_beam->get_mebs()->set_energy_resolution(eres);
        }
        else if (key == "energy_x") {
            float photon_energy;
            if (sscanf (val.c_str(), "%f", &photon_energy) != 1) {
                goto error_exit;
            }
            rt_beam->get_mebs()->set_photon_energy (photon_energy);
        }
        else {
            goto error_exit;
        }
        return PLM_SUCCESS;
    }

    if (section == "PEAK") {
        if (key == "energy") {
            if (sscanf (val.c_str(), "%lf", &(d_ptr->E0)) != 1) {
                goto error_exit;
            }
        }
        else if (key == "spread") {
            if (sscanf (val.c_str(), "%lf", &(d_ptr->spread)) != 1) {
                goto error_exit;
            }
        }
        else if (key == "weight") {
            if (sscanf (val.c_str(), "%lf", &(d_ptr->weight)) != 1) {
                goto error_exit;
            }
        }
        else if (key == "bragg_curve") {
#if defined (commentout_TODO)
            d_ptr->rt_plan->beam->load (val);
#endif
        }
        else {
            goto error_exit;
        }
        return PLM_SUCCESS;
    }

    print_and_exit ("Unknown section value: %s\n", section.c_str());
    return PLM_ERROR;

	error_exit:
    print_and_exit ("Unknown (key,val) combination: (%s,%s)\n", 
        key.c_str(), val.c_str());
    return PLM_ERROR;
}

Plm_return_code
Rt_parms::parse_args (int argc, char** argv)
{
    int i;
    for (i=1; i<argc; i++) {
        if (argv[i][0] != '-') break;

        if (!strcmp (argv[i], "--debug")) {
            d_ptr->rt_plan->set_debug (true);
        }
        else {
            print_usage ();
            break;
        }
    }

    if (!argv[i]) {
        print_usage ();
    }
    Rt_parms_parser rpp (this);
    return rpp.parse_config_file (argv[i]);
}
