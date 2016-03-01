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
#include "rbf_gauss.h"
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
rbf_value (const float *rbf_center, const float *loc, float radius)
{
    float val, r, dx, dy, dz;

    dx = rbf_center[0]-loc[0];
    dy = rbf_center[1]-loc[1];
    dz = rbf_center[2]-loc[2];
    r = sqrt (dx*dx + dy*dy + dz*dz);
    r = r / radius;

    //   if (r>1) return 0.;
    //   val = (1-r)*(1-r)*(1-r)*(1-r)*(4*r+1.); // Wendland
    val = exp( -r*r );   
    return val;
}

/*
Analytic expression for the integral of squared second derivatives
of the vector field of a single Gaussian RBF of radius c
*/
#if defined (commentout)
static float rbf_gauss_secderiv_self (float c)
{
    float factor = 1.968701243; // pow( M_PI/2, 3./2);
    return 15*factor/c;
} 
#endif
  
/*
  Analytic expression for the integral of squared second derivatives
  of the vector field of two Gaussian RBFs of radii c,
  separated by squared distance a2 = (x1-x2)*(x1-x2)+...
  (one-half of the overlap integral only).
*/
#if defined (commentout)
static float rbf_gauss_secderiv_cross (float c, float a2)
{
    float factor = 1.968701243; // pow( M_PI/2, 3./2);
    return factor/c*exp(-a2/(2*c*c))*(-10. + ( a2/(c*c)-5.)*(a2/(c*c)-5.) );
}
#endif

#if defined (commentout)
static float 
bspline_rbf_analytic_integral (Bspline_parms *parms, float *pix_spacing)
{
    Bspline_landmarks *blm = parms->landmarks;
    int i,j,d;
    float a2, s = 0.;

    for(i=0;i<blm->num_landmarks;i++)
	for(d=0;d<3;d++)
	    s += blm->rbf_coeff[3*i+d] * blm->rbf_coeff[3*i+d] *
		rbf_gauss_secderiv_self( parms->rbf_radius );

    if (blm->num_landmarks>1)
	for(i=0;i<blm->num_landmarks;i++)
	    for(j=i+1;j<blm->num_landmarks;j++)
		for(d=0;d<3;d++)
		{
		    a2 = (blm->landvox_fix[3*i+0]-blm->landvox_fix[3*j+0])*
			(blm->landvox_fix[3*i+0]-blm->landvox_fix[3*j+0])*
			pix_spacing[0] * pix_spacing[0] +
			(blm->landvox_fix[3*i+1]-blm->landvox_fix[3*j+1])*
			(blm->landvox_fix[3*i+1]-blm->landvox_fix[3*j+1])*
			pix_spacing[1] * pix_spacing[1] +
			(blm->landvox_fix[3*i+2]-blm->landvox_fix[3*j+2])*
			(blm->landvox_fix[3*i+2]-blm->landvox_fix[3*j+2])*
			pix_spacing[2] * pix_spacing[2];
	
		    s += 2. * blm->rbf_coeff[3*i+d] * blm->rbf_coeff[3*j+d] *
			rbf_gauss_secderiv_cross( parms->rbf_radius, a2 );
		}

    return s;
}
#endif

#if defined (commentout)
/*
Test RBF solution by calculating the residual error 
of landmark matching 
*/
static void bspline_rbf_test_solution(Volume *vector_field, Bspline_parms *parms)
{
    Bspline_landmarks *blm = parms->landmarks;
    float *vf;
    float totdist, dx, dist, maxdist; 
    int i, j, d, dd, fv, rbfcenter[3];

    Rbf_parms *rbf_par;

    rbf_par = (Rbf_parms *)malloc( sizeof(Rbf_parms));
    rbf_par->radius = parms->rbf_radius;
    rbf_par->bparms = parms;
    rbf_par->vector_field = vector_field;

    if (vector_field->pix_type != PT_VF_FLOAT_INTERLEAVED )
	print_and_exit("Sorry, this type of vector field is not supported\n");

    //fill in vector field at fixed landmark location
    vf = (float*) vector_field->img;

    for(i=0;i<blm->num_landmarks;i++) {
	fv = blm->landvox_fix[3*i+2] * vector_field->dim[0] * vector_field->dim[1] 
	    +blm->landvox_fix[3*i+1] * vector_field->dim[0] 
	    +blm->landvox_fix[3*i+0] ;

	for(d=0;d<3;d++) blm->landmark_dxyz[3*i+d] = vf[3*fv+d];
    }

    maxdist = -1;
    totdist = 0;
    for(i=0;i<blm->num_landmarks;i++)
    {
	dist = 0;
	for(d = 0; d<3; d++)
	{
	    dx = blm->fixed_landmarks->points[3*i+d] 
		+ blm->landmark_dxyz[3*i+d] 
		- blm->moving_landmarks->points[3*i+d];
	    for(j=0;j<blm->num_landmarks;j++)
	    {

		for(dd=0;dd<3;dd++) rbfcenter[dd] = blm->landvox_fix[3*j+dd];
		dx += blm->rbf_coeff[3*j+d]* rbf_value( rbfcenter, 
		    blm->landvox_fix[3*i+0],
		    blm->landvox_fix[3*i+1],
		    blm->landvox_fix[3*i+2], 
		    rbf_par->radius,
		    rbf_par->vector_field->pix_spacing );
					
	    }
	    dist = dist + dx*dx;
	}
	dist = sqrt(dist);
	totdist += dist;
	if (dist>maxdist) maxdist = dist;
	//printf("L%2d  %f\n", i, dist);
    }

    totdist/=blm->num_landmarks;

    printf("Landmark mismatch: AVERAGE %f, MAX %f\n", totdist, maxdist);
}
#endif

// find RBF coeff by solving the linear equations using ITK's SVD routine
// using analytical regularization of Gaussian exp(-r*r) RBFs to minimize
// the sum of squares of second derivatives.
// Output:
// parms->blm->rbf_coeff contains RBF coefficients
static void 
bspline_rbf_find_coeffs_reg (
    float *coeff,                  /* Output */
    Landmark_warp *lw              /* Input */
)
{
    float rbfv1, rbfv2;
    int i, j, k, d;
    float rbf_prefactor, reg_term, r2, tmp;
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
    for (i=0; i<num_landmarks; i++) {
	for (j=0; j<num_landmarks; j++) {
	    float rbf_center[3];
	    for (d=0; d<3; d++) {
		rbf_center[d] = lw->m_fixed_landmarks.point(i,d);
	    }
	    rbfv1 = rbf_value (rbf_center, 
		lw->m_fixed_landmarks.point(j).p, 
		lw->adapt_radius[j]);
		
	    for (d=0;d<3;d++) {
		b (3*i +d, 0) -= rbfv1 
		    * (lw->m_fixed_landmarks.point(j,d) 
			- lw->m_moving_landmarks.point(j,d));
	    }
	}
    }

    // matrix
    for (i = 0; i < num_landmarks; i++) {
	for (j = 0; j < num_landmarks; j++) {
	    tmp = 0;
	    for (k = 0; k < num_landmarks; k++) {

		float rbf_center[3];
		for (d=0; d<3; d++) {
		    rbf_center[d] = lw->m_fixed_landmarks.point(k,d);
		}

		rbfv1 = rbf_value (rbf_center, 
		    lw->m_fixed_landmarks.point(i).p, 
		    lw->adapt_radius[k]);
		rbfv2 = rbf_value (rbf_center, 
		    lw->m_fixed_landmarks.point(j).p, 
		    lw->adapt_radius[k]);

		tmp += rbfv1*rbfv2;
	    }

	    for (d=0;d<3;d++)
	    {
		A(3*i+d, 3*j+d) = tmp;
		//printf("i j d = %d %d %d   A = %f\n", i,j,d, A(3*i+d, 3*j+d));
	    }

	}
    }
    //add regularization terms to the matrix
    rbf_prefactor = sqrt(M_PI/2.)*sqrt(M_PI/2.)*sqrt(M_PI/2.)/lw->rbf_radius;
    for (d=0;d<3;d++) {
	for (i=0;i<num_landmarks;i++) {
	    for (j=0;j<num_landmarks;j++) {

		tmp = A(3*i+d, 3*j+d);
		reg_term = 0.;			
			
		if (i==j) {
		    reg_term = rbf_prefactor * 15.;
		}
		else
		{
		    float dx = lw->m_fixed_landmarks.point(i,0)
			- lw->m_fixed_landmarks.point(j,0);
		    float dy = lw->m_fixed_landmarks.point(i,1)
			- lw->m_fixed_landmarks.point(j,1);
		    float dz = lw->m_fixed_landmarks.point(i,2)
			- lw->m_fixed_landmarks.point(j,2);

		    // r2 = sq distance between landmarks i,j in mm
		    r2 = dx * dx + dy * dy + dz * dz;
		    r2 = r2 / (lw->adapt_radius[i] * lw->adapt_radius[j]);
		    reg_term = rbf_prefactor * exp(-r2/2.) 
			* (-10 + (r2-5.)*(r2-5.));
		}
		//printf("i j d = %d %d %d  regterm = %f\n", i,j,d, reg_term);
		A (3*i+d,3*j+d) = tmp + reg_term * lw->young_modulus;
	    }
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

#if defined (commentout)
// find RBF coeff by solving the linear equations using ITK's SVD routine
// Output:
// parms->blm->rbf_coeff contains RBF coefficients
static void 
bspline_rbf_find_coeffs_noreg (
    Volume *vector_field, 
    Bspline_parms *parms
)
{
    Bspline_landmarks *blm = parms->landmarks;
    float *vf;
    float rbfv;
    int i, j, d, fv, rbfcenter[3];

    typedef vnl_matrix <double> Vnl_matrix;
    typedef vnl_svd <double> SVDSolverType;
    Vnl_matrix A, b;

    Rbf_parms *rbf_par;

    rbf_par = (Rbf_parms *)malloc( sizeof(Rbf_parms));
    rbf_par->radius = parms->rbf_radius;
    rbf_par->bparms = parms;
    rbf_par->vector_field = vector_field;

    if (vector_field->pix_type != PT_VF_FLOAT_INTERLEAVED )
	print_and_exit("Sorry, this type of vector field is not supported\n");

    //fill in vector field at fixed landmark location
    vf = (float*) vector_field->img;

    for(i=0;i<blm->num_landmarks;i++) {
	fv = blm->landvox_fix[3*i+2] * vector_field->dim[0] * vector_field->dim[1] 
	    +blm->landvox_fix[3*i+1] * vector_field->dim[0] 
	    +blm->landvox_fix[3*i+0] ;

	for(d=0;d<3;d++) blm->landmark_dxyz[3*i+d] = vf[3*fv+d];
    }

    A.set_size (3 * blm->num_landmarks, 3 * blm->num_landmarks);
    A.set_identity ();

    b.set_size (3 * blm->num_landmarks, 1);
    b.fill (0.0);

    // right-hand side
    for(i=0;i<blm->num_landmarks;i++) {
	for(d=0;d<3;d++) {
	    b (3*i +d, 0) = 
		-(blm->fixed_landmarks.point(i,d) 
		    + blm->landmark_dxyz[3*i+d] 
		    - blm->moving_landmarks.point(i,d));
	}
    }

    // matrix
    for(i=0;i<blm->num_landmarks;i++) {
	for(j=0;j<blm->num_landmarks;j++) {

	    for(d=0;d<3;d++) rbfcenter[d] = blm->landvox_fix[3*j+d];
	
	    rbfv = rbf_value( rbfcenter, 
		blm->landvox_fix[3*i+0],
		blm->landvox_fix[3*i+1],
		blm->landvox_fix[3*i+2], 
		rbf_par->radius,
		rbf_par->vector_field->pix_spacing );

	    for(d=0;d<3;d++)
		A(3*i+d, 3*j+d) = rbfv ;

	}
    }

    //    A.print (std::cout);
    //    b.print (std::cout);

    SVDSolverType svd (A, 1e-6);
    Vnl_matrix x = svd.solve (b);

    //    x.print (std::cout);

    for(i=0;i<3*blm->num_landmarks;i++) blm->rbf_coeff[i] = x(i,0);
}
#endif

// find RBF coeff by solving the linear equations using ITK's SVD routine
// Output:
// parms->blm->rbf_coeff contains RBF coefficients
static void 
bspline_rbf_find_coeffs (
    float *coeff,                   /* Output */
    Landmark_warp *lw               /* Input */
)
{
    bspline_rbf_find_coeffs_reg (coeff, lw);
    //bspline_rbf_find_coeffs_noreg(vector_field, parms);

    int i;
    for (i=0; i < lw->m_fixed_landmarks.get_count(); i++) {
	printf("coeff %4d  %.4f %.4f %.4f\n",  i,
	    coeff[3*i+0],
	    coeff[3*i+1],
	    coeff[3*i+2]);
    }

#if defined (commentout)
    float s;

    //bspline_rbf_test_solution( vector_field, parms);
    s = bspline_rbf_analytic_integral( parms, vector_field->pix_spacing );
    printf("Analytic INTSECDER %f\n",s);
#endif
}

/*
Adds RBF contributions to the vector field
landmark_dxyz is not updated by this function
Version without truncation: scan over the entire vf
and add up all RBFs in each voxel
NOW SUPPORTS DIRECTION COSINES
*/
void
rbf_gauss_update_vf (
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

    printf("Gauss RBF, updating the vector field\n");

    if (vf->pix_type != PT_VF_FLOAT_INTERLEAVED )
	print_and_exit("Sorry, this type of vector field is not supported\n");

    vf_img = (float*) vf->img;

    LOOP_Z (ijk, fxyz, vf) {
	LOOP_Y (ijk, fxyz, vf) {
	    LOOP_X (ijk, fxyz, vf) {
		/* Compute linear index of voxel */
		plm_long fv = volume_index (vf->dim, ijk);

		for (lidx=0; lidx < num_landmarks; lidx++) {
			
		    rbf = rbf_value (
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

/*
Adds RBF contributions to the vector field
landmark_dxyz is not updated by this function
Version without truncation: scan over the entire vf
and add up all RBFs in each voxel
VERSION WITHOUT DIRECTION COSINES (correct for 100 010 001 only)
*/
void
rbf_gauss_update_vf_no_dircos (
    Volume *vf,                  /* Modified */
    Landmark_warp *lw,           /* Input */
    float *coeff                 /* Input */
)
{
    int lidx, d;
    plm_long fi, fj, fk, fv;
    float fxyz[3];
    float *vf_img;
    float rbf;
    int num_landmarks = lw->m_fixed_landmarks.get_count();

    printf("Gauss RBF, updating the vector field\n");

    if (vf->pix_type != PT_VF_FLOAT_INTERLEAVED )
	print_and_exit("Sorry, this type of vector field is not supported\n");

    vf_img = (float*) vf->img;

    for (fk = 0; fk < vf->dim[2];  fk++) {
	fxyz[2] = vf->origin[2] + fk * vf->spacing[2];
	for (fj = 0; fj < vf->dim[1];  fj++) {
	    fxyz[1] = vf->origin[1] + fj * vf->spacing[1];
	    for (fi = 0; fi < vf->dim[0];  fi++) {
		fxyz[0] = vf->origin[0] + fi * vf->spacing[0];
		for (lidx=0; lidx < num_landmarks; lidx++) {
		    fv = fk * vf->dim[0] * vf->dim[1] 
			+ fj * vf->dim[0] + fi;
			
		    rbf = rbf_value (
			lw->m_fixed_landmarks.point(lidx).p, 
			fxyz, 
			lw->adapt_radius[lidx]);

		    for (d=0; d<3; d++) {
			vf_img[3*fv+d] += coeff[3*lidx+d] * rbf;
#if defined (commentout)
			printf ("Adding: %d (%d %d %d) (%g * %g) %g\n", 
			    lidx, 
			    fi, fj, fk, 
			    blm->rbf_coeff[3*lidx+d], rbf, 
			    blm->rbf_coeff[3*lidx+d] * rbf);
#endif
		    }
		}
	    }
	}
    }
}


void
rbf_gauss_warp (Landmark_warp *lw)
{
    float *coeff;
    float origin[3], spacing[3];
    float direction_cosines[9];
    plm_long dim[3];
    int i;
    Volume::Pointer moving;
    Volume *vf_out, *warped_out;

    lw->adapt_radius = (float *)malloc(lw->m_fixed_landmarks.get_count()*sizeof(float));
    lw->cluster_id = (int *)malloc(lw->m_fixed_landmarks.get_count()*sizeof(int));

    if (lw->num_clusters > 0) {
	rbf_cluster_kmeans_plusplus( lw );
	rbf_cluster_find_adapt_radius( lw );
    }
    else {
	for(i = 0; i < lw->m_fixed_landmarks.get_count(); i++) 
	    lw->adapt_radius[i]=lw->rbf_radius;
    }

    for (i = 0; i < lw->m_fixed_landmarks.get_count(); i++) 
	printf("%f\n", lw->adapt_radius[i]);

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
    rbf_gauss_update_vf (vf_out, lw, coeff);

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

    printf ("Done with rbf_gauss_warp\n");
}
