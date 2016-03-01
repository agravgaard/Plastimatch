/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_fsf_demons_h_
#define _itk_fsf_demons_h_

#include "itkFastSymmetricForcesDemonsRegistrationWithMaskFilter.h"

class Registration_data;
class Xform;
class Stage_parms;
class itk_demons_util;

#include <itk_demons_registration_filter.h>

class itk_fsf_demons_filter: public itk_demons_registration_filter
{

    typedef itk::FastSymmetricForcesDemonsRegistrationWithMaskFilter<
        FloatImageType,
        FloatImageType,
        DeformationFieldType> FastSymForcesDemonsFilterType;

    typedef FastSymForcesDemonsFilterType::DemonsRegistrationFunctionType DemonsRegFunctionType;

    typedef DemonsRegFunctionType::GradientType GradientType;

    public:
        itk_fsf_demons_filter();
        ~itk_fsf_demons_filter();
        void update_specific_parameters(const Stage_parms* stage);
};

#endif
