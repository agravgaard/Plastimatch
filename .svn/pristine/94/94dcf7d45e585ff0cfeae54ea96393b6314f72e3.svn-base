/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   Test routine for data exchange b/w multiple dll's & matlab
   ----------------------------------------------------------------------- */
#include <iostream>
#include <math.h>
#include "mex.h"
#include "scorewin.h"
#include "s_wncc.h"

extern void _main();

//int initialized = 0;
//double global_array[10];

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
    double v;

    /* Check for proper number of arguments */
    if (nrhs != 1) {
	mexErrMsgTxt("MEXT2 requires one input argument(s).");
    } else if (nlhs != 0) {
	mexErrMsgTxt("MEXT1 requires zero output argument(s).");
    }

    mexPrintf("Hello world\n");
#if 0
#endif

    invp = mxGetPr(prhs[0]);
    inv = *((double**)invp);
    v = *inv;
    mexPrintf("Got a %g\n",v);
    (*inv)++;

    return;
}

