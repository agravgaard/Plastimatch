/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "rt_mebs.h"

int main (int argc, char* argv[])
{
    float dmin, dmax;
    int Emin, Emax;
    int particle;

    Particle_type particle_type;

    if (argc < 4) {
        printf (
            "Usage:\n"
            "  sobp d dmin dmax    // Optimize in mm from dmin to dmax\n"
            "  sobp e emin emax    // Optimize in MeV from emin to emax\n");
        exit (0);
    }

    sscanf(argv[1],"%d", &particle);

    if(particle ==1)
    {
        particle_type = PARTICLE_TYPE_P;
    }
    else if (particle ==2)
    {
        particle_type = PARTICLE_TYPE_HE;
    }
    else if (particle ==3)
    {
        particle_type = PARTICLE_TYPE_LI;
    }
    else if (particle ==4)
    {
        particle_type = PARTICLE_TYPE_BE;
    }
    else if (particle ==5)
    {
        particle_type = PARTICLE_TYPE_B;
    }
    else if (particle ==6)
    {
        particle_type = PARTICLE_TYPE_C;
    }
    else if (particle ==8)
    {
        particle_type = PARTICLE_TYPE_O;
    }
    else
    {
        particle_type = PARTICLE_TYPE_P;
        printf("Invalid particle type");
    }

    if (particle_type != PARTICLE_TYPE_P) // no data for ions... to be implemented (ion bragg peaks!!)
    {
        particle_type = PARTICLE_TYPE_P;
        printf("Ions data are not ready yet - beam switched to proton beams");
    }

    Rt_mebs mebs(particle_type);

    // construction of the sobp using the proximal and distal limits
    if (argv[2][0]=='d')
    {
        sscanf (argv[3], "%f", &dmin);
        sscanf (argv[4], "%f", &dmax);
        mebs.set_prescription_depths(dmin, dmax);
    }
    // construction of the sobp using the lower and higher energy
    else if (argv[2][0]=='e')
    {
        sscanf (argv[3], "%d", &Emin);
        sscanf (argv[4], "%d", &Emax);
		mebs.set_energies(Emin, Emax);
    }
	std::vector<float> weight;
	std::vector<float> energy;
    mebs.optimizer(&weight, &energy);
	for (int i = 0; i < energy.size(); i++)
	{
		mebs.add_peak(energy[i], mebs.get_spread(), weight[i]);
	}
	mebs.generate();
	mebs.printparameters();

    return 0;
}
