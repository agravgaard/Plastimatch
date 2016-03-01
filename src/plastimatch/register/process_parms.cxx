/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "itk_adjust.h"
#include "logfile.h"
#include "shared_parms.h"
#include "print_and_exit.h"
#include "process_parms.h"
#include "registration_data.h"

typedef std::list<std::pair<std::string,std::string> > Key_value_list;

class Process_parms_private
{
public:
    Shared_parms *shared;
    
    std::string action;
    Key_value_list key_value_list;
public:
    Process_parms_private () {
        shared = new Shared_parms;
    }
    Process_parms_private (const Process_parms_private& s) {
        this->shared = new Shared_parms (*s.shared);
    }
    ~Process_parms_private () {
        delete shared;
    }
};

Process_parms::Process_parms ()
{
    d_ptr = new Process_parms_private;

}

Process_parms::Process_parms (const Process_parms& s) 
{
    d_ptr = new Process_parms_private (*s.d_ptr);
}

Process_parms::~Process_parms ()
{
    delete d_ptr;
}

void 
Process_parms::set_action (const std::string& action)
{
    d_ptr->action = action;
}

void 
Process_parms::set_key_value (const std::string& key, const std::string& value)
{
    d_ptr->key_value_list.push_back (
        make_pair (key, value));

}

void 
Process_parms::execute_process (Registration_data::Pointer& regd) const
{
    if (d_ptr->action == "adjust") {
        lprintf ("*** Executing adjust process ***\n");
        bool adjust_fixed = false;
        bool adjust_moving = false;
        std::string parms = "";
        for (Key_value_list::iterator it = d_ptr->key_value_list.begin();
             it != d_ptr->key_value_list.end();
             it++)
        {
            const std::string& key = it->first;
            const std::string& value = it->second;
            if (key == "parms") {
                parms = value;
            }
            else if (key == "images") {
                if (value == "fixed") {
                    adjust_fixed = true;
                } else if (value == "moving") {
                    adjust_moving = true;
                } else if (value == "fixed,moving") {
                    adjust_fixed = true;
                    adjust_moving = true;
                } else {
                    print_and_exit ("Unknown adjustment line\n");
                }
            }
            else {
                print_and_exit ("Unknown adjustment line\n");
            }
        }
        
        if (adjust_fixed) {
            regd->fixed_image->set_itk (
                itk_adjust (
                    regd->fixed_image->itk_float(),
                    parms));
        }
        if (adjust_moving) {
            regd->moving_image->set_itk (
                itk_adjust (
                    regd->moving_image->itk_float(),
                    parms));
        }
    }
}
