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

#define maxSteps 500
#define tstep 0.005f

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
         float transferOffset, float transferScale,
         __constant float* invViewMatrix
          ,__read_only image3d_t volume,
          __read_only image2d_t transferFunc,
          sampler_t volumeSampler,
          sampler_t transferFuncSampler
#if defined(VOL2) || defined(VOL3) || defined(VOL4) || defined(VOL5)
		  ,__read_only image3d_t volume2
		  ,__read_only image2d_t transferFunc2
#endif
#if defined(VOL3) || defined(VOL4) || defined(VOL5)
		  ,__read_only image3d_t volume3
		  ,__read_only image2d_t transferFunc3
#endif
#if defined(VOL4) || defined(VOL5)
		  ,__read_only image3d_t volume4
		  ,__read_only image2d_t transferFunc4
#endif
#if defined(VOL5)
		  ,__read_only image3d_t volume5
		  ,__read_only image2d_t transferFunc5
#endif
         )

{	
    uint x = get_global_id(0);
    uint y = get_global_id(1);

    float u = (x / (float) imageW)*2.0f-1.0f;
    float v = (y / (float) imageH)*2.0f-1.0f;

    //float tstep = 0.01f;
    float4 boxMin = (float4)(-1.0f, -1.0f, -1.0f,1.0f);
    float4 boxMax = (float4)(1.0f, 1.0f, 1.0f,1.0f);

    // calculate eye ray in world space
    float4 eyeRay_o;
    float4 eyeRay_d;

    eyeRay_o = (float4)(invViewMatrix[3], invViewMatrix[7], invViewMatrix[11], 1.0f);   

    float4 temp = normalize(((float4)(u, v, -2.0f,0.0f)));
    eyeRay_d.x = dot(temp, ((float4)(invViewMatrix[0],invViewMatrix[1],invViewMatrix[2],invViewMatrix[3])));
    eyeRay_d.y = dot(temp, ((float4)(invViewMatrix[4],invViewMatrix[5],invViewMatrix[6],invViewMatrix[7])));
    eyeRay_d.z = dot(temp, ((float4)(invViewMatrix[8],invViewMatrix[9],invViewMatrix[10],invViewMatrix[11])));
    eyeRay_d.w = 0.0f;

    // find intersection with box
	float tnear, tfar;
	int hit = intersectBox(eyeRay_o, eyeRay_d, boxMin, boxMax, &tnear, &tfar);
    if (!hit) {
        if ((x < imageW) && (y < imageH)) {
            // write output color
            uint i =(y * imageW) + x;
            d_output[i] = 0;
        }
        return;
    }
	if (tnear < 0.0f) tnear = 0.0f;     // clamp to near plane

    // march along ray from back to front, accumulating color
    temp = (float4)(0.0f,0.0f,0.0f,0.0f);
    float t = tfar;

    for(uint i=0; i<maxSteps*5; i++) {		
        float4 pos = eyeRay_o + eyeRay_d*t;
        pos = pos*0.5f+0.5f;    // map position to [0, 1] coordinates
        // read from 3D texture        
  
        //float sample = (float)(read_imageui(volume, volumeSampler, pos).x)/255;
#ifdef TYPE_FLOAT1
		float sample = read_imagef(volume, volumeSampler, pos).x;
#elif TYPE_UINT1
		float sample = (float)(read_imageui(volume, volumeSampler, pos).x)/255;
#elif TYPE_INT1
		float sample = (float)(read_imagei(volume, volumeSampler, pos).x)/255;
#endif
        // lookup in transfer function texture
        float2 transfer_pos = (float2)((sample-transferOffset)*transferScale, 0.5f);
        float4 col = read_imagef(transferFunc, transferFuncSampler, transfer_pos);
        // accumulate result
        float a = col.w*density;
        temp = mix(temp, col, (float4)(a, a, a, a));
		
#if defined(VOL2) || defined(VOL3) || defined(VOL4) || defined(VOL5)
		pos= (float4)(eyeRay_o.x+0.0f, eyeRay_o.y+0.0f, eyeRay_o.z, eyeRay_o.w+0.0f) + eyeRay_d*t; //Mehdi
		if( (pos.x>=-1.0f) && (pos.x<=1.0f) && (pos.y>=-1.0f) && (pos.y<=1.0f) )
		{
			pos= pos*0.5f+0.5f; //Mehdi

#ifdef TYPE_FLOAT2
			float sample = read_imagef(volume2, volumeSampler, pos).x;
#elif TYPE_UINT2
			float sample = (float)(read_imageui(volume2, volumeSampler, pos).x)/255;
#elif TYPE_INT2
			float sample = (float)(read_imagei(volume2, volumeSampler, pos).x)/255;
#endif

			transfer_pos = (float2)((sample-transferOffset)*transferScale, 0.5f); //Mehdi
			col = read_imagef(transferFunc, transferFuncSampler, transfer_pos); //Mehdi
			a = col.w*density; //Mehdi
			temp = mix(temp, col, (float4)(a, a, a, a)); //Mehdi
		}
#endif //VOL2
		
#if defined(VOL3) || defined(VOL4) || defined(VOL5)
		pos= (float4)(eyeRay_o.x+0.0f, eyeRay_o.y+0.0f, eyeRay_o.z, eyeRay_o.w+0.0f) + eyeRay_d*t; //Mehdi
		if( (pos.x>=-1.0f) && (pos.x<=1.0f) && (pos.y>=-1.0f) && (pos.y<=1.0f) )
		{
			pos= pos*0.5f+0.5f; //Mehdi

#ifdef TYPE_FLOAT3
			float sample = read_imagef(volume3, volumeSampler, pos).x;
#elif TYPE_UINT3
			float sample = (float)(read_imageui(volume3, volumeSampler, pos).x)/255;
#elif TYPE_INT3
			float sample = (float)(read_imagei(volume3, volumeSampler, pos).x)/255;
#endif

			transfer_pos = (float2)((sample-transferOffset)*transferScale, 0.5f); //Mehdi
			col = read_imagef(transferFunc, transferFuncSampler, transfer_pos); //Mehdi
			a = col.w*density; //Mehdi
			temp = mix(temp, col, (float4)(a, a, a, a)); //Mehdi
		}
#endif //VOL3

#if defined(VOL4) || defined(VOL5)
		pos= (float4)(eyeRay_o.x+0.0f, eyeRay_o.y+0.0f, eyeRay_o.z, eyeRay_o.w+0.0f) + eyeRay_d*t; //Mehdi
		if( (pos.x>=-1.0f) && (pos.x<=1.0f) && (pos.y>=-1.0f) && (pos.y<=1.0f) )
		{
			pos= pos*0.5f+0.5f; //Mehdi

#ifdef TYPE_FLOAT4
			float sample = read_imagef(volume4, volumeSampler, pos).x;
#elif TYPE_UINT4
			float sample = (float)(read_imageui(volume4, volumeSampler, pos).x)/255;
#elif TYPE_INT4
			float sample = (float)(read_imagei(volume4, volumeSampler, pos).x)/255;
#endif

			transfer_pos = (float2)((sample-transferOffset)*transferScale, 0.5f); //Mehdi
			col = read_imagef(transferFunc, transferFuncSampler, transfer_pos); //Mehdi
			a = col.w*density; //Mehdi
			temp = mix(temp, col, (float4)(a, a, a, a)); //Mehdi
		}
#endif //VOL4

#if defined(VOL5)
		pos= (float4)(eyeRay_o.x+0.0f, eyeRay_o.y+0.0f, eyeRay_o.z, eyeRay_o.w+0.0f) + eyeRay_d*t; //Mehdi
		if( (pos.x>=-1.0f) && (pos.x<=1.0f) && (pos.y>=-1.0f) && (pos.y<=1.0f) )
		{
			pos= pos*0.5f+0.5f; //Mehdi

#ifdef TYPE_FLOAT5
			float sample = read_imagef(volume5, volumeSampler, pos).x;
#elif TYPE_UINT5
			float sample = (float)(read_imageui(volume5, volumeSampler, pos).x)/255;
#elif TYPE_INT5
			float sample = (float)(read_imagei(volume5, volumeSampler, pos).x)/255;
#endif

			transfer_pos = (float2)((sample-transferOffset)*transferScale, 0.5f); //Mehdi
			col = read_imagef(transferFunc, transferFuncSampler, transfer_pos); //Mehdi
			a = col.w*density; //Mehdi
			temp = mix(temp, col, (float4)(a, a, a, a)); //Mehdi
		}		
#endif //VOL5
		
        t -= tstep;
        if (t < tnear) break;
    }
    temp *= brightness;

    if ((x < imageW) && (y < imageH)) {
        // write output color
        uint i =(y * imageW) + x;
        d_output[i] = rgbaFloatToInt(temp);
    }
}

