/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _autolabel_trainer_h_
#define _autolabel_trainer_h_

#include "plmsegment_config.h"
#include <map>
#include <stdio.h>

class Dlib_trainer;

class PLMSEGMENT_API Autolabel_trainer
{
  public:
    Autolabel_trainer ();
    ~Autolabel_trainer ();

  public:
    std::string m_output_dir;
    std::string m_input_dir;
    std::string m_task;

  private:
    Dlib_trainer *m_dt_tsv1;
    Dlib_trainer *m_dt_tsv2_x;
    Dlib_trainer *m_dt_tsv2_y;
    Dlib_trainer *m_dt_la1;

  public:
    void load_inputs ();
    void save_csv ();
    void save_tsacc ();
    void set_input_dir (const char* input_dir);
    void set_task (const char* task);
    void train ();

  private:
    void load_input_dir_recursive (std::string input_dir);
    void load_input_file (const char* nrrd_fn, const char* fcsv_fn);
};

class PLMSEGMENT_API Autolabel_train_parms {
public:
    std::string input_dir;
    std::string output_dir;
    std::string task;
};

PLMSEGMENT_API void autolabel_train (Autolabel_train_parms *parms);

#endif
