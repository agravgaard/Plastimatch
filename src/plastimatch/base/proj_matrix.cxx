/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "file_util.h"
#include "plm_math.h"
#include "proj_matrix.h"
#include "string_util.h"

Proj_matrix::Proj_matrix ()
{
    ic[0] = ic[1] = 0.;
    vec_zero (matrix, 12);
    sad = sid = 0.;
    vec_zero (cam, 3);
    vec_zero (nrm, 3);
    vec_zero (extrinsic, 16);
    vec_zero (intrinsic, 12);
}

Proj_matrix*
Proj_matrix::clone ()
{
    Proj_matrix *pmat;
    
    pmat = new Proj_matrix;
    if (!pmat) return 0;

    /* No dynamically allocated memory in proj_matrix */
    memcpy (pmat, this, sizeof (Proj_matrix));

    return pmat;
}

std::string
Proj_matrix::get ()
{
    std::string s;
    s = PLM_to_string (ic, 2);
    s += " " + PLM_to_string (matrix, 12);
    s += " " + PLM_to_string (sad);
    s += " " + PLM_to_string (sid);
    s += " " + PLM_to_string (cam, 3);
    s += " " + PLM_to_string (nrm, 3);
    s += " " + PLM_to_string (extrinsic, 16);
    s += " " + PLM_to_string (intrinsic, 12);
    return s;
}

void
Proj_matrix::set (const std::string& s)
{
}

static
void
proj_matrix_write (
    Proj_matrix *pmat,
    FILE *fp
)
{
    fprintf (fp, "%18.8e %18.8e\n", pmat->ic[0], pmat->ic[1]);
    fprintf (fp,
	"%18.8e %18.8e %18.8e %18.8e\n" 
	"%18.8e %18.8e %18.8e %18.8e\n" 
	"%18.8e %18.8e %18.8e %18.8e\n", 
	pmat->matrix[0], pmat->matrix[1], pmat->matrix[2], pmat->matrix[3],
	pmat->matrix[4], pmat->matrix[5], pmat->matrix[6], pmat->matrix[7],
	pmat->matrix[8], pmat->matrix[9], pmat->matrix[10], pmat->matrix[11]
    );
    fprintf (fp, "%18.8e\n%18.8e\n", pmat->sad, pmat->sid);

    /* NRM */
    //fprintf (fp, "%18.8e %18.8e %18.8e\n", nrm[0], nrm[1], nrm[2]);
    fprintf (fp, "%18.8e %18.8e %18.8e\n", pmat->extrinsic[8], 
	pmat->extrinsic[9], pmat->extrinsic[10]);

    fprintf (fp,
	"Extrinsic\n"
	"%18.8e %18.8e %18.8e %18.8e\n" 
	"%18.8e %18.8e %18.8e %18.8e\n" 
	"%18.8e %18.8e %18.8e %18.8e\n" 
	"%18.8e %18.8e %18.8e %18.8e\n", 
	pmat->extrinsic[0], pmat->extrinsic[1], pmat->extrinsic[2], 
	pmat->extrinsic[3], pmat->extrinsic[4], pmat->extrinsic[5], 
	pmat->extrinsic[6], pmat->extrinsic[7], pmat->extrinsic[8], 
	pmat->extrinsic[9], pmat->extrinsic[10], pmat->extrinsic[11],
	pmat->extrinsic[12], pmat->extrinsic[13], pmat->extrinsic[14], 
	pmat->extrinsic[15]
    );
    fprintf (fp,
	"Intrinsic\n"
	"%18.8e %18.8e %18.8e %18.8e\n" 
	"%18.8e %18.8e %18.8e %18.8e\n" 
	"%18.8e %18.8e %18.8e %18.8e\n", 
	pmat->intrinsic[0], pmat->intrinsic[1], 
	pmat->intrinsic[2], pmat->intrinsic[3],
	pmat->intrinsic[4], pmat->intrinsic[5], 
	pmat->intrinsic[6], pmat->intrinsic[7],
	pmat->intrinsic[8], pmat->intrinsic[9], 
	pmat->intrinsic[10], pmat->intrinsic[11]
    );
}

void
Proj_matrix::debug ()
{
    proj_matrix_write (this, stdout);
}

void
Proj_matrix::save (
    const char *fn
)
{
    FILE *fp;

    if (!fn) return;

    make_parent_directories (fn);
    fp = fopen (fn, "w");
    if (!fp) {
	fprintf (stderr, "Error opening %s for write\n", fn);
	exit (-1);
    }

    proj_matrix_write (this, fp);

    fclose (fp);
}

void
Proj_matrix::set (
    const double* cam, 
    const double* tgt, 
    const double* vup, 
    double sid, 
    const double* ic, 
    const double* ps, 
    const int* ires
)
{
    const int cols = 4;
    double nrm[3];       /* Panel normal */
    double plt[3];       /* Panel left (toward first column) */
    double pup[3];       /* Panel up (toward top row) */

    vec3_copy (this->cam, cam);
    this->sid = sid;
    this->sad = vec3_dist (cam, tgt);
    this->ic[0] = ic[0];
    this->ic[1] = ic[1];

    /* Compute imager coordinate sys (nrm,pup,plt) 
       ---------------
       nrm = cam - tgt
       plt = nrm x vup
       pup = plt x nrm
       ---------------
    */
    vec3_sub3 (nrm, cam, tgt);
    vec3_normalize1 (nrm);
    vec3_cross (plt, nrm, vup);
    vec3_normalize1 (plt);
    vec3_cross (pup, plt, nrm);
    vec3_normalize1 (pup);

#if defined (commentout)
    printf ("CAM = %g %g %g\n", cam[0], cam[1], cam[2]);
    printf ("TGT = %g %g %g\n", tgt[0], tgt[1], tgt[2]);
    printf ("NRM = %g %g %g\n", nrm[0], nrm[1], nrm[2]);
    printf ("PLT = %g %g %g\n", plt[0], plt[1], plt[2]);
    printf ("PUP = %g %g %g\n", pup[0], pup[1], pup[2]);
#endif

    /* Build extrinsic matrix - rotation part */
    vec_zero (this->extrinsic, 16);
    vec3_copy (&this->extrinsic[0], plt);
    vec3_copy (&this->extrinsic[4], pup);
    vec3_copy (&this->extrinsic[8], nrm);
    vec3_invert (&this->extrinsic[0]);
    vec3_invert (&this->extrinsic[4]);
    vec3_invert (&this->extrinsic[8]);
    m_idx (this->extrinsic,cols,3,3) = 1.0;

    /* Build extrinsic matrix - translation part */
    this->extrinsic[3] = vec3_dot (plt, tgt);
    this->extrinsic[7] = vec3_dot (pup, tgt);
    this->extrinsic[11] = vec3_dot (nrm, tgt) + this->sad;

#if defined (commentout)
    printf ("EXTRINSIC\n%g %g %g %g\n%g %g %g %g\n"
	"%g %g %g %g\n%g %g %g %g\n",
	this->extrinsic[0], this->extrinsic[1], 
	this->extrinsic[2], this->extrinsic[3],
	this->extrinsic[4], this->extrinsic[5], 
	this->extrinsic[6], this->extrinsic[7],
	this->extrinsic[8], this->extrinsic[9], 
	this->extrinsic[10], this->extrinsic[11],
	this->extrinsic[12], this->extrinsic[13], 
	this->extrinsic[14], this->extrinsic[15]);
#endif

    /* Build intrinsic matrix */
    vec_zero (this->intrinsic, 12);
    m_idx (this->intrinsic,cols,0,0) = 1 / ps[0];
    m_idx (this->intrinsic,cols,1,1) = 1 / ps[1];
    m_idx (this->intrinsic,cols,2,2) = 1 / sid;

#if defined (commentout)
    printf ("INTRINSIC\n%g %g %g %g\n%g %g %g %g\n%g %g %g %g\n",
	this->intrinsic[0], this->intrinsic[1], 
	this->intrinsic[2], this->intrinsic[3],
	this->intrinsic[4], this->intrinsic[5], 
	this->intrinsic[6], this->intrinsic[7],
	this->intrinsic[8], this->intrinsic[9], 
	this->intrinsic[10], this->intrinsic[11]);
#endif

    /* Build projection matrix */
    mat_mult_mat (this->matrix, this->intrinsic,3,4, this->extrinsic,4,4);
}

void
Proj_matrix::get_nrm (
    double nrm[3]
)
{
    vec3_copy (nrm, &this->extrinsic[8]);
    vec3_invert (nrm);
}

void
Proj_matrix::get_pdn (
    double pdn[3]
)
{
    vec3_copy (pdn, &this->extrinsic[4]);
}

void
Proj_matrix::get_prt (
    double prt[3]
)
{
    vec3_copy (prt, &this->extrinsic[0]);
}
