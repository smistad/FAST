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
        color = (float4)(0.0f,0.0f,0.0f,0.0f);
        // Look for neighbors instead
        float highestConfidence = minConfidence;
        for(int a = -1; a <= 1; ++a) {
            for(int b = -1; b <= 1; ++b) {
                int2 nPos = {position.x + a, position.y + b};
                // Out of bounds check:
                if(nPos.x < 0 || nPos.y < 0 || nPos.x >= get_global_size(0) || nPos.y >= get_global_size(1))
                    continue;
                for(int channel = 0; channel < channels; ++channel) {
                    float intensity = inputTensor[(nPos.x + nPos.y*get_global_size(0))*channels + channel];

                    if(intensity >= highestConfidence) {
                        float4 colorToUse = vload4(channel, colors);
                        color = colorToUse;
                        color.w = 0.0f; // Set opacity to zero
                        highestConfidence = intensity;
                    }
                }
            }
        }
    } else {
        color.w *= maxOpacity;
    }
    write_imagef(output, position, color);
}

