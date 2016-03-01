/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __plmregister_config_h__
#define __plmregister_config_h__

#include "plm_config.h"

#if ((defined(_WIN32) || defined(WIN32)) && (defined (PLM_BUILD_SHARED_LIBS)))
# ifdef plmregister_EXPORTS
#   define PLMREGISTER_C_API EXTERNC __declspec(dllexport)
#   define PLMREGISTER_API __declspec(dllexport)
# else
#   define PLMREGISTER_C_API EXTERNC __declspec(dllimport)
#   define PLMREGISTER_API __declspec(dllimport)
# endif
# ifdef plmregistercuda_EXPORTS
#   define PLMREGISTERCUDA_C_API EXTERNC __declspec(dllexport)
#   define PLMREGISTERCUDA_API __declspec(dllexport)
# else
#   define PLMREGISTERCUDA_C_API EXTERNC __declspec(dllimport)
#   define PLMREGISTERCUDA_API __declspec(dllimport)
# endif
#else
# define PLMREGISTERCUDA_C_API EXTERNC 
# define PLMREGISTERCUDA_API 
# define PLMREGISTER_C_API EXTERNC 
# define PLMREGISTER_API 
#endif

#endif
