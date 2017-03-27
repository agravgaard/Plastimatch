/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "itkImageRegionIterator.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"

#if PLM_DCM_USE_DCMTK
#include "dcmtk_rt_study.h"
#endif
#include "file_util.h"
#include "itk_image_cast.h"
#include "itk_image_create.h"
#include "itk_image_load.h"
#include "itk_image_save.h"
#include "itk_image_type.h"
#include "itk_metadata.h"
#include "logfile.h"
#include "mha_io.h"
#include "nki_io.h"
#include "path_util.h"
#include "plm_image.h"
#include "plm_image_p.h"
#include "plm_image_type.h"
#include "plm_image_header.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "volume.h"

Plm_image::Plm_image () {
    this->init ();
}
Plm_image::Plm_image (const char* fname) {
    this->init ();
    this->load_native (fname);
}
Plm_image::Plm_image (const std::string& fname)
{
    this->init ();
    this->load_native (fname.c_str());
}
Plm_image::Plm_image (const char* fname, Plm_image_type type)
{
    this->init ();
    this->load (fname, type);
}
Plm_image::Plm_image (const std::string& fname, Plm_image_type type)
{
    this->init ();
    this->load (fname.c_str(), type);
}
Plm_image::Plm_image (UCharImageType::Pointer img)
{
    this->init ();
    this->set_itk (img);
}
Plm_image::Plm_image (CharImageType::Pointer img)
{
    this->init ();
    this->set_itk (img);
}
Plm_image::Plm_image (ShortImageType::Pointer img)
{
    this->init ();
    this->set_itk (img);
}
Plm_image::Plm_image (FloatImageType::Pointer img)
{
    this->init ();
    this->set_itk (img);
}
Plm_image::Plm_image (Plm_image_type type, const Plm_image_header& pih)
{
    this->init ();
    this->create (type, pih);
}
Plm_image::Plm_image (const Volume::Pointer& vol)
{
    this->init ();
    this->set_volume (vol);
}
Plm_image::Plm_image (Volume* vol)
{
    this->init ();
    this->set_volume (vol);
}
Plm_image::~Plm_image () {
    delete d_ptr;
}


/* -----------------------------------------------------------------------
    Creation / Destruction
   ----------------------------------------------------------------------- */
/* This function can only be called by constructor */
void
Plm_image::init ()
{
    d_ptr = new Plm_image_private;
    m_original_type = PLM_IMG_TYPE_UNDEFINED;
    m_type = PLM_IMG_TYPE_UNDEFINED;
}

/* This function can be called by anyone */
void
Plm_image::free ()
{
    d_ptr->m_vol.reset ();
    d_ptr->m_vol_list.clear ();

    m_original_type = PLM_IMG_TYPE_UNDEFINED;
    m_type = PLM_IMG_TYPE_UNDEFINED;

    m_itk_char = 0;
    m_itk_uchar = 0;
    m_itk_short = 0;
    m_itk_ushort = 0;
    m_itk_int32 = 0;
    m_itk_uint32 = 0;
    m_itk_float = 0;
    m_itk_double = 0;
    m_itk_uchar_vec = 0;
}

void
Plm_image::free_volume ()
{
    d_ptr->m_vol.reset();
}

bool
Plm_image::have_image ()
{
    return m_type != PLM_IMG_TYPE_UNDEFINED;
}


/* -----------------------------------------------------------------------
   Creating
   ----------------------------------------------------------------------- */
void
Plm_image::create (Plm_image_type type, const Plm_image_header& pih)
{
    switch (type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
    case PLM_IMG_TYPE_ITK_SHORT:
    case PLM_IMG_TYPE_ITK_USHORT:
    case PLM_IMG_TYPE_ITK_ULONG:
	print_and_exit ("Unhandled image type in Plm_image::create"
			" (type = %d)\n", this->m_type);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
        this->m_type = type;
        this->m_original_type = type;
	this->m_itk_float = itk_image_create<float> (pih);
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_GPUIT_UINT16:
    case PLM_IMG_TYPE_GPUIT_UINT32:
    case PLM_IMG_TYPE_GPUIT_FLOAT:
    case PLM_IMG_TYPE_GPUIT_FLOAT_FIELD:
    default:
	print_and_exit ("Unhandled image type in Plm_image::create"
			" (type = %d)\n", this->m_type);
	break;
    }
}

/* -----------------------------------------------------------------------
   Cloning
   ----------------------------------------------------------------------- */
Plm_image::Pointer
Plm_image::clone (void)
{
    Plm_image::Pointer pli = Plm_image::New();
    if (!pli) return pli;

    pli->m_original_type = this->m_original_type;
    pli->m_type = this->m_type;

    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	pli->m_itk_uchar = this->m_itk_uchar;
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	pli->m_itk_short = this->m_itk_short;
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
	pli->m_itk_ushort = this->m_itk_ushort;
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	pli->m_itk_uint32 = this->m_itk_uint32;
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	pli->m_itk_float = this->m_itk_float;
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_GPUIT_UINT16:
    case PLM_IMG_TYPE_GPUIT_UINT32:
    case PLM_IMG_TYPE_GPUIT_FLOAT:
    case PLM_IMG_TYPE_GPUIT_FLOAT_FIELD:
        pli->d_ptr->m_vol = this->d_ptr->m_vol->clone ();
        break;
    default:
	print_and_exit ("Unhandled image type in Plm_image::clone"
			" (type = %d)\n", this->m_type);
	break;
    }

    return pli;
}

Plm_image::Pointer
Plm_image::clone (const Plm_image::Pointer& pli)
{
    return pli->clone ();
}

/* -----------------------------------------------------------------------
   Loading
   ----------------------------------------------------------------------- */
Plm_image::Pointer
plm_image_load (const char* fname, Plm_image_type type)
{
    Plm_image::Pointer pli = Plm_image::New();
    if (pli->load (fname, type)) {
        return pli;
    } 
    else {
        return Plm_image::Pointer();
    }
}

Plm_image::Pointer
plm_image_load (const std::string& fname, Plm_image_type type)
{
    return plm_image_load (fname.c_str(), type);
}

Plm_image::Pointer
plm_image_load_native (const char* fname)
{
    Plm_image::Pointer pli = Plm_image::New();

    if (pli->load_native (fname)) {
        return pli;
    }
    else {
        return Plm_image::Pointer();
    }
}

Plm_image::Pointer
plm_image_load_native (const std::string& fname)
{
    return plm_image_load_native (fname.c_str());
}

bool
Plm_image::load (const char* fname, Plm_image_type type)
{
    this->free ();
    if (!this->load_native(fname)) {
        return false;
    }
    this->convert (type);
    return true;
}

bool
Plm_image::load_native (const char* fname)
{
    itk::ImageIOBase::IOPixelType pixel_type;
    itk::ImageIOBase::IOComponentType component_type;
    int num_dimensions, num_components;

    if (is_directory (fname)) {
	/* GCS FIX: The call to is_directory is redundant -- we already 
	   called plm_file_format_deduce() in warp_main() */
	return load_native_dicom (fname);
    }

    if (!file_exists (fname) && !string_starts_with (fname, "slicer:")) {
 	lprintf ("Couldn't open %s for read\n", fname);
        return false;
    }

    /* Check for NKI filetype, which doesn't use ITK reader */
    if (extension_is (fname, "scan") || extension_is (fname, "SCAN")) {
        return load_native_nki (fname);
    }

    std::string fn = fname;
    itk_image_get_props (fname, &num_dimensions, &pixel_type, 
	&component_type, &num_components);

    /* Handle ss_image as a special case */
    if (num_components > 1 && component_type == itk::ImageIOBase::UCHAR) {
	this->m_itk_uchar_vec = itk_image_load_uchar_vec (fname);
	this->m_original_type = PLM_IMG_TYPE_ITK_UCHAR_VEC;
	this->m_type = PLM_IMG_TYPE_ITK_UCHAR_VEC;
	return true;
    }

    switch (component_type) {
    case itk::ImageIOBase::CHAR:
	this->m_itk_char = itk_image_load_char (fname, 0);
	this->m_original_type = PLM_IMG_TYPE_ITK_CHAR;
	this->m_type = PLM_IMG_TYPE_ITK_CHAR;
	break;
    case itk::ImageIOBase::UCHAR:
	this->m_itk_uchar = itk_image_load_uchar (fname, 0);
	this->m_original_type = PLM_IMG_TYPE_ITK_UCHAR;
	this->m_type = PLM_IMG_TYPE_ITK_UCHAR;
	break;
    case itk::ImageIOBase::SHORT:
	this->m_itk_short = itk_image_load_short (fname, 0);
	this->m_original_type = PLM_IMG_TYPE_ITK_SHORT;
	this->m_type = PLM_IMG_TYPE_ITK_SHORT;
	break;
    case itk::ImageIOBase::USHORT:
	this->m_itk_ushort = itk_image_load_ushort (fname, 0);
	this->m_original_type = PLM_IMG_TYPE_ITK_USHORT;
	this->m_type = PLM_IMG_TYPE_ITK_USHORT;
	break;
#if (CMAKE_SIZEOF_UINT == 4)
    case itk::ImageIOBase::INT:
#endif
#if (CMAKE_SIZEOF_ULONG == 4)
    case itk::ImageIOBase::LONG:
#endif
	this->m_itk_int32 = itk_image_load_int32 (fname, 0);
	this->m_original_type = PLM_IMG_TYPE_ITK_LONG;
	this->m_type = PLM_IMG_TYPE_ITK_LONG;
	break;
#if (CMAKE_SIZEOF_UINT == 4)
    case itk::ImageIOBase::UINT:
#endif
#if (CMAKE_SIZEOF_ULONG == 4)
    case itk::ImageIOBase::ULONG:
#endif
	this->m_itk_uint32 = itk_image_load_uint32 (fname, 0);
	this->m_original_type = PLM_IMG_TYPE_ITK_ULONG;
	this->m_type = PLM_IMG_TYPE_ITK_ULONG;
	break;
    case itk::ImageIOBase::FLOAT:
	this->m_itk_float = itk_image_load_float (fname, 0);
	this->m_original_type = PLM_IMG_TYPE_ITK_FLOAT;
	this->m_type = PLM_IMG_TYPE_ITK_FLOAT;
	break;
    case itk::ImageIOBase::DOUBLE:
	this->m_itk_double = itk_image_load_double (fname, 0);
	this->m_original_type = PLM_IMG_TYPE_ITK_DOUBLE;
	this->m_type = PLM_IMG_TYPE_ITK_DOUBLE;
	break;
    default:
	lprintf ("Error, unsupported input type in load_native(): %d\n",
	    component_type);
        return false;
    }
    return true;
}

bool
Plm_image::load_native (const std::string& fn)
{
    return this->load_native (fn.c_str());
}

bool
Plm_image::load_native_dicom (const char* fname)
{
#if PLM_DCM_USE_DCMTK

    Dcmtk_rt_study drs (fname);
    drs.parse_directory ();
    this->set (drs.get_image ());

    /* GCS FIX: Here we need to set the metadata loaded from 
       DICOM into the Plm_image */

#else
    /* GCS FIX: We don't yet have a way of getting original pixel type 
	for dicom.  Force SHORT */
    /* FIX: Patient position / direction cosines not set */
    this->m_itk_short = itk_image_load_short (fname, 0);
    this->m_original_type = PLM_IMG_TYPE_ITK_SHORT;
    this->m_type = PLM_IMG_TYPE_ITK_SHORT;
#endif

    return true;
}

bool
Plm_image::load_native_nki (const char* fname)
{
    Volume *v = nki_load (fname);
    if (v) {
        d_ptr->m_vol.reset(v);
        this->m_original_type = PLM_IMG_TYPE_ITK_SHORT;
        this->m_type = PLM_IMG_TYPE_GPUIT_SHORT;
        return true;
    }
    return false;
}


/* -----------------------------------------------------------------------
   Saving
   ----------------------------------------------------------------------- */
void
Plm_image::save_short_dicom (
    const char* fname, 
    Rt_study_metadata *rsm
)
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	itk_image_save_short_dicom (this->m_itk_uchar, fname, rsm);
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	itk_image_save_short_dicom (this->m_itk_short, fname, rsm);
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
	itk_image_save_short_dicom (this->m_itk_ushort, fname, rsm);
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	itk_image_save_short_dicom (this->m_itk_uint32, fname, rsm);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	itk_image_save_short_dicom (this->m_itk_float, fname, rsm);
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
	this->convert_to_itk_uchar ();
	itk_image_save_short_dicom (this->m_itk_uchar, fname, rsm);
	break;
    case PLM_IMG_TYPE_GPUIT_SHORT:
	this->convert_to_itk_short ();
	itk_image_save_short_dicom (this->m_itk_short, fname, rsm);
	break;
    case PLM_IMG_TYPE_GPUIT_UINT32:
	this->convert_to_itk_uint32 ();
	itk_image_save_short_dicom (this->m_itk_uint32, fname, rsm);
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->convert_to_itk_float ();
	itk_image_save_short_dicom (this->m_itk_float, fname, rsm);
	break;
    case PLM_IMG_TYPE_GPUIT_UINT16:
    default:
	print_and_exit ("Unhandled image type in Plm_image::save_short_dicom"
			" (type = %d)\n", this->m_type);
	break;
    }
}

void
Plm_image::save_short_dicom (
    const std::string& fname, 
    Rt_study_metadata *rsm
)
{
    this->save_short_dicom (fname.c_str(), rsm);
}

void
Plm_image::save_image (const char* fname)
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_CHAR:
	itk_image_save (this->m_itk_char, fname);
	break;
    case PLM_IMG_TYPE_ITK_UCHAR:
	itk_image_save (this->m_itk_uchar, fname);
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	itk_image_save (this->m_itk_short, fname);
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
	itk_image_save (this->m_itk_ushort, fname);
	break;
    case PLM_IMG_TYPE_ITK_LONG:
	itk_image_save (this->m_itk_int32, fname);
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	itk_image_save (this->m_itk_uint32, fname);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	itk_image_save (this->m_itk_float, fname);
	break;
    case PLM_IMG_TYPE_ITK_DOUBLE:
	itk_image_save (this->m_itk_double, fname);
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
	this->convert_to_itk_uchar ();
	itk_image_save (this->m_itk_uchar, fname);
	break;
    case PLM_IMG_TYPE_GPUIT_SHORT:
	this->convert_to_itk_short ();
	itk_image_save (this->m_itk_short, fname);
	break;
    case PLM_IMG_TYPE_GPUIT_UINT32:
	this->convert_to_itk_uint32 ();
	itk_image_save (this->m_itk_uint32, fname);
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->convert_to_itk_float ();
	itk_image_save (this->m_itk_float, fname);
	break;
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
	itk_image_save (this->m_itk_uchar_vec, fname);
	break;
    case PLM_IMG_TYPE_GPUIT_UINT16:
    default:
	print_and_exit ("Unhandled image type in Plm_image::save_image"
	    " (type = %s)\n", plm_image_type_string (this->m_type));
	break;
    }
}

void
Plm_image::save_image (const std::string& fname)
{
    this->save_image (fname.c_str());
}

/* -----------------------------------------------------------------------
   Getting and setting
   ----------------------------------------------------------------------- */
void 
Plm_image::set (const Plm_image::Pointer& pli)
{
    switch (pli->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
        m_itk_uchar = pli->m_itk_uchar;
	break;
    case PLM_IMG_TYPE_ITK_CHAR:
        m_itk_char = pli->m_itk_char;
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
        m_itk_ushort = pli->m_itk_ushort;
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
        m_itk_short = pli->m_itk_short;
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
        m_itk_uint32 = pli->m_itk_uint32;
	break;
    case PLM_IMG_TYPE_ITK_LONG:
        m_itk_int32 = pli->m_itk_int32;
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
        m_itk_float = pli->m_itk_float;
	break;
    case PLM_IMG_TYPE_ITK_DOUBLE:
        m_itk_double = pli->m_itk_double;
	break;
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
        m_itk_uchar_vec = pli->m_itk_uchar_vec;
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_GPUIT_UINT16:
    case PLM_IMG_TYPE_GPUIT_UINT32:
    case PLM_IMG_TYPE_GPUIT_INT32:
    case PLM_IMG_TYPE_GPUIT_FLOAT:
    case PLM_IMG_TYPE_GPUIT_FLOAT_FIELD:
    case PLM_IMG_TYPE_GPUIT_UCHAR_VEC:
        d_ptr->m_vol = pli->d_ptr->m_vol;
	break;
    default:
	print_and_exit ("Unhandled image type in Plm_image::set()"
	    " (type = %s)\n", plm_image_type_string (pli->m_type));
	break;
    }

    this->m_original_type = pli->m_original_type;
    this->m_type = pli->m_type;
}

void 
Plm_image::set_volume (const Volume::Pointer& v, Plm_image_type type)
{
    this->free ();
    d_ptr->m_vol = v;
    m_original_type = type;
    m_type = type;
}

void 
Plm_image::set_volume (const Volume::Pointer& v)
{
    switch (v->pix_type) {
    case PT_UCHAR:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_UCHAR);
	break;
    case PT_SHORT:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_SHORT);
	break;
    case PT_UINT16:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_UINT16);
	break;
    case PT_UINT32:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_UINT32);
	break;
    case PT_INT32:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_INT32);
	break;
    case PT_FLOAT:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_FLOAT);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_FLOAT_FIELD);
        break;
    case PT_UCHAR_VEC_INTERLEAVED:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_UCHAR_VEC);
	break;
    default:
	print_and_exit ("Undefined conversion in Plm_image::set_volume\n");
	break;
    }
}

void 
Plm_image::set_volume (Volume *v, Plm_image_type type)
{
    this->free ();
    d_ptr->m_vol.reset (v);
    m_original_type = type;
    m_type = type;
}

void 
Plm_image::set_volume (Volume *v)
{
    switch (v->pix_type) {
    case PT_UCHAR:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_UCHAR);
	break;
    case PT_SHORT:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_SHORT);
	break;
    case PT_UINT16:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_UINT16);
	break;
    case PT_UINT32:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_UINT32);
	break;
    case PT_INT32:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_INT32);
	break;
    case PT_FLOAT:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_FLOAT);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_FLOAT_FIELD);
        break;
    case PT_UCHAR_VEC_INTERLEAVED:
        this->set_volume (v, PLM_IMG_TYPE_GPUIT_UCHAR_VEC);
	break;
    default:
	print_and_exit ("Undefined conversion in Plm_image::set_volume\n");
	break;
    }
}

Volume::Pointer&
Plm_image::get_volume ()
{
    return d_ptr->m_vol;
}

Volume::Pointer&
Plm_image::get_volume_uchar ()
{
    convert_to_gpuit_uchar ();
    return d_ptr->m_vol;
}

Volume::Pointer&
Plm_image::get_volume_short ()
{
    convert_to_gpuit_short ();
    return d_ptr->m_vol;
}

Volume::Pointer&
Plm_image::get_volume_float ()
{
    convert_to_gpuit_float ();
    return d_ptr->m_vol;
}

Volume::Pointer&
Plm_image::get_volume_uchar_vec () {
    convert_to_gpuit_uchar_vec ();
    return d_ptr->m_vol;
}

Volume *
Plm_image::get_vol ()
{
    return d_ptr->m_vol.get();
}

const Volume *
Plm_image::get_vol () const
{
    return d_ptr->m_vol.get();
}

void 
Plm_image::set_itk (UCharImageType::Pointer img)
{
    this->free ();
    m_original_type = PLM_IMG_TYPE_ITK_UCHAR;
    m_type = PLM_IMG_TYPE_ITK_UCHAR;
    this->m_itk_uchar = img;
}

void 
Plm_image::set_itk (CharImageType::Pointer img)
{
    this->free ();
    m_original_type = PLM_IMG_TYPE_ITK_CHAR;
    m_type = PLM_IMG_TYPE_ITK_CHAR;
    this->m_itk_char = img;
}

void 
Plm_image::set_itk (UShortImageType::Pointer img)
{
    this->free ();
    m_original_type = PLM_IMG_TYPE_ITK_USHORT;
    m_type = PLM_IMG_TYPE_ITK_USHORT;
    this->m_itk_ushort = img;
}

void 
Plm_image::set_itk (ShortImageType::Pointer img)
{
    this->free ();
    m_original_type = PLM_IMG_TYPE_ITK_SHORT;
    m_type = PLM_IMG_TYPE_ITK_SHORT;
    this->m_itk_short = img;
}

void 
Plm_image::set_itk (UInt32ImageType::Pointer img)
{
    this->free ();
    m_original_type = PLM_IMG_TYPE_ITK_ULONG;
    m_type = PLM_IMG_TYPE_ITK_ULONG;
    this->m_itk_uint32 = img;
}

void 
Plm_image::set_itk (Int32ImageType::Pointer img)
{
    this->free ();
    m_original_type = PLM_IMG_TYPE_ITK_LONG;
    m_type = PLM_IMG_TYPE_ITK_LONG;
    this->m_itk_int32 = img;
}

void 
Plm_image::set_itk (FloatImageType::Pointer img)
{
    this->free ();
    m_original_type = PLM_IMG_TYPE_ITK_FLOAT;
    m_type = PLM_IMG_TYPE_ITK_FLOAT;
    this->m_itk_float = img;
}

void 
Plm_image::set_itk (DoubleImageType::Pointer img)
{
    this->free ();
    m_original_type = PLM_IMG_TYPE_ITK_DOUBLE;
    m_type = PLM_IMG_TYPE_ITK_DOUBLE;
    this->m_itk_double = img;
}

void 
Plm_image::set_itk (UCharVecImageType::Pointer img)
{
    this->free ();
    m_original_type = PLM_IMG_TYPE_ITK_UCHAR_VEC;
    m_type = PLM_IMG_TYPE_ITK_UCHAR_VEC;
    this->m_itk_uchar_vec = img;
}

/* -----------------------------------------------------------------------
   Conversion
   ----------------------------------------------------------------------- */
#define CONVERT_ITK_ITK(out_type,in_type)				\
    (this->m_itk_##out_type = cast_##out_type (this->m_itk_##in_type),	\
	this->m_itk_##in_type = 0)

void
Plm_image::convert_to_itk_uchar (void)
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	return;
    case PLM_IMG_TYPE_ITK_CHAR:
	CONVERT_ITK_ITK (uchar, char);
	return;
    case PLM_IMG_TYPE_ITK_USHORT:
	CONVERT_ITK_ITK (uchar, ushort);
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	CONVERT_ITK_ITK (uchar, short);
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	CONVERT_ITK_ITK (uchar, uint32);
	break;
    case PLM_IMG_TYPE_ITK_LONG:
	CONVERT_ITK_ITK (uchar, int32);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	CONVERT_ITK_ITK (uchar, float);
	break;
    case PLM_IMG_TYPE_ITK_DOUBLE:
	CONVERT_ITK_ITK (uchar, double);
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
	this->m_itk_uchar = this->convert_gpuit_to_itk<
            UCharImageType::Pointer, unsigned char> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->m_itk_uchar = this->convert_gpuit_to_itk<
            UCharImageType::Pointer, float> (this->get_vol());
	break;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to itk_uchar\n",
	    plm_image_type_string (this->m_type));
	return;
    }
    this->m_type = PLM_IMG_TYPE_ITK_UCHAR;
}

void
Plm_image::convert_to_itk_char (void)
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_CHAR:
	return;
    case PLM_IMG_TYPE_ITK_SHORT:
	CONVERT_ITK_ITK (char, short);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	CONVERT_ITK_ITK (char, float);
	break;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to itk_char\n",
	    plm_image_type_string (this->m_type));
	return;
    }
    this->m_type = PLM_IMG_TYPE_ITK_CHAR;
}

void
Plm_image::convert_to_gpuit_uchar (void)
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
        this->convert_itk_to_gpuit<UCharImageType::Pointer,unsigned char> (
            this->m_itk_uchar);
        this->m_itk_uchar = 0;
        break;
    case PLM_IMG_TYPE_ITK_SHORT:
        this->convert_itk_to_gpuit<ShortImageType::Pointer,unsigned char> (
            this->m_itk_short);
        this->m_itk_short = 0;
        break;
    case PLM_IMG_TYPE_ITK_FLOAT:
        this->convert_itk_to_gpuit<FloatImageType::Pointer,unsigned char> (
            this->m_itk_float);
        this->m_itk_float = 0;
        break;
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_GPUIT_FLOAT:
        volume_convert_to_uchar (this->get_vol());
        break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
        return;
    default:
        print_and_exit (
            "Error: unhandled conversion from %s to itk_uchar\n",
            plm_image_type_string (this->m_type));
        return;
    }
}


void
Plm_image::convert_to_itk_short (void)
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_SHORT:
	return;
    case PLM_IMG_TYPE_ITK_ULONG:
	CONVERT_ITK_ITK (short, uint32);
	break;
    case PLM_IMG_TYPE_ITK_LONG:
	CONVERT_ITK_ITK (short, int32);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	CONVERT_ITK_ITK (short, float);
	break;
    case PLM_IMG_TYPE_GPUIT_SHORT:
	this->m_itk_short = this->convert_gpuit_to_itk<
            ShortImageType::Pointer, short> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->m_itk_short = this->convert_gpuit_to_itk<
            ShortImageType::Pointer, float> (this->get_vol());
	break;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to itk_short\n",
	    plm_image_type_string (this->m_type));
	return;
    }
    this->m_type = PLM_IMG_TYPE_ITK_SHORT;
}

void
Plm_image::convert_to_itk_ushort (void)
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_SHORT:
	CONVERT_ITK_ITK (ushort, short);
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
	return;
    case PLM_IMG_TYPE_ITK_FLOAT:
	CONVERT_ITK_ITK (ushort, float);
	return;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->m_itk_ushort = this->convert_gpuit_to_itk<
            UShortImageType::Pointer, float> (this->get_vol());
	break;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to itk_ushort\n",
	    plm_image_type_string (this->m_type));
	return;
    }
    this->m_type = PLM_IMG_TYPE_ITK_USHORT;
}

void
Plm_image::convert_to_itk_int32 (void)
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_ULONG:
	return;
    case PLM_IMG_TYPE_ITK_FLOAT:
	CONVERT_ITK_ITK (int32, float);
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
	this->m_itk_int32 = this->convert_gpuit_to_itk<
            Int32ImageType::Pointer, unsigned char> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_SHORT:
	this->m_itk_int32 = this->convert_gpuit_to_itk<
            Int32ImageType::Pointer, short> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_UINT32:
	this->m_itk_int32 = this->convert_gpuit_to_itk<
            Int32ImageType::Pointer, uint32_t> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->m_itk_int32 = this->convert_gpuit_to_itk<
            Int32ImageType::Pointer, float> (this->get_vol());
	break;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to itk_int32\n",
	    plm_image_type_string (this->m_type));
	return;
    }
    this->m_type = PLM_IMG_TYPE_ITK_ULONG;
}

void
Plm_image::convert_to_itk_uint32 (void)
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	CONVERT_ITK_ITK (uint32, uchar);
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	CONVERT_ITK_ITK (uint32, short);
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	return;
    case PLM_IMG_TYPE_ITK_FLOAT:
	CONVERT_ITK_ITK (uint32, float);
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
	this->m_itk_uint32 = this->convert_gpuit_to_itk<
            UInt32ImageType::Pointer, unsigned char> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_SHORT:
	this->m_itk_uint32 = this->convert_gpuit_to_itk<
            UInt32ImageType::Pointer, short> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_UINT32:
	this->m_itk_uint32 = this->convert_gpuit_to_itk<
            UInt32ImageType::Pointer, uint32_t> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->m_itk_uint32 = this->convert_gpuit_to_itk<
            UInt32ImageType::Pointer, float> (this->get_vol());
	break;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to itk_uint32\n",
	    plm_image_type_string (this->m_type));
	return;
    }
    this->m_type = PLM_IMG_TYPE_ITK_ULONG;
}

void
Plm_image::convert_to_itk_float ()
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	CONVERT_ITK_ITK (float, uchar);
	break;
    case PLM_IMG_TYPE_ITK_CHAR:
	CONVERT_ITK_ITK (float, char);
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
	CONVERT_ITK_ITK (float, ushort);
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	CONVERT_ITK_ITK (float, short);
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	CONVERT_ITK_ITK (float, uint32);
	break;
    case PLM_IMG_TYPE_ITK_LONG:
	CONVERT_ITK_ITK (float, int32);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	return;
    case PLM_IMG_TYPE_ITK_DOUBLE:
	CONVERT_ITK_ITK (float, double);
	return;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
	this->m_itk_float = this->convert_gpuit_to_itk<
            FloatImageType::Pointer, unsigned char> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_SHORT:
	this->m_itk_float = this->convert_gpuit_to_itk<
            FloatImageType::Pointer, short> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->m_itk_float = this->convert_gpuit_to_itk<
            FloatImageType::Pointer, float> (this->get_vol());
	break;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to itk_float\n",
	    plm_image_type_string (this->m_type));
	return;
    }
    this->m_type = PLM_IMG_TYPE_ITK_FLOAT;
}

void
Plm_image::convert_to_itk_double ()
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	CONVERT_ITK_ITK (double, uchar);
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	CONVERT_ITK_ITK (double, short);
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	CONVERT_ITK_ITK (double, uint32);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	CONVERT_ITK_ITK (double, float);
	break;
    case PLM_IMG_TYPE_ITK_DOUBLE:
	return;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
	this->m_itk_double = this->convert_gpuit_to_itk<
            DoubleImageType::Pointer, unsigned char> (this->get_vol());
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->m_itk_double = this->convert_gpuit_to_itk<
            DoubleImageType::Pointer, float> (this->get_vol());
        break;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to itk_double\n",
	    plm_image_type_string (this->m_type));
	return;
    }
    this->m_type = PLM_IMG_TYPE_ITK_DOUBLE;
}

void
Plm_image::convert_to_itk_float_field (void)
{
    switch (m_type) {
    case PLM_IMG_TYPE_ITK_FLOAT_FIELD:
	/* do nothing */
	break;
    default:
	print_and_exit (
        "Error: unhandled conversion from %s to itk_float_field\n",
	    plm_image_type_string (this->m_type));
	return;
    }
    this->m_type = PLM_IMG_TYPE_ITK_FLOAT_FIELD;
}

void
Plm_image::convert_to_itk_uchar_vec (void)
{
    switch (m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	lprintf ("Converting from ITK_UCHAR to ITK_UCHAR_VEC\n");
	this->convert_itk_uchar_to_itk_uchar_vec ();
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	lprintf ("Converting from ITK_ULONG to ITK_UCHAR_VEC\n");
	this->convert_itk_uint32_to_itk_uchar_vec ();
	break;
    case PLM_IMG_TYPE_GPUIT_UINT32:
	lprintf ("Converting from GPUIT_UINT32 to ITK_UCHAR_VEC\n");
        this->convert_gpuit_uint32_to_itk_uchar_vec ();
	break;
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR_VEC:
	lprintf ("Converting from GPUIT_UCHAR_VEC to ITK_UCHAR_VEC\n");
        this->convert_gpuit_uchar_vec_to_itk_uchar_vec ();
	break;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to itk_uchar_vec\n",
	    plm_image_type_string (this->m_type));
	return;
    }
    this->m_type = PLM_IMG_TYPE_ITK_UCHAR_VEC;
}

void
Plm_image::convert_to_itk (void)
{
    switch (m_type) {

    case PLM_IMG_TYPE_ITK_CHAR:
    case PLM_IMG_TYPE_ITK_UCHAR:
    case PLM_IMG_TYPE_ITK_SHORT:
    case PLM_IMG_TYPE_ITK_USHORT:
    case PLM_IMG_TYPE_ITK_LONG:
    case PLM_IMG_TYPE_ITK_ULONG:
    case PLM_IMG_TYPE_ITK_FLOAT:
    case PLM_IMG_TYPE_ITK_DOUBLE:
    case PLM_IMG_TYPE_ITK_FLOAT_FIELD:
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
	/* Do nothing */
	break;

    case PLM_IMG_TYPE_GPUIT_UCHAR:
	this->convert_to_itk_uchar ();
	break;
    case PLM_IMG_TYPE_GPUIT_SHORT:
	this->convert_to_itk_short ();
	break;
    case PLM_IMG_TYPE_GPUIT_UINT32:
	this->convert_to_itk_uint32 ();
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->convert_to_itk_float ();
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR_VEC:
        this->convert_to_itk_uchar_vec ();
        break;
    case PLM_IMG_TYPE_GPUIT_FLOAT_FIELD:
    default:
	print_and_exit (
	    "Error: unhandled conversion in Plm_image::convert_to_itk "
	    " with type %s.\n",
	    plm_image_type_string (this->m_type));
	break;
    }
}

void
Plm_image::convert_to_gpuit_short ()
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_GPUIT_SHORT:
	return;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	volume_convert_to_short (this->get_vol());
	return;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_UINT32:
    case PLM_IMG_TYPE_ITK_UCHAR:
    case PLM_IMG_TYPE_ITK_SHORT:
    case PLM_IMG_TYPE_ITK_ULONG:
    case PLM_IMG_TYPE_ITK_FLOAT:
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to gpuit_short\n",
	    plm_image_type_string (this->m_type));
	return;
    }
}

void
Plm_image::convert_to_gpuit_uint16 ()
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_GPUIT_SHORT:
	return;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	volume_convert_to_uint16 (this->get_vol());
	return;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_UINT32:
    case PLM_IMG_TYPE_ITK_UCHAR:
    case PLM_IMG_TYPE_ITK_SHORT:
    case PLM_IMG_TYPE_ITK_ULONG:
    case PLM_IMG_TYPE_ITK_FLOAT:
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to gpuit_uint16\n",
	    plm_image_type_string (this->m_type));
	return;
    }
}

void
Plm_image::convert_to_gpuit_uint32 ()
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_GPUIT_UINT32:
	return;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	volume_convert_to_uint32 (this->get_vol());
	return;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_ITK_UCHAR:
    case PLM_IMG_TYPE_ITK_SHORT:
    case PLM_IMG_TYPE_ITK_ULONG:
    case PLM_IMG_TYPE_ITK_FLOAT:
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to gpuit_uint32\n",
	    plm_image_type_string (this->m_type));
	return;
    }
}

void
Plm_image::convert_to_gpuit_int32 ()
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_GPUIT_INT32:
	return;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	volume_convert_to_int32 (this->get_vol());
	return;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_ITK_UCHAR:
    case PLM_IMG_TYPE_ITK_SHORT:
    case PLM_IMG_TYPE_ITK_ULONG:
    case PLM_IMG_TYPE_ITK_FLOAT:
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to gpuit_int32\n",
	    plm_image_type_string (this->m_type));
	return;
    }
}

void
Plm_image::convert_to_gpuit_float ()
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	this->convert_itk_to_gpuit<UCharImageType::Pointer,float> (
            this->m_itk_uchar);
	/* Free itk data */
	this->m_itk_uchar = 0;
	break;
    case PLM_IMG_TYPE_ITK_CHAR:
	this->convert_itk_to_gpuit<CharImageType::Pointer,float> (
            this->m_itk_char);
	/* Free itk data */
	this->m_itk_char = 0;
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
	this->convert_itk_to_gpuit<UShortImageType::Pointer,float> (
            this->m_itk_ushort);
	/* Free itk data */
	this->m_itk_ushort = 0;
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	this->convert_itk_to_gpuit<ShortImageType::Pointer,float> (
            this->m_itk_short);
	/* Free itk data */
	this->m_itk_short = 0;
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	this->convert_itk_to_gpuit<UInt32ImageType::Pointer,float> (
            this->m_itk_uint32);
	/* Free itk data */
	this->m_itk_uint32 = 0;
	break;
    case PLM_IMG_TYPE_ITK_LONG:
	this->convert_itk_to_gpuit<Int32ImageType::Pointer,float> (
            this->m_itk_int32);
	/* Free itk data */
	this->m_itk_int32 = 0;
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	this->convert_itk_to_gpuit<FloatImageType::Pointer,float> (
            this->m_itk_float);
	/* Free itk data */
	this->m_itk_float = 0;
	break;
    case PLM_IMG_TYPE_ITK_DOUBLE:
	this->convert_itk_to_gpuit<DoubleImageType::Pointer,float> (
            this->m_itk_double);
	/* Free itk data */
	this->m_itk_double = 0;
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_GPUIT_UINT32:
    case PLM_IMG_TYPE_GPUIT_INT32:
	volume_convert_to_float (this->get_vol());
	return;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	return;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s (%d) to gpuit_float\n",
	    plm_image_type_string (this->m_type), this->m_type);
	return;
    }
}

void
Plm_image::convert_to_gpuit_uchar_vec ()
{
    switch (this->m_type) {
    case PLM_IMG_TYPE_GPUIT_UCHAR_VEC:
	return;
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
	this->convert_itk_uchar_vec_to_gpuit_uchar_vec ();
	return;
    default:
	print_and_exit (
	    "Error: unhandled conversion from %s to gpuit_uchar_vec\n",
	    plm_image_type_string (this->m_type));
	return;
    }
}

void
Plm_image::convert_to_original_type (void)
{
    this->convert (this->m_original_type);
}

void
Plm_image::convert (Plm_image_type new_type)
{
    switch (new_type) {
    case PLM_IMG_TYPE_UNDEFINED:
	/* Do nothing */
	return;
    case PLM_IMG_TYPE_ITK_CHAR:
	this->convert_to_itk_char ();
	break;
    case PLM_IMG_TYPE_ITK_UCHAR:
	this->convert_to_itk_uchar ();
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	this->convert_to_itk_short ();
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
	this->convert_to_itk_ushort ();
	break;
    case PLM_IMG_TYPE_ITK_LONG:
	this->convert_to_itk_int32 ();
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	this->convert_to_itk_uint32 ();
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	this->convert_to_itk_float ();
	break;
    case PLM_IMG_TYPE_ITK_DOUBLE:
	this->convert_to_itk_double ();
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
	this->convert_to_gpuit_uchar ();
	break;
    case PLM_IMG_TYPE_GPUIT_SHORT:
	this->convert_to_gpuit_short ();
	break;
    case PLM_IMG_TYPE_GPUIT_UINT16:
	this->convert_to_gpuit_uint16 ();
	break;
    case PLM_IMG_TYPE_GPUIT_UINT32:
	this->convert_to_gpuit_uint32 ();
	break;
    case PLM_IMG_TYPE_GPUIT_INT32:
	this->convert_to_gpuit_int32 ();
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
	this->convert_to_gpuit_float ();
	break;
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
	this->convert_to_itk_uchar_vec ();
	break;
    default:
	print_and_exit (
	    "Unhandled image type in Plm_image::convert (%s -> %s)\n", 
	    plm_image_type_string (this->m_type),
	    plm_image_type_string (new_type));
	break;
    }
    this->m_type = new_type;
}

void
Plm_image::convert_and_save (const char* fname, Plm_image_type new_type)
{
    this->convert (new_type);
    this->save_image (fname);
}

void
Plm_image::convert_and_save (const std::string& fname, Plm_image_type new_type)
{
    this->convert_and_save (fname.c_str(), new_type);
}

/* geometry */
int 
Plm_image::planes ()
{
    switch (m_type) {
    case PLM_IMG_TYPE_UNDEFINED:
	return 0;
    case PLM_IMG_TYPE_ITK_FLOAT_FIELD:
    case PLM_IMG_TYPE_GPUIT_FLOAT_FIELD:
        return 3;
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
        return this->m_itk_uchar_vec->GetVectorLength();
    case PLM_IMG_TYPE_GPUIT_UCHAR_VEC:
        return this->get_vol()->vox_planes;
    default:
        return 1;
    }
}

size_t 
Plm_image::dim (size_t d1)
{
    int d = (int) d1;
    switch (m_type) {
    case PLM_IMG_TYPE_UNDEFINED:
	return 0;
    case PLM_IMG_TYPE_ITK_CHAR:
        return this->m_itk_char->GetLargestPossibleRegion().GetSize()[d];
    case PLM_IMG_TYPE_ITK_UCHAR:
        return this->m_itk_uchar->GetLargestPossibleRegion().GetSize()[d];
    case PLM_IMG_TYPE_ITK_SHORT:
        return this->m_itk_short->GetLargestPossibleRegion().GetSize()[d];
    case PLM_IMG_TYPE_ITK_USHORT:
        return this->m_itk_ushort->GetLargestPossibleRegion().GetSize()[d];
    case PLM_IMG_TYPE_ITK_LONG:
        return this->m_itk_int32->GetLargestPossibleRegion().GetSize()[d];
    case PLM_IMG_TYPE_ITK_ULONG:
        return this->m_itk_uint32->GetLargestPossibleRegion().GetSize()[d];
    case PLM_IMG_TYPE_ITK_FLOAT:
        return this->m_itk_float->GetLargestPossibleRegion().GetSize()[d];
    case PLM_IMG_TYPE_ITK_DOUBLE:
        return this->m_itk_double->GetLargestPossibleRegion().GetSize()[d];
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_GPUIT_UINT16:
    case PLM_IMG_TYPE_GPUIT_UINT32:
    case PLM_IMG_TYPE_GPUIT_INT32:
    case PLM_IMG_TYPE_GPUIT_FLOAT:
    case PLM_IMG_TYPE_GPUIT_FLOAT_FIELD:
    case PLM_IMG_TYPE_GPUIT_UCHAR_VEC:
        return d_ptr->m_vol->dim[d];
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
    default:
	print_and_exit (
	    "Unhandled call to Plm_image::dim (type = %s)\n", 
	    plm_image_type_string (this->m_type));
	break;
    }
    return 0;
}

/* GCS FIX: This is inefficient. */
float 
Plm_image::origin (size_t d1)
{
    int d = (int) d1;
    switch (m_type) {
    case PLM_IMG_TYPE_UNDEFINED:
	return 0;
    case PLM_IMG_TYPE_ITK_CHAR:
        return itk_image_origin(this->m_itk_char)[d];
    case PLM_IMG_TYPE_ITK_UCHAR:
        return itk_image_origin(this->m_itk_uchar)[d];
    case PLM_IMG_TYPE_ITK_SHORT:
        return itk_image_origin(this->m_itk_short)[d];
    case PLM_IMG_TYPE_ITK_USHORT:
        return itk_image_origin(this->m_itk_ushort)[d];
    case PLM_IMG_TYPE_ITK_LONG:
        return itk_image_origin(this->m_itk_int32)[d];
    case PLM_IMG_TYPE_ITK_ULONG:
        return itk_image_origin(this->m_itk_uint32)[d];
    case PLM_IMG_TYPE_ITK_FLOAT:
        return itk_image_origin(this->m_itk_float)[d];
    case PLM_IMG_TYPE_ITK_DOUBLE:
        return itk_image_origin(this->m_itk_double)[d];
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_GPUIT_UINT16:
    case PLM_IMG_TYPE_GPUIT_UINT32:
    case PLM_IMG_TYPE_GPUIT_INT32:
    case PLM_IMG_TYPE_GPUIT_FLOAT:
    case PLM_IMG_TYPE_GPUIT_FLOAT_FIELD:
    case PLM_IMG_TYPE_GPUIT_UCHAR_VEC:
        return d_ptr->m_vol->origin[d];
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
    default:
	print_and_exit (
	    "Unhandled call to Plm_image::origin (type = %s)\n", 
	    plm_image_type_string (this->m_type));
	break;
    }
    return 0.f;
}

float 
Plm_image::spacing (size_t d1)
{
    int d = (int) d1;
    switch (m_type) {
    case PLM_IMG_TYPE_UNDEFINED:
        return 0;
    case PLM_IMG_TYPE_ITK_CHAR:
        return this->m_itk_char->GetSpacing()[d];
    case PLM_IMG_TYPE_ITK_UCHAR:
        return this->m_itk_uchar->GetSpacing()[d];
    case PLM_IMG_TYPE_ITK_SHORT:
        return this->m_itk_short->GetSpacing()[d];
    case PLM_IMG_TYPE_ITK_USHORT:
        return this->m_itk_ushort->GetSpacing()[d];
    case PLM_IMG_TYPE_ITK_LONG:
        return this->m_itk_int32->GetSpacing()[d];
    case PLM_IMG_TYPE_ITK_ULONG:
        return this->m_itk_uint32->GetSpacing()[d];
    case PLM_IMG_TYPE_ITK_FLOAT:
        return this->m_itk_float->GetSpacing()[d];
    case PLM_IMG_TYPE_ITK_DOUBLE:
        return this->m_itk_double->GetSpacing()[d];
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_GPUIT_UINT16:
    case PLM_IMG_TYPE_GPUIT_UINT32:
    case PLM_IMG_TYPE_GPUIT_INT32:
    case PLM_IMG_TYPE_GPUIT_FLOAT:
    case PLM_IMG_TYPE_GPUIT_FLOAT_FIELD:
    case PLM_IMG_TYPE_GPUIT_UCHAR_VEC:
        return d_ptr->m_vol->spacing[d];
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
    default:
	print_and_exit (
	    "Unhandled call to Plm_image::spacing (type = %s)\n", 
	    plm_image_type_string (this->m_type));
	break;
    }
    return 0.f;
}

/* Printing debug information */
void
Plm_image::print ()
{
    lprintf ("Type = %s\n", plm_image_type_string_simple (this->m_type));
    lprintf ("Planes = %d\n", this->planes());
    Plm_image_header pih;
    pih.set_from_plm_image (this);
    pih.print ();
}

/* Return 1 if the two headers are the same */
int
Plm_image::compare_headers (
    const Plm_image::Pointer& pli1, 
    const Plm_image::Pointer& pli2)
{
    Plm_image_header pih1, pih2;

    pih1.set_from_plm_image (pli1);
    pih2.set_from_plm_image (pli2);

    return Plm_image_header::compare (&pih1, &pih2);
}
