/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <stdio.h>
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_file.h"
#include "dcmtk_metadata.h"
#include "dcmtk_series.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_image_set.h"
#include "print_and_exit.h"
#include "rt_study_metadata.h"
#include "volume.h"

class Dcmtk_series_private {
public:
    Dcmtk_file_list m_flist;
    Rt_study_metadata::Pointer m_drs;

public:
    Dcmtk_series_private () {
    }
    ~Dcmtk_series_private () {
    }
};

Dcmtk_series::Dcmtk_series ()
{
    d_ptr = new Dcmtk_series_private;
}

Dcmtk_series::~Dcmtk_series ()
{
    delete d_ptr;
}

const std::list<Dcmtk_file::Pointer>&
Dcmtk_series::get_flist () const
{
    return d_ptr->m_flist;
}

const char*
Dcmtk_series::get_cstr (const DcmTagKey& tag_key) const
{
    return d_ptr->m_flist.front()->get_cstr(tag_key);
}

bool
Dcmtk_series::get_int16_array (const DcmTagKey& tag_key, 
    const int16_t** val, unsigned long* count) const
{
    return d_ptr->m_flist.front()->get_int16_array (tag_key, val, count);
}

bool
Dcmtk_series::get_sequence (const DcmTagKey& tag_key,
    DcmSequenceOfItems*& seq) const
{
    return d_ptr->m_flist.front()->get_sequence (tag_key, seq);
}

std::string 
Dcmtk_series::get_string (const DcmTagKey& tag_key) const
{
    const char* c = d_ptr->m_flist.front()->get_cstr(tag_key);
    if (!c) c = "";
    return std::string(c);
}

bool
Dcmtk_series::get_uint16 (const DcmTagKey& tag_key, uint16_t* val) const
{
    return d_ptr->m_flist.front()->get_uint16 (tag_key, val);
}

bool
Dcmtk_series::get_uint16_array (const DcmTagKey& tag_key, 
    const uint16_t** val, unsigned long* count) const
{
    return d_ptr->m_flist.front()->get_uint16_array (tag_key, val, count);
}

std::string 
Dcmtk_series::get_modality (void) const
{
    return get_string (DCM_Modality);
}

std::string 
Dcmtk_series::get_referenced_uid (void) const
{
    bool rc;
    if (this->get_modality() != "RTSTRUCT") {
	return "";
    }

    DcmItem* rfors = 0;
    rc = d_ptr->m_flist.front()->get_dataset()->findAndGetSequenceItem (
	DCM_ReferencedFrameOfReferenceSequence, rfors).good();
    if (!rc) {
	return "";
    }
    lprintf ("Found DCM_ReferencedFrameOfReferenceSequence!\n");

    DcmItem* rss = 0;
    rc = rfors->findAndGetSequenceItem (
	DCM_RTReferencedStudySequence, rss).good();
    if (!rc) {
	return "";
    }
    lprintf ("Found DCM_RTReferencedStudySequence!\n");

    return "";
}

size_t
Dcmtk_series::get_number_of_files (void) const
{
    return d_ptr->m_flist.size();
}

void
Dcmtk_series::insert (Dcmtk_file::Pointer& df)
{
    d_ptr->m_flist.push_back (df);
}

void
Dcmtk_series::sort (void)
{
    d_ptr->m_flist.sort (dcmtk_file_compare_z_position);
}

void
Dcmtk_series::set_rt_study_metadata (Rt_study_metadata::Pointer& drs)
{
    d_ptr->m_drs = drs;
}

void
Dcmtk_series::debug (void) const
{
    std::list<Dcmtk_file::Pointer>::const_iterator it;
    for (it = d_ptr->m_flist.begin(); it != d_ptr->m_flist.end(); ++it) {
        const Dcmtk_file::Pointer& df = (*it);
	df->debug ();
    }
}
