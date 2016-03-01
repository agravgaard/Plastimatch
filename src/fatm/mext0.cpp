/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   Matlab mex test routine
   ----------------------------------------------------------------------- */
#include "mex.h"

extern void _main();

void mexFunction(int          nlhs,
		 mxArray      *plhs[],
		 int          nrhs,
		 const mxArray *prhs[]
		 )
{
    mexPrintf("Hello world\n");
    return;
}
