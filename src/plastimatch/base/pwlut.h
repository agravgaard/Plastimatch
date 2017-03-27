/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pwlut_h_
#define _pwlut_h_

#include "plmutil_config.h"
#include <list>
#include <utility>
#include "itk_image_type.h"
#include "float_pair_list.h"

/*! \brief 
 * The Pwlut class implements a lookup table with piecewise 
 * linear segments
 */
class PLMBASE_API Pwlut {
public:
    Pwlut ();
protected:
    Float_pair_list fpl;
    float left_slope;
    float right_slope;
    Float_pair_list::const_iterator ait_start;
    Float_pair_list::const_iterator ait_end;
public:
    /*! \name Inputs */
    ///@{
    /*! \brief Set the input/output pairs in the lookup table */
    void set_lut (const std::string& pwlut_string);
    /*! \brief Set the input/output pairs in the lookup table */
    void set_lut (const Float_pair_list& pwlut_fpl);
    /*! \brief Interpolate a lookup value */
    float lookup (float vin) const;
    ///@}
};

#endif
