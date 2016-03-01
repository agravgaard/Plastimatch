/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dcmtk_metadata_h_
#define _dcmtk_metadata_h_

#include "plmbase_config.h"
#include "dcmtk_file.h"
#include "metadata.h"

class DcmDataset;
class DcmTagKey;

class Dcmtk_file;

void
dcmtk_copy_from_metadata (
    DcmDataset *dataset, 
    const Metadata::Pointer& meta, 
    const DcmTagKey& tagkey, 
    const char* default_value);

void
dcmtk_copy_into_metadata (
    Metadata::Pointer& meta, 
    const Dcmtk_file::Pointer& df, 
    const DcmTagKey& tagkey);

void
dcmtk_copy_into_metadata (
    Metadata::Pointer& meta, 
    const Dcmtk_file* df, 
    const DcmTagKey& tagkey);

#endif
