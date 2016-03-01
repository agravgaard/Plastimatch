/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   Test routine for data exchange b/w multiple dll's & matlab
   ----------------------------------------------------------------------- */
#include <math.h>
#include "mex.h"
#include "scorewin.h"
#include "mexutils.h"

extern void _main();

int initialized = 0;
double global_array[10];

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
    double *outvp;
    double *invp;
    double *outv;
    double *inv;

    /* Check for proper number of arguments */
    verify_mex_nargs ("MEXT1", nlhs, nrhs, 0, 1, 2, 2);
#if defined (commentout)
    if (nrhs > 2) {
	mexErrMsgTxt("MEXT1 requires zero or one input arguments.");
    } else if (nlhs != 2) {
	mexErrMsgTxt("MEXT1 requires one output argument.");
    }
#endif

    /* Check for proper argument types */
    if (nrhs == 1) {
        check_args (prhs[0]);
    }

    mexPrintf("Hello world\n");
#if 0
#endif

    if (!initialized) {
	int i;
	for (i=0; i<10; i++) {
	    global_array[i] = (double) i;
	}
	initialized = 1;
    }

    /* Make the output */
    plhs[0] = mxCreateDoubleMatrix(1,1,mxREAL);
    outvp = mxGetPr(plhs[0]);
    *outvp = 3;

    plhs[1] = bundle_pointer_for_matlab ((void*) &global_array[3]);

    if (nrhs == 1) {
#if defined (commentout)
	invp = mxGetPr(prhs[0]);
	inv = *((double**)invp);
#endif
	inv = (double*) unbundle_pointer_for_matlab (prhs[0]);
	if (inv == &global_array[3]) {
	    mexPrintf("Victory\n");
	} else {
	    mexPrintf("Defeat\n");
	}
    }

    return;
}

