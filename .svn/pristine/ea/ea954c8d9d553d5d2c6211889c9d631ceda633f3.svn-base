/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
  This function is called like this:
  score = mexfancc(A,B,awin,bwin,wthresh,sthresh)

  Or, like this:
  pat = mexfancc('compile',A,awin)
  score = mexfancc('run',pat,B,bwin,wthresh,sthresh)
   ----------------------------------------------------------------------- */
#include <math.h>
#include "mex.h"
#include "scorewin_2.h"
#include "s_fancc.h"
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

void
do_mexfancc_compile (
    int          nlhs,
    mxArray      *plhs[],
    int          nrhs,
    const mxArray *prhs[]
)
{
    /* Check for proper number of arguments */
    verify_mex_nargs ("mexfancc", nlhs, nrhs, 3, 3, 1, 1);

    /* Check for proper argument types */
    verify_mex_rda (prhs[1]);
    verify_mex_rda (prhs[2]);

    build_image (&a, prhs[1]);
    build_image_rect (&awin, prhs[2]);


}

void
do_mexfancc_run (
    int          nlhs,
    mxArray      *plhs[],
    int          nrhs,
    const mxArray *prhs[]
)
{

}

void
do_mexfancc_compile_and_run (
    int          nlhs,
    mxArray      *plhs[],
    int          nrhs,
    const mxArray *prhs[]
)
{
    double *wthresh, *sthresh;
    Image a,aw,b,bw;
    Image_Rect awin,bwin;

    /* Check for proper number of arguments */
    verify_mex_nargs ("mexfancc", nlhs, nrhs, 6, 6, 1, 1);

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
    S_Fancc_Data* udp = (S_Fancc_Data*) malloc (sizeof(S_Fancc_Data));
    udp->weight_threshold = *wthresh;
    udp->std_dev_threshold = *sthresh;

    Score_Win sws;
    sws.initialize = s_fancc_initialize;
    sws.score_point = s_fancc_score_point;
    sws.cleanup = s_fancc_cleanup;
    sws.user_data = (void*) udp;

    /* POW */
    scorewin_2 (&a,&aw,&b,&bw,awin,bwin,&sws);

    /* Copy results to matlab array */
    plhs[0] = mxCreateDoubleMatrix (sws.score->dims[1],
				    sws.score->dims[0],
				    mxREAL);
    memcpy (mxGetPr(plhs[0]),sws.score->data,image_bytes(sws.score));

    /* Free up some variables */
    delete sws.score;
    free (sws.user_data);
}


void mexFunction (
		 int          nlhs,
		 mxArray      *plhs[],
		 int          nrhs,
		 const mxArray *prhs[]
		 )
{
    /* Check for proper number of arguments */
    verify_mex_nargs ("mexfancc", nlhs, nrhs, 3, 6, 1, 1);

    /* Check if the first argument is a string */
    if (mxIsChar(prhs[0])) {
	int buflen;
	char *input_buf;
	
	verify_mex_string (prhs[0]);
	buflen = mxGetN (prhs[0]) + 1;
	input_buf = (char*) mxCalloc (buflen, sizeof(char));
	mxGetString (prhs[0], input_buf, buflen);
	if (strcmp (input_buf, "compile") == 0) {
	    do_mexfancc_compile (nlhs, plhs, nrhs, prhs);
	    return;
	} else if (strcmp (input_buf, "run") == 0) {
	    do_mexfancc_run (nlhs, plhs, nrhs, prhs);
	    return;
	} else {
	    mexErrMsgTxt("Illegal command");
	}
    } else {
	do_mexfancc_compile_and_run (nlhs, plhs, nrhs, prhs);
    }

#if 0
    mexPrintf("Hello world\n");
#endif

}
