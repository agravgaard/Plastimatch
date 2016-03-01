/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "itkArray.h"
#include "itkCommand.h"


#include "itk_fsf_demons.h"
#include "itk_demons_util.h"
#include "itk_image.h"
#include "itk_resample.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_timer.h"
#include "print_and_exit.h"
#include "registration_data.h"
#include "stage_parms.h"
#include "xform.h"
#include "itkESMDemonsRegistrationWithMaskFunction.h"


itk_fsf_demons_filter::itk_fsf_demons_filter()
{
    m_demons_filter = FastSymForcesDemonsFilterType::New();
}

itk_fsf_demons_filter::~itk_fsf_demons_filter()
{
}

void itk_fsf_demons_filter::update_specific_parameters(const Stage_parms* stage)

{
    FastSymForcesDemonsFilterType* fsf_demons_filter=dynamic_cast<FastSymForcesDemonsFilterType*>(m_demons_filter.GetPointer());

    fsf_demons_filter->SetUseGradientType(static_cast<GradientType>(stage->demons_gradient_type));
    fsf_demons_filter->SetMaximumUpdateStepLength(stage->demons_step_length);
}


