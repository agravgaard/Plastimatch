#include <stdio.h>
#include "smart_pointer.h"

class A {
public:
    SMART_POINTER_SUPPORT(A);
public:
    int *val;
public:
    A () {
        val = new int;
        *val = 3;
    }
    ~A () {
        delete val;
    }
};

int 
main (int argc, char* argv[])
{
    A::Pointer a;
    if (a) {
        printf ("a is non-null\n");
    } else {
        printf ("a is null\n");
    }

    A::Pointer b = A::New();
    if (b) {
        printf ("b is non-null\n");
    } else {
        printf ("b is null\n");
    }

    A::Pointer c = A::New();
    c.reset();
    if (c) {
        printf ("c is non-null\n");
    } else {
        printf ("c is null\n");
    }


    return 0;
}
