/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -------------------------------------------------------------------------
    REFS:
    http://en.wikipedia.org/wiki/B-spline
    http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/surface/bspline-construct.html
    http://graphics.idav.ucdavis.edu/education/CAGDNotes/Quadratic-B-Spline-Surface-Refinement/Quadratic-B-Spline-Surface-Refinement.html
    ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bspline_cuda.h"
#include "bspline_interpolate.h"
#include "bspline_optimize.h"
#include "bspline_opts.h"
#include "bspline_xform.h"
#if (CUDA_FOUND)
#include "cuda_util.h"
#include "delayload.h"
#endif
#include "mha_io.h"
#include "vf.h"
#include "volume.h"
#include "volume_grad.h"

int
main (int argc, char* argv[])
{
    Bspline_options options;
    Bspline_parms *parms = &options.parms;
    Bspline_xform *bxf;
    Volume *moving, *fixed, *moving_grad;
    Volume *vector_field = 0;
    Volume *moving_warped = 0;
    plm_long roi_offset[3];

    bspline_opts_parse_args (&options, argc, argv);

#if (CUDA_FOUND)
    if (parms->threading == BTHR_CUDA) {
        LOAD_LIBRARY_SAFE (libplmcuda);
        LOAD_SYMBOL (CUDA_selectgpu, libplmcuda);
        CUDA_selectgpu (parms->gpuid);
        UNLOAD_LIBRARY (libplmcuda);
    }
#endif

    fixed = read_mha (options.fixed_fn);
    if (!fixed) exit (-1);
    moving = read_mha (options.moving_fn);
    if (!moving) exit (-1);

    parms->fixed = fixed;
    parms->moving = moving;

    volume_convert_to_float (moving);
    volume_convert_to_float (fixed);

    printf ("Making gradient\n");
    moving_grad = volume_make_gradient (moving);

    parms->moving_grad = moving_grad;

#if defined (commentout)
    /* Load and adjust landmarks */
    if (options.fixed_landmarks && options.moving_landmarks) {
        parms->landmarks = bspline_landmarks_load (
            options.fixed_landmarks, options.moving_landmarks);
        bspline_landmarks_adjust (parms->landmarks, fixed, moving);
    }
#endif

    /* Debug */
    //write_mha ("moving_grad.mha", moving_grad);

    /* Allocate memory and build lookup tables */
    printf ("Allocating lookup tables\n");
    memset (roi_offset, 0, 3*sizeof(plm_long));
    if (options.input_xf_fn) {
        bxf = bspline_xform_load (options.input_xf_fn);
        if (!bxf) {
            fprintf (stderr, "Failed to load %s\n", options.input_xf_fn);
            exit (-1);
        }
    } else {
        bxf = new Bspline_xform;
        bxf->initialize (
            fixed->origin,
            fixed->spacing,
            fixed->dim,
            roi_offset,
            fixed->dim,
            options.vox_per_rgn,
            fixed->get_direction_matrix()
        );
    }

    /* Run the optimization */
    printf ("Running optimization.\n");
    bspline_optimize (bxf, parms);
    printf ("Done running optimization.\n");

    /* Save output transform */
    if (options.output_xf_fn) {
        bxf->save (options.output_xf_fn);
    }

    /* Create vector field from bspline coefficients and save */
    if (options.output_vf_fn 
        || options.output_warped_fn 
#if defined (commentout)
        || (options.warped_landmarks && options.fixed_landmarks 
            && options.moving_landmarks)
#endif
    )
    {
        printf ("Creating vector field.\n");
        vector_field = new Volume (fixed->dim, fixed->origin, 
            fixed->spacing, fixed->direction_cosines, 
            PT_VF_FLOAT_INTERLEAVED, 3);
        if (parms->threading == BTHR_CUDA) {
#if (CUDA_FOUND)
            LOAD_LIBRARY_SAFE (libplmcuda);
            LOAD_SYMBOL (CUDA_bspline_interpolate_vf, libplmcuda);
            CUDA_bspline_interpolate_vf (vector_field, bxf);
            UNLOAD_LIBRARY (libplmcuda);
#else
            bspline_interpolate_vf (vector_field, bxf);
#endif
        } else {
            bspline_interpolate_vf (vector_field, bxf);
        }
    }

    /* Create warped output image and save */
    if (options.output_warped_fn) {
        printf ("Warping image.\n");
        moving_warped = vf_warp (0, moving, vector_field);
        if (moving_warped) {
            printf ("Writing warped image.\n");
            write_mha (options.output_warped_fn, moving_warped);
        } else {
            printf ("Sorry, couldn't create warped image.\n");
        }
    }

    /* Write the vector field */
    if (options.output_vf_fn) {
        printf ("Writing vector field.\n");
        write_mha (options.output_vf_fn, vector_field);
    }

    /* Free memory */
    printf ("Done warping images.\n");
    delete bxf;
    delete fixed;
    delete moving;
    delete moving_grad;
    delete moving_warped;
    delete vector_field;

    printf ("Done freeing memory\n");

    return 0;
}
