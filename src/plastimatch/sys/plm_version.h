/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_version_h_
#define _plm_version_h_

#include "plmsys_config.h"

#if defined (PLM_RELEASE_VERSION_STRING)
  #define PLASTIMATCH_VERSION PLM_RELEASE_VERSION_STRING
#elif defined (PLM_CONFIG_VERSION_STRING)
  #define PLASTIMATCH_VERSION PLM_CONFIG_VERSION_STRING
#else
  #define PLASTIMATCH_VERSION PLM_DEFAULT_VERSION_STRING "-beta"
#endif

#if (PLASTIMATCH_HAVE_BUILD_NUMBER)
  #define PLASTIMATCH_VERSION_STRING  PLASTIMATCH_VERSION " (" PLASTIMATCH_BUILD_NUMBER ")"
#else
  #define PLASTIMATCH_VERSION_STRING  PLASTIMATCH_VERSION
#endif

#endif
