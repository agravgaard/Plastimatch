/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _wed_parms_h_
#define _wed_parms_h_

#include "plmdose_config.h"
#include <string>

class PLMDOSE_API Wed_Parms {
public:
    Wed_Parms ();
    ~Wed_Parms ();

    bool parse_args (int argc, char** argv);
    //void parse_group (int argc, char** argv, int line);

private:
    void parse_config (const char* config_fn);
    int set_key_val (const char* key, const char* val, int section);
    int get_group_lines (char* groupfile);

public:
    /* [SETTINGS] */
    int debug;
    int group;
    short mode;                      /*Running in wed, dew, or segdepth?*/
    std::string input_ct_fn;         /* input:  patient volume */
    std::string input_dose_fn;       /* input:  dose volume */
    std::string input_proj_ct_fn;    /* input:  ct in proj coordinates */
    std::string input_proj_wed_fn;   /* input:  wed in proj coordinates */
    std::string input_wed_dose_fn;   /* input:  dose in wed coordinates */
    std::string input_target_fn;     /* input:  segment volume */
    std::string input_skin_fn;       /* input:  skin volume */
    std::string output_proj_ct_fn;   /* output: ct in proj coordinates */
    std::string output_proj_wed_fn;  /* output: wed in proj coordinates */
    std::string output_proj_dose_fn; /* output: dose in proj coordinates */
    std::string output_wed_ct_fn;    /* output: ct in wed coordinates */
    std::string output_wed_dose_fn;  /* output: dose in wed coordinates */
    std::string output_ct_fn;        /* output: ct in world coordinates */
    std::string output_dew_ct_fn;    /* output: ct in world coordinates */
    std::string output_dew_dose_fn;  /* output: dose in world coordinates */

//    std::string output_ap_fn;        /* output: aperture volume */
//    std::string output_depth_fn;     /* output: depth volume */

    /* [BEAM] */
    bool have_ray_step;
    float ray_step;                  /* Uniform ray step size (mm) */
    float src[3];
    float isocenter[3];
    float beam_res;

    /* [APERTURE] */
    float vup[3];
    int ires[2];
    bool have_ic;
    bool have_ires;
    float ic[2];
    float ap_spacing[2];
    float ap_offset;

    /* [DEW VOLUME] */
    float dew_dim[3];
    float dew_origin[3];
    float dew_spacing[3];

    /* [PROJ VOLUME] */
    float sinogram;
    int sinogram_res;
};

#endif
