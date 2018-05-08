__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void renderToTexture(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float red,
        __private float green,
        __private float blue,
        __private float minConfidence,
        __private float maxOpacity
) {
    const int2 position = {get_global_id(0), get_global_id(1)};

    float intensity = read_imagef(input, sampler, position).x;

    float4 color = {0.0f, 0.0f, 0.0f, 0.0f};
    if(intensity < minConfidence)
        intensity = 0;
    // Avoid multiple colors on top of eachother
    /*
    if(color.x != color.y || color.y != color.z || color.x != color.z) {
        if(color.w < intensity) {
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
    */
        intensity *= maxOpacity;
        color = color + intensity * (float4)(red, green, blue, 1.0f);
        color.w = intensity;
        color = clamp(color, 0.0f, 1.0f);
    //}

    write_imagef(output, (int2)(position.x, get_image_height(output) - position.y - 1), color);
}

