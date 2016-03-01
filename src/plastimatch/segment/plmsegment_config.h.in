/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __plmsegment_config_h__
#define __plmsegment_config_h__

#include "plm_config.h"

#if ((defined(_WIN32) || defined(WIN32)) && (defined (PLM_BUILD_SHARED_LIBS)))
# ifdef plmsegment_EXPORTS
#   define PLMSEGMENT_C_API EXTERNC __declspec(dllexport)
#   define PLMSEGMENT_API __declspec(dllexport)
# else
#   define PLMSEGMENT_C_API EXTERNC __declspec(dllimport)
#   define PLMSEGMENT_API __declspec(dllimport)
# endif
#else
# define PLMSEGMENT_C_API EXTERNC 
# define PLMSEGMENT_API 
#endif

#endif
