/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "bspline_xform_legacy.h"
#include "logfile.h"

Bspline_xform* 
bspline_xform_legacy_load (const char* filename)
{
    Bspline_xform* bxf;
    char buf[1024];
    FILE* fp;
    int rc;
    float img_origin[3];         /* Image origin (in mm) */
    float img_spacing[3];        /* Image spacing (in mm) */
    unsigned int a, b, c;        /* For fscanf */
    plm_long img_dim[3];           /* Image size (in vox) */
    plm_long roi_offset[3];      /* Position of first vox in ROI (in vox) */
    plm_long roi_dim[3];                 /* Dimension of ROI (in vox) */
    plm_long vox_per_rgn[3];     /* Knot spacing (in vox) */
    float dc[9];                 /* Direction cosines */

    fp = fopen (filename, "r");
    if (!fp) return 0;

    /* Initialize parms */
    bxf = new Bspline_xform;

    /* Skip first line */
    if (fgets (buf, 1024, fp) == NULL) {
        logfile_printf ("File error.\n");
        goto free_exit;
    }

    /* Read header */
    rc = fscanf (fp, "img_origin = %f %f %f\n", 
        &img_origin[0], &img_origin[1], &img_origin[2]);
    if (rc != 3) {
        logfile_printf ("Error parsing input xform (img_origin): %s\n", filename);
        goto free_exit;
    }
    rc = fscanf (fp, "img_spacing = %f %f %f\n", 
        &img_spacing[0], &img_spacing[1], &img_spacing[2]);
    if (rc != 3) {
        logfile_printf ("Error parsing input xform (img_spacing): %s\n", filename);
        goto free_exit;
    }
    rc = fscanf (fp, "img_dim = %d %d %d\n", &a, &b, &c);
    if (rc != 3) {
        logfile_printf ("Error parsing input xform (img_dim): %s\n", filename);
        goto free_exit;
    }
    img_dim[0] = a;
    img_dim[1] = b;
    img_dim[2] = c;

    rc = fscanf (fp, "roi_offset = %d %d %d\n", &a, &b, &c);
    if (rc != 3) {
        logfile_printf ("Error parsing input xform (roi_offset): %s\n", filename);
        goto free_exit;
    }
    roi_offset[0] = a;
    roi_offset[1] = b;
    roi_offset[2] = c;

    rc = fscanf (fp, "roi_dim = %d %d %d\n", &a, &b, &c);
    if (rc != 3) {
        logfile_printf ("Error parsing input xform (roi_dim): %s\n", filename);
        goto free_exit;
    }
    roi_dim[0] = a;
    roi_dim[1] = b;
    roi_dim[2] = c;

    rc = fscanf (fp, "vox_per_rgn = %d %d %d\n", &a, &b, &c);
    if (rc != 3) {
        logfile_printf ("Error parsing input xform (vox_per_rgn): %s\n", filename);
        goto free_exit;
    }
    vox_per_rgn[0] = a;
    vox_per_rgn[1] = b;
    vox_per_rgn[2] = c;

    /* JAS 2012.03.29 : check for direction cosines
     * we must be careful because older plastimatch xforms do not have this */
    rc = fscanf (fp, "direction_cosines = %f %f %f %f %f %f %f %f %f\n",
        &dc[0], &dc[1], &dc[2], &dc[3], &dc[4],
        &dc[5], &dc[6], &dc[7], &dc[8]);
    if (rc != 9) {
        dc[0] = 1.; dc[3] = 0.; dc[6] = 0.;
        dc[1] = 0.; dc[4] = 1.; dc[7] = 0.;
        dc[2] = 0.; dc[5] = 0.; dc[8] = 1.;
    }
    

    /* Allocate memory and build LUTs */
    bxf->initialize (img_origin, img_spacing, img_dim,
        roi_offset, roi_dim, vox_per_rgn, dc);

    /* This loads from itk-like planar format */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < bxf->num_coeff / 3; j++) {
            rc = fscanf (fp, "%f\n", &bxf->coeff[j*3 + i]);
            if (rc != 1) {
                logfile_printf ("Error parsing input xform (idx = %d,%d): %s\n", i, j, filename);
                goto free_exit;
            }
        }
    }

    fclose (fp);
    return bxf;

free_exit:
    fclose (fp);
    delete bxf;
    return 0;
}

