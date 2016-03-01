/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cuda.h>
#include "cuda_probe.h"

int
cuda_probe (void)
{
    int devicecount = -1;
    printf ("Testing for CUDA...\n");
    cudaError_t rc = cudaGetDeviceCount (&devicecount);
    if (rc != cudaSuccess) {
	printf("Call to cudaGetDeviceCount() returned failure (%d)\n",
            (int) rc);
	return 0;
    }

    if (devicecount == 0) {
	printf("Suitable CUDA environment not detected!\n");
	return 0;
    }
    printf ("Devices found: %d\n", devicecount);

    // It is possible at this point that devicecount = 1 and still be
    // without an actual CUDA device.  CUDA 2.0 and 2.1 exhibit this
    // behavior.  Apparently 2.x will detect an emulator device and
    // throw a 1 by reference when you call cudaGetDeviceCount().
    // You are apparently able to distinguish between an actual
    // CUDA device and the emulator by checking the major and minor
    // revision numbers on the compute capability.  Emulated devices
    // are supposed to return 9999 for both major and minor revision
    // numbers.  Some, however, report that while this is the behavior
    // for CUDA 2.0, CUDA 2.1 returns different nonsensical numbers
    // when the detected device is emulated.  Therefore, the best
    // solution (until the behavior is standardised across releases)
    // is to specifically check for compute capabilities we KNOW are
    // working with Plastimatch.
    //


    // Get CUDA device properties.
    cudaDeviceProp props;
    cudaGetDeviceProperties (&props, 0);
    printf ("Compute Capability %d.%d\n", props.major, props.minor);

    if (props.major == 1 || props.major == 2 || props.major == 3) {
    	/* GCS: Cuda 2.2 with Tesla returns compatibility 1.3 */
        /* JAS: Tesla C2050 is compute capability 2.0 */
    	printf ("Device is presumed cuda capable.\n");
    	return 1;
    }

    // Unless proven otherwise, we assume no CUDA.
    printf ("Device is presumed NOT cuda capable.\n");
    return 0;
}
