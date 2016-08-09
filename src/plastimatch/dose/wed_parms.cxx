/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "plm_math.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "wed_parms.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

Wed_Parms::Wed_Parms ()
{
    this->debug = 0;
    this->group = 0;
    this->mode = 0;
    this->ray_step = 1.0f;

    this->src[0] = -1000.f;
    this->src[1] = 0.f;
    this->src[2] = 0.f;
    this->isocenter[0] = 0.f;
    this->isocenter[1] = 0.f;
    this->isocenter[2] = 0.f;
    this->beam_res = 1.f;

    this->vup[0] = 0.f;
    this->vup[1] = 0.f;
    this->vup[2] = 1.f;
    this->have_ires = false;
    this->ires[0] = 200;
    this->ires[1] = 200;
    this->have_ic = false;
    this->ic[0] = 99.5f;
    this->ic[1] = 99.5f;
    this->ap_spacing[0] = 1.;
    this->ap_spacing[1] = 1.;
    this->ap_offset = 100;

    this->dew_dim[0] = -999.;
    this->dew_dim[1] = -999.;
    this->dew_dim[2] = -999.;
    this->dew_origin[0] = -999.;
    this->dew_origin[1] = -999.;
    this->dew_origin[2] = -999.;
    this->dew_spacing[0] = -999.;
    this->dew_spacing[1] = -999.;
    this->dew_spacing[2] = -999.;

    this->sinogram = 0;
    this->sinogram_res = 360;
}

Wed_Parms::~Wed_Parms ()
{
}

static void
print_usage (void)
{
    printf ("Usage: wed config_file\n");
    printf ("Options:\n");
    printf ("\t--dew (reverse wed calculation)\n");
    printf ("\t--segdepth \n");
    printf ("\t--projwed \n");
    exit (1);
}

int
Wed_Parms::get_group_lines(char* groupfile)
{
    std::string line;
    std::ifstream text(groupfile);
    int numlines = 0;
    if (text.is_open())  {
        while (text.good()) {
            getline(text,line);	    
            if ( (!line.empty()) && (line.compare(0,1,"#")) )  {
                numlines++;
            }
        }
    }
    return numlines;
}

#if defined (commentout)
void
Wed_Parms::parse_group(int argc, char** argv, int linenumber)
{
    int linecounter = 0;
    for (int i=1; i<argc; i++) {
        if (!strcmp (argv[i], "--group")) {
            std::string line;
            std::ifstream text(argv[i+1]);
            if (text.is_open()) {
                while (text.good()) {
                    getline(text,line);
                    if ( (!line.empty()) && (line.compare(0,1,"#")) )  {
                        if (linecounter == linenumber)  {

                            std::string pvol_file;
                            std::string dose_file;
                            std::string dose_wed_file;

                            std::stringstream linestream(line);

                            linestream >> pvol_file >> dose_file >> dose_wed_file;

                            if (pvol_file.size()>=4)  {
                                if (pvol_file.compare(pvol_file.size()-4,4,".mha"))  {
                                    print_and_exit ("%s is not in <name>.mha format.\n", pvol_file.c_str());
                                    return;
                                }
                            }
                            else {print_and_exit ("%s is not in <name>.mha format.\n", pvol_file.c_str());}

                            if (dose_file.size()>=4)  {
                                if (dose_file.compare(dose_file.size()-4,4,".mha"))  {
                                    print_and_exit ("%s is not an .mha file.\n", dose_file.c_str());
                                    return;
                                }
                            }
                            else {print_and_exit ("%s is not in <name>.mha format.\n", dose_file.c_str());}

                            this->input_ct_fn = pvol_file;
                            this->input_dose_fn = dose_file.c_str();

                            //add "_wed" to  pvol_file names
                            pvol_file.insert (pvol_file.size()-4,"_wed");   
                            this->output_ct_fn = pvol_file;
                            this->output_dose_fn = dose_wed_file.c_str();
                        }
                        linecounter++;
                    }
                }
            }
        }
    }
}
#endif

int
Wed_Parms::set_key_val (
    const char* key, 
    const char* val, 
    int section
)
{
    switch (section) {

        /* [INPUT SETTINGS] */
    case 0:
        //Whether wed or reverse, input patient and rpl vol
        /* patient is legacy term */
        if (!strcmp (key, "ct") || !strcmp (key, "patient")) {
            this->input_ct_fn = val;
        }
        else if (!strcmp (key, "proj_wed")) {
            this->input_proj_wed_fn = val;
        }
        //Any mode will use the skin dose if specified
        else if (!strcmp (key, "skin")) {
            this->input_skin_fn = val;
        }
        //If normal wed procedure, input dose
        else if (!strcmp (key, "dose")) {
            this->input_dose_fn = val;
        }
        //If reverse wed procedure, input dose_wed
        else if (!strcmp (key, "wed_dose") || !strcmp (key, "dose_wed")) {
            this->input_wed_dose_fn = val;
        }
        //If in depth/segmentation mode, input segment
        else if (!strcmp (key, "target")) {
            this->input_target_fn = val;
        }
        break;
        
        /* [OUTPUT SETTINGS] */
    case 1:
        if (!strcmp (key, "proj_ct")) {
            this->output_proj_ct_fn = val;
        }
        else if (!strcmp (key, "proj_wed")) {
            this->output_proj_wed_fn = val;
        }
        else if (!strcmp (key, "proj_dose")) {
            this->output_proj_dose_fn = val;
        }
        /* patient_wed is legacy term */
        else if (!strcmp (key, "wed_ct") || !strcmp (key, "patient_wed")) {
            this->output_wed_ct_fn = val;
        }
        /* dose_wed is legacy term */
        else if (!strcmp (key, "wed_dose") || !strcmp (key, "dose_wed")) {
            this->output_wed_dose_fn = val;
        }
        else if (!strcmp (key, "ct")) {
            this->output_ct_fn = val;
        }
        else if (!strcmp (key, "dew_ct")) {
            this->output_dew_ct_fn = val;
        }
        /* dose is legacy term */
        else if (!strcmp (key, "dew_dose") || !strcmp (key, "dose")) {
            this->output_dew_dose_fn = val;
        }
#if defined (commentout)
        else if (!strcmp (key, "aperture")) {
            this->output_ap_fn = val;
        }
#endif
        break;

        /* [BEAM] */
    case 2:
        if (!strcmp (key, "ray_step")) {
            if (sscanf (val, "%f", &this->ray_step) != 1) {
                goto error_exit;
            }
        }
        else if (!strcmp (key, "pos")) {
            if (sscanf (val, "%f %f %f", 
                    &(this->src[0]), 
                    &(this->src[1]), 
                    &(this->src[2])) != 3)
            {
                goto error_exit;
            }
        }
        else if (!strcmp (key, "isocenter")) {
            if (sscanf (val, "%f %f %f", 
                    &(this->isocenter[0]), 
                    &(this->isocenter[1]), 
                    &(this->isocenter[2])) != 3)
            {
                goto error_exit;
            }
        }
        else if (!strcmp (key, "res")) {
            if (sscanf (val, "%f", &(this->beam_res)) != 1) {
                goto error_exit;
            }
        }
        break;

        /* [APERTURE] */
    case 3:
        if (!strcmp (key, "up")) {
            if (sscanf (val, "%f %f %f", &(this->vup[0]), &(this->vup[1]), &(this->vup[2])) != 3) {
                goto error_exit;
            }
        }
        else if (!strcmp (key, "center")) {
            if (sscanf (val, "%f %f", &(this->ic[0]), &(this->ic[1])) != 2) {
                goto error_exit;
            }
            this->have_ic = true;
        }
        else if (!strcmp (key, "offset")) {
            if (sscanf (val, "%f", &(this->ap_offset)) != 1) {
                goto error_exit;
            }
        }
        else if (!strcmp (key, "spacing")) {
            if (sscanf (val, "%f %f", 
                    &(this->ap_spacing[0]), 
                    &(this->ap_spacing[1])) != 2)
            {
                goto error_exit;
            }
        }
        else if (!strcmp (key, "resolution")) {
            if (sscanf (val, "%i %i", 
                    &(this->ires[0]), 
                    &(this->ires[1])) != 2)
            {
                goto error_exit;
            }
            this->have_ires = true;
        }
        break;

        /* [DEW VOLUME] */
    case 4:
        if (!strcmp (key, "dew_dim")) {
            if (sscanf (val, "%f %f %f", &(this->dew_dim[0]), &(this->dew_dim[1]), &(this->dew_dim[2])) != 3) {
                goto error_exit;
            }
        }
        else if (!strcmp (key, "dew_origin")) {
            if (sscanf (val, "%f %f %f", &(this->dew_origin[0]), &(this->dew_origin[1]), &(this->dew_origin[2])) != 3) {
                goto error_exit;
            }
        }
        if (!strcmp (key, "dew_spacing")) {
            if (sscanf (val, "%f %f %f", &(this->dew_spacing[0]), &(this->dew_spacing[1]), &(this->dew_spacing[2])) != 3) {
                goto error_exit;
            }
        }
        break;

        /* [PROJECTION VOLUME] */
    case 5:
        if (!strcmp (key, "sinogram")) {
            if (sscanf (val, "%f", &(this->sinogram)) != 1) {
                goto error_exit;
            }
        }
        else if (!strcmp (key, "resolution")) {
            if (sscanf (val, "%i", &(this->sinogram_res)) != 1) {
                goto error_exit;
            }
        }
        break;
    }
    return 0;

error_exit:
    print_and_exit ("Unknown (key,val) combination: (%s,%s)\n", key, val);
    return -1;
}

void
Wed_Parms::parse_config (
    const char* config_fn
)
{
    /* Read file into string */
    std::ifstream t (config_fn);
    std::stringstream buffer;
    buffer << t.rdbuf();

    std::string buf;
    std::string buf_ori;    /* An extra copy for diagnostics */
    int section = -1;

    std::stringstream ss (buffer.str());

    while (getline (ss, buf)) {
        buf_ori = buf;
        buf = string_trim (buf);
        buf_ori = string_trim (buf_ori, "\r\n");

        if (buf == "") continue;
        if (buf[0] == '#') continue;

        if (buf[0] == '[') {
            if (buf.find ("[INPUT SETTINGS]") != std::string::npos
                || buf.find ("[input settings]") != std::string::npos)
            {
                section = 0;
                continue;
            }
            if (buf.find ("[OUTPUT SETTINGS]") != std::string::npos
                || buf.find ("[output settings]") != std::string::npos)
            {
                section = 1;
                continue;
            }
            else if (buf.find ("[BEAM]") != std::string::npos
                || buf.find ("[beam]") != std::string::npos)
            {
                section = 2;
                continue;
            }
            else if (buf.find ("[APERTURE]") != std::string::npos
                || buf.find ("[aperture]") != std::string::npos)
            {
                section = 3;
                continue;
            }
            else if (buf.find ("[DEW VOLUME]") != std::string::npos
                || buf.find ("[dew volume]") != std::string::npos)
            {
                section = 4;
                continue;
            }
            else if (buf.find ("[PROJECTION VOLUME]") != std::string::npos
                || buf.find ("[projective volume]") != std::string::npos)
            {
                section = 5;
                continue;
            }
            else {
                printf ("Parse error: %s\n", buf_ori.c_str());
            }
        }

        size_t key_loc = buf.find ("=");
        if (key_loc == std::string::npos) {
            continue;
        }

        std::string key = buf.substr (0, key_loc);
        std::string val = buf.substr (key_loc+1);
        key = string_trim (key);
        val = string_trim (val);

        if (key != "" && val != "") {
            if (this->set_key_val (key.c_str(), val.c_str(), section) < 0) {
                printf ("Parse error: %s\n", buf_ori.c_str());
            }
        }
    }
}

bool
Wed_Parms::parse_args (int argc, char** argv)
{
    int i;
    for (i=1; i<argc; i++) {
        if (argv[i][0] != '-') break;

        if (!strcmp (argv[i], "--debug")) {
            this->debug = 1;
        }
        if (!strcmp (argv[i], "--group")) {
            if (!argv[i+1])  { //group needs an argument
                print_usage ();
                return false;
            }
            else {
                this->group = get_group_lines(argv[i+1]);
                return true;
            }
        }
        if (!strcmp (argv[i], "--dew")) {
            this->mode = 1;
        }
        else if (!strcmp (argv[i], "--segdepth")) {
            this->mode = 2;
        }
        else if (!strcmp (argv[i], "--projwed")) {
            this->mode = 3;
        }
        else {
            print_usage ();
            break;
        }
    }

    if (!argv[i]) {
        print_usage ();
    } else {
        this->parse_config (argv[i]);
    }

    //Input CT always required
    if (this->input_ct_fn == "") {
        print_and_exit ("** ERROR: Input patient image not specified in configuration file!\n");
    }

    return true;
}
