/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _proj_image_h_
#define _proj_image_h_

#include "plmbase_config.h"
#include <string>

class Proj_image;
class Proj_matrix;

class PLMBASE_API Proj_image {
public:
    Proj_image (void);
    Proj_image (const char* img_filename, const char* mat_filename);
    Proj_image (const std::string& img_filename, 
        const std::string& mat_filename = "");
    Proj_image (const char* img_filename, const double xy_offset[2]);
    ~Proj_image (void);

public:
    int dim[2];              /* dim[0] = cols, dim[1] = rows */
    double xy_offset[2];     /* Offset of center pixel */
    Proj_matrix *pmat;       /* Geometry of panel and source */
    float* img;		     /* Pixel data */

public:
    void clear ();
    bool have_image ();
    void init ();
    void save (const char *img_filename, const char *mat_filename);
    void load (const std::string& img_filename, std::string mat_filename = "");
    void load_pfm (const char* img_filename, const char* mat_filename);
    void load_raw (const char* img_filename, const char* mat_filename);
    void load_hnd (const char* img_filename);
    void set_xy_offset (const double xy_offset[2]);

    void debug_header ();
    void stats ();
};

PLMBASE_C_API void proj_image_create_pmat (Proj_image *proj);
PLMBASE_C_API void proj_image_create_img (Proj_image *proj, int dim[2]);

#endif
