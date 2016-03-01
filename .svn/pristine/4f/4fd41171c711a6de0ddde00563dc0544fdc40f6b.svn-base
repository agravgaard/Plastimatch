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
#ifndef _viscous_global_h_
#define _viscous_global_h_

/**************************************
*	CUDA parameters
**************************************/
#define NTHREAD_PER_BLOCK 256
//      number of threads per block along each direction

#define NBLOCKX 1024
//      the leading dimension of the 2d thread grid

#define DEVICENUMBER 0
//      device number to be used



/**************************************
*	parameters
**************************************/
#define NX0 256
#define NY0 256
#define NZ0 64
#define DATA_SIZE ((NX0*NY0*NZ0)*sizeof(float))
#define NSCALE 2
//	number of scales in the image pyramid: 0 is the finest, NSCALE-1 is the coarest


#define MAX_ITER 20
//	max # of iterations in the finest grid
//	# of iterations at each level = 2^scale*MAX_ITER



//	histogram related parameters
#define nBin 256
// 	number of histogram bins

#define hValue 4
//	int, parameter in the Gaussian for convolution histogram
#define HIST_SIZE (nBin*nBin*sizeof(float))


#define sValue 3
// 	int, parameter in the Gaussian for updating velocity
#define sLength (6*sValue+1)


//	image domain parameters
#define ALPHA 500
//	parameter in the force calculation

#define du 0.6
//	parameter to dynamically define dt 

#define METHOD 1
//	1 for Bnorm, 2 for MI 

#define threshJaco 0.5
//	threshold for Jacobian to regridding


#define EPS 0.000001


/***************************************
*	global variables declaration
***************************************/
#ifndef GCS_REPRESS_EXTERNS
extern char inputfilename_move[100];
//	image move
extern char inputfilename_static[100];
// 	image static
extern char outputfilename[100];
//      image out
extern char output_mv_x[100];
extern char output_mv_y[100];
extern char output_mv_z[100];


extern float *h_im_static, *h_im_move;
//	image pyramid
extern float *d_im_static[NSCALE], *d_im_move[NSCALE];
// 	vector flow
extern float *d_mv_x[NSCALE], *d_mv_y[NSCALE], *d_mv_z[NSCALE];


//	gaussian kernel
extern float *GaussKernelH, *GaussKernelHx;

//	 histogram related
extern float *d_jointHistogram;
extern float *d_jointHistogram_conv;
extern float *d_probx, *d_proby;
extern float *d_Bsum;

extern dim3 nblocks;
extern dim3 nblocks_hist;

extern int NX, NY, NZ, sDATA_SIZE;
//	dimension at current pyramid level
extern float max_im_move, min_im_move;
//	max and min intensity of the moving image


extern cudaArray *d_im_move_array;
extern texture<float, 3, cudaReadModeElementType> d_im_move_tex;


extern cudaArray *d_mv_x_array, *d_mv_y_array, *d_mv_z_array;
extern texture<float, 3, cudaReadModeElementType> d_mv_x_tex;
extern texture<float, 3, cudaReadModeElementType> d_mv_y_tex;
extern texture<float, 3, cudaReadModeElementType> d_mv_z_tex;

extern int deviceCount;
extern cudaDeviceProp dP;
//      device properties

#endif /* GCS_REPRESS_EXTERNS */

/*************************************

* 	simple math functions

***************************************/
__host__ __device__ float minmod(float x, float y);
__device__ float ImageGradient(float Im, float I, float Ip);


/****************************************
*	Data processing
****************************************/
void dataPreprocessing(float *image, float *maxValue, float *minValue);
__global__ void intensityRescale(float *image, float maxValue, float minValue, int type);
// type >0 forward calculation: rescale image intensity to [0,1]
// type <0 backward: map intensity to its original scale

void loadData(float *dest, int sizeInByte, const char *filename);
void outputData(void *src, int size, const char *outputfilename);

/***************************************
*	function declaration
***************************************/
void initData();
void initGaussKernel();
void fina();
void compute(float *d_im_move, float *d_im_static, float *d_mv_x, float *d_mv_y, float *d_mv_z, int maxIter);


/****************************************
*	kernel declaration
****************************************/
__global__ void upSample(float *src, float *dest, int NX, int NY, int NZ);
__global__ void downSample(float *src, float *dest, int NX, int NY, int NZ, int s);

__global__ void ImageWarp(float *mv_x, float *mv_y, float *mv_z, float *dest, int NX, int NY, int NZ);
__global__ void ImageWarp_mv(float *mv_x, float *mv_y, float *mv_z, int NX, int NY, int NZ);
__global__ void ImageWarp_final(float *mv_x, float *mv_y, float *mv_z, float *dest, int NX, int NY, int NZ);

__global__ void forceComp(float *d_im_out, float *d_im_static, float *d_Likelihood, float *d_v_x, float *d_v_y, float *d_v_z, int NX, int NY, int NZ);
__global__ void flowComp(float *d_mv_x, float *d_mv_y, float *d_mv_z, float *d_v_x, float *d_v_y, float *d_v_z, float *jacobian, float *flow, int NX, int NY, int NZ);
__global__ void flowUpdate(float *d_mv_x, float *d_mv_y, float *d_mv_z, float *d_disp_x, float *d_disp_y, float *d_disp_z, float dt, int NX, int NY, int NZ);

__global__ void marginalDist(float *jointHist, float *probx, float *proby);
__global__ void mutualInfoGPU(float *jointHist, float *probx, float *proby, float *likelihood);
__global__ void copyHist(unsigned int *hist, float *jointHist);
__global__ void marginalBnorm_sum(float *jointHist, float *probx, float *proby, float *Bsum);
__global__ void marginalDistAlongY(float *jointHist, float *dest);
__global__ void BnormGPU(float *jointHist, float *probx, float *proby, float *Bsum, float *likelihood);
__global__ void transToFloat2(const float *input1, const float *input2, float2 *output, const int n);

#endif
