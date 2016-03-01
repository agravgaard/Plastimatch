/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rt_beam_h_
#define _rt_beam_h_

#include "file_util.h"
#include "plmdose_config.h"
#include <string>
#include <vector>

#include "aperture.h"
#include "particle_type.h"
#include "rpl_volume.h"
#include "rt_mebs.h"
#include "smart_pointer.h"

class Rt_beam_private;
class Rt_mebs;

/*! \brief 
 * The Rt_beam class encapsulates a single SOBP Rt beam, including 
 * its associated aperture and range compensator.
 */
class PLMDOSE_API Rt_beam {
public:
    SMART_POINTER_SUPPORT (Rt_beam);
    Rt_beam_private *d_ptr;
public:
    Rt_beam ();
    Rt_beam (const Rt_beam* rt_beam);
    ~Rt_beam ();

public:
    /*! \name Inputs */
    /*! \brief Load PDD from XiO or txt file */
    bool load (const char* fn);

    /*! \brief Get the position of the beam source in world coordinates. */
    const double* get_source_position () const;
    /*! \brief Get the x, y, or z coordinate of the beam source 
      in world coordinates. */
    double get_source_position (int dim) const;
    /*! \brief Set the position of the beam source in world coordinates. */
    void set_source_position (const float position[3]);
    /*! \brief Set the position of the beam source in world coordinates. */
    void set_source_position (const double position[3]);

    /*! \brief Get the position of the beam isocenter in world coordinates. */
    const double* get_isocenter_position () const;
    /*! \brief Get the x, y, or z coordinate of the beam source 
      in world coordinates. */
    double get_isocenter_position (int dim) const;
    /*! \brief Set the position of the beam isocenter in world coordinates. */
    void set_isocenter_position (const float position[3]);
    /*! \brief Set the position of the beam isocenter in world coordinates. */
    void set_isocenter_position (const double position[3]);

    /*! \brief Get "flavor" parameter of dose calculation algorithm */
    char get_flavor () const;
    /*! \brief Set "flavor" parameter of dose calculation algorithm */
    void set_flavor (char flavor);

    /*! \brief Get "homo_approx" parameter of dose calculation algorithm */
    char get_homo_approx () const;
    /*! \brief Set "homo_approx" parameter of dose calculation algorithm */
    void set_homo_approx (char homo_approx);

    /*! \brief Get mebs */
    Rt_mebs::Pointer get_mebs();

    /*! \brief Get "beam_weight" parameter of dose calculation algorithm */
    float get_beam_weight () const;
    /*! \brief Set "beam_weight" parameter of dose calculation algorithm */
    void set_beam_weight (float beam_weight);

    /*! \brief Get "rc_MC_model" for the model of the range compensator, y = Monte Carlo, n = Highland */
    char get_rc_MC_model () const;
    /*! \brief Set "rc_MC_model" for the model of the range compensator, y = Monte Carlo, n = Highland */
    void set_rc_MC_model (char rc_MC_model);

    /* Set source size in mm */
    void set_source_size(float source_size);

    /* Get source size in mm */
    float get_source_size() const;

    /*! \brief Request debugging information to be written to directory */
    void set_debug (const std::string& dir);

    /*! \name Outputs */
    void dump (const char* dir);     /* Print debugging information */

    /* Compute beam modifiers, SOBP etc. according to the teatment strategy */
    void compute_prerequisites_beam_tools(Plm_image::Pointer& target);

    /* Different strategies preparation */
    void compute_beam_data_from_spot_map();
    void compute_beam_data_from_manual_peaks();
    void compute_beam_data_from_manual_peaks(Plm_image::Pointer& target);
    void compute_beam_data_from_manual_peaks_passive_slicerRt(Plm_image::Pointer& target);
    void compute_beam_data_from_prescription(Plm_image::Pointer& target);
    void compute_beam_data_from_target(Plm_image::Pointer& target);
    void compute_default_beam();

    /* This computes the aperture and range compensator */
    void compute_beam_modifiers (Volume *seg_vol);
    void compute_beam_modifiers (Volume *seg_vol, std::vector<double>& map_wed_min, std::vector<double>& map_wed_max); // returns also the wed max and min maps

    /* copy the aperture and range compensator from the rpl_vol if not defined in the input file */
    void update_aperture_and_range_compensator();

    /* Set/ Get target */
    Plm_image::Pointer& get_target ();
    const Plm_image::Pointer& get_target () const;
    void set_target(Plm_image::Pointer& target);

    /* Set/ Get dose_volume*/
    Plm_image::Pointer& get_dose ();
    const Plm_image::Pointer& get_dose () const;
    void set_dose(Plm_image::Pointer& dose);

    /* Get aperture */
    Aperture::Pointer& get_aperture ();
    const Aperture::Pointer& get_aperture () const;
    void set_aperture_vup (const float[]);
    void set_aperture_distance (float);
    void set_aperture_origin (const float[]);
    void set_aperture_resolution (const int[]);
    void set_aperture_spacing (const float[]);

    void set_step_length(float step);
    float get_step_length();

    /* Set smearing */
    void set_smearing (float smearing);
    float get_smearing();

    /* Set/Get intput file names */
    void set_aperture_in (const std::string& str);
    std::string get_aperture_in();

    void set_range_compensator_in (const std::string& str);
    std::string get_range_compensator_in();

    /* Set/Get output file names */
    void set_aperture_out(std::string str);
    std::string get_aperture_out();

    void set_proj_dose_out(std::string str);
    std::string get_proj_dose_out();

    void set_proj_img_out(std::string str);
    std::string get_proj_img_out();

    void set_range_compensator_out(std::string str);
    std::string get_range_compensator_out();

    void set_sigma_out(std::string str);
    std::string get_sigma_out();

    void set_wed_out(std::string str);
    std::string get_wed_out();

    void set_beam_line_type(std::string str);
    std::string get_beam_line_type();

    bool get_intersection_with_aperture(double* idx_ap, int* idx, double* rest, double* ct_xyz);
    bool is_ray_in_the_aperture(int* idx, unsigned char* ap_img);

    /* computes the minimal geometric distance of the target for this beam
       -- used for smearing */
    float compute_minimal_target_distance(Volume* target_vol, float background);

    /* functions that pass through to mebs object */
    void set_energy_resolution (float eres);
    float get_energy_resolution () const;
    void set_proximal_margin (float proximal_margin);
    float get_proximal_margin() const;
    void set_distal_margin (float distal_margin);
    float get_distal_margin() const;
    void set_prescription (float prescription_min, float prescription_max);
    
public: 

    /* Volumes useful for dose calculation */
    /* raw volume */
    Rpl_volume* rpl_vol; // contains the radiologic path length along a ray
    Rpl_volume* rpl_ct_vol_HU; // contains the HU units along the ray
    Rpl_volume* sigma_vol;  // contains the sigma (lateral spread of the pencil beam - used to calculate the off-axis term) along the ray

    /* larger volumes for Hong and divergent geometry algorithms */
    Rpl_volume* rpl_vol_lg;
    Rpl_volume* rpl_ct_vol_HU_lg;
    Rpl_volume* sigma_vol_lg;
    Rpl_volume* rpl_dose_vol; // contains the dose vol for the divergent geometry algorithm
    
private:
    bool load_xio (const char* fn);
    bool load_txt (const char* fn);
};

#endif
