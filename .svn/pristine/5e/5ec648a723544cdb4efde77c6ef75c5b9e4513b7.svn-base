/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_image_set_h_
#define _plm_image_set_h_

#include "plmbase_config.h"
#include "compiler_warnings.h"
#include "itk_image.h"
#include "metadata.h"
#include "plm_image_type.h"
#include "smart_pointer.h"
#include "volume.h"

class Plm_image_set_private;

/*! \brief 
 * The Plm_image_set class represents a set of 
 * three-dimensional volumes.  It is used for importing 
 * DICOM or XiO volumes which are scanned at multiple slice spacings.
 */
class PLMBASE_API Plm_image_set {
public:
    SMART_POINTER_SUPPORT (Plm_image_set);
    Plm_image_set_private *d_ptr;
public:
    Plm_image_set ();
    ~Plm_image_set ();

};

#endif
