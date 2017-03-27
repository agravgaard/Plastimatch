/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"

#include "logfile.h"
#include "registration_parms.h"
#include "shared_parms.h"

Shared_parms::Shared_parms ()
{
    this->metric[DEFAULT_IMAGE_KEY] = Metric_parms ();
    this->fixed_roi_enable = true;
    this->moving_roi_enable = true;
    this->fixed_stiffness_enable = true;

    /* At some future date, this will be changed to "false" */
    // this->legacy_subsampling = false;
    this->legacy_subsampling = true;
}

Shared_parms::Shared_parms (const Shared_parms& s)
{
    this->metric = s.metric;
    this->fixed_roi_enable = s.fixed_roi_enable;
    this->moving_roi_enable = s.moving_roi_enable;
    this->fixed_stiffness_enable = s.fixed_stiffness_enable;
    this->legacy_subsampling = s.legacy_subsampling;

    /* filenames do not propagate when copied */
}

Shared_parms::~Shared_parms ()
{
}

void
Shared_parms::copy (const Shared_parms *s)
{
    this->metric = s->metric;
    this->fixed_roi_enable = s->fixed_roi_enable;
    this->moving_roi_enable = s->moving_roi_enable;
    this->fixed_stiffness_enable = s->fixed_stiffness_enable;
    this->legacy_subsampling = s->legacy_subsampling;

    /* filenames do not propagate when copied */
}

void
Shared_parms::log ()
{
    lprintf ("LOG Shared parms\n");
    std::map<std::string, Metric_parms>::iterator it;
    for (it = this->metric.begin(); it != this->metric.end(); ++it) {
        lprintf ("Shared metric | %s | %d\n", it->first.c_str(), 
            it->second.metric_type);
    }
}
