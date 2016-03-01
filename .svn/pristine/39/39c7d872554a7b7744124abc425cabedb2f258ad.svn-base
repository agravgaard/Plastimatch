/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dcmtk_slice_data_h_
#define _dcmtk_slice_data_h_

#include "plmbase_config.h"
#include <string>

#include "plm_int.h"
#include "volume.h"

class Dcmtk_slice_data
{
public:
    std::string fn;
    Volume::Pointer vol;

    size_t slice_size;
    float *slice_float;
    int16_t *slice_int16;

    float intercept;
    float slope;

    char slice_uid[100];
    std::string ipp;
    std::string iop;
    std::string sloc;
    std::string sthk;
    int instance_no;
};

#endif
