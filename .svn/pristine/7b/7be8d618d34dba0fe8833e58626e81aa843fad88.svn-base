/* mpbdv.f -- translated by f2c (version 20090411).
   You must link the resulting object file with libf2c:
	on Microsoft Windows system, link with libf2c.lib;
	on Linux or Unix systems, link with .../path/to/libf2c.a -lm
	or, if you install libf2c.a in a standard place, with -lf2c -lm
	-- in that order, at the end of the command line, as in
		cc *.o -lf2c -lm
	Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

		http://www.netlib.org/f2c/libf2c.zip
*/

/* GCS: Fortran built-ins */
#include <math.h>
typedef double doublereal;
typedef long integer;
double d_sign(doublereal *a, doublereal *b)
{
    double x;
    x = (*a >= 0 ? *a : - *a);
    return( *b >= 0 ? x : -x);
}

double pow_dd(doublereal *ap, doublereal *bp)
{
    return(pow(*ap, *bp) );
}
#undef abs
#define abs(x) ((x) >= 0 ? (x) : -(x))

/* Table of constant values */

static doublereal c_b2 = 1.;
static doublereal c_b11 = 2.;

/* $$$        PROGRAM MPBDV */
/* $$$C */
/* $$$C       ========================================================= */
/* $$$C       Purpose: This program computes the parabolic cylinder */
/* $$$C                functions Dv(x) and their derivatives using */
/* $$$C                subroutine PBDV */
/* $$$C       Input:   x --- Argument of Dv(x) */
/* $$$C                v --- Order of Dv(x) */
/* $$$C       Output:  DV(na) --- Dn+v0(x) */
/* $$$C                DP(na) --- Dn+v0'(x) */
/* $$$C                ( na = |n|, n = int(v), v0 = v-n, |v0| < 1 */
/* $$$C                  n = 0,ס1,ס2,תתת, |n| ף 100 ) */
/* $$$C                PDF --- Dv(x) */
/* $$$C                PDD --- Dv'(x) */
/* $$$C       Example: v = 5.5,  x =10.0,  v0 = 0.5,  n = 0,1,...,5 */
/* $$$C */
/* $$$C                  n+v0      Dv(x)           Dv'(x) */
/* $$$C                --------------------------------------- */
/* $$$C                  0.5   .43971930D-10  -.21767183D-09 */
/* $$$C                  1.5   .43753148D-09  -.21216995D-08 */
/* $$$C                  2.5   .43093569D-08  -.20452956D-07 */
/* $$$C                  3.5   .41999741D-07  -.19491595D-06 */
/* $$$C                  4.5   .40491466D-06  -.18355745D-05 */
/* $$$C                  5.5   .38601477D-05  -.17073708D-04 */
/* $$$C */
/* $$$C                Dv(x)= .38601477D-05,  Dv'(x)=-.17073708D-04 */
/* $$$C       ========================================================= */
/* $$$C */
/* $$$        IMPLICIT DOUBLE PRECISION (A-H,O-Z) */
/* $$$        DIMENSION DV(0:100),DP(0:100) */
/* $$$        WRITE(*,*)'Please enter v and  x ' */
/* $$$        READ(*,*)V,X */
/* $$$        WRITE(*,20)V,X */
/* $$$        NV=INT(V) */
/* $$$        V0=V-NV */
/* $$$        NA=ABS(NV) */
/* $$$        CALL PBDV(V,X,DV,DP,PDF,PDD) */
/* $$$        WRITE(*,*) */
/* $$$        WRITE(*,*)'   v       Dv(x)           Dv''(x)' */
/* $$$        WRITE(*,*)'---------------------------------------' */
/* $$$        DO 10 K=0,NA */
/* $$$           VK=K*ISIGN(1,NV)+V0 */
/* $$$10         WRITE(*,30)VK,DV(K),DP(K) */
/* $$$        WRITE(*,*) */
/* $$$        WRITE(*,40)V,PDF,PDD */
/* $$$20      FORMAT(1X,'v =',F6.2,',    ','x =',F6.2) */
/* $$$30      FORMAT(1X,F5.1,2D16.8) */
/* $$$40      FORMAT(1X,'v =',F5.1,',  Dv(x)=',D14.8,',   Dv''(x)=',D14.8) */
/* $$$        END */
/* Subroutine */ int pbdv_(doublereal *v, doublereal *x, doublereal *dv, 
	doublereal *dp, doublereal *pdf, doublereal *pdd)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double d_sign(doublereal *, doublereal *), exp(doublereal);

    /* Local variables */
    static doublereal f;
    static integer k, l, m;
    static doublereal f0, f1, s0, v0, v1, v2;
    static integer ja, na;
    static doublereal ep, pd, xa;
    static integer nk;
    static doublereal vh;
    static integer nv;
    static doublereal pd0, pd1;
    extern /* Subroutine */ int dvla_(doublereal *, doublereal *, doublereal *
	    ), dvsa_(doublereal *, doublereal *, doublereal *);


/*       ==================================================== */
/*       Purpose: Compute parabolic cylinder functions Dv(x) */
/*                and their derivatives */
/*       Input:   x --- Argument of Dv(x) */
/*                v --- Order of Dv(x) */
/*       Output:  DV(na) --- Dn+v0(x) */
/*                DP(na) --- Dn+v0'(x) */
/*                ( na = |n|, v0 = v-n, |v0| < 1, */
/*                  n = 0,ס1,ס2,תתת ) */
/*                PDF --- Dv(x) */
/*                PDD --- Dv'(x) */
/*       Routines called: */
/*             (1) DVSA for computing Dv(x) for small |x| */
/*             (2) DVLA for computing Dv(x) for large |x| */
/*       ==================================================== */

    xa = abs(*x);
    vh = *v;
    *v += d_sign(&c_b2, v);
    nv = (integer) (*v);
    v0 = *v - nv;
    na = abs(nv);
    ep = exp(*x * -.25 * *x);
    if (na >= 1) {
	ja = 1;
    }
    if (*v >= 0.f) {
	if (v0 == 0.f) {
	    pd0 = ep;
	    pd1 = *x * ep;
	} else {
	    i__1 = ja;
	    for (l = 0; l <= i__1; ++l) {
		v1 = v0 + l;
		if (xa <= 5.8f) {
		    dvsa_(&v1, x, &pd1);
		}
		if (xa > 5.8f) {
		    dvla_(&v1, x, &pd1);
		}
		if (l == 0) {
		    pd0 = pd1;
		}
/* L10: */
	    }
	}
	dv[0] = pd0;
	dv[1] = pd1;
	i__1 = na;
	for (k = 2; k <= i__1; ++k) {
	    *pdf = *x * pd1 - (k + v0 - 1.) * pd0;
	    dv[k] = *pdf;
	    pd0 = pd1;
/* L15: */
	    pd1 = *pdf;
	}
    } else {
	if (*x <= 0.f) {
	    if (xa <= 5.8) {
		dvsa_(&v0, x, &pd0);
		v1 = v0 - 1.;
		dvsa_(&v1, x, &pd1);
	    } else {
		dvla_(&v0, x, &pd0);
		v1 = v0 - 1.;
		dvla_(&v1, x, &pd1);
	    }
	    dv[0] = pd0;
	    dv[1] = pd1;
	    i__1 = na;
	    for (k = 2; k <= i__1; ++k) {
		pd = (-(*x) * pd1 + pd0) / (k - 1. - v0);
		dv[k] = pd;
		pd0 = pd1;
/* L20: */
		pd1 = pd;
	    }
	} else if (*x <= 2.f) {
	    v2 = nv + v0;
	    if (nv == 0) {
		v2 += -1.;
	    }
	    nk = (integer) (-v2);
	    dvsa_(&v2, x, &f1);
	    v1 = v2 + 1.;
	    dvsa_(&v1, x, &f0);
	    dv[nk] = f1;
	    dv[nk - 1] = f0;
	    for (k = nk - 2; k >= 0; --k) {
		f = *x * f0 + (k - v0 + 1.) * f1;
		dv[k] = f;
		f1 = f0;
/* L25: */
		f0 = f;
	    }
	} else {
	    if (xa <= 5.8f) {
		dvsa_(&v0, x, &pd0);
	    }
	    if (xa > 5.8f) {
		dvla_(&v0, x, &pd0);
	    }
	    dv[0] = pd0;
	    m = na + 100;
	    f1 = 0.;
	    f0 = 1e-30;
	    for (k = m; k >= 0; --k) {
		f = *x * f0 + (k - v0 + 1.) * f1;
		if (k <= na) {
		    dv[k] = f;
		}
		f1 = f0;
/* L30: */
		f0 = f;
	    }
	    s0 = pd0 / f;
	    i__1 = na;
	    for (k = 0; k <= i__1; ++k) {
/* L35: */
		dv[k] = s0 * dv[k];
	    }
	}
    }
    i__1 = na - 1;
    for (k = 0; k <= i__1; ++k) {
	v1 = abs(v0) + k;
	if (*v >= 0.) {
	    dp[k] = *x * .5 * dv[k] - dv[k + 1];
	} else {
	    dp[k] = *x * -.5 * dv[k] - v1 * dv[k + 1];
	}
/* L40: */
    }
    *pdf = dv[na - 1];
    *pdd = dp[na - 1];
    *v = vh;
    return 0;
} /* pbdv_ */

/* Subroutine */ int dvsa_(doublereal *va, doublereal *x, doublereal *pd)
{
    /* System generated locals */
    doublereal d__1;

    /* Builtin functions */
    double sqrt(doublereal), exp(doublereal), pow_dd(doublereal *, doublereal 
	    *);

    /* Local variables */
    static integer m;
    static doublereal r__, a0, g0, g1, r1, ep, gm, pi, vm, vt, ga0, va0, sq2, 
	    eps;
    extern /* Subroutine */ int gamma_(doublereal *, doublereal *);


/*       =================================================== */
/*       Purpose: Compute parabolic cylinder function Dv(x) */
/*                for small argument */
/*       Input:   x  --- Argument */
/*                va --- Order */
/*       Output:  PD --- Dv(x) */
/*       Routine called: GAMMA for computing ג(x) */
/*       =================================================== */

    eps = 1e-15;
    pi = 3.141592653589793;
    sq2 = sqrt(2.);
    ep = exp(*x * -.25 * *x);
    va0 = (1. - *va) * .5;
    if (*va == 0.f) {
	*pd = ep;
    } else {
	if (*x == 0.f) {
	    if (va0 <= 0.f && va0 == (doublereal) ((integer) va0)) {
		*pd = 0.;
	    } else {
		gamma_(&va0, &ga0);
		d__1 = *va * -.5;
		*pd = sqrt(pi) / (pow_dd(&c_b11, &d__1) * ga0);
	    }
	} else {
	    d__1 = -(*va);
	    gamma_(&d__1, &g1);
	    d__1 = *va * -.5 - 1.;
	    a0 = pow_dd(&c_b11, &d__1) * ep / g1;
	    vt = *va * -.5;
	    gamma_(&vt, &g0);
	    *pd = g0;
	    r__ = 1.;
	    for (m = 1; m <= 250; ++m) {
		vm = (m - *va) * .5;
		gamma_(&vm, &gm);
		r__ = -r__ * sq2 * *x / m;
		r1 = gm * r__;
		*pd += r1;
		if (abs(r1) < abs(*pd) * eps) {
		    goto L15;
		}
/* L10: */
	    }
L15:
	    *pd = a0 * *pd;
	}
    }
    return 0;
} /* dvsa_ */

/* Subroutine */ int dvla_(doublereal *va, doublereal *x, doublereal *pd)
{
    /* System generated locals */
    doublereal d__1;

    /* Builtin functions */
    double exp(doublereal), pow_dd(doublereal *, doublereal *), cos(
	    doublereal);

    /* Local variables */
    static integer k;
    static doublereal r__, a0, x1, gl, ep, pi, vl, eps;
    extern /* Subroutine */ int vvla_(doublereal *, doublereal *, doublereal *
	    ), gamma_(doublereal *, doublereal *);


/*       ==================================================== */
/*       Purpose: Compute parabolic cylinder functions Dv(x) */
/*                for large argument */
/*       Input:   x  --- Argument */
/*                va --- Order */
/*       Output:  PD --- Dv(x) */
/*       Routines called: */
/*             (1) VVLA for computing Vv(x) for large |x| */
/*             (2) GAMMA for computing ג(x) */
/*       ==================================================== */

    pi = 3.141592653589793;
    eps = 1e-12;
    ep = exp(*x * -.25f * *x);
    d__1 = abs(*x);
    a0 = pow_dd(&d__1, va) * ep;
    r__ = 1.;
    *pd = 1.;
    for (k = 1; k <= 16; ++k) {
	r__ = r__ * -.5 * (k * 2.f - *va - 1.f) * (k * 2.f - *va - 2.f) / (k *
		 *x * *x);
	*pd += r__;
	if ((d__1 = r__ / *pd, abs(d__1)) < eps) {
	    goto L15;
	}
/* L10: */
    }
L15:
    *pd = a0 * *pd;
    if (*x < 0.) {
	x1 = -(*x);
	vvla_(va, &x1, &vl);
	d__1 = -(*va);
	gamma_(&d__1, &gl);
	*pd = pi * vl / gl + cos(pi * *va) * *pd;
    }
    return 0;
} /* dvla_ */

/* Subroutine */ int vvla_(doublereal *va, doublereal *x, doublereal *pv)
{
    /* System generated locals */
    doublereal d__1, d__2;

    /* Builtin functions */
    double exp(doublereal), sqrt(doublereal), pow_dd(doublereal *, doublereal 
	    *), sin(doublereal), cos(doublereal);

    /* Local variables */
    static integer k;
    static doublereal r__, a0, x1, gl, qe, pi, pdl, dsl, eps;
    extern /* Subroutine */ int dvla_(doublereal *, doublereal *, doublereal *
	    ), gamma_(doublereal *, doublereal *);


/*       =================================================== */
/*       Purpose: Compute parabolic cylinder function Vv(x) */
/*                for large argument */
/*       Input:   x  --- Argument */
/*                va --- Order */
/*       Output:  PV --- Vv(x) */
/*       Routines called: */
/*             (1) DVLA for computing Dv(x) for large |x| */
/*             (2) GAMMA for computing ג(x) */
/*       =================================================== */

    pi = 3.141592653589793;
    eps = 1e-12;
    qe = exp(*x * .25f * *x);
    d__1 = abs(*x);
    d__2 = -(*va) - 1.;
    a0 = pow_dd(&d__1, &d__2) * sqrt(2. / pi) * qe;
    r__ = 1.;
    *pv = 1.;
    for (k = 1; k <= 18; ++k) {
	r__ = r__ * .5 * (k * 2.f + *va - 1.f) * (k * 2.f + *va) / (k * *x * *
		x);
	*pv += r__;
	if ((d__1 = r__ / *pv, abs(d__1)) < eps) {
	    goto L15;
	}
/* L10: */
    }
L15:
    *pv = a0 * *pv;
    if (*x < 0.) {
	x1 = -(*x);
	dvla_(va, &x1, &pdl);
	d__1 = -(*va);
	gamma_(&d__1, &gl);
	dsl = sin(pi * *va) * sin(pi * *va);
	*pv = dsl * gl / pi * pdl - cos(pi * *va) * *pv;
    }
    return 0;
} /* vvla_ */

/* Subroutine */ int gamma_(doublereal *x, doublereal *ga)
{
    /* Initialized data */

    static doublereal g[26] = { 1.,.5772156649015329,-.6558780715202538,
	    -.0420026350340952,.1665386113822915,-.0421977345555443,
	    -.009621971527877,.007218943246663,-.0011651675918591,
	    -2.152416741149e-4,1.280502823882e-4,-2.01348547807e-5,
	    -1.2504934821e-6,1.133027232e-6,-2.056338417e-7,6.116095e-9,
	    5.0020075e-9,-1.1812746e-9,1.043427e-10,7.7823e-12,-3.6968e-12,
	    5.1e-13,-2.06e-14,-5.4e-15,1.4e-15,1e-16 };

    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double sin(doublereal);

    /* Local variables */
    static integer k, m;
    static doublereal r__, z__;
    static integer m1;
    static doublereal pi, gr;


/*       ================================================== */
/*       Purpose: Compute gamma function ג(x) */
/*       Input :  x  --- Argument of ג(x) */
/*                       ( x is not equal to 0,-1,-2,תתת) */
/*       Output:  GA --- ג(x) */
/*       ================================================== */

    pi = 3.141592653589793;
    if (*x == (doublereal) ((integer) (*x))) {
	if (*x > 0.) {
	    *ga = 1.;
	    m1 = (integer) (*x - 1);
	    i__1 = m1;
	    for (k = 2; k <= i__1; ++k) {
/* L10: */
		*ga *= k;
	    }
	} else {
	    *ga = 1e300;
	}
    } else {
	if (abs(*x) > 1.) {
	    z__ = abs(*x);
	    m = (integer) z__;
	    r__ = 1.;
	    i__1 = m;
	    for (k = 1; k <= i__1; ++k) {
/* L15: */
		r__ *= z__ - k;
	    }
	    z__ -= m;
	} else {
	    z__ = *x;
	}
	gr = g[25];
	for (k = 25; k >= 1; --k) {
/* L20: */
	    gr = gr * z__ + g[k - 1];
	}
	*ga = 1. / (gr * z__);
	if (abs(*x) > 1.) {
	    *ga *= r__;
	    if (*x < 0.) {
		*ga = -pi / (*x * *ga * sin(pi * *x));
	    }
	}
    }
    return 0;
} /* gamma_ */

