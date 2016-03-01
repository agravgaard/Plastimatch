/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_sym_log_demons_h_
#define _itk_sym_log_demons_h_

#include "itkSymmetricLogDomainDemonsRegistrationFilterWithMaskExtension.h"
#include <itk_demons_registration_filter.h>

class itk_sym_log_domain_demons_filter: public itk_demons_registration_filter
{

    typedef itk::SymmetricLogDomainDemonsRegistrationFilterWithMaskExtension<
        FloatImageType,
        FloatImageType,
        DeformationFieldType> SymmetricLogDomainDemonsFilterType;

    typedef SymmetricLogDomainDemonsFilterType::DemonsRegistrationFunctionType SymmetricLogDomainDemonsFunctionType;

    typedef SymmetricLogDomainDemonsFunctionType::GradientType GradientType;

public:
    itk_sym_log_domain_demons_filter();
    ~itk_sym_log_domain_demons_filter();
    void update_specific_parameters (const Stage_parms* stage);
};

#endif
