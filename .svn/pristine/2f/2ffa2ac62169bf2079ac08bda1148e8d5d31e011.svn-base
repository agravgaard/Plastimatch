/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>

#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <itksys/RegularExpression.hxx>
#include "itkDirectory.h"
#include "itkRegularExpressionSeriesFileNames.h"

#include "metadata.h"
#include "plm_endian.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "print_and_exit.h"
#include "rt_study_metadata.h"
#include "volume.h"
#include "xio_ct.h"
#include "xio_ct_transform.h"
#include "xio_studyset.h"

Xio_ct_transform::Xio_ct_transform ()
{
    this->set ("HFS");
}

Xio_ct_transform::Xio_ct_transform (const Metadata::Pointer& meta)
{
    this->set (meta);
}

void
Xio_ct_transform::set (const Metadata::Pointer& meta)
{
    std::string ppos = meta->get_metadata(0x0018, 0x5100);
    this->set (ppos.c_str());
}

void
Xio_ct_transform::set (const char* ppos)
{
    /* Use patient position to determine the transformation from XiO
       coordinates to a valid set of DICOM coordinates.
       The origin of the DICOM coordinates will be the same as the
       origin of the XiO coordinates, which is generally not the same
       as the origin of the original DICOM CT scan. */

    /* Offsets */
    this->x_offset = 0;
    this->y_offset = 0;

    /* Direction cosines */
    for (int i = 0; i <= 8; i++) {
	this->direction_cosines[i] = 0.;
    }
    this->direction_cosines[0] = 1.0f;
    this->direction_cosines[4] = 1.0f;
    this->direction_cosines[8] = 1.0f;

    std::string patient_pos = "HFS";
    if (ppos) {
        patient_pos = ppos;
    }

    if (patient_pos == "HFS" ||	patient_pos == "") {

	this->direction_cosines[0] = 1.0f;
	this->direction_cosines[4] = 1.0f;
	this->direction_cosines[8] = 1.0f;

    } else if (patient_pos == "HFP") {

	this->direction_cosines[0] = -1.0f;
	this->direction_cosines[4] = -1.0f;
	this->direction_cosines[8] = 1.0f;

    } else if (patient_pos == "FFS") {

	this->direction_cosines[0] = -1.0f;
	this->direction_cosines[4] = 1.0f;
	this->direction_cosines[8] = -1.0f;

    } else if (patient_pos == "FFP") {

	this->direction_cosines[0] = 1.0f;
	this->direction_cosines[4] = -1.0f;
	this->direction_cosines[8] = -1.0f;

    }
}

void
Xio_ct_transform::set_from_rdd (
    Plm_image *pli,
    Rt_study_metadata *rsm)
{
    /* Use original XiO CT geometry and a DICOM directory to determine
       the transformation from XiO coordinates to DICOM coordinates. */

    Volume *v = pli->get_vol ();

    /* Offsets */
    this->x_offset = 0;
    this->y_offset = 0;

    /* Direction cosines */
    for (int i = 0; i <= 8; i++) {
	this->direction_cosines[i] = 0.;
    }
    this->direction_cosines[0] = 1.0f;
    this->direction_cosines[4] = 1.0f;
    this->direction_cosines[8] = 1.0f;

    Metadata::Pointer& meta = rsm->get_image_metadata ();
    const Plm_image_header *pih = rsm->get_image_header ();
    std::string patient_pos = meta->get_metadata(0x0018, 0x5100);

    if (patient_pos == "HFS" ||	patient_pos == "") {

	/* Offsets */
	this->x_offset = v->origin[0] - pih->origin(0);
	this->y_offset = v->origin[1] - pih->origin(1);

	/* Direction cosines */
	this->direction_cosines[0] = 1.0f;
	this->direction_cosines[4] = 1.0f;
	this->direction_cosines[8] = 1.0f;

    } else if (patient_pos == "HFP") {

	/* Offsets */
	this->x_offset = v->origin[0] + pih->origin(0);
	this->y_offset = v->origin[1] + pih->origin(1);

	/* Direction cosines */
	this->direction_cosines[0] = -1.0f;
	this->direction_cosines[4] = -1.0f;
	this->direction_cosines[8] = 1.0f;

    } else if (patient_pos == "FFS") {

	/* Offsets */
	this->x_offset = v->origin[0] + pih->origin(0);
	this->y_offset = v->origin[1] - pih->origin(1);

	/* Direction cosines */
	this->direction_cosines[0] = -1.0f;
	this->direction_cosines[4] = 1.0f;
	this->direction_cosines[8] = -1.0f;

    } else if (patient_pos == "FFP") {

	/* Offsets */
	this->x_offset = v->origin[0] - pih->origin(0);
	this->y_offset = v->origin[1] + pih->origin(1);

	/* Direction cosines */
	this->direction_cosines[0] = 1.0f;
	this->direction_cosines[4] = -1.0f;
	this->direction_cosines[8] = -1.0f;
    }
}

