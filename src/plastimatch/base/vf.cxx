/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "interpolate.h"
#include "interpolate_macros.h"
#include "plm_math.h"
#include "plmbase_config.h"
#include "volume_macros.h"
#include "vf.h"
#include "volume.h"

Volume*
vf_warp (Volume *vout, Volume *vin, Volume *vf)
{
    int d;
    plm_long ijk[3];
    float fxyz[3];
    float* vf_img = (float*) vf->img;
    float* vout_img;
    float* m_img = (float*) vin->img;

    const float* vin_proj = vin->get_proj();
    const float* vf_proj = vf->get_proj();
    const float* vin_step = vin->get_step();
    const float* vf_step = vf->get_step();

    printf ("Direction cosines: "
	"vin = %f %f %f ...\n"
	"vf = %f %f %f ...\n",
	vin->direction_cosines[0],
	vin->direction_cosines[1],
	vin->direction_cosines[2],
	vf->direction_cosines[0],
	vf->direction_cosines[1],
	vf->direction_cosines[2]
    );
    printf ("spac: "
	"vin = %f %f %f ...\n"
	"vf = %f %f %f ...\n",
	vin->spacing[0],
	vin->spacing[1],
	vin->spacing[2],
	vf->spacing[0],
	vf->spacing[1],
	vf->spacing[2]
    );
    printf ("proj: "
	"vin = %f %f %f ...\n"
	"vf = %f %f %f ...\n",
	vin_proj[3*0+0],
	vin_proj[3*0+1],
	vin_proj[3*0+2],
	vf_proj[3*0+0],
	vf_proj[3*0+1],
	vf_proj[3*0+2]
    );
    printf ("step: "
	"vin = %f %f %f ...\n"
	"vf = %f %f %f ...\n",
	vin_step[3*0+0],
	vin_step[3*0+1],
	vin_step[3*0+2],
	vf_step[3*0+0],
	vf_step[3*0+1],
	vf_step[3*0+2]
    );

    if (!vout) {
	vout = volume_clone_empty (vin);
    }
    vout_img = (float*) vout->img;
    
    /* Assumes size, spacing of vout same as size, spacing of vf */
    for (d = 0; d < 3; d++) {
	if (vout->dim[d] != vf->dim[d]) {
	    printf("Dimension mismatch between fixed and moving\n");
	    return 0;
	}
	if (vout->spacing[d] != vf->spacing[d]) {
	    printf("Resolutions mismatch between fixed and moving\n");
	    return 0;
	}
	if (vout->origin[d] != vf->origin[d]) {
	    printf("Origin mismatch between fixed and moving\n");
	    return 0;
	}
    }

    LOOP_Z (ijk, fxyz, vf) {
	LOOP_Y (ijk, fxyz, vf) {
	    LOOP_X (ijk, fxyz, vf) {
		/* Compute linear index of voxel */
		plm_long fv = volume_index (vf->dim, ijk);

		float *dxyz = &vf_img[3*fv];
		float mo_xyz[3] = {
		    fxyz[0] + dxyz[0] - vin->origin[0],
		    fxyz[1] + dxyz[1] - vin->origin[1],
		    fxyz[2] + dxyz[2] - vin->origin[2]
		};
		float m_val;
		float li_1[3];  /* Fraction of interpolant in lower index */
		float li_2[3];  /* Fraction of interpolant in upper index */
		float mijk[3];
		plm_long mijk_r[3], mijk_f[3];
		plm_long mvf;

		mijk[2] = PROJECT_Z(mo_xyz,vin->proj);
		mijk[1] = PROJECT_Y(mo_xyz,vin->proj);
		mijk[0] = PROJECT_X(mo_xyz,vin->proj);

                if (!vin->is_inside (mijk)) continue;

#if defined (commentout)
		/* Nearest neighbor */
		mijk_r[2] = ROUND_INT(mijk[2]);
		mijk_r[1] = ROUND_INT(mijk[1]);
		mijk_r[0] = ROUND_INT(mijk[0]);
		mv = (mk * vin->dim[1] + mj) * vin->dim[0] + mi;
		if (mk < 0 || mk >= vin->dim[2]) continue;
		if (mj < 0 || mj >= vin->dim[1]) continue;
		if (mi < 0 || mi >= vin->dim[0]) continue;
		vout_img[fv] = vin_img[mv];
#endif

		/* Get tri-linear interpolation fractions */
		li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, vin);

		/* Find linear index of "corner voxel" in moving image */
		mvf = volume_index (vin->dim, mijk_f);
		
		/* Compute moving image intensity using linear interpolation */
		LI_VALUE (m_val, 
		    li_1[0], li_2[0],
		    li_1[1], li_2[1],
		    li_1[2], li_2[2],
		    mvf, m_img, vin);

		/* Assign the value */
		vout_img[fv] = m_val;
	    }
	}
    }
    return vout;
}
