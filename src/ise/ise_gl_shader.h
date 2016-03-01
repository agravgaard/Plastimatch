/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __ISE_GL_SHADER_H__
#define __ISE_GL_SHADER_H__

#include <GL/gl.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>

typedef struct __ShaderInfo {
    CGcontext mcg_context;
    CGprofile mcg_profile;
    CGprogram mcg_loadedProgram;
    CGparameter mcg_scaleParam;
    float m_loval;
    float m_hival;
    CGparameter mcg_hiParam;
    CGparameter mcg_loParam;
    CGparameter mcg_slopeParam;
} ShaderInfo;

struct __ShaderInfo* shader_init (void);
void shader_destroy (struct __ShaderInfo* si);
void shader_update_lut (struct __ShaderInfo* si, unsigned short bot, 
		   unsigned short top);
void shader_apply (struct __ShaderInfo* si);
void shader_disable (struct __ShaderInfo* si);

#endif
