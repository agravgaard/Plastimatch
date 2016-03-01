/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */

// JAS 2010.11.04
// nvcc has the nasty limitation of not being able to
// use functions from other object files.  So in order
// to share __device__ functions amongst multiple .cu
// files, we resort to writing them in this somewhat
// kludgy "inc" file.

// Add the line:
//   #include "cuda_kernel_util.inc" 
//
// to the bottom of your CUDA .cu file to use these
// helper __device__ functions.

// ----------------------

// Note: n is the # of elements (NOT size_t)
template <typename T>
__device__ inline void
shared_memset (
    T* s,
    T c,
    int n
)
{
    int b = (n + block_size - 1) / block_size;

    for (int i=0; i<b; i++) {
        if ( (thread_idx_local + i*block_size) < n ) {
            s[thread_idx_local + i*block_size] = c;
        }
    }
}

// JAS 2010.11.11
// You can use this hack to get floating point atomicAdd for devices with
// compute less than 2.0.  However, note that atomicExch() can be performed on
// both global and shared memory on a 1.2 device, but a 1.1 device can only do
// atomicExch() on global memory.  So, be careful.  If you use this function to
// operate on shared memory, surround the call with an #if __CUDA_ARCH__ >= 120
__device__ inline void
atomic_add_float (
    float* addr,
    float val
)
{
#if __CUDA_ARCH__ >= 200
    atomicAdd (addr, val);
#elif __CUDA_ARCH__ >= 110 && __CUDA_ARCH__ < 200
    float old;
    bool success = false;
    while (!success) {
        old = atomicExch(addr, -1.0f);
        if (old != -1.0f) {
            success = true;
            old += val;
            atomicExch(addr, old);
        }
        __threadfence();
    }
#else
    /* cannot be done on old CUDA devices
     * this is just here to make the compiler happy */
    ;
#endif
}


// JAS 2010.11.13
// This is a useful function for copying the contents of shared memory to
// global memory.  This sort of thing is incredibly useful because you can
// organize your output within shared memory (which has no coalescence
// requirements) and then copy the ordered output to global memory as a
// series of blocks.  The global memory output will be the contents of each
// thread block's shared memory -- one after another.
template <typename T>
__device__ inline void
stog_memcpy (
    T* global,
    T* shared,
    int set_size
)
{
    int idx;
    int chunks = (set_size + block_size - 1) / block_size;
    int offset = set_size * block_idx;
    for (int i=0; i<chunks; i++) {
        idx = threadIdx.x + i*block_size;
        if (idx < set_size) {
            global[offset + idx] = shared[idx];
        }
    }
}



// Tell vim to use CUDA syntax highlighting
// vim:ft=cuda:
