__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void renderToTexture(
        __read_only image3d_t image,
        __write_only image2d_t texture,
        __private float level,
        __private float window,
        __private int slicePlane,
		__private float planeA,
		__private float planeB,
		__private float planeC,
		__private float planeD,
		__private unsigned int minX,
		__private unsigned int minY,
		__private unsigned int minZ
        ) {

	//all of the folowing maths are based on the plane equation: Ax + By + Cz = D
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	
	// TODO make sure that these positions are correct
	int4 pos;
	if (slicePlane == 0) {

		const float planeY = x + minY;
		const float planeZ = y + minZ;
		pos = (int4)(-(planeY*planeB + planeZ*planeC - planeD) / planeA, planeY, planeZ, 0);
	}
	else if (slicePlane == 1) {

		const float planeX = x + minX;
		const float planeZ = y + minZ;
		pos = (int4)(planeX, -(planeX*planeA + planeZ*planeC - planeD) / planeB, planeZ, 0);
	}
	else {

		const float planeX = x + minX;
		const float planeY = y + minY;
		pos = (int4)(planeX, planeY, -(planeX*planeA + planeY*planeB - planeD) / planeC, 0);
	}

	// TODO components support
#ifdef TYPE_FLOAT
	float value = read_imagef(image, sampler, pos).x;
#elif TYPE_UINT
	float value = read_imageui(image, sampler, pos).x;
#else
	float value = read_imagei(image, sampler, pos).x;
#endif
	value = (value - level + window / 2) / window;
	value = clamp(value, 0.0f, 1.0f);
	//value = 1.0f;
	//if (value !=0)
		write_imagef(texture, (int2)(x, y), (float4)(value, value, value, 1.0));
	//else
	//	write_imagef(texture, (int2)(x, y), (float4)(value, value, value, 0.0));

}
