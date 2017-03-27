/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   Algorithm for doing an XVI archive.
   
   ** This algorithm does not use dbase file, which reduces need
   to access xvi computer **

   (1) Get input directory from command line
   (2) Identify CBCT images according to 
       <INDIR>/IMAGES/img_<CBCT_UID>/Reconstruction/<RECON_UID>.SCAN
   (3) Load Reconstruction/<RECON_UID>.INI
   (3a) Parse this file to get Name, TreatmentID, 
   (4) Load Reconstruction/<RECON_UID>.INI.XVI
   (4a) Parse this file to get Date, Xform
   (5) Identify reference image in Mosaiq (optional?)
   
   ** Needs fix for multiple reference studies **

   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdlib.h>
#include <string.h>
#include "INIReader.h"
#if PLM_DCM_USE_DCMTK
#include "dcmtk_config.h"
#include "dcmtk/dcmdata/dctk.h"
#endif

#include "dcmtk_sro.h"
#include "dir_list.h"
#include "file_util.h"
#include "path_util.h"
#include "plm_clp.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "rt_study.h"
#include "string_util.h"
#include "xvi_archive.h"

Rt_study::Pointer
load_reference_ct (
    const std::string& patient_ct_set_dir,
    const std::string& cbct_ref_uid)
{
    std::string reference_ct_dir = string_format ("%s/%s",
        patient_ct_set_dir.c_str(), cbct_ref_uid.c_str());
    if (is_directory (reference_ct_dir)) {
        Rt_study::Pointer reference_study = Rt_study::New();
        reference_study->load (reference_ct_dir);
        return reference_study;
    } else {
        printf ("Error.  No matching reference CT found.\n");
        return Rt_study::Pointer();
    }
}

void
do_xvi_archive (Xvi_archive_parms *parms)
{
    std::string patient_ct_set_dir = compose_filename (
        parms->patient_dir, "CT_SET");
    std::string patient_images_dir = compose_filename (
        parms->patient_dir, "IMAGES");

    Dir_list images_dir (patient_images_dir);
    if (images_dir.num_entries == 0) {
        printf ("Error.  No images found.\n");
        return;
    }

    /* For potential CBCT image */
    for (int i = 0; i < images_dir.num_entries; i++) {
        if (images_dir.entries[i][0] == '.') {
            continue;
        }
        /* Find pertinent filenames */
        std::string recon_dir = string_format ("%s/%s/Reconstruction",
            patient_images_dir.c_str(), images_dir.entries[i]);
        Dir_list recon_list (recon_dir);
        std::string scan_fn, recon_ini_fn, recon_xvi_fn;
        for (int j = 0; j < recon_list.num_entries; j++) {
            if (!extension_is (recon_list.entries[j], "SCAN")) {
                continue;
            }
            scan_fn = compose_filename (recon_dir, recon_list.entries[j]);
            std::string recon_uid 
                = strip_extension (std::string(recon_list.entries[j]));
            recon_ini_fn = compose_filename (recon_dir, 
                string_format ("%s.INI", recon_uid.c_str()));
            recon_xvi_fn = compose_filename (recon_dir, 
                string_format ("%s.INI.XVI", recon_uid.c_str()));
            break;
        }
        printf ("Recon dir is \"%s\".\n", recon_dir.c_str());
        if (scan_fn == ""
            || !file_exists (scan_fn) 
            || !file_exists (recon_ini_fn)
            || !file_exists (recon_xvi_fn))
        {
            printf ("Missing file in Recon dir.  Skipping.\n");
            continue;
        }

        printf ("cbct_dir = %s/%s\n", 
            patient_images_dir.c_str(), images_dir.entries[i]);
        
        /* Load the INI file */
        INIReader recon_ini (recon_ini_fn);
        std::string cbct_ref_uid = 
            recon_ini.Get ("IDENTIFICATION", "ReferenceUID", "");
        std::string patient_name = 
            string_format ("%s^%s",
                recon_ini.Get ("IDENTIFICATION", "LastName", "").c_str(),
                recon_ini.Get ("IDENTIFICATION", "FirstName", "").c_str());
        std::string status_line_string = 
            recon_ini.Get ("XVI", "StatusLineText", "");
        std::string linac_string = "";
        size_t n = status_line_string.find ("Plan Description:");
        if (n != std::string::npos) {
            linac_string = status_line_string.substr (
                n + strlen ("Plan Description:"));
            n = linac_string.find_first_not_of (" \t\r\n");
            linac_string = linac_string.substr (n);
            n = linac_string.find_first_of (" \t\r\n");
            if (n != std::string::npos) {
                linac_string = linac_string.substr (0, n);
            }
        }
        printf ("name = %s\n", patient_name.c_str());
        printf ("reference_uid = %s\n", cbct_ref_uid.c_str());
        printf ("linac_string = %s\n", linac_string.c_str());

#if defined (commentout)
        /* Verify if the file belongs to this reference CT */
        if (cbct_ref_uid != reference_uid) {
            printf ("Reference UID mismatch.  Skipping.\n");
            continue;
        }
#endif

        /* Load the matching reference CT */
        Rt_study::Pointer reference_study = load_reference_ct (
            patient_ct_set_dir, cbct_ref_uid);
        if (!reference_study) {
            printf ("No matching CT for this CBCT.  Skipping.\n");
            continue;
        }

        /* Extract metadata from reference CT */
        Rt_study_metadata::Pointer& reference_meta = 
            reference_study->get_rt_study_metadata ();
        printf ("Reference Meta: %s %s\n",
            reference_meta->get_patient_name().c_str(),
            reference_meta->get_patient_id().c_str());

        /* Load the INI.XVI file */
        INIReader recon_xvi (recon_xvi_fn);
        std::string date_time_string = 
            recon_xvi.Get ("ALIGNMENT", "DateTime", "");
        std::string date_string, time_string;
        size_t semicol_pos = date_time_string.find (";");
        if (semicol_pos != std::string::npos) {
            printf ("semicol_pos = %d\n", (int) semicol_pos);
            date_string 
                = string_trim (date_time_string.substr (0, semicol_pos));
            printf ("date = |%s|\n", date_string.c_str());
            time_string 
                = string_trim (date_time_string.substr (semicol_pos+1));
            while (1) {
                size_t colon_pos = time_string.find (":");
                if (colon_pos == std::string::npos) {
                    break;
                }
                time_string = time_string.substr (0, colon_pos)
                    + time_string.substr (colon_pos+1);
            }
            printf ("time = |%s|\n", time_string.c_str());
        }
        std::string unmatched_transform_string = 
            recon_xvi.Get ("ALIGNMENT", "OnlineToRefTransformUnmatched", "");
        printf ("unmatched xform = %s\n", unmatched_transform_string.c_str());
        std::string registration_string = 
            recon_xvi.Get ("ALIGNMENT", "OnlineToRefTransformCorrection", "");
        printf ("correction xform = %s\n", registration_string.c_str());
        if (unmatched_transform_string == "") {
            printf ("No unmatched xform for this CBCT.  Skipping.\n");
            continue;
        }

        /* Load the .SCAN */
        Rt_study cbct_study;
        cbct_study.load_image (scan_fn);
        if (!cbct_study.have_image()) {
            printf ("ERROR: decompression failure with patient %s\n",
                reference_meta->get_patient_id().c_str());
            exit (1);
        }

        /* Set DICOM image header fields */
        Rt_study_metadata::Pointer& cbct_meta 
            = cbct_study.get_rt_study_metadata ();
        cbct_meta->set_patient_name (patient_name);
        if (parms->patient_id_override != "") {
            cbct_meta->set_patient_id (parms->patient_id_override);
        } else {
            cbct_meta->set_patient_id (
                reference_meta->get_patient_id().c_str());
        }
        if (date_string != "" && time_string != "") {
            cbct_meta->set_study_date (date_string);
            cbct_meta->set_study_time (time_string);
            cbct_meta->set_image_metadata(DCM_InstanceCreationDate, date_string);
            cbct_meta->set_image_metadata(DCM_InstanceCreationTime, time_string);
        }
        std::string study_description = "CBCT: " + linac_string;
        cbct_meta->set_study_metadata (DCM_StudyDescription, study_description);
        cbct_meta->set_study_uid (reference_meta->get_study_uid());
        cbct_meta->set_image_metadata (DCM_WindowCenter, "500");
        cbct_meta->set_image_metadata (DCM_WindowWidth, "2000");
        std::string patient_position
            = reference_meta->get_image_metadata (DCM_PatientPosition);
        cbct_meta->set_image_metadata (DCM_PatientPosition, patient_position);
        std::string cbct_series_description
            = "CBCT " + date_string + " " + time_string;
        cbct_meta->set_image_metadata (DCM_SeriesDescription,
            cbct_series_description);

        /* Set DICOM SRO header fields */
        std::string sro_series_description
            = "REG " + date_string + " " + time_string;
        reference_meta->set_sro_metadata (DCM_SeriesDescription,
            sro_series_description);
        
        printf ("REF CT patient position is %s\n", patient_position.c_str());

        // XiO incorrectly sets patient position metadata in their header
        // This maneuver is intended to correct this
        std::vector<float> uta
            = parse_float_string (unmatched_transform_string);
        if (within_abs_tolerance (uta[2], 1.f, 0.001f)
            && within_abs_tolerance (uta[5], 1.f, 0.001f)
            && within_abs_tolerance (uta[8], -1.f, 0.001f))
        {
            if (patient_position == "HFP") {
                patient_position = "FFP";
                printf ("Patient position corrected to %s\n",
                    patient_position.c_str());
            } else if (patient_position == "HFS") {
                patient_position = "FFS";
                printf ("Patient position corrected to %s\n",
                    patient_position.c_str());
            }
        }

        /* Fix patient orientation based on reference CT */
        float dc[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
        if (patient_position == "HFS") {
            /* Do nothing */
        }
        else if (patient_position == "HFP") {
            // dc = { -1, 0, 0, 0, -1, 0, 0, 0, 1 };
            dc[0] = dc[4] = -1;
            cbct_study.get_image()->get_volume()->set_direction_cosines (dc);
        }
        else if (patient_position == "FFS") {
            // dc = { -1, 0, 0, 0, 1, 0, 0, 0, -1 };
            dc[0] = dc[8] = -1;
            cbct_study.get_image()->get_volume()->set_direction_cosines (dc);
        }
        else if (patient_position == "FFP") {
            // dc = { 1, 0, 0, 0, -1, 0, 0, 0, -1 };
            dc[4] = dc[8] = -1;
            cbct_study.get_image()->get_volume()->set_direction_cosines (dc);
        }
        else {
            /* Punt */
            patient_position = "HFS";
        }
        float origin[3];
        cbct_study.get_image()->get_volume()->get_origin(origin);
        origin[0] = dc[0] * origin[0];
        origin[1] = dc[4] * origin[1];
        origin[2] = dc[8] * origin[2];
        cbct_study.get_image()->get_volume()->set_origin (origin);

        if (parms->write_debug_files) {
            /* Nb this has to be done before writing dicom, since 
               that operation scrambles the image (!) */
            cbct_study.save_image ("cbct.nrrd");
        }

        /* Write the DICOM image */
        std::string output_dir = string_format (
            "cbct_output/%s/%s", 
            reference_meta->get_patient_id().c_str(),
            images_dir.entries[i]);
        cbct_study.save_dicom (output_dir);

        /* Create the DICOM SRO */
        AffineTransformType::Pointer aff = AffineTransformType::New();
        AffineTransformType::ParametersType xfp(12);
        float xvip[16];
        int rc = sscanf (registration_string.c_str(), 
            "%f %f %f %f %f %f %f %f "
            "%f %f %f %f %f %f %f %f",
            &xvip[0], &xvip[1], &xvip[2], &xvip[3], 
            &xvip[4], &xvip[5], &xvip[6], &xvip[7], 
            &xvip[8], &xvip[9], &xvip[10], &xvip[11], 
            &xvip[12], &xvip[13], &xvip[14], &xvip[15]);
        if (rc != 16) {
            printf ("Error parsing transform string.\n");
            exit (1);
        }

        printf ("XVI\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n", 
            xvip[0], xvip[1], xvip[2], xvip[3], 
            xvip[4], xvip[5], xvip[6], xvip[7], 
            xvip[8], xvip[9], xvip[10], xvip[11], 
            xvip[12], xvip[13], xvip[14], xvip[15]);
        
        if (patient_position == "HFS") {
            xfp[0] =   xvip[8];
            xfp[1] =   xvip[9];
            xfp[2] =   xvip[10];
            xfp[3] =   xvip[4];
            xfp[4] =   xvip[5];
            xfp[5] =   xvip[6];
            xfp[6] = - xvip[0];
            xfp[7] = - xvip[1];
            xfp[8] = - xvip[2];

            // B
            xfp[9]  =   (xfp[0]*xvip[12] + xfp[3]*xvip[13] + xfp[6]*xvip[14]);
            xfp[10] =   (xfp[1]*xvip[12] + xfp[4]*xvip[13] + xfp[7]*xvip[14]);
            xfp[11] = - (xfp[2]*xvip[12] + xfp[5]*xvip[13] + xfp[8]*xvip[14]);

            // "A", Verified
            xfp[9]  = - (xfp[0]*xvip[12] + xfp[1]*xvip[13] + xfp[2]*xvip[14]);
            xfp[10] = - (xfp[3]*xvip[12] + xfp[4]*xvip[13] + xfp[5]*xvip[14]);
            xfp[11] = - (xfp[6]*xvip[12] + xfp[7]*xvip[13] + xfp[8]*xvip[14]);
        
        }
        else if (patient_position == "HFP") {
            xfp[0] =   xvip[8];
            xfp[1] =   xvip[9];
            xfp[2] = - xvip[10];
            xfp[3] =   xvip[4];
            xfp[4] =   xvip[5];
            xfp[5] = - xvip[6];
            xfp[6] =   xvip[0];
            xfp[7] =   xvip[1];
            xfp[8] = - xvip[2];

            // "A", Unlikely
            xfp[9]  =   (xfp[0]*xvip[12] + xfp[1]*xvip[13] + xfp[2]*xvip[14]);
            xfp[10] = - (xfp[3]*xvip[12] + xfp[4]*xvip[13] + xfp[5]*xvip[14]);
            xfp[11] = - (xfp[6]*xvip[12] + xfp[7]*xvip[13] + xfp[8]*xvip[14]);
        
            // "B", Possible
            xfp[9]  =   (xfp[0]*xvip[12] + xfp[3]*xvip[13] + xfp[6]*xvip[14]);
            xfp[10] =   (xfp[1]*xvip[12] + xfp[4]*xvip[13] + xfp[7]*xvip[14]);
            xfp[11] = - (xfp[2]*xvip[12] + xfp[5]*xvip[13] + xfp[8]*xvip[14]);

        }
        else if (patient_position == "FFS") {
            xfp[0] = - xvip[8];
            xfp[1] = - xvip[9];
            xfp[2] = - xvip[10];
            xfp[3] =   xvip[4];
            xfp[4] =   xvip[5];
            xfp[5] =   xvip[6];
            xfp[6] =   xvip[0];
            xfp[7] =   xvip[1];
            xfp[8] =   xvip[2];

            // "B", Unlikely
            xfp[9]  = - (xfp[0]*xvip[12] + xfp[3]*xvip[13] + xfp[6]*xvip[14]);
            xfp[10] =   (xfp[1]*xvip[12] + xfp[4]*xvip[13] + xfp[7]*xvip[14]);
            xfp[11] = - (xfp[2]*xvip[12] + xfp[5]*xvip[13] + xfp[8]*xvip[14]);

            // "A", Possible
            xfp[9]  =   (xfp[0]*xvip[12] + xfp[1]*xvip[13] + xfp[2]*xvip[14]);
            xfp[10] = - (xfp[3]*xvip[12] + xfp[4]*xvip[13] + xfp[5]*xvip[14]);
            xfp[11] = - (xfp[6]*xvip[12] + xfp[7]*xvip[13] + xfp[8]*xvip[14]);
        
            // "C", Possible
            xfp[9]  = - (xfp[0]*xvip[12] + xfp[1]*xvip[13] + xfp[2]*xvip[14]);
            xfp[10] = - (xfp[3]*xvip[12] + xfp[4]*xvip[13] + xfp[5]*xvip[14]);
            xfp[11] = - (xfp[6]*xvip[12] + xfp[7]*xvip[13] + xfp[8]*xvip[14]);
        
        }
        else if (patient_position == "FFP") {
            xfp[0] = - xvip[8];
            xfp[1] = - xvip[9];
            xfp[2] =   xvip[10];
            xfp[3] =   xvip[4];
            xfp[4] =   xvip[5];
            xfp[5] = - xvip[6];
            xfp[6] = - xvip[0];
            xfp[7] = - xvip[1];
            xfp[8] =   xvip[2];

            // A
            xfp[9]  =   (xfp[0]*xvip[12] + xfp[1]*xvip[13] + xfp[2]*xvip[14]);
            xfp[10] =   (xfp[3]*xvip[12] + xfp[4]*xvip[13] + xfp[5]*xvip[14]);
            xfp[11] = - (xfp[6]*xvip[12] + xfp[7]*xvip[13] + xfp[8]*xvip[14]);
        
            // "B", Mostly Verified
            xfp[9]  =   (xfp[0]*xvip[12] + xfp[3]*xvip[13] + xfp[6]*xvip[14]);
            xfp[10] =   (xfp[1]*xvip[12] + xfp[4]*xvip[13] + xfp[7]*xvip[14]);
            xfp[11] = - (xfp[2]*xvip[12] + xfp[5]*xvip[13] + xfp[8]*xvip[14]);
        }

        // Convert cm to mm
        xfp[9]  *= 10;
        xfp[10] *= 10;
        xfp[11] *= 10;
        
#if defined (commentout)
        aff->SetParametersByValue (xfp);
        vnl_matrix_fixed< double, 3, 3 > xfp_rot_inv = 
            aff->GetMatrix().GetInverse();
        printf ("XFORM-R INV\n%f %f %f\n%f %f %f\n%f %f %f\n",
            xfp_rot_inv[0][0],
            xfp_rot_inv[0][1],
            xfp_rot_inv[0][2],
            xfp_rot_inv[1][0],
            xfp_rot_inv[1][1],
            xfp_rot_inv[1][2],
            xfp_rot_inv[2][0],
            xfp_rot_inv[2][1],
            xfp_rot_inv[2][2]);
#endif
        
        // dicom translation = - 10 * dicom_rotation * xvi translation
        // Old, "perfect" HFS setting
#if defined (commentout)
        xfp[9]  = -10 * (xfp[0]*xvip[12] + xfp[1]*xvip[13] + xfp[2]*xvip[14]);
        xfp[10] = -10 * (xfp[3]*xvip[12] + xfp[4]*xvip[13] + xfp[5]*xvip[14]);
        xfp[11] = -10 * (xfp[6]*xvip[12] + xfp[7]*xvip[13] + xfp[8]*xvip[14]);

        xfp[9]  = -10 * (xfp[0]*xvip[12] + xfp[3]*xvip[13] + xfp[6]*xvip[14]);
        xfp[10] = -10 * (xfp[1]*xvip[12] + xfp[4]*xvip[13] + xfp[7]*xvip[14]);
        xfp[11] = -10 * (xfp[2]*xvip[12] + xfp[5]*xvip[13] + xfp[8]*xvip[14]);
#endif
        
        Xform::Pointer xf = Xform::New();
        xf->set_aff (xfp);

        printf ("XFORM\n%f %f %f\n%f %f %f\n%f %f %f\n%f %f %f\n",
            xfp[0],
            xfp[1],
            xfp[2],
            xfp[3],
            xfp[4],
            xfp[5],
            xfp[6],
            xfp[7],
            xfp[8],
            xfp[9],
            xfp[10],
            xfp[11]
        );

        if (parms->write_debug_files) {
            reference_study->save_image ("ct.nrrd");
            xf->save ("xf.tfm");
        }
        
        Dcmtk_sro::save (
            xf,
            reference_study->get_rt_study_metadata (),
            cbct_study.get_rt_study_metadata (),
            output_dir, true);
    }
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: xvi_archive [options]\n");
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Xvi_archive_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Input files and directories */
    parser->add_long_option ("", "patient-directory", 
        "base directory containing patient images", 1, "");

    /* Other options */
    parser->add_long_option ("", "patient-id-override", 
        "set the patient id", 1, "");
    parser->add_long_option ("", "write-debug-files",
        "write converted image and xform files for debugging", 0);
    
    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Input files */
    parms->patient_dir = parser->get_string("patient-directory");
    if (parms->patient_dir == "") {
        throw (dlib::error (
                "Error.  The use of --patient-directory is needed"));
    }

    /* Other options */
    parms->patient_id_override = parser->get_string("patient-id-override");
    if (parser->have_option ("write-debug-files")) {
        parms->write_debug_files = true;
    }
}


int
main (int argc, char *argv[])
{
    Xvi_archive_parms parms;
    
    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 0);

    /* Do the job */
    do_xvi_archive (&parms);
}
