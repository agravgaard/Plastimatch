/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdlib.h>
#include <string.h>
#include "itkImage.h"
#include "itkArray.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkCommand.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkImageRegistrationMethod.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkQuaternionRigidTransformGradientDescentOptimizer.h"
#include "itkAmoebaOptimizer.h"
#include "itkOnePlusOneEvolutionaryOptimizer.h"
#include "itkLBFGSOptimizer.h"
#include "itkLBFGSBOptimizer.h"
#include "itkNormalVariateGenerator.h"
#include "itkFRPROptimizer.h"


#include "itk_optimizer.h"
#include "itk_registration.h"
#include "itk_registration_private.h"
#include "logfile.h"
#include "print_and_exit.h"
#include "stage_parms.h"

/* Types of optimizers */
typedef itk::OnePlusOneEvolutionaryOptimizer      OnePlusOneOptimizerType;
typedef itk::FRPROptimizer                        FRPROptimizerType;
typedef itk::AmoebaOptimizer                      AmoebaOptimizerType;
typedef itk::RegularStepGradientDescentOptimizer  RSGOptimizerType;
typedef itk::VersorRigid3DTransformOptimizer      VersorOptimizerType;
typedef itk::QuaternionRigidTransformGradientDescentOptimizer QuatOptimizerType;
typedef itk::LBFGSOptimizer LBFGSOptimizerType;
typedef itk::LBFGSBOptimizer LBFGSBOptimizerType;

typedef itk::Statistics::NormalVariateGenerator  OptimizerNormalGeneratorType;

void
Itk_registration_private::optimizer_set_max_iterations (int its)
{
    if (stage->optim_type == OPTIMIZATION_AMOEBA) {
	typedef AmoebaOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
        optimizer->SetMaximumNumberOfIterations(its);
    }
    else if (stage->optim_type == OPTIMIZATION_ONEPLUSONE) {
        typedef OnePlusOneOptimizerType * OptimizerPointer;
        OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
                           registration->GetOptimizer());
        optimizer->SetMaximumIteration(its);
    }
    else if (stage->optim_type == OPTIMIZATION_FRPR) {
        typedef FRPROptimizerType * OptimizerPointer;
        OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
                           registration->GetOptimizer());
        optimizer->SetMaximumIteration(its);
    }
    else if (stage->optim_type == OPTIMIZATION_RSG) {
	typedef RSGOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	optimizer->SetNumberOfIterations(its);
    }
    else if (stage->optim_type == OPTIMIZATION_VERSOR) {
	typedef VersorOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	optimizer->SetNumberOfIterations(its);
    }
    else if (stage->optim_type == OPTIMIZATION_QUAT) {
	typedef QuatOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
	    registration->GetOptimizer());
	optimizer->SetNumberOfIterations(its);
    }
    else if (stage->optim_type == OPTIMIZATION_LBFGS) {
	typedef LBFGSOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	optimizer->SetMaximumNumberOfFunctionEvaluations (its);
    }
    else if (stage->optim_type == OPTIMIZATION_LBFGSB) {
	typedef LBFGSBOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	optimizer->SetMaximumNumberOfIterations (its);
	optimizer->SetMaximumNumberOfEvaluations (its);
    } else {
        print_and_exit ("Error: Unknown optimizer value.\n");
    }
}

double
Itk_registration_private::optimizer_get_value ()
{
    if (stage->optim_type == OPTIMIZATION_AMOEBA) {
	typedef AmoebaOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetCachedValue();
    }
    else if (stage->optim_type == OPTIMIZATION_ONEPLUSONE) {
        typedef OnePlusOneOptimizerType * OptimizerPointer;
        OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
                           registration->GetOptimizer());
        return optimizer->GetValue();
    }
    else if (stage->optim_type == OPTIMIZATION_FRPR) {
        typedef FRPROptimizerType * OptimizerPointer;
        OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
                           registration->GetOptimizer());
        return optimizer->GetValue();
    }
    else if (stage->optim_type == OPTIMIZATION_RSG) {
	typedef RSGOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetValue();
    }
    else if (stage->optim_type == OPTIMIZATION_VERSOR) {
	typedef VersorOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetValue();
    }
    else if (stage->optim_type == OPTIMIZATION_QUAT) {
	typedef QuatOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetValue();
    }
    else if (stage->optim_type == OPTIMIZATION_LBFGS) {
	typedef LBFGSOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetCachedValue();
    }
    else if (stage->optim_type == OPTIMIZATION_LBFGSB) {
	typedef LBFGSBOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetCachedValue();
    } else {
        print_and_exit ("Error: Unknown optimizer value.\n");
    }
    return 0.0;        /* Suppress compiler warning */
}

double
Itk_registration_private::optimizer_get_step_length ()
{
    if (stage->optim_type == OPTIMIZATION_AMOEBA) {
#if defined (commentout)
	typedef AmoebaOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
#endif
	return -1.0;
    }
    else if (stage->optim_type == OPTIMIZATION_ONEPLUSONE) {
        return -1.0;
    }
    else if (stage->optim_type == OPTIMIZATION_FRPR) {
        typedef FRPROptimizerType * OptimizerPointer;
        OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
                           registration->GetOptimizer());
        return optimizer->GetStepLength();
    }
    else if (stage->optim_type == OPTIMIZATION_RSG) {
	typedef RSGOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetCurrentStepLength();
    }
    else if (stage->optim_type == OPTIMIZATION_VERSOR) {
	typedef VersorOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetCurrentStepLength();
    }
    else if (stage->optim_type == OPTIMIZATION_QUAT) {
#if defined (commentout)
	typedef QuatOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
#endif
	return -1.0;
    }
    else if (stage->optim_type == OPTIMIZATION_LBFGS) {
#if defined (commentout)
	typedef LBFGSOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
#endif
	return -1.0;
    }
    else if (stage->optim_type == OPTIMIZATION_LBFGSB) {
	typedef LBFGSBOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetInfinityNormOfProjectedGradient();
    } else {
        print_and_exit ("Error: Unknown optimizer value.\n");
    }
    return 0.0;        /* Suppress compiler warning */
}

int
Itk_registration_private::optimizer_get_current_iteration ()
{
    if (stage->optim_type == OPTIMIZATION_AMOEBA) {
#if defined (commentout)
	typedef AmoebaOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
#endif
	return -1;
    }
    if (stage->optim_type == OPTIMIZATION_ONEPLUSONE) {
        typedef OnePlusOneOptimizerType * OptimizerPointer;
        OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
                           registration->GetOptimizer());
        return optimizer->GetCurrentIteration();
    }
    else if (stage->optim_type == OPTIMIZATION_FRPR) {
        typedef FRPROptimizerType * OptimizerPointer;
        OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
                           registration->GetOptimizer());
        return optimizer->GetCurrentIteration();
    }
    else if (stage->optim_type == OPTIMIZATION_RSG) {
	typedef RSGOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetCurrentIteration();
    }
    else if (stage->optim_type == OPTIMIZATION_VERSOR) {
	typedef VersorOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetCurrentIteration();
    }
    else if (stage->optim_type == OPTIMIZATION_QUAT) {
	typedef QuatOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetCurrentIteration();
    }
    else if (stage->optim_type == OPTIMIZATION_LBFGS) {
#if defined (commentout)
	typedef LBFGSOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
#endif
	return -1;
    }
    else if (stage->optim_type == OPTIMIZATION_LBFGSB) {
	typedef LBFGSBOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
			   registration->GetOptimizer());
	return optimizer->GetCurrentIteration();
    } else {
        print_and_exit ("Error: Unknown optimizer value.\n");
    }
    return 0;        /* Suppress compiler warning */
}

const itk::Array<double>&
Itk_registration_private::optimizer_get_current_position ()
{
    if (stage->optim_type == OPTIMIZATION_AMOEBA) {
	typedef AmoebaOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
            registration->GetOptimizer());
	return optimizer->GetCachedCurrentPosition();
    }
    else if (stage->optim_type == OPTIMIZATION_ONEPLUSONE) {
        typedef OnePlusOneOptimizerType * OptimizerPointer;
        OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
            registration->GetOptimizer());
        return optimizer->GetCurrentPosition();
    }
    else if (stage->optim_type == OPTIMIZATION_FRPR) {
        typedef FRPROptimizerType * OptimizerPointer;
        OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
            registration->GetOptimizer());
        return optimizer->GetCurrentPosition();
    }
    else if (stage->optim_type == OPTIMIZATION_RSG) {
        return registration->GetTransform()->GetParameters();
    }
    else if (stage->optim_type == OPTIMIZATION_VERSOR) {
	typedef VersorOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
            registration->GetOptimizer());
	return optimizer->GetCurrentPosition();
    }
    else if (stage->optim_type == OPTIMIZATION_QUAT) {
	typedef QuatOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
            registration->GetOptimizer());
	return optimizer->GetCurrentPosition();
    }
    else if (stage->optim_type == OPTIMIZATION_LBFGS) {
	typedef LBFGSOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
            registration->GetOptimizer());
	return optimizer->GetCurrentPosition();
    }
    else if (stage->optim_type == OPTIMIZATION_LBFGSB) {
	typedef LBFGSBOptimizerType * OptimizerPointer;
	OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >(
            registration->GetOptimizer());
	return optimizer->GetCurrentPosition();
    } else {
        print_and_exit ("Error: Unknown optimizer value.\n");
    }
    exit (1);    /* Suppress compiler warning */
}

void
Itk_registration_private::optimizer_stop ()
{
    /* calling StopOptimization() doesn't always stop 
       optimization */
    if (stage->optim_type == OPTIMIZATION_RSG) {
        /* calling optimizer_set_max_iterations () doesn't 
           seem to always stop rsg. */
        typedef itk::RegularStepGradientDescentOptimizer *OptimizerPointer;
        OptimizerPointer optimizer = dynamic_cast< 
            OptimizerPointer >(registration->GetOptimizer());
        optimizer->StopOptimization();
    } else {
        optimizer_set_max_iterations (1);
    }
}

void
set_optimization_amoeba (RegistrationType::Pointer registration, 
    Stage_parms* stage)
{
    AmoebaOptimizerType::Pointer optimizer = AmoebaOptimizerType::New();
    optimizer->SetParametersConvergenceTolerance(stage->amoeba_parameter_tol);
    optimizer->SetFunctionConvergenceTolerance(stage->convergence_tol);  // Was 10000
    optimizer->SetMaximumNumberOfIterations(stage->max_its);
    registration->SetOptimizer(optimizer);
}

void
set_optimization_rsg (RegistrationType::Pointer registration, 
		      Stage_parms* stage)
{
    RSGOptimizerType::Pointer optimizer = RSGOptimizerType::New();
    optimizer->SetMaximumStepLength(stage->max_step);
    optimizer->SetMinimumStepLength(stage->min_step);
    optimizer->SetNumberOfIterations(stage->max_its);
    optimizer->SetGradientMagnitudeTolerance (stage->rsg_grad_tol);
    registration->SetOptimizer(optimizer);
}


void
set_optimization_oneplusone (
    RegistrationType::Pointer registration,
    Stage_parms* stage)
{
  OnePlusOneOptimizerType::Pointer optimizer = OnePlusOneOptimizerType::New();
  optimizer->SetNormalVariateGenerator(OptimizerNormalGeneratorType::New() );
  optimizer->SetMaximumIteration(stage->max_its);
  optimizer->SetEpsilon(stage->opo_epsilon);
  optimizer->Initialize(stage->opo_initial_search_rad); // Initial search radius
  registration->SetOptimizer(optimizer);
}


void
set_optimization_frpr(RegistrationType::Pointer registration,
                           Stage_parms* stage)
{
  FRPROptimizerType::Pointer optimizer = FRPROptimizerType::New();
  optimizer->SetMaximize(false);
  optimizer->SetStepLength(5);
  optimizer->SetStepTolerance(stage->frpr_step_tol);
  optimizer->SetMaximumIteration(stage->max_its);
  optimizer->SetMaximumLineIteration(stage->frpr_max_line_its);
  registration->SetOptimizer(optimizer);
}

void
set_optimization_versor (RegistrationType::Pointer registration, 
			 Stage_parms* stage)
{
    VersorOptimizerType::Pointer optimizer = VersorOptimizerType::New();
    optimizer->SetMaximumStepLength(stage->max_step);
    optimizer->SetMinimumStepLength(stage->min_step);
    optimizer->SetNumberOfIterations(stage->max_its);
    optimizer->SetGradientMagnitudeTolerance (stage->rsg_grad_tol);

    registration->SetOptimizer(optimizer);
}

void
set_optimization_quat (RegistrationType::Pointer registration, 
    Stage_parms* stage)
{
    QuatOptimizerType::Pointer optimizer = QuatOptimizerType::New();
    optimizer->SetLearningRate(stage->learn_rate);
    lprintf ("Learning Rate was set to : %f\n", 
        optimizer->GetLearningRate());
    optimizer->SetNumberOfIterations(stage->max_its);
    registration->SetOptimizer(optimizer);
}

void
set_optimization_lbfgs (RegistrationType::Pointer registration, 
			 Stage_parms* stage)
{
    LBFGSOptimizerType::Pointer optimizer = LBFGSOptimizerType::New();
    
    optimizer->SetGradientConvergenceTolerance (stage->grad_tol);
    optimizer->SetLineSearchAccuracy (0.9);
    optimizer->SetDefaultStepLength (5.0);
#if defined (commentout)
    optimizer->SetMaximumNumberOfFunctionEvaluations (100);
    optimizer->SetMaximumNumberOfFunctionEvaluations (50);
    optimizer->SetMaximumNumberOfFunctionEvaluations (10);
#endif
    optimizer->SetMaximumNumberOfFunctionEvaluations (50);

    optimizer->TraceOn();
    registration->SetOptimizer(optimizer);
}

void
set_optimization_lbfgsb (RegistrationType::Pointer registration, 
			 Stage_parms* stage)
{
    LBFGSBOptimizerType::Pointer optimizer = LBFGSBOptimizerType::New();

    LBFGSBOptimizerType::BoundSelectionType boundSelect (
        registration->GetTransform()->GetNumberOfParameters());
    LBFGSBOptimizerType::BoundValueType upperBound (
        registration->GetTransform()->GetNumberOfParameters());
    LBFGSBOptimizerType::BoundValueType lowerBound (
        registration->GetTransform()->GetNumberOfParameters());

    boundSelect.Fill(0);
    upperBound.Fill(0.0);
    lowerBound.Fill(0.0);

    optimizer->SetBoundSelection(boundSelect);
    optimizer->SetUpperBound(upperBound);
    optimizer->SetLowerBound(lowerBound);

    /* GCS FIX: I think this is right for # of evaluations.  Not at all sure 
       about # of corrections or cost fn convergence factor. */
    optimizer->SetCostFunctionConvergenceFactor (1e+7);
    optimizer->SetProjectedGradientTolerance (stage->pgtol);
    optimizer->SetMaximumNumberOfIterations (stage->max_its);
    optimizer->SetMaximumNumberOfEvaluations (2 * stage->max_its);
    optimizer->SetMaximumNumberOfCorrections (5);

    registration->SetOptimizer(optimizer);
}

void
set_optimization_scales_translation (RegistrationType::Pointer registration, 
				     Stage_parms* stage)
{
    itk::Array<double> optimizerScales(3);

    const double translationScale = 1.0 / (double) stage->translation_scale_factor;
    optimizerScales[0] = translationScale;
    optimizerScales[1] = translationScale;
    optimizerScales[2] = translationScale;
    registration->GetOptimizer()->SetScales(optimizerScales);
}

void
set_optimization_scales_versor (
    RegistrationType::Pointer registration,
    Stage_parms* stage)
{
    double rotation_scale, translation_scale;
    itk::Array<double> optimizerScales(6);

    if (stage->optim_type == OPTIMIZATION_AMOEBA) {
	rotation_scale = 1.0;
	translation_scale = 1.0;
    } else {
        rotation_scale = 1.0 / (double) stage->rotation_scale_factor;
        translation_scale = 1.0 / (double) stage->translation_scale_factor;
    }

    optimizerScales[0] = rotation_scale;
    optimizerScales[1] = rotation_scale;
    optimizerScales[2] = rotation_scale;
    optimizerScales[3] = translation_scale;
    optimizerScales[4] = translation_scale;
    optimizerScales[5] = translation_scale;

    registration->GetOptimizer()->SetScales(optimizerScales);
}

void
set_optimization_scales_quaternion (
    RegistrationType::Pointer registration, 
    Stage_parms* stage)
{
    double rotation_scale, translation_scale;
    itk::Array<double> optimizerScales(7);

    rotation_scale = 1.0 / (double) stage->rotation_scale_factor;
    translation_scale = 1.0 / (double) stage->translation_scale_factor;

    /* GCS FIX: Changing the scale fudge_factor is one way to avoid 
       ITK "Too many samples..." problem */
    //double fudge_factor = 1000000.0;
    double fudge_factor = 1.0;
    rotation_scale = rotation_scale * fudge_factor;
    translation_scale = translation_scale * fudge_factor;

    optimizerScales[0] = rotation_scale;
    optimizerScales[1] = rotation_scale;
    optimizerScales[2] = rotation_scale;
    optimizerScales[3] = rotation_scale;
    optimizerScales[4] = translation_scale;
    optimizerScales[5] = translation_scale;
    optimizerScales[6] = translation_scale;

    registration->GetOptimizer()->SetScales(optimizerScales);
}

void
set_optimization_scales_affine (RegistrationType::Pointer registration, 
				Stage_parms* stage)
{
    itk::Array<double> optimizerScales(12);

    const double matrix_scale = 1.0;
    //const double translationScale = 1.0 / 10000.0;
    const double translation_scale = 1.0 / (double) stage->translation_scale_factor;
    //const double translation_scale = 1.0 / 1000000.0;
    optimizerScales[0] = matrix_scale;
    optimizerScales[1] = matrix_scale;
    optimizerScales[2] = matrix_scale;
    optimizerScales[3] = matrix_scale;
    optimizerScales[4] = matrix_scale;
    optimizerScales[5] = matrix_scale;
    optimizerScales[6] = matrix_scale;
    optimizerScales[7] = matrix_scale;
    optimizerScales[8] = matrix_scale;
    optimizerScales[9] = translation_scale;
    optimizerScales[10] = translation_scale;
    optimizerScales[11] = translation_scale;

    registration->GetOptimizer()->SetScales(optimizerScales);
}

void
set_optimization_scales_similarity (RegistrationType::Pointer registration,
                Stage_parms* stage)
{
    itk::Array<double> optimizerScales(7);

    const double rotation_scale = 1.0 / (double) stage->rotation_scale_factor;
    const double translation_scale = 1.0/ (double) stage->translation_scale_factor;
    const double scaling_scale = 1.0/ (double) stage->scaling_scale_factor;

    optimizerScales[0] =  rotation_scale;
    optimizerScales[1] =  rotation_scale;
    optimizerScales[2] =  rotation_scale;
    optimizerScales[3] =  translation_scale;
    optimizerScales[4] =  translation_scale;
    optimizerScales[5] =  translation_scale;
    optimizerScales[6] =  scaling_scale;

    registration->GetOptimizer()->SetScales(optimizerScales);
}

void
Itk_registration_private::set_optimization ()
{
    if (stage->xform_type == STAGE_TRANSFORM_QUATERNION)
    {
	stage->optim_type = OPTIMIZATION_QUAT;
    }
    else if (stage->optim_type == OPTIMIZATION_VERSOR
	&& (stage->xform_type == STAGE_TRANSFORM_TRANSLATION
        || stage->xform_type == STAGE_TRANSFORM_AFFINE
        || stage->xform_type == STAGE_TRANSFORM_SIMILARITY))
    {
	stage->optim_type = OPTIMIZATION_RSG;
    }
    else if (stage->xform_type == STAGE_TRANSFORM_BSPLINE 
	&& (stage->optim_type != OPTIMIZATION_LBFGS
	    && stage->optim_type != OPTIMIZATION_LBFGSB))
    {
	stage->optim_type = OPTIMIZATION_LBFGSB;
    }

    switch (stage->optim_type) {
    case OPTIMIZATION_AMOEBA:
	set_optimization_amoeba(registration,stage);
	break;
    case OPTIMIZATION_ONEPLUSONE:
    set_optimization_oneplusone(registration,stage);
    break;
    case OPTIMIZATION_FRPR:
    set_optimization_frpr(registration,stage);
    break;
    case OPTIMIZATION_RSG:
	set_optimization_rsg(registration,stage);
	break;
    case OPTIMIZATION_VERSOR:
	set_optimization_versor(registration,stage);
	break;
    case OPTIMIZATION_QUAT:
	set_optimization_quat(registration,stage);
	break;
    case OPTIMIZATION_LBFGS:
	set_optimization_lbfgs(registration,stage);
	break;
    case OPTIMIZATION_LBFGSB:
	set_optimization_lbfgsb(registration,stage);
	break;
    default:
        print_and_exit ("Unknown optimizer used in ITK optimization\n");
        break;
    }
    switch (stage->xform_type) {
    case STAGE_TRANSFORM_TRANSLATION:
	set_optimization_scales_translation (registration, stage);
	break;
    case STAGE_TRANSFORM_VERSOR:
	set_optimization_scales_versor (registration, stage);
	break;
    case STAGE_TRANSFORM_QUATERNION:
	set_optimization_scales_quaternion (registration, stage);
	break;
    case STAGE_TRANSFORM_AFFINE:
	set_optimization_scales_affine (registration, stage);
	break;
    case STAGE_TRANSFORM_SIMILARITY:
    set_optimization_scales_similarity (registration, stage);
    break;
    case STAGE_TRANSFORM_BSPLINE:
	/* LBFGS/LBFGSB only. No optimizer scales. */
	break;
    default:
        print_and_exit ("Unknown xform type used in ITK optimization\n");
        break;
    }
}
