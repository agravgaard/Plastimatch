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



#ifndef _3DImageSmooth_H_
#define _3DImageSmooth_H_

#define    KERNEL_RADIUS   8
const int  KERNEL_W = (2 * KERNEL_RADIUS + 1);


// Assuming ROW_TILE_W, KERNEL_RADIUS_ALIGNED and dataW 
// are multiples of coalescing granularity size,
// all global memory operations are coalesced in convolutionRowGPU()
#define            ROW_TILE_W              128
#define            KERNEL_RADIUS_ALIGNED   16


// Assuming COLUMN_TILE_W and dataW are multiples
// of coalescing granularity size, all global memory operations 
// are coalesced in convolutionColumnGPU()
#define           COLUMN_TILE_W        16
#define           COLUMN_TILE_H        48


//#define           UNROLL_INNER  // for fast convolution


////////////////////////////////////////////////////////////////////////////////
// Common host and device functions
////////////////////////////////////////////////////////////////////////////////
//Round a / b to nearest higher integer value
inline int iDivUp(int a, int b){
    return (a % b != 0) ? (a / b + 1) : (a / b);
}

//24-bit multiplication is faster on G80,
//but we must be sure to multiply integers
//only within [-8M, 8M - 1] range
#define IMUL(a, b) __mul24(a, b)


__device__ __constant__ float             d_Kernel[KERNEL_W];


////////////////////////////////////////////////////////////////////////////////
// Loop unrolling templates, needed for best performance
////////////////////////////////////////////////////////////////////////////////

template<int i> __device__ float convolutionRow(float *data){
    return
        data[KERNEL_RADIUS - i] * d_Kernel[i]
        + convolutionRow<i - 1>(data);
}

template<> __device__ float convolutionRow<-1>(float *data){
    return 0;
}

template<int i> __device__ float convolutionColumn(float *data){
    return 
        data[(KERNEL_RADIUS - i) * COLUMN_TILE_W] * d_Kernel[i]
        + convolutionColumn<i - 1>(data);
}

template<> __device__ float convolutionColumn<-1>(float *data){
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// GPU convolution
////////////////////////////////////////////////////////////////////////////////

//subroutines

void   ImageSmooth(float *d_image, float *d_image_conv, int3 Dims);
__global__ void myconv2dGPU(float *src, float *dest, float *kernel, int M, int N, int kn);

#endif







