/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <math.h>
#include "autotune_opencl.h"

void 
divideWork (
    cl_device_id *devices, 
    cl_uint device_count, 
    int dimensions, 
    size_t work_per_device[MAX_GPU_COUNT][3], 
    size_t *work_total
)
{
    float compute_ratio;
    size_t allotted_work, total_allotted_work;
    cl_device_id device;
    cl_uint compute_units, clock_frequency, total_compute_ability, compute_ability[MAX_GPU_COUNT], max_compute_ability, max_device;
    int dim = dimensions - 1;
	
    total_compute_ability = 0;
    max_compute_ability = 0;
    max_device = 0;
    for (cl_uint i = 0; i < device_count; i++) {
	device = devices[i];
	clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
	clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
	compute_ability[i] = compute_units * clock_frequency;
	total_compute_ability += compute_ability[i];
	if (compute_ability[i] > max_compute_ability) {
	    max_compute_ability = compute_ability[i];
	    max_device = i;
	}
    }

    total_allotted_work = 0;
    for (cl_uint i = 0; i < device_count; i++) {
	for (int j = 0; j < dim; j++) {
	    work_per_device[i][j] = work_total[j];
	}

	compute_ratio = (float)compute_ability[i] / (float)total_compute_ability;
	compute_ratio = floor(compute_ratio * 100) / 100;
	allotted_work = (size_t)(floor((float)work_total[dim] * compute_ratio));
	work_per_device[i][dim] = allotted_work;
	total_allotted_work += allotted_work;


    }
    work_per_device[max_device][dim] += work_total[dim] - total_allotted_work;
}
