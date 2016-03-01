/* -----------------------------------------------------------------------
   This file is derived from dlib source code, http://dlib.net, and 
   is licensed under the "Boost Software License - Version 1.0".
   Modifications by Greg Sharp <gregsharp@geocities.com>
   ----------------------------------------------------------------------- */
#ifndef _plm_dlib_clp_h_
#define _plm_dlib_clp_h_

#include "plmcli_config.h"
#include <iostream>
#include <map>
#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include "dlib/cmd_line_parser.h"
#include "sys/plm_int.h"

#include "plm_version.h"

typedef dlib::cmd_line_parser<char>::check_1a_c Clp;

namespace dlib {
class Plm_clp
    : public cmd_line_parser<char>::check_1a_c 
{
    public:
    std::map<string_type,string_type> default_value_map;
    std::map<string_type,string_type> option_map;
    std::map<string_type,string_type> description_map;
    std::map<string_type,string_type> short_to_long_map;
    std::map<string_type,string_type> long_to_short_map;
    void (*usage_fn) (dlib::Plm_clp*,int,char*[]);
    int argc;
    char **argv;
    unsigned long wrap_len;
    public:
    
    void add_default_options (void);
    void check_default_options (void);
    void add_long_option (
        const string_type& short_name,
        const string_type& long_name,
        const string_type& description,
        unsigned long number_of_arguments = 0,
        const string_type& default_value = ""
    );

    const string_type&
    long_option_name (const string_type& name)
    {
        std::map<string_type,string_type>::iterator it;
        it = this->short_to_long_map.find (name);
        if (it != this->short_to_long_map.end()) {
            return it->second;
        } else {
            return name;
        }
    }

    const string_type&
    short_option_name (const string_type& name)
    {
        std::map<string_type,string_type>::iterator it;
        it = this->long_to_short_map.find (name);
        if (it != this->long_to_short_map.end()) {
            return it->second;
        } else {
            return name;
        }
    }

    void 
    add_option_with_default (
        const string_type& name,
        const string_type& description,
        unsigned long number_of_arguments = 0,
        const string_type& default_value = ""
    ) {
        this->add_option (name, description, number_of_arguments);
        if (default_value != "") {
            default_value_map.insert (
                std::pair<string_type,string_type> (name, default_value));
        }
    }

    bool
    have_option (const string_type& name)
    {
        if (option (long_option_name (name))) {
            return true;
        }
        if (option (short_option_name (name))) {
            return true;
        }
        return false;
    }

    /* Get command line arg specified with either long or short option */
    string_type
    get_value_base (const string_type& name)
    {
        /* Option specified as long name */
        const string_type& long_name = long_option_name (name);
        if (option (long_name)) {
            return this->option(long_name).argument();
        }

        /* Option specified as short name */
        const string_type& short_name = short_option_name (name);
        if (option (short_name)) {
            return this->option(short_name).argument();
        }

        /* Not specified on command line */
        return "";
    }

    /* Get string value from either long or short option, filling in 
       default value if no option specified */
    string_type
    get_value (
        const string_type& name
    ) {
        /* Option specified on command line */
        if (this->have_option(name)) {
            return this->get_value_base (name);
        }

        /* Default value */
        const string_type& long_name = long_option_name (name);
        std::map<string_type,string_type>::iterator it;
        it = this->default_value_map.find (long_name);
        if (it != this->default_value_map.end()) {
            return it->second;
        }

        /* Not specified on command line, and no default value */
        return "";
    }

    /* Get non-string value, either long or short option, filling in 
       default value if no option specified */
    template <class T>
    void
    get_value (
        T& dest,
        const string_type& name
    ) {
        try {
            dest = dlib::sa = this->get_value (name);
        }
        catch (...) {
            string_type error_string = 
                "Error. Option "
                + get_option_string (name) 
                + " had an illegal or missing argument.";
            throw dlib::error (error_string);
        }
    }

    /* Shorthand functions for specific well-known types */
    void assign_int13 (int *arr, const string_type& name);
    void assign_int_2 (int *arr, const string_type& name);
    void assign_int_6 (int *arr, const string_type& name);
    void assign_plm_long_13 (plm_long *arr, const string_type& name);
    void assign_plm_long_6 (plm_long *arr, const string_type& name);
    void assign_float_13 (float *arr, const string_type& name);
    void assign_float_2 (float *arr, const string_type& name);
    void assign_float_9 (float *arr, const string_type& name);
    void assign_float_vec (
        std::vector<float>* float_vec, 
        const string_type& name);
    int get_int (const string_type& name) {
        int out;
        get_value (out, name);
        return out;
    }
    float get_float (const string_type& name) {
        float out;
        get_value (out, name);
        return out;
    }
    std::string get_string (const string_type& name) {
        return get_value (name);
    }

    void 
    print_options (
        std::basic_ostream<char>& out
    ) {
        typedef char ct;
        typedef std::basic_string<ct> string;
        typedef string::size_type size_type;

        try {
            out << _dT(ct,"Options:");

            // this loop here is just the bottom loop but without the print 
            // statements.  I'm doing this to figure out what len should be.
            size_type max_len = 0; 
            this->reset();
            while (this->move_next()) 
            {
                /* Skip past options which aren't in the map */
                const std::string name = this->element().name();
                std::map<string_type,string_type>::iterator it;
                it = this->option_map.find (name);
                if (it == this->option_map.end()) {
                    continue;
                }

                size_type len = 0; 
                len = it->second.size();

                if (this->element().number_of_arguments() == 1) {
                    len += 6;
                } else {
                    for (unsigned long i = 0; 
                         i < this->element().number_of_arguments(); ++i)
                    {
                        len += 7;
                        if (i+1 > 9)
                            ++len;
                    }
                }

                len += 3;
                if (len < 33)
                    max_len = std::max(max_len,len);
            }

            this->reset();
            while (this->move_next())
            {
                /* Skip past options which aren't in the map */
                const std::string name = this->element().name();
                std::map<string_type,string_type>::iterator it;
                it = this->option_map.find (name);
                if (it == this->option_map.end()) {
                    continue;
                }

                size_type len = 0; 
                out << _dT(ct,"\n");
                out << it->second;
                len = it->second.size();

                if (this->element().number_of_arguments() == 1) {
                    out << _dT(ct," <arg>");
                    len += 6;
                } else {
                    for (unsigned long i = 0; 
                         i < this->element().number_of_arguments(); ++i)
                    {
                        out << _dT(ct," <arg") << i+1 << _dT(ct,">");
                        len += 7;
                        if (i+1 > 9)
                            ++len;
                    }
                }

                out << "   ";
                len += 3;

                while (len < max_len) {
                    ++len;
                    out << " ";
                }

                const unsigned long ml = static_cast<unsigned long>(max_len);
                // now print the description but make it wrap around 
                // nicely if it is to long to fit on one line.
                if (len <= max_len)
                    out << wrap_string(this->element().description(),
                        0, ml+1, wrap_len);
                else
                    out << "\n" 
                        << wrap_string(this->element().description(),
                            ml, ml+1, wrap_len);
            }
            this->reset();
        }
        catch (...)
        {
            this->reset();
            throw;
        }
    }

    void check_help (void) {
        if (this->option("h") || this->option("help")) {
            usage_fn (this, argc, argv);
            exit (0);
        }
    }

    string_type
    get_option_string (const string_type& name) {
        if (name.length() == 1) {
            return "-" + name;
        } else {
            return "--" + name;
        }
    }

    void check_required (const string_type& name) {
        if (!this->have_option(name)) {
            string_type error_string = "Error, you must specify the "
                + get_option_string(name) + " option.\n";
            throw dlib::error (error_string);
        }
    }

/* Throws an exception if none of the arguments are specified 
   on the command line.  The last argument should be zero, 
   to satisfy variable argument list macros. */
    void check_required_any (const char* first_opt, ...) {
        int option_exists = 0;
        va_list argptr;
        const char *opt;

        va_start (argptr, first_opt);
        opt = first_opt;
        do {    
            if (this->option(opt)) {
                option_exists = 1;
                break;
            }
        } while ((opt = va_arg(argptr, char*)));
        va_end (argptr);

        if (!option_exists) {
            string_type error_string = 
                "Error, you must specify one of the following options: ";
            va_start (argptr, first_opt);
            opt = first_opt;
            do {        
                if (opt != first_opt) {
                    error_string += ", ";
                }
                error_string += get_option_string(opt);
            } while ((opt = va_arg(argptr, char*)));
            va_end (argptr);
            error_string += ".\n";
            throw dlib::error (error_string);
        }
    }
}; /* end class */
}  /* end namespace */

/* This is a helper function designed to remove clutter caused by 
   exception catching broilerplate. 
   The optional swallow argument deletes the leading arguments, 
   for ease of use by the plastimatch program.  For example if you 
   set swallow = 1, then the following command:
   
   plastimatch autolabel [options]

   gets parsed as if it were the following:

   autolabel [options]
*/
template<class T>
static void
plm_clp_parse (
    T arg, 
    void (*parse_fn) (T,dlib::Plm_clp*,int,char*[]),
    void (*usage_fn) (dlib::Plm_clp*,int,char*[]),
    int argc,
    char* argv[],
    int swallow = 0)
{
    dlib::Plm_clp parser;
    parser.usage_fn = usage_fn;
    parser.argc = argc;
    parser.argv = argv;
    parser.wrap_len = 79;

    try {
        (*parse_fn) (arg, &parser, argc - swallow, argv + swallow);
    }
    catch (std::exception& e) {
        /* Catch cmd_line_parse_error exceptions and print usage message. */
        std::cout << e.what() << std::endl;
        (*usage_fn) (&parser, argc, argv);
        exit (1);
    }
    catch (...) {
        std::cerr << "An unspecified error occurred.\n";
        exit (1);
    }
}
    
#endif
