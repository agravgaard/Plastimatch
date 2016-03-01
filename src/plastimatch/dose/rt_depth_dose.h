/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rt_depth_dose_h_
#define _rt_depth_dose_h_

#include "plmdose_config.h"

class PLMDOSE_API Rt_depth_dose {
public:
    Rt_depth_dose ();
    Rt_depth_dose (double E0, double spread, double dres, 
        double dend);
    ~Rt_depth_dose ();

    bool load (const char* fn);     /* load from file */
    bool generate ();               /* generate analytically */

    /* debug: print bragg curve to file */
    void dump (const char* fn) const;

	/* Get dose maximum information */
	int get_index_of_dose_max();

	float lookup_energy_integration(float depth, float dz) const;
	float lookup_energy (float depth);

private:
    bool load_xio (const char* fn);
    bool load_txt (const char* fn);

public:
    float* d_lut;                   /* depth array (mm) */
    float* e_lut;                   /* energy array (MeV) */
	float* f_lut;					/* integrated energy array (MeV) */

    float E0;                      /* initial ion energy (MeV) */
    double spread;                  /* beam energy sigma (MeV) */
    double dres;                    /* spatial resolution of bragg curve (mm)*/
    double dend;                    /* maximum w.e.d. (mm) */
    int num_samples;                /* # of discrete bragg curve samples */

	int index_of_dose_max;
};

#endif
