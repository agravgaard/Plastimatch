/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dcmtk_module_h_
#define _dcmtk_module_h_

#include "plmbase_config.h"
#include <string>
#include "rt_study_metadata.h"

class DcmDataset;

class PLMBASE_API Dcmtk_module {
public:
    /* C.7.1.1 */
    static void set_patient (DcmDataset *dataset,
        const Metadata::Pointer& meta);
    /* C.7.2.1 */
    static void set_general_study (DcmDataset *dataset, 
        const Rt_study_metadata::Pointer& rsm);
    /* C.7.3.1 */
    static void set_general_series (DcmDataset *dataset, 
        const Metadata::Pointer& meta, const char* modality);
    /* C.7.4.1 */
    static void set_frame_of_reference (DcmDataset *dataset, 
        const Rt_study_metadata::Pointer& rsm);
    /* C.7.5.1 */
    static void set_general_equipment (DcmDataset *dataset,
	const Metadata::Pointer& meta);
    /* C.8.8.1 */
    static void set_rt_series (DcmDataset *dataset, 
        const Metadata::Pointer& meta, const char* modality);
};

#endif
