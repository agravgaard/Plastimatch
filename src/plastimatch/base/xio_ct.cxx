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

#include "metadata.h"
#include "plm_endian.h"
#include "plm_image.h"
#include "print_and_exit.h"
#include "volume.h"
#include "xio_ct.h"
#include "xio_ct_transform.h"
#include "xio_studyset.h"

typedef struct xio_ct_header Xio_ct_header;
struct xio_ct_header {
    float slice_size[2];
    int dim[2];
    int bit_depth;
    float spacing[2];
    float z_loc;
};

static void
xio_ct_load_header (Xio_ct_header *xch, const char *filename)
{
    /* Open file */
    std::ifstream ifs (filename, std::ifstream::in);
    if (ifs.fail()) {
        print_and_exit ("Error opening file %s for read\n", filename);
    }

    /* Get version */
    std::string line;
    getline (ifs, line);
    int rc, xio_ct_version;
    rc = sscanf (line.c_str(), "%x", &xio_ct_version);
    if (rc != 1) {
	/* Couldn't parse version string -- default to oldest format. */
	xio_ct_version = 0x00071015;
    }

    /* Skip lines */
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);

    /* Get slice location */
    if (!getline (ifs, line)) {
	print_and_exit ("Error reading slice location\n");
    }
    rc = sscanf (line.c_str(), "%g", &xch->z_loc);
    if (rc != 1) {
	print_and_exit ("Error parsing slice location (%s)\n", line.c_str());
    }

    /* Skip 3 lines */
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);

    /* Get slice width, height */
    if (!getline (ifs, line)) {
	print_and_exit ("Error reading slice width, height");
    }
    rc = sscanf (line.c_str(), "%g,%g", &xch->slice_size[0], 
	&xch->slice_size[1]);
    if (rc != 2) {
	print_and_exit ("Error parsing slice width (%s)\n", line.c_str());
    }

    /* Get image resolution */
    if (!getline (ifs, line)) {
	print_and_exit ("Error reading image resolution");
    }
    rc = sscanf (line.c_str(), "%d,%d,%d", &xch->dim[0], &xch->dim[1], 
	&xch->bit_depth);
    if (rc != 3) {
	print_and_exit ("Error parsing image resolution (%s)\n", line.c_str());
    }

    /* Skip 9 lines */
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);

    /* Get pixel size */
    if (!getline (ifs, line)) {
	print_and_exit ("Error reading pixel size\n");
    }
    rc = sscanf (line.c_str(), "%g,%g", &xch->spacing[0], &xch->spacing[1]);
    if (rc != 2) {
	print_and_exit ("Error parsing pixel size (%s)\n", line.c_str());
    }

    /* EPF files have a zero as the second spacing.  Fudge these. */
    if (xch->spacing[1] == 0.f) {
        xch->spacing[1] = xch->spacing[0];
    }

    printf ("%g %g %d %d %g %g %g\n", 
	xch->slice_size[0], xch->slice_size[1], xch->dim[0], xch->dim[1], 
	xch->spacing[0], xch->spacing[1], xch->z_loc);
}

static void
xio_ct_load_image (
    Plm_image *pli, 
    int slice_no,
    const char *filename
)
{
    FILE *fp;
    Volume *v;
    short *img, *slice_img;
    int rc1;
    size_t rc2;

    v = pli->get_vol ();
    img = (short*) v->img;
    slice_img = &img[slice_no * v->dim[0] * v->dim[1]];

    fp = fopen (filename, "rb");
    if (!fp) {
	print_and_exit ("Error opening file %s for read\n", filename);
    }

    /* Load image */
    rc1 = fseek (fp, - v->dim[0] * v->dim[1] * sizeof(short), SEEK_END);
    if (rc1 == -1) {
	print_and_exit ("Error seeking backward when reading image file\n");
    }
    rc2 = fread (slice_img, sizeof(short), v->dim[0] * v->dim[1], fp);
    if (rc2 != (size_t) (v->dim[0] * v->dim[1])) {
	perror ("File error: ");
	print_and_exit (
	    "Error reading xio ct image (%s)\n"
	    "  rc = %u, ferror = %d\n", 
	    filename, (unsigned int) rc2, ferror (fp));
    }

    /* Switch big-endian to native */
    endian2_big_to_native ((void*) slice_img, v->dim[0] * v->dim[1]);

    /* Some older versions of xio set invalid pixels to -32768, while 
       newer versions use -1030.  Seemingly... not enough test data
       to be sure.  Anyway, fudge the values so it looks good. */
    for (int i = 0; i < v->dim[0] * v->dim[1]; i++) {
        if (slice_img[i] < -1030) {
            slice_img[i] = -1030;
        }
    }

    fclose (fp);
}

static void
xio_ct_create_volume (
    Plm_image *pli, 
    Xio_ct_header *xch,
    int best_chunk_len,
    float best_chunk_diff
)
{
    Volume *v;
    plm_long dim[3];
    float origin[3];
    float spacing[3];

    dim[0] = xch->dim[0];
    dim[1] = xch->dim[1];
    dim[2] = best_chunk_len;

    origin[0] = - xch->slice_size[0];
    origin[1] = - xch->slice_size[1];
    origin[2] = xch->z_loc;

    spacing[0] = xch->spacing[0];
    spacing[1] = xch->spacing[1];
    spacing[2] = best_chunk_diff;

    v = new Volume (dim, origin, spacing, 0, PT_SHORT, 1);
    pli->set_volume (v);

    printf ("img: %p\n", v->img);
    printf ("Image dim: %u %u %u\n", (unsigned int) v->dim[0], 
	(unsigned int) v->dim[1], (unsigned int) v->dim[2]);
}

void
xio_ct_load (Plm_image *pli, Xio_studyset *studyset)
{
    int i;

    Xio_ct_header xch;
    std::string ct_file;

    if (studyset->number_slices > 0) {
	ct_file = studyset->studyset_dir 
	    + "/" + studyset->slices[0].filename_scan.c_str();
        xio_ct_load_header (&xch, ct_file.c_str());
	xio_ct_create_volume (pli, &xch, studyset->number_slices, 
	    studyset->thickness);

	for (i = 0; i < studyset->number_slices; i++) {
	    ct_file = studyset->studyset_dir 
		+ "/" + studyset->slices[i].filename_scan.c_str();
	    xio_ct_load_image (pli, i, ct_file.c_str());
	}
    }

    /* The code that loads the structure set needs the ct 
       pixel spacing too, so save that. */
    studyset->ct_pixel_spacing[0] = xch.spacing[0];
    studyset->ct_pixel_spacing[1] = xch.spacing[1];
}

void
xio_ct_apply_transform (Plm_image *pli, Xio_ct_transform *transform)
{
    /* Transform coordinates of an XiO CT scan to DICOM coordinates */

    Volume *v = pli->get_vol ();

    /* Set origins */
    v->origin[0] = (v->origin[0] * transform->direction_cosines[0]) + transform->x_offset;
    v->origin[1] = (v->origin[1] * transform->direction_cosines[4]) + transform->y_offset;

    /* Set direction cosines */
    v->set_direction_cosines (transform->direction_cosines);
}
