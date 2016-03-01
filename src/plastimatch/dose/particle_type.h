/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _particle_type_h_
#define _particle_type_h_

#include "plmdose_config.h"
#include <string>

/* Particle type: 0=photon, 1= proton, ions: 2= helium, 3=lithium, 4=beryllium, 5=bore, 6=carbon, 7= nitrogen(not used), 8=oxygen */
enum Particle_type {
    PARTICLE_TYPE_UNKNOWN=-20,
    PARTICLE_TYPE_X=0, 
    PARTICLE_TYPE_P=1, 
    PARTICLE_TYPE_HE=2, 
    PARTICLE_TYPE_LI=3, 
    PARTICLE_TYPE_BE=4, 
    PARTICLE_TYPE_B=5, 
    PARTICLE_TYPE_C=6,
	PARTICLE_TYPE_N=7,		// Not used for particle therapy
    PARTICLE_TYPE_O=8
};

Particle_type
particle_type_parse (const std::string& s);
const char*
particle_type_string (Particle_type p);

#endif
