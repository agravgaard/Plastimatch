/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsegment_config.h"
#include <iostream>
#include <map>
#include <vector>
#include <float.h>
#include <math.h>
#include "dlib/cmd_line_parser.h"
#include "dlib/data_io.h"
#include "dlib/mlp.h"
#include "dlib/revision.h"
#include "dlib/svm.h"

#include "dlib_trainer.h"
#include "file_util.h"
#include "plm_math.h"

using namespace dlib;

typedef dlib::cmd_line_parser<char>::check_1a_c clp;
typedef std::map<unsigned long, double> sparse_sample_type;
typedef dlib::matrix< sparse_sample_type::value_type::second_type,0,1
                      > dense_sample_type;
typedef double label_type;

/* ---------------------------------------------------------------------
   option_range class
   --------------------------------------------------------------------- */
struct option_range {
public:
    bool log_range;
    float min_value;
    float max_value;
    float incr;
public:
    option_range () {
        log_range = false;
        min_value = 0;
        max_value = 100;
        incr = 10;
    }
    void set_option (clp& parser, std::string const& option, 
        float default_val);
    float get_min_value ();
    float get_max_value ();
    float get_next_value (float curr_val);
};

void
option_range::set_option (
    clp& parser,
    std::string const& option, 
    float default_val
)
{
    int rc;

    /* No option specified */
    if (!parser.option (option)) {
        log_range = 0;
        min_value = default_val;
        max_value = default_val;
        incr = 1;
        return;
    }

    /* Range specified */
    rc = sscanf (parser.option(option).argument().c_str(), "%f:%f:%f", 
        &min_value, &incr, &max_value);
    if (rc == 3) {
        log_range = 1;
        return;
    }

    /* Single value specified */
    if (rc == 1) {
        log_range = 0;
        max_value = min_value;
        incr = 1;
        return;
    }

    else {
        std::cerr << "Error parsing option" << option << "\n";
        exit (-1);
    }
}

float 
option_range::get_min_value ()
{
    if (log_range) {
        return exp10_ (min_value);
    } else {
        return min_value;
    }
}

float 
option_range::get_max_value ()
{
    if (log_range) {
        return exp10_ (max_value);
    } else {
        return max_value;
    }
}

float 
option_range::get_next_value (float curr_value)
{
    if (log_range) {
        curr_value = log10 (curr_value);
        curr_value += incr;
        curr_value = exp10_ (curr_value);
    } else {
        curr_value += incr;
    }
    return curr_value;
}

/* ---------------------------------------------------------------------
   training functions
   --------------------------------------------------------------------- */
void
Dlib_trainer::save_csv (const std::string& out_csv_fn)
{
    /* Save the output file */
    printf ("Saving csv...\n");
    make_parent_directories (out_csv_fn);
    FILE *fp = plm_fopen (out_csv_fn, "w");
    std::vector<Dlib_trainer::Dense_sample_type>::iterator s_it
	= this->m_samples.begin();
    std::vector<Dlib_trainer::Label_type>::iterator l_it
	= this->m_labels.begin();
    while (s_it != this->m_samples.end()) {
	fprintf (fp, "%f,", *l_it);
	for (int i = 0; i < 256; i++) {
	    fprintf (fp, ",%f", (*s_it)(i));
	}
	fprintf (fp, "\n");
	++s_it, ++l_it;
    }
    fclose (fp);
    printf ("Done.\n");
}

void
Dlib_trainer::save_net (const std::string& out_net_fn)
{
    make_parent_directories (out_net_fn.c_str());
    std::ofstream fout (out_net_fn.c_str(), std::ios::binary);
    serialize (m_krr_df, fout);
    fout.close();

#if defined (commentout)
    for (unsigned int j = 0; j < dense_samples.size(); j++) {
        printf ("%g %g\n", labels[j], best_network(dense_samples[j]));
    }
#endif
}

void
Dlib_trainer::save_tsacc (const std::string& out_net_fn)
{
    FILE *fp = plm_fopen (out_net_fn, "w");
    for (unsigned int j = 0; j < m_samples.size(); j++) {
        fprintf (fp, "%g %g\n", m_labels[j], m_krr_df(m_samples[j]));
    }
    fclose (fp);
}

void
Dlib_trainer::set_krr_gamma (
    double gamma_min, double gamma_max, double gamma_step)
{
    m_gamma_search = true;
    m_gamma_min = gamma_min;
    m_gamma_max = gamma_max;
    m_gamma_step = gamma_step;
}

void
Dlib_trainer::train_krr (void)
{
    //typedef radial_basis_kernel<dense_sample_type> kernel_type;
    krr_trainer<Kernel_type> trainer;
    option_range gamma_range;
    double best_gamma = DBL_MAX;
    float best_loo = FLT_MAX;

    //get_rbk_gamma (parser, dense_samples, gamma_range);

    for (float log_gamma = m_gamma_min;
         log_gamma <= m_gamma_max;
         log_gamma += m_gamma_step)
    {
        // LOO cross validation
        double loo_error;

        if (m_verbose) {
            trainer.set_search_lambdas(logspace(-9, 4, 100));
            trainer.be_verbose();
        }
        double gamma = exp10_ (log_gamma);
        trainer.set_kernel (Kernel_type (gamma));
#if DLIB_REVISION == 4093
        /* dlib 17.34 */
        dlib::decision_function<Kernel_type> krr_df = 
            trainer.train (m_samples, m_labels, loo_error);
#elif DLIB_MAJOR_VERSION > 17 || (DLIB_MAJOR_VERSION == 17 && DLIB_MINOR_VERSION >= 44)
        /* dlib 17.44, dlib 18.7 */
        /* GCS FIX: How to get loo_error from loo_values? */
        std::vector<double> loo_values;
        double lambda_used;
        dlib::decision_function<Kernel_type> krr_df = 
            trainer.train (m_samples, m_labels, loo_values, lambda_used);

        loo_error = mean_squared_error (m_labels, loo_values);
#else
        error, unknown DLIB version!;
#endif

        if (loo_error < best_loo) {
            best_loo = loo_error;
            best_gamma = gamma;
            m_krr_df = krr_df;
        }
        printf ("10^%f %9.6f\n", log_gamma, loo_error);
    }

    printf ("Best result: gamma=10^%f (%g), loo_error=%9.6f\n",
        log10(best_gamma), best_gamma, best_loo);
}
