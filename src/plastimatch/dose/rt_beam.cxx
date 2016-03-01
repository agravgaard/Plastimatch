/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmdose_config.h"
#include "proj_volume.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string_util.h>
#include <math.h>

#include "bragg_curve.h"
#include "rt_beam.h"
#include "rt_plan.h"

class Rt_beam_private {
public:

    /* dose volume */
    Plm_image::Pointer dose_vol;

    double source[3];
    double isocenter[3];
    char flavor;
    char homo_approx;

    float beamWeight;

    Rt_mebs::Pointer mebs;
    std::string debug_dir;

    float smearing;
    char rc_MC_model;
    float source_size;

    float step_length;

    Aperture::Pointer aperture;
    Plm_image::Pointer target;

    std::string aperture_in;
    std::string range_compensator_in;

    std::string aperture_out;
    std::string proj_dose_out;
    std::string proj_img_out;
    std::string range_compensator_out;
    std::string sigma_out;
    std::string wed_out;
    std::string beam_line_type;

public:
    Rt_beam_private ()
    {
        this->dose_vol = Plm_image::New();

        this->source[0] = -1000.f;
        this->source[1] = 0.f;
        this->source[2] = 0.f;
        this->isocenter[0] = 0.f;
        this->isocenter[1] = 0.f;
        this->isocenter[2] = 0.f;
        this->flavor = 'a';
        this->homo_approx = 'n';

        this->beamWeight = 1.f;
        this->mebs = Rt_mebs::New();
        this->debug_dir = "";
        this->smearing = 0.f;
        this->rc_MC_model = 'n';
        this->source_size = 0.f;
        this->step_length = 1.f;

        aperture = Aperture::New();

        this->aperture_in = "";
        this->range_compensator_in = "";
        this->aperture_out = "";
        this->proj_dose_out = "";
        this->proj_img_out = "";
        this->range_compensator_out = "";
        this->sigma_out = "";
        this->wed_out = "";
        this->beam_line_type = "active";
    }
    Rt_beam_private (const Rt_beam_private* rtbp)
    {
        this->dose_vol = Plm_image::New();

        this->source[0] = rtbp->source[0];
        this->source[1] = rtbp->source[1];
        this->source[2] = rtbp->source[2];
        this->isocenter[0] = rtbp->isocenter[0];
        this->isocenter[1] = rtbp->isocenter[1];
        this->isocenter[2] = rtbp->isocenter[2];
        this->flavor = rtbp->flavor;
        this->homo_approx = rtbp->homo_approx;

        /* Copy the mebs object */
        this->beamWeight = rtbp->beamWeight;
        this->mebs = Rt_mebs::New (rtbp->mebs);
        this->debug_dir = rtbp->debug_dir;
        this->smearing = rtbp->smearing;
        this->source_size = rtbp->source_size;
        this->step_length = rtbp->step_length;

        /* Copy the aperture object */
        aperture = Aperture::New (rtbp->aperture);

        this->aperture_in = rtbp->aperture_in;
        this->range_compensator_in = rtbp->range_compensator_in;
        this->aperture_out = rtbp->aperture_out;
        this->proj_dose_out = rtbp->proj_dose_out;
        this->proj_img_out = rtbp->proj_img_out;
        this->range_compensator_out = rtbp->range_compensator_out;
        this->sigma_out = rtbp->sigma_out;
        this->wed_out = rtbp->wed_out;
        this->beam_line_type = rtbp->beam_line_type;
    }
};

Rt_beam::Rt_beam ()
{
    this->d_ptr = new Rt_beam_private();
    this->rpl_vol = new Rpl_volume();

    /* Creation of the volumes useful for dose calculation */

    if (this->get_flavor() == 'f')
    {    
        this->rpl_ct_vol_HU = new Rpl_volume();
        this->sigma_vol = new Rpl_volume();
    }

    if (this->get_flavor() == 'g')
    {    
        this->rpl_ct_vol_HU = new Rpl_volume();
        this->sigma_vol = new Rpl_volume();
        this->rpl_vol_lg = new Rpl_volume();
        this->rpl_ct_vol_HU_lg = new Rpl_volume();
        this->sigma_vol_lg = new Rpl_volume();
        this->rpl_dose_vol = new Rpl_volume();
    }

    if (this->get_flavor() == 'h')
    {    
        this->rpl_ct_vol_HU = new Rpl_volume();
        this->sigma_vol = new Rpl_volume();
        this->rpl_vol_lg = new Rpl_volume();
        this->rpl_ct_vol_HU_lg = new Rpl_volume();
        this->sigma_vol_lg = new Rpl_volume();
        this->rpl_dose_vol = new Rpl_volume();
    }
}

Rt_beam::Rt_beam (const Rt_beam* rt_beam)
{
    /* Copy all the private settings (?) */
    this->d_ptr = new Rt_beam_private (rt_beam->d_ptr);
    
    /* The below calculation volumes don't need to be copied 
       from input beam */
    this->rpl_vol = 0;
    this->rpl_ct_vol_HU = 0;
    this->sigma_vol = 0;
    this->rpl_vol_lg = 0;
    this->rpl_ct_vol_HU_lg = 0;
    this->sigma_vol_lg = 0;
    this->rpl_dose_vol = 0;
}

Rt_beam::~Rt_beam ()
{
    delete this->d_ptr;
}

bool
Rt_beam::load (const char* fn)
{
    FILE* fp = fopen (fn, "r");
    char linebuf[128];

    if (!fp) {
        return false;
    }

    fgets (linebuf, 128, fp);
    fclose (fp);

    if (!strncmp (linebuf, "00001037", strlen ("00001037"))) {
        return this->load_xio (fn);
    } else {
        return this->load_txt (fn);
    }
}

const double*
Rt_beam::get_source_position () const
{
    return d_ptr->source;
}

double
Rt_beam::get_source_position (int dim) const
{
    return d_ptr->source[dim];
}

void
Rt_beam::set_source_position (const float* position)
{
    for (int d = 0; d < 3; d++) {
        d_ptr->source[d] = position[d];
    }
}

void
Rt_beam::set_source_position (const double* position)
{
    for (int d = 0; d < 3; d++) {
        d_ptr->source[d] = position[d];
    }
}

const double*
Rt_beam::get_isocenter_position () const
{
    return d_ptr->isocenter;
}

double
Rt_beam::get_isocenter_position (int dim) const
{
    return d_ptr->isocenter[dim];
}

void
Rt_beam::set_isocenter_position (const float* position)
{
    for (int d = 0; d < 3; d++) {
        d_ptr->isocenter[d] = position[d];
    }
}

void
Rt_beam::set_isocenter_position (const double* position)
{
    for (int d = 0; d < 3; d++) {
        d_ptr->isocenter[d] = position[d];
    }
}

char
Rt_beam::get_flavor (void) const
{
    return d_ptr->flavor;
}

void
Rt_beam::set_flavor (char flavor)
{
    d_ptr->flavor = flavor;
}

char 
Rt_beam::get_homo_approx () const
{
    return d_ptr->homo_approx;
}
    
void 
Rt_beam::set_homo_approx (char homo_approx)
{
	d_ptr->homo_approx = homo_approx;
}

Rt_mebs::Pointer
Rt_beam::get_mebs()
{
	return d_ptr->mebs;
}

float
Rt_beam::get_beam_weight (void) const
{
    return d_ptr->beamWeight;
}

void
Rt_beam::set_beam_weight (float beamWeight)
{
    d_ptr->beamWeight = beamWeight;
}

void
Rt_beam::set_rc_MC_model (char rc_MC_model)
{
    d_ptr->rc_MC_model = rc_MC_model;
}

char
Rt_beam::get_rc_MC_model (void) const
{
    return d_ptr->rc_MC_model;
}

void
Rt_beam::set_source_size(float source_size)
{
    d_ptr->source_size = source_size;
}

float
Rt_beam::get_source_size() const
{
    return d_ptr->source_size;
}

void
Rt_beam::set_debug (const std::string& dir)
{
    d_ptr->debug_dir = dir;
}

void
Rt_beam::dump (const char* dir)
{
    d_ptr->mebs->dump (dir);
}

void 
Rt_beam::compute_prerequisites_beam_tools(Plm_image::Pointer& target)
{
    if (d_ptr->mebs->get_have_particle_number_map() == true && d_ptr->beam_line_type == "passive")
    {
        printf("***WARNING*** Passively scattered beam line with spot map file detected: %s.\nBeam line set to active scanning.\n", d_ptr->mebs->get_particle_number_in().c_str());
        printf("Any manual peaks set, depth prescription, target or range compensator will not be considered.\n");
        this->compute_beam_data_from_spot_map();
        return;
    }

    /* The priority gets to spot map > manual peaks > dose prescription > target */
    if (d_ptr->mebs->get_have_particle_number_map() == true)
    {
        printf("Spot map file detected: Any manual peaks set, depth prescription, target or range compensator will not be considered.\n");
        this->compute_beam_data_from_spot_map();
        return;
    }
    if (d_ptr->mebs->get_have_manual_peaks() == true)
    {
        printf("Manual peaks detected [PEAKS]: Any prescription or target depth will not be considered.\n");
        this->get_mebs()->set_have_manual_peaks(true);
        this->compute_beam_data_from_manual_peaks(target);
        return;
    }
    if (d_ptr->mebs->get_have_prescription() == true)
    {
        this->get_mebs()->set_have_prescription(true);
        /* Apply margins */
        this->get_mebs()->set_target_depths(d_ptr->mebs->get_prescription_min(), d_ptr->mebs->get_prescription_max());
        printf("Prescription depths detected. Any target depth will not be considered.\n");
        this->compute_beam_data_from_prescription(target);
        return;
    }
    if (target->get_vol())
    {
        printf("Target detected.\n");
        this->get_mebs()->set_have_manual_peaks(false);
        this->get_mebs()->set_have_prescription(false);
        this->compute_beam_data_from_target(target);
        return;
    }
	
    /* If we arrive to this point, it is because no beam was defined
       Creation of a default beam: 100 MeV */
    printf("***WARNING*** No spot map, manual peaks, depth prescription or target detected.\n");
    printf("Beam set to a 100 MeV mono-energetic beam. Proximal and distal margins not considered.\n");
    this->compute_default_beam();
    return;
}

void
Rt_beam::compute_beam_data_from_spot_map()
{
    this->get_mebs()->clear_depth_dose();
    this->get_mebs()->extract_particle_number_map_from_txt(this->get_aperture());

    /* If an aperture is defined in the input file, the aperture is erased. 
       The range compensator is not considered if the beam line is defined as active scanning */
    this->update_aperture_and_range_compensator();
}

void
Rt_beam::compute_beam_data_from_manual_peaks(Plm_image::Pointer& target)
{
    /* The spot map will be identical for passive or scanning beam lines */
    int ap_dim[2] = {this->get_aperture()->get_dim()[0], this->get_aperture()->get_dim()[1]};
    this->get_mebs()->generate_part_num_from_weight(ap_dim);
    if ((target->get_vol() && (d_ptr->aperture_in =="" || d_ptr->range_compensator_in =="")) && (d_ptr->mebs->get_have_manual_peaks() == true || d_ptr->mebs->get_have_prescription() == true)) // we build the associate range compensator and aperture
    {
        if (d_ptr->beam_line_type == "active")
        {
            this->rpl_vol->compute_beam_modifiers_active_scanning(target->get_vol(), d_ptr->smearing, d_ptr->mebs->get_proximal_margin(), d_ptr->mebs->get_distal_margin());
        }
        else
        {
            this->rpl_vol->compute_beam_modifiers_passive_scattering(target->get_vol(), d_ptr->smearing, d_ptr->mebs->get_proximal_margin(), d_ptr->mebs->get_distal_margin());
        }
    }
    /* the aperture and range compensator are erased and the ones defined in the input file are considered */
    this->update_aperture_and_range_compensator();
}

void
Rt_beam::compute_beam_data_from_manual_peaks_passive_slicerRt(Plm_image::Pointer& target)
{
    /* The spot map will be identical for passive or scanning beam lines */
    int ap_dim[2] = {this->get_aperture()->get_dim()[0], this->get_aperture()->get_dim()[1]};
    this->get_mebs()->generate_part_num_from_weight(ap_dim);

    this->rpl_vol->compute_beam_modifiers_passive_scattering_slicerRt(target, d_ptr->smearing, d_ptr->mebs->get_proximal_margin(), d_ptr->mebs->get_distal_margin());
    
    /* the aperture and range compensator are erased and the ones defined in the input file are considered */
    this->update_aperture_and_range_compensator();
}

void
Rt_beam::compute_beam_data_from_manual_peaks()
{
    /* The spot map will be identical for passive or scanning beam lines */
    int ap_dim[2] = {this->get_aperture()->get_dim()[0], this->get_aperture()->get_dim()[1]};
    this->get_mebs()->generate_part_num_from_weight(ap_dim);
    /* the aperture and range compensator are erased and the ones defined in the input file are considered */
    this->update_aperture_and_range_compensator();
}

void
Rt_beam::compute_beam_data_from_prescription(Plm_image::Pointer& target)
{
    /* The spot map will be identical for passive or scanning beam lines */
    /* Identic to compute from manual peaks, with a preliminary optimization */
    d_ptr->mebs->optimize_sobp();
    this->compute_beam_data_from_manual_peaks(target);
}

void
Rt_beam::compute_beam_data_from_target(Plm_image::Pointer& target)
{
    /* Compute beam aperture, range compensator 
       + SOBP for passively scattered beam lines */
	
    if (this->get_beam_line_type() != "passive")
    {
        d_ptr->mebs->compute_particle_number_matrix_from_target_active(this->rpl_vol, this->get_target(), d_ptr->smearing);
    }
    else
    {
        this->compute_beam_modifiers (d_ptr->target->get_vol(), this->get_mebs()->get_min_wed_map(), this->get_mebs()->get_max_wed_map());
        this->compute_beam_data_from_prescription(target);
    }
}

void 
Rt_beam::compute_default_beam()
{
	/* Computes a default 100 MeV peak */
	this->get_mebs()->add_peak(100, 1, 1);
	this->compute_beam_data_from_manual_peaks();
}

void 
Rt_beam::compute_beam_modifiers (Volume *seg_vol)
{
    if (d_ptr->beam_line_type == "active")
    {
        this->rpl_vol->compute_beam_modifiers_active_scanning(seg_vol, d_ptr->smearing, d_ptr->mebs->get_proximal_margin(), d_ptr->mebs->get_distal_margin());
    }
    else
    {
        this->rpl_vol->compute_beam_modifiers_passive_scattering(seg_vol, d_ptr->smearing, d_ptr->mebs->get_proximal_margin(), d_ptr->mebs->get_distal_margin());
    }

    d_ptr->mebs->set_prescription_depths(this->rpl_vol->get_min_wed(), this->rpl_vol->get_max_wed());
    this->rpl_vol->apply_beam_modifiers ();
    return;
}

void 
Rt_beam::compute_beam_modifiers (Volume *seg_vol, std::vector<double>& map_wed_min, std::vector<double>& map_wed_max)
{
    if (d_ptr->beam_line_type == "active")
    {
        this->rpl_vol->compute_beam_modifiers_active_scanning(seg_vol, d_ptr->smearing, d_ptr->mebs->get_proximal_margin(), d_ptr->mebs->get_distal_margin(), map_wed_min, map_wed_max);
    }
    else
    {
        this->rpl_vol->compute_beam_modifiers_passive_scattering(seg_vol, d_ptr->smearing, d_ptr->mebs->get_proximal_margin(), d_ptr->mebs->get_distal_margin(), map_wed_min, map_wed_max);
    }
    d_ptr->mebs->set_prescription_depths(this->rpl_vol->get_min_wed(), this->rpl_vol->get_max_wed());
    this->rpl_vol->apply_beam_modifiers ();
    return;
}

void
Rt_beam::update_aperture_and_range_compensator()
{
    /* The aperture is copied from rpl_vol
       the range compensator and/or the aperture are erased if defined in the input file */
    if (d_ptr->aperture_in != "")
    {
        Plm_image::Pointer ap_img = Plm_image::New (d_ptr->aperture_in, PLM_IMG_TYPE_ITK_UCHAR);
        this->get_aperture()->set_aperture_image(d_ptr->aperture_in.c_str());
        this->get_aperture()->set_aperture_volume(ap_img->get_volume_uchar());
        if (this->rpl_vol->get_minimum_distance_target() == 0) // means that there is no target defined
        {
            printf("Smearing applied to the aperture. The smearing width is defined in the aperture frame.\n");
            d_ptr->aperture->apply_smearing_to_aperture(d_ptr->smearing, d_ptr->aperture->get_distance());
        }
        else
        {
            printf("Smearing applied to the aperture. The smearing width is defined at the target minimal distance.\n");
            d_ptr->aperture->apply_smearing_to_aperture(d_ptr->smearing, this->rpl_vol->get_minimum_distance_target());
        }
    }
    /* Set range compensator */
    if (d_ptr->range_compensator_in != "" && d_ptr->beam_line_type != "active")
    {
        Plm_image::Pointer rgc_img = Plm_image::New (d_ptr->range_compensator_in, PLM_IMG_TYPE_ITK_FLOAT);
        this->get_aperture()->set_range_compensator_image(d_ptr->range_compensator_in.c_str());
        this->get_aperture()->set_range_compensator_volume(rgc_img->get_volume_float());
		
        if (this->rpl_vol->get_minimum_distance_target() == 0) // means that there is no target defined
        {
            printf("Smearing applied to the range compensator. The smearing width is defined in the aperture frame.\n");
            d_ptr->aperture->apply_smearing_to_range_compensator(d_ptr->smearing, d_ptr->aperture->get_distance());
        }
        else
        {
            printf("Smearing applied to the range compensator. The smearing width is defined at the target minimal distance.\n");
            d_ptr->aperture->apply_smearing_to_range_compensator(d_ptr->smearing, this->rpl_vol->get_minimum_distance_target());
        }
    }
}

Plm_image::Pointer&
Rt_beam::get_target ()
{
    return d_ptr->target;
}

const Plm_image::Pointer&
Rt_beam::get_target () const 
{
    return d_ptr->target;
}

void 
Rt_beam::set_target(Plm_image::Pointer& target)
{
    d_ptr->target = target;
}

Plm_image::Pointer&
Rt_beam::get_dose ()
{
    return d_ptr->dose_vol;
}

const Plm_image::Pointer&
Rt_beam::get_dose () const 
{
    return d_ptr->dose_vol;
}

void 
Rt_beam::set_dose(Plm_image::Pointer& dose)
{
    d_ptr->dose_vol = dose;
}

Aperture::Pointer&
Rt_beam::get_aperture () 
{
    return d_ptr->aperture;
}

const Aperture::Pointer&
Rt_beam::get_aperture () const
{
    return d_ptr->aperture;
}

void
Rt_beam::set_aperture_vup (const float vup[])
{
    d_ptr->aperture->set_vup (vup);
}

void
Rt_beam::set_aperture_distance (float ap_distance)
{
    d_ptr->aperture->set_distance (ap_distance);
}

void
Rt_beam::set_aperture_origin (const float ap_origin[])
{
    this->get_aperture()->set_origin (ap_origin);
}

void
Rt_beam::set_aperture_resolution (const int ap_resolution[])
{
    this->get_aperture()->set_dim (ap_resolution);
}

void
Rt_beam::set_aperture_spacing (const float ap_spacing[])
{
    this->get_aperture()->set_spacing (ap_spacing);
}

void 
Rt_beam::set_step_length(float step)
{
	d_ptr->step_length = step;
}

float 
Rt_beam::get_step_length()
{
	return d_ptr->step_length;
}

void
Rt_beam::set_smearing (float smearing)
{
    d_ptr->smearing = smearing;
}

float 
Rt_beam::get_smearing()
{
    return d_ptr->smearing;
}

void 
Rt_beam::set_aperture_in (const std::string& str)
{
    d_ptr->aperture_in = str;
}

std::string 
Rt_beam::get_aperture_in()
{
    return d_ptr->aperture_in;
}

void 
Rt_beam::set_range_compensator_in (const std::string& str)
{
    d_ptr->range_compensator_in = str;
}

std::string 
Rt_beam::get_range_compensator_in()
{
    return d_ptr->range_compensator_in;
}

void 
Rt_beam::set_aperture_out(std::string str)
{
    d_ptr->aperture_out = str;
}

std::string 
Rt_beam::get_aperture_out()
{
    return d_ptr->aperture_out;
}

void 
Rt_beam::set_proj_dose_out(std::string str)
{
    d_ptr->proj_dose_out = str;
}

std::string 
Rt_beam::get_proj_dose_out()
{
    return d_ptr->proj_dose_out;
}

void 
Rt_beam::set_proj_img_out(std::string str)
{
    d_ptr->proj_img_out = str;
}

std::string 
Rt_beam::get_proj_img_out()
{
    return d_ptr->proj_img_out;
}

void 
Rt_beam::set_range_compensator_out(std::string str)
{
    d_ptr->range_compensator_out = str;
}

std::string 
Rt_beam::get_range_compensator_out()
{
    return d_ptr->range_compensator_out;
}

void 
Rt_beam::set_sigma_out(std::string str)
{
    d_ptr->sigma_out = str;
}

std::string 
Rt_beam::get_sigma_out()
{
    return d_ptr->sigma_out;
}

void 
Rt_beam::set_wed_out(std::string str)
{
    d_ptr->wed_out = str;
}

std::string 
Rt_beam::get_wed_out()
{
    return d_ptr->wed_out;
}

void 
Rt_beam::set_beam_line_type(std::string str)
{
	if (str == "active")
	{
		d_ptr->beam_line_type = str;
	}
	else
	{
		d_ptr->beam_line_type = "passive";
	}
}

std::string
Rt_beam::get_beam_line_type()
{
    return d_ptr->beam_line_type;
}

bool
Rt_beam::load_xio (const char* fn)
{
#if defined (commentout)
    int i, j;
    char* ptoken;
    char linebuf[128];
    FILE* fp = fopen (fn, "r");

    // Need to check for a magic number (00001037) here?
    
    /* skip the first 4 lines */
    for (i=0; i<4; i++) {
        fgets (linebuf, 128, fp);
    }

    /* line 5 contains the # of samples */
    fgets (linebuf, 128, fp);
    sscanf (linebuf, "%i", &this->num_samples);

    this->d_lut = (float*)malloc (this->num_samples*sizeof(float));
    this->e_lut = (float*)malloc (this->num_samples*sizeof(float));
    
    memset (this->d_lut, 0, this->num_samples*sizeof(float));
    memset (this->e_lut, 0, this->num_samples*sizeof(float));

    /* load in the depths (10 samples per line) */
    for (i=0, j=0; i<(this->num_samples/10)+1; i++) {
        fgets (linebuf, 128, fp);
        ptoken = strtok (linebuf, ",\n\0");
        while (ptoken) {
            this->d_lut[j++] = (float) strtod (ptoken, NULL);
            ptoken = strtok (NULL, ",\n\0");
        }
    }
    this->dmax = this->d_lut[j-1];

    /* load in the energies (10 samples per line) */
    for (i=0, j=0; i<(this->num_samples/10)+1; i++) {
        fgets (linebuf, 128, fp);
        ptoken = strtok (linebuf, ",\n\0");
        while (ptoken) {
            this->e_lut[j] = (float) strtod (ptoken, NULL);
            ptoken = strtok (NULL, ",\n\0");
            j++;
        }
    }

    fclose (fp);
#endif
    return true;
}

bool
Rt_beam::load_txt (const char* fn)
{
#if defined (commentout)
    char linebuf[128];
    FILE* fp = fopen (fn, "r");

    while (fgets (linebuf, 128, fp)) {
        float range, dose;

        if (2 != sscanf (linebuf, "%f %f", &range, &dose)) {
            break;
        }

        this->num_samples++;
        this->d_lut = (float*) realloc (
                        this->d_lut,
                        this->num_samples * sizeof(float));

        this->e_lut = (float*) realloc (
                        this->e_lut,
                        this->num_samples * sizeof(float));

        this->d_lut[this->num_samples-1] = range;
        this->e_lut[this->num_samples-1] = dose;
        this->dmax = range;         /* Assume entries are sorted */
    }
    fclose (fp);
#endif
    return true;
}

bool
Rt_beam::get_intersection_with_aperture(double* idx_ap, int* idx, double* rest, double* ct_xyz)
{
	double ray[3] = {0,0,0};
	double length_on_normal_axis = 0;
	
	vec3_copy(ray, ct_xyz);
	vec3_sub2(ray, d_ptr->source);

	length_on_normal_axis = -vec3_dot(ray, rpl_ct_vol_HU->get_proj_volume()->get_nrm()); // MD Fix: why is the aperture not updated at this point? and why proj vol is?
	if (length_on_normal_axis < 0)
	{
		return false;
	}

	vec3_scale2(ray, this->get_aperture()->get_distance()/length_on_normal_axis);

	vec3_add2(ray, d_ptr->source);
	vec3_sub2(ray, rpl_ct_vol_HU->get_proj_volume()->get_ul_room());
					
	idx_ap[0] = vec3_dot(ray, rpl_ct_vol_HU->get_proj_volume()->get_incr_c()) / (this->get_aperture()->get_spacing(0) * this->get_aperture()->get_spacing(0));
	idx_ap[1] = vec3_dot(ray, rpl_ct_vol_HU->get_proj_volume()->get_incr_r()) / (this->get_aperture()->get_spacing(1) * this->get_aperture()->get_spacing(1));
	idx[0] = (int) floor(idx_ap[0]);
	idx[1] = (int) floor(idx_ap[1]);
	rest[0] = idx_ap[0] - (double) idx[0];
	rest[1] = idx_ap[1] - (double) idx[1];
	return true;
}

bool 
Rt_beam::is_ray_in_the_aperture(int* idx, unsigned char* ap_img)
{
	if ((float) ap_img[idx[0] + idx[1] * this->get_aperture()->get_dim(0)] == 0) {return false;}
	if (idx[0] + 1 < this->get_aperture()->get_dim(0))
	{
		if ((float) ap_img[idx[0] + 1 + idx[1] * this->get_aperture()->get_dim(0)] == 0) {return false;}
	}
	if (idx[1] + 1 < this->get_aperture()->get_dim(1))
	{
		if ((float) ap_img[idx[0] + (idx[1] + 1) * this->get_aperture()->get_dim(0)] == 0) {return false;}
	}
	if (idx[0] + 1 < this->get_aperture()->get_dim(0) && idx[1] + 1 < this->get_aperture()->get_dim(1))
	{
		if ((float) ap_img[idx[0] + 1 + (idx[1] + 1) * this->get_aperture()->get_dim(0)] == 0) {return false;}
	}
	 return true;
}

float 
Rt_beam::compute_minimal_target_distance(Volume* target_vol, float background)
{
    float* target_img = (float*) target_vol->img;

    float min = FLT_MAX;
    int idx = 0;
    int dim[3] = {target_vol->dim[0], target_vol->dim[1], target_vol->dim[2]};
    float target_image_origin[3] = {target_vol->origin[0], target_vol->origin[1], target_vol->origin[2]};
    float target_image_spacing[3] = {target_vol->spacing[0], target_vol->spacing[1], target_vol->spacing[2]};
    float source[3] = {(float) this->get_source_position(0), (float) this->get_source_position(1), (float) this->get_source_position(2)};

    float voxel_xyz[3] = {0, 0, 0};
    float min_tmp;

    for (int k = 0; k < dim[2]; k++) 
    {
        for (int j = 0; j < dim[1]; j++) 
        {
            for (int i = 0; i < dim[0]; i++) 
            {
                idx = i + (dim[0] * (j + dim[1] * k));
                if (target_img[idx] > background)
                {
                    voxel_xyz[0] = target_image_origin[0] + (float) i * target_image_spacing[0];
                    voxel_xyz[1] = target_image_origin[1] + (float) j * target_image_spacing[1];
                    voxel_xyz[2] = target_image_origin[2] + (float) k * target_image_spacing[2];
                    min_tmp = vec3_dist(voxel_xyz, source);
                    if (min_tmp < min) {min = min_tmp;}
                }
            }
        }
    }
    return min;
}

void Rt_beam::set_energy_resolution (float eres)
{
    d_ptr->mebs->set_energy_resolution (eres);
}

float Rt_beam::get_energy_resolution () const
{
    return d_ptr->mebs->get_energy_resolution ();
}

void Rt_beam::set_proximal_margin (float proximal_margin)
{
    d_ptr->mebs->set_proximal_margin (proximal_margin);
}

float Rt_beam::get_proximal_margin () const
{
    return d_ptr->mebs->get_proximal_margin ();
}

void Rt_beam::set_distal_margin (float distal_margin)
{
    d_ptr->mebs->set_distal_margin (distal_margin);
}

float Rt_beam::get_distal_margin () const
{
    return d_ptr->mebs->get_distal_margin ();
}

void Rt_beam::set_prescription (float prescription_min, float prescription_max)
{
    d_ptr->mebs->set_prescription (prescription_min, prescription_max);
}
