/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cxt_io.h"
#include "file_util.h"
#include "metadata.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "rt_study_metadata.h"
#include "rtss.h"
#include "rtss_contour.h"
#include "rtss_roi.h"
#include "string_util.h"

#define CXT_BUFLEN 2048

void
cxt_load (
    Rtss *cxt,                       /* Output: load into this object */
    Rt_study_metadata *rsm,          /* Output: load into this object */
    const char *cxt_fn               /* Input: file to load from */
)
{
    Rtss_contour* curr_contour;

    float val_x = 0;
    float val_y = 0;
    float val_z = 0;

    int have_offset = 0;
    int have_dim = 0;
    int have_spacing = 0;

    std::ifstream fp (cxt_fn);
    if (!fp.is_open()) {
	print_and_exit ("Could not open contour file for read: %s\n", cxt_fn);
    }

    Metadata::Pointer meta = rsm->get_study_metadata ();

    /* Part 1: Dicom info */
    while (1) {
        std::string tag, val;

        getline (fp, tag);
        if (!fp.good()) {
            fprintf (stderr, 
                "Error. cxt file is not formatted correctly: %s\n",
                cxt_fn);
            exit (-1);
        }

        size_t loc = tag.find (' ');
	if (loc != std::string::npos) {
	    val = string_trim (tag.substr (loc + 1));
	    tag = tag.substr (0, loc);
	}
	printf ("%s|%s|\n", tag.c_str(), val.c_str());

	if (tag == "ROI_NAMES") {
            break;
        }
        else if (val == "") {
            continue;
	}
        else if (tag == "PATIENT_NAME") {
	    meta->set_metadata (0x0010, 0x0010, val.c_str());
	}
        else if (tag == "PATIENT_ID") {
	    meta->set_metadata (0x0010, 0x0020, val.c_str());
	}
        else if (tag == "PATIENT_SEX") {
	    meta->set_metadata (0x0010, 0x0040, val.c_str());
	}
        else if (tag == "STUDY_ID") {
            meta->set_metadata (0x0020, 0x0010, val.c_str());
	}
        else if (tag == "CT_STUDY_UID") {
            rsm->set_study_uid (val.c_str());
	}
        else if (tag == "CT_SERIES_UID") {
            rsm->set_ct_series_uid (val.c_str());
	}
        else if (tag == "CT_FRAME_OF_REFERENCE_UID") {
            rsm->set_frame_of_reference_uid (val.c_str());
	}
        else if (tag == "OFFSET") {
	    if (3 == sscanf (val.c_str(), "%f %f %f", 
                    &val_x, &val_y, &val_z)) {
		have_offset = 1;
		cxt->m_offset[0] = val_x;
		cxt->m_offset[1] = val_y;
		cxt->m_offset[2] = val_z;
	    }
	}
        else if (tag == "DIMENSION") {
	    int int_x, int_y, int_z;
	    if (3 == sscanf (val.c_str(), "%d %d %d", 
                    &int_x, &int_y, &int_z)) {
		have_dim = 1;
		cxt->m_dim[0] = int_x;
		cxt->m_dim[1] = int_y;
		cxt->m_dim[2] = int_z;
	    }
	}
        else if (tag == "SPACING") {
	    if (3 == sscanf (val.c_str(), "%f %f %f", 
		    &val_x, &val_y, &val_z)) {
		have_spacing = 1;
		cxt->m_spacing[0] = val_x;
		cxt->m_spacing[1] = val_y;
		cxt->m_spacing[2] = val_z;
	    }
	}
    }
    if (have_offset && have_dim && have_spacing) {
	cxt->have_geometry = 1;
    }

    /* Part 2: Structures info */
    printf ("Starting structure parsing\n");
    while (1) {
        std::string line, color, name;
	int struct_id;
        int rc;

        getline (fp, line);
        if (!fp.good()) {
            fprintf (stderr, 
                "Error. cxt file is not formatted correctly: %s\n",
                cxt_fn);
            exit (-1);
        }

        std::vector<std::string> tokens = string_split (line, '|');
        if (tokens.size() < 3) {
            /* Normal loop exit */
            break;
        }
        rc = sscanf (tokens[0].c_str(), "%d", &struct_id);
        if (rc != 1) {
            goto not_successful;
        }
        color = tokens[1];
        name = tokens[2];
	cxt->add_structure (name, color, struct_id);
    }

    /* Part 3: Contour info */
    printf ("Starting contour parsing\n");
    while (1) {
        int rc, k;
	int num_pt = 0;
	int struct_id = 0;
	int slice_idx;
	Rtss_roi* curr_structure;
        std::string line;

        if (!getline (fp, line)) {
            /* Normal loop exit */
            break;
        }

        std::vector<std::string> tokens = string_split (line, '|');
        if (tokens.size() == 0) {
	    /* No data on this line; should not happen, but we'll 
               accept this. */
            continue;
        }
        if (tokens.size() != 6) {
            goto not_successful;
        }

	/* Structure id: required */
        rc = sscanf (tokens[0].c_str(), "%d", &struct_id);
        if (rc != 1) {
            goto not_successful;
        }

        /* Contour thickness: not used */

        /* Num vertices: required */
        rc = sscanf (tokens[2].c_str(), "%d", &num_pt);
        if (rc != 1) {
	    goto not_successful;
        }

        /* Slice idx: optional */
        rc = sscanf (tokens[3].c_str(), "%d", &slice_idx);
        if (rc != 1) {
            slice_idx = -1;
        }

        /* Slice uid: optional */
        std::string slice_uid = tokens[4];

	/* Find structure which was created when parsing structure. 
           If there is no header line for this structure, we will 
	   will not add the contour into the structure */
        curr_structure = cxt->find_structure_by_id (struct_id);
	if (!curr_structure) {
	    continue;
	}

        curr_contour = curr_structure->add_polyline ();
        curr_contour->num_vertices = num_pt;
        curr_contour->slice_no = slice_idx;
	if (slice_uid[0]) {
	    curr_contour->ct_slice_uid = slice_uid;
	} else {
	    curr_contour->ct_slice_uid = "";
	}

        curr_contour->x = (float*) malloc (num_pt * sizeof(float));
        curr_contour->y = (float*) malloc (num_pt * sizeof(float));
        curr_contour->z = (float*) malloc (num_pt * sizeof(float));
        if (curr_contour->x == 0 || curr_contour->y == 0 
            || curr_contour->z == 0)
        {
            print_and_exit ("Error allocating memory");
        }
        const char *p = tokens[5].c_str();
        for (k = 0; k < num_pt; k++) {
            float x, y, z;
            int nchar;
            if (*p == '\\') {
                p++;
            }
            rc = sscanf (p, "%f\\%f\\%f%n", &x, &y, &z, &nchar);
            if (rc < 3) {
		goto not_successful;
            }
            curr_contour->x[k] = x;
            curr_contour->y[k] = y;
            curr_contour->z[k] = z;
            p += nchar;
        }
        slice_idx = 0;
        num_pt = 0;
    }
    return;

not_successful:
    print_and_exit ("Error parsing input file: %s\n", cxt_fn);
}

void
cxt_save (
    Rtss *cxt,                  /* Input: save this object */
    const Rt_study_metadata::Pointer& rsm, /* In: save this object */
    const char* cxt_fn,         /* Input: File to save to */
    bool prune_empty            /* Input: Should we prune empty structures? */
)
{
    FILE *fp;

    /* Prepare output directory */
    make_parent_directories (cxt_fn);

    fp = fopen (cxt_fn, "wb");
    if (!fp) {
	fprintf (stderr, 
	    "Could not open contour file for write: %s\n", cxt_fn);
        exit (-1);
    }

    /* Part 1: Dicom info */
    Metadata::Pointer meta = rsm->get_study_metadata ();
    /* GCS FIX: There needs to be a way that tells if these are 
       loaded or some default anonymous value */
    if (rsm) {
	fprintf (fp, "CT_SERIES_UID %s\n", rsm->get_ct_series_uid());
    } else {
	fprintf (fp, "CT_SERIES_UID\n");
    }
    if (rsm) {
	fprintf (fp, "CT_STUDY_UID %s\n", rsm->get_study_uid());
    } else {
	fprintf (fp, "CT_STUDY_UID\n");
    }
    if (rsm) {
	fprintf (fp, "CT_FRAME_OF_REFERENCE_UID %s\n", 
	    rsm->get_frame_of_reference_uid());
    } else {
	fprintf (fp, "CT_FRAME_OF_REFERENCE_UID\n");
    }
    fprintf (fp, "PATIENT_NAME %s\n",
	meta->get_metadata (0x0010, 0x0010).c_str());
    fprintf (fp, "PATIENT_ID %s\n",
	meta->get_metadata (0x0010, 0x0020).c_str());
    fprintf (fp, "PATIENT_SEX %s\n",
	meta->get_metadata (0x0010, 0x0040).c_str());
    fprintf (fp, "STUDY_ID %s\n", 
	meta->get_metadata (0x0020, 0x0010).c_str());
    if (cxt->have_geometry) {
	fprintf (fp, "OFFSET %g %g %g\n", cxt->m_offset[0],
	    cxt->m_offset[1], cxt->m_offset[2]);
	fprintf (fp, "DIMENSION %u %u %u\n", (unsigned int) cxt->m_dim[0], 
	    (unsigned int) cxt->m_dim[1], (unsigned int) cxt->m_dim[2]);
	fprintf (fp, "SPACING %g %g %g\n", cxt->m_spacing[0], 
	    cxt->m_spacing[1], cxt->m_spacing[2]);
    }

    /* Part 2: Structures info */
    fprintf (fp, "ROI_NAMES\n");
    for (size_t i = 0; i < cxt->num_structures; i++) {
	Rtss_roi *curr_structure = cxt->slist[i];
	if (prune_empty && curr_structure->num_contours <= 0) {
	    continue;
	}
	fprintf (fp, "%d|%s|%s\n", 
	    curr_structure->id, 
	    (curr_structure->color.empty() 
		? "255\\0\\0"
		: curr_structure->color.c_str()), 
	    curr_structure->name.c_str());
    }
    fprintf (fp, "END_OF_ROI_NAMES\n");

    /* Part 3: Contour info */
    for (size_t i = 0; i < cxt->num_structures; i++) {
	Rtss_roi *curr_structure = cxt->slist[i];
	if (prune_empty && curr_structure->num_contours <= 0) {
	    continue;
	}
	for (size_t j = 0; j < curr_structure->num_contours; j++) {
	    int k;
	    Rtss_contour *curr_polyline = curr_structure->pslist[j];

	    /* struct_no|contour_thickness|num_points|slice_no|slice_uid|points */
	    /* I don't think contour thickness is used. */
	    fprintf (fp, "%d||%d|", curr_structure->id, 
		curr_polyline->num_vertices);
	    /* slice_no and slice_uid are optional */
	    if (curr_polyline->slice_no >= 0) {
		fprintf (fp, "%d|", curr_polyline->slice_no);
	    } else {
		fprintf (fp, "|");
	    }
	    if (curr_polyline->ct_slice_uid.empty()) {
		fprintf (fp, "|");
	    } else {
		fprintf (fp, "%s|", curr_polyline->ct_slice_uid.c_str());
	    }
	    for (k = 0; k < curr_polyline->num_vertices; k++) {
		if (k > 0) {
		    fprintf (fp, "\\");
		}
		fprintf (fp, "%f\\%f\\%f",
		    curr_polyline->x[k], 
		    curr_polyline->y[k], 
		    curr_polyline->z[k]);
	    }
	    fprintf (fp, "\n");
	}
    }

    fclose (fp);
}
