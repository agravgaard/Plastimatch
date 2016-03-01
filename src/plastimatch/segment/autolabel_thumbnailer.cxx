/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsegment_config.h"
#include "itkImageRegionIterator.h"

#include "autolabel_thumbnailer.h"
#include "dlib_trainer.h"
#include "plm_image.h"
#include "thumbnail.h"

Autolabel_thumbnailer::Autolabel_thumbnailer ()
{
    thumb = new Thumbnail;
}

Autolabel_thumbnailer::~Autolabel_thumbnailer ()
{
    if (thumb) delete thumb;
}

void
Autolabel_thumbnailer::set_input_image (const char* fn)
{
    pli = plm_image_load (fn, PLM_IMG_TYPE_ITK_FLOAT);
    thumb->set_input_image (pli);
    thumb->set_thumbnail_dim (16);
    thumb->set_thumbnail_spacing (25.0f);
}

void
Autolabel_thumbnailer::set_input_image (const std::string& fn)
{
    this->set_input_image (fn.c_str());
}

Dlib_trainer::Dense_sample_type 
Autolabel_thumbnailer::make_sample (float slice_loc)
{
    thumb->set_slice_loc (slice_loc);
    FloatImageType::Pointer thumb_img = thumb->make_thumbnail ();

    itk::ImageRegionIterator< FloatImageType > thumb_it (
        thumb_img, thumb_img->GetLargestPossibleRegion());
    Dlib_trainer::Dense_sample_type d;
    for (int j = 0; j < 256; j++) {
        d(j) = thumb_it.Get();
        ++thumb_it;
    }
    return d;
}
