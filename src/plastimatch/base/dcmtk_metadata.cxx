/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <stdio.h>
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_file.h"
#include "dcmtk_metadata.h"
#include "dcmtk_series.h"
#include "metadata.h"

void
dcmtk_copy_from_metadata (
    DcmDataset *dataset, 
    const Metadata::Pointer& meta, 
    const DcmTagKey& tagkey, 
    const char* default_value)
{
    if (meta) {
        const char* md = meta->get_metadata_ (
            tagkey.getGroup(), tagkey.getElement());
        if (md) {
            dataset->putAndInsertString (tagkey, md);
            return;
        }
    }
    dataset->putAndInsertString (tagkey, default_value);
}

void
dcmtk_copy_into_metadata (
    Metadata::Pointer& meta, 
    const Dcmtk_file::Pointer& df, 
    const DcmTagKey& tag_key)
{
    dcmtk_copy_into_metadata (meta, df.get(), tag_key);
}

void
dcmtk_copy_into_metadata (
    Metadata::Pointer& meta, 
    const Dcmtk_file* df, 
    const DcmTagKey& tag_key)
{
    const char* value = df->get_cstr (tag_key);
    if (value) {
        meta->set_metadata (
            tag_key.getGroup(), tag_key.getElement(), value);
    }
}
