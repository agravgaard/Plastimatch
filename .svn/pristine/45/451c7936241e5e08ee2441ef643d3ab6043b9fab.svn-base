/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   rt_mebs (for mono-energetic_beam_set) is a class that creates beams of different energies,
   including SOBP (Spread Out Bragg Peak) or any multi-energy beam configuration. 
   ----------------------------------------------------------------------- */
#ifndef _rt_mebs_h_
#define _rt_mebs_h_

#include "plmdose_config.h"
#include <stdio.h>
#include <vector>
#include <aperture.h>
#include "logfile.h"
#include "particle_type.h"
#include "plmdose_config.h"
#include "plm_config.h"
#include "rpl_volume.h"
#include "rt_lut.h"
#include "smart_pointer.h"

class Rt_depth_dose;
class Rt_mebs_private;

class PLMDOSE_API Rt_mebs {
public:
    SMART_POINTER_SUPPORT (Rt_mebs);
    Rt_mebs_private *d_ptr;
public:
    Rt_mebs ();
    Rt_mebs (Particle_type part);
    Rt_mebs (const Rt_mebs::Pointer& rt_mebs);
    ~Rt_mebs ();

    /* Remove all depth dose */
    void clear_depth_dose ();

    /* Add a pristine peak to a mebs */
    void add_depth_dose (Rt_depth_dose* depth_dose);
    void add_peak (double E0, double spread, double weight);

    /* Return simple depth dose result at depth */
    float lookup_energy (float depth);

    /* use manually weighted peaks */
    bool generate ();

    /* Save the depth dose to a file */
    void dump (const char* dir);

    /* Print the parameters of the mebs */
    void printparameters();

    /* reset the mebs depth dose curve */
    void reset_mebs_depth_dose_curve();

    /* set the prescription parameters: target and prescription depths, energy */
    void set_energies(float new_E_min, float new_E_max);
    void set_energies(float new_E_min, float new_E_max, float new_step);
    void set_target_depths(float new_depth_min, float new_depth_max);
    void set_target_depths(float new_depth_min, float new_depth_max, float new_step);
    void set_prescription_depths(float new_prescription_min, float new_prescription_max);
    void set_margins(float proximal_margin, float distal_margin);
    void update_prescription_depths_from_energies();
    void update_energies_from_prescription();
	
    /* Set/Get private members */
    float* get_d_lut();
    float* get_e_lut();
    float* get_f_lut();
    void set_particle_type(Particle_type particle_type);
    Particle_type get_particle_type();
    void set_alpha(double alpha);
    double get_alpha();
    void set_p(double p);
    double get_p();
    int get_energy_number(); /* set energy_number is not implemented as it must not be changed externally*/
    std::vector<float> get_energy();
    std::vector<float> get_weight();
    void set_energy_resolution(float eres);
    float get_energy_resolution();
    void set_energy_min(float E_min);
    float get_energy_min();
    void set_energy_max(float E_max);
    float get_energy_max();
    int get_num_samples();
    void set_target_min_depth(float dmin);
    float get_target_min_depth();
    void set_target_max_depth(float dmax);
    float get_target_max_depth();
    void set_depth_resolution(float dres);
    float get_depth_resolution();
    void set_depth_end(float dend);
    float get_depth_end();
    float get_prescription_min();
    float get_prescription_max();
    void set_proximal_margin (float proximal_margin);
    float get_proximal_margin();
    void set_distal_margin (float distal_margin);
    float get_distal_margin();
    void set_spread (double spread);
    double get_spread();
    void set_photon_energy(float energy);
    float get_photon_energy();
    std::vector<Rt_depth_dose*> get_depth_dose();
    std::vector<float>& get_num_particles();
    void set_prescription(float prescription_min, float prescription_max);
    void set_have_prescription(bool have_prescription);
    bool get_have_prescription();
    void set_have_copied_peaks(bool have_copied_peaks);
    bool get_have_copied_peaks();
    void set_have_manual_peaks(bool have_manual_peaks);
    bool get_have_manual_peaks();
    void set_have_particle_number_map(bool have_particle_number_map);
    bool get_have_particle_number_map();
    std::vector<double>& get_min_wed_map();
    std::vector<double>& get_max_wed_map();

    void set_particle_number_in (const std::string& str);
    std::string get_particle_number_in ();

    void set_particle_number_out (const std::string& str);
    std::string get_particle_number_out ();

    void add_depth_dose_weight(float weight);

    float check_and_correct_max_energy(float E, float depth);
    float check_and_correct_min_energy(float E, float depth);

    /* Optimize, then generate mebs depth curve from prescription 
       range and modulation */
    void optimize_sobp ();
    /* Weight optimizer */
    void optimizer (std::vector<float>* weight_tmp, std::vector<float>* energy_tmp);

    void get_optimized_peaks(float dmin, float dmax, std::vector<float>* weight_tmp, std::vector<Rt_depth_dose*>* depth_dose);
    void initialize_energy_weight_and_depth_dose_vectors(std::vector<float>* weight_tmp, std::vector<float>* energy_tmp, std::vector<Rt_depth_dose*>* depth_dose_tmp);

    void scale_num_part(double A, int* ap_dim);
    double get_particle_number_xyz(int* idx, double* rest, int idx_beam, const int* ap_dim);

    /* This computes the E_min and E_max map from a target for all pencil beam*/
    void generate_part_num_from_weight(int* ap_dim);
    void extract_particle_number_map_from_txt(Aperture::Pointer& ap);
    void compute_particle_number_matrix_from_target_active(Rpl_volume* rpl_vol, Plm_image::Pointer& target, float smearing);
    void compute_particle_number_matrix_from_target_active_slicerRt (Rpl_volume* rpl_vol, Plm_image::Pointer& target, float smearing);

    void export_spot_map_as_txt(Aperture::Pointer ap);
};

#endif
