/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "itk_tps.h"
#include "landmark_warp.h"
#include "path_util.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "raw_pointset.h"
#include "rbf_gauss.h"
#include "rbf_wendland.h"
#include "string_util.h"
#include "volume.h"
#include "xform.h"

class Landmark_warp_main_parms {
public:
    std::string fixed_lm_fn;
    std::string moving_lm_fn;
    std::string input_vf_fn;
    std::string input_img_fn;
    std::string output_img_fn;
    std::string output_vf_fn;
    std::string output_lm_fn;

    std::string fixed_img_fn;
    bool have_dim;
    plm_long dim[3];
    bool have_origin;
    float origin[3];
    bool have_spacing;
    float spacing[3];
    bool have_direction_cosines;
    float direction_cosines[9];

    std::string algorithm;

    Landmark_warp lw;

public:
    Landmark_warp_main_parms () {
	have_dim = 0;
	have_origin = 0;
	have_spacing = 0;
	have_direction_cosines = 0;
    }
};

static void
do_landmark_warp_itk_tps (Landmark_warp *lw)
{
    itk_tps_warp (lw);
}

static void
do_landmark_warp_wendland (Landmark_warp *lw)
{
    rbf_wendland_warp (lw);
}

static void
do_landmark_warp_gauss (Landmark_warp *lw)
{
    rbf_gauss_warp (lw);
}

static void
load_input_files (Landmark_warp_main_parms *parms)
{
    Landmark_warp *lw = &parms->lw;

    /* Load the pointsets */
    lw->load_pointsets (parms->fixed_lm_fn.c_str(), 
        parms->moving_lm_fn.c_str());
    if (!lw) {
	print_and_exit ("Error, landmarks were not loaded successfully.\n");
    }

    /* Load the input image */
    if (parms->input_img_fn != "") {
	lw->m_input_img = plm_image_load_native (parms->input_img_fn);
	if (!lw->m_input_img) {
	    print_and_exit ("Error reading input image: %s\n", 
		parms->input_img_fn.c_str());
	}
	/* Default geometry, if unknown, comes from moving image */
	lw->m_pih.set_from_plm_image (lw->m_input_img);
    }

    /* Set the output geometry.  
       Note: --offset, --spacing, and --dim get priority over --fixed. 
       Therefore, if these options are completely specified, we don't need 
       to load the fixed image.
    */
    if (!parms->have_dim || !parms->have_origin 
	|| !parms->have_spacing || !parms->have_direction_cosines)
    {
	if (parms->fixed_img_fn != "") {
            Plm_image::Pointer pli 
                = plm_image_load_native (parms->fixed_img_fn);
	    if (!pli) {
		print_and_exit ("Error loading fixed image: %s\n",
		    parms->fixed_img_fn.c_str());
	    }
	    lw->m_pih.set_from_plm_image (pli);
	}
    }
    if (parms->have_dim) {
	lw->m_pih.set_dim (parms->dim);
    }
    if (parms->have_origin) {
	lw->m_pih.set_origin (parms->origin);
    }
    if (parms->have_spacing) {
	lw->m_pih.set_spacing (parms->spacing);
    }
//	if (parms->have_direction_cosines) {
//	lw->m_pih.set_direction_cosines (parms->direction_cosines);
//    }
}

#if defined (commentout)
void
pointset_save_fcsv_by_cluster (Raw_pointset* ps, int *clust_id, int which_cluster, const char *fn)
{
    int i;
    int symbol;
    FILE *fp;
    
    // symbolType, see
    //http://www.slicer.org/slicerWiki/index.php/Modules:Fiducials-Documentation-3.4
    symbol =which_cluster+2; 
    if (symbol > 13) symbol -=13;

    fp = fopen (fn, "w");
    if (!fp) return;

    int num_points_in_cluster=0;
    for (i = 0; i < ps->num_points; i++) {
	if (clust_id[i] == which_cluster) num_points_in_cluster++;	
    }

    fprintf (fp, 
	"# Fiducial List file %s\n"
	"# version = 2\n"
	"# name = plastimatch-fiducials\n"
	"# numPoints = %d\n"
	"# symbolScale = 5\n"
	"# symbolType = %d\n"
	"# visibility = 1\n"
	"# textScale = 4.5\n"
	"# color = 0.4,1,1\n"
	"# selectedColor = 1,0.5,0.5\n"
	"# opacity = 1\n"
	"# ambient = 0\n"
	"# diffuse = 1\n"
	"# specular = 0\n"
	"# power = 1\n"
	"# locked = 0\n"
	"# numberingScheme = 0\n"
	"# columns = label,x,y,z,sel,vis\n",
	fn, 
	num_points_in_cluster,
	symbol);

    for (i = 0; i < ps->num_points; i++) {
	if (clust_id[i] == which_cluster)
	    fprintf (fp, "p-%03d-c%02d,%f,%f,%f,1,1\n", 
		i, clust_id[i],
		- ps->points[i*3+0], 
		- ps->points[i*3+1], 
		ps->points[i*3+2]);
    }
    fclose (fp);
}
#endif

static void
save_output_files (Landmark_warp_main_parms *parms)
{
    Landmark_warp *lw = &parms->lw;

    /* GCS FIX: float output only, and no dicom. */
    if (lw->m_warped_img && parms->output_img_fn != "") {
	lw->m_warped_img->save_image (parms->output_img_fn);
    }
    if (lw->m_vf && parms->output_vf_fn != "") {
	xform_save (lw->m_vf, parms->output_vf_fn);
    }
    if (lw->m_vf && parms->output_lm_fn != "")
    {
	if ( lw->num_clusters > 0 ) {
            /* GCS FIX 2015-06-23: If this is ever needed, it can be 
               re-implemented e.g. by specifying a selection mask.
               N.b. see above code for reference. */
#if defined (commentout)
	    // if clustering required, save each cluster to a separate fcsv
	    for( int ii = 0; ii<lw->num_clusters; ii++) {
                std::string fn_base = strip_extension (parms->output_lm_fn);
                std::string fn_out = string_format ("%s_cl_%d.fcsv",
                    fn_base.c_str(), ii);
		
		//write FIXED landmarks to check for clustering issues
		pointset_save_fcsv_by_cluster(lw->m_fixed_landmarks, 
                    lw->cluster_id, ii, fn_out.c_str());
            }
#endif
        }
        else {
            lw->m_warped_landmarks.save (parms->output_lm_fn.c_str());
        }
    }
}

static void
do_landmark_warp (Landmark_warp_main_parms *parms)
{
    Landmark_warp *lw = &parms->lw;

    load_input_files (parms);

    if (parms->algorithm == "tps") {
	do_landmark_warp_itk_tps (lw);
    }
    else if (parms->algorithm == "gauss") {
	do_landmark_warp_gauss (lw);
    }
    else if (parms->algorithm == "wendland") {
	do_landmark_warp_wendland (lw);
    }

    if (parms->output_lm_fn != "") {
	calculate_warped_landmarks (lw);
    }

    save_output_files (parms);
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    std::cout << "Usage: landmark_warp [options]\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Landmark_warp_main_parms *parms, 
    dlib::Plm_clp *parser, 
    int argc, 
    char* argv[]
)
{
    Landmark_warp *lw = &parms->lw;

    /* Add --help, --version */
    parser->add_default_options ();

    /* Basic options */
    parser->add_long_option ("f", "fixed-landmarks", 
	"Input fixed landmarks", 1, "");
    parser->add_long_option ("m", "moving-landmarks",
	"Output moving landmarks", 1, "");
    parser->add_long_option ("v", "input-vf",
	"Input vector field (applied prior to landmark warping)", 1, "");
    parser->add_long_option ("I", "input-image",
	"Input image to warp", 1, "");
    parser->add_long_option ("O", "output-image",
	"Output warped image", 1, "");
    parser->add_long_option ("V", "output-vf", 
	"Output vector field", 1, "");
    parser->add_long_option ("L", "output-landmarks", 
	"Output warped landmarks", 1, "");

    /* Output geometry options */
    parser->add_long_option ("", "origin", 
	"Location of first image voxel in mm \"x y z\"", 1, "");
    parser->add_long_option ("", "spacing", 
	"Voxel spacing in mm \"x [y z]\"", 1, "");
    parser->add_long_option ("", "dim", 
	"Size of output image in voxels \"x [y z]\"", 1, "");
    parser->add_long_option ("F", "fixed", 
	"Fixed image (match output size to this image)", 1, "");

    /* Algorithm options */
    parser->add_long_option ("a", "algorithm",
	"RBF warping algorithm {tps,gauss,wendland}", 1, "gauss");
    parser->add_long_option ("r", "radius",
	"Radius of radial basis function (in mm)", 1, "50.0");
    parser->add_long_option ("Y", "stiffness",
	"Young modulus (default = 0.0)", 1, "0.0");
    parser->add_long_option ("d", "default-value",
	"Value to set for pixels with unknown value", 1, "-1000");
    parser->add_long_option ("N", "numclusters",
	"Number of clusters of landmarks", 1, "0");

    /* Parse the command line arguments */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that required inputs were given */
    parser->check_required ("fixed-landmarks");
    parser->check_required ("moving-landmarks");

    /* Copy values into output struct */
    parms->fixed_lm_fn = parser->get_string("fixed-landmarks");
    parms->moving_lm_fn = parser->get_string("moving-landmarks");
    parms->input_vf_fn = parser->get_string("input-vf");
    parms->input_img_fn = parser->get_string("input-image");
    parms->output_img_fn = parser->get_string("output-image");
    parms->output_vf_fn = parser->get_string("output-vf");
    parms->output_lm_fn = parser->get_string("output-landmarks");

    if (parser->option ("origin")) {
	parms->have_origin = 1;
	parser->assign_float_13 (parms->origin, "origin");
    }
    if (parser->option ("spacing")) {
	parms->have_spacing = 1;
	parser->assign_float_13 (parms->spacing, "spacing");
    }
    if (parser->option ("dim")) {
	parms->have_dim = 1;
	parser->assign_plm_long_13 (parms->dim, "dim");
    }
    parms->fixed_img_fn = parser->get_string("fixed");

    parms->algorithm = parser->get_string("algorithm");
    lw->rbf_radius = parser->get_float("radius");
    lw->young_modulus = parser->get_float("stiffness");
    lw->default_val = parser->get_float("default-value");
    lw->num_clusters = parser->get_float("numclusters");
}

int
main (int argc, char *argv[])
{
    Landmark_warp_main_parms parms;

    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv);
    do_landmark_warp (&parms);

    return 0;
}
