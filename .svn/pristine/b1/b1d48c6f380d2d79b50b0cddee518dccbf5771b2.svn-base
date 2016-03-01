/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsegment_config.h"

#include "logfile.h"
#include "mabs_seg_weights.h"

void
Mabs_seg_weights::print () const
{
    lprintf ("MSW (%s): %f %f %f %f %s\n",
        structure == "" ? "default" : structure.c_str(),
        rho, sigma, minsim, confidence_weight, thresh.c_str());
}


/* If there is an exact match, return its weights.  
   Else if null structure exists, return its weights.
   Else return null pointer */
const Mabs_seg_weights*
Mabs_seg_weights_list::find (
    const std::string& structure) const
{

    lprintf ("MSW searching for (%s)\n", structure.c_str());
    
    const Mabs_seg_weights* msw = &default_weights;
    std::list<Mabs_seg_weights>::const_iterator msw_it;
    for (msw_it = weights_list.begin();
         msw_it != weights_list.end(); msw_it++)
    {
        if (msw_it->structure == structure) {
            lprintf ("MSW search found exact match.\n");
            msw_it->print();
            return &(*msw_it);
        }
        else if (msw_it->structure == "") {
            lprintf ("MSW search found default.\n");
            msw = &(*msw_it);
        }
    }
    lprintf ("MSW search complete.\n");
    msw->print();
    return msw;
}
