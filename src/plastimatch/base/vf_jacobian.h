/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _vf_jacobian_h_
#define _vf_jacobian_h_

#include "plmbase_config.h"
#include <string>
#include "itk_image_type.h"

class Plm_image;

class PLMBASE_API Jacobian_stats {
public:
    float min;
    float max;
    std::string outputstats_fn;
public:
    Jacobian_stats () {
	outputstats_fn = " ";
	min=0;
	max=0;
    }
};

class PLMBASE_API Jacobian {
public:
    /*Xform * */
    DeformationFieldType::Pointer vf;
    std::string vfjacstats_fn;
    float jacobian_min;
    float jacobian_max;

public:
    Jacobian();
    void set_input_vf (DeformationFieldType::Pointer vf);
    void set_output_vfstats_name (const std::string& vfjacstats);
    FloatImageType::Pointer make_jacobian ();
private:  
    void write_output_statistics(Jacobian_stats *);
};

#endif
