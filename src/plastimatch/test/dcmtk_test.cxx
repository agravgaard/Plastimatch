/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler_warnings.h"
#include "dcmtk_rt_study.h"

#if defined (GCS_FIX)
void
dcmtk_series_set_test (char *dicom_dir)
{
    Dcmtk_rt_study drs;
    printf ("Searching directory: %s\n", dicom_dir);
    drs.insert_directory (dicom_dir);
    drs.sort_all ();
    //drs.debug ();

    Rt_study rtds;
    drs.load_rtds (&rtds);

    if (rtds.m_img) {
        rtds.m_img->save_image ("img.mha");
    }
    if (rtds.m_rtss) {
        printf ("Trying to save ss.cxt\n");
        rtds.m_rtss->save_cxt (0, Pstring("ss.cxt"), false);
    }
    if (rtds.m_dose) {
        rtds.m_dose->save_image ("dose.mha");
    }
}
#endif

int
main (int argc, char *argv[])
{
    char *dicom_dir;
    UNUSED_VARIABLE (dicom_dir);
    if (argc == 2) {
	dicom_dir = argv[1];
    } else {
	printf ("Usage: dcmtk_test dicom_dir\n");
	exit (1);
    }

#if defined (GCS_FIX)
    dcmtk_series_set_test (dicom_dir);
#endif

    return 0;
}

