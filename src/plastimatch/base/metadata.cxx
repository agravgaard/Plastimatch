/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if PLM_DCM_USE_DCMTK
#include "dcmtk_config.h"
#include "dcmtk/dcmdata/dctagkey.h"
#endif

#include "dicom_util.h"
#include "logfile.h"
#include "make_string.h"
#include "metadata.h"

static std::string KEY_NOT_FOUND = "";

Metadata::Metadata ()
{
}

Metadata::~Metadata ()
{
}

std::string
Metadata::make_key (unsigned short key1, unsigned short key2) const
{
    return make_string (key1, 4, '0', std::hex) 
	+ ',' + make_string (key2, 4, '0', std::hex);
}

const std::string&
Metadata::get_metadata (const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it;
    it = m_data.find (key);
    if (it == m_data.end()) {
	/* key not found in map -- check parent */
	if (m_parent) {
	    return m_parent->get_metadata (key);
	}
	/* key not found */
	return KEY_NOT_FOUND;
    } else {
	/* key found in map */
	return it->second;
    }
}

const std::string&
Metadata::get_metadata (unsigned short key1, unsigned short key2) const
{
#if defined (commentout)
    std::cout << "Getting metadata: " << make_key (key1, key2) 
        << " is " << get_metadata (make_key (key1, key2)) << std::endl;
#endif
    return get_metadata (make_key (key1, key2));
}

void
Metadata::set_metadata (const std::string& key, const std::string& val)
{
#if defined (commentout)
    std::cout << "Setting metadata: " << key << " to " << val << std::endl;
#endif
    m_data[key] = val;
}

void
Metadata::set_metadata (unsigned short key1, unsigned short key2, 
    const std::string& val)
{
    set_metadata (make_key (key1, key2), val);
}

#if PLM_DCM_USE_DCMTK
const std::string&
Metadata::get_metadata (const DcmTagKey& key) const
{
    return get_metadata (key.getGroup(), key.getElement());
}

void
Metadata::set_metadata (const DcmTagKey& key, const std::string& val)
{
    return set_metadata (key.getGroup(), key.getElement(), val);
}

#endif

void
Metadata::set_metadata (const std::vector<std::string>& metadata)
{
    std::vector<std::string>::const_iterator it = metadata.begin();
    while (it < metadata.end()) {
        const std::string& str = (*it);
        size_t eq_pos = str.find_first_of ('=');
        if (eq_pos != std::string::npos) {
            std::string key = str.substr (0, eq_pos);
            std::string val = str.substr (eq_pos+1);
            /* Set newer-style metadata, used by dcmtk */
            this->set_metadata (key, val);
        }
        ++it;
    }
}

void
Metadata::remove_metadata (unsigned short key1, unsigned short key2)
{
    m_data.erase (make_key (key1, key2));
}

void
Metadata::create_anonymous ()
{
    /* PatientsName */
    this->set_metadata (0x0010, 0x0010, "ANONYMOUS");
    /* PatientID */
    this->set_metadata (0x0010, 0x0020, dicom_anon_patient_id());
    /* PatientSex */
    this->set_metadata (0x0010, 0x0040, "O");
    /* PatientPosition */
    this->set_metadata (0x0018, 0x5100, "HFS");
}

void
Metadata::print_metadata () const
{
    std::map<std::string, std::string>::const_iterator it;
    lprintf ("Metadata\n");
    for (it = m_data.begin(); it != m_data.end(); it++) {
        lprintf ("%s | %s\n", it->first.c_str(), it->second.c_str());
    }
}
