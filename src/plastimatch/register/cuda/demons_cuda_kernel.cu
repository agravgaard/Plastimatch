/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
// declare texture reference for nabla computation
texture<float4, 2, cudaReadModeElementType> tex;

// declare texture reference for filter computation
texture<float4, 1, cudaReadModeElementType> tex_filter;

// declare texture references for vector field estimation
texture<float, 1, cudaReadModeElementType> tex_nabla_x;
texture<float, 1, cudaReadModeElementType> tex_nabla_y;
texture<float, 1, cudaReadModeElementType> tex_nabla_z;
texture<float, 1, cudaReadModeElementType> tex_moving_image;
texture<float, 1, cudaReadModeElementType> tex_static_image;

////////////////////////////////////////////////////////////////////////////////
// Kernel Functions
////////////////////////////////////////////////////////////////////////////////


__global__ void k_initial_vector(float* idata, unsigned int size)
{
    unsigned int index = blockIdx.y*4096 + blockIdx.x*256 + threadIdx.y*16 + threadIdx.x;    

    if(index<size)
    {
        idata[index]=(float)0.0;
    }   
}

__global__ void k_initial_vector_nk(int* idata, unsigned int size)
{
    int index = blockIdx.y*4096 + blockIdx.x*256 + threadIdx.y*16 + threadIdx.x;    

    if(index<size)
    {
        idata[index]=0;
    }   
}

__global__ void k_convert(float* d_inx, float* d_iny,float* d_inz,float* d_outx, float* d_outy,float* d_outz,float spacingx,float spacingy,float spacingz, float3 dimen)
{
    // compute index values
    int blocks_in_y = (unsigned int)ceil((double)dimen.x/(double)blockDim.x);
    int x_index = ((unsigned int)blockIdx.x%blocks_in_y)*blockDim.x+threadIdx.x;
    int y_index = ((unsigned int)floor((double)blockIdx.x/blocks_in_y))*blockDim.y + threadIdx.y;
    int z_index = blockIdx.y*blockDim.z+threadIdx.z;
    int address = x_index + y_index*dimen.x + z_index*dimen.x*dimen.y;
    float tempx, tempy, tempz;

    if(x_index<dimen.x && y_index<dimen.y && z_index<dimen.z){
        tempx = (float)((int)ceil(d_inx[address]/spacingx));
        tempy = (float)((int)ceil(d_iny[address]/spacingy));
        tempz = (float)((int)ceil(d_inz[address]/spacingz));
    }

    __syncthreads();

    if(x_index<dimen.x && y_index<dimen.y && z_index<dimen.z){
        d_outx[address]=tempx;
        d_outy[address]=tempy;
        d_outz[address]=tempz;
    }

}


__global__ void compute_nabla_x(float* odata, float3 vol1_dim, int size, int tex_size,
                                int tex_blocks_per_x, float pix_space)
{
#if defined (commentout)
    // kernel assumes that volume dimension in x direction is divisible by 4
    // compute index values
    unsigned int blocks_in_y = (unsigned int)ceil((double)(vol1_dim.x/4)/(double)blockDim.x);
    unsigned int x_index = ((unsigned int)blockIdx.x%blocks_in_y)*blockDim.x+threadIdx.x;
    unsigned int y_index = ((unsigned int)floor((double)blockIdx.x/blocks_in_y))*blockDim.y + threadIdx.y;
    unsigned int z_index = blockIdx.y*blockDim.z+threadIdx.z;
    unsigned int linear_address= 4*x_index + (unsigned int)vol1_dim.x*y_index + (unsigned int)vol1_dim.x*(unsigned int)vol1_dim.y*z_index;

    float4 current;
    if(x_index<vol1_dim.x && y_index<vol1_dim.y && z_index<vol1_dim.z)
    {   
        // fetch data from texture
        unsigned int tex_address = linear_address/4;
        unsigned int current_x = tex_address%tex_size;
        unsigned int current_y = tex_address/tex_size;
        current = texfetch(tex,current_x,current_y);

        if(x_index==0)
        {
            // fetch data from texture
            unsigned int right_x = (tex_address+1)%tex_size;
            unsigned int right_y = (tex_address+1)/tex_size;
            float4 right = texfetch(tex,right_x,right_y);
            
            // write outward data
            odata[linear_address]=(current.y-current.x)/(2.0*pix_space);
            odata[linear_address+1]=(current.z-current.x)/(4.0*pix_space);
            odata[linear_address+2]=(current.w-current.y)/(4.0*pix_space);
            odata[linear_address+3]=(right.x-current.z)/(4.0*pix_space);
        }
        else if(x_index==vol1_dim.x-4)
        {
            // fetch data from texture
            unsigned int left_x = (tex_address-1)%tex_size;
            unsigned int left_y = (tex_address-1)/tex_size;
            float4 left = texfetch(tex,left_x,left_y);

            // write outward data
            odata[linear_address]=(current.y-left.w)/(4.0*pix_space);
            odata[linear_address+1]=(current.z-current.x)/(4.0*pix_space);
            odata[linear_address+2]=(current.w-current.y)/(4.0*pix_space);
            odata[linear_address+3]=(current.w-current.z)/(2.0*pix_space);
        }
        else
        {
            // fetch data from texture
            unsigned int right_x = (tex_address+1)%tex_size;
            unsigned int right_y = (tex_address+1)/tex_size;
            float4 right = texfetch(tex,right_x,right_y);

            unsigned int left_x = (tex_address-1)%tex_size;
            unsigned int left_y = (tex_address-1)/tex_size;
            float4 left = texfetch(tex,left_x,left_y);

            // write outward data
            odata[linear_address]=(current.y-left.w)/(4.0*pix_space);
            odata[linear_address+1]=(current.z-current.x)/(4.0*pix_space);
            odata[linear_address+2]=(current.w-current.y)/(4.0*pix_space);
            odata[linear_address+3]=(right.x-current.z)/(4.0*pix_space);
        }
    }
#endif
}   

__global__ void compute_nabla_y(float* odata, float3 vol1_dim, int size, int tex_size,
                                int tex_blocks_per_x, float pix_space)
{
#if defined (commentout)
    // compute index values
    unsigned int blocks_in_y = (unsigned int)ceil((double)(vol1_dim.x/4)/(double)blockDim.x);
    unsigned int x_index = ((unsigned int)blockIdx.x%blocks_in_y)*blockDim.x+threadIdx.x;
    unsigned int y_index = ((unsigned int)floor((double)blockIdx.x/blocks_in_y))*blockDim.y + threadIdx.y;
    unsigned int z_index = blockIdx.y*blockDim.z+threadIdx.z;
    unsigned int linear_address= 4*x_index + (unsigned int)vol1_dim.x*y_index + (unsigned int)vol1_dim.x*(unsigned int)vol1_dim.y*z_index;
    unsigned int tex_inc = vol1_dim.x/4;

    if(x_index<vol1_dim.x && y_index<vol1_dim.y && z_index<vol1_dim.z)
    {   
        unsigned int tex_address = linear_address/4;

        if(y_index==0)
        {
            // calculate address and fetch data
            unsigned int current_x = tex_address%tex_size;
            unsigned int current_y = tex_address/tex_size;
            float4 current = texfetch(tex,current_x,current_y);

            unsigned int right_x = (tex_address+tex_inc)%tex_size;
            unsigned int right_y = (tex_address+tex_inc)/tex_size;
            float4 right = texfetch(tex,right_x,right_y);
            
            // calculate and write
            odata[linear_address]=(right.x-current.x)/(2.0*pix_space);
            odata[linear_address+1]=(right.y-current.y)/(2.0*pix_space);
            odata[linear_address+2]=(right.z-current.z)/(2.0*pix_space);
            odata[linear_address+3]=(right.w-current.w)/(2.0*pix_space);
        }
        else if(y_index==vol1_dim.y-1)
        {
            // calculate address and fetch data
            unsigned int current_x = tex_address%tex_size;
            unsigned int current_y = tex_address/tex_size;
            float4 current = texfetch(tex,current_x,current_y);

            unsigned int left_x = (tex_address-tex_inc)%tex_size;
            unsigned int left_y = (tex_address-tex_inc)/tex_size;
            float4 left = texfetch(tex,left_x,left_y);
    
            // calculate and write
            odata[linear_address]=(current.x-left.x)/(2.0*pix_space);
            odata[linear_address+1]=(current.y-left.y)/(2.0*pix_space);
            odata[linear_address+2]=(current.z-left.z)/(2.0*pix_space);
            odata[linear_address+3]=(current.w-left.w)/(2.0*pix_space);
        }
        else
        {
            // calculate address and fetch data
            unsigned int right_x = (tex_address+tex_inc)%tex_size;
            unsigned int right_y = (tex_address+tex_inc)/tex_size;
            float4 right = texfetch(tex,right_x,right_y);

            unsigned int left_x = (tex_address-tex_inc)%tex_size;
            unsigned int left_y = (tex_address-tex_inc)/tex_size;
            float4 left = texfetch(tex,left_x,left_y);

            // calculate and write
            odata[linear_address]=(right.x-left.x)/(4.0*pix_space);
            odata[linear_address+1]=(right.y-left.y)/(4.0*pix_space);
            odata[linear_address+2]=(right.z-left.z)/(4.0*pix_space);
            odata[linear_address+3]=(right.w-left.w)/(4.0*pix_space);
        }
    }
#endif
}   

__global__ void compute_nabla_z(float* odata, float3 vol1_dim, int size, int tex_size,
                                int tex_blocks_per_x, float pix_space)
{
#if defined (commentout)
    // kernel assumes that volume dimension in x direction is divisible by 4
    // compute index values
    unsigned int blocks_in_y = (unsigned int)ceil((double)(vol1_dim.x/4)/(double)blockDim.x);
    unsigned int x_index = ((unsigned int)blockIdx.x%blocks_in_y)*blockDim.x+threadIdx.x;
    unsigned int y_index = ((unsigned int)floor((double)blockIdx.x/blocks_in_y))*blockDim.y + threadIdx.y;
    unsigned int z_index = blockIdx.y*blockDim.z+threadIdx.z;
    unsigned int linear_address= 4*x_index + (unsigned int)vol1_dim.x*y_index + (unsigned int)vol1_dim.x*(unsigned int)vol1_dim.y*z_index;
    unsigned int tex_inc = (unsigned int)vol1_dim.y*(unsigned int)(vol1_dim.x/4);

    if(x_index<vol1_dim.x && y_index<vol1_dim.y && z_index<vol1_dim.z)
    {   
        unsigned int tex_address = linear_address/4;

        if(z_index==0)
        {
            // calculate address and fetch data
            unsigned int current_x = tex_address%tex_size;
            unsigned int current_y = tex_address/tex_size;
            float4 current = texfetch(tex,current_x,current_y);

            unsigned int right_x = (tex_address+tex_inc)%tex_size;
            unsigned int right_y = (tex_address+tex_inc)/tex_size;
            float4 right = texfetch(tex,right_x,right_y);
            
            // calculate and write
            odata[linear_address]=(right.x-current.x)/(2.0*pix_space);
            odata[linear_address+1]=(right.y-current.y)/(2.0*pix_space);
            odata[linear_address+2]=(right.z-current.z)/(2.0*pix_space);
            odata[linear_address+3]=(right.w-current.w)/(2.0*pix_space);
        }
        else if(z_index==vol1_dim.z-1)
        {
            // calculate address and fetch data
            unsigned int current_x = tex_address%tex_size;
            unsigned int current_y = tex_address/tex_size;
            float4 current = texfetch(tex,current_x,current_y);

            unsigned int left_x = (tex_address-tex_inc)%tex_size;
            unsigned int left_y = (tex_address-tex_inc)/tex_size;
            float4 left = texfetch(tex,left_x,left_y);

            // calculate and write
            odata[linear_address]=(current.x-left.x)/(2.0*pix_space);
            odata[linear_address+1]=(current.y-left.y)/(2.0*pix_space);
            odata[linear_address+2]=(current.z-left.z)/(2.0*pix_space);
            odata[linear_address+3]=(current.w-left.w)/(2.0*pix_space);
        }
        else
        {
            // calculate address and fetch data
            unsigned int right_x = (tex_address+tex_inc)%tex_size;
            unsigned int right_y = (tex_address+tex_inc)/tex_size;
            float4 right = texfetch(tex,right_x,right_y);

            unsigned int left_x = (tex_address-tex_inc)%tex_size;
            unsigned int left_y = (tex_address-tex_inc)/tex_size;
            float4 left = texfetch(tex,left_x,left_y);

            // calculate and write
            odata[linear_address]=(right.x-left.x)/(4.0*pix_space);
            odata[linear_address+1]=(right.y-left.y)/(4.0*pix_space);
            odata[linear_address+2]=(right.z-left.z)/(4.0*pix_space);
            odata[linear_address+3]=(right.w-left.w)/(4.0*pix_space);
        }
    }
#endif
}    

__global__ void k_evf_nk(float* current_vector_in_mm_x,
                      float* current_vector_in_mm_y,
                      float* current_vector_in_mm_z, 
                      int* thread_id,
                      int* voxel_id,
					  float3 dimen,
                      float3 spacing,
					  float size)
{
#if defined (commentout)
    // update vector field

    // get index of voxel
    // compute index values
    unsigned int blocks_in_y = (unsigned int)ceil((double)dimen.x/(double)blockDim.x);
    unsigned int x_index = ((unsigned int)blockIdx.x%blocks_in_y)*blockDim.x+threadIdx.x;
    unsigned int y_index = ((unsigned int)floor((double)blockIdx.x/blocks_in_y))*blockDim.y + threadIdx.y;
    unsigned int z_index = blockIdx.y*blockDim.z+threadIdx.z;
    unsigned int linear_address = x_index + y_index*dimen.x + z_index*dimen.x*dimen.y;
    float denom, result_x, result_y, result_z;

    int my_thread_id = threadIdx.x + threadIdx.y*blockDim.x + threadIdx.z*blockDim.x*blockDim.y; // Thread ID within block
    int my_linear_address = x_index + y_index*dimen.x + z_index*dimen.x*dimen.y;

    if((x_index<dimen.x) && (y_index<dimen.y) && (z_index<dimen.z))
    {
        // calculate displacements along axes
        int disp_x = ((int)ceil(current_vector_in_mm_x[linear_address]/spacing.x));
        int disp_y = ((int)ceil(current_vector_in_mm_y[linear_address]/spacing.y));
        int disp_z = ((int)ceil(current_vector_in_mm_z[linear_address]/spacing.z));

        int tempx1 = x_index + disp_x;
        int tempy1 = y_index + disp_y;
        int tempz1 = z_index + disp_z;

        // calculate linear memory displacement
        int temp = tempx1 + tempy1*(unsigned int)dimen.x + tempz1*(unsigned int)dimen.x*(unsigned int)dimen.y;

		// Check if the voxel displacements lie outside the volume of interest
		if((tempx1>dimen.x-1) || (tempx1<0) || (tempy1>dimen.y-1) || (tempy1<0) || (tempz1>dimen.z-1) || (tempz1<0))
        {
            // retrieve data from texture
            float nabla_x = texfetch(tex_nabla_x,(int)linear_address);
            float nabla_y = texfetch(tex_nabla_y,(int)linear_address);
            float nabla_z = texfetch(tex_nabla_z,(int)linear_address);
            float static_image = texfetch(tex_static_image,(int)linear_address);

            // perform calculations
            float nabla_squared = nabla_x*nabla_x + nabla_y*nabla_y + nabla_z*nabla_z;
            float diff = (static_image + (float)1000);
            float ns = nabla_squared + diff*diff;
			if( ns == 0)
			{
                // calculate result
				result_x = current_vector_in_mm_x[linear_address];
				result_y = current_vector_in_mm_y[linear_address];
				result_z = current_vector_in_mm_z[linear_address];
			}
			else
            {
                // calculate result
				denom = ((float)diff)/(ns);   
                result_x = current_vector_in_mm_x[linear_address] + denom*nabla_x;
				result_y = current_vector_in_mm_y[linear_address] + denom*nabla_y;
				result_z = current_vector_in_mm_z[linear_address] + denom*nabla_z;
			}
        }
		else{
            // retrieve data from texture
            float nabla_x = texfetch(tex_nabla_x,(int)linear_address);
            float nabla_y = texfetch(tex_nabla_y,(int)linear_address);
            float nabla_z = texfetch(tex_nabla_z,(int)linear_address);
            float static_image = texfetch(tex_static_image,(int)linear_address);
            float moving_image = texfetch(tex_moving_image,(int)temp);

            // perform calculations
            float nabla_squared = nabla_x*nabla_x + nabla_y*nabla_y + nabla_z*nabla_z;
            float diff = (static_image - moving_image);
            float ns = nabla_squared + diff*diff;
			if(ns == 0) 
			{
                // calculate result
				result_x = current_vector_in_mm_x[linear_address];
				result_y = current_vector_in_mm_y[linear_address];
				result_z = current_vector_in_mm_z[linear_address];
			}
			else{
                // calculate result
				denom = ((float)diff)/(ns);
				result_x = current_vector_in_mm_x[linear_address] + denom*nabla_x;
				result_y = current_vector_in_mm_y[linear_address] + denom*nabla_y;
				result_z = current_vector_in_mm_z[linear_address] + denom*nabla_z;

			}
		}
    }

    // synchronize threads between reading and writing
    __syncthreads();
    
    thread_id[linear_address] = my_thread_id;
    voxel_id[linear_address] = my_linear_address;

    if((x_index<dimen.x) && (y_index<dimen.y) && (z_index<dimen.z))
    {
            // write result
            current_vector_in_mm_x[linear_address]=result_x;
            current_vector_in_mm_y[linear_address]=result_y;
            current_vector_in_mm_z[linear_address]=result_z;
    }
#endif
}    


__global__ void k_evf(float* current_vector_in_mm_x,
                      float* current_vector_in_mm_y,
                      float* current_vector_in_mm_z, 
					  float3 dimen,
                      float3 spacing,
					  float size)
{
#if defined (commentout)
    // update vector field

    // get index of voxel
    // compute index values
    unsigned int blocks_in_y = (unsigned int)ceil((double)dimen.x/(double)blockDim.x);
    unsigned int x_index = ((unsigned int)blockIdx.x%blocks_in_y)*blockDim.x+threadIdx.x;
    unsigned int y_index = ((unsigned int)floor((double)blockIdx.x/blocks_in_y))*blockDim.y + threadIdx.y;
    unsigned int z_index = blockIdx.y*blockDim.z+threadIdx.z;
    unsigned int linear_address = x_index + y_index*dimen.x + z_index*dimen.x*dimen.y;
    float denom, result_x, result_y, result_z;

    if((x_index<dimen.x) && (y_index<dimen.y) && (z_index<dimen.z))
    {
        // calculate displacements along axes
        int disp_x = ((int)ceil(current_vector_in_mm_x[linear_address]/spacing.x));
        int disp_y = ((int)ceil(current_vector_in_mm_y[linear_address]/spacing.y));
        int disp_z = ((int)ceil(current_vector_in_mm_z[linear_address]/spacing.z));

        int tempx1 = x_index + disp_x;
        int tempy1 = y_index + disp_y;
        int tempz1 = z_index + disp_z;

        // calculate linear memory displacement
        int temp = tempx1 + tempy1*(unsigned int)dimen.x + tempz1*(unsigned int)dimen.x*(unsigned int)dimen.y;



		// Check if the voxel displacements lie outside the volume of interest
		if((tempx1>dimen.x-1) || (tempx1<0) || (tempy1>dimen.y-1) || (tempy1<0) || (tempz1>dimen.z-1) || (tempz1<0))
        {
            // retrieve data from texture
            float nabla_x = texfetch(tex_nabla_x,(int)linear_address);
            float nabla_y = texfetch(tex_nabla_y,(int)linear_address);
            float nabla_z = texfetch(tex_nabla_z,(int)linear_address);
            float static_image = texfetch(tex_static_image,(int)linear_address);

            // perform calculations
            float nabla_squared = nabla_x*nabla_x + nabla_y*nabla_y + nabla_z*nabla_z;
            float diff = (static_image + (float)1000);
            float ns = nabla_squared + diff*diff;
			if( ns == 0)
			{
                // calculate result
				result_x = current_vector_in_mm_x[linear_address];
				result_y = current_vector_in_mm_y[linear_address];
				result_z = current_vector_in_mm_z[linear_address];
			}
			else
            {
                // calculate result
				denom = ((float)diff)/(ns);   
                result_x = current_vector_in_mm_x[linear_address] + denom*nabla_x;
				result_y = current_vector_in_mm_y[linear_address] + denom*nabla_y;
				result_z = current_vector_in_mm_z[linear_address] + denom*nabla_z;
			}
        }
		else{
            // retrieve data from texture
            float nabla_x = texfetch(tex_nabla_x,(int)linear_address);
            float nabla_y = texfetch(tex_nabla_y,(int)linear_address);
            float nabla_z = texfetch(tex_nabla_z,(int)linear_address);
            float static_image = texfetch(tex_static_image,(int)linear_address);
            float moving_image = texfetch(tex_moving_image,(int)temp);

            // perform calculations
            float nabla_squared = nabla_x*nabla_x + nabla_y*nabla_y + nabla_z*nabla_z;
            float diff = (static_image - moving_image);
            float ns = nabla_squared + diff*diff;
			if(ns == 0) 
			{
                // calculate result
				result_x = current_vector_in_mm_x[linear_address];
				result_y = current_vector_in_mm_y[linear_address];
				result_z = current_vector_in_mm_z[linear_address];
			}
			else{
                // calculate result
				denom = ((float)diff)/(ns);
				result_x = current_vector_in_mm_x[linear_address] + denom*nabla_x;
				result_y = current_vector_in_mm_y[linear_address] + denom*nabla_y;
				result_z = current_vector_in_mm_z[linear_address] + denom*nabla_z;

			}
		}
    }

    // synchronize threads between reading and writing
    __syncthreads();

    if((x_index<dimen.x) && (y_index<dimen.y) && (z_index<dimen.z))
    {
            // write result
            current_vector_in_mm_x[linear_address]=result_x;
            current_vector_in_mm_y[linear_address]=result_y;
            current_vector_in_mm_z[linear_address]=result_z;
    }
#endif
}    



__global__ void gaussian_x(float* odata, float3 dimen, unsigned int blocks_per_y, unsigned int y_in_z,
                                   const float ker0, const float ker1, const float ker_end)
{
#if defined (commentout)
    // calculate index values and addresses
    unsigned int x_tex_index = (unsigned int)((blockIdx.x%blocks_per_y)*blockDim.x+threadIdx.x);
    unsigned int x_index = 4*x_tex_index;
    unsigned int y_index = (unsigned int)(((unsigned int)(blockIdx.x/blocks_per_y))*blockDim.y+threadIdx.y);
    unsigned int z_index = threadIdx.z + blockIdx.y*blockDim.z;
    unsigned int linear_address = x_index + y_index*dimen.x + z_index*dimen.x*dimen.y;
    unsigned int tex_address = linear_address/4;

    if(x_index<dimen.x-3 && y_index<dimen.y && z_index<dimen.z)
    {
        // fetch data from linear texture
        float4 current = texfetch(tex_filter,(int)tex_address);
        if(x_index==0)
        {   
            // fetch data from texture
            float4 right = texfetch(tex_filter,(int)tex_address+1);

            // write out results 
            odata[linear_address]=ker_end*current.x + ker0*current.y;
            odata[linear_address+1]=ker0*current.x+ker1*current.y+ker0*current.z;
            odata[linear_address+2]=ker0*current.y+ker1*current.z+ker0*current.w;
            odata[linear_address+3]=ker0*current.z+ker1*current.w+ker0*right.x;
        }
        else if(x_index==dimen.x-4)
        {
            // fetch data from texture
            float4 left = texfetch(tex_filter,(int)tex_address-1);

            // write out results 
            odata[linear_address]=ker0*left.w+ker1*current.x+ker0*current.y;
            odata[linear_address+1]=ker0*current.x+ker1*current.y+ker0*current.z;
            odata[linear_address+2]=ker0*current.y+ker1*current.z+ker0*current.w;
            odata[linear_address+3]=ker0*current.z+ker_end*current.w;
        }
        else
        {
            // fetch data from texture
            float4 right = texfetch(tex_filter,(int)tex_address+1);
            float4 left = texfetch(tex_filter,(int)tex_address-1);

            // write out results 
            odata[linear_address]=ker0*left.w+ker1*current.x+ker0*current.y;
            odata[linear_address+1]=ker0*current.x+ker1*current.y+ker0*current.z;
            odata[linear_address+2]=ker0*current.y+ker1*current.z+ker0*current.w;
            odata[linear_address+3]=ker0*current.z+ker1*current.w+ker0*right.x;
        }
    }
#endif
}

__global__ void gaussian_y(float* odata, float3 dimen, unsigned int blocks_per_y, unsigned int y_in_z,
                                   const float ker0, const float ker1, const float ker_end)
{
#if defined (commentout)
    // calculate index values and addresses
    unsigned int x_tex_index = (unsigned int)((blockIdx.x%blocks_per_y)*blockDim.x+threadIdx.x);
    unsigned int x_index = 4*x_tex_index;
    unsigned int y_index = (unsigned int)(((unsigned int)(blockIdx.x/blocks_per_y))*blockDim.y+threadIdx.y);
    unsigned int z_index = threadIdx.z + blockIdx.y*blockDim.z;
    unsigned int linear_address = x_index + y_index*dimen.x + z_index*dimen.x*dimen.y;
    unsigned int tex_address = linear_address/4;

    if(x_index<dimen.x-3 && y_index<dimen.y && z_index<dimen.z)
    {
        float4 current = texfetch(tex_filter,(int)tex_address);
        if(y_index==0)
        {   
            // get address and fetch data from linear texture
            int right_addr = (int)((linear_address+dimen.x)/4);
            float4 right = texfetch(tex_filter,right_addr);

            // write out results 
            odata[linear_address]=ker_end*current.x + ker0*right.x;
            odata[linear_address+1]=ker_end*current.y + ker0*right.y;
            odata[linear_address+2]=ker_end*current.z + ker0*right.z;
            odata[linear_address+3]=ker_end*current.w + ker0*right.w;
        }
        else if(y_index==dimen.y-1)
        {
            // get address and fetch data from linear texture
            int left_addr = (int)((linear_address-dimen.x)/4);
            float4 left = texfetch(tex_filter,left_addr);

            // write out results 
            odata[linear_address]=ker_end*current.x + ker0*left.x;
            odata[linear_address+1]=ker_end*current.y + ker0*left.y;
            odata[linear_address+2]=ker_end*current.z + ker0*left.z;
            odata[linear_address+3]=ker_end*current.w + ker0*left.w;
        }
        else
        {
            // get address and fetch data from linear texture
            int right_addr = (int)((linear_address+dimen.x)/4);
            float4 right = texfetch(tex_filter,right_addr);
            int left_addr = (int)((linear_address-dimen.x)/4);
            float4 left = texfetch(tex_filter,left_addr);

            // write out results 
            odata[linear_address]=ker0*left.x+ker1*current.x+ker0*right.x;
            odata[linear_address+1]=ker0*left.y+ker1*current.y+ker0*right.y;
            odata[linear_address+2]=ker0*left.z+ker1*current.z+ker0*right.z;
            odata[linear_address+3]=ker0*left.w+ker1*current.w+ker0*right.w;
        }
    }
#endif
}

__global__ void gaussian_z(float* odata, float3 dimen, unsigned int blocks_per_y, unsigned int y_in_z,
                                   const float ker0, const float ker1, const float ker_end)
{
#if defined (commentout)
    // calculate index values and addresses
    unsigned int x_tex_index = (unsigned int)((blockIdx.x%blocks_per_y)*blockDim.x+threadIdx.x);
    unsigned int x_index = 4*x_tex_index;
    unsigned int y_index = (unsigned int)(((unsigned int)(blockIdx.x/blocks_per_y))*blockDim.y+threadIdx.y);
    unsigned int z_index = threadIdx.z + blockIdx.y*blockDim.z;
    unsigned int linear_address = x_index + y_index*dimen.x + z_index*dimen.x*dimen.y;
    unsigned int tex_address = linear_address/4;
    int inc = (int)dimen.x*(int)dimen.y;

    if(x_index<dimen.x-3 && y_index<dimen.y && z_index<dimen.z)
    {
        // fetch data from texture
        float4 current = texfetch(tex_filter,(int)tex_address);
        if(z_index==0)
        {   
            // get address and fetch data from linear texture
            int right_addr = (int)((linear_address+inc)/4);
            float4 right = texfetch(tex_filter,right_addr);

            // write out results 
            odata[linear_address]=ker_end*current.x + ker0*right.x;
            odata[linear_address+1]=ker_end*current.y + ker0*right.y;
            odata[linear_address+2]=ker_end*current.z + ker0*right.z;
            odata[linear_address+3]=ker_end*current.w + ker0*right.w;
        }
        else if(z_index==dimen.z-1)
        {
            // get address and fetch data from linear texture
            int left_addr = (int)((linear_address-inc)/4);
            float4 left = texfetch(tex_filter,left_addr);

            // write out results 
            odata[linear_address]=ker_end*current.x + ker0*left.x;
            odata[linear_address+1]=ker_end*current.y + ker0*left.y;
            odata[linear_address+2]=ker_end*current.z + ker0*left.z;
            odata[linear_address+3]=ker_end*current.w + ker0*left.w;
        }
        else
        {
            // get address and fetch data from linear texture
            int right_addr = (int)((linear_address+inc)/4);
            float4 right = texfetch(tex_filter,right_addr);
            int left_addr = (int)((linear_address-inc)/4);
            float4 left = texfetch(tex_filter,left_addr);

            // write out results 
            odata[linear_address]=ker0*left.x+ker1*current.x+ker0*right.x;
            odata[linear_address+1]=ker0*left.y+ker1*current.y+ker0*right.y;
            odata[linear_address+2]=ker0*left.z+ker1*current.z+ker0*right.z;
            odata[linear_address+3]=ker0*left.w+ker1*current.w+ker0*right.w;
        }
    }
#endif
}



__global__ void dummy_kernel()
{
    //dummy kernel
}
