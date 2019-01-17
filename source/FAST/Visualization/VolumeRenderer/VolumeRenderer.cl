
const sampler_t transferFuncSampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
const sampler_t volumeSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
/*
__kernel void volumeRender(
        __read_only image3d_t volume,
        __write_only image2d_t framebuffer
        ) {
    int2 position = {get_global_id(0), get_global_id(1)};
    float4 value = {0.0f, 1.0f, 0.0f, 1.0f};
    write_imagef(framebuffer, position, value);
}
 */


#define tstep 0.5f

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

__kernel void volumeRender(
    __read_only image3d_t volume,
    __write_only image2d_t framebuffer,
    __read_only image2d_t transferFunc,
    float density,
    float brightness,
    float transferOffset,
    float transferScale,
    __constant float* invViewMatrix,
    __constant float* modelMatrix
    ) {

    const int imageW = get_image_width(framebuffer);
    const int imageH = get_image_height(framebuffer);
    const uint x = get_global_id(0);
    const uint y = get_global_id(1);

    float u = ((x / (float) imageW)*2.0f-1.0f);
    float v = ((y / (float) imageH)*2.0f-1.0f);

    //float tstep = 0.01f;
    const float4 boxMin = (float4)(0.0f, 0.0f, 0.0f,1.0f);
    const float4 boxMax = (float4)(get_image_width(volume), get_image_height(volume), get_image_depth(volume), 1.0f);
    const int maxSteps = max(max(boxMax.x, boxMax.y), boxMax.z);

    // calculate eye ray in world space
    float4 eyeRay_o;
    float4 eyeRay_d;

    // Ray origin is at the camera center
    eyeRay_o = (float4)(invViewMatrix[12], invViewMatrix[13], invViewMatrix[14], 1.0f);

    // Ray destination is the window coordinate u,v and -2 in z direction
    float4 temp = normalize(((float4)(u, v, -2.0f,0.0f)));
    // Go from normalized coords to
    eyeRay_d.x = dot(temp, ((float4)(invViewMatrix[0],invViewMatrix[4],invViewMatrix[8], 0)));
    eyeRay_d.y = dot(temp, ((float4)(invViewMatrix[1],invViewMatrix[5],invViewMatrix[9], 0)));
    eyeRay_d.z = dot(temp, ((float4)(invViewMatrix[2],invViewMatrix[6],invViewMatrix[10], 0)));
    eyeRay_d.w = 1.0f;

    // find intersection with box
    float tnear, tfar;
    int hit = intersectBox(eyeRay_o, eyeRay_d, boxMin, boxMax, &tnear, &tfar);
    if (!hit) {
        if ((x < imageW) && (y < imageH)) {
            // write output color
            uint i =(y * imageW) + x;
            write_imagef(framebuffer, (int2)(x, y), (float4)(0,0,0,0));
        }
        return;
    }

    if (tnear < 0.0f)
        tnear = 0.0f;     // clamp to near plane

    // march along ray from back to front, accumulating color
    temp = (float4)(0.0f,0.0f,0.0f,0.0f);
    float t = tfar;

    for(uint i = 0; i < maxSteps; i++) {
        float4 pos = eyeRay_o + eyeRay_d*t;
        //pos = pos*0.5f+0.5f;    // map position to [0, 1] coordinates

        // read from 3D texture
        float sample = clamp(0.0f, 1.0f, ((float)read_imagei(volume, volumeSampler, pos).x+1024.0f)/4000.0f);

        // lookup in transfer function texture
        float2 transfer_pos = (float2)((sample-transferOffset)*transferScale, 0.5f);
        float4 col = read_imagef(transferFunc, transferFuncSampler, transfer_pos);

        // accumulate result
        //float a = col.w*density;
        //temp = mix(temp, col, (float4)(a, a, a, a));
        temp = max(temp, (float4)(sample, sample, sample, sample));

        t -= tstep;
        if (t < tnear)
            break;
    }
    temp *= brightness;

    if ((x < imageW) && (y < imageH)) {
        //printf("%f %d %d\n", temp.x, x, y);
        // write output color
        write_imagef(framebuffer, (int2)(x, y), temp);
    }

}