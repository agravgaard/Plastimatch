/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"

#include "itk_sym_log_demons.h"
#include "stage_parms.h"

itk_sym_log_domain_demons_filter::itk_sym_log_domain_demons_filter()
{
    m_demons_filter = SymmetricLogDomainDemonsFilterType::New();
}

itk_sym_log_domain_demons_filter::~itk_sym_log_domain_demons_filter()
{

}

void itk_sym_log_domain_demons_filter::update_specific_parameters(const Stage_parms* stage)
{
    SymmetricLogDomainDemonsFilterType* log_filter=dynamic_cast<SymmetricLogDomainDemonsFilterType*>(m_demons_filter.GetPointer());
    log_filter->SetNumberOfBCHApproximationTerms(stage->num_approx_terms_log_demons);

    log_filter->SetSmoothVelocityField(stage->demons_smooth_deformation_field);
    log_filter->SetUseGradientType(static_cast<GradientType>(stage->demons_gradient_type));
    log_filter->SetMaximumUpdateStepLength(stage->demons_step_length);
}


