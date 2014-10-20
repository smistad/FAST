/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#define maxSteps 400
#define tstep 0.5f

const sampler_t geometrySampler =		CLK_NORMALIZED_COORDS_FALSE |
										CLK_ADDRESS_CLAMP_TO_EDGE   |
										CLK_FILTER_NEAREST;

const sampler_t transferFuncSampler =	CLK_NORMALIZED_COORDS_TRUE	|
										CLK_ADDRESS_CLAMP_TO_EDGE	|
										CLK_FILTER_LINEAR;

const sampler_t volumeSampler =			CLK_NORMALIZED_COORDS_FALSE	|
										CLK_ADDRESS_CLAMP_TO_EDGE	|
										CLK_FILTER_LINEAR;

/*
// intersect ray with a box
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm

int intersectBox(float4 r_o, float4 r_d, float4 boxmin, float4 boxmax, float *tnear, float *tfar)
{
    // compute intersection of ray with all six bbox planes
    float4 invR = (float4)(1.0f,1.0f,1.0f,1.0f) / r_d;
    float4 tbot = invR * (boxmin - r_o);
    float4 ttop = invR * (boxmax - r_o);

    // re-order intersections to find smallest and largest on each axis
    float4 tmin = min(ttop, tbot);
    float4 tmax = max(ttop, tbot);

    // find the largest tmin and the smallest tmax
    float largest_tmin = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    float smallest_tmax = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));

	*tnear = largest_tmin;
	*tfar = smallest_tmax;

	return smallest_tmax > largest_tmin;
}
*/

//Assuming that box minimum always starts from zero
int intersectBox(float4 r_o, float4 r_d, float4 boxmax, float *tnear, float *tfar)
{
    // compute intersection of ray with all six bbox planes
    float4 invR = (float4)(1.0f,1.0f,1.0f,1.0f) / r_d;
    float4 tbot = invR * -r_o;
    float4 ttop = invR * (boxmax - r_o);

    // re-order intersections to find smallest and largest on each axis
    float4 tmin = min(ttop, tbot);
    float4 tmax = max(ttop, tbot);

    // find the largest tmin and the smallest tmax
    float largest_tmin = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    float smallest_tmax = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));

	*tnear = largest_tmin;
	*tfar = smallest_tmax;

	return smallest_tmax > largest_tmin;
}

uint rgbaFloatToInt(float4 rgba)
{
    rgba.x = clamp(rgba.x,0.0f,1.0f);  
    rgba.y = clamp(rgba.y,0.0f,1.0f);  
    rgba.z = clamp(rgba.z,0.0f,1.0f);  
    rgba.w = clamp(rgba.w,0.0f,1.0f);  
    return ((uint)(rgba.w*255.0f)<<24) | ((uint)(rgba.z*255.0f)<<16) | ((uint)(rgba.y*255.0f)<<8) | (uint)(rgba.x*255.0f);
}

__kernel void
d_render(__global uint *d_output, 
         uint imageW, uint imageH,
         float density, float brightness,
         float zNear, float zFar,
		 float top, float right,
		 float projectionMatrix10, float projectionMatrix14,
         __constant float* invViewMatrix,
          __read_only image3d_t volume,
          __read_only image2d_t transferFunc,
		  __read_only image2d_t opacityFunc,
		  __constant float* invProjectionModelView
#if defined(VOL2) || defined(VOL3) || defined(VOL4) || defined(VOL5)
		  ,__read_only image3d_t volume2
		  ,__read_only image2d_t transferFunc2
		  ,__read_only image2d_t opacityFunc2
#endif
#if defined(VOL3) || defined(VOL4) || defined(VOL5)
		  ,__read_only image3d_t volume3
		  ,__read_only image2d_t transferFunc3
		  ,__read_only image2d_t opacityFunc3
#endif
#if defined(VOL4) || defined(VOL5)
		  ,__read_only image3d_t volume4
		  ,__read_only image2d_t transferFunc4
		  ,__read_only image2d_t opacityFunc4
#endif
#if defined(VOL5)
		  ,__read_only image3d_t volume5
		  ,__read_only image2d_t transferFunc5
		  ,__read_only image2d_t opacityFunc5
#endif
         )

{	
    uint x = get_global_id(0);
    uint y = get_global_id(1);

	if ((x >= imageW) || (y >= imageH)) return;
	uint outputIndex =(y * imageW) + x;

    float u = (((x / (float) imageW)*2.0f)-1.0f)*right;
    float v = (((y / (float) imageH)*2.0f)-1.0f)*top;

	//float winZ = ((read_imagef(geoDepthTexture, geometrySampler, (int2)(x,y)).x)*-2.0f)+1.0f;
	
	//float Z = projectionMatrix14 / (winZ+projectionMatrix10);


	float4 boxMin = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float4 boxMax = (float4)(276.0f, 249.0f, 200.0f , 0.0f);
	
    // calculate eye ray in world space
    float4 eyeRay_o;
    float4 eyeRay_d, temp_eyeRay_d;
	float4 volumeColor;

	eyeRay_o = (float4)(invViewMatrix[12], invViewMatrix[13], invViewMatrix[14], 0.0f);

	temp_eyeRay_d = normalize((float4)(u, v, -zNear,0.0f));
	
	eyeRay_d.x = dot(temp_eyeRay_d, ((float4)(invViewMatrix[0],invViewMatrix[4],invViewMatrix[8], 0.0f)));
    eyeRay_d.y = dot(temp_eyeRay_d, ((float4)(invViewMatrix[1],invViewMatrix[5],invViewMatrix[9], 0.0f)));
    eyeRay_d.z = dot(temp_eyeRay_d, ((float4)(invViewMatrix[2],invViewMatrix[6],invViewMatrix[10], 0.0f)));
	eyeRay_d.w = 1.0f;
	

    // find intersection with box
	float tnear, tfar;
	int hit = intersectBox(eyeRay_o, eyeRay_d, boxMax, &tnear, &tfar);

    if (!hit) 
	{	
		d_output[outputIndex] = rgbaFloatToInt((float4)(0.0f, 0.0f, 0.0f, 1.0f));
        return;
    }

	//if (tfar > -Z) tfar = -Z;
	if (tnear < zNear) tnear = zNear;	// clamp to near plane
	if (tfar > zFar) tfar= zFar;	// clamp to far  plane
	
    // march along ray from back to front, accumulating color
    volumeColor = (float4)(0.0f,0.0f,0.0f,0.0f);
	
    float t = tfar;
	
    for(uint i=0; i<maxSteps*500; i++) {

        float4 pos = eyeRay_o + eyeRay_d*t;    
        
		// read from 3D texture
#ifdef TYPE_FLOAT1
		float sample = read_imagef(volume, volumeSampler, pos).x;
#elif TYPE_UINT1
		float sample = (float)(read_imageui(volume, volumeSampler, pos).x)/255;
#elif TYPE_INT1
		float sample = (float)(read_imagei(volume, volumeSampler, pos).x)/255;
#endif
        // lookup in transfer function texture
        float2 transfer_pos = (float2)(sample, 0.5f);
        float4 col =  read_imagef(transferFunc, transferFuncSampler, transfer_pos);
		float4 alpha= read_imagef(opacityFunc, transferFuncSampler, transfer_pos);
		col.w=alpha.w;
        // accumulate result
        float a = col.w*density;
        volumeColor = mix(volumeColor, col, (float4)(a, a, a, a));
		
#if defined(VOL2) || defined(VOL3) || defined(VOL4) || defined(VOL5)
		//pos= (float4)(eyeRay_o.x+0.0f, eyeRay_o.y+0.0f, eyeRay_o.z, eyeRay_o.w+0.0f) + eyeRay_d*t; //Mehdi
		//if( (pos.x>=-1.0f) && (pos.x<=1.0f) && (pos.y>=-1.0f) && (pos.y<=1.0f) )
		{
			//pos= pos*0.5f+0.5f; //Mehdi

#ifdef TYPE_FLOAT2
			float sample = read_imagef(volume2, volumeSampler, pos).x;
#elif TYPE_UINT2
			float sample = (float)(read_imageui(volume2, volumeSampler, pos).x)/255;
#elif TYPE_INT2
			float sample = (float)(read_imagei(volume2, volumeSampler, pos).x)/255;
#endif

			transfer_pos = (float2)(sample, 0.5f); //Mehdi
			col = read_imagef(transferFunc2, transferFuncSampler, transfer_pos); //Mehdi
			float4 alpha= read_imagef(opacityFunc2, transferFuncSampler, transfer_pos);
			col.w=alpha.w;
			a = col.w*density; //Mehdi
			volumeColor = mix(volumeColor, col, (float4)(a, a, a, a)); //Mehdi
		}
#endif //VOL2
		
#if defined(VOL3) || defined(VOL4) || defined(VOL5)
		pos= (float4)(eyeRay_o.x+0.0f, eyeRay_o.y+0.0f, eyeRay_o.z, eyeRay_o.w+0.0f) + eyeRay_d*t; //Mehdi
		if( (pos.x>=-1.0f) && (pos.x<=1.0f) && (pos.y>=-1.0f) && (pos.y<=1.0f) )
		{
			//pos= pos*0.5f+0.5f; //Mehdi

#ifdef TYPE_FLOAT3
			float sample = read_imagef(volume3, volumeSampler, pos).x;
#elif TYPE_UINT3
			float sample = (float)(read_imageui(volume3, volumeSampler, pos).x)/255;
#elif TYPE_INT3
			float sample = (float)(read_imagei(volume3, volumeSampler, pos).x)/255;
#endif

			transfer_pos = (float2)(sample, 0.5f); //Mehdi
			col = read_imagef(transferFunc3, transferFuncSampler, transfer_pos); //Mehdi
			float4 alpha= read_imagef(opacityFunc3, transferFuncSampler, transfer_pos);
			col.w=alpha.w;
			a = col.w*density; //Mehdi
			volumeColor = mix(volumeColor, col, (float4)(a, a, a, a)); //Mehdi
		}
#endif //VOL3

#if defined(VOL4) || defined(VOL5)
		pos= (float4)(eyeRay_o.x+0.0f, eyeRay_o.y+0.0f, eyeRay_o.z, eyeRay_o.w+0.0f) + eyeRay_d*t; //Mehdi
		if( (pos.x>=-1.0f) && (pos.x<=1.0f) && (pos.y>=-1.0f) && (pos.y<=1.0f) )
		{
			//pos= pos*0.5f+0.5f; //Mehdi

#ifdef TYPE_FLOAT4
			float sample = read_imagef(volume4, volumeSampler, pos).x;
#elif TYPE_UINT4
			float sample = (float)(read_imageui(volume4, volumeSampler, pos).x)/255;
#elif TYPE_INT4
			float sample = (float)(read_imagei(volume4, volumeSampler, pos).x)/255;
#endif

			transfer_pos = (float2)(sample, 0.5f); //Mehdi
			col = read_imagef(transferFunc4, transferFuncSampler, transfer_pos); //Mehdi
			float4 alpha= read_imagef(opacityFunc4, transferFuncSampler, transfer_pos);
			col.w=alpha.w;
			a = col.w*density; //Mehdi
			volumeColor = mix(volumeColor, col, (float4)(a, a, a, a)); //Mehdi
		}
#endif //VOL4

#if defined(VOL5)
		pos= (float4)(eyeRay_o.x+0.0f, eyeRay_o.y+0.0f, eyeRay_o.z, eyeRay_o.w+0.0f) + eyeRay_d*t; //Mehdi
		if( (pos.x>=-1.0f) && (pos.x<=1.0f) && (pos.y>=-1.0f) && (pos.y<=1.0f) )
		{
			//pos= pos*0.5f+0.5f; //Mehdi

#ifdef TYPE_FLOAT5
			float sample = read_imagef(volume5, volumeSampler, pos).x;
#elif TYPE_UINT5
			float sample = (float)(read_imageui(volume5, volumeSampler, pos).x)/255;
#elif TYPE_INT5
			float sample = (float)(read_imagei(volume5, volumeSampler, pos).x)/255;
#endif

			transfer_pos = (float2)(sample, 0.5f); //Mehdi
			col = read_imagef(transferFunc5, transferFuncSampler, transfer_pos); //Mehdi
			float4 alpha= read_imagef(opacityFunc5, transferFuncSampler, transfer_pos);
			col.w=alpha.w;
			a = col.w*density; //Mehdi
			volumeColor = mix(temp, col, (float4)(a, a, a, a)); //Mehdi
		}		
#endif //VOL5
		
        t -= tstep;
        if (t < tnear) break;
    }
    volumeColor *= brightness;
	//float4 geoColor=read_imagef(geoColorTexture, geometrySampler, (int2)(x,y));
	//volumeColor = mix (geoColor, volumeColor ,volumeColor.w);
	volumeColor.w=1.0f;
	
	// write output color
	d_output[outputIndex] = rgbaFloatToInt(volumeColor);
    
}

