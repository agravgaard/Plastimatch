/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_and_exit.h"
#include "xio_studyset.h"

int Xio_studyset::gcd(int a, int b) {

    // Euclidean algorithm

    while (b != 0) {
	int t = b;
	b = a % b;
	a = t;
    }

    return a;

}

Xio_studyset::Xio_studyset (const std::string& input_dir)
{
    this->studyset_dir = input_dir;

    // Open index.dat file in input_dir
    std::string indexdat(input_dir);
    indexdat += "/index.dat";
    std::ifstream index (indexdat.c_str());

    int all_number_slices = 0;
    std::vector<Xio_studyset_slice> all_slices;

    if (index.is_open()) {
	// Get total number of slices
	index >> all_number_slices;

	// Loop through slices getting filename and location
	std::string slice_filename_scan;
	std::string slice_name;
	float slice_location;

	for (int i = 0; i < all_number_slices; i++) {
	    index >> slice_filename_scan;
	    index >> slice_location;

	    Xio_studyset_slice slice (slice_filename_scan, slice_location);
	    all_slices.push_back (slice);
	}

	// Sort slices in positive direction
	std::sort (all_slices.begin(), all_slices.end());
    } else {
	all_number_slices = 0;
    }

    // Workaround for multiple slice thickness
    // Create volume with uniform voxel sizes by finding greatest common divisor of slice thicknesses,
    // and duplicating slices to obtain a uniform Z axis.

    // Get slices thicknesses from CT files

    std::vector<float> slice_thickness;

    for (size_t i = 0; i < all_slices.size(); i++) {

	std::string ct_file = this->studyset_dir + "/" + all_slices[i].filename_scan;
	std::string line;

	// Open file
	std::ifstream ifs(ct_file.c_str(), std::ifstream::in);
	if (ifs.fail()) {
	    print_and_exit("Error opening CT file %s for read\n", ct_file.c_str());
	} else {
	    // Skip 14 lines
	    for (int i = 0; i < 14; i++) {
		getline(ifs, line);
	    }

	    getline(ifs, line);

	    int dummy;
	    float highres_thickness;

	    if (sscanf(line.c_str(), "%d,%g", &dummy, &highres_thickness) != 2) {
		print_and_exit("Error parsing slice thickness (%s)\n", line.c_str());
	    }

	    slice_thickness.push_back(highres_thickness);
	}
	
    }

    // Find greatest common divisor

    std::vector<int> slice_thickness_int;
    int slice_thickness_gcd = 1;

    for (size_t i = 0; i < all_slices.size(); i++) {
	// 1/1000 mm resolution
	int rounded_thickness = static_cast<int> (slice_thickness[i] * 1000.);
	if (rounded_thickness == 0) rounded_thickness = 1;
	slice_thickness_int.push_back(rounded_thickness);
    }

    if (all_slices.size() == 1) {
	slice_thickness_gcd = slice_thickness_int[0];
    }
    else if (all_slices.size() > 0) {
	slice_thickness_gcd = gcd(slice_thickness_int[0], slice_thickness_int[1]);
	for (size_t i = 2; i < all_slices.size(); i++) {
		slice_thickness_gcd = gcd(slice_thickness_gcd, slice_thickness_int[i]);
	}
    }

    // Build new slice list, determining duplication needed for each slice

    thickness = slice_thickness_gcd / 1000.;
    number_slices = 0;

    if (all_slices.size() > 0) {
	float location = all_slices[0].location - (slice_thickness[0] / 2.) + (thickness / 2.);

	for (size_t i = 0; i < all_slices.size(); i++) {
	    int duplicate = slice_thickness_int[i] / slice_thickness_gcd;

	    for (int j = 0; j < duplicate; j++) {
		    Xio_studyset_slice slice(all_slices[i].filename_scan, location);
		    slices.push_back(slice);
		    location += thickness;
		    number_slices++;
	    }
	}
    }

    // Initialize pixel spacing to zero.  This get set when the 
    // CT is loaded
    this->ct_pixel_spacing[0] = this->ct_pixel_spacing[1] = 0.f;
}

Xio_studyset::~Xio_studyset ()
{
}


Xio_studyset_slice::Xio_studyset_slice (std::string slice_filename_scan, const float slice_location)
{
    filename_scan = slice_filename_scan;
    location = slice_location;

    // Get name from slice filename
    size_t extension_dot = filename_scan.find_last_of("."); 
    name = filename_scan.substr(0, extension_dot);

    filename_contours = name + ".WC";
}

Xio_studyset_slice::~Xio_studyset_slice ()
{
}
