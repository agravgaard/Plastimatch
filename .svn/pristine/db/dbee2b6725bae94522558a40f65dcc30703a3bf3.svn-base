/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _landmark_warp_h_
#define _landmark_warp_h_

#include "plmregister_config.h"

#include "pointset.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "print_and_exit.h"
#include "volume.h"
#include "xform.h"

class Plm_image;
class Xform;

class PLMREGISTER_API Landmark_warp
{
public:
    /* Inputs */
#if defined (commentout)
    Raw_pointset *m_fixed_landmarks;
    Raw_pointset *m_moving_landmarks;
#endif
    Labeled_pointset m_fixed_landmarks;
    Labeled_pointset m_moving_landmarks;
    Plm_image::Pointer m_input_img;
    Plm_image_header m_pih;

    /* Config */
    float default_val;
    float rbf_radius;
    float young_modulus;
    int num_clusters; // if >0, use adaptive radius of RBF

    /* Internals */
    int *cluster_id;  // index of cluster the landmark belongs to
    float *adapt_radius; // adaptively found radius of RBF of each landmark

    /* Outputs */
    Plm_image *m_warped_img;
    Xform *m_vf; //also, INPUT vf for calculate_warped_landmarks()
    // if regularized, warped landmarks may not exactly match fixed
    Labeled_pointset m_warped_landmarks;

public:
    Landmark_warp ();
    ~Landmark_warp ();
    void
    load_pointsets (
	const char *fixed_lm_fn, 
	const char *moving_lm_fn
    );
};

PLMREGISTER_C_API Landmark_warp* landmark_warp_create (void);
PLMREGISTER_C_API void landmark_warp_destroy (Landmark_warp *lw);
PLMREGISTER_C_API Landmark_warp* landmark_warp_load_xform (const char *fn);
PLMREGISTER_C_API Landmark_warp* landmark_warp_load_pointsets (
        const char *fixed_lm_fn,
        const char *moving_lm_fn
);

PLMREGISTER_C_API void calculate_warped_landmarks( Landmark_warp *lw );
PLMREGISTER_C_API void calculate_warped_landmarks_by_vf( Landmark_warp *lw, Volume *vf );

#endif
