/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _metadata_h_
#define _metadata_h_

#include "plmbase_config.h"
#include <map>
#include <string>
#include <vector>
#if PLM_DCM_USE_DCMTK
#include "dcmtk_config.h"
#include "dcmtk/dcmdata/dctagkey.h"
#endif

#include "smart_pointer.h"

/*! \brief 
 * The Metadata class encapsulate DICOM metadata for a single series.
 * It is implemented as a map from string to string, where the 
 * key string is of the form "XXXX,XXXX".
 */
class PLMBASE_API Metadata
{
public:
    SMART_POINTER_SUPPORT (Metadata);
    Metadata ();
    ~Metadata ();

public:
    /* GCS: Note use of unsigned short instead of uint16_t, because of 
       broken stdint implementation in gdcm 1.X. */
    std::string
    make_key (unsigned short key1, unsigned short key2) const;
    const std::string&
    get_metadata (const std::string& key) const;
    const std::string&
    get_metadata (unsigned short key1, unsigned short key2) const;
    void
    set_metadata (const std::string& key, const std::string& val);
    void set_metadata (unsigned short key1, unsigned short key2,
        const std::string& val);
#if PLM_DCM_USE_DCMTK
    const std::string& get_metadata (const DcmTagKey& key) const;
    void set_metadata (const DcmTagKey& key, const std::string& val);
#endif

    /*! \brief Copy a list of strings of the form "XXXX,YYYY=string"
      into the metadata map. */
    void set_metadata (const std::vector<std::string>& metadata);

    /*! \brief Attach a parent to this metadata set.  The parent metadata
      is used when there is no match in the child metadata. */
    void set_parent (const Metadata::Pointer& parent) {
        m_parent = parent;
    }

    /*! \brief Remove a key from the metadata, if the key exists. */
    void remove_metadata (unsigned short key1, unsigned short key2);

    void create_anonymous ();
    void print_metadata () const;

public:
    Metadata::Pointer m_parent;
    std::map<std::string, std::string> m_data;

public:
    
};

#endif
