/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* This is a native C implementation.  See this:
    http://www.dgp.toronto.edu/~ah/csc418/fall_2001/notes/scanconv.html
    http://www.cc.gatech.edu/gvu/multimedia/nsfmmedia/graphics/elabor/polyscan/polyscan1.html
    http://graphics.cs.ucdavis.edu/education/GraphicsNotes/Scan-Conversion/Scan-Conversion.html
    http://www.cs.berkeley.edu/~sequin/CS184/TEXT/Algorithm.html
  */
#include "plm_config.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rasterize_slice.h"

typedef struct edge Edge;
struct edge {
    int ymax;
    float x;
    float xincr;
    Edge* next;
};

void
remove_old_edges (Edge** head, int y)
{
    Edge *p, *n;
    p = *head;
    while (p && p->ymax < y) p = p->next;
    *head = p;
    while (p) {
	n = p->next;
	while (n && n->ymax < y) n = n->next;
	p->next = n;
	p = n;
    }
}

void
insert_ordered_by_x (Edge** head, Edge* c)
{
    Edge* p;
    if ((p = *head)) {
	if (p->x > c->x) {
	    c->next = p;
	    *head = c;
	} else {
	    while (p->next && p->next->x < c->x) p = p->next;
	    c->next = p->next;
	    p->next = c;
	}
    } else {
	*head = c;
	c->next = 0;
    }
}

void
print_edges (Edge* p)
{
    while (p) {
	printf ("[%g %g %d] ", p->x, p->xincr, p->ymax);
	p = p->next;
    }
}

/* Returns true if point lies within polygon */
/* I don't use the below algorithm, but it looks interesing:
   http://softsurfer.com/Archive/algorithm_0103/algorithm_0103.htm */
bool
point_in_polygon (
    const float* x_in,           /* polygon vertices in mm */
    const float* y_in,           /* polygon vertices in mm */
    int num_vertices,
    float x_test,
    float y_test
)
{
    int num_crossings = 0;

    /* Check if last vertex == first vertex.  If so, remove it. */
    if (x_in[num_vertices-1] == x_in[0] && y_in[num_vertices-1] == y_in[0]) {
	num_vertices --;
    }

    for (int i = 0; i < num_vertices; i++) {
	int a = i, b = (i==num_vertices-1 ? 0 : i+1);
	/* Reorder segment so that y[a] > y[b] */
	if (y_in[a] == y_in[b]) continue;
	if (y_in[a] < y_in[b]) a = b, b = i;
	/* Reject segments too high or too low */
	/* If upper y is exactly equal to query location, reject */
	if (y_in[a] <= y_test) continue;
	if (y_in[b] > y_test) continue;

	/* Find x coordinate of segment */
	float frac = (y_in[a] - y_test) / (y_in[a] - y_in[b]);
	float x_line_seg = x_in[b] + frac * (x_in[a] - x_in[b]);

	/* If x_test is to the right x_line_seg, then we have a crossing.
	   Count as inside for left boundaries. */
	if (x_test >= x_line_seg) {
	    num_crossings ++;
	}
    }
    return (num_crossings % 2) == 1;
}

/* Rasterizes a single closed polygon on a slice */
void
rasterize_slice (
    unsigned char* acc_img,
    plm_long* dims,
    float* spacing,
    float* offset,
    size_t num_vertices,
    const float* x_in,          /* polygon vertices in mm */
    const float* y_in           /* polygon vertices in mm */
)
{
    unsigned char* imgp;
    Edge** edge_table;
    Edge* edge_list;	    /* Global edge list */
    Edge* ael;  		    /* Active edge list */
    float *x, *y;           /* vertices in pixel coordinates */

    /* Check if last vertex == first vertex.  If so, remove it. */
    if (x_in[num_vertices-1] == x_in[0] && y_in[num_vertices-1] == y_in[0]) {
	num_vertices --;
    }

    /* Convert from mm to pixel coordinates */
    x = (float*) malloc (sizeof (float) * num_vertices);
    y = (float*) malloc (sizeof (float) * num_vertices);
    for (size_t i = 0; i < num_vertices; i++) {
	x[i] = (x_in[i] - offset[0]) / spacing[0];
	y[i] = (y_in[i] - offset[1]) / spacing[1];
    }

    /* Make edge table */
    edge_table = (Edge**) malloc (dims[1] * sizeof(Edge*));
    edge_list = (Edge*) malloc (num_vertices * sizeof(Edge));
    memset (edge_table, 0, dims[1] * sizeof(Edge*));
    for (size_t i = 0; i < num_vertices; i++) {
	int ymin, ymax;
	size_t a = i, b = (i==num_vertices-1 ? 0 : i+1);
	/* Reorder segment so that y[a] > y[b] */
	if (y[a] == y[b]) continue;
	if (y[a] < y[b]) a = b, b = i;
	/* Reject segments too high or too low */
	ymin = (int) ceil(y[b]);
	if (ymin > dims[1]-1) continue;
	ymax = (int) floor(y[a]);
	if (ymax < 0) continue;
	/* If upper y lies on scan line, don't count it as an intersection */
	if (y[a] == ymax) ymax --;
	/* Reject segments that don't intersect a scan line */
	if (ymax < ymin) continue;
	/* Clip segments against image boundary */
	if (ymin < 0) ymin = 0;
	if (ymax > dims[1]-1) ymax = dims[1]-1;
	/* Shorten the segment & fill in edge data */
	edge_list[i].ymax = ymax;
	edge_list[i].xincr = (x[a] - x[b]) / (y[a] - y[b]);
	edge_list[i].x = x[b] + (ymin - y[b]) * edge_list[i].xincr;
	edge_list[i].next = 0;
	/* Insert into edge_table */
#if defined (commentout)
	printf ("[y:%g %g, x:%g %g] -> [y:%d %d, x:%g (%g)]\n", 
	    y[b], y[a], x[b], x[a],
	    ymin, ymax, edge_list[i].x, edge_list[i].xincr);
#endif
        insert_ordered_by_x (&edge_table[ymin], &edge_list[i]);
    }

    /* Debug edge table */
#if defined (commentout)
    printf ("-------------------------------------------\n");
    for (plm_long i = 0; i < dims[1]; i++) {
	if (edge_table[i]) {
	    printf ("%d: ", i);
	    print_edges (edge_table[i]);
	    printf ("\n");
	}
    }
#endif

    /* Loop through scanline, rendering each */
    imgp = acc_img;
    ael = 0;
    for (plm_long i = 0; i < dims[1]; i++) {
	int x, num_crossings;
	Edge *n, *c;
	/* Remove old edges from AEL */
	remove_old_edges (&ael, i);

	/* Add new edges to AEL */
	c = edge_table[i];
	while (c) {
	    n = c->next;
	    insert_ordered_by_x (&ael, c);
	    c = n;
	}

	/* Count scan intersections & rasterize */
	num_crossings = 0;
	x = 0;
	c = ael;
#if defined (commentout)
	printf ("%d ", i);
	print_edges (ael);
#endif
	while (x < dims[0]) {
	    int next_x;
	    while (1) {
		if (!c) {
		    next_x = dims[0];
		    break;
		} else if (x >= c->x) {
		    c = c->next;
		    num_crossings ++;
		    continue;
		} else {
		    next_x = (int) floor (c->x) + 1;
		    if (next_x > dims[0]) next_x = dims[0];
		    break;
		}
	    }
	    num_crossings = num_crossings % 2;
#if defined (commentout)
	    printf ("(%d %c %d)", x, num_crossings?'+':'-', next_x-1);
#endif
	    while (x < next_x) {
		*imgp++ = num_crossings;
		x++;
	    }
	}
#if defined (commentout)
	printf ("\n");
	getchar();
#endif

	/* Update x values on AEL */
	c = ael;
	while (c) {
	    c->x += c->xincr;
	    c = c->next;
	}

	/* Resort AEL - this could be done more efficiently */
	c = ael;
	while (c) {
	    if (c->next && c->x > c->next->x) {
		Edge* tmp = c->next;
		c->next = c->next->next;
		insert_ordered_by_x (&ael, tmp);
		c = tmp;
	    } else {
		c = c->next;
	    }
	}
    }

    /* Free things up */
    free (x);
    free (y);
    free (edge_table);
    free (edge_list);
}
