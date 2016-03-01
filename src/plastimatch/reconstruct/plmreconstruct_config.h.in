/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __plmreconstruct_config_h__
#define __plmreconstruct_config_h__

#include "plm_config.h"

#if ((defined(_WIN32) || defined(WIN32)) && (defined (PLM_BUILD_SHARED_LIBS)))
# ifdef plmreconstruct_EXPORTS
#   define PLMRECONSTRUCT_C_API EXTERNC __declspec(dllexport)
#   define PLMRECONSTRUCT_API __declspec(dllexport)
# else
#   define PLMRECONSTRUCT_C_API EXTERNC __declspec(dllimport)
#   define PLMRECONSTRUCT_API __declspec(dllimport)
# endif
# ifdef plmreconstructcuda_EXPORTS
#   define PLMRECONSTRUCTCUDA_C_API EXTERNC __declspec(dllexport)
#   define PLMRECONSTRUCTCUDA_API __declspec(dllexport)
# else
#   define PLMRECONSTRUCTCUDA_C_API EXTERNC __declspec(dllimport)
#   define PLMRECONSTRUCTCUDA_API __declspec(dllimport)
# endif
#else
# define PLMRECONSTRUCTCUDA_C_API EXTERNC 
# define PLMRECONSTRUCTCUDA_API 
# define PLMRECONSTRUCT_C_API EXTERNC 
# define PLMRECONSTRUCT_API 
#endif

#endif
