/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bspline.h"
#include "bspline_macros.h"
#include "bspline_regularize.h"
#include "bspline_regularize_numeric.h"
#include "bspline_score.h"
#include "bspline_xform.h"
#include "logfile.h"
#include "mha_io.h"
#include "plm_math.h"
#include "plm_timer.h"
#include "print_and_exit.h"
#include "volume_macros.h"
#include "volume.h"

/* Flavor 'a' */
static void
compute_score_numeric_internal (
    Bspline_score *bscore, 
    const Regularization_parms *parms, 
    const Bspline_regularize *rst,
    const Bspline_xform* bxf,
    const Volume* vol
)
{
    plm_long i, j, k;
    int c;
    float *img = (float*) vol->img;

    float dx = vol->spacing[0];
    float dy = vol->spacing[1];
    float dz = vol->spacing[2];

    float dxdydz = dx * dy * dz;

    float inv_dxdx = 1.0f / (dx * dx);
    float inv_dydy = 1.0f / (dy * dy);
    float inv_dzdz = 1.0f / (dz * dz);

    float inv_dxdy = 0.25f / (dx*dy);
    float inv_dxdz = 0.25f / (dx*dz);
    float inv_dydz = 0.25f / (dy*dz);

    /* Index of current point-of-interest (POI) */
    int idx_poi;

    /* Indices of POI's SxS neighbors */
    int idx_in, idx_ip;
    int idx_jn, idx_jp;
    int idx_kn, idx_kp;

    /* Indicies of POI's diagonal neighbors */
    int idx_injn, idx_injp, idx_ipjn, idx_ipjp;
    int idx_inkn, idx_inkp, idx_ipkn, idx_ipkp;
    int idx_jnkn, idx_jnkp, idx_jpkn, idx_jpkp;

    /* Deformation vector @ POI */
    float *vec_poi;

    /* Vectors of POI's SxS neighbors */
    float *vec_in, *vec_ip;
    float *vec_jn, *vec_jp;
    float *vec_kn, *vec_kp;

    /* Vectors of POI's diagonal neighbors */
    float *vec_injn, *vec_injp;
    float *vec_ipjn, *vec_ipjp;
    float *vec_inkn, *vec_inkp;
    float *vec_ipkn, *vec_ipkp;
    float *vec_jnkn, *vec_jnkp;
    float *vec_jpkn, *vec_jpkp;

    /* Vector's partial spatial derivatives */
    float d2_dx2[3],  d2_dxdy[3];
    float d2_dy2[3],  d2_dxdz[3];
    float d2_dz2[3],  d2_dydz[3];

    /* Voxel-specific stiffness */
    const float *fsimg = 0;
    if (rst->fixed_stiffness) {
        fsimg = rst->fixed_stiffness->get_raw<float>();
    }

    /* Square of 2nd derivative */
    float d2_sq;

    /* Smoothness */
    float S;

#if defined (DEBUG)
    FILE* fp[3];
    printf ("Warning: compiled with DEBUG : writing to to files:\n");
    printf ("  d2ux_dxy_sq.txt\n"); fp[0] = fopen ("d2ux_dxdy_sq.txt", "w");
    printf ("  d2uy_dxy_sq.txt\n"); fp[1] = fopen ("d2uy_dxdy_sq.txt", "w");
    printf ("  d2uz_dxy_sq.txt\n"); fp[2] = fopen ("d2uz_dxdy_sq.txt", "w");
#endif

    Plm_timer* timer = new Plm_timer;
    timer->start ();

    S = 0.0f;
    for (k = 1; k < vol->dim[2]-1; k++) {
        for (j = 1; j < vol->dim[1]-1; j++) {
            for (i = 1; i < vol->dim[0]-1; i++) {
		float dc_dv[3] = { 0, 0, 0 };
		float dc_dv_in[3] = { 0, 0, 0 };
		float dc_dv_ip[3] = { 0, 0, 0 };
		float dc_dv_jn[3] = { 0, 0, 0 };
		float dc_dv_jp[3] = { 0, 0, 0 };
		float dc_dv_kn[3] = { 0, 0, 0 };
		float dc_dv_kp[3] = { 0, 0, 0 };
		float dc_dv_injn[3] = { 0, 0, 0 };
		float dc_dv_injp[3] = { 0, 0, 0 };
		float dc_dv_ipjn[3] = { 0, 0, 0 };
		float dc_dv_ipjp[3] = { 0, 0, 0 };
		float dc_dv_inkn[3] = { 0, 0, 0 };
		float dc_dv_inkp[3] = { 0, 0, 0 };
		float dc_dv_ipkn[3] = { 0, 0, 0 };
		float dc_dv_ipkp[3] = { 0, 0, 0 };
		float dc_dv_jnkn[3] = { 0, 0, 0 };
		float dc_dv_jnkp[3] = { 0, 0, 0 };
		float dc_dv_jpkn[3] = { 0, 0, 0 };
		float dc_dv_jpkp[3] = { 0, 0, 0 };

		/* Load indicies relevant to current POI */
                idx_poi = volume_index (vol->dim, i, j, k);

                idx_in = volume_index (vol->dim, i-1,   j,   k);
                idx_ip = volume_index (vol->dim, i+1,   j,   k);
                idx_jn = volume_index (vol->dim,   i, j-1,   k);
                idx_jp = volume_index (vol->dim,   i, j+1,   k);
                idx_kn = volume_index (vol->dim,   i,   j, k-1);
                idx_kp = volume_index (vol->dim,   i,   j, k+1);

                idx_injn = volume_index (vol->dim, i-1, j-1,   k);
                idx_injp = volume_index (vol->dim, i-1, j+1,   k);
                idx_ipjn = volume_index (vol->dim, i+1, j-1,   k);
                idx_ipjp = volume_index (vol->dim, i+1, j+1,   k);
                idx_inkn = volume_index (vol->dim, i-1,   j, k-1);
                idx_inkp = volume_index (vol->dim, i-1,   j, k+1);
                idx_ipkn = volume_index (vol->dim, i+1,   j, k-1);
                idx_ipkp = volume_index (vol->dim, i+1,   j, k+1);
                idx_jnkn = volume_index (vol->dim,   i, j-1, k-1);
                idx_jnkp = volume_index (vol->dim,   i, j-1, k+1);
                idx_jpkn = volume_index (vol->dim,   i, j+1, k-1);
                idx_jpkp = volume_index (vol->dim,   i, j+1, k+1);

                /* Load vectors relevant to current POI */
                vec_poi = &img[3*idx_poi];

                vec_in = &img[3*idx_in]; vec_ip = &img[3*idx_ip];
                vec_jn = &img[3*idx_jn]; vec_jp = &img[3*idx_jp];
                vec_kn = &img[3*idx_kn]; vec_kp = &img[3*idx_kp];

                vec_injn = &img[3*idx_injn]; vec_injp = &img[3*idx_injp];
                vec_ipjn = &img[3*idx_ipjn]; vec_ipjp = &img[3*idx_ipjp];
                vec_inkn = &img[3*idx_inkn]; vec_inkp = &img[3*idx_inkp];
                vec_ipkn = &img[3*idx_ipkn]; vec_ipkp = &img[3*idx_ipkp];
                vec_jnkn = &img[3*idx_jnkn]; vec_jnkp = &img[3*idx_jnkp];
                vec_jpkn = &img[3*idx_jpkn]; vec_jpkp = &img[3*idx_jpkp];

		/* Get stiffness */
                float stiffness = 1.0;
                if (fsimg) {
                    stiffness = fsimg[idx_poi];
                }

                /* Compute components */
                d2_sq = 0.0f;
                for (c=0; c<3; c++) {
                    d2_dx2[c] = inv_dxdx 
			* (vec_ip[c] - 2.0f*vec_poi[c] + vec_in[c]);
                    d2_dy2[c] = inv_dydy 
			* (vec_jp[c] - 2.0f*vec_poi[c] + vec_jn[c]);
                    d2_dz2[c] = inv_dzdz 
			* (vec_kp[c] - 2.0f*vec_poi[c] + vec_kn[c]);

                    d2_dxdy[c] = inv_dxdy * (
                        vec_injn[c] - vec_injp[c] - vec_ipjn[c] + vec_ipjp[c]);
                    d2_dxdz[c] = inv_dxdz * (
                        vec_inkn[c] - vec_inkp[c] - vec_ipkn[c] + vec_ipkp[c]);
                    d2_dydz[c] = inv_dydz * (
                        vec_jnkn[c] - vec_jnkp[c] - vec_jpkn[c] + vec_jpkp[c]);

		    /* Accumulate score for this component, for this voxel */
                    d2_sq += 
			d2_dx2[c]*d2_dx2[c] + 
			d2_dy2[c]*d2_dy2[c] +
			d2_dz2[c]*d2_dz2[c] + 
			2.0f * (
			    d2_dxdy[c]*d2_dxdy[c] +
			    d2_dxdz[c]*d2_dxdz[c] +
			    d2_dydz[c]*d2_dydz[c]
                        );

		    /* Accumulate grad for this component, for this voxel */
		    dc_dv[c] = 
			- 4 * dxdydz * inv_dxdx * d2_dx2[c] 
			- 4 * dxdydz * inv_dydy * d2_dy2[c] 
			- 4 * dxdydz * inv_dzdz * d2_dz2[c];

		    dc_dv_in[c] = 2 * dxdydz * inv_dxdx * d2_dx2[c];
		    dc_dv_ip[c] = 2 * dxdydz * inv_dxdx * d2_dx2[c];
		    dc_dv_jn[c] = 2 * dxdydz * inv_dydy * d2_dy2[c];
		    dc_dv_jp[c] = 2 * dxdydz * inv_dydy * d2_dy2[c];
		    dc_dv_kn[c] = 2 * dxdydz * inv_dzdz * d2_dz2[c];
		    dc_dv_kp[c] = 2 * dxdydz * inv_dzdz * d2_dz2[c];

		    dc_dv_injn[c] = + 4 * dxdydz * inv_dxdy * d2_dxdy[c];
		    dc_dv_injp[c] = - 4 * dxdydz * inv_dxdy * d2_dxdy[c];
		    dc_dv_ipjn[c] = - 4 * dxdydz * inv_dxdy * d2_dxdy[c];
		    dc_dv_ipjp[c] = + 4 * dxdydz * inv_dxdy * d2_dxdy[c];
		    dc_dv_inkn[c] = + 4 * dxdydz * inv_dxdz * d2_dxdz[c];
		    dc_dv_inkp[c] = - 4 * dxdydz * inv_dxdz * d2_dxdz[c];
		    dc_dv_ipkn[c] = - 4 * dxdydz * inv_dxdz * d2_dxdz[c];
		    dc_dv_ipkp[c] = + 4 * dxdydz * inv_dxdz * d2_dxdz[c];
		    dc_dv_jnkn[c] = + 4 * dxdydz * inv_dydz * d2_dydz[c];
		    dc_dv_jnkp[c] = - 4 * dxdydz * inv_dydz * d2_dydz[c];
		    dc_dv_jpkn[c] = - 4 * dxdydz * inv_dydz * d2_dydz[c];
		    dc_dv_jpkp[c] = + 4 * dxdydz * inv_dydz * d2_dydz[c];

                    /* Apply stiffness to components */
                    if (fsimg) {
                        dc_dv[c] *= stiffness;

                        dc_dv_in[c] *= stiffness;
                        dc_dv_ip[c] *= stiffness;
                        dc_dv_jn[c] *= stiffness;
                        dc_dv_jp[c] *= stiffness;
                        dc_dv_kn[c] *= stiffness;
                        dc_dv_kp[c] *= stiffness;

                        dc_dv_injn[c] *= stiffness;
                        dc_dv_injp[c] *= stiffness;
                        dc_dv_ipjn[c] *= stiffness;
                        dc_dv_ipjp[c] *= stiffness;
                        dc_dv_inkn[c] *= stiffness;
                        dc_dv_inkp[c] *= stiffness;
                        dc_dv_ipkn[c] *= stiffness;
                        dc_dv_ipkp[c] *= stiffness;
                        dc_dv_jnkn[c] *= stiffness;
                        dc_dv_jnkp[c] *= stiffness;
                        dc_dv_jpkn[c] *= stiffness;
                        dc_dv_jpkp[c] *= stiffness;
                    }

#if defined (DEBUG)
                    fprintf (fp[c], "(%i,%i,%i) : %15e\n", 
			i,j,k, (d2_dxdy[c]*d2_dxdy[c]));
#endif
                }
                /* Update score */
                S += stiffness * d2_sq;

		/* Update gradient */
		int pidx, qidx;
		pidx = get_region_index  (i  , j  , k  , bxf);
		qidx = get_region_offset (i  , j  , k  , bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv);

		pidx = get_region_index  (i-1, j  , k  , bxf);
		qidx = get_region_offset (i-1, j  , k  , bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_in);
		pidx = get_region_index  (i+1, j  , k  , bxf);
		qidx = get_region_offset (i+1, j  , k  , bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_ip);
		pidx = get_region_index  (i  , j-1, k  , bxf);
		qidx = get_region_offset (i  , j-1, k  , bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_jn);
		pidx = get_region_index  (i  , j+1, k  , bxf);
		qidx = get_region_offset (i  , j+1, k  , bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_jp);
		pidx = get_region_index  (i  , j  , k-1, bxf);
		qidx = get_region_offset (i  , j  , k-1, bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_kn);
		pidx = get_region_index  (i  , j  , k+1, bxf);
		qidx = get_region_offset (i  , j  , k+1, bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_kp);

		pidx = get_region_index  (i-1, j-1, k  , bxf);
		qidx = get_region_offset (i-1, j-1, k  , bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_injn);
		pidx = get_region_index  (i-1, j+1, k  , bxf);
		qidx = get_region_offset (i-1, j+1, k  , bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_injp);
		pidx = get_region_index  (i+1, j-1, k  , bxf);
		qidx = get_region_offset (i+1, j-1, k  , bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_ipjn);
		pidx = get_region_index  (i+1, j+1, k  , bxf);
		qidx = get_region_offset (i+1, j+1, k  , bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_ipjp);
		pidx = get_region_index  (i-1, j  , k-1, bxf);
		qidx = get_region_offset (i-1, j  , k-1, bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_inkn);
		pidx = get_region_index  (i-1, j  , k+1, bxf);
		qidx = get_region_offset (i-1, j  , k+1, bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_inkp);
		pidx = get_region_index  (i+1, j  , k-1, bxf);
		qidx = get_region_offset (i+1, j  , k-1, bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_ipkn);
		pidx = get_region_index  (i+1, j  , k+1, bxf);
		qidx = get_region_offset (i+1, j  , k+1, bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_ipkp);
		pidx = get_region_index  (i  , j-1, k-1, bxf);
		qidx = get_region_offset (i  , j-1, k-1, bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_jnkn);
		pidx = get_region_index  (i  , j-1, k+1, bxf);
		qidx = get_region_offset (i  , j-1, k+1, bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_jnkp);
		pidx = get_region_index  (i  , j+1, k-1, bxf);
		qidx = get_region_offset (i  , j+1, k-1, bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_jpkn);
		pidx = get_region_index  (i  , j+1, k+1, bxf);
		qidx = get_region_offset (i  , j+1, k+1, bxf);
		bscore->update_total_grad_b (bxf, pidx, qidx, dc_dv_jpkp);
            }
        }
    }

    /* Integrate */
    S *= dx*dy*dz;

#if defined (DEBUG)
    for (i=0; i<3; i++) {
        fclose(fp[i]);
    }
#endif

    bscore->rmetric += S;
    bscore->time_rmetric = timer->report ();
    delete timer;
}

void
Bspline_regularize::compute_score_numeric (
    Bspline_score *bscore, 
    const Regularization_parms *parms, 
    const Bspline_regularize *rst,
    const Bspline_xform* bxf)
{
    Volume *vf = bspline_compute_vf (bxf);
    bscore->rmetric = 0.0;
    compute_score_numeric_internal (bscore, parms, rst, bxf, vf);
    delete vf;
}

void
Bspline_regularize::numeric_init (
    const Bspline_xform* bxf
)
{
}
