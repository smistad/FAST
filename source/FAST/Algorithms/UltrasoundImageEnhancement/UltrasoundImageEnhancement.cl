__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void enhance(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private int reject
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float intensity = (float)read_imageui(input, sampler, pos).x;
    if(intensity < reject)
        intensity = 0.0f;
    // Apply a nonlinear S-curve
    float k = -0.2;
    intensity = (intensity/255.0f)*2.0f-1.0f;
    uchar value  = (uchar)round(255*(((intensity - k*intensity)/(k - 2*k*fabs(intensity)+1))+1.0f)/2.0f);
    //uchar value = intensity;
    uint4 color = {value, value, value, 255};
    // Apply a hint of blue
    if(value > 0) {
        color.y -= 1;
        color.z += 4;
    }
    color = clamp(color, (uint)0, (uint)255);

    write_imageui(output, pos, color);
}