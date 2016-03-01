/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _MEXUTILS_H
#define _MEXUTILS_H

void
verify_mex_nargs (char* pgm_name, 
		  int nlhs, int nrhs, int ninmin, int ninmax,
		  int noutmin, int noutmax);
int
verify_mex_rda (const mxArray* arg);
int
verify_mex_rda_4 (const mxArray* arg);
void
verify_mex_args_rda (int narg, const mxArray* args[]);
void
verify_mex_string (const mxArray* arg);

int
verify_scalar_double (const mxArray* arg);

char*
mex_strdup (const mxArray* arg);

mxArray* 
bundle_pointer_for_matlab (void* vp);
int
check_bundled_pointer_for_matlab (const mxArray* mp);
void*
unbundle_pointer_for_matlab (const mxArray* mp);

#endif
