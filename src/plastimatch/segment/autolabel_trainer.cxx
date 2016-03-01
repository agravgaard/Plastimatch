/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsegment_config.h"

#include <stdio.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <itksys/RegularExpression.hxx>
#include "itkDirectory.h"
#include "itkImageRegionIterator.h"
#include "itkRegularExpressionSeriesFileNames.h"

#include "autolabel_task.h"
#include "autolabel_thumbnailer.h"
#include "autolabel_trainer.h"
#include "dlib_trainer.h"
#include "file_util.h"
#include "path_util.h"
#include "plm_image.h"
#include "pointset.h"
#include "print_and_exit.h"
#include "string_util.h"

Autolabel_trainer::Autolabel_trainer ()
{
    m_dt_la1 = 0;
    m_dt_tsv1 = 0;
    m_dt_tsv2_x = 0;
    m_dt_tsv2_y = 0;
}

Autolabel_trainer::~Autolabel_trainer ()
{
    if (m_dt_la1) delete m_dt_la1;
    if (m_dt_tsv1) delete m_dt_tsv1;
    if (m_dt_tsv2_x) delete m_dt_tsv2_x;
    if (m_dt_tsv2_y) delete m_dt_tsv2_y;
}

void
Autolabel_trainer::set_input_dir (const char* input_dir)
{
    if (!itksys::SystemTools::FileIsDirectory (input_dir)) {
	print_and_exit ("Error: \'%s\' is not a directory\n", input_dir);
    }

    /* We can't load the data yet, since we don't know the task */
    m_input_dir = input_dir;
}

void
Autolabel_trainer::load_input_dir_recursive (std::string input_dir)
{
    itksys::Directory itk_dir;

    if (!itk_dir.Load (input_dir.c_str())) {
	print_and_exit ("Error loading directory (%s)\n", input_dir.c_str());
    }

    for (unsigned long i = 0; i < itk_dir.GetNumberOfFiles(); i++)
    {
	/* Skip "." and ".." */
	if (!strcmp (itk_dir.GetFile(i), ".") 
	    || !strcmp (itk_dir.GetFile(i), ".."))
	{
	    continue;
	}

	/* Make fully specified filename */
	std::string fn = input_dir + "/" + itk_dir.GetFile(i);

	/* Process directories recursively */
	if (itksys::SystemTools::FileIsDirectory (fn.c_str())) {
	    load_input_dir_recursive (fn);
	}

	/* Check for .nrrd files */
	if (extension_is (fn.c_str(), "nrrd")) {
	    /* Does .nrrd file have a corresponding .fcsv file? */
	    std::string fcsv_fn = fn;
	    fcsv_fn.replace (fn.length()-4, 4, "fcsv");

	    if (file_exists (fcsv_fn.c_str())) {
		load_input_file (fn.c_str(), fcsv_fn.c_str());
	    }
	}
    }

}

/* This function creates "t_map", a map from t-spine # to (x,y,z) point */
static std::map<float, Point> 
load_tspine_map (const Labeled_pointset& ps)
{
    std::map<float, Point> t_map;
    for (unsigned int i = 0; i < ps.point_list.size(); i++) {
	if (ps.point_list[i].label == "C7") {
	    t_map.insert (std::pair<float,Point> (0, 
		    Point (ps.point_list[i].p[0],
			ps.point_list[i].p[1],
			ps.point_list[i].p[2])));
	}
	else if (ps.point_list[i].label == "L1") {
	    t_map.insert (std::pair<float,Point> (13, 
		    Point (ps.point_list[i].p[0],
			ps.point_list[i].p[1],
			ps.point_list[i].p[2])));
	}
	else {
	    float t;
	    int rc = sscanf (ps.point_list[i].label.c_str(), "T%f", &t);
	    if (rc != 1) {
		/* Not a vertebra point */
		continue;
	    }
	    if (t > 0.25 && t < 12.75) {
		t_map.insert (std::pair<float,Point> (t, 
			Point (ps.point_list[i].p[0],
			    ps.point_list[i].p[1],
			    ps.point_list[i].p[2])));
	    }
	}
    }
    return t_map;
}

void
Autolabel_trainer::load_input_file (
    const char* nrrd_fn,
    const char* fcsv_fn)
{
    printf ("Loading %s\nLoading %s\n---\n", nrrd_fn, fcsv_fn);

    /* Load the pointset */
    Labeled_pointset ps;
    ps.load_fcsv (fcsv_fn);

    /* Load the input image */
#if defined (commentout)
    Plm_image *pli;
    pli = plm_image_load (nrrd_fn, PLM_IMG_TYPE_ITK_FLOAT);
#endif

    /* Create thumbnail to be filled in */
    Autolabel_thumbnailer thumb;
    thumb.set_input_image (nrrd_fn);
#if defined (commentout)
    Thumbnail thumb;
    thumb.set_input_image (pli);
    thumb.set_thumbnail_dim (16);
    thumb.set_thumbnail_spacing (25.0f);
#endif

    /* Get the samples and labels for learning tasks that convert 
       slice to location */
    if (this->m_dt_tsv1 || this->m_dt_tsv2_x || this->m_dt_tsv2_y) {

        /* Create map from spine # to point from fcsv file */
        std::map<float,Point> t_map = load_tspine_map (ps);

        /* If we want to use interpolation, we need to sort, and make 
           a "back-map" from z-pos to t-spine here.  But this is 
           not currently implemented. */

        std::map<float, Point>::iterator it;
        for (it = t_map.begin(); it != t_map.end(); ++it) {
#if defined (commentout)
            thumb.set_slice_loc (it->second.p[2]);
            FloatImageType::Pointer thumb_img = thumb.make_thumbnail ();
            Dlib_trainer::Dense_sample_type d 
                = Autolabel_task::make_sample (thumb_img);
#endif
            Dlib_trainer::Dense_sample_type d 
                = thumb.make_sample (it->second.p[2]);

            if (this->m_dt_tsv1) {
                this->m_dt_tsv1->m_samples.push_back (d);
                this->m_dt_tsv1->m_labels.push_back (it->first);
            }
            if (this->m_dt_tsv2_x) {
                this->m_dt_tsv2_x->m_samples.push_back (d);
                this->m_dt_tsv2_x->m_labels.push_back (it->second.p[0]);
            }
            if (this->m_dt_tsv2_y) {
                this->m_dt_tsv2_y->m_samples.push_back (d);
                this->m_dt_tsv2_y->m_labels.push_back (it->second.p[1]);
            }
        }
    }

    /* Get the samples and labels for learning tasks that iterate 
       through all slices */
    if (this->m_dt_la1) {
        bool have_lla = false;
        float *lla_point = 0;
        for (unsigned int i = 0; i < ps.point_list.size(); i++) {
            if (ps.point_list[i].label == "LLA") {
                have_lla = true;
                lla_point = ps.point_list[i].p;
            }
        }
        if (have_lla) {
            for (size_t i = 0; i < thumb.pli->dim(2); i++) {
                float loc = thumb.pli->origin(2) + i * thumb.pli->spacing(2);
#if defined (commentout)
                thumb.set_slice_loc (loc);
                FloatImageType::Pointer thumb_img = thumb.make_thumbnail ();
                Dlib_trainer::Dense_sample_type d 
                    = Autolabel_task::make_sample (thumb_img);
#endif
                Dlib_trainer::Dense_sample_type d = thumb.make_sample (loc);

                float score = fabs(loc - lla_point[2]);
                if (score > 30) score = 30;
                if (this->m_dt_la1) {
                    this->m_dt_la1->m_samples.push_back (d);
                    this->m_dt_la1->m_labels.push_back (score);
                }
            }
        }
    }
    //delete pli;
}

void
Autolabel_trainer::load_inputs ()
{
    if (m_task == "" || m_input_dir == "") {
	print_and_exit ("Error: inputs not fully specified.\n");
    }

    if (m_task == "la") {
        m_dt_la1 = new Dlib_trainer;
    }
    else if (m_task == "tsv1") {
	m_dt_tsv1 = new Dlib_trainer;
    }
    else if (m_task == "tsv2") {
	m_dt_tsv2_x = new Dlib_trainer;
	m_dt_tsv2_y = new Dlib_trainer;
    }
    else {
	print_and_exit ("Error: unsupported autolabel-train task (%s)\n",
	    m_task.c_str());
    }

    load_input_dir_recursive (m_input_dir);
}

void
Autolabel_trainer::set_task (const char* task)
{
    m_task = task;
}

void
Autolabel_trainer::train ()
{
    if (this->m_dt_tsv1) {
        std::string output_net_fn = string_format (
            "%s/tsv1.net", m_output_dir.c_str());
        m_dt_tsv1->set_krr_gamma (-9, -6, 0.5);
        m_dt_tsv1->train_krr ();
        m_dt_tsv1->save_net (output_net_fn);
    }
    if (this->m_dt_tsv2_x) {
        std::string output_net_fn = string_format (
            "%s/tsv2_x.net", m_output_dir.c_str());
        m_dt_tsv2_x->set_krr_gamma (-9, -6, 0.5);
        m_dt_tsv2_x->train_krr ();
        m_dt_tsv2_x->save_net (output_net_fn);
    }
    if (this->m_dt_tsv2_y) {
        std::string output_net_fn = string_format (
            "%s/tsv2_y.net", m_output_dir.c_str());
        m_dt_tsv2_y->set_krr_gamma (-9, -6, 0.5);
        m_dt_tsv2_y->train_krr ();
        m_dt_tsv2_y->save_net (output_net_fn);
    }
    if (this->m_dt_la1) {
        std::string output_net_fn = string_format (
            "%s/la1.net", m_output_dir.c_str());
        m_dt_la1->set_krr_gamma (-9, -6, 0.5);
        m_dt_la1->train_krr ();
        m_dt_la1->save_net (output_net_fn);
    }
}

void
Autolabel_trainer::save_csv ()
{
    if (this->m_dt_tsv1) {
        std::string output_csv_fn = string_format (
            "%s/tsv1.csv", m_output_dir.c_str());
        this->m_dt_tsv1->save_csv (output_csv_fn);
    }
    if (this->m_dt_tsv2_x) {
        std::string output_csv_fn = string_format (
            "%s/tsv2_x.csv", m_output_dir.c_str());
        this->m_dt_tsv2_x->save_csv (output_csv_fn);
    }
    if (this->m_dt_tsv2_y) {
        std::string output_csv_fn = string_format (
            "%s/tsv2_y.csv", m_output_dir.c_str());
        this->m_dt_tsv2_y->save_csv (output_csv_fn);
    }
    if (this->m_dt_la1) {
        std::string output_csv_fn = string_format (
            "%s/la1.csv", m_output_dir.c_str());
        this->m_dt_la1->save_csv (output_csv_fn);
    }
}

/* tsacc = testset accuracy */
void
Autolabel_trainer::save_tsacc ()
{
    /* Save the output files */
    if (this->m_dt_tsv1) {
        std::string output_tsacc_fn = string_format (
            "%s/tsv1_tsacc.txt", m_output_dir.c_str());
        this->m_dt_tsv1->save_tsacc (output_tsacc_fn);
    }
    if (this->m_dt_tsv2_x) {
        std::string output_tsacc_fn = string_format (
            "%s/tsv2_x_tsacc.txt", m_output_dir.c_str());
        this->m_dt_tsv2_x->save_tsacc (output_tsacc_fn);
    }
    if (this->m_dt_tsv2_y) {
        std::string output_tsacc_fn = string_format (
            "%s/tsv2_y_tsacc.txt", m_output_dir.c_str());
        this->m_dt_tsv2_y->save_tsacc (output_tsacc_fn);
    }
    if (this->m_dt_la1) {
        std::string output_tsacc_fn = string_format (
            "%s/la1_tsacc.txt", m_output_dir.c_str());
        this->m_dt_la1->save_tsacc (output_tsacc_fn);
    }
}

void
autolabel_train (Autolabel_train_parms *parms)
{
    Autolabel_trainer at;

    at.set_input_dir (parms->input_dir.c_str());
    at.m_output_dir = parms->output_dir;
    at.set_task (parms->task.c_str());
    at.load_inputs ();
    at.train ();
    at.save_csv ();
    at.save_tsacc ();
}
