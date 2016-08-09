/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#ifndef _WIN32
#include <dlfcn.h>
#endif

#include "bspline.h"
#if (CUDA_FOUND)
#include "bspline_cuda.h"
#include "cuda_util.h"
#endif
#include "bspline_interpolate.h"
#include "bspline_landmarks.h"
#include "bspline_mi.h"
#include "bspline_mi_hist.h"
#include "bspline_mse.h"
#include "bspline_parms.h"
#include "bspline_regularize.h"
#include "bspline_state.h"
#include "bspline_xform.h"
#include "delayload.h"
#include "file_util.h"
#include "interpolate_macros.h"
#include "logfile.h"
#include "plm_math.h"
#include "string_util.h"
#include "registration_metric_type.h"
#include "volume.h"
#include "volume_macros.h"

static void
bspline_cuda_state_create (
    Bspline_xform* bxf,
    Bspline_state *bst,
    Bspline_parms *parms
);
static void
bspline_cuda_state_destroy (
    Bspline_state *bst,
    Bspline_parms *parms, 
    Bspline_xform *bxf
);


class Bspline_state_private 
{
public:
    Bspline_parms *parms;
    Bspline_xform *bxf;
public:
    Bspline_state_private () {
        parms = 0;
        bxf = 0;
    }
    ~Bspline_state_private () {
        /* Members not owned by this class */
    }
};


Bspline_state::Bspline_state ()
{
    d_ptr = new Bspline_state_private;
}

Bspline_state::~Bspline_state ()
{
    bspline_cuda_state_destroy (this, d_ptr->parms, d_ptr->bxf);
    delete d_ptr;
}

void
Bspline_state::initialize (
    Bspline_xform *bxf,
    Bspline_parms *parms)
{
    Regularization_parms* reg_parms = parms->reg_parms;
    Bspline_regularize* rst = &this->rst;
    Bspline_landmarks* blm = parms->blm;

    d_ptr->bxf = bxf;
    d_ptr->parms = parms;

    this->sm = 0;
    this->it = 0;
    this->feval = 0;
    this->dev_ptrs = 0;
    this->mi_hist = 0;

    this->ssd.set_num_coeff (bxf->num_coeff);

    if (reg_parms->lambda > 0.0f) {
        rst->fixed = parms->fixed;
        rst->moving = parms->moving;
        rst->fixed_stiffness = parms->fixed_stiffness;
        rst->initialize (reg_parms, bxf);
    }

    /* Initialize MI histograms */
    this->mi_hist = 0;
    if (parms->metric_type[0] == REGISTRATION_METRIC_MI_MATTES) {
        this->mi_hist = new Bspline_mi_hist_set (
            parms->mi_hist_type,
            parms->mi_hist_fixed_bins,
            parms->mi_hist_moving_bins);
    }
    bspline_cuda_state_create (bxf, this, parms);


    /* JAS Fix 2011.09.14
     *   The MI algorithm will get stuck for a set of coefficients all equaling
     *   zero due to the method we use to compute the cost function gradient.
     *   However, it is possible we could be inheriting coefficients from a
     *   prior stage, so we must check for inherited coefficients before
     *   applying an initial offset to the coefficient array. */
    if (parms->metric_type[0] == REGISTRATION_METRIC_MI_MATTES) {
        bool first_iteration = true;

        for (int i=0; i<bxf->num_coeff; i++) {
            if (bxf->coeff[i] != 0.0f) {
                first_iteration = false;
                break;
            }
        }

        if (first_iteration) {
            printf ("Initializing 1st MI Stage\n");
            for (int i = 0; i < bxf->num_coeff; i++) {
                bxf->coeff[i] = 0.01f;
            }
        }
    }

    /* Landmarks */
    blm->initialize (bxf);
}

static void
bspline_cuda_state_create (
    Bspline_xform* bxf,
    Bspline_state *bst,           /* Modified in routine */
    Bspline_parms *parms
)
{
#if (CUDA_FOUND)
    if (parms->threading != BTHR_CUDA) {
        return;
    }

    /* Set the gpuid */
    LOAD_LIBRARY_SAFE (libplmcuda);
    LOAD_SYMBOL (CUDA_selectgpu, libplmcuda);
    CUDA_selectgpu (parms->gpuid);
    UNLOAD_LIBRARY (libplmcuda);
    
    Volume *fixed = parms->fixed;
    Volume *moving = parms->moving;
    Volume *moving_grad = parms->moving_grad;

    Dev_Pointers_Bspline* dev_ptrs 
        = (Dev_Pointers_Bspline*) malloc (sizeof (Dev_Pointers_Bspline));
    bst->dev_ptrs = dev_ptrs;
    
    if (parms->metric_type[0] == REGISTRATION_METRIC_MSE) {
        /* Be sure we loaded the CUDA plugin */
        LOAD_LIBRARY_SAFE (libplmregistercuda);
        LOAD_SYMBOL (CUDA_bspline_mse_init_j, libplmregistercuda);

        switch (parms->implementation) {
        case 'j':
        case '\0':   /* Default */
            CUDA_bspline_mse_init_j (dev_ptrs, fixed, moving, moving_grad, bxf, parms);
            break;
        default:
            printf ("Warning: option -f %c unavailble.  Switching to -f j\n",
                parms->implementation);
            CUDA_bspline_mse_init_j (dev_ptrs, fixed, moving, moving_grad, bxf, parms);
            break;
        }

        UNLOAD_LIBRARY (libplmregistercuda);
    } 
    else if (parms->metric_type[0] == REGISTRATION_METRIC_MI_MATTES) {
        /* Be sure we loaded the CUDA plugin */
        LOAD_LIBRARY_SAFE (libplmregistercuda);
        LOAD_SYMBOL (CUDA_bspline_mi_init_a, libplmregistercuda);

        switch (parms->implementation) {
        case 'a':
            CUDA_bspline_mi_init_a (bxf, bst, dev_ptrs, fixed, moving, moving_grad);
            break;
        default:
            printf ("Warning: option -f %c unavailble.  Defaulting to -f a\n",
                parms->implementation);
            CUDA_bspline_mi_init_a (bxf, bst, dev_ptrs, fixed, moving, moving_grad);
            break;
        }

        UNLOAD_LIBRARY (libplmregistercuda);
    }
    else {
        printf ("No cuda initialization performed.\n");
    }
#endif
}

static void
bspline_cuda_state_destroy (
    Bspline_state *bst,
    Bspline_parms *parms, 
    Bspline_xform *bxf
)
{
#if (CUDA_FOUND)
    if (parms->threading != BTHR_CUDA) {
        return;
    }

    Volume *fixed = parms->fixed;
    Volume *moving = parms->moving;
    Volume *moving_grad = parms->moving_grad;

    if (parms->metric_type[0] == REGISTRATION_METRIC_MSE) {
        LOAD_LIBRARY_SAFE (libplmregistercuda);
        LOAD_SYMBOL (CUDA_bspline_mse_cleanup_j, libplmregistercuda);
        CUDA_bspline_mse_cleanup_j ((Dev_Pointers_Bspline *) bst->dev_ptrs, fixed, moving, moving_grad);
        UNLOAD_LIBRARY (libplmregistercuda);
    }
    else if (parms->metric_type[0] == REGISTRATION_METRIC_MI_MATTES) {
        LOAD_LIBRARY_SAFE (libplmregistercuda);
        LOAD_SYMBOL (CUDA_bspline_mi_cleanup_a, libplmregistercuda);
        CUDA_bspline_mi_cleanup_a ((Dev_Pointers_Bspline *) bst->dev_ptrs, fixed, moving, moving_grad);
        UNLOAD_LIBRARY (libplmregistercuda);
    }
#endif
    free (bst->dev_ptrs);
}

Bspline_state *
bspline_state_create (
    Bspline_xform *bxf, 
    Bspline_parms *parms
)
{
    Bspline_state *bst = (Bspline_state*) malloc (sizeof (Bspline_state));
    Regularization_parms* reg_parms = parms->reg_parms;
    Bspline_regularize* rst = &bst->rst;
    Bspline_landmarks* blm = parms->blm;

    memset (bst, 0, sizeof (Bspline_state));
    bst->ssd.set_num_coeff (bxf->num_coeff);

    if (reg_parms->lambda > 0.0f) {
        rst->fixed = parms->fixed;
        rst->moving = parms->moving;
        rst->initialize (reg_parms, bxf);
    }

    /* Initialize MI histograms */
    bst->mi_hist = 0;
    if (parms->metric_type[0] == REGISTRATION_METRIC_MI_MATTES) {
        bst->mi_hist = new Bspline_mi_hist_set (
            parms->mi_hist_type,
            parms->mi_hist_fixed_bins,
            parms->mi_hist_moving_bins);
    }
    bspline_cuda_state_create (bxf, bst, parms);

    /* JAS Fix 2011.09.14
     *   The MI algorithm will get stuck for a set of coefficients all equaling
     *   zero due to the method we use to compute the cost function gradient.
     *   However, it is possible we could be inheriting coefficients from a
     *   prior stage, so we must check for inherited coefficients before
     *   applying an initial offset to the coefficient array. */
    if (parms->metric_type[0] == REGISTRATION_METRIC_MI_MATTES) {
        bool first_iteration = true;

        for (int i=0; i<bxf->num_coeff; i++) {
            if (bxf->coeff[i] != 0.0f) {
                first_iteration = false;
                break;
            }
        }

        if (first_iteration) {
            printf ("Initializing 1st MI Stage\n");
            for (int i = 0; i < bxf->num_coeff; i++) {
                bxf->coeff[i] = 0.01f;
            }
        }
    }

    /* Landmarks */
    blm->initialize (bxf);

    return bst;
}
