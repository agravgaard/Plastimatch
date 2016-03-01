#include <stdio.h>
#include <stdint.h>

int main ()
{
    float o = -24.34210586547851562500;
    float s = 2.63158011436462402344;
    float vo = -25.00000000000000000000;
    float vs = 1.31579005718231201172;
    int i;
    float z;

    for (i = 0, z = o; i < 2; i++, z += s) {
        float t = (z - vo) / vs;
        printf ("0> %.20f\n", t);
    }
    printf ("----\n");
    for (i = 0, z = o; i < 3; i++, z += s) {
        float t = (z - vo) / vs;
        printf ("0> %.20f\n", t);
    }
}
