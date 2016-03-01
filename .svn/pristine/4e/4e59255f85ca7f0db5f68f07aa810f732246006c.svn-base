/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "delayload.h"
#include "opencl_probe.h"
#include "opencl_util.h"


/* opencl_probe() 
 * Return 1 if working device found, 0 if not found
 */
int 
opencl_probe ()
{
    Opencl_device ocl_dev;
    Opencl_buf *ocl_buf_in, *ocl_buf_out;
    cl_uint buf_entries;
    cl_uint buf_size;
    cl_uint *buf_in, *buf_out;
    cl_uint multiplier = 2;
    cl_int status;
    bool opencl_works;

    status = opencl_open_device (&ocl_dev);
    if (status != CL_SUCCESS) {
	return 0;
    }
    opencl_load_programs (&ocl_dev, "opencl_probe.cl");

    buf_entries = 100;
    buf_size = buf_entries * sizeof (cl_uint);
    buf_in = (cl_uint*) malloc (buf_size);
    buf_out = (cl_uint*) malloc (buf_size);
    for (cl_uint i = 0; i < buf_entries; i++) {
	buf_in[i] = i;
	buf_out[i] = 0;
    }

    ocl_buf_in = opencl_buf_create (&ocl_dev, 
	CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, 
	buf_size, buf_in);
    ocl_buf_out = opencl_buf_create (&ocl_dev, 
	CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, 
	buf_size, buf_out);
    opencl_kernel_create (&ocl_dev, "kernel_1");

    opencl_set_kernel_args (
	&ocl_dev, 
	sizeof (cl_mem), 
	&ocl_buf_out[0], 
	sizeof (cl_mem), 
	&ocl_buf_in[0], 
	sizeof (cl_uint), 
	&multiplier, 
	(size_t) 0
    );
    opencl_kernel_enqueue (&ocl_dev, buf_entries, 1);

    opencl_buf_read (&ocl_dev, ocl_buf_out, buf_size, buf_out);

    opencl_works = 1;
    for (cl_uint i = 0; i < buf_entries; i++) {
	if (buf_out[i] != 2 * i) {
	    opencl_works = 0;
	}
    }

    return opencl_works;
}
