
const sampler_t transferFuncSampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
const sampler_t volumeSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

#define tstep 0.5f

// intersect ray with a box
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm

int intersectBox(float4 r_o, float4 r_d, float4 boxmin, float4 boxmax, float *tnear, float *tfar) {
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

    const int width = get_image_width(framebuffer);
    const int height = get_image_height(framebuffer);

    // Grid position
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    // Normalized grid position
    const float u = ((x / (float) width)*2.0f-1.0f)*((float)width/height); // compensate for aspect ratio != 1
    const float v = ((y / (float) height)*2.0f-1.0f);

    // Bounding box of volume
    const float4 boxMin = (float4)(0.0f, 0.0f, 0.0f,1.0f);
    const float4 boxMax = (float4)(get_image_width(volume), get_image_height(volume), get_image_depth(volume), 1.0f);
    // Maximum depth to cast ray inside the volume
    const int maxSteps = max(max(boxMax.x, boxMax.y), boxMax.z);

    // Calculate ray origin and direction
    float4 rayOrigin;
    float4 rayDirection;

    // Ray origin is at the camera center
    rayOrigin = (float4)(invViewMatrix[12], invViewMatrix[13], invViewMatrix[14], 1.0f);

    // Calculate ray direction which is the direction of the vector (u,v,-2) - (0,0,0)
    float4 temp = normalize(((float4)(u, v, -2.0f,0.0f)));
    // Apply camera rotation on the ray direction vector
    rayDirection.x = dot(temp, ((float4)(invViewMatrix[0],invViewMatrix[4],invViewMatrix[8], 0)));
    rayDirection.y = dot(temp, ((float4)(invViewMatrix[1],invViewMatrix[5],invViewMatrix[9], 0)));
    rayDirection.z = dot(temp, ((float4)(invViewMatrix[2],invViewMatrix[6],invViewMatrix[10], 0)));
    rayDirection.w = 1.0f;

    // Find the distance to where the ray hits the box
    float tnear, tfar;
    int hit = intersectBox(rayOrigin, rayDirection, boxMin, boxMax, &tnear, &tfar);
    if (!hit) {
        // Ray doesn't hit the box at all
        // write output color
        write_imagef(framebuffer, (int2)(x, y), (float4)(0,0,0,0));
        return;
    }

    if (tnear < 0.0f)
        tnear = 0.0f;     // clamp to near plane

    // Traverse along ray from back to front, and keep the maximum intensity
    temp = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float distance = tfar; // Start at tfar
    for(int i = 0; i < maxSteps; ++i) {
        float4 pos = rayOrigin + rayDirection*distance;
        /*
        pos.w = 1;
        temp = pos;
        pos.x = dot(temp, ((float4)(modelMatrix[0],modelMatrix[4],modelMatrix[8], modelMatrix[12])));
        pos.y = dot(temp, ((float4)(modelMatrix[1],modelMatrix[5],modelMatrix[9], modelMatrix[13])));
        pos.z = dot(temp, ((float4)(modelMatrix[2],modelMatrix[6],modelMatrix[10], modelMatrix[14])));
         */

        // read from 3D texture
        float sample = clamp(0.0f, 1.0f, ((float)read_imagei(volume, volumeSampler, pos).x+1024.0f)/4000.0f);

        temp = max(temp, (float4)(sample, sample, sample, sample));

        distance -= tstep;
        if(distance < tnear) // passed the box, stop
            break;
    }

    // write output color
    write_imagef(framebuffer, (int2)(x, y), temp);

}