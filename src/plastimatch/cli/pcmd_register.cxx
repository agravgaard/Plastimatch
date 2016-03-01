/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "pcmd_register.h"
#include "registration.h"

void
do_command_register (int argc, char* argv[])
{
    char *command_filename;

    if (!strcmp (argv[1], "register")) {
        if (argc > 2) {
            command_filename = argv[2];
        } else {
            printf ("Usage: plastimatch register command_file\n");
            exit (1);
        }
    } else {
        command_filename = argv[1];
    }

    Registration reg;
    if (reg.set_command_file (command_filename) < 0) {
        printf ("Error.  could not load %s as command file.\n", 
            command_filename);
    }
    reg.do_registration ();
}
