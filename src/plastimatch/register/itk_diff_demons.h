/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_diff_demons_h_
#define _itk_diff_demons_h_

#include "itkDiffeomorphicDemonsRegistrationWithMaskFilter.h"
#include <itk_demons_registration_filter.h>

class itk_diffeomorphic_demons_filter: public itk_demons_registration_filter
{

    typedef itk::DiffeomorphicDemonsRegistrationWithMaskFilter<
        FloatImageType,
        FloatImageType,
        DeformationFieldType> DiffeomorphicDemonsFilterType;

    typedef DiffeomorphicDemonsFilterType::DemonsRegistrationFunctionType DiffeomorphicDemonsFunctionType;

    typedef DiffeomorphicDemonsFunctionType::GradientType GradientType;

public:
    itk_diffeomorphic_demons_filter();
    ~itk_diffeomorphic_demons_filter();
    void update_specific_parameters(const Stage_parms* stage);
};

#endif
