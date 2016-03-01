/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _metadata_h_
#define _metadata_h_

#include "plmbase_config.h"
#include <map>
#include <string>
#include "smart_pointer.h"

class PLMBASE_API Metadata
{
public:
    SMART_POINTER_SUPPORT (Metadata);
    Metadata ();
    ~Metadata ();

public:
    /* GCS: Note use of unsigned short instead of uint16_t, because of 
       broken stdint implementation in gdcm. */
    std::string
    make_key (unsigned short key1, unsigned short key2) const;
    const char*
    get_metadata_ (const std::string& key) const;
    const char*
    get_metadata_ (unsigned short key1, unsigned short key2) const;
    const std::string&
    get_metadata (const std::string& key) const;
    const std::string&
    get_metadata (unsigned short key1, unsigned short key2) const;
    void
    set_metadata (const std::string& key, const std::string& val);
    void
    set_metadata (unsigned short key1, unsigned short key2,
        const std::string& val);
    void set_parent (const Metadata::Pointer& parent) {
        m_parent = parent;
    }
    void create_anonymous ();
    void print_metadata () const;

public:
    Metadata::Pointer m_parent;
    std::map<std::string, std::string> m_data;

public:
    
};

#endif
