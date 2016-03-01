/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rt_dose_h_
#define _rt_dose_h_

#include "aperture.h"
#include "rt_depth_dose.h"
#include "rt_plan.h"
#include "plmdose_config.h"
#include "plm_image.h"

double
energy_direct (
    float rgdepth,             /* voxel to dose */
    Rt_beam* beam,
	int beam_idx
);

void compute_dose_ray_desplanques (
    Volume* dose_volume, 
    Volume::Pointer ct_vol, 
    Rt_beam* beam, 
    Volume::Pointer final_dose_volume, 
    int beam_index
);
void compute_dose_ray_sharp (
    const Volume::Pointer ct_vol,  
    Rt_beam* beam,
    Rpl_volume* rpl_dose_volume,
	int beam_index,
    const int* margins
);
void compute_dose_ray_shackleford (
    Volume::Pointer dose_volume, 
    Rt_plan* plan, 
    Rt_beam *beam,
    int beam_index, 
    std::vector<double>* area, 
    std::vector<double>* xy_grid, 
    int radius_sample, 
    int theta_sample
);
void add_rcomp_length_to_rpl_volume(Rt_beam* beam);

#endif
