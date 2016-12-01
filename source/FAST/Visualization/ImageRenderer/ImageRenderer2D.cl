__constant sampler_t interpolationSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

float4 readPixelAsFloat(image2d_t image, sampler_t sampler, float2 pos) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT) {
        return read_imagef(image, sampler, pos);
    } else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16) {
        return convert_float4(read_imagei(image, sampler, pos));
    } else {
        return convert_float4(read_imageui(image, sampler, pos));
    }
}

float4 readPixelAsFloat3D(image3d_t image, sampler_t sampler, float4 pos) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT) {
        return read_imagef(image, sampler, pos);
    } else if(dataType == CLK_SIGNED_INT8 || dataType == CLK_SIGNED_INT16) {
        return convert_float4(read_imagei(image, sampler, pos));
    } else {
        return convert_float4(read_imageui(image, sampler, pos));
    }
}

__kernel void render2Dimage(
        __read_only image2d_t image,
        __global float* PBOread,
        __global float* PBOwrite,
        __private float imageSpacingX,
        __private float imageSpacingY,
        __private float PBOspacing,
        __private float level,
        __private float window,
        __private float translationX,
        __private float translationY
        ) {
    const int2 PBOposition = {get_global_id(0), get_global_id(1)};
    const int linearPosition = PBOposition.x + (get_global_size(1) - 1 - PBOposition.y)*get_global_size(0);
    
    float2 imagePosition = convert_float2(PBOposition)*PBOspacing + (float2)(translationX, translationY);
    imagePosition.x /= imageSpacingX;
    imagePosition.y /= imageSpacingY;
    
    
    // Is image within bounds?
    if(imagePosition.x < get_image_width(image) && imagePosition.y < get_image_height(image)) {
        // Read image and put value in PBO
        float4 value = readPixelAsFloat(image, interpolationSampler, imagePosition);

        if(get_image_channel_order(image) == CLK_R) {
            value.y = value.x;
            value.z = value.x;
        }

        value = (value - level + window/2) / window;
        value = clamp(value, 0.0f, 1.0f);
        
        vstore4(value, linearPosition, PBOwrite);
    } else {
        // Read PBO
        float4 value = vload4(linearPosition, PBOread);
        // Write to PBU
        vstore4(value, linearPosition, PBOwrite);
    }
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

__kernel void render3Dimage(
        __read_only image3d_t image,
        __global float* PBOread,
        __global float* PBOwrite,
        __constant float* transform,
        __private float level,
        __private float window
    ) {

    const int2 PBOposition = {get_global_id(0), get_global_id(1)};
    const int linearPosition = PBOposition.x + (get_global_size(1) - 1 - PBOposition.y)*get_global_size(0);
    
    float4 imagePosition = transformPosition(transform, PBOposition);
    imagePosition.w = 1;
     
    // Is image within bounds?
    if(imagePosition.x < get_image_width(image) && imagePosition.y < get_image_height(image) && imagePosition.z < get_image_depth(image) &&
        imagePosition.x >= 0 && imagePosition.y >= 0 && imagePosition.z >= 0
        ) {
        // Read image and put value in PBO
        float4 value = readPixelAsFloat3D(image, interpolationSampler, imagePosition);
        if(get_image_channel_order(image) == CLK_R) {
            value.y = value.x;
            value.z = value.x;
        }

        value = (value - level + window/2) / window;
        value = clamp(value, 0.0f, 1.0f);
        
        vstore4(value, linearPosition, PBOwrite);
    } else {
        // Read PBO
        float4 value = vload4(linearPosition, PBOread);
        // Write to PBU
        vstore4(value, linearPosition, PBOwrite);
    }  
}
