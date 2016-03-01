/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <list>

#include "pcmd_header.h"
#include "plm_clp.h"
#include "plm_image.h"

class Header_parms {
public:
    std::list<std::string> input_fns;
};

static void
header_main (Header_parms* parms)
{
    Plm_image pli;

    std::list<std::string>::iterator it = parms->input_fns.begin();
    while (it != parms->input_fns.end()) {
        pli.load_native (*it);
        pli.print ();
        ++it;
    }
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf (
        "Usage: plastimatch header [options] input_file [input_file ...]\n");
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Header_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that no extraneous options were given */
    if (parser->number_of_arguments() == 0) {
	throw (dlib::error ("Error.  You must specify at least one "
                "file for printing header."));
    }

    /* Copy input filenames to parms struct */
    for (unsigned long i = 0; i < parser->number_of_arguments(); i++) {
        parms->input_fns.push_back ((*parser)[i]);
    }
}

void
do_command_header (int argc, char *argv[])
{
    Header_parms parms;
    
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);
    header_main (&parms);
}
