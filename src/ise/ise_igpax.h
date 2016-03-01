/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __ISE_IGPAX_H__
#define __ISE_IGPAX_H__

#include "ise_error.h"
#include "ise_structs.h"

void ise_igpax_init (void);
Ise_Error ise_igpax_open (IgpaxInfo* igpax, char* ip_address_server, char* ip_address_client);
Ise_Error ise_igpax_start_fluoro (IgpaxInfo* igpax, int image_source, Framerate framerate);
Ise_Error ise_igpax_send_command (IgpaxInfo* igpax, char cmd);
void ise_igpax_shutdown (IgpaxInfo* igpax);

#endif
