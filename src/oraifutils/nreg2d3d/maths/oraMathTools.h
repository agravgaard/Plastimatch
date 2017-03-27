#ifndef ORAMATHTOOLS_H_
#define ORAMATHTOOLS_H_

namespace ora
{

// This module contains some general math tool-functions
// not provided by ANSI-C++ or ITK.
//
// @version 1.0
// @author Markus 

/** Compute log(1+x) without losing precision for small values of x.
 * If p is very small, directly computing log(1+p) can be inaccurate.
 * If p is small enough, 1 + p = 1 in machine arithmetic and so log(1+p)
 * returns log(1) which equals zero.
 * We can avoid the loss of precision by using a Taylor series to evaluate
 * log(1 + p). For small p, log(1+p) ≈ p - p2/2 with an error roughly equal
 * to p3/3. So if |p| is less than 10^-4, the error in approximating
 * log(1+p) by p - p2/2 is less than 10^-12.
 * @param x Value to compute log(1+x).
 * @return Value of log(1+x) if x is greater than 10^-4, else p - p^2/2.
 */
double LogOnePlusX(double x);

}

#endif /* ORAMATHTOOLS_H_ */
