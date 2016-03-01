/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "file_util.h"
#include "logfile.h"
#include "parameter_parser.h"
#include "print_and_exit.h"
#include "string_util.h"

Parameter_parser::Parameter_parser () {
    key_regularization = true;
}

Plm_return_code
Parameter_parser::parse_config_string (
    const char* config_string
)
{
    std::stringstream ss (config_string);
    std::string buf;
    std::string buf_ori;    /* An extra copy for diagnostics */
    std::string section = "GLOBAL";
    bool found_first_section = false;

    while (getline (ss, buf)) {
        buf_ori = buf;
        buf = string_trim (buf);
        buf_ori = string_trim (buf_ori, "\r\n");

        if (buf == "") continue;
        if (buf[0] == '#') continue;

        /* Process "[SECTION]" */
        if (buf[0] == '[') {
            if (buf[buf.length()-1] != ']') {
                lprintf ("Parse error: %s\n", buf_ori.c_str());
                return PLM_ERROR;
            }

            /* Strip off brackets and make upper case */
            buf = buf.substr (1, buf.length()-2);
            std::string new_section = make_uppercase (buf);

            /* Inform subclass that previous section is ending */
            if (found_first_section) {
                this->end_section (section);
            }
            found_first_section = true;

            /* Inform subclass that a new section is beginning */
            section = new_section;
            Plm_return_code rc = this->begin_section (section);
            if (rc != PLM_SUCCESS) {
                lprintf ("Parse error: %s\n", buf_ori.c_str());
                return rc;
            }
            continue;
        }

        /* Process "key=value" */
        std::string key;
        std::string val;
        size_t key_loc = buf.find ("=");
        if (key_loc == std::string::npos) {
            key = buf;
            val = "";
        } else {
            key = buf.substr (0, key_loc);
            val = buf.substr (key_loc+1);
        }
        key = string_trim (key);
        val = string_trim (val);

        /* Key becomes lowercase, with "_" & "-" unified */
        if (this->key_regularization) {
            key = regularize_string (key);
        }

        /* Handle case of key/value pairs before section */
        if (!found_first_section) {
            this->begin_section (section);
        }
        found_first_section = true;

        /* No key?  What to do? */
        if (key == "") {
            continue;
        }

        Plm_return_code rc = this->set_key_value (section, key, val);
        if (rc != PLM_SUCCESS) {
            lprintf ("Parse error: %s\n", buf_ori.c_str());
            return rc;
        }
    }

    /* Don't forget to end the last section */
    this->end_section (section);

    return PLM_SUCCESS;
}

void 
Parameter_parser::enable_key_regularization (
    bool enable
)
{
    this->key_regularization = enable;
}

Plm_return_code
Parameter_parser::parse_config_string (
    const std::string& config_string
)
{
    return this->parse_config_string (config_string.c_str());
}

Plm_return_code
Parameter_parser::parse_config_file (
    const char* config_fn
)
{
    /* Confirm file can be read */
    if (!file_exists (config_fn)) {
        print_and_exit ("Error reading config file: %s\n", config_fn);
    }

    /* Read file into string */
    std::ifstream t (config_fn);
    std::stringstream buffer;
    buffer << t.rdbuf();

    return this->parse_config_string (buffer.str());
}

Plm_return_code
Parameter_parser::parse_config_file (
    const std::string& config_fn
)
{
    return this->parse_config_file (config_fn.c_str());
}
