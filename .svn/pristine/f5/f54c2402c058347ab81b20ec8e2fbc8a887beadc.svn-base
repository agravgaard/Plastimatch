/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include <windows.h>
#include "ise.h"
#include "ise_gl_shader.h"
#include "debug.h"


/* ---------------------------------------------------------------------------- *
    Global variables
 * ---------------------------------------------------------------------------- */


/* ---------------------------------------------------------------------------- *
    Function declarations
 * ---------------------------------------------------------------------------- */


/* ---------------------------------------------------------------------------- *
    Global functions
 * ---------------------------------------------------------------------------- */
ShaderInfo* 
shader_init (void)
{
    ShaderInfo* si;
    CGprogram prog;

    si = (ShaderInfo*) malloc (sizeof(ShaderInfo));

    si->mcg_context = cgCreateContext();

    // It seems that the shader profiles' symbolic names are simply defined as:
    // CG_PROFILE_<cgcprofilename>.  I have found no documentation to confirm this, 
    // but from the lists of profile targets I've found, it appears to be true.
    // nVidia-specific fragment profiles will be FPxx where xx denotes the version
    // ARB-based fragment profile will be ARBFPx where x denotes the version
    //
    // nVidia NV_fragment_program type.. requires cgc version 1.4 or better
    if (cgGLIsProfileSupported(CG_PROFILE_FP40)) {
	si->mcg_profile = CG_PROFILE_FP40;
    } else if (cgGLIsProfileSupported(CG_PROFILE_FP30)) {
	si->mcg_profile = CG_PROFILE_FP30;
    } else if (cgGLIsProfileSupported(CG_PROFILE_FP20)) {
	si->mcg_profile = CG_PROFILE_FP20;
    } else {
	si->mcg_profile = CG_PROFILE_ARBFP1;
    }

    cgGLEnableProfile (si->mcg_profile);
    prog = cgCreateProgramFromFile (si->mcg_context, CG_SOURCE, "shader.cg",
				    si->mcg_profile, NULL, NULL);
    if (!cgIsProgram(prog)) {
        CGerror Error = cgGetError();
	debug_printf ("Deadly error creating fragment shader program\n");
    }

    si->mcg_scaleParam = cgGetNamedParameter (prog, "scale");
    si->mcg_hiParam = cgGetNamedParameter (prog, "hival");
    si->mcg_loParam = cgGetNamedParameter (prog, "loval");
    cgGLLoadProgram (prog);
    si->mcg_loadedProgram = prog;

    //si->m_loval = 0.0;
    //si->m_hival = 4.0;
    si->m_loval = 0.0;
    si->m_hival = 1.0;

    return si;
}


void
shader_destroy (ShaderInfo* si)
{
    cgGLDisableProfile (si->mcg_profile);
    cgDestroyProgram (si->mcg_loadedProgram);
    cgDestroyContext (si->mcg_context);
}

void
shader_update_lut (ShaderInfo* si, unsigned short bot, 
		   unsigned short top)
{
    si->m_loval = (float) bot / (float) MAXGREY;
    /* Add +1 to avoid divide by zero when bot == top */
    si->m_hival = (float) top / (float) MAXGREY;
}

void 
shader_apply (ShaderInfo* si)
{
    //m_loval = loval / (float) 16000;
    //m_hival = hival / (float) 16000;
    //SetVals(loval, hival);

    //si->m_loval = 0.0;
    //si->m_hival = 4.0;


    cgGLBindProgram (si->mcg_loadedProgram);
    cgGLSetParameter1f (si->mcg_scaleParam, 4.0);
#if defined (commentout)
    debug_printf ("Setting hival: %g\n", (double) si->m_hival);
    debug_printf ("Setting loval: %g\n", (double) si->m_loval);
#endif
    cgGLSetParameter1f (si->mcg_hiParam, si->m_hival);
    cgGLSetParameter1f (si->mcg_loParam, si->m_loval);
    cgGLEnableProfile (si->mcg_profile);
}

void 
shader_disable (ShaderInfo* si)
{
    cgGLDisableProfile (si->mcg_profile);
}
