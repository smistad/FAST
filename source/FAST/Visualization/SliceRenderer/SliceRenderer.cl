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
#if defined(VOL2) || defined(VOL3) || defined(VOL4) || defined(VOL5)
		, __read_only image3d_t image2
		, __constant float* transformationMatix2
		, __constant unsigned int* imageSize2
#endif
#if defined(VOL3) || defined(VOL4) || defined(VOL5)
		, __read_only image3d_t image3
		, __constant float* transformationMatix3
		, __constant unsigned int* imageSize3
#endif
#if defined(VOL4) || defined(VOL5)
		, __read_only image3d_t image4
		, __constant float* transformationMatix4
		, __constant unsigned int* imageSize4
#endif
#if defined(VOL5)
		, __read_only image3d_t image5
		, __constant float* transformationMatix5
		, __constant unsigned int* imageSize5
#endif
        ) {

	//all of the folowing maths are based on the plane equation: Ax + By + Cz = D
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	
	// TODO make sure that these positions are correct
	float4 pos, transferedPos;
	float value;

	if (slicePlane == 0) {

		const float planeY = x + minY;
		const float planeZ = y + minZ;
		pos = (float4)(-(planeY*planeB + planeZ*planeC - planeD) / planeA, planeY, planeZ, 1);
	}
	else if (slicePlane == 1) {

		const float planeX = x + minX;
		const float planeZ = y + minZ;
		pos = (float4)(planeX, -(planeX*planeA + planeZ*planeC - planeD) / planeB, planeZ, 1);
	}
	else {

		const float planeX = x + minX;
		const float planeY = y + minY;
		pos = (float4)(planeX, planeY, -(planeX*planeA + planeY*planeB - planeD) / planeC, 1);
	}

	// TODO components support
#ifdef TYPE_FLOAT1
	value = read_imagef(image, sampler, pos).x;
#elif TYPE_UINT1
	value = read_imageui(image, sampler, pos).x;
#elif TYPE_INT1
	value = read_imagei(image, sampler, pos).x;
#endif

	value = (value - level + window / 2) / window;
	value = clamp(value, 0.0f, 1.0f);
	//if (value !=0)
		write_imagef(texture, (int2)(x, y), (float4)(value, value, value, 1.0));
	//else
	//	write_imagef(texture, (int2)(x, y), (float4)(value, value, value, 0.0));

#if defined(VOL2) || defined(VOL3) || defined(VOL4) || defined(VOL5)

	transferedPos.x = dot((float4)(transformationMatix2[0], transformationMatix2[4], transformationMatix2[8], transformationMatix2[12]), pos);
	transferedPos.y = dot((float4)(transformationMatix2[1], transformationMatix2[5], transformationMatix2[9], transformationMatix2[13]), pos);
	transferedPos.z = dot((float4)(transformationMatix2[2], transformationMatix2[6], transformationMatix2[10], transformationMatix2[14]), pos);

	if ((transferedPos.x >= 0) && (transferedPos.x <= imageSize2[0]) && (transferedPos.y >= 0) && (transferedPos.y <= imageSize2[1]) && (transferedPos.z >= 0) && (transferedPos.z <= imageSize2[2]))
	{
		
#ifdef TYPE_FLOAT2
		value = read_imagef(image2, sampler, transferedPos).x;
#elif TYPE_UINT2
		value = read_imageui(image2, sampler, transferedPos).x;
#elif TYPE_INT2
		value = read_imagei(image2, sampler, transferedPos).x;
#endif
			value = (value - level + window / 2) / window;
			value = clamp(value, 0.0f, 1.0f);
			write_imagef(texture, (int2)(x, y), (float4)(value, value, value, 1.0));
	}
#endif
	

#if defined(VOL3) || defined(VOL4) || defined(VOL5)

	transferedPos.x = dot((float4)(transformationMatix3[0], transformationMatix3[4], transformationMatix3[8], transformationMatix3[12]), pos);
	transferedPos.y = dot((float4)(transformationMatix3[1], transformationMatix3[5], transformationMatix3[9], transformationMatix3[13]), pos);
	transferedPos.z = dot((float4)(transformationMatix3[2], transformationMatix3[6], transformationMatix3[10], transformationMatix3[14]), pos);

	if ((transferedPos.x >= 0) && (transferedPos.x <= imageSize3[0]) && (transferedPos.y >= 0) && (transferedPos.y <= imageSize3[1]) && (transferedPos.z >= 0) && (transferedPos.z <= imageSize3[2]))
	{

#ifdef TYPE_FLOAT3
		value = read_imagef(image3, sampler, transferedPos).x;
#elif TYPE_UINT3
		value = read_imageui(image3, sampler, transferedPos).x;
#elif TYPE_INT3
		value = read_imagei(image3, sampler, transferedPos).x;
#endif

		value = (value - level + window / 2) / window;
		value = clamp(value, 0.0f, 1.0f);
		write_imagef(texture, (int2)(x, y), (float4)(value, value, value, 1.0));
	}
#endif
	

#if defined(VOL4) || defined(VOL5)

	transferedPos.x = dot((float4)(transformationMatix4[0], transformationMatix4[4], transformationMatix4[8], transformationMatix4[12]), pos);
	transferedPos.y = dot((float4)(transformationMatix4[1], transformationMatix4[5], transformationMatix4[9], transformationMatix4[13]), pos);
	transferedPos.z = dot((float4)(transformationMatix4[2], transformationMatix4[6], transformationMatix4[10], transformationMatix4[14]), pos);

	if ((transferedPos.x >= 0) && (transferedPos.x <= imageSize4[0]) && (transferedPos.y >= 0) && (transferedPos.y <= imageSize4[1]) && (transferedPos.z >= 0) && (transferedPos.z <= imageSize4[2]))
	{

#ifdef TYPE_FLOAT4
		value = read_imagef(image4, sampler, transferedPos).x;
#elif TYPE_UINT4
		value = read_imageui(image4, sampler, transferedPos).x;
#elif TYPE_INT4
		value = read_imagei(image4, sampler, transferedPos).x;
#endif

		value = (value - level + window / 2) / window;
		value = clamp(value, 0.0f, 1.0f);
		write_imagef(texture, (int2)(x, y), (float4)(value, value, value, 1.0));
	}
#endif
	

#if defined(VOL5)

	transferedPos.x = dot((float4)(transformationMatix5[0], transformationMatix5[4], transformationMatix5[8], transformationMatix5[12]), pos);
	transferedPos.y = dot((float4)(transformationMatix5[1], transformationMatix5[5], transformationMatix5[9], transformationMatix5[13]), pos);
	transferedPos.z = dot((float4)(transformationMatix5[2], transformationMatix5[6], transformationMatix5[10], transformationMatix5[14]), pos);

	if ((transferedPos.x >= 0) && (transferedPos.x <= imageSize5[0]) && (transferedPos.y >= 0) && (transferedPos.y <= imageSize5[1]) && (transferedPos.z >= 0) && (transferedPos.z <= imageSize5[2]))
	{

#ifdef TYPE_FLOAT5
		value = read_imagef(image5, sampler, transferedPos).x;
#elif TYPE_UINT5
		value = read_imageui(image5, sampler, transferedPos).x;
#elif TYPE_INT5
		value = read_imagei(image5, sampler, transferedPos).x;
#endif

		value = (value - level + window / 2) / window;
		value = clamp(value, 0.0f, 1.0f);
		write_imagef(texture, (int2)(x, y), (float4)(value, value, value, 1.0));
	}
#endif
	

}
