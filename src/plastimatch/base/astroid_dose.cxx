/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <string.h>

#include "astroid_dose.h"
#include "direction_cosines.h"
#include "plm_endian.h"
#include "plm_image.h"
#include "plm_int.h"
#include "print_and_exit.h"
#include "volume.h"
#include "xio_ct.h"
#include "xio_ct_transform.h"

typedef struct astroid_dose_header Astroid_dose_header;
struct astroid_dose_header {
    plm_long dim[3];
    float origin[3];
    float spacing[3];
    std::string dose_type;
};

static void
astroid_dose_load_header (Astroid_dose_header *adh, const char *filename)
{    
    FILE *fp;

    int rc;

    /* Dose cube definition */
    double rx; double ry; double rz;
    double ox; double oy; double oz;
    int nx; int ny; int nz;
    /* Element spacing */
    double dx; double dy; double dz;
    /* Origin */
    double topx; double topy; double topz;

    char line1[1024];
    char line2[1024];

    /* Astroid doesn't include the geometry in the dose export file.
       Therefore an additional <filename>.geometry file will be loaded which
       should contains a line that defines the geometry with 9 values
       in XiO coordinates:

       rx, rz, ry, ox, oz, oy, nx, nz, ny

       The second line of the geometry file may contain the dose type:
       PHYSICAL, EFFECTIVE or ERROR

       Default dose type is EFFECTIVE. If dose type = ERROR, the dose cube
       should contain signed instead of unsigned integers.
    */

    std::string filename_geometry = std::string(filename) + ".geometry";

    fp = fopen (filename_geometry.c_str(), "rb");
    if (!fp) {
	print_and_exit ("Error opening geometry file %s for read\n",
	    filename_geometry.c_str());
    }

    /* Dose grid */
    if (fgets (line1, sizeof(line1), fp) == NULL) {
        print_and_exit ("File error.");
    }

    rc = sscanf (line1, "%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%d",
                 &rx, &rz, &ry, &ox, &oz, &oy, &nx, &nz, &ny);

    if (rc != 9) {
        print_and_exit ("Error. Cannot parse dose cube definition: %s\n", line1);
    }

    printf ("rx = %lf, ry = %lf, rz = %lf\n", rx, ry, rz);
    printf ("ox = %lf, oy = %lf, oz = %lf\n", ox, oy, oz);
    printf ("nx = %d, ny = %d, nz = %d\n", nx, ny, nz);

    /* Calculate element spacing */
    dx = rx / (nx - 1);
    dy = ry / (ny - 1);
    dz = rz / (nz - 1);

    /* Calculate origin */
    //topx = ox - (rx / 2);
    //topy = oy - (ry / 2);
    //topz = -oz - (rz / 2);
    topx = ox - rx/2 - dx/4;
    topy = oy - ry/2;
    topz = -oz - rz/2 - dz/4;

    /* Put info into header */
    adh->dim[0] = nx;
    adh->dim[1] = nz;
    adh->dim[2] = ny;

    adh->spacing[0] = dx;
    adh->spacing[1] = dz;
    adh->spacing[2] = dy;

    adh->origin[0] = topx;
    adh->origin[1] = topz;
    adh->origin[2] = topy;

    if (fgets(line2, sizeof(line2), fp)) {
        /* Remove newline if exists */
        unsigned int len = strlen(line2);
        if (line2[len - 1] == '\n') line2[len - 1] = '\0';

        adh->dose_type = line2;
    } 
    else {
        adh->dose_type = "";
    }

    fclose (fp);
}

static void
astroid_dose_load_cube (
    Plm_image *pli,
    Astroid_dose_header *adh,
    const char *filename
)
{
    FILE *fp;
    Volume *v;
    plm_long i, j, k;
    size_t rc;

    v = pli->get_vol ();
    char* cube_img_read = (char*) v->img;

    fp = fopen (filename, "rb");
    if (!fp) {
	print_and_exit ("Error opening file %s for read\n", filename);
    }

    /* Read dose cube */
    rc = fread (cube_img_read, 4, v->dim[0] * v->dim[1] * v->dim[2], fp);
    if (rc != (size_t) (v->dim[0] * v->dim[1] * v->dim[2])) {
	perror ("File error: ");
	print_and_exit (
	    "Error reading astroid dose cube (%s)\n"
	    "  rc = %d, ferror = %d\n", 
	    filename, rc, ferror (fp));
    }

    /* Switch big-endian to native */
    endian4_big_to_native ((void*) cube_img_read, 
	v->dim[0] * v->dim[1] * v->dim[2]);

    /* Flip XiO Z axis */
    Volume* vflip;
    vflip = new Volume (v->dim, v->origin, v->spacing, 
	v->direction_cosines, v->pix_type, v->vox_planes);

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

    /* Convert volume to float for more accurate normalization */
    pli->convert (PLM_IMG_TYPE_GPUIT_FLOAT);

    /* Convert from cGy to Gy */
    vflip->scale_inplace (0.01);

    fclose (fp);
}

static void
astroid_dose_create_volume (
    Plm_image *pli, 
    Astroid_dose_header *adh
)
{
    Volume *v;

    if (adh->dose_type != "ERROR") {
	v = new Volume (adh->dim, adh->origin, adh->spacing, 0,
	    PT_UINT32, 1);
    } else {
	v = new Volume (adh->dim, adh->origin, adh->spacing, 0,
	    PT_INT32, 1);
        std::cout<<"Reading cube as int32"<<std::endl;
    }
    pli->set_volume (v);

    printf ("img: %p\n", v->img);
    printf ("Image dim: %u %u %u\n", (unsigned int) v->dim[0], 
	(unsigned int) v->dim[1], (unsigned int) v->dim[2]);
}

void
astroid_dose_load (
    Plm_image *pli,
    Metadata::Pointer& meta,
    const char *filename
)
{
    Astroid_dose_header adh;
    
    astroid_dose_load_header(&adh, filename);
    
    // If metadat not set, dose is usually "EFFECTIVE" (Gy(RBE))
    // If it is an "ERROR' type (dose difference) read as int32 instead of uint32 
    std::cout<<"Metadata " << meta->get_metadata(0x3004, 0x0004) << std::endl;
    if (meta->get_metadata(0x3004, 0x0004) == ""){
        if(adh.dose_type==""){
            /* Standard is Gy RBE */
            adh.dose_type = "EFFECTIVE";
            std::cout<<"setting dose type to effective " << std::endl;
        }
        meta->set_metadata(0x3004, 0x0004, adh.dose_type);
    }
    else {
        adh.dose_type = meta->get_metadata(0x3004, 0x0004);
    }
    
    astroid_dose_create_volume(pli, &adh);
    astroid_dose_load_cube(pli, &adh, filename);
}

void
astroid_dose_apply_transform (Plm_image *pli, Xio_ct_transform *transform)
{
    /* Transform coordinates of Astroid dose cube to DICOM coordinates */

    Volume *v;
    v = pli->get_vol ();

    /* Set origins */
    v->origin[0] = (v->origin[0] * transform->direction_cosines[0]) + transform->x_offset;
    v->origin[1] = (v->origin[1] * transform->direction_cosines[4]) + transform->y_offset;

    /* Set direction cosines */
    v->set_direction_cosines (transform->direction_cosines);
}
