__constant sampler_t interpolationSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

__kernel void render(
        __read_only image2d_t image,
        __global float* PBOread,
        __global float* PBOwrite,
        __private float imageSpacingX,
        __private float imageSpacingY,
        __private float PBOspacing,
        __private float level,
        __private float window
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
        float4 value;
        int dataType = get_image_channel_data_type(image);
        switch(dataType) {
            case CLK_FLOAT:
                value = read_imagef(image, interpolationSampler, imagePosition);
            break;
            case CLK_UNSIGNED_INT8:
            case CLK_UNSIGNED_INT16:
                value = convert_float4(read_imageui(image, interpolationSampler, imagePosition));
            break;
            case CLK_SIGNED_INT8:
            case CLK_SIGNED_INT16:
                value = convert_float4(read_imagei(image, interpolationSampler, imagePosition));
            break;
        }
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
