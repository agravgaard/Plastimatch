/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

#include "logfile.h"
#include "plm_math.h"
#include "rtss_contour.h"

Rtss_contour::Rtss_contour ()
{
    this->slice_no = -1;
    this->ct_slice_uid = "";
    this->num_vertices = 0;
    this->x = 0;
    this->y = 0;
    this->z = 0;
}

Rtss_contour::~Rtss_contour ()
{
    free (this->x);
    free (this->y);
    free (this->z);

    this->slice_no = -1;
    this->ct_slice_uid = "";
    this->num_vertices = 0;
    this->x = 0;
    this->y = 0;
    this->z = 0;
}

void
Rtss_contour::find_direction_cosines ()
{
    /* Need at least three points to find slice plane */
    if (this->num_vertices < 3) {
        lprintf ("Failed to find DC, not enough points\n");
        return;
    }

    /* Find triangle with longest minimum legs using greedy search */
    float p1[3], p2[3], p3[3];
    p1[0] = this->x[0];
    p1[1] = this->y[0];
    p1[2] = this->z[0];
    p2[0] = this->x[1];
    p2[1] = this->y[1];
    p2[2] = this->z[1];
    p3[0] = this->x[2];
    p3[1] = this->y[2];
    p3[2] = this->z[2];
    float p12_dist = vec3_distsq (p1, p2);
    float p23_dist = vec3_distsq (p2, p3);
    float p31_dist = vec3_distsq (p3, p1);
    printf ("%g %g %g\n", p12_dist, p23_dist, p31_dist);
    for (size_t k = 3; k < this->num_vertices; k++) {
        float pk[3];
        pk[0] = this->x[k];
        pk[1] = this->y[k];
        pk[2] = this->z[k];
        float p1k_dist = vec3_distsq (p1, pk);
        float p2k_dist = vec3_distsq (p2, pk);
        float p3k_dist = vec3_distsq (p3, pk);
        if (std::min (p1k_dist, p3k_dist) > std::min (p12_dist, p23_dist)) {
            vec3_copy (p2, pk);
            p12_dist = p1k_dist;
            p23_dist = p3k_dist;
            printf ("%g %g %g\n", p12_dist, p23_dist, p31_dist);
            continue;
        }
        if (std::min (p2k_dist, p3k_dist) > std::min (p12_dist, p31_dist)) {
            vec3_copy (p1, pk);
            p12_dist = p2k_dist;
            p31_dist = p3k_dist;
            printf ("%g %g %g\n", p12_dist, p23_dist, p31_dist);
            continue;
        }
        if (std::min (p2k_dist, p1k_dist) > std::min (p23_dist, p31_dist)) {
            vec3_copy (p3, pk);
            p23_dist = p2k_dist;
            p31_dist = p1k_dist;
            printf ("%g %g %g\n", p12_dist, p23_dist, p31_dist);
            continue;
        }
    }
    p12_dist = vec3_distsq (p1, p2);
    p23_dist = vec3_distsq (p2, p3);
    p31_dist = vec3_distsq (p3, p1);
    printf ("Final: %g %g %g\n", p12_dist, p23_dist, p31_dist);
    printf ("[%g %g %g]\n[%g %g %g]\n[%g %g %g]\n",
        p1[0], p1[1], p1[2], 
        p2[0], p2[1], p2[2], 
        p3[0], p3[1], p3[2]);
}
