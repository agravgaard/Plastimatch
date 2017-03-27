/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "interpolate.h"
#include "interpolate_macros.h"
#include "logfile.h"
#include "metric_parms.h"
#include "mha_io.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "print_and_exit.h"
#include "registration_data.h"
#include "registration_resample.h"
#include "shared_parms.h"
#include "stage_parms.h"
#include "string_util.h"
#include "translation_grid_search.h"
#include "translation_mi.h"
#include "translation_mse.h"
#include "volume.h"
#include "volume_grad.h"
#include "volume_macros.h"
#include "volume_resample.h"
#include "xform.h"

class Translation_grid_search
{
public:
    std::list<Metric_state::Pointer> similarity_data;
    
    float (*translation_score) (
        const Stage_parms *stage, const Volume::Pointer& fixed,
        const Volume::Pointer& moving, const float dxyz[3]);
    float best_score;
    float best_translation[3];
public:
    void do_search (
        Xform::Pointer& xf_out,
        const Stage_parms* stage,
        Stage_parms* auto_parms);
    void do_score (
        const Stage_parms* stage,
        const float dxyz[3]);
};

void
Translation_grid_search::do_search (
    Xform::Pointer& xf_out, 
    const Stage_parms* stage,
    Stage_parms* auto_parms)
{
    /* GCS FIX: region of interest is not used */

    /* GCS FIX: This algorithm will not work with tilted images.
       For these cases, we need to use bounding box to compute 
       search extent. */

    Volume::Pointer& fixed = this->similarity_data.front()->fixed_ss;
    Volume::Pointer& moving = this->similarity_data.front()->moving_ss;
        
    /* Compute maximum search extent */
    lprintf ("Computing grid search extent.\n");
    float search_min[3];
    float search_max[3];
    for (int d = 0; d < 3; d++) {
        float mo = stage->gridsearch_min_overlap[d];
        if (mo < 0.1) { mo = 0.1; }
        else if (mo > 0.9) { mo = 0.9; }

        float mov_siz = moving->dim[d] * moving->spacing[d];
        float fix_siz = fixed->dim[d] * fixed->spacing[d];
        lprintf ("Dimension %d, mo=%g F=(%g, %g) M=(%g, %g)\n",
            d, mo, fixed->origin[d], fix_siz,
            moving->origin[d], mov_siz);
        
        if (fix_siz > mov_siz) {
            search_min[d] = moving->origin[d] - fixed->origin[d] 
                - fix_siz + mo * mov_siz;
            search_max[d] = moving->origin[d] - fixed->origin[d] 
                + mov_siz - mo * mov_siz;
        } else {
            search_min[d] = moving->origin[d] - fixed->origin[d] 
                - fix_siz + mo * fix_siz;
            search_max[d] = moving->origin[d] - fixed->origin[d] 
                + mov_siz - mo * fix_siz;
        }
    }
    lprintf ("Native grid search extent: "
        "(%g, %g), (%g, %g), (%g, %g)\n",
        search_min[0], search_max[0], 
        search_min[1], search_max[1], 
        search_min[2], search_max[2]);

    /* Get default value */
    TranslationTransformType::Pointer old_trn = xf_out->get_trn ();
    this->best_translation[0] = old_trn->GetParameters()[0];
    this->best_translation[1] = old_trn->GetParameters()[1];
    this->best_translation[2] = old_trn->GetParameters()[2];
    this->best_score = FLT_MAX;
    this->do_score (stage, this->best_translation);

    /* Compute search range */
    int num_steps[3] = { 0, 0, 0 };
    float search_step[3] = { 0.f, 0.f, 0.f };
    float max_range = 0.f;
    for (int d = 0; d < 3; d++) {
        float search_range = search_max[d] - search_min[d];
        if (search_range > max_range) {
            max_range = search_range;
        }
    }

    /* Identify search strategy, and compute step size */
    Gridsearch_strategy_type strategy = stage->gridsearch_strategy;
    if (strategy == GRIDSEARCH_STRATEGY_AUTO) {
        strategy = auto_parms->gridsearch_strategy;
    }
    if (strategy == GRIDSEARCH_STRATEGY_GLOBAL || 
        strategy == GRIDSEARCH_STRATEGY_AUTO)
    {
        lprintf ("Global grid search\n");
        float nominal_step = max_range / 5;
        for (int d = 0; d < 3; d++) {
            float search_range = search_max[d] - search_min[d];
            num_steps[d] = ROUND_INT (search_range / nominal_step) + 1;
            if (num_steps[d] < stage->gridsearch_min_steps[d]) {
                num_steps[d] = stage->gridsearch_min_steps[d];
            }
            if (num_steps[d] > 1) {
                search_step[d] = search_range / (num_steps[d] - 1);
            }
        }
    }
    else {
        lprintf ("Local grid search\n");
        for (int d = 0; d < 3; d++) {
            num_steps[d] = 4;
            if (stage->gridsearch_step_size_type == GRIDSEARCH_STEP_SIZE_AUTO)
            {
                search_step[d] = auto_parms->gridsearch_step_size[d];
            } else {
                search_step[d] = stage->gridsearch_step_size[d];
            }
            search_min[d] = this->best_translation[d] - 1.5 * search_step[d];
        }
    }

    /* Update auto parms */
    auto_parms->gridsearch_strategy = GRIDSEARCH_STRATEGY_LOCAL;
    for (int d = 0; d < 3; d++) {
        auto_parms->gridsearch_step_size[d] = 0.6 * search_step[d];
    }

    /* Run grid search */
    for (plm_long k = 0; k < num_steps[2]; k++) {
        for (plm_long j = 0; j < num_steps[1]; j++) {
            for (plm_long i = 0; i < num_steps[0]; i++) {
                float translation[3] = {
                    search_min[0] + i * search_step[0],
                    search_min[1] + j * search_step[1],
                    search_min[2] + k * search_step[2] };
                this->do_score (stage, translation);
            }
        }
    }

    /* Find the best translation */
    TranslationTransformType::ParametersType xfp(3);
    xfp[0] = this->best_translation[0];
    xfp[1] = this->best_translation[1];
    xfp[2] = this->best_translation[2];

    /* Fixate translation into xform */
    TranslationTransformType::Pointer new_trn = TranslationTransformType::New();
    new_trn->SetParameters(xfp);
    xf_out->set_trn (new_trn);
}

void
Translation_grid_search::do_score (
    const Stage_parms* stage,
    const float dxyz[3])
{
    lprintf ("[%g %g %g]",
        dxyz[0], dxyz[1], dxyz[2]);

    std::list<Metric_state::Pointer>::iterator it;
    float acc_score = 0.f;
    for (it = this->similarity_data.begin(); 
         it != this->similarity_data.end(); ++it)
    {
        const Metric_state::Pointer& ssi = *it;
        float score = 0.f;
        switch (ssi->metric_type) {
        case SIMILARITY_METRIC_MSE:
        case SIMILARITY_METRIC_GM:
            score = translation_mse (stage, ssi, dxyz);
            break;
        case SIMILARITY_METRIC_MI_MATTES:
        case SIMILARITY_METRIC_MI_VW:
            score = translation_mi (stage, ssi, dxyz);
            break;
        default:
            print_and_exit ("Metric %d not implemented with grid search\n");
            break;
        }
        lprintf (" %g", score);
        acc_score += score;
    }

    if (this->similarity_data.size() > 1) {
        lprintf (" | %g", acc_score);
    }
    if (acc_score < this->best_score) {
        this->best_score = acc_score;
        this->best_translation[0] = dxyz[0];
        this->best_translation[1] = dxyz[1];
        this->best_translation[2] = dxyz[2];
        lprintf (" *");
    }
    lprintf ("\n");
}

Xform::Pointer
translation_grid_search_stage (
    Registration_data* regd, 
    const Xform::Pointer& xf_in,
    const Stage_parms* stage)
{
    Xform::Pointer xf_out = Xform::New ();
    Plm_image_header pih;

    Translation_grid_search tgsd;

    /* Copy similarity images and parms */
    populate_similarity_list (tgsd.similarity_data, regd, stage);

    /* Transform input xform to itk translation */
    xform_to_trn (xf_out.get(), xf_in.get(), &pih);

    /* Run the translation optimizer */
    tgsd.do_search (xf_out, stage, regd->get_auto_parms ());

    return xf_out;
}
