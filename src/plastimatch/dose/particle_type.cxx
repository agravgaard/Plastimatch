/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmdose_config.h"
#include "particle_type.h"

Particle_type
particle_type_parse (const std::string& s)
{
    if (s == "X") {
        return PARTICLE_TYPE_X;
    }
    else if (s == "P") {
        return PARTICLE_TYPE_P;
    }
    else if (s == "HE") {
        return PARTICLE_TYPE_HE;
    }
    else if (s == "LI") {
        return PARTICLE_TYPE_LI;
    }
    else if (s == "P") {
        return PARTICLE_TYPE_P;
    }
    else if (s == "BE") {
        return PARTICLE_TYPE_BE;
    }
    else if (s == "B") {
        return PARTICLE_TYPE_B;
    }
    else if (s == "C") {
        return PARTICLE_TYPE_C;
    }
		else if (s == "N") {
        return PARTICLE_TYPE_N;
    }
    else if (s == "O") {
        return PARTICLE_TYPE_O;
    }
    else {
        return PARTICLE_TYPE_UNKNOWN;
    }
}

const char*
particle_type_string (Particle_type p)
{
    switch (p) {
    case PARTICLE_TYPE_P:			// proton
        return "Proton";
    case PARTICLE_TYPE_HE:			// helium
        return "Helium";
    case PARTICLE_TYPE_LI:			// lithium
        return "Lithium";
    case PARTICLE_TYPE_BE:			// berilium
        return "Berillium";
    case PARTICLE_TYPE_B:			// bore
        return "Boron";
    case PARTICLE_TYPE_C:			// carbon
        return "Carbon";
	case PARTICLE_TYPE_N:			// nitrogen
		return "Nitrogen";
    case PARTICLE_TYPE_O:			// oxygen
        return "Oxygen";
    default:
        return "Unknown";
    }
}
