/* Fix for missing erf() by Tesuji
   http://www.daniweb.com/software-development/cpp/threads/294232/error-function-in-math-library
*/
double erf(double x)
    {
    /* erf(z) = 2/sqrt(pi) * Integral(0..x) exp( -t^2) dt
    erf(0.01) = 0.0112834772 erf(3.7) = 0.9999998325
    Abramowitz/Stegun: p299, |erf(z)-erf| <= 1.5*10^(-7)
    */
    double y = 1.0 / ( 1.0 + 0.3275911 * x);
    return 1 - (((((
    + 1.061405429 * y
    - 1.453152027) * y
    + 1.421413741) * y
    - 0.284496736) * y
    + 0.254829592) * y)
    * exp (-x * x);
    }

double erfc(double x) 
{
    return 1 - erf(x);
}
