/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print_and_exit.h"
#include "xio_plan.h"

std::string
xio_plan_get_studyset (const char *filename)
{
    /* Open file */
    std::ifstream ifs (filename, std::ifstream::in);
    if (ifs.fail()) {
        print_and_exit ("Error opening file %s for read\n", filename);
    }

    /* Get version string
       0062101a - xio version 4.33.02
       006d101a - xio version 4.50 */
    std::string line;
    getline (ifs, line);
    printf ("Version = %s\n", line.c_str());
    int rc, version_int;
    rc = sscanf (line.c_str(), "%x", &version_int);
    if (rc != 1) {
	/* Couldn't parse version string -- default to older format. */
	version_int = 0x62101a;
    }
    printf ("rc = %d, version_int = 0x%x\n", rc, version_int);
    
    /* Skip 4 lines for xio 4.33.02, skip 5 lines for xio 4.50. */
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);
    getline (ifs, line);
    if (version_int > 0x62101a) {
        getline (ifs, line);
    }

    /* Read and return studyset name */
    getline (ifs, line);
    return line;
}
