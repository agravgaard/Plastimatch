/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <dlfcn.h>
#endif

#include "delayload.h"
#include "demons.h"
#include "demons_cuda.h"
#include "demons_state.h"

void
demons_default_parms (Demons_parms* parms)
{
    parms->threading = THREADING_CPU_OPENMP;
    parms->accel = 1.0;
    parms->denominator_eps = 1.0;
    parms->filter_width[0] = 3;
    parms->filter_width[1] = 3;
    parms->filter_width[2] = 3;
    /* GCS FIX: ITK uses sum_d(pix_spacing[d]^2) / (#dim) for homog */
    parms->homog = 1.0;
    parms->max_its = 10;
    parms->filter_std = 5.0;
}

Volume*
demons (
    Volume* fixed, 
    Volume* moving, 
    Volume* moving_grad, 
    Volume* vf_init, 
    Demons_parms* parms
)
{
#if (CUDA_FOUND || OPENCL_FOUND)
    Volume* tmp;
#endif

    switch (parms->threading) {
#if CUDA_FOUND
    case THREADING_CUDA: {
        /* Eventually, all implementation should use demons_state */
        Demons_state demons_state;

        demons_state.init (fixed, moving, moving_grad, vf_init, parms);
        LOAD_LIBRARY_SAFE (libplmregistercuda);
        LOAD_SYMBOL (demons_cuda, libplmregistercuda);
        demons_cuda (&demons_state, fixed, moving, moving_grad, 
	    vf_init, parms);
        UNLOAD_LIBRARY (libplmregistercuda);
	/* GCS FIX: This leaks vf_est... */
	tmp = demons_state.vf_smooth;
	return tmp;
    }
#endif

#if OPENCL_FOUND
    case THREADING_OPENCL:
        tmp = demons_opencl (fixed, moving, moving_grad, vf_init, parms);
        return tmp;
#endif
    case THREADING_CPU_SINGLE:
    case THREADING_CPU_OPENMP:
    default:
        return demons_c (fixed, moving, moving_grad, vf_init, parms);
    }
}
