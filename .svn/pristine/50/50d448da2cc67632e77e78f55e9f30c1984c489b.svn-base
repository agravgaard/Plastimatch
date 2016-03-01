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
c*   Supporting functions in the image domain                      *
c*	 such as gradient, force computation, flow propagation, etc*
c******************************************************************/

#include "viscous_global.h"

__global__ void forceComp(float *d_im_out, float *d_im_static, float *d_Likelihood, float *d_v_x, float *d_v_y, float *d_v_z, int NX, int NY, int NZ)
{
	
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;

if (tid<NX*NY*NZ)
{
	float x = d_im_out[tid]*(nBin-1.0);
	float y = d_im_static[tid]*(nBin-1.0);
	
	int xmin = x;
	int xmax = (xmin == x)? xmin : xmin+1;
	int ymin = y;
	int ymax = (ymin == y)? ymin : ymin+1;
	
	float dLx = 	  d_Likelihood[ymin*nBin + xmin] * (1.0f-(x-xmin)) * (1.0f-(y-ymin))
			+ d_Likelihood[ymax*nBin + xmin] * (1.0f-(x-xmin)) * (y-ymin)  
			+ d_Likelihood[ymin*nBin + xmax] * (x-xmin)       * (1.0f-(y-ymin))
			+ d_Likelihood[ymax*nBin + xmax] * (x-xmin)	  * (y-ymin);		

	
			
// 	compute image gradient, save as x, y and z
	float z;
	int zmin = tid/(NX*NY);
	ymin = (tid%(NX*NY))/NX;
	xmin = tid%NX;

	if(xmin+1 < NX && xmin-1>=0)
		x = ImageGradient(d_im_out[zmin*NX*NY + ymin*NX + (xmin-1) ], d_im_out[zmin*NX*NY +ymin*NX + xmin], d_im_out[zmin*NX*NY +ymin*NX + (xmin+1)]);
	else 	x = 0;

	if(ymin+1 < NY && ymin-1>=0)
		y = ImageGradient(d_im_out[zmin*NX*NY +(ymin-1)*NX + xmin], d_im_out[zmin*NX*NY +ymin*NX+ xmin], d_im_out[zmin*NX*NY +(ymin+1)*NX+xmin]);
	else	y = 0;
	if(zmin+1 <NZ && zmin-1>=0)
		z = ImageGradient(d_im_out[(zmin-1)*NX*NY + ymin*NX + xmin], d_im_out[zmin*NX*NY + ymin*NX + xmin],d_im_out[(zmin+1)*NX*NY + ymin*NX + xmin]);
	else 	z = 0;

	d_v_x[tid] = 	-ALPHA*dLx*x;
	d_v_y[tid] = 	-ALPHA*dLx*y;
	d_v_z[tid] =	-ALPHA*dLx*z;

		
}
}

__global__ void flowComp(float *d_mv_x, float *d_mv_y, float *d_mv_z, float *d_v_x, float *d_v_y, float *d_v_z, float *jacobian, float *flow, int NX, int NY, int NZ)
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;

if (tid<NX*NY*NZ)
{
	float u1x, u1y, u1z;
	float u2x, u2y, u2z;
	float u3x, u3y, u3z;

	int z = tid/(NX*NY);
	int y = (tid%(NX*NY))/NX;
	int x = tid%NX;

	if(x+1<NX && x-1>=0)
	{
		u1x = ImageGradient(d_mv_x[z*NX*NY+y*NX+x-1], d_mv_x[z*NX*NY+y*NX+x], d_mv_x[z*NX*NY+y*NX+x+1]);
		u2x = ImageGradient(d_mv_y[z*NX*NY+y*NX+x-1], d_mv_y[z*NX*NY+y*NX+x], d_mv_y[z*NX*NY+y*NX+x+1]);
		u3x = ImageGradient(d_mv_z[z*NX*NY+y*NX+x-1], d_mv_z[z*NX*NY+y*NX+x], d_mv_y[z*NX*NY+y*NX+x+1]);
	}
	else 
	{
		u1x = 0;
		u2x = 0;
		u3x = 0;
	}

	if(y+1<NY && y-1>=0)
	{
		u1y = ImageGradient(d_mv_x[z*NX*NY+(y-1)*NX+x], d_mv_x[z*NX*NY+y*NX+x], d_mv_x[z*NX*NY+(y+1)*NX+x]);
		u2y = ImageGradient(d_mv_y[z*NX*NY+(y-1)*NX+x], d_mv_y[z*NX*NY+y*NX+x], d_mv_y[z*NX*NY+(y+1)*NX+x]);
		u3y = ImageGradient(d_mv_z[z*NX*NY+(y-1)*NX+x], d_mv_z[z*NX*NY+y*NX+x], d_mv_z[z*NX*NY+(y+1)*NX+x]);
	}
	else
	{
		u1y = 0; 
		u2y = 0;
		u3y = 0;
	}
	
	if(z+1<NZ && z-1>=0)
	{
		u1z = ImageGradient(d_mv_x[(z-1)*NX*NY+y*NX+x], d_mv_x[z*NX*NY+y*NX+x], d_mv_x[(z+1)*NX*NY+y*NX+x]);
		u2z = ImageGradient(d_mv_y[(z-1)*NX*NY+y*NX+x], d_mv_y[z*NX*NY+y*NX+x], d_mv_y[(z+1)*NX*NY+y*NX+x]);
		u3z = ImageGradient(d_mv_z[(z-1)*NX*NY+y*NX+x], d_mv_z[z*NX*NY+y*NX+x], d_mv_z[(z+1)*NX*NY+y*NX+x]);
	}
	else
	{
		u1z = 0; 
		u2z = 0;
		u3z = 0;
	}
	
	float R1 = d_v_x[tid] - d_v_x[tid]*u1x - d_v_y[tid]*u1y - d_v_z[tid]*u1z;
	float R2 = d_v_y[tid] - d_v_x[tid]*u2x - d_v_y[tid]*u2y - d_v_z[tid]*u2z;
	float R3 = d_v_z[tid] - d_v_x[tid]*u3x - d_v_y[tid]*u3y - d_v_z[tid]*u3z;

	float jaco = (1.0f-u1x)*(1.0f-u2y)*(1.0f-u3z)-u3x*u1y*u2z - u2x*u3y*u1z 
			-(1.0f-u1x)*u3y*u2z - u3x*(1.0f-u2y)*u1z - u2x*u1y*(1.0f-u3z);
	jacobian[tid] = (fabs(jaco)<= threshJaco) ? 1.0f : 0.0f;
	
	flow[tid] = sqrtf(R1 * R1 + R2 * R2 + R3 * R3);
	
// 	d_v_x, d_v_y, d_v_z become displacement
	d_v_x[tid] = R1; 
	d_v_y[tid] = R2;
	d_v_z[tid] = R3;
	
}	
}

__global__ void flowUpdate(float *d_mv_x, float *d_mv_y, float *d_mv_z, float *d_disp_x, float *d_disp_y, float *d_disp_z, float dt, int NX, int NY, int NZ)
{
	
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;

if (tid<NX*NY*NZ)
{
	
	d_mv_x[tid] += d_disp_x[tid]*dt;
	d_mv_y[tid] += d_disp_y[tid]*dt;
	d_mv_z[tid] += d_disp_z[tid]*dt;

}
}

__device__ float ImageGradient(float Im, float I, float Ip)
{
	float xp = Ip-I;
	float xm = I - Im;

	return minmod(xp, xm);


}


__global__ void ImageWarp(float *mv_x, float *mv_y, float *mv_z, float *dest, int NX, int NY, int NZ)
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;
//      obtain current id on

        if(tid < NX*NY*NZ)
	{
		int z = tid/(NX*NY);
		int y = (tid%(NX*NY))/NX;
		int x = tid%NX;

		float v_x = mv_x[tid];
		float v_y = mv_y[tid];
		float v_z = mv_z[tid];

		dest[tid] = tex3D(d_im_move_tex, x-v_x+0.5, y-v_y+0.5, z-v_z+0.5);	

	}
	
}

__global__ void ImageWarp_mv(float *mv_x, float *mv_y, float *mv_z, int NX, int NY, int NZ)
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;
//      obtain current id on

        if(tid < NX*NY*NZ)
	{
		int z = tid/(NX*NY);
		int y = (tid%(NX*NY))/NX;
		int x = tid%NX;

		float v_x = mv_x[tid];
		float v_y = mv_y[tid];
		float v_z = mv_z[tid];
		
		mv_x[tid] += tex3D(d_mv_x_tex, x-v_x+0.5, y-v_y+0.5,z-v_z+0.5) ;	
		mv_y[tid] += tex3D(d_mv_y_tex, x-v_x+0.5, y-v_y+0.5,z-v_z+0.5) ;	
		mv_z[tid] += tex3D(d_mv_z_tex, x-v_x+0.5, y-v_y+0.5,z-v_z+0.5) ;	
	}
	
}

__global__ void ImageWarp_final(float *mv_x, float *mv_y, float *mv_z, float *dest, int NX, int NY, int NZ)
{
	const int tid = (blockIdx.y*NBLOCKX + blockIdx.x)*blockDim.x + threadIdx.x;
//      obtain current id on

        if(tid < NX*NY*NZ)
	{
		int z = tid/(NX*NY);
		int y = (tid%(NX*NY))/NX;
		int x = tid%NX;
				
		mv_x[tid] = tex3D(d_mv_x_tex, x, y, z) ;	
		mv_y[tid] = tex3D(d_mv_y_tex, x, y, z) ;
		mv_z[tid] = tex3D(d_mv_z_tex, x, y, z) ;	
		dest[tid] = tex3D(d_im_move_tex, x-mv_x[tid]+0.5, y-mv_y[tid]+0.5, z-mv_z[tid]+0.5);
	}
	
}


__host__ __device__ float minmod(float x, float y)
{
	if (x*y<=0)
		return 0;

	if (x>0)
	{
		if(x>y)
			return y;
		else 	return x;
	}
	else
	{
		if(x>y)	return x;
		else 	return y;
	}
}
