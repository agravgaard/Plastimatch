#ifndef ITK_DEMONS_REGISTRATION_FILTER_H
#define ITK_DEMONS_REGISTRATION_FILTER_H

class Stage_parms;

#include "itkPDEDeformableRegistrationWithMaskFilter.h"
//#include "itkLogDomainDeformableRegistrationFilter.h"

#include "itk_image_type.h"

struct itk_demons_registration_filter
{
    protected:
        typedef itk::PDEDeformableRegistrationWithMaskFilter<FloatImageType,FloatImageType,DeformationFieldType> PDEDeformableRegistrationFilterType;
        PDEDeformableRegistrationFilterType::Pointer m_demons_filter;

    public:
        virtual void update_specific_parameters (const Stage_parms* parms)=0;
        virtual ~itk_demons_registration_filter(){};

        PDEDeformableRegistrationFilterType::Pointer get_demons_filter_impl()
        {
            return this->m_demons_filter;
        }
};

#endif // ITK_DEMONS_REGISTRATION_FILTER_H
