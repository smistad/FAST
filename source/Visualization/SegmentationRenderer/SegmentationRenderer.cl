__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void renderBorder(
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
    imagePosition = round(imagePosition);
    
    float2 offsets[8] = {
            {1, 0},
            {0, 1},
            {1, 1},
            {-1, 0},
            {0, -1},
            {-1, -1},
            {-1, 1},
            {1, -1}
    };
    
    float4 color;
    char useBackground = 1;

    // Is image within bounds?
    if(imagePosition.x < get_image_width(image) && imagePosition.y < get_image_height(image)) {
        imagePosition.y = get_image_height(image) - imagePosition.y - 1; // Flip image vertically
        // Read image and put value in PBO
        uint label = read_imageui(image, sampler, imagePosition).x;
        
        if(label > 0) {
            // Fill area check
            char getColor = 0;
            // Check neighbors
            // If any neighbors have a different label, we are at the border
            for(char n = 0; n < 8; n++) {
                if(read_imageui(image, sampler, imagePosition + offsets[n]).x != label) {
                    getColor = 1;
                }
            }
            if(getColor == 1) {
                useBackground = 0;
                // TODO some out of bounds check here on colors?
                color.xyz = vload3(label, colors);
                color.w = 1.0f;
            }
        }
    }
    
    if(useBackground == 1) {
        color = vload4(linearPosition, PBOread);
    }
    
    // Write to PBO
    vstore4(color, linearPosition, PBOwrite);
}


__kernel void renderArea(
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
    imagePosition = round(imagePosition);
       
    float4 color;
    char useBackground = 1;

    // Is image within bounds?
    if(imagePosition.x < get_image_width(image) && imagePosition.y < get_image_height(image)) {
        imagePosition.y = get_image_height(image) - imagePosition.y - 1; // Flip image vertically
        // Read image and put value in PBO
        uint label = read_imageui(image, sampler, imagePosition).x;
        
        if(label > 0) {
            useBackground = 0;
            // TODO some out of bounds check here on colors?
            color.xyz = vload3(label, colors);
            color.w = 1.0f;
        }
    }
    
    if(useBackground == 1) {
        color = vload4(linearPosition, PBOread);
    }
    
    // Write to PBO
    vstore4(color, linearPosition, PBOwrite);
}