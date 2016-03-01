/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __plmbase_config_h__
#define __plmbase_config_h__

#include "plm_config.h"

#if ((defined(_WIN32) || defined(WIN32)) && (defined (PLM_BUILD_SHARED_LIBS)))
# ifdef plmbase_EXPORTS
#   define PLMBASE_C_API EXTERNC __declspec(dllexport)
#   define PLMBASE_API __declspec(dllexport)
# else
#   define PLMBASE_C_API EXTERNC __declspec(dllimport)
#   define PLMBASE_API __declspec(dllimport)
# endif
#else
# define PLMBASE_C_API EXTERNC 
# define PLMBASE_API 
#endif

#endif
