/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __plmqt_config_h__
#define __plmqt_config_h__

#include "plm_config.h"

#ifdef API 
#  undef API
#endif
#ifdef C_API 
#  undef C_API
#endif

#if ((defined(_WIN32) || defined(WIN32)) && (defined (PLM_BUILD_SHARED_LIBS)))
# ifdef plmqt_EXPORTS
#   define C_API EXTERNC __declspec(dllexport)
#   define API __declspec(dllexport)
# else
#   define C_API EXTERNC __declspec(dllimport)
#   define API __declspec(dllimport)
# endif
#else
# define C_API EXTERNC 
# define API 
#endif

#endif
