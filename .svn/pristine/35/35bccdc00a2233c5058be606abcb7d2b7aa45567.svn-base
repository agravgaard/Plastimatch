/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __double_align8_h__
#define __double_align8_h__

/* JAS 2011.07.23
 * The following is a fix that allows us to more selectively enforce
 * the -malign-double compatibility required by object files compiled
 * by nvcc.  Any structures that are used by both nvcc compiled files
 * and gcc/g++ compiled files should use this.  The reason we do not
 * simply pass -malign-double to gcc/g++ in order to achieve this
 * compatibility is because Slicer's ITK does not come compiled with
 * the -malign-double flag on 32-bit systems... so, believe it or not
 * this might be the cleanest solution */
#if (__GNUC__) && (MACHINE_IS_32_BIT) && (CUDA_FOUND)
    typedef double double_align8 __attribute__ ((aligned(8)));
#else 
    typedef double double_align8;
#endif

#endif
