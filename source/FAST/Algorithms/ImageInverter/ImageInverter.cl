__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void invert3D(
        __read_only image3d_t input,
        __global DATA_TYPE* output,
        __private float min,
        __private float max,
        __private uint outputChannels
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    int dataType = get_image_channel_data_type(input);

    float4 value;
    if(dataType == CLK_FLOAT) {
        value = read_imagef(input, sampler, pos);
    } else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16) {
        value = convert_float4(read_imageui(input, sampler, pos));
    } else {
        value = convert_float4(read_imagei(input, sampler, pos));
    }
    value = (max - min) - value;

    output[(pos.x + pos.y*get_image_width(input) + pos.z*get_image_width(input)*get_image_height(input))*outputChannels] = value.x;
    if(outputChannels > 1)
        output[(pos.x + pos.y*get_image_width(input) + pos.z*get_image_width(input)*get_image_height(input))*outputChannels + 1] = value.y;
    if(outputChannels > 2)
        output[(pos.x + pos.y*get_image_width(input) + pos.z*get_image_width(input)*get_image_height(input))*outputChannels + 2] = value.z;
    if(outputChannels > 3)
        output[(pos.x + pos.y*get_image_width(input) + pos.z*get_image_width(input)*get_image_height(input))*outputChannels + 3] = value.w;
}
