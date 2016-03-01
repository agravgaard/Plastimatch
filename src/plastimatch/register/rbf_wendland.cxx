/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vnl/vnl_matrix_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/algo/vnl_svd.h"
#include "vnl/vnl_sample.h"

#include "landmark_warp.h"
#include "plm_image.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "raw_pointset.h"
#include "rbf_cluster.h"
#include "rbf_wendland.h"
#include "vf.h"
#include "volume.h"
#include "volume_macros.h"
#include "xform.h"

typedef struct rbf_params Rbf_parms;
struct rbf_params { // used to pass information to bspline_rbf_score
    float radius;    // radius of the RBF
    Bspline_parms *bparms;
    Volume *vector_field;
};

static float 
rbf_wendland_value (const float *rbf_center, const float *loc, float radius)
{
    float val, r, dx, dy, dz;

    dx = rbf_center[0]-loc[0];
    dy = rbf_center[1]-loc[1];
    dz = rbf_center[2]-loc[2];
    r = sqrt (dx*dx + dy*dy + dz*dz);
    r = r / radius;

    if (r>1) return 0.;
    val = (1-r)*(1-r)*(1-r)*(1-r)*(4*r+1.); // Wendland
    return val;
}

// find RBF coeff by solving the linear equations using ITK's SVD routine
// Output:
// parms->blm->rbf_coeff contains RBF coefficients
static void 
bspline_rbf_find_coeffs_noreg (
    float *coeff,                  /* Output */
    Landmark_warp *lw              /* Input */
)
{
    float rbfv;
    int i, j, d;
    int num_landmarks = lw->m_fixed_landmarks.get_count();

    typedef vnl_matrix <double> Vnl_matrix;
    typedef vnl_svd <double> SVDSolverType;
    Vnl_matrix A, b;

    //printf("Finding RBF coeffs, radius %.2f, Young modulus %e\n", 
	//lw->rbf_radius, lw->young_modulus);

    A.set_size (3 * num_landmarks, 3 * num_landmarks);
    A.fill(0.);

    b.set_size (3 * num_landmarks, 1);
    b.fill (0.0);

    // right-hand side
    for(i=0;i<num_landmarks;i++) {
	for(d=0;d<3;d++) {
	    b (3*i +d, 0) = 
		-(lw->m_fixed_landmarks.point(i,d) 
		    - lw->m_moving_landmarks.point(i,d));
	}
    }

    // matrix
    for(i=0;i<num_landmarks;i++) {
	for(j=0;j<num_landmarks;j++) {

    	    float rbf_center[3];
	    for (d=0; d<3; d++) {
	        rbf_center[d] = lw->m_fixed_landmarks.point(j,d);
	    }

	    rbfv = rbf_wendland_value (rbf_center, 
                lw->m_fixed_landmarks.point(i).p, 
                lw->adapt_radius[j]);

	    for(d=0;d<3;d++) A(3*i+d, 3*j+d) = rbfv ;

	}
    }

    //    A.print (std::cout);
    //    b.print (std::cout);

    SVDSolverType svd (A, 1e-6);
    Vnl_matrix x = svd.solve (b);

    //    x.print (std::cout);

    for (i=0; i<3*num_landmarks; i++) {
	coeff[i] = x(i,0);
    }

}

// find RBF coeff by solving the linear equations using ITK's SVD routine
// Output:
// parms->blm->rbf_coeff contains RBF coefficients
static void 
bspline_rbf_find_coeffs (
    float *coeff,                   /* Output */
    Landmark_warp *lw               /* Input */
)
{
    // Regularization for Wendland RBF not yet implemented
    //    bspline_rbf_find_coeffs_reg (coeff, lw);
    bspline_rbf_find_coeffs_noreg( coeff, lw);

    for (size_t i = 0; i < lw->m_fixed_landmarks.get_count(); i++) {
        printf("coeff %4d  %.4f %.4f %.4f\n", (int) i,
            coeff[3*i+0],
            coeff[3*i+1],
            coeff[3*i+2]);
    }
}

/*
  Adds RBF contributions to the vector field
  landmark_dxyz is not updated by this function
  Version without truncation: scan over the entire vf
  and add up all RBFs in each voxel
*/
void
rbf_wendland_update_vf (
    Volume *vf,                  /* Modified */
    Landmark_warp *lw,           /* Input */
    float *coeff                 /* Input */
)
{
    int lidx, d;
    plm_long ijk[3];
    float fxyz[3];
    float *vf_img;
    float rbf;
    int num_landmarks = lw->m_fixed_landmarks.get_count();

    printf("Wendland RBF, updating the vector field\n");

    if (vf->pix_type != PT_VF_FLOAT_INTERLEAVED )
        print_and_exit("Sorry, this type of vector field is not supported\n");

    vf_img = (float*) vf->img;

    LOOP_Z (ijk, fxyz, vf) {
        LOOP_Y (ijk, fxyz, vf) {
            LOOP_X (ijk, fxyz, vf) {
                /* Compute linear index of voxel */
                plm_long fv = volume_index (vf->dim, ijk);

                for (lidx=0; lidx < num_landmarks; lidx++) {
			
                    rbf = rbf_wendland_value (
                        lw->m_fixed_landmarks.point(lidx).p, 
                        fxyz, 
                        lw->adapt_radius[lidx]);

                    for (d=0; d<3; d++) {
                        vf_img[3*fv+d] += coeff[3*lidx+d] * rbf;
#if defined (commentout)
                        printf ("Adding: %d (%d %d %d) (%g * %g) %g\n", 
                            lidx, 
                            ijk[0], ijk[1], ijk[2],
                            coeff[3*lidx+d], rbf, 
                            coeff[3*lidx+d] * rbf);
#endif
                    }
                }
            }
        }
    }
}

void
rbf_wendland_warp (Landmark_warp *lw)
{
    float *coeff;
    float origin[3], spacing[3];
    plm_long dim[3];
    float direction_cosines[9];
    Volume::Pointer moving;
    Volume *vf_out, *warped_out;

    //printf ("Wendland Radial basis functions requested, radius %.2f\n", lw->rbf_radius);

    lw->adapt_radius = (float *)malloc(lw->m_fixed_landmarks.get_count() * sizeof(float));
    lw->cluster_id = (int *)malloc(lw->m_fixed_landmarks.get_count() * sizeof(int));

    if (lw->num_clusters > 0) {
        // cluster the landmarks; result in lw->cluster_id
        rbf_cluster_kmeans_plusplus( lw );
         // using cluster_id, fill in lw->adapt_radius
        rbf_cluster_find_adapt_radius( lw );
    }
    else {
        // use the specified radius
        for (size_t i = 0; i < lw->m_fixed_landmarks.get_count(); i++) {
            lw->adapt_radius[i]=lw->rbf_radius;
        }
    }

    for (size_t i = 0; i < lw->m_fixed_landmarks.get_count(); i++)
    {
        lw->adapt_radius[i]*=2;
        printf("%f\n", lw->adapt_radius[i]);
    }

    /* Solve for RBF weights */
    coeff = (float*) malloc (
        3 * lw->m_fixed_landmarks.get_count() * sizeof(float));
    bspline_rbf_find_coeffs (coeff, lw);

    /* Create output vector field */
    printf ("Creating output vf\n");
    lw->m_pih.get_origin (origin);
    lw->m_pih.get_spacing (spacing);
    lw->m_pih.get_dim (dim);
    lw->m_pih.get_direction_cosines (direction_cosines);
    vf_out = new Volume (dim, origin, spacing, direction_cosines, 
        PT_VF_FLOAT_INTERLEAVED, 3);

    printf ("Rendering vector field\n");
    rbf_wendland_update_vf (vf_out, lw, coeff);

    /* Create output (warped) image */
    printf ("Converting volume to float\n");
    moving = lw->m_input_img->get_volume_float ();

    printf ("Creating output vol\n");
    warped_out = new Volume (dim, origin, spacing, direction_cosines, 
        PT_FLOAT, 1);

    printf ("Warping image\n");
    vf_warp (warped_out, moving.get(), vf_out);

    printf ("Freeing coeff\n");
    free (coeff);

    /* Copy outputs to lw structure */
    lw->m_vf = new Xform;
    //lw->m_vf->set_gpuit_vf (vf_out);
    lw->m_vf->set_gpuit_vf (Volume::Pointer(vf_out));
    lw->m_warped_img = new Plm_image;
    lw->m_warped_img->set_volume (warped_out);

    printf ("Done with rbf_wendland_warp\n");
}
