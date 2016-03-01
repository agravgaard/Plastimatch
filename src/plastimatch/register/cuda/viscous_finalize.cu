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
c*   Finalize the reconstruction on the current scale and for the  *
c*   entire program, output results, release memory spaces for     *
c*   global variables, etc.                                        *
c******************************************************************/

#include <iostream>
#include <stdio.h>
#include <cutil.h>
#include <cutil_inline_runtime.h>
#include "viscous_global.h"

void fina()
{
//	map output image to its original scale
	nblocks.x = NBLOCKX;
	nblocks.y =  ((1 + (NX0*NY0*NZ0 - 1)/NTHREAD_PER_BLOCK) - 1) / NBLOCKX + 1; 

	printf("moving image: max = %f, min = %f\n", max_im_move, min_im_move);
	intensityRescale<<<nblocks, NTHREAD_PER_BLOCK>>>(d_im_move[0], max_im_move, min_im_move, -1);

//	output results
	outputData(d_im_move[0], DATA_SIZE, outputfilename);
	outputData(d_mv_x[0], DATA_SIZE, output_mv_x);
	outputData(d_mv_y[0], DATA_SIZE, output_mv_y);
	outputData(d_mv_z[0], DATA_SIZE, output_mv_z);

// 	free up the host and device 
//	image pyramid
	for(int scale =0; scale <NSCALE; scale++)
	{
		cudaFree(d_im_move[scale]);
		cudaFree(d_im_static[scale]);
		cudaFree(d_mv_x[scale]);
		cudaFree(d_mv_y[scale]);
		cudaFree(d_mv_z[scale]);
	}

	

//	Gaussian kernel
	cudaFree(GaussKernelH);
	cudaFree(GaussKernelHx);
	

//	histogram related
	cudaFree(d_jointHistogram);
	cudaFree(d_jointHistogram_conv);
	cudaFree(d_probx);
	cudaFree(d_proby);
	cudaFree(d_Bsum);
	
}


void outputData(void *src, int size, const char *outputfilename)
//      output data to file
{
      //  void *tempData_h = malloc( size );

	float *tempData_h = (float*) malloc (sizeof(float)*size);
  	if (tempData_h == NULL) 
	{
		fputs ("Memory error",stderr); 
		exit (2);
	}

        cutilSafeCall( cudaMemcpy( tempData_h, src, size, cudaMemcpyDeviceToHost) );
//      copy data from GPU to CPU

        FILE *fp;
        fp = fopen(outputfilename,"wb");
        if( fp == NULL )
        {
            std::cout << "Can not open file to write results.";
                exit(1);
        }
        fwrite (tempData_h, size, 1 , fp );

        fclose(fp);
//      write results to file
	
	//printf("denoised data =%f\n", tempData_h[53]);
        free(tempData_h);
//      free space

}
