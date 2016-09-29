// http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
char get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
    float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y)
{
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    float s, t;
    s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        // Collision detected
        if (i_x != NULL)
            *i_x = p0_x + (t * s1_x);
        if (i_y != NULL)
            *i_y = p0_y + (t * s1_y);
        return 1;
    }

    return 0; // No collision
}

__kernel void mesh_to_segmentation_2d(
		__global float2* coordinates,
		__global uint2* lines,
		__private int nrOfLines,
		__write_only image2d_t segmentation,
		__private float spacingX,
		__private float spacingY
	) {

	const int2 pos = {get_global_id(0), get_global_id(1)};

	// For each line, check if ray (arbitrary direction) from this pixel intersects
	// Count number of intersections, if even: not inside, if odd: inside
	int intersections = 0;
    float x1 = pos.x*spacingX;
    float y1 = pos.y*spacingY;
    const float t = 0.00000001; // threshold
    // x2 and y2 is 0
    float y2 = y1;
	for(int i = 0; i < nrOfLines; ++i) {
	    // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
	    // http://geomalgorithms.com/a03-_inclusion.html
	    // Find intersection point
	    float x3 = coordinates[lines[i].x].x;
	    float y3 = coordinates[lines[i].x].y;
	    float x4 = coordinates[lines[i].y].x;
	    float y4 = coordinates[lines[i].y].y;

	    // Order edge points so that it point downwards
	    if(y3 > y4) {
	        float tmp = y4;
	        y4 = y3;
	        y3 = tmp;
	        tmp = x4;
	        x4 = x3;
	        x3 = tmp;
	    }

	    if(y3 == y4) {
	        // Horizontal edge, drop
	        continue;
	    }

        float x,y;
        if(get_line_intersection(x1, y1, 0, y2, x3, y3, x4, y4, &x, &y) == 1) {
            // Have we hit an edge point?
            if(fabs(x-x4) < t && fabs(y-y4) < t) {
                intersections++;
            } else if(fabs(x-x3) < t && fabs(y-y3) < t) {
            } else {
                intersections++;
            }
        }
	}

	write_imageui(segmentation, pos, intersections % 2 == 0 ? 0:255);
}

__kernel void mesh_to_segmentation_3d(
		__global float3* coordinates,
		__global uint3* triangles,
		__private int nrOfTriangles,
		__global uchar* segmentation,
		__private float spacingX,
		__private float spacingY,
		__private float spacingZ
	) {
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};


	// For each triangle, check if ray (arbitrary direction) from this pixel intersects
	// See: http://geomalgorithms.com/a06-_intersect-2.html#intersect3D_RayTriangle()
	// Count number of intersections, if even: not inside, if odd: inside

}