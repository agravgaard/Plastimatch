/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* Converts ITK mha format to a raw cube */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plm_config.h"

#define BUFLEN 1024

void swap_short(unsigned char *pntr)
{
    unsigned char b0, b1;

    b0 = *pntr;
    b1 = *(pntr+1);
        
    *pntr = b1;
    *(pntr+1) = b0;
}

int main (int argc, char* argv[])
{
    FILE *fp1, *fp2;
    int nx, ny, nz, nb = 0;
    long bytesz;
    float sx, sy, sz;
    int i;
    unsigned char c;
    char buf[1024];

    if (argc != 3) {
        printf ("Usage: %s infile outfile\n", argv[0]);
        exit (1);
    }
    if (!(fp1 = fopen(argv[1],"rb"))) {
        printf ("Error opening file \"%s\" for read\n", argv[1]);
        exit (1);
    }
    if (!(fp2 = fopen(argv[2],"wb"))) {
        printf ("Error opening file \"%s\" for write\n", argv[2]);
        exit (1);
    }

    for (i=0; i<30; i++) { // 30 lines of MHA should have passed the header 
        if (fgets(buf,1024,fp1) == NULL) {
            printf ("File error.\n");
            exit (1);
        }
        if (strstr(buf, "DimSize")!=NULL) {
            sscanf(&(buf[9]), "%d%d%d", &nx, &ny, &nz);
        } else if (strstr(buf, "ElementSpacing")!=NULL) {
            sscanf(&(buf[16]), "%f%f%f", &sx, &sy, &sz);
        } else if (strstr(buf, "ElementType")!=NULL) {
            if (!strcmp(buf, "ElementType = MET_SHORT\n")) {
                nb = 2;
            } else if (!strcmp(buf, "ElementType = MET_USHORT\n")) {
                nb = 2;
            } else if (!strcmp(buf, "ElementType = MET_UCHAR\n")) {
                nb = 1;
            } else if (!strcmp(buf, "ElementType = MET_FLOAT\n")) {
                nb = 4;
            }
        }
    }

    printf("Writing...\n");
    //image
    bytesz = (long) nx * ny * nz * nb;
    printf("%ld bytes of image data!\n", bytesz);
    fseek(fp1, -bytesz, SEEK_END);

    fread(&c,1,1,fp1);
    while (!feof(fp1)) {
        fwrite(&c,1,1,fp2);
        fread(&c,1,1,fp1);
    }

    fclose(fp1);
    fclose(fp2);
    return 0;
}
