/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* Convert's Shinichiro's raw format to ITK mha format */
#include <stdio.h>
#include <stdlib.h>
#include "plm_config.h"

#define BUFLEN 1024

/*char header_pat[] = 
    "ObjectType = Image\n"
    "NDims = 3\n"
    "BinaryData = True\n"
    "BinaryDataByteOrderMSB = False\n"
    "Offset = %s\n"
    "ElementSpacing = %s\n"
    "DimSize = %s\n"
//    "AnatomicalOrientation = RAI\n"
    "AnatomicalOrientation = LAI\n"
    "ElementType = MET_SHORT\n"
    "ElementDataFile = LOCAL\n"
    ;
*/
char header_pat[] = 
    "ObjectType = Image\n"
    "NDims = 3\n"
    "BinaryData = True\n"
    "BinaryDataByteOrderMSB = False\n"
    "TransformMatrix = 1 0 0 0 1 0 0 0 1\n"
    "Offset = %s\n"
    "ElementSpacing = %s\n"
    "DimSize = %s\n"
    "AnatomicalOrientation = RPI\n"
    "ElementType = MET_UCHAR\n"
    "ElementDataFile = LOCAL\n"
    ;

char *dm;
char *ps;
char *os = "0 0 0";

int main (int argc, char* argv[])
{
    FILE *fp1, *fp2;
    unsigned char c;

    if (argc > 6 || argc <= 4) {
	printf ("Usage: %s infile outfile dims pix_spacing [offset_string]\n", argv[0]);
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
    dm = argv[3]; //printf("dim = %s\n ", dm);
    ps = argv[4]; //printf("ps = %s\n", ps);
    if (argc == 6) {
	os = argv[5];
    }
    fprintf (fp2, header_pat, os, ps, dm);

    
    while (1 == fread(&c,1,1,fp1)) {
	fwrite(&c,1,1,fp2);
    }

    fclose (fp1);
    fclose (fp2);
    return 0;
}
