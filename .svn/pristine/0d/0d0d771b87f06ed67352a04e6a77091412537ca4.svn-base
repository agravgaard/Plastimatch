/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_util.h"
#include "print_and_exit.h"
#include "rtss.h"
#include "rtss_roi.h"
#include "ss_list_io.h"

#define CXT_BUFLEN 2048

Rtss*
ss_list_load (Rtss* cxt, const char* ss_list_fn)
{
    FILE* fp;
    int struct_id;

    fp = fopen (ss_list_fn, "r");
    if (!fp) {
	print_and_exit (
	    "Could not open ss_list file for read: %s\n", ss_list_fn);
    }
    if (!cxt) {
	cxt = new Rtss;
    }

    /* Part 2: Structures info */
    struct_id = 0;
    while (1) {
        char color[CXT_BUFLEN];
        char name[CXT_BUFLEN];
        char buf[CXT_BUFLEN];
        char *p;
	int bit;
        int rc;

        p = fgets (buf, CXT_BUFLEN, fp);
        if (!p) {
	    break;
        }
        rc = sscanf (buf, "%d|%[^|]|%[^\r\n]", &bit, color, name);
        if (rc != 3) {
            print_and_exit (
		"Error. ss_list file not formatted correctly: %s\n",
		ss_list_fn);
        }

	Rtss_roi *curr_structure = cxt->add_structure (
	    std::string (name), std::string (color), struct_id);
	curr_structure->bit = bit;
	struct_id ++;
    }

    fclose (fp);
    return cxt;
}

void
ss_list_save (Rtss* cxt, const char* ss_list_fn)
{
    FILE *fp;
	
    make_parent_directories (ss_list_fn);
    fp = fopen (ss_list_fn, "wb");
    if (!fp) {
	print_and_exit (
	    "Could not open ss_list file for write: %s\n", ss_list_fn);
    }

    for (size_t i = 0; i < cxt->num_structures; i++) {
	Rtss_roi *curr_structure;
	curr_structure = cxt->slist[i];
	fprintf (fp, "%d|%s|%s\n", 
	    curr_structure->bit, 
	    (curr_structure->color.empty() 
		? "255\\0\\0"
		: curr_structure->color.c_str()),
	    curr_structure->name.c_str());
    }
    fclose (fp);
    printf ("Done.\n");
}

void
ss_list_save_colormap (Rtss* cxt, const char* colormap_fn)
{
    int color_no;
    FILE *fp;
	
    make_parent_directories (colormap_fn);
    fp = fopen (colormap_fn, "wb");
    if (!fp) {
	print_and_exit (
	    "Could not open colormap file for write: %s\n", colormap_fn);
    }

    fprintf (fp, "0 Background 0 0 0 255\n");

    /* Colormap should match labelmap.  This means that filled structures
       are numbered before empty structures.  We accomplish this by running 
       two passes: filled structures, then empty structures. */
    color_no = 0;
    for (size_t i = 0; i < cxt->num_structures; i++) {
	int r, g, b;
	Rtss_roi *curr_structure;
	curr_structure = cxt->slist[i];
	if (curr_structure->bit >= 0) {
	    curr_structure->get_rgb (&r, &g, &b);
            std::string adjusted_name = Rtss_roi::adjust_name (
                curr_structure->name);
	    fprintf (fp, "%d %s %d %d %d 255\n", 
		curr_structure->bit + 1, adjusted_name.c_str(), r, g, b);
	    color_no = curr_structure->bit + 1;
	}
    }

    for (size_t i = 0; i < cxt->num_structures; i++) {
	int r, g, b;
	Rtss_roi *curr_structure;

	curr_structure = cxt->slist[i];
	if (curr_structure->bit == -1) {
	    curr_structure->get_rgb (&r, &g, &b);
            std::string adjusted_name = Rtss_roi::adjust_name (
                curr_structure->name);
	    fprintf (fp, "%d %s %d %d %d 255\n", 
		color_no + 1, adjusted_name.c_str(), r, g, b);
	    color_no ++;
	}
    }
    fclose (fp);
}
