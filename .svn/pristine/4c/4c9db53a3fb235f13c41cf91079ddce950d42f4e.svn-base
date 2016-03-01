/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <math.h>
#include "vnl/algo/vnl_amoeba.h"
#include "vnl/vnl_cost_function.h"

/* Rosenbrock function */
double 
myfunc (const double *x)
{
    double value = (1 - x[0])*(1 - x[0]) 
        + 100 * (x[1] - x[0]*x[0]) * (x[1] - x[0]*x[0]);
    printf ("%g %g -> %g\n", x[0], x[1], value);
    return value;
}

/* vxl needs you to wrap the function within a class */
class Rosenbrock_function : public vnl_cost_function
{
public:
    virtual double f (vnl_vector<double> const& vnl_x) {
        /* vxl requires you using their own vnl_vector type, 
           therefore we copy into a standard C/C++ array. */
        double x[2];
        x[0] = vnl_x[0];
        x[1] = vnl_x[1];
        return myfunc(x);
    }
};

int
main (int argc, char* argv[])
{
    /* Create function object (for function to be minimized) */
    Rosenbrock_function rb;

    /* Create optimizer object */
    vnl_amoeba nm (rb);

    /* Set some optimizer parameters */
    nm.set_x_tolerance (0.00001);
    nm.set_f_tolerance (0.00001);
    nm.set_max_iterations (100);

    /* Set the starting point */
    vnl_vector<double> x(2);
    x[0] = 0;
    x[1] = 0;

    /* Run the optimizer */
    nm.minimize (x);

    return 0;
}
