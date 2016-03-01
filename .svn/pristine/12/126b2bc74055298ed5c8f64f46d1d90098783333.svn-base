/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#define PLM_CUDA_COMPILE 1
#include "plmreconstruct_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined (_WIN32)
#include <windows.h>
#endif

#include "drr.h"
#include "drr_cuda.h"

void*
drr_cuda_state_create (
    Proj_image *proj,
    Volume *vol,
    Drr_options *options
)
{
    return drr_cuda_state_create_cu (proj, vol, options);
}

void
drr_cuda_state_destroy (
    void *void_state
)
{
    drr_cuda_state_destroy_cu (void_state);
}
