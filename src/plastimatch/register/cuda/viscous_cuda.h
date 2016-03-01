/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _viscous_cuda_h_
#define _viscous_cuda_h_

#include "plmregister_config.h"
#include "delayload.h"

#if defined __cplusplus
extern "C" {
#endif

    PLMREGISTERCUDA_API
    DELAYLOAD_WRAP (
    int CUDA_viscous_main, 
        int argc, char** argv
    );
        
#if defined __cplusplus
}
#endif

#endif
