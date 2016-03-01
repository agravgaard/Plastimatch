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
c*   convolution related support functions                         *
c*	 in SDK as well as developped by Xuejun Gu and Yifei Lou   *
c******************************************************************/


/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
#include <cutil_inline.h>
#include "viscous_global.h"
#include "viscous_convolution.h"

////////////////////////////////////////////////////////////////////////////////
// Row convolution filter by Frame
////////////////////////////////////////////////////////////////////////////////
__global__ void convolutionRowGPU_byframe(
    float *d_Result,
    float *d_Data,
    int dataW,
	int dataH,
	int nF
){
    //Data cache
    __shared__ float data[KERNEL_RADIUS + ROW_TILE_W + KERNEL_RADIUS];

    //Current tile and apron limits, relative to row start
    const int         tileStart = IMUL(blockIdx.x, ROW_TILE_W);
    const int           tileEnd = tileStart + ROW_TILE_W - 1;
    const int        apronStart = tileStart - KERNEL_RADIUS;
    const int          apronEnd = tileEnd   + KERNEL_RADIUS;

    //Clamp tile and apron limits by image borders
    const int    tileEndClamped = min(tileEnd, dataW - 1);
    const int apronStartClamped = max(apronStart, 0);
    const int   apronEndClamped = min(apronEnd, dataW - 1);

    //Row start index in d_Data[]
    const int          rowStart = nF*dataW*dataH+IMUL(blockIdx.y, dataW);

    //Aligned apron start. Assuming dataW and ROW_TILE_W are multiples 
    //of half-warp size, rowStart + apronStartAligned is also a 
    //multiple of half-warp size, thus having proper alignment 
    //for coalesced d_Data[] read.
    const int apronStartAligned = tileStart - KERNEL_RADIUS_ALIGNED;

    const int loadPos = apronStartAligned + threadIdx.x;
    //Set the entire data cache contents
    //Load global memory values, if indices are within the image borders,
    //or initialize with zeroes otherwise
    if(loadPos >= apronStart){
        const int smemPos = loadPos - apronStart;

        data[smemPos] = 
            ((loadPos >= apronStartClamped) && (loadPos <= apronEndClamped)) ?
            d_Data[rowStart + loadPos] : 0;
    }


    //Ensure the completness of the loading stage
    //because results, emitted by each thread depend on the data,
    //loaded by another threads
    __syncthreads();
    const int writePos = tileStart + threadIdx.x;
    //Assuming dataW and ROW_TILE_W are multiples of half-warp size,
    //rowStart + tileStart is also a multiple of half-warp size,
    //thus having proper alignment for coalesced d_Result[] write.
    if(writePos <= tileEndClamped){
        const int smemPos = writePos - apronStart;
        float sum = 0;
        sum = convolutionRow<2 * KERNEL_RADIUS>(data + smemPos);

        d_Result[rowStart + writePos] = sum;
    }
}


////////////////////////////////////////////////////////////////////////////////
// Column convolution filter
////////////////////////////////////////////////////////////////////////////////
__global__ void convolutionColumnGPU(
    float *d_Result,
    float *d_Data,
    int dataW,
    int dataH,
    int smemStride,
    int gmemStride,
    int nF
){
    //Data cache
    __shared__ float data[COLUMN_TILE_W * (KERNEL_RADIUS + COLUMN_TILE_H + KERNEL_RADIUS)];

    //Current tile and apron limits, in rows
    const int         tileStart = IMUL(blockIdx.y, COLUMN_TILE_H);
    const int           tileEnd = tileStart + COLUMN_TILE_H - 1;
    const int        apronStart = tileStart - KERNEL_RADIUS;
    const int          apronEnd = tileEnd   + KERNEL_RADIUS;

    //Clamp tile and apron limits by image borders
    const int    tileEndClamped = min(tileEnd, dataH - 1);
    const int apronStartClamped = max(apronStart, 0);
    const int   apronEndClamped = min(apronEnd, dataH - 1);

    //Current column index
    const int       columnStart = IMUL(blockIdx.x, COLUMN_TILE_W) + threadIdx.x;

    //Shared and global memory indices for current column
    
    int smemPos = IMUL(threadIdx.y, COLUMN_TILE_W) + threadIdx.x;
    //int gmemPos = IMUL(apronStart + threadIdx.y, dataW) + columnStart;
    int gmemPos = IMUL(apronStart + threadIdx.y, dataW) + columnStart + nF *dataH *dataW;  // added by xuejun
    
    //Cycle through the entire data cache
    //Load global memory values, if indices are within the image borders,
    //or initialize with zero otherwise
    for(int y = apronStart + threadIdx.y; y <= apronEnd; y += blockDim.y){
        data[smemPos] = 
        ((y >= apronStartClamped) && (y <= apronEndClamped)) ? 
        d_Data[gmemPos] : 0;
        smemPos += smemStride;
        gmemPos += gmemStride;
    }

    //Ensure the completness of the loading stage
    //because results, emitted by each thread depend on the data, 
    //loaded by another threads
    __syncthreads();
    
    //Shared and global memory indices for current column
    smemPos = IMUL(threadIdx.y + KERNEL_RADIUS, COLUMN_TILE_W) + threadIdx.x;
    //gmemPos = IMUL(tileStart + threadIdx.y , dataW) + columnStart;
    gmemPos = IMUL(tileStart + threadIdx.y , dataW) + columnStart + nF *dataH *dataW;  // added by xuejun
    
    //Cycle through the tile body, clamped by image borders
    //Calculate and output the results
    for(int y = tileStart + threadIdx.y; y <= tileEndClamped; y += blockDim.y){
        float sum = 0;

        sum = convolutionColumn<2 * KERNEL_RADIUS>(data + smemPos);

        d_Result[gmemPos] = sum;
        smemPos += smemStride;
        gmemPos += gmemStride;
    }
}
////////////////////////////////////////////////////////////////////////////////
// Frame convolution filter
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Frame convolution filter
////////////////////////////////////////////////////////////////////////////////
__global__ void convolutionFrameGPU(
    float *d_Result,
    float *d_Data,
    int dataW,
    int dataH,
    int smemStride,
    int gmemStride
){
    //Data cache
    __shared__ float data[COLUMN_TILE_W * (KERNEL_RADIUS + COLUMN_TILE_H + KERNEL_RADIUS)];

    //Current tile and apron limits, in rows
    const int         tileStart = IMUL(blockIdx.y, COLUMN_TILE_H);
    const int           tileEnd = tileStart + COLUMN_TILE_H - 1;
    const int        apronStart = tileStart - KERNEL_RADIUS;
    const int          apronEnd = tileEnd   + KERNEL_RADIUS;

    //Clamp tile and apron limits by image borders
    const int    tileEndClamped = min(tileEnd, dataH - 1);
    const int apronStartClamped = max(apronStart, 0);
    const int   apronEndClamped = min(apronEnd, dataH - 1);

    //Current column index
    const int       columnStart = IMUL(blockIdx.x, COLUMN_TILE_W) + threadIdx.x;

    //Shared and global memory indices for current column
    int smemPos = IMUL(threadIdx.y, COLUMN_TILE_W) + threadIdx.x;
    int gmemPos = IMUL(apronStart + threadIdx.y, dataW) + columnStart;
    //Cycle through the entire data cache
    //Load global memory values, if indices are within the image borders,
    //or initialize with zero otherwise
    for(int y = apronStart + threadIdx.y; y <= apronEnd; y += blockDim.y){
        data[smemPos] = 
        ((y >= apronStartClamped) && (y <= apronEndClamped)) ? 
        d_Data[gmemPos] : 0;
        smemPos += smemStride;
        gmemPos += gmemStride;
    }

    //Ensure the completness of the loading stage
    //because results, emitted by each thread depend on the data, 
    //loaded by another threads
    __syncthreads();
    //Shared and global memory indices for current column
    smemPos = IMUL(threadIdx.y + KERNEL_RADIUS, COLUMN_TILE_W) + threadIdx.x;
    gmemPos = IMUL(tileStart + threadIdx.y , dataW) + columnStart;
    //Cycle through the tile body, clamped by image borders
    //Calculate and output the results
    for(int y = tileStart + threadIdx.y; y <= tileEndClamped; y += blockDim.y){
        float sum = 0;

        sum = convolutionColumn<2 * KERNEL_RADIUS>(data + smemPos);

        d_Result[gmemPos] = sum;
        smemPos += smemStride;
        gmemPos += gmemStride;
    }
}


  /**************************************************************************/
  /**********    Doing low pass filter on an image function     ****************/
  /**************************************************************************/


void ImageSmooth(float *d_image, float *d_image_conv, int3 Dims)
{
  
    
   //Dims: size of image

	int DATA_W = Dims.x, DATA_H = Dims.y, DATA_F = Dims.z;
	float *d_temp;
	int SDATA_SIZE = DATA_W*DATA_H*DATA_F*sizeof(float);
	
	cudaMalloc((void**)&d_temp, SDATA_SIZE);

	// row convolution:
	dim3 blockGridRows(iDivUp(DATA_W, ROW_TILE_W), DATA_H, 1);
	dim3 threadBlockRows(KERNEL_RADIUS_ALIGNED + ROW_TILE_W + KERNEL_RADIUS, 1);

	cudaMemset((void*)d_image_conv, 0, SDATA_SIZE);
	for (int nF = 0; nF < DATA_F; nF++){
		convolutionRowGPU_byframe<<<blockGridRows, threadBlockRows>>>(d_image_conv, d_image,DATA_W, DATA_H, nF);
		cutilCheckMsg("convolutionRowGPU() execution failed\n");
	}
    
	//column convolution
	dim3 blockGridColumns(iDivUp(DATA_W, COLUMN_TILE_W),iDivUp(DATA_H, COLUMN_TILE_H), 1);
	dim3 threadBlockColumns(COLUMN_TILE_W, 8);
	cudaMemset((void*)d_temp, 0, SDATA_SIZE);
    for (int nF = 0; nF < DATA_F; nF++){
        convolutionColumnGPU<<<blockGridColumns, threadBlockColumns>>>
	         (d_temp, d_image_conv, DATA_W, DATA_H, COLUMN_TILE_W * threadBlockColumns.y,DATA_W * threadBlockColumns.y,nF);
		cutilCheckMsg("convolutionColumnGPU() execution failed\n"); 
    } 
    
    // frame convolution
	dim3 blockGridFrames(iDivUp(DATA_W *DATA_H , COLUMN_TILE_W),iDivUp(DATA_F, COLUMN_TILE_H),1) ;
	dim3 threadBlockFrames(COLUMN_TILE_W, 8);
	cudaMemset((void*)d_image_conv, 0, SDATA_SIZE);
    convolutionFrameGPU<<<blockGridFrames, threadBlockFrames>>>
       (d_image_conv, d_temp, DATA_W *DATA_H, DATA_F, COLUMN_TILE_W * threadBlockFrames.y,DATA_W*DATA_H * threadBlockFrames.y);
    cutilCheckMsg("convolutionFrameGPU() execution failed\n");
    
	
	cudaFree(d_temp);

    
}

__global__ void myconv2dGPU(float *src, float *dest, float *kernel, int M, int N, int kn)
// [M,N] = size(src); kernel size = 2*kn +1 
// symmetric boundary condition
{

	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;

	if (tid<M*N)
	{
		dest[tid] = src[tid];				
		int x = tid % M, x0;
		int y = tid / M, y0;
	
		float sum = 0.0;
		

		for(int i = -kn; i<=kn; i++)
		{
			x0=x;
			if(x-i <0)
				x0 = -1+2*i-x;
			if(x-i >= M)
				x0 = 2*M-1-x+2*i;
			for(int j = -kn; j<=kn; j++)
			{
				y0 = y;				
				if(y-j<0)
					y0 = -1+2*j-y;
				if(y-j>=N)
					y0 = 2*N-1-y+2*j;
				sum += kernel[(j+kn)*(2*kn+1)+(i+kn)]*src[(y0-j)*M+(x0-i)];	
			}
		}
		dest[tid] = sum;

	}

}
