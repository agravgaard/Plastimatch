#include <stdio.h>

#define VSIZE 900

int main 
(
    int argc,
    char* argv[]
)
{
    int dim[3] = { VSIZE, VSIZE, VSIZE };
    int fudge = 100;
    int ijk[3];
    size_t v;
    for (ijk[2] = 0; ijk[2] < VSIZE; ijk[2]++) {
	for (ijk[1] = 0; ijk[1] < VSIZE; ijk[1]++) {
	    for (ijk[0] = 0; ijk[0] < VSIZE; ijk[0]++) {
		v = ((ijk[2] * dim[1] + ijk[1]) * dim[0] + ijk[0]) * fudge;
		if (v % (30 * dim[1] * dim[0] * fudge) == 0) {
		    printf ("v = %zu (%zu)\n", v, 
			v / (30 * dim[1] * dim[0] * fudge));
		}
		if (v % (((size_t) 30) * dim[1] * dim[0] * fudge) == 0) {
		    printf ("v = %zu (%zu)\n", v, 
			v / (((size_t) 30) * dim[1] * dim[0] * fudge));
		}
	    }
	}
    }
    printf ("pct = %zu\n", (30 * dim[1] * dim[0] * fudge));
    printf ("v = %zu\n", v);
    return 0;
}
