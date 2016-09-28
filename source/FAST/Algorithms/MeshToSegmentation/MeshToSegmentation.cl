__kernel void mesh_to_segmentation_2d(
		__global float2* coordinates,
		__global uint2* lines,
		__write_only image2d_t segmentation
	) {
}

__kernel void mesh_to_segmentation_3d(
		__global float3* coordinates,
		__global uint3* triangles,
		__global uchar* segmentation
	) {
}