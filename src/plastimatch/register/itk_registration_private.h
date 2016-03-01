/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_registration_private_h_
#define _itk_registration_private_h_

#include "plmregister_config.h"
#include "itkExceptionObject.h"
#include "itkImageRegistrationMethod.h"
#include "itk_image_type.h"
#include "stage_parms.h"

class Registration_data;
class Stage_parms;
class Xform;
typedef itk::ImageRegistrationMethod < 
    FloatImageType, FloatImageType > RegistrationType;

class Itk_registration_private {
public:
    Itk_registration_private (
        Registration_data* regd, 
        Xform *xf_out, 
        const Xform *xf_in, 
        Stage_parms* stage
    );
    ~Itk_registration_private ();

public:
    Registration_data *regd;
    Xform *xf_out;
    const Xform *xf_in;
    Stage_parms *stage;

    RegistrationType::Pointer registration;
    double best_value;
    Xform *xf_best;

public:
    double evaluate_initial_transform ();

    const itk::Array<double>& optimizer_get_current_position ();
    int optimizer_get_current_iteration ();
    double optimizer_get_value ();
    double optimizer_get_step_length ();
    void optimizer_stop ();
    void optimizer_set_max_iterations (int its);

    unsigned int compute_num_samples (FloatImageType::Pointer& fixed_ss);
    void set_best_xform ();
    void set_fixed_image_region ();
    void set_roi_images ();
    void set_metric (FloatImageType::Pointer& fixed_ss);
    void set_observer ();
    void set_optimization ();
    void set_transform ();
    void set_xf_out ();
    void show_stats ();
};

#endif
