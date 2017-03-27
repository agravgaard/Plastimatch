/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "itkImageRegionIteratorWithIndex.h"
#include "vnl/vnl_random.h"

#include "itk_directions.h"
#include "itk_image_type.h"
#include "itk_point.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_math.h"
#include "rt_study.h"
#include "rtss.h"
#include "segmentation.h"
#include "synthetic_mha.h"

class Synthetic_mha_parms_private {
public:
    vnl_random v;
    float gabor_freq;
    float gabor_proj[3];
};


Synthetic_mha_parms::Synthetic_mha_parms ()
{
    d_ptr = new Synthetic_mha_parms_private;

    output_type = PLM_IMG_TYPE_ITK_FLOAT;
    pattern = PATTERN_GAUSS;
    input_fn = "";

    for (int i = 0; i < 3; i++) {
        spacing[i] = 5.0f;
        dim[i] = 100;
        origin[i] = 0.0f;
        gauss_center[i] = 0.0f;
        gauss_std[i] = 100.0f;
        sphere_center[i] = 0.0f;
        sphere_radius[i] = 50.0f;
        donut_center[i] = 0.0f;
        lung_tumor_pos[i] = 0.0f;
        dose_center[i] = 0.0f;
        cylinder_center[i] = 0.0f;
        cylinder_radius[i] = 0.0f;
    }
    background = -1000.0f;
    foreground = 0.0f;
    background_alpha = 1.0f;
    foreground_alpha = 1.0f;
    m_want_ss_img = false;
    m_want_dose_img = false;
    image_normalization = NORMALIZATION_NONE;
    rect_size[0] = -50.0f;
    rect_size[1] = +50.0f;
    rect_size[2] = -50.0f;
    rect_size[3] = +50.0f;
    rect_size[4] = -50.0f;
    rect_size[5] = +50.0f;
    donut_radius[0] = 50.0f;
    donut_radius[1] = 50.0f;
    donut_radius[2] = 20.0f;
    donut_rings = 2;
    grid_spacing[0] = 10;
    grid_spacing[1] = 10;
    grid_spacing[2] = 10;
    penumbra = 5.0f;
    dose_size[0] = -50.0f;
    dose_size[1] = +50.0f;
    dose_size[2] = -50.0f;
    dose_size[3] = +50.0f;
    dose_size[4] = -50.0f;
    dose_size[5] = +50.0f;
    num_multi_sphere = 33;
    noise_mean = 0;
    noise_std = 1.f;
    gabor_use_k_fib = false;
    gabor_k_fib[0] = 0;
    gabor_k_fib[1] = 1;
    gabor_k[0] = 1.;
    gabor_k[1] = 0.;
    gabor_k[2] = 0.;
}

Synthetic_mha_parms::~Synthetic_mha_parms ()
{
    delete d_ptr;
}

static void 
synth_dose (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    /* sorry for this mess... */
    float x,x0,x1,y,y0,y1,f0,f1;

    float p[3];
    p[0] = phys[0] - parms->dose_center[0];
    p[1] = phys[1] - parms->dose_center[1];
    p[2] = phys[2] - parms->dose_center[2];

    /* uniform central dose */
    if (p[0] >= parms->dose_size[0] + parms->penumbra
        && p[0] <= parms->dose_size[1] - parms->penumbra
        && p[1] >= parms->dose_size[2] + parms->penumbra
        && p[1] <= parms->dose_size[3] - parms->penumbra
        && p[2] >= parms->dose_size[4] 
        && p[2] <= parms->dose_size[5])
    {
        *intens = parms->foreground;
        *label = 1;
    } else {
        *intens = parms->background;
        *label = 0;
    }

    if (p[2] >= parms->dose_size[4] && p[2] <= parms->dose_size[5]) {
        /* penumbra edges */
        if (p[1] > parms->dose_size[2]+parms->penumbra && p[1] < parms->dose_size[3]-parms->penumbra){
            x  = p[0];
            x0 = parms->dose_size[0];
            x1 = parms->dose_size[0] + parms->penumbra;
            f0 = parms->background;
            f1 = parms->foreground;
            if (x >= x0 && x < x1) {
                *intens = f0 + (x-x0)*((f1-f0)/(x1-x0));
            }

            x0 = parms->dose_size[1] - parms->penumbra;
            x1 = parms->dose_size[1];
            f0 = parms->foreground;
            f1 = parms->background;
            if (x >= x0 && x < x1) {
                *intens = f0 + (x-x0)*((f1-f0)/(x1-x0));
            }
        }
        if (p[0] > parms->dose_size[0]+parms->penumbra && p[0] < parms->dose_size[1]-parms->penumbra){
            y  = p[1];
            y0 = parms->dose_size[2];
            y1 = parms->dose_size[2] + parms->penumbra;
            f0 = parms->background;
            f1 = parms->foreground;
            if ((p[1] >= y0 && p[1] < y1)) {
                *intens = f0 + (y-y0)*((f1-f0)/(y1-y0));
            }

            y0 = parms->dose_size[3] - parms->penumbra;
            y1 = parms->dose_size[3];
            f0 = parms->foreground;
            f1 = parms->background;
            if (y >= y0 && y < y1) {
                *intens = f0 + (y-y0)*((f1-f0)/(y1-y0));
            }
        }
        
        /* penumbra corners */
        x = p[0];
        y = p[1];
        x0 = parms->dose_size[0];
        x1 = parms->dose_size[0] + parms->penumbra;
        y0 = parms->dose_size[2];
        y1 = parms->dose_size[2] + parms->penumbra;
        f0 = parms->background;
        f1 = parms->foreground;
        if (x > x0 && x < x1 && y > y0 && y < y1) {
            *intens = ((f0)/((x1-x0)*(y1-y0)))*(x1-x)*(y1-y) +
                ((f0)/((x1-x0)*(y1-y0)))*(x-x0)*(y1-y) +
                ((f0)/((x1-x0)*(y1-y0)))*(x1-x)*(y-y0) +
                ((f1)/((x1-x0)*(y1-y0)))*(x-x0)*(y-y0);
        }
        x = p[0];
        y = p[1];
        x0 = parms->dose_size[1] - parms->penumbra;
        x1 = parms->dose_size[1];
        y0 = parms->dose_size[2];
        y1 = parms->dose_size[2] + parms->penumbra;
        f0 = parms->background;
        f1 = parms->foreground;
        if (x > x0 && x < x1 && y > y0 && y < y1) {
            *intens = ((f0)/((x1-x0)*(y1-y0)))*(x1-x)*(y1-y) +
                ((f0)/((x1-x0)*(y1-y0)))*(x-x0)*(y1-y) +
                ((f1)/((x1-x0)*(y1-y0)))*(x1-x)*(y-y0) +
                ((f0)/((x1-x0)*(y1-y0)))*(x-x0)*(y-y0);
        }
        x = p[0];
        y = p[1];
        x0 = parms->dose_size[0];
        x1 = parms->dose_size[0] + parms->penumbra;
        y0 = parms->dose_size[3] - parms->penumbra;
        y1 = parms->dose_size[3];
        f0 = parms->background;
        f1 = parms->foreground;
        if (x > x0 && x < x1 && y > y0 && y < y1) {
            *intens = ((f0)/((x1-x0)*(y1-y0)))*(x1-x)*(y1-y) +
                ((f1)/((x1-x0)*(y1-y0)))*(x-x0)*(y1-y) +
                ((f0)/((x1-x0)*(y1-y0)))*(x1-x)*(y-y0) +
                ((f0)/((x1-x0)*(y1-y0)))*(x-x0)*(y-y0);
        }
        x = p[0];
        y = p[1];
        x0 = parms->dose_size[1] - parms->penumbra;
        x1 = parms->dose_size[1];
        y0 = parms->dose_size[3] - parms->penumbra;
        y1 = parms->dose_size[3];
        f0 = parms->background;
        f1 = parms->foreground;
        if (x > x0 && x < x1 && y > y0 && y < y1) {
            *intens = ((f1)/((x1-x0)*(y1-y0)))*(x1-x)*(y1-y) +
                ((f0)/((x1-x0)*(y1-y0)))*(x-x0)*(y1-y) +
                ((f0)/((x1-x0)*(y1-y0)))*(x1-x)*(y-y0) +
                ((f0)/((x1-x0)*(y1-y0)))*(x-x0)*(y-y0);
        }
    }
} /* z-direction */

static void 
synth_gauss (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    double f = 0;
    for (int d = 0; d < 3; d++) {
        double f1 = phys[d] - (double) parms->gauss_center[d];
        f1 = f1 / (double) parms->gauss_std[d];
        f += f1 * f1;
    }
    f = exp (-0.5 * f);            /* f \in (0,1] */

    *intens = (1 - f) * parms->background + f * parms->foreground;
    *label = (f > 0.2) ? 1 : 0;
}

static void 
synth_grid (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    int ijk[3];
    ijk[0] = (phys[0]/parms->spacing[0]) + (parms->dim[0]/2);
    ijk[1] = (phys[1]/parms->spacing[1]) + (parms->dim[1]/2);
    ijk[2] = (phys[2]/parms->spacing[2]) + (parms->dim[2]/2);

    if (
        ((ijk[0] % parms->grid_spacing[0] == 0) && (ijk[1] % parms->grid_spacing[1] == 0)) ||
        ((ijk[1] % parms->grid_spacing[1] == 0) && (ijk[2] % parms->grid_spacing[2] == 0)) ||
        ((ijk[2] % parms->grid_spacing[2] == 0) && (ijk[1] % parms->grid_spacing[1] == 0)) ||
        ((ijk[2] % parms->grid_spacing[2] == 0) && (ijk[0] % parms->grid_spacing[0] == 0))
    )
    {
        *intens = parms->foreground;
        *label = 1;
   } else {
        *intens = parms->background;
        *label = 0;
    }
}


static void 
synth_rect (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    if (phys[0] >= parms->rect_size[0] 
        && phys[0] <= parms->rect_size[1] 
        && phys[1] >= parms->rect_size[2] 
        && phys[1] <= parms->rect_size[3] 
        && phys[2] >= parms->rect_size[4] 
        && phys[2] <= parms->rect_size[5])
    {
        *intens 
            = (1 - parms->foreground_alpha) * (*intens) 
            + parms->foreground_alpha * parms->foreground;
        *label = 1;
    } else {
        *intens 
            = (1 - parms->background_alpha) * (*intens) 
            + parms->background_alpha * parms->background;
        *label = 0;
    }
}

static void 
synth_sphere (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    float f = 0;
    for (int d = 0; d < 3; d++) {
        float f1 = phys[d] - parms->sphere_center[d];
        f1 = f1 / parms->sphere_radius[d];
        f += f1 * f1;
    }
    if (f > 1.0) {
        *intens 
            = (1 - parms->background_alpha) * (*intens) 
            + parms->background_alpha * parms->background;
        *label = 0;
    } else {
        *intens 
            = (1 - parms->foreground_alpha) * (*intens) 
            + parms->foreground_alpha * parms->foreground;
        *label = 1;
    }
}

static void 
synth_multi_sphere (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    float f = 0;
    for (int d = 0; d < 3; d++) {
        float f1 = phys[d] - parms->sphere_center[d];
        f1 = f1 / parms->sphere_radius[d];
        f += f1 * f1;
    }
    if (f > 1.0) {
        *intens = parms->background;
        *label = 0;
    } else {
        *intens = parms->foreground;
        *label = 1;
    }
}

static void 
synth_donut (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    /* Set default values */
    *intens = parms->background;
    *label = 0;

    float p[3];
    for (int d = 0; d < 3; d++) {
        p[d] = (phys[d] - parms->donut_center[d]) / parms->donut_radius[d];
    }

    float dist = sqrt (p[0]*p[0] + p[1]*p[1]);

    /* Compute which ring we are inside */
    float ring_width = 1 / (float) parms->donut_rings;
    int ring_no = floor (dist / ring_width);

    /* If outside of all rings, return */
    if (ring_no >= parms->donut_rings) {
        return;
    }

    /* If within "background ring", return */
    if ((parms->donut_rings - ring_no) % 2 == 0) {
        return;
    }

    /* Compute distance from ring center */
    float ring_offset_1 = dist - ring_no * ring_width;
    float ring_offset_2 = (ring_no + 1) * ring_width - dist;
    float ring_offset = 0.5 * ring_width 
        - std::min (ring_offset_1, ring_offset_2);
    ring_offset = ring_offset / ring_width;

    /* If distance within donut, set to foreground */
    float dist_3d_sq = ring_offset * ring_offset + p[2] * p[2];

    if (dist_3d_sq < 1.) {
        *intens = parms->foreground;
        *label = 1;
    }
}

static void 
synth_lung (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    /* Set default values */
    *intens = parms->background;
    *label = 0;

    /* Get distance from central axis */
    float d2 = sqrt (phys[0]*phys[0] + phys[1]*phys[1]);

    /* If outside chest wall, return */
    if (d2 > 150) {
        return;
    }

    /* If within chest wall, set chest wall intensity */
    if (d2 > 130) {
        *intens = parms->foreground;
        *label = 1;
        return;
    }

    /* Get distance from tumor */
    float p[3] = { 
        phys[0] - parms->lung_tumor_pos[0],
        phys[1] - parms->lung_tumor_pos[1],
        phys[2] - parms->lung_tumor_pos[2]
    };
    float d3 = sqrt (p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);

    /* If within tumor, set tumor density */
    if (d3 < 20) {
        *label = 3;
        *intens = parms->foreground;
        return;
    }

    /* Otherwise, must be lung */
    *intens = -700;
    *label = 5;
}

static void 
synth_ramp (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{

    /* Get distance from origin */
    float d;
    if (parms->pattern == PATTERN_XRAMP) {
        d = phys[0] - parms->origin[0];
    }
    else if (parms->pattern == PATTERN_YRAMP) {
        d = phys[1] - parms->origin[1];
    }
    else {
        d = phys[2] - parms->origin[2];
    }

    /* Set intensity */
    *label = 0;
    *intens = d;
}

static void 
synth_noise (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    double r = parms->d_ptr->v.normal64();
    
    /* Set intensity */
    *label = 0;
    *intens = parms->noise_mean + (float) r * parms->noise_std;
}


static void 
synth_cylinder (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    float f = 0;
    for (int d = 0; d < 2; d++) {
        float f1 = phys[d] - parms->cylinder_center[d];
        f1 = f1 / parms->cylinder_radius[d];
        f += f1 * f1;
    }
    if (f > 1.0) {
        *intens 
            = (1 - parms->background_alpha) * (*intens) 
            + parms->background_alpha * parms->background;
        *label = 0;
    } else {
        *intens 
            = (1 - parms->foreground_alpha) * (*intens) 
            + parms->foreground_alpha * parms->foreground;
        *label = 1;
    }	
}

static void 
synth_gabor (
    float *intens, 
    unsigned char *label,
    const FloatPoint3DType& phys, 
    const Synthetic_mha_parms *parms
)
{
    /* Do gaussian first */
    float f = 0;
    float rel[3];
    for (int d = 0; d < 3; d++) {
        rel[d] = phys[d] - parms->gauss_center[d];
        float f1 = rel[d] / parms->gauss_std[d];
        f += f1 * f1;
    }
    f = exp (-0.5 * f);            /* f \in (0,1] */

    *intens = (1 - f) * parms->background + f * parms->foreground;
    *label = (f > 0.2) ? 1 : 0;

    /* Get the distance from the central plane in direction of 
       projection vector ("k" vector) */
    float proj_dist = 0.f;
    for (int d = 0; d < 3; d++) {
        proj_dist += rel[d] * parms->d_ptr->gabor_proj[d];
    }

    /* Modulate by cos for real part, or sin for imaginary part */
    *intens *= cos(parms->d_ptr->gabor_freq * proj_dist);
}

void
synthetic_mha (
    Rt_study *rt_study,
    Synthetic_mha_parms *parms
)
{
    FloatImageType::SizeType sz;
    FloatImageType::IndexType st;
    FloatImageType::RegionType rg;
    FloatImageType::PointType og;
    FloatImageType::SpacingType sp;
    FloatImageType::DirectionType itk_dc;
    FloatImageType::Pointer im_out;

    if (parms->input_fn != "") {
        /* Input image was specified */
        Plm_image pi (parms->input_fn);
        im_out = pi.itk_float ();

        /* GCS FIX: Ideally, the calling code will set the alpha values 
           properly.  Instead, here we set the background alpha to 0, 
           with the understanding that the caller probably wants to 
           paste a sphere or rectangle onto the existing image */
        parms->background_alpha = 0.0f;

    } else {
        /* No input image specified */
        if (parms->fixed_fn != "") {
            /* Get geometry from fixed image */
            Plm_image pi (parms->fixed_fn);
            Plm_image_header pih;
            pih.set_from_plm_image (&pi);
            og = pih.GetOrigin();
            sp = pih.GetSpacing();
            rg = pih.GetRegion();
            itk_dc = pih.GetDirection();
            for (int d1 = 0; d1 < 3; d1++) {
                parms->origin[d1] = og[d1];
            }
        } else {
            /* Get geometry from command line parms */
            for (int d1 = 0; d1 < 3; d1++) {
                st[d1] = 0;
                sz[d1] = parms->dim[d1];
                sp[d1] = parms->spacing[d1];
                og[d1] = parms->origin[d1];
            }
            rg.SetSize (sz);
            rg.SetIndex (st);
            itk_direction_from_dc (&itk_dc, parms->dc);
        }

        /* Create new ITK image for intensity */
        im_out = FloatImageType::New();
        im_out->SetRegions (rg);
        im_out->SetOrigin (og);
        im_out->SetSpacing (sp);
        im_out->SetDirection (itk_dc);
        im_out->Allocate ();

        /* Initialize to background */
        im_out->FillBuffer (parms->background);
    }

    /* Create new ITK images for ss and dose */
    UCharImageType::Pointer ss_img = UCharImageType::New();
    typedef itk::ImageRegionIteratorWithIndex< UCharImageType > 
        UCharIteratorType;
    UCharIteratorType ss_img_it;

    if (parms->m_want_ss_img) {
        ss_img->SetRegions (rg);
        ss_img->SetOrigin (og);
        ss_img->SetSpacing (sp);
        ss_img->Allocate();
        ss_img_it = UCharIteratorType (ss_img, 
            ss_img->GetLargestPossibleRegion());
        ss_img_it.GoToBegin();
    }

    FloatImageType::Pointer dose_img = FloatImageType::New();
    typedef itk::ImageRegionIteratorWithIndex< FloatImageType > 
        FloatIteratorType;
    FloatIteratorType dose_img_it;
    if (parms->m_want_dose_img) {
        dose_img->SetRegions (rg);
        dose_img->SetOrigin (og);
        dose_img->SetSpacing (sp);
        dose_img->Allocate();
        dose_img_it = FloatIteratorType (dose_img, 
            dose_img->GetLargestPossibleRegion());
        dose_img_it.GoToBegin();
    }

    /* Figure out Gabor settings */
    if (parms->pattern == PATTERN_GABOR) {
        /* Figure out projection direction */
        if (parms->gabor_use_k_fib) {
            /* Fibonacci method selected. */
            /* N.b. I'm not sure which is the better value for phi.
               So let's use the golden section value. */
            //float phi = (3. - sqrt(5.)) * M_PI;
            float phi = (((sqrt(5.) + 1.) / 2.) - 1.) * M_PI;
            float longitude = fmodf (phi * parms->gabor_k_fib[0], 2 * M_PI);
            float latitude_sin = -1. + 2. * ((float) parms->gabor_k_fib[0]) / 
                parms->gabor_k_fib[1];
            float latitude = asinf (latitude_sin);
            float latitude_cos = cos (latitude);
            parms->d_ptr->gabor_proj[0] = latitude_cos * cos (longitude);
            parms->d_ptr->gabor_proj[1] = latitude_cos * sin (longitude);
            parms->d_ptr->gabor_proj[2] = latitude_sin;
            
            lprintf ("Gabor: %d %d -> (%f %f) -> (%f %f %f)\n", 
                parms->gabor_k_fib[0],
                parms->gabor_k_fib[1],
                longitude, latitude, 
                parms->d_ptr->gabor_proj[0],
                parms->d_ptr->gabor_proj[1],
                parms->d_ptr->gabor_proj[2]);
        }
        else {
            /* Standard "k" vector supplied.  Need to normalize it. */
            float len = vec3_len (parms->gabor_k);
            if (len < FLOAT_SMALL_VECTOR_LENGTH) {
                parms->gabor_k[0] = 1.;
                parms->gabor_k[1] = 0.;
                parms->gabor_k[2] = 0.;
                len = 1.;
            }
            vec3_scale3 (parms->d_ptr->gabor_proj, parms->gabor_k, len);
        }

        /* Automatically set frequency based on width.  
           The value 1.5 is a heuristic. */
        float sum = 0.f;
        for (int d = 0; d < 3; d++) {
            sum += parms->gauss_std[d] * parms->gauss_std[d];
        }
        parms->d_ptr->gabor_freq = M_PI * 1.5 / sqrt(sum);
        parms->image_normalization = Synthetic_mha_parms::NORMALIZATION_GABOR;
    }

    /* Iterate through image, setting values */
    typedef itk::ImageRegionIteratorWithIndex< FloatImageType > IteratorType;
    IteratorType it_out (im_out, im_out->GetLargestPossibleRegion());
    for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
        FloatPoint3DType phys;
        //float intens = 0.0f;
        float intens = it_out.Get();
        unsigned char label_uchar = 0;

        /* Get 3D coordinates of voxel */
        FloatImageType::IndexType idx = it_out.GetIndex ();
        im_out->TransformIndexToPhysicalPoint (idx, phys);

        /* Compute intensity and label */
        switch (parms->pattern) {
        case PATTERN_GAUSS:
            synth_gauss (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_RECT:
            synth_rect (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_SPHERE:
            synth_sphere (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_MULTI_SPHERE:
            synth_multi_sphere (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_DONUT:
            synth_donut (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_DOSE:
            synth_dose (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_GRID:
            synth_grid (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_LUNG:
            synth_lung (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_XRAMP:
        case PATTERN_YRAMP:
        case PATTERN_ZRAMP:
            synth_ramp (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_NOISE:
            synth_noise (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_CYLINDER:
            synth_cylinder (&intens, &label_uchar, phys, parms);
            break;
        case PATTERN_GABOR:
            synth_gabor (&intens, &label_uchar, phys, parms);
            break;
        default:
            intens = 0.0f;
            label_uchar = 0;
            break;
        }

        /* Set intensity */
        it_out.Set (intens);

        /* Set structure */
        if (parms->m_want_ss_img) {
            ss_img_it.Set (label_uchar); 
            ++ss_img_it;
        }

        /* Set dose */
        if (parms->m_want_dose_img) {
            float dose = 0.;
            const float thresh = parms->background + 
                0.5 * (parms->foreground - parms->background);
            if (parms->foreground > parms->background 
                && intens > thresh)
            {
                dose = 15;
            } else if (parms->foreground < parms->background 
                && intens < thresh)
            {
                dose = 15;
            } else {
                dose = 0;
            }
            dose_img_it.Set (dose);
            ++dose_img_it;
        }
    }

    /* Normalize if requested */
    if (parms->image_normalization == Synthetic_mha_parms::NORMALIZATION_SUM_ONE) {
        float sum = 0.f;
        for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
            sum += it_out.Get();
        }
        if (sum > 1e-10) {
            for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
                it_out.Set(it_out.Get() / sum);
            }
        }
    }
    else if (parms->image_normalization == Synthetic_mha_parms::NORMALIZATION_SUM_SQR_ONE) {
        float sum = 0.f;
        for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
            float v = it_out.Get();
            sum += v * v;
        }
        sum = sqrt(sum);
        if (sum > 1e-10) {
            for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
                it_out.Set(it_out.Get() / sum);
            }
        }
    }
    else if (parms->image_normalization == Synthetic_mha_parms::NORMALIZATION_ZERO_MEAN_STD_ONE) {
        /* GCS FIX: I should really compute in a single pass.
           Oh God almighty, when did I get so lazy? */
        int num_vox = 0;
        float sum_1 = 0.f, sum_2 = 0.f;
        for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
            float v = it_out.Get();
            sum_1 += v;
            num_vox ++;
        }
        sum_1 = sum_1 / num_vox;
        for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
            float v = it_out.Get() - sum_1;
            it_out.Set(v);
            sum_2 += v * v;
        }
        sum_2 = sqrt(sum_2);
        if (sum_2 > 1e-10) {
            for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
                it_out.Set(it_out.Get() / sum_2);
            }
        }
    }
    else if (parms->image_normalization == Synthetic_mha_parms::NORMALIZATION_GABOR) {
        float sum_lo_1 = 0.f, sum_hi_1 = 0.f;
        float sum_lo_2 = 0.f, sum_hi_2 = 0.f;
        for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
            float v = it_out.Get();
            if (v < 0) {
                sum_lo_1 += v;
                sum_lo_2 += v * v;
            } else if (v > 0) {
                sum_hi_1 += v;
                sum_hi_2 += v * v;
            }
        }
        sum_lo_2 = M_SQRT1_2 * sqrt(sum_lo_2);
        sum_hi_2 = M_SQRT1_2 * sqrt(sum_hi_2);
        float sum_lo_3 = 0.f, sum_hi_3 = 0.f;
        if (sum_lo_2 > 1e-10 && sum_hi_2 > 1e-10) {
            if (sum_lo_2 < sum_hi_2) {
                sum_lo_2 = sum_hi_2 * -sum_lo_1 / sum_hi_1;
            } else {
                sum_hi_2 = sum_lo_2 * -sum_hi_1 / sum_lo_1;
            }
            for (it_out.GoToBegin(); !it_out.IsAtEnd(); ++it_out) {
                float v = it_out.Get();
                if (v < 0) {
                    it_out.Set(v / sum_lo_2);
                    sum_lo_3 += v / sum_lo_2;
                } else if (v > 0) {
                    it_out.Set(v / sum_hi_2);
                    sum_hi_3 += v / sum_hi_2;
                }
            }
        }
    }

    /* Insert images into rt_study */
    Plm_image::Pointer pli = Plm_image::New();
    pli->set_itk (im_out);
    rt_study->set_image (pli);
    if (parms->m_want_ss_img) {
        /* Create rtss & set into rt_study */
        Segmentation::Pointer seg = Segmentation::New ();
        rt_study->set_segmentation (seg);

        /* Insert ss_image into rtss */
        seg->set_ss_img (ss_img);

        /* Insert structure set into rtss */
        Rtss *rtss_ss = new Rtss;
        seg->set_structure_set (rtss_ss);

        /* Add structure names */
        switch (parms->pattern) {
        case PATTERN_LUNG:
            rtss_ss->add_structure (
                std::string ("Body"), std::string(), 1, 0);
            rtss_ss->add_structure (
                std::string ("Tumor"), std::string(), 2, 1);
            rtss_ss->add_structure (
                std::string ("Lung"), std::string(), 3, 2);
            break;
        default:
            rtss_ss->add_structure (
                std::string ("Foreground"), std::string(), 1, 0);
        }
    }
    if (parms->m_want_dose_img) {
        rt_study->set_dose (dose_img);
    }
}
