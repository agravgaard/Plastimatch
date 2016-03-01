/*******************************************************************
c* Multimodal Deformable Image Registration						   *
c* via Mutual Information or Bhattacharyya Distantce               *
c* Version: 1.0                                                    *
c* Language: C, CUDA                                               *
c*                                                                 *
c* Developer: Yifei Lou                                            *
c* Email: yifei.lou@ece.gatech.edu                                 *
c*                                                                 *
c* School of Electrical and Computer Engineering                   *   
c* Georgia Institute of Technology                                 *
c* Atlanta, GA, 30318                                              *
c* Website: http://groups.bme.gatech.edu/groups/bil/               *
c*                                                                 *
c* Copyright (c) 2011                                              *
c* All rights reserved.                                            *
c*                                                                 *
c* Permission to use, copy, or modify this code and its            *
c* documentation for scientific purpose is hereby granted          *
c* without fee, provided that this copyright notice appear in      *
c* all copies and that both that copyright notice and this         *
c* permission notice appear in supporting documentation. The use   *
c* for commercial purposes is prohibited without permission.       *
c*                                                                 *
c* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND          *
c* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,     *
c* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        *
c* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        *
c* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR            *
c* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    *
c* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES INCLUDING, BUT NOT *
c* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF*
c* USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED *
c* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT     *
c* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING  *
c* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF  *
c* THE POSSIBILITY OF SUCH DAMAGE.                                 *
c*                                                                 *
c******************************************************************/

/*******************************************************************
c* Short discription                                               *
c*   main function to register two images on the current scale     *
c*	 including upsample and downsample                             *
c******************************************************************/

#include <thrust/binary_search.h>
#include <thrust/device_vector.h>
#include <thrust/functional.h>
#include <thrust/sort.h>
#include <cublas.h>
#include <cutil.h>
#include <cutil_inline.h>
#include "viscous_convolution.h"
#include "viscous_global.h"

// hash a point in the unit square to the index of
// the grid bucket that contains it
struct point_to_bucket_index : public thrust::unary_function<float2,unsigned int>
{
 	__host__ __device__
  	point_to_bucket_index(unsigned int width, unsigned int height)
    		:w(width),h(height){}

  	__host__ __device__
  	unsigned int operator()(float2 p) const
  	{
// find the raster indices of p's bucket
    		unsigned int x = static_cast<unsigned int>(p.x * (w-1));
    		unsigned int y = static_cast<unsigned int>(p.y * (h-1));

// return the bucket's linear index
    		return y * w + x;
  	}

  	unsigned int w, h;
};

__global__ void downSample(float *src, float *dest, int NX, int NY, int NZ, int s)
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;

        if(tid < NX*NY*NZ)
	{
		int z = tid/(NX*NY);
		int y = (tid%(NX*NY))/NX;
		int x = tid%NX;

		float sum =0.0f;
		for(int xs = 0; xs<s; xs++)	
			for(int ys =0; ys<s; ys++)
				sum += src[s*x+xs + (s*y+ys)*NX0 + s*z*NX0*NY0];
		dest[tid] = sum/s/s;
	}


}

__global__ void upSample(float *src, float *dest, int NX, int NY, int NZ)
//      upsampling
{
const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;

        if(tid < NX*NY*NZ)
	{
		int z = tid/(NX*NY);
		int y = (tid%(NX*NY))/NX;
		int x = tid%NX;
		
		int xmin = x/2 - (x%2 == 0);
		int xmax = x/2 + (x%2 == 1);
		int ymin = y/2 - (y%2 == 0);
		int ymax = y/2 + (y%2 == 1);
		int zmin = z/2 - (z%2 == 0);
		int zmax = z/2 + (z%2 == 1);

		xmin = (xmin < 0) ? 0: xmin;
		ymin = (ymin < 0) ? 0: ymin;
		zmin = (zmin < 0) ? 0: zmin;

		xmax = (xmax < NX)? xmax : NX-1;
		ymax = (ymax < NY)? ymax : NY-1;
		zmax = (zmax < NZ)? zmax : NZ-1; 
		
		float wx = 0.25 + 0.5*(x%2==0);
                float wy = 0.25 + 0.5*(y%2==0);
		float wz = 0.25 + 0.5*(z%2==0);
		
		dest[tid] = 	src[xmin + ymin*NX/2 + zmin*NX/2*NY/2] * (1.0 - wx) * (1.0-wy) * (1.0-wz) +
			    	src[xmax + ymin*NX/2 + zmin*NX/2*NY/2] * wx * (1.0-wy) * (1.0-wz) + 
				src[xmin + ymax*NX/2 + zmin*NX/2*NY/2] * (1.0 - wx) * wy * (1.0-wz) +
				src[xmax + ymax*NX/2 + zmin*NX/2*NY/2] * wx * wy * (1.0-wz) +
				src[xmin + ymin*NX/2 + zmax*NX/2*NY/2] * (1.0 - wx) * (1.0-wy) * wz +
			    	src[xmax + ymin*NX/2 + zmax*NX/2*NY/2] * wx * (1.0-wy) * wz + 
				src[xmin + ymax*NX/2 + zmax*NX/2*NY/2] * (1.0 - wx) * wy * wz +
				src[xmax + ymax*NX/2 + zmax*NX/2*NY/2] * wx * wy * wz;
		dest[tid] = 2*dest[tid];
	
	}

}


void compute(float *d_im_move, float *d_im_static, float *d_mv_x, float *d_mv_y, float *d_mv_z, int maxIter)
//	d_mv_x, d_mv_y and d_im_move are updated
{

//	bind moving image to texture	
	const cudaExtent volumeSize = make_cudaExtent(NX, NY, NZ);    	
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float>();
	cutilSafeCall( cudaMalloc3DArray(&d_im_move_array, &channelDesc, volumeSize) );
    	cudaMemcpy3DParms copyParams = {0};
    	copyParams.srcPtr   = make_cudaPitchedPtr((void*)d_im_move, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    	copyParams.dstArray = d_im_move_array;
    	copyParams.extent   = volumeSize;
    	copyParams.kind     = cudaMemcpyDeviceToDevice;
    	cutilSafeCall( cudaMemcpy3D(&copyParams) );


	d_im_move_tex.normalized = false;                
    	d_im_move_tex.filterMode = cudaFilterModeLinear;  

    
    	cutilSafeCall(cudaBindTextureToArray(d_im_move_tex, d_im_move_array, channelDesc));


// 	bind vector flows to texture
	cutilSafeCall( cudaMalloc3DArray(&d_mv_x_array, &channelDesc, volumeSize) );
	cudaMemcpy3DParms copyParams_x = {0};
    	copyParams_x.srcPtr   = make_cudaPitchedPtr((void*)d_mv_x, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    	copyParams_x.dstArray = d_mv_x_array;
    	copyParams_x.extent   = volumeSize;
    	copyParams_x.kind     = cudaMemcpyDeviceToDevice;
    	cutilSafeCall( cudaMemcpy3D(&copyParams_x) );
	d_mv_x_tex.normalized = false;
	d_mv_x_tex.filterMode = cudaFilterModeLinear;


	cutilSafeCall( cudaMalloc3DArray(&d_mv_y_array, &channelDesc, volumeSize) );
	cudaMemcpy3DParms copyParams_y = {0};
    	copyParams_y.srcPtr   = make_cudaPitchedPtr((void*)d_mv_y, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    	copyParams_y.dstArray = d_mv_y_array;
    	copyParams_y.extent   = volumeSize;
    	copyParams_y.kind     = cudaMemcpyDeviceToDevice;
    	cutilSafeCall( cudaMemcpy3D(&copyParams_y) );
	d_mv_y_tex.normalized = false;
	d_mv_y_tex.filterMode = cudaFilterModeLinear;

	cutilSafeCall( cudaMalloc3DArray(&d_mv_z_array, &channelDesc, volumeSize) );
	cudaMemcpy3DParms copyParams_z = {0};
    	copyParams_z.srcPtr   = make_cudaPitchedPtr((void*)d_mv_z, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    	copyParams_z.dstArray = d_mv_z_array;
    	copyParams_z.extent   = volumeSize;
    	copyParams_z.kind     = cudaMemcpyDeviceToDevice;
    	cutilSafeCall( cudaMemcpy3D(&copyParams_z) );
	d_mv_z_tex.normalized = false;
	d_mv_z_tex.filterMode = cudaFilterModeLinear;
	
	float *d_im_out;
	cutilSafeCall( cudaMalloc((void **)&d_im_out, sDATA_SIZE) );



//	velocity
	float *d_v_x, *d_v_x_copy;
	float *d_v_y, *d_v_y_copy;
	float *d_v_z, *d_v_z_copy;
	cutilSafeCall( cudaMalloc((void **)&d_v_x, sDATA_SIZE) );
	cutilSafeCall( cudaMalloc((void **)&d_v_y, sDATA_SIZE) );
	cutilSafeCall( cudaMalloc((void **)&d_v_z, sDATA_SIZE) );
	cutilSafeCall( cudaMalloc((void **)&d_v_x_copy, sDATA_SIZE) );
	cutilSafeCall( cudaMalloc((void **)&d_v_y_copy, sDATA_SIZE) );
	cutilSafeCall( cudaMalloc((void **)&d_v_z_copy, sDATA_SIZE) );


//	setup for computing joint histogram via thrust
// 		the grid data structure keeps a range per grid bucket:
// 		each bucket_begin[i] indexes the first element of bucket i's list of points
// 		each bucket_end[i] indexes one past the last element of bucket i's list of points
  	thrust::device_vector<unsigned int> bucket_begin(nBin*nBin);
  	thrust::device_vector<unsigned int> bucket_end(nBin*nBin);

// 		allocate storage for each point's bucket index
  	thrust::device_vector<unsigned int> bucket_indices(NX*NY*NZ);

// 		allocate space to hold per-bucket sizes
        thrust::device_vector<unsigned int> bucket_sizes(nBin*nBin);

//		allocate float2 vector
	float2 *d_points;
	cudaMalloc((void**) &d_points, sizeof(float2)*NX*NY*NZ);



	int regrid = 0;
	float MI[1000];

	int3   Dims;
	Dims.x = NX;
	Dims.y = NY;
	Dims.z = NZ;



		for(int it=0; it<maxIter; it++)
	{
// 	upate image
		ImageWarp<<<nblocks, NTHREAD_PER_BLOCK>>>(d_mv_x, d_mv_y, d_mv_z, d_im_out, NX, NY, NZ);	

		
//	joint histogram via thrust ----- begin
//		convert to float2 vector
		transToFloat2<<<nblocks, NTHREAD_PER_BLOCK>>>(d_im_out, d_im_static, d_points, NX*NY*NZ);

//		use a thrust ptr to wrap the raw pointer
		thrust::device_ptr<float2> points_t(d_points);

// 		transform the points to their bucket indices
        	thrust::transform(points_t, points_t+NX*NY*NZ, bucket_indices.begin(), point_to_bucket_index(nBin,nBin));

//		sort the bucket index
        	thrust::sort(bucket_indices.begin(), bucket_indices.end());

// 		find the beginning of each bucket's list of points
        	thrust::counting_iterator<unsigned int> search_begin(0);
        	thrust::lower_bound(bucket_indices.begin(), bucket_indices.end(), search_begin,
                      search_begin + nBin*nBin, bucket_begin.begin());

// 		find the end of each bucket's list of points
        	thrust::upper_bound(bucket_indices.begin(), bucket_indices.end(), search_begin,
                      search_begin + nBin*nBin, bucket_end.begin());

//		take the difference between bounds to find each bucket size
       		thrust::transform(bucket_end.begin(), bucket_end.end(), bucket_begin.begin(),
                bucket_sizes.begin(), thrust :: minus<unsigned int>());

//		now hist contains the histogram
		unsigned int *hist = thrust::raw_pointer_cast(&bucket_sizes[0]);

		copyHist<<<nblocks_hist, NTHREAD_PER_BLOCK>>>(hist, d_jointHistogram);
// 	joint histogram via thrust ----- end


			
//	compute the convolution of joint histogram
		myconv2dGPU<<<nblocks_hist, NTHREAD_PER_BLOCK>>>(d_jointHistogram, d_jointHistogram_conv, GaussKernelH, nBin, nBin, 3*hValue);
	
		
//	normalize joint histogram
		float sum = cublasSasum (nBin*nBin, d_jointHistogram_conv , 1);	
		cublasSscal (nBin*nBin, 1.0f/sum, d_jointHistogram_conv, 1);
		

//	compute mutual info by GPU
		marginalDist<<<nBin, nBin>>>(d_jointHistogram_conv, d_probx, d_proby);

		switch (METHOD)
		{
			case 1:
				marginalBnorm_sum<<<nblocks_hist, NTHREAD_PER_BLOCK>>>(d_jointHistogram_conv, d_probx, d_proby, d_jointHistogram); 
				marginalDistAlongY<<<nBin, nBin>>>(d_jointHistogram, d_Bsum);
				BnormGPU<<<nblocks_hist, NTHREAD_PER_BLOCK>>>(d_jointHistogram_conv, d_probx, d_proby,d_Bsum, d_jointHistogram); 
				break;
			case 2: 
				mutualInfoGPU<<<nblocks_hist, NTHREAD_PER_BLOCK>>>(d_jointHistogram_conv, d_probx, d_proby, d_jointHistogram); 
				break;
		}
		MI[it] = cublasSasum (nBin*nBin, d_jointHistogram_conv, 1);	
		printf("mutual information (%d)= %f\n", it, MI[it]);


//	NOTE: after this step, jointHistogram becomes the likelihood
//	compute the first derivative w.r.t. x-dim of joint histogram	
		myconv2dGPU<<<nblocks_hist, NTHREAD_PER_BLOCK>>>(d_jointHistogram, d_jointHistogram_conv, GaussKernelHx, nBin, nBin,3*hValue);

// 	compute the force		
		forceComp<<<nblocks, NTHREAD_PER_BLOCK>>>(d_im_out, d_im_static, d_jointHistogram_conv, d_v_x, d_v_y, d_v_z, NX, NY, NZ);

		ImageSmooth(d_v_x, d_v_x_copy,Dims);
		ImageSmooth(d_v_y, d_v_y_copy,Dims);
		ImageSmooth(d_v_z, d_v_z_copy,Dims);
		
		flowComp<<<nblocks, NTHREAD_PER_BLOCK>>>(d_mv_x, d_mv_y, d_mv_z, d_v_x_copy, d_v_y_copy, d_v_z_copy, d_v_x, d_v_y, NX, NY, NZ);
// 	NOTE: d_v_x is Jacobian, d_v_y is the max flow
//		d_v_x_copy, d_v_y_copy, d_v_z_copy are the displacement

		thrust :: device_ptr<float> data_ptr(d_v_y);
		int maxInd = cublasIsamax(NX*NY*NZ, d_v_y, 1) -1;
		float maxflow = data_ptr[maxInd];
		float dt = (du/maxflow); // > 1) ? 1 : du/maxflow;
		printf("dt = %f \n", dt);

		flowUpdate<<<nblocks, NTHREAD_PER_BLOCK>>>(d_mv_x, d_mv_y, d_mv_z, d_v_x_copy, d_v_y_copy, d_v_z_copy,dt, NX, NY, NZ);

// 	regridding if Jacobian < threshJaco		
		sum = cublasSasum(NX*NY*NZ, d_v_x, 1);
		if (sum>0.5)
		{
			regrid ++;
			
			printf("regrid = %d\n", regrid);
//	save d_im_move to be d_im_out

			cudaUnbindTexture(d_im_move_tex);
			cudaMemcpy3DParms copyParams = {0};
    			copyParams.srcPtr   = make_cudaPitchedPtr((void*)d_im_out, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    			copyParams.dstArray = d_im_move_array;
    			copyParams.extent   = volumeSize;
    			copyParams.kind     = cudaMemcpyDeviceToDevice;
    			cutilSafeCall( cudaMemcpy3D(&copyParams) );
    			cutilSafeCall(cudaBindTextureToArray(d_im_move_tex, d_im_move_array));

		
//	update vector flow
			ImageWarp_mv<<<nblocks, NTHREAD_PER_BLOCK>>>(d_mv_x, d_mv_y, d_mv_z, NX, NY, NZ);	
			
			cudaMemcpy3DParms copyParams_x = {0};
    			copyParams_x.srcPtr   = make_cudaPitchedPtr((void*)d_mv_x, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    			copyParams_x.dstArray = d_mv_x_array;
    			copyParams_x.extent   = volumeSize;
    			copyParams_x.kind     = cudaMemcpyDeviceToDevice;
    			cutilSafeCall( cudaMemcpy3D(&copyParams_x) );
			cutilSafeCall(cudaBindTextureToArray(d_mv_x_tex, d_mv_x_array));

			cudaMemcpy3DParms copyParams_y = {0};
    			copyParams_y.srcPtr   = make_cudaPitchedPtr((void*)d_mv_y, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    			copyParams_y.dstArray = d_mv_y_array;
    			copyParams_y.extent   = volumeSize;
    			copyParams_y.kind     = cudaMemcpyDeviceToDevice;
    			cutilSafeCall( cudaMemcpy3D(&copyParams_y) );
			cutilSafeCall(cudaBindTextureToArray(d_mv_y_tex, d_mv_y_array));

			cudaMemcpy3DParms copyParams_z = {0};
    			copyParams_z.srcPtr   = make_cudaPitchedPtr((void*)d_mv_z, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    			copyParams_z.dstArray = d_mv_z_array;
    			copyParams_z.extent   = volumeSize;
    			copyParams_z.kind     = cudaMemcpyDeviceToDevice;
    			cutilSafeCall( cudaMemcpy3D(&copyParams_z) );
			cutilSafeCall(cudaBindTextureToArray(d_mv_z_tex, d_mv_z_array));
	

			cutilSafeCall( cudaMemset(d_mv_x, 0, sDATA_SIZE) );
			cutilSafeCall( cudaMemset(d_mv_y, 0, sDATA_SIZE) );
			cutilSafeCall( cudaMemset(d_mv_z, 0, sDATA_SIZE) );


			
			
		} // end for regridding
	

		

	} // for-loop iteration


	if (!regrid)
	{
		ImageWarp<<<nblocks, NTHREAD_PER_BLOCK>>>(d_mv_x, d_mv_y, d_mv_z, d_im_move, NX, NY, NZ);			
	}	
	else
	{
		cudaMemcpy3DParms copyParams = {0};		
		cudaUnbindTexture(d_im_move_tex);
		copyParams.srcPtr   = make_cudaPitchedPtr((void*)d_im_move, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    		copyParams.dstArray = d_im_move_array;
    		copyParams.extent   = volumeSize;
    		copyParams.kind     = cudaMemcpyDeviceToDevice;
    		cutilSafeCall( cudaMemcpy3D(&copyParams) );
    		cutilSafeCall(cudaBindTextureToArray(d_im_move_tex, d_im_move_array));

		ImageWarp_final<<<nblocks, NTHREAD_PER_BLOCK>>>(d_mv_x, d_mv_y, d_mv_z,d_im_move, NX, NY, NZ);
		

	}

	

	cudaFree(d_points);
	
	

	cudaFree(d_v_x);
	cudaFree(d_v_y);
	cudaFree(d_v_z);
	cudaFree(d_v_x_copy);
	cudaFree(d_v_y_copy);
	cudaFree(d_v_z_copy);	


	cudaUnbindTexture(d_im_move_tex);
	cudaFreeArray(d_im_move_array);

	cudaUnbindTexture(d_mv_x_tex);
	cudaFreeArray(d_mv_x_array);

	cudaUnbindTexture(d_mv_y_tex);
	cudaFreeArray(d_mv_y_array);

	cudaUnbindTexture(d_mv_z_tex);
	cudaFreeArray(d_mv_z_array);

	cudaFree(d_im_out);


}


__global__ void transToFloat2(const float *input1, const float *input2, float2 *output, const int n)
{
	 const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;
//      obtain current id on thread

        if (tid < n)
        {
		output[tid] = make_float2(input1[tid], input2[tid]);
	}
	
}
