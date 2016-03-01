/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"

#include "compiler_warnings.h"
#include "logfile.h"
#include "geometry_chooser.h"
#include "itk_image_load.h"
#include "itk_resample.h"
#include "plm_file_format.h"
#include "plm_image_header.h"

class Geometry_chooser_private {
public:
    Geometry_chooser_private () {
        have_dim = false;
        have_origin = false;
        have_spacing = false;
        have_direction_cosines = false;
        have_pih_fix = false;
        have_pih_ref = false;
        have_pih_cmp = false;
    }
public:
    bool have_dim;
    bool have_origin;
    bool have_spacing;
    bool have_direction_cosines;
    Plm_image_header pih_manual;
    bool have_pih_fix;
    Plm_image_header pih_fix;
    bool have_pih_ref;
    Plm_image_header pih_ref;
    bool have_pih_cmp;
    Plm_image_header pih_cmp;

    Plm_image_header pih_best;
};

Geometry_chooser::Geometry_chooser ()
{
    d_ptr = new Geometry_chooser_private;
}

Geometry_chooser::~Geometry_chooser ()
{
    delete d_ptr;
}

void 
Geometry_chooser::set_reference_image (const char* image_fn)
{
    FloatImageType::Pointer image = itk_image_load_float (image_fn, 0);
    this->set_reference_image (image);
}

template<class T>
void 
Geometry_chooser::set_reference_image (
    const T& image)
{
    d_ptr->pih_ref.set_from_itk_image (image);
    d_ptr->have_pih_ref = true;
}

void 
Geometry_chooser::set_compare_image (const char* image_fn)
{
    FloatImageType::Pointer image = itk_image_load_float (image_fn, 0);
    this->set_compare_image (image);
}

void 
Geometry_chooser::set_compare_image (
    const UCharImageType::Pointer image)
{
    d_ptr->pih_cmp.set_from_itk_image (image);
    d_ptr->have_pih_cmp = true;
}

void 
Geometry_chooser::set_compare_image (
    const FloatImageType::Pointer image)
{
    d_ptr->pih_cmp.set_from_itk_image (image);
    d_ptr->have_pih_cmp = true;
}

void 
Geometry_chooser::set_fixed_image (const char* image_fn)
{
    Plm_file_format ff = plm_file_format_deduce (image_fn);
    if (ff == PLM_FILE_FMT_VF) {
        DeformationFieldType::Pointer fixed = itk_image_load_float_field (
            image_fn);
        this->set_fixed_image (fixed);
    }
    else {
        /* Hope for the best... */
        FloatImageType::Pointer fixed = itk_image_load_float (image_fn, 0);
	this->set_fixed_image (fixed);
    }
}

void 
Geometry_chooser::set_fixed_image (const std::string& image_fn)
{
    this->set_fixed_image (image_fn.c_str());
}

template<class T>
void 
Geometry_chooser::set_fixed_image (
    const T& image)
{
    d_ptr->pih_fix.set_from_itk_image (image);
    d_ptr->have_pih_fix = true;
}

void 
Geometry_chooser::set_dim (const plm_long dim[3])
{
    d_ptr->pih_manual.set_dim (dim);
    d_ptr->have_dim = true;
}

void 
Geometry_chooser::set_origin (const float origin[3])
{
    d_ptr->pih_manual.set_origin (origin);
    d_ptr->have_origin = true;
}

void 
Geometry_chooser::set_spacing (const float spacing[3])
{
    d_ptr->pih_manual.set_spacing (spacing);
    d_ptr->have_spacing = true;
}

void 
Geometry_chooser::set_direction_cosines (
    const Direction_cosines& direction_cosines)
{
    d_ptr->pih_manual.set_direction_cosines (direction_cosines);
    d_ptr->have_direction_cosines = true;
}

const Plm_image_header *
Geometry_chooser::get_geometry ()
{
    if (d_ptr->have_pih_ref) {
        if (d_ptr->have_pih_cmp) {
            d_ptr->pih_best.set_geometry_to_contain (
                d_ptr->pih_ref, d_ptr->pih_cmp);
        } else {
            d_ptr->pih_best.set (d_ptr->pih_ref);
        }
    }
    
    if (d_ptr->have_pih_fix) {
        d_ptr->pih_best.set (d_ptr->pih_fix);
    }

    if (d_ptr->have_dim) {
        plm_long dim[3];
        d_ptr->pih_manual.get_dim (dim);
        /* Auto-adjust, keep same image extent */
        if ((d_ptr->have_pih_ref || d_ptr->have_pih_fix) 
            && !d_ptr->have_spacing)
        {
            float extent[3];
            d_ptr->pih_best.get_image_extent (extent);
            float new_spacing[3];
            for (int d = 0; d < 3; d++) {
                if (dim[d] < 2) {
                    new_spacing[d] = 1.0;
                } else {
                    new_spacing[d] = extent[d] / (dim[d] - 1);
                }
            }
            d_ptr->pih_best.set_spacing(new_spacing);
            lprintf ("Optimizing spacing: %g %g %g\n",
                new_spacing[0], new_spacing[1], new_spacing[2]);
        }
        d_ptr->pih_best.set_dim (dim);
    }
    if (d_ptr->have_origin) {
        float origin[3];
        d_ptr->pih_manual.get_origin (origin);
        d_ptr->pih_best.set_origin (origin);
    }
    if (d_ptr->have_spacing) {
        float spacing[3];
        d_ptr->pih_manual.get_spacing (spacing);
        /* Auto-adjust, keep same image extent */
        if ((d_ptr->have_pih_ref || d_ptr->have_pih_fix) 
            && !d_ptr->have_dim)
        {
            float extent[3];
            d_ptr->pih_best.get_image_extent (extent);
            plm_long new_dim[3];
            for (int d = 0; d < 3; d++) {
                new_dim[d] = 1 + FLOOR_PLM_LONG (extent[d] / spacing[d]);
            }
            d_ptr->pih_best.set_dim (new_dim);
            lprintf ("Optimizing dim: %d %d %d\n", 
                new_dim[0], new_dim[1], new_dim[2]);
        }
        d_ptr->pih_best.set_spacing (spacing);
    }
    if (d_ptr->have_direction_cosines) {
        float direction_cosines[9];
        d_ptr->pih_manual.get_direction_cosines (direction_cosines);
        d_ptr->pih_best.set_direction_cosines (direction_cosines);
    }
    return &d_ptr->pih_best;
}

/* Explicit instantiations */
template PLMUTIL_API void Geometry_chooser::set_fixed_image<UCharImageType::Pointer> (const UCharImageType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_fixed_image<FloatImageType::Pointer> (const FloatImageType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_fixed_image<DeformationFieldType::Pointer> (const DeformationFieldType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_reference_image<UCharImageType::Pointer> (const UCharImageType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_reference_image<CharImageType::Pointer> (const CharImageType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_reference_image<UShortImageType::Pointer> (const UShortImageType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_reference_image<ShortImageType::Pointer> (const ShortImageType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_reference_image<UInt32ImageType::Pointer> (const UInt32ImageType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_reference_image<Int32ImageType::Pointer> (const Int32ImageType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_reference_image<FloatImageType::Pointer> (const FloatImageType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_reference_image<DoubleImageType::Pointer> (const DoubleImageType::Pointer&);
template PLMUTIL_API void Geometry_chooser::set_reference_image<DeformationFieldType::Pointer> (const DeformationFieldType::Pointer&);
