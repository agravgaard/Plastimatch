/* -----------------------------------------------------------------------
See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
----------------------------------------------------------------------- */

#define BLOCK_SIZE 256

/* Define image/texture sampling parameters */
const sampler_t img_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void calculate_gradient_magnitude_image_kernel (
	__write_only __global float *grad_mag, 
	__read_only image3d_t grad_img_x,
	__read_only image3d_t grad_img_y,
	__read_only image3d_t grad_img_z,
	__constant int4 *dim
) {
	/* Find position in volume */
	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint z = get_global_id(2);

	if (x >= (*dim).x || y >= (*dim).y || z >= (*dim).z)
		return;

	long v = (z * (*dim).y * (*dim).x) + (y * (*dim).x) + x;
	
	int4 vPos = {x, y, z, 0};
	float4 vox_grad_x = read_imagef(grad_img_x, img_sampler, vPos);
	float4 vox_grad_y = read_imagef(grad_img_y, img_sampler, vPos);
	float4 vox_grad_z = read_imagef(grad_img_z, img_sampler, vPos);

	grad_mag[v] = vox_grad_x.x * vox_grad_x.x + vox_grad_y.x * vox_grad_y.x + vox_grad_z.x * vox_grad_z.x;
}

__kernel void estimate_kernel (
	__global float *vf_est_x_img,
	__global float *vf_est_y_img,
	__global float *vf_est_z_img,
	__read_only image3d_t vf_smooth_x_img,
	__read_only image3d_t vf_smooth_y_img,
	__read_only image3d_t vf_smooth_z_img,
	__read_only image3d_t fixed_img,
	__read_only image3d_t moving_img,
	__read_only image3d_t grad_mag_img,
	__read_only image3d_t grad_x_img,
	__read_only image3d_t grad_y_img,
	__read_only image3d_t grad_z_img,
	__global float *ssd,
	__global int *inliers,
	__constant int4 *dim,
	__constant int4 *moving_dim,
	__constant float4 *f2mo,
	__constant float4 *f2ms,
	__constant float4 *invmps,
	float homog,
	float denominator_eps,
	float accel
) {
	/* Find position in volume */
	uint i = get_global_id(0);
	uint j = get_global_id(1);
	uint k = get_global_id(2);

	if (i >= (*dim).x || j >= (*dim).y || k >= (*dim).z)
		return;

	long fv = (k * (*dim).y * (*dim).x) + (j * (*dim).x) + i;
	int4 fvPos = {i, j, k, 0};
	float4 smooth_voxel_x = read_imagef(vf_smooth_x_img, img_sampler, fvPos);
	float4 smooth_voxel_y = read_imagef(vf_smooth_y_img, img_sampler, fvPos);
	float4 smooth_voxel_z = read_imagef(vf_smooth_z_img, img_sampler, fvPos);

	float mi = (*f2mo).x + i * (*f2ms).x;
	float mj = (*f2mo).y + j * (*f2ms).y;
	float mk = (*f2mo).z + k * (*f2ms).z;

	/* Find correspondence with nearest neighbor interpolation & boundary checking */
	int mz = convert_int_rte(mk + (*invmps).z * smooth_voxel_z.x);	/* pixels (moving) */
	if (mz < 0 || mz >= (*moving_dim).z)
		return;

	int my = convert_int_rte(mj + (*invmps).y * smooth_voxel_y.x);	/* pixels (moving) */
	if (my < 0 || my >= (*moving_dim).y)
		return;

	int mx = convert_int_rte(mi + (*invmps).x * smooth_voxel_x.x);		/* pixels (moving) */
	if (mx < 0 || mx >= (*moving_dim).x)
		return;

	int4 mvPos = {mx, my, mz, 0};

	/* Find image difference at this correspondence */
	float4 fixed_voxel = read_imagef(fixed_img, img_sampler, fvPos);
	float4 moving_voxel = read_imagef(moving_img, img_sampler, mvPos);
	float diff = fixed_voxel.x - moving_voxel.x;		/* intensity */

	/* Compute denominator */
	float4 grad_mag_voxel = read_imagef(grad_mag_img, img_sampler, mvPos);
	float denom = grad_mag_voxel.x + homog * diff * diff;		/* intensity^2 per mm^2 */

	/* Compute SSD for statistics */
	inliers[fv] = 1;
	ssd[fv] = diff * diff;

	/* Threshold the denominator to stabilize estimation */
	if (denom < denominator_eps) 
		return;

	/* Compute new estimate of displacement */
	float mult = accel * diff / denom;					/* per intensity^2 */
	float4 grad_voxel_x = read_imagef(grad_x_img, img_sampler, mvPos);
	float4 grad_voxel_y = read_imagef(grad_y_img, img_sampler, mvPos);
	float4 grad_voxel_z = read_imagef(grad_z_img, img_sampler, mvPos);
	vf_est_x_img[fv] += mult * grad_voxel_x.x;			/* mm */
	vf_est_y_img[fv] += mult * grad_voxel_y.x;
	vf_est_z_img[fv] += mult * grad_voxel_z.x;
}

__kernel void vf_convolve_x_kernel (
	__write_only __global float *vf_smooth_x,
	__write_only __global float *vf_smooth_y,
	__write_only __global float *vf_smooth_z,
	__read_only image3d_t vf_est_x_img,
	__read_only image3d_t vf_est_y_img,
	__read_only image3d_t vf_est_z_img,
	__constant float *ker,
	__constant int4 *dim,
	int half_width
) {
	int i, i1;		/* i is the offset in the vf */
	int j, j1, j2;	/* j is the index of the kernel */
	int d;			/* d is the vector field direction */

	/* Find position in volume */
	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint z = get_global_id(2);

	if (x >= (*dim).x || y >= (*dim).y || z >= (*dim).z)
		return;

	long v = (z * (*dim).y * (*dim).x) + (y * (*dim).x) + x;

	j1 = x - half_width;
	j2 = x + half_width;
	if (j1 < 0) j1 = 0;
	if (j2 >= (*dim).x) {
		j2 = (*dim).x - 1;
	}
	i1 = j1 - x;
	j1 = j1 - x + half_width;
	j2 = j2 - x + half_width;

	int4 index;
	float sum_x = 0.0;
	float sum_y = 0.0;
	float sum_z = 0.0;

	for (i = i1, j = j1; j <= j2; i++, j++) {
		index.x = x + i;
		index.y = y;
		index.z = z;
		float4 partial_x = read_imagef(vf_est_x_img, img_sampler, index);
		float4 partial_y = read_imagef(vf_est_y_img, img_sampler, index);
		float4 partial_z = read_imagef(vf_est_z_img, img_sampler, index);
		sum_x += ker[j] * partial_x.x;
		sum_y += ker[j] * partial_y.x;
		sum_z += ker[j] * partial_z.x;
	}

	vf_smooth_x[v] = sum_x;
	vf_smooth_y[v] = sum_y;
	vf_smooth_z[v] = sum_z;
}

__kernel void vf_convolve_y_kernel (
	__write_only __global float *vf_est_x,
	__write_only __global float *vf_est_y,
	__write_only __global float *vf_est_z,
	__read_only image3d_t vf_smooth_x_img,
	__read_only image3d_t vf_smooth_y_img,
	__read_only image3d_t vf_smooth_z_img,
	__constant float *ker,
	__constant int4 *dim,
	int half_width

) {
	int i, i1;		/* i is the offset in the vf */
	int j, j1, j2;	/* j is the index of the kernel */
	int d;			/* d is the vector field direction */

	/* Find position in volume */
	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint z = get_global_id(2);

	if (x >= (*dim).x || y >= (*dim).y || z >= (*dim).z)
		return;

	long v = (z * (*dim).y * (*dim).x) + (y * (*dim).x) + x;

	j1 = y - half_width;
	j2 = y + half_width;
	if (j1 < 0) j1 = 0;
	if (j2 >= (*dim).y) {
		j2 = (*dim).y - 1;
	}
	i1 = j1 - y;
	j1 = j1 - y + half_width;
	j2 = j2 - y + half_width;

	int4 index;
	float sum_x = 0.0;
	float sum_y = 0.0;
	float sum_z = 0.0;

	for (i = i1, j = j1; j <= j2; i++, j++) {
		index.x = x;
		index.y = y + i;
		index.z = z;
		float4 partial_x = read_imagef(vf_smooth_x_img, img_sampler, index);
		float4 partial_y = read_imagef(vf_smooth_y_img, img_sampler, index);
		float4 partial_z = read_imagef(vf_smooth_z_img, img_sampler, index);
		sum_x += ker[j] * partial_x.x;
		sum_y += ker[j] * partial_y.x;
		sum_z += ker[j] * partial_z.x;
	}

	vf_est_x[v] = sum_x;
	vf_est_y[v] = sum_y;
	vf_est_z[v] = sum_z;
}

__kernel void vf_convolve_z_kernel (
	__write_only __global float *vf_smooth_x,
	__write_only __global float *vf_smooth_y,
	__write_only __global float *vf_smooth_z,
	__read_only image3d_t vf_est_x_img,
	__read_only image3d_t vf_est_y_img,
	__read_only image3d_t vf_est_z_img,
	__constant float *ker,
	__constant int4 *dim,
	int half_width
) {
	int i, i1;		/* i is the offset in the vf */
	int j, j1, j2;	/* j is the index of the kernel */
	int d;			/* d is the vector field direction */

	/* Find position in volume */
	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint z = get_global_id(2);

	if (x >= (*dim).x || y >= (*dim).y || z >= (*dim).z)
		return;

	long v = (z * (*dim).y * (*dim).x) + (y * (*dim).x) + x;

	j1 = z - half_width;
	j2 = z + half_width;
	if (j1 < 0) j1 = 0;
	if (j2 >= (*dim).z) {
		j2 = (*dim).z - 1;
	}
	i1 = j1 - z;
	j1 = j1 - z + half_width;
	j2 = j2 - z + half_width;

	int4 index;
	float sum_x = 0.0;
	float sum_y = 0.0;
	float sum_z = 0.0;

	for (i = i1, j = j1; j <= j2; i++, j++) {
		index.x = x;
		index.y = y;
		index.z = z + i;
		float4 partial_x = read_imagef(vf_est_x_img, img_sampler, index);
		float4 partial_y = read_imagef(vf_est_y_img, img_sampler, index);
		float4 partial_z = read_imagef(vf_est_z_img, img_sampler, index);
		sum_x += ker[j] * partial_x.x;
		sum_y += ker[j] * partial_y.x;
		sum_z += ker[j] * partial_z.x;
	}

	vf_smooth_x[v] = sum_x;
	vf_smooth_y[v] = sum_y;
	vf_smooth_z[v] = sum_z;
}

__kernel void reduction_float_kernel (
	__global float *vectorData,
	int totalElements,
	__local float *vector
) {
	/* Find position in vector */
	int threadID = get_local_id(0);
	int groupID = get_group_id(0);
	int xInVector = BLOCK_SIZE * groupID * 2 + threadID;

	/* Populate shared memory */
	vector[threadID] = (xInVector < totalElements) ? vectorData[xInVector] : 0;
	vector[threadID + BLOCK_SIZE] = (xInVector + BLOCK_SIZE < totalElements) ? vectorData[xInVector + BLOCK_SIZE] : 0;
	barrier(CLK_LOCAL_MEM_FENCE);

	/* Calculate partial sum */
	for (int stride = BLOCK_SIZE; stride > 0; stride >>= 1) {
		if (threadID < stride)
			vector[threadID] += vector[threadID + stride];
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	/* Store sum reduction for group */
	if (threadID == 0)
		vectorData[groupID] = vector[0];
}

__kernel void reduction_int_kernel (
	__global int *vectorData,
	int totalElements,
	__local int *vector
) {
	/* Find position in vector */
	int threadID = get_local_id(0);
	int groupID = get_group_id(0);
	int xInVector = BLOCK_SIZE * groupID * 2 + threadID;

	/* Populate shared memory */
	vector[threadID] = (xInVector < totalElements) ? vectorData[xInVector] : 0;
	vector[threadID + BLOCK_SIZE] = (xInVector + BLOCK_SIZE < totalElements) ? vectorData[xInVector + BLOCK_SIZE] : 0;
	barrier(CLK_LOCAL_MEM_FENCE);

	/* Calculate partial sum */
	for (int stride = BLOCK_SIZE; stride > 0; stride >>= 1) {
		if (threadID < stride)
			vector[threadID] += vector[threadID + stride];
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	/* Store sum reduction for group */
	if (threadID == 0)
		vectorData[groupID] = vector[0];
}

__kernel void volume_calc_grad_kernel (
	__write_only __global float *moving_grad_x,
	__write_only __global float *moving_grad_y,
	__write_only __global float *moving_grad_z,
	__read_only image3d_t moving_img,
	__constant int4 *dim,
	__constant float4 *pix_spacing_div2
) {
	/* Find position in volume */
	uint i = get_global_id(0);
	uint j = get_global_id(1);
	uint k = get_global_id(2);

	if (i >= (*dim).x || j >= (*dim).y || k >= (*dim).z)
		return;

	/* p is prev, n is next */
	int i_p = (i == 0) ? 0 : i - 1;
	int i_n = (i == (*dim).x - 1) ? (*dim).x - 1 : i + 1;
	int j_p = (j == 0) ? 0 : j - 1;
	int j_n = (j == (*dim).y - 1) ? (*dim).y - 1 : j + 1;
	int k_p = (k == 0) ? 0 : k - 1;
	int k_n = (k == (*dim).z - 1) ? (*dim).z - 1 : k + 1;

	long v = (k * (*dim).y * (*dim).x) + (j * (*dim).x) + i;

	int4 idx_p, idx_n;
	float4 p, n;
	idx_p = (int4)(i_p, j, k, 0);
	p = read_imagef(moving_img, img_sampler, idx_p);
	idx_n = (int4)(i_n, j, k, 0);
	n = read_imagef(moving_img, img_sampler, idx_n);
	moving_grad_x[v] = (n.x - p.x) * (*pix_spacing_div2).x;

	idx_p = (int4)(i, j_p, k, 0);
	p = read_imagef(moving_img, img_sampler, idx_p);
	idx_n = (int4)(i, j_n, k, 0);
	n = read_imagef(moving_img, img_sampler, idx_n);
	moving_grad_y[v] = (n.x - p.x) * (*pix_spacing_div2).y;

	idx_p = (int4)(i, j, k_p, 0);
	p = read_imagef(moving_img, img_sampler, idx_p);
	idx_n = (int4)(i, j, k_n, 0);
	n = read_imagef(moving_img, img_sampler, idx_n);
	moving_grad_z[v] = (n.x - p.x) * (*pix_spacing_div2).z;
}
