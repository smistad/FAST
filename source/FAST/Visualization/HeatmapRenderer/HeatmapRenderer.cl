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

    float4 color = {1.0f, 1.0f, 1.0f, 0.0f};
    // TODO blend colorsd
    for(int channel = 0; channel < channels; ++channel) { // TODO skip first?
        float intensity = inputTensor[(position.x + position.y*get_global_size(0))*channels + channel];
        //printf("confidence of channel %d: %f\n", channel, intensity);
        intensity = clamp(intensity, 0.0f, 1.0f);

        if(intensity < minConfidence) {
            intensity = 0;
        } else {
            intensity *= maxOpacity;
            float3 colorToUse = vload3(channel, colors);
            color = colorToUse.xyzz;
            color.w = intensity;
            color = clamp(color, 0.0f, 1.0f);
            //printf("%f %f %f\n", color.x, color.y, color.z);
        }
    }
    write_imagef(output, (int2)(position.x, get_image_height(output) - position.y - 1), color);
}

