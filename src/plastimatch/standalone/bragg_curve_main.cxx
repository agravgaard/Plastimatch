/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if (OPENMP_FOUND)
#include <omp.h>
#endif

#include "bragg_curve.h"
#include "bragg_curve_opts.h"

int
main (int argc, char* argv[])
{
    Bragg_curve_options options;
    FILE *fp = 0;
    double z;

    parse_args (&options, argc, argv);
    //printf ("sigma = %f\n", sigma);

    /* Set z max */
    if (!options.have_z_max) {
        double p = 1.77;
        double alpha = 0.0022;
        double R_0 = alpha * pow (options.E_0, p);
        options.z_max = 10 * 1.1 * R_0;
    }

    /* Set sigma E0 */
    if (!options.have_e_sigma) {
        options.e_sigma = 0.01 * options.E_0;
    }

    if (options.output_file) {
        fp = fopen (options.output_file, "w");
    } else {
        fp = stdout;
    }
    for (z = 0.0; z < options.z_max; z += options.z_spacing) {
        fprintf (fp, "%f %f\n", z, 
            bragg_curve (options.E_0, options.e_sigma, z));
    }
    if (options.output_file) {
        fclose (fp);
    }
    printf ("Done.\n");
    return 0;
}
