/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dlib_trainer_h_
#define _dlib_trainer_h_

#include "plmsegment_config.h"
#include <vector>
#include "dlib/svm.h"

class PLMSEGMENT_API Dlib_trainer
{
  public:
    Dlib_trainer () {
	m_gamma_search = false;
	m_gamma_min = -7;
	m_gamma_max = -7;
	m_gamma_step = 1;
	m_verbose = true;
    }
    ~Dlib_trainer () {}

  public:
    //typedef std::map<unsigned long, double> Sparse_sample_type;
    typedef dlib::matrix < double, 256, 1 > Dense_sample_type;
    typedef double Label_type;
    typedef dlib::radial_basis_kernel < 
	Dlib_trainer::Dense_sample_type > Kernel_type;

  public:
    std::string m_algorithm;
    std::vector<Dense_sample_type> m_samples;
    std::vector<Label_type> m_labels;

  public:
    bool m_gamma_search;
    double m_gamma_min;
    double m_gamma_max;
    double m_gamma_step;
    bool m_verbose;
    dlib::decision_function<Kernel_type> m_krr_df;

  public:
    void save_csv (const std::string& out_csv_fn);
    void save_net (const std::string& out_net_fn);
    void save_tsacc (const std::string& out_net_fn);
    void set_krr_gamma (
	double gamma_min, double gamma_max, double gamma_step);
    void train_krr (void);
};

#endif
