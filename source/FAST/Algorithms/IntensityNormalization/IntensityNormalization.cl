__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void scaleImage2D(
        __read_only image2d_t input,
        __write_only image2d_t output,
        __private float min,
        __private float max,
        __private float low,
        __private float high
        ) {
    const int2 pos = {get_global_id(0), get_global_id(1)};
    int dataType = get_image_channel_data_type(input);
    
    float4 value;
    if(dataType == CLK_FLOAT) {
        value = read_imagef(input, sampler, pos);
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        value = convert_float4(read_imageui(input, sampler, pos));
    } else {
        value = convert_float4(read_imagei(input, sampler, pos));
    }
    value = (value - min) / (max - min);
    value = value*(high - low) + low;
    
    write_imagef(output, pos, value);
}

#ifdef fast_3d_image_writes
__kernel void scaleImage3D(
        __read_only image3d_t input,
        __write_only image3d_t output,
        __private float min,
        __private float max,
        __private float low,
        __private float high
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    int dataType = get_image_channel_data_type(input);
    
    float4 value;
    if(dataType == CLK_FLOAT) {
        value = read_imagef(input, sampler, pos);
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        value = convert_float4(read_imageui(input, sampler, pos));
    } else {
        value = convert_float4(read_imagei(input, sampler, pos));
    }
    value = (value - min) / (max - min);
    value = value*(high - low) + low;
    
    write_imagef(output, pos, value);
}
#else
__kernel void scaleImage3D(
        __read_only image3d_t input,
        __global float* output,
        __private float min,
        __private float max,
        __private float low,
        __private float high,
        __private uint outputChannels
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    int dataType = get_image_channel_data_type(input);
    
    float4 value;
    if(dataType == CLK_FLOAT) {
        value = read_imagef(input, sampler, pos);
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16 || dataType == CLK_UNSIGNED_INT32) {
        value = convert_float4(read_imageui(input, sampler, pos));
    } else {
        value = convert_float4(read_imagei(input, sampler, pos));
    }
    value = (value - min) / (max - min);
    value = value*(high - low) + low;
    
    output[(pos.x + pos.y*get_image_width(input) + pos.z*get_image_width(input)*get_image_height(input))*outputChannels] = value.x;
    if(outputChannels > 1)
        output[(pos.x + pos.y*get_image_width(input) + pos.z*get_image_width(input)*get_image_height(input))*outputChannels + 1] = value.y;
    if(outputChannels > 2)
        output[(pos.x + pos.y*get_image_width(input) + pos.z*get_image_width(input)*get_image_height(input))*outputChannels + 2] = value.z;
    if(outputChannels > 3)
        output[(pos.x + pos.y*get_image_width(input) + pos.z*get_image_width(input)*get_image_height(input))*outputChannels + 3] = value.w;
}
#endif