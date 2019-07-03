__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void renderToTexture(
        __global float* inputTensor,
        __write_only image2d_t output,
        __constant float* colors,
        __private float minConfidence,
        __private float maxOpacity,
        __private int channels
) {
    const int2 position = {get_global_id(0), get_global_id(1)};

    float4 color = {0.0f, 0.0f, 0.0f, 0.0f};
    for(int channel = 0; channel < channels; ++channel) {
        float intensity = inputTensor[(position.x + position.y*get_global_size(0))*channels + channel];
        intensity = clamp(intensity, 0.0f, 1.0f);

        if(intensity >= minConfidence) {
            float4 colorToUse = vload4(channel, colors);
            color += colorToUse*intensity;
        }
    }
    color = clamp(color, 0.0f, 1.0f);
    if(color.w == 0) { // none with intensity >= minConfidence or 0 found
        color = (float4)(1.0f,1.0f,1.0f,0.0f);
    } else {
        color.w *= maxOpacity;
    }
    write_imagef(output, (int2)(position.x, get_image_height(output) - position.y - 1), color);
}

