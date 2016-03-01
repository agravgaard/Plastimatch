/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "print_and_exit.h"
#include "volume.h"
#include "volume_stats.h"

/* -----------------------------------------------------------------------
   Statistics like min, max, etc.
   ----------------------------------------------------------------------- */
template<class T> 
void
volume_stats_template (const Volume *vol, double *min_val, double *max_val, 
    double *avg, int *non_zero, int *num_vox)
{
    int first = 1;
    double sum = 0.0;
    T *img = (T*) vol->img;

    *non_zero = 0;
    *num_vox = 0;

    for (plm_long i = 0; i < vol->npix; i++) {
	double v = (double) img[i];
	if (first) {
	    *min_val = *max_val = v;
	    first = 0;
	}
	if (*min_val > v) *min_val = v;
	if (*max_val < v) *max_val = v;
	sum += v;
	(*num_vox) ++;
	if (v != 0.0) {
	    (*non_zero) ++;
	}
    }
    *avg = sum / (*num_vox);
}

void
volume_stats (const Volume *vol, double *min_val, double *max_val, 
    double *avg, int *non_zero, int *num_vox)
{
    switch (vol->pix_type) {
    case PT_UCHAR:
        volume_stats_template<unsigned char> (
            vol, min_val, max_val, avg, non_zero, num_vox);
        break;
    case PT_SHORT:
        volume_stats_template<short> (
            vol, min_val, max_val, avg, non_zero, num_vox);
        break;
    case PT_FLOAT:
        volume_stats_template<float> (
            vol, min_val, max_val, avg, non_zero, num_vox);
        break;
    case PT_UINT16:
    case PT_UINT32:
    case PT_INT32:
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	print_and_exit (
            "Sorry, unsupported type %d for volume_stats()\n",
            vol->pix_type);
	break;
    }
}

void
volume_stats (const Volume::Pointer vol, double *min_val, double *max_val, 
    double *avg, int *non_zero, int *num_vox)
{
    const Volume *v = vol.get();
    volume_stats (v, min_val, max_val, avg, non_zero, num_vox);
}
