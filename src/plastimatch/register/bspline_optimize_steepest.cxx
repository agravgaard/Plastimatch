/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#if (OPENMP_FOUND)
#include <omp.h>
#endif

#include "bspline.h"
#include "bspline_optimize.h"
#include "bspline_optimize_steepest.h"
#include "bspline_parms.h"
#include "bspline_state.h"
#include "bspline_xform.h"
#include "logfile.h"

/*
 * This is a simple gradient plotter based on
 * steepest_trust()
*/
void
bspline_optimize_steepest_trace (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Bspline_score* ssd = &bst->ssd;
    int i;
    float alpha = 1.0f;
    float ssd_grad_norm;
    float old_score;
    FILE *fp = 0;
    double htg;
    int success;
    float *grad_backup;
    int j;
    FILE *trace;
    char filename[20];
    float score_backup;
    float *x;           /* Start of line search */
    float *h;           /* Search direction */

    if (parms->debug) {
        fp = fopen("scores.txt", "w");
    }

    // GCS FIX: I'm pretty sure the below makes no sense.
    // JAS 04.19.2010
    // For Testing...
    if (bst->has_metric_type (SIMILARITY_METRIC_MI_MATTES)) {
        alpha = 1.0f;
        printf ("Using alpha_0 (%f)\n", alpha);
    }

    /* Allocate memory for search direction */
    x = (float*) malloc (bxf->num_coeff * sizeof(float));
    h = (float*) malloc (bxf->num_coeff * sizeof(float));
    grad_backup = (float*) malloc (bxf->num_coeff * sizeof(float));

    /* Set iteration */
    bst->it = 0;
    bst->feval = 0;
    success = 0;
    memcpy (x, bxf->coeff, bxf->num_coeff * sizeof(float));

    /* Get score and gradient */
    bspline_score (bod);
    old_score = bst->ssd.total_score;

    /* Get search direction */
    ssd_grad_norm = 0;
    for (i = 0; i < bxf->num_coeff; i++) {
        ssd_grad_norm += ssd->total_grad[i] * ssd->total_grad[i];
    }
    ssd_grad_norm = sqrt (ssd_grad_norm);
    htg = 0.0;
    for (i = 0; i < bxf->num_coeff; i++) {
        h[i] = - ssd->total_grad[i] / ssd_grad_norm;
        htg -= h[i] * ssd->total_grad[i];
    }

    /* Give a little feedback to the user */
    bspline_display_coeff_stats (bxf);
    /* Save some debugging information */
    bspline_save_debug_state (parms, bst, bxf);
    if (parms->debug) {
        fprintf (fp, "%f\n", ssd->total_score);
    }

    while (bst->it < parms->max_its && bst->feval < parms->max_feval) {
        double gr;
        int accept_step = 0;

        /* Update num function evaluations */
        bst->feval ++;

        /* Compute new search location */
        for (i = 0; i < bxf->num_coeff; i++) {
            bxf->coeff[i] = x[i] + alpha * h[i];
        }

        /* Get score and gradient */
        bspline_score (bod);

        /* Compute gain ratio with linear model */
        gr = (old_score - bst->ssd.total_score) / htg;
        if (gr < 0) {
            /* Cost increased.  Keep search direction and reduce trust rgn. */
            alpha = 0.5 * alpha;
        } else if (gr < 0.25) {
            alpha = 0.5 * alpha;
            accept_step = 1;
        } else if (gr > 0.75) {
            alpha = 3.0 * alpha;
            accept_step = 1;
        } else {
            accept_step = 1;
        }

        /* Give a little feedback to the user */
        bspline_display_coeff_stats (bxf);
        logfile_printf ("                    "
            "GR %6.2f NEW_A %6.4f ACCEPT? %d\n", gr, alpha, accept_step);

        /* Save some debugging information */
        bspline_save_debug_state (parms, bst, bxf);
        if (parms->debug) {
            fprintf (fp, "%f\n", ssd->total_score);
        }

        /* If score was reduced, we accept the line search (fall through). 
           Otherwise, we reduce trust region (continue). */
        if (!accept_step) continue;

        /* Update iteration number */
        bst->it ++;

        // At this point we have a good set of coefficients that
        // minimize the cost function.
        //
        // So, let's plot the gradient before starting a new
        // line search.
        success ++;
        memcpy (x, bxf->coeff, bxf->num_coeff * sizeof(float));
        memcpy (grad_backup, ssd->total_grad, bxf->num_coeff * sizeof(float));
        score_backup = ssd->total_score;
        sprintf (filename, "grad_%04i.csv", success);
        trace = fopen(filename, "w");
        printf ("Capturing Gradient (grad_%04i.csv)\n", success);

        // For each step along the gradient
        for (i = -35; i < 35; i++) {

            for (j = 0; j < bxf->num_coeff; j++) {
                bxf->coeff[j] = x[j] + i * 1 * h[j];
            }

            /* Get score */
            printf ("\t");
            bspline_score (bod);
        
            fprintf (trace, "%d, %10.10f\n", i, bst->ssd.total_score);
            fflush(trace);
        }
        fclose (trace);

        printf ("Finished Capturing Gradient.\n\n");
        memcpy (ssd->total_grad, grad_backup, bxf->num_coeff * sizeof(float));
        ssd->total_score = score_backup;

        /* Start new line search */
        ssd_grad_norm = 0;
        for (i = 0; i < bxf->num_coeff; i++) {
            ssd_grad_norm += ssd->total_grad[i] * ssd->total_grad[i];
        }
        ssd_grad_norm = sqrt (ssd_grad_norm);
        htg = 0.0;
        for (i = 0; i < bxf->num_coeff; i++) {
            h[i] = - ssd->total_grad[i] / ssd_grad_norm;
            htg -= h[i] * ssd->total_grad[i];
        }
        old_score = bst->ssd.total_score;
    }

    /* Save best result */
    memcpy (bxf->coeff, x, bxf->num_coeff * sizeof(float));
    bst->ssd.total_score = old_score;

    if (parms->debug) {
        fclose (fp);
    }
    
    free (grad_backup);
    free (x);
    free (h);
}


/* This combines a steepest descent direction with trust interval line search.
   See Eqn 2.8 + Eqn 2.20 in Madsen, Nielsen, and Tingleff's 
     booklet: "Methods for non-linear least squares probelms"
     http://www2.imm.dtu.dk/pubdb/views/edoc_download.php/3215/pdf/imm3215.pdf
   See also: http://www2.imm.dtu.dk/~hbn/immoptibox/ 

   It works ok, but seems to get caught in Stiefel's cage.
*/
void
bspline_optimize_steepest_trust (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();
    Bspline_score* ssd = &bst->ssd;
    int i;
    float alpha = 1.0f;
    float ssd_grad_norm;
    float old_score;
    FILE *fp = 0;
    float *x;           /* Start of line search */
    float *h;           /* Search direction */
    double htg;

    if (parms->debug) {
        fp = fopen("scores.txt", "w");
    }

    // GCS FIX: I'm pretty sure the below makes no sense.
    // JAS 04.19.2010
    // For testing...
    if (bst->has_metric_type (SIMILARITY_METRIC_MI_MATTES)) {
        alpha = 1.0f;
        printf ("Using alpha_0 (%f)\n", alpha);
    }

    /* Allocate memory for search direction */
    x = (float*) malloc (bxf->num_coeff * sizeof(float));
    h = (float*) malloc (bxf->num_coeff * sizeof(float));

    /* Set iteration */
    bst->it = 0;
    bst->feval = 0;
    memcpy (x, bxf->coeff, bxf->num_coeff * sizeof(float));

    /* Get score and gradient */
    bspline_score (bod);
    old_score = bst->ssd.total_score;

    /* Get search direction */
    ssd_grad_norm = 0;
    for (i = 0; i < bxf->num_coeff; i++) {
        ssd_grad_norm += ssd->total_grad[i] * ssd->total_grad[i];
    }
    ssd_grad_norm = sqrt (ssd_grad_norm);
    htg = 0.0;
    for (i = 0; i < bxf->num_coeff; i++) {
        h[i] = - ssd->total_grad[i] / ssd_grad_norm;
        htg -= h[i] * ssd->total_grad[i];
    }

    /* Give a little feedback to the user */
    bspline_display_coeff_stats (bxf);

    /* Save some debugging information */
    bspline_save_debug_state (parms, bst, bxf);
    if (parms->debug) {
        fprintf (fp, "%f\n", ssd->total_score);
    }

    while (bst->it < parms->max_its && bst->feval < parms->max_feval) {
        double gr;
        int accept_step = 0;

        /* Update num function evaluations */
        bst->feval ++;

        /* Compute new search location */
        for (i = 0; i < bxf->num_coeff; i++) {
            bxf->coeff[i] = x[i] + alpha * h[i];
        }

        /* Get score and gradient */
        bspline_score (bod);

        /* Compute gain ratio with linear model */
        gr = (old_score - bst->ssd.total_score) / htg;
        if (gr < 0) {
            /* Cost increased.  Keep search direction and reduce trust rgn. */
            alpha = 0.5 * alpha;
        } else if (gr < 0.25) {
            alpha = 0.5 * alpha;
            accept_step = 1;
        } else if (gr > 0.75) {
            alpha = 3.0 * alpha;
            accept_step = 1;
        } else {
            accept_step = 1;
        }

        /* Give a little feedback to the user */
        bspline_display_coeff_stats (bxf);
        logfile_printf ("                    "
            "GR %6.2f NEW_A %6.2f ACCEPT? %d\n", gr, alpha, accept_step);

        /* Save some debugging information */
        bspline_save_debug_state (parms, bst, bxf);
        if (parms->debug) {
            fprintf (fp, "%f\n", ssd->total_score);
        }

        /* If score was reduced, we accept the line search */
        if (!accept_step) continue;

        /* Update iteration number */
        bst->it ++;

        /* Start new line search */
        memcpy (x, bxf->coeff, bxf->num_coeff * sizeof(float));
        ssd_grad_norm = 0;
        for (i = 0; i < bxf->num_coeff; i++) {
            ssd_grad_norm += ssd->total_grad[i] * ssd->total_grad[i];
        }
        ssd_grad_norm = sqrt (ssd_grad_norm);
        htg = 0.0;
        for (i = 0; i < bxf->num_coeff; i++) {
            h[i] = - ssd->total_grad[i] / ssd_grad_norm;
            htg -= h[i] * ssd->total_grad[i];
        }
        old_score = bst->ssd.total_score;
    }

    /* Save best result */
    memcpy (bxf->coeff, x, bxf->num_coeff * sizeof(float));
    bst->ssd.total_score = old_score;

    if (parms->debug) {
        fclose (fp);
    }
    
    free (x);
    free (h);
}

/* This is a really terrible algorithm.  It takes steps without 
   doing any sort of line search. */
void
bspline_optimize_steepest_naive (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();
    Bspline_score* ssd = &bst->ssd;
    int i;
    //    float a = 0.003f;
    //    float alpha = 0.5f, A = 10.0f;
    float a, gamma;
    float gain = 1.5;
    float ssd_grad_norm;
    float old_score;
    FILE* fp = 0;

    if (parms->debug) {
        fp = fopen("scores.txt", "w");
    }

    /* Set iteration */
    bst->it = 0;
    bst->feval = 0;

    /* Get score and gradient */
    bspline_score (bod);
    old_score = bst->ssd.total_score;

    /* Set alpha based on norm gradient */
    ssd_grad_norm = 0;
    for (i = 0; i < bxf->num_coeff; i++) {
        ssd_grad_norm += fabs (ssd->total_grad[i]);
    }
    a = 1.0f / ssd_grad_norm;
    gamma = a;
    logfile_printf ("Initial gamma is %g\n", gamma);

    /* Give a little feedback to the user */
    bspline_display_coeff_stats (bxf);
    /* Save some debugging information */
    bspline_save_debug_state (parms, bst, bxf);
    if (parms->debug) {
        fprintf (fp, "%f\n", ssd->total_score);
    }

    while (bst->it < parms->max_its && bst->feval < parms->max_feval) {

        /* Update num iterations & num function evaluations */
        bst->it ++;
        bst->feval ++;

        logfile_printf ("Beginning iteration %d, gamma = %g\n", bst->it, gamma);

        /* Update b-spline coefficients from gradient */
        //gamma = a / pow(it + A, alpha);
        for (i = 0; i < bxf->num_coeff; i++) {
            bxf->coeff[i] = bxf->coeff[i] + gamma * ssd->total_grad[i];
        }

        /* Get score and gradient */
        bspline_score (bod);

        /* Update gamma */
        if (bst->ssd.total_score < old_score) {
            gamma *= gain;
        } else {
            gamma /= gain;
        }
        old_score = bst->ssd.total_score;

        /* Give a little feedback to the user */
        bspline_display_coeff_stats (bxf);
        /* Save some debugging information */
        bspline_save_debug_state (parms, bst, bxf);
        if (parms->debug) {
            fprintf (fp, "%f\n", ssd->total_score);
        }
    }

    if (parms->debug) {
        fclose (fp);
    }
}

void
bspline_optimize_steepest (
    Bspline_optimize *bod
)
{
    const int USE_NAIVE = 0;

    if (USE_NAIVE) {
        bspline_optimize_steepest_naive (bod);
    } else {
        bspline_optimize_steepest_trust (bod);
//      DEBUG
//      bspline_optimize_steepest_trace (
//          bxf, bst, parms, fixed, moving, moving_grad);
    }
}
