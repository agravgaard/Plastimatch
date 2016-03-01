// The contents of this file are in the public domain. 
// See LICENSE_FOR_EXAMPLE_PROGRAMS.txt (in dlib)
/*
    This is a command line program that can try different regression 
    algorithms on a libsvm-formatted data set.
*/
#include "plm_config.h"
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include <float.h>

#include "dlib/cmd_line_parser.h"
#include "dlib/data_io.h"
#include "dlib/mlp.h"
#include "dlib/revision.h"
#include "dlib/svm.h"
//#include "dlib/svm_threaded.h"

#include "option_range.h"
#include "plm_clp.h"
#include "plm_math.h"

typedef std::map<unsigned long, double> sparse_sample_type;
typedef dlib::matrix< sparse_sample_type::value_type::second_type,0,1
		> dense_sample_type;
typedef double label_type;

class Dlib_test_parms {
public:
    int a;
};

/* This particular is modified from libsvm_io.h, therefore part of 
   dlib and distributed according to Boost license. */
namespace dlib {
template <typename sample_type, typename label_type, typename alloc1, typename alloc2>
void load_libsvm_or_vw_data (
    const std::string& file_name,
    std::vector<sample_type, alloc1>& samples,
    std::vector<label_type, alloc2>& labels
)
{
    using namespace std;
    typedef typename sample_type::value_type pair_type;
    typedef typename basic_type<typename pair_type::first_type>::type key_type;
    typedef typename pair_type::second_type value_type;

    // You must use unsigned integral key types in your sparse vectors
    COMPILE_TIME_ASSERT(is_unsigned_type<key_type>::value);

    samples.clear();
    labels.clear();

    ifstream fin(file_name.c_str());

    if (!fin)
        throw sample_data_io_error("Unable to open file " + file_name);

    string line;
    istringstream sin;
    key_type key;
    value_type value;
    label_type label;
    sample_type sample;
    long line_num = 0;
    while (fin.peek() != EOF)
    {
        ++line_num;
        getline(fin, line);

        string::size_type pos = line.find_first_not_of(" \t\r\n");

        // ignore empty lines or comment lines
        if (pos == string::npos || line[pos] == '#')
            continue;

        sin.clear();
        sin.str(line);
        sample.clear();

        sin >> label;

        if (!sin)
            throw sample_data_io_error("On line: " + cast_to_string(line_num) + ", error while reading file " + file_name );

        // eat whitespace
        sin >> ws;

        /* Ignore pipe in vw format */
        if (sin.peek() == '|') {
            sin.get();
            sin >> ws;
        }

        while (sin.peek() != EOF && sin.peek() != '#')
        {

            sin >> key >> ws;

            // ignore what should be a : character
            if (sin.get() != ':')
                throw sample_data_io_error("On line: " + cast_to_string(line_num) + ", error while reading file " + file_name);

            sin >> value >> ws;

            if (sin && value != 0)
            {
                sample.insert(sample.end(), make_pair(key, value));
            }
        }

        samples.push_back(sample);
        labels.push_back(label);
    }
}
}

static void
set_range (
    Option_range& range, 
    dlib::Plm_clp* parser, 
    const std::string& option_name, 
    float default_value
) {
    if (parser->option (option_name)) {
        range.set_range (parser->get_string (option_name));
    } else {
        range.set_range (default_value);
    }
}

static float
get_positive_rate (
    std::vector<label_type>& labels
) {
    std::vector<label_type>::const_iterator label_it;
    int np = 0;
    int tot = 0;
    for (label_it = labels.begin(); label_it != labels.end(); label_it++)
    {
        if (*label_it > 0) {
            np++;
        }
        tot ++;
    }
    return (float) np / (float) tot;
}

static std::string
get_kernel (
    dlib::Plm_clp* parser
)
{
    std::string kernel = "rbk";
    if (parser->option ("learning-kernel")) {
	kernel = parser->get_string ("learning-kernel");
    }
    return kernel;
}

static void
get_rbk_gamma (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    Option_range& range
) {
    float default_gamma = 3.0 / compute_mean_squared_distance (
	randomly_subsample (dense_samples, 2000));
    set_range (range, parser, "rbk-gamma", default_gamma);
}

static void
get_krls_tolerance (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples, 
    Option_range& range
)
{
    float default_krls_tolerance = 0.001;
    set_range (range, parser, "krls-tolerance", default_krls_tolerance);
}

static double
get_mlp_hidden_units (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples
)
{
    int num_hidden = 5;
    if (parser->option ("mlp-hidden-units")) {
	num_hidden = parser->get_int ("mlp-hidden-units");
    }
    return num_hidden;
}

static double
get_mlp_num_iterations (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples
)
{
    int num_iterations = 5000;
    if (parser->option ("mlp-num-iterations")) {
	num_iterations = parser->get_int ("mlp-num-iterations");
    }
    return num_iterations;
}

static void
get_svc (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples, 
    Option_range& range
)
{
    float default_svc = 1000.;
    set_range (range, parser, "sv-c", default_svc);
}

#if defined (commentout)
static double
get_svr_epsilon_insensitivity (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples
)
{
    // Epsilon-insensitive regression means we do regression but stop 
    // trying to fit a data point once it is "close enough" to its 
    // target value.  This parameter is the value that controls what 
    // we mean by "close enough".  In this case, I'm saying I'm happy 
    // if the resulting regression function gets within 0.001 of the 
    // target value.
    double epsilon_insensitivity = 0.001;
    if (parser->option ("svr-epsilon-insensitivity")) {
	epsilon_insensitivity 
	    = sa = parser->option("svr-epsilon-insensitivity").argument();
    }
    return epsilon_insensitivity;
}
#endif

static void
krls_test (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    typedef dlib::radial_basis_kernel<dense_sample_type> kernel_type;
    Option_range gamma_range, krls_tol_range;

    get_rbk_gamma (parser, dense_samples, gamma_range);
    get_krls_tolerance (parser, dense_samples, krls_tol_range);

    // Split into training set and testing set
    float training_pct = 0.8;
    unsigned int training_samples = (unsigned int) floor (
	training_pct * dense_samples.size());

    const std::list<float>& krls_tol_list = krls_tol_range.get_range ();
    std::list<float>::const_iterator krls_tol_it;
    for (krls_tol_it = krls_tol_list.begin ();
	 krls_tol_it != krls_tol_list.end (); krls_tol_it++)
    {
        float krls_tol = *krls_tol_it;
        const std::list<float>& gamma_list = gamma_range.get_range ();
        std::list<float>::const_iterator gamma_it;
	for (gamma_it = gamma_list.begin ();
	     gamma_it != gamma_list.end (); gamma_it ++)
	{
            float gamma = *gamma_it;
            dlib::krls<kernel_type> net (kernel_type(gamma), krls_tol);

	    // Krls doesn't seem to come with any batch training function
	    for (unsigned int j = 0; j < training_samples; j++) {
		net.train (dense_samples[j], labels[j]);
	    }

	    // Test the performance (sorry, no cross-validation)
	    double total_err = 0.0;
	    for (unsigned int j = training_samples + 1; 
		 j < dense_samples.size(); j++)
	    {
		double diff = net(dense_samples[j]) - labels[j];
		total_err += diff * diff;
	    }

	    double testset_error = total_err 
		/ (dense_samples.size() - training_samples);
	    printf ("%3.6f %3.6f %3.9f\n", krls_tol, gamma, testset_error);
	}
    }
}

static void
krr_rbk_test (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    typedef dlib::radial_basis_kernel<dense_sample_type> kernel_type;
    dlib::krr_trainer<kernel_type> trainer;
    Option_range gamma_range;
    double best_gamma = DBL_MAX;
    float best_loo = FLT_MAX;

    get_rbk_gamma (parser, dense_samples, gamma_range);

    const std::list<float>& gamma_list = gamma_range.get_range ();
    std::list<float>::const_iterator gamma_it;
    for (gamma_it = gamma_list.begin ();
         gamma_it != gamma_list.end (); gamma_it ++)
    {
        float gamma = *gamma_it;
	// LOO cross validation
	double loo_error = 0.;
	if (parser->option("verbose")) {
	    trainer.set_search_lambdas(dlib::logspace(-9, 4, 100));
	    trainer.be_verbose();
	}
	trainer.set_kernel (kernel_type (gamma));

#if DLIB_REVISION == 4093
        /* dlib 17.34 */
	trainer.train (dense_samples, labels, loo_error);

#elif DLIB_MAJOR_VERSION > 17 || (DLIB_MAJOR_VERSION == 17 && DLIB_MINOR_VERSION >= 44)
        /* dlib 17.44, dlib 18.7 */
        std::vector<double> loo_values;
	double lambda_used;
	trainer.train (dense_samples, labels, loo_values, lambda_used);
        loo_error = dlib::mean_squared_error (labels, loo_values);
#else
        error, unknown DLIB version!;
#endif

	if (loo_error < best_loo) {
	    best_loo = loo_error;
	    best_gamma = gamma;
	}
	printf ("10^%f %9.6f\n", log10(gamma), loo_error);
    }

    printf ("Best result: gamma=10^%f (%g), loo_error=%9.6f\n",
	log10(best_gamma), best_gamma, best_loo);
    if (parser->option("train-best")) {
	printf ("Training network with best parameters\n");
	trainer.set_kernel (kernel_type (best_gamma));
        dlib::decision_function<kernel_type> best_network = 
	    trainer.train (dense_samples, labels);

	std::ofstream fout (parser->option("train-best").argument().c_str(), 
	    std::ios::binary);
	serialize (best_network, fout);
	fout.close();

#if defined (commentout)
	for (unsigned int j = 0; j < dense_samples.size(); j++) {
	    printf ("%g %g\n", labels[j], best_network(dense_samples[j]));
	}
#endif
    }
}

static void
krr_lin_test (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    typedef dlib::linear_kernel<dense_sample_type> kernel_type;
    dlib::krr_trainer<kernel_type> trainer;

    // LOO cross validation
    double loo_error;

#if DLIB_REVISION == 4093
        /* dlib 17.34 */
	trainer.train (dense_samples, labels, loo_error);

#elif DLIB_MAJOR_VERSION > 17 || (DLIB_MAJOR_VERSION == 17 && DLIB_MINOR_VERSION >= 44)
        /* dlib 17.44, dlib 18.7 */
        std::vector<double> loo_values;
	double lambda_used;
	trainer.train (dense_samples, labels, loo_values, lambda_used);
        loo_error = dlib::mean_squared_error (labels, loo_values);
#else
        error, unknown DLIB version!;
#endif
    std::cout << "mean squared LOO error: " << loo_error << std::endl;
}

static void
krr_test (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    const std::string& kernel = get_kernel (parser);

    if (kernel == "lin") {
	krr_lin_test (parser, dense_samples, labels);
    } else if (kernel == "rbk") {
	krr_rbk_test (parser, dense_samples, labels);
    } else {
	fprintf (stderr, "Unknown kernel type: %s\n", kernel.c_str());
	exit (-1);
    }
}

static void
mlp_test (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    // Create a multi-layer perceptron network.
    const int num_input = dense_samples[0].size();
    int num_hidden = get_mlp_hidden_units (parser, dense_samples);
    printf ("Creating ANN with size (%d, %d)\n", num_input, num_hidden);
    dlib::mlp::kernel_1a_c net (num_input, num_hidden);

    // Dlib barfs if output values are not normalized to [0,1]
    double label_min = *(std::min_element (labels.begin(), labels.end()));
    double label_max = *(std::max_element (labels.begin(), labels.end()));
    std::vector<label_type>::iterator it;
    for (it = labels.begin(); it != labels.end(); it++) {
	(*it) = ((*it) - label_min) / (label_max - label_min);
    }

    // Split into training set and testing set
    float training_pct = 0.8;
    unsigned int training_samples = (unsigned int) floor (
	training_pct * dense_samples.size());

    // Dlib doesn't seem to come with any batch training functions for mlp.
    // Also, note that only backprop is supported.
    int num_iterations = get_mlp_num_iterations (parser, dense_samples);
    for (int i = 0; i < num_iterations; i++) {
	for (unsigned int j = 0; j < training_samples; j++) {
	    net.train (dense_samples[j], labels[j]);
	}
    }

    // Test the performance (sorry, no cross-validation) */
    double total_err = 0.0;
    for (unsigned int j = training_samples + 1; j < dense_samples.size(); j++)
    {
	double diff = net(dense_samples[j]) - labels[j];
	diff = diff * (label_max - label_min);
	total_err += diff * diff;
    }
    std::cout 
	<< "MSE (no cross-validation): " 
	<< total_err / (dense_samples.size() - training_samples) << std::endl;
}

static void
svr_lin_test (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    Option_range svc_range;

#if DLIB_REVISION == 4093
    /* dlib 17.34 */
    double best_svc = DBL_MAX;
    float best_cv_error = FLT_MAX;

    typedef linear_kernel<dense_sample_type> kernel_type;
    svr_trainer<kernel_type> trainer;

    double epsilon_insensitivity = get_svr_epsilon_insensitivity (
        parser, dense_samples);
    trainer.set_epsilon_insensitivity (epsilon_insensitivity);

    double cv_error;
    for (float svc = svc_range.get_min_value();
	 svc <= svc_range.get_max_value();
         svc = svc_range.get_next_value(svc))
    {
        trainer.set_c (svc);
        cv_error = cross_validate_regression_trainer (trainer,
            dense_samples, labels, 10);
        if (cv_error < best_cv_error) {
            best_cv_error = cv_error;
            best_svc = svc;
	}
        printf ("%3.6f %3.9f\n", svc, cv_error);
    }

    printf ("Best result: svc=%3.6f, cv_error=%9.6f\n",
        best_svc, best_cv_error);

    if (parser->option("train-best")) {
        printf ("Training network with best parameters\n");
        trainer.set_c (best_svc);
        decision_function<kernel_type> best_network =
            trainer.train (dense_samples, labels);

        std::ofstream fout (parser->option("train-best").argument().c_str(),
            std::ios::binary);
        serialize (best_network, fout);
        fout.close();

        for (unsigned int j = 0; j < dense_samples.size(); j++) {
            printf ("%g %g\n", labels[j], best_network(dense_samples[j]));
        }
    }
#elif DLIB_MAJOR_VERSION > 17 || (DLIB_MAJOR_VERSION == 17 && DLIB_MINOR_VERSION >= 44)
    /* dlib 17.44, dlib 18.7 */
    /* GCS FIX: The above doesn't compile. */
#else
    error, unknown DLIB version!;
#endif
}


static void
svr_rbk_test (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    typedef dlib::radial_basis_kernel<dense_sample_type> kernel_type;
    dlib::svr_trainer<kernel_type> trainer;
    Option_range gamma_range, svc_range;

#if DLIB_REVISION == 4093
    /* dlib 17.34 */
    double best_gamma = DBL_MAX;
    double best_svc = DBL_MAX;
    float best_cv_error = FLT_MAX;

    get_rbk_gamma (parser, dense_samples, gamma_range);
    get_svc (parser, dense_samples, svc_range);

    double epsilon_insensitivity = get_svr_epsilon_insensitivity (
	parser, dense_samples);
    trainer.set_epsilon_insensitivity (epsilon_insensitivity);

    for (float svc = svc_range.get_min_value(); 
	 svc <= svc_range.get_max_value();
	 svc = svc_range.get_next_value (svc))
    {
	trainer.set_c (svc);
	for (float gamma = gamma_range.get_min_value(); 
	     gamma <= gamma_range.get_max_value();
	     gamma = gamma_range.get_next_value (gamma))
	{
	    double cv_error;
	    trainer.set_kernel (kernel_type (gamma));
	    cv_error = cross_validate_regression_trainer (trainer, 
		dense_samples, labels, 10);
            if (cv_error < best_cv_error) {
                best_cv_error = cv_error;
                best_gamma = gamma;
                best_svc = svc;
            }
	    printf ("%3.6f %3.6f %3.9f\n", svc, gamma, cv_error);
	}
    }
    printf ("Best result: svc=%3.6f gamma=10^%f (%g), cv_error=%9.6f\n",
        best_svc, log10(best_gamma), best_gamma, best_cv_error);
 
    if (parser->option("train-best")) {
        printf ("Training network with best parameters\n");
	trainer.set_c (best_svc);
        trainer.set_kernel (kernel_type (best_gamma));
        decision_function<kernel_type> best_network =
            trainer.train (dense_samples, labels);

        std::ofstream fout (parser->option("train-best").argument().c_str(),
            std::ios::binary);
        serialize (best_network, fout);
        fout.close();

        for (unsigned int j = 0; j < dense_samples.size(); j++) {
            printf ("%g %g\n", labels[j], best_network(dense_samples[j]));
        }
    }
#elif DLIB_MAJOR_VERSION > 17 || (DLIB_MAJOR_VERSION == 17 && DLIB_MINOR_VERSION >= 44)
    /* dlib 17.44, dlib 18.7 */
    /* GCS FIX: The above doesn't compile. */
#else
    error, unknown DLIB version!;
#endif
}

static void 
svr_test (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    const std::string& kernel = get_kernel (parser);

    if (kernel == "lin") {
        svr_lin_test (parser, dense_samples, labels);
    } else if (kernel == "rbk") {
        svr_rbk_test (parser, dense_samples, labels);
    } else {
        fprintf (stderr, "Unknown kernel type: %s\n", kernel.c_str());
        exit (-1);
    }
}

static void
svm_lin_test (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    typedef dlib::radial_basis_kernel<dense_sample_type> kernel_type;

    dlib::svm_c_trainer<kernel_type> trainer;

    double best_gamma = DBL_MAX;
    double best_svc = DBL_MAX;
    float best_loss = FLT_MAX;

    Option_range gamma_range;
    Option_range svc_range;
    get_rbk_gamma (parser, dense_samples, gamma_range);
    get_svc (parser, dense_samples, svc_range);

    float positive_rate = get_positive_rate (labels);
    printf ("Positive rate = %3.6f\n", positive_rate);

    const std::list<float>& svc_list = svc_range.get_range ();
    std::list<float>::const_iterator svc_it;
    for (svc_it = svc_list.begin ();
         svc_it != svc_list.end (); svc_it ++)
    {
        const std::list<float>& gamma_list = gamma_range.get_range ();
        std::list<float>::const_iterator gamma_it;
        for (gamma_it = gamma_list.begin ();
             gamma_it != gamma_list.end (); gamma_it ++)
        {
            float svc = *svc_it;
            float gamma = *gamma_it;
            trainer.set_kernel (kernel_type (gamma));
            trainer.set_c (svc);
            dlib::matrix<double,1,2> cv_error;
            cv_error = dlib::cross_validate_trainer (
                trainer, dense_samples, labels, 3);
            /* cv_error(0) = percent correct positive = TP / (TP + FN)
               cv_error(1) = percent correct negative = TN / (TN + FP) */
            float loss = cv_error(0) * positive_rate 
                + cv_error(1) * (1 - positive_rate);
            float tp = cv_error(0) * positive_rate;
            // float tn = cv_error(1) * (1 - positive_rate);
            float fn = (1 - cv_error(0)) * positive_rate;
            float fp = (1 - cv_error(1)) * (1 - positive_rate);
            float dice = 2 * tp / (2 * tp + fn + fp);
            if (loss < best_loss) {
                best_loss = loss;
                best_gamma = gamma;
                best_svc = svc;
            }
            printf ("%3.6f %.3e %3.6f %3.6f %3.6f %3.6f\n", 
                svc, gamma, cv_error(0), cv_error(1), loss, dice);
        }
    }

    if (parser->option("train-best")) {
	printf ("Training network with best parameters\n");
	trainer.set_kernel (kernel_type (best_svc));
	trainer.set_kernel (kernel_type (best_gamma));
        dlib::decision_function<kernel_type> best_network = 
	    trainer.train (dense_samples, labels);

	std::ofstream fout (parser->option("train-best").argument().c_str(), 
	    std::ios::binary);
	serialize (best_network, fout);
	fout.close();
    }
}

static void svm_train  (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    const std::string& kernel = get_kernel (parser);

    if (kernel == "lin") {
        svm_lin_test (parser, dense_samples, labels);
    } else if (kernel == "rbk") {
        svm_lin_test (parser, dense_samples, labels);
    } else {
        fprintf (stderr, "Unknown kernel type: %s\n", kernel.c_str());
        exit (-1);
    }
}


static void svm_test  (
    dlib::Plm_clp* parser,
    std::vector<dense_sample_type>& dense_samples,
    std::vector<label_type>& labels
)
{
    /* Load the network */
    typedef dlib::radial_basis_kernel<dense_sample_type> kernel_type;
    dlib::svm_c_trainer<kernel_type> trainer;
    dlib::decision_function<kernel_type> best_network;
    std::ifstream fin (parser->option("input-net").argument().c_str(), 
        std::ios::binary);
    deserialize (best_network, fin);

    size_t tp = 0, fp = 0, fn = 0, tn = 0;
    for (size_t j = 0; j < dense_samples.size(); j++) {
        if (best_network(dense_samples[j]) > 0) {
            if (labels[j] > 0) {
                tp ++;
            } else {
                fp ++;
            }
        } else {
            if (labels[j] > 0) {
                fn ++;
            } else {
                tn ++;
            }
        }
    }
    printf ("Testing result: TP=%d TN=%d FP=%d FN=%d Loss=%f Dice=%f\n",
        (int) tp, (int) tn, (int) fp, (int) fn,
        (fp + fn) / (float) (fp + fn + tp + tn),
        (2 * tp) / (float) (2 * tp + fp + fn));
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    std::cout << "Usage: dlib_test [options]\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Dlib_test_parms *parms, dlib::Plm_clp* parser, int argc, char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    // Algorithm-independent options
    parser->add_long_option ("", "learning-algorithm", 
        "choose the learning algorithm: {krls,krr,mlp,svr}", 1, "");
    parser->add_long_option ("", "learning-kernel", 
        "Learning kernel (for krls,krr,svr methods): {lin,rbk}", 1, "");
    parser->add_long_option ("", "input", 
        "A libsvm-formatted file to test", 1, "");
    parser->add_long_option ("", "input-net", 
        "Test a pre-existing network", 1, "");
    parser->add_long_option ("", "normalize",
        "Normalize the sample inputs to zero-mean unit variance?", 0);
    parser->add_long_option ("", "subsample", 
        "limit size of training set to this number: {int}", 1, "");
    parser->add_long_option ("", "train-best",
        "Train and save a network using best parameters", 1, "");

    // Algorithm-specific options
    parser->add_long_option ("", "rbk-gamma",
        "Width of radial basis kernels: {float}.", 1, "");
    parser->add_long_option ("", "krls-tolerance",
        "Numerical tolerance of krls linear dependency test: {float}.", 1, "");
    parser->add_long_option ("", "mlp-hidden-units",
        "Number of hidden units in mlp: {integer}.", 1, "");
    parser->add_long_option ("", "mlp-num-iterations",
        "Number of epochs to train the mlp: {integer}.", 1, "");
    parser->add_long_option ("", "sv-c",
        "SV regularization parameter \"C\": {float}.", 1, "");
    parser->add_long_option ("", "svr-epsilon-insensitivity",
        "SVR fitting tolerance parameter: {float}.", 1, "");
    parser->add_long_option ("", "verbose", "Use verbose trainers", 0);

    // Parse the command line arguments
    parser->parse(argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    // Check that an input file was given
    if (!parser->option("input")) {
        throw (dlib::error (
                "Error.  You must specify an input "
                "file with the --in option."));
    }

    /* DO THE TEST */
    std::string learning_algorithm = parser->get_string ("learning-algorithm");
    std::string input_data = parser->get_string ("input");
    std::string input_net = parser->get_string ("input-net");

    std::vector<sparse_sample_type> sparse_samples;
    std::vector<label_type> labels;

    /* Load the data */
    dlib::load_libsvm_or_vw_data (
	input_data, 
	sparse_samples, 
	labels
    );
    std::cout 
	<< "Loaded " << sparse_samples.size() << " samples"
            << std::endl
            << "First sample " << labels[0] << " | " 
            << sparse_samples[0][1] << " "
            << sparse_samples[0][2] << " ...\n";

    /* Randomize the order of the samples, labels */
    dlib::randomize_samples (sparse_samples, labels);

    /* Take a subset */
    if (parser->option ("subsample")) {
        int num_subsamples = parser->get_int ("subsample");
        int original_size = sparse_samples.size();
        if (num_subsamples < original_size) {
            sparse_samples.resize (num_subsamples);
            labels.resize (num_subsamples);
        }
    }

    /* Remove empty feature */
    dlib::fix_nonzero_indexing (sparse_samples);

    if (sparse_samples.size() < 1) {
	std::cout 
	    << "Sorry, I couldn't find any samples in your data set.\n"
                << "Aborting the operation.\n";
	exit (0);
    }

    /* Convert sparse to dense */
    std::vector<dense_sample_type> dense_samples;
    dense_samples = dlib::sparse_to_dense (sparse_samples);

    std::cout << "Each sample has size " << dense_samples[0].nr()
        << std::endl;

    /* Normalize inputs to N(0,1) */
    if (parser->option ("normalize")) {
        dlib::vector_normalizer<dense_sample_type> normalizer;
	normalizer.train (dense_samples);
	for (unsigned long i = 0; i < dense_samples.size(); ++i) {
	    dense_samples[i] = normalizer (dense_samples[i]);
	}
    }

    /* Handle the case where we are testing, and not training */
    if (input_net != "") {
        if (learning_algorithm == "svm") {
            svm_test (parser, dense_samples, labels);
        }
    }

    else {
        if (learning_algorithm == "") {
            // Do KRR if user didn't specify an algorithm
            std::cout << "No algorithm specified, default to KRR\n";
            krr_test (parser, dense_samples, labels);
        }
        else if (learning_algorithm == "krls") {
            krls_test (parser, dense_samples, labels);
        }
        else if (learning_algorithm == "krr") {
            krr_test (parser, dense_samples, labels);
        }
        else if (learning_algorithm == "mlp") {
            mlp_test (parser, dense_samples, labels);
        }
        else if (learning_algorithm == "svr") {
            svr_test (parser, dense_samples, labels);
        }
        else if (learning_algorithm == "svm") {
            svm_train (parser, dense_samples, labels);
        }
        else {
            fprintf (stderr, 
                "Error, algorithm \"%s\" is unknown.\n"
                "Please use -h to see the command line options\n",
                learning_algorithm.c_str());
            exit (-1);
        }
    }

}

int 
main (int argc, char* argv[])
{
    Dlib_test_parms parms;

    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 0);
}
