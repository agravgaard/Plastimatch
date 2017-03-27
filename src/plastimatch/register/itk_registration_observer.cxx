/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "itkRegularStepGradientDescentOptimizer.h"

#include "compiler_warnings.h"
#include "itk_image_type.h"
#include "itk_optimizer.h"
#include "itk_registration.h"
#include "itk_registration_private.h"
#include "logfile.h"
#include "metric_parms.h"
#include "plm_math.h"
#include "plm_timer.h"
#include "shared_parms.h"
#include "stage_parms.h"

/* Lots of ITK algorithms don't behave uniformly.
   We're going to keep all this state in the observer, and the 
   registration should query the observer to find the best registration. 

   RSG
   - Invokes itk::IterationEvent 
   - Current position at time of itk::IterationEvent is next position 
     So we need to check registration->GetTransform() instead
   - Final transform is not optimal

   Amoeba
   - Invokes itk::FunctionValueIterationEvent
   - Current position is not valid at StartEvent
   - Initial transform is evaluated
   - Final transform is optimal

   LBFGS
   - Invokes itk::FunctionAndGradientEvaluationIterationEvent
   - Current position is not valid at StartEvent

   LBFGSB
   - StartEvent is not invoked
*/
class Optimization_observer : public itk::Command
{
public:
    typedef Optimization_observer Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer < Self > Pointer;
    itkNewMacro(Self);

public:
    Itk_registration_private *irp;
    double m_prev_value;
    int m_feval;
    Plm_timer* timer;

protected:
    Optimization_observer() {
        m_prev_value = -DBL_MAX;
        m_feval = 0;
        timer = new Plm_timer;
        timer->start ();
    };
    ~Optimization_observer() {
        delete timer;
    }

public:
    void set_irp (Itk_registration_private *irp)
    {
        this->irp = irp;
    }

    void Execute (itk::Object * caller, const itk::EventObject & event)
    {
        Execute((const itk::Object *) caller, event);
    }

    void
    Execute (const itk::Object * object, const itk::EventObject & event)
    {
        /* GCS FIX, split metric vector into separate items in 
           Stage_similarity_data list */
        Metric_parms metric_parms;
        const Shared_parms *shared = irp->stage->get_shared_parms();
        std::map<std::string,Metric_parms>::const_iterator metric_it;
        for (metric_it = shared->metric.begin();
             metric_it != shared->metric.end(); ++metric_it) {
            metric_parms = metric_it->second;
            break;
        }
        
        if (typeid(event) == typeid(itk::StartEvent)) {
            m_feval = 0;
            m_prev_value = -DBL_MAX;
            lprintf ("StartEvent: ");
            if (irp->stage->xform_type != STAGE_TRANSFORM_BSPLINE) {
                std::stringstream ss;
                ss << irp->optimizer_get_current_position ();
                lprintf ("%s", ss.str().c_str());
            }
            lprintf ("\n");
            timer->start ();
        }
        else if (typeid(event) == typeid(itk::InitializeEvent)) {
            lprintf ("InitializeEvent: \n");
            timer->start ();
        }
        else if (typeid(event) == typeid(itk::EndEvent)) {
            lprintf ("EndEvent: ");
            if (irp->stage->xform_type != STAGE_TRANSFORM_BSPLINE) {
                std::stringstream ss;
                ss << irp->optimizer_get_current_position ();
                lprintf ("%s", ss.str().c_str());
            }
            lprintf ("\n");
            lprintf ("%s\n", irp->registration->GetOptimizer()
                ->GetStopConditionDescription().c_str());
        }
        else if (typeid(event) 
            == typeid(itk::FunctionAndGradientEvaluationIterationEvent))
        {
            int it = irp->optimizer_get_current_iteration();
            double val = irp->optimizer_get_value();
            double duration = timer->report ();

            lprintf ("%s [%2d,%3d] %9.3f [%6.3f secs]\n", 
                (metric_parms.metric_type == SIMILARITY_METRIC_MSE)
                ? "MSE" : "MI",
                it, m_feval, val, duration);
            timer->start ();
            m_feval++;
        }
        else if (typeid(event) == typeid(itk::IterationEvent)
            || typeid(event) == typeid(itk::FunctionEvaluationIterationEvent))
        {
            int it = irp->optimizer_get_current_iteration();
            double val = irp->optimizer_get_value();
            double ss = irp->optimizer_get_step_length();

            /* ITK amoeba generates spurious events */
            if (irp->stage->optim_type == OPTIMIZATION_AMOEBA) {
                if (m_feval % 2 == 1) {
                    m_feval ++;
                    return;
                }
            }

            /* Print out score & optimizer stats */
            if (irp->stage->optim_type == OPTIMIZATION_AMOEBA) {
                lprintf ("%s [%3d] %9.3f ",
                    (metric_parms.metric_type == SIMILARITY_METRIC_MSE) 
                    ? "MSE" : "MI",
                    m_feval / 2, val);
            } else {
                lprintf ("%s [%2d,%3d,%5.2f] %9.3f ",
                    (metric_parms.metric_type == SIMILARITY_METRIC_MSE) 
                    ? "MSE" : "MI",
                    it, m_feval, ss, val);
            }

            if (irp->stage->xform_type != STAGE_TRANSFORM_BSPLINE) {
                std::stringstream ss;
                ss << std::setprecision(3);
                ss << irp->optimizer_get_current_position ();
                lprintf ("%s", ss.str().c_str());
            }

            if (m_prev_value != -DBL_MAX) {
                double diff = fabs(m_prev_value - val);
                lprintf (" %10.2f", val - m_prev_value);
                if (it >= irp->stage->min_its 
                    && diff < irp->stage->convergence_tol)
                {
                    lprintf (" (tol)");
                    irp->optimizer_stop ();
                }
            }
            m_prev_value = val;

            if (val < irp->best_value) {
                irp->best_value = val;
                irp->set_best_xform ();
                lprintf (" *");
            }

            lprintf ("\n");
            m_feval ++;
        }
        else if (typeid(event) == typeid(itk::ProgressEvent)) {
            lprintf ("ProgressEvent: ");
            if (irp->stage->xform_type != STAGE_TRANSFORM_BSPLINE) {
                std::stringstream ss;
                ss << irp->optimizer_get_current_position ();
                lprintf ("%s", ss.str().c_str());
            }
            lprintf ("\n");
        }
        else {
            lprintf ("Unknown event type: %s\n", event.GetEventName());
        }
    }
};

void
Itk_registration_private::set_observer ()
{
    typedef Optimization_observer OOType;
    OOType::Pointer observer = OOType::New();
    observer->set_irp (this);
    registration->GetOptimizer()->AddObserver(itk::StartEvent(), observer);
    registration->GetOptimizer()->AddObserver(itk::InitializeEvent(), observer);
    registration->GetOptimizer()->AddObserver(itk::IterationEvent(), observer);
    registration->GetOptimizer()->AddObserver(itk::FunctionEvaluationIterationEvent(), observer);
    registration->GetOptimizer()->AddObserver(itk::ProgressEvent(), observer);
    registration->GetOptimizer()->AddObserver(itk::EndEvent(), observer);
}
