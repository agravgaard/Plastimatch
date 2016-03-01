#include <itk_demons_util.h>
#include "itkImageRegionIterator.h"

void itk_demons_util::deformation_stats (DeformationFieldType::Pointer vf)
{
    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator fi (vf, vf->GetLargestPossibleRegion());
    const DeformationFieldType::SizeType vf_size
    = vf->GetLargestPossibleRegion().GetSize();
    double max_sq_len = 0.0;
    double avg_sq_len = 0.0;

    for (fi.GoToBegin(); !fi.IsAtEnd(); ++fi) {
        //index = fi.GetIndex();
        const FloatVector3DType& d = fi.Get();
        double sq_len = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
        if (sq_len > max_sq_len) {
            max_sq_len = sq_len;
        }
        avg_sq_len += sq_len;
    }

    avg_sq_len /= (vf_size[0] * vf_size[1] * vf_size[2]);

    printf ("VF_MAX = %g   VF_AVG = %g\n", max_sq_len, avg_sq_len);
}
