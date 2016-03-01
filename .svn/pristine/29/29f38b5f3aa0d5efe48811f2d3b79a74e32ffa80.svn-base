#include <stdio.h>
#include <limits>
//#include "itkNumericTraits.h"

int main 
(
    int argc,
    char* argv[]
)
{
   short min_short = std::numeric_limits<short>::min();
   short max_short = std::numeric_limits<short>::max();
   unsigned short min_ushort = std::numeric_limits<unsigned short>::min();
   unsigned short max_ushort = std::numeric_limits<unsigned short>::max();
   int min_int = std::numeric_limits<int>::min();
   int max_int = std::numeric_limits<int>::max();
   unsigned int min_uint = std::numeric_limits<unsigned int>::min();
   unsigned int max_uint = std::numeric_limits<unsigned int>::max();
   long min_long = std::numeric_limits<long>::min();
   long max_long = std::numeric_limits<long>::max();
   unsigned long min_ulong = std::numeric_limits<unsigned long>::min();
   unsigned long max_ulong = std::numeric_limits<unsigned long>::max();
   float min_float = std::numeric_limits<float>::min();
   float max_float = std::numeric_limits<float>::max();

   printf ("short %ld %ld\n", (long) min_short, (long) max_short);
   printf ("ushort %ld %ld\n", (long) min_ushort, (long) max_ushort);
   printf ("int %d %d\n", min_int, max_int);
   printf ("uint %u %u\n", min_uint, max_uint);
   printf ("long %ld %ld\n", min_long, max_long);
   printf ("ulong %lu %lu\n", min_ulong, max_ulong);
   printf ("float %e %e\n", min_float, max_float);

   short ss1 = -33;
   printf ("%d < %d == %s\n", ss1, min_ushort, 
       (ss1 < min_ushort) ? "true" : "false");

   unsigned short us1 = 44000;
   printf ("%d > %d == %s\n", us1, max_short, 
       (us1 > max_short) ? "true" : "false");

   long sl1 = -33;
   printf ("%ld < %lu == %s\n", sl1, min_ulong, 
       (sl1 < min_ulong) ? "true" : "false");

   unsigned long ul1 = max_long + 100;
   printf ("%lu > %ld == %s\n", ul1, max_long, 
       (ul1 > max_long) ? "true" : "false");
}
