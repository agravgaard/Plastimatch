/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"

#include <stdlib.h>
#include "bspline_regularize.h"
#include "print_and_exit.h"

Bspline_regularize::Bspline_regularize ()
{
    /* all methods */
    this->reg_parms = 0;
    this->bxf = 0;
    this->fixed = 0;
    this->moving = 0;
    this->fixed_stiffness = 0;

    /* semi-analytic method */
    this->q_dxdyz_lut = 0;
    this->q_xdydz_lut = 0;
    this->q_dxydz_lut = 0;
    this->q_d2xyz_lut = 0;
    this->q_xd2yz_lut = 0;
    this->q_xyd2z_lut = 0;

    /* analytic method */
    this->QX_mats = 0;
    this->QY_mats = 0;
    this->QZ_mats = 0;
    this->QX = 0;
    this->QY = 0;
    this->QZ = 0;
    this->V_mats = 0;
    this->V = 0;
    this->cond = 0;
}

Bspline_regularize::~Bspline_regularize ()
{
    if (!reg_parms) {
        return;
    }

    /* Don't free reg_parms, fixed, moving, bxf; you don't own them */

    /* Semi-analytic method has LUTs to free */
    free (this->q_dxdyz_lut);
    free (this->q_xdydz_lut);
    free (this->q_dxydz_lut);
    free (this->q_d2xyz_lut);
    free (this->q_xd2yz_lut);
    free (this->q_xyd2z_lut);

    /* Numeric method has Q matrices to free */
    free (this->QX_mats);
    free (this->QY_mats);
    free (this->QZ_mats);
    free (this->QX);
    free (this->QY);
    free (this->QZ);
    free (this->V_mats);
    free (this->V);
    free (this->cond);
}

void
Bspline_regularize::initialize (
    Regularization_parms *reg_parms,
    Bspline_xform* bxf
)
{
    this->reg_parms = reg_parms;
    this->bxf = bxf;

    switch (reg_parms->implementation) {
    case 'a':
	this->numeric_init (bxf);
        break;
    case 'b':
    case 'c':
        this->analytic_init (bxf);
        break;
    case 'd':
	this->semi_analytic_init (bxf);
        break;
    default:
        print_and_exit (
            "Error: unknown reg_parms->implementation (%c)\n",
            reg_parms->implementation
        );
        break;
    }
}

void
Bspline_regularize::compute_score (
    Bspline_score *bspline_score,    /* Gets updated */
    const Regularization_parms* reg_parms,
    const Bspline_xform* bxf
)
{
    switch (reg_parms->implementation) {
    case 'a':
        this->compute_score_numeric (bspline_score, reg_parms, this, bxf);
        break;
    case 'b':
        this->compute_score_analytic (bspline_score, reg_parms, this, bxf);
        break;
    case 'c':
#if (OPENMP_FOUND)
        this->compute_score_analytic_omp (bspline_score, reg_parms, this, bxf);
#else
        this->compute_score_analytic (bspline_score, reg_parms, this, bxf);
#endif
        break;
    case 'd':
        this->compute_score_semi_analytic (bspline_score, reg_parms, this, bxf);
        break;
    default:
        print_and_exit (
            "Error: unknown reg_parms->implementation (%c)\n",
            reg_parms->implementation
        );
        break;
    }
}
