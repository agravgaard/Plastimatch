#include "f2c.h"

#ifdef KR_headers
extern double sin(), cos(), sinh(), cosh();

VOID c_cos(r, z) complex *r, *z;
#else
#undef abs
/* GCS: MSVC9 math.h header redefines complex unless __STDC__ is defined */
#if _WIN32
#define __STDC__ 1
#endif
#include "math.h"
#ifdef __cplusplus
extern "C" {
#endif

void c_cos(complex *r, complex *z)
#endif
{
	double zi = z->i, zr = z->r;
	r->r =   cos(zr) * cosh(zi);
	r->i = - sin(zr) * sinh(zi);
	}
#ifdef __cplusplus
}
#endif
