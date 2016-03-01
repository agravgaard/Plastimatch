/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "landmark_warp.h"
#include "raw_pointset.h"

Landmark_warp::Landmark_warp (void)
{
    default_val = 0;
    rbf_radius = 0;
    young_modulus = 0;
    num_clusters = 0;

    cluster_id = 0;
    adapt_radius = 0;

    m_warped_img = 0;
    m_vf = 0;
}

Landmark_warp::~Landmark_warp (void)
{
    if (cluster_id) free(cluster_id);
    if (adapt_radius) free(adapt_radius);
}

Landmark_warp*
landmark_warp_create (void)
{
    return new Landmark_warp;
}

void
landmark_warp_destroy (Landmark_warp *lw)
{
    delete lw;
}

/* GCS FIX: Oops.  This doesn't work because tps_xform is c++ code.
   If needed, we need to separate out Tps_xform as a separate c file. */
Landmark_warp*
landmark_warp_load_xform (const char *fn)
{
#if defined (commentout)
    Landmark_warp *lw;
    Tps_xform *tps;
    int i;

    tps = tps_xform_load (options->input_xform_fn);
    if (!tps) return 0;

    if (tps->num_tps_nodes <= 0) {
	tps_xform_destroy (tps);
	return 0;
    }

    lw = landmark_warp_create ();
    lw->fixed = pointset_create ();
    pointset_resize (lw->fixed, tps->num_tps_nodes);
    lw->moving = pointset_create ();
    pointset_resize (lw->moving, tps->num_tps_nodes);

    for (i = 0; i < tps->num_tps_nodes; i++) {
	lw->fixed[i*3 + 0] = tps->src[0];
	lw->fixed[i*3 + 1] = tps->src[1];
	lw->fixed[i*3 + 2] = tps->src[2];
	lw->moving[i*3 + 0] = tps->tgt[0];
	lw->moving[i*3 + 1] = tps->tgt[1];
	lw->moving[i*3 + 2] = tps->tgt[2];
    }

    /* Discard alpha values and image header. */
#endif

    return 0;
}



void
Landmark_warp::load_pointsets (
    const char *fixed_lm_fn, 
    const char *moving_lm_fn
)
{
    m_fixed_landmarks.load (fixed_lm_fn);
    m_moving_landmarks.load (moving_lm_fn);
}

Landmark_warp*
landmark_warp_load_pointsets (const char *fixed_lm_fn, const char *moving_lm_fn)
{
    Landmark_warp *lw;

    lw = landmark_warp_create ();
    lw->load_pointsets (fixed_lm_fn, moving_lm_fn);

    if (lw->m_fixed_landmarks.get_count() == 0 
        || lw->m_moving_landmarks.get_count() == 0)
    {
	landmark_warp_destroy (lw);
	return 0;
    }
    return lw;
}

/* 
NSh 2013-02-13 - moved the code below from cli/landmark_warp_main.cxx
*/

/*
  calculate voxel positions of landmarks given origin and pix_spacing
  output: landvox
  input: landmarks_mm and origin/spacing/dim
  NOTE: ROTATION IS NOT SUPPORTED! direction_cosines assumed to be 100 010 001.
*/
static void 
landmark_convert_mm_to_voxel (
    int *landvox, 
    const Labeled_pointset& landmarks_mm, 
    float *origin, 
    float *pix_spacing,
    plm_long *dim,
    const float *direction_cosines)
{
    for (int i = 0; i < landmarks_mm.get_count(); i++) {
	for (int d = 0; d < 3; d++) {
	    landvox[i*3 + d] = ROUND_INT (
		( landmarks_mm.point(i,d)
		    - origin[d]) / pix_spacing[d]);
	    if (landvox[i*3 + d] < 0 
		|| landvox[i*3 + d] >= dim[d])
	    {
		print_and_exit (
		    "Error, landmark %d outside of image for dim %d.\n"
		    "Location in vox = %d\n"
		    "Image boundary in vox = (%d %d)\n",
		    i, d, landvox[i*3 + d], 0, dim[d]-1);
	    }
	}
    }
}

/*Moves moving landmarks according to the current vector field.
  Output goes into lw->m_warped_landmarks
  LW = warped landmark
  We must solve LW + u(LW) = LM to get new LW, corresponding to current 
  vector field. Adapted from bspline_landmarks_warp(...) in 
  bspline_landmarks.c to use Landmark_warp */
void
calculate_warped_landmarks (Landmark_warp *lw)
{
    plm_long ri, rj, rk;
    plm_long fi, fj, fk, fv;
    plm_long mi, mj, mk;
    float fx, fy, fz;
    float mx, my, mz;
    int i, d, lidx;
    float dd, *vf, dxyz[3], *dd_min;
    
    int num_landmarks;
    int *landvox_mov, *landvox_fix, *landvox_warp;
    float *warped_landmarks;
    float *landmark_dxyz;
    Volume *vector_field;
    Volume::Pointer moving;
    plm_long fixed_dim[3];
    float fixed_spacing[3], fixed_origin[3], fixed_direction_cosines[9];

    num_landmarks = lw->m_fixed_landmarks.get_count();

    landvox_mov  = (int *)malloc( 3*num_landmarks * sizeof(int));
    landvox_fix  = (int *)malloc( 3*num_landmarks * sizeof(int));
    landvox_warp = (int *)malloc( 3*num_landmarks * sizeof(int));
    landmark_dxyz = (float *)malloc( 3*num_landmarks * sizeof(float));
    warped_landmarks = (float *)malloc( 3*num_landmarks * sizeof(float));

    if (lw->m_vf->get_type() != XFORM_GPUIT_VECTOR_FIELD) {
        Plm_image_header pih = lw->m_vf->get_plm_image_header();
        xform_to_gpuit_vf (lw->m_vf, lw->m_vf, &pih);
    }
    vector_field = lw->m_vf->get_gpuit_vf().get();
    moving = lw->m_input_img->get_volume_float ();

    // fixed dimensions set come from lw->m_pih //
    lw->m_pih.get_dim (fixed_dim);
    lw->m_pih.get_spacing (fixed_spacing);
    lw->m_pih.get_origin (fixed_origin);
    lw->m_pih.get_direction_cosines (fixed_direction_cosines);

    if (vector_field->pix_type != PT_VF_FLOAT_INTERLEAVED)
	print_and_exit ("Sorry, this type of vector field is not supported in landmarks_warp\n");	
    vf = (float *)vector_field->img;

    // fill in landvox'es //
    landmark_convert_mm_to_voxel (landvox_fix, lw->m_fixed_landmarks, 
	fixed_origin, fixed_spacing, fixed_dim, fixed_direction_cosines);
    landmark_convert_mm_to_voxel (landvox_mov, lw->m_moving_landmarks, 
	moving->origin, moving->spacing, moving->dim, 
	moving->direction_cosines);
    
    dd_min = (float *)malloc( num_landmarks * sizeof(float));
    for (d=0;d<num_landmarks;d++) dd_min[d] = 1e20F; //a very large number

    // roi_offset and roi_dim are not used here //
    for (rk = 0, fk = 0; rk < fixed_dim[2]; rk++, fk++) {
	fz = fixed_origin[2] + fixed_spacing[2] * fk;
	for (rj = 0, fj = 0; rj < fixed_dim[1]; rj++, fj++) {
	    fy = fixed_origin[1] + fixed_spacing[1] * fj;
	    for (ri = 0, fi = 0; ri < fixed_dim[0]; ri++, fi++) {
		fx = fixed_origin[0] + fixed_spacing[0] * fi;

		fv = fk * vector_field->dim[0] * vector_field->dim[1] 
		    + fj * vector_field->dim[0] +fi ;

		for (d=0;d<3;d++) dxyz[d] = vf[3*fv+d];

		// Find correspondence in moving image //
		mx = fx + dxyz[0];
		mi = ROUND_INT ((mx - moving->origin[0]) / moving->spacing[0]);
		if (mi < 0 || mi >= moving->dim[0]) continue;
		my = fy + dxyz[1];
		mj = ROUND_INT ((my - moving->origin[1]) / moving->spacing[1]);
		if (mj < 0 || mj >= moving->dim[1]) continue;
		mz = fz + dxyz[2];
		mk = ROUND_INT ((mz - moving->origin[2]) / moving->spacing[2]);
		if (mk < 0 || mk >= moving->dim[2]) continue;

		//saving vector field in a voxel which is the closest to landvox_mov
		//after being displaced by the vector field
		for (lidx = 0; lidx < num_landmarks; lidx++) {
		    dd = (mi - landvox_mov[lidx*3+0]) * (mi - landvox_mov[lidx*3+0])
			+(mj - landvox_mov[lidx*3+1]) * (mj - landvox_mov[lidx*3+1])
			+(mk - landvox_mov[lidx*3+2]) * (mk - landvox_mov[lidx*3+2]);
		    if (dd < dd_min[lidx]) { 
			dd_min[lidx]=dd;   
			for (d=0;d<3;d++) {
			    landmark_dxyz[3*lidx+d] = vf[3*fv+d];
			}
		    }
		} 
	    }
	}
    }

    for (i = 0; i < num_landmarks; i++) {
	for (d=0; d<3; d++) {
	    warped_landmarks[3*i+d]
		= lw->m_moving_landmarks.point(i,d)
		- landmark_dxyz[3*i+d];
	}
    }

    // calculate voxel positions of warped landmarks  //
    for (lidx = 0; lidx < num_landmarks; lidx++) {
	for (d = 0; d < 3; d++) {
	    landvox_warp[lidx*3 + d] 
		= ROUND_INT ((warped_landmarks[lidx*3 + d] 
			- fixed_origin[d]) / fixed_spacing[d]);
	    if (landvox_warp[lidx*3 + d] < 0 
		|| landvox_warp[lidx*3 + d] >= fixed_dim[d])
	    {
		print_and_exit (
		    "Error, warped landmark %d outside of fixed image for dim %d.\n"
		    "Location in vox = %d\n"
		    "Image boundary in vox = (%d %d)\n",
		    lidx, d, landvox_warp[lidx*3 + d], 0, fixed_dim[d]-1);
	    }
	} 
        lw->m_warped_landmarks.insert_lps (&warped_landmarks[3*lidx]);
    }

//debug only
    fy = 0;
    for (lidx = 0; lidx < num_landmarks; lidx++)
    {
	fx=0;
	for (d = 0; d < 3; d++) { 
	    fz = lw->m_fixed_landmarks.point(lidx,d) - lw->m_warped_landmarks.point(lidx,d);
	    fx += fz*fz;
	}
	printf("landmark %3d err %f mm\n", lidx, sqrt(fx));
	fy+=fx;
    }
    printf("landmark RMS err %f mm\n", sqrt(fy/num_landmarks));
// end debug only

    free (dd_min);
    free (landvox_mov);
    free (landvox_warp);
    free (landvox_fix);
    free (landmark_dxyz);
    free (warped_landmarks);
}



void
calculate_warped_landmarks_by_vf (Landmark_warp *lw , Volume *vector_field)

// same as calculate_warped_landmarks, but accepts vector_field directly,
// not through Xform lw->m_vf

/*Moves moving landmarks according to the current vector field.
  Output goes into lw->m_warped_landmarks
  LW = warped landmark
  We must solve LW + u(LW) = LM to get new LW, corresponding to current vector field.

  Adapted from bspline_landmarks_warp(...) in bspline_landmarks.c to use Landmark_warp */

{
    plm_long ri, rj, rk;
    plm_long fi, fj, fk, fv;
    plm_long mi, mj, mk;
    float fx, fy, fz;
    float mx, my, mz;
    int i, d, lidx;
    float dd, *vf, dxyz[3], *dd_min;
    
    int num_landmarks;
    int *landvox_mov, *landvox_fix, *landvox_warp;
    float *warped_landmarks;
    float *landmark_dxyz;
    Volume::Pointer moving;
    plm_long fixed_dim[3];
    float fixed_spacing[3], fixed_origin[3], fixed_direction_cosines[9];

    num_landmarks = lw->m_fixed_landmarks.get_count();

    landvox_mov  = (int *)malloc( 3*num_landmarks * sizeof(int));
    landvox_fix  = (int *)malloc( 3*num_landmarks * sizeof(int));
    landvox_warp = (int *)malloc( 3*num_landmarks * sizeof(int));
    landmark_dxyz = (float *)malloc( 3*num_landmarks * sizeof(float));
    warped_landmarks = (float *)malloc( 3*num_landmarks * sizeof(float));

    moving = lw->m_input_img->get_volume_float ();

    // fixed dimensions set come from lw->m_pih //
    lw->m_pih.get_dim (fixed_dim);
    lw->m_pih.get_spacing (fixed_spacing);
    lw->m_pih.get_origin (fixed_origin);
    lw->m_pih.get_direction_cosines (fixed_direction_cosines);

    if (vector_field->pix_type != PT_VF_FLOAT_INTERLEAVED)
	print_and_exit ("Sorry, this type of vector field is not supported in landmarks_warp\n");	
    vf = (float *)vector_field->img;

    // fill in landvox'es //
    landmark_convert_mm_to_voxel (landvox_fix, lw->m_fixed_landmarks, 
	fixed_origin, fixed_spacing, fixed_dim, fixed_direction_cosines);
    landmark_convert_mm_to_voxel (landvox_mov, lw->m_moving_landmarks, 
	moving->origin, moving->spacing, moving->dim, 
	moving->direction_cosines);
    
    printf("done landvox; n=%d\n", num_landmarks);

    printf("fix offs %f %f %f\n",  fixed_origin[0],fixed_origin[1],fixed_origin[2]);
    printf("fix dim  %d %d %d\n",  
        (int) fixed_dim[0], (int) fixed_dim[1], (int) fixed_dim[2]);
    printf("mov offs %f %f %f\n", moving->origin[0], 
        moving->origin[1], moving->origin[2]);
    printf("vf dim  %d %d %d\n", (int) vector_field->dim[0], 
        (int) vector_field->dim[1], (int) vector_field->dim[2]);

    for(i=0;i<num_landmarks;i++) {
        printf("%d %d %d    %d %d %d\n",
            landvox_fix[3*i+0],landvox_fix[3*i+1],landvox_fix[3*i+2],
            landvox_mov[3*i+0],landvox_mov[3*i+1],landvox_mov[3*i+2]
        );
    }

    dd_min = (float *)malloc( num_landmarks * sizeof(float));
    for (d=0;d<num_landmarks;d++) dd_min[d] = 1e20F; //a very large number

    // roi_offset and roi_dim are not used here //

    for (rk = 0, fk = 0; rk < fixed_dim[2]; rk++, fk++) {
	fz = fixed_origin[2] + fixed_spacing[2] * fk;
	for (rj = 0, fj = 0; rj < fixed_dim[1]; rj++, fj++) {
	    fy = fixed_origin[1] + fixed_spacing[1] * fj;
	    for (ri = 0, fi = 0; ri < fixed_dim[0]; ri++, fi++) {
		fx = fixed_origin[0] + fixed_spacing[0] * fi;

		fv = fk * vector_field->dim[0] * vector_field->dim[1] 
		    + fj * vector_field->dim[0] +fi ;

		for (d=0;d<3;d++) dxyz[d] = vf[3*fv+d];

		// Find correspondence in moving image //
		mx = fx + dxyz[0];
		mi = ROUND_INT ((mx - moving->origin[0]) / moving->spacing[0]);
		if (mi < 0 || mi >= moving->dim[0]) continue;
		my = fy + dxyz[1];
		mj = ROUND_INT ((my - moving->origin[1]) / moving->spacing[1]);
		if (mj < 0 || mj >= moving->dim[1]) continue;
		mz = fz + dxyz[2];
		mk = ROUND_INT ((mz - moving->origin[2]) / moving->spacing[2]);
		if (mk < 0 || mk >= moving->dim[2]) continue;

		//saving vector field in a voxel which is the closest to landvox_mov
		//after being displaced by the vector field
		for (lidx = 0; lidx < num_landmarks; lidx++) {
		    dd = (mi - landvox_mov[lidx*3+0]) * (mi - landvox_mov[lidx*3+0])
			+(mj - landvox_mov[lidx*3+1]) * (mj - landvox_mov[lidx*3+1])
			+(mk - landvox_mov[lidx*3+2]) * (mk - landvox_mov[lidx*3+2]);
		    if (dd < dd_min[lidx]) { 
			dd_min[lidx]=dd;   
			for (d=0;d<3;d++) {
			    landmark_dxyz[3*lidx+d] = vf[3*fv+d];
			}
		    }
		} 
	    }
	}
    }

    for (i = 0; i < num_landmarks; i++) {
	for (d=0; d<3; d++) {
	    warped_landmarks[3*i+d]
		= lw->m_moving_landmarks.point(3,d)
		- landmark_dxyz[3*i+d];
	}
    }

    // calculate voxel positions of warped landmarks  //
    for (lidx = 0; lidx < num_landmarks; lidx++) {
	for (d = 0; d < 3; d++) {
	    landvox_warp[lidx*3 + d] 
		= ROUND_INT ((warped_landmarks[lidx*3 + d] 
			- fixed_origin[d]) / fixed_spacing[d]);
	    if (landvox_warp[lidx*3 + d] < 0 
		|| landvox_warp[lidx*3 + d] >= fixed_dim[d])
	    {
		print_and_exit (
		    "Error, warped landmark %d outside of fixed image for dim %d.\n"
		    "Location in vox = %d\n"
		    "Image boundary in vox = (%d %d)\n",
		    lidx, d, landvox_warp[lidx*3 + d], 0, fixed_dim[d]-1);
	    }
	} 
        lw->m_warped_landmarks.insert_lps (&warped_landmarks[3*lidx]);
    }

    printf("done warping, printing rms\n");

//debug only
    fy = 0;
    for (lidx = 0; lidx < num_landmarks; lidx++)
    {
	fx=0;
	for (d = 0; d < 3; d++) { 
	    fz = (lw->m_fixed_landmarks.point(lidx,d) - lw->m_warped_landmarks.point(lidx,d));
	    fx += fz*fz;
	}
	printf("landmark %3d err %f mm\n", lidx, sqrt(fx));
	fy+=fx;
    }
    printf("landmark RMS err %f mm\n", sqrt(fy/num_landmarks));
// end debug only

    free (dd_min);
    free (landvox_mov);
    free (landvox_warp);
    free (landvox_fix);
    free (landmark_dxyz);
    free (warped_landmarks);
}
