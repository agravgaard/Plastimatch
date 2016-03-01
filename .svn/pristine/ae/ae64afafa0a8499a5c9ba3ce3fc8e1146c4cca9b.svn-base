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
#include "logfile.h"
#include "print_and_exit.h"
#include "string_util.h"

class Dcmtk_file_private {
public:
    std::string m_fn;
    DcmFileFormat *m_dfile;
    Volume_header m_vh;
    float m_zpos;
    bool m_valid;
    
public:
    Dcmtk_file_private () {
        m_dfile = new DcmFileFormat;
        m_fn = "";
        m_zpos = 0.f;
        m_valid = false;
    }
    ~Dcmtk_file_private () {
        delete m_dfile;
    }
};

Dcmtk_file::Dcmtk_file () {
    d_ptr = new Dcmtk_file_private;
}

Dcmtk_file::Dcmtk_file (const char *fn) {
    d_ptr = new Dcmtk_file_private;
    this->load_header (fn);
}

Dcmtk_file::~Dcmtk_file () {
    delete d_ptr;
}

bool
Dcmtk_file::is_valid () const
{
    return d_ptr->m_valid;
}

void
Dcmtk_file::debug () const
{
    printf (" %s\n", d_ptr->m_fn.c_str());
    d_ptr->m_vh.print ();
}

DcmDataset*
Dcmtk_file::get_dataset (void) const
{
    return d_ptr->m_dfile->getDataset();
}

/* Look up DICOM value from tag */
const char*
Dcmtk_file::get_cstr (const DcmTagKey& tag_key) const
{
    const char *c = 0;
    DcmDataset *dset = d_ptr->m_dfile->getDataset();
    if (dset->findAndGetString(tag_key, c).good() && c) {
	return c;
    }
    return 0;
}

bool
Dcmtk_file::get_uint8 (const DcmTagKey& tag_key, uint8_t* val) const
{
    return d_ptr->m_dfile->getDataset()->findAndGetUint8 (
        tag_key, (*val)).good();
}

bool
Dcmtk_file::get_uint16 (const DcmTagKey& tag_key, uint16_t* val) const
{
    return d_ptr->m_dfile->getDataset()->findAndGetUint16 (
        tag_key, (*val)).good();
}

bool
Dcmtk_file::get_float (const DcmTagKey& tag_key, float* val) const
{
    return d_ptr->m_dfile->getDataset()->findAndGetFloat32 (
        tag_key, (*val)).good();
}

bool
Dcmtk_file::get_ds_float (const DcmTagKey& tag_key, float* val) const
{
    const char *c = this->get_cstr (tag_key);
    if (!c) return false;

    int rc = sscanf (c, "%f", val);
    if (rc != 1) return false;

    return true;
}

bool
Dcmtk_file::get_uint8_array (const DcmTagKey& tag_key, 
    const uint8_t** val, unsigned long* count) const
{
    const Uint8* foo;
    OFCondition rc = d_ptr->m_dfile->getDataset()->findAndGetUint8Array (
	tag_key, foo, count, OFFalse);
    if (val) {
        *val = foo;
    }
    return rc.good();
}

bool
Dcmtk_file::get_int16_array (const DcmTagKey& tag_key, 
    const int16_t** val, unsigned long* count) const
{
    const Sint16* foo;
    OFCondition rc = d_ptr->m_dfile->getDataset()->findAndGetSint16Array (
	tag_key, foo, count, OFFalse);
    *val = foo;
    return rc.good();
}

bool
Dcmtk_file::get_uint16_array (const DcmTagKey& tag_key, 
    const uint16_t** val, unsigned long* count) const
{
    const Uint16* foo;
    OFCondition rc = d_ptr->m_dfile->getDataset()->findAndGetUint16Array (
	tag_key, foo, count, OFFalse);
    if (val) {
        *val = foo;
    }
    return rc.good();
}

bool
Dcmtk_file::get_element (const DcmTagKey& tag_key, DcmElement* val) const
{
    return d_ptr->m_dfile->getDataset()->findAndGetElement(tag_key, val).good();
}

bool
Dcmtk_file::get_sequence (const DcmTagKey& tag_key, 
    DcmSequenceOfItems*& seq) const
{
    return d_ptr->m_dfile->getDataset()->findAndGetSequence (
        tag_key, seq).good();
}

const Volume_header*
Dcmtk_file::get_volume_header () const
{
    return &d_ptr->m_vh;
}

const Direction_cosines&
Dcmtk_file::get_direction_cosines () const
{
    return d_ptr->m_vh.get_direction_cosines();
}

float
Dcmtk_file::get_z_position () const
{
#if defined (commentout)
    return d_ptr->m_vh.get_origin()[2];
#endif
    return d_ptr->m_zpos;
}

void
Dcmtk_file::load_header (const char *fn) {
    /* Save a copy of the filename */
    d_ptr->m_fn = fn;

    /* Open the file */
    OFCondition cond = d_ptr->m_dfile->loadFile (fn, EXS_Unknown, EGL_noChange);
    if (cond.bad()) {
        /* If it's not a dicom file, loadFile() fails. */
        return;
    }

    /* Load image header */
    DcmDataset *dset = d_ptr->m_dfile->getDataset();    
    OFCondition ofrc;
    const char *c;
    uint16_t rows;
    uint16_t cols;

    /* ImagePositionPatient */
    float origin[3];
    ofrc = dset->findAndGetString (DCM_ImagePositionPatient, c);
    if (ofrc.good() && c) {
	int rc = parse_dicom_float3 (origin, c);
	if (!rc) {
	    d_ptr->m_vh.set_origin (origin);
	}
    }

    /* Rows, Columns */
    ofrc = dset->findAndGetUint16 (DCM_Rows, rows);
    if (ofrc.good()) {
	ofrc = dset->findAndGetUint16 (DCM_Columns, cols);
	if (ofrc.good()) {
	    plm_long dim[3];
	    dim[0] = cols;
	    dim[1] = rows;
	    dim[2] = 1;
	    d_ptr->m_vh.set_dim (dim);
	}
    }

    /* ImageOrientationPatient */
    float direction_cosines[9];
    ofrc = dset->findAndGetString (DCM_ImageOrientationPatient, c);
    if (ofrc.good() && c) {
	int rc = parse_dicom_float6 (direction_cosines, c);
	if (!rc) {
	    direction_cosines[6] 
		= direction_cosines[1]*direction_cosines[5] 
		- direction_cosines[2]*direction_cosines[4];
	    direction_cosines[7] 
		= direction_cosines[2]*direction_cosines[3] 
		- direction_cosines[0]*direction_cosines[5];
	    direction_cosines[8] 
		= direction_cosines[0]*direction_cosines[4] 
		- direction_cosines[1]*direction_cosines[3];
	    d_ptr->m_vh.set_direction_cosines (direction_cosines);
	}
    }

    /* PixelSpacing */
    ofrc = dset->findAndGetString (DCM_PixelSpacing, c);
    if (ofrc.good() && c) {
	float spacing[3];
	int rc = parse_dicom_float2 (spacing, c);
	if (!rc) {
	    spacing[2] = 0.0;
	    d_ptr->m_vh.set_spacing (spacing);
	}
    }

    /* Compute z position */
    d_ptr->m_zpos =
        + direction_cosines[6] * origin[0]
        + direction_cosines[7] * origin[1]
        + direction_cosines[8] * origin[2];

    d_ptr->m_valid = true;
}

bool
dcmtk_file_compare_z_position (const Dcmtk_file* f1, const Dcmtk_file* f2)
{
    return f1->get_z_position () < f2->get_z_position ();
}

bool
dcmtk_file_compare_z_position (const Dcmtk_file::Pointer& f1, 
    const Dcmtk_file::Pointer& f2)
{
    return f1->get_z_position () < f2->get_z_position ();
}

void
dcmtk_file_test (const char *fn)
{
    Dcmtk_file df(fn);
}
