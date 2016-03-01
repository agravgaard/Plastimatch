/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include "config.h"
#include "ise.h"
#include "tracker.h"
#include "debug.h"

/*  IMPORTANT NOTE:
    If template is +/- 1, we can accumulate into unsigned long score
    without overflow for up to (2^15*4) entries, or a 362x362 window 
*/

/* ---------------------------------------------------------------------------- *
    Global variables
 * ---------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------- *
    Function prototypes
 * ---------------------------------------------------------------------------- */
static void create_template (TrackerInfo* ti);
static void score_region (TrackerInfo* ti, Frame* f);

/* ---------------------------------------------------------------------------- *
    Global functions
 * ---------------------------------------------------------------------------- */
void
tracker_init (TrackerInfo* ti)
{
    ti->m_curr_x = 1024;
    ti->m_curr_y = 768;
    //ti->m_search_w = 5;
    //ti->m_search_h = 5;
    //ti->m_search_w = 25;
    //ti->m_search_h = 25;
    ti->m_search_w = 35;
    ti->m_search_h = 35;
    /* Allocate memory for score */
    ti->m_score_size = sizeof(long) * ti->m_search_w * ti->m_search_h;
    ti->m_score = (long*) malloc (ti->m_score_size);
    /* Create template */
    create_template (ti);
    debug_printf ("Created score %p template %p\n", ti->m_score, ti->m_template);
}

void
tracker_shutdown (TrackerInfo* ti)
{
    free (ti->m_score);
    free (ti->m_template);
}

static void
dump_template (TrackerInfo* ti)
{
    int r, c;
    FILE* fp;

    fp = fopen("c:\\tmp\\template.txt", "w");
    for (r = 0; r < ti->m_template_h; r++) {
        for (c = 0; c < ti->m_template_w; c++) {
	    fprintf (fp, "%2d ", ti->m_template[r*ti->m_template_w+c]);
	}
	fprintf (fp, "\n");
    }
    fclose (fp);
}

static void
dump_score (TrackerInfo* ti)
{
    int sr, sc;
    FILE* fp;

    fp = fopen("c:\\tmp\\score.txt", "w");
    for (sr = 0; sr < ti->m_search_h; sr++) {
        for (sc = 0; sc < ti->m_search_w; sc++) {
	    fprintf (fp, "%6d ", ti->m_score[sr*ti->m_search_w+sc]);
	}
	fprintf (fp, "\n");
    }
    fclose (fp);
}

static void
create_template (TrackerInfo* ti)
{
    int r, c;
    int half_width = 10;
    int d_thresh = 90;
    ti->m_template_w = 2 * half_width + 1;
    ti->m_template_h = 2 * half_width + 1;
    ti->m_template_s1 = 0;
    ti->m_template_s2 = 0;
    ti->m_template = (short*) malloc (sizeof(short) * ti->m_template_w * ti->m_template_h);

    /* Assume row major */
    for (r = 0; r < ti->m_template_h; r++) {
        for (c = 0; c < ti->m_template_w; c++) {
            int d = (r - half_width)*(r - half_width) + (c - half_width)*(c - half_width);
            if (d < d_thresh) {
                ti->m_template_s1 ++;
                ti->m_template[r*ti->m_template_w+c] = 1;
            } else {
                ti->m_template_s2 ++;
                ti->m_template[r*ti->m_template_w+c] = -1;
            }
        }
    }
    debug_printf ("Created template: (%d x +1), (%d x -1)\n", ti->m_template_s1, ti->m_template_s2);
    //dump_template (ti);
}

void
track_frame (IseFramework* ig, unsigned int idx, Frame* f)
{
    TrackerInfo* ti = &ig->panel[idx].tracker_info;
    const int xmin = 200;
    const int xmax = 1848;
    const int ymin = 200;
    const int ymax = 1336;

    /* Don't track dark frames.  In the future, we'll extrapolate the
	position using a Kalman filter or something. */
    if (frame_is_dark(f)) {
	return;
    }

    score_region (ti, f);

    /* Truncate output */
    if (ti->m_curr_x < xmin) ti->m_curr_x = xmin;
    if (ti->m_curr_x > xmax) ti->m_curr_x = xmax;
    if (ti->m_curr_y < ymin) ti->m_curr_y = ymin;
    if (ti->m_curr_y > ymax) ti->m_curr_y = ymax;
}

/* Fast & inaccurate scoring function */
static void
score_region (TrackerInfo* ti, Frame* f)
{
    unsigned short* im = f->img;
    int im_w = 2048;
    int im_h = 1536;
    int sr, sc, tr, tc;
    int best_r, best_c;
    long best_score;
    int imr_base = ti->m_curr_y - (ti->m_search_h / 2) - (ti->m_template_h / 2);
    int imc_base = ti->m_curr_x - (ti->m_search_w / 2) - (ti->m_template_w / 2);
    debug_printf ("%d imr_base, %d imc_base\n",imr_base,imc_base);

    /* Initialize to zero so that skipped locations will show no correlation.  */
    memset ((void *) ti->m_score, 0, ti->m_score_size);

    /* Search for best fit to pattern */
    for (sr = 0; sr < ti->m_search_h; sr++) {
        for (sc = 0; sc < ti->m_search_w; sc++) {
            long* score = &ti->m_score[sr*ti->m_search_w+sc];
            unsigned long mean = 0;
            *score = 0;
            for (tr = 0; tr < ti->m_template_h; tr++) {
		int imr = imr_base + sr + tr;
                for (tc = 0; tc < ti->m_template_w; tc++) {
		    int imc = imc_base + sc + tc;
                    mean += im[imr*im_w+imc];
                    *score += im[imr*im_w+imc] * ti->m_template[tr*ti->m_template_w+tc];
                }
            }
            mean /= (ti->m_template_h * ti->m_template_w);
            *score -= (ti->m_template_s1-ti->m_template_s2) * mean;
        }
    }

    /* Return minimum score */
    best_r = -1, best_c = -1;
    best_score = 100000;
    for (sr = 0; sr < ti->m_search_h; sr++) {
        for (sc = 0; sc < ti->m_search_w; sc++) {
	    //debug_printf ("%ld ", ti->m_score[sr*ti->m_search_w+sc]);
            if (best_score > ti->m_score[sr*ti->m_search_w+sc]) {
                best_r = sr + ti->m_curr_y - (ti->m_search_h / 2);
                best_c = sc + ti->m_curr_x - (ti->m_search_w / 2);
                best_score = ti->m_score[sr*ti->m_search_w+sc];
            }
        }
        //debug_printf ("\n");
    }
    if (best_r != -1) {
        ti->m_curr_x = best_c;
        ti->m_curr_y = best_r;
    }
    debug_printf ("Finished tracking: %d, %d, %d\n", best_score, best_c, best_r);
    //ti->m_curr_x = 1024;
    //ti->m_curr_y = 768;
    //dump_score (ti);
}
