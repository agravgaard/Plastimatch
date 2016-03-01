/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bragg_curve_opts_h_
#define _bragg_curve_opts_h_

typedef struct bragg_curve_options Bragg_curve_options;
struct bragg_curve_options {
    float z_max;                /* in mm */
    int have_z_max;
    float z_spacing;            /* in mm */
    double E_0;                  /* in MeV */
    float e_sigma;              /* in MeV */
    float have_e_sigma;
    char* output_file;
};

void parse_args (Bragg_curve_options* options, int argc, char* argv[]);

#endif
