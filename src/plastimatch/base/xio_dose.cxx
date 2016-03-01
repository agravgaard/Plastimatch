/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <itksys/RegularExpression.hxx>
#include "itkDirectory.h"
#include "itkRegularExpressionSeriesFileNames.h"

#include "file_util.h"
#include "metadata.h"
#include "plm_endian.h"
#include "plm_image.h"
#include "print_and_exit.h"
#include "volume.h"
#include "xio_ct.h"
#include "xio_ct_transform.h"
#include "xio_dose.h"
#include "xio_studyset.h"

#define XIO_DATATYPE_UINT32 5

typedef struct xio_dose_header Xio_dose_header;
struct xio_dose_header {
    plm_long dim[3];
    float origin[3];
    float spacing[3];
    double dose_scale_factor;
    double dose_weight;
    int header_size;
    int header_pos_start_geometry;
    int header_pos_end_geometry;
};

static void
xio_dose_load_header (Xio_dose_header *xdh, const char *filename)
{
    FILE *fp;

    int i;
    int rc;
    int dummy;
    /* Header info */
    int xio_dose_version;
    int xio_dose_datatype;
    int xio_sources;
    double xio_dose_scalefactor, xio_dose_weight;
    /* Dose cube definition */
    double rx; double ry; double rz;
    double ox; double oy; double oz;
    int nx; int ny; int nz;
    /* Element spacing */
    double dx; double dy; double dz;
    /* Origin */
    double topx; double topy; double topz;

    char line1[1024];

    fp = fopen (filename, "rb");
    if (!fp) {
	print_and_exit ("Error opening file %s for read\n", filename);
    }

    /* XiO file format version */
    fgets (line1, sizeof(line1), fp);

    rc = sscanf ((const char*) line1, "%x", &xio_dose_version);
    if (rc != 1) {
	/* Couldn't parse version string -- default to oldest known format. */
	xio_dose_version = 0x0037101e;
    }

    /* Skip line */
    fgets (line1, sizeof(line1), fp);

    if (xio_dose_version >= 0x0062101e) {
	/* Skip line */
	fgets (line1, sizeof(line1), fp);
    }

    /* Number of subplans or beams */
    fgets (line1, sizeof(line1), fp);
    rc = sscanf (line1, "%d", &xio_sources);

    if (rc != 1) {
        print_and_exit ("Error. Cannot parse sources/subplans: %s\n", line1);
    }

    printf ("Dose file is a sum of %d sources/subplans:\n", xio_sources);

    /* One line for each source/subplan */
    for (i = 1; i <= xio_sources; i++) {
        fgets (line1, sizeof(line1), fp);
        printf ("Source/subplan %d: %s", i, line1);
    }

    /* Dose normalization info */
    fgets (line1, sizeof(line1), fp);
    rc = sscanf (line1, "%lf,%lf", &xio_dose_scalefactor, &xio_dose_weight);

    if (rc != 2) {
        print_and_exit ("Error. Cannot parse dose normalization: %s\n", line1);
    }

    printf ("Dose scale factor = %f\n", xio_dose_scalefactor);
    printf ("Dose weight = %f\n", xio_dose_weight);

    /* Skip line */
    fgets (line1, sizeof(line1), fp);

    /* Data type */
    fgets (line1, sizeof(line1), fp);
    rc = sscanf (line1, "%1d", &xio_dose_datatype);

    if (rc != 1) {
        print_and_exit ("Error. Cannot parse datatype: %s\n", line1);
    }

    if (xio_dose_datatype != XIO_DATATYPE_UINT32) {
        print_and_exit ("Error. Only unsigned 32-bit integer data is currently supported: %s\n", line1);
    }

    /* Set start position of geometry */
    xdh->header_pos_start_geometry = ftell(fp);

    /* Dose cube definition */
    fgets (line1, sizeof(line1), fp);

    rc = sscanf (line1, "%d,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%d",
                 &dummy, &rx, &rz, &ry, &ox, &oz, &oy, &nx, &nz, &ny);

    if (rc != 10) {
        print_and_exit ("Error. Cannot parse dose cube definition: %s\n", line1);
    }

    printf ("rx = %lf, ry = %lf, rz = %lf\n", rx, ry, rz);
    printf ("ox = %lf, oy = %lf, oz = %lf\n", ox, oy, oz);
    printf ("nx = %d, ny = %d, nz = %d\n", nx, ny, nz);

    /* Set end position of geometry */
    xdh->header_pos_end_geometry = ftell(fp);

    /* Calculate element spacing */
    dx = rx / (nx - 1);
    dy = ry / (ny - 1);
    dz = rz / (nz - 1);

    /* Calculate origin */
    /* ND 12/18/2014 - something does not work for dose placement, there is a
     * half a voxel origin. The values in the grid geometry are not defined at
     * the top of the voxel but the center.
     */
    //topx = ox - (rx / 2);
    //topy = oy - (ry / 2);
    //topz = -oz - (rz / 2);
    topx = ox - rx/2 - dx/4;
    topy = oy - ry/2;
    topz = -oz - rz/2 - dz/4;

    /* Put info into header */
    xdh->dim[0] = nx;
    xdh->dim[1] = nz;
    xdh->dim[2] = ny;

    xdh->spacing[0] = dx;
    xdh->spacing[1] = dz;
    xdh->spacing[2] = dy;

    xdh->origin[0] = topx;
    xdh->origin[1] = topz;
    xdh->origin[2] = topy;

    xdh->dose_scale_factor = xio_dose_scalefactor;
    xdh->dose_weight = xio_dose_weight;

    /* Get size of full header */
    rc = fseek(fp, - nx * ny * nz * sizeof (uint32_t), SEEK_END);
    if (rc == -1) {
	print_and_exit ("Error seeking backward when reading XiO dose header\n");
    }
    xdh->header_size = ftell(fp);

    fclose (fp);
}

static void
xio_dose_load_cube (
    Plm_image *pli,
    Xio_dose_header *xdh,
    const char *filename
)
{
    FILE *fp;
    Volume *v;
    uint32_t *cube_img_read;
    plm_long i, j, k;
    int rc1;
    size_t rc2;
    

    v = pli->get_vol ();
    cube_img_read = (uint32_t*) v->img;

    fp = fopen (filename, "rb");
    if (!fp) {
	print_and_exit ("Error opening file %s for read\n", filename);
    }

    /* Read dose cube */
    rc1 = fseek (fp, 
	- v->dim[0] * v->dim[1] * v->dim[2] * sizeof(uint32_t), SEEK_END);
    if (rc1 == -1) {
	print_and_exit ("Error seeking backward when reading image file\n");
    }
    rc2 = fread (cube_img_read, 
	sizeof(uint32_t), v->dim[0] * v->dim[1] * v->dim[2], fp);
    if (rc2 != (size_t) (v->dim[0] * v->dim[1] * v->dim[2])) {
	perror ("File error: ");
	print_and_exit (
	    "Error reading xio dose cube (%s)\n"
	    "  rc = %u, ferror = %d\n", 
	    filename, (unsigned int) rc2, ferror (fp));
    }

    /* Switch big-endian to native */
#if defined (commentout)
    for (i = 0; i < v->dim[0] * v->dim[1] * v->dim[2]; i++) {
	char lenbuf[4];
	char tmpc;
	memcpy (lenbuf, (char*) &cube_img_read[i], 4);
	tmpc = lenbuf[0]; lenbuf[0] = lenbuf[3]; lenbuf[3] = tmpc;
	tmpc = lenbuf[1]; lenbuf[1] = lenbuf[2]; lenbuf[2] = tmpc;
	memcpy ((char*) &cube_img_read[i], lenbuf, 4);
    }
#endif
    endian4_big_to_native ((void*) cube_img_read, 
	v->dim[0] * v->dim[1] * v->dim[2]);

    /* Flip XiO Z axis */
    Volume* vflip;
    vflip = new Volume (v->dim, v->origin, v->spacing, 
	v->direction_cosines, v->pix_type, 1);

    for (k=0;k<v->dim[2];k++) {
	for (j=0;j<v->dim[1];j++) {
	    for (i=0;i<v->dim[0];i++) {
		memcpy ((float*)vflip->img
		    + volume_index (vflip->dim, i, (vflip->dim[1]-1-j), k), 
		    (float*)v->img 
		    + volume_index (v->dim, i, j, k), v->pix_size);
	    }
	}
    }

    pli->set_volume (vflip);

    /* GCS 2011-01-24: No need to call volume_destroy(v) because set_gpuit() 
       will destroy an existing volume */

    /* Convert volume to float for more accurate normalization */
    pli->convert (PLM_IMG_TYPE_GPUIT_FLOAT);

    /* Normalize dose. Factor 0.01 is to convert from cGy to Gy */
    vflip->scale_inplace (xdh->dose_weight * xdh->dose_scale_factor * 0.01);

    fclose (fp);
}

static void
xio_dose_create_volume (
    Plm_image *pli, 
    Xio_dose_header *xdh
)
{
    Volume *v;

    v = new Volume (xdh->dim, xdh->origin, xdh->spacing, 0, 
	PT_UINT32, 1);
    pli->set_volume (v);

    printf ("img: %p\n", v->img);
    printf ("Image dim: %ld %ld %ld\n", 
        (long) v->dim[0], (long) v->dim[1], (long) v->dim[2]);
}

void
xio_dose_load (
    Plm_image *pli,
    Metadata::Pointer& meta,
    const char *filename
)
{
    Xio_dose_header xdh;

    xio_dose_load_header (&xdh, filename);
    xio_dose_create_volume (pli, &xdh);
    xio_dose_load_cube (pli, &xdh, filename);

    /* GCS FIX: The below seems to be hard coded for pXiO? */
    /* XiO dose is in Gy RBE */
    if (meta->get_metadata(0x3004, 0x0004) == ""){
        meta->set_metadata(0x3004, 0x0004, "EFFECTIVE");
    }
}

void
xio_dose_save (
    const Plm_image::Pointer& pli,
    Metadata::Pointer& meta,
    Xio_ct_transform *transform,
    const char *filename,
    const char *filename_template
)
{
    /* Because XiO dose files can contain huge amounts of information,
       most of which is not used by plastimatch, the only feasible
       saving method is to copy over the header from the input dose file.
       This means that saving XiO dose is only possible when the input
       is also XiO dose.

       The line in the dose header that defines is geometry will be
       adjusted to the new geometry */

    FILE *fp, *fpt;
    Xio_dose_header xdh;

    plm_long i, j, k;
    char header;
    size_t result;

    Volume::Pointer v = pli->get_volume_float ();

    /* Dose cube definition */
    double rx; double ry; double rz;
    double ox; double oy; double oz;
    int nx; int ny; int nz;

    make_parent_directories (filename);
    fp = fopen (filename, "wb");
    if (!fp) {
	print_and_exit ("Error opening file %s for write\n", filename);
    }

    fpt = fopen (filename_template, "rb");
    if (!fpt) {
	print_and_exit ("Error opening file %s for read\n", filename_template);
    }

    xio_dose_load_header(&xdh, filename_template);

    /* Write first part of header */
    for (i = 0; i < xdh.header_size; i++) {
        result = fread (&header, sizeof(header), 1, fpt);
        if (result != 1) {
            print_and_exit ("Error. Cannot read dose template header (1).\n");
        }
        fwrite (&header, sizeof(header), 1, fp);
    }

    /* Write dose cube definition */
    rx = v->spacing[0] * (v->dim[0] - 1);
    ry = v->spacing[2] * (v->dim[2] - 1);
    rz = v->spacing[1] * (v->dim[1] - 1);

    ox = (v->origin[0] + (rx / 2)) - transform->x_offset;
    oy = (v->origin[2] + (ry / 2)) - transform->y_offset;
    oz = - (v->origin[1] + (rz / 2));

    std::string patient_pos = meta->get_metadata(0x0018, 0x5100);

    if (patient_pos == "HFS" ||	patient_pos == "") {
	ox =   ox * v->direction_cosines[0];
	oy =   oy * v->direction_cosines[8];
	oz =   oz * v->direction_cosines[4];
    } else if (patient_pos == "HFP") {
	ox = - ox * v->direction_cosines[0];
	oy =   oy * v->direction_cosines[8];
	oz = - oz * v->direction_cosines[4];
    } else if (patient_pos == "FFS") {
	ox = - ox * v->direction_cosines[0];
	oy = - oy * v->direction_cosines[8];
	oz =  oz * v->direction_cosines[4];
    } else if (patient_pos == "FFP") {
	ox =   ox * v->direction_cosines[0];
	oy = - oy * v->direction_cosines[8];
	oz = - oz * v->direction_cosines[4];
    }

    nx = v->dim[0];
    ny = v->dim[2];
    nz = v->dim[1];

    fprintf (fp, "%d,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%d\n",
	0, rx, rz, ry, ox, oz, oy, nx, nz, ny);

    /* Write second part of header */
    fseek (fpt, xdh.header_pos_end_geometry, SEEK_SET);
    for (i = 0; i < xdh.header_size - xdh.header_pos_end_geometry; i++) {
        result = fread (&header, sizeof(header), 1, fpt);
        if (result != 1) {
            print_and_exit ("Error. Cannot read dose template header (2).\n");
        }
        fwrite (&header, sizeof(header), 1, fp);
    }

    /* Create new volume for output */
    Volume* v_write;
    v_write = new Volume (v->dim, v->origin, v->spacing, 
	v->direction_cosines, v->pix_type, v->vox_planes);

    /* Clone volume and flip XiO Z axis */
    for (k = 0; k < v->dim[2]; k++) {
	for (j=0;j<v->dim[1];j++) {
	    for (i=0;i<v->dim[0];i++) {
		memcpy ((float*)v_write->img
		    + volume_index (v_write->dim, i, (v_write->dim[1]-1-j), k), 
		    (float*)v->img 
		    + volume_index (v->dim, i, j, k), v->pix_size);
	    }
	}
    }

    /* Convert to floating point */
    volume_convert_to_float (v_write);

    /* Apply normalization backwards */
    v_write->scale_inplace (
	1 / (xdh.dose_weight * xdh.dose_scale_factor * 0.01));

    /* Convert to unsigned 32-bit integer */
    volume_convert_to_uint32 (v_write);
    uint32_t *cube_img_write = (uint32_t*) v_write->img;

    /* Switch native to big-endian */
#if defined (commentout)
    for (i = 0; i < v_write->dim[0] * v_write->dim[1] * v_write->dim[2]; i++) {
	char lenbuf[4];
	char tmpc;
	memcpy (lenbuf, (char*) &cube_img_write[i], 4);
	tmpc = lenbuf[0]; lenbuf[0] = lenbuf[3]; lenbuf[3] = tmpc;
	tmpc = lenbuf[1]; lenbuf[1] = lenbuf[2]; lenbuf[2] = tmpc;
	memcpy ((char*) &cube_img_write[i], lenbuf, 4);
    }
#endif
    endian4_native_to_big ((void*) cube_img_write, 
	v->dim[0] * v->dim[1] * v->dim[2]);

    /* Write dose cube */
    /* FIX: Not taking direction cosines into account */
    result = fwrite (cube_img_write, sizeof(uint32_t),
	v_write->dim[0] * v_write->dim[1] * v_write->dim[2], fp);
    if (result != (size_t) (v_write->dim[0] * v_write->dim[1] * v_write->dim[2])) {
	print_and_exit ("Error. Cannot write dose cube to %s.\n", filename);
    }

    fclose(fp);
    fclose(fpt);

    delete v_write;
}

void
xio_dose_apply_transform (Plm_image *pli, Xio_ct_transform *transform)
{
    /* Transform coordinates of XiO dose cube to DICOM coordinates */

    Volume *v = pli->get_vol ();

    /* Set origins */
    v->origin[0] = (v->origin[0] * transform->direction_cosines[0]) + transform->x_offset;
    v->origin[1] = (v->origin[1] * transform->direction_cosines[4]) + transform->y_offset;

    /* Set direction cosines */
    v->set_direction_cosines (transform->direction_cosines);
}
