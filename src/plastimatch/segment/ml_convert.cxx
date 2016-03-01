/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsegment_config.h"
#include <fstream>
#include <stdio.h>
#include "itkImageRegionIterator.h"

#include "dir_list.h"
#include "file_util.h"
#include "itk_image_create.h"
#include "itk_image_save.h"
#include "logfile.h"
#include "ml_convert.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_timer.h"
#include "print_and_exit.h"
#include "string_util.h"

class Ml_convert_private
{
public:
    std::string append_filename;
    std::string input_ml_results_filename;
    std::string label_filename;
    std::string mask_filename;
    std::string output_filename;
    std::string output_format;
    std::list<std::string> feature_dir;
    Plm_image_type plm_image_type;
public:
    void image_from_ml ();
    void ml_from_image ();
protected:
    template<class T>
    void image_from_ml_internal ();
    template<class T>
    T choose_value (
        float value
    );
};


void
Ml_convert_private::ml_from_image ()
{
    Plm_timer pli;
    pli.start ();

    /* Create files for ping-pong -- unfortunately we have to use FILE* 
       due to lack of fstream support for temporary filenames */
    FILE *fp[2], *current, *previous;
    fp[0] = make_tempfile ();
    fp[1] = make_tempfile ();
    current = fp[0];
    previous = fp[0];

    bool vw_format = true;
    if (this->output_format == "libsvm") {
        vw_format = false;
    }

    /* The index of the features */
    int idx = 0;

#define BUFSIZE 1024*1024
    char buf[BUFSIZE];
    size_t chars_in_buf = 0;
    char *buf_ptr;

    /* Load mask */
    bool have_mask = false;
    UCharImageType::Pointer mask_itk;
    itk::ImageRegionIterator< UCharImageType > mask_it;
    if (this->mask_filename != "") {
        Plm_image::Pointer mask = Plm_image::New (this->mask_filename);
        mask_itk = mask->itk_uchar();
        mask_it = itk::ImageRegionIterator< UCharImageType > (
            mask_itk, mask_itk->GetLargestPossibleRegion());
        have_mask = true;
    }
    /* Load labelmap */
    if (this->label_filename != "") {
        lprintf ("Processing labelmap\n");
        Plm_image::Pointer labelmap = Plm_image::New (this->label_filename);
        UCharImageType::Pointer labelmap_itk = labelmap->itk_uchar();

        /* Dump labels to file */
        UCharImageType::RegionType rg = labelmap_itk->GetLargestPossibleRegion ();
        itk::ImageRegionIterator< UCharImageType > labelmap_it (labelmap_itk, rg);
        if (have_mask) {
            mask_it.GoToBegin();
        }
        for (labelmap_it.GoToBegin(); !labelmap_it.IsAtEnd(); ++labelmap_it) {
            if (have_mask) {
                unsigned char m = (unsigned char) mask_it.Get();
                ++mask_it;
                if (!m) {
                    continue;
                }
            }
            unsigned char v = (unsigned char) labelmap_it.Get();
            fprintf (current, "%d %s\n", 
                v == 0 ? -1 : 1, vw_format ? "|" : "");
        }
    }
    else if (this->append_filename != "") {
        lprintf ("Processing append\n");
        FILE *app_fp = fopen (this->append_filename.c_str(), "r");
        rewind (current);
        /* Do the copy */
        while ((chars_in_buf = fread (buf, 1, BUFSIZE, app_fp)) != 0) {
            fwrite (buf, 1, chars_in_buf, current);
        }
        fclose (app_fp);
        /* Re-open input, and get the highest index.  Only needed for libsvm format. */
        if (!vw_format) {
            std::string line;
            std::ifstream app_fs (this->append_filename.c_str());
            std::getline (app_fs, line);
            app_fs.close ();
            std::vector<std::string> tokens = string_split (line, ' ');
            if (!tokens.empty()) {
                float junk;
                int rc = sscanf (tokens.back().c_str(), "%d:%f", &idx, &junk);
                if (rc != 2) {
                    idx = 0;
                }
            }
        }
    }
    
    /* Compile a complete list of feature input files */
    std::list<std::string> all_feature_files;
    std::list<std::string>::iterator fpath_it;
    for (fpath_it = this->feature_dir.begin();
         fpath_it != this->feature_dir.end();
         fpath_it++)
    {
        if (is_directory(*fpath_it)) {
            Dir_list dir_list (*fpath_it);
            for (int i = 0; i < dir_list.num_entries; i++) {
                /* Skip directories */
                std::string dir_entry = dir_list.entry (i);
                if (is_directory(dir_entry)) {
                    continue;
                }
                all_feature_files.push_back (dir_entry);
            }
        }
        else {
            all_feature_files.push_back (*fpath_it);
        }
    }
    
    /* Loop through feature files */
    for (fpath_it = all_feature_files.begin();
         fpath_it != all_feature_files.end();
         fpath_it++)
    {
        std::string dir_entry = *fpath_it;

        /* Load a feature image */
        Plm_image::Pointer feature = Plm_image::New (dir_entry);
        if (!feature->have_image()) {
            continue;
        }
        FloatImageType::Pointer feature_itk = feature->itk_float();

        /* Set up input and output file */
        lprintf ("Processing %s\n", dir_entry.c_str());
        if (current == fp[0]) {
            previous = fp[0];
            current = fp[1];
        } else {
            previous = fp[1];
            current = fp[0];
        }
        rewind (previous);
        rewind (current);
        
        /* Loop through pixels, appending them to each line of file */
        buf_ptr = 0;
        itk::ImageRegionIterator< FloatImageType > feature_it (
            feature_itk, feature_itk->GetLargestPossibleRegion ());
        if (have_mask) {
            mask_it.GoToBegin();
        }
        for (feature_it.GoToBegin(); !feature_it.IsAtEnd(); ++feature_it) {

            /* Check mask */
            if (have_mask) {
                unsigned char m = (unsigned char) mask_it.Get();
                ++mask_it;
                if (!m) {
                    continue;
                }
            }

            /* Get pixel value */
            float v = (float) feature_it.Get();
            
            /* Copy existing line from previous file into current file */
            bool eol_found = false;
            while (1) {
                if (chars_in_buf == 0) {
                    chars_in_buf = fread (buf, 1, BUFSIZE, previous);
                    if (chars_in_buf == 0) {
                        break;
                    }
                    buf_ptr = buf;
                }
                size_t write_size = 0;
                for (write_size = 0; write_size < chars_in_buf; write_size++) {
                    if (buf_ptr[write_size] == '\n') {
                        eol_found = true;
                        break;
                    }
                }
                fwrite (buf_ptr, 1, write_size, current);
                buf_ptr += write_size;
                chars_in_buf -= write_size;
                if (eol_found) {
                    buf_ptr += 1;
                    chars_in_buf -= 1;
                    break;
                }
            }

            /* Append new value */
            if (vw_format) {
                fprintf (current, " %s:%f\n", dir_entry.c_str(), v);
            } else {
                fprintf (current, " %d:%f\n", idx+1, v);
            }
        }
        idx ++;
    }

    /* Finally, re-write temp file into final output file */
    lprintf ("Processing final output\n");
    rewind (current);
    FILE *final_output = plm_fopen (this->output_filename.c_str(), "wb");
    while ((chars_in_buf = fread (buf, 1, BUFSIZE, current)) != 0) {
        fwrite (buf, 1, chars_in_buf, final_output);
    }
    fclose (final_output);
    
    fclose (fp[0]);
    fclose (fp[1]);
    printf ("Time = %f\n", (float) pli.report());
}

template<>
unsigned char
Ml_convert_private::choose_value<unsigned char> (
    float value
)
{
    if (value <= 0) {
        return 0;
    }
    else {
        return 1;
    }
}

template<>
float
Ml_convert_private::choose_value<float> (
    float value
)
{
    return value;
}

template<class T>
void
Ml_convert_private::image_from_ml_internal ()
{
    Plm_image::Pointer mask;
    bool have_mask = false;
    if (this->mask_filename != "") {
        have_mask = true;
        mask = Plm_image::New (this->mask_filename);
    }
    else if (this->label_filename != "") {
        have_mask = false;
        mask = Plm_image::New (this->label_filename);
    }
    else {
        print_and_exit ("Sorry, could not convert ml text file to image "
            "without knowing the image size");
    }

    typedef typename itk::Image<T,3> ImageType;
    typename ImageType::Pointer output_image = itk_image_create<T> (
        Plm_image_header (mask)
    );

    std::ifstream fp (this->input_ml_results_filename.c_str());
    
    itk::ImageRegionIterator< ImageType > out_it (output_image, 
        output_image->GetLargestPossibleRegion());
    itk::ImageRegionIterator< UCharImageType > mask_it;
    if (have_mask) {
        mask_it = itk::ImageRegionIterator< UCharImageType > (
            mask->itk_uchar(),
            mask->itk_uchar()->GetLargestPossibleRegion());
        mask_it.GoToBegin();
    }
    for (out_it.GoToBegin(); !out_it.IsAtEnd(); ++out_it) {
        if (have_mask) {
            unsigned char mask_value = mask_it.Get();
            ++mask_it;
            if (mask_value == 0) {
                out_it.Set (0);
                continue;
            }
        }

        std::string line;
        if (!std::getline (fp, line)) {
            print_and_exit ("Error, getline unexpected returned false during ml text read.\n");
        }
        float value;
        int rc = sscanf (line.c_str(), "%f", &value);
        if (rc != 1) {
            out_it.Set ((T) 0);
        }
        else {
            out_it.Set (this->choose_value<T> (value));
        }
    }
        
    itk_image_save (output_image, this->output_filename);
}

void
Ml_convert_private::image_from_ml ()
{
    switch (this->plm_image_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
    case PLM_IMG_TYPE_GPUIT_UCHAR:
        this->image_from_ml_internal<unsigned char> ();
        break;
    case PLM_IMG_TYPE_ITK_FLOAT:
    case PLM_IMG_TYPE_GPUIT_FLOAT:
        this->image_from_ml_internal<float> ();
        break;
    default:
        print_and_exit ("Warning: unimplemented image type in image_from_ml()\n");
        this->image_from_ml_internal<float> ();
        break;
    }
}

Ml_convert::Ml_convert ()
{
    d_ptr = new Ml_convert_private;
}

Ml_convert::~Ml_convert ()
{
    delete d_ptr;
}

void
Ml_convert::set_append_filename (const std::string& append_filename)
{
    d_ptr->append_filename = append_filename;
}

void
Ml_convert::set_input_ml_results_filename (
    const std::string& input_ml_results_filename)
{
    d_ptr->input_ml_results_filename = input_ml_results_filename;
}

void
Ml_convert::set_label_filename (const std::string& label_filename)
{
    d_ptr->label_filename = label_filename;
}

void
Ml_convert::set_mask_filename (const std::string& mask_filename)
{
    d_ptr->mask_filename = mask_filename;
}

void
Ml_convert::set_output_filename (const std::string& output_filename)
{
    d_ptr->output_filename = output_filename;
}

void
Ml_convert::set_output_format (const std::string& output_format)
{
    d_ptr->output_format = output_format;
}

void
Ml_convert::set_output_type (const std::string& output_type)
{
    d_ptr->plm_image_type = plm_image_type_parse (output_type.c_str());
}

void
Ml_convert::add_feature_path (const std::string& feature_path)
{
    d_ptr->feature_dir.push_back (feature_path);
}

void
Ml_convert::run ()
{
    if (d_ptr->input_ml_results_filename != "") {
        d_ptr->image_from_ml ();
    } else {
        d_ptr->ml_from_image ();
    }
}
