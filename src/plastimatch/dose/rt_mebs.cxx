/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmdose_config.h"
#include <fstream>
#include <string>
#include <utility>
#include <vector>
#include <stdio.h>
#include <math.h>

#include "bragg_curve.h"
#include "file_util.h"
#include "path_util.h"
#include "print_and_exit.h"
#include "rt_depth_dose.h"
#include "rt_mebs.h"
#include "string_util.h"

class Rt_mebs_private {
public:
    std::vector<Rt_depth_dose*> depth_dose;

    float* d_lut;               /* depth array (mm) */
    float* e_lut;               /* energy array (MeV) */
    float* f_lut;		/* integrated energy array (MeV) */
    int num_samples;	        /* number of depths */

    float beam_min_energy;
    float beam_max_energy;
    float energy_res;
    int energy_number;

    float target_min_depth;			/* lower depth  of target */
    float target_max_depth;			/* higher depth of target */
    float depth_res;				/* depth resolution */
    float depth_end;				/* end of the depth array */

    float prescription_depth_min;
    float prescription_depth_max;
    float proximal_margin;
    float distal_margin;

    double spread; /* spread for optimized SOBP, may be changed to a vector for each energy */

    /* p  & alpha are parameters that bind depth and energy 
       according to ICRU */
    Particle_type particle_type;			
    double alpha;
    double p;

    float photon_energy; // energy for photon mono-energetic beams

    /* When a new sobp is created from an existing sobp, 
       the peaks are copied (not manual).  Modifications of 
       an implicitly defined sobp (by adding a peak) will 
       delete any existing peaks. */
    bool have_copied_peaks;
    bool have_manual_peaks;
    bool have_prescription;
    bool have_particle_number_map;

    /* passive scattering: weights of the mono-energetic beams */
    std::vector<float> depth_dose_weight;
    std::vector<float> energies;

    /* min and max wed for each ray */
    std::vector<double> min_wed_map;
    std::vector<double> max_wed_map;

    /* particle number map for both beam line systems */
    std::vector<float> num_particles;

    /* particle number file paths */
    std::string particle_number_in;
    std::string particle_number_out;

public:
    Rt_mebs_private ()
    {
        this->d_lut = new float[0];
        this->e_lut = new float[0];
        this->f_lut = new float[0];
        this->num_samples = 0;

        this->beam_min_energy = 0.f;
        this->beam_max_energy = 0.f;
        this->energy_res = 1.f;
        this->energy_number = 1;

        this->target_min_depth = 0.f;			/* lower depth  of target */
        this->target_max_depth = 0.f;			/* higher depth of target */
        this->depth_res = 0.01f;				/* depth resolution */
        this->depth_end = 20.f;

        this->prescription_depth_min = 0.f;
        this->prescription_depth_max = 0.f;
        this->proximal_margin = 0.f;
        this->distal_margin = 0.f;

        this->spread = 1.0;

        this->particle_type = PARTICLE_TYPE_P;
        this->alpha = particle_parameters[0][0];
        this->p = particle_parameters[0][1];

        this->photon_energy = 6.f;

        this->have_copied_peaks = false;
        this->have_manual_peaks = false;
        this->have_prescription = false;
        this->have_particle_number_map = false;

        this->particle_number_in ="";
        this->particle_number_out ="";
    }
    Rt_mebs_private (Particle_type part)
    {
        this->d_lut = new float[0];
        this->e_lut = new float[0];
        this->f_lut = new float[0];
        this->num_samples = 0;

        this->beam_min_energy = 0.f;
        this->beam_max_energy = 0.f;
        this->energy_res = 1.f;
        this->energy_number = 1;

        this->target_min_depth = 0.f;			/* lower depth  of target */
        this->target_max_depth = 0.f;			/* higher depth of target */
        this->depth_res = 0.01f;				/* depth resolution */
        this->depth_end = 20.f;

        this->prescription_depth_min = 0.f;
        this->prescription_depth_max = 0.f;
        this->proximal_margin = 0.f;
        this->distal_margin =0.f;

        this->spread = 1.0;

        this->set_particle_type(part);

        this->photon_energy = 6.f;

        this->have_copied_peaks = false;
        this->have_manual_peaks = false;
        this->have_prescription = false;
        this->have_particle_number_map = false;

        this->particle_number_in ="";
        this->particle_number_out ="";
    }
    Rt_mebs_private (const Rt_mebs_private* rsp)
    {
        this->d_lut = new float[0];
        this->e_lut = new float[0];
        this->f_lut = new float[0];
        this->num_samples = rsp->num_samples;

        this->beam_min_energy = rsp->beam_min_energy;
        this->beam_max_energy = rsp->beam_max_energy;
        this->energy_res = rsp->energy_res;
        this->energy_number = rsp->energy_number;

        this->target_min_depth = rsp->target_min_depth;	/* lower depth  of target */
        this->target_max_depth = rsp->target_max_depth;	/* higher depth of target */
        this->depth_res = rsp->depth_res;				/* depth resolution */
        this->depth_end = rsp->depth_end;

        this->prescription_depth_min = rsp->prescription_depth_min;
        this->prescription_depth_max = rsp->prescription_depth_max;
        this->proximal_margin = rsp->proximal_margin;
        this->distal_margin = rsp->distal_margin;

        this->spread =rsp->spread;

        this->particle_type = rsp->particle_type;
        this->alpha = rsp->alpha;
        this->p = rsp->p;

        this->photon_energy = rsp->photon_energy;

        this->have_copied_peaks = true;
        this->have_manual_peaks = rsp->have_manual_peaks;
        this->have_prescription = rsp->have_prescription;
        this->have_particle_number_map = rsp->have_particle_number_map;

        this->particle_number_in = rsp->particle_number_in;
        this->particle_number_out = rsp->particle_number_out;

        /* copy the associated depth dose and update the dept dose curve */
        for (int i = 0; i < rsp->depth_dose.size(); i++)
        {
            this->depth_dose.push_back(rsp->depth_dose[i]);
        }
        for (int i = 0; i < rsp->depth_dose_weight.size(); i++)
        {
            this->depth_dose_weight.push_back(rsp->depth_dose_weight[i]);
        }
        for (int i = 0; i < rsp->energies.size(); i++)
        {
            this->energies.push_back(rsp->energies[i]);
        }
        for (int i = 0; i < rsp->num_particles.size(); i++)
        {
            this->num_particles.push_back(rsp->num_particles[i]);
        }

        /* update the global depth_dose curve */
        this->d_lut = (float*) malloc (this->num_samples*sizeof(float));
        this->e_lut = (float*) malloc (this->num_samples*sizeof(float));
        this->f_lut = (float*) malloc (this->num_samples*sizeof(float));
    
        memset (this->d_lut, 0, this->num_samples*sizeof(float));
        memset (this->e_lut, 0, this->num_samples*sizeof(float));
        memset (this->f_lut, 0, this->num_samples*sizeof(float));
        for (int i = 0; i < rsp->num_samples; i++)
        {
            this->d_lut[i] = rsp->d_lut[i];
            this->e_lut[i] = rsp->e_lut[i];
            this->f_lut[i] = rsp->f_lut[i];
        }
    }
    ~Rt_mebs_private ()
    {
        if (d_lut) delete[] d_lut;
        if (e_lut) delete[] e_lut;
        if (f_lut) delete[] f_lut;
        clear_depth_dose ();
    }
public:
    void set_particle_type (Particle_type part)
    {
        this->particle_type = part;
        switch (particle_type) {
        case PARTICLE_TYPE_P:
            alpha = particle_parameters[0][0];
            p = particle_parameters[0][1];
            break;
        case PARTICLE_TYPE_HE:
            alpha = particle_parameters[1][0];
            p = particle_parameters[1][1];
            lprintf ("data for helium particle are not available - based on proton beam data");
            break;
        case PARTICLE_TYPE_LI:
            alpha = particle_parameters[2][0];
            p = particle_parameters[2][1];
            lprintf ("data for lithium particle type are not available - based on proton beam data");
            break;
        case PARTICLE_TYPE_BE:
            alpha = particle_parameters[3][0];
            p = particle_parameters[3][1];
            lprintf ("data for berilium particle type are not available - based on proton beam data");
            break;
        case PARTICLE_TYPE_B:
            alpha = particle_parameters[4][0];
            p = particle_parameters[4][1];
            lprintf ("data for bore particle type are not available - based on proton beam data");
            break;
        case PARTICLE_TYPE_C:
            alpha = particle_parameters[5][0];
            p = particle_parameters[5][1];
            lprintf ("data for carbon particle type are not available - based on proton beam data");
            break;
        case PARTICLE_TYPE_O:
            alpha = particle_parameters[7][0];
            p = particle_parameters[7][1];
            lprintf ("data for oxygen particle type are not available - based on proton beam data");
            break;
        default:
            alpha = particle_parameters[0][0];
            p = particle_parameters[0][1];
            particle_type = PARTICLE_TYPE_P;
            lprintf ("particle not found - proton beam chosen");
            break;
        }
    }
    void clear_depth_dose ()
    {
        if (depth_dose.size() > 0)
        {
            printf("Mono energetic beamlet set is erased.\n");
        }
        int stop = depth_dose.size();
        /* std::vector<Rt_depth_dose*>::iterator it;
           for (it = depth_dose.begin(); it != depth_dose.end(); ++it) {
           delete *it;
           } */
        depth_dose.clear();
        /* stop = depth_dose.size();
           for (int i = 0; i < stop; i++)
           {   
           depth_dose.pop_back();
           } */
        stop = depth_dose_weight.size();
        for (int i = 0; i < stop; i++)
        {
            depth_dose_weight.pop_back();
        }
        stop = energies.size();
        for (int i = 0; i < stop; i++)
        {
            energies.pop_back();
        }
        stop = num_particles.size();
        for (int i = 0; i < stop; i++)
        {
            num_particles.pop_back();
        }
    }
	
};

Rt_mebs::Rt_mebs ()
{
    d_ptr = new Rt_mebs_private;
}

Rt_mebs::Rt_mebs (Particle_type part)
{
    d_ptr = new Rt_mebs_private(part);
}

Rt_mebs::Rt_mebs (const Rt_mebs::Pointer& rt_mebs)
{
    d_ptr = new Rt_mebs_private (rt_mebs->d_ptr);
}

Rt_mebs::~Rt_mebs ()
{
    delete d_ptr;
}

void
Rt_mebs::clear_depth_dose ()
{
    d_ptr->clear_depth_dose ();
}

void
Rt_mebs::add_depth_dose (Rt_depth_dose* depth_dose)
{
    if (d_ptr->have_copied_peaks == true) {
        d_ptr->clear_depth_dose ();
        d_ptr->have_copied_peaks = false;
    }
    if (depth_dose->dres != d_ptr->depth_res) {
        printf("*** ERROR: the depth dose added must have the same resolution than the depth_dose set.\n");
        printf("depth dose set - resolution: dres = %lf.\n", d_ptr->depth_res);
        printf("depth dose to be added - resolution: dres = %lf.\n", depth_dose->dres);
        return;
    }
    d_ptr->depth_dose.push_back (depth_dose);
    d_ptr->energy_number = d_ptr->depth_dose.size();
    d_ptr->depth_dose_weight.push_back(1);
    d_ptr->energies.push_back(depth_dose->E0);

    /* update the mebs depth dose length if this one is longer */
    if (depth_dose->num_samples > d_ptr->num_samples)
    {
        d_ptr->num_samples = depth_dose->num_samples;
    }
}

void
Rt_mebs::add_peak (double E0, double spread, double weight)
{
    if (d_ptr->have_copied_peaks == true) {
        d_ptr->clear_depth_dose ();
        d_ptr->have_copied_peaks = false;
    }

    switch(d_ptr->particle_type)
    {
    case PARTICLE_TYPE_P:			// proton
    {
        Rt_depth_dose *depth_dose = new Rt_depth_dose (
            E0, spread, d_ptr->depth_res, d_ptr->depth_end);
        if (depth_dose->dend > d_ptr->depth_end)
        {
            d_ptr->depth_end = depth_dose->dend;
        }
        printf ("Adding peak to sobp (%f, %f, %f) [%f, %f]\n", 
            (float) E0, (float) spread, (float) weight,
            d_ptr->depth_res, d_ptr->depth_end);
        d_ptr->depth_dose.push_back (depth_dose);
        d_ptr->energy_number = d_ptr->depth_dose.size();
        d_ptr->depth_dose_weight.push_back((float) weight);
        d_ptr->energies.push_back(E0);

        /* update the mebs depth dose length if this one is longer */
        if (depth_dose->num_samples > d_ptr->num_samples)
        {
            d_ptr->num_samples = depth_dose->num_samples;
        }
    }
    case PARTICLE_TYPE_HE:			// helium
    {
        //to be implemented
    }
    break;
    case PARTICLE_TYPE_LI:			// lithium
    {
        //to be implemented
    }
    break;
    case PARTICLE_TYPE_BE:			// berilium
    {
        //to be implemented
    }
    break;
    case PARTICLE_TYPE_B:			// bore
    {
        //to be implemented
    }
    break;
    case PARTICLE_TYPE_C:			// carbon
    {
        //to be implemented
    }
    break;
    case PARTICLE_TYPE_N:			// nitrogen
    {
        //to be implemented
    }
    break;
    case PARTICLE_TYPE_O:			// oxygen
    {
        //to be implemented
    }
    break;
    default:
    {
        //to be implemented
    }
    }
}

float
Rt_mebs::lookup_energy (
    float depth)
{	
    int i = 0;
    float energy = 0.0f;

    /* Sanity check */
    if (depth < 0 || depth > d_ptr->depth_end) {
        return 0.0f;
    }

    /* Find index into profile arrays */
    for (i = (int) floor(depth / d_ptr->depth_res); i < d_ptr->num_samples-1; i++) {
        if (d_ptr->d_lut[i] > depth) {
            i--;
            break;
        }
    }

    /* Clip input depth to maximum in lookup table */
    if (i == d_ptr->num_samples-1) {
        depth = d_ptr->d_lut[i];
    }
	
    /* Use index to lookup and interpolate energy */
    if (i >= 0 || i < d_ptr->num_samples-1) {
        // linear interpolation
        energy = d_ptr->e_lut[i]
            + (depth - d_ptr->d_lut[i])
            * ((d_ptr->e_lut[i+1] - d_ptr->e_lut[i]) 
                / (d_ptr->d_lut[i+1] - d_ptr->d_lut[i]));
    } else {
        // we wen't past the end of the lookup table
        energy = 0.0f;
    }
    return energy;
}

bool
Rt_mebs::generate ()
{
    printf("depth_dose number %d\n", (int)d_ptr->depth_dose.size());

    /* Construct the data structure first time through */
    if (d_ptr->d_lut) delete[] d_ptr->d_lut;    
    if (d_ptr->e_lut) delete[] d_ptr->e_lut;
    if (d_ptr->f_lut) delete[] d_ptr->f_lut;
    d_ptr->e_lut = new float [d_ptr->num_samples];
    d_ptr->f_lut = new float [d_ptr->num_samples];
    d_ptr->d_lut = new float [d_ptr->num_samples];

    for (int i = 0; i < d_ptr->num_samples; i++) {
        d_ptr->d_lut[i] = (float) i * d_ptr->depth_res;
        d_ptr->e_lut[i] = 0;
        d_ptr->f_lut[i] = 0;
    }

    for (int it = 0; it < d_ptr->depth_dose.size(); it++)
    {
        const Rt_depth_dose *ppp = d_ptr->depth_dose[it];
        /* Check that this peak has the same resolution */
        if (ppp->dres != d_ptr->depth_res) {
            print_and_exit ("Error, mismatch in resolution.\n MEBS: %lg, depth dose # %d: %lg.\n", d_ptr->depth_res, it, ppp->dres);
        }
        if (ppp->num_samples > d_ptr->num_samples) {
            print_and_exit ("Error, num_samples MEBS > num_sample depth dose.\n MEBS: %d, depth dose # %d: %d.\n", d_ptr->num_samples, it, ppp->num_samples);
        }

        /* Add weighted pristine peak to mebs */
        for (int i = 0; i < ppp->num_samples; i++) {
            d_ptr->e_lut[i] += d_ptr->depth_dose_weight[it] * ppp->e_lut[i];
        }

        /* Go on to next pristine peak */
        it++;
    }
	
    /* build the integrated dose */
    if (d_ptr->f_lut[0] && d_ptr->f_lut[0]) {d_ptr->f_lut[0] = d_ptr->e_lut[0];}
    for (int i = 1; i < d_ptr->num_samples; i++) {
        d_ptr->f_lut[i] = d_ptr->f_lut[i-1] + d_ptr->e_lut[i];
    }
    return true;
}

void
Rt_mebs::dump (const char* dir)
{
    std::string dirname = dir;

    /* Dump SOBP */
    std::string sobp_fn = string_format ("%s/bragg_curve.txt", dir);
    FILE* fp = fopen (sobp_fn.c_str(), "w");
    for (int i=0; i < d_ptr->num_samples; i++) {
        fprintf (fp, "%3.2f %3.2f\n", d_ptr->d_lut[i], d_ptr->e_lut[i]);
    }
    fclose (fp);

    /* Dump pristine peaks */
    std::vector<Rt_depth_dose*>::const_iterator it 
        = d_ptr->depth_dose.begin();
    while (it != d_ptr->depth_dose.end ()) {
        std::string fn = string_format ("%s/pristine_%4.2f.txt", dir, 
            (float) (*it)->E0);
        (*it)->dump (fn.c_str());
        it++;
    }
}

// return on the command line the parameters of the sobp to be build
void 
Rt_mebs::printparameters()
{
    printf ("\nParticle type : %s, alpha: %lg, p: %lg\n", particle_type_string (d_ptr->particle_type), d_ptr->alpha, d_ptr->p);
    printf("Number of depth_dose : %d\n",d_ptr->energy_number = d_ptr->depth_dose.size());
    printf("Energy set (in MeV):\n");
    for (int i = 0; i < d_ptr->energies.size(); i++)
    {
        printf("%lg ", d_ptr->energies[i]);
    }
    printf("\nweights set:\n");
    for (int i = 0; i < d_ptr->depth_dose_weight.size(); i++)
    {
        printf("%lg ", d_ptr->depth_dose_weight[i]);
    }
    printf("\nEnegy resolution: %g MeV \n",d_ptr->energy_res);
    printf("E_min : %g MeV; E_max : %g MeV\n",d_ptr->beam_min_energy, d_ptr->beam_max_energy);
    printf("num_samples: %d\n", d_ptr->num_samples);
    printf("depth_min_target : %3.2f mm\n",d_ptr->target_min_depth);
    printf("depth_max_target : %3.2f mm\n",d_ptr->target_max_depth);
    printf("depth_resolution : %3.2f mm \n",d_ptr->depth_res);
    printf("depth_end : %3.2f mm\n",d_ptr->depth_end);
    printf("prescription depths: proximal: %lg mm, distal: %lg mm\n",d_ptr->prescription_depth_min, d_ptr->prescription_depth_max);
    printf("margins: proximal: %lg mm, distal: %lg mm\n", d_ptr->proximal_margin, d_ptr->distal_margin);
}

/* reset the mebs depth dose curve */
void
Rt_mebs::reset_mebs_depth_dose_curve()
{
    if (d_ptr->d_lut) delete[] d_ptr->d_lut;
    d_ptr->d_lut = new float[d_ptr->num_samples];
    if (d_ptr->e_lut) delete[] d_ptr->e_lut;
    d_ptr->e_lut = new float[d_ptr->num_samples];
    if (d_ptr->f_lut) delete[] d_ptr->f_lut;
    d_ptr->f_lut = new float[d_ptr->num_samples];

    for (int i = 0; i < d_ptr->num_samples; i++)
    {
        d_ptr->d_lut[i] = (float) i * d_ptr->depth_res;
        d_ptr->e_lut[i] = 0;
        d_ptr->f_lut[i] = 0;
    }
}


/* set the mebs parameters by introducing the min and max energies */
void 
Rt_mebs::set_energies(float new_E_min, float new_E_max) 
{
    if (new_E_max <= 0 || new_E_min <= 0)
    {
        printf("The energies min and max of the Sobp must be positive!\n");
        printf("Emin = %g, Emax = %g \n", new_E_min, new_E_max);
        return;
    }
	
    if (new_E_max <= new_E_min)
    {
        printf("SOBP: The Energy_max must be superior to the Energy_min.Energies unchanged.\n");
        printf("Emin = %g, Emax = %g \n", new_E_min, new_E_max);
        return;
    }

    d_ptr->beam_min_energy = new_E_min;
    d_ptr->beam_max_energy = new_E_max;
    this->update_prescription_depths_from_energies();
}

void 
Rt_mebs::set_energies(float new_E_min, float new_E_max, float new_step) 
{
   d_ptr->energy_res = new_step;
   this->set_energies(new_E_min, new_E_max);
}

/* set the mebs parameters by introducing the min and max energies */
void 
Rt_mebs::set_target_depths(float new_depth_min, float new_depth_max) 
{
    if (new_depth_max <= 0 || new_depth_min <= 0)
    {
        printf("***ERROR*** The depth min and max of the target must be positive!\n");
        printf("depths min = %g, max = %g \n", new_depth_min, new_depth_max);
        return;
    }
	
    if (new_depth_max <= new_depth_min)
    {
        printf("***ERROR*** The Energy_max must be superior to the Energy_min.Energies unchanged.\n");
        printf("Emin = %g, Emax = %g \n", new_depth_min, new_depth_max);
        return;
    }

    if (new_depth_min - d_ptr->proximal_margin < 0)
    {
        printf("***ERROR*** The proximal margins are too big: depth - margins < 0.\n");
        printf("target_depth: %lg mm, proximal margin: %lg mm.\n", new_depth_min, d_ptr->proximal_margin);
        return;
    }

    d_ptr->target_min_depth = new_depth_min;
    d_ptr->target_max_depth = new_depth_max;
    d_ptr->prescription_depth_min = new_depth_min - d_ptr->proximal_margin;
    d_ptr->prescription_depth_max = new_depth_max + d_ptr->distal_margin;
    d_ptr->depth_end = d_ptr->prescription_depth_max + 20;
    this->update_energies_from_prescription();
}

void 
Rt_mebs::set_target_depths(float new_z_min, float new_z_max, float new_step)
{
	d_ptr->depth_res = new_step;
	this->set_target_depths(new_z_min, new_z_max);
}

void 
Rt_mebs::set_prescription_depths(float new_prescription_min, float new_prescription_max)
{
    if (new_prescription_min <= d_ptr->proximal_margin || new_prescription_max <= 0)
    {
        printf("***ERROR*** The prescription min - proximal margins and prescription max must be positive!\n");
        printf("prescription min = %g, max = %g \n", new_prescription_min, new_prescription_max);
        printf("proximal margin = %g mm.\n", d_ptr->proximal_margin);
        return;
    }
    if (new_prescription_max <= new_prescription_min)
    {
        printf("***ERROR*** The prescription max must be superior to the prescription min.\n");
        printf("prescription min = %g,  prescription max = %g \n", new_prescription_min, new_prescription_max);
        return;
    }
    if (new_prescription_min + d_ptr->proximal_margin > new_prescription_max - d_ptr->distal_margin)
    {
        printf("***WARNING*** prox_margin + distal margin > prescription_max - prescription min.\n");
        printf("proximal margin: %lg mm, distal margin: %lg mm.\n", d_ptr->proximal_margin, d_ptr->distal_margin);
        printf("prescription min: %lg mm, prescription max: %lg mm.\n", new_prescription_min, new_prescription_max);
        return;
    }
    d_ptr->prescription_depth_min = new_prescription_min;
    d_ptr->prescription_depth_max = new_prescription_max;
    d_ptr->depth_end = d_ptr->prescription_depth_max + 20;
    this->update_energies_from_prescription();
}

void 
Rt_mebs::set_margins(float proximal_margin, float distal_margin)
{
    if (proximal_margin < 0 || distal_margin < 0)
    {
        printf("***ERROR*** The margins must be positive or null!\n");
        printf("prescription min = %g, max = %g \n", proximal_margin, distal_margin);
        return;
    }
    d_ptr->proximal_margin = proximal_margin;
    d_ptr->distal_margin = distal_margin;
    // the prescription depths are updated later
}

/* update the mebs depths parameters from energy definition */
void 
Rt_mebs::update_prescription_depths_from_energies() 
{
    d_ptr->prescription_depth_min = ((10*d_ptr->alpha)*pow((double)d_ptr->beam_min_energy, d_ptr->p));
    d_ptr->prescription_depth_max = ((10*d_ptr->alpha)*pow((double)d_ptr->beam_max_energy, d_ptr->p));

    d_ptr->target_min_depth = d_ptr->prescription_depth_min + d_ptr->proximal_margin;
    d_ptr->target_max_depth = d_ptr->prescription_depth_max - d_ptr->distal_margin;
    if (d_ptr->target_min_depth > d_ptr->target_max_depth)
    {
        printf("***WARNING*** target volume impossible. The difference between the E_min and E_max is smaller than the sum of the margins.\n");
    }
    d_ptr->depth_end = d_ptr->prescription_depth_max + 20;
    d_ptr->num_samples = (int)ceil((d_ptr->depth_end/d_ptr->depth_res))+1;
    d_ptr->energy_number = (int) ceil((d_ptr->beam_max_energy - d_ptr->beam_min_energy) / d_ptr->energy_res) + 1;
    this->reset_mebs_depth_dose_curve();
}

/* update the mebs energy parameters from prescription definition */
void 
Rt_mebs::update_energies_from_prescription()
{
    int energy_min_index = (int) floor(pow((d_ptr->prescription_depth_min/(10*d_ptr->alpha)),(1/d_ptr->p)) / d_ptr->energy_res);
    int energy_max_index = (int) ceil(pow((d_ptr->prescription_depth_max/(10*d_ptr->alpha)),(1/d_ptr->p)) / d_ptr->energy_res);

    d_ptr->beam_min_energy = (float) energy_min_index * d_ptr->energy_res;
    d_ptr->beam_max_energy = (float) energy_max_index * d_ptr->energy_res;

    /* check that the E_max is sufficiently high for covering the distal part of the prescription */
    d_ptr->beam_max_energy += this->check_and_correct_max_energy(d_ptr->beam_max_energy, d_ptr->prescription_depth_max);
    energy_max_index = (int) (d_ptr->beam_max_energy / d_ptr->energy_res);
    /* check that the E_min is sufficiently low for covering the distal part of the prescription */
    d_ptr->beam_min_energy += this->check_and_correct_min_energy(d_ptr->beam_min_energy, d_ptr->prescription_depth_min);
    energy_min_index = (int) (d_ptr->beam_min_energy / d_ptr->energy_res);

    d_ptr->depth_end = d_ptr->prescription_depth_max + 20;
    d_ptr->num_samples = (int)ceil((d_ptr->depth_end/d_ptr->depth_res))+1;
    d_ptr->energy_number = (int) ceil((d_ptr->beam_max_energy - d_ptr->beam_min_energy) / d_ptr->energy_res) + 1;
    this->reset_mebs_depth_dose_curve();
}

float* 
Rt_mebs::get_d_lut()
{
    return d_ptr->d_lut;
}

float* 
Rt_mebs::get_e_lut()
{
    return d_ptr->e_lut;
}

float* 
Rt_mebs::get_f_lut()
{
    return d_ptr->f_lut;
}

void 
Rt_mebs::set_particle_type(Particle_type particle_type)
{
    d_ptr->set_particle_type (particle_type);
    if (d_ptr->prescription_depth_min !=0) {
        this->update_energies_from_prescription();
    }
}

Particle_type
Rt_mebs::get_particle_type()
{
    return d_ptr->particle_type;
}

void 
Rt_mebs::set_alpha(double alpha)
{
    d_ptr->alpha = alpha;
}

double 
Rt_mebs::get_alpha()
{
    return d_ptr->alpha;
}

void 
Rt_mebs::set_p(double p)
{
    d_ptr->p = p;
}

double 
Rt_mebs::get_p()
{
    return d_ptr->p;
}

int
Rt_mebs::get_energy_number()
{
    return d_ptr->energy_number;
}

std::vector<float> 
Rt_mebs::get_energy()
{
    return d_ptr->energies;
}

std::vector<float> 
Rt_mebs::get_weight()
{
    return d_ptr->depth_dose_weight;
}

void 
Rt_mebs::set_energy_resolution(float eres)
{
    if (eres > 0)
    {
        d_ptr->energy_res = eres;
        d_ptr->energy_number = (int) ceil((d_ptr->beam_max_energy - d_ptr->beam_min_energy) / d_ptr->energy_res) + 1;
    }
    else 
    {
        printf("***WARNING*** Energy resolution must be positive. Energy resolution unchanged");
    }
}

float 
Rt_mebs::get_energy_resolution()
{
    return d_ptr->energy_res;
}

void 
Rt_mebs::set_energy_min(float E_min)
{
    if (E_min > 0)
    {
        this->set_energies(E_min, d_ptr->beam_max_energy);
    }
    else 
    {
        printf("***WARNING*** Energy min must be positive. Energy min unchanged");
    }
}

float 
Rt_mebs::get_energy_min()
{
    return d_ptr->beam_min_energy;
}

void 
Rt_mebs::set_energy_max(float E_max)
{
    if (E_max > 0)
    {
        this->set_energies(d_ptr->beam_min_energy, E_max);
    }
    else
    {
        printf("***WARNING*** Energy max must be positive. Energy max unchanged");
    }
}

float 
Rt_mebs::get_energy_max()
{
    return d_ptr->beam_max_energy;
}

int 
Rt_mebs::get_num_samples()
{
    return d_ptr->num_samples;
}

void 
Rt_mebs::set_target_min_depth(float dmin)
{
    if (dmin > 0)
    {
        this->set_target_depths(dmin, d_ptr->target_max_depth);
    }
    else 
    {
        printf("***WARNING*** Depth min must be positive. Depth min unchanged");
    }
}

float 
Rt_mebs::get_target_min_depth()
{
    return d_ptr->target_min_depth;
}

void 
Rt_mebs::set_target_max_depth(float dmax)
{
    if (dmax > 0)
    {
        this->set_target_depths(d_ptr->target_min_depth, dmax);
    }
    else 
    {
        printf("***WARNING*** Depth max must be positive. Depth max unchanged");
    }
}

float 
Rt_mebs::get_target_max_depth()
{
    return d_ptr->target_max_depth;
}

void 
Rt_mebs::set_depth_resolution(float dres)
{
    if (dres > 0)
    {
        d_ptr->depth_res = dres;
        d_ptr->num_samples = (int)ceil((d_ptr->depth_end/d_ptr->depth_res))+1;
        this->reset_mebs_depth_dose_curve();
    }
    else 
    {
        printf("***WARNING*** Depth resolution must be positive. Depth resolution unchanged");
    }
}

float
Rt_mebs::get_depth_resolution()
{
    return d_ptr->depth_res;
}

void 
Rt_mebs::set_depth_end(float dend)
{
    if (dend > 0)
    {
        d_ptr->depth_end = dend;
        d_ptr->num_samples = (int)ceil((d_ptr->depth_end/d_ptr->depth_res))+1;
        this->reset_mebs_depth_dose_curve();
    }
    else 
    {
        printf("***WARNING*** Depth end must be positive. Depth end unchanged");
    }
}

float
Rt_mebs::get_depth_end()
{
    return d_ptr->depth_end;
}

float 
Rt_mebs::get_prescription_min()
{
    return d_ptr->prescription_depth_min;
}

float
Rt_mebs::get_prescription_max()
{
    return d_ptr->prescription_depth_max;
}

void
Rt_mebs::set_distal_margin (float distal_margin)
{
    this->set_margins(d_ptr->proximal_margin, distal_margin);
}

float 
Rt_mebs::get_distal_margin()
{
    return d_ptr->distal_margin;
}

void
Rt_mebs::set_proximal_margin (float proximal_margin)
{
    this->set_margins(proximal_margin, d_ptr->distal_margin);
}

float 
Rt_mebs::get_proximal_margin()
{
    return d_ptr->proximal_margin;
}

void 
Rt_mebs::set_spread (double spread)
{
    d_ptr->spread = spread;
}

double
Rt_mebs::get_spread()
{
    return d_ptr->spread;
}

void 
Rt_mebs::set_photon_energy(float energy)
{
    d_ptr->photon_energy = energy;
}

float 
Rt_mebs::get_photon_energy()
{
    return d_ptr->photon_energy;
}

std::vector<Rt_depth_dose*> 
Rt_mebs::get_depth_dose()
{
    return d_ptr->depth_dose;
}

std::vector<float>& 
Rt_mebs::get_num_particles()
{
    return d_ptr->num_particles;
}

void
Rt_mebs::set_prescription(float prescription_min, float prescription_max)
{
    d_ptr->have_prescription = true;
    this->set_prescription_depths(prescription_min, prescription_max);
}

void 
Rt_mebs::set_have_prescription(bool have_prescription)
{
    d_ptr->have_prescription = have_prescription;
}

bool 
Rt_mebs::get_have_prescription()
{
    return d_ptr->have_prescription;
}

void 
Rt_mebs::set_have_copied_peaks(bool have_copied_peaks)
{
    d_ptr->have_copied_peaks = have_copied_peaks;
}
	
bool 
Rt_mebs::get_have_copied_peaks()
{
    return d_ptr->have_copied_peaks;
}

void 
Rt_mebs::set_have_manual_peaks(bool have_manual_peaks)
{
    d_ptr->have_manual_peaks = have_manual_peaks;
}

bool 
Rt_mebs::get_have_manual_peaks()
{
    return d_ptr->have_manual_peaks;
}

void 
Rt_mebs::set_have_particle_number_map(bool have_particle_number_map)
{
    d_ptr->have_particle_number_map = have_particle_number_map;
}

bool 
Rt_mebs::get_have_particle_number_map()
{
    return d_ptr->have_particle_number_map;
}

std::vector<double>&
Rt_mebs::get_min_wed_map()
{
    return d_ptr->min_wed_map;
}

std::vector<double>&
Rt_mebs::get_max_wed_map()
{
    return d_ptr->max_wed_map;
}

void 
Rt_mebs::set_particle_number_in (const std::string& str)
{
    d_ptr->particle_number_in = str;
}

std::string 
Rt_mebs::get_particle_number_in ()
{
    return d_ptr->particle_number_in;
}

void 
Rt_mebs::set_particle_number_out (const std::string& str)
{
    d_ptr->particle_number_out = str;
}

std::string 
Rt_mebs::get_particle_number_out ()
{
    return d_ptr->particle_number_out;
}

void 
Rt_mebs::add_depth_dose_weight(float weight)
{
    d_ptr->depth_dose_weight.push_back(weight);
}

/* This function check (and correct if necessary) that E_max is the closest energy (+/- energy_resolution) 
	to reach the distal prescription */
/* This function is designed to return a float value that represents the increase/decrease of energy to correct it
	this is used by two parts of the program on different members (explaining this particular structure) */
float
Rt_mebs::check_and_correct_max_energy(float E, float depth)
{
    float E_init = E;
    /* Check that the depth dose is increasing (dose_max not passed) */
    float dose = bragg_curve(E, d_ptr->spread, depth);
    float dose_plus = bragg_curve(E, d_ptr->spread, depth + d_ptr->depth_res);
    while ( dose_plus < dose )
    {
        E += d_ptr->energy_res;
        dose = bragg_curve(E, d_ptr->spread, depth);
        dose_plus = bragg_curve(E, d_ptr->spread, depth+d_ptr->depth_res);
    }

    /* Check that this energy is really the smallest one that reach the distal prescription */
    /* This case happen if E is already superior at a first estimation, estimated by alpha and p */
    if (E < d_ptr->energy_res)
    {
        return E - E_init;
    }
    E -= d_ptr->energy_res;
    dose = bragg_curve(E, d_ptr->spread, depth);
    dose_plus = bragg_curve(E, d_ptr->spread, depth + d_ptr->depth_res);
    while ( dose_plus > dose)
    {
        E -= d_ptr->energy_res;
        dose = bragg_curve(E, d_ptr->spread, depth);
        dose_plus = bragg_curve(E, d_ptr->spread, depth + d_ptr->depth_res);
    }
    E += d_ptr->energy_res;
    return E - E_init;
}

/* This function check (and correct if necessary) that E_min is the closest energy (+/- energy_resolution) 
	to reach the proximal prescription */
/* This function is designed to return a float value that represents the increase/decrease of energy to correct it
	this is used by two parts of the program on different members (explaining this particular structure) */
float
Rt_mebs::check_and_correct_min_energy(float E, float depth)
{
    float E_init = E;
    /* Check that the depth dose is descreasing (dose_min passed) */
    float dose = bragg_curve(E, d_ptr->spread, depth);
    float dose_minus = bragg_curve(E, d_ptr->spread, depth - d_ptr->depth_res);
    while ( dose_minus < dose )
    {
        if (E < d_ptr->energy_res)
        {
            return E_init - E;
        }
        E -= d_ptr->energy_res;
        dose = bragg_curve(E, d_ptr->spread, depth);
        dose_minus = bragg_curve(E, d_ptr->spread, depth - d_ptr->depth_res);
    }

    /* Check that this energy is really the smallest one that stops before the proximal prescription */
    /* This case happens if E is already superior at a first estimation, estimated by alpha and p */
    E += d_ptr->energy_res;
    dose = bragg_curve(E, d_ptr->spread, depth);
    dose_minus = bragg_curve(E, d_ptr->spread, depth - d_ptr->depth_res);
    while ( dose_minus > dose)
    {
        E += d_ptr->energy_res;
        dose = bragg_curve(E, d_ptr->spread, depth);
        dose_minus = bragg_curve(E, d_ptr->spread, depth - d_ptr->depth_res);
    }
    E -= d_ptr->energy_res;
    return E - E_init;
}

void
Rt_mebs::optimize_sobp ()
{	
    this->update_energies_from_prescription();
    std::vector<float> weight_tmp;
    std::vector<float> energy_tmp;
    this->optimizer (&weight_tmp, &energy_tmp);

    for (int i = 0; i < energy_tmp.size(); i++)
    {
        add_peak(energy_tmp[i], d_ptr->spread, weight_tmp[i]);
    }
}

void 
Rt_mebs::optimizer (std::vector<float>* weight_tmp, std::vector<float>* energy_tmp)
{
    printf("prescription min/max: %lg mm, %lg mm.\n", d_ptr->prescription_depth_min, d_ptr->prescription_depth_max);
    std::vector<Rt_depth_dose*> depth_dose_tmp;
    this->initialize_energy_weight_and_depth_dose_vectors(weight_tmp, energy_tmp, &depth_dose_tmp);

    this->get_optimized_peaks(d_ptr->prescription_depth_min, d_ptr->prescription_depth_max, weight_tmp, &depth_dose_tmp);
}

void
Rt_mebs::initialize_energy_weight_and_depth_dose_vectors(std::vector<float>* weight_tmp, std::vector<float>* energy_tmp, std::vector<Rt_depth_dose*>* depth_dose_tmp)
{
    /* initialize the energies in the table */
    printf("\n %d Mono-energetic BP used:\n", d_ptr->energy_number);
    for (int i = 0; i < d_ptr->energy_number; i++)
    {
        energy_tmp->push_back(d_ptr->beam_max_energy - (float) i * d_ptr->energy_res);
        weight_tmp->push_back(0);
        printf("%lg ", (*energy_tmp)[i]);
        if ((*energy_tmp)[i] < 0)
        {
            d_ptr->energy_number--;
            (*energy_tmp).pop_back();
            weight_tmp->pop_back();
            printf("sobp: peak with energy < 0, Energy resolution error. Last peak deleted.\n");
        }
    }
    printf("\n");

    /* initialize the depth dose curve associated to the energy vector */
    /* Creates depth dose set */
    for (int i = 0; i < d_ptr->energy_number; i++)
    {
        Rt_depth_dose* depth_dose = new::Rt_depth_dose((*energy_tmp)[i],d_ptr->spread, d_ptr->depth_res,d_ptr->depth_end);
        depth_dose_tmp->push_back(depth_dose);
        if (d_ptr->num_samples < depth_dose->num_samples)
        {
            d_ptr->num_samples = depth_dose->num_samples;
        }
    }
}

void 
Rt_mebs::generate_part_num_from_weight(int* ap_dim)
{
    //int idx = 0;
    for (int i = 0; i < d_ptr->energy_number; i++)
    {
        for (int j = 0; j < ap_dim[0] * ap_dim[1]; j++)
        {
            d_ptr->num_particles.push_back(d_ptr->depth_dose_weight[i]);
        }
    }
}

void 
Rt_mebs::scale_num_part(double A, int* ap_dim)
{
    for (int i = 0; i < (int) d_ptr->energy_number * ap_dim[0] * ap_dim[1]; i++)
    {
        d_ptr->num_particles[i] = d_ptr->num_particles[i] * A;
    }
}

double 
Rt_mebs::get_particle_number_xyz(int* idx, double* rest, int idx_beam, const int* ap_dim)
{
    /* The boundaries possible errors like idx = dim are already excluded by the test on the aperture
       Practically, idx = dim -1 is not possible */
    double A = 0;
    double B = 0;
    int spot = 0;
    spot = ap_dim[0] * ap_dim[1] * idx_beam + ap_dim[0] * idx[1] + idx[0];
    A = d_ptr->num_particles[spot] + rest[0] * ( d_ptr->num_particles[spot+1] -  d_ptr->num_particles[spot]);
    spot = ap_dim[0] * ap_dim[1] * idx_beam + ap_dim[0] * (idx[1] +1 ) + idx[0];
    B =  d_ptr->num_particles[spot] + rest[0] * ( d_ptr->num_particles[spot+1] -  d_ptr->num_particles[spot]);
    return A + rest[1] * (B-A);
}

void 
Rt_mebs::extract_particle_number_map_from_txt(Aperture::Pointer& ap)
{
    /* Confirm file can be read */
    if (!file_exists (d_ptr->particle_number_in)) {
        printf ("Error reading config file: %s\n", d_ptr->particle_number_in.c_str());
        printf("Particle number map set to 0 for each dose spot \n");
        return;
    }

    /* Read file into string */
    std::ifstream t (d_ptr->particle_number_in.c_str());
    std::stringstream buffer;
    std::string strEnergy;
    std::stringstream ssenergy;

    buffer << t.rdbuf();

    std::string buf;

    std::stringstream ss (buffer.str());

    std::string val;
    std::string char_part_numb;
    char sep[] = " ";
    char* token;

    int spot_number = 0;
    int line_number = 0;
    int energy_number = 0;
    int idx = 0;
    double part_number = 0;
    double energy;

    while (getline (ss, buf)) 
    {
        buf = string_trim (buf);

        if (buf == "") continue;
        if (buf[0] == '#') continue;

        /* Check the dim for the last spot map */
        if (buf.find ("[Energy]") != std::string::npos && energy_number != 0 && line_number != ap->get_dim(1))
        {
            printf("***WARNING*** the number of spot line doesn't correspond to the aperture size\n");
            printf("spot line number expected: %d, spot line detected: %d.\n", ap->get_dim(1), line_number);
        }

        if (buf.find ("[Energy]") != std::string::npos)
        {
            val ="";
            val = buf.c_str();
            val = strtok(&val[0], "[Energy]");
            energy = strtod(val.c_str(),0);
            spot_number = 0;
            line_number = 0;

            if (energy > 0)
            {
                printf("Particle number map found for energy %lg.\n", energy);
                this->add_peak(energy, d_ptr->spread, 1);
                energy_number++;
                for (int i = 0; i < ap->get_dim(0) * ap->get_dim(1); i++)
                {
                    d_ptr->num_particles.push_back(0);
                }
            }
            continue;
        }
        if (energy_number == 0)
        {
            continue;
        }

        /* If we arrive here, it means that we read a spot map */
        val ="";
        val = buf.c_str();
        val = string_trim (val);
					
        token = strtok(&val[0], sep);
        spot_number = 0;

        while (token != NULL)
        {
            part_number = strtod(token,0);
            if (spot_number < ap->get_dim(0))
            {
                idx = (energy_number-1) * ap->get_dim(0) * ap->get_dim(1) + line_number * ap->get_dim(0) + spot_number;
                d_ptr->num_particles[idx] = part_number;
                spot_number++;
            }
            token = strtok(NULL, sep);
        }
        if (spot_number != ap->get_dim(0))
        {
            printf("***WARNING*** the number of spots doesn't correspond to the aperture size\n");
            printf("line %d: spot number expected: %d, spot number detected: %d.\n", line_number, ap->get_dim(0), spot_number);
        }
        line_number++;
    }

    /* Check the dim for the last spot map */
    if (energy_number != 0 && line_number != ap->get_dim(1))
    {
        printf("***WARNING*** the number of spot line doesn't correspond to the aperture size\n");
        printf("spot line number expected: %d, spot line detected: %d.\n", ap->get_dim(1), line_number);
    }
}

void
Rt_mebs::compute_particle_number_matrix_from_target_active (Rpl_volume* rpl_vol, Plm_image::Pointer& target, float smearing)
{
    int dim[2] = {rpl_vol->get_aperture()->get_dim()[0], rpl_vol->get_aperture()->get_dim()[1]};

    /* vector containing the min and the max of depth of the target */
    std::vector <double> dmin;
    std::vector <double> dmax;
    float min = 0;
    float max = 0;
    rpl_vol->compute_beam_modifiers_active_scanning(target->get_vol(), smearing, d_ptr->proximal_margin, d_ptr->distal_margin, dmin, dmax);

    /* Sanity check */
    if (dmin.size() != rpl_vol->get_aperture()->get_dim(0) * rpl_vol->get_aperture()->get_dim(1) 
        || dmax.size() != rpl_vol->get_aperture()->get_dim(0) * rpl_vol->get_aperture()->get_dim(1))
    {
        printf("ERROR: the aperture size doesn't correspond to the min and max depth maps of the target.\n");
        printf("Aperture size: %d, min depth map size: %d, max depth map size: %d.\n", rpl_vol->get_aperture()->get_dim(0) * rpl_vol->get_aperture()->get_dim(1), dmin.size(), dmax.size());
    }

    for (int i = 0; i < dmax.size(); i++)
    {
        if (dmax[i] > max)
        {
            max = dmax[i];
        }
    }
    min = max;
    for (int i = 0; i < dmin.size(); i++)
    {
        if (dmin[i] < min && dmin[i] != 0)
        {
            min = dmin[i];
        }
    }
    this->set_prescription_depths(min, max);
    printf("Min and max depths in the PTV (target + margins): %lg mm and %lg mm.\n", d_ptr->prescription_depth_min, d_ptr->prescription_depth_max);
    printf("Min and max energies for treating the PTV: %lg MeV and %lg MeV.\n", d_ptr->beam_min_energy, d_ptr->beam_max_energy);

    std::vector<float> energy_tmp;
    std::vector<float> weight_tmp;
    std::vector<Rt_depth_dose*> depth_dose_tmp;
    this->initialize_energy_weight_and_depth_dose_vectors(&weight_tmp, &energy_tmp, &depth_dose_tmp);

    /* initialization of the dose matrix slice for monoenergetic slice */
    for (int i = 0; i < dim[0] *  dim[1] * d_ptr->energy_number;i++)
    {
        d_ptr->num_particles.push_back(0);
    }

    printf("Optimization of the particle number map for any mono-energetic slice in progress...\n");
    /* Let's optimize the SOBP for each beamlet */
    for (int i = 0; i < dmin.size(); i++)
    {
        this->get_optimized_peaks(dmin[i], dmax[i], &weight_tmp, &depth_dose_tmp);
        for (int j = 0; j < d_ptr->energy_number; j++)
        {
            d_ptr->num_particles[i + j *  dim[0] *  dim[1] ] = weight_tmp[j];
            /* Reset weight_tmp for next turn */
            weight_tmp[j] = 0;
        }
    }
    for (int i = 0; i < energy_tmp.size(); i++)
    {
        add_peak(energy_tmp[i], d_ptr->spread, 1);
    }
}

void
Rt_mebs::compute_particle_number_matrix_from_target_active_slicerRt (Rpl_volume* rpl_vol, Plm_image::Pointer& target, float smearing)
{
    int dim[2] = {rpl_vol->get_aperture()->get_dim()[0], rpl_vol->get_aperture()->get_dim()[1]};

    /* vector containing the min and the max of depth of the target */
    std::vector <double> dmin;
    std::vector <double> dmax;
    float min = 0;
    float max = 0;
    rpl_vol->compute_beam_modifiers_core_slicerRt(target, true, smearing, d_ptr->proximal_margin, d_ptr->distal_margin, dmin, dmax);

    /* Sanity check */
    if (dmin.size() != rpl_vol->get_aperture()->get_dim(0) * rpl_vol->get_aperture()->get_dim(1) 
        || dmax.size() != rpl_vol->get_aperture()->get_dim(0) * rpl_vol->get_aperture()->get_dim(1))
    {
        printf("ERROR: the aperture size doesn't correspond to the min and max depth maps of the target.\n");
        printf("Aperture size: %d, min depth map size: %d, max depth map size: %d.\n", rpl_vol->get_aperture()->get_dim(0) * rpl_vol->get_aperture()->get_dim(1), dmin.size(), dmax.size());
    }

    for (int i = 0; i < dmax.size(); i++)
    {
        if (dmax[i] > max)
        {
            max = dmax[i];
        }
    }
    min = max;
    for (int i = 0; i < dmin.size(); i++)
    {
        if (dmin[i] < min && dmin[i] != 0)
        {
            min = dmin[i];
        }
    }
    this->set_prescription_depths(min, max);
    printf("Min and max depths in the PTV (target + margins): %lg mm and %lg mm.\n", d_ptr->prescription_depth_min, d_ptr->prescription_depth_max);
    printf("Min and max energies for treating the PTV: %lg MeV and %lg MeV.\n", d_ptr->beam_min_energy, d_ptr->beam_max_energy);

    std::vector<float> energy_tmp;
    std::vector<float> weight_tmp;
    std::vector<Rt_depth_dose*> depth_dose_tmp;
    this->initialize_energy_weight_and_depth_dose_vectors(&weight_tmp, &energy_tmp, &depth_dose_tmp);

    /* initialization of the dose matrix slice for monoenergetic slice */
    for (int i = 0; i < dim[0] *  dim[1] * d_ptr->energy_number;i++)
    {
        d_ptr->num_particles.push_back(0);
    }

    printf("Optimization of the particle number map for any mono-energetic slice in progress...\n");
    /* Let's optimize the SOBP for each beamlet */
    for (int i = 0; i < dmin.size(); i++)
    {
        this->get_optimized_peaks(dmin[i], dmax[i], &weight_tmp, &depth_dose_tmp);
        for (int j = 0; j < d_ptr->energy_number; j++)
        {
            d_ptr->num_particles[i + j *  dim[0] *  dim[1] ] = weight_tmp[j];
            /* Reset weight_tmp for next turn */
            weight_tmp[j] = 0;
        }
    }
    for (int i = 0; i < energy_tmp.size(); i++)
    {
        add_peak(energy_tmp[i], d_ptr->spread, 1);
    }
}

/* This function returns optimized weighted peaks for passive systems (SOBP weights)
	and active systems (beamlet particle numbers for each energy) */
void 
Rt_mebs::get_optimized_peaks(float dmin, float dmax, std::vector<float>* weight_tmp, std::vector<Rt_depth_dose*>* depth_dose_tmp)
{
    if (dmin == 0 || dmax == 0)
    {
        return;
    }
    int energy_min_index = (int) floor(pow((dmin/(10*d_ptr->alpha)),(1/d_ptr->p)) / d_ptr->energy_res);
    int energy_max_index = (int) ceil(pow((dmax/(10*d_ptr->alpha)),(1/d_ptr->p)) / d_ptr->energy_res);

    float E_min_sobp = (float) energy_min_index * d_ptr->energy_res;
    float E_max_sobp = (float) energy_max_index * d_ptr->energy_res;

    /* This is useful only for active scanning */
    /* check that the E_max is sufficiently high for covering the distal part of the prescription */
    E_max_sobp += this->check_and_correct_max_energy(E_max_sobp, dmax);

    /* check that the E_min is sufficiently low for covering the distal part of the prescription */
    E_min_sobp += this->check_and_correct_min_energy(E_min_sobp, dmin);

    int i0 = (int) ((d_ptr->beam_max_energy - E_max_sobp) / d_ptr->energy_res);
    int imax = (int) ((d_ptr->beam_max_energy - E_min_sobp) / d_ptr->energy_res);

    std::vector<float> d_lut_tmp (d_ptr->num_samples, 0);
    std::vector<float> e_lut_tmp (d_ptr->num_samples, 0);

    for (int i = 0; i < d_ptr->num_samples; i++)
    {
        d_lut_tmp[i] = (float) i * d_ptr->depth_res;
    }

    int idx_max = 0;

    for (int i = i0; i <= imax; i++)
    {
        idx_max = (*depth_dose_tmp)[i]->index_of_dose_max;

        if (idx_max > d_ptr->num_samples)
        {
            printf("***WARNING*** depth_dose %d, max_index > samples.\n", i);
            continue; // the weight remains at 0
        }
        if ( (*depth_dose_tmp)[i]->e_lut[idx_max] <= 0)
        {
            printf("***WARNING*** depth dose #%d is null.\n", i);
            continue; // the weight remains at 0
        }
        (*weight_tmp)[i] = (1-e_lut_tmp[idx_max])/ (*depth_dose_tmp)[i]->e_lut[idx_max];

        if ((*weight_tmp)[i] < 0)
        {
            (*weight_tmp)[i] = 0;
        }

        for (int j = 0; j < (*depth_dose_tmp)[i]->num_samples; j++)
        {
            e_lut_tmp[j] += (*weight_tmp)[i] *  (*depth_dose_tmp)[i]->e_lut[j];
        }
    }
    
    for (int k = 0; k < 40; k++)
    {
        for (int i = i0; i <= imax; i++)
        {
            if (e_lut_tmp[ (*depth_dose_tmp)[i]->index_of_dose_max] != 0)
            {
                (*weight_tmp)[i] /= e_lut_tmp[ (*depth_dose_tmp)[i]->index_of_dose_max];
            }
        }
        for (int j = 0; j < d_ptr->num_samples; j++)
        {
            e_lut_tmp[j] = 0;
        }
		
        for (int i = i0; i <= imax; i++)
        {
            for (int j = 0; j <  (*depth_dose_tmp)[i]->num_samples; j++)
            {
                e_lut_tmp[j] += (*weight_tmp)[i] *  (*depth_dose_tmp)[i]->e_lut[j];
            }
        }
    }

    double mean_sobp = 0;
    double mean_count = 0;
    for (int i = 0; i < d_ptr->num_samples; i++)
    {
        if (d_lut_tmp[i] >= dmin && d_lut_tmp[i] <= dmax)
        {
            mean_sobp += e_lut_tmp[i];
            mean_count++;
        }
    }

    if (mean_count == 0 || mean_sobp == 0)
    {
        printf("***WARNING*** The dose is null in the target interval\n");
        return;
    }

    for(int i = i0; i <= imax; i++)
    {
        (*weight_tmp)[i] /= (float) (mean_sobp / mean_count);
    }
}

void 
Rt_mebs::export_spot_map_as_txt(Aperture::Pointer ap)
{
    make_parent_directories (d_ptr->particle_number_out.c_str());
	
    printf("Trying to write spot maps in %s\n", d_ptr->particle_number_out.c_str());

    std::ofstream fichier(d_ptr->particle_number_out.c_str());

    if ( !fichier ){
        std::cerr << "Erreur de creation du fichier spot_map" << std::endl;
        return;
    }

    int idx = 0;
    for (int e = 0; e < d_ptr->energy_number; e++)
    {
        fichier << "[ENERGY] ";
        fichier << d_ptr->energies[e] << std::endl;
        for (int i = 0; i < ap->get_dim(0); i++)
        {
            for (int j = 0; j < ap->get_dim(1); j++)
            {
                idx = (e * ap->get_dim(0) + i) * ap->get_dim(1) +j;
                fichier << d_ptr->num_particles[idx] << " ";
            }
            fichier << std::endl;
        }
        fichier << std::endl;
    }
    fichier.close();
}

