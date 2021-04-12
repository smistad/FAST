__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

float3 hueToRGB(float H) {
    float R = fabs(H * 6.0f - 3.0f) - 1;
    float G = 2 - fabs(H * 6 - 2);
    float B = 2 - fabs(H * 6 - 4);
    return clamp((float3)(R, G, B), 0.0f, 1.0f);
}

__kernel void renderToTexture(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float maxOpacity,
        __private float maxVectorMagnitude
    ) {
    const int2 position = {get_global_id(0), get_global_id(1)};

    const float2 vector = read_imagef(input, sampler, position).xy;

    // Calculate angle of vector
    const float angle = (1.0f + atan2(vector.y, vector.x) / 3.141592f) / 2.0f; // mapping -pi,pi to 0,1

    // Use angle as hue and then convert to RGB
    const float3 color = hueToRGB(angle);

    // Use vector magnitude as opacity
    const float opacity = clamp(length(vector)/maxVectorMagnitude, 0.0f, maxOpacity);

    write_imagef(output, position, (float4)(color.x, color.y, color.z, opacity));
}
