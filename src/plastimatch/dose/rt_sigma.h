/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rt_sigma_h_
#define _rt_sigma_h_

#include "rt_plan.h"
#include "rpl_volume.h"

void compute_sigmas(Rt_plan* plan, const Rt_beam* beam, float energy, float* sigma_max, std::string size, int* margins);
void compute_sigma_pt(Rpl_volume* sigma_vol, Rpl_volume* rpl_volume, Rpl_volume* ct_vol, Rt_plan* plan, const Rt_beam* beam, float energy);
float compute_sigma_pt_homo(Rpl_volume* sigma_vol, Rpl_volume* rpl_vol, float energy);
float compute_sigma_pt_hetero(Rpl_volume* sigma_vol, Rpl_volume* rgl_vol, Rpl_volume* ct_vol, float energy);
void compute_sigma_source(Rpl_volume* sigma_vol, Rpl_volume* rpl_volume, Rt_plan* plan, const Rt_beam* beam, float energy);
void compute_sigma_range_compensator(Rpl_volume* sigma_vol, Rpl_volume* rpl_volume, Rt_plan* plan, const Rt_beam* beam, float energy, int* margins);

#endif
