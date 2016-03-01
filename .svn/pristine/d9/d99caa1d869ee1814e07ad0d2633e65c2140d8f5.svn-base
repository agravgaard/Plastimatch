/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <itksys/RegularExpression.hxx>
#include "itkDirectory.h"
#include "itkRegularExpressionSeriesFileNames.h"

#include "file_util.h"
#include "metadata.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "rt_study_metadata.h"
#include "rtss.h"
#include "rtss_contour.h"
#include "rtss_roi.h"
#include "string_util.h"
#include "xio_ct.h"
#include "xio_ct_transform.h"
#include "xio_structures.h"

static void
add_cms_contournames (Rtss *rtss, const char *filename)
{
    /* Open file */
    std::ifstream ifs (filename, std::ifstream::in);
    if (ifs.fail()) {
        print_and_exit ("Error opening file %s for read\n", filename);
    }

    int skip_lines = 2;

    /* Read version number */
    std::string line;
    getline (ifs, line);
    int rc, xio_version;
    rc = sscanf (line.c_str(), "%x", &xio_version);
    if (xio_version == 0x00061027) {
	printf ("Version 00061027 found.\n");
	skip_lines = 5;
    }

    /* Skip line */
    getline (ifs, line);

    while (getline (ifs, line)) {
	int rc;

	/* Get structure name */
        std::string structure_name = string_trim (line);

	/* Get structure number */
        getline (ifs, line);
        int structure_id;
	rc = sscanf (line.c_str(), "%d,", &structure_id);

	if (rc != 1) {
	    if (xio_version == 0x00061027) {
		/* This XiO version seems to write corrupted contourfiles 
		   when editing files created with previous versions. 
		   We'll assume that everything went ok.  */
		break;
	    }
	    /* GCS 2010-12-27: It's not only that version which does this.
	       What happens it that XiO leaves garbage at the end of the 
	       file.  The better way to handle this is probably to count 
	       the number of structures and then stop. */
#if defined (commentout)
	    print_and_exit ("Error parsing contournames: "
                "contour id not found (%s)\n", line2->data);
#endif
	}

	/* Xio structures can be zero.  This is possibly not tolerated 
	   by dicom.  So we modify before inserting into the cxt. */
	structure_id ++;
	if (structure_id <= 0) {
	    print_and_exit ("Error, structure_id was less than zero\n");
	}

	/* Add structure */
	rtss->add_structure (structure_name, "", structure_id);

	/* Skip extra lines */
	for (int i = 0; i < skip_lines; i++) {
            getline (ifs, line);
	}
    }
}

static void
add_cms_structure (
    Rtss *rtss, 
    const Xio_studyset& studyset,
    const char *filename, 
    float z_loc)
{
    FILE *fp;
    char buf[1024];

    fp = fopen (filename, "r");
    if (!fp) {
	printf ("Error opening file %s for read\n", filename);
	exit (-1);
    }

    /* Skip first five lines */
    fgets (buf, 1024, fp);
    fgets (buf, 1024, fp);
    fgets (buf, 1024, fp);
    fgets (buf, 1024, fp);
    fgets (buf, 1024, fp);

    while (1) {
	int rc;
	int structure_id, num_points;
	int point_idx, remaining_points;
	Rtss_roi *curr_structure;
	Rtss_contour *curr_polyline;

	/* Get num points */
	fgets (buf, 1024, fp);
	rc = sscanf (buf, "%d", &num_points);
	if (rc != 1) {
	    print_and_exit ("Error parsing file %s (num_points)\n", filename);
	}

	/* Get structure number */
	fgets (buf, 1024, fp);
	rc = sscanf (buf, "%d", &structure_id);
	if (rc != 1) {
	    print_and_exit ("Error parsing file %s (structure_id)\n", 
			    filename);
	}
	
	/* Xio structures can be zero.  This is possibly not tolerated 
	   by dicom.  So we modify before inserting into the cxt. */
	structure_id ++;
	if (structure_id <= 0) {
	    print_and_exit ("Error, structure_id was less than zero\n");
	}

	/* Can this happen? */
	if (num_points == 0) {
	    break;
	}

	/* Look up the cxt structure for this id */
	curr_structure = rtss->find_structure_by_id (structure_id);
	if (!curr_structure) {
	    print_and_exit ("Couldn't reference structure with id %d\n", 
			    structure_id);
	}

	printf ("[%f %d %d]\n", z_loc, structure_id, num_points);
	curr_polyline = curr_structure->add_polyline ();
	curr_polyline->slice_no = -1;
	curr_polyline->num_vertices = num_points;
	curr_polyline->x = (float*) malloc (num_points * sizeof(float));
	curr_polyline->y = (float*) malloc (num_points * sizeof(float));
	curr_polyline->z = (float*) malloc (num_points * sizeof(float));

	point_idx = 0;
	remaining_points = num_points;
	while (remaining_points > 0) {
	    int p, line_points, line_loc;

	    fgets (buf, 1024, fp);

	    if (remaining_points > 5) {
		line_points = 5;
	    } else {
		line_points = remaining_points;
	    }
	    line_loc = 0;

	    for (p = 0; p < line_points; p++) {
		float x, y;
		int rc, this_loc;

		rc = sscanf (&buf[line_loc], "%f, %f,%n", &x, &y, &this_loc);
		if (rc != 2) {
		    print_and_exit ("Error parsing file %s (points) %s\n", 
				    filename, &buf[line_loc]);
		}

        /* GCS 2014-10-16.  As reported by Thomas Botticello 
           and others, the XiO structures are off by 1/2 pixel.
           This adjustment must be done before coourdinate 
           transformation xio to dicom (e.g. prone).
        */
        /* ND 2014-12-18 intent to provide final fix for this 
            NOTE: There is an inherent residual error that cannot be fixed due 
            to XiO's rounding of values at the time of import.
        */
		curr_polyline->x[point_idx] = x 
                    - 0.5 * studyset.ct_pixel_spacing[0];
		curr_polyline->y[point_idx] = -y
                    - 0.5 * studyset.ct_pixel_spacing[1];
		curr_polyline->z[point_idx] = z_loc;
		point_idx ++;
		line_loc += this_loc;
	    }
	    remaining_points -= line_points;
	}
    }

    fclose (fp);
}

void
xio_structures_load (
    Rtss *rtss, 
    const Xio_studyset& studyset
)
{
    /* Get the index file */
    std::string index_file = std::string(studyset.studyset_dir) 
	+ "/" + "contournames";
    if (!itksys::SystemTools::FileExists (index_file.c_str(), true)) {
	print_and_exit ("No xio contournames file found in directory %s\n", 
	    studyset.studyset_dir.c_str());
    }

    /* Load the index file */
    rtss->init ();
    add_cms_contournames (rtss, index_file.c_str());

    /* Load all .WC files, adding data to CXT */
    std::string contour_file;
    for (int i = 0; i < studyset.number_slices; i++) {
	contour_file = studyset.studyset_dir 
	    + "/" + studyset.slices[i].filename_contours;
	add_cms_structure (
            rtss, studyset, 
            contour_file.c_str(), 
	    studyset.slices[i].location);
    }

    rtss->debug ();
}

/* This is idiotic */
static std::string
format_xio_filename (const char *output_dir, float z_loc)
{
    int neg;
    int z_round, z_ones, z_tenths;
    const char *neg_string;
    std::string fn;

    neg = (z_loc < 0);
    if (neg) z_loc = - z_loc;
    z_round = ROUND (z_loc * 10);
    z_ones = z_round / 10;
    z_tenths = z_round % 10;

    neg_string = neg ? "-" : "";

    if (z_ones == 0 && z_tenths == 0) {
	fn = string_format ("%s/T.%s0.WC", output_dir, neg_string);
    } 
    else if (z_ones == 0) {
	fn = string_format ("%s/T.%s.%d.WC", output_dir, neg_string, z_tenths);
    }
    else if (z_tenths == 0) {
	fn = string_format ("%s/T.%s%d.WC", output_dir, neg_string, z_ones);
    }
    else {
	fn = string_format ("%s/T.%s%d.%d.WC", output_dir, neg_string, 
	    z_ones, z_tenths);
    }
    return fn;
}

void
xio_structures_save (
    const Rt_study_metadata::Pointer& rsm,
    Rtss *cxt,
    Xio_ct_transform *transform,
    Xio_version xio_version,
    const char *output_dir
)
{
    FILE *fp;
    std::string fn;

    printf ("X_S_S: output_dir = %s\n", output_dir);

    if (!cxt->have_geometry) {
	print_and_exit ("Sorry, can't output xio format without ct geometry\n");
    }

    /* Write contournames */
    fn = string_format ("%s/%s", output_dir, "contournames");
    make_parent_directories (fn);
    fp = fopen (fn.c_str(), "w");
    if (!fp) {
	print_and_exit ("Error opening output file %s\n", fn.c_str());
    }

    if (xio_version == XIO_VERSION_4_2_1) {
	fprintf (fp, "00031027\n");
    } else {
	fprintf (fp, "00041027\n");
    }

    fprintf (fp, "%lu\n", (unsigned long) cxt->num_structures);

    for (size_t i = 0; i < cxt->num_structures; i++) {
	Rtss_roi *curr_structure = cxt->slist[i];
	int color = 1 + (i % 8);
	int pen = 1;
	/* Class 0 is "patient", class 1 is "Int" */
	int structure_class = (i == 0) ? 0 : 1;
	/* Name */
	fprintf (fp, "%s\n", curr_structure->name.c_str());
	/* Structure no, density, ??, class [, date] */
	fprintf (fp, "%lu,1.000000,0,%d%s\n", 
	    (unsigned long) (i+1), structure_class, 
	    (xio_version == XIO_VERSION_4_2_1) ? "" : ",19691231.190000");
	/* Grouping */
	fprintf (fp, "General\n");
	/* color, ??, pen, ??, ??, ?? */
	fprintf (fp, "%d,5,%d,1,0,0\n", color, pen);
    }
    fclose (fp);

    /* Write WC files */
    for (plm_long z = 0; z < cxt->m_dim[2]; z++) {
	float z_offset = 0.0f;

        Metadata::Pointer meta = rsm->get_study_metadata ();
	std::string patient_pos = meta->get_metadata(0x0018, 0x5100);

	if (patient_pos == "HFS" || patient_pos == "HFP" || patient_pos == "") {
	    z_offset = cxt->m_offset[2];
	} else if (patient_pos == "FFS" || patient_pos == "FFP") {
	    z_offset = - cxt->m_offset[2];
	}

	float z_loc = z_offset + z * cxt->m_spacing[2];
	fn = format_xio_filename (output_dir, z_loc);
	fp = fopen (fn.c_str(), "w");
	if (!fp) {
	    print_and_exit ("Error opening output file %s\n", fn.c_str());
	}
	fprintf (fp, "00061013\n\n");
	fprintf (fp, "0\n0.000,0.000,0.000\n");
	/* GCS FIX: These seem to be min/max */
	fprintf (fp, "-158.1,-135.6, 147.7,  81.6\n");
	for (size_t i = 0; i < cxt->num_structures; i++) {
	    Rtss_roi *curr_structure = cxt->slist[i];
	    for (size_t j = 0; j < curr_structure->num_contours; j++) {
		Rtss_contour *curr_polyline = curr_structure->pslist[j];
		if (z != curr_polyline->slice_no) {
		    continue;
		}
		fprintf (fp, "%d\n", curr_polyline->num_vertices);
		fprintf (fp, "%lu\n", (unsigned long) (i+1));
		for (int k = 0; k < curr_polyline->num_vertices; k++) {
		    fprintf (fp, "%6.1f,%6.1f", 
			curr_polyline->x[k] * transform->direction_cosines[0]
			    - transform->x_offset,
			curr_polyline->y[k] * transform->direction_cosines[4]
			    - transform->y_offset);
		    if ((k+1) % 5 == 0) {
			fprintf (fp, "\n");
		    }
		    else if (k < curr_polyline->num_vertices - 1) {
			fprintf (fp, ",");
		    }
		    else {
			fprintf (fp, "\n");
		    }
		}
	    }
	}
	fprintf (fp, "0\n0\n0\nBart\n");
	fclose (fp);
    }
}

void
xio_structures_apply_transform (
    Rtss *rtss, 
    Xio_ct_transform *transform
)
{
    /* Set offsets */
    rtss->m_offset[0] = (rtss->m_offset[0] * transform->direction_cosines[0])
	+ transform->x_offset;
    rtss->m_offset[1] = (rtss->m_offset[1] * transform->direction_cosines[4])
	+ transform->y_offset;

    /* Transform structures */
    for (size_t i = 0; i < rtss->num_structures; i++) {
	Rtss_roi *curr_structure = rtss->slist[i];
	for (size_t j = 0; j < curr_structure->num_contours; j++) {
	    Rtss_contour *curr_polyline = curr_structure->pslist[j];
	    for (int k = 0; k < curr_polyline->num_vertices; k++) {
		curr_polyline->x[k] =
		    (curr_polyline->x[k] * transform->direction_cosines[0])
		    + transform->x_offset;
		curr_polyline->y[k] =
		    (curr_polyline->y[k] * transform->direction_cosines[4])
		    + transform->y_offset;
	    }
	}
    }
}
