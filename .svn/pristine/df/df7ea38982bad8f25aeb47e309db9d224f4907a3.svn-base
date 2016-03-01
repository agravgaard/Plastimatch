/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* Correct mha files which have incorrect patient orientations */
#include "plmutil_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "itkImageRegionIteratorWithIndex.h"

#include "plm_math.h"
#include "synthetic_vf.h"

DeformationFieldType::Pointer
synthetic_vf (Synthetic_vf_parms* parms)
{
    /* Create ITK vf */
    DeformationFieldType::Pointer vf_out = DeformationFieldType::New();
    printf ("Setting header\n");
    parms->pih.print ();
    itk_image_set_header (vf_out, &parms->pih);
    printf ("Header was set\n");
    vf_out->Allocate();

    /* Iterate through vf, setting values */
    typedef itk::ImageRegionIteratorWithIndex< DeformationFieldType > 
	IteratorType;
    IteratorType it_out (vf_out, vf_out->GetRequestedRegion());

    /* Stock displacements can be initialized outside of loop */
    FloatVector3DType disp;
    for (int d = 0; d < 3; d++) {
	switch (parms->pattern) {
	case Synthetic_vf_parms::PATTERN_ZERO:
	default:
	    disp[d] = 0;
	    break;
	case Synthetic_vf_parms::PATTERN_TRANSLATION:
	    disp[d] = parms->translation[d];
	    break;
	}
    }

    for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
	FloatPoint3DType phys;

        /* Get 3D coordinate of voxel */
	DeformationFieldType::IndexType idx = it_out.GetIndex ();
	vf_out->TransformIndexToPhysicalPoint (idx, phys);

	switch (parms->pattern) {
	case Synthetic_vf_parms::PATTERN_GAUSSIAN: {
            float diff[3];
            float dist_2 = 0;
            float f;
            for (int d = 0; d < 3; d++) {
                diff[d] = (phys[d] - parms->gaussian_center[d]) 
                    / parms->gaussian_std[d];
                dist_2 += diff[d] * diff[d];
            }
            f = exp (-0.5 * dist_2);
            for (int d = 0; d < 3; d++) {
                disp[d] = parms->gaussian_mag[d] * f;
            }
            break;
        }
	case Synthetic_vf_parms::PATTERN_RADIAL: {
            float diff[3];
            float dist_2 = 0;
            for (int d = 0; d < 3; d++) {
                diff[d] = (phys[d] - parms->radial_center[d]) 
                    / parms->radial_mag[d] / 3;
                dist_2 += diff[d] * diff[d];
            }
            for (int d = 0; d < 3; d++) {
                if (dist_2 > 1.) {
                    diff[d] = diff[d] / sqrt(dist_2);
                }
                disp[d] = parms->radial_mag[d] * diff[d];
            }
	    break;
        }
        case Synthetic_vf_parms::PATTERN_ZERO:
        case Synthetic_vf_parms::PATTERN_TRANSLATION:
        default:
            /* Don't change disp */
            break;
	}
	it_out.Set (disp);
    }
    return vf_out;
}
