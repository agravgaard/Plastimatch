/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */

#include "dose_volume_functions.h"
#include "proj_volume.h"
#include "ray_data.h"
#include "rt_lut.h"

void dose_volume_create(Volume* dose_volume, float* sigma_max, Rpl_volume* volume, double range)
{
    /* we want to add extra margins around our volume take into account the dose that will be scattered outside of the rpl_volume */
    /* A 3 sigma margin is applied to the front_back volume, and the size of our volume will be the projection of this shape on the back_clipping_plane */
    
    float ap_ul_pixel[3]; // coordinates in the BEV (rpl_volume) volume
    float proj_pixel[3]; // coordinates of the ap_ul_pixel + 3 sigma margins on the back clipping plane
    float first_pixel[3]; // coordinates of the first_pixel of the volume to be created
    plm_long dim[3] = {0,0,0};
    float origin[3] = {0,0,0};
    float spacing[3] = {0,0,0};
    plm_long npix = 0;
    const float dc[9] = {
        dose_volume->get_direction_cosines()[0], dose_volume->get_direction_cosines()[1], dose_volume->get_direction_cosines()[2], 
        dose_volume->get_direction_cosines()[3], dose_volume->get_direction_cosines()[4], dose_volume->get_direction_cosines()[5], 
        dose_volume->get_direction_cosines()[6], dose_volume->get_direction_cosines()[7], dose_volume->get_direction_cosines()[8]};

    float sigma_margins = 3 * *sigma_max;
    double back_clip_useful = volume->compute_farthest_penetrating_ray_on_nrm(range) +5; // after this the volume will be void, the particules will not go farther + 2mm of margins

    ap_ul_pixel[0] = -volume->get_aperture()->get_center()[0]*volume->get_aperture()->get_spacing()[0];
    ap_ul_pixel[1] = -volume->get_aperture()->get_center()[1]*volume->get_aperture()->get_spacing()[1];
    ap_ul_pixel[2] = volume->get_aperture()->get_distance();

    proj_pixel[0] = (ap_ul_pixel[0] - sigma_margins)*(back_clip_useful + volume->get_aperture()->get_distance()) / volume->get_aperture()->get_distance();
    proj_pixel[1] = (ap_ul_pixel[1] - sigma_margins)*(back_clip_useful + volume->get_aperture()->get_distance()) / volume->get_aperture()->get_distance();
    proj_pixel[2] = back_clip_useful + volume->get_aperture()->get_distance();

    /* We build a matrix that starts from the proj_pixel projection on the front_clipping_plane */
    first_pixel[0] = floor(proj_pixel[0]);
    first_pixel[1] = floor(proj_pixel[1]);
    first_pixel[2] = floor(volume->get_front_clipping_plane() +volume->get_aperture()->get_distance());

    for (int i = 0; i < 3; i++)
    {
        origin[i] = first_pixel[i];
        if (i != 2)
        {   
            spacing[i] = 1;
            //spacing[i] = volume->get_aperture()->get_spacing(i); // MD Fix
            dim[i] = (plm_long) (2*abs(first_pixel[i]/spacing[i])+1);
        }
        else
        {
            spacing[i] = 1; //volume->get_proj_volume()->get_step_length();
            dim[i] = (plm_long) ((back_clip_useful - volume->get_front_clipping_plane())/spacing[i] + 1);
        }
    }
    npix = dim[0]*dim[1]*dim[2];
    dose_volume->create(dim, origin, spacing, dc, PT_FLOAT,1);
}

void
calculate_rpl_coordinates_xyz(std::vector<std:: vector<double> >* xyz_coordinates_volume, Rpl_volume* rpl_volume)
{
    double aperture[3] = {0.0,0.0,0.0};
    double entrance[3] = {0.0,0.0,0.0};
    double ray_bev[3] = {0.0,0.0,0.0};
    double vec_antibug_prt[3] = {0.0,0.0,0.0};

    int dim[3] = {rpl_volume->get_vol()->dim[0],rpl_volume->get_vol()->dim[1],rpl_volume->get_vol()->dim[2]};
    int idx2d = 0;   
    int idx3d = 0;

    for (int i = 0; i < rpl_volume->get_vol()->dim[0];i++){
        for (int j = 0; j < rpl_volume->get_vol()->dim[1];j++){
        
            idx2d = j * dim[0] + i;
            Ray_data* ray_data = &rpl_volume->get_Ray_data()[idx2d];

            vec3_cross(vec_antibug_prt, rpl_volume->get_aperture()->pdn, rpl_volume->get_proj_volume()->get_nrm());
            ray_bev[0] = vec3_dot(ray_data->ray, vec_antibug_prt);
            ray_bev[1] = vec3_dot(ray_data->ray, rpl_volume->get_aperture()->pdn);
            ray_bev[2] = -vec3_dot(ray_data->ray, rpl_volume->get_proj_volume()->get_nrm()); // ray_beam_eye_view is already normalized

            find_xyz_center(aperture, ray_bev, rpl_volume->get_aperture()->get_distance(),0, rpl_volume->get_vol()->spacing[2]);
            find_xyz_center_entrance(entrance, ray_bev, rpl_volume->get_front_clipping_plane());

            vec3_add2(entrance, aperture);

            for (int k = 0; k < rpl_volume->get_vol()->dim[2]; k++){
                idx3d = k*dim[0]*dim[1] + idx2d;
                for (int l = 0; l < 3; l++)
                {
                    (*xyz_coordinates_volume)[idx3d][l] = entrance[l] + (double) k * ray_bev[l];
                }
            }
        }
    }
}

void 
dose_volume_reconstruction (
    Rpl_volume* rpl_dose_vol, 
    Volume::Pointer dose_vol
)
{
    /* scan through patient CT Volume */
    plm_long ct_ijk[3];
    double ct_xyz[4];
    plm_long idx = 0;
    double dose = 0;

    float* dose_img = (float*) dose_vol->img;

    for (ct_ijk[2] = 0; ct_ijk[2] < dose_vol->dim[2]; ct_ijk[2]++) {
        for (ct_ijk[1] = 0; ct_ijk[1] < dose_vol->dim[1]; ct_ijk[1]++) {
            for (ct_ijk[0] = 0; ct_ijk[0] < dose_vol->dim[0]; ct_ijk[0]++) {
                dose = 0.0;

                /* Transform vol index into space coords */
                ct_xyz[0] = (double) (dose_vol->origin[0] + ct_ijk[0] * dose_vol->spacing[0]);
                ct_xyz[1] = (double) (dose_vol->origin[1] + ct_ijk[1] * dose_vol->spacing[1]);
                ct_xyz[2] = (double) (dose_vol->origin[2] + ct_ijk[2] * dose_vol->spacing[2]);
                ct_xyz[3] = (double) 1.0;
                idx = volume_index (dose_vol->dim, ct_ijk);
                dose = rpl_dose_vol->get_rgdepth(ct_xyz);

                if (dose <= 0) {
                    continue;
                }

                /* Insert the dose into the dose volume */
                dose_img[idx] += dose;
            }
        }
    }
}

void build_hong_grid(std::vector<double>* area, std::vector<double>* xy_grid, int radius_sample, int theta_sample)
{
    double dr = 1.0 / (double) radius_sample;
    double dt = 2.0 * M_PI / (double) theta_sample;

    for (int i = 0; i < radius_sample; i++)
    {
        (*area)[i] = M_PI * dr * dr * ( 2 * i + 1 ) / (double) theta_sample; // [(i+1)^2 - i^2] * dr^2

        for (int j = 0; j < theta_sample; j++)
        {
            (*xy_grid)[2*(i*theta_sample+j)] = ((double) i + 0.5)* dr * sin ((double) j * dt);
            (*xy_grid)[2*(i*theta_sample+j)+1] = ((double) i + 0.5) * dr * cos ((double) j * dt);
        }
    }	
}

void 
find_ijk_pixel(int* ijk_idx, double* xyz_ray_center, Volume* dose_volume)
{
    ijk_idx[0] = (int) floor((xyz_ray_center[0] - dose_volume->origin[0]) / dose_volume->spacing[0] + 0.5);
    ijk_idx[1] = (int) floor((xyz_ray_center[1] - dose_volume->origin[1]) / dose_volume->spacing[1] + 0.5);
    ijk_idx[2] = (int) floor((xyz_ray_center[2] - dose_volume->origin[2]) / dose_volume->spacing[2] + 0.5);
}

void 
find_ijk_pixel(int* ijk_idx, double* xyz_ray_center, Volume::Pointer dose_volume)
{
    ijk_idx[0] = (int) floor((xyz_ray_center[0] - dose_volume->origin[0]) / dose_volume->spacing[0] + 0.5);
    ijk_idx[1] = (int) floor((xyz_ray_center[1] - dose_volume->origin[1]) / dose_volume->spacing[1] + 0.5);
    ijk_idx[2] = (int) floor((xyz_ray_center[2] - dose_volume->origin[2]) / dose_volume->spacing[2] + 0.5);
}

void 
find_xyz_center_entrance(double* xyz_ray_center, double* ray, float z_axis_offset)
{
    xyz_ray_center[0] = z_axis_offset * ray[0];
    xyz_ray_center[1] = z_axis_offset * ray[1];
    xyz_ray_center[2] = z_axis_offset * ray[2];
}

void 
find_xyz_center(double* xyz_ray_center, double* ray, float z_axis_offset, int k, float z_spacing)
{
    float alpha = 0.0f;

    xyz_ray_center[2] = z_axis_offset+(double)k * z_spacing;
    alpha = xyz_ray_center[2] /(double) ray[2];
    xyz_ray_center[0] = alpha * ray[0];
    xyz_ray_center[1] = alpha * ray[1];

}

void 
find_xyz_from_ijk(double* xyz, Volume* volume, int* ijk)
{
    xyz[0] = volume->origin[0] + ijk[0]*volume->spacing[0];
    xyz[1] = volume->origin[1] + ijk[1]*volume->spacing[1];
    xyz[2] = volume->origin[2] + ijk[2]*volume->spacing[2];
}

double erf_gauss(double x)
{
    int sign = 1;
    if (x < 0) {sign = -1;}
    x = fabs(x);

    /* erf interpolation */
    double t = 1.0/(1.0 + ERF_P*x);
    double y = 1.0 - (((((ERF_A5*t + ERF_A4)*t) + ERF_A3)*t + ERF_A2)*t + ERF_A1)*t*exp(-x*x);
    return sign*y;
}

double double_gaussian_interpolation(double* gaussian_center, double* pixel_center, double sigma, double* spacing)
{
    double x1 = pixel_center[0] - 0.5 * spacing[0];
    double x2 = x1 + spacing[0];
    double y1 = pixel_center[1] - 0.5 * spacing[1];
    double y2 = y1 + spacing[1];

    double z = .25 
        * (erf_gauss((x2-gaussian_center[0])/(sigma*M_SQRT2)) - erf_gauss((x1-gaussian_center[0])/(sigma*M_SQRT2)))
        * (erf_gauss((y2-gaussian_center[1])/(sigma*M_SQRT2)) - erf_gauss((y1-gaussian_center[1])/(sigma*M_SQRT2)));
    return z;
}

double get_off_axis(double radius, double dr, double sigma)
{
    return M_PI / 8.0 * sigma * ( exp(- (radius - dr)*(radius -dr) / (2 * sigma * sigma)) -  exp(- (radius + dr)*(radius + dr) / (2 * sigma * sigma)));
}

/* MD Fix: don't consider any cosines directions */
void dose_normalization_to_dose(Volume::Pointer dose_volume, double dose, Rt_beam* beam)
{
	int idx = 0;
	double norm = 0;
	int ijk_max[3] = {0,0,0};
	float* img = (float*) dose_volume->img;

	for(int i = 0; i < dose_volume->dim[0]; i++)
	{
		for(int j = 0; j < dose_volume->dim[1]; j++)
		{
			for(int k = 0; k < dose_volume->dim[2]; k++)
			{
				idx = i + (dose_volume->dim[0] * (j + dose_volume->dim[1] * k));

				if (img[idx] > norm)
				{
					norm = img[idx];
					ijk_max[0] = i;
					ijk_max[1] = j;
					ijk_max[2] = k;
				}
			}
		}
	}
	if (norm > 0)
	{
		for (int i = 0; i < dose_volume->dim[0] * dose_volume->dim[1] * dose_volume->dim[2]; i++)
		{
			img[i] = img[i] / norm * dose;
		}
    int ap_dim[2] = {beam->get_aperture()->get_dim(0),beam->get_aperture()->get_dim(1)};
    beam->get_mebs()->scale_num_part(dose/norm, ap_dim);

		printf("Raw dose at the maximum (%lg, %lg, %lg) : %lg A.U.\nDose normalized at the maximum to ", dose_volume->origin[0] + ijk_max[0] * dose_volume->spacing[0], dose_volume->origin[1] + ijk_max[1] * dose_volume->spacing[1], dose_volume->origin[2] + ijk_max[2] * dose_volume->spacing[2], norm);
	}
	else
	{
		printf("Dose is null in the entire volume. Please check your input conditions.\n");
	}
}

/* MD Fix: don't consider any cosines directions */
void dose_normalization_to_dose_and_point(Volume::Pointer dose_volume, double dose, const float* rdp_ijk, const float* rdp, Rt_beam* beam)
{
    std::vector<float>& num_part = beam->get_mebs()->get_num_particles();
    double norm = dose_volume->get_ijk_value(rdp_ijk);
    float* img = (float*) dose_volume->img;

    if (norm > 0)
    {
        for (int i = 0; i < dose_volume->dim[0] * dose_volume->dim[1] * dose_volume->dim[2]; i++)
        {
            img[i] = img[i] / norm * dose;
        }
        int ap_dim[2] = {beam->get_aperture()->get_dim(0),beam->get_aperture()->get_dim(1)};
        beam->get_mebs()->scale_num_part(dose/norm, ap_dim);
        printf("Raw dose at the reference dose point (%lg, %lg, %lg) : %lg A.U.\nDose normalized at the reference dose point to ", rdp[0], rdp[1], rdp[2], norm);
    }
    else
    {
        printf("***WARNING***\nDose null at the reference dose point.\nDose normalized to the dose maximum in the volume.\n");
        dose_normalization_to_dose(dose_volume, dose,beam);
    }
}
