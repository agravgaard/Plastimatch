/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"

#include "bspline_parms.h"
#include "bspline_landmarks.h"
#include "bspline_regularize.h"
#include "logfile.h"

Bspline_parms::Bspline_parms ()
{
    this->threading = BTHR_CPU;
    this->optimization = BOPT_LBFGSB;
    this->min_its = 0;
    this->max_its = 10;
    this->max_feval = 10;
    this->debug = 0;
    this->debug_dir = ".";
    this->debug_stage = 0;
    this->gpuid = 0;
    this->convergence_tol = 1e-6;

    this->mi_hist_type = HIST_EQSP;
    this->mi_hist_fixed_bins = 32;
    this->mi_hist_moving_bins = 32;

    this->mi_fixed_image_minVal=0;
    this->mi_fixed_image_maxVal=0;
    this->mi_moving_image_minVal=0;
    this->mi_moving_image_maxVal=0;

    this->lbfgsb_factr = 1.0e+7;
    this->lbfgsb_pgtol = 1.0e-5;
    this->lbfgsb_mmax = -1;

    this->fixed_stiffness = 0;

    this->reg_parms = new Regularization_parms;

    this->blm = new Bspline_landmarks;
    this->rbf_radius = 0;
    this->rbf_young_modulus = 0;
    this->xpm_hist_dump = 0;
}

Bspline_parms::~Bspline_parms ()
{
    delete this->blm;
    delete this->reg_parms;
}

void
Bspline_parms::log ()
{
    logfile_printf ("BSPLINE PARMS\n");
    logfile_printf ("max_its = %d\n", this->max_its);
    logfile_printf ("max_feval = %d\n", this->max_feval);
}
