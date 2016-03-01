/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <algorithm>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <itksys/RegularExpression.hxx>
#include "itkDirectory.h"

#include "xio_dir.h"
#include "xio_patient.h"

Xio_patient::Xio_patient (
    const char* path
) 
{
    this->m_path = path;
    this->m_demographic_fn = "";
}

Xio_patient::~Xio_patient () 
{
}

void
Xio_patient::add_studyset_dir (
    const std::string& studyset_path
)
{
    this->studyset_dirs.push_back (studyset_path);
}

void
Xio_patient::add_plan_dir (
    const std::string& plan_path
)
{
    this->plan_dirs.push_back (plan_path);
}

void
Xio_patient::analyze ()
{
    itksys::Directory itk_dir;
    std::string plan_path = std::string(this->m_path) + "/plan";
    std::string studyset_path = std::string(this->m_path) + "/anatomy/studyset";

    if (itk_dir.Load (studyset_path.c_str())) {
	for (unsigned long i = 0; i < itk_dir.GetNumberOfFiles(); i++) {
	    std::string curr_file = itk_dir.GetFile(i);
	    std::string curr_path = studyset_path + "/" + itk_dir.GetFile(i);

	    if (Xio_dir::is_xio_studyset_dir (curr_path)) {
		printf ("Adding xsd: %s\n", curr_path.c_str());
		this->add_studyset_dir (curr_path);
	    }
	}
    }

    if (itk_dir.Load (plan_path.c_str())) {
	for (unsigned long i = 0; i < itk_dir.GetNumberOfFiles(); i++) {
	    std::string curr_file = itk_dir.GetFile(i);
	    std::string curr_path = plan_path + "/" + itk_dir.GetFile(i);

	    if (Xio_dir::is_xio_plan_dir (curr_path)) {
		printf ("Adding xtpd: %s\n", curr_path.c_str());
		this->add_plan_dir (curr_path);
	    }
	}
    }
}
