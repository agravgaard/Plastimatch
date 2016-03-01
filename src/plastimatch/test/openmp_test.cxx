/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#if (OPENMP_FOUND)
#include <omp.h>
#endif

#include "plm_timer.h"

#define LOOP1 2000
#define LOOP2 20000

void
initialize_vector (double input[LOOP1])
{
    int i;
    for (i = 0; i < LOOP1; i++) {
	input[i] = sqrt ((double) i);
    }
}

#if (OPENMP_FOUND)
void
display_num_threads (void)
{
    int nthreads, tid;

#pragma omp parallel private(tid)
    {
	/* Obtain and print thread id */
	tid = omp_get_thread_num();
	//printf("Hello World from thread = %d\n", tid);

	/* Only master thread does this */
	if (tid == 0) 
	{
	    nthreads = omp_get_num_threads();
	    printf("Number of threads = %d\n", nthreads);
	}

    }  /* All threads join master thread and terminate */
}

void
speedtest_openmp_1 (double output[LOOP1], double input[LOOP1])
{
    int i;
#pragma omp parallel for
    for (i = 0; i < LOOP1; i++) {
	int j;
	double d1 = input[i];
	double d2 = 1.0;
	for (j = 0; j < LOOP2; j++) {
	    d1 += 0.872013;
	    d2 *= d1;
	    if (d2 > 1.0) {
		d2 -= floor(d2);
	    }
	}
	output[i] = d2;
    }
}

void
speedtest_openmp_2 (double output[LOOP1], double input[LOOP1])
{
    int i;
    for (i = 0; i < LOOP1; i++) {
	int j;
	double d1 = input[i];
	double d2 = 1.0;
	for (j = 0; j < LOOP2; j++) {
	    d1 += 0.872013;
	    d2 *= d1;
	    if (d2 > 1.0) {
		d2 -= floor(d2);
	    }
	}
	output[i] = d2;
    }
}
#endif /* OPENMP_FOUND */

int
main (int argc, char* argv[])
{
    Plm_timer* timer = new Plm_timer;

    double input[LOOP1], output[LOOP1];

#if (OPENMP_FOUND)
    display_num_threads ();
    initialize_vector (input);
    timer->start ();
    speedtest_openmp_1 (output, input);
    printf ("Time [openmp] = %f seconds\n", timer->report ());

    initialize_vector (input);
    timer->start ();
    speedtest_openmp_2 (output, input);
    printf ("Time [serial] = %f seconds\n", timer->report ());

    delete timer;

#else
    printf ("Sorry, openmp was not supported by your compiler\n");
#endif

    return 0;
}
