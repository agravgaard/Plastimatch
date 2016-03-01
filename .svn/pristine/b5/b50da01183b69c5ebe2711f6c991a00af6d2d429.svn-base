/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _proj_matrix_h_
#define _proj_matrix_h_

#include "plmbase_config.h"

class PLMBASE_API Proj_matrix {
public:
    Proj_matrix ();

public:
    double ic[2];	  /* Image Center:  ic[0] = x, ic[1] = y */
    double matrix[12];	  /* Projection matrix */
    double sad;		  /* Distance: Source To Axis */
    double sid;		  /* Distance: Source to Image */
    double cam[3];	  /* Location of camera */
    double nrm[3];	  /* Ray from image center to source */

    double extrinsic[16];
    double intrinsic[12];

public:
    void get_nrm (double nrm[3]);
    void get_pdn (double pdn[3]);
    void get_prt (double prt[3]);
    void set (
        const double* cam, 
        const double* tgt, 
        const double* vup, 
        double sid, 
        const double* ic, 
        const double* ps, 
        const int* ires
    );
    void save (const char *fn);
    void debug ();
    Proj_matrix* clone ();
};

#endif
