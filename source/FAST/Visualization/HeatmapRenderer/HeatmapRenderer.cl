__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void render2D(
        __read_only image2d_t image,
        __global float* PBOread,
        __global float* PBOwrite,
        __private float imageSpacingX,
        __private float imageSpacingY,
        __private float PBOspacing,
        __private float red,
        __private float green,
        __private float blue,
        __private float minConfidence,
        __private float maxOpacity
) {
    const int2 PBOposition = {get_global_id(0), get_global_id(1)};
    const int linearPosition = PBOposition.x + (get_global_size(1) - 1 - PBOposition.y)*get_global_size(0);

    float2 imagePosition = convert_float2(PBOposition)*PBOspacing;
    imagePosition.x /= imageSpacingX;
    imagePosition.y /= imageSpacingY;
    imagePosition = round(imagePosition);

    float4 color = vload4(linearPosition, PBOread);

    float intensity = read_imagef(image, sampler, imagePosition).x;
    if(intensity < minConfidence)
        intensity = 0;
    // Avoid multiple colors on top of eachother
    if(color.x != color.y || color.y != color.z || color.x != color.z) {
        if(color.w < intensity) {
            //intensity = min(intensity, maxOpacity);
            intensity *= maxOpacity;
            float lowest = min(color.x, min(color.y, color.z));
            color.x = lowest;
            color.y = lowest;
            color.z = lowest;
            color = color + intensity * (float4)(red, green, blue, 1.0f);
            color.w = intensity;
            color = clamp(color, 0.0f, 1.0f);
        }
    } else {
        //intensity = min(intensity, maxOpacity);
        intensity *= maxOpacity;
        color = color + intensity * (float4)(red, green, blue, 1.0f);
        color.w = intensity;
        color = clamp(color, 0.0f, 1.0f);
    }


    // Write to PBO
    vstore4(color, linearPosition, PBOwrite);
}

