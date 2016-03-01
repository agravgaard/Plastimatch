/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdlib.h>

#include "bspline_regularize.h"
#include "bspline_score.h"
#include "bspline_xform.h"
#include "logfile.h"
#include "plm_timer.h"
#include "print_and_exit.h"

/* Flavor 'd' */
void 
Bspline_regularize::create_qlut_grad 
(
    const Bspline_xform* bxf,         /* Output: bxf with new LUTs */
    const float img_spacing[3],       /* Image spacing (in mm) */
    const plm_long vox_per_rgn[3])         /* Knot spacing (in vox) */
{
    plm_long i, j, k, p;
    int tx, ty, tz;
    float *A, *B, *C;
    float *Ax, *By, *Cz, *Axx, *Byy, *Czz;
    size_t q_lut_size;

    q_lut_size = sizeof(float) * bxf->vox_per_rgn[0] 
	* bxf->vox_per_rgn[1] 
	* bxf->vox_per_rgn[2] 
	* 64;
    logfile_printf("Creating gradient multiplier LUTs, %d bytes each\n", q_lut_size);

    this->q_dxdyz_lut = (float*) malloc ( q_lut_size );
    if (!this->q_dxdyz_lut) print_and_exit ("Error allocating memory for q_grad_lut\n");
	
    this->q_xdydz_lut = (float*) malloc ( q_lut_size );
    if (!this->q_xdydz_lut) print_and_exit ("Error allocating memory for q_grad_lut\n");

    this->q_dxydz_lut = (float*) malloc ( q_lut_size );
    if (!this->q_dxydz_lut) print_and_exit ("Error allocating memory for q_grad_lut\n");
	
    this->q_d2xyz_lut = (float*) malloc ( q_lut_size );
    if (!this->q_d2xyz_lut) print_and_exit ("Error allocating memory for q_grad_lut\n");

    this->q_xd2yz_lut = (float*) malloc ( q_lut_size );
    if (!this->q_xd2yz_lut) print_and_exit ("Error allocating memory for q_grad_lut\n");

    this->q_xyd2z_lut = (float*) malloc ( q_lut_size );
    if (!this->q_xyd2z_lut) print_and_exit ("Error allocating memory for q_grad_lut\n");

    A = (float*) malloc (sizeof(float) * bxf->vox_per_rgn[0] * 4);
    B = (float*) malloc (sizeof(float) * bxf->vox_per_rgn[1] * 4);
    C = (float*) malloc (sizeof(float) * bxf->vox_per_rgn[2] * 4);

    Ax = (float*) malloc (sizeof(float) * bxf->vox_per_rgn[0] * 4);
    By = (float*) malloc (sizeof(float) * bxf->vox_per_rgn[1] * 4);
    Cz = (float*) malloc (sizeof(float) * bxf->vox_per_rgn[2] * 4);

    Axx = (float*) malloc (sizeof(float) * bxf->vox_per_rgn[0] * 4);
    Byy = (float*) malloc (sizeof(float) * bxf->vox_per_rgn[1] * 4);
    Czz = (float*) malloc (sizeof(float) * bxf->vox_per_rgn[2] * 4);

    for (i = 0; i < bxf->vox_per_rgn[0]; i++) {
	float ii = ((float) i) / bxf->vox_per_rgn[0];
	float t3 = ii*ii*ii;
	float t2 = ii*ii;
	float t1 = ii;
	A[i*4+0] = (1.0/6.0) * (- 1.0 * t3 + 3.0 * t2 - 3.0 * t1 + 1.0);
	A[i*4+1] = (1.0/6.0) * (+ 3.0 * t3 - 6.0 * t2            + 4.0);
	A[i*4+2] = (1.0/6.0) * (- 3.0 * t3 + 3.0 * t2 + 3.0 * t1 + 1.0);
	A[i*4+3] = (1.0/6.0) * (+ 1.0 * t3);

	Ax[i*4+0] =(1.0/6.0) * (- 3.0 * t2 + 6.0 * t1 - 3.0           );
	Ax[i*4+1] =(1.0/6.0) * (+ 9.0 * t2 - 12.0* t1                 );
	Ax[i*4+2] =(1.0/6.0) * (- 9.0 * t2 + 6.0 * t1 + 3.0           );
	Ax[i*4+3] =(1.0/6.0) * (+ 3.0 * t2);

	Axx[i*4+0]=(1.0/6.0) * (- 6.0 * t1 + 6.0                     );
	Axx[i*4+1]=(1.0/6.0) * (+18.0 * t1 - 12.0                    );
	Axx[i*4+2]=(1.0/6.0) * (-18.0 * t1 + 6.0                     );
	Axx[i*4+3]=(1.0/6.0) * (+ 6.0 * t1);
    }
    for (j = 0; j < bxf->vox_per_rgn[1]; j++) {
	float jj = ((float) j) / bxf->vox_per_rgn[1];
	float t3 = jj*jj*jj;
	float t2 = jj*jj;
	float t1 = jj;
	B[j*4+0] = (1.0/6.0) * (- 1.0 * t3 + 3.0 * t2 - 3.0 * t1 + 1.0);
	B[j*4+1] = (1.0/6.0) * (+ 3.0 * t3 - 6.0 * t2            + 4.0);
	B[j*4+2] = (1.0/6.0) * (- 3.0 * t3 + 3.0 * t2 + 3.0 * t1 + 1.0);
	B[j*4+3] = (1.0/6.0) * (+ 1.0 * t3);

	By[j*4+0] =(1.0/6.0) * (- 3.0 * t2 + 6.0 * t1 - 3.0           );
	By[j*4+1] =(1.0/6.0) * (+ 9.0 * t2 - 12.0* t1                 );
	By[j*4+2] =(1.0/6.0) * (- 9.0 * t2 + 6.0 * t1 + 3.0           );
	By[j*4+3] =(1.0/6.0) * (+ 3.0 * t2);

	Byy[j*4+0]=(1.0/6.0) * (- 6.0 * t1 + 6.0                     );
	Byy[j*4+1]=(1.0/6.0) * (+18.0 * t1 - 12.0                    );
	Byy[j*4+2]=(1.0/6.0) * (-18.0 * t1 + 6.0                     );
	Byy[j*4+3]=(1.0/6.0) * (+ 6.0 * t1);
    }
    for (k = 0; k < bxf->vox_per_rgn[2]; k++) {
	float kk = ((float) k) / bxf->vox_per_rgn[2];
	float t3 = kk*kk*kk;
	float t2 = kk*kk;
	float t1 = kk;
	C[k*4+0] = (1.0/6.0) * (- 1.0 * t3 + 3.0 * t2 - 3.0 * t1 + 1.0);
	C[k*4+1] = (1.0/6.0) * (+ 3.0 * t3 - 6.0 * t2            + 4.0);
	C[k*4+2] = (1.0/6.0) * (- 3.0 * t3 + 3.0 * t2 + 3.0 * t1 + 1.0);
	C[k*4+3] = (1.0/6.0) * (+ 1.0 * t3);

	Cz[k*4+0] =(1.0/6.0) * (- 3.0 * t2 + 6.0 * t1 - 3.0           );
	Cz[k*4+1] =(1.0/6.0) * (+ 9.0 * t2 - 12.0* t1                 );
	Cz[k*4+2] =(1.0/6.0) * (- 9.0 * t2 + 6.0 * t1 + 3.0           );
	Cz[k*4+3] =(1.0/6.0) * (+ 3.0 * t2);

	Czz[k*4+0]=(1.0/6.0) * (- 6.0 * t1 + 6.0                     );
	Czz[k*4+1]=(1.0/6.0) * (+18.0 * t1 - 12.0                    );
	Czz[k*4+2]=(1.0/6.0) * (-18.0 * t1 + 6.0                     );
	Czz[k*4+3]=(1.0/6.0) * (+ 6.0 * t1);
    }

    p = 0;
    for (k = 0; k < bxf->vox_per_rgn[2]; k++) {
	for (j = 0; j < bxf->vox_per_rgn[1]; j++) {
	    for (i = 0; i < bxf->vox_per_rgn[0]; i++) {
		for (tz = 0; tz < 4; tz++) {
		    for (ty = 0; ty < 4; ty++) {
			for (tx = 0; tx < 4; tx++) {
				
			    this->q_dxdyz_lut[p] = Ax[i*4+tx] * By[j*4+ty] * C[k*4+tz];
			    this->q_xdydz_lut[p] = A[i*4+tx] * By[j*4+ty] * Cz[k*4+tz];
			    this->q_dxydz_lut[p] = Ax[i*4+tx] * B[j*4+ty] * Cz[k*4+tz];

			    this->q_d2xyz_lut[p] = Axx[i*4+tx] * B[j*4+ty] * C[k*4+tz];
			    this->q_xd2yz_lut[p] = A[i*4+tx] * Byy[j*4+ty] * C[k*4+tz];
			    this->q_xyd2z_lut[p] = A[i*4+tx] * B[j*4+ty] * Czz[k*4+tz];

			    p++;
			}
		    }
		}
	    }
	}
    }
    free (C);
    free (B);
    free (A);
    free (Ax); free(By); free(Cz); free(Axx); free(Byy); free(Czz);
}

#define USE_FAST_CODE 1

/*internal use only - get k-th component (k=0,1,2) of 
the vector field at location r using pre-rendered vf */
static float
f2 (int k, int r[3], float  *vf, int *dims)
{
    int d;
    d = r[2] * dims[0] * dims[1] + r[1] * dims[0] + r[0];
    return vf[3*d+k];
}

/*
first derivative of the vector field du_i/dx_i using pre-rendered vf
calculated at position r[3] = (ri rj rk) in voxels */
float
bspline_regularize_1st_derivative (
    int i,
    int r[3],
    float h[3],
    float *vf,
    int *dims
)
{
    int r1[3], r2[3];
    int d;

    for(d=0; d<3; d++) {  r1[d]=r[d]; r2[d]=r[d]; }
    r1[i] ++;
    r2[i] --;

    return (f2(i,r1, vf, dims)-f2(i,r2, vf, dims))/(2.*h[i]);
}

/* second derivative of k-th component of vector field wrt x_i and x_j, 
d2u_k/(dx_i dx_j) calculated at position r[3] = (ri rj rk) in voxels 
using pre-rendered vf */
float
bspline_regularize_2nd_derivative (
    int k,
    int i,
    int j,
    int r[3],
    float h[3],
    float *vf,
    int *dims
)
{
    int r1[3], r2[3], r3[3], r4[3], r5[3], r6[3];
    int d;

    if (i==j) {
	for(d=0; d<3; d++) {  r1[d]=r[d]; r2[d]=r[d]; }
	r1[i] ++;
	r2[i] --;
	return  ( f2(k, r1, vf, dims) + f2(k, r2, vf, dims) - 2*f2(k, r, vf, dims ) ) / (h[i]*h[i]);
    } else {
	for(d=0; d<3; d++) { 
	    r1[d]=r[d]; 
	    r2[d]=r[d]; 
	    r3[d]=r[d]; 
	    r4[d]=r[d];
	    r5[d]=r[d]; 
	    r6[d]=r[d]; 
	}
	/* voxel not used*/	r1[j]++;	r2[j]++; r2[i]++;
	r3[i]--;			/* r[] */		r4[i]++;
	r5[i]--; r5[j]--;   r6[j]--;  /*voxel not used */

	return ( -f2(k,r1,vf, dims)+f2(k,r2,vf,dims)-f2(k,r3,vf,dims)
	    +2*f2(k,r,vf,dims)
	    -f2(k,r4,vf, dims)+f2(k,r5,vf,dims)-f2(k,r6,vf,dims))/(2.*h[i]*h[j]);
    }
}

/* out[i] = 2nd derivative of i-th component (i=0,1,2<->x,y,z) of the vector field
with respect to variables derive1 and derive2 (0,1,2<->x,y,z),
or (derive1,derive2)th element of the Hessian of the i-th component of the vector field 
*/
void
Bspline_regularize::hessian_component (
    float out[3], 
    const Bspline_xform* bxf, 
    plm_long p[3], 
    plm_long qidx, 
    int derive1, 
    int derive2
)
{
    int i, j, k, m;
    int cidx;
    float* q_lut = 0;

    if (derive1==0 && derive2==0) q_lut = &this->q_d2xyz_lut[qidx*64];
    if (derive1==1 && derive2==1) q_lut = &this->q_xd2yz_lut[qidx*64];
    if (derive1==2 && derive2==2) q_lut = &this->q_xyd2z_lut[qidx*64];

    if (derive1==0 && derive2==1) q_lut = &this->q_dxdyz_lut[qidx*64];
    if (derive1==1 && derive2==0) q_lut = &this->q_dxdyz_lut[qidx*64];
	
    if (derive1==0 && derive2==2) q_lut = &this->q_dxydz_lut[qidx*64];
    if (derive1==2 && derive2==0) q_lut = &this->q_dxydz_lut[qidx*64];

    if (derive1==1 && derive2==2) q_lut = &this->q_xdydz_lut[qidx*64];
    if (derive1==2 && derive2==1) q_lut = &this->q_xdydz_lut[qidx*64];

    out[0] = out[1] = out[2] = 0;
    m = 0;
    for (k = 0; k < 4; k++) {
	for (j = 0; j < 4; j++) {
	    for (i = 0; i < 4; i++) {
		cidx = (p[2] + k) * bxf->cdims[1] * bxf->cdims[0]
		    + (p[1] + j) * bxf->cdims[0]
		    + (p[0] + i);
		cidx = cidx * 3;
		out[0] += q_lut[m] * bxf->coeff[cidx+0] ;
		out[1] += q_lut[m] * bxf->coeff[cidx+1] ;
		out[2] += q_lut[m] * bxf->coeff[cidx+2] ;
		m ++;
	    }
	}
    }
}

void
bspline_regularize_hessian_component_b (
    float out[3], 
    const Bspline_xform* bxf, 
    plm_long p[3], 
    plm_long qidx, 
    float *q_lut
)
{
    int i, j, k, m;
    int cidx;

    out[0] = out[1] = out[2] = 0;
    m = 0;
    for (k = 0; k < 4; k++) {
	for (j = 0; j < 4; j++) {
	    for (i = 0; i < 4; i++) {
		cidx = (p[2] + k) * bxf->cdims[1] * bxf->cdims[0]
		    + (p[1] + j) * bxf->cdims[0]
		    + (p[0] + i);
		cidx = cidx * 3;
		out[0] += q_lut[m] * bxf->coeff[cidx+0];
		out[1] += q_lut[m] * bxf->coeff[cidx+1];
		out[2] += q_lut[m] * bxf->coeff[cidx+2];
		m ++;
	    }
	}
    }
}

void
Bspline_regularize::hessian_update_grad (
    Bspline_score *bscore, 
    const Bspline_xform* bxf, 
    plm_long p[3], 
    plm_long qidx, 
    float dc_dv[3], 
    int derive1, 
    int derive2)
{
    int i, j, k, m;
    int cidx;
    float* q_lut = 0;

    if (derive1==0 && derive2==0) q_lut = &this->q_d2xyz_lut[qidx*64];
    if (derive1==1 && derive2==1) q_lut = &this->q_xd2yz_lut[qidx*64];
    if (derive1==2 && derive2==2) q_lut = &this->q_xyd2z_lut[qidx*64];

    if (derive1==0 && derive2==1) q_lut = &this->q_dxdyz_lut[qidx*64];
    if (derive1==1 && derive2==0) q_lut = &this->q_dxdyz_lut[qidx*64];
	
    if (derive1==0 && derive2==2) q_lut = &this->q_dxydz_lut[qidx*64];
    if (derive1==2 && derive2==0) q_lut = &this->q_dxydz_lut[qidx*64];

    if (derive1==1 && derive2==2) q_lut = &this->q_xdydz_lut[qidx*64];
    if (derive1==2 && derive2==1) q_lut = &this->q_xdydz_lut[qidx*64];

    m = 0;
    for (k = 0; k < 4; k++) {
	for (j = 0; j < 4; j++) {
	    for (i = 0; i < 4; i++) {
		cidx = (p[2] + k) * bxf->cdims[1] * bxf->cdims[0]
		    + (p[1] + j) * bxf->cdims[0]
		    + (p[0] + i);
		cidx = cidx * 3;
		bscore->total_grad[cidx+0] += dc_dv[0] * q_lut[m];
		bscore->total_grad[cidx+1] += dc_dv[1] * q_lut[m];
		bscore->total_grad[cidx+2] += dc_dv[2] * q_lut[m];
		m ++;
	    }
	}
    }
}

void
bspline_regularize_hessian_update_grad_b (
    Bspline_score *bscore, 
    const Bspline_xform* bxf, 
    plm_long p[3], 
    plm_long qidx, 
    float dc_dv[3], 
    float *q_lut
)
{
    int i, j, k, m;
    int cidx;

    m = 0;
    for (k = 0; k < 4; k++) {
	for (j = 0; j < 4; j++) {
	    for (i = 0; i < 4; i++) {
		cidx = (p[2] + k) * bxf->cdims[1] * bxf->cdims[0]
		    + (p[1] + j) * bxf->cdims[0]
		    + (p[0] + i);
		cidx = cidx * 3;
		bscore->total_grad[cidx+0] += dc_dv[0] * q_lut[m];
		bscore->total_grad[cidx+1] += dc_dv[1] * q_lut[m];
		bscore->total_grad[cidx+2] += dc_dv[2] * q_lut[m];
		m ++;
	    }
	}
    }
}

#if defined (USE_FAST_CODE)
static double
update_score_and_grad (
    Bspline_score *bscore, 
    const Bspline_xform* bxf, 
    plm_long p[3], 
    plm_long qidx, 
    float grad_coeff, 
    float weight, // 2 or 1 for cross/non-cross derivatives
    float *qlut
)
{
    int d3;
    float dxyz[3];
    float dc_dv[3];
    double score = 0.0;

    bspline_regularize_hessian_component_b (dxyz, bxf, p, qidx, qlut);

    for (d3=0; d3<3; d3++) {
	score += weight * (dxyz[d3]*dxyz[d3]);
	dc_dv[d3] = weight * grad_coeff * dxyz[d3];
    }

    bspline_regularize_hessian_update_grad_b (bscore, bxf, p, qidx, 
	dc_dv, qlut);

    return score;
}
#endif

void
Bspline_regularize::compute_score_semi_analytic (
    Bspline_score *bscore, 
    const Regularization_parms *parms, 
    const Bspline_regularize *rst,
    const Bspline_xform* bxf
)
{
    double grad_score;
    plm_long ri, rj, rk;
    plm_long fi, fj, fk;
    plm_long p[3];
    plm_long q[3];
    plm_long qidx;
    plm_long num_vox;
    //double interval;
    float grad_coeff;
    //float raw_score;

    grad_score = 0;
    num_vox = bxf->roi_dim[0] * bxf->roi_dim[1] * bxf->roi_dim[2];
    grad_coeff = parms->lambda / num_vox;

    Plm_timer* timer = new Plm_timer;
    timer->start ();

    bscore->rmetric = 0.;

    //printf("---- YOUNG MODULUS %f\n", parms->lambda);

    for (rk = 0, fk = bxf->roi_offset[2]; rk < bxf->roi_dim[2]; rk++, fk++) {
	p[2] = rk / bxf->vox_per_rgn[2];
	q[2] = rk % bxf->vox_per_rgn[2];
	for (rj = 0, fj = bxf->roi_offset[1]; rj < bxf->roi_dim[1]; rj++, fj++) {
	    p[1] = rj / bxf->vox_per_rgn[1];
	    q[1] = rj % bxf->vox_per_rgn[1];
	    for (ri = 0, fi = bxf->roi_offset[0]; ri < bxf->roi_dim[0]; ri++, fi++) {
		p[0] = ri / bxf->vox_per_rgn[0];
		q[0] = ri % bxf->vox_per_rgn[0];
			
		qidx = volume_index (bxf->vox_per_rgn, q);
#if defined (USE_FAST_CODE)
		grad_score += update_score_and_grad (
		    bscore, bxf, p, qidx, grad_coeff, 1,
		    &this->q_d2xyz_lut[qidx*64]);
		grad_score += update_score_and_grad (
		    bscore, bxf, p, qidx, grad_coeff, 1,
		    &this->q_xd2yz_lut[qidx*64]);
		grad_score += update_score_and_grad (
		    bscore, bxf, p, qidx, grad_coeff, 1,
		    &this->q_xyd2z_lut[qidx*64]);
		grad_score += update_score_and_grad (
		    bscore, bxf, p, qidx, grad_coeff, 2,
		    &this->q_dxdyz_lut[qidx*64]);
		grad_score += update_score_and_grad (
		    bscore, bxf, p, qidx, grad_coeff, 2,
		    &this->q_dxydz_lut[qidx*64]);
		grad_score += update_score_and_grad (
		    bscore, bxf, p, qidx, grad_coeff, 2,
		    &this->q_xdydz_lut[qidx*64]);
#else
		for (int d1=0;d1<3;d1++) {
		    for (int d2=d1;d2<3;d2++) { //six different components only
			float dxyz[3];
			float dc_dv[3];
			bspline_regularize_hessian_component (
			    dxyz, bxf, p, qidx, d1, d2);

			/* Note: dxyz[i] = du_i/(dx_d1 dx_d2) */
			float weight;
			if (d1!=d2) weight = 2 ; else weight = 1;
			for (int d3=0;d3<3;d3++) {
			    grad_score += weight*(dxyz[d3]*dxyz[d3]);
			}	
			dc_dv[0] = weight * grad_coeff * dxyz[0];
			dc_dv[1] = weight * grad_coeff * dxyz[1];
			dc_dv[2] = weight * grad_coeff * dxyz[2];

			bspline_regularize_hessian_update_grad (
			    bscore, bxf, p, qidx, dc_dv, d1, d2);
		    }
		}
#endif
	    }
	}

	bscore->time_rmetric = timer->report ();
	//raw_score = grad_score / num_vox;
	grad_score *= (parms->lambda / num_vox);
	//printf ("        GRAD_COST %.4f   RAW_GRAD %.4f   [%.3f secs]\n", grad_score, raw_score, interval);
	bscore->rmetric += grad_score;
    }
    //printf ("SCORE=%.4f\n", bscore->score);
    delete timer;
}

void
Bspline_regularize::semi_analytic_init (
    const Bspline_xform* bxf
)
{
    this->create_qlut_grad (bxf, bxf->img_spacing, bxf->vox_per_rgn);
}
