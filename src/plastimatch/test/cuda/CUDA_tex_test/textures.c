#include <stdio.h>
#include <stdlib.h>
#include "tex_stubs.h"


int main()
{
	int elements = 100;
	float* test_data = (float*)malloc(elements*sizeof(float));


	// Generate some work
	int i;
	for (i = 0; i < elements; i++)
		test_data[i] = (float)i;


	// Invoke the texture kernel
	CUDA_texture_test(test_data, elements);


	// Print results
	for (i = 0; i < elements; i++)
		printf("%f\n", test_data[i]);
	

	free(test_data);

	return 0;
}
