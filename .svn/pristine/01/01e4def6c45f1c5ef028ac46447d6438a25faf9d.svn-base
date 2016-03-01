/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#if (OPENMP_FOUND)
#include <omp.h>
#endif

#include "bspline_interpolate.h"
#include "bspline_xform.h"
#include "direction_cosines.h"
#include "file_util.h"
#include "interpolate_macros.h"
#include "logfile.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "volume_header.h"
#include "volume_macros.h"

Bspline_xform::Bspline_xform ()
{
    for (int d = 0; d < 3; d++) {
        this->img_origin[d] = 0.0f;
        this->img_spacing[d] = 1.0f;
        this->img_dim[d] = 0;
        this->roi_offset[d] = 0;
        this->roi_dim[d] = 0;
        this->vox_per_rgn[d] = 30;
        this->grid_spac[d] = 30.0f;
        this->rdims[d] = 0;
        this->cdims[d] = 0;
    }
    this->dc.set_identity ();
    this->num_knots = 0;
    this->num_coeff = 0;
    this->coeff = 0;

    this->lut_type = LUT_ALIGNED;

    this->cidx_lut = 0;
    this->c_lut = 0;
    this->qidx_lut = 0;
    this->q_lut = 0;

    this->bx_lut = 0;
    this->by_lut = 0;
    this->bz_lut = 0;

    this->ux_lut = 0;
    this->uy_lut = 0;
    this->uz_lut = 0;
}

Bspline_xform::~Bspline_xform ()
{
    if (this->coeff) {
        free (this->coeff);
    }
    if (this->q_lut) {
        free (this->q_lut);
    }
    if (this->c_lut) {
        free (this->c_lut);
    }
    if (this->bx_lut) {
        free (this->bx_lut);
    }
    if (this->by_lut) {
        free (this->by_lut);
    }
    if (this->bz_lut) {
        free (this->bz_lut);
    }
    delete[] ux_lut;
    delete[] uy_lut;
    delete[] uz_lut;
}

static float
bspline_basis_eval (
    int t_idx, 
    int vox_idx, 
    int vox_per_rgn)
{
                                
    float i = (float)vox_idx / vox_per_rgn;

    switch(t_idx) {
    case 0:
        return (1.0/6.0) * (- 1.0 * i*i*i + 3.0 * i*i - 3.0 * i + 1.0);
        break;
    case 1:
        return (1.0/6.0) * (+ 3.0 * i*i*i - 6.0 * i*i           + 4.0);
        break;
    case 2:
        return (1.0/6.0) * (- 3.0 * i*i*i + 3.0 * i*i + 3.0 * i + 1.0);
        break;
    case 3:
        return (1.0/6.0) * (+ 1.0 * i*i*i);
        break;
    default:
        return 0.0;
        break;
    }
}

void
Bspline_xform::save (const char* filename)
{
    FILE* fp;

    make_parent_directories (filename);
    fp = fopen (filename, "wb");
    if (!fp) return;

    fprintf (fp, "MGH_GPUIT_BSP <experimental>\n");
    fprintf (fp, "img_origin = %f %f %f\n", 
        this->img_origin[0], this->img_origin[1], this->img_origin[2]);
    fprintf (fp, "img_spacing = %f %f %f\n", 
        this->img_spacing[0], this->img_spacing[1], this->img_spacing[2]);
    fprintf (fp, "img_dim = %u %u %u\n", 
        (unsigned int) this->img_dim[0], (unsigned int) this->img_dim[1], 
        (unsigned int) this->img_dim[2]);
    fprintf (fp, "roi_offset = %d %d %d\n", 
        (unsigned int) this->roi_offset[0], (unsigned int) this->roi_offset[1], 
        (unsigned int) this->roi_offset[2]);
    fprintf (fp, "roi_dim = %d %d %d\n", 
        (unsigned int) this->roi_dim[0], (unsigned int) this->roi_dim[1], 
        (unsigned int) this->roi_dim[2]);
    fprintf (fp, "vox_per_rgn = %d %d %d\n", 
        (unsigned int) this->vox_per_rgn[0], 
        (unsigned int) this->vox_per_rgn[1], 
        (unsigned int) this->vox_per_rgn[2]);
    float *direction_cosines = this->dc.get_matrix ();
    fprintf (fp, "direction_cosines = %f %f %f %f %f %f %f %f %f\n", 
        direction_cosines[0], 
        direction_cosines[1], 
        direction_cosines[2], 
        direction_cosines[3], 
        direction_cosines[4], 
        direction_cosines[5], 
        direction_cosines[6], 
        direction_cosines[7], 
        direction_cosines[8]);
    /* No need to save grid_spac */

    /* This dumps in itk-like planar format */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < this->num_coeff / 3; j++) {
            fprintf (fp, "%.20f\n", this->coeff[j*3 + i]);
        }
    }           

    fclose (fp);
}

#if PLM_CONFIG_LEGACY_BSPLINE_XFORM_IO

Bspline_xform* 
bspline_xform_load (const char* filename)
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

#else /* PLM_CONFIG_LEGACY_BSPLINE_XFORM_IO is off */

Bspline_xform* 
bspline_xform_load (const char* filename)
{
    int rc;
    float img_origin[3] = {      /* Image origin (in mm) */
        0., 0., 0. };
    float img_spacing[3] = {     /* Image spacing (in mm) */
        1., 1., 1. };
    unsigned int a, b, c;        /* For fscanf */
    plm_long img_dim[3] = {      /* Image size (in vox) */
        0, 0, 0 };
    plm_long roi_offset[3] = {   /* Position of first vox in ROI (in vox) */
        0, 0, 0 };
    plm_long roi_dim[3] = {      /* Dimension of ROI (in vox) */
        0, 0, 0 };
    plm_long vox_per_rgn[3] = {  /* Knot spacing (in vox) */
        0, 0, 0 };
    float dc[9] = {              /* Direction cosines */
        1., 0., 0., 0., 1., 0., 0., 0., 1. };

    std::ifstream ifs (filename);
    if (!ifs.is_open()) {
        return 0;
    }

    /* Check magic number */
    std::string line;
    getline (ifs, line);
    if (!string_starts_with (line, "MGH_GPUIT_BSP")) {
        return 0;
    }

    /* Parse header */
    while (true) {
        getline (ifs, line);
        if (!ifs.good()) {
            logfile_printf ("Error parsing bspline xform\n");
            return 0;
        }

        std::string tag, val;
        if (!split_tag_val (line, tag, val)) {
            /* No "=" found.  Better be the first coefficient. */
            break;
        }
        
        rc = sscanf (line.c_str(), "img_origin = %f %f %f\n", 
            &img_origin[0], &img_origin[1], &img_origin[2]);
        if (rc == 3) continue;

        rc = sscanf (line.c_str(), "img_spacing = %f %f %f\n", 
            &img_spacing[0], &img_spacing[1], &img_spacing[2]);
        if (rc == 3) continue;

        rc = sscanf (line.c_str(), "img_dim = %d %d %d\n", &a, &b, &c);
        if (rc == 3) {
            img_dim[0] = a;
            img_dim[1] = b;
            img_dim[2] = c;
            continue;
        }

        rc = sscanf (line.c_str(), "roi_offset = %d %d %d\n", &a, &b, &c);
        if (rc == 3) {
            roi_offset[0] = a;
            roi_offset[1] = b;
            roi_offset[2] = c;
            continue;
        }

        rc = sscanf (line.c_str(), "roi_dim = %d %d %d\n", &a, &b, &c);
        if (rc == 3) {
            roi_dim[0] = a;
            roi_dim[1] = b;
            roi_dim[2] = c;
            continue;
        }

        rc = sscanf (line.c_str(), "vox_per_rgn = %d %d %d\n", &a, &b, &c);
        if (rc == 3) {
            vox_per_rgn[0] = a;
            vox_per_rgn[1] = b;
            vox_per_rgn[2] = c;
            continue;
        }

        rc = sscanf (line.c_str(), "direction_cosines = %f %f %f %f %f %f %f %f %f\n",
            &dc[0], &dc[1], &dc[2], &dc[3], &dc[4],
            &dc[5], &dc[6], &dc[7], &dc[8]);
        if (rc == 9) continue;

        logfile_printf ("Error loading bxf file\n%s\n", line.c_str());
        return 0;
    }

    logfile_printf ("1\n");

    /* Allocate memory and build LUTs */
    Bspline_xform* bxf = new Bspline_xform;
    bxf->initialize (img_origin, img_spacing, img_dim,
        roi_offset, roi_dim, vox_per_rgn, dc);

    if (bxf->num_coeff < 1) {
        logfile_printf ("Error loading bxf file, no coefficients\n");
        delete bxf;
        return 0;
    }

    /* Load from itk-like planar format */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < bxf->num_coeff / 3; j++) {
            /* The first line is already loaded from before */
            if (i != 0 || j != 0) {
                getline (ifs, line);
            }
            if (!ifs.good()) {
                logfile_printf ("Error parsing bspline xform (idx = %d,%d): %s\n", i, j, filename);
                delete bxf;
                return 0;
            }
            rc = sscanf (line.c_str(), "%f", &bxf->coeff[j*3 + i]);
            if (rc != 1) {
                logfile_printf ("Error parsing bspline xform (idx = %d,%d): %s\n", i, j, filename);
                delete bxf;
                return 0;
            }
        }
    }

    ifs.close();
    return bxf;
}
#endif

/* -----------------------------------------------------------------------
   Debugging routines
   ----------------------------------------------------------------------- */
void
bspline_xform_dump_coeff (Bspline_xform* bxf, const char* fn)
{
    int i;
    FILE* fp = fopen (fn,"wb");
    for (i = 0; i < bxf->num_coeff; i++) {
        fprintf (fp, "%20.20f\n", bxf->coeff[i]);
    }
    fclose (fp);
}

void
bspline_xform_dump_luts (Bspline_xform* bxf)
{
    plm_long i, j, k, p;
    int tx, ty, tz;
    FILE* fp = fopen ("qlut.txt","wb");

    /* Dump q_lut */
    for (k = 0, p = 0; k < bxf->vox_per_rgn[2]; k++) {
        for (j = 0; j < bxf->vox_per_rgn[1]; j++) {
            for (i = 0; i < bxf->vox_per_rgn[0]; i++) {
                fprintf (fp, "%3d %3d %3d\n", 
                    (unsigned int) k, (unsigned int) j, (unsigned int) i);
                for (tz = 0; tz < 4; tz++) {
                    for (ty = 0; ty < 4; ty++) {
                        for (tx = 0; tx < 4; tx++) {
                            fprintf (fp, " %f", bxf->q_lut[p++]);
                        }
                    }
                }
                fprintf (fp, "\n");
            }
        }
    }
    fclose (fp);

    /* Test q_lut */
#if defined (commentout)
    printf ("Testing q_lut\n");
    for (j = 0; j < bxf->vox_per_rgn[2] 
                 * bxf->vox_per_rgn[1] 
                 * bxf->vox_per_rgn[0]; j++) {
        float sum = 0.0;
        for (i = j*64; i < (j+1)*64; i++) {
            sum += bxf->q_lut[i];
        }
        if (fabs(sum-1.0) > 1.e-7) {
            printf ("%g ", fabs(sum-1.0));
        }
    }
    printf ("\n");
#endif

    fp = fopen ("clut.txt","wb");
    p = 0;
    for (k = 0; k < bxf->rdims[2]; k++) {
        for (j = 0; j < bxf->rdims[1]; j++) {
            for (i = 0; i < bxf->rdims[0]; i++) {
                                
                fprintf (fp, "%3u %3u %3u\n", 
                    (unsigned int) k, (unsigned int) j, (unsigned int) i);
                
                for (tz = 0; tz < 4; tz++) {
                    for (ty = 0; ty < 4; ty++) {
                        for (tx = 0; tx < 4; tx++) {
                            fprintf (fp, " %u", (unsigned int) bxf->c_lut[p++]);
                        }
                    }
                }
                fprintf (fp, "\n");
            }
        }
    }
    fclose (fp);
}

void
Bspline_xform::initialize 
(
    float img_origin[3],          /* Image origin (in mm) */
    float img_spacing[3],         /* Image spacing (in mm) */
    plm_long img_dim[3],          /* Image size (in vox) */
    plm_long roi_offset[3],       /* Position of first vox in ROI (in vox) */
    plm_long roi_dim[3],          /* Dimension of ROI (in vox) */
    plm_long vox_per_rgn[3],      /* Knot spacing (in vox) */
    float direction_cosines[9]    /* Direction cosines */
)
{
    plm_long d;
    plm_long i, j, k, p;
    plm_long tx, ty, tz;
    float *A, *B, *C;

    logfile_printf ("bspline_xform_initialize\n");

    this->dc.set (direction_cosines);
    for (d = 0; d < 3; d++) {
        /* copy input parameters over */
        this->img_origin[d] = img_origin[d];
        this->img_spacing[d] = img_spacing[d];
        this->img_dim[d] = img_dim[d];
        this->roi_offset[d] = roi_offset[d];
        this->roi_dim[d] = roi_dim[d];
        this->vox_per_rgn[d] = vox_per_rgn[d];

        /* grid spacing is in mm */
        this->grid_spac[d] = this->vox_per_rgn[d] * fabs (this->img_spacing[d]);

        /* rdims is the number of regions */
        this->rdims[d] = 1 + (this->roi_dim[d] - 1) / this->vox_per_rgn[d];

        /* cdims is the number of control points */
        this->cdims[d] = 3 + this->rdims[d];
    }

    /* total number of control points & coefficients */
    this->num_knots = this->cdims[0] * this->cdims[1] * this->cdims[2];
    this->num_coeff = this->cdims[0] * this->cdims[1] * this->cdims[2] * 3;

    /* Allocate coefficients */
    this->coeff = (float*) malloc (sizeof(float) * this->num_coeff);
    memset (this->coeff, 0, sizeof(float) * this->num_coeff);

    /* Create q_lut */
    this->q_lut = (float*) malloc (sizeof(float) 
        * this->vox_per_rgn[0] 
        * this->vox_per_rgn[1] 
        * this->vox_per_rgn[2] 
        * 64);
    if (!this->q_lut) {
        print_and_exit ("Error allocating memory for q_lut\n");
    }

    A = (float*) malloc (sizeof(float) * this->vox_per_rgn[0] * 4);
    B = (float*) malloc (sizeof(float) * this->vox_per_rgn[1] * 4);
    C = (float*) malloc (sizeof(float) * this->vox_per_rgn[2] * 4);

    for (i = 0; i < this->vox_per_rgn[0]; i++) {
        float ii = ((float) i) / this->vox_per_rgn[0];
        float t3 = ii*ii*ii;
        float t2 = ii*ii;
        float t1 = ii;
        A[i*4+0] = (1.0/6.0) * (- 1.0 * t3 + 3.0 * t2 - 3.0 * t1 + 1.0);
        A[i*4+1] = (1.0/6.0) * (+ 3.0 * t3 - 6.0 * t2            + 4.0);
        A[i*4+2] = (1.0/6.0) * (- 3.0 * t3 + 3.0 * t2 + 3.0 * t1 + 1.0);
        A[i*4+3] = (1.0/6.0) * (+ 1.0 * t3);
    }

    for (j = 0; j < this->vox_per_rgn[1]; j++) {
        float jj = ((float) j) / this->vox_per_rgn[1];
        float t3 = jj*jj*jj;
        float t2 = jj*jj;
        float t1 = jj;
        B[j*4+0] = (1.0/6.0) * (- 1.0 * t3 + 3.0 * t2 - 3.0 * t1 + 1.0);
        B[j*4+1] = (1.0/6.0) * (+ 3.0 * t3 - 6.0 * t2            + 4.0);
        B[j*4+2] = (1.0/6.0) * (- 3.0 * t3 + 3.0 * t2 + 3.0 * t1 + 1.0);
        B[j*4+3] = (1.0/6.0) * (+ 1.0 * t3);
    }

    for (k = 0; k < this->vox_per_rgn[2]; k++) {
        float kk = ((float) k) / this->vox_per_rgn[2];
        float t3 = kk*kk*kk;
        float t2 = kk*kk;
        float t1 = kk;
        C[k*4+0] = (1.0/6.0) * (- 1.0 * t3 + 3.0 * t2 - 3.0 * t1 + 1.0);
        C[k*4+1] = (1.0/6.0) * (+ 3.0 * t3 - 6.0 * t2            + 4.0);
        C[k*4+2] = (1.0/6.0) * (- 3.0 * t3 + 3.0 * t2 + 3.0 * t1 + 1.0);
        C[k*4+3] = (1.0/6.0) * (+ 1.0 * t3);
    }

    p = 0;
    for (k = 0; k < this->vox_per_rgn[2]; k++) {
        for (j = 0; j < this->vox_per_rgn[1]; j++) {
            for (i = 0; i < this->vox_per_rgn[0]; i++) {
                for (tz = 0; tz < 4; tz++) {
                    for (ty = 0; ty < 4; ty++) {
                        for (tx = 0; tx < 4; tx++) {
                            this->q_lut[p++] = A[i*4+tx] * B[j*4+ty] * C[k*4+tz];
                        }
                    }
                }
            }
        }
    }
    free (C);
    free (B);
    free (A);
        
    /* Create c_lut */
    this->c_lut = (plm_long*) malloc (sizeof(plm_long) 
        * this->rdims[0] 
        * this->rdims[1] 
        * this->rdims[2] 
        * 64);
    p = 0;
    for (k = 0; k < this->rdims[2]; k++) {
        for (j = 0; j < this->rdims[1]; j++) {
            for (i = 0; i < this->rdims[0]; i++) {
                for (tz = 0; tz < 4; tz++) {
                    for (ty = 0; ty < 4; ty++) {
                        for (tx = 0; tx < 4; tx++) {
                            this->c_lut[p++] = 
                                + (k + tz) * this->cdims[0] * this->cdims[1]
                                + (j + ty) * this->cdims[0] 
                                + (i + tx);
                        }
                    }
                }
            }
        }
    }

    /* Create b_luts */
    this->bx_lut = (float*)malloc(4*this->vox_per_rgn[0]*sizeof(float));
    this->by_lut = (float*)malloc(4*this->vox_per_rgn[1]*sizeof(float));
    this->bz_lut = (float*)malloc(4*this->vox_per_rgn[2]*sizeof(float));

    for (int j=0; j<4; j++) {
        for (int i=0; i<this->vox_per_rgn[0]; i++) {
            this->bx_lut[i*4+j] = bspline_basis_eval (
                j, i, this->vox_per_rgn[0]);
        }
        for (int i=0; i<this->vox_per_rgn[1]; i++) {
            this->by_lut[i*4+j] = bspline_basis_eval (
                j, i, this->vox_per_rgn[1]);
        }
        for (int i=0; i<this->vox_per_rgn[2]; i++) {
            this->bz_lut[i*4+j] = bspline_basis_eval (
                j, i, this->vox_per_rgn[2]);
        }
    }

    //dump_luts (bxf);

    logfile_printf ("rdims = (%d,%d,%d)\n", 
        this->rdims[0], this->rdims[1], this->rdims[2]);
    logfile_printf ("vox_per_rgn = (%d,%d,%d)\n", 
        this->vox_per_rgn[0], this->vox_per_rgn[1], this->vox_per_rgn[2]);
    logfile_printf ("cdims = (%d %d %d)\n", 
        this->cdims[0], this->cdims[1], this->cdims[2]);
}

/* -----------------------------------------------------------------------
   This extends the bspline grid.  Note, that the new roi_offset 
    in the bxf will not be the same as the one requested, because 
    bxf routines implicitly require that the first voxel of the 
    ROI matches the position of the control point. 
   ----------------------------------------------------------------------- */
/* GCS -- Is there an implicit assumption that the roi_origin > 0? */
void
bspline_xform_extend (
    Bspline_xform* bxf,      /* Output: bxf is initialized */
    int new_roi_offset[3],   /* Position of first vox in ROI (in vox) */
    int new_roi_dim[3]       /* Dimension of ROI (in vox) */
)
{
    int d;
    int roi_offset_diff[3];
    int roi_corner_diff[3];
    int eb[3];  /* # of control points to "extend before" existing grid */
    int ea[3];  /* # of control points to "extend after" existing grid */
    int extend_needed = 0;
    int new_rdims[3];
    int new_cdims[3];
    plm_long new_num_knots;
    plm_long new_num_coeff;
    float* new_coeff;
    plm_long old_idx;
    plm_long i, j, k;

    for (d = 0; d < 3; d++) {
        roi_offset_diff[d] = new_roi_offset[d] - bxf->roi_offset[d];
        roi_corner_diff[d] = (new_roi_offset[d] + new_roi_dim[d]) 
            - (bxf->roi_offset[d] + bxf->roi_offset[d]);

        if (roi_offset_diff[d] < 0) {
            eb[d] = (bxf->vox_per_rgn[d] - roi_offset_diff[d] - 1) 
                / bxf->vox_per_rgn[d];
            extend_needed = 1;
        } else {
            eb[d] = 0;
        }
        if (roi_corner_diff[d] > 0) {
            ea[d] = (bxf->vox_per_rgn[d] + roi_corner_diff[d] - 1) 
                / bxf->vox_per_rgn[d];
            extend_needed = 1;
        } else {
            ea[d] = 0;
        }
    }

    if (extend_needed) {
        /* Allocate new memory */
        for (d = 0; d < 3; d++) {
            new_rdims[d] = bxf->rdims[d] + ea[d] + eb[d];
            new_cdims[d] = bxf->cdims[d] + ea[d] + eb[d];
        }
        new_num_knots = bxf->cdims[0] * bxf->cdims[1] * bxf->cdims[2];
        new_num_coeff = bxf->cdims[0] * bxf->cdims[1] * bxf->cdims[2] * 3;
        new_coeff = (float*) malloc (sizeof(float) * new_num_coeff);
        memset (new_coeff, 0, sizeof(float) * new_num_coeff);

        /* Copy coefficients to new memory */
        for (old_idx = 0, k = 0; k < bxf->cdims[2]; k++) {
            for (j = 0; j < bxf->cdims[1]; j++) {
                for (i = 0; i < bxf->cdims[0]; i++) {
                    plm_long new_idx = 3 * (((((k + eb[2]) * new_cdims[1]) + (j + eb[1])) * new_cdims[0]) + (i + eb[0]));
                    for (d = 0; d < 3; d++, old_idx++, new_idx++) {
                        new_coeff[new_idx] = bxf->coeff[old_idx];
                    }
                }
            }
        }

        /* Free old memory */
        free (bxf->coeff);

        /* Copy over new data into bxf */
        for (d = 0; d < 3; d++) {
            bxf->rdims[d] = new_rdims[d];
            bxf->cdims[d] = new_cdims[d];
        }
        bxf->num_knots = new_num_knots;
        bxf->num_coeff = new_num_coeff;
        bxf->coeff = new_coeff;

        /* Special consideration to ROI */
        for (d = 0; d < 3; d++) {
            bxf->roi_offset[d] = bxf->roi_offset[d] - eb[d] * bxf->vox_per_rgn[d];
            bxf->roi_dim[d] = new_roi_dim[d] + (new_roi_offset[d] - bxf->roi_offset[d]);
        }
    }
}

void
Bspline_xform::fill_coefficients (float val)
{
    int i;

    for (i = 0; i < this->num_coeff; i++) {
        this->coeff[i] = val;
    }
}

/* Set volume header from B-spline Xform */
void 
Bspline_xform::get_volume_header (Volume_header *vh)
{
    vh->set (this->img_dim, this->img_origin, this->img_spacing, 
        this->dc.get_matrix());
}
