/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */

/* A simple test kernel */
__kernel void 
kernel_1 (
    __global  unsigned int * output,
    __global  unsigned int * input,
    const     unsigned int multiplier
)
{
    uint tid = get_global_id(0);
    
    output[tid] = input[tid] * multiplier;
}
