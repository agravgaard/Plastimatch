/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <itkImage.h>
#include <itkImageRegionIterator.h>

#include "itk_directions.h"
#include "itk_image_type.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_p.h"
#include "volume.h"

/* -----------------------------------------------------------------------
   Standard 3D image conversion
   ----------------------------------------------------------------------- */
template<class T, class U> 
T
Plm_image::convert_gpuit_to_itk (Volume *vol)
{
    typedef typename T::ObjectType ImageType;
    int i, d1, d2;
    U* img = (U*) vol->img;
    typename ImageType::SizeType sz;
    typename ImageType::IndexType st;
    typename ImageType::RegionType rg;
    typename ImageType::PointType og;
    typename ImageType::SpacingType sp;
    typename ImageType::DirectionType dc;

    /* Copy header & allocate data for itk */
    for (d1 = 0; d1 < 3; d1++) {
	st[d1] = 0;
	sz[d1] = vol->dim[d1];
	sp[d1] = vol->spacing[d1];
	og[d1] = vol->origin[d1];
	for (d2 = 0; d2 < 3; d2++) {
	    dc[d1][d2] = vol->direction_cosines[d1*3+d2];
	}
    }
    rg.SetSize (sz);
    rg.SetIndex (st);

    T itk_img = ImageType::New();
    itk_img->SetRegions (rg);
    itk_img->SetOrigin (og);
    itk_img->SetSpacing (sp);
    itk_img->SetDirection (dc);

    itk_img->Allocate();

    /* Copy data into itk */
    typedef itk::ImageRegionIterator< ImageType > IteratorType;
    IteratorType it (itk_img, rg);
    for (it.GoToBegin(), i=0; !it.IsAtEnd(); ++it, ++i) {
	/* Type conversion: U -> itk happens here */
	it.Set (img[i]);
    }

    /* Free input volume */
    this->free_volume ();

    /* Return the new image; caller will assign to correct member 
       and set type */
    return itk_img;
}

template<class T, class U> 
void
Plm_image::convert_itk_to_gpuit (T img)
{
    typedef typename T::ObjectType ImageType;
    int i, d1;
    typename ImageType::RegionType rg = img->GetLargestPossibleRegion ();
    typename ImageType::PointType og = img->GetOrigin();
    typename ImageType::SpacingType sp = img->GetSpacing();
    typename ImageType::SizeType sz = rg.GetSize();
    typename ImageType::DirectionType dc = img->GetDirection();
    typename ImageType::IndexType rgi = rg.GetIndex();

    /* Copy header & allocate data for gpuit float */
    plm_long dim[3];
    plm_long rgidx[3];
    float origin[3];
    float spacing[3];
    float direction_cosines[9];
    for (d1 = 0; d1 < 3; d1++) {
        dim[d1] = sz[d1];
        rgidx[d1] = rgi[d1];
        origin[d1] = og[d1];
        spacing[d1] = sp[d1];
    }
    dc_from_itk_direction (direction_cosines, &dc);

    /* Choose output data type */
    enum Volume_pixel_type pix_type;
    if (typeid (U) == typeid (unsigned char)){
        pix_type = PT_UCHAR;
        this->m_type = PLM_IMG_TYPE_GPUIT_UCHAR;
    }
    else if (typeid (U) == typeid (short)){
        pix_type = PT_SHORT;
        this->m_type = PLM_IMG_TYPE_GPUIT_SHORT;
    }
    else if (typeid (U) == typeid (float)) {
        pix_type = PT_FLOAT;
        this->m_type = PLM_IMG_TYPE_GPUIT_FLOAT;
    }
    else {
        lprintf ("unknown type conversion from itk to gpuit!\n");
        exit (0);
    }

    /* Create volume */
    Volume* vol = new Volume (dim, origin, spacing, direction_cosines, 
        pix_type, 1);
    U *vol_img = (U*) vol->img;

    /* Fix itk images with non-zero region indices */
    vol->move_origin_to_idx (rgidx);

    /* Copy data into gpuit */
    typedef typename itk::ImageRegionIterator< ImageType > IteratorType;
    IteratorType it (img, rg);
    for (it.GoToBegin(), i=0; !it.IsAtEnd(); ++it, ++i) {
        vol_img[i] = it.Get();
    }

    /* Fix volume into plm_image */
    d_ptr->m_vol.reset (vol);
}

/* -----------------------------------------------------------------------
   UCharVec image conversion
   ----------------------------------------------------------------------- */
void
Plm_image::convert_itk_uchar_to_itk_uchar_vec ()
{
    UCharImageType::Pointer itk_uchar = this->m_itk_uchar;

    /* Create the output image */
    UCharVecImageType::Pointer im_out = UCharVecImageType::New();
    itk_image_header_copy (im_out, itk_uchar);
    im_out->SetVectorLength (2);
    im_out->Allocate ();

    /* Copy data into itk */
    typedef itk::ImageRegionIterator< UCharImageType > UCharIteratorType;
    const UCharImageType::RegionType rgn_in 
	= itk_uchar->GetLargestPossibleRegion();
    UCharIteratorType it_in (itk_uchar, rgn_in);
    typedef itk::ImageRegionIterator< UCharVecImageType > UCharVecIteratorType;
    const UCharVecImageType::RegionType rgn_out
	= im_out->GetLargestPossibleRegion();
    UCharVecIteratorType it_out (im_out, rgn_out);

    itk::VariableLengthVector<unsigned char> v_out(2);
    for (it_in.GoToBegin(), it_out.GoToBegin();
	 !it_in.IsAtEnd();
	 ++it_in, ++it_out)
    {
	unsigned char v_in = it_in.Get ();
	v_out[0] = v_in;
	v_out[1] = 0;
	it_out.Set (v_out);
    }

    /* Free input volume */
    this->m_itk_uchar = 0;

    /* Set output volume */
    this->m_itk_uchar_vec = im_out;
}

void
Plm_image::convert_itk_uint32_to_itk_uchar_vec ()
{
    UInt32ImageType::Pointer itk_uint32 = this->m_itk_uint32;

    /* Create the output image */
    UCharVecImageType::Pointer im_out = UCharVecImageType::New();
    itk_image_header_copy (im_out, itk_uint32);
    im_out->SetVectorLength (4);
    im_out->Allocate ();

    /* Copy data into itk */
    typedef itk::ImageRegionIterator< UInt32ImageType > UInt32IteratorType;
    const UInt32ImageType::RegionType rgn_in 
	= itk_uint32->GetLargestPossibleRegion();
    UInt32IteratorType it_in (itk_uint32, rgn_in);
    typedef itk::ImageRegionIterator< UCharVecImageType > UCharVecIteratorType;
    const UCharVecImageType::RegionType rgn_out
	= im_out->GetLargestPossibleRegion();
    UCharVecIteratorType it_out (im_out, rgn_out);

    itk::VariableLengthVector<unsigned char> v_out(4);
    for (it_in.GoToBegin(), it_out.GoToBegin();
	 !it_in.IsAtEnd();
	 ++it_in, ++it_out)
    {
	uint32_t v_in = it_in.Get ();
	v_out[0] = v_in & 0x000000FF;
	v_out[1] = (v_in & 0x0000FF00) >> 8;
	v_out[2] = (v_in & 0x00FF0000) >> 16;
	v_out[3] = (v_in & 0xFF000000) >> 24;
	it_out.Set (v_out);
    }

    /* Free input volume */
    this->m_itk_uint32 = 0;

    /* Set output volume */
    this->m_itk_uchar_vec = im_out;
}

void
Plm_image::convert_gpuit_uint32_to_itk_uchar_vec ()
{
    int i, d;
    Volume* vol = this->get_vol ();
    uint32_t* img = (uint32_t*) vol->img;

    UCharVecImageType::Pointer im_out = UCharVecImageType::New();
    UCharVecImageType::RegionType rgn_out;
    UCharVecImageType::PointType og_out;
    UCharVecImageType::SpacingType sp_out;
    UCharVecImageType::RegionType::SizeType sz_out;
    UCharVecImageType::DirectionType dc;

    /* Copy header & allocate data for itk */
    for (d = 0; d < 3; d++) {
	sz_out[d] = vol->dim[d];
	og_out[d] = vol->origin[d];
	sp_out[d] = vol->spacing[d];
    }
    for (unsigned int d1 = 0; d1 < 3; d1++) {
	for (unsigned int d2 = 0; d2 < 3; d2++) {
	    dc[d1][d2] = vol->direction_cosines[d1*3+d2];
	}
    }
    rgn_out.SetSize (sz_out);
    im_out->SetRegions (rgn_out);
    im_out->SetOrigin (og_out);
    im_out->SetSpacing (sp_out);
    im_out->SetDirection (dc);

    /* Choose size of vectors for image */
    im_out->SetVectorLength (4);

    im_out->Allocate ();

    /* Copy data into itk */
    typedef itk::ImageRegionIterator< UCharVecImageType > IteratorType;
    IteratorType it (im_out, rgn_out);

    for (it.GoToBegin(), i=0; !it.IsAtEnd(); ++it, ++i) {
	/* GCS FIX: This is probably inefficient, unless the compiler 
	   is very, very smart (which I doubt) */
	/* GCS FIX: This puts the planes in the "wrong" order, 
	   with uint32_t MSB as first component of vector */
	it.Set (itk::VariableLengthVector<unsigned char> (
		(unsigned char*) &img[i], 4));
    }

    /* Free input volume */
    this->free_volume ();

    /* Set output volume */
    this->m_itk_uchar_vec = im_out;
}

void
Plm_image::convert_gpuit_uchar_vec_to_itk_uchar_vec ()
{
    int i, d;
    Volume* vol = this->get_vol ();
    unsigned char* img = (unsigned char*) vol->img;

    UCharVecImageType::Pointer im_out = UCharVecImageType::New();
    UCharVecImageType::RegionType rgn_out;
    UCharVecImageType::PointType og_out;
    UCharVecImageType::SpacingType sp_out;
    UCharVecImageType::RegionType::SizeType sz_out;
    UCharVecImageType::DirectionType dc;

    /* Copy header & allocate data for itk */
    for (d = 0; d < 3; d++) {
	sz_out[d] = vol->dim[d];
	og_out[d] = vol->origin[d];
	sp_out[d] = vol->spacing[d];
    }
    for (unsigned int d1 = 0; d1 < 3; d1++) {
	for (unsigned int d2 = 0; d2 < 3; d2++) {
	    dc[d1][d2] = vol->direction_cosines[d1*3+d2];
	}
    }
    rgn_out.SetSize (sz_out);
    im_out->SetRegions (rgn_out);
    im_out->SetOrigin (og_out);
    im_out->SetSpacing (sp_out);
    im_out->SetDirection (dc);

    /* Choose size of vectors for image, minimum of 2 planes for itk */
    int out_vec_len = vol->vox_planes;
    if (out_vec_len < 2) out_vec_len = 2;
    im_out->SetVectorLength (out_vec_len);

    im_out->Allocate ();

    /* Copy data into itk */
    typedef itk::ImageRegionIterator< UCharVecImageType > IteratorType;
    IteratorType it (im_out, rgn_out);

    itk::VariableLengthVector<unsigned char> v_out(out_vec_len);
    for (it.GoToBegin(), i=0; !it.IsAtEnd(); ++it) {
	for (int j = 0; j < vol->vox_planes; ++j, ++i) {
	    v_out[j] = img[i];
	}
	it.Set (v_out);
    }

    /* Free input volume */
    this->free_volume ();

    /* Set output volume */
    this->m_itk_uchar_vec = im_out;
}

void
Plm_image::convert_itk_uchar_vec_to_gpuit_uchar_vec ()
{
    UCharVecImageType::Pointer itk_uchar_vec = this->m_itk_uchar_vec;

    /* Copy header & allocate data for gpuit image */
    int i;
    UCharVecImageType::RegionType rg 
        = itk_uchar_vec->GetLargestPossibleRegion ();
    UCharVecImageType::PointType og = itk_uchar_vec->GetOrigin();
    UCharVecImageType::SpacingType sp = itk_uchar_vec->GetSpacing();
    UCharVecImageType::SizeType sz = rg.GetSize();
    UCharVecImageType::DirectionType itk_dc = itk_uchar_vec->GetDirection();
    plm_long dim[3];
    float origin[3];
    float spacing[3];
    float direction_cosines[9];
    for (int d = 0; d < 3; d++) {
	dim[d] = sz[d];
	origin[d] = og[d];
	spacing[d] = sp[d];
    }
    dc_from_itk_direction (direction_cosines, &itk_dc);
    int vox_planes = itk_uchar_vec->GetVectorLength ();

    Volume* vol = new Volume (dim, origin, spacing, direction_cosines, 
	PT_UCHAR_VEC_INTERLEAVED, vox_planes);

    unsigned char* vol_img = (unsigned char*) vol->img;

    /* Copy data into gpuit */
    typedef itk::ImageRegionIterator< UCharVecImageType > IteratorType;
    IteratorType it (itk_uchar_vec, rg);
    for (it.GoToBegin(), i=0; !it.IsAtEnd(); ++it) {
	itk::VariableLengthVector<unsigned char> v = it.Get();
	for (int j = 0; j < vox_planes; ++j, ++i) {
	    vol_img[i] = v[j];
	}
    }

    /* Free input volume */
    this->m_itk_uchar_vec = 0;

    /* Set output volume */
    d_ptr->m_vol.reset (vol);
    this->m_type = PLM_IMG_TYPE_GPUIT_UCHAR_VEC;
}


/* Explicit instantiations */
template PLMBASE_API UCharImageType::Pointer
Plm_image::convert_gpuit_to_itk<UCharImageType::Pointer, unsigned char> (Volume*);
template PLMBASE_API UCharImageType::Pointer
Plm_image::convert_gpuit_to_itk<UCharImageType::Pointer, float> (Volume*);
template PLMBASE_API ShortImageType::Pointer
Plm_image::convert_gpuit_to_itk<ShortImageType::Pointer, short> (Volume*);
template PLMBASE_API ShortImageType::Pointer
Plm_image::convert_gpuit_to_itk<ShortImageType::Pointer, float> (Volume*);
template PLMBASE_API UShortImageType::Pointer
Plm_image::convert_gpuit_to_itk<UShortImageType::Pointer, float> (Volume*);
template PLMBASE_API Int32ImageType::Pointer
Plm_image::convert_gpuit_to_itk<Int32ImageType::Pointer, unsigned char> (Volume*);
template PLMBASE_API Int32ImageType::Pointer
Plm_image::convert_gpuit_to_itk<Int32ImageType::Pointer, short> (Volume*);
template PLMBASE_API Int32ImageType::Pointer
Plm_image::convert_gpuit_to_itk<Int32ImageType::Pointer, uint32_t> (Volume*);
template PLMBASE_API Int32ImageType::Pointer
Plm_image::convert_gpuit_to_itk<Int32ImageType::Pointer, float> (Volume*);
template PLMBASE_API UInt32ImageType::Pointer
Plm_image::convert_gpuit_to_itk<UInt32ImageType::Pointer, unsigned char> (Volume*);
template PLMBASE_API UInt32ImageType::Pointer
Plm_image::convert_gpuit_to_itk<UInt32ImageType::Pointer, short> (Volume*);
template PLMBASE_API UInt32ImageType::Pointer
Plm_image::convert_gpuit_to_itk<UInt32ImageType::Pointer, uint32_t> (Volume*);
template PLMBASE_API UInt32ImageType::Pointer
Plm_image::convert_gpuit_to_itk<UInt32ImageType::Pointer, float> (Volume*);
template PLMBASE_API FloatImageType::Pointer
Plm_image::convert_gpuit_to_itk<FloatImageType::Pointer, unsigned char> (Volume*);
template PLMBASE_API FloatImageType::Pointer
Plm_image::convert_gpuit_to_itk<FloatImageType::Pointer, short> (Volume*);
template PLMBASE_API FloatImageType::Pointer
Plm_image::convert_gpuit_to_itk<FloatImageType::Pointer, float> (Volume*);
template PLMBASE_API DoubleImageType::Pointer
Plm_image::convert_gpuit_to_itk<DoubleImageType::Pointer, unsigned char> (Volume*);
template PLMBASE_API DoubleImageType::Pointer
Plm_image::convert_gpuit_to_itk<DoubleImageType::Pointer, float> (Volume*);

template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<UCharImageType::Pointer, unsigned char> (
    UCharImageType::Pointer);
template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<ShortImageType::Pointer, unsigned char> (
    ShortImageType::Pointer);
template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<FloatImageType::Pointer, unsigned char> (
    FloatImageType::Pointer);

template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<UCharImageType::Pointer,float> (
    UCharImageType::Pointer);
template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<CharImageType::Pointer,float> (
    CharImageType::Pointer);
template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<UShortImageType::Pointer,float> (
    UShortImageType::Pointer);
template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<ShortImageType::Pointer,float> (
    ShortImageType::Pointer);
template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<UInt32ImageType::Pointer,float> (
    UInt32ImageType::Pointer);
template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<Int32ImageType::Pointer,float> (
    Int32ImageType::Pointer);
template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<FloatImageType::Pointer,float> (
    FloatImageType::Pointer);
template PLMBASE_API void
Plm_image::convert_itk_to_gpuit<DoubleImageType::Pointer,float> (
    DoubleImageType::Pointer);
