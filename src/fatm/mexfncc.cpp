/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <math.h>
#include "mex.h"
#include "scorewin_2.h"
#include "s_fncc.h"
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
    Image a,aw,b,bw;
    Image_Rect awin,bwin;

    /* Check for proper number of arguments */
    verify_mex_nargs ("mexfncc", nlhs, nrhs, 6, 6, 1, 1);

    /* Check for proper argument types */
    verify_mex_args_rda (nrhs, prhs);

    wthresh = (double *) mxGetPr(prhs[4]);
    sthresh = (double *) mxGetPr(prhs[5]);

#if 0
    mexPrintf("Hello world\n");
#endif

    /* Fill in the Image structs.  Note that TM assumes row major order,
       so we gotta flip the rows and cols to get the right answer for 
       row major.  TM's instructions are:
       Image <GrayScale, float> image (width, height, array);
    */
#if defined (commentout)
    if (check_bundled_pointer_for_matlab (prhs[0])) {
	/* Input arguments are bundled GPyr's */
	/* Argument #9 is therefore the level */
	int level = (int) *((double *) mxGetPr(prhs[8]));
	Gpyr *a_g = (Gpyr*) unbundle_pointer_for_matlab (prhs[0]);
	Gpyr *aw_g = (Gpyr*) unbundle_pointer_for_matlab (prhs[1]);
	Gpyr *b_g = (Gpyr*) unbundle_pointer_for_matlab (prhs[2]);
	Gpyr *bw_g = (Gpyr*) unbundle_pointer_for_matlab (prhs[3]);
	
	/* Note: this isn't a copy b/c of the ref counting */
	a = (*a_g)[level];
	aw = (*aw_g)[level];
	b = (*b_g)[level];
	bw = (*bw_g)[level];
    } else {
	/* Input arguments are MATLAB images */
	a = Image<GrayScale,double>(mxGetM(prhs[0]),mxGetN(prhs[0]),
				    (double*) mxGetPr(prhs[0]));
	aw = Image<GrayScale,double>(mxGetM(prhs[1]),mxGetN(prhs[1]),
				     (double*) mxGetPr(prhs[1]));
	b = Image<GrayScale,double>(mxGetM(prhs[2]),mxGetN(prhs[2]),
				    (double*) mxGetPr(prhs[2]));
	bw = Image<GrayScale,double>(mxGetM(prhs[3]),mxGetN(prhs[3]),
				     (double*) mxGetPr(prhs[3]));
    }
#endif

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
    S_Fncc_Data* udp = (S_Fncc_Data*) malloc (sizeof(S_Fncc_Data));
    udp->weight_threshold = *wthresh;
    udp->std_dev_threshold = *sthresh;

    Score_Win sws;
    sws.initialize = s_fncc_initialize;
    sws.score_point = s_fncc_score_point;
    sws.cleanup = s_fncc_cleanup;
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

    return;
}
