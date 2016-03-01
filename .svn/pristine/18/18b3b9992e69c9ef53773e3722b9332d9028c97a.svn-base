/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "landmark_diff.h"
#include "raw_pointset.h"

static float
get_sd (
    float *P1,
    float *P2
)
{
    return sqrt (
             (P2[0]-P1[0])*(P2[0]-P1[0]) +
             (P2[1]-P1[1])*(P2[1]-P1[1]) +
             (P2[2]-P1[2])*(P2[2]-P1[2])
           );
}

static void
print_pointset (Raw_pointset *rps)
{
    for (int i=0; i<rps->num_points; i++) {
        printf ("  [%i] %f, %f, %f\n", i,
            rps->points[3*i+0],
            rps->points[3*i+1],
            rps->points[3*i+2]
        );
    }
}

static void
print_sd_stats (
    Raw_pointset *rps0,
    Raw_pointset *rps1
)
{
    float *sd = (float*)malloc (rps0->num_points * sizeof(float));

    float avg = 0.0f;
    for (int i=0; i<rps0->num_points; i++) {
        sd[i] = get_sd (&rps0->points[3*i], &rps1->points[3*i]);
        avg += sd[i];
    }
    avg /= rps0->num_points;

    float var = 0.0f;
    for (int i=0; i<rps0->num_points; i++) {
        var += (sd[i]-avg)*(sd[i]-avg);
        printf ("  [%i] %f\n", i, sd[i]);
    }
    var /= rps0->num_points;

    free (sd);

    printf ("\n");
    printf ("  Avg: %f\n", avg);
    printf ("  Var: %f\n", var);
    printf ("Stdev: %f\n", sqrt(var));
}

int
landmark_diff (
    Raw_pointset *rps0,
    Raw_pointset *rps1
)
{
    /* make sure both sets have same # of landmarks */
    if (rps0->num_points != rps1->num_points) {
        printf ("error: sets must contain same number of landmarks\n");
        return -1;
    }

    printf ("1st Pointset:\n");
    print_pointset (rps0);
    printf ("\n");

    printf ("2nd Pointset:\n");
    print_pointset (rps1);
    printf ("\n");

    printf ("Separation Distances:\n");
    print_sd_stats (rps0, rps1);


    return 0;
}
