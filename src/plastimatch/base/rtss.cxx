/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <limits>
#include <set>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "logfile.h"
#include "plm_image_header.h"
#include "plm_int.h"
#include "plm_math.h"
#include "rtss_contour.h"
#include "rtss_roi.h"
#include "rtss.h"
#include "rt_study_metadata.h"
#include "slice_list.h"
#include "string_util.h"

static std::string
assign_random_color ()
{
    static int idx = 0;
    static const char* colors[] = {
	"255 0 0",
	"255 255 0",
	"255 0 255",
	"0 255 255",
	"0 255 0",
	"0 0 255",
	"255 128 128",
	"255 255 128",
	"255 128 255",
	"128 255 255",
	"128 255 128",
	"128 128 255",
	"200 128 128",
	"200 200 128",
	"200 128 200",
	"128 200 200",
	"128 200 128",
	"128 128 200",
	"200 255 255",
	"200 200 255",
	"200 255 200",
	"255 200 200",
	"255 200 255",
	"255 255 200",
    };
    std::string color = std::string (colors[idx]);
    if (++idx > 23) {
	idx = 0;
    }
    return color;
}

#define SPACING_TOL 0.2    /* How close you need to be to be on the slice */

Rtss::Rtss ()
{
    this->init ();
}

Rtss::~Rtss ()
{
    this->clear ();
}

void
Rtss::init (void)
{
    this->have_geometry = 0;
    this->num_structures = 0;
    this->slist = 0;
}

void
Rtss::clear (void)
{
    for (size_t i = 0; i < this->num_structures; i++) {
	delete (this->slist[i]);
    }
    free (this->slist);

    this->init ();
}

std::string
Rtss::find_unused_structure_name (void)
{
    std::string test_name;
    for (int n = 1; n < std::numeric_limits<int>::max(); ++n) {
	bool dup_found = false;
        test_name = string_format ("%s (%d)", "Unknown structure", n);
	for (size_t i = 0; i < this->num_structures; ++i) {
	    Rtss_roi* curr_structure = this->slist[i];
	    if (test_name == curr_structure->name) {
		dup_found = true;
		break;
	    }
	}
	if (!dup_found) {
	    break;
	}
    }

    return test_name;
}

/* Add structure (if it doesn't already exist) */
Rtss_roi*
Rtss::add_structure (
    const std::string& structure_name, 
    const std::string& color, 
    int structure_id,
    int bit)
{
    Rtss_roi* new_structure;

    new_structure = this->find_structure_by_id (structure_id);
    if (new_structure) {
	return new_structure;
    }

    this->num_structures++;
    this->slist = (Rtss_roi**) 
        realloc (this->slist, 
            this->num_structures * sizeof(Rtss_roi*));
    new_structure 
	= this->slist[this->num_structures - 1] 
	= new Rtss_roi;

    new_structure->name = structure_name;
    if (structure_name == "" || structure_name == "Unknown structure") {
	new_structure->name = find_unused_structure_name ();
    }
    new_structure->name = string_trim (new_structure->name);
    new_structure->id = structure_id;
    new_structure->bit = bit;
    if (color != "") {
	new_structure->color = color;
    } else {
	new_structure->color = assign_random_color ();
    }
    new_structure->num_contours = 0;
    new_structure->pslist = 0;
    return new_structure;
}

void
Rtss::delete_structure (int index)
{
    Rtss_roi* curr_structure = this->slist[index];
    delete curr_structure;

    /* Remark: the below two lines are correct but redundant if 
       (index == this->num_structures-1), but this comment to explain 
       it is not worse than adding if statement. */
    this->slist[index] = this->slist[this->num_structures-1];
    this->num_structures --;
}

Rtss_roi*
Rtss::find_structure_by_id (int structure_id)
{
    for (size_t i = 0; i < this->num_structures; i++) {
	Rtss_roi* curr_structure;
	curr_structure = this->slist[i];
	if (curr_structure->id == structure_id) {
	    return curr_structure;
	}
    }
    return 0;
}

void 
Rtss::set_structure_name (size_t index, const std::string& name)
{
    if (index < this->num_structures) {
        this->slist[index]->name = name.c_str();
    }
}

std::string
Rtss::get_structure_name (size_t index)
{
    if (index < this->num_structures) {
        return std::string (this->slist[index]->name.c_str());
    } else {
        return "";
    }
}

void
Rtss::debug (void)
{
    Rtss_roi* curr_structure;

    if (this->have_geometry) {
	lprintf ("rps::dim = %u %u %u\n", 
	    (unsigned int) this->m_dim[0], 
	    (unsigned int) this->m_dim[1], 
	    (unsigned int) this->m_dim[2]);
	lprintf ("rps::offset = %g %g %g\n", 
	    this->m_offset[0], this->m_offset[1], this->m_offset[2]);
	lprintf ("rps::spacing = %g %g %g\n", 
	    this->m_spacing[0], this->m_spacing[1], this->m_spacing[2]);
    } else {
	lprintf ("rps has no geometry\n");
    }

    for (size_t i = 0; i < this->num_structures; i++) {
        curr_structure = this->slist[i];
	lprintf ("%u %d %s [%s] (%p) (%d contours)", 
	    (unsigned int) i, 
	    curr_structure->id, 
	    curr_structure->name.c_str(), 
	    (curr_structure->color.empty() 
                ? "none" : curr_structure->color.c_str()), 
	    curr_structure->pslist, 
	    (int) curr_structure->num_contours
	);
	if (curr_structure->num_contours) {
	    if (curr_structure->pslist[0]->num_vertices) {
		lprintf (" [%f,%f,%f,...]",
		    curr_structure->pslist[0]->x[0],
		    curr_structure->pslist[0]->y[0],
		    curr_structure->pslist[0]->z[0]);
	    } else {
		lprintf (" <no vertices>");
	    }
	}
	lprintf ("\n");
    }
}

void
Rtss::adjust_structure_names (void)
{
    Rtss_roi* curr_structure;

    for (size_t i = 0; i < this->num_structures; i++) {
        curr_structure = this->slist[i];
	bool changed = false;
        std::string tmp = curr_structure->name;
	for (int j = 0; j < curr_structure->name.length(); j++) {
	    /* GE Adv sim doesn't like names with strange punctuation. */
	    if (! isalnum (curr_structure->name[j])) {
		curr_structure->name[j] = '_';
		changed = true;
	    }
	}
    }
}

void
Rtss::prune_empty (void)
{
    for (size_t i = 0; i < this->num_structures; i++) {
	Rtss_roi* curr_structure;
	curr_structure = this->slist[i];
	if (curr_structure->num_contours == 0) {
	    delete curr_structure;
	    /* Remark: the below two lines are correct but redundant if 
	       (i == this->num_structures-1), but this comment to explain 
	       it is not worse than adding if statement. */
	    this->slist[i] = this->slist[this->num_structures-1];
	    i --;
	    this->num_structures --;
	}
    }
}

/* Copy structure name, id, color, but not contents */
Rtss*
Rtss::clone_empty (
    Rtss* cxt_out,
    Rtss* cxt_in
)
{
    /* Initialize output cxt */
    if (cxt_out) {
	cxt_out->clear ();
    } else {
	cxt_out = new Rtss;
    }

    for (size_t i = 0; i < cxt_in->num_structures; i++) {
	Rtss_roi *old_structure = cxt_in->slist[i];
	Rtss_roi *new_structure = cxt_out->add_structure (
	    old_structure->name, old_structure->color, old_structure->id);

	/* Copy bit */
	new_structure->bit = old_structure->bit;
    }
    return cxt_out;
}

/* Clear the polylines, but keep structure name, id, color */
void
Rtss::free_all_polylines (void)
{
    for (size_t i = 0; i < this->num_structures; i++) {
	Rtss_roi *curr_structure = this->slist[i];
	for (size_t j = 0; j < curr_structure->num_contours; j++) {
	    delete curr_structure->pslist[j];
	}
	free (curr_structure->pslist);

	curr_structure->num_contours = 0;
	curr_structure->pslist = 0;
    }
}

void
Rtss::find_rasterization_geometry (
    float offset[3],
    float spacing[3],
    plm_long dims[3],
    Direction_cosines& dc
)
{
    int first = 1;
    float min_x = 0.f, max_x = 0.f;
    float min_y = 0.f, max_y = 0.f;
    float min_z = 0.f, max_z = 0.f;
    std::set<float> z_values;

    /* GCS TODO: Here is where the direction cosine detector goes */
#if defined (commentout)
    for (size_t i = 0; i < this->num_structures; i++) {
	Rtss_roi *curr_structure = this->slist[i];
	for (size_t j = 0; j < curr_structure->num_contours; j++) {
	    Rtss_contour *curr_polyline = curr_structure->pslist[j];
            curr_polyline->find_direction_cosines ();
            continue;
        }
    }
#endif

    /* Scan points to find image size, spacing */
    for (size_t i = 0; i < this->num_structures; i++) {
	Rtss_roi *curr_structure = this->slist[i];
	for (size_t j = 0; j < curr_structure->num_contours; j++) {
	    Rtss_contour *curr_polyline = curr_structure->pslist[j];
	    for (int k = 0; k < curr_polyline->num_vertices; k++) {
		z_values.insert (curr_polyline->z[k]);
		if (first) {
		    min_x = max_x = curr_polyline->x[k];
		    min_y = max_y = curr_polyline->y[k];
		    min_z = max_z = curr_polyline->z[k];
		    first = 0;
		    continue;
		}
		if (curr_polyline->x[k] < min_x) {
		    min_x = curr_polyline->x[k];
		} else if (curr_polyline->x[k] > max_x) {
		    max_x = curr_polyline->x[k];
		}
		if (curr_polyline->y[k] < min_y) {
		    min_y = curr_polyline->y[k];
		} else if (curr_polyline->y[k] > max_y) {
		    max_y = curr_polyline->y[k];
		}
		if (curr_polyline->z[k] < min_z) {
		    min_z = curr_polyline->z[k];
		} else if (curr_polyline->z[k] > max_z) {
		    max_z = curr_polyline->z[k];
		}
	    }
	}
    }

    /* Use heuristics to set (x,y) values */
    float range_x = max_x - min_x;
    float range_y = max_y - min_y;
    float range = range_x;
    if (range_y > range) {
	range = range_y;
    }
    range = range * 1.05;
    spacing[0] = spacing[1] = range / 512;
    offset[0] = 0.5 * (max_x + min_x - range);
    offset[1] = 0.5 * (max_y + min_y - range);
    dims[0] = dims[1] = 512;

#if defined (commentout)
    printf ("----Z VALUES-----\n");
    for (std::set<float>::iterator it = z_values.begin(); 
	 it != z_values.end(); 
	 it++)
    {
	printf ("%f\n", *it);
    }
    printf ("---------\n");
#endif

    /* z value should be based on native slice spacing */
    int have_spacing = 0;
    float z_spacing = 0.f;
    float last_z = min_z;
    for (std::set<float>::iterator it = z_values.begin(); 
	 it != z_values.end(); 
	 it++)
    {
	float this_z = *it;
	float diff = this_z - last_z;
	if (fabs (diff) < SPACING_TOL) {
	    continue;
	}
	if (!have_spacing) {
	    z_spacing = this_z - min_z;
	    have_spacing = 1;
	} else {
	    if (fabs (diff - z_spacing) > SPACING_TOL) {
		lprintf ("Warning, slice spacing of RTSS may be unequal\n");
		lprintf ("%g - %g = %g vs. %g\n", 
		    this_z, last_z, diff, z_spacing);
	    }
	}
	last_z = this_z;
    }
    
    offset[2] = min_z;
    if (have_spacing) {
	dims[2] = ROUND_INT ((max_z - min_z) / z_spacing);
	spacing[2] = z_spacing;
    } else {
	dims[2] = 1;
	spacing[2] = 1;
    }
}

void
Rtss::find_rasterization_geometry (Plm_image_header *pih)
{
    /* use some generic default parameters */
    plm_long dim[3];
    float origin[3];
    float spacing[3];
    Direction_cosines dc;

    this->find_rasterization_geometry (origin, spacing, dim, dc);

    pih->set_from_gpuit (dim, origin, spacing, dc);
}

void
Rtss::set_rasterization_geometry (void)
{
    this->find_rasterization_geometry (
	this->rast_offset,
	this->rast_spacing,
	this->rast_dim,
        this->rast_dc
    );
    lprintf ("rast_dim = %u %u %u\n", 
	(unsigned int) this->rast_dim[0], 
	(unsigned int) this->rast_dim[1], 
	(unsigned int) this->rast_dim[2]);
    lprintf ("rast_offset = %g %g %g\n", 
	this->rast_offset[0], this->rast_offset[1], this->rast_offset[2]);
    lprintf ("rast_spacing = %g %g %g\n", 
	this->rast_spacing[0], this->rast_spacing[1], this->rast_spacing[2]);
}

void
Rtss::apply_slice_list (const Rt_study_metadata::Pointer& rsm)
{
    this->apply_slice_list (rsm->get_slice_list());
}

void
Rtss::apply_slice_list (const Slice_list *slice_list)
{
    if (!slice_list->slice_list_complete()) {
        return;
    }

    const Plm_image_header *pih = slice_list->get_image_header ();
    /* Geometry */
    pih->get_dim (this->m_dim);
    pih->get_origin (this->m_offset);
    pih->get_spacing (this->m_spacing);

    /* Slice numbers and slice uids */
    for (size_t i = 0; i < this->num_structures; i++) {
        Rtss_roi *curr_structure = this->slist[i];
        for (size_t j = 0; j < curr_structure->num_contours; j++) {
            Rtss_contour *curr_polyline = curr_structure->pslist[j];
            if (curr_polyline->num_vertices <= 0) {
                continue;
            }
            curr_polyline->slice_no = slice_list->get_slice_index (
                curr_polyline->z[0]);
            curr_polyline->ct_slice_uid = slice_list->get_slice_uid (
                curr_polyline->slice_no);
#if defined (commentout)
            rdd->get_slice_info (
                &curr_polyline->slice_no,
                &curr_polyline->ct_slice_uid,
                curr_polyline->z[0]);
#endif
        }
    }
}

void
Rtss::fix_polyline_slice_numbers (void)
{
    if (!this->have_geometry) return;

    for (size_t i = 0; i < this->num_structures; i++) {
	Rtss_roi *curr_structure = this->slist[i];
	for (size_t j = 0; j < curr_structure->num_contours; j++) {
	    Rtss_contour *curr_polyline = curr_structure->pslist[j];
	    if (curr_polyline->num_vertices == 0) {
		curr_polyline->slice_no = -1;
		continue;
	    }
	    float z = curr_polyline->z[0];
	    int slice_idx = ROUND_INT (
		(z - this->m_offset[2]) / this->m_spacing[2]);
	    if (slice_idx < 0 || slice_idx >= this->m_dim[2]) {
		curr_polyline->slice_no = -1;
	    } else {
		curr_polyline->slice_no = slice_idx;
	    }
	}
    }
}

void
Rtss::set_geometry (
    const Plm_image_header *pih
)
{
    pih->get_origin (this->m_offset);
    pih->get_spacing (this->m_spacing);
    pih->get_dim (this->m_dim);
    this->have_geometry = 1;

    this->fix_polyline_slice_numbers ();
}

void
Rtss::set_geometry (
    const Plm_image::Pointer& pli
)
{
    Plm_image_header pih;
    pih.set_from_plm_image (pli);
    this->set_geometry (&pih);
}

void
Rtss::keyholize (void)
{
#if defined (PLM_CONFIG_KEYHOLIZE)
    lprintf ("Keyholizing...\n");

    /* Loop through structures */
    for (int i = 0; i < this->num_structures; i++) {
	Rtss_roi *curr_structure = this->slist[i];

	/* Find groups of contours which lie on the same slice */
	std::vector<bool> used_contours;
	used_contours.assign (curr_structure->num_contours, false);

	for (int j = 0; j < curr_structure->num_contours; j++) {
	    std::vector<int> group_contours;
	    Rtss_contour *group_polyline = curr_structure->pslist[j];
	    if (group_polyline->num_vertices == 0) {
		group_polyline->slice_no = -1;
		continue;
	    }
	    if (used_contours[j] == true) {
		continue;
	    }
	    float group_z = group_polyline->z[0];
	    group_contours.push_back (j);
	    for (int k = j+1; k < curr_structure->num_contours; k++) {
		Rtss_contour *curr_polyline = curr_structure->pslist[k];
		if (curr_polyline->num_vertices == 0) {
		    curr_polyline->slice_no = -1;
		    continue;
		}
		float curr_z = curr_polyline->z[0];
		if (curr_z - group_z < SPACING_TOL) {
		    used_contours[k] = true;
		    group_contours.push_back (k);
		}
	    }

	    /* We have now found a group */
	    lprintf ("Keyholizing group:");
	    for (unsigned int k = 0; k < group_contours.size(); k++) {
		lprintf (" %d", group_contours[k]);
	    }
	    lprintf ("\n");

	    /* Find an outermost contour in group */
	    int cidx_xmin = -1;
	    float xmin = FLT_MAX;
	    for (unsigned int k = 0; k < group_contours.size(); k++) {
		int cidx = group_contours[k];
		Rtss_contour *curr_polyline = curr_structure->pslist[cidx];

		float curr_xmin = FLT_MAX;
		for (int l = 0; l < curr_polyline->num_vertices; l++) {
		    if (curr_polyline->x[l] < curr_xmin) {
			curr_xmin = curr_polyline->x[l];
		    }
		}

		if (curr_xmin < xmin) {
		    cidx_xmin = cidx;
		    xmin = curr_xmin;
		}
	    }
	    
	    /* Loop through other contours, find contours contained 
	       in this contour */
	    for (unsigned int k = 0; k < group_contours.size(); k++) {
		int cidx = group_contours[k];
		Rtss_contour *curr_polyline = curr_structure->pslist[cidx];
		if (cidx == cidx_xmin) {
		    continue;
		}

		float x = curr_polyline->x[0];
		float y = curr_polyline->y[0];
		
	    }
	}
    }
#endif
}
