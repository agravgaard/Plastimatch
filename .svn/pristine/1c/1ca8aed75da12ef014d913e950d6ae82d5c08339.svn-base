#include <float.h>
#include <stdio.h>

typedef union {
  long long i64;
  double d64;
} dbl_64;

double machine_eps (double value)
{
    dbl_64 s;
    s.d64 = value;
    s.i64++;
    return (s.i64 < 0 ? value - s.d64 : s.d64 - value);
}

int main (int argc, char* argv[])
{

    printf ("DBL_EPSILON = %G\n", DBL_EPSILON);

    printf ("Machine epsilon (alg 1) = %G\n", machine_eps((double)1.0));
    
    double machEps = 1.0f;
    printf( "current Epsilon, 1 + current Epsilon\n" );
    do {
       printf( "%G\t%.20f\n", machEps, (1.0f + machEps) );
       machEps /= 2.0f;
       // If next epsilon yields 1, then break, because current
       // epsilon is the machine epsilon.
    }
    while ((double)(1.0 + (machEps/2.0)) != 1.0);
 
    printf( "Calculated Machine epsilon: %G\n", machEps );
    return 0;
}
