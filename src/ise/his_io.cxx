/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <stdio.h>
#include "his_io.h"

bool
his_read (void *buf, int x_size, int y_size, const char *fn)
{
    FILE *fp = fopen (fn, "rb");
    if (!fp) return false;
    
    unsigned short rows;
    unsigned short cols;
    short ulx, uly, brx, bry;

    size_t rc;
    unsigned short file_type;
    rc = fread (&file_type, sizeof(unsigned short), 1, fp);
    if (rc != 1) goto error_exit;
    if (file_type != 0x7000) goto error_exit;

    fseek (fp, 12, SEEK_SET);

    rc = fread (&ulx, sizeof(short), 1, fp);
    if (rc != 1) goto error_exit;
    rc = fread (&uly, sizeof(short), 1, fp);
    if (rc != 1) goto error_exit;
    rc = fread (&brx, sizeof(short), 1, fp);
    if (rc != 1) goto error_exit;
    rc = fread (&bry, sizeof(short), 1, fp);
    if (rc != 1) goto error_exit;

    rows = bry - uly + 1;
    cols = brx - ulx + 1;

    if (rows != y_size) goto error_exit;
    if (cols != x_size) goto error_exit;

    fseek (fp, 68 + 32, SEEK_SET);

    rc = fread (buf, sizeof(short), x_size*y_size, fp);
    if (rc != x_size*y_size) goto error_exit;

    /* Success */
    fclose (fp);
    return true;

error_exit:
    fclose (fp);
    return false;
}


bool
is_his (int x_size, int y_size, const char *fn)
{
    FILE *fp = fopen (fn, "rb");
    if (!fp) return false;
    
    unsigned short rows;
    unsigned short cols;
    short ulx, uly, brx, bry;

    size_t rc;
    unsigned short file_type;
    rc = fread (&file_type, sizeof(unsigned short), 1, fp);
    if (rc != 1) goto error_exit;
    if (file_type != 0x7000) goto error_exit;

    fseek (fp, 12, SEEK_SET);

    rc = fread (&ulx, sizeof(short), 1, fp);
    if (rc != 1) goto error_exit;
    rc = fread (&uly, sizeof(short), 1, fp);
    if (rc != 1) goto error_exit;
    rc = fread (&brx, sizeof(short), 1, fp);
    if (rc != 1) goto error_exit;
    rc = fread (&bry, sizeof(short), 1, fp);
    if (rc != 1) goto error_exit;

    rows = bry - uly + 1;
    cols = brx - ulx + 1;

    if (rows != y_size) goto error_exit;
    if (cols != x_size) goto error_exit;

    /* Success */
    fclose (fp);
    return true;

error_exit:
    fclose (fp);
    return false;
}
