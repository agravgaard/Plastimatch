/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "file_util.h"

#define NBUF 10000
static double x[NBUF], y[NBUF], sum[NBUF];

static void usage()
{
    printf("Usage: plastimatch xio-dvh rtog_dvh_file output_directory\n");
}

void do_command_xio_dvh(int argc, char *argv[])
{
    FILE *fp, *fpout;
    int n;
    double s;
    char buf[255];
    char structurename[255];
    char patientid[255];
    char outfilename[255];
    char output_dir[255];
    char input_fn[255];
    int rc;

    if (argc!=4) { usage(); return; }

    strcpy(input_fn, argv[2]);
    strcpy(output_dir, argv[3]);

    fp = fopen(input_fn, "r");
    if (!fp) {fprintf(stderr, "cannot open input file %s\n", input_fn); exit(1);}

    /* If mkdir fails, write will fail too and throw an error later on */
    make_directory (output_dir);

    while(!feof(fp)) {

	fgets(buf, 200, fp); //XiO Dose Volume Histogram
	if (feof(fp)) break;

	fgets(patientid, 200, fp); // patient ID, abcde1234
	fgets(buf, 200, fp); // plan
	fgets(structurename, 200, fp); // structure name
	fgets(buf, 200, fp); // N.NN mm resolution

	fgets(buf, 200, fp); // number of bins
	rc = sscanf(buf, "%d bins", &n);
	if (rc!=1) {fprintf(stderr, "sscanf failed, reading number of bins from %s\n", buf); exit(1);}
	if (n >=NBUF) {fprintf(stderr, "too many bins in file (%d), limit %d\n", n, NBUF); exit(1);}

	fgets(buf, 200, fp); // date
	fgets(buf, 200, fp); // table header: min bin dose (cGy), bin volume (cc)

	//remove \n after fgets
	patientid[strlen(patientid)-1]=0;
	structurename[strlen(structurename)-1]=0;

	// change white space and dot to underscore to avoid problems with filenames 
	for (unsigned int i=0; i<strlen(structurename); i++) {
	    if (structurename[i]==' ' || structurename[i]=='.' ) structurename[i]='_';
	}

	// WIN32 recognizes / as path separator
	sprintf(outfilename, "%s/%s-%s.txt",output_dir, patientid, structurename);

	fprintf(stderr, "%s %s %d\n", patientid, structurename, n);

	for(int i=0; i<n; i++) {
	    x[i]=0; y[i]=0;
	    fgets(buf, 200, fp);
	    rc = sscanf(buf, "%lf, %lf", &x[i], &y[i]);
	    if (rc!=2) {fprintf(stderr,"sscanf failed, %s\n", buf);}
	}

	// integrate
	s = 0;
	for (int i=0;i<n;i++) {s+=y[i]; sum[i]=s;}

	// rescale and print out
	fpout = fopen(outfilename, "w");
	if (!fpout) {fprintf(stderr, "cannot open output file %s\n", outfilename); exit(1);}
	for (int i=0;i<n;i++) {
	    fprintf(fpout, "%f\t%f\n", x[i], 100./(sum[n-1]-sum[0])*(-sum[i]+sum[n-1]));
	}
	fclose(fpout);

	if (feof(fp)) break;
    }
    return;
}
