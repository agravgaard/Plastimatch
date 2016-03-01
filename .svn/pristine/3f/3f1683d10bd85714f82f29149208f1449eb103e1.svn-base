/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"

#include "compute_distance.h"

#define BUFLEN 2048

void
calculate_mass (SURFACE* surface)
{
    //computes center of mass for each triangle
    float x_sum = 0;
    float y_sum = 0;
    float z_sum = 0;

    VERTICES_LIST* vertices;
    TRIANGLE_LIST* triangles;
    MASS* center_mass;

    vertices = &surface->vertices;
    triangles = &surface->triangles;
    center_mass = &surface->centres;

    center_mass->num_triangles = triangles->num_triangles;

    for (int i = 0; i < triangles->num_triangles; i++) {
        center_mass->x = (float*) realloc (center_mass->x, (i + 1) * sizeof(float));
        center_mass->y = (float*) realloc (center_mass->y, (i + 1) * sizeof(float));
        center_mass->z = (float*) realloc (center_mass->z, (i + 1) * sizeof(float));

        x_sum = vertices->x[triangles->first[i] - 1] + vertices->x[triangles->second[i] - 1] + vertices->x[triangles->third[i] - 1];
        y_sum = vertices->y[triangles->first[i] - 1] + vertices->y[triangles->second[i] - 1] + vertices->y[triangles->third[i] - 1];
        z_sum = vertices->z[triangles->first[i] - 1] + vertices->z[triangles->second[i] - 1] + vertices->z[triangles->third[i] - 1];

        center_mass->x[i] = x_sum / 3;
        center_mass->y[i] = y_sum / 3;
        center_mass->z[i] = z_sum / 3;

        //printf("INDEX: %d %d %d\n",triangles->first[i],triangles->second[i],triangles->third[i]);
        //printf("CENTER OF MASS: %f %f %f\n",center_mass->x[i],center_mass->y[i],center_mass->z[i]);
    }
}

void
cp (SURFACE* surface)
{
    float x_min = 0;
    float y_min = 0;
    float z_min = 0;
    float d = 0;
    float d_prev = 0;
    //FILE* test;

    VERTICES_LIST* MDpoints;
    MASS* center_mass;
    CORR* corresp;

    corresp = &surface->correspondance;
    MDpoints = &surface->MDpoints;
    center_mass = &surface->centres;
    corresp->num_points = MDpoints->num_vertices;
    //printf("NUM CORR: %d \t NUM MDpoints:%d",corresp->num_points,MDpoints->num_vertices);
    corresp->corrpoint_index = (int*) realloc (corresp->corrpoint_index,
                                               corresp->num_points * sizeof(int));
    //test=fopen("testing_bari.txt","w");
    for (int k = 0; k < MDpoints->num_vertices; k++) {
        d = 999999999;
        d_prev = 999999999;
        for (int i = 0; i < center_mass->num_triangles; i++) {
            x_min = center_mass->x[i] - MDpoints->x[k];
            y_min = center_mass->y[i] - MDpoints->y[k];
            z_min = center_mass->z[i] - MDpoints->z[k];
            d = (x_min * x_min) + (y_min * y_min) + (z_min * z_min);

            if (d < d_prev) {
                corresp->corrpoint_index[k] = i;
                d_prev = d;
            }
            x_min = 0;
            y_min = 0;
            z_min = 0;
        }
        //fprintf(test,"%f %f %f\n",center_mass->x[ center_mass->corrpoint_index[k]], center_mass->y[ center_mass->corrpoint_index[k]],center_mass->z[ center_mass->corrpoint_index[k]]);
    }
    //printf("NUM CORR: %d \t NUM MDpoints:%d\n",corresp->num_points,MDpoints->num_vertices);

    //for(int r=0; r<center_mass->num_triangles/100; r++)
    //		printf("CORR: %d\n",center_mass->corrpoint_index[r]);
    //fclose(test);
}

void
compute_plane (SURFACE* surface)
{
    float matrix[3][3];
    int i = 0;

    VERTICES_LIST* vertices;
    PLANE* plane;
    TRIANGLE_LIST* triangles;
    CORR* corresp;

    vertices = &surface->vertices;
    triangles = &surface->triangles;
    corresp = &surface->correspondance;
    plane = &surface->planes;
    plane->num_planes = corresp->num_points;
    //printf("corresp->num_points: %d NUMPLANES: %d\n", corresp->num_points, plane->num_planes);

    plane->a0 = (float*) realloc (plane->a0, corresp->num_points * sizeof(float));
    plane->a1 = (float*) realloc (plane->a1, corresp->num_points * sizeof(float));
    plane->a2 = (float*) realloc (plane->a2, corresp->num_points * sizeof(float));
    plane->a3 = (float*) realloc (plane->a3, corresp->num_points * sizeof(float));

    for (i = 0; i < corresp->num_points; i++) {
        //printf("corresp->num_points: %d", corresp->num_points);
        //exit(-1);
        //a0
        matrix[0][0] = vertices->x[triangles->first[corresp->corrpoint_index[i]] - 1];
        matrix[1][0] = vertices->x[triangles->second[corresp->corrpoint_index[i]] - 1];
        matrix[2][0] = vertices->x[triangles->third[corresp->corrpoint_index[i]] - 1];
        matrix[0][1] = vertices->y[triangles->first[corresp->corrpoint_index[i]] - 1];
        matrix[1][1] = vertices->y[triangles->second[corresp->corrpoint_index[i]] - 1];
        matrix[2][1] = vertices->y[triangles->third[corresp->corrpoint_index[i]] - 1];
        matrix[0][2] = vertices->z[triangles->first[corresp->corrpoint_index[i]] - 1];
        matrix[1][2] = vertices->z[triangles->second[corresp->corrpoint_index[i]] - 1];
        matrix[2][2] = vertices->z[triangles->third[corresp->corrpoint_index[i]] - 1];
        //printf("TRIANGOLO: %d\n", corresp->corrpoint_index[i]);
        //printf("INDEX POINT: %d %d %d\n", triangles->first[corresp->corrpoint_index[i]]-1,triangles->second[corresp->corrpoint_index[i]]-1, triangles->third[corresp->corrpoint_index[i]]-1);
        //printf("MATRIX:\n");
        //printf("%f %f %f\n",matrix[0][0],matrix[0][1],matrix[0][2]);
        //printf("%f %f %f\n",matrix[1][0],matrix[1][1],matrix[1][2]);
        //printf("%f %f %f\n",matrix[2][0],matrix[2][1],matrix[2][2]);

        plane->a0[i] = matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) - matrix[1][0] * (matrix[0][1] * matrix[2][2] - matrix[2][1] * matrix[0][2]) + matrix[2][0] * (matrix[0][1] * matrix[1][2] - matrix[1][1] * matrix[0][2]);
        //printf("a0= %f\n", plane->a0[i]);
        //exit(-1);

        //a1
        matrix[0][0] = 1;
        matrix[1][0] = 1;
        matrix[2][0] = 1;
        matrix[0][1] = vertices->y[triangles->first[corresp->corrpoint_index[i]] - 1];
        matrix[1][1] = vertices->y[triangles->second[corresp->corrpoint_index[i]] - 1];
        matrix[2][1] = vertices->y[triangles->third[corresp->corrpoint_index[i]] - 1];
        matrix[0][2] = vertices->z[triangles->first[corresp->corrpoint_index[i]] - 1];
        matrix[1][2] = vertices->z[triangles->second[corresp->corrpoint_index[i]] - 1];
        matrix[2][2] = vertices->z[triangles->third[corresp->corrpoint_index[i]] - 1];

        plane->a1[i] = matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) - matrix[1][0] * (matrix[0][1] * matrix[2][2] - matrix[2][1] * matrix[0][2]) + matrix[2][0] * (matrix[0][1] * matrix[1][2] - matrix[1][1] * matrix[0][2]);
        //printf("a1= %f\n", plane->a1[i]);
        //a2
        matrix[0][0] = vertices->x[triangles->first[corresp->corrpoint_index[i]] - 1];
        matrix[1][0] = vertices->x[triangles->second[corresp->corrpoint_index[i]] - 1];
        matrix[2][0] = vertices->x[triangles->third[corresp->corrpoint_index[i]] - 1];
        matrix[0][1] = 1;
        matrix[1][1] = 1;
        matrix[2][1] = 1;
        matrix[0][2] = vertices->z[triangles->first[corresp->corrpoint_index[i]] - 1];
        matrix[1][2] = vertices->z[triangles->second[corresp->corrpoint_index[i]] - 1];
        matrix[2][2] = vertices->z[triangles->third[corresp->corrpoint_index[i]] - 1];

        plane->a2[i] = matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) - matrix[1][0] * (matrix[0][1] * matrix[2][2] - matrix[2][1] * matrix[0][2]) + matrix[2][0] * (matrix[0][1] * matrix[1][2] - matrix[1][1] * matrix[0][2]);
        //printf("a2= %f\n", plane->a2[i]);
        //a3
        matrix[0][0] = vertices->x[triangles->first[corresp->corrpoint_index[i]] - 1];
        matrix[1][0] = vertices->x[triangles->second[corresp->corrpoint_index[i]] - 1];
        matrix[2][0] = vertices->x[triangles->third[corresp->corrpoint_index[i]] - 1];
        matrix[0][1] = vertices->y[triangles->first[corresp->corrpoint_index[i]] - 1];
        matrix[1][1] = vertices->y[triangles->second[corresp->corrpoint_index[i]] - 1];
        matrix[2][1] = vertices->y[triangles->third[corresp->corrpoint_index[i]] - 1];
        matrix[0][2] = 1;
        matrix[1][2] = 1;
        matrix[2][2] = 1;

        plane->a3[i] = matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) - matrix[1][0] * (matrix[0][1] * matrix[2][2] - matrix[2][1] * matrix[0][2]) + matrix[2][0] * (matrix[0][1] * matrix[1][2] - matrix[1][1] * matrix[0][2]);
        //printf("a3= %f\n", plane->a3[i]);
        //exit(-1);
    }
    //printf("FIRST PLANE: %f %f %f %f",plane->a0[0],plane->a1[0], plane->a2[0], plane->a3[0]);
    //exit(-1);
}

void
do_cp (FILE* mesh, FILE* MDpoints, SURFACE* surface, FILE* output)
{
    float d = 0;
    float pt[3];
    float coeff[4];

    VERTICES_LIST* points;
    PLANE* plane;

    read_obj (mesh, surface);
    calculate_mass (surface);
    read_MDcontours (MDpoints, surface);
    cp (surface);
    compute_plane (surface);
    //printf("FIRST PLANE: %f %f %f %f",plane->a0[0],plane->a1[0], plane->a2[0], plane->a3[0]);
    //exit(-1);

    points = &surface->MDpoints;
    plane = &surface->planes;
    plane->num_planes = points->num_vertices;

    //printf("NUMPLANES: %d",plane->num_planes);
    //printf("FIRST PLANE: %f %f %f %f",plane->a0[0],plane->a1[0], plane->a2[0], plane->a3[0]);
    //exit(-1);

    for (int i = 0; i < plane->num_planes; i++) {
        pt[0] = points->x[i];
        pt[1] = points->y[i];
        pt[2] = points->z[i];
        coeff[0] = plane->a0[i];
        coeff[1] = plane->a1[i];
        coeff[2] = plane->a2[i];
        coeff[3] = plane->a3[i];

        d = fabs ((coeff[1] * pt[0] + coeff[2] * pt[1] + coeff[3] * pt[2] - coeff[0]) / sqrt (coeff[1] * coeff[1] + coeff[2] * coeff[2] + coeff[3] * coeff[3]));
        //fprintf(output,"%f %f %f %f %f\n",plane->a0[i],plane->a1[i],plane->a2[i],plane->a3[i],d);
        fprintf (output, "%f\n", d);
        //exit(-1);
    }
    fclose (output);
}
