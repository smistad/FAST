__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

float getPixelAsFloat(__read_only image2d_t image, int2 pos) {
    float value;
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT) {
        value = read_imagef(image, sampler, pos).x;
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        value = read_imageui(image, sampler, pos).x;
    } else {
        value = read_imagei(image, sampler, pos).x;
    }
    return value;
}

__kernel void WMAinitialize(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __write_only image2d_t memoryOut,
        __private int frameCount
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float value = getPixelAsFloat(input, pos);
    float sum = value*frameCount;
    float numerator = 0.0f;
    for(int i = 0; i < frameCount; ++i) {
        numerator += (i+1)*value;
    }
    float result = numerator / (frameCount*(frameCount + 1.0f)/2.0f);

    write_imagef(memoryOut, pos, (float4)(sum, numerator, 0, 0));

    int dataType = get_image_channel_data_type(output);
    if(dataType == CLK_FLOAT) {
        write_imagef(output, pos, result);
    } else {
        // output image is of integer type, have to apply rounding
        if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            write_imageui(output, pos, (uint)round(result));
        } else {
            write_imagei(output, pos, (int)round(result));
        }
    }
}

__kernel void WMAiteration(
        __read_only image2d_t input,
        __read_only image2d_t memoryIn,
        __read_only image2d_t last,
        __write_only image2d_t output,
        __write_only image2d_t memoryOut,
        __private int frameCount
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float newValue = getPixelAsFloat(input, pos);
    float2 oldMemory = read_imagef(memoryIn, sampler, pos).xy;
    float newTotal = oldMemory.x + newValue - getPixelAsFloat(last, pos);
    float newNumerator = oldMemory.y + frameCount*newValue - oldMemory.x;
    write_imagef(memoryOut, pos, (float4)(newTotal, newNumerator, 0, 0));

    int dataType = get_image_channel_data_type(output);
    if(dataType == CLK_FLOAT) {
        write_imagef(output, pos, newNumerator / (frameCount*(frameCount + 1.0f)/2.0f));
    } else {
        // output image is of integer type, have to apply rounding
        if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
            write_imageui(output, pos, (uint)round(newNumerator / (frameCount*(frameCount + 1.0f)/2.0f)));
        } else {
            write_imagei(output, pos, (int)round(newNumerator / (frameCount*(frameCount + 1.0f)/2.0f)));
        }
    }
}
