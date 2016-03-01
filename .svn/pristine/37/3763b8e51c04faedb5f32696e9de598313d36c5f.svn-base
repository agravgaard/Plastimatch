/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (defined(_WIN32) || defined(WIN32))
#include <io.h>        // windows //
#endif

#include "file_util.h"
#include "mha_io.h"
#include "plm_endian.h"
#include "plm_fwrite.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "volume.h"

#define LINELEN 512

/* -----------------------------------------------------------------------
   Private functions
   ----------------------------------------------------------------------- */
static void 
write_mha_internal (
    const char* filename,    /* Input: filename to write to */
    const Volume* vol        /* Input: volume to write */
)
{
    FILE* fp;
    char* mha_header = 
	"ObjectType = Image\n"
	"NDims = 3\n"
	"BinaryData = True\n"
	"BinaryDataByteOrderMSB = False\n"
	"TransformMatrix = %g %g %g %g %g %g %g %g %g\n"
	"Offset = %g %g %g\n"
	"CenterOfRotation = 0 0 0\n"
	"ElementSpacing = %g %g %g\n"
	"DimSize = %d %d %d\n"
	"AnatomicalOrientation = RAI\n"
	"%s"
	"ElementType = %s\n"
	"ElementDataFile = LOCAL\n";
    char* element_type;

    if (vol->pix_type == PT_VF_FLOAT_PLANAR) {
	fprintf (stderr, "Error, PT_VF_FLOAT_PLANAR not implemented\n");
	exit (-1);
    }
    fp = plm_fopen (filename,"wb");
    if (!fp) {
	fprintf (stderr, "Can't open file %s for write\n", filename);
	return;
    }
    switch (vol->pix_type) {
    case PT_UCHAR:
	element_type = "MET_UCHAR";
	break;
    case PT_SHORT:
	element_type = "MET_SHORT";
	break;
    case PT_UINT32:
	element_type = "MET_UINT";
	break;
    case PT_FLOAT:
	element_type = "MET_FLOAT";
	break;
    case PT_VF_FLOAT_INTERLEAVED:
	element_type = "MET_FLOAT";
	break;
    case PT_VF_FLOAT_PLANAR:
    default:
	fprintf (stderr, "Unhandled type in write_mha().\n");
	exit (-1);
    }
    fprintf (fp, mha_header, 
	vol->direction_cosines[0], 
        vol->direction_cosines[3], 
	vol->direction_cosines[6], 
        vol->direction_cosines[1], 
	vol->direction_cosines[4], 
        vol->direction_cosines[7], 
	vol->direction_cosines[2], 
        vol->direction_cosines[5], 
	vol->direction_cosines[8], 
	vol->origin[0], vol->origin[1], vol->origin[2], 
	vol->spacing[0], vol->spacing[1], vol->spacing[2], 
	vol->dim[0], vol->dim[1], vol->dim[2],
	(vol->pix_type == PT_VF_FLOAT_INTERLEAVED) 
	? "ElementNumberOfChannels = 3\n" : "",
	element_type);
    fflush (fp);

    if (vol->pix_type == PT_VF_FLOAT_INTERLEAVED) {
	plm_fwrite (vol->img, sizeof(float), 3 * vol->npix, fp, true);
    } else {
	plm_fwrite (vol->img, vol->pix_size, vol->npix, fp, true);
    }

    fclose (fp);
}

static Volume* 
read_mha_internal (
    const char* filename     /* Input: filename to read from */
)
{
    size_t rc;
    char linebuf[LINELEN];
    Volume* vol;
    int tmp;
    FILE* fp;
    bool have_direction_cosines = false;
    bool big_endian_input = false;
    unsigned int a, b, c;
    float dc[9];

    fp = fopen (filename,"rb");
    if (!fp) {
	fprintf (stderr, "File %s not found\n", filename);
	return 0;
    }
   
    fprintf(stdout, "reading %s\n", filename);
 
    vol  = new Volume;
    vol->pix_size = -1;
    vol->pix_type = PT_UNDEFINED;
    while (fgets (linebuf, LINELEN, fp)) {
	string_util_rtrim_whitespace (linebuf);
	if (strcmp (linebuf, "ElementDataFile = LOCAL") == 0) {
	    break;
	}
	if (sscanf (linebuf, "DimSize = %d %d %d", &a, &b, &c) == 3) {
	    vol->dim[0] = a;
	    vol->dim[1] = b;
	    vol->dim[2] = c;
	    vol->npix = vol->dim[0] * vol->dim[1] * vol->dim[2];
	    continue;
	}
	if (sscanf (linebuf, "Offset = %g %g %g",
		&vol->origin[0],
		&vol->origin[1],
		&vol->origin[2]) == 3) {
	    continue;
	}
	if (sscanf (linebuf, "ElementSpacing = %g %g %g",
		&vol->spacing[0],
		&vol->spacing[1],
		&vol->spacing[2]) == 3) {
	    continue;
	}
	if (sscanf (linebuf, "TransformMatrix = %g %g %g %g %g %g %g %g %g",
		&dc[0], &dc[3], &dc[6], 
                &dc[1], &dc[4], &dc[7], 
		&dc[2], &dc[5], &dc[8]) == 9)
	{
	    have_direction_cosines = true;
	    continue;
	}
	if (sscanf (linebuf, "ElementNumberOfChannels = %d", &tmp) == 1) {
	    if (vol->pix_type == PT_UNDEFINED || vol->pix_type == PT_FLOAT) {
		vol->pix_type = PT_VF_FLOAT_INTERLEAVED;
		vol->pix_size = 3*sizeof(float);
	    }
	    continue;
	}
	if (strcmp (linebuf, "ElementType = MET_FLOAT") == 0) {
	    if (vol->pix_type == PT_UNDEFINED) {
		vol->pix_type = PT_FLOAT;
		vol->pix_size = sizeof(float);
	    }
	    continue;
	}
	if (strcmp (linebuf, "ElementType = MET_SHORT") == 0) {
	    vol->pix_type = PT_SHORT;
	    vol->pix_size = sizeof(short);
	    continue;
	}
	if (strcmp (linebuf, "ElementType = MET_UCHAR") == 0) {
	    vol->pix_type = PT_UCHAR;
	    vol->pix_size = sizeof(unsigned char);
	    continue;
	}
	if (strcmp (linebuf, "BinaryDataByteOrderMSB = True") == 0) {
	    big_endian_input = true;
	}
    }

    if (vol->pix_size <= 0) {
	printf ("Oops, couldn't interpret mha data type\n");
	exit (-1);
    }

    /* Update proj and step matrices */
    if (have_direction_cosines) {
	vol->set_direction_cosines (dc);
    } else {
	vol->set_direction_cosines (0);
    }

    vol->img = malloc (vol->pix_size*vol->npix);
    if (!vol->img) {
	printf ("Oops, out of memory\n");
	exit (-1);
    }

    rc = fread (vol->img, vol->pix_size, vol->npix, fp);
    if (rc != (size_t) vol->npix) {
	printf ("Oops, bad read from file (%u)\n", (unsigned int) rc);
	exit (-1);
    }

    /* Swap endian-ness if necessary */
    if (vol->pix_type == PT_VF_FLOAT_INTERLEAVED) {
	if (big_endian_input) {
	    endian4_big_to_native (vol->img, 3 * vol->npix);
	} else {
	    endian4_little_to_native (vol->img, 3 * vol->npix);
	}
    } else if (vol->pix_size == 2) {
	if (big_endian_input) {
	    endian2_big_to_native (vol->img, vol->npix);
	} else {
	    endian2_little_to_native (vol->img, vol->npix);
	}
    } else if (vol->pix_size == 4) {
	if (big_endian_input) {
	    endian4_big_to_native (vol->img, vol->npix);
	} else {
	    endian4_little_to_native (vol->img, vol->npix);
	}
    } else if (vol->pix_size != 1) {
	print_and_exit ("Unknown pixel size: %u\n", vol->pix_size);
    }

    fclose (fp);

    return vol;
}

/* -----------------------------------------------------------------------
   Public functions
   ----------------------------------------------------------------------- */
void 
write_mha (const char* filename, const Volume* vol)
{
    write_mha_internal (filename, vol);
}

Volume* 
read_mha (const char* filename)
{
    return read_mha_internal (filename);
}
