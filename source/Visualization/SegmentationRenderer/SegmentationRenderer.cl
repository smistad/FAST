__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void render(
        __read_only image2d_t image,
        __global float* PBOread,
        __global float* PBOwrite,
        __private float imageSpacingX,
        __private float imageSpacingY,
        __private float PBOspacing,
        __global float* colors
        ) {
    const int2 PBOposition = {get_global_id(0), get_global_id(1)};
    const int linearPosition = PBOposition.x + PBOposition.y*get_global_size(0);
    
    float2 imagePosition = convert_float2(PBOposition)*PBOspacing;
    imagePosition.x /= imageSpacingX;
    imagePosition.y /= imageSpacingY;
    
    
    // Is image within bounds?
    if(imagePosition.x < get_image_width(image) && imagePosition.y < get_image_height(image)) {
        imagePosition.y = get_image_height(image) - imagePosition.y - 1; // Flip image vertically
        // Read image and put value in PBO
        uint label = read_imageui(image, sampler, imagePosition).x;
        
        if(label > 0) {
            // TODO some out of bounds check here on colors?
            float4 color;
            color.xyz = vload3(label, colors);
            color.w = 1.0f;

            vstore4(color, linearPosition, PBOwrite);
        } else {
            // Read PBO
            float4 value = vload4(linearPosition, PBOread);
            // Write to PBU
            vstore4(value, linearPosition, PBOwrite);
        }
    } else {
        // Read PBO
        float4 value = vload4(linearPosition, PBOread);
        // Write to PBU
        vstore4(value, linearPosition, PBOwrite);
    }
}