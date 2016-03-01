/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dicom_util.h"
#include "plm_uid_prefix.h"

void
print_usage (void)
{
    fprintf (stderr, "Usage: dicom_uid [prefix]\n");
    exit (-1);
}

int
main (int argc, char* argv[])
{
    char uid[100];
    const char* uid_root = PLM_UID_PREFIX;

    if (argc == 2) {
	uid_root = argv[1];
    } else if (argc != 1) {
	print_usage ();
    }

    if (strlen (uid_root) >= 32) {
	fprintf (stderr, "Sorry, uid prefix should be less than 32 characters\n");
	exit (-1);
    }

    dicom_uid (uid, uid_root);

    printf ("%s\n", uid);
    return (0);
}
