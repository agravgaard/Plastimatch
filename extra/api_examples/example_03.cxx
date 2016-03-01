#include <stdio.h>
#include "dcmtk_loader.h"
#include "print_and_exit.h"

int main (int argc, char* argv[])
{
    if (argc < 2) {
        print_and_exit ("Usage: example_03 directory");
    }
    Dcmtk_loader loader (argv[1]);
    loader.debug();


}
