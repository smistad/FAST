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

    write_imagef(texture, (int2)(imagePosition.x, get_image_height(image) - imagePosition.y - 1), color);
}

__kernel void render2D(
        __read_only image2d_t image,
        __global float* PBOread,
        __global float* PBOwrite,
        __private float imageSpacingX,
        __private float imageSpacingY,
        __private float PBOspacing,
        __global float* colors,
        __global char* fillArea,
        __private int borderRadius
        ) {
    const int2 PBOposition = {get_global_id(0), get_global_id(1)};
    const int linearPosition = PBOposition.x + (get_global_size(1) - 1 - PBOposition.y)*get_global_size(0);
    
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
        // Read image and put value in PBO
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
                        float2 offset = {a, b};
                        if(length(offset) < borderRadius && read_imageui(image, sampler, imagePosition + offset).x != label) {
                            getColor = 1;
                        }
                    }
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

float4 transformPosition(__constant float* transform, int2 PBOposition) {
    float4 position = {PBOposition.x, PBOposition.y, 0, 1};
    float transformedPosition[4];
    //printf("PBO pos: %d %d\n", PBOposition.x, PBOposition.y);
    
    // Multiply with transform
    // transform is column major
    for(int i = 0; i < 4; i++) {
        float sum = 0;
        sum += transform[i + 0*4]*position.x;
        sum += transform[i + 1*4]*position.y;
        sum += transform[i + 2*4]*position.z;
        sum += transform[i + 3*4]*position.w;
        transformedPosition[i] = sum;
    }
    //printf("Transformed pos: %f %f %f\n", transformedPosition[0], transformedPosition[1], transformedPosition[2]);
    
    float4 result = {transformedPosition[0], transformedPosition[1], transformedPosition[2], transformedPosition[3]};
    return result;
}

__kernel void render3D(
        __read_only image3d_t image,
        __global float* PBOread,
        __global float* PBOwrite,
        __constant float* transform,
        __global float* colors,
        __global char* fillArea
        ) {
    const int2 PBOposition = {get_global_id(0), get_global_id(1)};
    const int linearPosition = PBOposition.x + (get_global_size(1) - 1 - PBOposition.y)*get_global_size(0);
    

    float4 imagePosition = transformPosition(transform, PBOposition);
    imagePosition.w = 1;
       
    float4 color;
    char useBackground = 1;

    // Is image within bounds?
    if(imagePosition.x < get_image_width(image) && imagePosition.y < get_image_height(image) && imagePosition.z < get_image_depth(image) &&
        imagePosition.x >= 0 && imagePosition.y >= 0 && imagePosition.z >= 0
        ) {
        // Read image and put value in PBO
        uint label = read_imageui(image, sampler, imagePosition).x;
        
        if(label > 0) {
            // Fill area check
            char getColor = 0;
            // Check neighbors
            // If any neighbors have a different label, we are at the border

            if(fillArea[label] == 1) {
                getColor = 1;
            } else {
                for(int a = -1; a < 2; a++) {
                for(int b = -1; b < 2; b++) {
                for(int c = -1; c < 2; c++) {
                    if(read_imageui(image, sampler, imagePosition + (float4)(a,b,c,0)).x != label) {
                        getColor = 1;
                    }
                }}}
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