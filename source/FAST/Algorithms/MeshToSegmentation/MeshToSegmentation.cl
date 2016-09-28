__kernel void mesh_to_segmentation_2d(
		__global float2* coordinates,
		__global uint2* lines,
		__private int nrOfLines,
		__write_only image2d_t segmentation
	) {

	// For each line, check if ray (arbitrary direction) from this pixel intersects
	// Count number of intersections, if even: not inside, if odd: inside

}

__kernel void mesh_to_segmentation_3d(
		__global float3* coordinates,
		__global uint3* triangles,
		__private int nrOfTriangles,
		__global uchar* segmentation
	) {
	

	// For each triangle, check if ray (arbitrary direction) from this pixel intersects
	// See: http://geomalgorithms.com/a06-_intersect-2.html#intersect3D_RayTriangle()
	// Count number of intersections, if even: not inside, if odd: inside

}