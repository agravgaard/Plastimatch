#include "plm_clp.h"

static void
print_usage (dlib::Plm_clp& parser)
{
    std::cout << "Usage: dlib_test [options] --in input_file [file ...]\n";
    parser.print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_args (dlib::Plm_clp& parser, int argc, char* argv[])
{
    try {
	// Algorithm-independent options
        parser.add_long_option ("a","algorithm",
	    "Choose the learning algorithm: {krls,krr,mlp,svr}. "
	    "Choose the learning algorithm: {krls,krr,mlp,svr}. "
	    "Choose the learning algorithm: {krls,krr,mlp,svr}. ",
	    1,"Foob");
        parser.add_long_option ("b","","The 'b' option",1);
        parser.add_long_option ("h","help","Display this help message.");
        parser.add_long_option ("k","kernel",
	    "Learning kernel (for krls,krr,svr methods): {lin,rbk}.",1);
        parser.add_long_option ("","in","A libsvm-formatted file to test.",1);
        parser.add_long_option ("","normalize",
	    "Normalize the sample inputs to zero-mean unit variance?");
        parser.add_long_option ("","train-best",
	    "Train and save a network using best parameters", 1);

        parser.add_long_option ("", "verbose", "Use verbose trainers");

        parser.add_long_option ("c","cool-option",
	    "The 'c/cool' option",1,"default-c-value");

	// Parse the command line arguments
        parser.parse(argc,argv);

	// Check that options aren't given multiple times
        const char* one_time_opts[] = {"a", "h", "help", "in"};
        parser.check_one_time_options(one_time_opts);

        // Check if the -h option was given
        if (parser.option("h") || parser.option("help")) {
	    print_usage (parser);
	    exit (0);
        }

	// Check that an input file was given
        if (!parser.option("in")) {
	    std::cout << 
		"Error. "
		"You must specify an input file with the --in option.\n";
	    print_usage (parser);
	    exit (0);
	}
    }
    catch (std::exception& e) {
        // Catch cmd_line_parse_error exceptions and print usage message.
	std::cout << "Error. " << e.what() << std::endl;
	print_usage (parser);
	exit (1);
    }
    catch (...) {
	std::cout << "Some error occurred" << std::endl;
    }
    for (unsigned int i =0; i < parser.number_of_arguments(); ++i) {
	std::cout << "unlabeled option " << i << " is " 
		  << parser[i]
		  << std::endl;
    }
}

int 
main (int argc, char* argv[])
{
    dlib::Plm_clp parser;

    parse_args(parser, argc, argv);
}
