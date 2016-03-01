/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* Convert RTOG CT (and DOSE) to MHA
   -----------------------------------------
   Limitations.  Not all of these are tested for!!!
      Only 1 CT scan allowed
      CT slice spacing must be uniform
      Anatomy should be RAI, transverse slices
      Only 1 DOSE allowed
      CT's must be 2 byte
      (DOSE must be 2 byte unsigned by specification)
      Assume DOSE is PHYSICAL & GRAYS
      Assume INTEL architecture (little endian)
    ----------------------------------------
    Interpretation - based on CORVUS export:
      For CT:
      The (x,y) offset is roughly the position of the upper left corner
        of the upper left pixel, in rtog patient coordinates, which 
	means relative to the center of the image.
      The z offset (z value of the first slice) seems to be completely 
        arbitrary, but related to the strange first slice thickness value
      As usual, the pixel size is rounded
      For DOSE:
      The offset is the position of the center of the upper left pixel, 
        relative to rtog patient coordinate isocenter.  This differs from 
	the CT offset, which is relative to image center.
    ----------------------------------------
    Note about RTOG
      For CT, dim1 is rows & dim2 is columns, but grid1 is cols & grid2 is rows
      For DOSE, dim1 is cols & dim2 is rows
    ----------------------------------------
    Implementation based on interpretation
      Use nominal pixel size, even though it is more highly rounded.
      No need to flip CT Y positions, because they are already flipped 
        by specification.
      Verify that CT Z positions are increasing, and flip the position 
        accordingly.
      So I just set MHA origin at 1/2 pixel for both CT and dose
      < RMK: This works for (x,y), but not for (z) - NEED TO FIX >
      For MASK, need to flip Y
 */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#if (defined(_WIN32) || defined(WIN32))
#include <direct.h>
#define snprintf _snprintf
#define mkdir(a,b) _mkdir(a)
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "exchkeys.h"

#define BUFLEN 2048

char mha_header_pat[] = 
    "ObjectType = Image\n"
    "NDims = 3\n"
    "BinaryData = True\n"
    "BinaryDataByteOrderMSB = False\n"
    "Offset = %f %f %f\n"
    "ElementSpacing = %f %f %f\n"
    "DimSize = %d %d %d\n"
    "AnatomicalOrientation = RAI\n"
    "ElementType = %s\n"
    "ElementDataFile = LOCAL\n"
    ;

typedef struct program_parms Program_Parms;
struct program_parms {
    char* indir;
    char* outdir;
};

typedef struct ct_header CT_Header;
struct ct_header {
    int first_image;
    int last_image;
    float grid_1_units;
    float grid_2_units;
    int size_of_dimension_1;
    int size_of_dimension_2;
    float x_offset;
    float y_offset;
    float z_offset;
    float z_spacing;
    int ct_offset;
    int ct_air;
    int ct_water;
    void* image;
};

typedef struct dose_header DOSE_Header;
struct dose_header {
    int imno;
    int size_of_dimension_1;
    int size_of_dimension_2;
    int size_of_dimension_3;
    float coord_1_of_first_point;
    float coord_2_of_first_point;
    float coord_3_of_first_point;
    float horizontal_grid_interval;
    float vertical_grid_interval;
    float depth_grid_interval;
    float dose_scale;
    unsigned short* image;
    float* fimage;
};

typedef struct polyline Cxt_polyline;
struct polyline {
    int num_vertices;
    float* x;
    float* y;
    float* z;
};

typedef struct polyline_slice Cxt_polyline_Slice;
struct polyline_slice {
    int slice_no;
    int num_polyline;
    Cxt_polyline* pllist;
};

typedef struct structure STRUCTURE;
struct structure {
    int imno;
    char name[BUFLEN];
    int num_slices;
    Cxt_polyline_Slice* pslist;
};

typedef struct structure_list STRUCTURE_List;
struct structure_list {
    int num_structures;
    STRUCTURE* slist;
    int skin_no;
    unsigned char* skin_image;
};

typedef struct rtog_header RTOG_Header;
struct rtog_header {
    CT_Header ct;
    DOSE_Header dose;
    STRUCTURE_List structures;
};

typedef struct rtog_line RTOG_Line;
struct rtog_line {
    int key;
    int ival;
    float fval;
};

void
print_usage (void)
{
    printf ("Usage: rtog_to_mha [options]\n");
    printf ("  -d dir    input directory\n");
    printf ("  -o dir    output directory\n");
    exit (0);
}

void
gs_strncpy (char* dst, char* src, int n)
{
    int i = 0;
    for (i = 0; i < n; i++) {
        if (!(dst[i] = src[i])) {
            return;
        }
    }
    dst[i-1] = 0;
}

int
get_rtog_line (FILE* fp, char* key, char* val)
{
    char buf[BUFLEN];
    char* s;

    while (1) {
	if (!fgets (buf,BUFLEN,fp)) return 0;
	s = strstr (buf, ":= ");
	if (!s) continue;
	gs_strncpy (key, buf, s-buf);
	gs_strncpy (val, s+strlen(":= "), BUFLEN);
	if ((s = strpbrk (val, "\n\r"))) {
	    *s = 0;
	}
	return 1;
    }
}

int
parse_rtog_string (char** list, int list_size, char* str)
{
    char upstr[BUFLEN];
    char *u, *s;
    int i;

    /* Make str uppercase */
    for (u = upstr, s = str; (*u = toupper(*s)); u++, s++);

    /* Find in list and return enum */
    for (i = 0; i < list_size; i++) {
	char* list_string = list[i];
	if (!strncmp (upstr, list_string, strlen(list_string))) {
	    return i;
	}
    }
    return -1;
}

int
parse_rtog_key (char* key)
{
    return parse_rtog_string (key_list_words, RTOG_NUM_KEYS, key);
}

int
parse_rtog_value (char* value)
{
    return parse_rtog_string (key_value_words, RTOG_NUM_KEY_VALS, value);
}

int
parse_rtog_line (RTOG_Line* rtog_line, char* key, char* val)
{
    int rc;

    rtog_line->key = -1;
    rtog_line->ival = -1;
    rtog_line->fval = 0.0;

    /* Parse key */
    rtog_line->key = parse_rtog_key (key);
    if (rtog_line->key < 0) return -1;

    /* Parse value */
    switch (rtog_line->key) {
	/* Strings */
	case ekIMAGETYPE:
	    rtog_line->ival = parse_rtog_value (val);
	    if (rtog_line->ival < 0) return -1;
	    break;
	case ekSTRUCTURENAME:
	    /* Leave the data in "val" */
	    break;

	/* Integers */
	case ekIMAGENUMBER:
	case ekSIZEOFDIMENSION1:
	case ekSIZEOFDIMENSION2:
	case ekSIZEOFDIMENSION3:
	case ekCTOFFSET:
	case ekCTAIR:
	case ekCTWATER:
	case ekNUMBEROFDIMENSIONS:
	    rc = sscanf (val, "%d", &rtog_line->ival);
	    if (rc < 0) return -1;
	    break;

	/* floats */
	case ekGRID1UNITS:
	case ekGRID2UNITS:
	case ekXOFFSET:
	case ekYOFFSET:
	case ekZVALUE:
	case ekCOORD1OFFIRSTPOINT:
	case ekCOORD2OFFIRSTPOINT:
	case ekCOORD3OFFIRSTPOINT:
	case ekHORIZONTALGRIDINTERVAL:
	case ekVERTICALGRIDINTERVAL:
	case ekDEPTHGRIDINTERVAL:
	case ekDOSESCALE:
	    return sscanf (val, "%f", &rtog_line->fval);
	    if (rc < 0) return -1;
	    break;
    }
    return 0;
}

int
set_ct_ival (RTOG_Header* rtog_header, RTOG_Line* rtog_line, int imno, 
			    int* tgt, char* tgt_comment)
{
    if (rtog_header->ct.first_image == imno) {
	*tgt = rtog_line->ival;
    } else {
	if (*tgt != rtog_line->ival) {
	    printf ("Inconsistent %s\n", tgt_comment);
	    return -1;
	}
    }
    return 0;
}

int
set_ct_fval (RTOG_Header* rtog_header, RTOG_Line* rtog_line, int imno, 
			    float* tgt, char* tgt_comment)
{
    if (rtog_header->ct.first_image == imno) {
	*tgt = rtog_line->fval;
    } else {
	if (*tgt != rtog_line->fval) {
	    printf ("Inconsistent %s\n", tgt_comment);
	    return -1;
	}
    }
    return 0;
}

void
load_rtog_header (RTOG_Header* rtog_header, Program_Parms* parms)
{
    FILE* fp;
    int rc;
    RTOG_Line rtog_line;
    int imtype = -1;
    int imno = -1;
    float last_z = -100000.0;
    char fn[BUFLEN];
    char key[BUFLEN], val[BUFLEN];
    STRUCTURE* curr_struct = 0;

    /* Open header file */
    snprintf (fn, BUFLEN, "%s/aapm0000", parms->indir);
    fp = fopen (fn, "r");
    if (!fp) {
	printf ("Error: could not open file \"%s\" for read.\n", fn);
	print_usage ();
	exit (-1);
    }

    /* Initialize rtog_header structure */
    rtog_header->ct.first_image = -1;
    rtog_header->dose.imno = -1;
    rtog_header->structures.num_structures = 0;
    rtog_header->structures.slist = 0;

    /* Loop through file, adding to rtog_header struct */
    while (1) {
	if (!get_rtog_line (fp, key, val)) {
	    break;
	}
	if (parse_rtog_line (&rtog_line, key, val) < 0) {
	    printf ("parse_rtog_line() failed\n");
	    goto error_exit;
	}
	switch (parse_rtog_key(key)) {
	case -1:
	    goto error_exit;
	case ekIMAGENUMBER:
	    rc = sscanf (val, "%d", &imno);
	    if (rc != 1) goto error_exit;
	    break;
	case ekIMAGETYPE:
	    imtype = parse_rtog_value (val); 
	    switch (imtype) {
	    case -1:
		goto error_exit;
	    case evCTSCAN:
		if (rtog_header->ct.first_image < 0) {
		    rtog_header->ct.first_image = imno;
		}
		rtog_header->ct.last_image = imno;
		break;
	    case evDOSE:
		if (rtog_header->dose.imno == -1) {
		    rtog_header->dose.imno = imno;
		} else {
		    printf ("Warning, multiple dose sections found.\n");
		}
		break;
	    case evSTRUCTURE:
		rtog_header->structures.num_structures ++;
		rtog_header->structures.slist = (STRUCTURE*) 
		    realloc (rtog_header->structures.slist, 
			    rtog_header->structures.num_structures * sizeof (STRUCTURE));
		curr_struct = &rtog_header->structures.slist[rtog_header->structures.num_structures-1];
		curr_struct->imno = imno;
		curr_struct->name[0] = 0;
		curr_struct->num_slices = 0;
		curr_struct->pslist = 0;
		break;
	    default:
		printf ("Warning: unhandled image type: %s", val);
		break;
	    }
	    break;
	case ekZVALUE:
	    if (rtog_header->ct.first_image == imno) {
		rtog_header->ct.z_offset = rtog_line.fval;
	    } else if (rtog_header->ct.z_offset == last_z) {
		rtog_header->ct.z_spacing = rtog_line.fval - last_z;
		if (rtog_header->ct.z_spacing < 0) {
		    printf ("Error, z_spacing is decreasing.\n");
		    goto error_exit;
		}
	    } else {
		double SPACING_TOL = 1e-6;
		if (fabs(rtog_header->ct.z_spacing - (rtog_line.fval - last_z)) > SPACING_TOL) {
		    printf ("Inconsistent z_spacing: %f\n", rtog_header->ct.z_spacing - (rtog_line.fval - last_z));
		    goto error_exit;
		}
	    }
	    last_z = rtog_line.fval;
	    break;
	case ekGRID1UNITS:
	    if (imtype == evCTSCAN) {
		rc = set_ct_fval (rtog_header, &rtog_line, imno, &rtog_header->ct.grid_1_units, "grid_1");
		if (rc < 0) goto error_exit;
	    }
	    break;
	case ekGRID2UNITS:
	    if (imtype == evCTSCAN) {
		rc = set_ct_fval (rtog_header, &rtog_line, imno, &rtog_header->ct.grid_2_units, "grid_2");
		if (rc < 0) goto error_exit;
	    }
	    break;
	case ekSIZEOFDIMENSION1:
	    if (imtype == evCTSCAN) {
		rc = set_ct_ival (rtog_header, &rtog_line, imno, &rtog_header->ct.size_of_dimension_1, "size 1");
		if (rc < 0) goto error_exit;
	    }
	    else if (imtype == evDOSE && imno == rtog_header->dose.imno) {
		rtog_header->dose.size_of_dimension_1 = rtog_line.ival;
	    }
	    break;
	case ekSIZEOFDIMENSION2:
	    if (imtype == evCTSCAN) {
		rc = set_ct_ival (rtog_header, &rtog_line, imno, &rtog_header->ct.size_of_dimension_2, "size 2");
		if (rc < 0) goto error_exit;
	    }
	    else if (imtype == evDOSE && imno == rtog_header->dose.imno) {
		rtog_header->dose.size_of_dimension_2 = rtog_line.ival;
	    }
	    break;
	case ekSIZEOFDIMENSION3:
	    if (imtype == evDOSE && imno == rtog_header->dose.imno) {
		rtog_header->dose.size_of_dimension_3 = rtog_line.ival;
	    }
	case ekXOFFSET:
	    if (imtype == evCTSCAN) {
		rc = set_ct_fval (rtog_header, &rtog_line, imno, &rtog_header->ct.x_offset, "x_offset");
		if (rc < 0) goto error_exit;
	    }
	    break;
	case ekYOFFSET:
	    if (imtype == evCTSCAN) {
		rc = set_ct_fval (rtog_header, &rtog_line, imno, &rtog_header->ct.y_offset, "y_offset");
		if (rc < 0) goto error_exit;
	    }
	    break;
	case ekCTOFFSET:
	    if (imtype == evCTSCAN) {
		rc = set_ct_ival (rtog_header, &rtog_line, imno, &rtog_header->ct.ct_offset, "ct_offset");
		if (rc < 0) goto error_exit;
	    }
	    break;
	case ekCTAIR:
	    if (imtype == evCTSCAN) {
		rc = set_ct_ival (rtog_header, &rtog_line, imno, &rtog_header->ct.ct_air, "ct_air");
		if (rc < 0) goto error_exit;
	    }
	    break;
	case ekCTWATER:
	    if (imtype == evCTSCAN) {
		rc = set_ct_ival (rtog_header, &rtog_line, imno, &rtog_header->ct.ct_water, "ct_water");
		if (rc < 0) goto error_exit;
	    }
	    break;
	case ekNUMBEROFDIMENSIONS:
	    if (imtype == evDOSE) {
		if (rtog_line.ival != 3) {
		    printf ("Error: dose doesn't have 3 dimensions (%d)\n", rtog_line.ival);
		    goto error_exit;
		}
	    }
	    break;
	case ekCOORD1OFFIRSTPOINT:
	    if (imtype == evDOSE && imno == rtog_header->dose.imno) {
		rtog_header->dose.coord_1_of_first_point = rtog_line.fval;
	    }
	    break;
	case ekCOORD2OFFIRSTPOINT:
	    if (imtype == evDOSE && imno == rtog_header->dose.imno) {
		rtog_header->dose.coord_2_of_first_point = rtog_line.fval;
	    }
	    break;
	case ekCOORD3OFFIRSTPOINT:
	    if (imtype == evDOSE && imno == rtog_header->dose.imno) {
		rtog_header->dose.coord_3_of_first_point = rtog_line.fval;
	    }
	    break;
	case ekHORIZONTALGRIDINTERVAL:
	    if (imtype == evDOSE && imno == rtog_header->dose.imno) {
		rtog_header->dose.horizontal_grid_interval = rtog_line.fval;
		if (rtog_header->dose.horizontal_grid_interval < 0.0) {
		    printf ("Error: dose horizontal_grid_interval is less than zero\n");
		    goto error_exit;
		}
	    }
	    break;
	case ekVERTICALGRIDINTERVAL:
	    if (imtype == evDOSE && imno == rtog_header->dose.imno) {
		rtog_header->dose.vertical_grid_interval = rtog_line.fval;
		if (rtog_header->dose.vertical_grid_interval > 0.0) {
		    printf ("Error: dose vertical_grid_interval is greater than zero\n");
		    goto error_exit;
		}
		/* Set to positive */
		rtog_header->dose.vertical_grid_interval = -rtog_header->dose.vertical_grid_interval;
	    }
	    break;
	case ekDEPTHGRIDINTERVAL:
	    if (imtype == evDOSE && imno == rtog_header->dose.imno) {
		rtog_header->dose.depth_grid_interval = rtog_line.fval;
		if (rtog_header->dose.depth_grid_interval < 0.0) {
		    printf ("Error: dose depth_grid_interval is less than zero\n");
		    goto error_exit;
		}
	    }
	    break;
	case ekDOSESCALE:
	    if (imtype == evDOSE && imno == rtog_header->dose.imno) {
		rtog_header->dose.dose_scale = rtog_line.fval;
	    }
	    break;
	case ekSTRUCTURENAME:
	    if (imtype == evSTRUCTURE) {
		strncpy (curr_struct->name, val, BUFLEN);
	    }
	    break;
	default:
	    /* Silently ignore */
	    break;
	}
    }

    printf ("CT IMAGES: %d - %d\n", rtog_header->ct.first_image, rtog_header->ct.last_image);
    printf ("Image res: (%d,%d)\n", rtog_header->ct.size_of_dimension_1, rtog_header->ct.size_of_dimension_2);
    printf ("Pixel size: (%f,%f)\n", rtog_header->ct.grid_1_units, rtog_header->ct.grid_2_units);
    printf ("Offset: (%f,%f)\n", rtog_header->ct.x_offset, rtog_header->ct.y_offset);
    printf ("Z (off,spc): (%f,%f)\n", rtog_header->ct.z_offset, rtog_header->ct.z_spacing);
    printf ("CT (off,air,wat): (%d,%d,%d)\n", rtog_header->ct.ct_offset, rtog_header->ct.ct_air, rtog_header->ct.ct_water);

    printf ("DOSE IMG: %d\n", rtog_header->dose.imno);
    printf ("Image res: (%d,%d,%d)\n", rtog_header->dose.size_of_dimension_1, rtog_header->dose.size_of_dimension_2, rtog_header->dose.size_of_dimension_3);
    printf ("Offset: (%f,%f,%f)\n", rtog_header->dose.coord_1_of_first_point, rtog_header->dose.coord_2_of_first_point, rtog_header->dose.coord_3_of_first_point);
    printf ("Spacing: (%f,%f,%f)\n", rtog_header->dose.horizontal_grid_interval, rtog_header->dose.vertical_grid_interval, rtog_header->dose.depth_grid_interval);
    printf ("Dose scale: (%f)\n", rtog_header->dose.dose_scale);

    fclose (fp);
    return;

error_exit:
    printf ("Error parsing RTOG header file: %s:= %s", key, val);
    exit (-1);
}

void
load_ct (RTOG_Header* rtog_header, Program_Parms* parms)
{
    int i;
    unsigned short* b;
    int num_slices = rtog_header->ct.last_image 
        - rtog_header->ct.first_image + 1;
    size_t slice_voxels = rtog_header->ct.size_of_dimension_1 
        * rtog_header->ct.size_of_dimension_2;
    int num_voxels = slice_voxels * num_slices;

    rtog_header->ct.image = (void*) malloc (
        sizeof(unsigned short) * num_voxels);
    if (!rtog_header->ct.image) {
	printf ("Error: could not malloc ct image\n");
	exit (-1);
    }
    b = ((unsigned short*) rtog_header->ct.image) + num_voxels;
    printf ("Reading CT slices...\n");
    for (i = 0; i < num_slices; i++) {
	FILE* fp;
	char fn[BUFLEN];
	int slice_no = rtog_header->ct.first_image + i;
	snprintf (fn, BUFLEN, "%s/aapm%04d", parms->indir, slice_no);
	fp = fopen (fn, "rb");
	if (!fp) {
	    printf ("Error: could not open file \"%s\" for read.\n", fn);
	    exit (-1);
	}

	b -= slice_voxels;
	size_t rc = fread (b, sizeof(unsigned short), slice_voxels, fp);

	if (rc != slice_voxels) {
	    printf ("Error reading from file %s (%d bytes read)\n", 
                fn, (int) rc);
	}
	fclose (fp);
    }
}

/* Just make the output directory without checking if it exists. 
   If the mkdir fails, we'll catch this when we try to write */
void
make_output_dir (Program_Parms* parms)
{
    mkdir (parms->outdir, 0777);
}

void
correct_ct (RTOG_Header* rtog_header)
{
    int i;
    int num_slices = rtog_header->ct.last_image - rtog_header->ct.first_image + 1;
    int slice_voxels = rtog_header->ct.size_of_dimension_1 * rtog_header->ct.size_of_dimension_2;
    int num_voxels = slice_voxels * num_slices;
    unsigned short* ctimg = (unsigned short*) rtog_header->ct.image;

    for (i = 0; i < num_voxels; i++) {
	unsigned short raw = ctimg[i];
	/* swap bytes */
	unsigned short byte1 = (raw & 0xFF00) >> 8;
	unsigned short byte2 = (raw & 0x00FF) << 8;
	raw = byte1 | byte2;
	/* correct for ct offset */
	raw = ((short) raw) - rtog_header->ct.ct_offset;
	ctimg[i] = raw;
    }
}

void
write_ct (RTOG_Header* rtog_header, Program_Parms* parms)
{
    FILE* fp;
    char fn[BUFLEN];
    int num_slices = rtog_header->ct.last_image - rtog_header->ct.first_image + 1;
    int slice_voxels = rtog_header->ct.size_of_dimension_1 * rtog_header->ct.size_of_dimension_2;
    int num_voxels = slice_voxels * num_slices;

    make_output_dir (parms);

    printf ("Writing CT volume...\n");
    snprintf (fn,BUFLEN,"%s/ct.mha",parms->outdir);
    fp = fopen (fn, "wb");
    if (!fp) {
	printf ("Error opening %s for write\n", fn);
	exit (-1);
    }
    fprintf (fp, mha_header_pat, 
#if defined (commentout)
	/* See file header for offset interpretation */
	rtog_header->ct.x_offset,
	rtog_header->ct.y_offset,
	rtog_header->ct.z_offset,
#endif
	rtog_header->ct.grid_1_units * 10.0 / 2.0,
	rtog_header->ct.grid_2_units * 10.0 / 2.0,
	rtog_header->ct.z_spacing * 10.0 / 2.0,
	rtog_header->ct.grid_1_units * 10.0,
	rtog_header->ct.grid_2_units * 10.0,
	rtog_header->ct.z_spacing * 10.0,
	rtog_header->ct.size_of_dimension_2, 
	rtog_header->ct.size_of_dimension_1, 
	rtog_header->ct.last_image - rtog_header->ct.first_image + 1,
	"MET_SHORT"
	 );
    fwrite (rtog_header->ct.image, sizeof(unsigned short), num_voxels, fp);
    fclose (fp);
}

void
free_ct (RTOG_Header* rtog_header)
{
    free (rtog_header->ct.image);
}

void
load_dose (RTOG_Header* rtog_header, Program_Parms* parms)
{
    FILE* fp;
    char fn[BUFLEN];
    size_t num_voxels = rtog_header->dose.size_of_dimension_1
	* rtog_header->dose.size_of_dimension_2
	* rtog_header->dose.size_of_dimension_3;

    rtog_header->dose.image = (unsigned short*) malloc (sizeof(unsigned short) * num_voxels);
    if (!rtog_header->dose.image) {
	printf ("Error: could not malloc dose image\n");
	exit (-1);
    }
    rtog_header->dose.fimage = (float*) malloc (sizeof(float) * num_voxels);
    if (!rtog_header->dose.fimage) {
	printf ("Error: could not malloc dose fimage\n");
	exit (-1);
    }

    printf ("Loading dose...\n");
    snprintf (fn, BUFLEN, "%s/aapm%04d", parms->indir, rtog_header->dose.imno);
    fp = fopen (fn, "rb");
    if (!fp) {
	printf ("Error: could not open file \"%s\" for read.\n", fn);
	exit (-1);
    }
    size_t rc = fread (rtog_header->dose.image, sizeof(unsigned short), 
        num_voxels, fp);
    if (rc != num_voxels) {
	printf ("Error: could not read dose from file %s (%d bytes read)\n", 
            fn, (int) rc);
	exit (-1);
    }
    fclose (fp);
}

void
correct_dose (RTOG_Header* rtog_header)
{
    int i;
    unsigned short* dsimg = rtog_header->dose.image;
    float* dsfimg = rtog_header->dose.fimage;
    int num_voxels = rtog_header->dose.size_of_dimension_1
	* rtog_header->dose.size_of_dimension_2
	* rtog_header->dose.size_of_dimension_3;

    for (i = 0; i < num_voxels; i++) {
	unsigned short raw = dsimg[i];
	/* swap bytes */
	unsigned short byte1 = (raw & 0xFF00) >> 8;
	unsigned short byte2 = (raw & 0x00FF) << 8;
	raw = byte1 | byte2;
	/* correct for dose scale */
	dsfimg[i] = ((short) raw) * rtog_header->dose.dose_scale;
    }
}

void
write_dose (RTOG_Header* rtog_header, Program_Parms* parms)
{
    FILE* fp;
    char fn[BUFLEN];
    int i;
    float* b;
    int num_slices = rtog_header->dose.size_of_dimension_3;
    int slice_voxels = rtog_header->dose.size_of_dimension_1 * rtog_header->dose.size_of_dimension_2;
    int num_voxels = rtog_header->dose.size_of_dimension_1
	* rtog_header->dose.size_of_dimension_2
	* rtog_header->dose.size_of_dimension_3;

    make_output_dir (parms);

    printf ("Writing DOSE volume...\n");
    snprintf (fn,BUFLEN,"%s/dose.mha",parms->outdir);
    fp = fopen (fn, "wb");
    if (!fp) {
	printf ("Error opening %s for write\n", fn);
	exit (-1);
    }
    fprintf (fp, mha_header_pat, 
#if defined (commentout)
	/* See file header for offset interpretation */
	rtog_header->dose.coord_1_of_first_point,
	rtog_header->dose.coord_2_of_first_point,
	rtog_header->dose.coord_3_of_first_point,
#endif
	rtog_header->dose.horizontal_grid_interval * 10 / 2.0,
	rtog_header->dose.vertical_grid_interval * 10 / 2.0,
	rtog_header->dose.depth_grid_interval * 10 / 2.0,
	rtog_header->dose.horizontal_grid_interval * 10,
	rtog_header->dose.vertical_grid_interval * 10,
	rtog_header->dose.depth_grid_interval * 10,
	rtog_header->dose.size_of_dimension_1, 
	rtog_header->dose.size_of_dimension_2, 
	rtog_header->dose.size_of_dimension_3,
	"MET_FLOAT"
	 );
    b = rtog_header->dose.fimage + num_voxels;
    for (i = 0; i < num_slices; i++) {
	b -= slice_voxels;
	fwrite (b, sizeof(float), slice_voxels, fp);
    }
    fclose (fp);
}

void
free_dose (RTOG_Header* rtog_header)
{
    free (rtog_header->dose.image);
    free (rtog_header->dose.fimage);
}

void
load_structure (STRUCTURE* structure, Program_Parms* parms)
{
    int nlev, scan_no, nseg, npts;
    Cxt_polyline_Slice* curr_ps = 0;
    Cxt_polyline* curr_pl = 0;
    int curr_pt = 0;
    char buf[BUFLEN];
    float x, y, z;

    FILE* fp;
    char fn[BUFLEN];
    snprintf (fn, BUFLEN, "%s/aapm%04d", parms->indir, structure->imno);
    fp = fopen (fn, "rb");
    if (!fp) {
	printf ("Error: could not open file \"%s\" for read.\n", fn);
	exit (-1);
    }

    /* Parse structure file */
    while (fgets (buf,BUFLEN,fp)) {
	if (1 == sscanf (buf,"\"NUMBER OF LEVELS\" %d",&nlev)) {
	    /* Yeah, whatever */
	} else if (1 == sscanf (buf,"\"SCAN # \" %d",&scan_no)) {
	    structure->num_slices ++;
	    structure->pslist = (Cxt_polyline_Slice*) realloc (structure->pslist, 
			structure->num_slices * sizeof (Cxt_polyline_Slice));
	    curr_ps = &structure->pslist[structure->num_slices-1];
	    curr_ps->slice_no = scan_no;
	    curr_ps->num_polyline = 0;
	    curr_ps->pllist = 0;
	} else if (1 == sscanf (buf,"\"NUMBER OF SEGMENTS \" %d",&nseg)) {
	    /* Yeah, whatever */
	} else if (1 == sscanf (buf, "\"NUMBER OF POINTS \" %d",&npts)) {
	    curr_ps->num_polyline ++;
	    curr_ps->pllist = (Cxt_polyline*) realloc (curr_ps->pllist,
		curr_ps->num_polyline * sizeof (Cxt_polyline));
	    curr_pl = &curr_ps->pllist[curr_ps->num_polyline-1];
	    curr_pl->num_vertices = npts;
	    curr_pl->x = (float*) malloc (npts * sizeof(float));
	    curr_pl->y = (float*) malloc (npts * sizeof(float));
	    curr_pl->z = (float*) malloc (npts * sizeof(float));
	    curr_pt = 0;
	} else if (3 == sscanf (buf, "%g, %g, %g", &x, &y, &z)) {
	    if (curr_pt >= npts) {
		printf ("Error parsing structure file (too many points in polyline)\nfile=%s\n", fn);
		exit (-1);
	    }
	    curr_pl->x[curr_pt] = x;
	    curr_pl->y[curr_pt] = y;
	    curr_pl->z[curr_pt] = z;
	    curr_pt++;
	} else {
	    printf ("Error parsing structure file\nfile=%s\nline=%s\n", fn, buf);
	    exit (-1);
	}
    }

    fclose (fp);
}

void
load_skin (RTOG_Header* rtog_header, Program_Parms* parms)
{
    int i;

    /* Find the skin structure */
    for (i = 0; i < rtog_header->structures.num_structures; i++) {
	if (!strcmp (rtog_header->structures.slist[i].name, "SKIN")) {
	    printf ("Found skin: %d/%d im=%d\n", i, 
		rtog_header->structures.num_structures,
		rtog_header->structures.slist[i].imno);
	    break;
	}
    }
    if (i == rtog_header->structures.num_structures) {
	printf ("Error: SKIN structure not found\n");
	exit (-1);
    }

    /* Load the skin polylines */
    load_structure (&rtog_header->structures.slist[i], parms);

    /* Save the index to skin */
    rtog_header->structures.skin_no = i;
}

void
render_slice (RTOG_Header* rtog_header, unsigned char* slice_img, 
    unsigned char* acc_img, Cxt_polyline_Slice* ps)
{
#if defined (commentout)
    int i, j;
    int slice_voxels = rtog_header->ct.size_of_dimension_1 * rtog_header->ct.size_of_dimension_2;

    float offset[2] = {
	(float) (rtog_header->ct.grid_1_units / 2.0),
	(float) (rtog_header->ct.grid_2_units  / 2.0)
    };
    float spacing[2] = {
	rtog_header->ct.grid_1_units,
	rtog_header->ct.grid_2_units
    };
    plm_long dims[2] = {
	rtog_header->ct.size_of_dimension_2,
	rtog_header->ct.size_of_dimension_1
    };

    for (i = 0; i < ps->num_polyline; i++) {
	memset (acc_img, 0, dims[0]*dims[1]*sizeof(unsigned char));
	rasterize_slice (acc_img, dims, spacing, offset, 
	    ps->pllist[i].num_vertices,
	    ps->pllist[i].x,
	    ps->pllist[i].y);
	for (j = 0; j < slice_voxels; j++) {
	    slice_img[j] ^= acc_img[j];
	}
    }
#endif
}

void
render_skin (RTOG_Header* rtog_header, Program_Parms* parms)
{
    int i;
    int num_slices = rtog_header->ct.last_image - rtog_header->ct.first_image + 1;
    int slice_voxels = rtog_header->ct.size_of_dimension_1 * rtog_header->ct.size_of_dimension_2;
    int num_voxels = slice_voxels * num_slices;
    STRUCTURE* skin = &rtog_header->structures.slist[rtog_header->structures.skin_no];
    unsigned char* acc_img = (unsigned char*) malloc (sizeof(unsigned char) * slice_voxels);

    rtog_header->structures.skin_image = (unsigned char*) malloc (sizeof(unsigned char) * num_voxels);
    memset (rtog_header->structures.skin_image, 0, sizeof(unsigned char) * num_voxels);

    for (i = 0; i < skin->num_slices; i++) {
	int slice_no = skin->pslist[i].slice_no;  /* First slice_no is 1 (no slice 0) */
	unsigned char* slice_img = &rtog_header->structures.skin_image[(num_slices-slice_no) * slice_voxels];
	render_slice (rtog_header, slice_img, acc_img, &skin->pslist[i]);
    }
    free (acc_img);
}

void
correct_skin (RTOG_Header* rtog_header)
{
    int i, j, k;
    int num_slices = rtog_header->ct.last_image - rtog_header->ct.first_image + 1;
    int slice_voxels = rtog_header->ct.size_of_dimension_1 * rtog_header->ct.size_of_dimension_2;

    for (i = 0; i < num_slices; i++) {
	unsigned char* slice_img = &rtog_header->structures.skin_image[i * slice_voxels];
	for (j = 0; j < rtog_header->ct.size_of_dimension_1 / 2; j++) {
	    unsigned char* row1 = &slice_img[j*rtog_header->ct.size_of_dimension_2];
	    unsigned char* row2 = &slice_img[(rtog_header->ct.size_of_dimension_1-j-1)*rtog_header->ct.size_of_dimension_2];
	    for (k = 0; k < rtog_header->ct.size_of_dimension_2; k++) {
		unsigned char tmp = row1[k];
		row1[k] = row2[k];
		row2[k] = tmp;
	    }
	}
    }
}

void
write_skin (RTOG_Header* rtog_header, Program_Parms* parms)
{
    FILE* fp;
    char fn[BUFLEN];
    int num_slices = rtog_header->ct.last_image - rtog_header->ct.first_image + 1;
    int slice_voxels = rtog_header->ct.size_of_dimension_1 * rtog_header->ct.size_of_dimension_2;
    int num_voxels = slice_voxels * num_slices;

    make_output_dir (parms);

    printf ("Writing patient mask...\n");
    snprintf (fn,BUFLEN,"%s/mask.mha",parms->outdir);
    fp = fopen (fn, "wb");
    if (!fp) {
	printf ("Error opening %s for write\n", fn);
	exit (-1);
    }
    fprintf (fp, mha_header_pat, 
	rtog_header->ct.grid_1_units * 10.0 / 2.0,
	rtog_header->ct.grid_2_units * 10.0 / 2.0,
	rtog_header->ct.z_spacing * 10.0 / 2.0,
	rtog_header->ct.grid_1_units * 10.0,
	rtog_header->ct.grid_2_units * 10.0,
	rtog_header->ct.z_spacing * 10.0,
	rtog_header->ct.size_of_dimension_2, 
	rtog_header->ct.size_of_dimension_1, 
	rtog_header->ct.last_image - rtog_header->ct.first_image + 1,
	"MET_UCHAR"
	 );
    fwrite (rtog_header->structures.skin_image, sizeof(unsigned char), num_voxels, fp);
    fclose (fp);
}

void
free_skin (RTOG_Header* rtog_header)
{
    free (rtog_header->structures.skin_image);
}

void
write_rtog_dose (Program_Parms* parms)
{
#if defined (commentout)
    Volume* vol;
    FILE* fp;
    plm_long i;
    int slice_voxels;
    unsigned short *img_us, *p;
    float *img_f;

    vol = read_mha (parms->mha_in_fn);
    if (!vol) exit (-1);

    printf ("Scaling and converting (%ld pix)...\n", (long) vol->npix);
    img_f = (float*) vol->img;
    img_us = (unsigned short*) malloc (vol->npix * sizeof(unsigned short));
    for (i = 0; i < vol->npix; i++) {
	float raw = img_f[i];

	/* correct for scale */
	unsigned short raw_us = (unsigned short) (raw / parms->scale);

	/* swap bytes */
	unsigned short byte1 = (raw_us & 0xFF00) >> 8;
	unsigned short byte2 = (raw_us & 0x00FF) << 8;
	raw_us = byte1 | byte2;
	img_us[i] = raw_us;
    }

    printf ("Writing...\n");
    fp = fopen (parms->rtog_dose_out_fn, "wb");
    slice_voxels = vol->dim[0] * vol->dim[1];
    p = img_us + vol->npix;
    for (i = 0; i < vol->dim[2]; i++) {
	p -= slice_voxels;
	fwrite (p, sizeof(unsigned short), slice_voxels, fp);
    }
    fclose (fp);

    free (img_us);
    free (vol->img);
    free (vol);
#endif
}

#if defined (commentout)
int
main (int argc, char* argv[])
{
    Program_Parms parms;
    RTOG_Header rtog_header;

    parse_args (&parms, argc, argv);
    load_rtog_header (&rtog_header, &parms);

    /* Convert the CT cube */
    load_ct (&rtog_header, &parms);
    correct_ct (&rtog_header);
    write_ct (&rtog_header, &parms);
    free_ct (&rtog_header);

    /* Convert the DOSE cube */
    if (rtog_header.dose.imno > 0) {
	load_dose (&rtog_header, &parms);
	correct_dose (&rtog_header);
	write_dose (&rtog_header, &parms);
	free_dose (&rtog_header);
    }
#if defined (commentout)
#endif

    /* Convert the SKIN contours to a mask image */
    load_skin (&rtog_header, &parms);
    render_skin (&rtog_header, &parms);
    correct_skin (&rtog_header);
    write_skin (&rtog_header, &parms);
    free_skin (&rtog_header);

    return 0;
}
#endif
