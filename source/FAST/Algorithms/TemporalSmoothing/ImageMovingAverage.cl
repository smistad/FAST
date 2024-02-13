__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

float getPixelAsFloat(__read_only image2d_t image, int2 pos) {
    float value;
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT) {
        value = read_imagef(image, sampler, pos).x;
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        value = read_imageui(image, sampler, pos).x;
    } else {
        value = read_imagei(image, sampler, pos).x;
    }
    return value;
}

__kernel void MAinitialize(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __write_only image2d_t memoryOut,
        __private int frameCount
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float value = getPixelAsFloat(input, pos);
    write_imagef(memoryOut, pos, value);
    int dataType = get_image_channel_data_type(output);
    if(dataType == CLK_FLOAT) {
        write_imagef(output, pos, value);
    } else {
        // output image is of integer type, have to apply rounding
        if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
            write_imageui(output, pos, (uint)value);
        } else {
            write_imagei(output, pos, (int)value);
        }
    }
}

__kernel void MAiteration(
        __read_only image2d_t input,
        __read_only image2d_t memoryIn,
        __read_only image2d_t last,
        __write_only image2d_t output,
        __write_only image2d_t memoryOut,
        __private int frameCount
    ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};

    float newValue = getPixelAsFloat(input, pos);
    float oldMemory = read_imagef(memoryIn, sampler, pos).x;
    float lastValue = getPixelAsFloat(last, pos);
    float result = oldMemory + 1.0f/(float)frameCount * (newValue - lastValue);
    write_imagef(memoryOut, pos, result);

    int dataType = get_image_channel_data_type(output);
    if(dataType == CLK_FLOAT) {
        write_imagef(output, pos, result);
    } else {
        // output image is of integer type, have to apply rounding
        if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
            write_imageui(output, pos, (uint)round(result));
        } else {
            write_imagei(output, pos, (int)round(result));
        }
    }
}
