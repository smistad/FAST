__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void renderToTexture(
        __read_only image2d_t image,
        __write_only image2d_t texture,
        __global float* colors,
        __global char* fillArea,
        __private int borderRadius,
        __private float opacity
        ) {
    const int2 imagePosition = {get_global_id(0), get_global_id(1)};

    // Default color
    float4 color = {1, 1, 1, 0};

    // Read segmentation label
    uint label = read_imageui(image, sampler, imagePosition).x;

    if(label > 0) {
        // Fill area check
        char getColor = 0;
        if(fillArea[label] == 1) {
            getColor = 1;
        } else {
            // Check neighbors
            // If any neighbors have a different label, we are at the border
            for(int a = -borderRadius; a <= borderRadius; ++a) {
                for(int b = -borderRadius; b <= borderRadius; ++b) {
                    int2 offset = {a, b};
                    if(read_imageui(image, sampler, imagePosition + offset).x != label) {
                        if(borderRadius == 1 || length(convert_float2(offset)) < borderRadius) {
                            getColor = 1;
                        }
                    }
                }
            }
        }
        if(getColor == 1) {
            // TODO some out of bounds check here on colors?
            color.xyz = vload3(label, colors);
            color.w = opacity;
        }
    }

    write_imageui(texture, imagePosition, convert_uint4(round(color*255)));
}
