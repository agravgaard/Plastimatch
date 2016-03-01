/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "mex.h"

#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fdk_opts.h"
#include "fdk_util.h"
#include "file_util.h"
#include <math.h>
#include "plm_math.h"
#include "proj_image.h"
#include "proj_matrix.h"
#include "ramp_filter.h"
#include "volume.h"
#include "drr.h"
#include "drr_opts.h"

void
mexFunction (
    int nlhs, mxArray *plhs[],
    int nrhs, const mxArray *prhs[])
{
    /* declare input variables */
    
/*
    mxArray *proj_in = prhs[0];
    double *proj = mxGetPr(proj_in);
    mxArray *cam_in = prhs[1];
    double *cam = mxGetPr(cam_in);
    mxArray *tgt_in = prhs[2];
    double *tgt = mxGetPr(tgt_in);
    mxArray *vup_in = prhs[3];
    double *vup = mxGetPr(vup_in);
    mxArray *sid_in = prhs[4];
    double *sid = mxGetPr(sid_in);
    mxArray *ic_in = prhs[5];
    double *ic = mxGetPr(ic_in);
    mxArray *ps_in = prhs[6];
    double *ps = mxGetPr(ps_in);
    mxArray *ires_in = prhs[7];
    double *ires = mxGetPr(ires_in);
*/
   
    /* DRR API example */
    Volume *vol;
    Proj_image *proj;
    
    /* hardcoded values */
    double cam[3] = { 1500,1500,1500 };
    double tgt[3] = { 20,20,20 };       //iso from RT plan
    double vup[3] = { 0,0,1 };
    double sid[3] = { 100,100,100 };
    double ic[3] = { 0,0,0 };
    double ps[3] = { 1,1,1 };
    int ires[2] = { 512,512 };
    char options[3] = "cpu";
    
    
    
    /* Create the CT volume */
    int dim[3] = { 512, 512, 152 };
    float offset[3] = { -255.5, -255.5, -123.75 };
    float spacing[3] = { 1.0, 1.0, 2.5 };
    enum Volume_pixel_type pix_type = PT_FLOAT;
    float direction_cosines[9] = {
          1.0, 0.0, 0.0,
          0.0, 1.0, 0.0,
          0.0, 0.0, 1.0 };
    vol = new Volume (dim, offset, spacing, direction_cosines, 
	pix_type, 1, 0);

    /* Fill in the CT volume with values */
    float *img = (float*) vol->img;
    img[100] = 32.6;

    /* Create empty projection image */
    proj = proj_image_create ();
    /* Add storage for image bytes */
    proj_image_create_img (proj, ires);
    /* Add empty projection matrix */
    proj_image_create_pmat (proj);

    /* Set up the projection matrix */
    proj_matrix_set (proj->pmat, cam, tgt, vup, *sid, ic, ps, ires);

    /* Render the drr */
    drr_render_volume_perspective (proj, vol, ps, 0, options);

    /* Do something with the image */
    /* declare output variables */
/*
    double *drr = mxGetPr(drr_out);
*/
    
    printf("pixel (32,10) is: %g\n", proj->img[32*ires[0]+10]);


    
    /* output */
/*
    plhs[0] = drr_out;
*/
    
    
    
    /* Clean up memory */
    delete vol;
    proj_image_destroy (proj);
}
