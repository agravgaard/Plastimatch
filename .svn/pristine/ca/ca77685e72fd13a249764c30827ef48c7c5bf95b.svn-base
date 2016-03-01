/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <math.h>
#include "mex.h"
#include "scorewin.h"
#include "s_ncc.h"
#include "mexutils.h"

extern void _main();

void
check_args (const mxArray* arg)
{
    int mrows = mxGetM(arg);
    int ncols = mxGetN(arg);
    if (!mxIsDouble(arg) || mxIsComplex(arg)) {
	mexErrMsgTxt("Inputs must be a noncomplex scalar double.");
    }
}

void
check_args_2 (const mxArray* arg)
{
    int mrows = mxGetM(arg);
    int ncols = mxGetN(arg);
    if (!mxIsDouble(arg) || mxIsComplex(arg) || (mrows * ncols != 4)) {
	mexErrMsgTxt("Inputs must be a noncomplex scalar double of size 4.");
    }
}


/* -----------------------------------------------------------------------
   This function is called like this:
   score = wncc(A,AW,B,BW,awin,bwin,wthresh,sthresh);
   ----------------------------------------------------------------------- */
void mexFunction(
		 int          nlhs,
		 mxArray      *plhs[],
		 int          nrhs,
		 const mxArray *prhs[]
		 )
{
    double *wthresh, *sthresh;
    Image a, b;
    Image_Rect awin, bwin;

    /* Check for proper number of arguments */
    verify_mex_nargs ("mexncc", nlhs, nrhs, 6, 6, 1, 1);

    /* Check for proper argument types */
    verify_mex_args_rda (nrhs, prhs);

    wthresh = (double *) mxGetPr(prhs[4]);
    sthresh = (double *) mxGetPr(prhs[5]);

    /* Input arguments are MATLAB images */
    build_image (&a, prhs[0]);
    build_image (&b, prhs[1]);

    /* Fill in the ImageRect structs.  Ditto.  TM's instructions are:
       ImageRect window (presumably he means width_in_cols) are:
       window.p (leftmost_column, topmost_row);
       window.dims (width_in_rows, height_in_rows);
    */
    build_image_rect (&awin, prhs[2]);
    build_image_rect (&bwin, prhs[3]);

    /* Fire up the scorewin struct */
    S_Ncc_Data* udp = (S_Ncc_Data*) malloc (sizeof(S_Ncc_Data));
    udp->weight_threshold = *wthresh;
    udp->std_dev_threshold = *sthresh;

    Score_Win sws;
    sws.initialize = s_ncc_initialize;
    sws.score_point = s_ncc_score_point;
    sws.user_data = (void*) udp;

    /* POW */
    scorewin (&a,0,&b,0,awin,bwin,&sws);

    /* Copy results to matlab array */
    plhs[0] = mxCreateDoubleMatrix (sws.score->dims[1],
				    sws.score->dims[0],
				    mxREAL);
    memcpy (mxGetPr(plhs[0]),sws.score->data,image_bytes(sws.score));

    /* Free up some variables */
    delete sws.score;
    free (sws.user_data);

    return;
}
