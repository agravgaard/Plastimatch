/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rasterizer_h_
#define _rasterizer_h_

#include "plmbase_config.h"

#include "itk_image_type.h"

class Rtss;
class Plm_image_header;

class PLMBASE_API Rasterizer {
  public:
    Rasterizer ();
    ~Rasterizer ();
  public:
    bool want_prefix_imgs;
    bool want_labelmap;
    bool want_ss_img;

    /* The default behavior is to "or" overlapping contours.  Set 
       this member to true if you want to "xor" instead */
    bool xor_overlapping;

    float origin[3];
    float spacing[3];
    plm_long dim[3];

    unsigned char* acc_img;
    Volume* uchar_vol;
    Volume* labelmap_vol;

    Plm_image* m_ss_img;
    bool m_use_ss_img_vec;

    size_t curr_struct_no;
    int curr_bit;

  public:
    void rasterize (
	Rtss *cxt,
	Plm_image_header *pih,
	bool want_prefix_imgs,
	bool want_labelmap,
	bool want_ss_img,
	bool use_ss_img_vec, 
	bool xor_overlapping
    );
  private:
    void init (
	Rtss *cxt,            /* Input */
	Plm_image_header *pih,             /* Input */
	bool want_prefix_imgs,             /* Input */
	bool want_labelmap,                /* Input */
	bool want_ss_img,                  /* Input */
        bool use_ss_img_vec,               /* Input */
	bool xor_overlapping               /* Input */
    );
    bool process_next (
	Rtss *cxt                          /* In/out */
    );
    const char* current_name (
	Rtss *cxt
    );
};

#endif
