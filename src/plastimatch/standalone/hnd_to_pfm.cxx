/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_util.h"
#include "path_util.h"
#include "print_and_exit.h"
#include "proj_image.h"

int 
main (int argc, char* argv[]) 
{
    Proj_image *proj;
    char *hnd_fn, *pfm_fn, *mat_fn, *tmp;

    if (argc != 3) {
	printf ("Usage: hndtopfm hndfile pfmfile\n");
	return 1;
    }

    hnd_fn = argv[1];
    pfm_fn = argv[2];

    /* Create filename for matrix file */
    tmp = strdup (pfm_fn);
    strip_extension (tmp);
    mat_fn = (char*) malloc (strlen (tmp) + 5);
    sprintf (mat_fn, "%s.txt", tmp);
    free (tmp);

    /* Read image */
    double xy_offset[2] = {0., 0.};
    proj = new Proj_image (hnd_fn, xy_offset);
    if (!proj->have_image ()) {
	print_and_exit ("Couldn't load file for read: %s\n", hnd_fn);
    }

    /* Write image and header */
    proj_image_save (proj, pfm_fn, mat_fn);

    delete proj;
    return 0;
}
