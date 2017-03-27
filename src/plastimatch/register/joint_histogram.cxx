/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include "file_util.h"
#include "joint_histogram.h"
#include "logfile.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "volume.h"
#include "xpm.h"

Joint_histogram::Joint_histogram ()
{
    this->m_hist = 0;
    this->f_hist = 0;
    this->j_hist = 0;
}

Joint_histogram::Joint_histogram (Mi_hist_type type,
    plm_long fixed_bins, plm_long moving_bins)
    : moving (type, moving_bins), 
      fixed (type, fixed_bins), 
      joint (type, moving_bins * fixed_bins)
{
    this->allocate ();
}

Joint_histogram::~Joint_histogram ()
{
    delete[] this->f_hist;
    delete[] this->m_hist;
    delete[] this->j_hist;
}

void 
Joint_histogram::allocate ()
{
    this->m_hist = new double [this->moving.bins]();
    this->f_hist = new double [this->fixed.bins]();
    this->j_hist = new double [this->joint.bins]();
}


void 
Joint_histogram::reset_histograms ()
{
    memset (this->f_hist, 0, this->fixed.bins * sizeof(double));
    memset (this->m_hist, 0, this->moving.bins * sizeof(double));
    memset (this->j_hist, 0,
        this->fixed.bins * this->moving.bins * sizeof(double));
}

/* -----------------------------------------------------------------------
   Initialization and teardown
   ----------------------------------------------------------------------- */
static inline double
vopt_bin_error (int start, int end, double* s_lut, double* ssq_lut, double* cnt_lut)
{
    double sq_diff;
    double diff;
    double delta;
    double v, n;

    sq_diff = ssq_lut[end] - ssq_lut[start];
    diff = s_lut[end] - s_lut[start];
    delta = (double)end - (double)start + 1.0;
    n = cnt_lut[end] - cnt_lut[start];
    v = sq_diff - (diff*diff)/delta;

    /* Penalize solutions that have bins with less than one voxel */
    if (n < 1.0) {
        return DBL_MAX;
    }

    return v;
} 

static void
bspline_initialize_mi_bigbin (
    double* hist, 
    Histogram* hparms, 
    Volume* vol
)
{
    int idx_bin;
    float* img = (float*) vol->img;

    if (!img) {
        logfile_printf ("ERROR: trying to pre-scan empty image!\n");
        exit (-1);
    }

    /* build a quick histogram */
    for (plm_long i=0; i<vol->npix; i++) {
        idx_bin = floor ((img[i] - hparms->offset) / hparms->delta);
        if (hparms->type == HIST_VOPT) {
            idx_bin = hparms->key_lut[idx_bin];
        }
        hist[idx_bin]++;
    }

    /* look for biggest bin */
    for (plm_long i=0; i<hparms->bins; i++) {
        if (hist[i] > hist[hparms->big_bin]) {
            hparms->big_bin = i;
        }
    }
//    printf ("big_bin: %i\n", hparms->big_bin);
}

static void
bspline_initialize_mi_hist_eqsp (Histogram* hparms, Volume* vol)
{
    plm_long i;
    float min_vox, max_vox;
    float* img = (float*) vol->img;

    if (!img) {
        logfile_printf ("Error trying to create histogram from empty image\n");
        exit (-1);
    }

    min_vox = max_vox = img[0];
    for (i = 0; i < vol->npix; i++) {
        if (img[i] < min_vox) {
            min_vox = img[i];
        } else if (img[i] > max_vox) {
            max_vox = img[i];
        }
    }

    /* To avoid rounding issues, top and bottom bin are only half full */
    hparms->delta = (max_vox - min_vox) / (hparms->bins - 1);
    hparms->offset = min_vox - 0.5 * hparms->delta;
}

#if defined (commentout)
static void
bspline_mi_hist_vopt_dump_ranges (
    Bspline_mi_hist* hparms,
    Volume* vol,
    const std::string& prefix
)
{
    FILE* fp;
    std::string fn;
    char buff[1024];
    plm_long i, j;

    fn = prefix + "_vopt_ranges.txt";
    fp = fopen (fn.c_str(), "wb");
    if (!fp) return;

    printf ("Writing %s vopt debug files to disk...\n", prefix.c_str());

    int old_bin = hparms->key_lut[0];
    float left = hparms->offset;
    float right = left;
    j = 0;
    for (i=0; i<hparms->keys; i++) {
        if (hparms->key_lut[i] == old_bin) {
            right += hparms->delta;
        } else {
            fprintf (fp, "Bin %u [%6.2f .. %6.2f]\n", (unsigned int) j, 
                left, right);
            sprintf (buff, "%s_vopt_lvl_%03u.mha", prefix.c_str(), 
                (unsigned int) j);
            dump_vol_clipped (buff, vol, left, right);

            old_bin = hparms->key_lut[i];
            left = right;
            right += hparms->delta;
            j++;
        }
    }
    /* Pick up the last bin */
    fprintf (fp, "Bin %u [%6.2f .. %6.2f]\n", (unsigned int) j, 
        left, right);
    sprintf (buff, "%s_vopt_lvl_%03u.mha", prefix.c_str(), 
        (unsigned int) j);
    dump_vol_clipped (buff, vol, left, right);
    fclose (fp);
}
#endif

/* JAS - 2011.08.08
 * Experimental implementation of V-Optimal Histograms
 * ref: 
 *   http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.92.3195&rep=rep1&type=pdf
 */
static void
bspline_initialize_mi_hist_vopt (Histogram* hparms, Volume* vol)
{
    int idx_bin;
    plm_long curr, next, bottom;
    float min_vox, max_vox;

    int* tracker;
    double* tmp_hist;
    double* tmp_avg;
    double* s_lut;
    double* ssq_lut;
    double* err_lut;
    double* cnt_lut;
    double candidate;
    float* img = (float*) vol->img;

    if (!img) {
        logfile_printf ("Error trying to create histogram from empty image\n");
        exit (-1);
    }

    hparms->keys = VOPT_RES;
    tmp_hist = (double*) malloc (hparms->keys * sizeof (double));
    tmp_avg  = (double*) malloc (hparms->keys * sizeof (double));
    memset (tmp_hist, 0, hparms->keys * sizeof (double));
    memset (tmp_avg,  0, hparms->keys * sizeof (double));

    s_lut   = (double*) malloc (hparms->keys * sizeof (double));
    ssq_lut = (double*) malloc (hparms->keys * sizeof (double));
    err_lut = (double*) malloc (hparms->bins * hparms->keys * sizeof (double));
    cnt_lut = (double*) malloc (hparms->keys* sizeof (double));
    memset (err_lut, 0, hparms->bins * hparms->keys * sizeof (double));

    tracker = (int*) malloc (hparms->bins * hparms->keys * sizeof (int));
    memset (tracker, 0, hparms->bins * hparms->keys * sizeof (int));

    /* Determine input image value range */
    min_vox = max_vox = img[0];
    for (plm_long i=1; i < vol->npix; i++) {
        if (img[i] < min_vox) {
            min_vox = img[i];
        } else if (img[i] > max_vox) {
            max_vox = img[i];
        }
    }

    /* To avoid rounding issues, top and bottom bin are only half full */
    hparms->delta = (max_vox - min_vox) / (hparms->keys - 1);
    hparms->offset = min_vox - 0.5 * hparms->delta;

    /* Construct high resolution histogram w/ bin contribution averages*/
    for (plm_long i=0; i<vol->npix; i++) {
        idx_bin = floor ((img[i] - hparms->offset) / hparms->delta);
        tmp_hist[idx_bin]++;
        tmp_avg[idx_bin] += img[i] - hparms->offset;
    }

    /* Sorted estimation table */
    for (plm_long i=0; i<hparms->keys; i++) {
        if (tmp_hist[i] > 0) {
            tmp_avg[i] = tmp_avg[i] / tmp_hist[i];
        }
        else if (i > 0) {
            tmp_avg[i] = tmp_avg[i-1];
        }
    }

    /* Create lookup tables for error computations */
    s_lut[0] = tmp_avg[0];
    ssq_lut[0] = (tmp_avg[0] * tmp_avg[0]);
    cnt_lut[0] = tmp_hist[0];
    for (plm_long i=1; i<hparms->keys; i++) {
        s_lut[i] = s_lut[i-1] + tmp_avg[i];
        ssq_lut[i] = ssq_lut[i-1] + (tmp_avg[i] * tmp_avg[i]);
        cnt_lut[i] = cnt_lut[i-1] + tmp_hist[i];
//        printf ("[%i] %f\n", i, tmp_avg[i]);
    }

    free (tmp_avg);
    free (tmp_hist);

    /* Compute the one-bin scores */
    for (plm_long i=0; i<hparms->keys; i++) {
        err_lut[i] = vopt_bin_error (0, i, s_lut, ssq_lut, cnt_lut);
    }

    /* Compute best multi-bin scores */
    for (plm_long j=1; j<hparms->bins; j++) {
        for (plm_long i=0; i<hparms->keys; i++) {

            err_lut[hparms->keys*j+i] = DBL_MAX;
            tracker[hparms->keys*j+i] = 0;

            for (plm_long k=0; k<i; k++) {
                candidate = err_lut[hparms->keys*(j-1)+k] + vopt_bin_error (k+1, i, s_lut, ssq_lut, cnt_lut);
                if (candidate <= err_lut[hparms->keys*j+i]) {
                    err_lut[hparms->keys*j+i] = candidate;
                    tracker[hparms->keys*j+i] = k;
                }
            }
        }
    }

    free (s_lut);
    free (ssq_lut);
    free (err_lut);
    free (cnt_lut);

    /* Build the linear key table */
    hparms->key_lut = (int*) malloc (hparms->keys * sizeof (int));
    memset (hparms->key_lut, 0, hparms->keys * sizeof (int));

    curr = hparms->keys-1;
    for (plm_long j=hparms->bins-1; j>=0; j--) {
        next = tracker[hparms->keys*j+curr];
        bottom = next+1;
        if (j == 0) { bottom = 0; }

//        printf ("[%i] from %i to %i\tErr: %6.2e\n",
//                  j, bottom, curr, err_lut[hparms->keys*j+curr]);
        for (plm_long i=bottom; i<=curr; i++) {
            hparms->key_lut[i] = j;
        }

        curr = next;
    }

    free (tracker);
}


static void
bspline_initialize_mi_hist (Histogram* hparms, Volume* vol)
{
    /* If user wants more than VOPT can offer, fallback to EQSP */
    if ((hparms->bins > VOPT_RES) && (hparms->type == HIST_VOPT)) {
        printf ("WARNING: Falling back to EQSP histograms.\n"
                "         (Reason: # bins > %i)\n", VOPT_RES);
        hparms->type = HIST_EQSP;
    }

    /* Histogram type specific init procedures */
    if (hparms->type == HIST_EQSP) {
        bspline_initialize_mi_hist_eqsp (hparms, vol);
    }
    else if (hparms->type == HIST_VOPT) {
        bspline_initialize_mi_hist_vopt (hparms, vol);
    }
    else {
        print_and_exit ("Error: Encountered invalid histogram type.  "
            "Terminating...\n");
    }
}

void
Joint_histogram::initialize (Volume *fixed, Volume *moving)
{
    bspline_initialize_mi_hist (&this->fixed, fixed);
    bspline_initialize_mi_hist (&this->moving, moving);

#if defined (commentout)
    if (parms->debug) {
        bspline_mi_hist_vopt_dump_ranges (&mi_hist->fixed, fixed, "fixed");
        bspline_mi_hist_vopt_dump_ranges (&mi_hist->moving, moving, "moving");
    }
#endif

    /* Initialize biggest bin trackers for OpenMP MI */
    bspline_initialize_mi_bigbin (this->f_hist, &this->fixed, fixed);
    bspline_initialize_mi_bigbin (this->m_hist, &this->moving, moving);

    /* This estimate /could/ be wrong for certain image sets */
    /* Will be auto corrected after first evaluation if incorrect */
    this->joint.big_bin = this->fixed.big_bin * this->moving.bins 
        + this->moving.big_bin;
}

void
Joint_histogram::dump_hist (int it, const std::string& prefix)
{
    double* f_hist = this->f_hist;
    double* m_hist = this->m_hist;
    double* j_hist = this->j_hist;
    plm_long i, j, v;
    FILE *fp;
    std::string fn;
    std::string buf;

    buf = string_format ("hist_fix_%02d.csv", it);
    //sprintf (buf, "hist_fix_%02d.csv", it);
    fn = prefix + buf;
    make_parent_directories (fn.c_str());
    fp = fopen (fn.c_str(), "wb");
    if (!fp) return;
    for (plm_long i = 0; i < this->fixed.bins; i++) {
        fprintf (fp, "%u %f\n", (unsigned int) i, f_hist[i]);
    }
    fclose (fp);

    //sprintf (buf, "hist_mov_%02d.csv", it);
    buf = string_format ("hist_mov_%02d.csv", it);
    fn = prefix + buf;
    make_parent_directories (fn.c_str());
    fp = fopen (fn.c_str(), "wb");
    if (!fp) return;
    for (i = 0; i < this->moving.bins; i++) {
        fprintf (fp, "%u %f\n", (unsigned int) i, m_hist[i]);
    }
    fclose (fp);

    //sprintf (buf, "hist_jnt_%02d.csv", it);
    buf = string_format ("hist_jnt_%02d.csv", it);
    fn = prefix + buf;
    make_parent_directories (fn.c_str());
    fp = fopen (fn.c_str(), "wb");
    if (!fp) return;
    for (i = 0, v = 0; i < this->fixed.bins; i++) {
        for (j = 0; j < this->moving.bins; j++, v++) {
            if (j_hist[v] > 0) {
                fprintf (fp, "%u %u %u %g\n", (unsigned int) i, 
                    (unsigned int) j, (unsigned int) v, j_hist[v]);
            }
        }
    }
    fclose (fp);
}

void
Joint_histogram::add_pvi_8
(
    const Volume *fixed, 
    const Volume *moving, 
    int fidx, 
    int mvf, 
    float li_1[3],           /* Fraction of interpolant in lower index */
    float li_2[3])           /* Fraction of interpolant in upper index */
{
    float w[8];
    int n[8];
    int idx_fbin, idx_mbin, idx_jbin, idx_pv;
    int offset_fbin;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    double *f_hist = this->f_hist;
    double *m_hist = this->m_hist;
    double *j_hist = this->j_hist;


    /* Compute partial volumes from trilinear interpolation weights */
    w[0] = li_1[0] * li_1[1] * li_1[2]; // Partial Volume w0
    w[1] = li_2[0] * li_1[1] * li_1[2]; // Partial Volume w1
    w[2] = li_1[0] * li_2[1] * li_1[2]; // Partial Volume w2
    w[3] = li_2[0] * li_2[1] * li_1[2]; // Partial Volume w3
    w[4] = li_1[0] * li_1[1] * li_2[2]; // Partial Volume w4
    w[5] = li_2[0] * li_1[1] * li_2[2]; // Partial Volume w5
    w[6] = li_1[0] * li_2[1] * li_2[2]; // Partial Volume w6
    w[7] = li_2[0] * li_2[1] * li_2[2]; // Partial Volume w7

    /* Note that Sum(wN) for N within [0,7] should = 1 */

    // Calculate Point Indices for 8 neighborhood
    n[0] = mvf;
    n[1] = n[0] + 1;
    n[2] = n[0] + moving->dim[0];
    n[3] = n[2] + 1;
    n[4] = n[0] + moving->dim[0]*moving->dim[1];
    n[5] = n[4] + 1;
    n[6] = n[4] + moving->dim[0];
    n[7] = n[6] + 1;

    // Calculate fixed histogram bin and increment it
    idx_fbin = floor ((f_img[fidx] - this->fixed.offset) 
        / this->fixed.delta);
    if (this->fixed.type == HIST_VOPT) {
        idx_fbin = this->fixed.key_lut[idx_fbin];
    }
    f_hist[idx_fbin]++;

    offset_fbin = idx_fbin * this->moving.bins;

    // Add PV weights to moving & joint histograms   
    for (idx_pv=0; idx_pv<8; idx_pv++) {
        idx_mbin = floor ((m_img[n[idx_pv]] - this->moving.offset) 
            / this->moving.delta);
        if (this->moving.type == HIST_VOPT) {
            idx_mbin = this->moving.key_lut[idx_mbin];
        }
        idx_jbin = offset_fbin + idx_mbin;
        m_hist[idx_mbin] += w[idx_pv];
        j_hist[idx_jbin] += w[idx_pv];
    }
}

/* This algorithm uses a un-normalized score. */
float
Joint_histogram::compute_score (int num_vox)
{
    double* f_hist = this->f_hist;
    double* m_hist = this->m_hist;
    double* j_hist = this->j_hist;

    plm_long i, j;
    plm_long v;
    double fnv = (double) num_vox;
    double score = 0;
    double hist_thresh = 0.001 / (this->moving.bins * this->fixed.bins);

    /* Compute cost */
    for (i = 0, v = 0; i < this->fixed.bins; i++) {
        for (j = 0; j < this->moving.bins; j++, v++) {
            if (j_hist[v] > hist_thresh) {
                score -= j_hist[v] 
                    * logf (fnv * j_hist[v] / (m_hist[j] * f_hist[i]));
            }
        }
    }

    score = score / fnv;
    return (float) score;
}

void dump_xpm_hist (Joint_histogram* mi_hist, char* file_base, int iter)
{
    int z;
    char c;

    // Graph Properties
    int graph_offset_x = 10;
    int graph_offset_y = 10;
    int graph_padding = 20;
    int graph_bar_height = 100;
    int graph_bar_width = 5;
    int graph_bar_spacing = (int)((float)graph_bar_width * (7.0/5.0));
    plm_long graph_color_levels = 22;

    //  int fixed_bar_height;   // max bar height (pixels)
    //  int moving_bar_height;
    plm_long joint_color;

    float fixed_scale;  // pixels per amt
    float moving_scale;
    float joint_scale;

    float moving_max_val=0; 
    float fixed_max_val=0;
    float joint_max_val=0;

    int fixed_total_width = mi_hist->fixed.bins * graph_bar_spacing;
    int moving_total_width = mi_hist->moving.bins * graph_bar_spacing;

    int graph_moving_x_pos = graph_offset_x + graph_bar_height + graph_padding;
    int graph_moving_y_pos = graph_offset_y + fixed_total_width + graph_padding + graph_bar_height;

    int graph_fixed_x_pos = graph_offset_x;
    int graph_fixed_y_pos = graph_offset_y + fixed_total_width;

    int border_padding = 5;
    int border_width = moving_total_width + 2*border_padding;
    int border_height = fixed_total_width + 2*border_padding;
    int border_x_pos = graph_moving_x_pos - border_padding;
    int border_y_pos = graph_offset_y - border_padding + (int)((float)graph_bar_width * (2.0/5.0));

    int canvas_width = 2*graph_offset_x + graph_bar_height + moving_total_width + graph_padding;
    int canvas_height = 2*graph_offset_y + graph_bar_height + fixed_total_width + graph_padding;
    
    double *m_hist = mi_hist->m_hist;
    double *f_hist = mi_hist->f_hist;
    double *j_hist = mi_hist->j_hist;
    
    char filename[20];

    sprintf(filename, "%s_%04i.xpm", file_base, iter);

    // ----------------------------------------------
    // Find max value for fixed
    for(plm_long i=0; i<mi_hist->fixed.bins; i++)
        if (f_hist[i] > fixed_max_val)
            fixed_max_val = f_hist[i];
    
    // Find max value for moving
    for(plm_long i=0; i<mi_hist->moving.bins; i++)
        if (m_hist[i] > moving_max_val)
            moving_max_val = m_hist[i];
    
    // Find max value for joint
    // (Ignoring bin 0)
    for(plm_long j=0; j<mi_hist->fixed.bins; j++) {
        for(plm_long i=0; i<mi_hist->moving.bins; i++) {
            if ( (i > 0) && (j > 1) )
                if (j_hist[j*mi_hist->moving.bins + i] > joint_max_val)
                    joint_max_val = j_hist[j*mi_hist->moving.bins + i];
        }
    }


    // Generate scaling factors
    fixed_scale = (float)graph_bar_height / fixed_max_val;
    moving_scale = (float)graph_bar_height / moving_max_val;
    joint_scale = (float)graph_color_levels / joint_max_val;
    // ----------------------------------------------


    
    // ----------------------------------------------
    // Pull out a canvas and brush!
    Xpm_canvas* xpm = new Xpm_canvas (canvas_width, canvas_height, 1);
    Xpm_brush* brush = new Xpm_brush;
    
    // setup the palette
    xpm->add_color ('a', 0xFFFFFF);    // white
    xpm->add_color ('b', 0x000000);    // black
    xpm->add_color ('z', 0xFFCC00);    // orange

    // generate a nice BLUE->RED gradient
    c = 'c';
    z = 0x0000FF;
    for (plm_long i=0; i<(graph_color_levels+1); i++)
    {
        xpm->add_color (c, z);

        z -= 0x00000B;      // BLUE--
        z += 0x0B0000;      //  RED++

        c = (char)((int)c + 1); // LETTER++
    }

    // Prime the XPM Canvas
    xpm->prime ('a');
    // ----------------------------------------------
    

    printf("Drawing Histograms... ");

    
    /* Generate Moving Histogram */
    brush->set_type (XPM_BOX);
    brush->set_color ('b');
    brush->set_pos (graph_moving_x_pos, graph_moving_y_pos);
    brush->set_width (graph_bar_width);
    brush->set_height (0);

    int tmp_h;
    for(plm_long i=0; i<mi_hist->moving.bins; i++)
    {
        tmp_h = (int)(m_hist[i] * moving_scale);
        brush->set_height (tmp_h);
        brush->set_y (graph_moving_y_pos - tmp_h);
        xpm->draw (brush);
        brush->inc_x (graph_bar_spacing);
    }

    
    /* Generate Fixed Histogram */
    brush->set_type (XPM_BOX);
    brush->set_color ('b');
    brush->set_pos (graph_fixed_x_pos, graph_fixed_y_pos);
    brush->set_width (0);
    brush->set_height (graph_bar_width);

    for (plm_long i=0; i<mi_hist->fixed.bins; i++)
    {
        brush->set_width ((int)(f_hist[i] * fixed_scale));
        xpm->draw (brush);
        brush->inc_x ((-1)*graph_bar_spacing);
    }


    /* Generate Joint Histogram */
    brush->set_type (XPM_BOX);
    brush->set_color ('b');
    brush->set_pos (graph_moving_x_pos, graph_fixed_y_pos);
    brush->set_width (graph_bar_width);
    brush->set_height (graph_bar_width);

    z = 0;
    for(plm_long j=0; j<mi_hist->fixed.bins; j++) {
        for(plm_long i=0; i<mi_hist->moving.bins; i++) {
            joint_color = (size_t)(j_hist[z++] * joint_scale);
            if (joint_color > 0) {
                // special handling for bin 0
                if (joint_color > graph_color_levels) {
                    //  printf ("Clamp @ P(%i,%i)\n", i, j);
                    //  brush.color = (char)(graph_color_levels + 99);
                    brush->set_color ('z');
                } else {
                    brush->set_color ((char)(joint_color + 99));
                }
            } else {
                brush->set_color ('a');
            }
            xpm->draw (brush);     
            brush->inc_x (graph_bar_spacing);
        }
        // get ready to render new row
        brush->set_x (graph_moving_x_pos);
        brush->inc_y ((-1)*graph_bar_spacing);
    }

    /* Generate Joint Histogram Border */
    brush->set_type (XPM_BOX); // top
    brush->set_color ('b');
    brush->set_pos (border_x_pos, border_y_pos);
    brush->set_width (border_width);
    brush->set_height (1);
    xpm->draw (brush);

    brush->set_width (1); // left
    brush->set_height (border_height);
    xpm->draw (brush);

    brush->set_width (border_width); // bottom
    brush->set_height (1);
    brush->inc_y (border_height);
    xpm->draw (brush);

    brush->set_width (1); // right
    brush->set_height (border_height);
    brush->set_pos (border_width, border_y_pos);
    xpm->draw (brush);

    printf("done.\n");
    
    // Write to file
    xpm->write (filename);

    delete xpm;
    delete brush; 
}
