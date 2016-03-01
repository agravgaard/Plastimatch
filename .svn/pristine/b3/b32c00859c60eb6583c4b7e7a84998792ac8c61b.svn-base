/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"

#include "landmark_warp.h"
#include "itk_image_save.h"
#include "itk_pointset.h"
#include "itk_tps.h"
#include "itk_warp.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "raw_pointset.h"
#include "xform.h"

#define BUFLEN 2048

template<class T>
static void
do_tps_core (
    Landmark_warp *lw,                         /* Input and output */
    DoublePointSetType::Pointer mov_lm,        /* Input */
    DoublePointSetType::Pointer fix_lm,        /* Input */
    T default_val                              /* Input */
)
{
    TpsTransformType::Pointer tps = TpsTransformType::New ();
    Xform xform_tps;

    printf ("Setting landmarks to TPS\n");
    tps->SetSourceLandmarks (fix_lm);
    tps->SetTargetLandmarks (mov_lm);
    printf ("Computing matrix\n");
    tps->ComputeWMatrix ();

    printf ("Setting xform\n");
    xform_tps.set_itk_tps (tps);

    printf ("Converting to VF\n");
    lw->m_vf = new Xform;
    xform_to_itk_vf (lw->m_vf, &xform_tps, &lw->m_pih);

    printf ("Warping...\n");
    typename DeformationFieldType::Pointer vf = DeformationFieldType::New ();
    vf = lw->m_vf->get_itk_vf ();
    typename itk::Image<T, 3>::Pointer im_warped 
	= itk_warp_image (lw->m_input_img->itk_float (), vf, 1, default_val);

    /* Set outputs */
    lw->m_warped_img = new Plm_image;
    lw->m_warped_img->set_itk (im_warped);
}

template<class T>
void
do_tps (
    TPS_parms* parms, 
    typename itk::Image<T, 3>::Pointer img_fixed,
    typename itk::Image<T, 3>::Pointer img_moving, 
    T default_val
)
{
    typedef typename itk::Image<T, 3> ImgType;
    typedef typename TpsTransformType::PointSetType PointSetType;
    typedef typename PointSetType::Pointer PointSetPointer;
    typedef typename PointSetType::PointIdentifier PointIdType;

    Plm_image_header pih;
    DoublePoint3DType p1;
    DoublePoint3DType p2;
    char line[BUFLEN];
    Xform xform_tmp, xform;
    FILE* reference;
    FILE* target;

    pih.set_from_itk_image (img_fixed);

    typename PointSetType::Pointer sourceLandMarks = PointSetType::New ();
    typename PointSetType::Pointer targetLandMarks = PointSetType::New ();

    typename PointSetType::PointsContainer::Pointer sourceLandMarkContainer = sourceLandMarks->GetPoints ();
    typename PointSetType::PointsContainer::Pointer targetLandMarkContainer = targetLandMarks->GetPoints ();

    PointIdType id = itk::NumericTraits< PointIdType >::Zero;
    PointIdType id2 = itk::NumericTraits< PointIdType >::Zero;

    reference = fopen (parms->reference, "r");
    target = fopen (parms->target, "r");

    if (!reference || !target) {
        fprintf (stderr, "An error occurred while opening the landmark files!");
        exit (-1);
    }

    while (fgets (line, BUFLEN, reference)) {
        if (sscanf (line, "%lf %lf %lf", &p1[0], &p1[1], &p1[2]) == 3) {
            sourceLandMarkContainer->InsertElement (id++, p1);
            printf ("reference Landmark: %f %f %f\n", p1[0], p1[1], p1[2]);
        } else {
            printf ("Error! can't read the reference landmarks file");
            exit (-1);
        }
    }
    id = itk::NumericTraits< PointIdType >::Zero;
    while (fgets (line, BUFLEN, target)) {
        if (sscanf (line, "%lf %lf %lf", &p2[0], &p2[1], &p2[2]) == 3) {
            targetLandMarkContainer->InsertElement (id2++, p2);
            printf ("target Landmark: %f %f %f \n", p2[0], p2[1], p2[2]);
        } else {
            printf ("Error! can't read the target landmarks file");
            exit (-1);
        }
    }

    fclose (reference);
    fclose (target);

    TpsTransformType::Pointer tps = TpsTransformType::New ();
    tps->SetSourceLandmarks (sourceLandMarks);
    tps->SetTargetLandmarks (targetLandMarks);
    tps->ComputeWMatrix ();

    xform.set_itk_tps (tps);
    xform_to_itk_vf (&xform_tmp, &xform, &pih);

    typename DeformationFieldType::Pointer vf = DeformationFieldType::New ();
    vf = xform_tmp.get_itk_vf ();

    printf ("Warping...\n");

    typename ImgType::Pointer im_warped = itk_warp_image (img_moving, vf, 1, default_val);

    printf ("Saving...\n");
    itk_image_save (im_warped, parms->warped);
    itk_image_save (vf, parms->vf);
}

void
itk_tps_warp (
    Landmark_warp *lw
)
{
    printf ("Hello world\n");

    /* Convert image to itk float */
    if (lw->m_input_img) {
	lw->m_input_img->itk_float ();
    }

    printf ("Gonna convert pointsets\n");
    lw->m_fixed_landmarks.debug ();

    /* Convert pointsets to itk pointsets */
    DoublePointSetType::Pointer mov_lm = 
	itk_double_pointset_from_pointset (lw->m_moving_landmarks);
    DoublePointSetType::Pointer fix_lm = 
	itk_double_pointset_from_pointset (lw->m_fixed_landmarks);

    printf ("Conversion complete.\n");
    itk_pointset_debug (fix_lm);

    /* Run ITK TPS warper */
    do_tps_core (
	lw, 
	mov_lm, 
	fix_lm, 
	(float) lw->default_val
    );
}

/* Explicit instantiations */
/* RMK: Visual studio 2005 without service pack requires <float> specifier
   on the explicit extantiations.  The current hypothesis is that this
   is because the template is nested. */
template PLMREGISTER_API void do_tps<unsigned char>(TPS_parms* parms, itk::Image<unsigned char, 3>::Pointer img_fixed, itk::Image<unsigned char, 3>::Pointer img_moving, unsigned char);
template PLMREGISTER_API void do_tps<float>(TPS_parms* parms, itk::Image<float, 3>::Pointer img_fixed, itk::Image<float, 3>::Pointer img_moving, float);
template PLMREGISTER_API void do_tps<short>(TPS_parms* parms, itk::Image<short, 3>::Pointer img_fixed, itk::Image<short, 3>::Pointer img_moving, short);
