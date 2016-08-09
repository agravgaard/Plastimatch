/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdlib.h>
#include <stdio.h>
#include "nkidecompress.h"

#include "file_util.h"
#include "logfile.h"
#include "nki_io.h"
#include "plm_int.h"
#include "volume.h"

#define LINELEN 512

Volume* 
nki_load (const char* filename)
{
    char linebuf[LINELEN];

    FILE *fp = fopen (filename,"rb");
    if (!fp) {
	fprintf (stderr, "File %s not found\n", filename);
	return 0;
    }
    fprintf(stdout, "reading %s\n", filename);

    /* Find file size */   
    uint64_t fsize = file_size (filename);

    /* Make a buffer large enough to hold file */
    signed char *src = (signed char*) malloc (
        fsize * sizeof(signed char));

    /* Loop, reading lines from file */
    int dim1 = -1, dim2 = -1, dim3 = -1;
    bool have_start_pos = false;
    int junk;
    fpos_t pos;
    size_t compressed_size = 0;
    while (fgetpos (fp, &pos), fgets (linebuf, LINELEN, fp))
    {
	if (sscanf (linebuf, "dim1=%d", &dim1) == 1) {
            continue;
        }
	if (sscanf (linebuf, "dim2=%d", &dim2) == 1) {
            continue;
        }
	if (sscanf (linebuf, "dim3=%d", &dim3) == 1) {
            continue;
        }
	if (sscanf (linebuf, "nki_compression=%d", &junk) == 1) {
            /* End of ascii header, look for beginning of compressed data. */
            fsetpos (fp, &pos);
            int prev = fgetc (fp);
            do {
                int curr = fgetc (fp);
                if (prev == 0x0c && curr == 0x0c) {
                    have_start_pos = true;
                    break;
                }
                prev = curr;
            } while (prev != EOF);

            /* Found beginning of compressed data, slurp up rest of file into a buffer */
            signed char *p = src;
            while (1) {
                size_t bytes_read = fread (p, 1, 2048, fp);
                p += bytes_read;
                compressed_size += bytes_read;
                if (bytes_read != 2048) break;
            }
            /* Done! */
            break;
        }
    }
    fclose (fp);

    if (dim1 == -1 || dim2 == -1 || dim3 == -1 || !have_start_pos) {
        lprintf ("Failure to parse NKI header\n");
        free (src);
        return 0;
    }

    short int *nki = (short*) malloc (sizeof(short) * dim1 * dim2 * dim3);

    int rc = nki_private_decompress (nki, src, compressed_size);
    free (src);

    if (rc == 0) {
        /* Decompression failure */
        lprintf ("NKI decompression failure.\n");
        free (nki);
        return 0;
    }

    Volume *vol  = new Volume;
    vol->pix_size = 2;
    vol->pix_type = PT_SHORT;
    vol->spacing[0] = 1;
    vol->spacing[1] = 1;
    vol->spacing[2] = 1;
    vol->set_direction_cosines (0);

    /* Shuffle pixels */
    plm_long nki_dim[3] = {dim1,dim2,dim3};
    plm_long tgt_dim[3] = {dim3,dim2,dim1};
    short *tgt = (short*) malloc (sizeof(short) * dim1 * dim2 * dim3);
    for (int k = 0; k < dim1; k++) {
        for (int j = 0; j < dim2; j++) {
            for (int i = 0; i < dim3; i++) {
                tgt[volume_index(tgt_dim,i,j,dim1-k-1)]
                    = nki[volume_index(nki_dim,k,j,i)];
            }
        }
    }
    vol->dim[0] = dim3;
    vol->dim[1] = dim2;
    vol->dim[2] = dim1;
    vol->origin[0] = -0.5 * dim3 + 0.5;
    vol->origin[1] = -0.5 * dim2 + 0.5;
    vol->origin[2] = -0.5 * dim1 + 0.5;
    vol->img = (void*) tgt;
    vol->npix = dim1*dim2*dim3; //bug fixed by YKPark 20131029

    free (nki);

    return vol;
}
