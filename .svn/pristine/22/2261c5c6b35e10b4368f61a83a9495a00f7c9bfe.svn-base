/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <algorithm>
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <itksys/RegularExpression.hxx>
#include "itkDirectory.h"

#include "file_util.h"
#include "path_util.h"
#include "xio_dir.h"
#include "xio_patient.h"
#include "xio_plan.h"

/* -----------------------------------------------------------------------
   Private functions
   ----------------------------------------------------------------------- */
int
Xio_dir::is_xio_patient_dir (std::string dir)
{
    itksys::Directory itk_dir;
    if (!itk_dir.Load (dir.c_str())) {
	return 0;
    }

    /* A patient directory has either an anatomy or a plan directory */
    for (unsigned long i = 0; i < itk_dir.GetNumberOfFiles(); i++) {
	std::string curr_file = itk_dir.GetFile(i);
	std::string curr_path = dir + "/" + itk_dir.GetFile(i);
	
	if (itksys::SystemTools::FileIsDirectory (curr_path.c_str())) {
	    if (curr_file == "anatomy") return 1;
	    if (curr_file == "plan")  return 1;
	}
    }
    return 0;
}

int
Xio_dir::is_xio_studyset_dir (std::string dir)
{
    itksys::Directory itk_dir;
    if (!itk_dir.Load (dir.c_str())) {
	return 0;
    }

    /* A studyset directory has either a *.CT file or a *.WC file */
    for (unsigned long i = 0; i < itk_dir.GetNumberOfFiles(); i++) {
	std::string curr_file = itk_dir.GetFile(i);
	std::string curr_path = dir + "/" + itk_dir.GetFile(i);

	if (itksys::SystemTools::FileIsDirectory (curr_path.c_str())) {
	    continue;
	}

	if (extension_is (curr_file.c_str(), ".WC")) return 1;
	if (extension_is (curr_file.c_str(), ".CT")) return 1;
    }
    return 0;
}

int
Xio_dir::is_xio_plan_dir (const std::string& dir)
{
    itksys::Directory itk_dir;
    if (!itk_dir.Load (dir.c_str())) {
	return 0;
    }

    /* A plan directory has a plan file */
    for (unsigned long i = 0; i < itk_dir.GetNumberOfFiles(); i++) {
	std::string curr_file = itk_dir.GetFile(i);
	std::string curr_path = dir + "/" + itk_dir.GetFile(i);

	if (itksys::SystemTools::FileIsDirectory (curr_path.c_str())) {
	    continue;
	}

	if (curr_file == "plan") return 1;
    }
    return 0;
}

Xio_patient*
Xio_dir::add_patient_dir (std::string dir)
{
    Xio_patient *xpd = new Xio_patient (dir.c_str());
    this->patient_dir.push_back (xpd);
    return xpd;
}

void
Xio_dir::analyze_recursive (std::string dir)
{
    itksys::Directory itk_dir;

    if (!itk_dir.Load (dir.c_str())) {
	return;
    }

    /* Look for top-level patient directory */
    if (is_xio_patient_dir (dir)) {
	printf ("Found plan dir\n");
	Xio_patient *xpd = this->add_patient_dir (dir);
	xpd->analyze ();

	std::string demographic_file = dir + "/demographic";
	if (file_exists (demographic_file.c_str())) {
	    xpd->m_demographic_fn = demographic_file.c_str();
	}
	return;
    }

    /* Look for plan directories.
       GCS FIX: Each plan counts as a separate patient */
    else if (is_xio_plan_dir (dir)) {
	Xio_patient *xpd = this->add_patient_dir (dir);
	xpd->add_plan_dir (dir);

	std::string demographic_file = dir + "/../../demographic";
	if (file_exists (demographic_file.c_str())) {
	    xpd->m_demographic_fn = demographic_file.c_str();
	}
	return;
    }

    /* Look for studyset directories.  
       GCS FIX: Each studyset counts as a separate patient */
    else if (is_xio_studyset_dir (dir)) {
	Xio_patient *xpd = this->add_patient_dir (dir);
	xpd->add_studyset_dir (dir);

	std::string demographic_file = dir + "/../../../demographic";
	if (file_exists (demographic_file.c_str())) {
	    xpd->m_demographic_fn = demographic_file.c_str();
	}
	return;
    }

    for (unsigned long i = 0; i < itk_dir.GetNumberOfFiles(); i++) {
	std::string curr_file = itk_dir.GetFile(i);
	std::string curr_path = dir + "/" + itk_dir.GetFile(i);
	
	if (curr_file == "." || curr_file == "..") continue;

	if (itksys::SystemTools::FileIsDirectory (curr_path.c_str())) {
	    this->analyze_recursive (curr_path);
	}
    }
}

/* -----------------------------------------------------------------------
   Public functions
   ----------------------------------------------------------------------- */
Xio_dir::Xio_dir (const char *input_dir)
{
    this->path = input_dir;
    this->analyze ();
}

Xio_dir::~Xio_dir ()
{
    std::vector<Xio_patient*>::iterator it;
    for (it = this->patient_dir.begin(); it < this->patient_dir.end(); ++it) {
	delete *it;
    }
}

void
Xio_dir::analyze ()
{
    if (!is_directory (this->path)) {
	return;
    }

    this->analyze_recursive (this->path);
}

int
Xio_dir::num_patients () const
{
    return (int) this->patient_dir.size();
}

std::string 
xio_plan_dir_get_studyset_dir (const std::string& xio_plan_dir)
{
    std::string studyset_dir;
    std::string plan_dir;
    std::string patient_dir;

    if (Xio_dir::is_xio_plan_dir (xio_plan_dir)) {
	/* Get studyset name from plan */
	std::string plan_file = xio_plan_dir + "/plan";
	printf ("plan_file: %s\n", plan_file.c_str());
	studyset_dir = xio_plan_get_studyset (plan_file.c_str());

	/* Obtain patient directory from plan directory */
	plan_dir = file_util_parent (xio_plan_dir);
	patient_dir = file_util_parent (plan_dir);
	printf ("plan_dir: %s\n", plan_file.c_str());
	printf ("patient_dir: %s\n", patient_dir.c_str());

	/* Set studyset directory */
	studyset_dir = std::string(patient_dir) +
	    "/anatomy/studyset/" + studyset_dir;
    }

    return studyset_dir;
}
