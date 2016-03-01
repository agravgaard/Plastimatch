/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <float.h>
#include <math.h>
#include "itkContinuousIndex.h"
#include "itkImageRegionIterator.h"
#include "itkVectorLinearInterpolateImageFunction.h"

#include "itk_image.h"
#include "itk_image_type.h"
#include "pointset_warp.h"

void
pointset_warp (
    Labeled_pointset *warped_pointset,
    Labeled_pointset *input_pointset,
    DeformationFieldType::Pointer vf)
{
    float *dist_array = new float[input_pointset->get_count()];

    for (size_t i = 0; i < input_pointset->get_count(); i++) {
        /* Clone pointset (to set labels) */
        warped_pointset->insert_lps (
            input_pointset->point_list[i].get_label(),
            input_pointset->point_list[i].p[0],
            input_pointset->point_list[i].p[1],
            input_pointset->point_list[i].p[2]);

        /* Initialize distance array */
        dist_array[i] = FLT_MAX;
    }

    /* Loop through vector field */
    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator fi (vf, vf->GetLargestPossibleRegion());
    for (fi.GoToBegin(); !fi.IsAtEnd(); ++fi) {
        
        /* Compute location of correspondence in moving image */
	DeformationFieldType::IndexType idx = fi.GetIndex ();
	FloatPoint3DType fixed_location;
        FloatPoint3DType moving_location;
        vf->TransformIndexToPhysicalPoint (idx, fixed_location);
	const FloatVector3DType& dxyz = fi.Get();
        moving_location[0] = fixed_location[0] + dxyz[0];
        moving_location[1] = fixed_location[1] + dxyz[1];
        moving_location[2] = fixed_location[2] + dxyz[2];

        /* Loop through landmarks */
        for (size_t i = 0; i < input_pointset->get_count(); i++) {

            /* Get distance from correspondence to landmark */
            float dv[3] = {
                moving_location[0] - input_pointset->point_list[i].p[0],
                moving_location[1] - input_pointset->point_list[i].p[1],
                moving_location[2] - input_pointset->point_list[i].p[2]
            };
            float dist = dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2];

            /* Update correspondence if current voxel is closest so far */
            if (dist < dist_array[i]) {
                dist_array[i] = dist;
                warped_pointset->point_list[i].p[0] = fixed_location[0];
                warped_pointset->point_list[i].p[1] = fixed_location[1];
                warped_pointset->point_list[i].p[2] = fixed_location[2];
            }
        }
    }

    /* Loop through landmarks, refining estimate */
    typedef itk::VectorLinearInterpolateImageFunction< 
        DeformationFieldType, float > InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New ();
    interpolator->SetInputImage (vf);
    for (size_t i = 0; i < input_pointset->get_count(); i++) {
        for (int its = 0; its < 10; its++) {
            float dv[3];

            /* Get current estimate of warped point */
            FloatPoint3DType fixed_location;
            fixed_location[0] = warped_pointset->point_list[i].p[0];
            fixed_location[1] = warped_pointset->point_list[i].p[1];
            fixed_location[2] = warped_pointset->point_list[i].p[2];

            /* Get deformation at current estimate */
            itk::ContinuousIndex<float,3> ci;
            vf->TransformPhysicalPointToContinuousIndex (fixed_location, ci);
            FloatVector3DType dxyz 
                = interpolator->EvaluateAtContinuousIndex (ci);

            /* Get moving image location */
            FloatPoint3DType moving_location;
            moving_location[0] = fixed_location[0] + dxyz[0];
            moving_location[1] = fixed_location[1] + dxyz[1];
            moving_location[2] = fixed_location[2] + dxyz[2];

            /* Get distance from correspondence to landmark */
            dv[0] = input_pointset->point_list[i].p[0] - moving_location[0];
            dv[1] = input_pointset->point_list[i].p[1] - moving_location[1];
            dv[2] = input_pointset->point_list[i].p[2] - moving_location[2];
            //float dist_1 = dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2];
            
            /* Compute update */
            float lambda = 0.5;
            fixed_location[0] += lambda * dv[0];
            fixed_location[1] += lambda * dv[1];
            fixed_location[2] += lambda * dv[2];

            /* GCS TODO: check if the update improves the estimate */

            /* Make update */
            warped_pointset->point_list[i].p[0] = fixed_location[0];
            warped_pointset->point_list[i].p[1] = fixed_location[1];
            warped_pointset->point_list[i].p[2] = fixed_location[2];
        }
    }

    delete[] dist_array;
}
