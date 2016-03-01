/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsegment_config.h"
#include <list>
#include <stdio.h>
#include <stdlib.h>

#include "dice_statistics.h"
#include "hausdorff_distance.h"
#include "logfile.h"
#include "mabs_stats.h"
#include "plm_timer.h"
#include "string_util.h"

class Mabs_stats_private {
public:
    /* Keep track of score, for choosing best parameters */
    std::map<std::string, double> score_map;

    /* Which distance map algorithm to use */
    std::string dmap_alg;

    /* Timing statistics */
    double time_dice;
    double time_hausdorff;
public:
    Mabs_stats_private () {
        this->dmap_alg = "";
        this->time_dice = 0;
        this->time_hausdorff = 0;
    }
};

Mabs_stats::Mabs_stats ()
{
    this->d_ptr = new Mabs_stats_private;
}

Mabs_stats::~Mabs_stats ()
{
    delete this->d_ptr;
}

void
Mabs_stats::set_distance_map_algorithm (
    const std::string& dmap_alg)
{
    d_ptr->dmap_alg = dmap_alg;
}

std::string
Mabs_stats::compute_statistics (
    const std::string& score_id,
    const UCharImageType::Pointer& ref_img,
    const UCharImageType::Pointer& cmp_img)
{
    Plm_timer timer;
    timer.start();
    lprintf ("Computing Dice...\n");
    Dice_statistics dice;
    dice.set_reference_image (ref_img);
    dice.set_compare_image (cmp_img);
    dice.run ();
    d_ptr->time_dice += timer.report();

    timer.start();
    lprintf ("Computing Hausdorff (alg=%s)\n", 
        d_ptr->dmap_alg == "" ? "default" : d_ptr->dmap_alg.c_str());
    Hausdorff_distance hd;
    hd.set_reference_image (ref_img);
    hd.set_compare_image (cmp_img);
    hd.set_distance_map_algorithm (d_ptr->dmap_alg);
    /* GCS FIX: This should not be hard-coded */
    hd.set_maximum_distance (500);
    hd.run ();
    d_ptr->time_hausdorff += timer.report();

    /* Update registration statistics -- these are used 
       to keep track of which registration 
       approach is the best */
    std::map<std::string, double>::const_iterator score_it 
        = d_ptr->score_map.find (score_id);
    if (score_it == d_ptr->score_map.end()) {
        d_ptr->score_map[score_id] = dice.get_dice();
    } else {
        d_ptr->score_map[score_id] += dice.get_dice();
    }

    std::string stats_string = string_format (
        "dice=%f,tp=%d,tn=%d,fp=%d,fn=%d,"
        "hd=%f,95hd=%f,ahd=%f,"
        "bhd=%f,95bhd=%f,abhd=%f",
        dice.get_dice(),
        (int) dice.get_true_positives(),
        (int) dice.get_true_negatives(),
        (int) dice.get_false_positives(),
        (int) dice.get_false_negatives(),
        hd.get_hausdorff(),
        hd.get_percent_hausdorff(),
        hd.get_avg_average_hausdorff(),
        hd.get_boundary_hausdorff(),
        hd.get_percent_boundary_hausdorff(),
        hd.get_avg_average_boundary_hausdorff()
    );

    return stats_string;
}

std::string
Mabs_stats::choose_best ()
{
    /* Select best pre-alignment result */
    std::map<std::string, double>::const_iterator score_it;
    score_it = d_ptr->score_map.begin();
    double best_score = 0;
    std::string best_registration_name = "";
    while (score_it != d_ptr->score_map.end()) {
        std::string registration_name = score_it->first;
        double this_score = d_ptr->score_map[registration_name];
        printf ("Recovered score: %s, %f\n", registration_name.c_str(),
            (float) this_score);
        if (this_score > best_score) {
            best_score = this_score;
            best_registration_name = registration_name;
        }
        score_it ++;
    }

    return best_registration_name;
}

double
Mabs_stats::get_time_dice ()
{
    return d_ptr->time_dice;
}

double
Mabs_stats::get_time_hausdorff ()
{
    return d_ptr->time_hausdorff;
}
