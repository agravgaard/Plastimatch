/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "plmsys.h"

#include "autotune_opencl.h"
#include "drr_opencl.h"
#include "drr_opencl_p.h"
#include "drr.h"
#include "drr_opts.h"
#include "plm_math.h"
#include "opencl_util.h"
#include "opencl_util_nvidia.h"
#include "proj_image.h"
#include "proj_matrix.h"
#include "volume.h"
#include "volume_limit.h"


#if defined (commentout)

/* Globals */
cl_int error;					/* Use for error checking */
cl_context context[MAX_GPU_COUNT];				/* Context from device */
cl_command_queue command_queue[MAX_GPU_COUNT];	/* Command Queue from Context */
cl_program program[MAX_GPU_COUNT];				/* Program from .cl file */
cl_uint device_count;			/* number of devices available */
cl_device_id *devices;			/* pointer to devices */
cl_device_id device;			/* object for individual device in for loop */
char device_name[MAX_GPU_COUNT][256];	/* device names */

void drr_render_volume_perspective_cl (
    Proj_image *proj,
    Volume *vol,
    double ps[2],
    Drr_options *options,
    int img_size,
    int float3_size,
    cl_mem *g_dev_vol,
    cl_mem *g_dev_img,
    cl_mem *c_vol_dim,
    cl_mem *c_img_dim,
    cl_mem *c_offset,
    cl_mem *c_pix_spacing,
    cl_mem *c_vol_limits,
    cl_mem *c_p1,
    cl_mem *c_ul_room,
    cl_mem *c_incr_r,
    cl_mem *c_incr_c,
    cl_mem *c_pixel_device,
    cl_kernel *drr_kernel,
    cl_ulong *drr_total,
    cl_ulong *img_total,
    size_t drr_global_work_size[MAX_GPU_COUNT][2],
    size_t drr_local_work_size[MAX_GPU_COUNT][2],
    int4 *pixels_per_device,
    int2 *pixel_offset,
    int *img_size_device)
{
    double p1[3];
    double ic_room[3];
    double ul_room[3];
    double incr_r[3];
    double incr_c[3];
    double tmp[3];
    Volume_limit vol_limit;
    double nrm[3], pdn[3], prt[3];
    Proj_matrix *pmat = proj->pmat;
    int res_r = options->image_window[1] - options->image_window[0] + 1;
    int res_c = options->image_window[3] - options->image_window[2] + 1;
    float p1_f[3];
    float ul_room_f[3];
    float incr_r_f[3];
    float incr_c_f[3];
    Volume_limit_f vol_limit_f;

    /* Set defaults */
    int2 img_dim = {res_r, res_c};
    int preprocess_attenuation = DRR_PREPROCESS_ATTENUATION;

    /* Create variables for kernel and memory timing */
    cl_event drr_event[MAX_GPU_COUNT], img_event[MAX_GPU_COUNT];

    proj_matrix_get_nrm(pmat, nrm);
    proj_matrix_get_pdn(pmat, pdn);
    proj_matrix_get_prt(pmat, prt);

    /* Compute position of image center in room coordinates */
    vec3_scale3(tmp, nrm, - pmat->sid);
    vec3_add3(ic_room, pmat->cam, tmp);

    /* Compute incremental change in 3d position for each change 
       in panel row/column. */
    vec3_scale3(incr_c, prt, ps[1]);
    vec3_scale3(incr_r, pdn, ps[0]);

    /* Get position of upper left pixel on panel */
    vec3_copy(ul_room, ic_room);
    vec3_scale3(tmp, incr_r, - pmat->ic[0]);
    vec3_add2(ul_room, tmp);
    vec3_scale3(tmp, incr_c, - pmat->ic[1]);
    vec3_add2(ul_room, tmp);

    /* drr_ray_trace uses p1 & p2, p1 is the camera, p2 is in the 
       direction of the ray */
    vec3_copy(p1, pmat->cam);

    /* Compute volume boundary box */
    volume_limit_set (&vol_limit, vol);

    /* Convert all doubles to floats */
    for (int i = 0; i < 3; i++) {
	p1_f[i] = (float)p1[i];
	ul_room_f[i] = (float)ul_room[i];
	incr_r_f[i] = (float)incr_r[i];
	incr_c_f[i] = (float)incr_c[i];
	vol_limit_f.dir[i] = vol_limit.dir[i];
	vol_limit_f.lower_limit[i] = vol_limit.lower_limit[i];
	vol_limit_f.upper_limit[i] = vol_limit.upper_limit[i];
    }

    for (cl_uint i = 0; i < device_count; i++) {
	/* Copy global memory from host to device */
	error = clEnqueueWriteBuffer(command_queue[i], g_dev_img[i], CL_FALSE, 0, img_size_device[i], (float*)proj->img + pixel_offset[i].y, 0, NULL, &img_event[i]);
	oclCheckError(error, CL_SUCCESS);

	/* Copy constant memory from host to device */
	error |= clEnqueueWriteBuffer(command_queue[i], c_vol_dim[i], CL_FALSE, 0, 3 * sizeof(int), &vol->dim, 0, NULL, NULL);
	error |= clEnqueueWriteBuffer(command_queue[i], c_img_dim[i], CL_FALSE, 0, sizeof(int2), &img_dim, 0, NULL, NULL);
	error |= clEnqueueWriteBuffer(command_queue[i], c_offset[i], CL_FALSE, 0, float3_size, &vol->offset, 0, NULL, NULL);
	error |= clEnqueueWriteBuffer(command_queue[i], c_pix_spacing[i], CL_FALSE, 0, float3_size, &vol->pix_spacing, 0, NULL, NULL);
	error |= clEnqueueWriteBuffer(command_queue[i], c_vol_limits[i], CL_FALSE, 0, sizeof(Volume_limit_f), &vol_limit_f, 0, NULL, NULL);
	error |= clEnqueueWriteBuffer(command_queue[i], c_p1[i], CL_FALSE, 0, float3_size, p1_f, 0, NULL, NULL);
	error |= clEnqueueWriteBuffer(command_queue[i], c_ul_room[i], CL_FALSE, 0, float3_size, &ul_room_f, 0, NULL, NULL);
	error |= clEnqueueWriteBuffer(command_queue[i], c_incr_r[i], CL_FALSE, 0, float3_size, incr_r_f, 0, NULL, NULL);
	error |= clEnqueueWriteBuffer(command_queue[i], c_incr_c[i], CL_FALSE, 0, float3_size, incr_c_f, 0, NULL, NULL);
	error |= clEnqueueWriteBuffer(command_queue[i], c_pixel_device[i], CL_FALSE, 0, sizeof(int4), &pixels_per_device[i], 0, NULL, NULL);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Wait for all queues to finish */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Count device time */
    for (cl_uint i = 0; i < device_count; i++) {
	img_total[i] += opencl_timer (img_event[i]);
    }

    /* Sets drr kernel arguments */
    for (cl_uint i = 0; i < device_count; i++) {
	error |= clSetKernelArg(drr_kernel[i], 0, sizeof(cl_mem), (void *) &g_dev_vol[i]);
	error |= clSetKernelArg(drr_kernel[i], 1, sizeof(cl_mem), (void *) &g_dev_img[i]);
	error |= clSetKernelArg(drr_kernel[i], 2, sizeof(cl_mem), (void *) &c_vol_dim[i]);
	error |= clSetKernelArg(drr_kernel[i], 3, sizeof(cl_mem), (void *) &c_img_dim[i]);
	error |= clSetKernelArg(drr_kernel[i], 4, sizeof(cl_mem), (void *) &c_offset[i]);
	error |= clSetKernelArg(drr_kernel[i], 5, sizeof(cl_mem), (void *) &c_pix_spacing[i]);
	error |= clSetKernelArg(drr_kernel[i], 6, sizeof(cl_mem), (void *) &c_vol_limits[i]);
	error |= clSetKernelArg(drr_kernel[i], 7, sizeof(cl_mem), (void *) &c_p1[i]);
	error |= clSetKernelArg(drr_kernel[i], 8, sizeof(cl_mem), (void *) &c_ul_room[i]);
	error |= clSetKernelArg(drr_kernel[i], 9, sizeof(cl_mem), (void *) &c_incr_r[i]);
	error |= clSetKernelArg(drr_kernel[i], 10, sizeof(cl_mem), (void *) &c_incr_c[i]);
	error |= clSetKernelArg(drr_kernel[i], 11, sizeof(cl_mem), (void *) &c_pixel_device[i]);
	error |= clSetKernelArg(drr_kernel[i], 12, sizeof(float), &options->scale);
	error |= clSetKernelArg(drr_kernel[i], 13, sizeof(int), &options->output_format);
	error |= clSetKernelArg(drr_kernel[i], 14, sizeof(int), &preprocess_attenuation);
	error |= clSetKernelArg(drr_kernel[i], 15, sizeof(int), &options->exponential_mapping);
	error |= clSetKernelArg(drr_kernel[i], 16, sizeof(int), &pixel_offset[i].x);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Wait for all queues to finish */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Invoke all drr kernels */
    for (cl_uint i = 0; i < device_count; i++) {
	error = clEnqueueNDRangeKernel(command_queue[i], drr_kernel[i], 2, NULL, drr_global_work_size[i], drr_local_work_size[i], 0, NULL, &drr_event[i]);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Wait for all kernels to finish */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Count kernel time */
    for (cl_uint i = 0; i < device_count; i++)
	drr_total[i] += opencl_timer (drr_event[i]);

    /* Copy img/multispectral from device to host */
    for (cl_uint i = 0; i < device_count; i++) {
	error = clEnqueueReadBuffer(command_queue[i], g_dev_img[i], CL_FALSE, 0, img_size_device[i], (float*)proj->img + pixel_offset[i].y, 0, NULL, &img_event[i]);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Count device to host time */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Count device time */
    for (cl_uint i = 0; i < device_count; i++) {
	img_total[i] += opencl_timer (img_event[i]);
    }
}

void create_matrix_and_drr_cl (
	Volume* vol,
	Proj_image *proj,
	double cam[3],
	double tgt[3],
	double nrm[3], 
	int a,
	Drr_options* options,
	int img_size,
	int float3_size,
	cl_mem *g_dev_vol,
	cl_mem *g_dev_img,
	cl_mem *c_vol_dim,
	cl_mem *c_img_dim,
	cl_mem *c_offset,
	cl_mem *c_pix_spacing,
	cl_mem *c_vol_limits,
	cl_mem *c_p1,
	cl_mem *c_ul_room,
	cl_mem *c_incr_r,
	cl_mem *c_incr_c,
	cl_mem *c_pixel_device,
	cl_kernel *drr_kernel,
	cl_ulong *drr_total,
	cl_ulong *img_total,
	size_t drr_global_work_size[MAX_GPU_COUNT][2],
	size_t drr_local_work_size[MAX_GPU_COUNT][2],
	int4 *pixels_per_device,
	int2 *pixel_offset,
	int *img_size_device)
{
	char mat_fn[256];
	char img_fn[256];
	Proj_matrix *pmat = proj->pmat;
	double vup[3] = { options->vup[0], options->vup[1], options->vup[2] };
	double sid = options->sid;
	Plm_timer* timer = plm_timer_create ();

	/* Set ic = image center (in pixels), and ps = pixel size (in mm)
	   Note: pixels are numbered from 0 to ires-1 */
	double ic[2] = { options->image_center[0], options->image_center[1] };

	/* Set image resolution */
	int ires[2] = { options->image_resolution[0], options->image_resolution[1] };

	/* Set physical size of imager in mm */
	int isize[2] = { options->image_size[0], options->image_size[1] };

	/* Set pixel size in mm */
	double ps[2] = { (double)isize[0]/(double)ires[0], (double)isize[1]/(double)ires[1] };

	/* Create projection matrix */
	sprintf(mat_fn, "%s%04d.txt", options->output_prefix, a);
	proj_matrix_set(pmat, cam, tgt, vup, sid, ic, ps, ires);

	if (options->output_format == OUTPUT_FORMAT_PFM) {
		sprintf(img_fn, "%s%04d.pfm", options->output_prefix, a);
	} else if (options->output_format == OUTPUT_FORMAT_PGM) {
		sprintf(img_fn, "%s%04d.pgm", options->output_prefix, a);
	} else {
		sprintf(img_fn, "%s%04d.raw", options->output_prefix, a);
	}

	drr_render_volume_perspective_cl(proj, vol, ps, options, img_size, float3_size, g_dev_vol, g_dev_img, c_vol_dim, c_img_dim, c_offset, c_pix_spacing, c_vol_limits, c_p1, c_ul_room, c_incr_r, c_incr_c, c_pixel_device, drr_kernel, drr_total, img_total, drr_global_work_size, drr_local_work_size, pixels_per_device, pixel_offset, img_size_device);

	plm_timer_start(timer);
	proj_image_save(proj, img_fn, mat_fn);
	printf("I/O time: %f sec\n", plm_timer_report(timer));
    plm_timer_destroy (timer);
}

#endif /* commentout */

void drr_opencl_render_volume (
    Volume* vol,
    Drr_options* options
)
{
    /* Declare all global memory buffers */
    cl_mem g_dev_vol[MAX_GPU_COUNT];
    cl_mem g_dev_img[MAX_GPU_COUNT];

    /* Delcare all constant memory buffers */
    cl_mem c_vol_dim[MAX_GPU_COUNT];
    cl_mem c_img_dim[MAX_GPU_COUNT];
    cl_mem c_offset[MAX_GPU_COUNT];
    cl_mem c_pix_spacing[MAX_GPU_COUNT];
    cl_mem c_vol_limits[MAX_GPU_COUNT];
    cl_mem c_p1[MAX_GPU_COUNT];
    cl_mem c_ul_room[MAX_GPU_COUNT];
    cl_mem c_incr_r[MAX_GPU_COUNT];
    cl_mem c_incr_c[MAX_GPU_COUNT];
    cl_mem c_pixel_device[MAX_GPU_COUNT];

    /* Declare all kernels */
    cl_kernel drr_kernel[MAX_GPU_COUNT];

    /* Declare other OpenCL variables */
    cl_event vol_event[MAX_GPU_COUNT];
    cl_ulong drr_total[MAX_GPU_COUNT];
    cl_ulong img_total[MAX_GPU_COUNT];
    cl_ulong vol_total[MAX_GPU_COUNT];
    cl_ulong preprocess_total[MAX_GPU_COUNT];
    int4 pixels_per_device[MAX_GPU_COUNT];
    int2 pixel_offset[MAX_GPU_COUNT];
    int img_size_device[MAX_GPU_COUNT];
    size_t program_length;
    size_t drr_local_work_size[MAX_GPU_COUNT][2];
    size_t drr_global_work_size[MAX_GPU_COUNT][2];
    size_t work_per_device[MAX_GPU_COUNT][3];

    /* Calculate dynamic size of memory buffers */
    int image_height = options->image_window[1] - options->image_window[0] + 1;
    int image_width = options->image_window[3] - options->image_window[2] + 1;
    int float3_size = 3 * sizeof(float);
    int vol_size = (vol->dim[0] * vol->dim[1] * vol->dim[2]) * sizeof(float);
    int img_size = image_height * image_width * sizeof(float);
    size_t work_total[3] = {image_width, image_height, 0};

    Opencl_device ocl_dev;
    opencl_open_device (&ocl_dev);
    opencl_load_programs (&ocl_dev, "drr_opencl.cl");

#if defined (commentout)
    /* Calculate number of voxels per device */
    divideWork (devices, device_count, 2, work_per_device, work_total);
#endif

    Proj_image *proj;
    Proj_matrix *pmat;
    Plm_timer timer = plm_timer_create ();

#if defined (commentout)
    /* Initialize timers */
    for (cl_uint i = 0; i < device_count; i++) {
	drr_total[i] = 0;
	img_total[i] = 0;
	vol_total[i] = 0;
    }

    /* Allocate pixels to each device */
    for (cl_uint i = 0; i < device_count; i++) {
	pixels_per_device[i].x = (int)work_per_device[i][0];
	pixels_per_device[i].y = (int)work_per_device[i][1];
	pixels_per_device[i].z = pixels_per_device[i].x * pixels_per_device[i].y;
    }

    /* Determine pixel offset on each device and memory buffer size */
    for (cl_uint i = 0; i < device_count; i++) {
	pixel_offset[i].x = 0;
	pixel_offset[i].y = 0;
	for (cl_uint j = 0; j < i; j++) {
	    pixel_offset[i].x += pixels_per_device[j].y;
	    pixel_offset[i].y += pixels_per_device[j].z;
	}
	img_size_device[i] = pixels_per_device[i].z * sizeof(float);
    }

    for (cl_uint i = 0; i < device_count; i++) {

	/* Allocate global memory on all devices */
	g_dev_vol[i] = clCreateBuffer(context[i],  CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, vol_size, NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	g_dev_img[i] = clCreateBuffer(context[i],  CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, img_size_device[i], NULL, &error);
	oclCheckError(error, CL_SUCCESS);

	/* Allocate constant memory on all devices */
	c_vol_dim[i] = clCreateBuffer(context[i],  CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, 3 * sizeof(int), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_img_dim[i] = clCreateBuffer(context[i],  CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(int2), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_offset[i] = clCreateBuffer(context[i],  CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, float3_size, NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_pix_spacing[i] = clCreateBuffer(context[i],  CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, float3_size, NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_vol_limits[i] = clCreateBuffer(context[i],  CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, 3 * sizeof(Volume_limit), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_p1[i] = clCreateBuffer(context[i],  CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, float3_size, NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_ul_room[i] = clCreateBuffer(context[i],  CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, float3_size, NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_incr_r[i] = clCreateBuffer(context[i],  CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, float3_size, NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_incr_c[i] = clCreateBuffer(context[i],  CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, float3_size, NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_pixel_device[i] = clCreateBuffer(context[i],  CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(int4), NULL, &error);
	oclCheckError(error, CL_SUCCESS);

	/* Create the drr kernel on all devices */
	//drr_kernel[i] = clCreateKernel(program[i], "kernel_drr", &error);
	drr_kernel[0] = clCreateKernel(program, "kernel_drr", &error);
	oclCheckError(error, CL_SUCCESS);
    }


    /* Wait for all queues to finish */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Calculate the drr runtime environment */
    for (cl_uint i = 0; i < device_count; i++) {
	drr_local_work_size[i][0] = 128;
	drr_local_work_size[i][1] = 1;
	drr_global_work_size[i][0] = shrRoundUp((int)drr_local_work_size[i][0], pixels_per_device[i].x);
	drr_global_work_size[i][1] = shrRoundUp((int)drr_local_work_size[i][1], pixels_per_device[i].y);
    }

    /* Copy memory from host to device */
    for (cl_uint i = 0; i < device_count; i++) {
	error = clEnqueueWriteBuffer(command_queue[i], g_dev_vol[i], CL_FALSE, 0, vol_size, vol->img, 0, NULL, &vol_event[i]);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Waits for volume to finish copying */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Count host to device time */
    for (cl_uint i = 0; i < device_count; i++)
	vol_total[i] += opencl_timer (vol_event[i]);

#if defined (DRR_PREPROCESS_ATTENUATION)
    /* Create variables for preprocess kernel and memory timing */
    size_t preprocess_local_work_size[MAX_GPU_COUNT];
    size_t preprocess_global_work_size[MAX_GPU_COUNT];
    cl_event preprocess_event[MAX_GPU_COUNT];

    /* Initialize preprocess timers */
    for (cl_uint i = 0; i < device_count; i++)
	preprocess_total[i] = 0;

    if (vol->pix_type != PT_FLOAT) {
	float *old_img = (float*)vol->img;
	float *new_img = (float*)malloc(vol->npix * sizeof(float));

	for (int i = 0; i < vol->npix; i++) {
	    new_img[i] = old_img[i];
	}

	vol->pix_type = PT_FLOAT;
	free(vol->img);
	vol->img = new_img;

	/* Update global memory from host to device */
	for (cl_uint i = 0; i < device_count; i++) {
	    error = clEnqueueWriteBuffer(command_queue[i], g_dev_vol[i], CL_FALSE, 0, vol_size, vol->img, 0, NULL, &vol_event[i]);
	    oclCheckError(error, CL_SUCCESS);
	}

	/* Waits for volume to finish copying */
	for (cl_uint i = 0; i < device_count; i++)
	    clFinish(command_queue[i]);

	/* Count host to device time */
	for (cl_uint i = 0; i < device_count; i++)
	    vol_total[i] += opencl_timer (vol_event[i]);
    }

    /* Creates the kernel object */
    cl_kernel preprocess_kernel[MAX_GPU_COUNT];
    for (cl_uint i = 0; i < device_count; i++) {
	//preprocess_kernel[i] = clCreateKernel(program[i], "preprocess_attenuation_cl", &error);
	preprocess_kernel[i] = clCreateKernel (program, "preprocess_attenuation_cl", &error);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Sets preprocess kernel arguments */
    for (cl_uint i = 0; i < device_count; i++) {
	error = clSetKernelArg(preprocess_kernel[i], 0, sizeof(cl_mem), (void *) &g_dev_vol[i]);
	error |= clSetKernelArg(preprocess_kernel[i], 1, sizeof(int), &vol->npix);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Wait for all queues to finish */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Enqueue kernel and execute */
    for (cl_uint i=0; i < device_count; i++) {
	preprocess_local_work_size[i] = 512;
	preprocess_global_work_size[i] = shrRoundUp((int)preprocess_local_work_size[i], vol->npix);
    }

    for (cl_uint i = 0; i < device_count; i++) {
	error = clEnqueueNDRangeKernel(command_queue[i], preprocess_kernel[i], 1, NULL, &preprocess_global_work_size[i], &preprocess_local_work_size[i], 0, NULL, &preprocess_event[i]);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Waits for queue to finish */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Count kernel time */
    for (cl_uint i = 0; i < device_count; i++)
	preprocess_total[i] += opencl_timer (preprocess_event[i]);

    /* Release kernels and memory */
    for (cl_uint i = 0; i < device_count; i++) {
	clReleaseKernel(preprocess_kernel[i]);
    }

#endif /* DRR_PREPROCESS_ATTENUATION */

    /* tgt is isocenter */
    double tgt[3] = { options->isocenter[0], options->isocenter[1], options->isocenter[2] };

    plm_timer_start (timer);

    /* Allocate data for image and matrix */
    proj = proj_image_create();
    proj_image_create_pmat(proj);
    proj_image_create_img(proj, options->image_resolution);
    pmat = proj->pmat;

    /* If nrm was specified, only create a single image */
    if (options->have_nrm) {
	double cam[3];
	double nrm[3] = { options->nrm[0], options->nrm[1], options->nrm[2] };

	/* Make sure nrm is normal */
	vec3_normalize1(nrm);

	/* Place camera at distance "sad" from the volume isocenter */
	cam[0] = tgt[0] + options->sad * nrm[0];
	cam[1] = tgt[1] + options->sad * nrm[1];
	cam[2] = tgt[2] + options->sad * nrm[2];

	create_matrix_and_drr_cl(vol, proj, cam, tgt, nrm, 0, options, img_size, float3_size, g_dev_vol, g_dev_img, c_vol_dim, c_img_dim, c_offset, c_pix_spacing, c_vol_limits, c_p1, c_ul_room, c_incr_r, c_incr_c, c_pixel_device, drr_kernel, drr_total, img_total, drr_global_work_size, drr_local_work_size, pixels_per_device, pixel_offset, img_size_device);
    } else {
	/* Otherwise, loop through camera angles */
	for (int a = 0; a < options->num_angles; a++) {
	    double angle = a * options->angle_diff;
	    double cam[3];
	    double nrm[3];

	    shrLog("Rendering DRR %d\n", a);

	    /* Place camera at distance "sad" from the volume isocenter */
	    cam[0] = tgt[0] + options->sad * cos(angle);
	    cam[1] = tgt[1] - options->sad * sin(angle);
	    cam[2] = tgt[2];
		
	    /* Compute normal vector */
	    vec3_sub3(nrm, tgt, cam);
	    vec3_normalize1(nrm);

	    create_matrix_and_drr_cl(vol, proj, cam, tgt, nrm, a, options, img_size, float3_size, g_dev_vol, g_dev_img, c_vol_dim, c_img_dim, c_offset, c_pix_spacing, c_vol_limits, c_p1, c_ul_room, c_incr_r, c_incr_c, c_pixel_device, drr_kernel, drr_total, img_total, drr_global_work_size, drr_local_work_size, pixels_per_device, pixel_offset, img_size_device);
	}
    }

    proj_image_destroy(proj);

    for (cl_uint i = 0; i < device_count; i++) {
	/* Release kernels */
	clReleaseKernel(drr_kernel[i]);

	/* Release constant memory buffers */
	clReleaseMemObject(c_pixel_device[i]);
	clReleaseMemObject(c_incr_c[i]);
	clReleaseMemObject(c_incr_r[i]);
	clReleaseMemObject(c_ul_room[i]);
	clReleaseMemObject(c_p1[i]);
	clReleaseMemObject(c_vol_limits[i]);
	clReleaseMemObject(c_pix_spacing[i]);
	clReleaseMemObject(c_offset[i]);
	clReleaseMemObject(c_img_dim[i]);
	clReleaseMemObject(c_vol_dim[i]);

	/* Release global memory buffers */
	clReleaseMemObject(g_dev_img[i]);
	clReleaseMemObject(g_dev_vol[i]);
    }

    /***************************************************************/

    /**************************************************************** 
     * STEP 4: Perform timing										* 
     ****************************************************************/

    float overall_runtime = 0;
    for (cl_uint i = 0; i < device_count; i++) {
	float drr_kernel_runtime = drr_total[i] * 1.0e-6f;
	float preprocess_kernel_runtime = preprocess_total[i] * 1.0e-6f;
	float img_copy_runtime = img_total[i] * 1.0e-6f;
	float vol_copy_runtime = vol_total[i] * 1.0e-6f;
	float total_runtime = (drr_kernel_runtime + preprocess_kernel_runtime + img_copy_runtime + vol_copy_runtime) * 1.0e-3f;
	overall_runtime += total_runtime;

	shrLog("Device %d: %s\n", i, device_name[i]);
	shrLog("\tDRR Kernel run time: %f ms\n", drr_kernel_runtime);
	shrLog("\tHu Kernel run time: %f ms\n", preprocess_kernel_runtime);
	shrLog("\tImage host/device & device/host copy run time: %f ms\n", img_copy_runtime);
	shrLog("\tVolume host to device copy run time: %f ms\n", vol_copy_runtime);
	shrLog("\tTotal run time: %f s\n\n", total_runtime);
    }

    /***************************************************************/

    /**************************************************************** 
     * STEP 4: Cleanup OpenCL and finish								* 
     ****************************************************************/

    for (cl_uint i = 0; i < device_count; i++) {
	//clReleaseProgram(program[i]);
	clReleaseProgram(program);
	clReleaseCommandQueue(command_queue[i]);
	clReleaseContext(context[i]);
    }

    shrLog("Done DRR_OPENCL...\n\n");
    shrLog("Total OpenCL run time: %f s\n", overall_runtime);
    printf("Total run time: %g s\n", plm_timer_report(timer));
    plm_timer_destroy (timer);
#endif /* commentout */
}
