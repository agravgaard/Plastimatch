/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmdose_config.h"

#include "rt_lut.h"
#include "rpl_volume.h" // for PMMA constants

double get_proton_range(double energy)
{
    int i_lo = 0, i_hi = PROTON_TABLE_SIZE;
    double energy_lo = lookup_proton_range_water[i_lo][0];
    double range_lo = lookup_proton_range_water[i_lo][1];
    double energy_hi = lookup_proton_range_water[i_hi][0];
    double range_hi = lookup_proton_range_water[i_hi][1];

    if (energy <= energy_lo) {
        return range_lo;
    }
    if (energy >= energy_hi) {
        return range_hi;
    }

    /* Use binary search to find lookup table entries */
    for (int dif = i_hi - i_lo; dif > 1; dif = i_hi - i_lo) {
        int i_test = i_lo + ((dif + 1) / 2);
        double energy_test = lookup_proton_range_water[i_test][0];
        if (energy > energy_test) {
            energy_lo = energy_test;
            i_lo = i_test;
        } else {
            energy_hi = energy_test;
            i_hi = i_test;
        }
    }

    range_lo = lookup_proton_range_water[i_lo][1];
    range_hi = lookup_proton_range_water[i_hi][1];
    return range_lo + 
        (energy-energy_lo) * (range_hi-range_lo) / (energy_hi-energy_lo);
}

double get_proton_stop (double energy)
{
    int i_lo = 0, i_hi = PROTON_TABLE_SIZE;
    double energy_lo = lookup_proton_stop_water[i_lo][0];
    double stop_lo = lookup_proton_stop_water[i_lo][1];
    double energy_hi = lookup_proton_stop_water[i_hi][0];
    double stop_hi = lookup_proton_stop_water[i_hi][1];

    if (energy <= energy_lo) {
        return stop_lo;
    }
    if (energy >= energy_hi) {
        return stop_hi;
    }

    /* Use binary search to find lookup table entries */
    for (int dif = i_hi - i_lo; dif > 1; dif = i_hi - i_lo) {
        int i_test = i_lo + ((dif + 1) / 2);
        double energy_test = lookup_proton_stop_water[i_test][0];
        if (energy > energy_test) {
            energy_lo = energy_test;
            i_lo = i_test;
        } else {
            energy_hi = energy_test;
            i_hi = i_test;
        }
    }

    stop_lo = lookup_proton_stop_water[i_lo][1];
    stop_hi = lookup_proton_stop_water[i_hi][1];
    return stop_lo + 
        (energy-energy_lo) * (stop_hi-stop_lo) / (energy_hi-energy_lo);
}

double get_theta0_Highland(double range)
{
	/* lucite sigma0 (in rads) computing- From the figure A2 of the Hong's paper (be careful, in this paper the fit shows sigma0^2)*/
	if (range > 150)
	{
		return 0.05464 + 5.8348E-6 * range -5.21006E-9 * range * range;
	}
	else 
	{
		return 0.05394 + 1.80222E-5 * range -5.5430E-8 * range * range;
	}
}

double get_theta0_MC(float energy)
{
	return 4.742E-6 * energy * energy -1.918E-3 * energy + 1.158;
}

double get_theta_rel_Highland(double rc_over_range)
{
	return rc_over_range * ( 1.6047 -2.7295 * rc_over_range + 2.1578 * rc_over_range * rc_over_range);
}

double get_theta_rel_MC(double rc_over_range)
{
	return 3.833E-2 * pow(rc_over_range, 0.657) + 2.118E-2 * pow(rc_over_range, 6.528);
}

double get_scat_or_Highland(double rc_over_range)
{
	/* calculation of rc_eff - see Hong's paper graph A3 - linear interpolation of the curve */
	if (rc_over_range >= 0 && rc_over_range < 0.5)
    {
        return 1 - (0.49 + 0.060 / 0.5 * rc_over_range);
    }
    else if (rc_over_range >= 0.5 && rc_over_range <0.8)
    {
        return 1 - (0.55 + 0.085 / 0.3 * (rc_over_range-0.5));
    }
    else if (rc_over_range >= 0.8 && rc_over_range <0.9)
    {
        return 1 - (0.635 + 0.055 / 0.1 * (rc_over_range-0.8));
    }
    else if (rc_over_range >= 0.9 && rc_over_range <0.95)
    {
        return 1 - (0.690 + (rc_over_range-0.9));
    }
    else if (rc_over_range >= 0.95 && rc_over_range <= 1)
    {
        return 1 - (0.740 + 0.26/0.05 * (rc_over_range-0.95));
    }
    else if (rc_over_range > 1)
    {
        return 0;
    }
    else
    {
        return 0;
    }
}

double get_scat_or_MC(double rc_over_range)
{
	return 0.023 * rc_over_range + 0.332;
}

double compute_X0_from_HU(double CT_HU)
{
    if (CT_HU <= -1000)
    {
        return 30390;
    }
    else if (CT_HU > -1000 && CT_HU< 0)
    {
        return exp(3.7271E-06 * CT_HU * CT_HU -3.009E-03 * CT_HU + 3.5857);
    }
    else if (CT_HU >= 0 && CT_HU < 55)
    {
        return -0.0284 * CT_HU + 36.08;
    }
    else
    {
        return 9.8027E-06 * CT_HU * CT_HU -.028939 * CT_HU + 36.08;
    }
}

/* Make GCC compiler less whiny */
#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 2)
#pragma GCC diagnostic ignored "-Wmissing-braces"
#endif

/* This table has 111 entries */
const double lookup_proton_range_water[][2] ={
1.000E-03,	6.319E-06,
1.500E-03,	8.969E-06,	
2.000E-03,	1.137E-05,	
2.500E-03,	1.357E-05,	
3.000E-03,	1.560E-05,	
4.000E-03,	1.930E-05,	
5.000E-03,	2.262E-05,	
6.000E-03,	2.567E-05,	
7.000E-03,	2.849E-05,	
8.000E-03,	3.113E-05,	
9.000E-03,	3.363E-05,	
1.000E-02,	3.599E-05,	
1.250E-02,	4.150E-05,	
1.500E-02,	4.657E-05,	
1.750E-02,	5.131E-05,	
2.000E-02,	5.578E-05,	
2.250E-02,	6.005E-05,	
2.500E-02,	6.413E-05,	
2.750E-02,	6.806E-05,	
3.000E-02,	7.187E-05,	
3.500E-02,	7.916E-05,	
4.000E-02,	8.613E-05,	
4.500E-02,	9.284E-05,	
5.000E-02,	9.935E-05,	
5.500E-02,	1.057E-04,	
6.000E-02,	1.120E-04,	
6.500E-02,	1.182E-04,	
7.000E-02,	1.243E-04,	
7.500E-02,	1.303E-04,	
8.000E-02,	1.364E-04,	
8.500E-02,	1.425E-04,	
9.000E-02,	1.485E-04,	
9.500E-02,	1.546E-04,	
1.000E-01,	1.607E-04,	
1.250E-01,	1.920E-04,	
1.500E-01,	2.249E-04,	
1.750E-01,	2.598E-04,	
2.000E-01,	2.966E-04,	
2.250E-01,	3.354E-04,	
2.500E-01,	3.761E-04,	
2.750E-01,	4.186E-04,	
3.000E-01,	4.631E-04,	
3.500E-01,	5.577E-04,	
4.000E-01,	6.599E-04,	
4.500E-01,	7.697E-04,	
5.000E-01,	8.869E-04,	
5.500E-01,	1.012E-03,	
6.000E-01,	1.144E-03,	
6.500E-01,	1.283E-03,	
7.000E-01,	1.430E-03,	
7.500E-01,	1.584E-03,	
8.000E-01,	1.745E-03,	
8.500E-01,	1.913E-03,	
9.000E-01,	2.088E-03,	
9.500E-01,	2.270E-03,	
1.000E+00,	2.458E-03,	
1.250E+00,	3.499E-03,	
1.500E+00,	4.698E-03,	
1.750E+00,	6.052E-03,	
2.000E+00,	7.555E-03,	
2.250E+00,	9.203E-03,	
2.500E+00,	1.099E-02,	
2.750E+00,	1.292E-02,	
3.000E+00,	1.499E-02,	
3.500E+00,	1.952E-02,	
4.000E+00,	2.458E-02,	
4.500E+00,	3.015E-02,	
5.000E+00,	3.623E-02,	
5.500E+00,	4.279E-02,	
6.000E+00,	4.984E-02,	
6.500E+00,	5.737E-02,	
7.000E+00,	6.537E-02,	
7.500E+00,	7.384E-02,	
8.000E+00,	8.277E-02,	
8.500E+00,	9.215E-02,	
9.000E+00,	1.020E-01,	
9.500E+00,	1.123E-01,	
1.000E+01,	1.230E-01,	
1.250E+01,	1.832E-01,	
1.500E+01,	2.539E-01,	
1.750E+01,	3.350E-01,	
2.000E+01,	4.260E-01,	
2.500E+01,	6.370E-01,	
2.750E+01,	7.566E-01,	
3.000E+01,	8.853E-01,	
3.500E+01,	1.170E+00,	
4.000E+01,	1.489E+00,	
4.500E+01,	1.841E+00,	
5.000E+01,	2.227E+00,	
5.500E+01,	2.644E+00,	
6.000E+01,	3.093E+00,	
6.500E+01,	3.572E+00,	
7.000E+01,	4.080E+00,	
7.500E+01,	4.618E+00,	
8.000E+01,	5.184E+00,	
8.500E+01,	5.777E+00,	
9.000E+01,	6.398E+00,	
9.500E+01,	7.045E+00,	
1.000E+02,	7.718E+00,	
1.250E+02,	1.146E+01,	
1.500E+02,	1.577E+01,	
1.750E+02,	2.062E+01,	
2.000E+02,	2.596E+01,	
2.250E+02,	3.174E+01,	
2.500E+02,	3.794E+01,	
2.750E+02,	4.452E+01,	
3.000E+02,	5.145E+01,	
3.500E+02,	6.628E+01,	
4.000E+02,	8.225E+01,	
4.500E+02,	9.921E+01,	
5.000E+02,	1.170E+02,	
};

/* This table has 111 entries */
const double lookup_proton_stop_water[][2] =
{
0.0010,	176.9,
0.0015,	198.4,
0.0020,	218.4,
0.0025,	237.0,
0.0030,	254.4,
0.0040,	286.4,
0.0050,	315.3,
0.0060,	342.0,
0.0070,	366.7,
0.0080,	390.0,
0.0090,	412.0,
0.0100,	432.9,
0.0125,	474.5,
0.0150,	511.0,
0.0175,	543.7,
0.0200,	573.3,
0.0225,	600.1,
0.0250,	624.5,
0.0275,	646.7,
0.0300,	667.1,
0.0350,	702.8,
0.0400,	732.4,
0.0450,	756.9,
0.0500,	776.8,
0.0550,	792.7,
0.0600,	805.0,
0.0650,	814.2,
0.0700,	820.5,
0.0750,	824.3,
0.0800,	826.0,
0.0850,	825.8,
0.0900,	823.9,
0.0950,	820.6,
0.1000,	816.1,
0.1250,	781.4,
0.1500,	737.1,
0.1750,	696.9,
0.2000,	661.3,
0.2250,	629.4,
0.2500,	600.6,
0.2750,	574.4,
0.3000,	550.4,
0.3500,	508.0,
0.4000,	471.9,
0.4500,	440.6,
0.5000,	413.2,
0.5500,	389.1,
0.6000,	368.0,
0.6500,	349.2,
0.7000,	332.5,
0.7500,	317.5,
0.8000,	303.9,
0.8500,	291.7,
0.9000,	280.5,
0.9500,	270.2,
1.0000,	260.8,
1.2500,	222.9,
1.5000,	195.7,
1.7500,	174.9,
2.0000,	158.6,
2.2500,	145.4,
2.5000,	134.4,
2.7500,	125.1,
3.0000,	117.2,
3.5000,	104.2,
4.0000,	94.04,
4.5000,	85.86,
5.0000,	79.11,
5.5000,	73.43,
6.0000,	68.58,
6.5000,	64.38,
7.0000,	60.71,
7.5000,	57.47,
8.0000,	54.60,
8.5000,	52.02,
9.0000,	49.69,
9.5000,	47.59,
10.000,	45.67,
12.500,	38.15,
15.000,	32.92,
17.500,	29.05,
20.000,	26.07,
25.000,	21.75,
27.500,	20.13,
30.000,	18.76,
35.000,	16.56,
40.000,	14.88,
45.000,	13.54,
50.000,	12.45,
55.000,	11.54,
60.000,	10.78,
65.000,	10.13,
70.000,	9.559,
75.000,	9.063,
80.000,	8.625,
85.000,	8.236,
90.000,	7.888,
95.000,	7.573,
100.00,	7.289,
125.00,	6.192,
150.00,	5.445,
175.00,	4.903,
200.00,	4.492,
225.00,	4.170,
250.00,	3.911,
275.00,	3.698,
300.00,	3.520,
350.00,	3.241,
400.00,	3.032,
450.00,	2.871,
500.00,	2.743,
};

/* matrix that contains the alpha and p parameters from equation: Range = f(E, alpha, p)
    First line is proton, second line is He... */

extern const double particle_parameters[][2] = {
    0.00217, 1.7709,//P
	/* To be updated to the right values for ions */
    0.0022, 1.77,   //HE
    0.0022, 1.77,   //LI
    0.0022, 1.77,   //BE
    0.0022, 1.77,   //B
    0.0022, 1.77,   //C
    0.00,   0.00,   //N not used for ion therapy - set to 0
    0.0022, 1.77    //O
};
