/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dcmtk_series_h_
#define _dcmtk_series_h_

#include "plmbase_config.h"
#include <list>

#include "dcmtk_file.h"
#include "rt_study_metadata.h"
#include "plm_image.h"
#include "plm_int.h"

class DcmTagKey;

class Dcmtk_file;
class Dcmtk_series_private;
class Rt_study_metadata;
class Plm_image;

typedef std::list<Dcmtk_file::Pointer> Dcmtk_file_list;

class Dcmtk_series 
{
public:
    Dcmtk_series ();
    ~Dcmtk_series ();

public:
    Dcmtk_series_private *d_ptr;

public:
    const std::list<Dcmtk_file::Pointer>& get_flist () const;
    const char* get_cstr (const DcmTagKey& tag_key) const;
    bool get_int16_array (const DcmTagKey& tag_key, 
        const int16_t** val, unsigned long* count) const;
    bool get_sequence (const DcmTagKey& tag_key, 
        DcmSequenceOfItems*& seq) const;
    std::string get_string (const DcmTagKey& tag_key) const;
    bool get_uint16 (const DcmTagKey& tag_key, uint16_t* val) const;
    bool get_uint16_array (const DcmTagKey& tag_key, 
        const uint16_t** val, unsigned long* count) const;

    std::string get_modality (void) const;
    std::string get_referenced_uid (void) const;

    size_t get_number_of_files () const;

    void insert (Dcmtk_file::Pointer& df);
    void sort (void);

    void set_rt_study_metadata (Rt_study_metadata::Pointer& drs);

    void debug (void) const;
};

#endif
