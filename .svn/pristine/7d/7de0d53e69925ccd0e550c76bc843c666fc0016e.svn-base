/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if OPENMP_FOUND
#include <omp.h>
#endif

#include "delayload.h"
#include "fdk.h"
#include "fdk_opts.h"
#include "fdk_cuda.h"
#include "fdk_opencl.h"
#include "fdk_util.h"
#include "mha_io.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "proj_image_dir.h"
#include "threading.h"
#include "volume.h"

int 
main (int argc, char* argv[])
{
    Fdk_parms parms;
    Volume* vol;
    Proj_image_dir *proj_dir;

//    LOAD_LIBRARY (libplmopencl);

//    LOAD_SYMBOL (opencl_reconstruct_conebeam, libplmopencl);
    
    /* Parse command line arguments */
    fdk_parse_args (&parms, argc, argv);

    /* Look for input files */
    proj_dir = new Proj_image_dir (parms.input_dir);
    if (proj_dir->num_proj_images < 1) {
	print_and_exit ("Error: couldn't find input files in directory %s\n",
	    parms.input_dir);
    }

    /* Set the panel offset */
    double xy_offset[2] = { parms.xy_offset[0], parms.xy_offset[1] };
    proj_dir->set_xy_offset (xy_offset);

    /* Choose subset of input files if requested */
    if (parms.image_range_requested) {
	proj_dir->select (parms.first_img, parms.skip_img, 
	    parms.last_img);
    }

    /* Allocate memory */
    vol = my_create_volume (&parms);

    printf ("Reconstructing...\n");
    switch (parms.threading) {
#if (CUDA_FOUND)
    case THREADING_CUDA:
	CUDA_reconstruct_conebeam (vol, proj_dir, &parms);
	break;
#endif
#if (OPENCL_FOUND)
    case THREADING_OPENCL:
    opencl_reconstruct_conebeam (vol, proj_dir, &parms);
    //OPENCL_reconstruct_conebeam_and_convert_to_hu (vol, proj_dir, &parms);
    break;
#endif
    case THREADING_CPU_SINGLE:
    case THREADING_CPU_OPENMP:
    default:
	reconstruct_conebeam (vol, proj_dir, &parms);
    }

    /* Free memory */
    delete proj_dir;

    /* Prepare HU values in output volume */
    convert_to_hu (vol, &parms);

    /* Do bowtie filter corrections */
    //fdk_do_bowtie (vol, &parms);

    /* Write output */
    printf ("Writing output volume(s)...\n");
    write_mha (parms.output_file, vol);

    /* Free memory */
    delete vol;

//    UNLOAD_LIBRARY (libplmopencl);

    printf(" done.\n\n");

    return 0;
}
