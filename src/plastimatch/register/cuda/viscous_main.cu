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
c*   main code of the multi-modal deformable registration          *
c*    it calls all the other components                            *
c******************************************************************/



// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>


#define GCS_REPRESS_EXTERNS
// includes, gloable variables
#include "viscous_global.h"
//#include "convolution.cu"
#include "viscous_cuda.h"
 
// includes, project
#include <cutil_inline.h>
#include <cublas.h>
#include <thrust/device_ptr.h>
#include <thrust/reduce.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/generate.h>
#include <thrust/sort.h>
#include <thrust/binary_search.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/random.h>
#include <thrust/transform.h>
#include <thrust/functional.h>

#include <cuda.h>   // for float2

using namespace std;
using namespace thrust;

//	include files
//#include "initialize.cu"
//#include "funcHistogram.cu"
//#include "funcImageDomain.cu"
//#include "compute.cu"
//#include "finalize.cu"


/* Global variables */
/***************************************
*	global variables declaration
***************************************/
char inputfilename_move[100]   = "./inputDTI.raw"; 
//	image move
char inputfilename_static[100] = "./inputT2.raw";
// 	image static
char outputfilename[100] = "./outputDTI.raw"; 
//      image out
char output_mv_x[100] = "./output_xFlow.dat";
char output_mv_y[100] = "./output_yFlow.dat";
char output_mv_z[100] = "./output_zFlow.dat";

float *h_im_static, *h_im_move;
//	image pyramid
float *d_im_static[NSCALE], *d_im_move[NSCALE];
// 	vector flow
float *d_mv_x[NSCALE], *d_mv_y[NSCALE], *d_mv_z[NSCALE];


//	gaussian kernel
float *GaussKernelH, *GaussKernelHx;

//	 histogram related
float *d_jointHistogram;
float *d_jointHistogram_conv;
float *d_probx, *d_proby;
float *d_Bsum;

dim3 nblocks;
dim3 nblocks_hist;

int NX, NY, NZ, sDATA_SIZE;
//	dimension at current pyramid level
float max_im_move, min_im_move;
//	max and min intensity of the moving image


cudaArray *d_im_move_array;
texture<float, 3, cudaReadModeElementType> d_im_move_tex;


cudaArray *d_mv_x_array, *d_mv_y_array, *d_mv_z_array;
texture<float, 3, cudaReadModeElementType> d_mv_x_tex;
texture<float, 3, cudaReadModeElementType> d_mv_y_tex;
texture<float, 3, cudaReadModeElementType> d_mv_z_tex;

int deviceCount;
cudaDeviceProp dP;

/****************************************************
	main program
****************************************************/
int 
CUDA_viscous_main (
    int argc, 
    char** argv
)
{
    cout << endl << "****************************************" << endl;
    cout << "Computation parameters..." << endl;
    cout << "****************************************" << endl ;

    int device = DEVICENUMBER;
//      device number

    cudaSetDevice(device);
    cout << "Using device # " << device << endl;
//      choose which device to use

    cudaGetDeviceCount(&deviceCount);
    cudaGetDeviceProperties(&dP,device);
    cout<<"Max threads per block: "<<dP.maxThreadsPerBlock<<endl;
    cout<<"Max Threads DIM: "<<dP.maxThreadsDim[0]<<" x "<<dP.maxThreadsDim[1]<<" x "<<dP.maxThreadsDim[2]<<endl;
    cout<<"Max Grid Size: "<<dP.maxGridSize[0]<<" x "<<dP.maxGridSize[1]<<" x "<<dP.maxGridSize[2]<<endl;
    printf("Device %d: \"%s\" with Compute %d.%d capability\n", 
        device, dP.name, dP.major, dP.minor);
//      obtain computing resource


    nblocks_hist.x = NBLOCKX;
    nblocks_hist.y =  ((1 + (nBin*nBin - 1)/NTHREAD_PER_BLOCK) - 1) / NBLOCKX + 1; 

    cout << endl << "****************************************" << endl;
    cout << "Computing starts..." << endl;
    cout << "****************************************" << endl << endl;

#if defined (commentout)
//	mark the start total time timer 
    unsigned int totalTimer = 0;
    cutilCheckError( cutCreateTimer( &totalTimer));
    cutilCheckError( cutStartTimer( totalTimer));
#endif

/******************************************************
	initialize
******************************************************/
    cout << "\n\n";
    cout << "Initializing MI 3Dreg program...\n\n";
	
//////  CBLAS initialization ////////////////////////////

    cout << "Initializing CUBLAS..." << endl;

    cublasStatus status = cublasInit();
    if (status != CUBLAS_STATUS_SUCCESS)
    {
        fprintf (stderr, "!!!! CUBLAS initialization error\n");
        getchar();
        exit(0);
    }
//      initialize CUBLAS
	
    initData();
    initGaussKernel();
	
/******************************************************
	start iterations
******************************************************/
#if defined (commentout)
    // mark the start time
    unsigned int timer = 0;
    cutilCheckError( cutCreateTimer( &timer));
    cutilCheckError( cutStartTimer( timer));
#endif

    cout << "\n\n";
    cout << "Performing registration...\n\n";

    for(int scale = NSCALE-1; scale >=0; scale--)
    {
        NX = NX0/pow(2, scale);
        NY = NY0/pow(2, scale);
        NZ = (NZ0-1)/pow(2, scale) + 1;
	
        sDATA_SIZE = (NX*NY*NZ)* sizeof(float);

        nblocks.x = NBLOCKX;
        nblocks.y =  ((1 + (NX*NY*NZ - 1)/NTHREAD_PER_BLOCK) - 1) 
            / NBLOCKX + 1;
        printf ("current scale = %d, size of image = %d x %d x %d ... \n", 
            scale, NX, NY, NZ);
        if(scale<NSCALE-1)
        {
            upSample<<<nblocks, NTHREAD_PER_BLOCK>>>(
                d_mv_x[scale+1], d_mv_x[scale], NX, NY, NZ);
            upSample<<<nblocks, NTHREAD_PER_BLOCK>>>(
                d_mv_y[scale+1], d_mv_y[scale], NX, NY, NZ);
            upSample<<<nblocks, NTHREAD_PER_BLOCK>>>(
                d_mv_z[scale+1], d_mv_z[scale], NX, NY, NZ);
        }
        compute (
            d_im_move[scale], d_im_static[scale], 
            d_mv_x[scale], d_mv_y[scale], d_mv_z[scale], MAX_ITER);
        printf("\n\n");
    }

    cudaThreadSynchronize();
#if defined (commentout)
    cutilCheckError( cutStopTimer( timer));
    printf("\n\n****************************************\n");
    printf( "Computing time: %f (ms)\n", cutGetTimerValue( timer));
    printf("****************************************\n\n\n");
    cutilCheckError( cutDeleteTimer( timer));
#endif
//      mark the end timer and print

/******************************************************
	finalize
******************************************************/

    printf("Finalizing program...\n\n");
	
    fina();

/****   shut down CBLAS ********/

    status = cublasShutdown();
    if (status != CUBLAS_STATUS_SUCCESS)
    {
        fprintf (stderr, "!!!! shutdown error (A)\n");
        getchar();
        exit(0);
    }
//      Shut down CUBLAS

    cudaThreadSynchronize();

//	mark the end total timer
#if defined (commentout)
    cutilCheckError( cutStopTimer( totalTimer));
    printf("\n\n****************************************\n");
    printf( "Entire program time: %f (ms)\n", cutGetTimerValue( totalTimer));
    printf("****************************************\n\n\n");
    cutilCheckError( cutDeleteTimer( totalTimer));
#endif

    printf("Have a nice day!\n");
	
    cudaThreadExit();	

#if defined (commentout)
    cutilExit(argc, argv);
#endif
    return 0;
}
