/* 
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

Title : OpenCL implementation of FDK Algorithm

Authors : Atapattu, Chatura
Houck, Dustin
O'Brien, Ronald
Partel, Michael

Description : This is an OpenCL implementation of the FDK
algorithm used to convert 2D cone-beam images
into a 3D volumetric image

Created : 11/01/2009

Modified : ongoing

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
*/

/*******************
*  C     #includes *
*******************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "plmbase.h"

/*******************
* OPENCL #includes *
*****************/
#include "opencl_util.h"

/*******************
* FDK    #includes *
*******************/
#include "autotune_opencl.h"
#include "fdk_opencl_p.h"
#include "fdk_opts.h"
#include "fdk_util.h"
#include "plm_math.h"
#include "opencl_util_nvidia.h"
#include "proj_image.h"
#include "proj_image_dir.h"
#include "volume.h"

#undef MAX_GPU_COUNT
#define MAX_GPU_COUNT 1

/*
* Summary:		This OpenCL stub function performs most of the IO and flow control.  
				The actual back projection is performed by the OpenCL kernel "fdk_kernel()"
* Parameters:	Pointer to volume and runtime options
* Return:		Float of runtime
*/
void OPENCL_reconstruct_conebeam_and_convert_to_hu (Volume *vol, Proj_image_dir *proj_dir, Fdk_options *options)
{
    int image_num, num_images, matrix_size, vol_size;
    float scale, overall_runtime, fdk_kernel_runtime, hu_kernel_runtime, img_copy_runtime, vol_copy_runtime, total_runtime;
    char device_name[MAX_GPU_COUNT][256];			/* Device names */
    Proj_image *cbi;
    kernel_args_fdk *kargs;

    /* Declare global memory */
    cl_mem g_dev_vol[MAX_GPU_COUNT];

    /* Declare image/texture memory */
    cl_mem t_dev_img[MAX_GPU_COUNT];

    /* Declare constant memory */
    cl_mem c_dev_matrix[MAX_GPU_COUNT];
    cl_mem c_nrm[MAX_GPU_COUNT];
    cl_mem c_vol_offset[MAX_GPU_COUNT];
    cl_mem c_vol_pix_spacing[MAX_GPU_COUNT];
    cl_mem c_vol_dim[MAX_GPU_COUNT];
    cl_mem c_ic[MAX_GPU_COUNT];
    cl_mem c_img_dim[MAX_GPU_COUNT];
    cl_mem c_sad[MAX_GPU_COUNT];
    cl_mem c_scale[MAX_GPU_COUNT];
    cl_mem c_voxel_device[MAX_GPU_COUNT];

    /* Declare OpenCL kernels */
    cl_kernel fdk_kernel[MAX_GPU_COUNT];
    cl_kernel hu_kernel[MAX_GPU_COUNT];

    /* Declare other OpenCL variables */
    cl_event fdk_event[MAX_GPU_COUNT], hu_event[MAX_GPU_COUNT], img_event[MAX_GPU_COUNT], vol_event[MAX_GPU_COUNT];
    cl_ulong fdk_total[MAX_GPU_COUNT], hu_total[MAX_GPU_COUNT], img_total[MAX_GPU_COUNT], vol_total[MAX_GPU_COUNT];
    size_t fdk_local_work_size[MAX_GPU_COUNT][3];
    size_t fdk_global_work_size[MAX_GPU_COUNT][3];
    size_t hu_local_work_size[MAX_GPU_COUNT];
    size_t hu_global_work_size[MAX_GPU_COUNT];
    cl_context context[MAX_GPU_COUNT];				/* Context from device */
    cl_command_queue command_queue[MAX_GPU_COUNT];	/* Command Queue from Context */
    cl_program program[MAX_GPU_COUNT];				/* Program from .cl file */
    cl_int error;
    cl_uint device_count;							/* Number of devices available */
    cl_device_id device;							/* Object for individual device in 'for loop' */
    cl_device_id *devices;							/* Pointer to devices */
    cl_platform_id platform;
    cl_image_format img_format;
    size_t program_length, img_row_pitch;
    size_t work_per_device[MAX_GPU_COUNT][3];
    size_t work_total[3] = {vol->dim[0], vol->dim[1], vol->dim[2]};
    int4 voxels_per_device[MAX_GPU_COUNT];
    int2 voxel_offset[MAX_GPU_COUNT];

    /**************************************************************** 
     * STEP 1: Set global variables and algorithm parameters			* 
     ****************************************************************/

    /* Set logfile name and start logs */
    shrSetLogFileName ("fdk_opencl.txt");

    shrLog("Starting FDK_OPENCL...\n\n"); 

    /* Structure for passing arugments to kernel: (See fdk_cuda.h) */
    kargs = (kernel_args_fdk *) malloc(sizeof(kernel_args_fdk));

    /* Calculate the scale */
    num_images = proj_dir->num_proj_images;
    image_num = 1 + (options->last_img - options->first_img) / options->skip_img;
    scale = (float)(sqrt(3.0)/(double)image_num);
    scale = scale * options->scale;

    /* Load static kernel arguments */
    kargs->scale = scale;
    kargs->vol_offset.x = vol->offset[0];
    kargs->vol_offset.y = vol->offset[1];
    kargs->vol_offset.z = vol->offset[2];
    kargs->vol_dim.x = vol->dim[0];
    kargs->vol_dim.y = vol->dim[1];
    kargs->vol_dim.z = vol->dim[2];
    kargs->vol_pix_spacing.x = vol->pix_spacing[0];
    kargs->vol_pix_spacing.y = vol->pix_spacing[1];
    kargs->vol_pix_spacing.z = vol->pix_spacing[2];

    /* Retrieve 2D image to get dimensions */
    cbi = proj_image_dir_load_image(proj_dir, 0);

    /* Verify that dimensions are within limits */
    if (cbi->dim[0] > CL_DEVICE_IMAGE2D_MAX_WIDTH || cbi->dim[1] > CL_DEVICE_IMAGE2D_MAX_HEIGHT) {
	shrLog("Image dimensions too large for %s\n", CL_DEVICE_NAME);
	shrLog("Maximum height and width: %d x %d\n", CL_DEVICE_IMAGE2D_MAX_HEIGHT, CL_DEVICE_IMAGE2D_MAX_WIDTH);
	shrLog("Image height and width: %d x %d\n", cbi->dim[1], cbi->dim[0]);
	shrLog("Exiting...\n\n");
	exit(-1);
    }

    /* Calculate dynamic size of memory buffers */
    vol_size = vol->npix * sizeof(float);
    matrix_size = 12 * sizeof(float);
    int img_dim[2] = {cbi->dim[0], cbi->dim[1]};

    /* Free cbi image */
    proj_image_destroy(cbi);

    /* Set parameters for texture/image memory */
    size_t img_origin[3] = {0, 0, 0};
    size_t img_region[3] = {img_dim[0], img_dim[1], 1};
    img_row_pitch = img_dim[0] * sizeof(float);
    img_format.image_channel_order = CL_R;
    img_format.image_channel_data_type = CL_FLOAT;

    /***************************************************************/

    /**************************************************************** 
     * STEP 2: Setup OpenCL											* 
     ****************************************************************/

    /* Get the OpenCL platform */
    error = oclGetPlatformID(&platform);
    oclCheckError(error, CL_SUCCESS);

    /* Get devices of type GPU */
    error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &device_count);
    oclCheckError(error, CL_SUCCESS);

    /* Make sure using no more than the maximum number of GPUs */
    if (device_count > MAX_GPU_COUNT)
	device_count = MAX_GPU_COUNT;

    devices = (cl_device_id *)malloc(device_count * sizeof(cl_device_id));
    error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, device_count, devices, NULL);
    oclCheckError(error, CL_SUCCESS);

    /* Create context properties */
    cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};

    /* Calculate number of voxels per device */
    divideWork(devices, device_count, 3, work_per_device, work_total);

    shrLog("Using %d device(s):\n", device_count);

    /* Create context and command queue for each device */
    for (cl_uint i = 0; i < device_count; i++) {
	/* Context */
	context[i] = clCreateContext(properties, 1, &devices[i], NULL, NULL, &error);
	oclCheckError(error, CL_SUCCESS);

	/* Device info */
	device = oclGetDev(context[i], 0);
	clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name[i]), device_name[i], NULL);
	oclCheckError(error, CL_SUCCESS);
	shrLog("\tDevice %d: %s handling %d x %d x %d voxels\n", i, device_name[i], work_per_device[i][0], work_per_device[i][1], work_per_device[i][2]);

	/* Command queue */
	command_queue[i] = clCreateCommandQueue(context[i], device, CL_QUEUE_PROFILING_ENABLE, &error);
	oclCheckError(error, CL_SUCCESS);
    }

    shrLog("\n%u voxels in volume\n", vol->npix);
    shrLog("%u projections to process\n", 1+(options->last_img - options->first_img) / options->skip_img);
    shrLog("%u total operations\n", vol->npix * (1+(options->last_img - options->first_img) / options->skip_img));
    shrLog("========================================\n\n");

    /* Program Setup */
    char* source_path = shrFindFilePath("fdk_opencl.cl", "");
    oclCheckError(source_path != NULL, shrTRUE);
    char *source = oclLoadProgSource(source_path, "", &program_length);
    oclCheckError(source != NULL, shrTRUE);

    /* Create the program */
    for (cl_uint i = 0; i < device_count; i++) {
	program[i] = clCreateProgramWithSource(context[i], 1, (const char **)&source, &program_length, &error);
	oclCheckError(error, CL_SUCCESS);

	/* Build the program */
	error = clBuildProgram(program[i], 0, NULL, NULL, NULL, NULL);
	if (error != CL_SUCCESS) {
	    /* Write out standard error, Build Log and PTX, then return error */
	    shrLogEx(LOGBOTH | ERRORMSG, error, STDERROR);
	    oclLogBuildInfo(program[i], oclGetFirstDev(context[i]));
	    oclLogPtx(program[i], oclGetFirstDev(context[i]), "fdk_opencl.ptx");
	}
    }

    /***************************************************************/

    /**************************************************************** 
     * STEP 2: Perform FDK algorithm on each device 					* 
     ****************************************************************/

    /* Allocate voxels to each device  */
    for (cl_uint i = 0; i < device_count; i++) {
	voxels_per_device[i].x = (int)work_per_device[i][0];
	voxels_per_device[i].y = (int)work_per_device[i][1];
	voxels_per_device[i].z = (int)work_per_device[i][2];
	voxels_per_device[i].w = voxels_per_device[i].x * voxels_per_device[i].y * voxels_per_device[i].z;
    }

    /* Determine voxel offset on each device */
    for (cl_uint i = 0; i < device_count; i++) {
	voxel_offset[i].x = 0;
	voxel_offset[i].y = 0;
	for (cl_uint j = 0; j < i; j++) {
	    voxel_offset[i].x += voxels_per_device[j].z;
	    voxel_offset[i].y += voxels_per_device[j].w;
	}
    }

    /* Calculate local and global work sizes on each device */
    for (cl_uint i = 0; i < device_count; i++) {
	fdk_local_work_size[i][0] = 512;
	fdk_local_work_size[i][1] = 1;
	fdk_local_work_size[i][2] = 1;
	fdk_global_work_size[i][0] = shrRoundUp((int)fdk_local_work_size[i][0], voxels_per_device[i].x);
	fdk_global_work_size[i][1] = shrRoundUp((int)fdk_local_work_size[i][1], voxels_per_device[i].y);
	fdk_global_work_size[i][2] = shrRoundUp((int)fdk_local_work_size[i][2], voxels_per_device[i].z);
	hu_local_work_size[i] = 512;
	hu_global_work_size[i] = shrRoundUp((int)hu_local_work_size[i], voxels_per_device[i].w);
    }

    for (cl_uint i = 0; i < device_count; i++) {
	/* Create volume buffer */
	g_dev_vol[i] = clCreateBuffer(context[i], CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, voxels_per_device[i].w * sizeof(float), NULL, &error);
	oclCheckError(error, CL_SUCCESS);

	/* Create texture/image memory buffers on device */
	t_dev_img[i] = clCreateImage2D(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &img_format, img_dim[0], img_dim[1], 0, NULL, &error);
	oclCheckError(error, CL_SUCCESS);

	/* Create constant memory buffers on device */
	c_dev_matrix[i] = clCreateBuffer(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, matrix_size, NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_nrm[i] = clCreateBuffer(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float4), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_vol_offset[i] = clCreateBuffer(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float4), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_vol_pix_spacing[i] = clCreateBuffer(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float4), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_vol_dim[i] = clCreateBuffer(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(int4), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_ic[i] = clCreateBuffer(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float2), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_img_dim[i] = clCreateBuffer(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(int2), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_sad[i] = clCreateBuffer(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_scale[i] = clCreateBuffer(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
	c_voxel_device[i] = clCreateBuffer(context[i], CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(int4), NULL, &error);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Wait for all queues to finish */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Creates the kernel objects */
    for (cl_uint i = 0; i < device_count; i++) {
	fdk_kernel[i] = clCreateKernel(program[i], "kernel_fdk", &error);
	//fdk_kernel[i] = clCreateKernel(program[i], "kernel_fdk_bilinear", &error);
	//fdk_kernel[i] = clCreateKernel(program[i], "kernel_fdk_bicubic", &error);
	oclCheckError(error, CL_SUCCESS);
	hu_kernel[i] = clCreateKernel(program[i], "convert_to_hu_cl", &error);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Initialize all timers */
    for (cl_uint i = 0; i < device_count; i++) {
	fdk_total[i] = 0;
	hu_total[i] = 0;
	img_total[i] = 0;
	vol_total[i] = 0;
    }

    /* Project each image into the volume one at a time */
    for (image_num = options->first_img; image_num < proj_dir->num_proj_images; image_num++) {

	/* Load the current image and properties */
	cbi = proj_image_dir_load_image(proj_dir, image_num);

	if (options->filter == FDK_FILTER_TYPE_RAMP)
	    proj_image_filter (cbi);

	/* Load dynamic kernel arguments */
	kargs->img_dim.x = cbi->dim[0];
	kargs->img_dim.y = cbi->dim[1];
	kargs->ic.x = cbi->pmat->ic[0];
	kargs->ic.y = cbi->pmat->ic[1];
	kargs->nrm.x = cbi->pmat->nrm[0];
	kargs->nrm.y = cbi->pmat->nrm[1];
	kargs->nrm.z = cbi->pmat->nrm[2];
	kargs->sad = cbi->pmat->sad;
	kargs->sid = cbi->pmat->sid;

	for (int j = 0; j < 12; j++)
	    kargs->matrix[j] = (float)cbi->pmat->matrix[j];

	/* Loop to copy data from host to each device */
	for (cl_uint i = 0; i < device_count; i++) {
	    /* Copy texture/image memory from host to device */
	    error = clEnqueueWriteImage(command_queue[i], t_dev_img[i], CL_FALSE, img_origin, img_region, img_row_pitch, 0, cbi->img, 0, NULL, &img_event[i]);
	    oclCheckError(error, CL_SUCCESS);

	    /* Copy constant memory from host to device */
	    error = clEnqueueWriteBuffer(command_queue[i], c_dev_matrix[i], CL_FALSE, 0, matrix_size, &kargs->matrix, 0, NULL, NULL);
	    error |= clEnqueueWriteBuffer(command_queue[i], c_nrm[i], CL_FALSE, 0, sizeof(float4), &kargs->nrm, 0, NULL, NULL);
	    error |= clEnqueueWriteBuffer(command_queue[i], c_vol_offset[i], CL_FALSE, 0, sizeof(float4), &kargs->vol_offset, 0, NULL, NULL);
	    error |= clEnqueueWriteBuffer(command_queue[i], c_vol_pix_spacing[i], CL_FALSE, 0, sizeof(float4), &kargs->vol_pix_spacing, 0, NULL, NULL);
	    error |= clEnqueueWriteBuffer(command_queue[i], c_vol_dim[i], CL_FALSE, 0, sizeof(int4), &kargs->vol_dim, 0, NULL, NULL);
	    error |= clEnqueueWriteBuffer(command_queue[i], c_ic[i], CL_FALSE, 0, sizeof(float2), &kargs->ic, 0, NULL, NULL);		
	    error |= clEnqueueWriteBuffer(command_queue[i], c_img_dim[i], CL_FALSE, 0, sizeof(int2), &kargs->img_dim, 0, NULL, NULL);
	    error |= clEnqueueWriteBuffer(command_queue[i], c_sad[i], CL_FALSE, 0, sizeof(float), &kargs->sad, 0, NULL, NULL);
	    error |= clEnqueueWriteBuffer(command_queue[i], c_scale[i], CL_FALSE, 0, sizeof(float), &kargs->scale, 0, NULL, NULL);
	    error |= clEnqueueWriteBuffer(command_queue[i], c_voxel_device[i], CL_FALSE, 0, sizeof(int4), &voxels_per_device[i], 0, NULL, NULL);
	    oclCheckError(error, CL_SUCCESS);
	}

	/* Wait for all queues to finish */
	for (cl_uint i = 0; i < device_count; i++)
	    clFinish(command_queue[i]);

	/* Count host to device time */
	for (cl_uint i = 0; i < device_count; i++)
	    img_total[i] += opencl_timer (img_event[i]);

	/* Set fdk kernel arguments */
	for (cl_uint i = 0; i < device_count; i++) {
	    error |= clSetKernelArg(fdk_kernel[i], 0, sizeof(cl_mem), (void *) &g_dev_vol[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 1, sizeof(cl_mem), (void *) &t_dev_img[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 2, sizeof(cl_mem), (void *) &c_dev_matrix[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 3, sizeof(cl_mem), (void *) &c_nrm[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 4, sizeof(cl_mem), (void *) &c_vol_offset[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 5, sizeof(cl_mem), (void *) &c_vol_pix_spacing[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 6, sizeof(cl_mem), (void *) &c_vol_dim[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 7, sizeof(cl_mem), (void *) &c_ic[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 8, sizeof(cl_mem), (void *) &c_img_dim[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 9, sizeof(cl_mem), (void *) &c_sad[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 10, sizeof(cl_mem), (void *) &c_scale[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 11, sizeof(cl_mem), (void *) &c_voxel_device[i]);
	    error |= clSetKernelArg(fdk_kernel[i], 12, sizeof(int), &voxel_offset[i].x);
	    oclCheckError(error, CL_SUCCESS);
	}

	/* Wait for all queues to finish */
	for (cl_uint i = 0; i < device_count; i++) {
	    clFinish(command_queue[i]);
	}

	/* Invoke all fdk kernels */
	for (cl_uint i = 0; i < device_count; i++) {
	    error = clEnqueueNDRangeKernel(command_queue[i], fdk_kernel[i], 3, NULL, fdk_global_work_size[i], fdk_local_work_size[i], 0, NULL, &fdk_event[i]);
	    oclCheckError(error, CL_SUCCESS);
	}

	/* Wait for fdk kernel to finish */
	for (cl_uint i = 0; i < device_count; i++) {
	    clFinish(command_queue[i]);
	}

	/* Count fdk kernel time */
	for (cl_uint i = 0; i < device_count; i++) {
	    fdk_total[i] += opencl_timer (fdk_event[i]);
	}

	/* Free the current image */
	proj_image_destroy(cbi);
    }

    /* Sets hu kernel arguments */
    for (cl_uint i = 0; i < device_count; i++) {
	error = clSetKernelArg(hu_kernel[i], 0, sizeof(cl_mem), (void *) &g_dev_vol[i]);
	error |= clSetKernelArg(hu_kernel[i], 1, sizeof(int), &voxels_per_device[i].w);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Wait for all queues to finish */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Invoke all hu kernels */
    for (cl_uint i = 0; i < device_count; i++) {
	error = clEnqueueNDRangeKernel(command_queue[i], hu_kernel[i], 1, NULL, &hu_global_work_size[i], &hu_local_work_size[i], 0, NULL, &hu_event[i]);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Waits for hu kernel to finish */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Count hu kernel time */
    for (cl_uint i = 0; i < device_count; i++)
	hu_total[i] += opencl_timer (hu_event[i]);

    /* Copy reconstructed volume from device to host */
    for (cl_uint i = 0; i < device_count; i++) {
	error = clEnqueueReadBuffer(command_queue[i], g_dev_vol[i], CL_FALSE, 0, voxels_per_device[i].w * sizeof(float), (float*)vol->img + voxel_offset[i].y, 0, NULL, &vol_event[i]);
	oclCheckError(error, CL_SUCCESS);
    }

    /* Waits for volume to finish copying */
    for (cl_uint i = 0; i < device_count; i++)
	clFinish(command_queue[i]);

    /* Count device to host time */
    for (cl_uint i = 0; i < device_count; i++)
	vol_total[i] += opencl_timer (vol_event[i]);

    for (cl_uint i = 0; i < device_count; i++) {
	/* Release kernels */
	clReleaseKernel(fdk_kernel[i]);
	clReleaseKernel(hu_kernel[i]);

	/* Release constant memory buffers */
	clReleaseMemObject(c_voxel_device[i]);
	clReleaseMemObject(c_scale[i]);
	clReleaseMemObject(c_sad[i]);
	clReleaseMemObject(c_img_dim[i]);
	clReleaseMemObject(c_ic[i]);
	clReleaseMemObject(c_vol_dim[i]);
	clReleaseMemObject(c_vol_pix_spacing[i]);
	clReleaseMemObject(c_vol_offset[i]);
	clReleaseMemObject(c_nrm[i]);
	clReleaseMemObject(c_dev_matrix[i]);

	/* Release texture/image memory buffers */
	clReleaseMemObject(t_dev_img[i]);

	/* Release global memory buffers */
	clReleaseMemObject(g_dev_vol[i]);	
    }

    /***************************************************************/

    /**************************************************************** 
     * STEP 3: Perform timing										* 
     ****************************************************************/

    overall_runtime = 0;
    for (cl_uint i = 0; i < device_count; i++) {
	fdk_kernel_runtime = fdk_total[i] * 1.0e-6f;
	hu_kernel_runtime = hu_total[i] * 1.0e-6f;
	img_copy_runtime = img_total[i] * 1.0e-6f;
	vol_copy_runtime = vol_total[i] * 1.0e-6f;
	total_runtime = (fdk_kernel_runtime + hu_kernel_runtime + img_copy_runtime + vol_copy_runtime) * 1.0e-3f;
	overall_runtime += total_runtime;

	shrLog("Device %d: %s\n", i, device_name[i]);
	shrLog("\tFDK Kernel run time: %f ms\n", fdk_kernel_runtime);
	shrLog("\tHu Kernel run time: %f ms\n", hu_kernel_runtime);
	shrLog("\tCBI host to device copy run time: %f ms\n", img_copy_runtime);
	shrLog("\tVolume device to host copy run time: %f ms\n", vol_copy_runtime);
	shrLog("\tTotal run time: %f s\n\n", total_runtime);
    }

    /***************************************************************/

    /**************************************************************** 
     * STEP 4: Cleanup OpenCL and finish								* 
     ****************************************************************/

    for (cl_uint i = 0; i < device_count; i++) {
	clReleaseProgram(program[i]);
	clReleaseCommandQueue(command_queue[i]);
	clReleaseContext(context[i]);
    }

    shrLog("Done FDK_OPENCL...\n\n");
    shrLog("Total OpenCL run time: %f s\n\n", overall_runtime);
}
