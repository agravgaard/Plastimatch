
/*******************************************************************
c* Multimodal Deformable Image Registration			   *
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
c*   histogram related functions                                   *
c******************************************************************/

#include "viscous_global.h"

__global__ void marginalDist(float *jointHist, float *probx, float *proby)
{
 	__shared__ float xData[256];
	__shared__ float yData[256];

// each thread loads one element from global to shared memory
       	unsigned int tid = threadIdx.x;
       	unsigned int j =  blockIdx.x*blockDim.x + threadIdx.x;
	unsigned int i = threadIdx.x*blockDim.x + blockIdx.x;

       	xData[tid] = fabs(jointHist[i]);
	yData[tid] = fabs(jointHist[j]);
       	__syncthreads();

// do reduction in shared memory
       	if(tid < 128)
       	{
          	xData[tid] += xData[tid+128];
		yData[tid] += yData[tid+128];
	        __syncthreads();
       	}
       	if(tid < 64)
       	{
        	xData[tid] += xData[tid+64];
		yData[tid] += yData[tid+64];
                __syncthreads();
       	}
       	if(tid < 32)
       	{
               	xData[tid] += xData[tid+32];
               	xData[tid] += xData[tid+16];
               	xData[tid] += xData[tid+8];
               	xData[tid] += xData[tid+4];
               	xData[tid] += xData[tid+2];
                xData[tid] += xData[tid+1];

		yData[tid] += yData[tid+32];
               	yData[tid] += yData[tid+16];
               	yData[tid] += yData[tid+8];
               	yData[tid] += yData[tid+4];
               	yData[tid] += yData[tid+2];
                yData[tid] += yData[tid+1];
  	}

// write result for this block to global memory
       	if (tid == 0) 
	{
		probx[blockIdx.x] = xData[0];
		proby[blockIdx.x] = yData[0];

	}
}



__global__ void mutualInfoGPU(float *jointHist, float *probx, float *proby, float *likelihood)
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;
	if (tid<nBin*nBin)
	{
		int i = tid % nBin;
		int j = tid / nBin;

		likelihood[tid] = jointHist[tid];
		if (likelihood[tid] >0)
		{
			if(probx[i]>0 && proby[j]>0)	
				likelihood[tid] = log2f(likelihood[tid]/probx[i]/proby[j]);
			else
				likelihood[tid] = log2f(likelihood[tid]);	

			jointHist[tid] = jointHist[tid]*likelihood[tid];
		}
		
	}

}

__global__ void marginalBnorm_sum(float *jointHist, float *probx, float *proby, float *Bsum)
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;
	if (tid<nBin*nBin)
	{
		int i = tid % nBin;
		int j = tid / nBin;

		Bsum[tid] = sqrtf( proby[j]*fabs(jointHist[tid])/(probx[i]+EPS) );

	}

}


__global__ void marginalDistAlongY(float *jointHist, float *dest)
{
       __shared__ float data[256];

// each thread loads one element from global to shared memory
       unsigned int tid = threadIdx.x;
       unsigned int i = threadIdx.x*blockDim.x + blockIdx.x;

       data[tid] = jointHist[i];
       __syncthreads();

// do reduction in shared memory
       if(tid < 128)
       {
               data[tid] += data[tid+128];
               __syncthreads();
       }
       if(tid < 64)
       {
               data[tid] += data[tid+64];
               __syncthreads();
       }
       if(tid < 32)
       {
               data[tid] += data[tid+32];
               data[tid] += data[tid+16];
               data[tid] += data[tid+8];
               data[tid] += data[tid+4];
               data[tid] += data[tid+2];
               data[tid] += data[tid+1];
       }

// write result for this block to global memory
       if (tid == 0) dest[blockIdx.x] = data[0];
}




__global__ void BnormGPU(float *jointHist, float *probx, float *proby, float *Bsum, float *likelihood)
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;
	if (tid<nBin*nBin)
	{
		int i = tid % nBin;
		int j = tid / nBin;

		likelihood[tid] = -sqrtf(probx[i]*proby[j]/(jointHist[tid]+EPS))-Bsum[i];
		jointHist[tid] = sqrtf(probx[i]*proby[j]*fabs(jointHist[tid]));
		

	}

}

__global__ void copyHist(unsigned int *hist, float *jointHist)
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;

	if (tid<nBin*nBin)
	{
		jointHist[tid] = (float) hist[tid];
	}

}
