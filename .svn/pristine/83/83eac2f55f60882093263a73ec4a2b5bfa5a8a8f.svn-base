/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <math.h>
#include "nlopt.h"

typedef struct {
    double a, b;
} my_constraint_data;

double myfunc (int n, const double *x, double *grad, void *my_func_data)
{
    if (grad) {
        grad[0] = 0.0;
        grad[1] = 0.5 / sqrt(x[1]);
    }
    return sqrt(x[1]);
}

double myconstraint(int n, const double *x, double *grad, void *data)
{
    my_constraint_data *d = (my_constraint_data *) data;
    double a = d->a, b = d->b;
    if (grad) {
        grad[0] = 3 * a * (a*x[0] + b) * (a*x[0] + b);
        grad[1] = -1.0;
    }
    return ((a*x[0] + b) * (a*x[0] + b) * (a*x[0] + b) - x[1]);
}

int
main (int argc, char* argv[])
{
    my_constraint_data data[2] = { {2,0}, {-1,1} };
    double x[2] = { 1.234, 5.678 };  /* some initial guess */
    double lb[2] = { -HUGE_VAL, 0 }, ub[2] = { HUGE_VAL, HUGE_VAL }; /* lower and upper bounds */
    double minf; /* the minimum objective value, upon return */

    if (nlopt_minimize_constrained(NLOPT_LD_MMA, 2, myfunc, NULL,
	    2, myconstraint, data, sizeof(my_constraint_data),
	    lb, ub, x, &minf,
	    -HUGE_VAL, 0.0, 0.0, 1e-4, NULL, 0, 0.0) < 0) {
	printf("nlopt failed!\n");
    }
    else {
	printf("found minimum at f(%g,%g) = %g\n", x[0], x[1], minf);
    }
    return 0;
}
