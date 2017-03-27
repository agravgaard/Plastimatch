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
c*   initialization of the entire program including				   *
c*   data preprocessing, construction of image pyramid             *
c*   and Gaussian smoothing kernels                                *
c******************************************************************/

#include <iostream>
#include <thrust/device_ptr.h>
#include <cublas.h>
#include <cutil.h>
#include <cutil_inline.h>
#include "viscous_convolution.h"
#include "viscous_global.h"

void dataPreprocessing(float *image, float *maxValue, float *minValue)
{
	thrust :: device_ptr<float> data_ptr(image);
	int maxInd = cublasIsamax(NX0*NY0*NZ0, image, 1) -1;
	int minInd = cublasIsamin(NX0*NY0*NZ0, image, 1) -1;
	*maxValue = data_ptr[maxInd];
	*minValue = data_ptr[minInd];

	nblocks.x = NBLOCKX;
	nblocks.y =  ((1 + (NX0*NY0*NZ0 - 1)/NTHREAD_PER_BLOCK) - 1) / NBLOCKX + 1; 

	intensityRescale<<<nblocks, NTHREAD_PER_BLOCK>>>(image, *maxValue, *minValue, 1);

}


__global__ void intensityRescale(float *image, float maxValue, float minValue, int type)
//	type > 0: forward
//	type < 0: backward
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;

        if(tid < NX0*NY0*NZ0)
	{	
		if(type>0)		
			image[tid] = (image[tid] - minValue)/(maxValue-minValue);
		else
			image[tid] = image[tid]*(maxValue-minValue) + minValue;
	
	}
}

__global__ void short2float(short *raw, float *image, int type)
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;

        if(tid < NX0*NY0*NZ0)
	{	
		if (type>0)		
			image[tid] = (float) raw[tid];
		else
			raw[tid] = (short) image[tid];
	}
	
}

void initData()
{

// 	load static and moving images on host
	float *h_im_static = (float*)malloc(DATA_SIZE);
	loadData(h_im_static, DATA_SIZE, inputfilename_static);
	float *h_im_move = (float *)malloc(DATA_SIZE);
	loadData(h_im_move, DATA_SIZE, inputfilename_move);	


//	create image pyramid on device
//		finest level 0
	cutilSafeCall( cudaMalloc((void**) &d_im_move[0], DATA_SIZE ) );
	cutilSafeCall( cudaMalloc((void**) &d_im_static[0],DATA_SIZE) );

	cutilSafeCall( cudaMemcpy(d_im_static[0], h_im_static, DATA_SIZE, cudaMemcpyHostToDevice) );
	cutilSafeCall( cudaMemcpy(d_im_move[0], h_im_move, DATA_SIZE, cudaMemcpyHostToDevice) );

	
	free(h_im_static);
	free(h_im_move);

//	scale intensity to [0, 1]
	dataPreprocessing(d_im_static[0],&max_im_move, &min_im_move);
	dataPreprocessing(d_im_move[0], &max_im_move, &min_im_move);


	
//		vector flow
	cutilSafeCall( cudaMalloc((void **)&d_mv_x[0], DATA_SIZE) );
	cutilSafeCall( cudaMalloc((void **)&d_mv_y[0], DATA_SIZE) );
	cutilSafeCall( cudaMalloc((void **)&d_mv_z[0], DATA_SIZE) );

	cutilSafeCall( cudaMemset(d_mv_x[0], 0, DATA_SIZE) );
	cutilSafeCall( cudaMemset(d_mv_y[0], 0, DATA_SIZE) );
	cutilSafeCall( cudaMemset(d_mv_z[0], 0, DATA_SIZE) );


//		coarse levels
	for(int scale = 1; scale < NSCALE; scale ++)
	{
		NX = NX0/pow(2, scale);
		NY = NY0/pow(2, scale);
		NZ = (NZ0-1)/pow(2, scale) +1;

		sDATA_SIZE = sizeof(float)*NX*NY*NZ;

		cutilSafeCall( cudaMalloc((void**) &d_im_move[scale],   sDATA_SIZE ) );
		cutilSafeCall( cudaMalloc((void**) &d_im_static[scale], sDATA_SIZE ) );

		nblocks.x = NBLOCKX;
        	nblocks.y =  ((1 + (NX*NY*NZ - 1)/NTHREAD_PER_BLOCK) - 1) / NBLOCKX + 1; 

		int s = pow(2, scale);
	
		downSample<<<nblocks, NTHREAD_PER_BLOCK>>>(d_im_move[0], d_im_move[scale], NX, NY, NZ, s);
		downSample<<<nblocks, NTHREAD_PER_BLOCK>>>(d_im_static[0], d_im_static[scale],NX, NY, NZ, s);

		cutilSafeCall( cudaMalloc((void **)&d_mv_x[scale], sDATA_SIZE ) );
		cutilSafeCall( cudaMalloc((void **)&d_mv_y[scale], sDATA_SIZE ) );
		cutilSafeCall( cudaMalloc((void **)&d_mv_z[scale], sDATA_SIZE ) );

		cutilSafeCall( cudaMemset(d_mv_x[scale], 0, sDATA_SIZE ) );
		cutilSafeCall( cudaMemset(d_mv_y[scale], 0, sDATA_SIZE ) );
		cutilSafeCall( cudaMemset(d_mv_z[scale], 0, sDATA_SIZE ) );
		

	}
	
	

        std::cout << "Load data successfully.\n\n";


//	allocate space for histogram related variables
	cutilSafeCall( cudaMalloc(&d_jointHistogram, HIST_SIZE) );
	cutilSafeCall( cudaMalloc(&d_jointHistogram_conv, HIST_SIZE) );
	cutilSafeCall( cudaMalloc((void **)&d_probx, sizeof(float)*nBin) );
	cutilSafeCall( cudaMalloc((void **)&d_proby, sizeof(float)*nBin) );
	cutilSafeCall( cudaMalloc((void **)&d_Bsum,sizeof(float)*nBin ) );
	
}





void initGaussKernel()
{
    std::cout <<	"Initializing Gaussian kernel..." << std::endl;
	float *h_GaussKernelH  = (float *)malloc(sizeof(float)*(6*hValue+1)*(6*hValue+1));
	float *h_GaussKernelHx = (float *)malloc(sizeof(float)*(6*hValue+1)*(6*hValue+1));
	cutilSafeCall( cudaMalloc(&GaussKernelH, sizeof(float)*(6*hValue+1)*(6*hValue+1) ) );
	cutilSafeCall( cudaMalloc(&GaussKernelHx, sizeof(float)*(6*hValue+1)*(6*hValue+1) ) );
	

	float sum = 0.0;
	for(int i = -3*hValue; i <= 3*hValue; i++)
	{
		for(int j = -3*hValue; j <= 3*hValue; j++)
		{
			
			float temp = expf(-(i*i+j*j)/2.0/hValue/hValue);
			h_GaussKernelH[(i+3*hValue)+(j+3*hValue)*(6*hValue+1)] = temp;
			sum += temp;

			h_GaussKernelHx[(i+3*hValue)+(j+3*hValue)*(6*hValue+1)] = -i*temp/hValue/hValue;
			
		}
	}
	for(int i = -3*hValue; i <= 3*hValue; i++)
	{
		for(int j = -3*hValue; j <= 3*hValue; j++)
		{
                	h_GaussKernelH[(i+3*hValue)+(j+3*hValue)*(6*hValue+1)] /= sum;
			h_GaussKernelHx[(i+3*hValue)+(j+3*hValue)*(6*hValue+1)] /= sum;		
                }
        }	

	cutilSafeCall( cudaMemcpy(GaussKernelH,  h_GaussKernelH,  sizeof(float)*(6*hValue+1)*(6*hValue+1), cudaMemcpyHostToDevice) );
	cutilSafeCall( cudaMemcpy(GaussKernelHx, h_GaussKernelHx, sizeof(float)*(6*hValue+1)*(6*hValue+1), cudaMemcpyHostToDevice) );
	
	

	
	
	free(h_GaussKernelH);
	free(h_GaussKernelHx);	

	float *h_GaussKernel1D = (float *)malloc(sizeof(float)*KERNEL_W);
	float sumK = 0.0;
        for(int i = 0; i < KERNEL_W; i++)
	{
            	h_GaussKernel1D[i] = expf(-(i-KERNEL_RADIUS)*(i-KERNEL_RADIUS)/2.0/sValue/sValue);
		sumK += h_GaussKernel1D[i];
	}
	
	
	for(int i = 0; i < KERNEL_W; i++)
		h_GaussKernel1D[i] /= sumK;

	cudaMemcpyToSymbol(d_Kernel, h_GaussKernel1D, KERNEL_W * sizeof(float));

	
	free(h_GaussKernel1D);

}

void loadData(float *dest, int sizeInByte, const char *filename)
//	load data
{
	FILE *fp = fopen(filename,"rb");
        if( fp == NULL )
        {
                printf("   File \'%s\' could not be opened!\n", filename);
                exit(1);
        }

        else{
                printf("Reading File \'%s\' ... \n", filename);

                size_t temp = fread(dest, 1, sizeInByte, fp);
                fclose(fp);
        }
        
}
