/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include "itk_diff_demons.h"
#include "stage_parms.h"

itk_diffeomorphic_demons_filter::itk_diffeomorphic_demons_filter()
{
    m_demons_filter = DiffeomorphicDemonsFilterType::New();
}

itk_diffeomorphic_demons_filter::~itk_diffeomorphic_demons_filter()
{
}

void itk_diffeomorphic_demons_filter::update_specific_parameters(const Stage_parms* stage)
{
    //Setting gradient type
    DiffeomorphicDemonsFilterType* diff_demons_filter=dynamic_cast<DiffeomorphicDemonsFilterType*>(m_demons_filter.GetPointer());

    diff_demons_filter->SetSmoothDeformationField(stage->demons_smooth_deformation_field);
    diff_demons_filter->SetUseGradientType(static_cast<GradientType>(stage->demons_gradient_type));
    diff_demons_filter->SetMaximumUpdateStepLength(stage->demons_step_length);
}


