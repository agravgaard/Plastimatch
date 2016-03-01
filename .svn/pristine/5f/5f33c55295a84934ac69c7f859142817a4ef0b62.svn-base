/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>

#include "compiler_warnings.h"
#include "file_util.h"
#include "hnd_io_p.h"
#include "hnd_io.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "proj_image.h"
#include "proj_matrix.h"

#define HND_INTENSITY_MAX (139000)
//#define HND_INTENSITY_MAX (10000)

/* -----------------------------------------------------------------------
   Private functions
   ----------------------------------------------------------------------- */
static void 
hnd_adjust_intensity (Proj_image *proj)
{
    int i;
    float *img = proj->img;

    for (i = 0; i < proj->dim[0] * proj->dim[1]; i++) {
        if (img[i] == 0.0f) {
            continue;
        }
        img[i] = img[i] / HND_INTENSITY_MAX;
        img[i] = 1.0 - img[i];

        if (img[i] < 0.0f) img[i] = 0.0;
        /* Exponential mapping -- maybe this helps a little */
        img[i] = exp(img[i]) - 1;
    }
}

static void
hnd_set_proj_matrix (
    Proj_image *proj, 
    double angle, 
    double sad, 
    double sid, 
    const double xy_offset[2]  /* Offset from center position by how much? */
)
{
    double vup[3] = {0, 0, 1};
    double tgt[3] = {0.0, 0.0, 0.0};
    Proj_matrix *pmat = proj->pmat;

    /* Set physical size of imager in mm */
    int isize[2] = { 400, 300 };      /* Actual resolution */

    /* Set pixel size in mm */
    double ps[2] = { (double) isize[0] / (double) proj->dim[0],
                     (double) isize[1] / (double) proj->dim[1] };

    double cam[3];
    double tmp[3];

    pmat->sad = sad;
    pmat->sid = sid;

    /* Set ic = image center (in pixels) */
    pmat->ic[0] = 0.5 * proj->dim[0] - 0.5;
    pmat->ic[1] = 0.5 * proj->dim[1] - 0.5;
    if (xy_offset) {
        pmat->ic[0] += xy_offset[0];
        pmat->ic[1] += xy_offset[1];
    }

    /* Change from varian angles to plastimatch angles */
    cam[0] = sad * cos ((angle + 270) * M_PI / 180.0);
    cam[1] = sad * sin ((angle + 270) * M_PI / 180.0);
    cam[2] = 0.0;

    /* Place camera at distance "sad" from the volume isocenter */
    vec3_sub3 (pmat->nrm, tgt, cam);
    vec3_normalize1 (pmat->nrm);
    vec3_scale3 (tmp, pmat->nrm, sad);
    vec3_copy (cam, tgt);
    vec3_sub2 (cam, tmp);

    pmat->set (cam, tgt, vup, sid, pmat->ic, ps, proj->dim);
}

/* -----------------------------------------------------------------------
   Public functions
   ----------------------------------------------------------------------- */
void
hnd_load (Proj_image *proj, const char *fn, const double xy_offset[2])
{
    Hnd_header hnd;
    FILE *fp;

    uint32_t* buf;
    unsigned char *pt_lut;
    uint32_t a;
    unsigned char v;
    int lut_idx, lut_off;
    size_t num_read;
    char dc;
    short ds;
    long dl, diff = 0l;
    uint32_t i;

    /* JAS 2012.02.06
     * Use a bit bucket to supress compiler warnings for now.
     * TODO: Implement proper error checking instead
     */
    size_t bit_bucket;
    UNUSED_VARIABLE (bit_bucket);

    /* GCS FIX: This is hard coded during debug */
    //double xy_offset[2] = { -2.21 * 4, 0.24 * 4 };  // Good values for test

    if (!proj) return;

    if (!file_exists (fn)) {
        print_and_exit ("Error: hnd_file (%s) does not exist.\n", fn);
    }

    fp = fopen (fn, "rb");
    if (fp == NULL) {
        print_and_exit ("Cannot open %s for read\n", fn);
    }

    bit_bucket = fread ((void *) hnd.sFileType, sizeof(char), 32, fp);
    bit_bucket = fread ((void *) &hnd.FileLength, sizeof(uint32_t), 1, fp);
    bit_bucket = fread ((void *) hnd.sChecksumSpec, sizeof(char), 4, fp);
    bit_bucket = fread ((void *) &hnd.nCheckSum, sizeof(uint32_t), 1, fp);
    bit_bucket = fread ((void *) hnd.sCreationDate, sizeof(char), 8, fp);
    bit_bucket = fread ((void *) hnd.sCreationTime, sizeof(char), 8, fp);
    bit_bucket = fread ((void *) hnd.sPatientID, sizeof(char), 16, fp);
    bit_bucket = fread ((void *) &hnd.nPatientSer, sizeof(uint32_t), 1, fp);
    bit_bucket = fread ((void *) hnd.sSeriesID, sizeof(char), 16, fp);
    bit_bucket = fread ((void *) &hnd.nSeriesSer, sizeof(uint32_t), 1, fp);
    bit_bucket = fread ((void *) hnd.sSliceID, sizeof(char), 16, fp);
    bit_bucket = fread ((void *) &hnd.nSliceSer, sizeof(uint32_t), 1, fp);
    bit_bucket = fread ((void *) &hnd.SizeX, sizeof(uint32_t), 1, fp);
    bit_bucket = fread ((void *) &hnd.SizeY, sizeof(uint32_t), 1, fp);
    bit_bucket = fread ((void *) &hnd.dSliceZPos, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) hnd.sModality, sizeof(char), 16, fp);
    bit_bucket = fread ((void *) &hnd.nWindow, sizeof(uint32_t), 1, fp);
    bit_bucket = fread ((void *) &hnd.nLevel, sizeof(uint32_t), 1, fp);
    bit_bucket = fread ((void *) &hnd.nPixelOffset, sizeof(uint32_t), 1, fp);
    bit_bucket = fread ((void *) hnd.sImageType, sizeof(char), 4, fp);
    bit_bucket = fread ((void *) &hnd.dGantryRtn, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dSAD, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dSFD, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dCollX1, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dCollX2, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dCollY1, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dCollY2, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dCollRtn, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dFieldX, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dFieldY, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dBladeX1, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dBladeX2, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dBladeY1, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dBladeY2, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dIDUPosLng, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dIDUPosLat, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dIDUPosVrt, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dIDUPosRtn, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dPatientSupportAngle, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dTableTopEccentricAngle, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dCouchVrt, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dCouchLng, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dCouchLat, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dIDUResolutionX, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dIDUResolutionY, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dImageResolutionX, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dImageResolutionY, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dEnergy, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dDoseRate, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dXRayKV, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dXRayMA, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dMetersetExposure, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dAcqAdjustment, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dCTProjectionAngle, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dCTNormChamber, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dGatingTimeTag, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dGating4DInfoX, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dGating4DInfoY, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dGating4DInfoZ, sizeof(double), 1, fp);
    bit_bucket = fread ((void *) &hnd.dGating4DInfoTime, sizeof(double), 1, fp);

    printf ("%s %f\n",  fn, hnd.dCTProjectionAngle);

    pt_lut = (unsigned char*) malloc (
        sizeof (unsigned char) * hnd.SizeX * hnd.SizeY);
    buf = (uint32_t*) malloc (
        sizeof(uint32_t) * hnd.SizeX * hnd.SizeY);

    /* Read LUT */
    fseek (fp, 1024, SEEK_SET);
    bit_bucket = fread (pt_lut, sizeof(unsigned char), (hnd.SizeY-1)*hnd.SizeX / 4, fp);

    /* Read first row */
    for (i = 0; i < hnd.SizeX; i++) {
        bit_bucket = fread (&a, sizeof(uint32_t), 1, fp);
        buf[i] = a;
    }

    /* Read first pixel of second row */
    bit_bucket = fread (&a, sizeof(uint32_t), 1, fp);
    buf[i++] = a;
    
    /* Decompress the rest */
    lut_idx = 0;
    lut_off = 0;
    while (i < hnd.SizeX * hnd.SizeY) {
        uint32_t r11, r12, r21;

        r11 = buf[i-hnd.SizeX-1];
        r12 = buf[i-hnd.SizeX];
        r21 = buf[i-1];
        v = pt_lut[lut_idx];
        switch (lut_off) {
        case 0:
            v = v & 0x03;
            lut_off ++;
            break;
        case 1:
            v = (v & 0x0C) >> 2;
            lut_off ++;
            break;
        case 2:
            v = (v & 0x30) >> 4;
            lut_off ++;
            break;
        case 3:
            v = (v & 0xC0) >> 6;
            lut_off = 0;
            lut_idx ++;
            break;
        }
        switch (v) {
        case 0:
            num_read = fread (&dc, sizeof(unsigned char), 1, fp);
            if (num_read != 1) goto read_error;
            diff = dc;
            break;
        case 1:
            num_read = fread (&ds, sizeof(unsigned short), 1, fp);
            if (num_read != 1) goto read_error;
            diff = ds;
            break;
        case 2:
            num_read = fread (&dl, sizeof(uint32_t), 1, fp);
            if (num_read != 1) goto read_error;
            diff = dl;
            break;
        }

        buf[i] = r21 + r12 + diff - r11;
        i++;
    }

    /* Convert hnd to proj_image */
    proj->dim[0] = hnd.SizeX;
    proj->dim[1] = hnd.SizeY;
    proj->img = (float*) malloc (
        hnd.SizeX * hnd.SizeY * sizeof(float));
    if (!proj->img) {
        print_and_exit ("Error allocating memory\n");
    }
    for (i = 0; i < hnd.SizeX * hnd.SizeY; i++) {
        proj->img[i] = (float) buf[i];
    }

    /* Convert from "raw" intensities to attenuation values */
    hnd_adjust_intensity (proj);

    /* Set the matrix */
    /* Note: Varian HND seems to give 0 for the SFD.  We will hard code 
       the sid to 1500 until told otherwise. */
    /* GCS: Sometimes the SAD is 100 (cm) and sometimes it is 1000 (mm).
       So we will hard code the SAD to 1000 mm. */
    proj->pmat = new Proj_matrix;
    hnd_set_proj_matrix (proj, hnd.dCTProjectionAngle, 1000, 1500,
        xy_offset);

    /* Clean up */
    free (pt_lut);
    free (buf);
    fclose (fp);
    return;

 read_error:

    fprintf (stderr, "Error reading hnd file\n");
    free (pt_lut);
    free (buf);
    fclose (fp);
}
