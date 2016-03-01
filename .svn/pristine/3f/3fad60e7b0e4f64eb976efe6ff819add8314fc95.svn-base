/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _parameter_parser_h_
#define _parameter_parser_h_

#include "plmbase_config.h"
#include "plm_return_code.h"

class PLMBASE_API Parameter_parser {
public:
    Parameter_parser ();
public:
    bool key_regularization;
public:
    /* Callbacks */
    virtual Plm_return_code begin_section (
        const std::string& section) = 0;
    virtual Plm_return_code end_section (
        const std::string& section) = 0;
    virtual Plm_return_code set_key_value (
        const std::string& section, 
        const std::string& key, 
        const std::string& val) = 0;

    /* Pass in "true" to enable key regularization, or "false" to 
       disable it.   Default is "true". */
    void enable_key_regularization (
        bool enable
    );

    /* Return zero if config string is correctly parsed */
    Plm_return_code parse_config_string (
        const char* config_string
    );
    Plm_return_code parse_config_string (
        const std::string& config_string
    );
    Plm_return_code parse_config_file (
        const char* config_fn
    );
    Plm_return_code parse_config_file (
        const std::string& config_fn
    );
};

#endif
