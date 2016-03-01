/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _mabs_stats_h_
#define _mabs_stats_h_

#include "plmsegment_config.h"
#include <string>
#include "itk_image.h"

class Mabs_stats_private;

class PLMSEGMENT_API Mabs_stats {
public:
    Mabs_stats ();
    ~Mabs_stats ();
public:
    Mabs_stats_private *d_ptr;
public:
    void set_distance_map_algorithm (
        const std::string& dmap_alg);
    std::string compute_statistics (
        const std::string& score_id,
        const UCharImageType::Pointer& ref_img,
        const UCharImageType::Pointer& cmp_img);
    std::string choose_best ();

    double get_time_dice ();
    double get_time_hausdorff ();
};

#endif
