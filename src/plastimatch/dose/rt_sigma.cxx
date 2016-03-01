/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */

#include "ray_data.h"
#include "rpl_volume.h"
#include "rt_beam.h"
#include "rt_lut.h"
#include "rt_sigma.h"

void compute_sigmas (
    Rt_plan* plan,
    const Rt_beam* beam,
    float energy,
    float* sigma_max, 
	std::string size, 
	int* margins)
{
    /* We compute the sigmas for source, range compensator and patient as described in the Hong's paper */
    /* First we set the volume in which the sigmas have to be calculated: the normal rpl_sigma_volume,  */
    /* or the extended rpl_sigma_volume for margins. */

    Rpl_volume* sigma_vol;
    Rpl_volume* ct_vol;
    Rpl_volume* rgl_vol;

    if (size == "small")
    {
        sigma_vol = beam->sigma_vol;
        ct_vol = beam->rpl_ct_vol_HU;
        rgl_vol = beam->rpl_vol;
    }
    else
    {
        sigma_vol = beam->sigma_vol_lg;
        ct_vol = beam->rpl_ct_vol_HU_lg;
        rgl_vol = beam->rpl_vol_lg;
    }

    /* Now that the volumes were defined, we can compute the sigmas in them and do the quadratic sigmas sum */
    /* sigma^2 patient */
    compute_sigma_pt (sigma_vol, rgl_vol, ct_vol, plan, beam, energy);
    /* + sigma^2 source */
    if (beam->get_source_size() > 0)
    {            
        compute_sigma_source(sigma_vol, rgl_vol, plan, beam, energy);
    }
    else
    {
        printf("Sigma source computed - sigma_src_max = 0 mm. (Source size <= 0)\n");
    }
    /* + sigma^2 range compensator */
    if (beam->get_aperture()->have_range_compensator_image() && energy > 1)
    {            
        compute_sigma_range_compensator(sigma_vol, rgl_vol, plan, beam, energy, margins);
    }
    else
    {
        printf("Sigma range compensator computed - sigma_rc_max = 0 mm. (No range compensator or the energy is too small)\n");
    } 
    /* Last step: sigma = sqrt(sigma_pt^2 + sigma_src^2 + sigma_rc^2), at this point sigma_vol contains the sum of the sigmas' square */    /* We update also the value of sigma_max */
    float* sigma_img = (float*) sigma_vol->get_vol()->img;
    plm_long dim[3] = { 
        sigma_vol->get_vol()->dim[0], 
        sigma_vol->get_vol()->dim[1], 
        sigma_vol->get_vol()->dim[2]
    };
    *sigma_max = 0;

    for (int i = 0; i < dim[0] * dim[1] * dim[2]; i++)
    {
        sigma_img[i] = sqrt(sigma_img[i]);
        if (sigma_img[i] > *sigma_max)
        {
            *sigma_max = sigma_img[i];
        }
    }
    printf("Global sigma computed - Global sigma_max = %lg mm.\n", *sigma_max);
    return;
}

void compute_sigma_pt (
    Rpl_volume* sigma_vol,
    Rpl_volume* rpl_volume,
    Rpl_volume* ct_vol,
    Rt_plan* plan,
    const Rt_beam* beam,
    float energy)
{
    float sigma_max = 0;

    if (beam->get_homo_approx() == 'y')
    {
        sigma_max = compute_sigma_pt_homo(sigma_vol, rpl_volume, energy);
    }
    else
    {
        sigma_max = compute_sigma_pt_hetero(sigma_vol, rpl_volume, ct_vol, energy);
    }
    printf("Sigma patient computed - sigma_pt_max = %lg mm.\n", sigma_max);
    return;
}

float compute_sigma_pt_homo (
    Rpl_volume* sigma_vol,
    Rpl_volume* rpl_vol,
    float energy)
{
    float sigma_max = 0;
	int dim[3] = {sigma_vol->get_vol()->dim[0],  sigma_vol->get_vol()->dim[1], sigma_vol->get_vol()->dim[2]};
	int dim_rpl[3] = {rpl_vol->get_vol()->dim[0], rpl_vol->get_vol()->dim[1], rpl_vol->get_vol()->dim[2]};
	int idx = 0;

    if (dim[0] != dim_rpl[0] || dim[1] != dim_rpl[1] || dim[2] != dim_rpl[2])
    {
        printf("Error: rpl_vol & sigma_vol have different dimensions. Sigma volume not built\n");
        return 0;
    }
    /* At this time, sigma_vol contains the range length, WITHOUT range compensator */
    float* sigma_volume = (float*) sigma_vol->get_vol()->img;
    float* rpl_img = (float*) rpl_vol->get_vol()->img;
    unsigned char* ap_img = NULL;

	double x_over_range = 0;
	
    if (rpl_vol->get_aperture()->have_aperture_image())
    {
        ap_img = (unsigned char*) rpl_vol->get_aperture()->get_aperture_volume()->img;
    } 
    /*  Hong method to calculate the sigma value for homogeneous medium */
    /* Range value in water extracted from a fit based on 1-250MeV from the NIST data - ranges in mm */
    double range = 10 * get_proton_range(energy);
    
    /* Sigma0 value from the Hong fit - See paper Hong "A pencil beam algorithm for proton dose calculation" - sigma in mm: x10 */
    double sigma0 = 0.02275 * range + 1.2085E-6 * range * range;

    /* Calculation of the sigma values from the medium equivalent depth  */
    for (int i = 0; i < dim[0] * dim [1]; i++)
    {
		for (int k = 0; k < dim[2]; k++)
		{
			idx = k * dim[0] * dim[1] +i;
			if (!rpl_vol->get_aperture()->have_aperture_image() || (rpl_vol->get_aperture()->have_aperture_image() && ap_img[i] > 0))
			{
				if (rpl_img[idx] <= 0) 
				{
					sigma_volume[idx] = 0;
				}
				else if (rpl_img[idx] >= range)
				{
					sigma_volume[idx] = sigma0 * sigma0; // sigma will contains the square of the sigmas to do the quadratic sum
            
					/* sigma_max update */
					if (sigma0 > sigma_max)
					{
						sigma_max = sigma0;
					}
				}
				else
				{
					x_over_range = rpl_img[idx] / range;

					/* sigma = y0 * Hong (x/range) */
					sigma_volume[idx] = sigma0 * x_over_range * ( 0.26232 + 0.64298 * x_over_range + 0.0952393 * x_over_range * x_over_range);
            
					/* sigma_max update */
					if (sigma_volume[idx] > sigma_max)
					{
						sigma_max = sigma_volume[idx];
					}
					sigma_volume[idx] *= sigma_volume[idx]; // We return sigma^2 to sigma_vol
				}
			}
		}
    }
    return sigma_max;
}

float compute_sigma_pt_hetero (
    Rpl_volume* sigma_vol,
    Rpl_volume* rgl_vol,
    Rpl_volume* ct_vol,
    float energy)
{
    float sigma_max = 0;

    float* sigma_img = (float*) sigma_vol->get_vol()->img;
    float* rpl_img = (float*) rgl_vol->get_vol()->img;
    float* ct_img = (float*) ct_vol->get_vol()->img;
    unsigned char* ap_img = 0;
    if (rgl_vol->get_aperture()->have_aperture_image())
    {
        ap_img = (unsigned char*) rgl_vol->get_aperture()->get_aperture_volume()->img;
    }
    plm_long dim[3] = { sigma_vol->get_vol()->dim[0], sigma_vol->get_vol()->dim[1], sigma_vol->get_vol()->dim[2]};

    std::vector<float> sigma_ray (dim[2],0);
    std::vector<float> HU_ray (dim[2],0);
    std::vector<float> range_length_ray (dim[2], 0);

    /* some variables useful for the sigma integration */
    int idx = 0;
    float spacing = sigma_vol->get_vol()->spacing[2]/10; // in cm to correspond to the Highland formula

    float E = energy;
	float mc2 = (float) PROTON_REST_MASS;		/* proton mass at rest (MeV) */
    float c = (float) LIGHT_SPEED;						/* speed of light (m/s) */
    float p = 0.0;								/* Proton momentum (passed in) */
    float v = 0.0;								/* Proton velocity (passed in) */
    float function_to_be_integrated;			/* right term to be integrated in the Highland equation */
    float inverse_rad_length_integrated = 0;	/* and left part */

    float sum = 0.0;		                      /* integration expressions, right part of equation */
    float POI_depth = 0.0;                          /* depth of the point of interest (where is calculated the sigma value)in cm - centered at the pixel center */
    float pixel_depth = 0.0;                        /* depth of the contributing pixel to total sigma (in cm) - center between 2 pixels, the difference in rglength comes from the center of the previous pixel to the center of this pixel */
    float step = 0.0;                               /* step of integration, will depends on the radiologic length */

    /* initializiation of all the rays on which the integration will be done */
    printf ("sigma_img: %d %d %d\n", (int) sigma_vol->get_vol()->dim[0], 
        (int) sigma_vol->get_vol()->dim[1], (int) sigma_vol->get_vol()->dim[2]);
    printf("dim: %d %d %d\n", (int) dim[0], (int) dim[1], (int) dim[2]);
	
    for (int apert_idx = 0; apert_idx < dim[0] * dim[1]; apert_idx++)
    {   
        if (!rgl_vol->get_aperture()->have_aperture_image() || (rgl_vol->get_aperture()->have_aperture_image() && ap_img[apert_idx] > 0))
        {
            int first_non_null_loc = 0;
            for (int s = 0; s < dim[2]; s++)
            {
                idx = dim[0]*dim[1]*s + apert_idx;
                range_length_ray[s] = rpl_img[idx];   // at this point sigma is still a range_length volume without range compensator
                sigma_ray[s] = 0;
                HU_ray[s] = ct_img[idx];           // the density ray is initialized with density
            }
    
            //Now we can compute the sigma rays!!!!
            /* Step 1: the sigma is filled with zeros, so we let them = 0 as long as rg_length is 0, meaning the ray is out of the physical volume */
            /* we mark the first pixel in the volume, and the calculations will start with this one */
      
            for (int s = 0; s < dim[2]; s++)
            {
                if (range_length_ray[s] > 0)
                {
                    first_non_null_loc = s;
                    break;
                }
                if (s == dim[2]-1)
                {
                    first_non_null_loc = dim[2]-1;
                }
            }
    
            /* Step 2: Each pixel in the volume will receive its sigma (in reality y0) value, according to the differential Highland formula */
  
            std::vector<double> pv_cache (dim[2], 0);
            std::vector<double> inv_rad_len (dim[2], 0);
            std::vector<double> stop_cache (dim[2], 0);

            E = energy; // we set the energy of the particles to the nominal energy for this ray

            for (int s = first_non_null_loc; s < dim[2]; s++)
            {
                p = sqrt(2*E*mc2+E*E)/c; // in MeV.s.m-1
                v = c*sqrt(1-pow((mc2/(E+mc2)),2)); //in m.s-1
                pv_cache[s] = p * v;

                inv_rad_len[s] = 1.0f / compute_X0_from_HU(HU_ray[s]);
                stop_cache[s] = compute_PrSTPR_from_HU(HU_ray[s]) * get_proton_stop(E); // dE/dx_mat = dE /dx_watter * STPR (lut in g/cm2)

                sum = 0;
                inverse_rad_length_integrated = 0;

                POI_depth = (float) (s+0.5)*spacing;

                E = energy;
                /*integration */
                for (int t = first_non_null_loc; t <= s && E > 0.1;t++) // 0.1 cut off energy, if set to 0, the very small energies create a  singularity and a giant/wrong sigma
                {
                    if (t == s)
                    {
                        pixel_depth = (t+.25f)*spacing; // we integrate only up to the voxel center, not the whole pixel
                        step = spacing/2;
                    }
                    else
                    {
                        pixel_depth = (t+0.5f)*spacing;
                        step = spacing;
                    }
              
                    function_to_be_integrated = pow(((POI_depth - pixel_depth)/pv_cache[t]),2) * inv_rad_len[t]; //i in cm
                    sum += function_to_be_integrated*step;
                    inverse_rad_length_integrated += step * inv_rad_len[t];
  
                    /* energy is updated after passing through dz */
                    E = E - step * stop_cache[t];
                }
        
                // We have reached the POI pixel and we can store the y0 value
                sigma_ray[s] = 141.0f *(1.0f+1.0f/9.0f*log10(inverse_rad_length_integrated))* (float) sqrt(sum); // in mm
				
                if (E < 0.25) // sigma formula is not defined anymore
                {
                    break;
                }
            }

            /* We fill the rest of the sigma_ray with the max_sigma value to avoid to gather the dose on the axis with a sigma = 0 after the range */
            /* And we copy the temporary sigma_ray in the sigma volume for each ray */
            for (int s = 0; s < dim[2]; s++)
            {
                if (s != 0)
                {
                    if (sigma_ray[s] < sigma_ray[s-1])
                    {
                        sigma_ray[s] = sigma_ray[s-1];
                    }
                }
            
                /* We update the sigma_max value */
                if (sigma_ray[s] > sigma_max)
                {
                    sigma_max = sigma_ray[s];
                }
                sigma_img[dim[0]*dim[1]*s + apert_idx] = sigma_ray[s] * sigma_ray[s]; // We want to have sigma^2 to make a quadratic sum of the sigmas            
            }
        }
    }
    return sigma_max;
}

void compute_sigma_source (Rpl_volume* sigma_vol, Rpl_volume* rpl_volume, Rt_plan* plan, const Rt_beam *beam, float energy)
{
    /* Method of the Hong's algorithm - See Hong's paper */
    float* sigma_img = (float*) sigma_vol->get_vol()->img;
    float* rgl_img = (float*) rpl_volume->get_vol()->img;
    unsigned char* ap_img = (unsigned char*) beam->get_aperture()->get_aperture_volume()->img;

    int idx = 0;
    double proj = 0; // projection factor of the ray on the z axis
    double dist_cp = 0; // distance cp - source for the ray
    float sigma_max = 0;
    float sigma = 0;

    /* MD Fix: Why plan->ap->nrm is incorrect at this point??? */
    double nrm[3] = {0,0,0};
    vec3_sub3(nrm, beam->get_source_position(), beam->get_isocenter_position());
    vec3_normalize1(nrm);

    plm_long dim[3] = { sigma_vol->get_vol()->dim[0], sigma_vol->get_vol()->dim[1], sigma_vol->get_vol()->dim[2]};
    float range = get_proton_range(energy);

    for (int i = 0; i < dim[0] * dim[1]; i++)
    {
        if (ap_img[i] > 0)
        {
            Ray_data* ray_data = &sigma_vol->get_Ray_data()[i];
            proj = -vec3_dot(ray_data->ray, nrm);
            dist_cp = vec3_dist(ray_data->cp,beam->get_source_position());

            for (int j = 0; j < dim[2]; j++)
            {
                if ( rgl_img[idx] < range +10) // +10 because we calculate sigma_src a little bit farther after the range to be sure (+1 cm)
                {
                    idx = dim[0]*dim[1]*j + i;
                    sigma = beam->get_source_size() * ((dist_cp + proj * (float) j * sigma_vol->get_vol()->spacing[2])/ beam->get_aperture()->get_distance() - 1);
                    sigma_img[idx] += sigma * sigma; // we return the square of sigma_source for the quadratic sum, added to the sigma patient already contained in the sigma volume
                    /* We update sigma_max */
                    if (sigma_max < sigma)
                    {
                        sigma_max = sigma;
                    } 
                }
                else
                {
                    break;
                }
            }
        }
    }
    printf("Sigma source computed - sigma_source_max = %lg mm.\n", sigma_max);
	return;
}

void compute_sigma_range_compensator(Rpl_volume* sigma_vol, Rpl_volume* rpl_volume, Rt_plan* plan, const Rt_beam *beam, float energy, int* margins)
{
	/* There are two methods for computing the beam spread due to a range compensator */
    /* Method1 rc_MC_model:  Hong's algorithm (Highland)- See Hong's paper */
	/* Method2:  model built on Monte Carlo simulations - Paper still unpublished */
    /* The range compensator is supposed to be made of lucite and placed right after the aperture*/
	
    if (energy < 1)
    {
        printf("Sigma range compensator = 0 mm, the energy is too small (<1 MeV).\n");
        return;
    }
    /* Range value in lucite extracted from a fit based on 1-250MeV from the NIST data - ranges in mm */
    double range = 10 * get_proton_range((double) energy);

	/* Theta0 computation */
	double theta0 = 0;

	if (beam->get_rc_MC_model() != 'y')
	{
			theta0 = get_theta0_Highland(range);
	}
	else
	{
			theta0 = get_theta0_MC(energy);
	}

    /* sigma calculation and length computations */
    float* sigma_img = (float*) sigma_vol->get_vol()->img;
    float* rgl_img = (float*) rpl_volume->get_vol()->img;
    float* rc_img = (float*) beam->get_aperture()->get_range_compensator_volume ()->img;

    unsigned char* ap_img = 0;
    if (rpl_volume->get_aperture()->have_aperture_image())
    {
        ap_img = (unsigned char*) beam->get_aperture()->get_aperture_volume()->img;
    }
    plm_long dim[3] = { 
        sigma_vol->get_vol()->dim[0], 
        sigma_vol->get_vol()->dim[1], 
        sigma_vol->get_vol()->dim[2]
    };
    int idx = 0;
    int idx2d_sm = 0;
    int idx2d_lg = 0;
    int ijk[3] = {0,0,0};
    double proj = 0; // projection factor of the ray on the z axis
    double dist_cp = 0; // distance cp - source for the ray

    double rc_over_range;
    double rc_eff;
    double theta_srm;
    double sigma_max = 0;

    float POI;
    float l_eff;
    double sigma;
    double nrm[3] = {0,0,0};
		
	/* MD Fix: Why plan->ap->nrm is incorrect at this point??? */
    vec3_sub3(nrm, beam->get_source_position(), beam->get_isocenter_position());
    vec3_normalize1(nrm);
	
    if (margins[0] == 0 && margins[1] == 0 || beam->get_flavor() != 'h')
    {
        for (int i = 0; i < dim[0] * dim[1]; i++)
        {
            /* calculation of sigma_srm, see graph A3 from the Hong's paper */
			if (!rpl_volume->get_aperture()->have_aperture_image() || (ap_img && ap_img[i] > 0))
            {
                Ray_data* ray_data = &sigma_vol->get_Ray_data()[i];
	        
                proj = -vec3_dot(ray_data->ray, nrm);
				if (proj == 0) 
				{ 
					printf("error: some rays are perpendicular to the beam axis \n");
					return;
				}

                dist_cp = vec3_dist(ray_data->cp, beam->get_source_position());
                rc_over_range =  rc_img[i] / proj * PMMA_DENSITY * PMMA_STPR / range; // energy is >1, so range > 0 (range is in water: rho * WER)
	
                if (rc_over_range < 1)
                {
					if (beam->get_rc_MC_model() != 'y')
					{
						theta_srm =  theta0 * get_theta_rel_Highland(rc_over_range);
						rc_eff = get_scat_or_Highland(rc_over_range) * rc_img[i];
					}
					else
					{
						theta_srm =  theta0 * get_theta_rel_MC(rc_over_range);
						rc_eff = get_scat_or_MC(rc_over_range) * rc_img[i];
					}
	
                    for (int j = 0; j < dim[2]; j++)
                    {
                        idx = dim[0]*dim[1]*j + i;
                        if ( rgl_img[idx] < range +10) // +10 because we calculate sigma_rg_compensator a little bit farther after the range to be sure (+1 cm)
                        {
                            POI = dist_cp + (float) j * sigma_vol->get_vol()->spacing[2] - beam->get_aperture()->get_distance() / proj;
                            l_eff = rc_eff * proj;
	            
                            /* sigma = sigma_srm * (z_POI + z_eff) */
                            if (POI + l_eff >= 0)
                            {
                                sigma = theta_srm * (POI + l_eff);
                            }
                            else
                            {
                                sigma = 0;
                                printf("Warning: the image volume intersect the range compensator - in this area the sigma_range compensator will be null.\n");
                            }
	               
                            sigma_img[idx] += sigma * sigma; // we return the square of sigma_source for the quadratic sum, added to the sigma patient already contained in the sigma volume

                            /* We update sigma_max */
                            if (sigma_max < sigma)
                            {
                                sigma_max = sigma;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    else // if margins != 0 we deal with a large volume containing margins
    {
        for(int j = margins[1]; j < dim[1] - margins[1]; j++)
        {
            ijk[1] = j - margins[1];
            for (int i = margins[0]; i < dim[0] - margins[0]; i++)
            {
                ijk[0] = i - margins[0];
                idx2d_sm = ijk[1] * (dim[0] - 2 * margins[0]) + ijk[0];
                idx2d_lg = j * dim[0] + i;

                /* calculation of sigma_srm, see graph A3 from the Hong's paper */
                if (!rpl_volume->get_aperture()->have_aperture_image() || (rpl_volume->get_aperture()->have_aperture_image() && ap_img[idx2d_sm] > 0))
                {
					Ray_data* ray_data = &sigma_vol->get_Ray_data()[idx2d_lg];
	        
					proj = -vec3_dot(ray_data->ray, nrm);

					if (proj == 0) 
					{ 
						printf("error: some rays are perpendicular to the beam axis \n");
						return;
					}

					dist_cp = vec3_dist(ray_data->cp, beam->get_source_position());
                    rc_over_range = rc_img[idx2d_sm] / proj * PMMA_DENSITY * PMMA_STPR / range; // energy is >1, so range > 0

                    if (rc_over_range < 1)
                    {
						if (beam->get_rc_MC_model() != 'y')
						{
							theta_srm =  theta0 * get_theta_rel_Highland(rc_over_range);
							rc_eff = get_scat_or_Highland(rc_over_range) * rc_img[idx2d_sm];
						}
						else
						{
							theta_srm =  theta0 * get_theta_rel_MC(rc_over_range);
							rc_eff = get_scat_or_MC(rc_over_range) * rc_img[idx2d_sm];
						}

                        for (int k = 0; k < dim[2]; k++)
                        {
                            idx = dim[0]*dim[1]*k + idx2d_lg;
                            if ( (rgl_img[idx] + rc_img[idx2d_sm]) < range +10) // +10 because we calculate sigma_rg_compensator a little bit farther after the range to be sure (+1 cm)
                            {
                                POI = dist_cp + (float) k * sigma_vol->get_vol()->spacing[2] - beam->get_aperture()->get_distance() / proj;
                                l_eff =  rc_eff * proj;
		            
                                /* sigma = sigma_srm * (z_POI- z_eff) */
                                if (POI + l_eff >= 0)
                                {
                                    sigma = theta_srm * (POI - l_eff);
                                }
                                else
                                {
                                    sigma = 0;
                                    printf("Warning: the image volume intersect the range compensator - in this area the sigma_range compensator will be null.\n");
                                }
								
                                sigma_img[idx] += sigma * sigma; // we return the square of sigma_source for the quadratic sum, added to the sigma patient already contained in the sigma volume

                                /* We update sigma_max */
                                if (sigma_max < sigma)
                                {
                                    sigma_max = sigma;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }	
    printf("Sigma range compensator computed - sigma_rc_max = %lg mm.\n", sigma_max);
    return;
}
