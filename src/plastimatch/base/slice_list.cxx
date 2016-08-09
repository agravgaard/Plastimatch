/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <list>
#include <deque>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "dicom_util.h"
#include "logfile.h"
#include "plm_image_header.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "slice_list.h"

class Slice_data {
public:
    Slice_data () : z(0.f), uid("") { }
public:
    float z;
    std::string uid;
};

class Slice_group {
public:
    Plm_image_header group_pih;
    /* Slices are sorted in order, starting with origin slice */
    std::list<Slice_data> slice_data;
};

class Slice_list_private {
public:
    bool m_have_pih;
    bool m_have_slice_uids;
    Plm_image_header m_pih;

    // Sorted slices in "ascending z order", starting with origin slice
    std::vector<Slice_data> sorted_slices;
    // Slice groups are also sorted in the same order
    std::list<Slice_group> slice_groups;

public:
    Slice_list_private () {
        this->m_have_pih = false;
        this->m_have_slice_uids = false;
    }
};

Slice_list::Slice_list ()
{
    this->d_ptr = new Slice_list_private;
}

Slice_list::~Slice_list ()
{
    delete this->d_ptr;
}

const Plm_image_header*
Slice_list::get_image_header (void) const
{
    return &d_ptr->m_pih;
}

void
Slice_list::set_image_header (const Plm_image_header& pih)
{
    d_ptr->m_pih = pih;
    d_ptr->sorted_slices.resize (pih.dim(2));
    d_ptr->m_have_pih = true;
}

void
Slice_list::set_image_header (ShortImageType::Pointer img)
{
    Plm_image_header pih (img);
    this->set_image_header (pih);
}

const char*
Slice_list::get_slice_uid (int index) const
{
    if (!d_ptr->m_have_slice_uids) {
	return "";
    }
    if (index < 0 || ((size_t) index) >= d_ptr->sorted_slices.size()) {
	return "";
    }
    return d_ptr->sorted_slices[index].uid.c_str();
}

void
Slice_list::reset_slice_uids ()
{
    d_ptr->sorted_slices.clear();
    if (d_ptr->m_have_pih) {
        d_ptr->sorted_slices.resize (d_ptr->m_pih.dim(2));
    }
}

void
Slice_list::set_slice_uid (int index, const char* slice_uid)
{
    if (index >= (int) d_ptr->sorted_slices.size()) {
        print_and_exit (
            "Illegal call to Slice_list::set_slice_uid.  "
            "Index %d > Size %d.\n", 
            index, d_ptr->sorted_slices.size());
    }
    d_ptr->sorted_slices[index].uid = std::string (slice_uid);
}

bool
Slice_list::slice_list_complete () const
{
    /* This is equivalent to the old "m_loaded" flag */
    return d_ptr->m_have_pih && d_ptr->m_have_slice_uids;
}

void
Slice_list::set_slice_list_complete ()
{
    d_ptr->m_have_slice_uids = true;
}

int 
Slice_list::num_slices ()
{
    if (!d_ptr->m_have_pih) {
	return 0;
    }

    return d_ptr->m_pih.dim (2);
}

int
Slice_list::get_slice_index (float z) const
{
    if (!this->slice_list_complete()) {
	return -1;
    }

    /* NOTE: This algorithm doesn't work if there are duplicate slices */
    int slice_no = ROUND_INT ((z - d_ptr->m_pih.origin(2)) 
	/ d_ptr->m_pih.spacing(2));
    if (slice_no < 0 || slice_no >= d_ptr->m_pih.dim(2)) {
	return -1;
    }
    return slice_no;
}
