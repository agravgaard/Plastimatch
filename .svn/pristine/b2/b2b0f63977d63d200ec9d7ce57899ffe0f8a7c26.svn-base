/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "demons.h"
#include "demons_opencl_p.h"
#include "opencl_util.h"
#include "volume.h"

Volume*
demons_opencl (
    Volume* fixed, 
    Volume* moving, 
    Volume* moving_grad, 
    Volume* vf_init, 
    Demons_parms* parms)
{
#if defined (commentout)
    int	it;						/* Iterations */
    float f2mo[3];				/* Offset difference (in cm) from fixed to moving */
    float f2ms[3];				/* Slope to convert fixed to moving */
    float invmps[3];			/* 1/pixel spacing of moving image */
    int vol_size, interleaved_vol_size, inlier_size, inliers, num_elements;
    int *inliers_null;
    int fw[3];
    float ssd, overall_runtime, estimate_kernel_runtime, convolve_kernel, other_kernels, global_copy_runtime, image_copy_runtime, total_runtime;
    float *kerx, *kery, *kerz, *vf_x, *vf_y, *vf_z, *vf_est_img, *vf_smooth_img, *ssd_null;
    double diff_run;
    char device_name[256];	/* Device names */
    char *source_path, *source;
    Volume *vf_est, *vf_smooth;
    Plm_timer* timer = new Plm_timer;

    /* Declare global memory */
    cl_mem g_moving_grad_x, g_moving_grad_y, g_moving_grad_z;
    cl_mem g_moving_grad_mag;
    cl_mem g_vf_est_x_img, g_vf_est_y_img, g_vf_est_z_img;
    cl_mem g_vf_smooth_x_img, g_vf_smooth_y_img, g_vf_smooth_z_img;
    cl_mem g_ssd;
    cl_mem g_inliers;

    /* Declare image/texture memory */
    cl_mem t_fixed_img;
    cl_mem t_moving_img;
    cl_mem t_moving_grad_x_img, t_moving_grad_y_img, t_moving_grad_z_img;
    cl_mem t_moving_grad_mag_img;
    cl_mem t_vf_est_x_img, t_vf_est_y_img, t_vf_est_z_img;
    cl_mem t_vf_smooth_x_img, t_vf_smooth_y_img, t_vf_smooth_z_img;

    /* Declare constant memory */
    cl_mem c_dim;
    cl_mem c_moving_dim;
    cl_mem c_pix_spacing_div2;
    cl_mem c_invmps;
    cl_mem c_f2mo;
    cl_mem c_f2ms;
    cl_mem c_kerx, c_kery, c_kerz;

    /* Declare OpenCL kernels */
    cl_kernel moving_gradient_kernel;
    cl_kernel gradient_magnitude_kernel;
    cl_kernel estimate_kernel;
    cl_kernel reduction_float_kernel;
    cl_kernel reduction_int_kernel;
    cl_kernel convolve_x_kernel;
    cl_kernel convolve_y_kernel;
    cl_kernel convolve_z_kernel;

    /* Declare other OpenCL variables */
    float4 pix_spacing_div2;
    cl_event volume_calc_grad_event, moving_grad_mag_event, estimate_event, reduction_event, convolve_event, global_x_event, global_y_event, global_z_event, copy_x_event, copy_y_event, copy_z_event;
    cl_ulong volume_calc_grad_total, moving_grad_mag_total, estimate_total, reduction_total, convolve_total, global_total, copy_total;
    size_t program_length, vol_row_pitch, vol_slice_pitch, reduction_global_work_size, reduction_local_work_size;
    size_t demons_local_work_size[3];
    size_t demons_global_work_size[3];
    cl_int error;									/* Use for error checking */
    cl_uint device_count;							/* Number of devices available */
    cl_context context;								/* Context from device */
    cl_command_queue command_queue;					/* Command Queue from Context */
    cl_image_format texture_format;					/* Format for reading textures */
    cl_platform_id platform;						/* Platform for system */
    cl_program program;								/* Program from .cl file */
    cl_device_id device;							/* Object for individual device in 'for loop' */
    cl_device_id *devices;							/* Pointer to devices */


    /**************************************************************** 
     * STEP 1: Setup OpenCL											* 
     ****************************************************************/

    /* Set logfile name and start logs */
    shrSetLogFileName("demons_opencl.txt");
    shrLog("\nStarting Demons OpenCL...\n"); 

    /* Get the OpenCL platform */
    error = oclGetPlatformID(&platform);
    oclCheckError(error, CL_SUCCESS);

    /* Get devices of type GPU */
    error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &device_count);
    oclCheckError(error, CL_SUCCESS);
    devices = (cl_device_id *)malloc(device_count * sizeof(cl_device_id) );
    error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, device_count, devices, NULL);
    oclCheckError(error, CL_SUCCESS);

    /* Create context properties */
    cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};
    context = clCreateContext(properties, device_count, devices, NULL, NULL, &error);
    oclCheckError(error, CL_SUCCESS);

    shrLog("Found %d device(s):\n", device_count);

    for (cl_uint i = 0; i < device_count; i++) {
	/* Device info */
	device = oclGetDev(context, i);
	clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
	oclCheckError(error, CL_SUCCESS);
	shrLog("\tDevice %d: %s\n", i, device_name);
    }

    /* Command queue */
    device = oclGetMaxFlopsDev(context);
    command_queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &error);
    oclCheckError(error, CL_SUCCESS);

    /* Program setup */
    source_path = shrFindFilePath("demons_opencl.cl", "");
    oclCheckError(source_path != NULL, shrTRUE);
    source = oclLoadProgSource(source_path, "", &program_length);
    oclCheckError(source != NULL, shrTRUE);

    /* Create the program */
    program = clCreateProgramWithSource(context, 1, (const char **)&source, &program_length, &error);
    oclCheckError(error, CL_SUCCESS);

    /* Build the program */
    error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (error != CL_SUCCESS) {
	/* write out standard error, Build Log and PTX, then return error */
	shrLogEx(LOGBOTH | ERRORMSG, error, STDERROR);
	oclLogBuildInfo(program, oclGetFirstDev(context));
	oclLogPtx(program, oclGetFirstDev(context), "demons_opencl.ptx");
	exit(-1);
    }

    shrLog("\n");

    /***************************************************************/

    /**************************************************************** 
     * STEP 2: Perform Demons Algorithm								* 
     ****************************************************************/

    /* Allocate memory for vector fields */
    if (vf_init) {
	/* If caller has an initial estimate, we copy it */
	vf_smooth = volume_clone (vf_init);
	vf_convert_to_interleaved (vf_smooth);
    } else {
	/* Otherwise initialize to zero */
	vf_smooth = new Volume (fixed->dim, fixed->offset, 
	    fixed->pix_spacing, PT_VF_FLOAT_INTERLEAVED, 
	    fixed->direction_cosines, 0);
    }
    vf_est = new Volume (fixed->dim, fixed->offset, fixed->pix_spacing, 
	PT_VF_FLOAT_INTERLEAVED, fixed->direction_cosines, 0);
    vf_est_img = (float*) vf_est->img;
    vf_smooth_img = (float*) vf_smooth->img;

    /* Make sure volume is not too large for OpenCL image size */
    if (fixed->dim[0] > CL_DEVICE_IMAGE3D_MAX_WIDTH || fixed->dim[1] > CL_DEVICE_IMAGE3D_MAX_HEIGHT || fixed->dim[2] > CL_DEVICE_IMAGE3D_MAX_DEPTH) {
	shrLog("Volume dimensions too large for %s\n", CL_DEVICE_NAME);
	shrLog("Maximum height, width and depth: %d x %d x %d\n", CL_DEVICE_IMAGE3D_MAX_HEIGHT, CL_DEVICE_IMAGE3D_MAX_WIDTH, CL_DEVICE_IMAGE3D_MAX_DEPTH);
	shrLog("Volume height, width and depth: %d x %d x %d\n", fixed->dim[1], fixed->dim[0], fixed->dim[2]);
	shrLog("Exiting...\n\n");
	exit(-1);
    }

    /* Set global and local work sizes */
    demons_local_work_size[0] = BLOCK_SIZE;
    demons_local_work_size[1] = 1;
    demons_local_work_size[2] = 1;
    demons_global_work_size[0] = shrRoundUp((int)demons_local_work_size[0], fixed->dim[0]);
    demons_global_work_size[1] = shrRoundUp((int)demons_local_work_size[1], fixed->dim[1]);
    demons_global_work_size[2] = shrRoundUp((int)demons_local_work_size[2], fixed->dim[2]);
    reduction_local_work_size = BLOCK_SIZE;

    /*
      Calculate Moving Gradient
    */

    /* Calculate half pixel spacing */
    pix_spacing_div2.x = (float)(0.5 / moving->pix_spacing[0]);
    pix_spacing_div2.y = (float)(0.5 / moving->pix_spacing[1]);
    pix_spacing_div2.z = (float)(0.5 / moving->pix_spacing[2]);

    /* Calculate dynamic size of memory buffers */
    vol_size = moving->dim[0] * moving->dim[1] * moving->dim[2] * sizeof(float);
    interleaved_vol_size = 3 * fixed->dim[0] * fixed->dim[1] * fixed->dim[2] * sizeof(float);
    inlier_size = moving->dim[0] * moving->dim[1] * moving->dim[2] * sizeof(int);

    /* Determine image memory parameters */
    size_t vol_origin[3] = {0, 0, 0};
    size_t vol_region[3] = {moving->dim[0], moving->dim[1], moving->dim[2]};
    vol_row_pitch = moving->dim[0] * sizeof(float);
    vol_slice_pitch = moving->dim[0] * moving->dim[1] * sizeof(float);
    texture_format.image_channel_order = CL_R;
    texture_format.image_channel_data_type = CL_FLOAT;

    /* Create volume memory buffers on each device */
    g_moving_grad_x = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, vol_size, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    g_moving_grad_y = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, vol_size, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    g_moving_grad_z = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, vol_size, NULL, &error);
    oclCheckError(error, CL_SUCCESS);

    /* Create texture/image memory buffers on each device */
    t_fixed_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &texture_format, fixed->dim[0], fixed->dim[1], fixed->dim[2], 0, 0, fixed->img, &error);
    oclCheckError(error, CL_SUCCESS);
    t_moving_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &texture_format, moving->dim[0], moving->dim[1], moving->dim[2], 0, 0, moving->img, &error);
    oclCheckError(error, CL_SUCCESS);

    /* Create constant memory buffers on each device */
    c_dim = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float4), fixed->dim, &error);
    oclCheckError(error, CL_SUCCESS);
    c_moving_dim = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float4), moving->dim, &error);
    oclCheckError(error, CL_SUCCESS);
    c_pix_spacing_div2 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float4), &pix_spacing_div2, &error);
    oclCheckError(error, CL_SUCCESS);

    /* Create volume calc grad kernel on each device */
    moving_gradient_kernel = clCreateKernel(program, "volume_calc_grad_kernel", &error);
    oclCheckError(error, CL_SUCCESS);

    /* Wait for command queue to finish */
    clFinish(command_queue);

    /* Initialize timing totals */
    volume_calc_grad_total = 0;
    moving_grad_mag_total = 0;
    estimate_total = 0;
    reduction_total = 0;
    convolve_total = 0;
    global_total = 0;
    copy_total = 0;

    /* Set kernel arguments */
    error |= clSetKernelArg(moving_gradient_kernel, 0, sizeof(cl_mem), (void *) &g_moving_grad_x);
    error |= clSetKernelArg(moving_gradient_kernel, 1, sizeof(cl_mem), (void *) &g_moving_grad_y);
    error |= clSetKernelArg(moving_gradient_kernel, 2, sizeof(cl_mem), (void *) &g_moving_grad_z);
    error |= clSetKernelArg(moving_gradient_kernel, 3, sizeof(cl_mem), (void *) &t_moving_img);
    error |= clSetKernelArg(moving_gradient_kernel, 4, sizeof(cl_mem), (void *) &c_dim);
    error |= clSetKernelArg(moving_gradient_kernel, 5, sizeof(cl_mem), (void *) &c_pix_spacing_div2);
    oclCheckError(error, CL_SUCCESS);

    /* Wait for command queue to finish */
    clFinish(command_queue);

    /* Invoke all kernels */
    error = clEnqueueNDRangeKernel(command_queue, moving_gradient_kernel, 3, NULL, demons_global_work_size, demons_local_work_size, 0, NULL, &volume_calc_grad_event);
    oclCheckError(error, CL_SUCCESS);

    /* Wait for kernel to finish */
    clFinish(command_queue);

    /* Calculate kernel runtime */
    volume_calc_grad_total += opencl_timer (volume_calc_grad_event);

    /* Create volume memory buffer on each device */
    g_moving_grad_mag = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, vol_size, NULL, &error);
    oclCheckError(error, CL_SUCCESS);

    /* Create texture/image memory buffer on each device */
    t_moving_grad_x_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &texture_format, moving->dim[0], moving->dim[1], moving->dim[2], 0, 0, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    t_moving_grad_y_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &texture_format, moving->dim[0], moving->dim[1], moving->dim[2], 0, 0, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    t_moving_grad_z_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &texture_format, moving->dim[0], moving->dim[1], moving->dim[2], 0, 0, NULL, &error);

    /* Copy from global memory buffer to texture/image memory buffer */
    error = clEnqueueCopyBufferToImage(command_queue, g_moving_grad_x, t_moving_grad_x_img, 0, vol_origin, vol_region, 0, NULL, &copy_x_event);
    error |= clEnqueueCopyBufferToImage(command_queue, g_moving_grad_y, t_moving_grad_y_img, 0, vol_origin, vol_region, 0, NULL, &copy_y_event);
    error |= clEnqueueCopyBufferToImage(command_queue, g_moving_grad_z, t_moving_grad_z_img, 0, vol_origin, vol_region, 0, NULL, &copy_z_event);
    oclCheckError(error, CL_SUCCESS);

    /* Release unneeded global memory buffers */
    clReleaseMemObject(g_moving_grad_z);
    clReleaseMemObject(g_moving_grad_y);
    clReleaseMemObject(g_moving_grad_x);

    /* Create the calculate gradient magnitude image kernel on each device */
    gradient_magnitude_kernel = clCreateKernel(program, "calculate_gradient_magnitude_image_kernel", &error);
    oclCheckError(error, CL_SUCCESS);

    /* Wait for command queue to finish */
    clFinish(command_queue);

    /* Calculate global/image copy runtime */
    copy_total += opencl_timer (copy_x_event);
    copy_total += opencl_timer (copy_y_event);
    copy_total += opencl_timer (copy_z_event);

    /*
      Create gradient magnitude image 
    */

    /* Set kernel arguments */
    error |= clSetKernelArg(gradient_magnitude_kernel, 0, sizeof(cl_mem), (void *) &g_moving_grad_mag);
    error |= clSetKernelArg(gradient_magnitude_kernel, 1, sizeof(cl_mem), (void *) &t_moving_grad_x_img);
    error |= clSetKernelArg(gradient_magnitude_kernel, 2, sizeof(cl_mem), (void *) &t_moving_grad_y_img);
    error |= clSetKernelArg(gradient_magnitude_kernel, 3, sizeof(cl_mem), (void *) &t_moving_grad_z_img);
    error |= clSetKernelArg(gradient_magnitude_kernel, 4, sizeof(cl_mem), (void *) &c_dim);
    oclCheckError(error, CL_SUCCESS);

    /* Wait for command queue to finish */
    clFinish(command_queue);

    /* Invoke all kernels */
    error = clEnqueueNDRangeKernel(command_queue, gradient_magnitude_kernel, 3, NULL, demons_global_work_size, demons_local_work_size, 0, NULL, &moving_grad_mag_event);
    oclCheckError(error, CL_SUCCESS);

    /* Wait for kernel to finish */
    clFinish(command_queue);

    /* Calculate kernel runtime */
    moving_grad_mag_total += opencl_timer (moving_grad_mag_event);

    /* Create texture memory buffer on each device */
    t_moving_grad_mag_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &texture_format, moving->dim[0], moving->dim[1], moving->dim[2], 0, 0, NULL, &error);
    oclCheckError(error, CL_SUCCESS);

    /* Copy from global memory buffer to image memory buffer */
    error = clEnqueueCopyBufferToImage(command_queue, g_moving_grad_mag, t_moving_grad_mag_img, 0, vol_origin, vol_region, 0, NULL, &copy_x_event);
    oclCheckError(error, CL_SUCCESS);

    /* Release unneeded global memory buffers */
    clReleaseMemObject(g_moving_grad_mag);

    /* Create the estimate and convolve kernels on each device */
    estimate_kernel = clCreateKernel(program, "estimate_kernel", &error);
    oclCheckError(error, CL_SUCCESS);
    reduction_float_kernel = clCreateKernel(program, "reduction_float_kernel", &error);
    oclCheckError(error, CL_SUCCESS);
    reduction_int_kernel = clCreateKernel(program, "reduction_int_kernel", &error);
    oclCheckError(error, CL_SUCCESS);
    convolve_x_kernel = clCreateKernel(program, "vf_convolve_x_kernel", &error);
    oclCheckError(error, CL_SUCCESS);
    convolve_y_kernel = clCreateKernel(program, "vf_convolve_y_kernel", &error);
    oclCheckError(error, CL_SUCCESS);
    convolve_z_kernel = clCreateKernel(program, "vf_convolve_z_kernel", &error);
    oclCheckError(error, CL_SUCCESS);

    /* Calculate global/image copy runtime */
    copy_total += opencl_timer (copy_x_event);

    /* Validate filter widths */
    validate_filter_widths (fw, parms->filter_width);

    /* Create the seperable smoothing kernels for the x, y, and z directions */
    kerx = create_ker (parms->filter_std / fixed->pix_spacing[0], fw[0]/2);
    kery = create_ker (parms->filter_std / fixed->pix_spacing[1], fw[1]/2);
    kerz = create_ker (parms->filter_std / fixed->pix_spacing[2], fw[2]/2);
    kernel_stats (kerx, kery, kerz, fw);

    /* Calculate width of smoothing kernels */
    int kerx_size = sizeof(float) * ((fw[0] / 2) * 2 + 1);
    int kery_size = sizeof(float) * ((fw[1] / 2) * 2 + 1);
    int kerz_size = sizeof(float) * ((fw[2] / 2) * 2 + 1);

    /* Compute some variables for converting pixel sizes / offsets */
    for (int i = 0; i < 3; i++) {
	invmps[i] = 1 / moving->pix_spacing[i];
	f2mo[i] = (fixed->offset[i] - moving->offset[i]) / moving->pix_spacing[i];
	f2ms[i] = fixed->pix_spacing[i] / moving->pix_spacing[i];
    }

    /* Initialize memory and split interleaved volume to linear */
    vf_x = (float*)malloc(vol_size);
    vf_y = (float*)malloc(vol_size);
    vf_z = (float*)malloc(vol_size);

    for (int i = 0; i < vf_smooth->npix; i++) {
	vf_x[i] = vf_est_img[3 * i];
	vf_y[i] = vf_est_img[3 * i + 1];
	vf_z[i] = vf_est_img[3 * i + 2];
    }

    /* Allocate and set zero arrays for ssd and inliers */
    ssd_null = (float*)malloc(vol_size);
    memset(ssd_null, 0, vol_size);
    inliers_null = (int*)malloc(inlier_size);
    memset(inliers_null, 0, inlier_size);

    /* Create global memory buffer on each device */
    g_vf_est_x_img = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, vol_size, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    g_vf_est_y_img = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, vol_size, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    g_vf_est_z_img = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, vol_size, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    g_vf_smooth_x_img = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vol_size, vf_x, &error);
    oclCheckError(error, CL_SUCCESS);
    g_vf_smooth_y_img = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vol_size, vf_x, &error);
    oclCheckError(error, CL_SUCCESS);
    g_vf_smooth_z_img = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, vol_size, vf_x, &error);
    oclCheckError(error, CL_SUCCESS);
    g_ssd = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, vol_size, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    g_inliers = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, inlier_size, NULL, &error);
    oclCheckError(error, CL_SUCCESS);

    /* Create texture memory buffer on each device */
    t_vf_est_x_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &texture_format, fixed->dim[0], fixed->dim[1], fixed->dim[2], 0, 0, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    t_vf_est_y_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &texture_format, fixed->dim[0], fixed->dim[1], fixed->dim[2], 0, 0, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    t_vf_est_z_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &texture_format, fixed->dim[0], fixed->dim[1], fixed->dim[2], 0, 0, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    t_vf_smooth_x_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &texture_format, fixed->dim[0], fixed->dim[1], fixed->dim[2], 0, 0, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    t_vf_smooth_y_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &texture_format, fixed->dim[0], fixed->dim[1], fixed->dim[2], 0, 0, NULL, &error);
    oclCheckError(error, CL_SUCCESS);
    t_vf_smooth_z_img = clCreateImage3D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &texture_format, fixed->dim[0], fixed->dim[1], fixed->dim[2], 0, 0, NULL, &error);
    oclCheckError(error, CL_SUCCESS);

    /* Create constant memory buffer on each device */
    c_f2mo = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float4), f2mo, &error);
    oclCheckError(error, CL_SUCCESS);
    c_f2ms = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float4), f2ms, &error);
    oclCheckError(error, CL_SUCCESS);
    c_invmps = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float4), invmps, &error);
    oclCheckError(error, CL_SUCCESS);
    c_kerx = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, kerx_size, kerx, &error);
    oclCheckError(error, CL_SUCCESS);
    c_kery = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, kery_size, kery, &error);
    oclCheckError(error, CL_SUCCESS);
    c_kerz = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, kerz_size, kerz, &error);
    oclCheckError(error, CL_SUCCESS);

    int half_width_x = fw[0] / 2;
    int half_width_y = fw[1] / 2;
    int half_width_z = fw[2] / 2;

    timer->start ();

    /* Set kernel arguments */
    error |= clSetKernelArg(estimate_kernel, 0, sizeof(cl_mem), (void *) &g_vf_est_x_img);
    error |= clSetKernelArg(estimate_kernel, 1, sizeof(cl_mem), (void *) &g_vf_est_y_img);
    error |= clSetKernelArg(estimate_kernel, 2, sizeof(cl_mem), (void *) &g_vf_est_z_img);
    error |= clSetKernelArg(estimate_kernel, 3, sizeof(cl_mem), (void *) &t_vf_smooth_x_img);
    error |= clSetKernelArg(estimate_kernel, 4, sizeof(cl_mem), (void *) &t_vf_smooth_y_img);
    error |= clSetKernelArg(estimate_kernel, 5, sizeof(cl_mem), (void *) &t_vf_smooth_z_img);
    error |= clSetKernelArg(estimate_kernel, 6, sizeof(cl_mem), (void *) &t_fixed_img);
    error |= clSetKernelArg(estimate_kernel, 7, sizeof(cl_mem), (void *) &t_moving_img);
    error |= clSetKernelArg(estimate_kernel, 8, sizeof(cl_mem), (void *) &t_moving_grad_mag_img);
    error |= clSetKernelArg(estimate_kernel, 9, sizeof(cl_mem), (void *) &t_moving_grad_x_img);
    error |= clSetKernelArg(estimate_kernel, 10, sizeof(cl_mem), (void *) &t_moving_grad_y_img);
    error |= clSetKernelArg(estimate_kernel, 11, sizeof(cl_mem), (void *) &t_moving_grad_z_img);
    error |= clSetKernelArg(estimate_kernel, 12, sizeof(cl_mem), (void *) &g_ssd);
    error |= clSetKernelArg(estimate_kernel, 13, sizeof(cl_mem), (void *) &g_inliers);
    error |= clSetKernelArg(estimate_kernel, 14, sizeof(cl_mem), (void *) &c_dim);
    error |= clSetKernelArg(estimate_kernel, 15, sizeof(cl_mem), (void *) &c_moving_dim);
    error |= clSetKernelArg(estimate_kernel, 16, sizeof(cl_mem), (void *) &c_f2mo);
    error |= clSetKernelArg(estimate_kernel, 17, sizeof(cl_mem), (void *) &c_f2ms);
    error |= clSetKernelArg(estimate_kernel, 18, sizeof(cl_mem), (void *) &c_invmps);
    error |= clSetKernelArg(estimate_kernel, 19, sizeof(cl_float), &parms->homog);
    error |= clSetKernelArg(estimate_kernel, 20, sizeof(cl_float), &parms->denominator_eps);
    error |= clSetKernelArg(estimate_kernel, 21, sizeof(cl_float), &parms->accel);
    oclCheckError(error, CL_SUCCESS);

    /* Set kernel arguments */
    error |= clSetKernelArg(reduction_float_kernel, 0, sizeof(cl_mem), (void *) &g_ssd);
    error |= clSetKernelArg(reduction_float_kernel, 1, sizeof(cl_int), &num_elements);
    error |= clSetKernelArg(reduction_float_kernel, 2, sizeof(cl_float) * BLOCK_SIZE * 2, NULL);
    oclCheckError(error, CL_SUCCESS);

    /* Set kernel arguments */
    error |= clSetKernelArg(reduction_int_kernel, 0, sizeof(cl_mem), (void *) &g_inliers);
    error |= clSetKernelArg(reduction_int_kernel, 1, sizeof(cl_int), &num_elements);
    error |= clSetKernelArg(reduction_int_kernel, 2, sizeof(cl_int) * BLOCK_SIZE * 2, NULL);
    oclCheckError(error, CL_SUCCESS);

    /* Set kernel arguments */
    error |= clSetKernelArg(convolve_x_kernel, 0, sizeof(cl_mem), (void *) &g_vf_smooth_x_img);
    error |= clSetKernelArg(convolve_x_kernel, 1, sizeof(cl_mem), (void *) &g_vf_smooth_y_img);
    error |= clSetKernelArg(convolve_x_kernel, 2, sizeof(cl_mem), (void *) &g_vf_smooth_z_img);
    error |= clSetKernelArg(convolve_x_kernel, 3, sizeof(cl_mem), (void *) &t_vf_est_x_img);
    error |= clSetKernelArg(convolve_x_kernel, 4, sizeof(cl_mem), (void *) &t_vf_est_y_img);
    error |= clSetKernelArg(convolve_x_kernel, 5, sizeof(cl_mem), (void *) &t_vf_est_z_img);
    error |= clSetKernelArg(convolve_x_kernel, 6, sizeof(cl_mem), (void *) &c_kerx);
    error |= clSetKernelArg(convolve_x_kernel, 7, sizeof(cl_mem), (void *) &c_dim);
    error |= clSetKernelArg(convolve_x_kernel, 8, sizeof(cl_int), &half_width_x);
    oclCheckError(error, CL_SUCCESS);

    /* Set kernel arguments */
    error |= clSetKernelArg(convolve_y_kernel, 0, sizeof(cl_mem), (void *) &g_vf_est_x_img);
    error |= clSetKernelArg(convolve_y_kernel, 1, sizeof(cl_mem), (void *) &g_vf_est_y_img);
    error |= clSetKernelArg(convolve_y_kernel, 2, sizeof(cl_mem), (void *) &g_vf_est_z_img);
    error |= clSetKernelArg(convolve_y_kernel, 3, sizeof(cl_mem), (void *) &t_vf_smooth_x_img);
    error |= clSetKernelArg(convolve_y_kernel, 4, sizeof(cl_mem), (void *) &t_vf_smooth_y_img);
    error |= clSetKernelArg(convolve_y_kernel, 5, sizeof(cl_mem), (void *) &t_vf_smooth_z_img);
    error |= clSetKernelArg(convolve_y_kernel, 6, sizeof(cl_mem), (void *) &c_kery);
    error |= clSetKernelArg(convolve_y_kernel, 7, sizeof(cl_mem), (void *) &c_dim);
    error |= clSetKernelArg(convolve_y_kernel, 8, sizeof(cl_int), &half_width_y);
    oclCheckError(error, CL_SUCCESS);

    /* Set kernel arguments */
    error |= clSetKernelArg(convolve_z_kernel, 0, sizeof(cl_mem), (void *) &g_vf_smooth_x_img);
    error |= clSetKernelArg(convolve_z_kernel, 1, sizeof(cl_mem), (void *) &g_vf_smooth_y_img);
    error |= clSetKernelArg(convolve_z_kernel, 2, sizeof(cl_mem), (void *) &g_vf_smooth_z_img);
    error |= clSetKernelArg(convolve_z_kernel, 3, sizeof(cl_mem), (void *) &t_vf_est_x_img);
    error |= clSetKernelArg(convolve_z_kernel, 4, sizeof(cl_mem), (void *) &t_vf_est_y_img);
    error |= clSetKernelArg(convolve_z_kernel, 5, sizeof(cl_mem), (void *) &t_vf_est_z_img);
    error |= clSetKernelArg(convolve_z_kernel, 6, sizeof(cl_mem), (void *) &c_kerz);
    error |= clSetKernelArg(convolve_z_kernel, 7, sizeof(cl_mem), (void *) &c_dim);
    error |= clSetKernelArg(convolve_z_kernel, 8, sizeof(cl_int), &half_width_z);
    oclCheckError(error, CL_SUCCESS);

    /*
      Main loop through iterations
    */
    for (it = 0; it < parms->max_its; it++) {
	/*
	  Estimate displacement, store into vf_est
	*/
	inliers = 0; ssd = 0.0;

	/* Clear out sdd and inlier global memory buffers */
	error = clEnqueueWriteBuffer(command_queue, g_ssd, CL_FALSE, 0, vol_size, ssd_null, 0, NULL, NULL);
	error |= clEnqueueWriteBuffer(command_queue, g_inliers, CL_FALSE, 0, inlier_size, inliers_null, 0, NULL, NULL);
	oclCheckError(error, CL_SUCCESS);

	/* Copy old smooth memory buffer to estimate memory buffer */
	error = clEnqueueCopyBuffer(command_queue, g_vf_smooth_x_img, g_vf_est_x_img, 0, 0, vol_size, 0, NULL, &global_x_event);
	error |= clEnqueueCopyBuffer(command_queue, g_vf_smooth_y_img, g_vf_est_y_img, 0, 0, vol_size, 0, NULL, &global_y_event);
	error |= clEnqueueCopyBuffer(command_queue, g_vf_smooth_z_img, g_vf_est_z_img, 0, 0, vol_size, 0, NULL, &global_z_event);
	oclCheckError(error, CL_SUCCESS);

	/* Copy from global memory buffer to image memory buffer */
	error = clEnqueueCopyBufferToImage(command_queue, g_vf_smooth_x_img, t_vf_smooth_x_img, 0, vol_origin, vol_region, 0, NULL, &copy_x_event);
	error |= clEnqueueCopyBufferToImage(command_queue, g_vf_smooth_y_img, t_vf_smooth_y_img, 0, vol_origin, vol_region, 0, NULL, &copy_y_event);
	error |= clEnqueueCopyBufferToImage(command_queue, g_vf_smooth_z_img, t_vf_smooth_z_img, 0, vol_origin, vol_region, 0, NULL, &copy_z_event);
	oclCheckError(error, CL_SUCCESS);

	/* Wait for all memory events to finish */
	clFinish(command_queue);

	/* Calculate memory runtimes */
	global_total += opencl_timer (global_x_event);
	global_total += opencl_timer (global_x_event);
	global_total += opencl_timer (global_x_event);
	copy_total += opencl_timer (copy_x_event);
	copy_total += opencl_timer (copy_x_event);
	copy_total += opencl_timer (copy_x_event);

	/* Wait for command queue to finish */
	clFinish(command_queue);

	/* Invoke all kernels */
	error = clEnqueueNDRangeKernel(command_queue, estimate_kernel, 3, NULL, demons_global_work_size, demons_local_work_size, 0, NULL, &estimate_event);
	oclCheckError(error, CL_SUCCESS);

	/* Wait for kernel to finish */
	clFinish(command_queue);

	/* Calculate kernel runtime */
	estimate_total += opencl_timer (estimate_event);

	num_elements = moving->dim[0] * moving->dim[1] * moving->dim[2];
	while (num_elements > 1) {
	    error |= clSetKernelArg(reduction_float_kernel, 1, sizeof(cl_int), &num_elements);
	    error |= clSetKernelArg(reduction_int_kernel, 1, sizeof(cl_int), &num_elements);

	    reduction_global_work_size = ((num_elements + (2 * BLOCK_SIZE) - 1) / (BLOCK_SIZE * 2)) * BLOCK_SIZE;

	    /* Invoke all kernels */
	    error = clEnqueueNDRangeKernel(command_queue, reduction_int_kernel, 1, NULL, &reduction_global_work_size, &reduction_local_work_size, 0, NULL, &reduction_event);
	    oclCheckError(error, CL_SUCCESS);

	    /* Wait for kernel to finish */
	    clFinish(command_queue);

	    /* Calculate kernel runtime */
	    reduction_total += opencl_timer (reduction_event);

	    /* Invoke all kernels */
	    error = clEnqueueNDRangeKernel(command_queue, reduction_float_kernel, 1, NULL, &reduction_global_work_size, &reduction_local_work_size, 0, NULL, &reduction_event);
	    oclCheckError(error, CL_SUCCESS);

	    /* Wait for kernel to finish */
	    clFinish(command_queue);

	    /* Calculate kernel runtime */
	    reduction_total += opencl_timer (reduction_event);

	    num_elements = (num_elements + (2 * BLOCK_SIZE) - 1) / (BLOCK_SIZE * 2);
	}

	/* Copy global memory buffers to host on each device */
	error = clEnqueueReadBuffer(command_queue, g_ssd, CL_FALSE, 0, sizeof(float), &ssd, 0, NULL, NULL);
	error |= clEnqueueReadBuffer(command_queue, g_inliers, CL_FALSE, 0, sizeof(int), &inliers, 0, NULL, NULL);
	oclCheckError(error, CL_SUCCESS);

	/* Wait for command queue to finish */
	clFinish(command_queue);

	/* Print estimate statistics */
	printf ("----- SSD = %.01f (%d/%d)\n", ssd/inliers, inliers, fixed->npix);


	/*
	  Smooth the estimate into vf_smooth.  The volumes are ping-ponged.
	*/

	/* Copy from global memory buffer to image memory buffer */
	error = clEnqueueCopyBufferToImage(command_queue, g_vf_est_x_img, t_vf_est_x_img, 0, vol_origin, vol_region, 0, NULL, &copy_x_event);
	error |= clEnqueueCopyBufferToImage(command_queue, g_vf_est_y_img, t_vf_est_y_img, 0, vol_origin, vol_region, 0, NULL, &copy_y_event);
	error |= clEnqueueCopyBufferToImage(command_queue, g_vf_est_z_img, t_vf_est_z_img, 0, vol_origin, vol_region, 0, NULL, &copy_z_event);
	oclCheckError(error, CL_SUCCESS);

	/* Wait for all memory events to finish */
	clFinish(command_queue);

	/* Calculate memory runtimes */
	copy_total += opencl_timer (copy_x_event);
	copy_total += opencl_timer (copy_x_event);
	copy_total += opencl_timer (copy_x_event);

	/* Wait for command queue to finish */
	clFinish(command_queue);

	/* Invoke all kernels */
	error = clEnqueueNDRangeKernel(command_queue, convolve_x_kernel, 3, NULL, demons_global_work_size, demons_local_work_size, 0, NULL, &convolve_event);
	oclCheckError(error, CL_SUCCESS);

	/* Wait for kernel to finish */
	clFinish(command_queue);

	/* Calculate kernel runtime */
	convolve_total += opencl_timer (convolve_event);

	/* Copy from global memory buffer to image memory buffer */
	error = clEnqueueCopyBufferToImage(command_queue, g_vf_smooth_x_img, t_vf_smooth_x_img, 0, vol_origin, vol_region, 0, NULL, &copy_x_event);
	error |= clEnqueueCopyBufferToImage(command_queue, g_vf_smooth_y_img, t_vf_smooth_y_img, 0, vol_origin, vol_region, 0, NULL, &copy_y_event);
	error |= clEnqueueCopyBufferToImage(command_queue, g_vf_smooth_z_img, t_vf_smooth_z_img, 0, vol_origin, vol_region, 0, NULL, &copy_z_event);
	oclCheckError(error, CL_SUCCESS);

	/* Wait for all memory events to finish */
	clFinish(command_queue);

	/* Calculate memory runtimes */
	copy_total += opencl_timer (copy_x_event);
	copy_total += opencl_timer (copy_x_event);
	copy_total += opencl_timer (copy_x_event);

	/* Wait for command queue to finish */
	clFinish(command_queue);

	/* Invoke all kernels */
	error = clEnqueueNDRangeKernel(command_queue, convolve_y_kernel, 3, NULL, demons_global_work_size, demons_local_work_size, 0, NULL, &convolve_event);
	oclCheckError(error, CL_SUCCESS);

	/* Wait for kernel to finish */
	clFinish(command_queue);

	/* Calculate kernel runtime */
	convolve_total += opencl_timer (convolve_event);

	/* Create volume buffer */
	error = clEnqueueCopyBufferToImage(command_queue, g_vf_est_x_img, t_vf_est_x_img, 0, vol_origin, vol_region, 0, NULL, &copy_x_event);
	error |= clEnqueueCopyBufferToImage(command_queue, g_vf_est_y_img, t_vf_est_y_img, 0, vol_origin, vol_region, 0, NULL, &copy_y_event);
	error |= clEnqueueCopyBufferToImage(command_queue, g_vf_est_z_img, t_vf_est_z_img, 0, vol_origin, vol_region, 0, NULL, &copy_z_event);
	oclCheckError(error, CL_SUCCESS);

	/* Wait for all memory events to finish */
	clFinish(command_queue);

	/* Calculate memory runtimes */
	copy_total += opencl_timer (copy_x_event);
	copy_total += opencl_timer (copy_x_event);
	copy_total += opencl_timer (copy_x_event);

	/* Wait for command queue to finish */
	clFinish(command_queue);

	/* Invoke all kernels */
	error = clEnqueueNDRangeKernel(command_queue, convolve_z_kernel, 3, NULL, demons_global_work_size, demons_local_work_size, 0, NULL, &convolve_event);
	oclCheckError(error, CL_SUCCESS);

	/* Wait for kernel to finish */
	clFinish(command_queue);

	/* Calculate kernel runtime */
	convolve_total += opencl_timer (convolve_event);
    }

    /* Copy global memory buffers to host on each device */
    error = clEnqueueReadBuffer(command_queue, g_vf_smooth_x_img, CL_FALSE, 0, vol_size, vf_x, 0, NULL, &global_x_event);
    error = clEnqueueReadBuffer(command_queue, g_vf_smooth_y_img, CL_FALSE, 0, vol_size, vf_y, 0, NULL, &global_y_event);
    error = clEnqueueReadBuffer(command_queue, g_vf_smooth_z_img, CL_FALSE, 0, vol_size, vf_z, 0, NULL, &global_z_event);
    oclCheckError(error, CL_SUCCESS);

    /* Wait for all memory events to finish */
    clFinish(command_queue);

    /* Calculate memory runtimes */
    global_total += opencl_timer (global_x_event);
    global_total += opencl_timer (global_y_event);
    global_total += opencl_timer (global_z_event);

    /* Combine linear memory to interleaved  */
    for (int i = 0; i < vf_smooth->npix; i++) {
	vf_smooth_img[3 * i] = vf_x[i];
	vf_smooth_img[3 * i + 1] = vf_y[i];
	vf_smooth_img[3 * i + 2] = vf_z[i];
    }

    free (kerx);
    free (kery);
    free (kerz);
    delete vf_est;

    /***************************************************************/

    /**************************************************************** 
     * STEP 3: Print statistics										* 
     ****************************************************************/

    diff_run = timer->report ();
    printf ("Time for %d iterations = %f (%f sec / it)\n", parms->max_its, diff_run, diff_run / parms->max_its);

    shrLog("\n");
    overall_runtime = 0;
    estimate_kernel_runtime = estimate_total * 1.0e-6f;
    convolve_kernel = convolve_total * 1.0e-6f;
    other_kernels = (volume_calc_grad_total + moving_grad_mag_total + reduction_total) * 1.0e-6f;
    global_copy_runtime = global_total * 1.0e-6f;
    image_copy_runtime = copy_total * 1.0e-6f;
    total_runtime = (estimate_kernel_runtime + convolve_kernel + other_kernels + global_copy_runtime + image_copy_runtime) * 1.0e-3f;
    overall_runtime += total_runtime;

    shrLog("Estimate kernel run time: %f ms\n", estimate_kernel_runtime);
    shrLog("Convolve kernels run time: %f ms\n", convolve_kernel);
    shrLog("Other kernels run time: %f ms\n", other_kernels);
    shrLog("Global memory copy run time: %f ms\n", global_copy_runtime);
    shrLog("Image memory copy run time: %f ms\n", image_copy_runtime);
    shrLog("Total OpenCL run time: %f s\n\n", total_runtime);

    /***************************************************************/

    /**************************************************************** 
     * STEP 4: Cleanup OpenCL and finish								* 
     ****************************************************************/

    /* Release kernels */
    clReleaseKernel(convolve_z_kernel);
    clReleaseKernel(convolve_y_kernel);
    clReleaseKernel(convolve_x_kernel);
    clReleaseKernel(estimate_kernel);
    clReleaseKernel(gradient_magnitude_kernel);
    clReleaseKernel(moving_gradient_kernel);

    /* Release constant memory buffers */
    clReleaseMemObject(c_kerz);
    clReleaseMemObject(c_kery);
    clReleaseMemObject(c_kerx);
    clReleaseMemObject(c_invmps);
    clReleaseMemObject(c_f2ms);
    clReleaseMemObject(c_f2mo);
    clReleaseMemObject(c_moving_dim);
    clReleaseMemObject(c_pix_spacing_div2);
    clReleaseMemObject(c_dim);

    /* Release texture/image memory buffers */
    clReleaseMemObject(t_vf_smooth_z_img);
    clReleaseMemObject(t_vf_smooth_y_img);
    clReleaseMemObject(t_vf_smooth_x_img);
    clReleaseMemObject(t_vf_est_z_img);
    clReleaseMemObject(t_vf_est_y_img);
    clReleaseMemObject(t_vf_est_x_img);
    clReleaseMemObject(t_moving_grad_mag_img);
    clReleaseMemObject(t_moving_grad_z_img);
    clReleaseMemObject(t_moving_grad_y_img);
    clReleaseMemObject(t_moving_grad_x_img);
    clReleaseMemObject(t_moving_img);
    clReleaseMemObject(t_fixed_img);

    /* Release global memory buffers */
    clReleaseMemObject(g_inliers);
    clReleaseMemObject(g_ssd);
    clReleaseMemObject(g_vf_smooth_z_img);
    clReleaseMemObject(g_vf_smooth_y_img);
    clReleaseMemObject(g_vf_smooth_x_img);
    clReleaseMemObject(g_vf_est_z_img);
    clReleaseMemObject(g_vf_est_y_img);
    clReleaseMemObject(g_vf_est_x_img);

    /* Cleanup OpenCL */
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    shrLog("Done Demons OpenCL...\n\n");
	
    delete timer;

    /****************************************************************/

    return vf_smooth;
#endif

    return (Volume*) 0;
}
