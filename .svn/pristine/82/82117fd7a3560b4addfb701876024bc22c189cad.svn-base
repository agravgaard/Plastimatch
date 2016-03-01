/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "mex.h"
#include <stdio.h>
#include "config.h"

#define MSGBUF 128
#define BUNDLED_POINTER_MAGIC 1.234e5

#if defined (WIN32)
#define snprintf _snprintf
#endif

void
verify_mex_nargs (char* pgm_name, 
		  int nlhs, int nrhs, int ninmin, int ninmax,
		  int noutmin, int noutmax)
{
    char msgbuf[MSGBUF];
    if (nrhs < ninmin || nrhs > ninmax) {
	if (ninmin == ninmax) {
	    snprintf (msgbuf, MSGBUF, 
		      "%s: Requires %d input arguments.",
		      pgm_name, ninmin);
	} else {
	    snprintf (msgbuf, MSGBUF, 
		      "%s: Requires between %d and %d input arguments.",
		      pgm_name, ninmin, ninmax);
	}
	mexErrMsgTxt(msgbuf);
    }
    if (nlhs < noutmin || nlhs > noutmax) {
	if (noutmin == noutmax) {
	    snprintf (msgbuf, MSGBUF, 
		      "%s: Requires %d output arguments.",
		      pgm_name, noutmin);
	} else {
	    snprintf (msgbuf, MSGBUF, 
		      "%s: Requires between %d and %d output arguments.",
		      pgm_name, noutmin, noutmax);
	}
	mexErrMsgTxt(msgbuf);
    }
}

/* rda == real double array */
int
verify_mex_rda (const mxArray* arg)
{
    if (mxIsDouble(arg) && !mxIsComplex(arg)) {
	return 1;
    } else {
	return 0;
    }
}

int
verify_mex_rda_4 (const mxArray* arg)
{
    size_t mrows = mxGetM(arg);
    size_t ncols = mxGetN(arg);
    if (mxIsDouble(arg) && !mxIsComplex(arg) && (mrows * ncols == 4)) {
	return 1;
    } else {
	return 0;
    }
}

int
verify_scalar_double (const mxArray* arg)
{
    size_t mrows = mxGetM(arg);
    size_t ncols = mxGetN(arg);
    if (mxIsDouble(arg) && !mxIsComplex(arg) && mrows == 1 && ncols == 1) {
	return 1;
    } else {
	return 0;
    }
}

void
verify_mex_args_rda (int narg, const mxArray* args[])
{
    int i;
    for (i = 0; i < narg; i++) {
	verify_mex_rda (args[i]);
    }
}

void
verify_mex_string (const mxArray* arg)
{
    int number_of_dimensions = mxGetNumberOfDimensions(arg);
    if (number_of_dimensions > 2 || mxGetM(arg) != 1) {
	mexErrMsgTxt("Only simple strings supported.");
    }
}

char*
mex_strdup (const mxArray* arg)
{
    int buflen;
    char* buf;

    verify_mex_string (arg);
    buflen = mxGetN (arg) + 1;
    buf = (char*) mxCalloc (buflen, sizeof(char));
    mxGetString (arg, buf, buflen);
    return buf;
}

mxArray* 
bundle_pointer_for_matlab (void* vp)
{
    mxArray* mp;
    double* dp;

    mp = mxCreateDoubleMatrix(2,2,mxREAL);
    dp = mxGetPr(mp);
    *((double**)dp) = (double*) vp;
    *(dp+3) = BUNDLED_POINTER_MAGIC;
    return mp;
}

int
check_bundled_pointer_for_matlab (const mxArray* mp)
{
    double* dp;
    dp = mxGetPr(mp);
    return (*(dp+3) == BUNDLED_POINTER_MAGIC);
}

void*
unbundle_pointer_for_matlab (const mxArray* mp)
{
    double* dp;
    void* vp;

    dp = mxGetPr(mp);
    if (*(dp+3) != BUNDLED_POINTER_MAGIC) {
	mexErrMsgTxt("NO BUNDLED POINTER MAGIC!");
    }
    vp = *((void**)dp);
    return vp;
}
