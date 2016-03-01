/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __plmdose_config_h__
#define __plmdose_config_h__

#include "plm_config.h"

#if ((defined(_WIN32) || defined(WIN32)) && (defined (PLM_BUILD_SHARED_LIBS)))
# ifdef plmdose_EXPORTS
#   define PLMDOSE_C_API EXTERNC __declspec(dllexport)
#   define PLMDOSE_API __declspec(dllexport)
# else
#   define PLMDOSE_C_API EXTERNC __declspec(dllimport)
#   define PLMDOSE_API __declspec(dllimport)
# endif
#else
# define PLMDOSE_C_API EXTERNC 
# define PLMDOSE_API 
#endif

#endif
