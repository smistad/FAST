// http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

int get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
    float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y)
{
    float epsilon = 0.001f;

    float s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom, t;
    s10_x = p1_x - p0_x;
    s10_y = p1_y - p0_y;
    s32_x = p3_x - p2_x;
    s32_y = p3_y - p2_y;

    denom = s10_x * s32_y - s32_x * s10_y;
    if (denom == 0)
        return 0; // Collinear
    bool denomPositive = denom > 0;

    s02_x = p0_x - p2_x;
    s02_y = p0_y - p2_y;
    s_numer = s10_x * s02_y - s10_y * s02_x;
    if ((s_numer < epsilon) == denomPositive)
        return 0; // No collision

    t_numer = s32_x * s02_y - s32_y * s02_x;
    if ((t_numer < epsilon) == denomPositive)
        return 0; // No collision

    if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
        return 0; // No collision
    // Collision detected
    t = t_numer / denom;
    *i_x = p0_x + (t * s10_x);
    *i_y = p0_y + (t * s10_y);

    return 1;
}

__kernel void mesh_to_segmentation_2d(
		__global float* coordinates,
		__global uint2* lines,
		__private uint nrOfLines,
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
    const float threshold = 0.00000001;
    // x2 and y2 is 0
    float y2 = y1;
	for(int i = 0; i < nrOfLines; ++i) {
	    // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
	    // http://geomalgorithms.com/a03-_inclusion.html
	    // Find intersection point
	    float3 coordinate3 = vload3(lines[i].x, coordinates);
	    float3 coordinate4 = vload3(lines[i].y, coordinates);
	    float x3 = coordinate3.x;
	    float y3 = coordinate3.y;
	    float x4 = coordinate4.x;
	    float y4 = coordinate4.y;

	    // Order edge points so that it always point downwards
	    if(y3 > y4) {
            // swap y
	        float tmp = y4;
	        y4 = y3;
	        y3 = tmp;

	        // swap x
	        tmp = x4;
	        x4 = x3;
	        x3 = tmp;
	    }

	    if(y3 == y4) {
	        // Horizontal edge, drop
	        continue;
	    }

        float x, y;
        if(get_line_intersection(x1, y1, 0, y2, x3, y3, x4, y4, &x, &y) == 1) {
            // Have we hit an edge point?
            if(fabs(x-x4) < threshold && fabs(y-y4) < threshold) { // Through a top point
                //++intersections;
            } else if(fabs(x-x3) < threshold && fabs(y-y3) < threshold) { // Through a bottom point
                ++intersections;
            } else { // Passes through the middle of the line
                ++intersections;
            }
        }
	}

	write_imageui(segmentation, pos, intersections % 2 == 0 ? 0:1);
}

int rayIntersectsTriangle(float3 p, float3 d, float3 v0, float3 v1, float3 v2) {

	float3 e1,e2,h,s,q;
	float a,f,u,v;
	e1 = v1 - v0;
	e2 = v2 - v0;

	h = cross(d, e2);
	a = dot(e1,h);

	if (a > -0.00000000001f && a < 0.00000000001f)
        return 0;

	f = 1.0f/a;
	s = p - v0;
	u = f * (dot(s,h));

	if (u < 0.0 || u > 1.0)
		return 0;

	q = cross(s,e1);
	v = f * dot(d,q);

	if (v < 0.0 || u + v > 1.0)
        return 0;

	// at this stage we can compute t to find out where
	// the intersection point is on the line
	float t = f * dot(e2,q);

	if (t > 0.00000001f) { // ray intersection
        return 1;

	} else { // this means that there is a line intersection
		 // but not a ray intersection
		 return 0;
     }

}

__kernel void mesh_to_segmentation_3d(
		__global float* coordinates,
		__global uint* triangles,
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
	int intersections = 0;

	// Ray start point
    float3 P0 = {pos.x*spacingX, pos.y*spacingY, pos.z*spacingZ};
    if(P0.y == 0) {
        // There is a strange problem if y component is exactly zero
        // This is a hack to avoid this
        P0.y = 0.00001;
    }

    // Set arbitrarty direction
    float3 P1 = P0;
    P1.x += 0.1;

    for(int i = 0; i < nrOfTriangles; ++i) {
        const uint3 triangle = vload3(i, triangles);
        const float3 u = vload3(triangle.x, coordinates);
        const float3 v = vload3(triangle.y, coordinates);
        const float3 w = vload3(triangle.z, coordinates);

        if(rayIntersectsTriangle(P0, P1, u, v, w) >= 1) {
            intersections += 1;
        }
    }

	segmentation[pos.x + pos.y*get_global_size(0) + pos.z*get_global_size(0)*get_global_size(1)] = intersections % 2 == 0 ? 0:1;
}
