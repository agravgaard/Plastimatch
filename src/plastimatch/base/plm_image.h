/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_image_h_
#define _plm_image_h_

#include "plmbase_config.h"
#include "compiler_warnings.h"
#include "itk_image.h"
#include "metadata.h"
#include "plm_image_type.h"
#include "smart_pointer.h"
#include "volume.h"

class Plm_image_header;
class Plm_image;
class Plm_image_private;
class Rt_study_metadata;
class Slice_index;

/*! \brief 
 * The Plm_image class represents a three-dimensional volume.  
 * The volume is an abstraction that can contain a volume in either 
 * native format (Volume), or ITK format (itk::Image), in any 
 * type (unsigned char, float, etc.), or in several commonly used 
 * extensions ()
 */
class PLMBASE_API Plm_image {
public:
    SMART_POINTER_SUPPORT (Plm_image);
    Plm_image_private *d_ptr;
public:
    Plm_image ();
    Plm_image (const char* fname);
    Plm_image (const std::string& fname);
    Plm_image (const char* fname, Plm_image_type type);
    Plm_image (const std::string& fname, Plm_image_type type);
    Plm_image (UCharImageType::Pointer img);
    Plm_image (CharImageType::Pointer img);
    Plm_image (ShortImageType::Pointer img);
    Plm_image (FloatImageType::Pointer img);
    Plm_image (const Volume::Pointer& vol);
    Plm_image (Volume* vol);
    Plm_image (Plm_image_type type, const Plm_image_header& pih);
    ~Plm_image ();

public:
    Plm_image_type m_original_type;
    Plm_image_type m_type;

    /* The actual image is one of the following. */
    UCharImageType::Pointer m_itk_uchar;
    CharImageType::Pointer m_itk_char;
    UShortImageType::Pointer m_itk_ushort;
    ShortImageType::Pointer m_itk_short;
    UInt32ImageType::Pointer m_itk_uint32;
    Int32ImageType::Pointer m_itk_int32;
    FloatImageType::Pointer m_itk_float;
    DoubleImageType::Pointer m_itk_double;
    UCharVecImageType::Pointer m_itk_uchar_vec;

private:
    /* Please don't use copy constructors.  They suck. */
    Plm_image (Plm_image& xf) {
        UNUSED_VARIABLE (xf);
    }
    /* Please don't use overloaded operators.  They suck. */
    Plm_image& operator= (Plm_image& xf) {
        UNUSED_VARIABLE (xf);
        return *this;
    }
    
    void convert_to_itk_char ();
    void convert_to_itk_uchar ();
    void convert_to_itk_short ();
    void convert_to_itk_ushort ();
    void convert_to_itk_int32 (void);
    void convert_to_itk_uint32 ();
    void convert_to_itk_float ();
    void convert_to_itk_double ();
    void convert_to_gpuit_short ();
    void convert_to_gpuit_uint16 ();
    void convert_to_gpuit_uint32 ();
    void convert_to_gpuit_int32 ();
    void convert_to_gpuit_float ();
    void convert_to_gpuit_uchar ();
    void convert_to_gpuit_uchar_vec ();

public:
    /* creation / destruction */
    void init ();
    void free ();
    bool have_image ();
    Plm_image* clone (void);
    void create (Plm_image_type type, const Plm_image_header& pih);

    /* Loading */
    bool load (const char* fname, Plm_image_type type);
    bool load_native (const char* fname);
    bool load_native (const std::string& fn);
    bool load_native_dicom (const char* fname);
    bool load_native_nki (const char* fname);

    /* Saving */
    void save_short_dicom (const char* fname, Rt_study_metadata *rsm);	
    void save_short_dicom (const std::string& fname, Rt_study_metadata *rsm);	
    void save_image (const char* fname);
    void save_image (const std::string& fname);
    void convert_and_save (const char* fname, Plm_image_type new_type);
    void convert_and_save (const std::string& fname, Plm_image_type new_type);

    /* assignment */
    void set (const Plm_image::Pointer& pli);
    void set_volume (const Volume::Pointer& v, Plm_image_type type);
    void set_volume (const Volume::Pointer& v);
    void set_volume (Volume *v, Plm_image_type type);
    void set_volume (Volume *v);
    void set_itk (UCharImageType::Pointer img);
    void set_itk (CharImageType::Pointer img);
    void set_itk (UShortImageType::Pointer img);
    void set_itk (ShortImageType::Pointer img);
    void set_itk (UInt32ImageType::Pointer img);
    void set_itk (Int32ImageType::Pointer img);
    void set_itk (FloatImageType::Pointer img);
    void set_itk (DoubleImageType::Pointer img);
    void set_itk (UCharVecImageType::Pointer img);

    /* conversion */
    FloatImageType::Pointer& itk_float () {
        convert_to_itk_float ();
        return m_itk_float;
    }
    ShortImageType::Pointer& itk_short () {
        convert_to_itk_short ();
        return m_itk_short;
    }
    UCharImageType::Pointer& itk_uchar () {
        convert_to_itk_uchar ();
        return m_itk_uchar;
    }
    UCharVecImageType::Pointer& itk_uchar_vec () {
        convert_to_itk_uchar_vec ();
        return m_itk_uchar_vec;
    }

    Volume::Pointer& get_volume ();
    Volume::Pointer& get_volume_uchar ();
    Volume::Pointer& get_volume_short ();
    Volume::Pointer& get_volume_float ();
    Volume::Pointer& get_volume_uchar_vec ();

    Volume* get_vol ();
    const Volume* get_vol () const;

    void convert (Plm_image_type new_type);
    void convert_to_original_type (void);
    void convert_to_itk (void);
    void convert_to_itk_float_field (void);
    void convert_to_itk_uchar_vec (void);

    /* geometry */
    int planes ();
    size_t dim (size_t);
    float origin (size_t);
    float spacing (size_t);

    /* debug */
    void print ();

    /* Static functions */
    static int compare_headers (
        const Plm_image::Pointer& pli1, 
        const Plm_image::Pointer& pli2);
    static Plm_image::Pointer clone (const Plm_image::Pointer& pli);

protected:
    /* Utility functions */
    void free_volume ();

    /* Specific converters: implemented in plm_image_convert.cxx */
    void convert_itk_uchar_to_itk_uchar_vec ();
    void convert_itk_uint32_to_itk_uchar_vec ();
    void convert_gpuit_uint32_to_itk_uchar_vec ();
    void convert_gpuit_uchar_vec_to_itk_uchar_vec ();
    void convert_itk_uchar_vec_to_gpuit_uchar_vec ();

    /* Generic converters: implemented in plm_image_convert.cxx */
    template<class T, class U> T convert_gpuit_to_itk (Volume *vol);
    template<class T, class U> void convert_itk_to_gpuit (T img);
};

/* -----------------------------------------------------------------------
   Public functions
   ----------------------------------------------------------------------- */
PLMBASE_API Plm_image::Pointer plm_image_load (
    const char* fname, Plm_image_type type);
PLMBASE_API Plm_image::Pointer plm_image_load (
    const std::string& fname, Plm_image_type type);
PLMBASE_API Plm_image::Pointer plm_image_load_native (
    const char* fname);
PLMBASE_API Plm_image::Pointer plm_image_load_native (
    const std::string& fname);

#endif
