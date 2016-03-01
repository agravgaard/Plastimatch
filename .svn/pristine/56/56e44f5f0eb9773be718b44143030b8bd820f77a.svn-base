/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __plmsys_config_h__
#define __plmsys_config_h__

#include "plm_config.h"

#if ((defined(_WIN32) || defined(WIN32)) && (defined (PLM_BUILD_SHARED_LIBS)))
# ifdef plmsys_EXPORTS
#   define PLMSYS_C_API EXTERNC __declspec(dllexport)
#   define PLMSYS_API __declspec(dllexport)
# else
#   define PLMSYS_C_API EXTERNC __declspec(dllimport)
#   define PLMSYS_API __declspec(dllimport)
# endif
#else
# define PLMSYS_C_API EXTERNC
# define PLMSYS_API 
#endif

#endif
